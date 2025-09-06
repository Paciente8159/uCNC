/*
	Name: bsd_socket.h
	Description: BSD/POSIX Socket wrapper for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 20-08-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BSD_SOCKET_H
#define BSD_SOCKET_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdint.h>

#ifndef F_SETFL 
#define F_SETFL 0x800
#endif
#ifndef O_NONBLOCK
#define O_NONBLOCK 0x800
#endif
#ifndef AF_INET
#define AF_INET 2
#endif
#ifndef SOCK_STREAM
#define SOCK_STREAM     1
#endif
#ifndef SOCK_DGRAM
#define SOCK_DGRAM      2
#endif
#ifndef SOCK_RAW
#define SOCK_RAW        3
#endif

struct bsd_sockaddr_in
{
	uint16_t sin_family;
	uint16_t sin_port;
	uint32_t sin_addr;
	unsigned char sin_zero[8];
};

static inline uint16_t bsd_htons(uint16_t x) {
    return (uint16_t)(((x & 0x00ffu) << 8) | ((x & 0xff00u) >> 8));
}
static inline uint32_t bsd_htonl(uint32_t x) {
    return ((x & 0x000000ffUL) << 24) |
           ((x & 0x0000ff00UL) << 8)  |
           ((x & 0x00ff0000UL) >> 8)  |
           ((x & 0xff000000UL) >> 24);
}

/* Socket API prototypes — These must be implemented either in the MCU of for a specific module driver to provide the TCP/IP stack interface */
int bsd_socket(int domain, int type, int protocol);
int bsd_bind(int sockfd, const struct bsd_sockaddr_in *addr, int addrlen);
int bsd_listen(int sockfd, int backlog);
int bsd_accept(int sockfd, struct bsd_sockaddr_in *addr, int *addrlen);
// optional (can be removed)
// int bsd_setsockopt(int sockfd, int level, int optname, const void *optval, int optlen);
// int bsd_getsockopt(int sockfd, int level, int optname, void *optval, int *optlen);
int bsd_fcntl(int fd, int cmd, long arg);
int bsd_recv(int sockfd, void *buf, size_t len, int flags);
int bsd_send(int sockfd, const void *buf, size_t len, int flags);
int bsd_close(int fd);

#ifdef __cplusplus
}
#endif

#endif
