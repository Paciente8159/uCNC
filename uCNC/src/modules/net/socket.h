/*
	Name: socket.h
	Description: Implements a simple Raw Socket Server based on BSD/POSIX Sockets for µCNC.

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

#ifndef SOCKET_H
#define SOCKET_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdint.h>
#include "../../module.h"

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

/* Socket API prototypes — These must be implemented either in the MCU of for a specific module driver to provide the TCP/IP stack interface*/
int bsd_socket(int domain, int type, int protocol);
int bsd_bind(int sockfd, const struct bsd_sockaddr_in *addr, int addrlen);
int bsd_listen(int sockfd, int backlog);
int bsd_accept(int sockfd, struct bsd_sockaddr_in *addr, int *addrlen);
int bsd_setsockopt(int sockfd, int level, int optname, const void *optval, int optlen);
int bsd_fcntl(int fd, int cmd, long arg);
int bsd_recv(int sockfd, void *buf, size_t len, int flags);
int bsd_send(int sockfd, const void *buf, size_t len, int flags);
int bsd_close(int fd);

#ifndef MAX_SOCKETS
#define MAX_SOCKETS 4
#endif
#ifndef SOCKET_MAX_CLIENTS
#define SOCKET_MAX_CLIENTS 5
#endif
#ifndef SOCKET_MAX_DATA_SIZE
#define SOCKET_MAX_DATA_SIZE 256
#endif

#ifndef IP_ANY
#define IP_ANY 0
#endif

typedef void (*socket_data_delegate)(int client_index, void* data, size_t data_len);
typedef void (*socket_connect_delegate)(int client_index);

typedef struct socket_if_{
	int socket_if;
	int socket_clients[SOCKET_MAX_CLIENTS];
	socket_data_delegate client_ondata_cb;
	socket_connect_delegate client_onconnected_cb;
	socket_connect_delegate client_ondisconnected_cb;
} socket_if_t;

// creates a new socket connection and starts to listen for new clients (non blocking). Returns -1 if it fails. Otherwise returns the socket interface number (from bsd_socket)
socket_if_t* socket_start(uint32_t ip_listen, uint16_t port, int domain, int type, int protocol);
void socket_add_ondata_handler(socket_if_t* socket, socket_data_delegate callback);
void socket_add_onconnected_handler(socket_if_t* socket, socket_connect_delegate callback);
void socket_add_ondisconnected_handler(socket_if_t* socket, socket_connect_delegate callback);
// sends data to a specific socket interface to a client
int socket_send(socket_if_t *socket, int client, char* data, size_t data_len, int flags);
// sends data to a specific socket interface to all clients
int socket_broadcast(socket_if_t *socket, char* data, size_t data_len, int flags);
// runs the loop that handles new client accpts and handles each socket/client data handling (non blocking)
void socker_server_run(socket_if_t* socket);
// initializes sockets server
DECL_MODULE(socket_server);

#ifdef __cplusplus
}
#endif

#endif
