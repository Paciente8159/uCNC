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
static int esp32_listeners[ESP32_MAX_LISTENERS];
static const socket_device_events_t *esp32_socket_events;
static bool esp32_net_started = false;
/*
 * The RX buffer used by service() is shared. While it is being exposed to
 * the socket core/application, nested service calls may still process TX and
 * socket errors, but must not accept clients or perform another RX operation.
 */
static bool esp32_rx_dispatching = false;

static int esp32_map_errno(int err)
{
	switch (err)
	{
	case EWOULDBLOCK:
		return SOCKET_DEVICE_WOULD_BLOCK;

	case ENOMEM:
	case ENOBUFS:
		return SOCKET_DEVICE_NO_MEMORY;

	case EBADF:
	case ENOTSOCK:
		return SOCKET_DEVICE_INVALID;

	case ENOTCONN:
	case EPIPE:
	case ECONNRESET:
	case ECONNABORTED:
		return SOCKET_DEVICE_CLOSED;

	default:
		return SOCKET_DEVICE_ERROR;
	}
}

static bool esp32_set_nonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);

	if (flags < 0)
	{
		return false;
	}

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		return false;
	}

	return true;
}

static int esp32_find_client(socket_handle_t handle)
{
	if (handle == SOCKET_INVALID_HANDLE)
	{
		return -1;
	}

	int fd = (int)handle;

	for (int i = 0; i < ESP32_MAX_CLIENTS; i++)
	{
		if (esp32_clients[i].in_use &&
			esp32_clients[i].native_fd == fd)
		{
			return i;
		}
	}

	return -1;
}

static int esp32_find_free_client(void)
{
	for (int i = 0; i < ESP32_MAX_CLIENTS; i++)
	{
		if (!esp32_clients[i].in_use)
		{
			return i;
		}
	}

	return -1;
}

static void esp32_release_client(int idx, bool close_socket)
{
	if (idx < 0 || idx >= ESP32_MAX_CLIENTS)
	{
		return;
	}

	esp32_client_t *client = &esp32_clients[idx];

	if (!client->in_use)
	{
		return;
	}

	int fd = client->native_fd;

	/*
	 * Clear backend state before closing or notifying the core.
	 * This protects against re-entrant application callbacks.
	 */
	client->in_use = false;
	client->write_blocked = false;
	client->native_fd = -1;
	client->srv_handle = SOCKET_INVALID_HANDLE;

	if (close_socket && fd >= 0)
	{
		lwip_close(fd);
	}
}

static int esp32_socket_device_init(
	const socket_device_events_t *events)
{
	if (!events)
	{
		return SOCKET_DEVICE_INVALID;
	}

	esp32_socket_events = events;
	esp32_rx_dispatching = false;

	memset(
		esp32_clients,
		0,
		sizeof(esp32_clients));

	for (int i = 0; i < ESP32_MAX_CLIENTS; i++)
	{
		esp32_clients[i].in_use = false;
		esp32_clients[i].native_fd = -1;
		esp32_clients[i].write_blocked = false;
		esp32_clients[i].srv_handle =
			SOCKET_INVALID_HANDLE;
	}

	for (int i = 0; i < ESP32_MAX_LISTENERS; i++)
	{
		esp32_listeners[i] = -1;
	}

	/*
	 * ESP-IDF/lwIP sockets require no BSD-style
	 * global startup operation.
	 */
	return SOCKET_DEVICE_OK;
}

static socket_handle_t esp32_socket_listen(
	uint32_t ip_listen,
	uint16_t port,
	int domain,
	int type,
	int protocol,
	uint8_t backlog)
{
	/*
	 * This backend currently supports IPv4 TCP only.
	 */
	if (domain != AF_INET ||
		type != SOCK_STREAM)
	{
		return SOCKET_INVALID_HANDLE;
	}

	int listener_slot = -1;

	for (int i = 0; i < ESP32_MAX_LISTENERS; i++)
	{
		if (esp32_listeners[i] < 0)
		{
			listener_slot = i;
			break;
		}
	}

	if (listener_slot < 0)
	{
		return SOCKET_INVALID_HANDLE;
	}

	int fd = lwip_socket(
		domain,
		type,
		protocol);

	if (fd < 0)
	{
		return SOCKET_INVALID_HANDLE;
	}

	/*
	 * The socket-device contract requires listeners
	 * to be strictly non-blocking.
	 */
	if (!esp32_set_nonblocking(fd))
	{
		lwip_close(fd);
		return SOCKET_INVALID_HANDLE;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(ip_listen);

	if (lwip_bind(
			fd,
			(struct sockaddr *)&addr,
			sizeof(addr)) < 0)
	{
		lwip_close(fd);
		return SOCKET_INVALID_HANDLE;
	}

	int listen_backlog =
		(backlog > 0)
			? backlog
			: SOCKET_MAX_CLIENTS;

	if (lwip_listen(
			fd,
			listen_backlog) < 0)
	{
		lwip_close(fd);
		return SOCKET_INVALID_HANDLE;
	}

	esp32_listeners[listener_slot] = fd;

	/*
	 * Listener handle is the native fd.
	 */
	return (socket_handle_t)fd;
}

static int esp32_socket_send(
	socket_handle_t client,
	const void *data,
	size_t len,
	int flags)
{
	if (client == SOCKET_INVALID_HANDLE ||
		!data)
	{
		return SOCKET_DEVICE_INVALID;
	}

	if (len == 0)
	{
		return 0;
	}

	int idx = esp32_find_client(client);

	if (idx < 0)
	{
		return SOCKET_DEVICE_INVALID;
	}

	esp32_client_t *entry =
		&esp32_clients[idx];

	int result = lwip_send(
		entry->native_fd,
		data,
		len,
		flags);

	if (result > 0)
	{
		return result;
	}

	if (result == 0)
	{
		/*
		 * For a non-empty TCP send this should not normally
		 * occur. Treat it as a closed connection.
		 */
		return SOCKET_DEVICE_CLOSED;
	}

	int err = errno;

	if (err == EWOULDBLOCK ||
		err == EAGAIN)
	{
		entry->write_blocked = true;

		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	return esp32_map_errno(err);
}

static int esp32_socket_close(
	socket_handle_t handle)
{
	if (handle == SOCKET_INVALID_HANDLE)
	{
		return SOCKET_DEVICE_INVALID;
	}

	int fd = (int)handle;

	/*
	 * Check listener handles.
	 *
	 * Since both clients and listeners now use their native fd
	 * as the generic handle, there cannot be an active client
	 * and active listener with the same fd.
	 */
	for (int i = 0; i < ESP32_MAX_LISTENERS; i++)
	{
		if (esp32_listeners[i] == fd)
		{
			esp32_listeners[i] = -1;
			lwip_close(fd);

			return SOCKET_DEVICE_OK;
		}
	}

	/*
	 * Check clients by native fd.
	 */
	int idx = esp32_find_client(handle);

	if (idx >= 0)
	{
		esp32_release_client(idx, true);

		/*
		 * Explicit local close does not generate
		 * a disconnected backend event.
		 */
		return SOCKET_DEVICE_OK;
	}

	return SOCKET_DEVICE_INVALID;
}

static void esp32_disconnect_client(
	int idx,
	int reason)
{
	if (idx < 0 ||
		idx >= ESP32_MAX_CLIENTS ||
		!esp32_clients[idx].in_use)
	{
		return;
	}

	socket_handle_t handle =
		(socket_handle_t)esp32_clients[idx].native_fd;

	/*
	 * The backend contract says native resources must
	 * already be released before disconnected() is called.
	 */
	esp32_release_client(idx, true);

	if (esp32_socket_events &&
		esp32_socket_events->disconnected)
	{
		esp32_socket_events->disconnected(
			handle,
			reason);
	}
}

static void esp32_socket_service(void)
{
	if (!esp32_socket_events)
	{
		return;
	}

	static uint8_t next_listener = 0;
	static uint8_t next_client = 0;

	fd_set readfds;
	fd_set writefds;
	fd_set exceptfds;

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);

	int maxfd = -1;
	bool have_sockets = false;

	/*
	 * Build listener read set.
	 */
	for (int i = 0; i < ESP32_MAX_LISTENERS; i++)
	{
		int fd = esp32_listeners[i];

		if (fd < 0 || esp32_rx_dispatching)
		{
			continue;
		}

		FD_SET(fd, &readfds);

		if (fd > maxfd)
		{
			maxfd = fd;
		}

		have_sockets = true;
	}

	/*
	 * Build client read/write/exception sets.
	 */
	for (int i = 0; i < ESP32_MAX_CLIENTS; i++)
	{
		if (!esp32_clients[i].in_use)
		{
			continue;
		}

		int fd = esp32_clients[i].native_fd;

		if (fd < 0)
		{
			continue;
		}

		/* Do not overwrite the shared RX buffer from a nested service call. */
		if (!esp32_rx_dispatching)
		{
			FD_SET(fd, &readfds);
		}
		FD_SET(fd, &exceptfds);

		if (esp32_clients[i].write_blocked)
		{
			FD_SET(fd, &writefds);
		}

		if (fd > maxfd)
		{
			maxfd = fd;
		}

		have_sockets = true;
	}

	if (!have_sockets || maxfd < 0)
	{
		return;
	}

	/*
	 * Zero timeout: service() must never block.
	 */
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	int ready = lwip_select(
		maxfd + 1,
		&readfds,
		&writefds,
		&exceptfds,
		&timeout);

	if (ready <= 0)
	{
		return;
	}

	/*
	 * Process at most one pending accept.
	 *
	 * Start from next_listener to avoid permanently
	 * favoring listener zero.
	 */
	for (int n = 0; n < ESP32_MAX_LISTENERS; n++)
	{
		int listener_idx =
			(next_listener + n) %
			ESP32_MAX_LISTENERS;

		int listener_fd =
			esp32_listeners[listener_idx];

		if (listener_fd < 0 ||
			!FD_ISSET(listener_fd, &readfds))
		{
			continue;
		}

		int fd = lwip_accept(
			listener_fd,
			NULL,
			NULL);

		if (fd >= 0)
		{
			if (!esp32_set_nonblocking(fd))
			{
				lwip_close(fd);
			}
			else
			{
				int slot =
					esp32_find_free_client();

				if (slot < 0)
				{
					lwip_close(fd);
				}
				else
				{
					esp32_clients[slot].in_use = true;
					esp32_clients[slot].native_fd = fd;
					esp32_clients[slot].write_blocked = false;
					esp32_clients[slot].srv_handle =
						(socket_handle_t)listener_fd;

					/*
					 * Client handle is also the native fd.
					 */
					bool accepted = true;

					if (esp32_socket_events->connected)
					{
						accepted =
							esp32_socket_events->connected(
								(socket_handle_t)listener_fd,
								(socket_handle_t)fd);
					}

					if (!accepted)
					{
						/*
						 * Core has no logical client slot.
						 * Backend owns rejection.
						 */
						esp32_release_client(
							slot,
							true);
					}
				}
			}
		}

		next_listener =
			(uint8_t)((listener_idx + 1) %
					  ESP32_MAX_LISTENERS);

		/*
		 * One accept maximum per service call.
		 */
		break;
	}

	/*
	 * Process at most one ready client.
	 */
	for (int n = 0; n < ESP32_MAX_CLIENTS; n++)
	{
		int idx =
			(next_client + n) %
			ESP32_MAX_CLIENTS;

		if (!esp32_clients[idx].in_use)
		{
			continue;
		}

		int fd =
			esp32_clients[idx].native_fd;

		if (fd < 0)
		{
			continue;
		}

		bool has_exception =
			FD_ISSET(fd, &exceptfds);

		bool has_read =
			FD_ISSET(fd, &readfds);

		bool has_write =
			esp32_clients[idx].write_blocked &&
			FD_ISSET(fd, &writefds);

		if (!has_exception &&
			!has_read &&
			!has_write)
		{
			continue;
		}

		next_client =
			(uint8_t)((idx + 1) %
					  ESP32_MAX_CLIENTS);

		socket_handle_t handle =
			(socket_handle_t)fd;

		/*
		 * Handle socket errors before normal RX/TX readiness.
		 */
		if (has_exception)
		{
			int socket_error = 0;
			socklen_t error_len =
				sizeof(socket_error);

			int result =
				lwip_getsockopt(
					fd,
					SOL_SOCKET,
					SO_ERROR,
					&socket_error,
					&error_len);

			if (result < 0)
			{
				socket_error = errno;
			}

			int reason =
				(socket_error != 0)
					? esp32_map_errno(socket_error)
					: SOCKET_DEVICE_ERROR;

			esp32_disconnect_client(
				idx,
				reason);

			break;
		}

		/*
		 * Process one RX operation maximum.
		 */
		if (has_read)
		{
			static char srv_buffer[
				SOCKET_MAX_DATA_SIZE + 1];

			ssize_t len =
				lwip_recv(
					fd,
					srv_buffer,
					SOCKET_MAX_DATA_SIZE,
					MSG_PEEK);

			if (len > 0)
			{
				bool consumed = true;

				srv_buffer[len] = '\0';

				if (esp32_socket_events->data)
				{
					esp32_rx_dispatching = true;

					consumed = esp32_socket_events->data(
						handle,
						srv_buffer,
						(size_t)len);

					esp32_rx_dispatching = false;
				}

				/*
				 * data() may synchronously cause socket_close().
				 * Do not touch this client entry unless it
				 * still represents the same connection.
				 */
				if (!esp32_clients[idx].in_use ||
					esp32_clients[idx].native_fd != fd)
				{
					break;
				}

				if (!consumed)
				{
					/*
					 * The first receive used MSG_PEEK, so the payload is
					 * still queued in lwIP and will be offered again during
					 * a later service pass.
					 */
					break;
				}

				/*
				 * The application accepted the complete payload. Remove
				 * exactly those bytes from lwIP's receive queue now.
				 */
				ssize_t removed =
					lwip_recv(
						fd,
						srv_buffer,
						(size_t)len,
						0);

				if (removed != len)
				{
					/*
					 * The application has already processed all len bytes.
					 * Continuing after a partial removal could deliver a
					 * suffix twice, so fail the connection instead.
					 */
					int reason = SOCKET_DEVICE_ERROR;

					if (removed < 0)
					{
						reason = esp32_map_errno(errno);
					}
					else if (removed == 0)
					{
						reason = SOCKET_DEVICE_CLOSED;
					}

					esp32_disconnect_client(
						idx,
						reason);

					break;
				}
			}
			else if (len == 0)
			{
				/*
				 * Orderly FIN from remote peer.
				 */
				esp32_disconnect_client(
					idx,
					SOCKET_DEVICE_OK);

				break;
			}
			else
			{
				int err = errno;

				if (err != EWOULDBLOCK &&
					err != EAGAIN)
				{
					esp32_disconnect_client(
						idx,
						esp32_map_errno(err));

					break;
				}
			}
		}

		/*
		 * data() above may have closed the client.
		 */
		if (!esp32_clients[idx].in_use ||
			esp32_clients[idx].native_fd != fd)
		{
			break;
		}

		/*
		 * Notify TX progress only after a previous
		 * WOULD_BLOCK condition.
		 */
		if (has_write &&
			esp32_clients[idx].write_blocked)
		{
			esp32_clients[idx].write_blocked =
				false;

			if (esp32_socket_events->writable)
			{
				esp32_socket_events->writable(
					handle);
			}
		}

		/*
		 * One ready client maximum per service call.
		 */
		break;
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
