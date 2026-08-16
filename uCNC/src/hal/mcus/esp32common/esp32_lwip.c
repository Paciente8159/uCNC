/*
	Name: esp32_lwip.c
	Description: Allocation-free ESP32/lwIP BSD-socket backend for uCNC.

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

#ifndef ESP32_MAX_CLIENTS
#define ESP32_MAX_CLIENTS SOCKET_MAX_CONNECTIONS
#endif

#if ESP32_MAX_LISTENERS == 0
#error "ESP32_MAX_LISTENERS must be greater than zero"
#endif

#if ESP32_MAX_CLIENTS == 0
#error "ESP32_MAX_CLIENTS must be greater than zero"
#endif

#if ESP32_MAX_LISTENERS > 65535U
#error "ESP32_MAX_LISTENERS must fit in uint16_t"
#endif

#if ESP32_MAX_CLIENTS > 65535U
#error "ESP32_MAX_CLIENTS must fit in uint16_t"
#endif

#define ESP32_CLIENT_BITSET_BYTES ((ESP32_MAX_CLIENTS + 7U) / 8U)

/*
 * ESP-IDF v4.4.x limits CONFIG_LWIP_MAX_SOCKETS to 16 in its stock Kconfig.
 * Native sockets include listeners and accepted clients. Therefore a build
 * configured for 4 listeners plus 16 simultaneous clients needs an SDK with a
 * raised native limit; otherwise socket()/accept() simply bound runtime
 * capacity. The backend tables deliberately follow uCNC's compile-time limits
 * rather than hiding a smaller target-specific client default.
 */

static int esp32_listeners[ESP32_MAX_LISTENERS];
static int esp32_client_fds[ESP32_MAX_CLIENTS];
static socket_device_token_t esp32_client_tokens[ESP32_MAX_CLIENTS];
static const socket_device_events_t *esp32_events;
static uint16_t esp32_next_listener;
static uint16_t esp32_next_client;
static uint8_t esp32_read_notified[ESP32_CLIENT_BITSET_BYTES];
static uint8_t esp32_error_hint[ESP32_CLIENT_BITSET_BYTES];
static uint8_t esp32_prefer_accept;

static bool esp32_bit_test(const uint8_t *bits, uint16_t index)
{
	uint8_t mask = (uint8_t)(1U << (index & 7U));
	return (bits[index >> 3] & mask) != 0U;
}

static void esp32_bit_set(uint8_t *bits, uint16_t index)
{
	uint8_t mask = (uint8_t)(1U << (index & 7U));
	bits[index >> 3] |= mask;
}

static void esp32_bit_clear(uint8_t *bits, uint16_t index)
{
	uint8_t mask = (uint8_t)(1U << (index & 7U));
	bits[index >> 3] &= (uint8_t)~mask;
}

/* EAGAIN and EWOULDBLOCK are often the same value, so do not use switch. */
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
	/* One backend call is one native attempt; the core may retry later. */
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

/* lwIP can use memory/buffer exhaustion as temporary socket data pressure. */
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

/* Mapping for control operations where resource exhaustion is not data flow. */
static int esp32_map_control_errno(int error)
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

/*
 * Data-path fatal errors deliberately do not return SOCKET_DEVICE_TIMEOUT:
 * that code belongs to the generic core's bounded blocking-send deadline.
 * Orderly FIN is recognized separately from recv() == 0.
 */
static int esp32_map_fatal_errno(int error)
{
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
	uint16_t i;
	for (i = 0U; i < (uint16_t)ESP32_MAX_LISTENERS; ++i)
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
	uint16_t i;
	for (i = 0U; i < (uint16_t)ESP32_MAX_LISTENERS; ++i)
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
	uint16_t i;
	for (i = 0U; i < (uint16_t)ESP32_MAX_CLIENTS; ++i)
	{
		if (esp32_client_fds[i] == fd)
		{
			return (int)i;
		}
	}
	return -1;
}

static int esp32_find_free_client(void)
{
	uint16_t i;
	for (i = 0U; i < (uint16_t)ESP32_MAX_CLIENTS; ++i)
	{
		if (esp32_client_fds[i] < 0)
		{
			return (int)i;
		}
	}
	return -1;
}

/*
 * Clear backend identity before native close. There are no asynchronous
 * backend callbacks, so a later descriptor reuse can only be observed after a
 * new accepted() call installs a fresh core generation token.
 */
static void esp32_release_client(int index, bool close_native)
{
	int fd;

	if (index < 0 || index >= (int)ESP32_MAX_CLIENTS ||
		esp32_client_fds[index] < 0)
	{
		return;
	}

	fd = esp32_client_fds[index];
	esp32_client_fds[index] = -1;
	esp32_client_tokens[index] = SOCKET_DEVICE_INVALID_TOKEN;
	esp32_bit_clear(esp32_read_notified, (uint16_t)index);
	esp32_bit_clear(esp32_error_hint, (uint16_t)index);

	if (close_native && fd >= 0)
	{
		(void)lwip_close(fd);
	}
}

/* Remote/fatal client closure. Local close() never uses this helper. */
static int esp32_fail_client(int index, int reason)
{
	socket_device_token_t token;

	if (index < 0 || index >= (int)ESP32_MAX_CLIENTS ||
		esp32_client_fds[index] < 0)
	{
		return SOCKET_DEVICE_INVALID;
	}

	if (reason != SOCKET_DEVICE_CLOSED &&
		reason != SOCKET_DEVICE_ERROR &&
		reason != SOCKET_DEVICE_INVALID)
	{
		reason = SOCKET_DEVICE_ERROR;
	}

	token = esp32_client_tokens[index];
	esp32_release_client(index, true);

	if (token != SOCKET_DEVICE_INVALID_TOKEN)
	{
		esp32_events->closed(token, reason);
	}

	return reason;
}

/*
 * select() exception readiness is only a hint. ESP-IDF documents SO_ERROR as
 * the way to obtain its reason. We intentionally defer this read until recv()
 * has first had a chance to return all already-buffered payload, preserving the
 * final-data-before-close ordering required by the uCNC core.
 */
static int esp32_consume_error_hint(int index)
{
	int fd;
	int native_error = 0;
	socklen_t length = (socklen_t)sizeof(native_error);
	int mapped;

	if (index < 0 || index >= (int)ESP32_MAX_CLIENTS ||
		esp32_client_fds[index] < 0)
	{
		return SOCKET_DEVICE_INVALID;
	}

	fd = esp32_client_fds[index];
	if (lwip_getsockopt(fd, SOL_SOCKET, SO_ERROR,
						&native_error, &length) < 0)
	{
		int error = errno;
		if (esp32_errno_is_data_backpressure(error))
		{
			return SOCKET_DEVICE_WOULD_BLOCK;
		}
		return esp32_map_fatal_errno(error);
	}

	/* SO_ERROR is consumed by getsockopt(); the hint must not survive it. */
	esp32_bit_clear(esp32_error_hint, (uint16_t)index);
	if (native_error == 0)
	{
		return SOCKET_DEVICE_WOULD_BLOCK;
	}
	if (esp32_errno_is_data_backpressure(native_error))
	{
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	mapped = esp32_map_fatal_errno(native_error);
	return mapped;
}

static int esp32_socket_init(const socket_device_events_t *events)
{
	uint16_t i;

	if (!events || !events->accepted || !events->readable || !events->closed)
	{
		return SOCKET_DEVICE_INVALID;
	}

	for (i = 0U; i < (uint16_t)ESP32_MAX_LISTENERS; ++i)
	{
		esp32_listeners[i] = -1;
	}
	for (i = 0U; i < (uint16_t)ESP32_MAX_CLIENTS; ++i)
	{
		esp32_client_fds[i] = -1;
		esp32_client_tokens[i] = SOCKET_DEVICE_INVALID_TOKEN;
	}
	memset(esp32_read_notified, 0, sizeof(esp32_read_notified));
	memset(esp32_error_hint, 0, sizeof(esp32_error_hint));

	esp32_next_listener = 0U;
	esp32_next_client = 0U;
	esp32_prefer_accept = 1U;
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
	/* Best effort; availability is controlled by CONFIG_LWIP_SO_REUSE. */
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

	/* Zero is mapped to one pending connection, never an unbounded request. */
	native_backlog = backlog > 0U ? (int)backlog : 1;
	/* lwIP's TCP listen backlog is an 8-bit quantity. */
	if (native_backlog > 255)
	{
		native_backlog = 255;
	}
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

	/* Positive backend results are int byte counts. */
	attempt = capacity > (size_t)INT_MAX ? (size_t)INT_MAX : capacity;

	/*
	 * O_NONBLOCK is set on every client. Do not loop here: one recv() call is
	 * the complete backend attempt. lwIP retains any unread suffix internally.
	 */
	received = lwip_recv(fd, destination, attempt, 0);
	if (received > 0)
	{
		/* Core keeps READABLE set after a positive read and will drain again. */
		return (int)received;
	}

	if (received == 0)
	{
		return esp32_fail_client(index, SOCKET_DEVICE_CLOSED);
	}

	if (esp32_errno_is_data_backpressure(errno))
	{
		if (esp32_bit_test(esp32_error_hint, (uint16_t)index))
		{
			int reason = esp32_consume_error_hint(index);
			if (reason != SOCKET_DEVICE_WOULD_BLOCK)
			{
				return esp32_fail_client(index, reason);
			}
		}

		/* Core has now observed the hint as empty; re-arm on new readiness. */
		esp32_bit_clear(esp32_read_notified, (uint16_t)index);
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	return esp32_fail_client(index, esp32_map_fatal_errno(errno));
}

static int esp32_socket_send(socket_device_handle_t handle,
							 const void *source,
							 size_t length)
{
	size_t attempt;
	ssize_t sent;
	int index;
	int fd;

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

	attempt = length > (size_t)INT_MAX ? (size_t)INT_MAX : length;

	/*
	 * Exactly one native send attempt. Every accepted client is O_NONBLOCK, so
	 * ESP-IDF/lwIP returns immediately with a full/partial byte count or an
	 * errno. The BSD socket layer copies accepted TCP payload into stack-owned
	 * storage; this backend never retains source or a continuation offset.
	 *
	 * No write fd-set or writable event is needed. The lwIP TCP/IP task advances
	 * ACK/TX state independently; a blocking generic socket_send() calls poll()
	 * and retries this function, while a non-blocking caller owns continuation.
	 */
	sent = lwip_send(fd, source, attempt, 0);
	if (sent > 0)
	{
		return (int)sent;
	}

	if (sent < 0 && esp32_errno_is_data_backpressure(errno))
	{
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	return esp32_fail_client(index,
						 sent == 0 ? SOCKET_DEVICE_ERROR
								   : esp32_map_fatal_errno(errno));
}

static int esp32_socket_close(socket_device_handle_t handle)
{
	int close_result;
	int mapped;
	int index;
	int fd;

	if (!esp32_handle_to_fd(handle, &fd))
	{
		return SOCKET_DEVICE_INVALID;
	}

	index = esp32_find_listener_fd(fd);
	if (index >= 0)
	{
		/* Listener children are owned/closed separately by the generic core. */
		esp32_listeners[index] = -1;
		close_result = lwip_close(fd);
		if (close_result == 0)
		{
			return SOCKET_DEVICE_OK;
		}
		mapped = esp32_map_control_errno(errno);
		/* The handle is stale regardless; do not advertise a retryable close. */
		return mapped == SOCKET_DEVICE_INVALID ? mapped : SOCKET_DEVICE_ERROR;
	}

	index = esp32_find_client_fd(fd);
	if (index >= 0)
	{
		/* Invalidate first. Local closure never emits events->closed(). */
		esp32_release_client(index, false);
		close_result = lwip_close(fd);
		if (close_result == 0)
		{
			return SOCKET_DEVICE_OK;
		}
		mapped = esp32_map_control_errno(errno);
		return mapped == SOCKET_DEVICE_INVALID ? mapped : SOCKET_DEVICE_ERROR;
	}

	return SOCKET_DEVICE_INVALID;
}

/* Accepts at most one core-visible client and advances the listener cursor. */
static bool esp32_poll_one_accept(const fd_set *read_set)
{
	uint16_t checked;

	/* Do not drain native accepts if there is no fixed backend client record. */
	if (esp32_find_free_client() < 0)
	{
		return false;
	}

	for (checked = 0U; checked < (uint16_t)ESP32_MAX_LISTENERS; ++checked)
	{
		uint16_t listener_index = esp32_next_listener;
		int listener_fd;
		int client_fd;
		int client_index;
		socket_device_token_t token;

		esp32_next_listener = (uint16_t)(
			(esp32_next_listener + 1U) % (uint16_t)ESP32_MAX_LISTENERS);
		listener_fd = esp32_listeners[listener_index];
		if (listener_fd < 0 || !FD_ISSET(listener_fd, read_set))
		{
			continue;
		}

		/* Listener is O_NONBLOCK; accept never waits for a connection. */
		client_fd = lwip_accept(listener_fd, NULL, NULL);
		if (client_fd < 0)
		{
			/* Readiness is a hint; a later poll retries temporary/resource errors. */
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
			/* A previous owner-context event cannot allocate here, but be safe. */
			(void)lwip_close(client_fd);
			return false;
		}

		esp32_client_fds[client_index] = client_fd;
		esp32_client_tokens[client_index] = SOCKET_DEVICE_INVALID_TOKEN;
		esp32_bit_clear(esp32_read_notified, (uint16_t)client_index);
		esp32_bit_clear(esp32_error_hint, (uint16_t)client_index);

		/*
		 * This is the first and only uCNC-visible event for the new descriptor.
		 * The select snapshot was built before accept(), so RX/close cannot be
		 * emitted for this descriptor until a later poll after its token is stored.
		 */
		token = esp32_events->accepted((socket_device_handle_t)listener_fd,
									  (socket_device_handle_t)client_fd);
		if (token == SOCKET_DEVICE_INVALID_TOKEN)
		{
			/* Core rejected it; release immediately and never emit closed(). */
			esp32_release_client(client_index, true);
		}
		else
		{
			esp32_client_tokens[client_index] = token;
		}

		/* accepted() invocation consumes one normalized event-budget unit. */
		return true;
	}

	return false;
}

/* Emits at most one readable hint and advances the client cursor. */
static bool esp32_poll_one_client(const fd_set *read_set,
								  const fd_set *error_set)
{
	uint16_t checked;

	for (checked = 0U; checked < (uint16_t)ESP32_MAX_CLIENTS; ++checked)
	{
		uint16_t index = esp32_next_client;
		int fd;
		bool has_read;
		bool has_error;

		esp32_next_client = (uint16_t)(
			(esp32_next_client + 1U) % (uint16_t)ESP32_MAX_CLIENTS);
		fd = esp32_client_fds[index];
		if (fd < 0)
		{
			continue;
		}

		has_read = FD_ISSET(fd, read_set);
		has_error = FD_ISSET(fd, error_set);
		if (has_error)
		{
			/* Defer SO_ERROR until recv() has drained any final queued bytes. */
			esp32_bit_set(esp32_error_hint, index);
		}

		if ((has_read || has_error) &&
			!esp32_bit_test(esp32_read_notified, index))
		{
			socket_device_token_t token = esp32_client_tokens[index];
			if (token == SOCKET_DEVICE_INVALID_TOKEN)
			{
				/* Should only be visible transiently inside accepted(). */
				continue;
			}

			esp32_bit_set(esp32_read_notified, index);
			esp32_events->readable(token);
			return true;
		}
	}

	return false;
}

static void esp32_socket_poll(uint16_t budget)
{
	fd_set read_set;
	fd_set error_set;
	struct timeval timeout;
	uint16_t emitted = 0U;
	int max_fd = -1;
	int ready;
	uint16_t i;

	if (!esp32_events || budget == 0U)
	{
		return;
	}

	FD_ZERO(&read_set);
	FD_ZERO(&error_set);

	for (i = 0U; i < (uint16_t)ESP32_MAX_LISTENERS; ++i)
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

	for (i = 0U; i < (uint16_t)ESP32_MAX_CLIENTS; ++i)
	{
		int fd = esp32_client_fds[i];
		if (fd < 0)
		{
			continue;
		}

		/*
		 * Once readable() is outstanding the generic core owns the drain loop,
		 * so another read readiness sample is unnecessary until recv() reports
		 * WOULD_BLOCK. Exception readiness is still watched for close/error state.
		 */
		if (!esp32_bit_test(esp32_read_notified, i))
		{
			FD_SET(fd, &read_set);
		}
		FD_SET(fd, &error_set);
		if (fd > max_fd)
		{
			max_fd = fd;
		}
	}

	if (max_fd < 0)
	{
		return;
	}

	/* Zero timeout is mandatory: backend poll() never waits for network I/O. */
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	ready = lwip_select(max_fd + 1, &read_set, NULL, &error_set, &timeout);
	if (ready <= 0)
	{
		/* EINTR/select-level errors are left for a bounded later poll pass. */
		return;
	}

	/*
	 * There is intentionally no writable fd-set. lwIP's own TCP/IP task advances
	 * TX/ACK state; repeated blocking-core send attempts observe that progress by
	 * retrying send() after these bounded, zero-timeout service passes.
	 *
	 * Alternate accept and client events so an accept flood cannot starve RX.
	 */
	while (emitted < budget)
	{
		bool did_emit;

		if (esp32_prefer_accept != 0U)
		{
			did_emit = esp32_poll_one_accept(&read_set);
			if (!did_emit)
			{
				did_emit = esp32_poll_one_client(&read_set, &error_set);
			}
		}
		else
		{
			did_emit = esp32_poll_one_client(&read_set, &error_set);
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
		esp32_prefer_accept ^= 1U;
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
