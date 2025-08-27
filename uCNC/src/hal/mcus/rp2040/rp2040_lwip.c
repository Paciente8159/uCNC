
/*
	Name: rp2040_lwip.c
	Description: Implements the µCNC BSD Socket for ESP32.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 25-08-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
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

#ifndef BSD_MAX_FDS
#define BSD_MAX_FDS 8
#endif

	typedef enum
	{
		FD_EMPTY = 0,
		FD_LISTENER,
		FD_CLIENT
	} fd_kind_t;

#include "lwip/pbuf.h" // add this with your other lwIP includes

	typedef struct
	{
		fd_kind_t kind;
		bool in_use;
		bool nonblock;
		bool ready;
		struct tcp_pcb *pcb;
		struct tcp_pcb *listen_pcb;
		uint16_t port;

		// RX
		struct pbuf *rx_chain;
		uint32_t rx_off;
		bool rx_closed;

		// TX
		const uint8_t *tx_buf; // pointer into the unsent buffer
		size_t tx_len;				 // bytes left to send
	} fd_entry_t;

	static fd_entry_t g_fds[BSD_MAX_FDS];

	static int fd_alloc(void)
	{
		for (int i = 0; i < BSD_MAX_FDS; i++)
		{
			if (!g_fds[i].in_use)
			{
				memset(&g_fds[i], 0, sizeof(g_fds[i]));
				g_fds[i].in_use = true;
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
			tcp_close(e->listen_pcb);
			e->listen_pcb = NULL;
		}
		if (e->pcb)
		{
			tcp_arg(e->pcb, NULL);
			tcp_recv(e->pcb, NULL);
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
	}

	static err_t recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err); // fwd decl
	static err_t sent_cb(void *arg, struct tcp_pcb *tpcb, u16_t len);

	static err_t accept_cb(void *arg, struct tcp_pcb *newpcb, err_t err)
	{
		(void)err;
		int srv_fd = (intptr_t)arg;

		int cfd = fd_alloc();
		if (cfd < 0)
		{
			tcp_abort(newpcb);
			return ERR_ABRT;
		}
		g_fds[cfd].ready = true;
		g_fds[cfd].kind = FD_CLIENT;
		g_fds[cfd].pcb = newpcb;
		g_fds[cfd].nonblock = g_fds[srv_fd].nonblock;
		g_fds[cfd].rx_chain = NULL;
		g_fds[cfd].rx_off = 0;
		g_fds[cfd].rx_closed = false;

		tcp_arg(newpcb, (void *)(intptr_t)cfd);
		tcp_recv(newpcb, recv_cb); // <- receive path
		tcp_sent(newpcb, sent_cb);

		return ERR_OK;
	}

	int bsd_socket(int domain, int type, int protocol)
	{
		(void)protocol;
		if (domain != AF_INET || type != SOCK_STREAM)
			return -1;
		int fd = fd_alloc();
		if (fd < 0)
			return -1;
		g_fds[fd].kind = FD_LISTENER;
		return fd;
	}

	int bsd_bind(int sockfd, const struct bsd_sockaddr_in *addr, int addrlen)
	{
		(void)addrlen;
		if (sockfd < 0 || sockfd >= BSD_MAX_FDS)
			return -1;
		g_fds[sockfd].port = (uint16_t)((addr->sin_port << 8) | (addr->sin_port >> 8));
		return 0;
	}

	int bsd_listen(int sockfd, int backlog)
	{
		fd_entry_t *e = &g_fds[sockfd];
		if (e->kind != FD_LISTENER)
			return -1;
		struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
		if (!pcb)
			return -1;
		tcp_setprio(pcb, TCP_PRIO_NORMAL);
		pcb->so_options |= SOF_REUSEADDR;
		if (tcp_bind(pcb, IP_ANY_TYPE, e->port) != ERR_OK)
		{
			tcp_close(pcb);
			return -1;
		}
		e->listen_pcb = tcp_listen_with_backlog(pcb, backlog > 0 ? backlog : 1);
		tcp_arg(e->listen_pcb, (void *)(intptr_t)sockfd);
		tcp_accept(e->listen_pcb, accept_cb);
		return 0;
	}

	int bsd_accept(int sockfd, struct bsd_sockaddr_in *addr, int *addrlen)
	{
		(void)addr;
		(void)addrlen;
		for (int i = 0; i < BSD_MAX_FDS; i++)
		{
			if (g_fds[i].in_use && g_fds[i].kind == FD_CLIENT && g_fds[i].ready)
			{
				struct tcp_pcb *pcb = g_fds[i].pcb;
				if (pcb && pcb->state == ESTABLISHED)
				{
					g_fds[i].ready = false;
					return i;
				}
			}
		}
		return -1; // no client ready
	}

	int bsd_fcntl(int fd, int cmd, long arg)
	{
		if (fd < 0 || fd >= BSD_MAX_FDS)
			return -1;
		if (cmd == F_SETFL)
		{
			g_fds[fd].nonblock = (arg & O_NONBLOCK);
			return 0;
		}
		return -1;
	}

	static err_t sent_cb(void *arg, struct tcp_pcb *tpcb, u16_t len)
	{
		(void)tpcb;
		int fd = (int)(intptr_t)arg;
		if (fd < 0 || fd >= BSD_MAX_FDS)
			return ERR_OK;

		fd_entry_t *e = &g_fds[fd];
		if (!e->tx_buf || e->tx_len == 0)
			return ERR_OK;

		u16_t space = tcp_sndbuf(e->pcb);
		if (space == 0)
			return ERR_OK;

		size_t to_send = e->tx_len;
		if (to_send > space)
			to_send = space;

		if (tcp_write(e->pcb, e->tx_buf, (u16_t)to_send, TCP_WRITE_FLAG_COPY) == ERR_OK)
		{
			e->tx_buf += to_send;
			e->tx_len -= to_send;
			tcp_output(e->pcb);
			if (e->tx_len == 0)
			{
				e->tx_buf = NULL; // done
			}
		}
		return ERR_OK;
	}

	int bsd_send(int sockfd, const void *buf, size_t len, int flags)
	{
		(void)flags;
		if (sockfd < 0 || sockfd >= BSD_MAX_FDS || !buf || len == 0)
			return -1;

		fd_entry_t *e = &g_fds[sockfd];
		if (!e->in_use || e->kind != FD_CLIENT || !e->pcb)
			return -1;

		if (e->tx_len != 0)
		{
			// still sending previous buffer
			return -1; // EWOULDBLOCK
		}

		u16_t space = tcp_sndbuf(e->pcb);
		if (space == 0)
		{
			// can't send anything right now
			e->tx_buf = buf;
			e->tx_len = len;
			return -1; // no immediate send, queued for sent_cb
		}

		size_t to_send = len;
		if (to_send > space)
		{
			to_send = space;
			// queue remainder
			e->tx_buf = ((const uint8_t *)buf) + to_send;
			e->tx_len = len - to_send;
		}

		if (tcp_write(e->pcb, buf, (u16_t)to_send, TCP_WRITE_FLAG_COPY) != ERR_OK)
			return -1;

		tcp_output(e->pcb);

		return (int)to_send;
	}

	int bsd_close(int fd)
	{
		fd_free(fd);
		return 0;
	}

	int bsd_recv(int sockfd, void *buf, size_t len, int flags)
	{
		(void)flags;
		if (sockfd < 0 || sockfd >= BSD_MAX_FDS || !buf || len == 0)
			return -1;

		fd_entry_t *e = &g_fds[sockfd];
		if (!e->in_use || e->kind != FD_CLIENT || !e->pcb)
			return -1;

		if (!e->rx_chain)
		{
			// no buffered data
			if (e->rx_closed)
			{
				// remote has FIN'd and nothing is left to read
				return 0;
			}
			return -1; // EWOULDBLOCK semantics for nonblocking users
		}

		// copy out up to len bytes from rx_chain starting at rx_off
		uint8_t *dst = (uint8_t *)buf;
		size_t remaining = len;
		size_t copied = 0;

		struct pbuf *q = e->rx_chain;
		uint32_t off = e->rx_off;

		while (q && remaining > 0)
		{
			if (off >= q->len)
			{
				off -= q->len;
				q = q->next;
				continue;
			}
			size_t chunk = q->len - (size_t)off;
			if (chunk > remaining)
				chunk = remaining;

			memcpy(dst + copied, ((uint8_t *)q->payload) + off, chunk);

			copied += chunk;
			remaining -= chunk;
			off = 0;
			q = q->next;
		}

		if (copied == 0)
		{
			// no data could be copied (shouldn't happen if rx_chain is non-NULL),
			// treat as temporarily unavailable
			return -1;
		}

		// advance rx_off by copied bytes and free fully-consumed pbufs
		e->rx_off += (uint32_t)copied;

		size_t to_free = 0;
		struct pbuf *head = e->rx_chain;
		while (head && e->rx_off >= head->len)
		{
			e->rx_off -= head->len;
			to_free += head->len;
			struct pbuf *next = head->next;
			head->next = NULL;
			pbuf_free(head);
			head = next;
		}
		e->rx_chain = head;
		if (!e->rx_chain)
		{
			e->rx_off = 0;
		}

		// tell lwIP we've consumed these bytes so the window can open
		tcp_recved(e->pcb, (u16_t)copied);

		return (int)copied;
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
			// leave e->rx_chain as-is; app may drain then close
			return err;
		}

		if (p == NULL)
		{
			// remote closed connection (FIN). Drain remaining rx_chain via bsd_recv, then report 0.
			e->rx_closed = true;
			return ERR_OK;
		}

		// Append pbuf(s) to our queued chain. We keep ownership until bsd_recv consumes.
		if (!e->rx_chain)
		{
			e->rx_chain = p;
			e->rx_off = 0;
		}
		else
		{
			// Concatenate without changing refs; p will be part of the chain and freed later.
			pbuf_cat(e->rx_chain, p);
		}

		return ERR_OK;
	}

#ifdef __cplusplus
}
#endif
#endif
