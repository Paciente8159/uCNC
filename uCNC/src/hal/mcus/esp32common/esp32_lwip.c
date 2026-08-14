/*
	Name: esp32_lwip.c
	Description: Allocation-free ESP32/lwIP BSD-socket backend for uCNC.

	This backend is a thin, readiness-driven adapter over the ESP-IDF lwIP BSD
	socket API. lwIP remains responsible for TCP sequencing, retransmission,
	flow/congestion control and connection teardown. The backend owns only fixed
	listener/client records and never copies payload into a private RX/TX buffer.

	All public backend functions, including poll(), must be called from the one
	uCNC socket-owner context. ESP-IDF's BSD socket layer safely marshals work to
	the lwIP TCP/IP FreeRTOS task; no native callback invokes the uCNC core.

	Static state is approximately:
	  ESP32_MAX_LISTENERS * sizeof(int) +
	  ESP32_MAX_CLIENTS * sizeof(esp32_client_t)
	plus one event-table pointer and small round-robin cursors. On a 32-bit ESP32,
	esp32_client_t is normally 12 bytes. No backend payload buffer is allocated.

	Copyright: Copyright (c) Joao Martins
	Author: Joao Martins

	uCNC is free software: you can redistribute it and/or modify it under the
	terms of the GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later
	version. Please see <http://www.gnu.org/licenses/>.
*/

#include "../../../cnc.h"

#if (defined(ESP32) || defined(ESP32S3) || defined(ESP32C3))
#if defined(ENABLE_SOCKETS)

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>

#include "lwip/sockets.h"

#ifndef ESP32_MAX_LISTENERS
#define ESP32_MAX_LISTENERS MAX_SOCKETS
#endif

/*
 * Use the core's independently configurable total-connection limit instead of
 * always reserving MAX_SOCKETS * SOCKET_MAX_CLIENTS records.
 */
#ifndef ESP32_MAX_CLIENTS
#define ESP32_MAX_CLIENTS SOCKET_MAX_CONNECTIONS
#endif

/*
 * ESP-IDF must also provide enough native descriptors. To make every fixed
 * slot simultaneously usable, CONFIG_LWIP_MAX_SOCKETS must be at least the
 * number of active listeners plus active clients. A smaller native limit is
 * safe, but socket()/accept() will cap the reachable runtime capacity.
 */

#if ESP32_MAX_LISTENERS == 0
#error "ESP32_MAX_LISTENERS must be greater than zero"
#endif

#if ESP32_MAX_CLIENTS == 0
#error "ESP32_MAX_CLIENTS must be greater than zero"
#endif

enum
{
	ESP32_CLIENT_IN_USE = 1U << 0,
	/* A readable hint is outstanding in the core. */
	ESP32_CLIENT_READ_NOTIFIED = 1U << 1,
	/* A partial/blocked send requires one later writable transition. */
	ESP32_CLIENT_WANT_WRITE = 1U << 2
};

typedef struct esp32_client_
{
	int fd;
	socket_device_token_t token;
	/* Zero means no deferred fatal error; otherwise a normalized result. */
	int8_t pending_error;
	uint8_t flags;
} esp32_client_t;

static int esp32_listeners[ESP32_MAX_LISTENERS];
static esp32_client_t esp32_clients[ESP32_MAX_CLIENTS];
static const socket_device_events_t *esp32_events;
static uint16_t esp32_next_listener;
static uint16_t esp32_next_client;
static bool esp32_prefer_accept;

/* EAGAIN and EWOULDBLOCK are commonly the same value, so avoid switch cases. */
static bool esp32_errno_is_temporary(int error)
{
#ifdef EAGAIN
	if (error == EAGAIN)
	{
		return true;
	}
#endif
#ifdef EWOULDBLOCK
	if (error == EWOULDBLOCK)
	{
		return true;
	}
#endif
#ifdef EINTR
	if (error == EINTR)
	{
		return true;
	}
#endif
#ifdef EINPROGRESS
	if (error == EINPROGRESS)
	{
		return true;
	}
#endif
#ifdef EALREADY
	if (error == EALREADY)
	{
		return true;
	}
#endif
	return false;
}

/*
 * Normalizes non-success native errors. An orderly peer FIN is not an errno;
 * it is recognized only from recv() returning zero and maps to CLOSED there.
 * Resets, aborts, timeouts and link failures remain distinguishable from an
 * orderly close by mapping to ERROR.
 */
static int esp32_map_errno(int error)
{
	if (esp32_errno_is_temporary(error))
	{
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

#ifdef ENOMEM
	if (error == ENOMEM)
	{
		return SOCKET_DEVICE_NO_MEMORY;
	}
#endif
#ifdef ENOBUFS
	if (error == ENOBUFS)
	{
		return SOCKET_DEVICE_NO_MEMORY;
	}
#endif
#ifdef EBADF
	if (error == EBADF)
	{
		return SOCKET_DEVICE_INVALID;
	}
#endif
#ifdef ENOTSOCK
	if (error == ENOTSOCK)
	{
		return SOCKET_DEVICE_INVALID;
	}
#endif
#ifdef EINVAL
	if (error == EINVAL)
	{
		return SOCKET_DEVICE_INVALID;
	}
#endif

	return SOCKET_DEVICE_ERROR;
}

/* lwIP can report temporary internal pbuf/netconn pressure as ENOMEM/ENOBUFS. */
static bool esp32_errno_is_data_backpressure(int error)
{
	if (esp32_errno_is_temporary(error))
	{
		return true;
	}
#ifdef ENOMEM
	if (error == ENOMEM)
	{
		return true;
	}
#endif
#ifdef ENOBUFS
	if (error == ENOBUFS)
	{
		return true;
	}
#endif
	return false;
}

static bool esp32_fd_is_selectable(int fd)
{
	if (fd < 0)
	{
		return false;
	}
#ifdef FD_SETSIZE
	if (fd >= FD_SETSIZE)
	{
		return false;
	}
#endif
	return true;
}

static bool esp32_handle_to_fd(socket_device_handle_t handle, int *fd)
{
	if (!fd || handle == SOCKET_DEVICE_INVALID_HANDLE ||
		handle > (socket_device_handle_t)INT_MAX)
	{
		return false;
	}

	*fd = (int)handle;
	return *fd >= 0;
}

static bool esp32_set_nonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	return flags >= 0 && fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0;
}

static int esp32_find_listener_fd(int fd)
{
	for (uint16_t i = 0; i < ESP32_MAX_LISTENERS; ++i)
	{
		if (esp32_listeners[i] == fd)
		{
			return (int)i;
		}
	}
	return -1;
}

static int esp32_find_free_listener(void)
{
	for (uint16_t i = 0; i < ESP32_MAX_LISTENERS; ++i)
	{
		if (esp32_listeners[i] < 0)
		{
			return (int)i;
		}
	}
	return -1;
}

static int esp32_find_client_fd(int fd)
{
	for (uint16_t i = 0; i < ESP32_MAX_CLIENTS; ++i)
	{
		if ((esp32_clients[i].flags & ESP32_CLIENT_IN_USE) != 0U &&
			esp32_clients[i].fd == fd)
		{
			return (int)i;
		}
	}
	return -1;
}

static int esp32_find_free_client(void)
{
	for (uint16_t i = 0; i < ESP32_MAX_CLIENTS; ++i)
	{
		if ((esp32_clients[i].flags & ESP32_CLIENT_IN_USE) == 0U)
		{
			return (int)i;
		}
	}
	return -1;
}

/* Invalidate the record before native close so a reused fd cannot revive it. */
static void esp32_release_client(int index, bool close_native)
{
	esp32_client_t *client;
	int fd;

	if (index < 0 || index >= (int)ESP32_MAX_CLIENTS)
	{
		return;
	}

	client = &esp32_clients[index];
	if ((client->flags & ESP32_CLIENT_IN_USE) == 0U)
	{
		return;
	}

	fd = client->fd;
	client->fd = -1;
	client->token = SOCKET_DEVICE_INVALID_TOKEN;
	client->pending_error = 0;
	client->flags = 0U;

	if (close_native && fd >= 0)
	{
		(void)lwip_close(fd);
	}
}

/*
 * Remote/fatal close path. The native state is gone before closed() is emitted;
 * the copied token is the last reference to this connection generation.
 */
static int esp32_fail_client(int index, int reason)
{
	socket_device_token_t token;

	if (index < 0 || index >= (int)ESP32_MAX_CLIENTS ||
		(esp32_clients[index].flags & ESP32_CLIENT_IN_USE) == 0U)
	{
		return SOCKET_DEVICE_INVALID;
	}

	if (reason >= 0 || reason == SOCKET_DEVICE_WOULD_BLOCK)
	{
		reason = SOCKET_DEVICE_ERROR;
	}

	token = esp32_clients[index].token;
	esp32_release_client(index, true);

	if (token != SOCKET_DEVICE_INVALID_TOKEN)
	{
		esp32_events->closed(token, reason);
	}

	return reason;
}

static int esp32_socket_init(const socket_device_events_t *events)
{
	if (!events || !events->accepted || !events->readable ||
		!events->writable || !events->closed)
	{
		return SOCKET_DEVICE_INVALID;
	}

	for (uint16_t i = 0; i < ESP32_MAX_LISTENERS; ++i)
	{
		esp32_listeners[i] = -1;
	}

	memset(esp32_clients, 0, sizeof(esp32_clients));
	for (uint16_t i = 0; i < ESP32_MAX_CLIENTS; ++i)
	{
		esp32_clients[i].fd = -1;
		esp32_clients[i].token = SOCKET_DEVICE_INVALID_TOKEN;
	}

	esp32_next_listener = 0U;
	esp32_next_client = 0U;
	esp32_prefer_accept = true;
	esp32_events = events;

	/* Wi-Fi/netif startup remains owned by the existing ESP32 network module. */
	return SOCKET_DEVICE_OK;
}

static socket_device_handle_t esp32_socket_listen(
	const socket_device_endpoint_t *endpoint,
	uint8_t backlog)
{
	struct sockaddr_in address;
	int listener_index;
	int native_backlog;
	int fd;

	if (!endpoint || endpoint->port == 0U || !esp32_events)
	{
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	listener_index = esp32_find_free_listener();
	if (listener_index < 0)
	{
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	fd = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (!esp32_fd_is_selectable(fd))
	{
		if (fd >= 0)
		{
			(void)lwip_close(fd);
		}
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	if (!esp32_set_nonblocking(fd))
	{
		(void)lwip_close(fd);
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

#ifdef SO_REUSEADDR
	/* Best effort: CONFIG_LWIP_SO_REUSE controls availability in ESP-IDF. */
	{
		int reuse = 1;
		(void)lwip_setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
						   &reuse, (socklen_t)sizeof(reuse));
	}
#endif

	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(endpoint->port);
	address.sin_addr.s_addr = htonl(endpoint->address);

	if (lwip_bind(fd, (struct sockaddr *)&address,
			  (socklen_t)sizeof(address)) < 0)
	{
		(void)lwip_close(fd);
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	/* Zero maps to the smallest bounded queue, never to an unbounded request. */
	native_backlog = backlog > 0U ? (int)backlog : 1;
#ifdef SOMAXCONN
	if (SOMAXCONN > 0 && native_backlog > SOMAXCONN)
	{
		native_backlog = SOMAXCONN;
	}
#endif

	if (lwip_listen(fd, native_backlog) < 0)
	{
		(void)lwip_close(fd);
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	esp32_listeners[listener_index] = fd;
	return (socket_device_handle_t)fd;
}

static int esp32_socket_recv(socket_device_handle_t handle,
							 void *destination,
							 size_t capacity)
{
	esp32_client_t *client;
	size_t attempt;
	ssize_t received;
	int index;
	int fd;

	if (capacity == 0U)
	{
		return 0;
	}
	if (!destination || !esp32_handle_to_fd(handle, &fd))
	{
		return SOCKET_DEVICE_INVALID;
	}

	index = esp32_find_client_fd(fd);
	if (index < 0)
	{
		return SOCKET_DEVICE_INVALID;
	}
	client = &esp32_clients[index];

	/* The positive public result is int, even when size_t is wider. */
	attempt = capacity > (size_t)INT_MAX ? (size_t)INT_MAX : capacity;
	received = lwip_recv(fd, destination, attempt, 0);

	if (received > 0)
	{
		/* Keep READ_NOTIFIED set: the core drains again after a positive read. */
		return (int)received;
	}

	if (received == 0)
	{
		int reason = client->pending_error != 0
					 ? (int)client->pending_error
					 : SOCKET_DEVICE_CLOSED;
		return esp32_fail_client(index, reason);
	}

	if (esp32_errno_is_data_backpressure(errno))
	{
		/*
		 * SO_ERROR may have been consumed in poll(). Drain any native buffered
		 * payload first; once none remains, complete the deferred fatal close.
		 */
		if (client->pending_error != 0)
		{
			return esp32_fail_client(index, (int)client->pending_error);
		}

		client->flags &= (uint8_t)~ESP32_CLIENT_READ_NOTIFIED;
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	return esp32_fail_client(index, esp32_map_errno(errno));
}

static int esp32_socket_send(socket_device_handle_t handle,
							 const void *source,
							 size_t length)
{
	esp32_client_t *client;
	size_t attempt;
	ssize_t sent;
	int index;
	int fd;
	int reason;

	if (length == 0U)
	{
		return 0;
	}
	if (!source || !esp32_handle_to_fd(handle, &fd))
	{
		return SOCKET_DEVICE_INVALID;
	}

	index = esp32_find_client_fd(fd);
	if (index < 0)
	{
		return SOCKET_DEVICE_INVALID;
	}
	client = &esp32_clients[index];

	/* A select exception is drained through recv() to preserve final RX bytes. */
	if (client->pending_error != 0)
	{
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	attempt = length > (size_t)INT_MAX ? (size_t)INT_MAX : length;
	sent = lwip_send(fd, source, attempt, 0);

	if (sent > 0)
	{
		if ((size_t)sent < length)
		{
			client->flags |= ESP32_CLIENT_WANT_WRITE;
		}
		/*
		 * Do not clear an already armed WANT_WRITE after an unrelated later
		 * send happens to complete. A previous partial/blocked attempt may still
		 * have an application-owned suffix waiting for its writable transition.
		 * Only poll(), when it actually emits writable(), consumes this interest.
		 */
		return (int)sent;
	}

	if (sent < 0 && esp32_errno_is_data_backpressure(errno))
	{
		client->flags |= ESP32_CLIENT_WANT_WRITE;
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	reason = sent == 0 ? SOCKET_DEVICE_ERROR : esp32_map_errno(errno);

	/*
	 * If a readable condition is already outstanding, recv() may still hold
	 * final peer bytes. Defer the fatal close and let the core drain them first.
	 */
	if ((client->flags & ESP32_CLIENT_READ_NOTIFIED) != 0U)
	{
		client->pending_error = (int8_t)(reason < 0 ? reason : SOCKET_DEVICE_ERROR);
		client->flags &= (uint8_t)~ESP32_CLIENT_WANT_WRITE;
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	return esp32_fail_client(index, reason);
}

static int esp32_socket_close(socket_device_handle_t handle)
{
	int close_result;
	int index;
	int fd;

	if (!esp32_handle_to_fd(handle, &fd))
	{
		return SOCKET_DEVICE_INVALID;
	}

	index = esp32_find_listener_fd(fd);
	if (index >= 0)
	{
		/* Closing a listener does not implicitly close its accepted children. */
		esp32_listeners[index] = -1;
		close_result = lwip_close(fd);
		return close_result == 0 ? SOCKET_DEVICE_OK : esp32_map_errno(errno);
	}

	index = esp32_find_client_fd(fd);
	if (index >= 0)
	{
		/* Local closure invalidates first and never emits events->closed(). */
		esp32_release_client(index, false);
		close_result = lwip_close(fd);
		return close_result == 0 ? SOCKET_DEVICE_OK : esp32_map_errno(errno);
	}

	return SOCKET_DEVICE_INVALID;
}

/* Accepts at most one core-visible client and advances the listener cursor. */
static bool esp32_poll_one_accept(const fd_set *read_set)
{
	for (uint16_t checked = 0; checked < ESP32_MAX_LISTENERS; ++checked)
	{
		uint16_t index = esp32_next_listener;
		int listener_fd;
		int client_fd;
		int client_index;
		socket_device_token_t token;

		esp32_next_listener =
			(uint16_t)((esp32_next_listener + 1U) % ESP32_MAX_LISTENERS);
		listener_fd = esp32_listeners[index];
		if (listener_fd < 0 || !FD_ISSET(listener_fd, read_set))
		{
			continue;
		}

		client_fd = lwip_accept(listener_fd, NULL, NULL);
		if (client_fd < 0)
		{
			/* Readiness is only a hint; EAGAIN and resource pressure retry later. */
			continue;
		}

		if (!esp32_fd_is_selectable(client_fd) ||
			!esp32_set_nonblocking(client_fd))
		{
			(void)lwip_close(client_fd);
			continue;
		}

		client_index = esp32_find_free_client();
		if (client_index < 0)
		{
			(void)lwip_close(client_fd);
			continue;
		}

		esp32_clients[client_index].fd = client_fd;
		esp32_clients[client_index].token = SOCKET_DEVICE_INVALID_TOKEN;
		esp32_clients[client_index].pending_error = 0;
		esp32_clients[client_index].flags = ESP32_CLIENT_IN_USE;

		token = esp32_events->accepted((socket_device_handle_t)listener_fd,
									  (socket_device_handle_t)client_fd);
		if (token == SOCKET_DEVICE_INVALID_TOKEN)
		{
			/* The core rejected it; backend ownership ends with no close event. */
			esp32_release_client(client_index, true);
		}
		else
		{
			esp32_clients[client_index].token = token;
		}

		/* accepted() was invoked and therefore consumes one event-budget unit. */
		return true;
	}

	return false;
}

static int esp32_socket_error(int fd)
{
	int native_error = 0;
	socklen_t length = (socklen_t)sizeof(native_error);

	if (lwip_getsockopt(fd, SOL_SOCKET, SO_ERROR,
						&native_error, &length) < 0)
	{
		native_error = errno;
	}

	if (native_error == 0)
	{
		return SOCKET_DEVICE_ERROR;
	}
	return esp32_map_errno(native_error);
}

/* Emits at most one readiness event and advances the client cursor. */
static bool esp32_poll_one_client(const fd_set *read_set,
								  const fd_set *write_set,
								  const fd_set *error_set)
{
	for (uint16_t checked = 0; checked < ESP32_MAX_CLIENTS; ++checked)
	{
		uint16_t index = esp32_next_client;
		esp32_client_t *client;
		bool has_error;
		bool has_read;

		esp32_next_client =
			(uint16_t)((esp32_next_client + 1U) % ESP32_MAX_CLIENTS);
		client = &esp32_clients[index];
		if ((client->flags & ESP32_CLIENT_IN_USE) == 0U)
		{
			continue;
		}

		has_error = FD_ISSET(client->fd, error_set);
		has_read = FD_ISSET(client->fd, read_set);
		if (has_error && client->pending_error == 0)
		{
			int reason = esp32_socket_error(client->fd);
			if (reason == SOCKET_DEVICE_WOULD_BLOCK)
			{
				reason = SOCKET_DEVICE_ERROR;
			}
			client->pending_error = (int8_t)reason;
		}

		/* Error readiness is pulled through recv() so buffered final RX survives. */
		if ((has_read || has_error) &&
			(client->flags & ESP32_CLIENT_READ_NOTIFIED) == 0U)
		{
			socket_device_token_t token = client->token;
			client->flags |= ESP32_CLIENT_READ_NOTIFIED;
			esp32_events->readable(token);
			return true;
		}

		if (!has_error && client->pending_error == 0 &&
			(client->flags & ESP32_CLIENT_WANT_WRITE) != 0U &&
			FD_ISSET(client->fd, write_set))
		{
			socket_device_token_t token = client->token;
			client->flags &= (uint8_t)~ESP32_CLIENT_WANT_WRITE;
			esp32_events->writable(token);
			return true;
		}
	}

	return false;
}

static void esp32_socket_poll(uint16_t budget)
{
	fd_set read_set;
	fd_set write_set;
	fd_set error_set;
	struct timeval timeout;
	uint16_t emitted = 0U;
	int max_fd = -1;
	int ready;

	if (!esp32_events || budget == 0U)
	{
		return;
	}

	FD_ZERO(&read_set);
	FD_ZERO(&write_set);
	FD_ZERO(&error_set);

	for (uint16_t i = 0; i < ESP32_MAX_LISTENERS; ++i)
	{
		int fd = esp32_listeners[i];
		if (fd < 0)
		{
			continue;
		}
		FD_SET(fd, &read_set);
		if (fd > max_fd)
		{
			max_fd = fd;
		}
	}

	for (uint16_t i = 0; i < ESP32_MAX_CLIENTS; ++i)
	{
		esp32_client_t *client = &esp32_clients[i];
		if ((client->flags & ESP32_CLIENT_IN_USE) == 0U)
		{
			continue;
		}

		FD_SET(client->fd, &read_set);
		FD_SET(client->fd, &error_set);
		if ((client->flags & ESP32_CLIENT_WANT_WRITE) != 0U &&
			client->pending_error == 0)
		{
			FD_SET(client->fd, &write_set);
		}
		if (client->fd > max_fd)
		{
			max_fd = client->fd;
		}
	}

	if (max_fd < 0)
	{
		return;
	}

	/* A zero timeout is mandatory: poll() never waits for the network task. */
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	ready = lwip_select(max_fd + 1, &read_set, &write_set, &error_set, &timeout);
	if (ready <= 0)
	{
		/* EINTR and other select-level failures are retried by a later poll(). */
		return;
	}

	/* Alternate accept and client work so an accept flood cannot starve RX/TX. */
	while (emitted < budget)
	{
		bool did_emit;

		if (esp32_prefer_accept)
		{
			did_emit = esp32_poll_one_accept(&read_set);
			if (!did_emit)
			{
				did_emit = esp32_poll_one_client(&read_set, &write_set,
											 &error_set);
			}
		}
		else
		{
			did_emit = esp32_poll_one_client(&read_set, &write_set,
										 &error_set);
			if (!did_emit)
			{
				did_emit = esp32_poll_one_accept(&read_set);
			}
		}

		if (!did_emit)
		{
			break;
		}

		++emitted;
		esp32_prefer_accept = !esp32_prefer_accept;
	}
}

socket_device_t wifi_socket = {
	.init = esp32_socket_init,
	.listen = esp32_socket_listen,
	.recv = esp32_socket_recv,
	.send = esp32_socket_send,
	.close = esp32_socket_close,
	.poll = esp32_socket_poll};

#endif /* ENABLE_SOCKETS */
#endif /* ESP32 || ESP32S3 || ESP32C3 */
