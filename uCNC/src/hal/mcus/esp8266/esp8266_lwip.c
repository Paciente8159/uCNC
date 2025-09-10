/*
	Name: esp8266_lwip.c
	Description: Glue for LWIP for ESP8266.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 22-08-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

#if (MCU == MCU_ESP8266)

#ifdef ENABLE_SOCKETS

/* bsd_socket_esp8266.c
 *
 * Minimal BSD-like socket shim over ESP8266 lwIP raw TCP.
 * Supports: AF_INET + SOCK_STREAM, bind/listen/accept, nonblocking I/O,
 *           setsockopt(SO_REUSEADDR, TCP_NODELAY), send/recv/close.
 *
 * IMPORTANT: lwIP raw TCP API must be used from the TCP/IP thread. If you call
 * these from other contexts, marshal to TCP/IP thread accordingly.
 *
 * Requires lwIP 2.x as in ESP8266 Arduino core.
 */

#include <stddef.h>
#include <string.h>
#include <stdbool.h>
// bsd_socket.c — BSD socket shim for ESP8266 Arduino (NONOS), nonblocking by default.

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#include <lwip/opt.h>
#include <lwip/sockets.h>
#include <lwip/tcp.h>
#include <lwip/ip_addr.h>
#include <lwip/inet.h>
#include <lwip/err.h>
#include <lwip/mem.h>
#include <lwip/pbuf.h>
#include "../../../modules/net/socket.h"

#define MAX_BSD_SOCKETS (MAX_SOCKETS * SOCKET_MAX_CLIENTS)

#ifndef AF_INET
#define AF_INET 2
#endif
#ifndef SOCK_STREAM
#define SOCK_STREAM 1
#endif
#ifndef SOCK_DGRAM
#define SOCK_DGRAM 2
#endif
#ifndef SOCK_RAW
#define SOCK_RAW 3
#endif

typedef enum
{
	SOCK_UNUSED,
	SOCK_LISTEN,
	SOCK_CONNECTED
} sock_state_t;

typedef struct
{
	sock_state_t state;
	struct tcp_pcb *pcb;
	struct tcp_pcb *pending; // For listener: first unclaimed pcb
	struct pbuf *rx_buf;
	u16_t rx_off; // offset into rx_buf->payload
	u32_t rx_len; // total queued unread bytes (optional)
	bool rx_eof;	// FIN received (optional for BSD-like 0 return)
} bsd_sock_t;

static bsd_sock_t socks[MAX_BSD_SOCKETS];

static int alloc_sock(void)
{
	for (int i = 0; i < MAX_BSD_SOCKETS; i++)
	{
		if (socks[i].state == SOCK_UNUSED)
			return i;
	}
	return -1;
}

// ---- lwIP callbacks ----
static err_t recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	int idx = (int)(intptr_t)arg;
	if (!p)
	{
		// peer closed (FIN). Defer close until app drains.
		socks[idx].rx_eof = true;
		return ERR_OK;
	}

	if (socks[idx].rx_buf)
	{
		// Append new data to existing unread chain
		pbuf_cat(socks[idx].rx_buf, p);
	}
	else
	{
		socks[idx].rx_buf = p;
		socks[idx].rx_off = 0;
	}
	socks[idx].rx_len += p->tot_len; // optional running count
	return ERR_OK;
}

static err_t accept_cb(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	int idx = (int)(intptr_t)arg;
	if (socks[idx].pending)
	{
		tcp_abort(newpcb); // only queue one for simplicity
		return ERR_ABRT;
	}
	tcp_backlog_delayed(newpcb); // Delay backlog acknowledgment
	socks[idx].pending = newpcb;

	return ERR_OK;
}

// ---- BSD‑like API ----
int bsd_socket(int domain, int type, int protocol)
{
	if (domain != AF_INET || type != SOCK_STREAM)
		return -1;
	int idx = alloc_sock();
	if (idx < 0)
		return -1;
	memset(&socks[idx], 0, sizeof(socks[idx]));
	socks[idx].state = SOCK_CONNECTED; // provisional until bind/listen
	socks[idx].pcb = tcp_new();
	if (!socks[idx].pcb)
	{
		socks[idx].state = SOCK_UNUSED;
		return -1;
	}
	tcp_arg(socks[idx].pcb, (void *)(intptr_t)idx);
	tcp_recv(socks[idx].pcb, recv_cb);
	return idx;
}

int bsd_bind(int sockfd, const struct bsd_sockaddr_in *addr, int addrlen)
{
	if (sockfd < 0 || sockfd >= MAX_BSD_SOCKETS)
		return -1;
	ip_addr_t ip;
	ip.addr = addr->sin_addr;
	return tcp_bind(socks[sockfd].pcb, &ip, ntohs(addr->sin_port)) == ERR_OK ? 0 : -1;
}

int bsd_listen(int sockfd, int backlog)
{
	if (sockfd < 0 || sockfd >= MAX_BSD_SOCKETS)
		return -1;
	struct tcp_pcb *lpcb = tcp_listen_with_backlog(socks[sockfd].pcb, backlog);
	if (!lpcb)
		return -1;
	socks[sockfd].pcb = lpcb;
	socks[sockfd].state = SOCK_LISTEN;
	tcp_arg(lpcb, (void *)(intptr_t)sockfd);
	tcp_accept(lpcb, accept_cb);
	return 0;
}

int bsd_accept(int sockfd, struct bsd_sockaddr_in *addr, int *addrlen)
{
	if (sockfd < 0 || sockfd >= MAX_BSD_SOCKETS)
		return -1;
	if (!socks[sockfd].pending)
		return -1; // no pending client
	int idx = alloc_sock();
	if (idx < 0)
		return -1;
	memset(&socks[idx], 0, sizeof(socks[idx]));
	socks[idx].pcb = socks[sockfd].pending;
	tcp_backlog_accepted(socks[idx].pcb); // Finalize backlog acceptance
	socks[idx].state = SOCK_CONNECTED;
	socks[idx].rx_buf = NULL;
	socks[idx].rx_off = 0;
	socks[idx].rx_len = 0;
	socks[idx].rx_eof = false;
	socks[sockfd].pending = NULL;

	tcp_arg(socks[idx].pcb, (void *)(intptr_t)idx);
	tcp_recv(socks[idx].pcb, recv_cb);
	if (addr && addrlen && *addrlen >= sizeof(*addr))
	{
		memset(addr, 0, sizeof(*addr));
		addr->sin_family = AF_INET;
		addr->sin_port = htons(socks[idx].pcb->remote_port);
		addr->sin_addr = socks[idx].pcb->remote_ip.addr;
		*addrlen = sizeof(*addr);
	}
	return idx;
}

int bsd_recv(int sockfd, void *buf, size_t len, int flags)
{
	if (sockfd < 0 || sockfd >= MAX_BSD_SOCKETS)
		return -1;

	struct pbuf *p = socks[sockfd].rx_buf;
	if (!p)
	{
		// No data buffered. If EOF previously seen, return 0 (BSD semantics).
		if (socks[sockfd].rx_eof)
			return 0;
		return -1; // EWOULDBLOCK for nonblocking minimal shim
	}

	uint8_t *dst = (uint8_t *)buf;
	size_t to_copy = len;
	size_t copied = 0;
	u16_t off = socks[sockfd].rx_off;

	while (p && to_copy > 0)
	{
		u16_t avail = p->len - off;
		u16_t take = (avail > to_copy) ? (u16_t)to_copy : avail;

		memcpy(dst + copied, ((uint8_t *)p->payload) + off, take);
		copied += take;
		to_copy -= take;
		off += take;

		if (off == p->len)
		{
			// Fully consumed this pbuf: detach and free just this node
			struct pbuf *old = p;
			p = p->next;
			old->next = NULL; // prevent freeing the rest of chain
			pbuf_free(old);
			off = 0;
		}
	}

	socks[sockfd].rx_buf = p;
	socks[sockfd].rx_off = off;
	if (socks[sockfd].rx_len >= copied)
		socks[sockfd].rx_len -= (u32_t)copied;

	if (copied > 0)
	{
		// Inform TCP we've consumed data so the window advances
		tcp_recved(socks[sockfd].pcb, (u16_t)copied);
		return (int)copied;
	}

	// No bytes copied. If FIN observed and buffer empty, return 0.
	if (!p && socks[sockfd].rx_eof)
		return 0;

	return -1; // would block
}

int bsd_send(int sockfd, const void *buf, size_t len, int flags)
{
	if (sockfd < 0 || sockfd >= MAX_BSD_SOCKETS)
		return -1;
	// err_t err = tcp_write(socks[sockfd].pcb, buf, len, TCP_WRITE_FLAG_COPY);
	// if (err != ERR_OK)
	// 	return -1;

	size_t remaining = len;
	uint8_t *p = (uint8_t *)buf;

	while (remaining > 0)
	{
		u16_t space = tcp_sndbuf(socks[sockfd].pcb);
		while (space == 0)
		{
			yield();
			space = tcp_sndbuf(socks[sockfd].pcb);
		}

		size_t to_send = remaining;
		if (to_send > space)
			to_send = space;

		err_t err = ERR_OK;
		do
		{
			yield();
			err = tcp_write(socks[sockfd].pcb, p, (u16_t)to_send, TCP_WRITE_FLAG_COPY);
		} while (err != ERR_OK);

		if (err != ERR_OK)
		{
			return -1;

		err = tcp_output(socks[sockfd].pcb);
		if (err != ERR_OK)
		{
			return -1;
		}

		p += to_send;
		remaining -= to_send;
	}
	return len;
}

int bsd_close(int sockfd)
{
	if (sockfd < 0 || sockfd >= MAX_BSD_SOCKETS)
		return -1;
	if (socks[sockfd].pcb)
	{
		tcp_arg(socks[sockfd].pcb, NULL);
		tcp_recv(socks[sockfd].pcb, NULL);
		tcp_close(socks[sockfd].pcb);
	}
	if (socks[sockfd].rx_buf)
	{
		pbuf_free(socks[sockfd].rx_buf);
		socks[sockfd].rx_buf = NULL;
	}
	memset(&socks[sockfd], 0, sizeof(socks[sockfd]));
	socks[sockfd].state = SOCK_UNUSED;
	return 0;
}

int bsd_fcntl(int fd, int cmd, long arg)
{
	return 0;
}

#endif
#endif