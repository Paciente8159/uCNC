/*
	Name: socket.c
	Description: Allocation-free, readiness-driven TCP stream server for uCNC.

	The backend reports normalized readiness/lifecycle events during poll(). The
	core records those events in fixed state and dispatches application callbacks
	after poll() returns. RX is pulled into one shared scratch buffer, one bounded
	chunk at a time.

	Copyright: Copyright (c) Joao Martins
	Author: Joao Martins

	uCNC is free software: you can redistribute it and/or modify it under the
	terms of the GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later
	version. Please see <http://www.gnu.org/licenses/>.
*/

#include "../../cnc.h"

#ifdef ENABLE_SOCKETS

#include "socket.h"

#include <limits.h>
#include <string.h>

#if MAX_SOCKETS > UINT8_MAX
#error "MAX_SOCKETS must fit in the internal uint8_t listener index"
#endif

#if SOCKET_RX_BUFFER_SIZE == 0
#error "SOCKET_RX_BUFFER_SIZE must be greater than zero"
#endif

/* Internal listener states stored in socket_if_t::state. */
enum
{
	SOCKET_IF_FREE = 0,
	SOCKET_IF_ACTIVE,
	SOCKET_IF_STOPPING
};

/* Internal fixed-client flags. */
enum
{
	SOCKET_CLIENT_IN_USE = 1U << 0,
	SOCKET_CLIENT_CONNECT_PENDING = 1U << 1,
	SOCKET_CLIENT_CONNECT_NOTIFIED = 1U << 2,
	SOCKET_CLIENT_READABLE = 1U << 3,
	SOCKET_CLIENT_CLOSE_PENDING = 1U << 4
};

/*
 * Internal flat client record.
 *
 * Native handles live here rather than in every listener x client combination.
 * listener_idx/local_idx map the record back to the stable public client index.
 * generation remains preserved while a record is free so its next token cannot
 * equal a token from its previous lifetime.
 */
typedef struct socket_client_state_
{
	socket_device_handle_t handle;
	uint16_t generation;
	uint8_t listener_idx;
	uint8_t local_idx;
	uint8_t flags;
	int8_t close_reason;
#ifdef ENABLE_SOCKET_TIMEOUTS
	uint32_t activity_ms;
#endif
} socket_client_state_t;

/* Static storage: no allocation occurs in this module. */
static socket_if_t raw_sockets[MAX_SOCKETS];
static socket_client_state_t socket_clients[SOCKET_MAX_CONNECTIONS];
static uint8_t socket_rx_buffer[SOCKET_RX_BUFFER_SIZE];
static socket_device_t *socket_device;

/* Prevent recursive application dispatch if application code calls dotasks(). */
static bool socket_core_running;

/* Prevent recursive backend polling through socket_server_poll(). */
static bool socket_device_polling;

/* Round-robin position for bounded per-client dispatch. */
static uint16_t socket_dispatch_cursor;

/* Forward declarations for the event sink given to the backend. */
static socket_device_token_t socket_device_accepted(socket_device_handle_t listener,
												 socket_device_handle_t client);
static void socket_device_readable(socket_device_token_t token);
static void socket_device_closed(socket_device_token_t token, int reason);

static const socket_device_events_t socket_device_events = {
	.accepted = socket_device_accepted,
	.readable = socket_device_readable,
	.closed = socket_device_closed};

/*
 * Returns the static listener index, or -1 if socket is NULL, foreign, or free.
 * Address comparison is used instead of subtracting potentially foreign pointers.
 */
static int socket_listener_index(const socket_if_t *socket)
{
	if (!socket)
	{
		return -1;
	}

	for (uint16_t i = 0; i < MAX_SOCKETS; ++i)
	{
		if (socket == &raw_sockets[i] && raw_sockets[i].state != SOCKET_IF_FREE)
		{
			return (int)i;
		}
	}

	return -1;
}

/* Returns the active listener whose native handle matches, otherwise NULL. */
static socket_if_t *socket_find_listener(socket_device_handle_t listener,
										 uint8_t *listener_idx)
{
	for (uint16_t i = 0; i < MAX_SOCKETS; ++i)
	{
		if (raw_sockets[i].state == SOCKET_IF_ACTIVE &&
			raw_sockets[i].listener == listener)
		{
			if (listener_idx)
			{
				*listener_idx = (uint8_t)i;
			}
			return &raw_sockets[i];
		}
	}

	return NULL;
}

/* Returns a free listener slot, or NULL when the fixed pool is exhausted. */
static socket_if_t *socket_find_free_listener(void)
{
	for (uint16_t i = 0; i < MAX_SOCKETS; ++i)
	{
		if (raw_sockets[i].state == SOCKET_IF_FREE)
		{
			return &raw_sockets[i];
		}
	}

	return NULL;
}

/* Returns a free flat client record index, or SOCKET_INVALID_CLIENT_SLOT. */
static uint16_t socket_find_free_client(void)
{
	for (uint16_t i = 0; i < SOCKET_MAX_CONNECTIONS; ++i)
	{
		if ((socket_clients[i].flags & SOCKET_CLIENT_IN_USE) == 0U)
		{
			return i;
		}
	}

	return SOCKET_INVALID_CLIENT_SLOT;
}

/*
 * Encodes a pool index and generation into a non-zero opaque token.
 * The low 16 bits store index + 1 so token zero remains permanently invalid.
 */
static socket_device_token_t socket_make_token(uint16_t slot, uint16_t generation)
{
	return ((socket_device_token_t)generation << 16) |
		   (socket_device_token_t)(slot + 1U);
}

/*
 * Resolves and validates a generation-tagged token.
 * Returns NULL for stale, malformed, free, or out-of-range tokens.
 */
static socket_client_state_t *socket_resolve_token(socket_device_token_t token,
												 uint16_t *slot_out)
{
	uint16_t encoded_slot;
	uint16_t slot;
	uint16_t generation;
	socket_client_state_t *client;

	if (token == SOCKET_DEVICE_INVALID_TOKEN)
	{
		return NULL;
	}

	encoded_slot = (uint16_t)(token & 0xFFFFU);
	if (encoded_slot == 0U)
	{
		return NULL;
	}

	slot = (uint16_t)(encoded_slot - 1U);
	if (slot >= SOCKET_MAX_CONNECTIONS)
	{
		return NULL;
	}

	generation = (uint16_t)(token >> 16);
	client = &socket_clients[slot];
	if ((client->flags & SOCKET_CLIENT_IN_USE) == 0U ||
		client->generation != generation)
	{
		return NULL;
	}

	if (slot_out)
	{
		*slot_out = slot;
	}

	return client;
}

/*
 * Resolves a listener-local public client index to its flat client record.
 * A pending-close record is returned; callers decide whether that state is valid
 * for their operation.
 */
static socket_client_state_t *socket_get_client(const socket_if_t *socket,
											 uint8_t client_idx,
											 uint16_t *slot_out)
{
	uint16_t slot;
	int listener_idx;
	socket_client_state_t *client;

	listener_idx = socket_listener_index(socket);
	if (listener_idx < 0 || client_idx >= SOCKET_MAX_CLIENTS)
	{
		return NULL;
	}

	slot = socket->client_slots[client_idx];
	if (slot == SOCKET_INVALID_CLIENT_SLOT || slot >= SOCKET_MAX_CONNECTIONS)
	{
		return NULL;
	}

	client = &socket_clients[slot];
	if ((client->flags & SOCKET_CLIENT_IN_USE) == 0U ||
		client->listener_idx != (uint8_t)listener_idx ||
		client->local_idx != client_idx)
	{
		return NULL;
	}

	if (slot_out)
	{
		*slot_out = slot;
	}

	return client;
}

/* Returns true if any flat client record still belongs to listener_idx. */
static bool socket_listener_has_records(uint8_t listener_idx)
{
	for (uint16_t i = 0; i < SOCKET_MAX_CONNECTIONS; ++i)
	{
		if ((socket_clients[i].flags & SOCKET_CLIENT_IN_USE) != 0U &&
			socket_clients[i].listener_idx == listener_idx)
		{
			return true;
		}
	}

	return false;
}

/*
 * Clears a listener only after stop-listening has delivered every pending client
 * disconnect. This preserves callback pointers and protocol context until then.
 */
static void socket_finalize_stopping_listener(uint8_t listener_idx)
{
	socket_if_t *socket;

	if (listener_idx >= MAX_SOCKETS)
	{
		return;
	}

	socket = &raw_sockets[listener_idx];
	if (socket->state != SOCKET_IF_STOPPING ||
		socket_listener_has_records(listener_idx))
	{
		return;
	}

	socket->listener = SOCKET_DEVICE_INVALID_HANDLE;
	for (uint8_t i = 0; i < SOCKET_MAX_CLIENTS; ++i)
	{
		socket->client_slots[i] = SOCKET_INVALID_CLIENT_SLOT;
	}
	socket->client_ondata_cb = NULL;
	socket->client_onidle_cb = NULL;
	socket->client_onconnected_cb = NULL;
	socket->client_ondisconnected_cb = NULL;
	socket->protocol = NULL;
	socket->state = SOCKET_IF_FREE;
}

/*
 * Releases a logical client record and listener-local mapping.
 * generation is deliberately preserved for stale-event protection.
 */
static void socket_release_client(uint16_t slot)
{
	socket_client_state_t *client;
	uint8_t listener_idx;
	uint8_t local_idx;

	if (slot >= SOCKET_MAX_CONNECTIONS)
	{
		return;
	}

	client = &socket_clients[slot];
	listener_idx = client->listener_idx;
	local_idx = client->local_idx;

	if (listener_idx < MAX_SOCKETS && local_idx < SOCKET_MAX_CLIENTS &&
		raw_sockets[listener_idx].client_slots[local_idx] == slot)
	{
		raw_sockets[listener_idx].client_slots[local_idx] =
			SOCKET_INVALID_CLIENT_SLOT;
	}

	client->handle = SOCKET_DEVICE_INVALID_HANDLE;
	client->listener_idx = UINT8_MAX;
	client->local_idx = UINT8_MAX;
	client->flags = 0U;
	client->close_reason = SOCKET_DEVICE_INVALID;
#ifdef ENABLE_SOCKET_TIMEOUTS
	client->activity_ms = 0U;
#endif

	if (listener_idx < MAX_SOCKETS)
	{
		socket_finalize_stopping_listener(listener_idx);
	}
}

/*
 * Event sink: allocates a static logical client and returns its generation token.
 * No application callback is invoked here; connection notification is deferred.
 */
static socket_device_token_t socket_device_accepted(socket_device_handle_t listener,
												 socket_device_handle_t native_client)
{
	socket_if_t *socket;
	socket_client_state_t *client;
	uint8_t listener_idx;
	uint8_t local_idx;
	uint16_t slot;

	if (native_client == SOCKET_DEVICE_INVALID_HANDLE)
	{
		return SOCKET_DEVICE_INVALID_TOKEN;
	}

	socket = socket_find_listener(listener, &listener_idx);
	if (!socket)
	{
		return SOCKET_DEVICE_INVALID_TOKEN;
	}

	for (local_idx = 0; local_idx < SOCKET_MAX_CLIENTS; ++local_idx)
	{
		if (socket->client_slots[local_idx] == SOCKET_INVALID_CLIENT_SLOT)
		{
			break;
		}
	}

	if (local_idx >= SOCKET_MAX_CLIENTS)
	{
		return SOCKET_DEVICE_INVALID_TOKEN;
	}

	slot = socket_find_free_client();
	if (slot == SOCKET_INVALID_CLIENT_SLOT)
	{
		return SOCKET_DEVICE_INVALID_TOKEN;
	}

	client = &socket_clients[slot];
	++client->generation;
	if (client->generation == 0U)
	{
		++client->generation;
	}

	client->handle = native_client;
	client->listener_idx = listener_idx;
	client->local_idx = local_idx;
	client->flags = SOCKET_CLIENT_IN_USE | SOCKET_CLIENT_CONNECT_PENDING;
	client->close_reason = SOCKET_DEVICE_INVALID;
#ifdef ENABLE_SOCKET_TIMEOUTS
	client->activity_ms = mcu_millis();
#endif

	socket->client_slots[local_idx] = slot;
	return socket_make_token(slot, client->generation);
}

/* Event sink: coalesces duplicate readable hints for a live generation token. */
static void socket_device_readable(socket_device_token_t token)
{
	socket_client_state_t *client = socket_resolve_token(token, NULL);
	if (client && (client->flags & SOCKET_CLIENT_CLOSE_PENDING) == 0U)
	{
		client->flags |= SOCKET_CLIENT_READABLE;
	}
}

/*
 * Event sink: records a backend-released remote/fatal close.
 * Late events are rejected by token generation matching.
 */
static void socket_device_closed(socket_device_token_t token, int reason)
{
	socket_client_state_t *client = socket_resolve_token(token, NULL);
	if (!client || (client->flags & SOCKET_CLIENT_CLOSE_PENDING) != 0U)
	{
		return;
	}

	client->handle = SOCKET_DEVICE_INVALID_HANDLE;
	client->flags &= (uint8_t)~SOCKET_CLIENT_READABLE;
	client->flags |= SOCKET_CLIENT_CLOSE_PENDING;
	client->close_reason = (int8_t)((reason < 0 && reason >= SOCKET_DEVICE_TIMEOUT)
								  ? reason
								  : SOCKET_DEVICE_ERROR);
}

bool socket_register_device(socket_device_t *device)
{
	if (!device /*|| socket_device*/ || !device->init || !device->listen ||
		!device->recv || !device->send || !device->close || !device->poll)
	{
		proto_print("1\r\n");
		return false;
	}

	for (uint16_t i = 0; i < MAX_SOCKETS; ++i)
	{
		if (raw_sockets[i].state != SOCKET_IF_FREE)
		{
			proto_print("2\r\n");
			return false;
		}
	}

	if (device->init(&socket_device_events) < 0)
	{
		proto_print("3\r\n");
		return false;
	}

	socket_device = device;
	return true;
}

socket_if_t *socket_start(uint32_t ip_listen, uint16_t port)
{
	socket_device_endpoint_t endpoint;
	socket_device_handle_t listener;
	socket_if_t *socket;

	if (!socket_device || port == 0U)
	{
		return NULL;
	}

	socket = socket_find_free_listener();
	if (!socket)
	{
		return NULL;
	}

	endpoint.address = ip_listen;
	endpoint.port = port;
	listener = socket_device->listen(&endpoint, SOCKET_MAX_CLIENTS);
	if (listener == SOCKET_DEVICE_INVALID_HANDLE)
	{
		return NULL;
	}

	memset(socket, 0, sizeof(*socket));
	socket->listener = listener;
	for (uint8_t i = 0; i < SOCKET_MAX_CLIENTS; ++i)
	{
		socket->client_slots[i] = SOCKET_INVALID_CLIENT_SLOT;
	}
	socket->state = SOCKET_IF_ACTIVE;
	return socket;
}

void socket_stop(socket_if_t *socket)
{
	int listener_idx = socket_listener_index(socket);

	if (listener_idx < 0 || socket->state == SOCKET_IF_FREE)
	{
		return;
	}

	if (socket->state == SOCKET_IF_ACTIVE)
	{
		socket_device_handle_t listener = socket->listener;
		socket->listener = SOCKET_DEVICE_INVALID_HANDLE;
		socket->state = SOCKET_IF_STOPPING;

		if (socket_device && listener != SOCKET_DEVICE_INVALID_HANDLE)
		{
			(void)socket_device->close(listener);
		}
	}

	for (uint8_t i = 0; i < SOCKET_MAX_CLIENTS; ++i)
	{
		if (socket->client_slots[i] != SOCKET_INVALID_CLIENT_SLOT)
		{
			(void)socket_close(socket, i);
		}
	}

	socket_finalize_stopping_listener((uint8_t)listener_idx);
}

void socket_add_ondata_handler(socket_if_t *socket,
								   socket_data_delegate callback)
{
	if (socket_listener_index(socket) >= 0)
	{
		socket->client_ondata_cb = callback;
	}
}

void socket_add_onidle_handler(socket_if_t *socket,
								   socket_idle_delegate callback)
{
	if (socket_listener_index(socket) >= 0)
	{
		socket->client_onidle_cb = callback;
	}
}

void socket_add_onconnected_handler(socket_if_t *socket,
									  socket_connect_delegate callback)
{
	if (socket_listener_index(socket) >= 0)
	{
		socket->client_onconnected_cb = callback;
	}
}


void socket_add_ondisconnected_handler(socket_if_t *socket,
										 socket_disconnect_delegate callback)
{
	if (socket_listener_index(socket) >= 0)
	{
		socket->client_ondisconnected_cb = callback;
	}
}

void socket_set_protocol(socket_if_t *socket, void *protocol)
{
	if (socket_listener_index(socket) >= 0)
	{
		socket->protocol = protocol;
	}
}

void *socket_get_protocol(const socket_if_t *socket)
{
	return socket_listener_index(socket) >= 0 ? socket->protocol : NULL;
}

static int socket_close_client(socket_client_state_t *client, int reason)
{
	socket_device_handle_t handle;
	int result = SOCKET_DEVICE_OK;

	if (!client || (client->flags & SOCKET_CLIENT_CLOSE_PENDING) != 0U)
	{
		return SOCKET_DEVICE_INVALID;
	}

	handle = client->handle;
	client->handle = SOCKET_DEVICE_INVALID_HANDLE;
	client->flags &= (uint8_t)~SOCKET_CLIENT_READABLE;
	client->flags |= SOCKET_CLIENT_CLOSE_PENDING;
	client->close_reason = (int8_t)reason;

	if (socket_device && handle != SOCKET_DEVICE_INVALID_HANDLE)
	{
		result = socket_device->close(handle);
	}

	return result < 0 ? result : SOCKET_DEVICE_OK;
}

int socket_send(socket_if_t *socket,
				uint8_t client_idx,
				const void *data,
				size_t data_len,
				bool noblock)
{
	socket_client_state_t *client;
	const uint8_t *bytes = (const uint8_t *)data;
	size_t total = 0U;
	uint32_t start_ms = 0U;

	if (!socket_device || (data_len > 0U && !data) || data_len > (size_t)INT_MAX)
	{
		return SOCKET_DEVICE_INVALID;
	}

	client = socket_get_client(socket, client_idx, NULL);
	if (!client || client->handle == SOCKET_DEVICE_INVALID_HANDLE ||
		(client->flags & (SOCKET_CLIENT_CONNECT_NOTIFIED |
						  SOCKET_CLIENT_CLOSE_PENDING)) != SOCKET_CLIENT_CONNECT_NOTIFIED)
	{
		return SOCKET_DEVICE_INVALID;
	}

	if (data_len == 0U)
	{
		return 0;
	}

	if (!noblock)
	{
		start_ms = mcu_millis();
	}

	while (total < data_len)
	{
		int result;
		size_t remaining = data_len - total;

		if (client->handle == SOCKET_DEVICE_INVALID_HANDLE ||
			(client->flags & SOCKET_CLIENT_CLOSE_PENDING) != 0U)
		{
			return client->close_reason < 0 ? client->close_reason : SOCKET_DEVICE_CLOSED;
		}

		result = socket_device->send(client->handle, &bytes[total], remaining);
		if (result > 0)
		{
			if ((size_t)result > remaining)
			{
				(void)socket_close_client(client, SOCKET_DEVICE_ERROR);
				return SOCKET_DEVICE_ERROR;
			}
			total += (size_t)result;
#ifdef ENABLE_SOCKET_TIMEOUTS
			client->activity_ms = mcu_millis();
#endif
			continue;
		}

		if (result != SOCKET_DEVICE_WOULD_BLOCK && result != 0)
		{
			return result;
		}

		if (noblock)
		{
			return (int)total;
		}

		if ((uint32_t)(mcu_millis() - start_ms) >= (uint32_t)SOCKET_SEND_TIMEOUT_MS)
		{
			(void)socket_close_client(client, SOCKET_DEVICE_TIMEOUT);
			return SOCKET_DEVICE_TIMEOUT;
		}

		/* Poll native state only. Application callbacks stay deferred. */
		socket_server_poll();
	}

	return (int)total;
}

int socket_close(socket_if_t *socket, uint8_t client_idx)
{
	socket_client_state_t *client = socket_get_client(socket, client_idx, NULL);
	return socket_close_client(client, SOCKET_DEVICE_OK);
}

bool socket_client_is_connected(const socket_if_t *socket,
									uint8_t client_idx)
{
	socket_client_state_t *client = socket_get_client(socket, client_idx, NULL);
	return client && client->handle != SOCKET_DEVICE_INVALID_HANDLE &&
		   (client->flags & (SOCKET_CLIENT_CONNECT_NOTIFIED |
							SOCKET_CLIENT_CLOSE_PENDING)) == SOCKET_CLIENT_CONNECT_NOTIFIED;
}

/*
 * Dispatches at most one unit of logical client work.
 * Returns true when a pending event/RX attempt/idle callback consumed this turn.
 */
static bool socket_dispatch_client(uint16_t slot)
{
	socket_client_state_t *client;
	socket_if_t *socket;
	uint8_t local_idx;

	if (slot >= SOCKET_MAX_CONNECTIONS)
	{
		return false;
	}

	client = &socket_clients[slot];
	if ((client->flags & SOCKET_CLIENT_IN_USE) == 0U ||
		client->listener_idx >= MAX_SOCKETS ||
		client->local_idx >= SOCKET_MAX_CLIENTS)
	{
		return false;
	}

	socket = &raw_sockets[client->listener_idx];
	local_idx = client->local_idx;

	/* Closure has priority and makes the public slot invalid before notification. */
	if ((client->flags & SOCKET_CLIENT_CLOSE_PENDING) != 0U)
	{
		socket_disconnect_delegate callback = socket->client_ondisconnected_cb;
		void *protocol = socket->protocol;
		int reason = client->close_reason;
		bool notified =
			(client->flags & SOCKET_CLIENT_CONNECT_NOTIFIED) != 0U;

		socket_release_client(slot);
		if (notified && callback)
		{
			callback(local_idx, reason, protocol);
		}
		return true;
	}

	/* First application-visible event for an accepted connection. */
	if ((client->flags & SOCKET_CLIENT_CONNECT_PENDING) != 0U)
	{
		client->flags &= (uint8_t)~SOCKET_CLIENT_CONNECT_PENDING;
		client->flags |= SOCKET_CLIENT_CONNECT_NOTIFIED;
		if (socket->client_onconnected_cb)
		{
			socket->client_onconnected_cb(local_idx, socket->protocol);
		}
		return true;
	}

	/*
	 * Pull one bounded RX chunk. Leave READABLE set after a positive read so the
	 * next round-robin visit can continue draining; clear it on WOULD_BLOCK.
	 */
	if ((client->flags & SOCKET_CLIENT_READABLE) != 0U)
	{
		int received;

		if (!socket_device || client->handle == SOCKET_DEVICE_INVALID_HANDLE)
		{
			client->flags &= (uint8_t)~SOCKET_CLIENT_READABLE;
			return true;
		}

		received = socket_device->recv(client->handle,
									 socket_rx_buffer,
									 sizeof(socket_rx_buffer));
		if (received > 0)
		{
			if ((size_t)received > sizeof(socket_rx_buffer))
			{
				client->handle = SOCKET_DEVICE_INVALID_HANDLE;
				client->flags &=
					(uint8_t)~SOCKET_CLIENT_READABLE;
				client->flags |= SOCKET_CLIENT_CLOSE_PENDING;
				client->close_reason = SOCKET_DEVICE_ERROR;
				return true;
			}
#ifdef ENABLE_SOCKET_TIMEOUTS
			client->activity_ms = mcu_millis();
#endif
			if (socket->client_ondata_cb)
			{
				socket->client_ondata_cb(local_idx,
									 socket_rx_buffer,
									 (size_t)received,
									 socket->protocol);
			}
		}
		else if (received == 0 || received == SOCKET_DEVICE_WOULD_BLOCK)
		{
			client->flags &= (uint8_t)~SOCKET_CLIENT_READABLE;
		}
		else
		{
			/* Contract requires backend recv() to have emitted closed() first. */
			client->flags &= (uint8_t)~SOCKET_CLIENT_READABLE;
			if ((client->flags & SOCKET_CLIENT_CLOSE_PENDING) == 0U)
			{
				/* Defensive logical cleanup for a contract-violating backend. */
				client->handle = SOCKET_DEVICE_INVALID_HANDLE;
				client->flags |= SOCKET_CLIENT_CLOSE_PENDING;
				client->close_reason = (int8_t)((received >= SOCKET_DEVICE_TIMEOUT)
										   ? received
										   : SOCKET_DEVICE_ERROR);
			}
		}
		return true;
	}


	/* Lowest-priority cooperative per-client callback. */
	if ((client->flags & SOCKET_CLIENT_CONNECT_NOTIFIED) != 0U &&
		socket->client_onidle_cb)
	{
#ifdef ENABLE_SOCKET_TIMEOUTS
		uint32_t idle_ms = mcu_millis() - client->activity_ms;
		socket->client_onidle_cb(local_idx, idle_ms, socket->protocol);
#else
		socket->client_onidle_cb(local_idx, 0U, socket->protocol);
#endif
		return true;
	}

	return false;
}

void socket_server_poll(void)
{
	if (!socket_device || socket_device_polling)
	{
		return;
	}

	socket_device_polling = true;
	socket_device->poll((uint16_t)SOCKET_DEVICE_POLL_BUDGET);
	socket_device_polling = false;
}

void socket_server_dotasks(void)
{
	if (!socket_device || socket_core_running)
	{
		return;
	}

	socket_core_running = true;
	socket_server_poll();

	/* Find at most one client with work, starting from the round-robin cursor. */
	for (uint16_t checked = 0; checked < SOCKET_MAX_CONNECTIONS; ++checked)
	{
		uint16_t slot = socket_dispatch_cursor;
		socket_dispatch_cursor =
			(uint16_t)((socket_dispatch_cursor + 1U) % SOCKET_MAX_CONNECTIONS);
		if (socket_dispatch_client(slot))
		{
			break;
		}
	}

	socket_core_running = false;
}

int socket_server_hasclients(const socket_if_t *socket)
{
	int count = 0;
	int listener_idx = socket ? socket_listener_index(socket) : -1;

	if (socket && listener_idx < 0)
	{
		return 0;
	}

	for (uint16_t i = 0; i < SOCKET_MAX_CONNECTIONS; ++i)
	{
		if ((socket_clients[i].flags & SOCKET_CLIENT_IN_USE) != 0U &&
			(!socket || socket_clients[i].listener_idx == (uint8_t)listener_idx))
		{
			++count;
		}
	}

	return count;
}

DECL_MODULE(socket_server)
{
	RUNONCE
	{
		for (uint16_t i = 0; i < MAX_SOCKETS; ++i)
		{
			memset(&raw_sockets[i], 0, sizeof(raw_sockets[i]));
			raw_sockets[i].listener = SOCKET_DEVICE_INVALID_HANDLE;
			for (uint8_t j = 0; j < SOCKET_MAX_CLIENTS; ++j)
			{
				raw_sockets[i].client_slots[j] = SOCKET_INVALID_CLIENT_SLOT;
			}
			raw_sockets[i].state = SOCKET_IF_FREE;
		}

		for (uint16_t i = 0; i < SOCKET_MAX_CONNECTIONS; ++i)
		{
			memset(&socket_clients[i], 0, sizeof(socket_clients[i]));
			socket_clients[i].handle = SOCKET_DEVICE_INVALID_HANDLE;
			socket_clients[i].listener_idx = UINT8_MAX;
			socket_clients[i].local_idx = UINT8_MAX;
			socket_clients[i].close_reason = SOCKET_DEVICE_INVALID;
		}

		socket_core_running = false;
		socket_device_polling = false;
		socket_dispatch_cursor = 0U;
		RUNONCE_COMPLETE();
	}
}

#endif
