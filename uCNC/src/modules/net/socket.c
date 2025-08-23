/*
	Name: socket.c
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

#include "../../cnc.h"
#include "socket.h"
#include <string.h>

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

socket_if_t *socket_start(uint32_t ip_listen, uint16_t port, int domain, int type, int protocol)
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
	addr.sin_port = bsd_htons(port);			// default port (can be changed by user)
	addr.sin_addr = bsd_htonl(ip_listen); // any address

	if (bsd_bind(s, &addr, sizeof(addr)) < 0)
	{
		bsd_close(s);
		return NULL;
	}

	if (bsd_fcntl(s, F_SETFL, O_NONBLOCK) < 0)
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
	memset(raw_sockets[idx].socket_clients, -1, sizeof(raw_sockets[idx].socket_clients));
	return &raw_sockets[idx];
}

void socket_add_ondata_handler(socket_if_t *socket, socket_data_delegate callback)
{
	socket->client_ondata_cb = callback;
}

void socket_add_onconnected_handler(socket_if_t *socket, socket_connect_delegate callback)
{
	socket->client_onconnected_cb = callback;
}

void socket_add_ondisconnected_handler(socket_if_t *socket, socket_connect_delegate callback)
{
	socket->client_ondisconnected_cb = callback;
}

int socket_send(socket_if_t *socket, int client, char *data, size_t data_len, int flags)
{
	if (!socket)
		return -1;
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
			if (iface->client_onconnected_cb)
			{
				iface->client_onconnected_cb(client_fd);
			}
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
			if (iface->client_ondisconnected_cb)
			{
				iface->client_ondisconnected_cb(iface->socket_clients[idx]);
			}
			iface->socket_clients[idx] = -1;
		}
	}
}

void socket_server_dotasks(void)
{
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		socket_if_t *socket = &raw_sockets[i];
		char buffer[SOCKET_MAX_DATA_SIZE];

		if (socket->socket_if >= 0)
		{
			/* Accept new clients if TCP */
			int client_fd;
			struct bsd_sockaddr_in cli_addr;
			int cli_len = sizeof(cli_addr);

			client_fd = bsd_accept(socket->socket_if, &cli_addr, &cli_len);
			if (client_fd >= 0)
			{
				add_client(socket, client_fd);
			}

			/* Poll each client for data (non-blocking) */
			for (int c = 0; c < SOCKET_MAX_CLIENTS; c++)
			{
				int fd = socket->socket_clients[c];
				if (fd >= 0)
				{
					int len = bsd_recv(fd, buffer, sizeof(buffer), 0);
					if (len > 0)
					{
						if (socket->client_ondata_cb)
						{
							socket->client_ondata_cb(fd, buffer, (size_t)len);
						}
					}
					else if (len == 0)
					{
						remove_client(socket, c);
					}
				}
			}
		}
	}
}

int socket_server_hasclients(socket_if_t *socket)
{
	int clients = 0;
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		socket_if_t *s = &raw_sockets[i];
		if (s == socket || !socket)
			for (int c = 0; c < SOCKET_MAX_CLIENTS; c++)
			{
				if (s->socket_clients[c] >= 0)
				{
					clients++;
				}
			}
	}

	return clients;
}

DECL_MODULE(socket_server)
{
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		raw_sockets[i].socket_if = -1;
		memset(raw_sockets[i].socket_clients, -1, sizeof(raw_sockets[i].socket_clients));
		raw_sockets[i].client_ondata_cb = NULL;
		raw_sockets[i].client_onconnected_cb = NULL;
		raw_sockets[i].client_ondisconnected_cb = NULL;
	}
}
