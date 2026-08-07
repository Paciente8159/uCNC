/*
	Name: socket.c
	Description: Implements a simple Raw Socket Server for µCNC.

	The socket core is event-driven: the platform/network backend pushes
	connected/data/writable/disconnected events into this core through the
	socket_device_t callback table. The core no longer polls accept()/recv().

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
#ifdef ENABLE_SOCKETS

#include "socket.h"
#include <string.h>

/* Global socket interfaces */
static socket_if_t raw_sockets[MAX_SOCKETS];
static socket_device_t *socket_device;

static bool socket_device_connected(socket_handle_t listener, socket_handle_t client);
static void socket_device_data(socket_handle_t client, char *data, size_t len);
static void socket_device_writable(socket_handle_t client);
static void socket_device_disconnected(socket_handle_t client, int reason);

static const socket_device_events_t socket_device_events =
{
	.connected = socket_device_connected,
	.data = socket_device_data,
	.writable = socket_device_writable,
	.disconnected = socket_device_disconnected
};

bool socket_register_device(socket_device_t *device)
{
	if (!device)
	{
		return false;
	}

	socket_device = device;
	if (device->init)
	{
		return device->init(&socket_device_events) >= 0;
	}
	return true;
}

static socket_if_t *find_socket_if(socket_handle_t listener)
{
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (raw_sockets[i].socket_if == listener)
			return &raw_sockets[i];
	}
	return NULL;
}

static int find_free_socket_if(void)
{
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (raw_sockets[i].socket_if == SOCKET_INVALID_HANDLE)
			return i;
	}
	return -1;
}

socket_if_t *socket_start_listen(uint32_t ip_listen, uint16_t port, int domain, int type, int protocol)
{
	int idx = find_free_socket_if();
	if (idx < 0)
		return NULL;

	if (!socket_device || !socket_device->listen)
	{
		return NULL;
	}

	socket_handle_t listener = socket_device->listen(ip_listen, port, domain, type, protocol, SOCKET_MAX_CLIENTS);
	if (listener == SOCKET_INVALID_HANDLE)
		return NULL;

	memset(&raw_sockets[idx], 0, sizeof(raw_sockets[idx]));
	raw_sockets[idx].socket_if = listener;
	for (int i = 0; i < SOCKET_MAX_CLIENTS; i++)
	{
		raw_sockets[idx].socket_clients[i] = SOCKET_INVALID_HANDLE;
	}
	return &raw_sockets[idx];
}

void socket_stop_listening(socket_if_t *socket)
{
	if (!socket)
	{
		return;
	}

	for (uint8_t i = 0; i < SOCKET_MAX_CLIENTS; i++)
	{
		if (socket->socket_clients[i] != SOCKET_INVALID_HANDLE)
		{
			if (socket_device && socket_device->close)
				socket_device->close(socket->socket_clients[i]);
			if (socket->client_ondisconnected_cb)
			{
				socket->client_ondisconnected_cb(i, socket->protocol);
			}
#ifdef ENABLE_SOCKET_TIMEOUTS
			socket->client_activity[i] = 0;
#endif
			socket->socket_clients[i] = SOCKET_INVALID_HANDLE;
		}
	}

	if (socket->socket_if != SOCKET_INVALID_HANDLE)
	{
		if (socket_device && socket_device->close)
			socket_device->close(socket->socket_if);
		socket->socket_if = SOCKET_INVALID_HANDLE;
	}
	socket->client_ondata_cb = NULL;
	socket->client_onconnected_cb = NULL;
	socket->client_ondisconnected_cb = NULL;
	socket->client_onidle_cb = NULL;
	socket->protocol = NULL;
}

void socket_add_ondata_handler(socket_if_t *socket, socket_data_delegate callback)
{
	if (socket)
	{
		socket->client_ondata_cb = callback;
	}
}

void socket_add_onidle_handler(socket_if_t *socket, socket_idle_delegate callback)
{
	if (socket)
	{
		socket->client_onidle_cb = callback;
	}
}

void socket_add_onconnected_handler(socket_if_t *socket, socket_connect_delegate callback)
{
	if (socket)
	{
		socket->client_onconnected_cb = callback;
	}
}

void socket_add_ondisconnected_handler(socket_if_t *socket, socket_connect_delegate callback)
{
	if (socket)
	{
		socket->client_ondisconnected_cb = callback;
	}
}

int socket_send(socket_if_t *socket, uint8_t client_idx, char *data, size_t data_len, int flags)
{
	if (!socket || client_idx >= SOCKET_MAX_CLIENTS || !socket_device || !socket_device->send)
	{
		return SOCKET_DEVICE_INVALID;
	}

	socket_handle_t client = socket->socket_clients[client_idx];
	if (client == SOCKET_INVALID_HANDLE)
	{
		return SOCKET_DEVICE_INVALID;
	}

	int sent = 0;
	while (data_len)
	{
		int n = socket_device->send(client, data, data_len, flags);
		if (n <= 0)
		{
			/* Stop immediately on backpressure or error. Never busy-wait. */
			return (sent > 0) ? sent : n;
		}
		data += n;
		data_len -= (size_t)n;
		sent += n;
	}

	return sent;
}

int socket_broadcast(socket_if_t *socket, char *data, size_t data_len, int flags)
{
	if (!socket || !socket_device || !socket_device->send)
	{
		return SOCKET_DEVICE_INVALID;
	}

	int result = SOCKET_DEVICE_INVALID;
	bool have_client = false;
	for (int i = 0; i < SOCKET_MAX_CLIENTS; i++)
	{
		if (socket->socket_clients[i] != SOCKET_INVALID_HANDLE)
		{
			int s = socket_send(socket, (uint8_t)i, data, data_len, flags);
			if (!have_client)
			{
				/* Initialize the result from the first active client so a
				   successful send can produce a positive result. */
				result = s;
				have_client = true;
			}
			else if (s < result)
			{
				result = s;
			}
		}
	}

	return have_client ? result : 0;
}

/* Backend event sink: a new client connection was established */
static bool socket_device_connected(socket_handle_t listener, socket_handle_t client)
{
	socket_if_t *iface = find_socket_if(listener);
	if (!iface || client == SOCKET_INVALID_HANDLE)
	{
		return false;
	}

	for (uint8_t i = 0; i < SOCKET_MAX_CLIENTS; i++)
	{
		if (iface->socket_clients[i] == SOCKET_INVALID_HANDLE)
		{
			iface->socket_clients[i] = client;
#ifdef ENABLE_SOCKET_TIMEOUTS
			iface->client_activity[i] = mcu_millis();
#endif
			if (iface->client_onconnected_cb)
			{
				iface->client_onconnected_cb(i, iface->protocol);
			}
			return true;
		}
	}

	/* No logical client slot available; the backend owns rejection/cleanup */
	return false;
}

/* Backend event sink: TCP payload is available */
static void socket_device_data(socket_handle_t client, char *data, size_t len)
{
	socket_if_t *iface = NULL;
	uint8_t client_idx = 0;
	bool found = false;

	for (int i = 0; i < MAX_SOCKETS && !found; i++)
	{
		for (uint8_t c = 0; c < SOCKET_MAX_CLIENTS; c++)
		{
			if (raw_sockets[i].socket_clients[c] == client)
			{
				iface = &raw_sockets[i];
				client_idx = c;
				found = true;
				break;
			}
		}
	}

	if (!found)
	{
		/* Unknown/stale handle; ignore */
		return;
	}

#ifdef ENABLE_SOCKET_TIMEOUTS
	iface->client_activity[client_idx] = mcu_millis();
#endif
	data[len] = '\0';
	if (iface->client_ondata_cb)
	{
		iface->client_ondata_cb(client_idx, data, len, iface->protocol);
	}
}

/* Backend event sink: reserved for future TX-progress notification */
static void socket_device_writable(socket_handle_t client)
{
	(void)client;
}

/* Backend event sink: remote orderly close or fatal transport error */
static void socket_device_disconnected(socket_handle_t client, int reason)
{
	(void)reason;
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		for (uint8_t c = 0; c < SOCKET_MAX_CLIENTS; c++)
		{
			if (raw_sockets[i].socket_clients[c] == client)
			{
				raw_sockets[i].socket_clients[c] = SOCKET_INVALID_HANDLE;
#ifdef ENABLE_SOCKET_TIMEOUTS
				raw_sockets[i].client_activity[c] = 0;
#endif
				/* Mark the client invalid before notifying so a disconnect
				   callback cannot write to an already closed transport */
				if (raw_sockets[i].client_ondisconnected_cb)
				{
					raw_sockets[i].client_ondisconnected_cb(c, raw_sockets[i].protocol);
				}
				return;
			}
		}
	}
}

// closes a connection to a client (local explicit close)
void socket_close(socket_if_t *socket, uint8_t client_idx)
{
	if (!socket || client_idx >= SOCKET_MAX_CLIENTS)
	{
		return;
	}

	socket_handle_t client = socket->socket_clients[client_idx];
	if (client == SOCKET_INVALID_HANDLE)
	{
		return;
	}

	/* Mark the logical client invalid first so a disconnect callback cannot
	   accidentally write to an already closed transport */
	socket->socket_clients[client_idx] = SOCKET_INVALID_HANDLE;
#ifdef ENABLE_SOCKET_TIMEOUTS
	socket->client_activity[client_idx] = 0;
#endif

	if (socket_device && socket_device->close)
	{
		socket_device->close(client);
	}

	if (socket->client_ondisconnected_cb)
	{
		socket->client_ondisconnected_cb(client_idx, socket->protocol);
	}
}

void socket_server_dotasks(void)
{
	static uint8_t srv_idx = 0;
	static uint8_t clt_idx = 0;
	const uint8_t i = srv_idx;
	const uint8_t c = clt_idx;

	if (!socket_device)
		return;

	if (socket_device->service)
	{
		/* Let the backend push connected/data/disconnected events */
		socket_device->service();
	}

	/* Cooperative idle processing: inspect at most one logical client per
	   socket_server_dotasks() call to keep the work strictly bounded */
	socket_if_t *socket = &raw_sockets[i];
	if (socket->socket_if != SOCKET_INVALID_HANDLE && socket->client_onidle_cb &&
		socket->socket_clients[c] != SOCKET_INVALID_HANDLE)
	{
#ifdef ENABLE_SOCKET_TIMEOUTS
		uint32_t idle = mcu_millis() - socket->client_activity[c];
		socket->client_onidle_cb(c, idle, socket->protocol);
#else
		socket->client_onidle_cb(c, 0, socket->protocol);
#endif
	}

	/* Round-robin through clients and listeners, one step per call */
	if (++clt_idx >= SOCKET_MAX_CLIENTS)
	{
		clt_idx = 0;
		srv_idx = (uint8_t)((srv_idx + 1) < MAX_SOCKETS ? (srv_idx + 1) : 0);
	}
}

int socket_server_hasclients(socket_if_t *socket)
{
	int clients = 0;
	if (socket != NULL)
	{
		for (int c = 0; c < SOCKET_MAX_CLIENTS; c++)
		{
			if (socket->socket_clients[c] != SOCKET_INVALID_HANDLE)
			{
				clients++;
			}
		}
	}

	return clients;
}

int socket_get_clientindex(socket_if_t *socket, socket_handle_t socket_fd)
{
	for (int c = 0; c < SOCKET_MAX_CLIENTS; c++)
	{
		if (socket->socket_clients[c] == socket_fd)
		{
			return c;
		}
	}
	return -1;
}

DECL_MODULE(socket_server)
{
	RUNONCE
	{
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			raw_sockets[i].socket_if = SOCKET_INVALID_HANDLE;
			for (int j = 0; j < SOCKET_MAX_CLIENTS; j++)
			{
				raw_sockets[i].socket_clients[j] = SOCKET_INVALID_HANDLE;
			}
			raw_sockets[i].client_ondata_cb = NULL;
			raw_sockets[i].client_onidle_cb = NULL;
			raw_sockets[i].client_onconnected_cb = NULL;
			raw_sockets[i].client_ondisconnected_cb = NULL;
			raw_sockets[i].protocol = NULL;
		}
		RUNONCE_COMPLETE();
	}
}

#endif
