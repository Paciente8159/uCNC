/*
	Name: socket_device.h
	Description: Generic, allocation-free TCP transport backend interface for uCNC.

	This header defines the contract between the generic uCNC TCP stream core
	(socket.c) and a platform/network backend. A backend may be implemented using
	BSD sockets, lwIP raw callbacks, a WIZnet hardware TCP engine, an RTOS network
	stack, or another reliable byte-stream provider.

	This is deliberately a TCP stream interface. It is not a BSD socket API and
	it does not expose UDP or IP raw sockets. TCP sequencing, retransmission,
	congestion control, checksums and connection state are owned by the native
	TCP implementation beneath the backend.

	No function in this interface allocates memory. Backends must use static
	state, native stack-owned storage, or caller-provided storage only.

	Copyright: Copyright (c) Joao Martins
	Author: Joao Martins

	uCNC is free software: you can redistribute it and/or modify it under the
	terms of the GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later
	version. Please see <http://www.gnu.org/licenses/>.
*/

#ifndef SOCKET_DEVICE_H
#define SOCKET_DEVICE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

	/*
	 * Opaque native backend handle.
	 *
	 * Pointer width is used so the type can hold a small hardware socket number,
	 * a POSIX file descriptor, a Windows SOCKET, or a native control-block
	 * pointer without truncation. Only the backend interprets this value.
	 */
	typedef uintptr_t socket_device_handle_t;

#define SOCKET_DEVICE_INVALID_HANDLE ((socket_device_handle_t)UINTPTR_MAX)

	/*
	 * Core-generated connection token carried by backend events.
	 *
	 * The token contains a static connection-pool index and a generation value.
	 * It prevents a late event for a closed native handle from being delivered to
	 * a new connection that happens to reuse the same native handle.
	 *
	 * Backends must treat this as an opaque value. A backend receives a token from
	 * events->accepted() and must store it until that native client is closed.
	 */
	typedef uint32_t socket_device_token_t;

#define SOCKET_DEVICE_INVALID_TOKEN ((socket_device_token_t)0U)

	/*
	 * Normalized results returned by the backend and by the public socket core.
	 *
	 * Positive values are reserved for byte counts from recv() and send(). Zero is
	 * a successful result only for operations whose contract explicitly permits
	 * it. Negative values have the following stable meanings.
	 */
	typedef enum socket_device_result_
	{
		SOCKET_DEVICE_OK = 0,
		/* Unspecified, non-retryable backend or transport failure. */
		SOCKET_DEVICE_ERROR = -1,
		/* Temporary RX/TX backpressure. Retrying after poll() is valid. */
		SOCKET_DEVICE_WOULD_BLOCK = -2,
		/* The TCP connection is no longer usable. */
		SOCKET_DEVICE_CLOSED = -3,
		/* A pointer, token, handle, state, argument, or operation is invalid. */
		SOCKET_DEVICE_INVALID = -4,
		/* A fixed backend/core pool or queue has no free capacity. */
		SOCKET_DEVICE_NO_MEMORY = -5,
		/* A bounded core operation expired before it could complete. */
		SOCKET_DEVICE_TIMEOUT = -6
	} socket_device_result_t;

	typedef union ipv4_address_
	{
		uint32_t ip;
		uint8_t octets[4];
	} ipv4_address_t;

	/* IPv4 TCP listening endpoint. Address and port use host byte order. */
	typedef struct socket_device_endpoint_
	{
		/* IPv4 address in host byte order. Zero means all local interfaces. */
		uint32_t address;
		/* TCP port in host byte order. */
		uint16_t port;
	} socket_device_endpoint_t;

	/*
	 * Transport event sink supplied by the socket core to the backend.
	 *
	 * OWNER-CONTEXT RULE
	 * ------------------
	 * These functions may be called only from the same owner context that calls
	 * socket_server_poll()/socket_server_dotasks(), normally from inside
	 * backend->poll(). They must not
	 * be called directly from an ISR, a second core, or another RTOS task.
	 *
	 * A backend driven asynchronously must place compact native events in a
	 * bounded static queue or retain native readiness state, then emit normalized
	 * events from poll(). This rule makes the socket core deterministic without
	 * locks and prevents application callbacks from executing in network/ISR
	 * context.
	 *
	 * Event functions never invoke application callbacks synchronously. They only
	 * update fixed core state. Application callbacks are dispatched later by the
	 * socket core after backend->poll() returns.
	 */
	typedef struct socket_device_events_
	{
		/*
		 * Registers a newly accepted native TCP client with the core.
		 *
		 * listener is the exact native listener handle previously returned by
		 * backend->listen(). client is a new, usable native client handle.
		 *
		 * On success, returns a non-zero generation-tagged token. The backend must
		 * store that token and use it for every later readable() and closed()
		 * event for this client.
		 *
		 * SOCKET_DEVICE_INVALID_TOKEN means that no core listener/client slot is
		 * available, the listener is stopping, or an argument is invalid. In that
		 * case the backend still owns client and must close/release it immediately.
		 * It must not emit closed() for a client that was rejected here.
		 *
		 * This function does not take ownership of the native handle. Ownership
		 * remains with the backend until local close() or remote/fatal closure.
		 */
		socket_device_token_t (*accepted)(socket_device_handle_t listener,
										  socket_device_handle_t client);

		/*
		 * Reports that recv() may be able to return TCP payload for token.
		 *
		 * This is a readiness hint, not a promise. recv() may still return
		 * SOCKET_DEVICE_WOULD_BLOCK. Duplicate/coalesced hints are permitted. The
		 * core drains data in bounded chunks and retains the hint until recv()
		 * reports WOULD_BLOCK.
		 *
		 * The backend must retain RX bytes in its native transport or backend state
		 * until recv() consumes them. It must not acknowledge/discard TCP payload
		 * merely because this function returned.
		 */
		void (*readable)(socket_device_token_t token);


		/*
		 * Reports orderly remote closure or a fatal transport error.
		 *
		 * Before calling this function the backend must invalidate the native
		 * client and release all backend-owned resources for it. The token becomes
		 * stale immediately after this call and must never be emitted again.
		 *
		 * reason must be SOCKET_DEVICE_CLOSED for an orderly remote close, or a
		 * negative socket_device_result_t describing a fatal failure. Do not use
		 * SOCKET_DEVICE_OK for a remote close.
		 *
		 * Local backend->close() must never generate this event.
		 */
		void (*closed)(socket_device_token_t token, int reason);
	} socket_device_events_t;

	/*
	 * Allocation-free TCP backend interface.
	 *
	 * All functions execute in the socket owner's context unless a function's
	 * comment explicitly describes internal asynchronous activity. Functions
	 * must not invoke application callbacks. Only the event sink above may be
	 * called, and only under its owner-context rules.
	 */
	typedef struct socket_device_
	{
		/*
		 * Initializes the backend and stores the event sink.
		 *
		 * Called exactly once when this device is registered. events and every
		 * function pointer inside it remain valid for the registered lifetime.
		 * The backend must not modify the table.
		 *
		 * The backend must initialize all static state but must not emit transport
		 * events before this function returns.
		 *
		 * Returns SOCKET_DEVICE_OK on success, otherwise a negative
		 * socket_device_result_t. Registration fails on any negative result.
		 */
		int (*init)(const socket_device_events_t *events);

		/*
		 * Creates, binds and starts a non-blocking IPv4 TCP listener.
		 *
		 * endpoint fields are in host byte order. backlog is the desired pending
		 * connection limit; a backend may clamp it to a native/hardware maximum but
		 * must never treat zero as an unbounded allocation request.
		 *
		 * Every accepted native client must be configured for strictly non-blocking
		 * operation before it is passed to events->accepted().
		 *
		 * Returns a stable native listener handle on success or
		 * SOCKET_DEVICE_INVALID_HANDLE on any failure. No event is emitted for a
		 * listener whose creation failed.
		 */
		socket_device_handle_t (*listen)(const socket_device_endpoint_t *endpoint,
										 uint8_t backlog);

		/*
		 * Performs one strictly non-blocking receive attempt.
		 *
		 * destination points to capacity writable bytes owned by the socket core.
		 * The backend must not retain destination after returning. TCP data is
		 * binary; no terminator is required or added.
		 *
		 * For capacity > 0, returns:
		 *   > 0  number of bytes copied, never greater than capacity;
		 *   SOCKET_DEVICE_WOULD_BLOCK when no payload is currently available;
		 *   SOCKET_DEVICE_CLOSED or another negative result on closure/failure.
		 *
		 * For capacity == 0, returns 0 without consuming transport data.
		 *
		 * If recv() discovers orderly closure or a fatal error, the backend must
		 * release/invalidate the client, call events->closed(token, reason) before
		 * returning, and then return SOCKET_DEVICE_CLOSED or the same fatal result.
		 * The backend therefore needs a native-handle-to-token association.
		 */
		int (*recv)(socket_device_handle_t client, void *destination, size_t capacity);

		/*
		 * Performs one strictly non-blocking send attempt.
		 *
		 * source points to length immutable bytes. The backend must not retain the
		 * pointer after returning; a positive result means those bytes were copied
		 * or otherwise accepted by the native TCP transport, not acknowledged by
		 * the remote peer.
		 *
		 * For length > 0, returns:
		 *   > 0  number of bytes accepted, never greater than length;
		 *   SOCKET_DEVICE_WOULD_BLOCK when zero bytes were accepted temporarily;
		 *   SOCKET_DEVICE_CLOSED or another negative result on closure/failure.
		 * A partial positive result is valid and must not be converted to an error.
		 *
		 * For length == 0, returns 0 without changing connection state.
		 *
		 * If send() discovers closure/failure, it follows the same rule as recv():
		 * release/invalidate the native client, emit events->closed(), then return
		 * the corresponding negative result.
		 */
		int (*send)(socket_device_handle_t client, const void *source, size_t length);

		/*
		 * Locally closes and releases a listener or client native handle.
		 *
		 * This is synchronous with respect to ownership: after return, handle is
		 * stale and all later operations on it must fail. Graceful TCP FIN handling
		 * may be performed internally if it is bounded; this function must not wait
		 * indefinitely for peer acknowledgement.
		 *
		 * Local closure must not emit events->closed(), because the socket core
		 * already owns and schedules the local disconnection notification.
		 *
		 * Returns SOCKET_DEVICE_OK on success. Returning SOCKET_DEVICE_INVALID for
		 * an already stale handle is allowed. Other failures use a negative
		 * socket_device_result_t, but the backend must still make its best effort to
		 * release all native resources.
		 */
		int (*close)(socket_device_handle_t handle);

		/*
		 * Performs one non-blocking, bounded backend service pass.
		 *
		 * budget is the maximum number of normalized transport events the backend
		 * should emit during this call. A backend may do smaller constant-time
		 * housekeeping in addition to that limit, but it must not indefinitely
		 * drain sockets, block for network activity, sleep, or wait on an RTOS
		 * primitive.
		 *
		 * Asynchronous/ISR/other-core native events must be normalized and emitted
		 * here. accepted(), readable() and closed() may be called in any necessary
		 * order, but no token may be used after closed(). TX readiness is deliberately
		 * not an event: blocking core sends poll until completion, while nonblocking
		 * callers own their retry policy.
		 *
		 * poll() is never called recursively by the socket core. A backend must not
		 * call socket_server_poll() or socket_server_dotasks() from inside poll() or
		 * an event sink.
		 */
		void (*poll)(uint16_t budget);
	} socket_device_t;

#ifdef __cplusplus
}
#endif

#endif
