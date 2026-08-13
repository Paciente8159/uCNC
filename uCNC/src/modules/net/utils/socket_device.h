/*
	Name: socket_device.h
	Description: Generic event-driven socket backend interface for µCNC.

	This header defines the contract between the generic µCNC socket core
	(socket.c) and the platform/network backends. Backends push network
	events (connected, data, writable, disconnected) into the socket core
	through a callback table supplied at registration time.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 08-07-2026

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef SOCKET_DEVICE_H
#define SOCKET_DEVICE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Backend fallback constants (µCNC protocol code still uses these) */
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

	/* Generic socket handle. Pointer-width so it can hold both small embedded
	   identifiers and native Windows SOCKET values without truncation. */
	typedef uintptr_t socket_handle_t;

#define SOCKET_INVALID_HANDLE ((socket_handle_t)UINTPTR_MAX)

	/* Backend result codes. Semantics must remain distinct:
	   WOULD_BLOCK != CLOSED != GENERIC ERROR */
	typedef enum
	{
		SOCKET_DEVICE_OK = 0,
		SOCKET_DEVICE_ERROR = -1,
		SOCKET_DEVICE_WOULD_BLOCK = -2,
		SOCKET_DEVICE_CLOSED = -3,
		SOCKET_DEVICE_INVALID = -4,
		SOCKET_DEVICE_NO_MEMORY = -5
	} socket_device_result_t;

	/* Event sink supplied by the socket core to the backend. */
	typedef struct socket_device_events_
	{
		/*
		 * Called when a client has been accepted.
		 *
		 * Returns false when the core cannot accept it. The backend must then close
		 * the client without generating disconnected().
		 */
		bool (*connected)(socket_handle_t listener, socket_handle_t client);

		/*
		 * Delivers received TCP payload.
		 *
		 * data[len] must be writable and contain '\0'. The buffer is valid only
		 * while this callback is executing.
		 *
		 * Returns true when the complete payload was consumed by the core.
		 *
		 * Returns false when delivery is temporarily refused because the callback
		 * for that client is already active. In that case, the backend must:
		 *
		 * - not discard the payload;
		 * - not acknowledge it as consumed;
		 * - retain it or leave it queued in the native transport;
		 * - offer the same bytes again during a later service pass.
		 */
		bool (*data)(socket_handle_t client, char *data, size_t len);

		/*
		 * Called after send() previously returned WOULD_BLOCK and the connection may
		 * accept data again. This is a notification, not a guarantee that an
		 * arbitrarily large send will succeed.
		 */
		void (*writable)(socket_handle_t client);

		/*
		 * Called after remote closure or a fatal transport error.
		 *
		 * The backend must invalidate the handle and release its native resources
		 * before invoking this callback. reason == SOCKET_DEVICE_OK indicates an
		 * orderly remote close.
		 */
		void (*disconnected)(socket_handle_t client, int reason);
	} socket_device_events_t;

	/* Event-driven backend interface. */
	typedef struct socket_device_
	{
		/*
		 * Initializes the backend and stores the event callback table.
		 *
		 * The events pointer remains valid for the lifetime of the registered
		 * device and must not be modified by the backend.
		 *
		 * Returns SOCKET_DEVICE_OK on success or a negative
		 * socket_device_result_t on failure.
		 */
		int (*init)(const socket_device_events_t *events);

		/*
		 * Creates, binds and starts a non-blocking listener.
		 *
		 * Accepted clients must also be configured as non-blocking before the
		 * backend invokes events->connected().
		 *
		 * If events->connected() returns false, the backend must close and
		 * release that client without generating events->disconnected().
		 *
		 * Returns a stable listener handle on success or
		 * SOCKET_INVALID_HANDLE on failure.
		 */
		socket_handle_t (*listen)(
			uint32_t ip_listen,
			uint16_t port,
			int domain,
			int type,
			int protocol,
			uint8_t backlog);

		/*
		 * Performs one strictly non-blocking send attempt.
		 *
		 * For len > 0:
		 *
		 *   > 0
		 *       Number of bytes accepted by the transport. The result must never
		 *       exceed len. A partial positive result is valid. Accepted means
		 *       copied or retained by the transport, not received by the peer.
		 *
		 *   SOCKET_DEVICE_WOULD_BLOCK
		 *       No bytes were accepted because of temporary TX backpressure.
		 *       Retrying after backend servicing is valid.
		 *
		 *   SOCKET_DEVICE_CLOSED
		 *       The connection is no longer usable.
		 *
		 *   other negative result
		 *       A non-retryable error occurred and no bytes were accepted.
		 *
		 * For len == 0, returns 0 without changing the connection.
		 *
		 * The backend must not retain the caller's data pointer after returning.
		 * flags are passed through where supported by the native transport.
		 */
		int (*send)(
			socket_handle_t client,
			const void *data,
			size_t len,
			int flags);

		/*
		 * Closes and releases a listener or client handle.
		 *
		 * Local closure must not generate events->disconnected(), because the
		 * socket core performs the local disconnection notification.
		 *
		 * After this function returns, the handle is stale and all subsequent
		 * operations using it must fail.
		 *
		 * Returns SOCKET_DEVICE_OK on success or a negative
		 * socket_device_result_t on failure.
		 */
		int (*close)(socket_handle_t handle);

		/*
		 * Performs one non-blocking, bounded backend service step.
		 *
		 * It may invoke connected, data, writable and disconnected callbacks.
		 * It must never wait for network activity or indefinitely drain pending
		 * work.
		 *
		 * IMPORTANT:
		 *
		 * Application callbacks may call socket_send(). While handling TX
		 * backpressure, socket_send() may run the µCNC task loop, which may call
		 * service() recursively before the outer callback has returned.
		 *
		 * The backend must therefore tolerate nested service() calls:
		 *
		 * - It must not overwrite a buffer still exposed through data().
		 * - It must not redeliver an RX callback that the core reports as busy.
		 * - It may suppress nested RX and accept processing.
		 * - It should continue making TX and disconnection progress where the
		 *   architecture permits it.
		 * - It must not hold locks across callbacks that prevent nested servicing.
		 *
		 * Any event callback may synchronously close the current connection.
		 * After invoking a callback, the backend must verify that the connection
		 * still exists and still represents the same handle before accessing its
		 * state again.
		 */
		void (*service)(void);

	} socket_device_t;

#ifdef __cplusplus
}
#endif

#endif
