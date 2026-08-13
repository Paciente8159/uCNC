/*
	Name: esp8266_lwip.c
	Description: Event-driven lwIP TCP socket backend for the ESP8266 (µCNC socket device).

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 20-07-2026

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

#if (MCU == MCU_ESP8266)
#ifdef ENABLE_SOCKETS

#include <lwip/opt.h>
#include <lwip/tcp.h>
#include <lwip/ip_addr.h>
#include <lwip/inet.h>
#include <lwip/err.h>
#include <lwip/mem.h>
#include <lwip/pbuf.h>
#include <string.h>
#include <errno.h>
#include "../../../modules/net/socket.h"

#define MAX_BSD_SOCKETS \
	(MAX_SOCKETS * (SOCKET_MAX_CLIENTS + 1))

typedef enum
{
	SOCK_UNUSED = 0,
	SOCK_BOUND,
	SOCK_LISTEN,
	SOCK_CONNECTED,
	SOCK_CLOSING
} sock_state_t;

typedef struct
{
	sock_state_t state;
	uint32_t generation;

	/* lwIP PCB owned by this backend */
	struct tcp_pcb *pcb;

	/*
	 * One outstanding lwIP receive pbuf chain.
	 *
	 * We deliberately retain at most one receive chain.
	 * If another pbuf arrives before this one is consumed,
	 * recv_cb() returns ERR_MEM and lwIP will retry later.
	 */
	struct pbuf *rx_buf;
	u16_t rx_off;
	u32_t rx_len;

	/* Deferred µCNC events */
	bool event_pending;
	bool event_connected;
	bool event_peer_closed;
	bool event_error;
	bool event_writable;

	/* TX state */
	bool write_blocked;

	/*
	 * Local close requested by µCNC.
	 *
	 * If tcp_close() temporarily returns ERR_MEM, service()
	 * retries the graceful close later.
	 */
	bool local_close;

	/* µCNC socket handles */
	socket_handle_t srv_handle;
	socket_handle_t client_handle;

} bsd_sock_t;

static bsd_sock_t socks[MAX_BSD_SOCKETS];
static uint32_t sock_generations[MAX_BSD_SOCKETS];
static const socket_device_events_t *esp8266_socket_events;

/*
 * service() may be entered recursively from an application callback.  RX and
 * accept delivery are restricted to the outermost pass so rx_dispatch_buffer
 * cannot be overwritten while data() is still using it.
 */
static unsigned int service_depth;
static char rx_dispatch_buffer[SOCKET_MAX_DATA_SIZE + 1];

static void *sock_arg_from_index(int idx)
{
	return (void *)(intptr_t)(idx + 1);
}

static int sock_index_from_arg(void *arg)
{
	intptr_t value = (intptr_t)arg;

	if (value <= 0 || value > MAX_BSD_SOCKETS)
	{
		return -1;
	}

	return (int)(value - 1);
}

static int alloc_sock(void)
{
	for (int i = 0; i < MAX_BSD_SOCKETS; i++)
	{
		if (socks[i].state == SOCK_UNUSED)
		{
			return i;
		}
	}

	return -1;
}

static void init_sock(bsd_sock_t *s, int idx)
{
	uint32_t generation = ++sock_generations[idx];

	/* Keep zero reserved for reset/unallocated entries. */
	if (generation == 0)
	{
		generation = ++sock_generations[idx];
	}

	memset(s, 0, sizeof(*s));
	s->generation = generation;
	s->srv_handle = SOCKET_INVALID_HANDLE;
	s->client_handle = SOCKET_INVALID_HANDLE;
}

static void free_rx_chain(bsd_sock_t *s)
{
	if (!s)
	{
		return;
	}

	if (s->rx_buf)
	{
		pbuf_free(s->rx_buf);
		s->rx_buf = NULL;
	}

	s->rx_len = 0;
	s->rx_off = 0;
}

static void reset_sock(bsd_sock_t *s)
{
	if (!s)
	{
		return;
	}

	free_rx_chain(s);

	memset(s, 0, sizeof(*s));

	s->state = SOCK_UNUSED;
	s->srv_handle = SOCKET_INVALID_HANDLE;
	s->client_handle = SOCKET_INVALID_HANDLE;
}

static bool same_client(
	const bsd_sock_t *s,
	uint32_t generation,
	socket_handle_t handle,
	const struct tcp_pcb *pcb)
{
	return s->state == SOCK_CONNECTED &&
		!s->local_close &&
		s->generation == generation &&
		s->client_handle == handle &&
		s->pcb == pcb;
}

static void refresh_event_pending(bsd_sock_t *s)
{
	if (s->state == SOCK_UNUSED)
	{
		s->event_pending = false;
		return;
	}

	s->event_pending =
		(s->local_close && s->state == SOCK_CLOSING) ||
		s->event_error ||
		s->event_connected ||
		(s->rx_buf && s->rx_len) ||
		s->event_peer_closed ||
		s->event_writable;
}

static void err_cb(void *arg, err_t err)
{
	(void)err;

	int idx = sock_index_from_arg(arg);

	if (idx < 0)
	{
		return;
	}

	bsd_sock_t *s = &socks[idx];

	/*
	 * lwIP has already destroyed the PCB before tcp_err()
	 * is called.
	 */
	s->pcb = NULL;

	/*
	 * If µCNC already requested a local close, there is
	 * nothing to notify upward.
	 */
	if (s->local_close)
	{
		reset_sock(s);
		return;
	}

	free_rx_chain(s);

	s->state = SOCK_CLOSING;
	s->event_error = true;
	s->event_pending = true;
}

static err_t sent_cb(void *arg, struct tcp_pcb *pcb, u16_t len)
{
	(void)pcb;
	(void)len;

	int idx = sock_index_from_arg(arg);

	if (idx < 0)
	{
		return ERR_OK;
	}

	bsd_sock_t *s = &socks[idx];

	if (s->state == SOCK_UNUSED)
	{
		return ERR_OK;
	}

	/*
	 * A graceful close may previously have failed with
	 * ERR_MEM. New ACK progress may allow tcp_close()
	 * to succeed now.
	 */
	if (s->local_close || s->event_peer_closed)
	{
		s->event_pending = true;
	}

	/*
	 * Report TX progress only after a previous send
	 * encountered backpressure.
	 */
	if (s->write_blocked)
	{
		s->write_blocked = false;
		s->event_writable = true;
		s->event_pending = true;
	}

	return ERR_OK;
}

static err_t recv_cb(
	void *arg,
	struct tcp_pcb *pcb,
	struct pbuf *p,
	err_t err)
{
	int idx = sock_index_from_arg(arg);

	if (idx < 0)
	{
		if (p)
		{
			pbuf_free(p);
		}

		return ERR_OK;
	}

	bsd_sock_t *s = &socks[idx];

	/*
	 * µCNC has already closed this connection locally.
	 *
	 * If lwIP delivers data before the graceful close completes,
	 * consume and acknowledge it without forwarding it upward.
	 */
	if (s->local_close)
	{
		if (p)
		{
			tcp_recved(pcb, p->tot_len);
			pbuf_free(p);
		}

		return ERR_OK;
	}

	/*
	 * Unexpected receive-side error.
	 *
	 * Fatal TCP errors normally arrive through tcp_err(), but
	 * treat any non-OK recv error as a transport failure.
	 */
	if (err != ERR_OK)
	{
		if (p)
		{
			pbuf_free(p);
		}

		s->event_error = true;
		s->event_pending = true;

		return ERR_OK;
	}

	/*
	 * p == NULL means the remote peer sent FIN.
	 *
	 * Do not disconnect immediately if unread RX data remains.
	 * service() will first deliver the queued payload.
	 */
	if (!p)
	{
		s->event_peer_closed = true;
		s->event_pending = true;

		return ERR_OK;
	}

	/*
	 * Only retain one receive chain at a time.
	 *
	 * Returning ERR_MEM without freeing p tells lwIP that we
	 * cannot accept this data yet. lwIP will retry once the
	 * receive callback is called again later.
	 */
	if (s->rx_buf)
	{
		return ERR_MEM;
	}

	u16_t total_len = p->tot_len;

	/*
	 * We need the pbuf after recv_cb() returns.
	 *
	 * Take our own reference, then release the reference supplied
	 * to this receive callback. The backend's retained reference
	 * is released later from service().
	 */
	pbuf_ref(p);

	s->rx_buf = p;
	s->rx_off = 0;
	s->rx_len = total_len;

	pbuf_free(p);

	s->event_pending = true;

	return ERR_OK;
}

static err_t accept_cb(
	void *arg,
	struct tcp_pcb *newpcb,
	err_t err)
{
	if (err != ERR_OK || !newpcb)
	{
		return err;
	}

	int srv_idx = sock_index_from_arg(arg);

	if (srv_idx < 0)
	{
		tcp_abort(newpcb);
		return ERR_ABRT;
	}

	bsd_sock_t *server = &socks[srv_idx];

	if (server->state != SOCK_LISTEN)
	{
		tcp_abort(newpcb);
		return ERR_ABRT;
	}

	int idx = alloc_sock();

	if (idx < 0)
	{
		/* Backend has no room for another connection */
		tcp_abort(newpcb);
		return ERR_ABRT;
	}

	bsd_sock_t *client = &socks[idx];

	init_sock(client, idx);

	client->state = SOCK_CONNECTED;
	client->pcb = newpcb;

	client->srv_handle = server->srv_handle;
	client->client_handle = (socket_handle_t)idx;

	client->event_connected = true;
	client->event_pending = true;

	/*
	 * Tell lwIP that this accepted connection is being held
	 * pending until the µCNC cooperative layer consumes it.
	 */
	tcp_backlog_delayed(newpcb);

	/*
	 * Install the TCP callbacks immediately so data cannot arrive
	 * on the accepted PCB without a receive handler.
	 */
	tcp_arg(newpcb, sock_arg_from_index(idx));
	tcp_err(newpcb, err_cb);
	tcp_recv(newpcb, recv_cb);
	tcp_sent(newpcb, sent_cb);

	return ERR_OK;
}

/* ---------------- µCNC socket device API ---------------- */

static int esp8266_socket_device_init(
	const socket_device_events_t *events)
{
	if (!events)
	{
		return SOCKET_DEVICE_INVALID;
	}

	esp8266_socket_events = events;

	memset(socks, 0, sizeof(socks));
	memset(sock_generations, 0, sizeof(sock_generations));
	service_depth = 0;

	for (int i = 0; i < MAX_BSD_SOCKETS; i++)
	{
		socks[i].state = SOCK_UNUSED;
		socks[i].srv_handle = SOCKET_INVALID_HANDLE;
		socks[i].client_handle = SOCKET_INVALID_HANDLE;
	}

	return SOCKET_DEVICE_OK;
}

static socket_handle_t esp8266_socket_listen(
	uint32_t ip_listen,
	uint16_t port,
	int domain,
	int type,
	int protocol,
	uint8_t backlog)
{
	(void)protocol;

	if (domain != AF_INET || type != SOCK_STREAM)
	{
		return SOCKET_INVALID_HANDLE;
	}

	int idx = alloc_sock();

	if (idx < 0)
	{
		return SOCKET_INVALID_HANDLE;
	}

	bsd_sock_t *s = &socks[idx];

	init_sock(s, idx);

	s->state = SOCK_BOUND;
	s->srv_handle = (socket_handle_t)idx;
	s->client_handle = SOCKET_INVALID_HANDLE;

	s->pcb = tcp_new();

	if (!s->pcb)
	{
		reset_sock(s);
		return SOCKET_INVALID_HANDLE;
	}

	ip_addr_t ip;
	ip_addr_t *bind_addr;

	if (ip_listen == IP_ANY)
	{
		bind_addr = IP_ADDR_ANY;
	}
	else
	{
		/*
		 * µCNC passes the IP in host byte order, matching the
		 * semantics previously used by the BSD abstraction.
		 */
		ip.addr = htonl(ip_listen);
		bind_addr = &ip;
	}

	err_t err = tcp_bind(
		s->pcb,
		bind_addr,
		port);

	if (err != ERR_OK)
	{
		tcp_close(s->pcb);
		s->pcb = NULL;

		reset_sock(s);

		return SOCKET_INVALID_HANDLE;
	}

	struct tcp_pcb *listener =
		tcp_listen_with_backlog(
			s->pcb,
			backlog ? backlog : 1);

	if (!listener)
	{
		tcp_close(s->pcb);
		s->pcb = NULL;

		reset_sock(s);

		return SOCKET_INVALID_HANDLE;
	}

	s->pcb = listener;
	s->state = SOCK_LISTEN;

	tcp_arg(
		listener,
		sock_arg_from_index(idx));

	tcp_accept(
		listener,
		accept_cb);

	return (socket_handle_t)idx;
}

static int esp8266_socket_send(
	socket_handle_t client,
	const void *data,
	size_t len,
	int flags)
{
	(void)flags;

	if (client == SOCKET_INVALID_HANDLE ||
		!data)
	{
		return SOCKET_DEVICE_INVALID;
	}

	if (len == 0)
	{
		return 0;
	}

	int idx = (int)client;

	if (idx < 0 || idx >= MAX_BSD_SOCKETS)
	{
		return SOCKET_DEVICE_INVALID;
	}

	bsd_sock_t *s = &socks[idx];

	if (!s->pcb ||
		s->state != SOCK_CONNECTED ||
		s->local_close)
	{
		return SOCKET_DEVICE_CLOSED;
	}

	u16_t available = tcp_sndbuf(s->pcb);

	if (available == 0)
	{
		s->write_blocked = true;

		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	u16_t chunk =
		(len < (size_t)available)
			? (u16_t)len
			: available;

	err_t err = tcp_write(
		s->pcb,
		data,
		chunk,
		TCP_WRITE_FLAG_COPY);

	if (err == ERR_MEM)
	{
		s->write_blocked = true;

		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	if (err != ERR_OK)
	{
		return SOCKET_DEVICE_ERROR;
	}

	s->write_blocked = false;

	/*
	 * tcp_write() only queues the payload.
	 * Ask lwIP to attempt transmission now.
	 *
	 * Even if tcp_output() cannot immediately transmit,
	 * the bytes were successfully accepted by tcp_write(),
	 * so chunk remains the correct return value.
	 */
	tcp_output(s->pcb);

	return (int)chunk;
}

static int esp8266_socket_close(
	socket_handle_t handle)
{
	if (handle == SOCKET_INVALID_HANDLE)
	{
		return SOCKET_DEVICE_INVALID;
	}

	int idx = (int)handle;

	if (idx < 0 || idx >= MAX_BSD_SOCKETS)
	{
		return SOCKET_DEVICE_INVALID;
	}

	bsd_sock_t *s = &socks[idx];

	if (s->state == SOCK_UNUSED)
	{
		return SOCKET_DEVICE_INVALID;
	}

	/*
	 * Listener close.
	 *
	 * Listening PCBs do not have buffered application data,
	 * so tcp_close() should normally succeed immediately.
	 */
	if (s->state == SOCK_LISTEN)
	{
		struct tcp_pcb *pcb = s->pcb;

		if (pcb)
		{
			tcp_arg(pcb, NULL);
			tcp_accept(pcb, NULL);

			err_t err = tcp_close(pcb);

			if (err != ERR_OK)
			{
				/*
				 * A listener has no application data whose
				 * delivery needs preserving, so force cleanup
				 * if normal close unexpectedly fails.
				 */
				tcp_abort(pcb);
			}

			s->pcb = NULL;
		}

		reset_sock(s);

		return SOCKET_DEVICE_OK;
	}

	/*
	 * Client close requested locally.
	 *
	 * Do not emit a disconnected event: socket.c already
	 * handles the local disconnect notification.
	 */
	s->local_close = true;
	s->state = SOCK_CLOSING;

	s->event_connected = false;
	s->event_peer_closed = false;
	s->event_error = false;
	s->event_writable = false;
	s->write_blocked = false;

	free_rx_chain(s);

	if (!s->pcb)
	{
		reset_sock(s);
		return SOCKET_DEVICE_OK;
	}

	err_t err = tcp_close(s->pcb);

	if (err == ERR_OK)
	{
		s->pcb = NULL;
		reset_sock(s);

		return SOCKET_DEVICE_OK;
	}

	if (err == ERR_MEM)
	{
		/*
		 * lwIP cannot queue the close yet.
		 *
		 * Keep the PCB and retry later from service().
		 */
		s->event_pending = true;

		return SOCKET_DEVICE_OK;
	}

	/*
	 * Unexpected close failure.
	 * Force the PCB down rather than leaking it.
	 */
	tcp_abort(s->pcb);
	s->pcb = NULL;

	reset_sock(s);

	return SOCKET_DEVICE_ERROR;
}

static void esp8266_socket_service(void)
{
	if (!esp8266_socket_events)
	{
		return;
	}

	/*
	 * Application callbacks can re-enter service().  Only the outermost pass
	 * may dispatch connected() or data(); nested passes can still advance
	 * local closes, errors, FIN handling and TX notifications.
	 */
	bool allow_rx_and_accept = (service_depth == 0);
	service_depth++;

	for (int i = 0; i < MAX_BSD_SOCKETS; i++)
	{
		bsd_sock_t *s = &socks[i];

		if (s->state == SOCK_UNUSED)
		{
			continue;
		}

		/*
		 * ----------------------------------------------------
		 * Locally requested graceful close
		 * ----------------------------------------------------
		 */
		if (s->local_close &&
			s->state == SOCK_CLOSING)
		{
			if (!s->pcb)
			{
				reset_sock(s);
				continue;
			}

			err_t err = tcp_close(s->pcb);

			if (err == ERR_OK)
			{
				s->pcb = NULL;
				reset_sock(s);
			}
			else if (err == ERR_MEM)
			{
				/*
				 * Still waiting for lwIP TCP resources.
				 * Retry on a later service pass.
				 */
				s->event_pending = true;
			}
			else
			{
				tcp_abort(s->pcb);
				s->pcb = NULL;

				reset_sock(s);
			}

			continue;
		}

		/* Derive the scheduling bit from the actual state.  This also preserves
		 * RX/FIN work that arrived before connected() was dispatched. */
		refresh_event_pending(s);

		if (!s->event_pending)
		{
			continue;
		}

		/*
		 * Consume this scheduling flag.
		 *
		 * Individual handlers below may set it again if
		 * additional work remains.
		 */
		s->event_pending = false;

		/*
		 * ----------------------------------------------------
		 * Fatal TCP error
		 * ----------------------------------------------------
		 */
		if (s->event_error)
		{
			socket_handle_t handle =
				s->client_handle;

			s->event_error = false;

			if (s->pcb)
			{
				/*
				 * recv_cb() may report an error while the PCB
				 * still exists. tcp_err(), by contrast, sets
				 * pcb to NULL before service() sees the event.
				 */
				tcp_abort(s->pcb);
				s->pcb = NULL;
			}

			reset_sock(s);

			if (handle != SOCKET_INVALID_HANDLE &&
				esp8266_socket_events->disconnected)
			{
				esp8266_socket_events->disconnected(
					handle,
					SOCKET_DEVICE_ERROR);
			}

			continue;
		}

		/*
		 * ----------------------------------------------------
		 * Newly accepted connection
		 * ----------------------------------------------------
		 */
		if (s->event_connected)
		{
			if (!allow_rx_and_accept)
			{
				s->event_pending = true;
				continue;
			}

			uint32_t generation = s->generation;
			struct tcp_pcb *pcb = s->pcb;
			s->event_connected = false;

			socket_handle_t listener =
				s->srv_handle;

			socket_handle_t client =
				s->client_handle;

			/*
			 * Match tcp_backlog_delayed() from accept_cb().
			 *
			 * The TCP connection has now been consumed by the
			 * application/backend layer whether µCNC ultimately
			 * accepts or rejects the logical client.
			 */
			if (s->pcb)
			{
				tcp_backlog_accepted(s->pcb);
			}

			bool accepted = false;

			if (esp8266_socket_events->connected)
			{
				accepted =
					esp8266_socket_events->connected(
						listener,
						client);
			}

			/* connected() may synchronously close/reset this slot. */
			if (s->generation != generation ||
				s->pcb != pcb ||
				s->client_handle != client)
			{
				continue;
			}

			if (!accepted)
			{
				/*
				 * No logical µCNC client slot available.
				 */
				if (s->pcb)
				{
					tcp_arg(s->pcb, NULL);
					tcp_recv(s->pcb, NULL);
					tcp_sent(s->pcb, NULL);
					tcp_err(s->pcb, NULL);

					tcp_abort(s->pcb);

					s->pcb = NULL;
				}

				reset_sock(s);
			}
			else
			{
				/* Do not lose data/FIN/writable work that was already queued
				 * while the logical connection was being accepted. */
				refresh_event_pending(s);
			}

			continue;
		}

		/*
		 * ----------------------------------------------------
		 * Received TCP payload
		 * ----------------------------------------------------
		 */
		if (s->rx_buf && s->rx_len)
		{
			if (!allow_rx_and_accept)
			{
				/* The shared callback buffer is still exposed by the outer
				 * data() call.  Keep the pbuf untouched for a later pass. */
				s->event_pending = true;
				continue;
			}

			uint32_t generation = s->generation;
			socket_handle_t handle = s->client_handle;
			struct tcp_pcb *pcb = s->pcb;
			struct pbuf *rx_buf = s->rx_buf;
			u16_t rx_off = s->rx_off;

			size_t wanted =
				(s->rx_len > SOCKET_MAX_DATA_SIZE)
					? SOCKET_MAX_DATA_SIZE
					: (size_t)s->rx_len;

			u16_t copied =
				pbuf_copy_partial(
					s->rx_buf,
					rx_dispatch_buffer,
					(u16_t)wanted,
					s->rx_off);

			if (copied != (u16_t)wanted)
			{
				/*
				 * Corrupt/inconsistent receive state.
				 */
				s->event_error = true;
				s->event_pending = true;

				continue;
			}

			rx_dispatch_buffer[copied] = '\0';
			bool consumed = true;

			if (esp8266_socket_events->data)
			{
				consumed = esp8266_socket_events->data(
					handle,
					rx_dispatch_buffer,
					copied);
			}

			/* data() may synchronously close the client or a nested service
			 * pass may complete a fatal disconnect.  Never commit RX state to
			 * a reset or reused slot. */
			if (!same_client(s, generation, handle, pcb) ||
				s->rx_buf != rx_buf ||
				s->rx_off != rx_off)
			{
				continue;
			}

			if (!consumed)
			{
				/* New data() contract: no bytes were consumed.  Do not move the
				 * pbuf cursor and do not enlarge lwIP's receive window.  The same
				 * bytes will be copied and offered again on a later outer pass. */
				s->event_pending = true;
				continue;
			}

			/* Commit only after the core confirms complete consumption. */
			s->rx_off += copied;
			s->rx_len -= copied;
			tcp_recved(s->pcb, copied);

			if (s->rx_len == 0)
			{
				pbuf_free(s->rx_buf);
				s->rx_buf = NULL;
				s->rx_off = 0;
			}

			refresh_event_pending(s);
			continue;
		}

		/*
		 * ----------------------------------------------------
		 * Remote orderly close / FIN
		 * ----------------------------------------------------
		 */
		if (s->event_peer_closed)
		{
			socket_handle_t handle =
				s->client_handle;

			/*
			 * Only get here after any queued receive data has
			 * already been delivered.
			 */
			err_t err = ERR_OK;

			if (s->pcb)
			{
				err = tcp_close(s->pcb);
			}

			if (err == ERR_OK)
			{
				s->pcb = NULL;

				reset_sock(s);

				if (handle != SOCKET_INVALID_HANDLE &&
					esp8266_socket_events->disconnected)
				{
					esp8266_socket_events->disconnected(
						handle,
						0);
				}
			}
			else if (err == ERR_MEM)
			{
				/*
				 * Retry graceful close later.
				 */
				s->event_pending = true;
			}
			else
			{
				if (s->pcb)
				{
					tcp_abort(s->pcb);
					s->pcb = NULL;
				}

				reset_sock(s);

				if (handle != SOCKET_INVALID_HANDLE &&
					esp8266_socket_events->disconnected)
				{
					esp8266_socket_events->disconnected(
						handle,
						SOCKET_DEVICE_ERROR);
				}
			}

			continue;
		}

		/*
		 * ----------------------------------------------------
		 * TX became writable again
		 * ----------------------------------------------------
		 */
		if (s->event_writable)
		{
			uint32_t generation = s->generation;
			struct tcp_pcb *pcb = s->pcb;
			s->event_writable = false;

			socket_handle_t handle =
				s->client_handle;

			if (esp8266_socket_events->writable)
			{
				esp8266_socket_events->writable(
					handle);
			}

			if (same_client(s, generation, handle, pcb))
			{
				refresh_event_pending(s);
			}

			continue;
		}
	}

	service_depth--;
}

socket_device_t wifi_socket = {
    .init    = esp8266_socket_device_init,
    .listen  = esp8266_socket_listen,
    .send    = esp8266_socket_send,
    .close   = esp8266_socket_close,
    .service = esp8266_socket_service
};

#endif
#endif
