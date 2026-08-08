/*
	Name: rp2040_lwip.c
	Description: Implements an event-driven lwIP TCP socket backend for the RP2040 (µCNC socket device).

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

#if (MCU == MCU_RP2040)

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

#ifndef BSD_MAX_FDS
#define BSD_MAX_FDS (MAX_SOCKETS * SOCKET_MAX_CLIENTS)
#endif

	typedef enum
	{
		FD_EMPTY = 0,
		FD_LISTENER,
		FD_CLIENT
	} fd_kind_t;

	typedef struct
	{
		/* Event flags set from the lwIP TCP callbacks. They are checked and
		   cleared inside rp2040_socket_service(), which runs in the µCNC
		   cooperative context, so the event sink is never invoked from the
		   lwIP callback context. */
		volatile bool accepted;
		volatile bool data_pending;
		volatile bool peer_closed;
		volatile bool got_err;

		/* Client socket handles (back-reference used by the events) */
		socket_handle_t srv_handle;
		socket_handle_t client_handle;

		fd_kind_t kind;
		bool in_use;
		bool write_blocked;
		struct tcp_pcb *pcb;
		struct tcp_pcb *listen_pcb;

		/* RX: pbuf chain owned by this backend until consumed */
		struct pbuf *rx_chain;
		uint32_t rx_off;
		bool rx_closed;

		/* TX: pointer into the caller's buffer (valid only during send()) */
		const uint8_t *tx_buf;
		size_t tx_len;
		int srv_fd;
	} fd_entry_t;

	static fd_entry_t g_fds[BSD_MAX_FDS];
	static const socket_device_events_t *rp2040_socket_events;

	static int fd_alloc(void)
	{
		for (int i = 0; i < BSD_MAX_FDS; i++)
		{
			if (!g_fds[i].in_use)
			{
				memset(&g_fds[i], 0, sizeof(g_fds[i]));
				g_fds[i].in_use = true;
				g_fds[i].srv_fd = -1;
				g_fds[i].srv_handle = SOCKET_INVALID_HANDLE;
				g_fds[i].client_handle = SOCKET_INVALID_HANDLE;
				return i;
			}
		}
		return -1;
	}

	static void fd_free(int fd)
	{
		if (fd < 0 || fd >= BSD_MAX_FDS)
			return;
		fd_entry_t *e = &g_fds[fd];

		if (e->listen_pcb)
		{
			tcp_arg(e->listen_pcb, NULL);
			tcp_accept(e->listen_pcb, NULL);
			tcp_close(e->listen_pcb);
			e->listen_pcb = NULL;
		}
		if (e->pcb)
		{
			tcp_arg(e->pcb, NULL);
			tcp_recv(e->pcb, NULL);
			tcp_sent(e->pcb, NULL);
			tcp_err(e->pcb, NULL);
			tcp_close(e->pcb);
			e->pcb = NULL;
		}
		if (e->rx_chain)
		{
			pbuf_free(e->rx_chain);
			e->rx_chain = NULL;
			e->rx_off = 0;
			e->rx_closed = false;
		}
		memset(e, 0, sizeof(*e));
		e->srv_fd = -1;
		e->srv_handle = SOCKET_INVALID_HANDLE;
		e->client_handle = SOCKET_INVALID_HANDLE;
	}

	static err_t recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err); // fwd decl
	static err_t sent_cb(void *arg, struct tcp_pcb *tpcb, u16_t len);

	static err_t accept_cb(void *arg, struct tcp_pcb *newpcb, err_t err)
	{
		(void)err;
		int srv_fd = (int)(intptr_t)arg;

		int cfd = fd_alloc();
		if (cfd < 0)
		{
			tcp_abort(newpcb);
			return ERR_ABRT;
		}
		g_fds[cfd].kind = FD_CLIENT;
		g_fds[cfd].pcb = newpcb;
		g_fds[cfd].rx_chain = NULL;
		g_fds[cfd].rx_off = 0;
		g_fds[cfd].rx_closed = false;
		g_fds[cfd].srv_fd = srv_fd;
		g_fds[cfd].srv_handle = (socket_handle_t)srv_fd;
		g_fds[cfd].client_handle = (socket_handle_t)cfd;
		g_fds[cfd].accepted = true;

		tcp_arg(newpcb, (void *)(intptr_t)cfd);
		tcp_recv(newpcb, recv_cb); // <- receive path
		tcp_sent(newpcb, sent_cb);

		return ERR_OK;
	}

	static err_t sent_cb(void *arg, struct tcp_pcb *tpcb, u16_t len)
	{
		(void)tpcb;
		(void)len;
		int fd = (int)(intptr_t)arg;
		if (fd < 0 || fd >= BSD_MAX_FDS)
			return ERR_OK;

		fd_entry_t *e = &g_fds[fd];

		/* A previous send returned WOULD_BLOCK: report TX progress */
		if (e->write_blocked)
		{
			e->write_blocked = false;
			if (rp2040_socket_events && rp2040_socket_events->writable)
			{
				rp2040_socket_events->writable(e->client_handle);
			}
		}
		return ERR_OK;
	}

	static err_t recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
	{
		(void)tpcb;
		int fd = (int)(intptr_t)arg;

		if (fd < 0 || fd >= BSD_MAX_FDS)
		{
			if (p)
				pbuf_free(p);
			return ERR_OK;
		}

		fd_entry_t *e = &g_fds[fd];

		if (err != ERR_OK)
		{
			if (p)
				pbuf_free(p);
			e->got_err = true;
			return err;
		}

		if (p == NULL)
		{
			/* Remote closed connection (FIN). Drain remaining rx_chain via
			   the socket core, then report disconnect. */
			e->rx_closed = true;
			e->peer_closed = true;
			return ERR_OK;
		}

		/* Append pbuf(s) to our queued chain. We keep ownership until the
		   socket core consumes them (data event). */
		if (!e->rx_chain)
		{
			e->rx_chain = p;
			e->rx_off = 0;
		}
		else
		{
			/* Concatenate without changing refs; p will be part of the chain and freed later. */
			pbuf_cat(e->rx_chain, p);
		}
		e->data_pending = true;

		return ERR_OK;
	}

	static void rp2040_socket_service(void)
	{
		if (!rp2040_socket_events)
		{
			return;
		}

		/* Bounded pass: at most one event per client per service() call.
		   This keeps the work in the µCNC cooperative context bounded. */
		for (int i = 0; i < BSD_MAX_FDS; i++)
		{
			fd_entry_t *e = &g_fds[i];
			if (!e->in_use)
			{
				continue;
			}

			if (e->kind == FD_CLIENT && e->accepted)
			{
				e->accepted = false;
				if (!rp2040_socket_events->connected(e->srv_handle, e->client_handle))
				{
					/* No µCNC client slot: reject and discard */
					fd_free(i);
				}
				continue;
			}

			if (e->kind == FD_CLIENT && e->got_err)
			{
				e->got_err = false;
				socket_handle_t handle = e->client_handle;
				fd_free(i);
				rp2040_socket_events->disconnected(handle, SOCKET_DEVICE_ERROR);
				continue;
			}

			if (e->kind == FD_CLIENT && e->peer_closed)
			{
				socket_handle_t handle = e->client_handle;
				fd_free(i);
				rp2040_socket_events->disconnected(handle, 0);
				continue;
			}

			if (e->kind == FD_CLIENT && e->data_pending)
			{
				uint8_t *buf = NULL;

				/* Copy the received chain into a flat writable buffer with a
				   NUL terminator as required by the µCNC socket core. */
				size_t total = 0;
				struct pbuf *q = e->rx_chain;
				while (q)
				{
					total += q->len;
					q = q->next;
				}
				if (total > SOCKET_MAX_DATA_SIZE)
				{
					total = SOCKET_MAX_DATA_SIZE;
				}

				if (total)
				{
					static char srv_buffer[SOCKET_MAX_DATA_SIZE + 1];
					buf = (uint8_t *)srv_buffer;

					size_t copied = 0;
					uint32_t off = 0;
					q = e->rx_chain;
					while (q && copied < total)
					{
						size_t chunk = q->len - (size_t)off;
						if (chunk > (total - copied))
							chunk = total - copied;
						memcpy(buf + copied, ((uint8_t *)q->payload) + off, chunk);
						copied += chunk;
						off = 0;
						q = q->next;
					}
					buf[copied] = '\0';

					rp2040_socket_events->data(e->client_handle, (char *)buf, copied);

					/* Consume `copied` bytes from the chain and re-enable RX */
					e->rx_off += (uint32_t)copied;
					struct pbuf *head = e->rx_chain;
					uint32_t consumed = 0;
					while (head && e->rx_off >= head->len)
					{
						e->rx_off -= head->len;
						consumed += head->len;
						struct pbuf *next = head->next;
						head->next = NULL;
						pbuf_free(head);
						head = next;
					}
					e->rx_chain = head;
					if (!e->rx_chain)
						e->rx_off = 0;
					tcp_recved(e->pcb, (u16_t)copied);
					(void)consumed;
				}

				e->data_pending = false;
			}
		}
	}

	static int rp2040_socket_device_init(const socket_device_events_t *events)
	{
		if (!events)
		{
			return -1;
		}
		rp2040_socket_events = events;
		memset(g_fds, 0, sizeof(g_fds));
		for (int i = 0; i < BSD_MAX_FDS; i++)
		{
			g_fds[i].srv_fd = -1;
			g_fds[i].srv_handle = SOCKET_INVALID_HANDLE;
			g_fds[i].client_handle = SOCKET_INVALID_HANDLE;
		}
		return 0;
	}

	static socket_handle_t rp2040_socket_listen(uint32_t ip_listen, uint16_t port, int domain, int type, int protocol, uint8_t backlog)
	{
		(void)ip_listen;
		(void)protocol;
		if (domain != AF_INET || type != SOCK_STREAM)
		{
			return SOCKET_INVALID_HANDLE;
		}

		int fd = fd_alloc();
		if (fd < 0)
		{
			return SOCKET_INVALID_HANDLE;
		}
		g_fds[fd].kind = FD_LISTENER;

		struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
		if (!pcb)
		{
			fd_free(fd);
			return SOCKET_INVALID_HANDLE;
		}
		tcp_setprio(pcb, TCP_PRIO_NORMAL);
		pcb->so_options |= SOF_REUSEADDR;
		if (tcp_bind(pcb, IP_ANY_TYPE, port) != ERR_OK)
		{
			tcp_close(pcb);
			fd_free(fd);
			return SOCKET_INVALID_HANDLE;
		}

		struct tcp_pcb *ln = tcp_listen_with_backlog(pcb, backlog > 0 ? backlog : 1);
		if (!ln)
		{
			tcp_close(pcb);
			fd_free(fd);
			return SOCKET_INVALID_HANDLE;
		}
		g_fds[fd].listen_pcb = ln;
		tcp_arg(ln, (void *)(intptr_t)fd);
		tcp_accept(ln, accept_cb);

		/* 64-bit host safety not needed on RP2040, but keep handles pointer-width */
		return (socket_handle_t)fd;
	}

	static int rp2040_socket_send(socket_handle_t client, const void *data, size_t len, int flags)
	{
		(void)flags;
		if (client == SOCKET_INVALID_HANDLE)
		{
			return SOCKET_DEVICE_INVALID;
		}

		int fd = (int)client;
		if (fd < 0 || fd >= BSD_MAX_FDS || !data || len == 0)
		{
			return SOCKET_DEVICE_INVALID;
		}

		fd_entry_t *e = &g_fds[fd];
		if (!e->in_use || e->kind != FD_CLIENT || !e->pcb)
		{
			return SOCKET_DEVICE_INVALID;
		}

		u16_t space = tcp_sndbuf(e->pcb);
		if (space == 0)
		{
			/* Temporary TX backpressure: never wait, never spin */
			e->write_blocked = true;
			return SOCKET_DEVICE_WOULD_BLOCK;
		}

		size_t to_send = len;
		if (to_send > space)
		{
			to_send = space;
		}

		err_t err = tcp_write(e->pcb, data, (u16_t)to_send, TCP_WRITE_FLAG_COPY);
		if (err != ERR_OK)
		{
			if (err == ERR_MEM)
			{
				e->write_blocked = true;
				return SOCKET_DEVICE_WOULD_BLOCK;
			}
			return SOCKET_DEVICE_ERROR;
		}

		tcp_output(e->pcb);
		return (int)to_send;
	}

	static int rp2040_socket_close(socket_handle_t handle)
	{
		if (handle == SOCKET_INVALID_HANDLE)
		{
			return SOCKET_DEVICE_INVALID;
		}

		int fd = (int)handle;
		if (fd < 0 || fd >= BSD_MAX_FDS || !g_fds[fd].in_use)
		{
			return SOCKET_DEVICE_INVALID;
		}

		fd_free(fd);
		return 0;
	}

	socket_device_t wifi_socket =
	{
		.init = rp2040_socket_device_init,
		.listen = rp2040_socket_listen,
		.send = rp2040_socket_send,
		.close = rp2040_socket_close,
		.service = rp2040_socket_service
	};

#ifdef __cplusplus
}
#endif
#endif
