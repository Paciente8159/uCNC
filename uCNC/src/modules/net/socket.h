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
#include "utils/bsd_socket.h"
#include "../../module.h"

#ifndef MAX_SOCKETS
#define MAX_SOCKETS 4
#endif
#ifndef SOCKET_MAX_CLIENTS
#define SOCKET_MAX_CLIENTS 2
#endif
#ifndef SOCKET_MAX_DATA_SIZE
#define SOCKET_MAX_DATA_SIZE 32
#endif

#ifndef IP_ANY
#define IP_ANY 0
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

#ifndef SOCKET_IDLE_TIMEOUT
#define SOCKET_IDLE_TIMEOUT 60
#endif

typedef void (*socket_data_delegate)(uint8_t client_idx, char* data, size_t data_len, void* protocol);
typedef void (*socket_connect_delegate)(uint8_t client_idx, void* protocol);
typedef void (*socket_idle_delegate)(uint8_t client_idx, uint32_t idle_ms, void* protocol);

typedef struct socket_if_{
	int socket_if;
	int socket_clients[SOCKET_MAX_CLIENTS];
	#ifdef ENABLE_SOCKET_TIMEOUTS
	uint32_t client_activity[SOCKET_MAX_CLIENTS];
	#endif
	socket_data_delegate client_ondata_cb;
	socket_idle_delegate client_onidle_cb;
	socket_connect_delegate client_onconnected_cb;
	socket_connect_delegate client_ondisconnected_cb;
	void* protocol;
} socket_if_t;

// creates a new socket connection and starts to listen for new clients (non blocking). Returns -1 if it fails. Otherwise returns the socket interface number (from bsd_socket)
socket_if_t* socket_start_listen(uint32_t ip_listen, uint16_t port, int domain, int type, int protocol);
void socket_stop_listening(socket_if_t* socket);
void socket_add_ondata_handler(socket_if_t* socket, socket_data_delegate callback);
void socket_add_onidle_handler(socket_if_t* socket, socket_idle_delegate callback);
void socket_add_onconnected_handler(socket_if_t* socket, socket_connect_delegate callback);
void socket_add_ondisconnected_handler(socket_if_t* socket, socket_connect_delegate callback);
// sends data to a specific socket interface to a client
int socket_send(socket_if_t *socket, uint8_t client_idx, char* data, size_t data_len, int flags);
// sends data to a specific socket interface to all clients
int socket_broadcast(socket_if_t *socket, char* data, size_t data_len, int flags);
// closes a connection to a client
void socket_free(socket_if_t* socket, uint8_t client_idx);
// runs the loop that handles new client accpts and handles each socket/client data handling (non blocking)
void socket_server_dotasks(void);
// returns the number of active clients in a socket. if socket is NULL returns all connected clients in all sockets
int socket_server_hasclients(socket_if_t* socket);
// initializes sockets server
DECL_MODULE(socket_server);

#ifdef __cplusplus
}
#endif

#endif
