/*
	Name: esp8266_lwip.c
	Description: Allocation-free lwIP raw TCP backend for the ESP8266.

	This backend adapts the ESP8266 Arduino/NONOS lwIP raw callback API to the
	cooperative uCNC socket_device_t contract. Native callbacks only retain state;
	they never call the uCNC event sink. Normalized events are emitted later from
	esp8266_socket_poll(), in the same owner context as socket_server_dotasks().

	The backend allocates no application memory. tcp_new(), tcp_write() and the RX
	pbufs use lwIP's own bounded pools. Received pbufs remain owned by this backend
	until esp8266_socket_recv() returns their bytes to the socket core.

	Copyright: Copyright (c) Joao Martins
	Author: Joao Martins

	uCNC is free software: you can redistribute it and/or modify it under the
	terms of the GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later
	version. Please see <http://www.gnu.org/licenses/>.
*/

#include "../../../cnc.h"

#if (MCU == MCU_ESP8266)
#ifdef ENABLE_SOCKETS

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <lwip/err.h>
#include <lwip/inet.h>
#include <lwip/ip_addr.h>
#include <lwip/opt.h>
#include <lwip/pbuf.h>
#include <lwip/tcp.h>

#include "../../../modules/net/socket.h"

/*
 * lwIP calls tcp_poll() in units of its slow TCP timer (normally 500 ms).
 * The poll callback is only a fallback writable hint when tcp_write() returned
 * ERR_MEM without any later ACK callback. It does not call the uCNC core.
 */
#ifndef ESP8266_SOCKET_TCP_POLL_INTERVAL
#define ESP8266_SOCKET_TCP_POLL_INTERVAL 2U
#endif

#if ESP8266_SOCKET_TCP_POLL_INTERVAL == 0 || ESP8266_SOCKET_TCP_POLL_INTERVAL > UINT8_MAX
#error "ESP8266_SOCKET_TCP_POLL_INTERVAL must be between 1 and 255"
#endif

/*
 * Bound the amount of retained TCP payload delivered during one cooperative
 * socket-core turn. The core's shared RX allocation is unchanged; this only
 * prevents one readable dispatch from parsing a 1 KiB burst and generating a
 * large sequence of small Telnet replies before the ESP8266 owner loop yields.
 */
#ifndef ESP8266_SOCKET_RECV_SLICE
#define ESP8266_SOCKET_RECV_SLICE 128U
#endif

#if ESP8266_SOCKET_RECV_SLICE == 0 || ESP8266_SOCKET_RECV_SLICE > UINT16_MAX
#error "ESP8266_SOCKET_RECV_SLICE must be between 1 and 65535"
#endif

/* Handles reserve the low 16 bits for a one-based backend table slot. */
#define ESP8266_SOCKET_HANDLE_SLOT_MASK ((uintptr_t)0xFFFFU)
#define ESP8266_SOCKET_TOTAL_SLOTS ((uint32_t)MAX_SOCKETS + (uint32_t)SOCKET_MAX_CONNECTIONS)

#if (MAX_SOCKETS + SOCKET_MAX_CONNECTIONS) >= UINT16_MAX
#error "ESP8266 socket listener + client count must be less than 65535"
#endif

enum
{
	ESP8266_LISTENER_FREE = 0,
	ESP8266_LISTENER_ACTIVE
};

enum
{
	ESP8266_CLIENT_FREE = 0,
	ESP8266_CLIENT_ACCEPT_PENDING,
	ESP8266_CLIENT_CONNECTED
};

enum
{
	ESP8266_CLIENT_RX_EVENT = 1U << 0,
	ESP8266_CLIENT_CLOSE_PENDING = 1U << 1,
	/*
	 * TX_INTEREST means a partial/blocked send still needs future native
	 * progress. TX_EVENT is a separate latched progress edge waiting for
	 * owner-context poll() delivery. They must never be consumed together:
	 * another send can re-arm TX_INTEREST while an older TX_EVENT is pending.
	 */
	ESP8266_CLIENT_TX_INTEREST = 1U << 2,
	ESP8266_CLIENT_TX_EVENT = 1U << 3,
	ESP8266_CLIENT_BACKLOG_DELAYED = 1U << 4
};

typedef struct esp8266_listener_
{
	struct tcp_pcb *pcb;
	uint16_t generation;
	uint8_t state;
} esp8266_listener_t;

typedef struct esp8266_client_
{
	struct tcp_pcb *pcb;
	struct pbuf *rx;
	socket_device_handle_t listener_handle;
	socket_device_token_t token;
	uint32_t rx_available;
	uint16_t generation;
	int8_t close_reason;
	uint8_t state;
	uint8_t flags;
} esp8266_client_t;

/*
 * Static backend RAM is:
 *   MAX_SOCKETS * sizeof(esp8266_listener_t) +
 *   SOCKET_MAX_CONNECTIONS * sizeof(esp8266_client_t).
 *
 * With 32-bit pointers/handles this is normally 8 bytes per listener and
 * 28 bytes per client (480 bytes with the socket.h defaults). Retained RX
 * pbuf memory belongs to, and is bounded by, lwIP's configured pools/window.
 */
static esp8266_listener_t esp8266_listeners[MAX_SOCKETS];
static esp8266_client_t esp8266_clients[SOCKET_MAX_CONNECTIONS];
static const socket_device_events_t *esp8266_events;
static uint16_t esp8266_poll_cursor;

static err_t esp8266_accept_callback(void *arg,
									 struct tcp_pcb *newpcb,
									 err_t error);
static err_t esp8266_recv_callback(void *arg,
								 struct tcp_pcb *pcb,
								 struct pbuf *p,
								 err_t error);
static err_t esp8266_sent_callback(void *arg,
								 struct tcp_pcb *pcb,
								 u16_t length);
static err_t esp8266_poll_callback(void *arg, struct tcp_pcb *pcb);
static void esp8266_error_callback(void *arg, err_t error);

/* Returns a generation-tagged handle for a zero-based global backend slot. */
static socket_device_handle_t esp8266_make_handle(uint16_t global_slot,
												  uint16_t generation)
{
	return (socket_device_handle_t)(((uint32_t)generation << 16) |
									(uint32_t)(global_slot + 1U));
}

static uint16_t esp8266_next_generation(uint16_t generation)
{
	++generation;
	if (generation == 0U)
	{
		++generation;
	}
	return generation;
}

/*
 * Decodes only handles produced by this 32-bit target backend. The generation
 * check prevents a stale core/native handle from naming a reused static slot.
 */
static bool esp8266_decode_handle(socket_device_handle_t handle,
								  uint16_t *global_slot,
								  uint16_t *generation)
{
	uintptr_t value = (uintptr_t)handle;
	uint16_t encoded_slot;

	if (handle == SOCKET_DEVICE_INVALID_HANDLE ||
		(value & ~(uintptr_t)UINT32_MAX) != 0U)
	{
		return false;
	}

	encoded_slot = (uint16_t)(value & ESP8266_SOCKET_HANDLE_SLOT_MASK);
	if (encoded_slot == 0U ||
		(uint32_t)encoded_slot > ESP8266_SOCKET_TOTAL_SLOTS)
	{
		return false;
	}

	if (global_slot)
	{
		*global_slot = (uint16_t)(encoded_slot - 1U);
	}
	if (generation)
	{
		*generation = (uint16_t)((uint32_t)value >> 16);
	}
	return true;
}

static socket_device_handle_t esp8266_listener_handle(
	const esp8266_listener_t *listener)
{
	uint16_t index = (uint16_t)(listener - esp8266_listeners);
	return esp8266_make_handle(index, listener->generation);
}

static socket_device_handle_t esp8266_client_handle(
	const esp8266_client_t *client)
{
	uint16_t index = (uint16_t)(client - esp8266_clients);
	return esp8266_make_handle((uint16_t)(MAX_SOCKETS + index),
							   client->generation);
}

static esp8266_listener_t *esp8266_listener_from_handle(
	socket_device_handle_t handle)
{
	uint16_t slot;
	uint16_t generation;
	esp8266_listener_t *listener;

	if (!esp8266_decode_handle(handle, &slot, &generation) ||
		slot >= MAX_SOCKETS)
	{
		return NULL;
	}

	listener = &esp8266_listeners[slot];
	if (listener->state != ESP8266_LISTENER_ACTIVE ||
		listener->generation != generation)
	{
		return NULL;
	}
	return listener;
}

static esp8266_client_t *esp8266_client_from_handle(
	socket_device_handle_t handle)
{
	uint16_t slot;
	uint16_t generation;
	uint16_t client_slot;
	esp8266_client_t *client;

	if (!esp8266_decode_handle(handle, &slot, &generation) ||
		slot < MAX_SOCKETS)
	{
		return NULL;
	}

	client_slot = (uint16_t)(slot - MAX_SOCKETS);
	if (client_slot >= SOCKET_MAX_CONNECTIONS)
	{
		return NULL;
	}

	client = &esp8266_clients[client_slot];
	if (client->state == ESP8266_CLIENT_FREE ||
		client->generation != generation)
	{
		return NULL;
	}
	return client;
}

/* Callback arguments must be exact pointers into the fixed client table. */
static esp8266_client_t *esp8266_client_from_callback_arg(void *arg)
{
	for (uint16_t i = 0; i < SOCKET_MAX_CONNECTIONS; ++i)
	{
		if (arg == &esp8266_clients[i] &&
			esp8266_clients[i].state != ESP8266_CLIENT_FREE)
		{
			return &esp8266_clients[i];
		}
	}
	return NULL;
}

static esp8266_listener_t *esp8266_listener_from_callback_arg(void *arg)
{
	for (uint16_t i = 0; i < MAX_SOCKETS; ++i)
	{
		if (arg == &esp8266_listeners[i] &&
			esp8266_listeners[i].state == ESP8266_LISTENER_ACTIVE)
		{
			return &esp8266_listeners[i];
		}
	}
	return NULL;
}

static esp8266_listener_t *esp8266_allocate_listener(void)
{
	for (uint16_t i = 0; i < MAX_SOCKETS; ++i)
	{
		esp8266_listener_t *listener = &esp8266_listeners[i];
		if (listener->state == ESP8266_LISTENER_FREE)
		{
			listener->generation =
				esp8266_next_generation(listener->generation);
			listener->pcb = NULL;
			listener->state = ESP8266_LISTENER_ACTIVE;
			return listener;
		}
	}
	return NULL;
}

static esp8266_client_t *esp8266_allocate_client(void)
{
	for (uint16_t i = 0; i < SOCKET_MAX_CONNECTIONS; ++i)
	{
		esp8266_client_t *client = &esp8266_clients[i];
		if (client->state == ESP8266_CLIENT_FREE)
		{
			uint16_t generation =
				esp8266_next_generation(client->generation);

			memset(client, 0, sizeof(*client));
			client->generation = generation;
			client->listener_handle = SOCKET_DEVICE_INVALID_HANDLE;
			client->token = SOCKET_DEVICE_INVALID_TOKEN;
			client->close_reason = (int8_t)SOCKET_DEVICE_INVALID;
			client->state = ESP8266_CLIENT_ACCEPT_PENDING;
			return client;
		}
	}
	return NULL;
}

static void esp8266_free_rx(esp8266_client_t *client)
{
	if (client->rx)
	{
		pbuf_free(client->rx);
		client->rx = NULL;
	}
	client->rx_available = 0U;
	client->flags &= (uint8_t)~ESP8266_CLIENT_RX_EVENT;
}

static void esp8266_reset_listener(esp8266_listener_t *listener)
{
	uint16_t generation = listener->generation;
	memset(listener, 0, sizeof(*listener));
	listener->generation = generation;
	listener->state = ESP8266_LISTENER_FREE;
}

static void esp8266_reset_client(esp8266_client_t *client)
{
	uint16_t generation = client->generation;
	esp8266_free_rx(client);
	memset(client, 0, sizeof(*client));
	client->generation = generation;
	client->listener_handle = SOCKET_DEVICE_INVALID_HANDLE;
	client->token = SOCKET_DEVICE_INVALID_TOKEN;
	client->close_reason = (int8_t)SOCKET_DEVICE_INVALID;
	client->state = ESP8266_CLIENT_FREE;
}

static void esp8266_detach_client_callbacks(struct tcp_pcb *pcb)
{
	if (!pcb)
	{
		return;
	}

	/* tcp_err(NULL) must precede close/abort: tcp_abort may call errf. */
	tcp_err(pcb, NULL);
	tcp_recv(pcb, NULL);
	tcp_sent(pcb, NULL);
	tcp_poll(pcb, NULL, 0U);
	tcp_arg(pcb, NULL);
}

/*
 * Releases native/backend ownership without emitting a uCNC close event.
 * tcp_close() is attempted once; ERR_MEM is resolved by a bounded abort rather
 * than retaining a stale public handle or retrying indefinitely.
 */
static void esp8266_release_client(esp8266_client_t *client)
{
	struct tcp_pcb *pcb = client->pcb;

	client->pcb = NULL;
	if (pcb)
	{
		if ((client->flags & ESP8266_CLIENT_BACKLOG_DELAYED) != 0U)
		{
			tcp_backlog_accepted(pcb);
			client->flags &= (uint8_t)~ESP8266_CLIENT_BACKLOG_DELAYED;
		}

		esp8266_detach_client_callbacks(pcb);
		if (tcp_close(pcb) != ERR_OK)
		{
			tcp_abort(pcb);
		}
	}
	esp8266_reset_client(client);
}

/* Owner-context only: release first, then invalidate the core token by event. */
static void esp8266_finish_client(esp8266_client_t *client, int reason)
{
	socket_device_token_t token = client->token;

	esp8266_release_client(client);
	if (token != SOCKET_DEVICE_INVALID_TOKEN)
	{
		esp8266_events->closed(token,
			(reason < 0) ? reason : SOCKET_DEVICE_ERROR);
	}
}

static int esp8266_map_error(err_t error)
{
	switch (error)
	{
	case ERR_OK:
		return SOCKET_DEVICE_OK;
#ifdef ERR_WOULDBLOCK
	case ERR_WOULDBLOCK:
#endif
#ifdef ERR_INPROGRESS
	case ERR_INPROGRESS:
#endif
	case ERR_MEM:
#ifdef ERR_BUF
	case ERR_BUF:
#endif
		return SOCKET_DEVICE_WOULD_BLOCK;

#ifdef ERR_CLSD
	case ERR_CLSD:
		return SOCKET_DEVICE_CLOSED;
#endif

#ifdef ERR_ARG
	case ERR_ARG:
#endif
#ifdef ERR_VAL
	case ERR_VAL:
#endif
		return SOCKET_DEVICE_INVALID;

	default:
		return SOCKET_DEVICE_ERROR;
	}
}

static void esp8266_mark_close(esp8266_client_t *client, int reason)
{
	if ((client->flags & ESP8266_CLIENT_CLOSE_PENDING) == 0U)
	{
		client->close_reason = (int8_t)((reason < 0) ? reason :
										 SOCKET_DEVICE_ERROR);
		client->flags |= ESP8266_CLIENT_CLOSE_PENDING;
	}
	client->flags &=
		(uint8_t)~(ESP8266_CLIENT_TX_INTEREST | ESP8266_CLIENT_TX_EVENT);
}

/*
 * Takes ownership of p on success. pbuf_cat() transfers the new chain without
 * changing its reference count. The 16-bit guard preserves lwIP's pbuf tot_len
 * invariant on ESP8266 builds without receive-window scaling.
 */
static bool esp8266_append_rx(esp8266_client_t *client, struct pbuf *p)
{
	uint32_t incoming;
	bool was_empty;

	if (!p)
	{
		return false;
	}

	incoming = (uint32_t)p->tot_len;
	if (incoming == 0U ||
		client->rx_available > (uint32_t)UINT16_MAX - incoming)
	{
		return false;
	}

	was_empty = (client->rx == NULL);
	if (was_empty)
	{
		client->rx = p;
	}
	else
	{
		pbuf_cat(client->rx, p);
	}
	client->rx_available += incoming;

	/* One event is enough while the retained stream was already non-empty. */
	if (was_empty)
	{
		client->flags |= ESP8266_CLIENT_RX_EVENT;
	}
	return true;
}

static err_t esp8266_recv_callback(void *arg,
								 struct tcp_pcb *pcb,
								 struct pbuf *p,
								 err_t error)
{
	esp8266_client_t *client = esp8266_client_from_callback_arg(arg);

	if (!client || client->pcb != pcb)
	{
		if (p)
		{
			pbuf_free(p);
		}
		return ERR_OK;
	}

	if (error != ERR_OK)
	{
		if (p)
		{
			pbuf_free(p);
		}
		esp8266_mark_close(client, esp8266_map_error(error));
		return ERR_OK;
	}

	/* p == NULL is the raw lwIP indication for an orderly peer FIN. */
	if (!p)
	{
		esp8266_mark_close(client, SOCKET_DEVICE_CLOSED);
		return ERR_OK;
	}

	/*
	 * Returning ERR_MEM leaves p owned by lwIP as refused_data. This path is
	 * only needed for a pathological >64 KiB retained chain; normal ESP8266
	 * receive windows are much smaller. No payload is acknowledged here.
	 */
	if (!esp8266_append_rx(client, p))
	{
		return ERR_MEM;
	}

#ifdef PBUF_FLAG_TCP_FIN
	/* lwIP can attach a deferred FIN to a previously refused pbuf. */
	if ((p->flags & PBUF_FLAG_TCP_FIN) != 0U)
	{
		esp8266_mark_close(client, SOCKET_DEVICE_CLOSED);
	}
#endif

	return ERR_OK;
}

static err_t esp8266_sent_callback(void *arg,
								 struct tcp_pcb *pcb,
								 u16_t length)
{
	esp8266_client_t *client = esp8266_client_from_callback_arg(arg);
	(void)length;

	if (client && client->pcb == pcb &&
		client->state == ESP8266_CLIENT_CONNECTED &&
		(client->flags & ESP8266_CLIENT_TX_INTEREST) != 0U &&
		(client->flags & ESP8266_CLIENT_CLOSE_PENDING) == 0U)
	{
		/*
		 * Atomically move this interest into the pending-event latch.
		 * A later partial send may independently re-arm TX_INTEREST before
		 * owner-context poll() consumes TX_EVENT.
		 */
		client->flags &= (uint8_t)~ESP8266_CLIENT_TX_INTEREST;
		client->flags |= ESP8266_CLIENT_TX_EVENT;
	}
	return ERR_OK;
}

static err_t esp8266_poll_callback(void *arg, struct tcp_pcb *pcb)
{
	esp8266_client_t *client = esp8266_client_from_callback_arg(arg);

	if (client && client->pcb == pcb &&
		client->state == ESP8266_CLIENT_CONNECTED &&
		(client->flags & ESP8266_CLIENT_TX_INTEREST) != 0U &&
		(client->flags & ESP8266_CLIENT_CLOSE_PENDING) == 0U)
	{
		/*
		 * ERR_MEM may have meant global lwIP pool exhaustion rather than a
		 * full per-PCB sndbuf. A periodic, edge-coalesced retry hint is needed
		 * when no ACK callback will occur for this PCB.
		 */
		client->flags &= (uint8_t)~ESP8266_CLIENT_TX_INTEREST;
		client->flags |= ESP8266_CLIENT_TX_EVENT;
	}
	return ERR_OK;
}

static void esp8266_error_callback(void *arg, err_t error)
{
	esp8266_client_t *client = esp8266_client_from_callback_arg(arg);

	if (!client)
	{
		return;
	}

	/* lwIP has already freed the PCB before invoking tcp_err(). */
	client->pcb = NULL;
	esp8266_mark_close(client, esp8266_map_error(error));

	/*
	 * Retained RX is intentionally not freed. poll()/recv() reports and drains
	 * those already-accepted bytes before emitting the fatal close event.
	 */
}

static err_t esp8266_accept_callback(void *arg,
									 struct tcp_pcb *newpcb,
									 err_t error)
{
	esp8266_listener_t *listener =
		esp8266_listener_from_callback_arg(arg);
	esp8266_client_t *client;

	if (!listener || error != ERR_OK || !newpcb)
	{
		if (newpcb)
		{
			tcp_abort(newpcb);
			return ERR_ABRT;
		}
		return error;
	}

	client = esp8266_allocate_client();
	if (!client)
	{
		/* No fixed backend slot: reject without ever creating a core token. */
		tcp_abort(newpcb);
		return ERR_ABRT;
	}

	client->pcb = newpcb;
	client->listener_handle = esp8266_listener_handle(listener);
	client->flags = ESP8266_CLIENT_BACKLOG_DELAYED;

	/* Install callbacks before returning so no native RX lacks an owner. */
	tcp_arg(newpcb, client);
	tcp_err(newpcb, esp8266_error_callback);
	tcp_recv(newpcb, esp8266_recv_callback);
	tcp_sent(newpcb, esp8266_sent_callback);
	tcp_poll(newpcb,
			 esp8266_poll_callback,
			 (u8_t)ESP8266_SOCKET_TCP_POLL_INTERVAL);

	/* poll() will complete the backlog/core acceptance handshake. */
	tcp_backlog_delayed(newpcb);
	return ERR_OK;
}

static int esp8266_socket_init(const socket_device_events_t *events)
{
	if (!events || !events->accepted || !events->readable ||
		!events->writable || !events->closed)
	{
		return SOCKET_DEVICE_INVALID;
	}

	memset(esp8266_listeners, 0, sizeof(esp8266_listeners));
	memset(esp8266_clients, 0, sizeof(esp8266_clients));
	for (uint16_t i = 0; i < SOCKET_MAX_CONNECTIONS; ++i)
	{
		esp8266_clients[i].listener_handle = SOCKET_DEVICE_INVALID_HANDLE;
		esp8266_clients[i].token = SOCKET_DEVICE_INVALID_TOKEN;
		esp8266_clients[i].close_reason = (int8_t)SOCKET_DEVICE_INVALID;
	}

	esp8266_poll_cursor = 0U;
	esp8266_events = events;
	return SOCKET_DEVICE_OK;
}

static socket_device_handle_t esp8266_socket_listen(
	const socket_device_endpoint_t *endpoint,
	uint8_t backlog)
{
	esp8266_listener_t *listener;
	struct tcp_pcb *pcb;
	struct tcp_pcb *listen_pcb;
	ip_addr_t bind_address;
	const ip_addr_t *bind_address_ptr;
	uint8_t native_backlog;
	err_t error;

	if (!endpoint || endpoint->port == 0U)
	{
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	listener = esp8266_allocate_listener();
	if (!listener)
	{
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	pcb = tcp_new();
	if (!pcb)
	{
		esp8266_reset_listener(listener);
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	if (endpoint->address == IP_ANY)
	{
		bind_address_ptr = IP_ADDR_ANY;
	}
	else
	{
		/* endpoint->address is host order; lwIP stores IPv4 in network order. */
		bind_address.addr = htonl(endpoint->address);
		bind_address_ptr = &bind_address;
	}

	error = tcp_bind(pcb, bind_address_ptr, endpoint->port);
	if (error != ERR_OK)
	{
		if (tcp_close(pcb) != ERR_OK)
		{
			tcp_abort(pcb);
		}
		esp8266_reset_listener(listener);
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	/* The core never needs a backlog larger than its per-listener client pool. */
	native_backlog = backlog;
	if (native_backlog == 0U)
	{
		native_backlog = 1U;
	}
	if (native_backlog > SOCKET_MAX_CLIENTS)
	{
		native_backlog = SOCKET_MAX_CLIENTS;
	}

	listen_pcb = tcp_listen_with_backlog(pcb, native_backlog);
	if (!listen_pcb)
	{
		/* On allocation failure lwIP leaves the original bound PCB valid. */
		if (tcp_close(pcb) != ERR_OK)
		{
			tcp_abort(pcb);
		}
		esp8266_reset_listener(listener);
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	listener->pcb = listen_pcb;
	tcp_arg(listen_pcb, listener);
	tcp_accept(listen_pcb, esp8266_accept_callback);
	return esp8266_listener_handle(listener);
}

static int esp8266_socket_recv(socket_device_handle_t handle,
							   void *destination,
							   size_t capacity)
{
	esp8266_client_t *client = esp8266_client_from_handle(handle);
	uint8_t *output = (uint8_t *)destination;
	size_t limit;
	size_t copied = 0U;

	if (!client || client->state != ESP8266_CLIENT_CONNECTED)
	{
		return SOCKET_DEVICE_INVALID;
	}
	if (capacity == 0U)
	{
		return 0;
	}
	if (!destination)
	{
		return SOCKET_DEVICE_INVALID;
	}

	if (!client->rx)
	{
		if ((client->flags & ESP8266_CLIENT_CLOSE_PENDING) != 0U)
		{
			int reason = client->close_reason;
			esp8266_finish_client(client, reason);
			return reason;
		}
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	/* tcp_recved() and pbuf lengths are u16_t; the public result is int. */
	limit = capacity;
	if (limit > (size_t)ESP8266_SOCKET_RECV_SLICE)
	{
		limit = (size_t)ESP8266_SOCKET_RECV_SLICE;
	}
	if (limit > (size_t)UINT16_MAX)
	{
		limit = (size_t)UINT16_MAX;
	}
	if (limit > (size_t)INT_MAX)
	{
		limit = (size_t)INT_MAX;
	}

	while (copied < limit && client->rx)
	{
		struct pbuf *head = client->rx;
		size_t chunk;

		if (head->len == 0U)
		{
			client->rx = head->next;
			head->next = NULL;
			head->tot_len = 0U;
			pbuf_free(head);
			continue;
		}

		chunk = (size_t)head->len;
		if (chunk > limit - copied)
		{
			chunk = limit - copied;
		}
		/* pbuf_header() takes s16_t; split unusually large pbufs safely. */
		if (chunk > (size_t)INT16_MAX)
		{
			chunk = (size_t)INT16_MAX;
		}

		memcpy(output + copied, head->payload, chunk);
		copied += chunk;
		client->rx_available -= (uint32_t)chunk;

		if (chunk == (size_t)head->len)
		{
			client->rx = head->next;
			head->next = NULL;
			head->tot_len = head->len;
			pbuf_free(head);
		}
		else if (pbuf_header(head, -(s16_t)chunk) != 0U)
		{
			/* Removing bytes from an RX PBUF_POOL head should never fail. */
			esp8266_free_rx(client);
			esp8266_mark_close(client, SOCKET_DEVICE_ERROR);
			break;
		}
	}

	if (!client->rx)
	{
		client->rx_available = 0U;
	}

	if (copied > 0U)
	{
		/* Only bytes actually returned to the core reopen the TCP RX window. */
		if (client->pcb)
		{
			tcp_recved(client->pcb, (u16_t)copied);
		}
		return (int)copied;
	}

	if ((client->flags & ESP8266_CLIENT_CLOSE_PENDING) != 0U)
	{
		int reason = client->close_reason;
		esp8266_finish_client(client, reason);
		return reason;
	}
	return SOCKET_DEVICE_WOULD_BLOCK;
}

static int esp8266_socket_send(socket_device_handle_t handle,
							   const void *source,
							   size_t length)
{
	esp8266_client_t *client = esp8266_client_from_handle(handle);
	u16_t available;
	u16_t attempt;
	err_t error;
	int mapped;

	if (!client || client->state != ESP8266_CLIENT_CONNECTED)
	{
		return SOCKET_DEVICE_INVALID;
	}
	if (length == 0U)
	{
		return 0;
	}
	if (!source)
	{
		return SOCKET_DEVICE_INVALID;
	}

	/* A fatal callback can leave final RX to drain after lwIP freed the PCB. */
	if (!client->pcb)
	{
		return (client->flags & ESP8266_CLIENT_CLOSE_PENDING) != 0U
				   ? client->close_reason
				   : SOCKET_DEVICE_CLOSED;
	}

	available = tcp_sndbuf(client->pcb);
	if (available == 0U)
	{
		client->flags |= ESP8266_CLIENT_TX_INTEREST;
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	attempt = available;
	if ((size_t)attempt > length)
	{
		attempt = (u16_t)length;
	}
	/* A u16_t conversion of 65536-or-more must never turn the attempt into zero. */
	if (attempt == 0U)
	{
		attempt = UINT16_MAX;
	}

	error = tcp_write(client->pcb,
					  source,
					  attempt,
					  TCP_WRITE_FLAG_COPY);
	if (error != ERR_OK)
	{
		mapped = esp8266_map_error(error);
		if (mapped == SOCKET_DEVICE_WOULD_BLOCK)
		{
			client->flags |= ESP8266_CLIENT_TX_INTEREST;
			return mapped;
		}

		esp8266_mark_close(client, mapped);
		if (!client->rx)
		{
			esp8266_finish_client(client, mapped);
		}
		return mapped;
	}

	if ((size_t)attempt < length)
	{
		/*
		 * Arm interest before tcp_output(). This closes the lost-wake-up
		 * window if the ESP8266/lwIP integration processes native progress
		 * reentrantly while output is being kicked.
		 */
		client->flags |= ESP8266_CLIENT_TX_INTEREST;
	}

	/* tcp_write(COPY) accepted the bytes independently of output progress. */
	(void)tcp_output(client->pcb);

	/*
	 * Do not clear pre-existing TX_INTEREST/TX_EVENT after a later full send.
	 *
	 * Interest belongs to an earlier partial/blocked operation whose remaining
	 * suffix is owned by the protocol layer. This successful call may be an
	 * unrelated message and cannot prove that the older suffix was resumed.
	 * Native progress moves interest into TX_EVENT; owner-context poll() later
	 * consumes only that event when it emits writable().
	 */
	return (int)attempt;
}

static int esp8266_socket_close(socket_device_handle_t handle)
{
	esp8266_listener_t *listener = esp8266_listener_from_handle(handle);

	if (listener)
	{
		struct tcp_pcb *pcb = listener->pcb;

		/* Reject native accepts not yet exposed to the core for this listener. */
		for (uint16_t i = 0; i < SOCKET_MAX_CONNECTIONS; ++i)
		{
			esp8266_client_t *client = &esp8266_clients[i];
			if (client->state == ESP8266_CLIENT_ACCEPT_PENDING &&
				client->listener_handle == handle)
			{
				esp8266_release_client(client);
			}
		}

		listener->pcb = NULL;
		if (pcb)
		{
			tcp_accept(pcb, NULL);
			tcp_arg(pcb, NULL);
			if (tcp_close(pcb) != ERR_OK)
			{
				tcp_abort(pcb);
			}
		}
		esp8266_reset_listener(listener);
		return SOCKET_DEVICE_OK;
	}

	{
		esp8266_client_t *client = esp8266_client_from_handle(handle);
		if (!client)
		{
			return SOCKET_DEVICE_INVALID;
		}

		/* Local closure never emits events->closed(). */
		esp8266_release_client(client);
		return SOCKET_DEVICE_OK;
	}
}

static void esp8266_accept_in_owner_context(esp8266_client_t *client)
{
	socket_device_token_t token;

	if (client->pcb &&
		(client->flags & ESP8266_CLIENT_BACKLOG_DELAYED) != 0U)
	{
		tcp_backlog_accepted(client->pcb);
		client->flags &= (uint8_t)~ESP8266_CLIENT_BACKLOG_DELAYED;
	}

	token = esp8266_events->accepted(client->listener_handle,
									 esp8266_client_handle(client));
	if (token == SOCKET_DEVICE_INVALID_TOKEN)
	{
		/* Core capacity/listener rejection: native close, but no close event. */
		esp8266_release_client(client);
		return;
	}

	client->token = token;
	client->state = ESP8266_CLIENT_CONNECTED;
}

static void esp8266_socket_poll(uint16_t budget)
{
	uint16_t inspected = 0U;
	uint16_t emitted = 0U;

	if (!esp8266_events || budget == 0U)
	{
		return;
	}

	while (inspected < SOCKET_MAX_CONNECTIONS && emitted < budget)
	{
		uint16_t index = esp8266_poll_cursor;
		esp8266_client_t *client;

		esp8266_poll_cursor = (uint16_t)(
			(esp8266_poll_cursor + 1U) % SOCKET_MAX_CONNECTIONS);
		++inspected;
		client = &esp8266_clients[index];

		if (client->state == ESP8266_CLIENT_ACCEPT_PENDING)
		{
			esp8266_accept_in_owner_context(client);
			++emitted;
			continue;
		}
		if (client->state != ESP8266_CLIENT_CONNECTED)
		{
			continue;
		}

		/* Final retained bytes always precede FIN/reset notification. */
		if ((client->flags & ESP8266_CLIENT_CLOSE_PENDING) != 0U &&
			!client->rx)
		{
			int reason = client->close_reason;
			esp8266_finish_client(client, reason);
			++emitted;
			continue;
		}

		if ((client->flags & ESP8266_CLIENT_RX_EVENT) != 0U && client->rx)
		{
			socket_device_token_t token = client->token;
			client->flags &= (uint8_t)~ESP8266_CLIENT_RX_EVENT;
			esp8266_events->readable(token);
			++emitted;
			continue;
		}

		if ((client->flags & ESP8266_CLIENT_TX_EVENT) != 0U &&
			(client->flags & ESP8266_CLIENT_CLOSE_PENDING) == 0U)
		{
			socket_device_token_t token = client->token;
			/*
			 * Consume only this earned edge. TX_INTEREST may have been
			 * re-armed by a newer partial send while TX_EVENT was pending.
			 */
			client->flags &= (uint8_t)~ESP8266_CLIENT_TX_EVENT;
			esp8266_events->writable(token);
			++emitted;
		}
	}
}

socket_device_t wifi_socket = {
	.init = esp8266_socket_init,
	.listen = esp8266_socket_listen,
	.recv = esp8266_socket_recv,
	.send = esp8266_socket_send,
	.close = esp8266_socket_close,
	.poll = esp8266_socket_poll};

#endif /* ENABLE_SOCKETS */
#endif /* MCU == MCU_ESP8266 */
