/*
	Name: socket.h
	Description: Allocation-free TCP stream server API for uCNC.

	The public core owns fixed listener/connection tables and one shared RX
	scratch buffer. Native TCP buffering and TCP protocol correctness remain the
	responsibility of the registered socket_device_t backend.

	The API is cooperative and single-owner. Call socket_server_dotasks()
	frequently from the uCNC owner loop. Application callbacks execute only from
	that function and never directly from an ISR, another core, an RTOS network
	task, or backend poll().

	Copyright: Copyright (c) Joao Martins
	Author: Joao Martins

	uCNC is free software: you can redistribute it and/or modify it under the
	terms of the GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later
	version. Please see <http://www.gnu.org/licenses/>.
*/

#ifndef SOCKET_H
#define SOCKET_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "utils/socket_device.h"
#include "../../module.h"

	/* Maximum number of simultaneously allocated TCP listeners. */
#ifndef MAX_SOCKETS
#define MAX_SOCKETS 3 /*1 telnet+1 websocket+1 http*/
#endif

	/* Maximum number of clients addressable through one listener. */
#ifndef SOCKET_MAX_CLIENTS
#define SOCKET_MAX_CLIENTS 2
#endif

	/*
	 * Maximum total clients shared by all listeners.
	 *
	 * The default preserves the capacity of the former rectangular
	 * MAX_SOCKETS x SOCKET_MAX_CLIENTS layout. Small targets should lower this
	 * independently when the total simultaneous client count is smaller than
	 * the per-listener theoretical maximum.
	 */
#ifndef SOCKET_MAX_CONNECTIONS
#define SOCKET_MAX_CONNECTIONS (MAX_SOCKETS * SOCKET_MAX_CLIENTS)
#endif

	/*
	 * Size of the single shared core RX scratch buffer.
	 *
	 * This is not allocated per client. Each on-data callback must completely
	 * inspect/copy/parse its borrowed chunk before returning. Protocols that need
	 * bytes across callbacks must keep their own bounded, protocol-specific state.
	 */
#ifndef SOCKET_RX_BUFFER_SIZE
#ifdef SOCKET_MAX_DATA_SIZE
#define SOCKET_RX_BUFFER_SIZE SOCKET_MAX_DATA_SIZE
#else
#define SOCKET_RX_BUFFER_SIZE 1024
#endif
#endif

	/* Compatibility name for code that only uses the old value as a size. */
#ifndef SOCKET_MAX_DATA_SIZE
#define SOCKET_MAX_DATA_SIZE SOCKET_RX_BUFFER_SIZE
#endif

	/* Maximum normalized backend events emitted by one backend poll pass. */
#ifndef SOCKET_DEVICE_POLL_BUDGET
#define SOCKET_DEVICE_POLL_BUDGET 4U
#endif

	/* Idle time supplied when ENABLE_SOCKET_TIMEOUTS is enabled. */
#ifndef SOCKET_IDLE_TIMEOUT
#define SOCKET_IDLE_TIMEOUT 60U
#endif

	/* IPv4 wildcard address in host byte order. */
#ifndef IP_ANY
#define IP_ANY 0U
#endif

	/* Invalid index stored in each listener's local-to-global client map. */
#define SOCKET_INVALID_CLIENT_SLOT UINT16_MAX

#if MAX_SOCKETS == 0
#error "MAX_SOCKETS must be greater than zero"
#endif

#if SOCKET_MAX_CLIENTS == 0
#error "SOCKET_MAX_CLIENTS must be greater than zero"
#endif

#if SOCKET_MAX_CLIENTS > 32
#error "SOCKET_MAX_CLIENTS must be <= 32 because broadcast results use bit masks"
#endif

#if SOCKET_MAX_CONNECTIONS == 0 || SOCKET_MAX_CONNECTIONS >= UINT16_MAX
#error "SOCKET_MAX_CONNECTIONS must be between 1 and 65534"
#endif

	struct socket_if_;
	typedef struct socket_if_ socket_if_t;

	/*
	 * Called after a TCP client has been accepted into a listener-local slot.
	 *
	 * client_idx is stable for this connection's application lifetime and is in
	 * [0, SOCKET_MAX_CLIENTS). The connection is usable during the callback;
	 * socket_send() or socket_close() may be called.
	 */
	typedef void (*socket_connect_delegate)(uint8_t client_idx, void *protocol);

	/*
	 * Called with one borrowed binary TCP payload chunk.
	 *
	 * data points to data_len immutable bytes in the core's shared RX buffer.
	 * The pointer is valid only during this callback. The data is not NUL
	 * terminated and may contain zero bytes. The callback must consume, copy, or
	 * incorporate the complete chunk into bounded protocol state before return.
	 *
	 * TCP chunk boundaries are arbitrary: one protocol message may be split over
	 * many callbacks and several messages may be coalesced into one callback.
	 */
	typedef void (*socket_data_delegate)(uint8_t client_idx,
								 const uint8_t *data,
								 size_t data_len,
								 void *protocol);

	/*
	 * Called after a previous partial/blocked send may be able to progress.
	 *
	 * This is a readiness hint, not a guarantee. The callback should resume its
	 * per-client write state with socket_send() and preserve any remainder if the
	 * result is again partial or SOCKET_DEVICE_WOULD_BLOCK.
	 */
	typedef void (*socket_writable_delegate)(uint8_t client_idx, void *protocol);

	/*
	 * Called once when an application-visible client is disconnected.
	 *
	 * The client slot is already invalid when this callback runs; socket_send()
	 * and socket_close() for client_idx will fail. reason is SOCKET_DEVICE_CLOSED
	 * for an orderly remote close, SOCKET_DEVICE_OK for a local explicit close,
	 * or another negative socket_device_result_t for a fatal transport failure.
	 */
	typedef void (*socket_disconnect_delegate)(uint8_t client_idx,
									 int reason,
									 void *protocol);

	/*
	 * Called cooperatively when a connected client has no higher-priority pending
	 * socket event during its round-robin dispatch turn.
	 *
	 * idle_ms is milliseconds since the last successfully received or accepted
	 * TX byte when ENABLE_SOCKET_TIMEOUTS is enabled; otherwise it is zero. This
	 * callback does not automatically close a client at SOCKET_IDLE_TIMEOUT.
	 */
	typedef void (*socket_idle_delegate)(uint8_t client_idx,
								 uint32_t idle_ms,
								 void *protocol);

	/*
	 * Per-listener result of a best-effort broadcast attempt.
	 *
	 * Bit N corresponds to listener-local client N. A partial client has already
	 * accepted an unspecified positive prefix, so the whole broadcast must not be
	 * retried blindly. Reliable broadcast requires independent per-client write
	 * state and individual socket_send() continuation.
	 */
	typedef struct socket_broadcast_result_
	{
		uint32_t active_mask;
		uint32_t complete_mask;
		uint32_t partial_mask;
		uint32_t blocked_mask;
		uint32_t failed_mask;
	} socket_broadcast_result_t;

	/*
	 * Listener object allocated from the static core listener pool.
	 *
	 * Fields are exposed for zero-overhead static C integration but are owned by
	 * socket.c. Application/backend code must not modify them directly. Use the
	 * registration and protocol-context functions declared below.
	 */
	struct socket_if_
	{
		socket_device_handle_t listener;
		uint16_t client_slots[SOCKET_MAX_CLIENTS];
		socket_data_delegate client_ondata_cb;
		socket_idle_delegate client_onidle_cb;
		socket_connect_delegate client_onconnected_cb;
		socket_writable_delegate client_onwritable_cb;
		socket_disconnect_delegate client_ondisconnected_cb;
		void *protocol;
		uint8_t state;
	};

	/*
	 * Registers the one TCP backend used by the socket core.
	 *
	 * All backend function pointers are mandatory. The device is validated and
	 * device->init() is called exactly once. Registration is rejected if device
	 * is NULL, incomplete, initialization fails, another device is registered,
	 * or a listener is active/stopping.
	 *
	 * Returns true only when registration and initialization succeed.
	 */
	bool socket_register_device(socket_device_t *device);

	/*
	 * Creates an IPv4 TCP listener from the static listener pool.
	 *
	 * ip_listen and port use host byte order. IP_ANY listens on all local IPv4
	 * interfaces. The listener and its accepted clients are strictly non-blocking.
	 *
	 * Returns a stable socket_if_t pointer on success or NULL when the backend is
	 * unavailable, no listener slot is free, arguments are invalid, or native
	 * listen/bind fails.
	 */
	socket_if_t *socket_start(uint32_t ip_listen, uint16_t port);

	/*
	 * Stops accepting and schedules local closure of all clients on a listener.
	 *
	 * The native listener is closed synchronously. Client disconnected callbacks
	 * are deferred through socket_server_dotasks(); their reason is
	 * SOCKET_DEVICE_OK. The socket_if_t remains in a stopping state and is not
	 * reused until every pending disconnect callback has been dispatched.
	 *
	 * Repeated calls are harmless. A NULL or already-free listener is ignored.
	 */
	void socket_stop(socket_if_t *socket);

	/* Sets/replaces the borrowed RX-chunk callback; NULL disables it. */
	void socket_add_ondata_handler(socket_if_t *socket,
								   socket_data_delegate callback);

	/* Sets/replaces the cooperative idle callback; NULL disables it. */
	void socket_add_onidle_handler(socket_if_t *socket,
								   socket_idle_delegate callback);

	/* Sets/replaces the accepted-client callback; NULL disables it. */
	void socket_add_onconnected_handler(socket_if_t *socket,
									  socket_connect_delegate callback);

	/* Sets/replaces the TX-readiness callback; NULL disables it. */
	void socket_add_onwritable_handler(socket_if_t *socket,
									 socket_writable_delegate callback);

	/* Sets/replaces the client-disconnected callback; NULL disables it. */
	void socket_add_ondisconnected_handler(socket_if_t *socket,
										 socket_disconnect_delegate callback);

	/*
	 * Stores the listener's opaque application protocol context.
	 * The core never dereferences this pointer. It is passed unchanged to every
	 * listener callback and must remain valid until the listener becomes free.
	 */
	void socket_set_protocol(socket_if_t *socket, void *protocol);

	/* Returns the stored protocol context, or NULL for a NULL listener. */
	void *socket_get_protocol(const socket_if_t *socket);

	/*
	 * Performs one strictly non-blocking send attempt for one client.
	 *
	 * data points to data_len immutable bytes and is never retained by the core or
	 * backend. A positive return value is the number of bytes accepted by the
	 * native TCP transport and may be smaller than data_len. The caller owns and
	 * must preserve/recreate the unsent suffix.
	 *
	 * Returns 0 for data_len == 0. Returns SOCKET_DEVICE_WOULD_BLOCK when no bytes
	 * were accepted temporarily, SOCKET_DEVICE_INVALID for an invalid listener,
	 * client or pointer, and another negative result on transport failure.
	 *
	 * This function never polls the backend, blocks, waits, allocates, copies into
	 * a core TX queue, or calls an application callback.
	 */
	int socket_send(socket_if_t *socket,
					uint8_t client_idx,
					const void *data,
					size_t data_len);

	/*
	 * Performs one best-effort send attempt for every active listener client.
	 *
	 * result is mandatory and is always cleared before use. Returns the number of
	 * active clients attempted, or SOCKET_DEVICE_INVALID for invalid arguments.
	 * Inspect all masks; this operation does not provide reliable retry semantics
	 * after partial sends. See socket_broadcast_result_t.
	 */
	int socket_broadcast(socket_if_t *socket,
						 const void *data,
						 size_t data_len,
						 socket_broadcast_result_t *result);

	/*
	 * Schedules a local explicit close for one client.
	 *
	 * The native handle is invalidated and closed synchronously. The application
	 * disconnected callback is deferred through socket_server_dotasks() and gets
	 * reason SOCKET_DEVICE_OK. The client index must not be used after success.
	 *
	 * Returns SOCKET_DEVICE_OK on success, SOCKET_DEVICE_INVALID for an invalid or
	 * already closed client, or a negative backend close result. Even if native
	 * close reports an error, logical closure remains scheduled.
	 */
	int socket_close(socket_if_t *socket, uint8_t client_idx);

	/*
	 * Executes one bounded cooperative socket iteration.
	 *
	 * The function first calls backend->poll(SOCKET_DEVICE_POLL_BUDGET), then
	 * dispatches at most one application callback or one RX receive attempt for
	 * one round-robin client. Calling it recursively is safe but the nested call
	 * immediately returns without polling or dispatching.
	 */
	void socket_server_dotasks(void);

#ifndef sockets_dotasks
#define sockets_dotasks socket_server_dotasks
#endif

	/*
	 * Returns the number of logically active/stopping clients.
	 *
	 * If socket is non-NULL, counts that listener only. If socket is NULL, counts
	 * all listeners. Clients awaiting their disconnected callback remain counted
	 * until that callback is dispatched and their static slot is released.
	 */
	int socket_server_hasclients(const socket_if_t *socket);

	/*
	 * Returns true only if client_idx currently names a live, sendable client of
	 * socket. Clients pending local/remote disconnection return false.
	 */
	bool socket_client_is_connected(const socket_if_t *socket,
									uint8_t client_idx);

	/* Initializes all static socket core state. */
	DECL_MODULE(socket_server);

#ifdef __cplusplus
}
#endif

#endif
