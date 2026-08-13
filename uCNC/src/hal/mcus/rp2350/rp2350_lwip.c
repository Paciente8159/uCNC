/*
	Name: rp2350_lwip.c
	Description: Implements an event-driven lwIP TCP socket backend for the RP2350 (µCNC socket device).

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 25-08-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

#if (MCU == MCU_RP2350)

#ifdef __cplusplus
extern "C"
{
#endif

#include "pico/cyw43_arch.h"
#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "lwip/pbuf.h"
#include "../../../modules/net/socket.h"

#ifndef RP2350_SOCKET_MAX_HANDLES
#define RP2350_SOCKET_MAX_HANDLES \
	(MAX_SOCKETS * (SOCKET_MAX_CLIENTS + 1))
#endif

/*
 * Number of complete lwIP receive-pbuf chains that can be retained
 * per TCP client before applying backpressure with ERR_MEM.
 *
 * This only allocates pointers, not SOCKET_MAX_DATA_SIZE buffers.
 */
#ifndef RP2350_SOCKET_RX_QUEUE_DEPTH
#define RP2350_SOCKET_RX_QUEUE_DEPTH 4
#endif

	typedef enum
	{
		FD_EMPTY = 0,
		FD_LISTENER,
		FD_CLIENT
	} fd_kind_t;

	typedef struct
	{
		fd_kind_t kind;
		bool in_use;
		uint32_t generation;

		/*
		 * Deferred events.
		 *
		 * lwIP callbacks only modify these flags/state.
		 * µCNC callbacks are invoked later from rp2350_socket_service().
		 */
		bool accepted;
		bool peer_closed;
		bool got_err;
		bool writable_pending;

		bool write_blocked;

		int error_reason;

		/*
		 * lwIP PCBs.
		 *
		 * A listener uses listen_pcb.
		 * A client uses pcb.
		 */
		struct tcp_pcb *pcb;
		struct tcp_pcb *listen_pcb;

		/*
		 * Generic µCNC handles.
		 */
		socket_handle_t srv_handle;
		socket_handle_t client_handle;

		/*
		 * Receive queue.
		 *
		 * Each entry is one complete pbuf chain supplied by one
		 * tcp_recv() callback. Do NOT pbuf_chain() separate received
		 * packets together.
		 */
		struct pbuf *rx_queue[RP2350_SOCKET_RX_QUEUE_DEPTH];

		uint8_t rx_head;
		uint8_t rx_tail;
		uint8_t rx_count;

		/*
		 * Offset already consumed from rx_queue[rx_head].
		 */
		uint16_t rx_off;

		/*
		 * Owning listener index.
		 */
		int srv_fd;

	} fd_entry_t;

	static fd_entry_t g_fds[RP2350_SOCKET_MAX_HANDLES];
	static uint32_t g_fd_generations[RP2350_SOCKET_MAX_HANDLES];

	static const socket_device_events_t *rp2350_socket_events;

	/*
	 * Application callbacks may re-enter service(). Only the outermost pass
	 * may poll CYW43/lwIP or dispatch accept/RX events, which protects the
	 * shared RX callback buffer from being overwritten while it is exposed.
	 */
	static unsigned int rp2350_service_depth;
	static char rp2350_rx_dispatch_buffer[SOCKET_MAX_DATA_SIZE + 1];

	static int fd_alloc(void)
	{
		for (int i = 0; i < RP2350_SOCKET_MAX_HANDLES; i++)
		{
			if (!g_fds[i].in_use)
			{
				uint32_t generation = ++g_fd_generations[i];

				/* Keep zero reserved for reset/unallocated entries. */
				if (generation == 0)
				{
					generation = ++g_fd_generations[i];
				}

				memset(&g_fds[i], 0, sizeof(g_fds[i]));

				g_fds[i].in_use = true;
				g_fds[i].kind = FD_EMPTY;
				g_fds[i].generation = generation;

				g_fds[i].srv_fd = -1;

				g_fds[i].srv_handle = SOCKET_INVALID_HANDLE;
				g_fds[i].client_handle = SOCKET_INVALID_HANDLE;

				return i;
			}
		}

		return -1;
	}

	static void rx_queue_clear(fd_entry_t *e)
	{
		if (!e)
		{
			return;
		}

		while (e->rx_count)
		{
			struct pbuf *p = e->rx_queue[e->rx_head];

			e->rx_queue[e->rx_head] = NULL;
			e->rx_head = (uint8_t)((e->rx_head + 1) % RP2350_SOCKET_RX_QUEUE_DEPTH);
			e->rx_count--;

			if (p)
			{
				/*
				 * Release the reference owned by this backend.
				 */
				pbuf_free(p);
			}
		}

		e->rx_head = 0;
		e->rx_tail = 0;
		e->rx_count = 0;
		e->rx_off = 0;
	}

	static void fd_free(int fd)
	{
		if (fd < 0 || fd >= RP2350_SOCKET_MAX_HANDLES)
		{
			return;
		}

		fd_entry_t *e = &g_fds[fd];

		if (!e->in_use)
		{
			return;
		}

		/*
		 * Release queued receive buffers first.
		 */
		rx_queue_clear(e);

		/*
		 * Close listener PCB.
		 */
		if (e->listen_pcb)
		{
			struct tcp_pcb *pcb = e->listen_pcb;

			e->listen_pcb = NULL;

			tcp_arg(pcb, NULL);
			tcp_accept(pcb, NULL);

			err_t close_err = tcp_close(pcb);

			/*
			 * tcp_close() may fail with ERR_MEM.
			 *
			 * socket_device->close() means the connection is definitely
			 * finished, so fall back to an abort instead of leaking the PCB.
			 */
			if (close_err != ERR_OK)
			{
				tcp_abort(pcb);
			}
		}

		/*
		 * Close client PCB.
		 *
		 * If tcp_err() has already executed then e->pcb will already
		 * be NULL because lwIP freed it before invoking err_cb().
		 */
		if (e->pcb)
		{
			struct tcp_pcb *pcb = e->pcb;

			e->pcb = NULL;

			tcp_arg(pcb, NULL);
			tcp_recv(pcb, NULL);
			tcp_sent(pcb, NULL);
			tcp_poll(pcb, NULL, 0);
			tcp_err(pcb, NULL);

			err_t close_err = tcp_close(pcb);

			if (close_err != ERR_OK)
			{
				tcp_abort(pcb);
			}
		}

		memset(e, 0, sizeof(*e));

		e->srv_fd = -1;
		e->srv_handle = SOCKET_INVALID_HANDLE;
		e->client_handle = SOCKET_INVALID_HANDLE;
	}

	static bool fd_is_same_client(
		const fd_entry_t *e,
		uint32_t generation,
		socket_handle_t handle,
		const struct tcp_pcb *pcb)
	{
		return e->in_use &&
			e->kind == FD_CLIENT &&
			e->generation == generation &&
			e->client_handle == handle &&
			e->pcb == pcb;
	}

	static err_t accept_cb(
		void *arg,
		struct tcp_pcb *newpcb,
		err_t err);

	static err_t recv_cb(
		void *arg,
		struct tcp_pcb *tpcb,
		struct pbuf *p,
		err_t err);

	static err_t sent_cb(
		void *arg,
		struct tcp_pcb *tpcb,
		u16_t len);

	static err_t poll_cb(
		void *arg,
		struct tcp_pcb *tpcb);

	static void err_cb(
		void *arg,
		err_t err);

	static int rp2350_socket_map_error(err_t err)
	{
		switch (err)
		{
		case ERR_MEM:
			return SOCKET_DEVICE_NO_MEMORY;

		case ERR_CLSD:
			return SOCKET_DEVICE_CLOSED;

		default:
			return SOCKET_DEVICE_ERROR;
		}
	}

	static err_t accept_cb(
		void *arg,
		struct tcp_pcb *newpcb,
		err_t err)
	{
		if (err != ERR_OK)
		{
			return err;
		}

		if (!newpcb)
		{
			return ERR_ARG;
		}

		int srv_fd = (int)(intptr_t)arg;

		if (srv_fd < 0 ||
			srv_fd >= RP2350_SOCKET_MAX_HANDLES ||
			!g_fds[srv_fd].in_use ||
			g_fds[srv_fd].kind != FD_LISTENER)
		{
			/*
			 * We are inside an lwIP callback. If tcp_abort() is called
			 * here we must return ERR_ABRT.
			 */
			tcp_abort(newpcb);
			return ERR_ABRT;
		}

		int cfd = fd_alloc();

		if (cfd < 0)
		{
			tcp_abort(newpcb);
			return ERR_ABRT;
		}

		fd_entry_t *e = &g_fds[cfd];

		e->kind = FD_CLIENT;

		e->pcb = newpcb;
		e->listen_pcb = NULL;

		e->srv_fd = srv_fd;

		e->srv_handle = (socket_handle_t)srv_fd;
		e->client_handle = (socket_handle_t)cfd;

		e->accepted = true;

		e->peer_closed = false;
		e->got_err = false;
		e->writable_pending = false;
		e->write_blocked = false;

		e->rx_head = 0;
		e->rx_tail = 0;
		e->rx_count = 0;
		e->rx_off = 0;

		/*
		 * All callbacks receive our fd index through tcp_arg().
		 */
		tcp_arg(
			newpcb,
			(void *)(intptr_t)cfd);

		tcp_recv(
			newpcb,
			recv_cb);

		tcp_sent(
			newpcb,
			sent_cb);

		tcp_err(
			newpcb,
			err_cb);

		/*
		 * Polling provides a fallback wakeup for TX ERR_MEM cases
		 * where no future ACK necessarily arrives.
		 *
		 * Interval 1 corresponds to one lwIP coarse TCP timer tick.
		 */
		tcp_poll(
			newpcb,
			poll_cb,
			1);

		return ERR_OK;
	}

	static err_t sent_cb(
		void *arg,
		struct tcp_pcb *tpcb,
		u16_t len)
	{
		(void)tpcb;
		(void)len;

		int fd = (int)(intptr_t)arg;

		if (fd < 0 ||
			fd >= RP2350_SOCKET_MAX_HANDLES)
		{
			return ERR_OK;
		}

		fd_entry_t *e = &g_fds[fd];

		if (!e->in_use ||
			e->kind != FD_CLIENT)
		{
			return ERR_OK;
		}

		/*
		 * A previous socket_send() encountered backpressure.
		 *
		 * Only set a deferred event here. Do not call µCNC application
		 * code from inside the lwIP callback.
		 */
		if (e->write_blocked)
		{
			e->write_blocked = false;
			e->writable_pending = true;
		}

		return ERR_OK;
	}

	static err_t poll_cb(
		void *arg,
		struct tcp_pcb *tpcb)
	{
		(void)tpcb;

		int fd = (int)(intptr_t)arg;

		if (fd < 0 ||
			fd >= RP2350_SOCKET_MAX_HANDLES)
		{
			return ERR_OK;
		}

		fd_entry_t *e = &g_fds[fd];

		if (!e->in_use ||
			e->kind != FD_CLIENT)
		{
			return ERR_OK;
		}

		/*
		 * tcp_write() may return ERR_MEM even when there is no future
		 * tcp_sent() callback guaranteed to wake us.
		 *
		 * Ask the socket core to retry when lwIP polls us.
		 */
		if (e->write_blocked)
		{
			e->write_blocked = false;
			e->writable_pending = true;
		}

		return ERR_OK;
	}

	static void err_cb(
		void *arg,
		err_t err)
	{
		int fd = (int)(intptr_t)arg;

		if (fd < 0 ||
			fd >= RP2350_SOCKET_MAX_HANDLES)
		{
			return;
		}

		fd_entry_t *e = &g_fds[fd];

		if (!e->in_use ||
			e->kind != FD_CLIENT)
		{
			return;
		}

		/*
		 * IMPORTANT:
		 *
		 * lwIP has already freed the tcp_pcb before calling tcp_err().
		 * Never call tcp_close(), tcp_abort(), tcp_recv(), etc. on it.
		 */
		e->pcb = NULL;

		e->error_reason =
			rp2350_socket_map_error(err);

		e->got_err = true;

		e->write_blocked = false;
		e->writable_pending = false;
	}

	static err_t recv_cb(
		void *arg,
		struct tcp_pcb *tpcb,
		struct pbuf *p,
		err_t err)
	{
		int fd = (int)(intptr_t)arg;

		if (fd < 0 ||
			fd >= RP2350_SOCKET_MAX_HANDLES)
		{
			/*
			 * We have no valid owner for this connection anymore.
			 *
			 * Since we're aborting from an lwIP callback, release p and
			 * return ERR_ABRT.
			 */
			if (p)
			{
				pbuf_free(p);
			}

			tcp_abort(tpcb);
			return ERR_ABRT;
		}

		fd_entry_t *e = &g_fds[fd];

		if (!e->in_use ||
			e->kind != FD_CLIENT)
		{
			if (p)
			{
				pbuf_free(p);
			}

			tcp_abort(tpcb);
			return ERR_ABRT;
		}

		/*
		 * If lwIP passed an error, return it without freeing p.
		 *
		 * The tcp_recv contract says a callback returning something
		 * other than ERR_OK/ERR_ABRT must not free the pbuf.
		 */
		if (err != ERR_OK)
		{
			e->error_reason =
				rp2350_socket_map_error(err);

			e->got_err = true;

			return err;
		}

		/*
		 * p == NULL means the peer sent FIN.
		 *
		 * Do not close immediately. There may still be queued receive
		 * data waiting to be delivered to µCNC.
		 */
		if (!p)
		{
			e->peer_closed = true;
			return ERR_OK;
		}

		/*
		 * No payload worth queueing.
		 */
		if (p->tot_len == 0)
		{
			pbuf_free(p);
			return ERR_OK;
		}

		/*
		 * Queue full:
		 *
		 * Do NOT free p and return ERR_MEM. This tells lwIP that we
		 * have not accepted this receive buffer yet.
		 */
		if (e->rx_count >= RP2350_SOCKET_RX_QUEUE_DEPTH)
		{
			return ERR_MEM;
		}

		/*
		 * Take our own reference to this complete pbuf chain.
		 *
		 * We must then release the reference passed to the tcp_recv
		 * callback before returning ERR_OK.
		 */
		pbuf_ref(p);

		e->rx_queue[e->rx_tail] = p;

		e->rx_tail =
			(uint8_t)((e->rx_tail + 1) %
					  RP2350_SOCKET_RX_QUEUE_DEPTH);

		e->rx_count++;

		/*
		 * Release the callback's reference.
		 *
		 * Our pbuf_ref() above keeps the pbuf alive until service()
		 * consumes it.
		 */
		pbuf_free(p);

		return ERR_OK;
	}

	static size_t rp2350_socket_copy_rx(
		fd_entry_t *e,
		char *buffer)
	{
		if (!e ||
			!buffer ||
			e->rx_count == 0)
		{
			return 0;
		}

		struct pbuf *p =
			e->rx_queue[e->rx_head];

		if (!p)
		{
			e->got_err = true;
			e->error_reason = SOCKET_DEVICE_ERROR;
			return 0;
		}

		if (e->rx_off >= p->tot_len)
		{
			/* Inconsistent queue state: do not silently discard data. */
			e->got_err = true;
			e->error_reason = SOCKET_DEVICE_ERROR;
			return 0;
		}

		size_t remaining =
			(size_t)p->tot_len -
			(size_t)e->rx_off;

		size_t to_copy = remaining;

		if (to_copy > SOCKET_MAX_DATA_SIZE)
		{
			to_copy = SOCKET_MAX_DATA_SIZE;
		}

		u16_t copied =
			pbuf_copy_partial(
				p,
				buffer,
				(u16_t)to_copy,
				e->rx_off);

		if (copied != (u16_t)to_copy)
		{
			e->got_err = true;
			e->error_reason = SOCKET_DEVICE_ERROR;

			return 0;
		}

		buffer[copied] = '\0';

		return (size_t)copied;
	}

	static void rp2350_socket_service(void)
	{
		if (!rp2350_socket_events)
		{
			return;
		}

		bool allow_rx_and_accept = (rp2350_service_depth == 0);
		rp2350_service_depth++;

		/*
		 * For pico_cyw43_arch_lwip_poll the application must
		 * periodically drive CYW43/lwIP itself. Never poll recursively:
		 * it can run raw lwIP callbacks while an RX buffer is exposed.
		 */
#if defined(PICO_CYW43_ARCH_POLL) && PICO_CYW43_ARCH_POLL
		if (allow_rx_and_accept)
		{
			cyw43_arch_poll();
		}
#endif

		/*
		 * Bounded pass:
		 *
		 * At most one µCNC socket event is emitted for each handle
		 * during one service invocation.
		 */
		for (int i = 0;
			 i < RP2350_SOCKET_MAX_HANDLES;
			 i++)
		{
			fd_entry_t *e = &g_fds[i];

			if (!e->in_use ||
				e->kind != FD_CLIENT)
			{
				continue;
			}

			/*
			 * Fatal error has highest priority.
			 *
			 * The PCB may already have been freed by lwIP.
			 */
			if (e->got_err)
			{
				socket_handle_t handle =
					e->client_handle;

				int reason =
					e->error_reason;

				e->got_err = false;

				fd_free(i);

				if (rp2350_socket_events->disconnected)
				{
					rp2350_socket_events->disconnected(
						handle,
						reason);
				}

				continue;
			}

			/*
			 * Notify µCNC of the accepted connection before delivering
			 * any data that may already have arrived.
			 */
			if (e->accepted)
			{
				if (!allow_rx_and_accept)
				{
					continue;
				}

				uint32_t generation = e->generation;
				struct tcp_pcb *pcb = e->pcb;

				socket_handle_t listener =
					e->srv_handle;

				socket_handle_t client =
					e->client_handle;

				e->accepted = false;

				bool accepted = false;

				if (rp2350_socket_events->connected)
				{
					accepted =
						rp2350_socket_events->connected(
							listener,
							client);
				}

				/*
				 * No free µCNC client slot.
				 */
				if (!accepted)
				{
					/*
					 * The connected callback returning false cannot call
					 * application code, so this descriptor is still ours.
					 */
					if (fd_is_same_client(
							e,
							generation,
							client,
							pcb))
					{
						fd_free(i);
					}
				}

				/*
				 * Do not touch e after the event callback. Application
				 * code may have closed the socket.
				 */
				continue;
			}

			/*
			 * Deliver queued RX before handling peer FIN.
			 */
			if (e->rx_count)
			{
				if (!allow_rx_and_accept)
				{
					/* The outer data() callback still owns the shared buffer. */
					continue;
				}

				uint32_t generation = e->generation;
				socket_handle_t client =
					e->client_handle;
				struct tcp_pcb *pcb = e->pcb;
				uint8_t rx_head = e->rx_head;
				uint16_t rx_off = e->rx_off;
				struct pbuf *p = e->rx_queue[rx_head];

				size_t len =
					rp2350_socket_copy_rx(
						e,
						rp2350_rx_dispatch_buffer);

				if (len > 0)
				{
					bool consumed = true;

					if (rp2350_socket_events->data)
					{
						consumed = rp2350_socket_events->data(
							client,
							rp2350_rx_dispatch_buffer,
							len);
					}

					/* data() may close the client, and a nested service pass may
					 * complete a fatal disconnect. Never commit into a reset or
					 * reused descriptor. */
					if (!fd_is_same_client(
							e,
							generation,
							client,
							pcb) ||
						e->rx_count == 0 ||
						e->rx_head != rx_head ||
						e->rx_queue[rx_head] != p ||
						e->rx_off != rx_off)
					{
						continue;
					}

					if (!consumed)
					{
						/* New data() contract: retain the pbuf, cursor and TCP
						 * receive window unchanged. The same bytes are offered again
						 * during a later outer service pass. */
						continue;
					}

					/* Commit only after complete consumption was confirmed. */
					e->rx_off = (uint16_t)(e->rx_off + len);
					tcp_recved(e->pcb, (u16_t)len);

					if (e->rx_off >= p->tot_len)
					{
						e->rx_queue[e->rx_head] = NULL;
						e->rx_head =
							(uint8_t)((e->rx_head + 1) %
									  RP2350_SOCKET_RX_QUEUE_DEPTH);
						e->rx_count--;
						e->rx_off = 0;
						pbuf_free(p);
					}

					continue;
				}

				/*
				 * An RX processing error was raised.
				 * Deal with it on the next service pass.
				 */
				if (e->got_err)
				{
					continue;
				}
			}

			/*
			 * Only report FIN after all queued payload has been delivered.
			 */
			if (e->peer_closed &&
				e->rx_count == 0)
			{
				socket_handle_t client =
					e->client_handle;

				e->peer_closed = false;

				fd_free(i);

				if (rp2350_socket_events->disconnected)
				{
					rp2350_socket_events->disconnected(
						client,
						0);
				}

				continue;
			}

			/*
			 * Deferred TX-progress notification.
			 */
			if (e->writable_pending)
			{
				socket_handle_t client =
					e->client_handle;

				e->writable_pending = false;

				if (rp2350_socket_events->writable)
				{
					rp2350_socket_events->writable(
						client);
				}

				/*
				 * writable() may eventually contain application logic
				 * which closes the connection, so don't touch e again.
				 */
				continue;
			}
		}

		rp2350_service_depth--;
	}

	static int rp2350_socket_device_init(
		const socket_device_events_t *events)
	{
		if (!events)
		{
			return SOCKET_DEVICE_INVALID;
		}

		rp2350_socket_events = events;

		memset(
			g_fds,
			0,
			sizeof(g_fds));

		memset(
			g_fd_generations,
			0,
			sizeof(g_fd_generations));

		rp2350_service_depth = 0;

		for (int i = 0;
			 i < RP2350_SOCKET_MAX_HANDLES;
			 i++)
		{
			g_fds[i].kind = FD_EMPTY;

			g_fds[i].srv_fd = -1;

			g_fds[i].srv_handle =
				SOCKET_INVALID_HANDLE;

			g_fds[i].client_handle =
				SOCKET_INVALID_HANDLE;
		}

		return SOCKET_DEVICE_OK;
	}

	static socket_handle_t rp2350_socket_listen(
		uint32_t ip_listen,
		uint16_t port,
		int domain,
		int type,
		int protocol,
		uint8_t backlog)
	{
		(void)protocol;

		if (domain != AF_INET ||
			type != SOCK_STREAM)
		{
			return SOCKET_INVALID_HANDLE;
		}

		/*
		 * Current RP2350 implementation only supports listening on
		 * all local IPv4 interfaces.
		 *
		 * Do not silently ignore a non-zero requested bind address.
		 */
		if (ip_listen != IP_ANY)
		{
			return SOCKET_INVALID_HANDLE;
		}

		int fd = fd_alloc();

		if (fd < 0)
		{
			return SOCKET_INVALID_HANDLE;
		}

		fd_entry_t *e = &g_fds[fd];

		e->kind = FD_LISTENER;

		e->srv_handle =
			(socket_handle_t)fd;

		struct tcp_pcb *pcb =
			tcp_new_ip_type(
				IPADDR_TYPE_V4);

		if (!pcb)
		{
			fd_free(fd);
			return SOCKET_INVALID_HANDLE;
		}

		/*
		 * Store it immediately so fd_free() owns all cleanup paths.
		 */
		e->listen_pcb = pcb;

		tcp_setprio(
			pcb,
			TCP_PRIO_NORMAL);

		pcb->so_options |= SOF_REUSEADDR;

		err_t err =
			tcp_bind(
				pcb,
				IP_ANY_TYPE,
				port);

		if (err != ERR_OK)
		{
			fd_free(fd);
			return SOCKET_INVALID_HANDLE;
		}

		struct tcp_pcb *listener =
			tcp_listen_with_backlog(
				pcb,
				backlog ? backlog : 1);

		if (!listener)
		{
			/*
			 * tcp_listen_with_backlog() failed; the original PCB is
			 * still owned by e->listen_pcb and fd_free() releases it.
			 */
			fd_free(fd);
			return SOCKET_INVALID_HANDLE;
		}

		/*
		 * tcp_listen_with_backlog() replaces the original PCB with
		 * the listener PCB.
		 */
		e->listen_pcb = listener;

		tcp_arg(
			listener,
			(void *)(intptr_t)fd);

		tcp_accept(
			listener,
			accept_cb);

		return (socket_handle_t)fd;
	}

	static int rp2350_socket_send(
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

		int fd = (int)client;

		if (fd < 0 ||
			fd >= RP2350_SOCKET_MAX_HANDLES)
		{
			return SOCKET_DEVICE_INVALID;
		}

		fd_entry_t *e = &g_fds[fd];

		if (!e->in_use ||
			e->kind != FD_CLIENT ||
			!e->pcb)
		{
			return SOCKET_DEVICE_INVALID;
		}

		u16_t space =
			tcp_sndbuf(e->pcb);

		if (space == 0)
		{
			/*
			 * Never wait here.
			 */
			e->write_blocked = true;

			return SOCKET_DEVICE_WOULD_BLOCK;
		}

		size_t to_send = len;

		if (to_send > (size_t)space)
		{
			to_send = (size_t)space;
		}

		/*
		 * tcp_write() accepts a u16 length.
		 * tcp_sndbuf() already constrains to_send accordingly.
		 */
		err_t err =
			tcp_write(
				e->pcb,
				data,
				(u16_t)to_send,
				TCP_WRITE_FLAG_COPY);

		if (err == ERR_MEM)
		{
			/*
			 * Queue/buffer pressure.
			 *
			 * sent_cb() or poll_cb() will eventually mark this socket
			 * writable again.
			 */
			e->write_blocked = true;

			return SOCKET_DEVICE_WOULD_BLOCK;
		}

		if (err != ERR_OK)
		{
			return rp2350_socket_map_error(err);
		}

		e->write_blocked = false;

		/*
		 * If the application happened to retry successfully before a
		 * previously queued writable event was dispatched, discard that
		 * now-stale notification.
		 */
		e->writable_pending = false;

		/*
		 * Data has been accepted into lwIP's TX queue at this point.
		 */
		(void)tcp_output(e->pcb);

		return (int)to_send;
	}

	static int rp2350_socket_close(
		socket_handle_t handle)
	{
		if (handle == SOCKET_INVALID_HANDLE)
		{
			return SOCKET_DEVICE_INVALID;
		}

		int fd = (int)handle;

		if (fd < 0 ||
			fd >= RP2350_SOCKET_MAX_HANDLES)
		{
			return SOCKET_DEVICE_INVALID;
		}

		if (!g_fds[fd].in_use)
		{
			return SOCKET_DEVICE_INVALID;
		}

		/*
		 * Explicit local close.
		 *
		 * fd_free() deliberately does not invoke the µCNC disconnected
		 * event. The generic socket core handles local-close notification.
		 */
		fd_free(fd);

		return SOCKET_DEVICE_OK;
	}

	socket_device_t wifi_socket =
		{
			.init = rp2350_socket_device_init,
			.listen = rp2350_socket_listen,
			.send = rp2350_socket_send,
			.close = rp2350_socket_close,
			.service = rp2350_socket_service};

#ifdef __cplusplus
}
#endif
#endif
