/*
	Name: esp32_lwip.c
	Description: Implements the µCNC BSD Socket for ESP32.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 24-08-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

#if (MCU == MCU_ESP32)

#ifdef ENABLE_SOCKETS

#include <string.h>
#include <errno.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <fcntl.h>

// Helpers to convert between your BSD typedefs and lwIP
static inline void to_sockaddr(const struct bsd_sockaddr_in* b, struct sockaddr_in* a) {
    memset(a, 0, sizeof(*a));
    a->sin_family = (sa_family_t)b->sin_family;
    a->sin_port   = (in_port_t)b->sin_port;        // network order
    a->sin_addr.s_addr = (in_addr_t)b->sin_addr;   // network order
}

static inline void from_sockaddr(const struct sockaddr_in* a, struct bsd_sockaddr_in* b) {
    memset(b, 0, sizeof(*b));
    b->sin_family = (uint16_t)a->sin_family;
    b->sin_port   = (uint16_t)a->sin_port;        // network order
    b->sin_addr   = (uint32_t)a->sin_addr.s_addr; // network order
}

// Create socket
int bsd_socket(int domain, int type, int protocol) {
    return socket(domain, type, protocol);
}

// Bind to address (enables SO_REUSEADDR like WiFiServer)
int bsd_bind(int sockfd, const struct bsd_sockaddr_in *addr, int addrlen) {
    (void)addrlen;
    struct sockaddr_in sa;
    to_sockaddr(addr, &sa);

    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    return bind(sockfd, (struct sockaddr*)&sa, (socklen_t)sizeof(sa));
}

// Accept one client nonblocking; accepted socket is set nonblocking
int bsd_accept(int sockfd, struct bsd_sockaddr_in *addr, int *addrlen) {
    struct sockaddr_in sa;
    socklen_t sl = sizeof(sa);
    memset(&sa, 0, sizeof(sa));

    // Attempt accept (nonblocking listener => returns -1 with EWOULDBLOCK/EAGAIN if no client)
    int c;
#ifdef ESP_IDF_VERSION_MAJOR
    c = lwip_accept(sockfd, (struct sockaddr*)&sa, &sl);
#else
    c = lwip_accept_r(sockfd, (struct sockaddr*)&sa, &sl);
#endif

    if (c < 0) {
        return -1;
    }

    // Keepalive like WiFiServer
    int one = 1;
    setsockopt(c, SOL_SOCKET, SO_KEEPALIVE, (const char*)&one, sizeof(one));

    // Nonblocking accepted socket
    fcntl(c, F_SETFL, O_NONBLOCK);

    if (addr) {
        from_sockaddr(&sa, addr);
    }
    if (addrlen) {
        *addrlen = (int)sizeof(struct bsd_sockaddr_in);
    }
    return c;
}

// setsockopt pass-through (with logging)
int bsd_setsockopt(int sockfd, int level, int optname, const void *optval, int optlen) {
    return setsockopt(sockfd, level, optname, optval, (socklen_t)optlen);
}

// Nonblocking recv/send (with logging on activity/errors)
int bsd_recv(int sockfd, void *buf, size_t len, int flags) {
    return recv(sockfd, buf, len, flags);
}

int bsd_send(int sockfd, const void *buf, size_t len, int flags) {
    return send(sockfd, buf, len, flags);
}

// Close
int bsd_close(int fd) {
#ifdef ESP_IDF_VERSION_MAJOR
    return lwip_close(fd);
#else
    return lwip_close_r(fd);
#endif
}

// Listen and set listening socket to nonblocking, WiFiServer-style
int bsd_listen(int sockfd, int backlog) {
    if (listen(sockfd, backlog) < 0) {
        return -1;
    }

    // Matches WiFiServer: ignore ENOSYS as non-fatal
    int flags = O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, flags) < 0) {
        if (errno != ENOSYS) {
            return -1; // Only fatal on other errors
        }
    }

    return 0;
}

// fcntl wrapper; now takes int arg like WiFiServer does
int bsd_fcntl(int fd, int cmd, long arg) {
    int r = fcntl(fd, cmd, (int)arg);
    if (r < 0) {
        if (errno == ENOSYS) {
            return 0; // non-fatal
        }
    }
    return r;
}

#endif
#endif
