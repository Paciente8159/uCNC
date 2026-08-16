/*
	Name: rp2040_lwip.c
	Description: Allocation-free uCNC TCP backend for the RP2040/Pico W
	             using the lwIP raw callback API.


	Copyright: Copyright (c) Joao Martins
	Author: Joao Martins

	uCNC is free software: you can redistribute it and/or modify it under the
	terms of the GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later
	version. Please see <http://www.gnu.org/licenses/>.
*/

#include "../../../cnc.h"

#if (MCU == MCU_RP2040)

#ifdef __cplusplus
extern "C"
{
#endif

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "pico/cyw43_arch.h"
#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include "lwip/opt.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "../../../modules/net/socket.h"

/* Handles encode a 16-bit generation plus a one-based backend table slot. */
#if (MAX_SOCKETS + SOCKET_MAX_CONNECTIONS) >= UINT16_MAX
#error "RP2040 socket listener + client count must be below 65535"
#endif

/* Client flag bits are intentionally packed into one byte. */
#define RP2040_CLIENT_IN_USE          0x01U
#define RP2040_CLIENT_ACCEPT_PENDING  0x02U
#define RP2040_CLIENT_READABLE_PENDING 0x04U
#define RP2040_CLIENT_CLOSE_PENDING   0x08U

typedef struct rp2040_listener_
{
	struct tcp_pcb *pcb;
	uint16_t generation;
} rp2040_listener_t;

typedef struct rp2040_client_
{
	struct tcp_pcb *pcb;
	struct pbuf *rx;
	socket_device_handle_t listener_handle;
	socket_device_token_t token;
	uint16_t generation;
	uint16_t rx_offset;
	int8_t close_reason;
	uint8_t flags;
} rp2040_client_t;

typedef enum rp2040_event_kind_
{
	RP2040_EVENT_NONE = 0,
	RP2040_EVENT_ACCEPTED,
	RP2040_EVENT_READABLE,
	RP2040_EVENT_CLOSED
} rp2040_event_kind_t;

static rp2040_listener_t rp2040_listeners[MAX_SOCKETS];
static rp2040_client_t rp2040_clients[SOCKET_MAX_CONNECTIONS];
static const socket_device_events_t *rp2040_events;
static uint16_t rp2040_poll_cursor;

/* Raw-API callbacks. lwIP invokes each one with its core protection held. */
static err_t rp2040_accept_cb(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t rp2040_recv_cb(void *arg,
						  struct tcp_pcb *tpcb,
						  struct pbuf *p,
						  err_t err);
static void rp2040_err_cb(void *arg, err_t err);

static uint16_t rp2040_next_generation(uint16_t generation)
{
	++generation;
	return generation == 0U ? 1U : generation;
}

static socket_device_handle_t rp2040_make_handle(uint16_t native_slot,
												  uint16_t generation)
{
	return ((socket_device_handle_t)generation << 16) |
		   (socket_device_handle_t)(native_slot + 1U);
}

static bool rp2040_decode_handle(socket_device_handle_t handle,
								 uint16_t *native_slot,
								 uint16_t *generation)
{
	uint16_t encoded_slot;

	if (!native_slot || !generation || handle == SOCKET_DEVICE_INVALID_HANDLE)
	{
		return false;
	}

	encoded_slot = (uint16_t)(handle & 0xFFFFU);
	if (encoded_slot == 0U)
	{
		return false;
	}

	*native_slot = (uint16_t)(encoded_slot - 1U);
	*generation = (uint16_t)((handle >> 16) & 0xFFFFU);
	return *generation != 0U;
}

static socket_device_handle_t rp2040_listener_handle(size_t index,
													  uint16_t generation)
{
	return rp2040_make_handle((uint16_t)index, generation);
}

static socket_device_handle_t rp2040_client_handle(size_t index,
													uint16_t generation)
{
	return rp2040_make_handle((uint16_t)(MAX_SOCKETS + index), generation);
}

/* All helpers below that touch records/PCBs require lwIP core protection. */
static rp2040_listener_t *rp2040_find_listener(socket_device_handle_t handle)
{
	uint16_t slot;
	uint16_t generation;

	if (!rp2040_decode_handle(handle, &slot, &generation) ||
		slot >= MAX_SOCKETS)
	{
		return NULL;
	}

	if (!rp2040_listeners[slot].pcb ||
		rp2040_listeners[slot].generation != generation)
	{
		return NULL;
	}

	return &rp2040_listeners[slot];
}

static rp2040_client_t *rp2040_find_client(socket_device_handle_t handle)
{
	uint16_t slot;
	uint16_t generation;
	uint16_t client_index;

	if (!rp2040_decode_handle(handle, &slot, &generation) ||
		slot < MAX_SOCKETS)
	{
		return NULL;
	}

	client_index = (uint16_t)(slot - MAX_SOCKETS);
	if (client_index >= SOCKET_MAX_CONNECTIONS ||
		(rp2040_clients[client_index].flags & RP2040_CLIENT_IN_USE) == 0U ||
		rp2040_clients[client_index].generation != generation)
	{
		return NULL;
	}

	return &rp2040_clients[client_index];
}

static rp2040_listener_t *rp2040_alloc_listener(size_t *index_out)
{
	size_t i;

	for (i = 0U; i < MAX_SOCKETS; ++i)
	{
		if (!rp2040_listeners[i].pcb)
		{
			rp2040_listeners[i].generation =
				rp2040_next_generation(rp2040_listeners[i].generation);
			*index_out = i;
			return &rp2040_listeners[i];
		}
	}

	return NULL;
}

static rp2040_client_t *rp2040_alloc_client(size_t *index_out)
{
	size_t i;

	for (i = 0U; i < SOCKET_MAX_CONNECTIONS; ++i)
	{
		rp2040_client_t *client = &rp2040_clients[i];

		if ((client->flags & RP2040_CLIENT_IN_USE) == 0U)
		{
			uint16_t generation =
				rp2040_next_generation(client->generation);

			memset(client, 0, sizeof(*client));
			client->generation = generation;
			client->listener_handle = SOCKET_DEVICE_INVALID_HANDLE;
			client->token = SOCKET_DEVICE_INVALID_TOKEN;
			client->close_reason = (int8_t)SOCKET_DEVICE_INVALID;
			client->flags = RP2040_CLIENT_IN_USE;
			*index_out = i;
			return client;
		}
	}

	return NULL;
}

static void rp2040_reset_listener(rp2040_listener_t *listener)
{
	uint16_t generation;

	if (!listener)
	{
		return;
	}

	generation = listener->generation;
	memset(listener, 0, sizeof(*listener));
	listener->generation = generation;
}

static void rp2040_reset_client(rp2040_client_t *client)
{
	uint16_t generation;

	if (!client)
	{
		return;
	}

	generation = client->generation;
	memset(client, 0, sizeof(*client));
	client->generation = generation;
	client->listener_handle = SOCKET_DEVICE_INVALID_HANDLE;
	client->token = SOCKET_DEVICE_INVALID_TOKEN;
	client->close_reason = (int8_t)SOCKET_DEVICE_INVALID;
}

/*
 * Detach a live client PCB from this record, then abort it. This is used only
 * for a transport failure discovered synchronously by the backend. The record
 * and any already-retained RX pbuf remain alive so final data can still drain.
 */
static void rp2040_abort_client_pcb(rp2040_client_t *client)
{
	struct tcp_pcb *pcb;

	if (!client)
	{
		return;
	}

	pcb = client->pcb;
	client->pcb = NULL;
	if (!pcb)
	{
		return;
	}

	tcp_arg(pcb, NULL);
	tcp_recv(pcb, NULL);
	tcp_err(pcb, NULL);
	tcp_abort(pcb);
}

/*
 * Release a listener. Listener tcp_close() is synchronous in lwIP and frees the
 * listen PCB. tcp_abort() is deliberately not used because lwIP does not permit
 * aborting a listen PCB.
 */
static void rp2040_release_listener(rp2040_listener_t *listener)
{
	struct tcp_pcb *pcb;

	if (!listener || !listener->pcb)
	{
		return;
	}

	pcb = listener->pcb;
	listener->pcb = NULL;

	tcp_arg(pcb, NULL);
	tcp_accept(pcb, NULL);
	(void)tcp_close(pcb);
	rp2040_reset_listener(listener);
}

/*
 * Release a client synchronously from the backend's ownership perspective.
 * The record is invalidated before native teardown, preventing late callbacks
 * from targeting a live token. No uCNC event is emitted here.
 */
static void rp2040_release_client(rp2040_client_t *client)
{
	struct tcp_pcb *pcb;
	struct pbuf *rx;
	err_t close_result;

	if (!client || (client->flags & RP2040_CLIENT_IN_USE) == 0U)
	{
		return;
	}

	pcb = client->pcb;
	rx = client->rx;

	/* Invalidate the backend record before touching native teardown. */
	client->flags = 0U;
	client->pcb = NULL;
	client->rx = NULL;
	client->token = SOCKET_DEVICE_INVALID_TOKEN;

	if (pcb)
	{
		tcp_arg(pcb, NULL);
		tcp_recv(pcb, NULL);
		tcp_err(pcb, NULL);

		/*
		 * Modern lwIP internally defers FIN enqueue on ERR_MEM and returns
		 * ERR_OK. For older variants that can return an error here, abort is
		 * the bounded fallback; local close must never wait for ACK/capacity.
		 */
		close_result = tcp_close(pcb);
		if (close_result != ERR_OK)
		{
			tcp_abort(pcb);
		}
	}

	if (rx)
	{
		pbuf_free(rx);
	}

	rp2040_reset_client(client);
}

/* Fatal/native state normalization. Context-specific ERR_MEM handling is separate. */
static int rp2040_fatal_result(err_t err)
{
	switch (err)
	{
	case ERR_CLSD:
	case ERR_CONN:
		return SOCKET_DEVICE_CLOSED;

	case ERR_ARG:
	case ERR_VAL:
		return SOCKET_DEVICE_INVALID;

	case ERR_MEM:
		return SOCKET_DEVICE_NO_MEMORY;

	case ERR_TIMEOUT:
	case ERR_RTE:
	case ERR_IF:
	case ERR_ABRT:
	case ERR_RST:
	case ERR_BUF:
	case ERR_INPROGRESS:
	case ERR_WOULDBLOCK:
	case ERR_USE:
	case ERR_ALREADY:
	case ERR_ISCONN:
	default:
		return SOCKET_DEVICE_ERROR;
	}
}

/* Called from raw callbacks; it never emits a uCNC event. */
static void rp2040_mark_close(rp2040_client_t *client, int reason)
{
	if (!client || (client->flags & RP2040_CLIENT_IN_USE) == 0U)
	{
		return;
	}

	if ((client->flags & RP2040_CLIENT_CLOSE_PENDING) == 0U)
	{
		client->close_reason =
			(int8_t)((reason < 0) ? reason : SOCKET_DEVICE_ERROR);
		client->flags |= RP2040_CLIENT_CLOSE_PENDING;
	}
}

static err_t rp2040_accept_cb(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	rp2040_listener_t *listener = (rp2040_listener_t *)arg;
	rp2040_client_t *client;
	size_t listener_index;
	size_t client_index;

	if (err != ERR_OK)
	{
		return err;
	}

	if (!newpcb)
	{
		return ERR_ARG;
	}

	if (!listener || listener->pcb == NULL)
	{
		tcp_abort(newpcb);
		return ERR_ABRT;
	}

	listener_index = (size_t)(listener - rp2040_listeners);
	if (listener_index >= MAX_SOCKETS)
	{
		tcp_abort(newpcb);
		return ERR_ABRT;
	}

	client = rp2040_alloc_client(&client_index);
	if (!client)
	{
		/* Fixed backend capacity exhausted: reject this native connection. */
		tcp_abort(newpcb);
		return ERR_ABRT;
	}

	client->pcb = newpcb;
	client->listener_handle =
		rp2040_listener_handle(listener_index, listener->generation);
	client->flags |= RP2040_CLIENT_ACCEPT_PENDING;

	tcp_setprio(newpcb, TCP_PRIO_NORMAL);
	tcp_arg(newpcb, client);
	tcp_recv(newpcb, rp2040_recv_cb);
	tcp_err(newpcb, rp2040_err_cb);

	/*
	 * No tcp_sent()/tcp_poll() callback is installed. Native ACKs/timers update
	 * pcb->snd_buf and queues internally; the core observes that on its next
	 * strictly non-blocking send() attempt.
	 */
	return ERR_OK;
}

static err_t rp2040_recv_cb(void *arg,
							struct tcp_pcb *tpcb,
							struct pbuf *p,
							err_t err)
{
	rp2040_client_t *client = (rp2040_client_t *)arg;

	if (!client ||
		(client->flags & RP2040_CLIENT_IN_USE) == 0U ||
		client->pcb != tpcb)
	{
		if (p)
		{
			pbuf_free(p);
		}

		/*
		 * A mismatched callback is stale with respect to our fixed record.
		 * Detach its callbacks before aborting so tcp_abort() cannot deliver an
		 * error callback through the old arg into a record that may now belong
		 * to a newer generation.
		 */
		tcp_arg(tpcb, NULL);
		tcp_recv(tpcb, NULL);
		tcp_err(tpcb, NULL);
		tcp_abort(tpcb);
		return ERR_ABRT;
	}

	if (err != ERR_OK)
	{
		/*
		 * An error on this callback makes the PCB unusable. Preserve any
		 * previously retained client->rx, but discard this callback's pbuf
		 * because we are aborting the native PCB now.
		 */
		if (p)
		{
			pbuf_free(p);
		}
		rp2040_mark_close(client, rp2040_fatal_result(err));
		client->pcb = NULL;
		tcp_arg(tpcb, NULL);
		tcp_recv(tpcb, NULL);
		tcp_err(tpcb, NULL);
		tcp_abort(tpcb);
		return ERR_ABRT;
	}

	/* p == NULL is an orderly FIN. Retained payload drains before close. */
	if (!p)
	{
		rp2040_mark_close(client, SOCKET_DEVICE_CLOSED);
		return ERR_OK;
	}

	if (p->tot_len == 0U)
	{
		pbuf_free(p);
		return ERR_OK;
	}

	if (client->rx)
	{
		/*
		 * Do not free or acknowledge this pbuf. Returning ERR_MEM tells lwIP
		 * that the upper layer cannot take it yet; lwIP keeps it in native
		 * refused_data storage and retries the receive callback later.
		 */
		return ERR_MEM;
	}

	/* Returning ERR_OK transfers this pbuf chain to the backend/application. */
	client->rx = p;
	client->rx_offset = 0U;
	client->flags |= RP2040_CLIENT_READABLE_PENDING;
	return ERR_OK;
}

static void rp2040_err_cb(void *arg, err_t err)
{
	rp2040_client_t *client = (rp2040_client_t *)arg;

	if (!client || (client->flags & RP2040_CLIENT_IN_USE) == 0U)
	{
		return;
	}

	/*
	 * lwIP documents that the PCB is already deallocated when tcp_err() runs.
	 * Never dereference it here. Already-retained backend RX remains valid and
	 * is delivered before the normalized close event.
	 */
	client->pcb = NULL;
	rp2040_mark_close(client, rp2040_fatal_result(err));
	if (client->rx)
	{
		client->flags |= RP2040_CLIENT_READABLE_PENDING;
	}
}

static int rp2040_socket_init(const socket_device_events_t *events)
{
	if (!events || !events->accepted || !events->readable || !events->closed)
	{
		return SOCKET_DEVICE_INVALID;
	}

	memset(rp2040_listeners, 0, sizeof(rp2040_listeners));
	memset(rp2040_clients, 0, sizeof(rp2040_clients));
	rp2040_events = events;
	rp2040_poll_cursor = 0U;
	return SOCKET_DEVICE_OK;
}

static socket_device_handle_t rp2040_socket_listen(
	const socket_device_endpoint_t *endpoint,
	uint8_t backlog)
{
	rp2040_listener_t *listener;
	struct tcp_pcb *pcb;
	struct tcp_pcb *listen_pcb;
	ip_addr_t bind_address;
	socket_device_handle_t handle = SOCKET_DEVICE_INVALID_HANDLE;
	size_t listener_index;
	err_t err;

	if (!endpoint || endpoint->port == 0U)
	{
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	/*
	 * uCNC's listener-local client limit is the useful upper bound here. A
	 * zero request is explicitly bounded to one pending native connection.
	 */
	if (backlog == 0U)
	{
		backlog = 1U;
	}
	if (backlog > SOCKET_MAX_CLIENTS)
	{
		backlog = SOCKET_MAX_CLIENTS;
	}

	cyw43_arch_lwip_begin();

	listener = rp2040_alloc_listener(&listener_index);
	if (!listener)
	{
		cyw43_arch_lwip_end();
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
	if (!pcb)
	{
		rp2040_reset_listener(listener);
		cyw43_arch_lwip_end();
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	listener->pcb = pcb;
	tcp_setprio(pcb, TCP_PRIO_NORMAL);
#if SO_REUSE
	pcb->so_options |= SOF_REUSEADDR;
#endif

	if (endpoint->address == IP_ANY)
	{
		ip_addr_set_any(IPADDR_TYPE_V4, &bind_address);
	}
	else
	{
		/* endpoint->address is the uCNC host-order form 0xAABBCCDD. */
		IP_ADDR4(&bind_address,
				 (uint8_t)(endpoint->address >> 24),
				 (uint8_t)(endpoint->address >> 16),
				 (uint8_t)(endpoint->address >> 8),
				 (uint8_t)endpoint->address);
	}

	err = tcp_bind(pcb, &bind_address, endpoint->port);
	if (err != ERR_OK)
	{
		rp2040_release_listener(listener);
		cyw43_arch_lwip_end();
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	listen_pcb = tcp_listen_with_backlog(pcb, backlog);
	if (!listen_pcb)
	{
		/* On failure the original bound PCB remains owned by the caller. */
		rp2040_release_listener(listener);
		cyw43_arch_lwip_end();
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	listener->pcb = listen_pcb;
	tcp_arg(listen_pcb, listener);
	tcp_accept(listen_pcb, rp2040_accept_cb);
	handle = rp2040_listener_handle(listener_index, listener->generation);

	cyw43_arch_lwip_end();
	return handle;
}

static int rp2040_socket_recv(socket_device_handle_t handle,
								void *destination,
								size_t capacity)
{
	rp2040_client_t *client;
	struct pbuf *p;
	uint8_t *output = (uint8_t *)destination;
	size_t remaining;
	size_t wanted;
	u16_t copied;
	socket_device_token_t close_token;
	int close_reason;

	if (capacity == 0U)
	{
		return 0;
	}
	if (!destination)
	{
		return SOCKET_DEVICE_INVALID;
	}

	cyw43_arch_lwip_begin();

	client = rp2040_find_client(handle);
	if (!client || client->token == SOCKET_DEVICE_INVALID_TOKEN)
	{
		cyw43_arch_lwip_end();
		return SOCKET_DEVICE_INVALID;
	}

	p = client->rx;
	if (p)
	{
		if (client->rx_offset >= p->tot_len)
		{
			close_token = client->token;
			close_reason = SOCKET_DEVICE_ERROR;
			rp2040_release_client(client);
			cyw43_arch_lwip_end();
			rp2040_events->closed(close_token, close_reason);
			return close_reason;
		}

		remaining = (size_t)p->tot_len - client->rx_offset;
		wanted = capacity;
		if (wanted > remaining)
		{
			wanted = remaining;
		}
		if (wanted > (size_t)UINT16_MAX)
		{
			wanted = (size_t)UINT16_MAX;
		}

		copied = pbuf_copy_partial(p,
								   output,
								   (u16_t)wanted,
								   client->rx_offset);
		if (copied == 0U)
		{
			close_token = client->token;
			close_reason = SOCKET_DEVICE_ERROR;
			rp2040_release_client(client);
			cyw43_arch_lwip_end();
			rp2040_events->closed(close_token, close_reason);
			return close_reason;
		}

		client->rx_offset = (uint16_t)(client->rx_offset + copied);

		if (client->pcb)
		{
			/* Advertise receive-window space only for bytes returned to uCNC. */
			tcp_recved(client->pcb, copied);
		}

		if (client->rx_offset == p->tot_len)
		{
			client->rx = NULL;
			client->rx_offset = 0U;
			pbuf_free(p);
		}

		/*
		 * Return final payload before reporting a pending FIN/fatal close.
		 * poll() or a later recv() will emit closed() after no retained RX
		 * remains, so the core cannot prioritize closure over this data.
		 */
		cyw43_arch_lwip_end();
		return (int)copied;
	}

	if ((client->flags & RP2040_CLIENT_CLOSE_PENDING) != 0U || !client->pcb)
	{
		close_token = client->token;
		close_reason =
			(client->flags & RP2040_CLIENT_CLOSE_PENDING) != 0U
				? (int)client->close_reason
				: SOCKET_DEVICE_ERROR;
		rp2040_release_client(client);
		cyw43_arch_lwip_end();
		rp2040_events->closed(close_token, close_reason);
		return close_reason;
	}

	cyw43_arch_lwip_end();
	return SOCKET_DEVICE_WOULD_BLOCK;
}

static int rp2040_socket_send(socket_device_handle_t handle,
								const void *source,
								size_t length)
{
	rp2040_client_t *client;
	size_t attempt;
	u16_t available;
	err_t err;
	socket_device_token_t close_token;
	int close_reason;
	bool emit_close = false;

	if (length == 0U)
	{
		return 0;
	}
	if (!source)
	{
		return SOCKET_DEVICE_INVALID;
	}

	cyw43_arch_lwip_begin();

	client = rp2040_find_client(handle);
	if (!client || client->token == SOCKET_DEVICE_INVALID_TOKEN)
	{
		cyw43_arch_lwip_end();
		return SOCKET_DEVICE_INVALID;
	}

	/*
	 * Once native closure is known, do not enqueue new TX. If final RX is still
	 * retained, the close event must remain deferred until that RX drains.
	 */
	if ((client->flags & RP2040_CLIENT_CLOSE_PENDING) != 0U || !client->pcb)
	{
		close_reason =
			(client->flags & RP2040_CLIENT_CLOSE_PENDING) != 0U
				? (int)client->close_reason
				: SOCKET_DEVICE_ERROR;

		if (client->rx)
		{
			cyw43_arch_lwip_end();
			return close_reason;
		}

		close_token = client->token;
		rp2040_release_client(client);
		cyw43_arch_lwip_end();
		rp2040_events->closed(close_token, close_reason);
		return close_reason;
	}

	available = tcp_sndbuf(client->pcb);
	if (available == 0U)
	{
		cyw43_arch_lwip_end();
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	attempt = length;
	if (attempt > (size_t)available)
	{
		attempt = (size_t)available;
	}
	if (attempt > (size_t)UINT16_MAX)
	{
		attempt = (size_t)UINT16_MAX;
	}

	/*
	 * Exactly one native enqueue attempt. COPY guarantees the caller's source
	 * pointer is not retained after this function returns.
	 */
	err = tcp_write(client->pcb,
					(const uint8_t *)source,
					(u16_t)attempt,
					TCP_WRITE_FLAG_COPY);

	if (err == ERR_MEM || err == ERR_WOULDBLOCK)
	{
		cyw43_arch_lwip_end();
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	if (err != ERR_OK)
	{
		close_reason = rp2040_fatal_result(err);
		close_token = client->token;
		rp2040_mark_close(client, close_reason);
		rp2040_abort_client_pcb(client);

		if (!client->rx)
		{
			rp2040_release_client(client);
			emit_close = true;
		}

		cyw43_arch_lwip_end();
		if (emit_close)
		{
			rp2040_events->closed(close_token, close_reason);
		}
		return close_reason;
	}

	/*
	 * tcp_write() has accepted the whole attempted prefix. tcp_output() merely
	 * prompts immediate transmission; its failure does not undo the enqueue.
	 * lwIP timers/ACK processing retain ownership and retry queued segments.
	 */
	(void)tcp_output(client->pcb);

	cyw43_arch_lwip_end();
	return (int)attempt;
}

static int rp2040_socket_close(socket_device_handle_t handle)
{
	rp2040_listener_t *listener;
	rp2040_client_t *client;
	size_t i;

	cyw43_arch_lwip_begin();

	listener = rp2040_find_listener(handle);
	if (listener)
	{
		/*
		 * Clients already registered with the uCNC core are closed by
		 * socket_stop() itself. Only accept-pending clients are invisible to the
		 * core; release those here so listener shutdown cannot orphan them.
		 */
		for (i = 0U; i < SOCKET_MAX_CONNECTIONS; ++i)
		{
			rp2040_client_t *pending = &rp2040_clients[i];

			if ((pending->flags & RP2040_CLIENT_IN_USE) != 0U &&
				pending->token == SOCKET_DEVICE_INVALID_TOKEN &&
				pending->listener_handle == handle)
			{
				rp2040_release_client(pending);
			}
		}

		rp2040_release_listener(listener);
		cyw43_arch_lwip_end();
		return SOCKET_DEVICE_OK;
	}

	client = rp2040_find_client(handle);
	if (client)
	{
		/* Local closure deliberately emits no events->closed() callback. */
		rp2040_release_client(client);
		cyw43_arch_lwip_end();
		return SOCKET_DEVICE_OK;
	}

	cyw43_arch_lwip_end();
	return SOCKET_DEVICE_INVALID;
}

static void rp2040_socket_poll(uint16_t budget)
{
	uint16_t emitted = 0U;

	/*
	 * Poll-architecture builds have no background driver/lwIP servicing.
	 * This call is non-blocking native housekeeping and is required for ACKs,
	 * timers, refused RX retries and TX-capacity progress. Background/RTOS
	 * architectures service those independently.
	 */
#if defined(PICO_CYW43_ARCH_POLL) && PICO_CYW43_ARCH_POLL
	cyw43_arch_poll();
#endif

	while (emitted < budget)
	{
		rp2040_event_kind_t event = RP2040_EVENT_NONE;
		uint16_t selected_index = UINT16_MAX;
		uint16_t selected_generation = 0U;
		socket_device_handle_t client_handle = SOCKET_DEVICE_INVALID_HANDLE;
		socket_device_handle_t listener_handle = SOCKET_DEVICE_INVALID_HANDLE;
		socket_device_token_t token = SOCKET_DEVICE_INVALID_TOKEN;
		int reason = SOCKET_DEVICE_INVALID;
		uint16_t checked;

		/* Snapshot at most one event in round-robin order. */
		cyw43_arch_lwip_begin();
		for (checked = 0U; checked < SOCKET_MAX_CONNECTIONS; ++checked)
		{
			uint16_t index = rp2040_poll_cursor;
			rp2040_client_t *candidate = &rp2040_clients[index];

			rp2040_poll_cursor =
				(uint16_t)((rp2040_poll_cursor + 1U) %
						   SOCKET_MAX_CONNECTIONS);

			if ((candidate->flags & RP2040_CLIENT_IN_USE) == 0U)
			{
				continue;
			}

			/* Acceptance is always the first normalized event for a client. */
			if ((candidate->flags & RP2040_CLIENT_ACCEPT_PENDING) != 0U)
			{
				candidate->flags &= (uint8_t)~RP2040_CLIENT_ACCEPT_PENDING;
				event = RP2040_EVENT_ACCEPTED;
				selected_index = index;
				selected_generation = candidate->generation;
				client_handle =
					rp2040_client_handle(index, candidate->generation);
				listener_handle = candidate->listener_handle;
				break;
			}

			if (candidate->token == SOCKET_DEVICE_INVALID_TOKEN)
			{
				continue;
			}

			/* Retained final payload must be visible before closure. */
			if (candidate->rx &&
				(candidate->flags & RP2040_CLIENT_READABLE_PENDING) != 0U)
			{
				candidate->flags &=
					(uint8_t)~RP2040_CLIENT_READABLE_PENDING;
				event = RP2040_EVENT_READABLE;
				token = candidate->token;
				break;
			}

			if (!candidate->rx &&
				(candidate->flags & RP2040_CLIENT_CLOSE_PENDING) != 0U)
			{
				event = RP2040_EVENT_CLOSED;
				token = candidate->token;
				reason = (int)candidate->close_reason;
				rp2040_release_client(candidate);
				break;
			}
		}
		cyw43_arch_lwip_end();

		if (event == RP2040_EVENT_NONE)
		{
			break;
		}

		switch (event)
		{
		case RP2040_EVENT_ACCEPTED:
		{
			socket_device_token_t accepted_token =
				rp2040_events->accepted(listener_handle, client_handle);

			/*
			 * Store the token only if this is still the exact generation that
			 * produced the event. Never hold lwIP protection while calling the
			 * uCNC event sink.
			 */
			cyw43_arch_lwip_begin();
			if (selected_index < SOCKET_MAX_CONNECTIONS)
			{
				rp2040_client_t *selected =
					&rp2040_clients[selected_index];

				if ((selected->flags & RP2040_CLIENT_IN_USE) != 0U &&
					selected->generation == selected_generation &&
					selected->token == SOCKET_DEVICE_INVALID_TOKEN)
				{
					if (accepted_token == SOCKET_DEVICE_INVALID_TOKEN)
					{
						/* Core rejection is a local release: no closed event. */
						rp2040_release_client(selected);
					}
					else
					{
						selected->token = accepted_token;
						if (selected->rx)
						{
							selected->flags |=
								RP2040_CLIENT_READABLE_PENDING;
						}
					}
				}
			}
			cyw43_arch_lwip_end();
			break;
		}

		case RP2040_EVENT_READABLE:
			rp2040_events->readable(token);
			break;

		case RP2040_EVENT_CLOSED:
			rp2040_events->closed(token, reason);
			break;

		default:
			break;
		}

		++emitted;
	}
}

/* Public backend object registered by the RP2040 network initialization. */
socket_device_t wifi_socket = {
	.init = rp2040_socket_init,
	.listen = rp2040_socket_listen,
	.recv = rp2040_socket_recv,
	.send = rp2040_socket_send,
	.close = rp2040_socket_close,
	.poll = rp2040_socket_poll};

#ifdef __cplusplus
}
#endif

#endif /* MCU == MCU_RP2040 */
