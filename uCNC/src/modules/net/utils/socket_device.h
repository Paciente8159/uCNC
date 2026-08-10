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
		/* Called when a new client connection has been established.
		   Returns true if the socket core accepted the client, false if no
		   client slot is available (the backend must then close/discard it). */
		bool (*connected)(socket_handle_t listener, socket_handle_t client);

		/* Called when TCP payload is available. data[len] must be writable
		   and must contain '\0'. The buffer is only valid during the callback. */
		void (*data)(socket_handle_t client, char *data, size_t len);

		/* Called after a previous send encountered backpressure and the
		   transport has become writable again. May be a reserved/no-op for
		   now but must exist for TX-progress mapping (ex: lwIP tcp_sent). */
		void (*writable)(socket_handle_t client);

		/* Called after a remote orderly close or fatal transport error.
		   reason == 0 means an orderly remote close. The backend must have
		   released its native transport resources before calling this. */
		void (*disconnected)(socket_handle_t client, int reason);
	} socket_device_events_t;

	/* Event-driven backend interface. */
	typedef struct socket_device_
	{
		/* Called once when the device is registered. Must store the event
		   callback table and initialize the backend. Returns 0 on success,
		   a negative value on failure. */
		int (*init)(const socket_device_events_t *events);

		/* Performs socket + bind + configure non-blocking + listen and
		   registers the listener in backend state. Returns a valid handle
		   or SOCKET_INVALID_HANDLE on failure. */
		socket_handle_t (*listen)(uint32_t ip_listen, uint16_t port, int domain, int type, int protocol, uint8_t backlog);

		/* Strictly non-blocking single send attempt. Returns the number of
		   bytes accepted (> 0), SOCKET_DEVICE_WOULD_BLOCK on TX backpressure
		   or another negative socket_device_result_t on error. A partial
		   positive send is valid. */
		int (*send)(socket_handle_t client, const void *data, size_t len, int flags);

		/* Closes a listener or client handle. Must NOT generate a
		   disconnected event (the core handles local close notifications). */
		int (*close)(socket_handle_t handle);

		/*
		 * Non-blocking and bounded service poll.
		 *
		 * Must never wait for network activity and must not indefinitely
		 * drain pending network work. Each invocation must perform a
		 * bounded amount of processing before returning.
		 */
		void (*service)(void);
	} socket_device_t;

#ifdef __cplusplus
}
#endif

#endif
