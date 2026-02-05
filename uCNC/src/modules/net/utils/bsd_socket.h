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
#define SOCK_STREAM 1
#endif
#ifndef SOCK_DGRAM
#define SOCK_DGRAM 2
#endif
#ifndef SOCK_RAW
#define SOCK_RAW 3
#endif

	struct __attribute__((__packed__)) bsd_sockaddr_in
	{
		uint16_t sin_family;
		uint16_t sin_port;
		uint32_t sin_addr;
		unsigned char sin_zero[8];
	};

/**
 * Default is little-endian
 * If not the macros should be replaced by a simply passing the value
 */
#ifndef bsd_htons
#define bsd_htons(x)((uint16_t)(((x & 0x00ffu) << 8) | ((x & 0xff00u) >> 8)))
#endif

#ifndef bsd_htonl
#define bsd_htonl(x)((uint32_t)(((x & 0x000000ffUL) << 24) | ((x & 0x0000ff00UL) << 8) | ((x & 0x00ff0000UL) >> 8) | ((x & 0xff000000UL) >> 24)))
#endif

	/* Socket API prototypes — These must be implemented either in the MCU of for a specific module driver to provide the TCP/IP stack interface */
	typedef struct socket_device_
	{
		int (*socket)(int domain, int type, int protocol);
		int (*bind)(int sockfd, const struct bsd_sockaddr_in *addr, int addrlen);
		int (*listen)(int sockfd, int backlog);
		int (*accept)(int sockfd, struct bsd_sockaddr_in *addr, int *addrlen);
		// optional (can be removed)
		// int (*setsockopt)(int sockfd, int level, int optname, const void *optval, int optlen);
		// int (*getsockopt)(int sockfd, int level, int optname, void *optval, int *optlen);
		int (*fcntl)(int fd, int cmd, long arg);
		int (*recv)(int sockfd, void *buf, size_t len, int flags);
		int (*send)(int sockfd, const void *buf, size_t len, int flags);
		int (*close)(int fd);
	} socket_device_t;

#ifdef __cplusplus
}
#endif

#endif
