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

    /* lwIP PCB references (owned by this backend) */
    struct tcp_pcb *pcb;
    struct tcp_pcb *pending;  /* listener: accepted but not yet delivered */

    /* RX: pbuf chain owned by this backend until the socket core consumes */
    struct pbuf *rx_buf;
    u16_t rx_off;
    u32_t rx_len;
    bool rx_eof;

    /* Event flags set from the lwIP tcpip-thread callbacks and consumed by
       esp8266_socket_service() in the µCNC cooperative context */
    bool event_pending;
    bool event_peer_closed;
    bool event_error;
    bool write_blocked;

    /* µCNC socket handles (back-references) */
    socket_handle_t srv_handle;
    socket_handle_t client_handle;
} bsd_sock_t;

static bsd_sock_t socks[MAX_BSD_SOCKETS];
static const socket_device_events_t *esp8266_socket_events;

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
    if (idx < 0 || idx >= MAX_BSD_SOCKETS)
        return;
    bsd_sock_t *s = &socks[idx];

    s->pcb = NULL;
    s->rx_eof = true;
    s->state = SOCK_CLOSING;
    s->event_error = true;
    s->event_pending = true;

    free_rx_chain(s);
}

static err_t sent_cb(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    (void)pcb;
    (void)len;
    if (!arg) {
        return ERR_OK;
    }
    int idx = (int)(intptr_t)arg;
    if (idx < 0 || idx >= MAX_BSD_SOCKETS)
        return ERR_OK;

    bsd_sock_t *s = &socks[idx];

    /* A previous send returned WOULD_BLOCK: report TX progress */
    if (s->write_blocked) {
        s->write_blocked = false;
        if (esp8266_socket_events && esp8266_socket_events->writable) {
            esp8266_socket_events->writable(s->client_handle);
        }
    }
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
        s->event_error = true;
        s->event_pending = true;
        return err;
    }

    if (!p) {
        /* Remote closed connection (FIN) */
        s->rx_eof = true;
        s->event_peer_closed = true;
        s->event_pending = true;
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
    s->event_pending = true;
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
    s->event_pending = true;
    return ERR_OK;
}

/* ---------------- µCNC socket device API ---------------- */

static int esp8266_socket_device_init(const socket_device_events_t *events)
{
    if (!events) {
        return -1;
    }
    esp8266_socket_events = events;
    memset(socks, 0, sizeof(socks));
    for (int i = 0; i < MAX_BSD_SOCKETS; i++) {
        socks[i].srv_handle = SOCKET_INVALID_HANDLE;
        socks[i].client_handle = SOCKET_INVALID_HANDLE;
    }
    return 0;
}

static socket_handle_t esp8266_socket_listen(uint32_t ip_listen, uint16_t port, int domain, int type, int protocol, uint8_t backlog)
{
    (void)protocol;
    if (domain != AF_INET || type != SOCK_STREAM) {
        return SOCKET_INVALID_HANDLE;
    }

    int idx = alloc_sock();
    if (idx < 0) {
        return SOCKET_INVALID_HANDLE;
    }

    bsd_sock_t *s = &socks[idx];
    memset(s, 0, sizeof(*s));

    s->pcb = tcp_new();
    if (!s->pcb) {
        s->state = SOCK_UNUSED;
        return SOCKET_INVALID_HANDLE;
    }

    s->state = SOCK_BOUND;
    s->srv_handle = (socket_handle_t)idx;
    s->client_handle = SOCKET_INVALID_HANDLE;

    ip_addr_t ip;
    ip.addr = ip_listen;

    if (tcp_bind(s->pcb, &ip, port) != ERR_OK) {
        tcp_close(s->pcb);
        s->pcb = NULL;
        memset(s, 0, sizeof(*s));
        s->state = SOCK_UNUSED;
        s->srv_handle = SOCKET_INVALID_HANDLE;
        s->client_handle = SOCKET_INVALID_HANDLE;
        return SOCKET_INVALID_HANDLE;
    }

    struct tcp_pcb *lpcb = tcp_listen_with_backlog(s->pcb, backlog > 0 ? backlog : 1);
    if (!lpcb) {
        tcp_close(s->pcb);
        s->pcb = NULL;
        memset(s, 0, sizeof(*s));
        s->state = SOCK_UNUSED;
        s->srv_handle = SOCKET_INVALID_HANDLE;
        s->client_handle = SOCKET_INVALID_HANDLE;
        return SOCKET_INVALID_HANDLE;
    }

    s->pcb = lpcb;
    s->state = SOCK_LISTEN;

    tcp_arg(lpcb, (void *)(intptr_t)idx);
    tcp_accept(lpcb, accept_cb);

    return (socket_handle_t)idx;
}

static int esp8266_socket_send(socket_handle_t client, const void *data, size_t len, int flags)
{
    (void)flags;
    if (client == SOCKET_INVALID_HANDLE) {
        return SOCKET_DEVICE_INVALID;
    }

    int idx = (int)client;
    if (idx < 0 || idx >= MAX_BSD_SOCKETS) {
        return SOCKET_DEVICE_INVALID;
    }

    bsd_sock_t *s = &socks[idx];
    if (!s->pcb || s->state != SOCK_CONNECTED) {
        return SOCKET_DEVICE_INVALID;
    }

    u16_t snd = tcp_sndbuf(s->pcb);
    if (snd == 0) {
        /* Temporary TX backpressure: never wait, never spin */
        s->write_blocked = true;
        return SOCKET_DEVICE_WOULD_BLOCK;
    }

    u16_t chunk = (len < snd) ? (u16_t)len : snd;

    err_t e = tcp_write(s->pcb, data, chunk, TCP_WRITE_FLAG_COPY);
    if (e == ERR_MEM) {
        s->write_blocked = true;
        return SOCKET_DEVICE_WOULD_BLOCK;
    }
    if (e != ERR_OK) {
        return SOCKET_DEVICE_ERROR;
    }

    tcp_output(s->pcb);

    return (int)chunk;
}

static int esp8266_socket_close(socket_handle_t handle)
{
    if (handle == SOCKET_INVALID_HANDLE) {
        return SOCKET_DEVICE_INVALID;
    }

    int sockfd = (int)handle;
    if (sockfd < 0 || sockfd >= MAX_BSD_SOCKETS) {
        return SOCKET_DEVICE_INVALID;
    }

    bsd_sock_t *s = &socks[sockfd];
    if (s->state == SOCK_UNUSED) {
        return SOCKET_DEVICE_INVALID;
    }

    if (s->pcb) {
        tcp_arg(s->pcb, NULL);
        tcp_recv(s->pcb, NULL);
        tcp_sent(s->pcb, NULL);
        tcp_err(s->pcb, NULL);
        tcp_abort(s->pcb);
        s->pcb = NULL;
    }
    if (s->pending) {
        tcp_abort(s->pending);
        s->pending = NULL;
    }

    free_rx_chain(s);

    memset(s, 0, sizeof(*s));
    s->state = SOCK_UNUSED;
    s->srv_handle = SOCKET_INVALID_HANDLE;
    s->client_handle = SOCKET_INVALID_HANDLE;

    return 0;
}

static void esp8266_socket_service(void)
{
    if (!esp8266_socket_events) {
        return;
    }

    /* The lwIP TCP callbacks run in the tcpip-thread context; the event
       flags are consumed here in the µCNC cooperative context. Bounded:
       at most one event per client per service() call. */
    for (int i = 0; i < MAX_BSD_SOCKETS; i++) {
        bsd_sock_t *s = &socks[i];
        if (s->state == SOCK_UNUSED || !s->event_pending) {
            continue;
        }

        s->event_pending = false;

        /* Pending accepted connection */
        if (s->state == SOCK_LISTEN && s->pending) {
            int idx = alloc_sock();
            if (idx < 0) {
                tcp_abort(s->pending);
                s->pending = NULL;
                continue;
            }

            bsd_sock_t *cli = &socks[idx];
            memset(cli, 0, sizeof(*cli));

            cli->pcb = s->pending;
            s->pending = NULL;
            cli->state = SOCK_CONNECTED;
            cli->client_handle = (socket_handle_t)idx;
            cli->srv_handle = s->srv_handle;

            tcp_arg(cli->pcb, (void *)(intptr_t)idx);
            tcp_err(cli->pcb, err_cb);
            tcp_recv(cli->pcb, recv_cb);
            tcp_sent(cli->pcb, sent_cb);

            if (!esp8266_socket_events->connected(cli->srv_handle, cli->client_handle)) {
                /* No µCNC client slot: reject and discard */
                sock_state_t st = cli->state;
                (void)st;
                if (cli->pcb) {
                    tcp_arg(cli->pcb, NULL);
                    tcp_recv(cli->pcb, NULL);
                    tcp_sent(cli->pcb, NULL);
                    tcp_err(cli->pcb, NULL);
                    tcp_abort(cli->pcb);
                    cli->pcb = NULL;
                }
                memset(cli, 0, sizeof(*cli));
                cli->state = SOCK_UNUSED;
                cli->srv_handle = SOCKET_INVALID_HANDLE;
                cli->client_handle = SOCKET_INVALID_HANDLE;
            }
            continue;
        }

        /* Fatal transport error */
        if (s->event_error) {
            s->event_error = false;
            socket_handle_t handle = s->client_handle;
            if (s->pcb) {
                tcp_arg(s->pcb, NULL);
                tcp_recv(s->pcb, NULL);
                tcp_sent(s->pcb, NULL);
                tcp_err(s->pcb, NULL);
                tcp_abort(s->pcb);
                s->pcb = NULL;
            }
            free_rx_chain(s);
            memset(s, 0, sizeof(*s));
            s->state = SOCK_UNUSED;
            s->srv_handle = SOCKET_INVALID_HANDLE;
            s->client_handle = SOCKET_INVALID_HANDLE;
            esp8266_socket_events->disconnected(handle, SOCKET_DEVICE_ERROR);
            continue;
        }

        /* Orderly remote close */
        if (s->event_peer_closed) {
            s->event_peer_closed = false;
            if (s->rx_buf) {
                /* Still queued data: deliver it as one last data event */
                static char srv_buffer[SOCKET_MAX_DATA_SIZE + 1];
                size_t copied = 0;
                struct pbuf *p = s->rx_buf;
                u16_t off = s->rx_off;
                while (p && copied < SOCKET_MAX_DATA_SIZE) {
                    u16_t avail = p->len - off;
                    u16_t take = avail;
                    if ((size_t)take > (SOCKET_MAX_DATA_SIZE - copied))
                        take = (u16_t)(SOCKET_MAX_DATA_SIZE - copied);
                    memcpy(srv_buffer + copied, ((uint8_t *)p->payload) + off, take);
                    copied += take;
                    if (off + take >= p->len) {
                        struct pbuf *old = p;
                        p = p->next;
                        old->next = NULL;
                        pbuf_free(old);
                        off = 0;
                    } else {
                        off += take;
                    }
                }
                s->rx_buf = p;
                s->rx_off = off;
                if (copied) {
                    srv_buffer[copied] = '\0';
                    esp8266_socket_events->data(s->client_handle, srv_buffer, copied);
                    if (s->pcb)
                        tcp_recved(s->pcb, (u16_t)copied);
                }
                free_rx_chain(s);
            }

            socket_handle_t handle = s->client_handle;
            if (s->pcb) {
                tcp_arg(s->pcb, NULL);
                tcp_recv(s->pcb, NULL);
                tcp_sent(s->pcb, NULL);
                tcp_err(s->pcb, NULL);
                tcp_abort(s->pcb);
                s->pcb = NULL;
            }
            memset(s, 0, sizeof(*s));
            s->state = SOCK_UNUSED;
            s->srv_handle = SOCKET_INVALID_HANDLE;
            s->client_handle = SOCKET_INVALID_HANDLE;
            esp8266_socket_events->disconnected(handle, 0);
            continue;
        }

        /* Received payload */
        if (s->rx_buf) {
            static char srv_buffer[SOCKET_MAX_DATA_SIZE + 1];
            size_t copied = 0;
            struct pbuf *p = s->rx_buf;
            u16_t off = s->rx_off;
            while (p && copied < SOCKET_MAX_DATA_SIZE) {
                u16_t avail = p->len - off;
                u16_t take = avail;
                if ((size_t)take > (SOCKET_MAX_DATA_SIZE - copied))
                    take = (u16_t)(SOCKET_MAX_DATA_SIZE - copied);
                memcpy(srv_buffer + copied, ((uint8_t *)p->payload) + off, take);
                copied += take;
                if (off + take >= p->len) {
                    struct pbuf *old = p;
                    p = p->next;
                    old->next = NULL;
                    pbuf_free(old);
                    off = 0;
                } else {
                    off += take;
                }
            }
            s->rx_buf = p;
            s->rx_off = off;

            if (s->rx_len >= (u32_t)copied)
                s->rx_len -= (u32_t)copied;
            else
                s->rx_len = 0;

            if (copied) {
                srv_buffer[copied] = '\0';
                esp8266_socket_events->data(s->client_handle, srv_buffer, copied);
                if (s->pcb)
                    tcp_recved(s->pcb, (u16_t)copied);
            }
        }
    }
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
