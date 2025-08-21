/*
	Name: bsd_socket.c
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

#include "../cnc.h"
#include "bsd_socket.h"
#include <string.h>

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

/* Global socket interfaces */
static socket_if_t raw_sockets[MAX_SOCKETS];

static int find_free_socket_if(void)
{
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (raw_sockets[i].socket_if == -1)
			return i;
	}
	return -1;
}

void socket_server_int(void)
{
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		raw_sockets[i].socket_if = -1;
		raw_sockets[i].current_client = -1;
		memset(raw_sockets[i].socket_clients, -1, sizeof(raw_sockets[i].socket_clients));
		raw_sockets[i].received_data_handler = NULL;
	}
}

socket_if_t *socket_start(int domain, int type, int protocol, uint32_t ip_listen, uint16_t port, socket_received_data_delegate *handler)
{
	int idx = find_free_socket_if();
	if (idx < 0)
		return NULL;

	int s = bsd_socket(domain, type, protocol);
	if (s < 0)
		return NULL;

	struct bsd_sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = (uint16_t)domain;
	addr.sin_port = bsd_htons(port); // default port (can be changed by user)
	addr.sin_addr = bsd_htonl(ip_listen);	 // any address

	if (bsd_bind(s, &addr, sizeof(addr)) < 0)
	{
		bsd_close(s);
		return NULL;
	}

	if (type == 1 /* SOCK_STREAM */)
	{
		if (bsd_listen(s, SOCKET_MAX_CLIENTS) < 0)
		{
			bsd_close(s);
			return NULL;
		}
	}

	raw_sockets[idx].socket_if = s;
	raw_sockets[idx].current_client = -1;
	memset(raw_sockets[idx].socket_clients, -1, sizeof(raw_sockets[idx].socket_clients));
	raw_sockets[idx].received_data_handler = handler;
	return &raw_sockets[idx];
}

int socket_send(socket_if_t *socket, char *data, size_t data_len, int flags)
{
	if (!socket)
		return -1;
	int client = socket->current_client;
	if (client < 0)
		return -1;
	return bsd_send(client, data, data_len, flags);
}

int socket_broadcast(socket_if_t *socket, char *data, size_t data_len, int flags)
{
	if (!socket)
		return -1;
	int sent = 0;
	for (int i = 0; i < SOCKET_MAX_CLIENTS; i++)
	{
		if (socket->socket_clients[i] >= 0)
		{
			if (bsd_send(socket->socket_clients[i], data, data_len, flags) >= 0)
			{
				sent++;
			}
		}
	}
	return sent;
}

/* Helper: add new client */
static void add_client(socket_if_t *iface, int client_fd)
{
	for (int i = 0; i < SOCKET_MAX_CLIENTS; i++)
	{
		if (iface->socket_clients[i] < 0)
		{
			iface->socket_clients[i] = client_fd;
			iface->current_client = client_fd;
			return;
		}
	}
	/* No space — close the connection */
	bsd_close(client_fd);
}

/* Helper: remove disconnected client */
static void remove_client(socket_if_t *iface, int idx)
{
	if (idx >= 0 && idx < SOCKET_MAX_CLIENTS)
	{
		if (iface->socket_clients[idx] >= 0)
		{
			bsd_close(iface->socket_clients[idx]);
			iface->socket_clients[idx] = -1;
		}
	}
}

void socker_server_run(void)
{
	char buffer[SOCKET_MAX_DATA_SIZE];

	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		socket_if_t *iface = &raw_sockets[i];
		if (iface->socket_if >= 0)
		{
			/* Accept new clients if TCP */
			int client_fd;
			struct bsd_sockaddr_in cli_addr;
			int cli_len = sizeof(cli_addr);

			client_fd = bsd_accept(iface->socket_if, &cli_addr, &cli_len);
			if (client_fd >= 0)
			{
				add_client(iface, client_fd);
			}

			/* Poll each client for data (non-blocking) */
			for (int c = 0; c < SOCKET_MAX_CLIENTS; c++)
			{
				int fd = iface->socket_clients[c];
				if (fd >= 0)
				{
					int len = bsd_recv(fd, buffer, sizeof(buffer), 0);
					if (len > 0)
					{
						if (iface->received_data_handler)
						{
							iface->received_data_handler(buffer, (size_t)len);
						}
					}
					else if (len == 0)
					{
						remove_client(iface, c);
					}
				}
			}
		}
	}
}
