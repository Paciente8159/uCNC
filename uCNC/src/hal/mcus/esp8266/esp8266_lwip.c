/*
	Name: esp8266_lwip.c
	Description: Glue for LWIP for ESP8266.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 20-07-2026

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

#define MAX_BSD_SOCKETS (MAX_SOCKETS * SOCKET_MAX_CLIENTS)

typedef enum {
    SOCK_UNUSED = 0,
    SOCK_INIT,
    SOCK_BOUND,
    SOCK_LISTEN,
    SOCK_CONNECTED,
    SOCK_CLOSING
} sock_state_t;

typedef struct {
    sock_state_t state;
    struct tcp_pcb *pcb;
    struct tcp_pcb *pending;
    struct pbuf *rx_buf;
    u16_t rx_off;
    u32_t rx_len;
    bool rx_eof;
} bsd_sock_t;

static bsd_sock_t socks[MAX_BSD_SOCKETS];

static int alloc_sock(void)
{
    for (int i = 0; i < MAX_BSD_SOCKETS; i++)
        if (socks[i].state == SOCK_UNUSED)
            return i;
    return -1;
}

static void free_rx_chain(bsd_sock_t *s)
{
    if (s->rx_buf) {
        pbuf_free(s->rx_buf);
        s->rx_buf = NULL;
    }
    s->rx_len = 0;
    s->rx_off = 0;
}

static void err_cb(void *arg, err_t err)
{
    int idx = (int)(intptr_t)arg;
    bsd_sock_t *s = &socks[idx];

    s->pcb = NULL;
    s->rx_eof = true;
    s->state = SOCK_CLOSING;

    free_rx_chain(s);
}

static err_t sent_cb(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    /* Nonblocking: we do not wait for tx_acked */
    return ERR_OK;
}

static err_t recv_cb(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    int idx = (int)(intptr_t)arg;
    bsd_sock_t *s = &socks[idx];

    if (err != ERR_OK) {
        if (p) pbuf_free(p);
        s->rx_eof = true;
        s->pcb = NULL;
        s->state = SOCK_CLOSING;
        return err;
    }

    if (!p) {
        s->rx_eof = true;
        return ERR_OK;
    }

    if (s->rx_len + p->tot_len > SOCKET_MAX_DATA_SIZE) {
        pbuf_free(p);
        return ERR_MEM;
    }

    if (s->rx_buf)
        pbuf_cat(s->rx_buf, p);
    else {
        s->rx_buf = p;
        s->rx_off = 0;
    }

    s->rx_len += p->tot_len;
    return ERR_OK;
}

static err_t accept_cb(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    int idx = (int)(intptr_t)arg;
    bsd_sock_t *s = &socks[idx];

    if (s->pending) {
        tcp_abort(newpcb);
        return ERR_ABRT;
    }

    tcp_backlog_delayed(newpcb);
    s->pending = newpcb;
    return ERR_OK;
}

/* ---------------- BSD API ---------------- */

static int bsd_socket(int domain, int type, int protocol)
{
    if (domain != AF_INET || type != SOCK_STREAM)
        return -1;

    int idx = alloc_sock();
    if (idx < 0)
        return -1;

    bsd_sock_t *s = &socks[idx];
    memset(s, 0, sizeof(*s));

    s->pcb = tcp_new();
    if (!s->pcb) {
        s->state = SOCK_UNUSED;
        return -1;
    }

    s->state = SOCK_INIT;

    tcp_arg(s->pcb, (void *)(intptr_t)idx);
    tcp_err(s->pcb, err_cb);
    tcp_recv(s->pcb, recv_cb);
    tcp_sent(s->pcb, sent_cb);

    /* TCP_NODELAY + KEEPALIVE */
    s->pcb->so_options |= SOF_KEEPALIVE;
    tcp_nagle_disable(s->pcb);

    return idx;
}

static int bsd_bind(int sockfd, const struct bsd_sockaddr_in *addr, int addrlen)
{
    if (sockfd < 0 || sockfd >= MAX_BSD_SOCKETS)
        return -1;

    bsd_sock_t *s = &socks[sockfd];
    if (s->state != SOCK_INIT)
        return -1;

    ip_addr_t ip;
    ip.addr = addr->sin_addr;

    if (tcp_bind(s->pcb, &ip, ntohs(addr->sin_port)) != ERR_OK)
        return -1;

    s->state = SOCK_BOUND;
    return 0;
}

static int bsd_listen(int sockfd, int backlog)
{
    if (sockfd < 0 || sockfd >= MAX_BSD_SOCKETS)
        return -1;

    bsd_sock_t *s = &socks[sockfd];
    if (s->state != SOCK_BOUND)
        return -1;

    struct tcp_pcb *lpcb = tcp_listen_with_backlog(s->pcb, backlog);
    if (!lpcb)
        return -1;

    s->pcb = lpcb;
    s->state = SOCK_LISTEN;

    tcp_arg(lpcb, (void *)(intptr_t)sockfd);
    tcp_accept(lpcb, accept_cb);
    tcp_err(lpcb, err_cb);

    return 0;
}

static int bsd_accept(int sockfd, struct bsd_sockaddr_in *addr, int *addrlen)
{
    if (sockfd < 0 || sockfd >= MAX_BSD_SOCKETS)
        return -1;

    bsd_sock_t *srv = &socks[sockfd];
    if (!srv->pending)
        return -1;

    int idx = alloc_sock();
    if (idx < 0)
        return -1;

    bsd_sock_t *cli = &socks[idx];
    memset(cli, 0, sizeof(*cli));

    cli->pcb = srv->pending;
    srv->pending = NULL;

    cli->state = SOCK_CONNECTED;

    tcp_arg(cli->pcb, (void *)(intptr_t)idx);
    tcp_err(cli->pcb, err_cb);
    tcp_recv(cli->pcb, recv_cb);
    tcp_sent(cli->pcb, sent_cb);

    if (addr && addrlen && *addrlen >= sizeof(*addr)) {
        memset(addr, 0, sizeof(*addr));
        addr->sin_family = AF_INET;
        addr->sin_port = htons(cli->pcb->remote_port);
        addr->sin_addr = cli->pcb->remote_ip.addr;
        *addrlen = sizeof(*addr);
    }

    return idx;
}

static int bsd_recv(int sockfd, void *buf, size_t len, int flags)
{
    if (sockfd < 0 || sockfd >= MAX_BSD_SOCKETS)
        return -1;

    bsd_sock_t *s = &socks[sockfd];

    if (!s->rx_buf) {
        if (s->rx_eof)
            return 0;
        errno = EWOULDBLOCK;
        return -1;
    }

    struct pbuf *p = s->rx_buf;
    uint8_t *dst = buf;
    size_t copied = 0;
    size_t to_copy = len;
    u16_t off = s->rx_off;

    while (p && to_copy > 0) {
        u16_t avail = p->len - off;
        u16_t take = (avail > to_copy) ? to_copy : avail;

        memcpy(dst + copied, ((uint8_t *)p->payload) + off, take);

        copied += take;
        to_copy -= take;
        off += take;

        if (off == p->len) {
            struct pbuf *old = p;
            p = p->next;
            old->next = NULL;
            pbuf_free(old);
            off = 0;
        }
    }

    s->rx_buf = p;
    s->rx_off = off;

    if (copied >= s->rx_len)
        s->rx_len = 0;
    else
        s->rx_len -= copied;

    if (s->pcb)
        tcp_recved(s->pcb, copied);

    return copied;
}

static int bsd_send(int sockfd, const void *buf, size_t len, int flags)
{
    if (sockfd < 0 || sockfd >= MAX_BSD_SOCKETS)
        return -1;

    bsd_sock_t *s = &socks[sockfd];
    if (!s->pcb)
        return -1;

    size_t remaining = len;
    const uint8_t *p = buf;

    while (remaining) {
        u16_t snd = tcp_sndbuf(s->pcb);
        if (snd == 0) {
            errno = EWOULDBLOCK;
            return -1;
        }

        u16_t chunk = (remaining < snd) ? remaining : snd;

        err_t e = tcp_write(s->pcb, p, chunk, TCP_WRITE_FLAG_COPY);
        if (e == ERR_MEM) {
            errno = EWOULDBLOCK;
            return -1;
        }
        if (e != ERR_OK)
            return -1;

        if (tcp_output(s->pcb) != ERR_OK)
            return -1;

        remaining -= chunk;
        p += chunk;
    }

    return len;
}

static int bsd_close(int sockfd)
{
    if (sockfd < 0 || sockfd >= MAX_BSD_SOCKETS)
        return -1;

    bsd_sock_t *s = &socks[sockfd];

    if (s->pcb) {
        tcp_arg(s->pcb, NULL);
        tcp_recv(s->pcb, NULL);
        tcp_sent(s->pcb, NULL);
        tcp_err(s->pcb, NULL);

        tcp_shutdown(s->pcb, 1, 1);
        tcp_close(s->pcb);
    }

    free_rx_chain(s);

    memset(s, 0, sizeof(*s));
    s->state = SOCK_UNUSED;

    return 0;
}

socket_device_t wifi_socket = {
    .socket = bsd_socket,
    .bind   = bsd_bind,
    .listen = bsd_listen,
    .accept = bsd_accept,
    .recv   = bsd_recv,
    .send   = bsd_send,
    .close  = bsd_close
};

#endif
#endif
