/*
	Name: esp32_lwip.c
	Description: Event-driven lwIP socket backend for ESP32 (µCNC socket device).

	This backend uses the ESP-IDF lwIP socket API (lwip_socket/lwip_bind/
	lwip_listen/lwip_accept/lwip_recv/lwip_send/lwip_select/lwip_close) and
	pushes connected/data/writable/disconnected events into the µCNC socket
	core through the socket_device_t callback table. It contains no C++
	dependencies and lives in esp32common.

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

#include "../../../cnc.h"

#if (defined(ESP32) || defined(ESP32S3) || defined(ESP32C3))
#if defined(ENABLE_SOCKETS)

#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "lwip/sockets.h"
#include "../../../modules/net/socket.h"

#ifndef ESP32_MAX_LISTENERS
#define ESP32_MAX_LISTENERS MAX_SOCKETS
#endif
#ifndef ESP32_MAX_CLIENTS
#define ESP32_MAX_CLIENTS (MAX_SOCKETS * SOCKET_MAX_CLIENTS)
#endif

typedef struct
{
	bool in_use;
	int native_fd;
	bool write_blocked;
	socket_handle_t srv_handle;
} esp32_client_t;

static esp32_client_t esp32_clients[ESP32_MAX_CLIENTS];
static int esp32_listeners[MAX_SOCKETS];
static const socket_device_events_t *esp32_socket_events;
static bool esp32_net_started = false;

static int esp32_map_errno(int err)
{
	switch (err)
	{
	case EWOULDBLOCK: /* EAGAIN has the same value on ESP32 */
		return SOCKET_DEVICE_WOULD_BLOCK;
	case ENOMEM:
	case ENOBUFS:
		return SOCKET_DEVICE_NO_MEMORY;
	case EBADF:
	case ENOTSOCK:
		return SOCKET_DEVICE_INVALID;
	case ENOTCONN:
	case EPIPE:
		return SOCKET_DEVICE_CLOSED;
	default:
		return SOCKET_DEVICE_ERROR;
	}
}

static int esp32_socket_device_init(const socket_device_events_t *events)
{
	if (!events)
	{
		return -1;
	}
	esp32_socket_events = events;

	memset(esp32_clients, 0, sizeof(esp32_clients));
	for (int i = 0; i < ESP32_MAX_CLIENTS; i++)
	{
		esp32_clients[i].native_fd = -1;
		esp32_clients[i].srv_handle = SOCKET_INVALID_HANDLE;
	}
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		esp32_listeners[i] = -1;
	}

	/* lwIP socket API needs no global startup on ESP32 */
	esp32_net_started = true;
	return 0;
}

static socket_handle_t esp32_socket_listen(uint32_t ip_listen, uint16_t port, int domain, int type, int protocol, uint8_t backlog)
{
	(void)domain;
	(void)protocol;
	if (type != SOCK_STREAM)
	{
		return SOCKET_INVALID_HANDLE;
	}

	int listener = -1;
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (esp32_listeners[i] < 0)
		{
			listener = i;
			break;
		}
	}
	if (listener < 0)
	{
		return SOCKET_INVALID_HANDLE;
	}

	int fd = lwip_socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		return SOCKET_INVALID_HANDLE;
	}

	/* Non-blocking listener */
	int fl = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, fl | O_NONBLOCK);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(ip_listen); /* IP_ANY == 0 preserved */

	if (lwip_bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0 ||
		lwip_listen(fd, backlog > 0 ? backlog : SOCKET_MAX_CLIENTS) < 0)
	{
		lwip_close(fd);
		return SOCKET_INVALID_HANDLE;
	}

	esp32_listeners[listener] = fd;
	return (socket_handle_t)fd;
}

static int esp32_socket_send(socket_handle_t client, const void *data, size_t len, int flags)
{
	if (client == SOCKET_INVALID_HANDLE)
	{
		return SOCKET_DEVICE_INVALID;
	}

	int idx = (int)client;
	if (idx < 0 || idx >= ESP32_MAX_CLIENTS || !esp32_clients[idx].in_use)
	{
		return SOCKET_DEVICE_INVALID;
	}

	int result = lwip_send(esp32_clients[idx].native_fd, data, len, flags);
	if (result < 0)
	{
		int err = errno;
		if (err == EWOULDBLOCK || err == EAGAIN)
		{
			esp32_clients[idx].write_blocked = true;
			return SOCKET_DEVICE_WOULD_BLOCK;
		}
		return esp32_map_errno(err);
	}
	return result;
}

static int esp32_socket_close(socket_handle_t handle)
{
	if (handle == SOCKET_INVALID_HANDLE)
	{
		return SOCKET_DEVICE_INVALID;
	}

	/* Listener slot? */
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (esp32_listeners[i] == (int)handle)
		{
			lwip_close(esp32_listeners[i]);
			esp32_listeners[i] = -1;
			return 0;
		}
	}

	/* Client slot? */
	int idx = (int)handle;
	if (idx >= 0 && idx < ESP32_MAX_CLIENTS && esp32_clients[idx].in_use)
	{
		lwip_close(esp32_clients[idx].native_fd);
		esp32_clients[idx].in_use = false;
		esp32_clients[idx].write_blocked = false;
		esp32_clients[idx].native_fd = -1;
		esp32_clients[idx].srv_handle = SOCKET_INVALID_HANDLE;
		return 0;
	}

	return SOCKET_DEVICE_INVALID;
}

static void esp32_socket_service(void)
{
	if (!esp32_socket_events)
	{
		return;
	}

	fd_set readfds, writefds;
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);

	int maxfd = -1;
	int count = 0;

	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (esp32_listeners[i] >= 0)
		{
			FD_SET(esp32_listeners[i], &readfds);
			if (esp32_listeners[i] > maxfd)
				maxfd = esp32_listeners[i];
			count++;
		}
	}
	for (int i = 0; i < ESP32_MAX_CLIENTS; i++)
	{
		if (esp32_clients[i].in_use)
		{
			FD_SET(esp32_clients[i].native_fd, &readfds);
			if (esp32_clients[i].write_blocked)
			{
				FD_SET(esp32_clients[i].native_fd, &writefds);
			}
			if (esp32_clients[i].native_fd > maxfd)
				maxfd = esp32_clients[i].native_fd;
			count++;
		}
	}

	if (count == 0)
	{
		return;
	}

	struct timeval timeout = {0, 0};
	int ready = lwip_select(maxfd + 1, &readfds, &writefds, NULL, &timeout);
	if (ready <= 0)
	{
		return;
	}

	/* Accept new clients (bounded: one listener with a pending client per call) */
	int accepted_this_call = 0;
	for (int i = 0; i < MAX_SOCKETS && accepted_this_call < MAX_SOCKETS; i++)
	{
		if (esp32_listeners[i] < 0 || !FD_ISSET(esp32_listeners[i], &readfds))
		{
			continue;
		}

		int fd = lwip_accept(esp32_listeners[i], NULL, NULL);
		if (fd < 0)
		{
			continue;
		}

		/* Explicitly configure the accepted client as non-blocking */
		int fl = fcntl(fd, F_GETFL, 0);
		fcntl(fd, F_SETFL, fl | O_NONBLOCK);

		int slot = -1;
		for (int c = 0; c < ESP32_MAX_CLIENTS; c++)
		{
			if (!esp32_clients[c].in_use)
			{
				slot = c;
				break;
			}
		}
		if (slot < 0)
		{
			lwip_close(fd);
			continue;
		}

		esp32_clients[slot].in_use = true;
		esp32_clients[slot].write_blocked = false;
		esp32_clients[slot].native_fd = fd;
		esp32_clients[slot].srv_handle = (socket_handle_t)esp32_listeners[i];

		if (!esp32_socket_events->connected((socket_handle_t)esp32_listeners[i], (socket_handle_t)slot))
		{
			/* No µCNC client slot: close and discard on the backend side */
			lwip_close(fd);
			esp32_clients[slot].in_use = false;
			esp32_clients[slot].write_blocked = false;
			esp32_clients[slot].native_fd = -1;
			esp32_clients[slot].srv_handle = SOCKET_INVALID_HANDLE;
		}
		accepted_this_call++;
	}

	/* Read pending data (bounded: one recv per ready client per call) */
	static char srv_buffer[SOCKET_MAX_DATA_SIZE + 1];
	for (int i = 0; i < ESP32_MAX_CLIENTS; i++)
	{
		if (!esp32_clients[i].in_use)
		{
			continue;
		}

		socket_handle_t handle = (socket_handle_t)i;
		int fd = esp32_clients[i].native_fd;

		if (FD_ISSET(fd, &readfds))
		{
			ssize_t len = lwip_recv(fd, srv_buffer, SOCKET_MAX_DATA_SIZE, 0);
			if (len > 0)
			{
				srv_buffer[len] = '\0';
				esp32_socket_events->data(handle, srv_buffer, (size_t)len);
			}
			else if (len == 0)
			{
				/* Orderly remote disconnect */
				esp32_clients[i].in_use = false;
				esp32_clients[i].write_blocked = false;
				esp32_clients[i].native_fd = -1;
				esp32_clients[i].srv_handle = SOCKET_INVALID_HANDLE;
				lwip_close(fd);
				esp32_socket_events->disconnected(handle, 0);
			}
			else
			{
				int err = errno;
				if (err != EWOULDBLOCK && err != EAGAIN)
				{
					esp32_clients[i].in_use = false;
					esp32_clients[i].write_blocked = false;
					esp32_clients[i].native_fd = -1;
					esp32_clients[i].srv_handle = SOCKET_INVALID_HANDLE;
					lwip_close(fd);
					esp32_socket_events->disconnected(handle, esp32_map_errno(err));
				}
			}
		}

		/* Writable notification for clients previously blocked on send */
		if (esp32_clients[i].in_use && esp32_clients[i].write_blocked &&
			FD_ISSET(esp32_clients[i].native_fd, &writefds))
		{
			esp32_clients[i].write_blocked = false;
			esp32_socket_events->writable(handle);
		}
	}
}

socket_device_t wifi_socket =
{
	.init = esp32_socket_device_init,
	.listen = esp32_socket_listen,
	.send = esp32_socket_send,
	.close = esp32_socket_close,
	.service = esp32_socket_service
};

#endif /* ENABLE_SOCKETS */
#endif /* ESP32 */
