/*
	Name: rp2350_lwip.c
	Description: Allocation-free uCNC TCP backend for the RP2350/Pico W
	             using the lwIP raw callback API.

	The generic socket core owns logical listeners, logical connections and the
	application callbacks. This backend owns lwIP PCBs, retained RX pbufs and the
	mapping from native handles to core generation tokens.

	lwIP callbacks never call the uCNC event sink. They only update bounded,
	static backend state. rp2350_socket_poll() later emits normalized events in
	the socket owner's context and observes the caller-provided event budget.

	No application buffer is retained. RX pbufs remain owned by this backend
	until recv() returns their bytes, and tcp_recved() acknowledges exactly the
	number of bytes returned to the core. TX uses TCP_WRITE_FLAG_COPY, therefore
	a positive send() result means that lwIP copied/accepted that prefix.

	Copyright: Copyright (c) Joao Martins
	Author: Joao Martins

	uCNC is free software: you can redistribute it and/or modify it under the
	terms of the GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later
	version. Please see <http://www.gnu.org/licenses/>.
*/

#include "../../../cnc.h"

#if (MCU == MCU_RP2350)

#ifdef __cplusplus
extern "C"
{
#endif

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "pico/cyw43_arch.h"
#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include "lwip/opt.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "../../../modules/net/socket.h"

/*
 * Number of complete pbuf chains retained per client.
 *
 * Only pointers are stored here; payload remains in lwIP-owned pbuf storage.
 * When this queue is full recv_cb() returns ERR_MEM without freeing the pbuf,
 * causing lwIP to retain/backpressure and retry it later.
 */
#ifndef RP2350_SOCKET_RX_QUEUE_DEPTH
#define RP2350_SOCKET_RX_QUEUE_DEPTH 4U
#endif

#if RP2350_SOCKET_RX_QUEUE_DEPTH == 0
#error "RP2350_SOCKET_RX_QUEUE_DEPTH must be greater than zero"
#endif

/* Handles encode a 16-bit generation plus a one-based native slot id. */
#if (MAX_SOCKETS + SOCKET_MAX_CONNECTIONS) >= UINT16_MAX
#error "RP2350 socket listener + client count must be below 65535"
#endif

typedef struct rp2350_listener_
{
	struct tcp_pcb *pcb;
	uint16_t generation;
	bool in_use;
} rp2350_listener_t;

typedef struct rp2350_client_
{
	struct tcp_pcb *pcb;
	struct pbuf *rx_queue[RP2350_SOCKET_RX_QUEUE_DEPTH];

	socket_device_handle_t handle;
	socket_device_handle_t listener_handle;
	socket_device_token_t token;

	uint16_t generation;
	uint16_t rx_offset;
	uint8_t rx_head;
	uint8_t rx_tail;
	uint8_t rx_count;

	int close_reason;

	bool in_use;
	bool accept_pending;
	bool readable_pending;
	bool writable_pending;
	bool want_write;
	bool close_pending;
} rp2350_client_t;

typedef enum rp2350_event_kind_
{
	RP2350_EVENT_NONE = 0,
	RP2350_EVENT_ACCEPTED,
	RP2350_EVENT_READABLE,
	RP2350_EVENT_WRITABLE,
	RP2350_EVENT_CLOSED
} rp2350_event_kind_t;

static rp2350_listener_t rp2350_listeners[MAX_SOCKETS];
static rp2350_client_t rp2350_clients[SOCKET_MAX_CONNECTIONS];
static const socket_device_events_t *rp2350_events;
static uint16_t rp2350_poll_cursor;

/* Raw-API callbacks. lwIP invokes each one while its core is protected. */
static err_t rp2350_accept_cb(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t rp2350_recv_cb(void *arg,
						  struct tcp_pcb *tpcb,
						  struct pbuf *p,
						  err_t err);
static err_t rp2350_sent_cb(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t rp2350_tcp_poll_cb(void *arg, struct tcp_pcb *tpcb);
static void rp2350_err_cb(void *arg, err_t err);

static uint16_t rp2350_next_generation(uint16_t generation)
{
	++generation;
	return generation == 0U ? 1U : generation;
}

static socket_device_handle_t rp2350_make_handle(uint16_t native_slot,
											  uint16_t generation)
{
	return ((socket_device_handle_t)generation << 16) |
		   (socket_device_handle_t)(native_slot + 1U);
}

static bool rp2350_decode_handle(socket_device_handle_t handle,
								 uint16_t *native_slot,
								 uint16_t *generation)
{
	uint16_t encoded_slot;

	if (handle == SOCKET_DEVICE_INVALID_HANDLE)
	{
		return false;
	}

	encoded_slot = (uint16_t)(handle & 0xFFFFU);
	if (encoded_slot == 0U)
	{
		return false;
	}

	*native_slot = (uint16_t)(encoded_slot - 1U);
	*generation = (uint16_t)((handle >> 16) & 0xFFFFU);
	return *generation != 0U;
}

static socket_device_handle_t rp2350_listener_handle(size_t index,
												  uint16_t generation)
{
	return rp2350_make_handle((uint16_t)index, generation);
}

static socket_device_handle_t rp2350_client_handle(size_t index,
											  uint16_t generation)
{
	return rp2350_make_handle((uint16_t)(MAX_SOCKETS + index), generation);
}

/* Must be called with lwIP core protection held. */
static rp2350_listener_t *rp2350_find_listener(socket_device_handle_t handle)
{
	uint16_t slot;
	uint16_t generation;

	if (!rp2350_decode_handle(handle, &slot, &generation) ||
		slot >= MAX_SOCKETS)
	{
		return NULL;
	}

	if (!rp2350_listeners[slot].in_use ||
		rp2350_listeners[slot].generation != generation)
	{
		return NULL;
	}

	return &rp2350_listeners[slot];
}

/* Must be called with lwIP core protection held. */
static rp2350_client_t *rp2350_find_client(socket_device_handle_t handle)
{
	uint16_t slot;
	uint16_t generation;
	uint16_t client_index;

	if (!rp2350_decode_handle(handle, &slot, &generation) ||
		slot < MAX_SOCKETS)
	{
		return NULL;
	}

	client_index = (uint16_t)(slot - MAX_SOCKETS);
	if (client_index >= SOCKET_MAX_CONNECTIONS ||
		!rp2350_clients[client_index].in_use ||
		rp2350_clients[client_index].generation != generation)
	{
		return NULL;
	}

	return &rp2350_clients[client_index];
}

/* Must be called with lwIP core protection held. */
static rp2350_listener_t *rp2350_alloc_listener(size_t *index_out)
{
	for (size_t i = 0; i < MAX_SOCKETS; ++i)
	{
		if (!rp2350_listeners[i].in_use)
		{
			uint16_t generation =
				rp2350_next_generation(rp2350_listeners[i].generation);

			memset(&rp2350_listeners[i], 0, sizeof(rp2350_listeners[i]));
			rp2350_listeners[i].generation = generation;
			rp2350_listeners[i].in_use = true;
			*index_out = i;
			return &rp2350_listeners[i];
		}
	}

	return NULL;
}

/* Must be called with lwIP core protection held. */
static rp2350_client_t *rp2350_alloc_client(size_t *index_out)
{
	for (size_t i = 0; i < SOCKET_MAX_CONNECTIONS; ++i)
	{
		if (!rp2350_clients[i].in_use)
		{
			uint16_t generation =
				rp2350_next_generation(rp2350_clients[i].generation);

			memset(&rp2350_clients[i], 0, sizeof(rp2350_clients[i]));
			rp2350_clients[i].generation = generation;
			rp2350_clients[i].handle =
				rp2350_client_handle(i, generation);
			rp2350_clients[i].listener_handle =
				SOCKET_DEVICE_INVALID_HANDLE;
			rp2350_clients[i].token = SOCKET_DEVICE_INVALID_TOKEN;
			rp2350_clients[i].close_reason = SOCKET_DEVICE_INVALID;
			rp2350_clients[i].in_use = true;
			*index_out = i;
			return &rp2350_clients[i];
		}
	}

	return NULL;
}

/* Must be called with lwIP core protection held. */
static void rp2350_clear_rx(rp2350_client_t *client)
{
	while (client->rx_count > 0U)
	{
		struct pbuf *p = client->rx_queue[client->rx_head];

		client->rx_queue[client->rx_head] = NULL;
		client->rx_head = (uint8_t)((client->rx_head + 1U) %
										RP2350_SOCKET_RX_QUEUE_DEPTH);
		--client->rx_count;
		if (p)
		{
			pbuf_free(p);
		}
	}

	client->rx_head = 0U;
	client->rx_tail = 0U;
	client->rx_offset = 0U;
}

/*
 * Releases a listener synchronously. The record is invalidated before native
 * close/abort, so a callback cannot act on it. Must hold lwIP protection.
 */
static void rp2350_release_listener(rp2350_listener_t *listener)
{
	struct tcp_pcb *pcb;
	uint16_t generation;

	if (!listener || !listener->in_use)
	{
		return;
	}

	pcb = listener->pcb;
	generation = listener->generation;
	listener->in_use = false;
	listener->pcb = NULL;

	if (pcb)
	{
		tcp_arg(pcb, NULL);
		tcp_accept(pcb, NULL);
		if (tcp_close(pcb) != ERR_OK)
		{
			tcp_abort(pcb);
		}
	}

	memset(listener, 0, sizeof(*listener));
	listener->generation = generation;
}

/*
 * Releases a client synchronously without emitting a core event. The caller
 * snapshots the token/reason first when a remote/fatal close must be reported.
 * Must hold lwIP protection.
 */
static void rp2350_release_client(rp2350_client_t *client)
{
	struct tcp_pcb *pcb;
	uint16_t generation;

	if (!client || !client->in_use)
	{
		return;
	}

	pcb = client->pcb;
	generation = client->generation;

	/* Invalidate first, then detach every callback before native teardown. */
	client->in_use = false;
	client->pcb = NULL;
	client->token = SOCKET_DEVICE_INVALID_TOKEN;
	client->accept_pending = false;
	client->readable_pending = false;
	client->writable_pending = false;
	client->want_write = false;
	client->close_pending = false;

	if (pcb)
	{
		tcp_arg(pcb, NULL);
		tcp_recv(pcb, NULL);
		tcp_sent(pcb, NULL);
		tcp_poll(pcb, NULL, 0U);
		tcp_err(pcb, NULL);

		if (tcp_close(pcb) != ERR_OK)
		{
			tcp_abort(pcb);
		}
	}

	rp2350_clear_rx(client);
	memset(client, 0, sizeof(*client));
	client->generation = generation;
	client->handle = SOCKET_DEVICE_INVALID_HANDLE;
	client->listener_handle = SOCKET_DEVICE_INVALID_HANDLE;
	client->token = SOCKET_DEVICE_INVALID_TOKEN;
	client->close_reason = SOCKET_DEVICE_INVALID;
}

/* Error mapping used for fatal callbacks and raw API failures. */
static int rp2350_fatal_result(err_t err)
{
	switch (err)
	{
	case ERR_CLSD:
		return SOCKET_DEVICE_CLOSED;
	case ERR_MEM:
		return SOCKET_DEVICE_NO_MEMORY;
	case ERR_ARG:
	case ERR_VAL:
		return SOCKET_DEVICE_INVALID;
	default:
		return SOCKET_DEVICE_ERROR;
	}
}

/* Called only from raw callbacks, therefore it does not emit a core event. */
static void rp2350_mark_close(rp2350_client_t *client, int reason)
{
	if (!client->close_pending)
	{
		client->close_reason = reason < 0 ? reason : SOCKET_DEVICE_ERROR;
		client->close_pending = true;
	}

	client->want_write = false;
	client->writable_pending = false;
}

#if defined(TCP_LISTEN_BACKLOG) && TCP_LISTEN_BACKLOG
static void rp2350_backlog_accepted(rp2350_listener_t *listener)
{
	if (listener && listener->in_use && listener->pcb)
	{
		tcp_accepted(listener->pcb);
	}
}
#else
#define rp2350_backlog_accepted(listener) ((void)(listener))
#endif

static err_t rp2350_accept_cb(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	rp2350_listener_t *listener = (rp2350_listener_t *)arg;
	rp2350_client_t *client;
	size_t listener_index;
	size_t client_index;

	if (err != ERR_OK)
	{
		return err;
	}

	if (!newpcb)
	{
		return ERR_ARG;
	}

	if (!listener || !listener->in_use || listener->pcb == NULL)
	{
		tcp_abort(newpcb);
		return ERR_ABRT;
	}

	listener_index = (size_t)(listener - rp2350_listeners);
	if (listener_index >= MAX_SOCKETS)
	{
		tcp_abort(newpcb);
		return ERR_ABRT;
	}

	client = rp2350_alloc_client(&client_index);
	if (!client)
	{
		/* The connection was processed but cannot enter our fixed pool. */
		rp2350_backlog_accepted(listener);
		tcp_abort(newpcb);
		return ERR_ABRT;
	}

	client->pcb = newpcb;
	client->listener_handle =
		rp2350_listener_handle(listener_index, listener->generation);
	client->accept_pending = true;

	tcp_setprio(newpcb, TCP_PRIO_NORMAL);
	tcp_arg(newpcb, client);
	tcp_recv(newpcb, rp2350_recv_cb);
	tcp_sent(newpcb, rp2350_sent_cb);
	tcp_err(newpcb, rp2350_err_cb);

	/* Coarse polling supplies a retry edge when no ACK is guaranteed. */
	tcp_poll(newpcb, rp2350_tcp_poll_cb, 1U);
	rp2350_backlog_accepted(listener);
	return ERR_OK;
}

static err_t rp2350_recv_cb(void *arg,
						  struct tcp_pcb *tpcb,
						  struct pbuf *p,
						  err_t err)
{
	rp2350_client_t *client = (rp2350_client_t *)arg;

	if (!client || !client->in_use || client->pcb != tpcb)
	{
		if (p)
		{
			pbuf_free(p);
		}
		tcp_abort(tpcb);
		return ERR_ABRT;
	}

	if (err != ERR_OK)
	{
		/* Make ownership deterministic for an unexpected fatal RX callback. */
		if (p)
		{
			pbuf_free(p);
		}
		client->pcb = NULL;
		rp2350_mark_close(client, rp2350_fatal_result(err));
		tcp_abort(tpcb);
		return ERR_ABRT;
	}

	/* p == NULL is FIN. Retained payload must be drained before close. */
	if (!p)
	{
		rp2350_mark_close(client, SOCKET_DEVICE_CLOSED);
		return ERR_OK;
	}

	if (p->tot_len == 0U)
	{
		pbuf_free(p);
		return ERR_OK;
	}

	if (client->rx_count >= RP2350_SOCKET_RX_QUEUE_DEPTH)
	{
		/* Do not free or acknowledge: lwIP keeps and retries this pbuf. */
		return ERR_MEM;
	}

	/* Returning ERR_OK transfers this pbuf to the application/backend. */
	client->rx_queue[client->rx_tail] = p;
	client->rx_tail = (uint8_t)((client->rx_tail + 1U) %
										 RP2350_SOCKET_RX_QUEUE_DEPTH);
	++client->rx_count;
	client->readable_pending = true;
	return ERR_OK;
}

static err_t rp2350_sent_cb(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	rp2350_client_t *client = (rp2350_client_t *)arg;
	(void)len;

	if (client && client->in_use && client->pcb == tpcb &&
		client->want_write && !client->close_pending)
	{
		client->want_write = false;
		client->writable_pending = true;
	}

	return ERR_OK;
}

static err_t rp2350_tcp_poll_cb(void *arg, struct tcp_pcb *tpcb)
{
	rp2350_client_t *client = (rp2350_client_t *)arg;

	if (client && client->in_use && client->pcb == tpcb &&
		client->want_write && !client->close_pending)
	{
		/* A hint only; send() may still return WOULD_BLOCK on the retry. */
		client->want_write = false;
		client->writable_pending = true;
	}

	return ERR_OK;
}

static void rp2350_err_cb(void *arg, err_t err)
{
	rp2350_client_t *client = (rp2350_client_t *)arg;

	if (!client || !client->in_use)
	{
		return;
	}

	/* lwIP has already freed the PCB before tcp_err() is invoked. */
	client->pcb = NULL;
	rp2350_mark_close(client, rp2350_fatal_result(err));

	/* Existing pbufs remain valid and are delivered before the close event. */
	if (client->rx_count > 0U)
	{
		client->readable_pending = true;
	}
}

static int rp2350_socket_init(const socket_device_events_t *events)
{
	if (!events || !events->accepted || !events->readable ||
		!events->writable || !events->closed)
	{
		return SOCKET_DEVICE_INVALID;
	}

	memset(rp2350_listeners, 0, sizeof(rp2350_listeners));
	memset(rp2350_clients, 0, sizeof(rp2350_clients));
	rp2350_events = events;
	rp2350_poll_cursor = 0U;
	return SOCKET_DEVICE_OK;
}

static socket_device_handle_t rp2350_socket_listen(
	const socket_device_endpoint_t *endpoint,
	uint8_t backlog)
{
	rp2350_listener_t *listener;
	struct tcp_pcb *pcb;
	struct tcp_pcb *listen_pcb;
	ip_addr_t bind_address;
	socket_device_handle_t handle = SOCKET_DEVICE_INVALID_HANDLE;
	size_t listener_index;
	err_t err;

	if (!endpoint || endpoint->port == 0U)
	{
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	/* A zero backlog still maps to one pending native connection. */
	if (backlog == 0U)
	{
		backlog = 1U;
	}
	if (backlog > SOCKET_MAX_CLIENTS)
	{
		backlog = SOCKET_MAX_CLIENTS;
	}

	cyw43_arch_lwip_begin();
	listener = rp2350_alloc_listener(&listener_index);
	if (!listener)
	{
		cyw43_arch_lwip_end();
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
	if (!pcb)
	{
		rp2350_release_listener(listener);
		cyw43_arch_lwip_end();
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	listener->pcb = pcb;
	tcp_setprio(pcb, TCP_PRIO_NORMAL);
	pcb->so_options |= SOF_REUSEADDR;

	if (endpoint->address == IP_ANY)
	{
		ip_addr_set_any(IPADDR_TYPE_V4, &bind_address);
	}
	else
	{
		/* endpoint->address is the documented host-order 0xAABBCCDD. */
		IP_ADDR4(&bind_address,
				 (uint8_t)(endpoint->address >> 24),
				 (uint8_t)(endpoint->address >> 16),
				 (uint8_t)(endpoint->address >> 8),
				 (uint8_t)endpoint->address);
	}

	err = tcp_bind(pcb, &bind_address, endpoint->port);
	if (err != ERR_OK)
	{
		rp2350_release_listener(listener);
		cyw43_arch_lwip_end();
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	listen_pcb = tcp_listen_with_backlog(pcb, backlog);
	if (!listen_pcb)
	{
		/* On failure lwIP leaves the original bound PCB with the caller. */
		rp2350_release_listener(listener);
		cyw43_arch_lwip_end();
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	listener->pcb = listen_pcb;
	tcp_arg(listen_pcb, listener);
	tcp_accept(listen_pcb, rp2350_accept_cb);
	handle = rp2350_listener_handle(listener_index, listener->generation);
	cyw43_arch_lwip_end();
	return handle;
}

static int rp2350_socket_recv(socket_device_handle_t handle,
								void *destination,
								size_t capacity)
{
	rp2350_client_t *client;
	uint8_t *output = (uint8_t *)destination;
	size_t copied_total = 0U;
	size_t limit;
	socket_device_token_t close_token = SOCKET_DEVICE_INVALID_TOKEN;
	int close_reason = SOCKET_DEVICE_INVALID;

	if (capacity == 0U)
	{
		return 0;
	}
	if (!destination)
	{
		return SOCKET_DEVICE_INVALID;
	}

	limit = capacity > (size_t)INT_MAX ? (size_t)INT_MAX : capacity;
	cyw43_arch_lwip_begin();
	client = rp2350_find_client(handle);
	if (!client || client->token == SOCKET_DEVICE_INVALID_TOKEN)
	{
		cyw43_arch_lwip_end();
		return SOCKET_DEVICE_INVALID;
	}

	while (copied_total < limit && client->rx_count > 0U)
	{
		struct pbuf *p = client->rx_queue[client->rx_head];
		size_t remaining;
		size_t wanted;
		u16_t copied;

		if (!p || client->rx_offset >= p->tot_len)
		{
			close_token = client->token;
			close_reason = SOCKET_DEVICE_ERROR;
			rp2350_release_client(client);
			cyw43_arch_lwip_end();
			rp2350_events->closed(close_token, close_reason);
			return close_reason;
		}

		remaining = (size_t)p->tot_len - client->rx_offset;
		wanted = limit - copied_total;
		if (wanted > remaining)
		{
			wanted = remaining;
		}

		copied = pbuf_copy_partial(p,
								 output + copied_total,
								 (u16_t)wanted,
								 client->rx_offset);
		if (copied == 0U)
		{
			close_token = client->token;
			close_reason = SOCKET_DEVICE_ERROR;
			rp2350_release_client(client);
			cyw43_arch_lwip_end();
			rp2350_events->closed(close_token, close_reason);
			return close_reason;
		}

		client->rx_offset = (uint16_t)(client->rx_offset + copied);
		copied_total += copied;
		if (client->pcb)
		{
			/* Acknowledge only bytes actually copied to the core. */
			tcp_recved(client->pcb, copied);
		}

		if (client->rx_offset == p->tot_len)
		{
			client->rx_queue[client->rx_head] = NULL;
			client->rx_head = (uint8_t)((client->rx_head + 1U) %
											RP2350_SOCKET_RX_QUEUE_DEPTH);
			--client->rx_count;
			client->rx_offset = 0U;
			pbuf_free(p);
		}
	}

	if (copied_total > 0U)
	{
		/* The final positive RX result must be dispatched before close. */
		cyw43_arch_lwip_end();
		return (int)copied_total;
	}

	if (client->close_pending)
	{
		close_token = client->token;
		close_reason = client->close_reason;
		rp2350_release_client(client);
		cyw43_arch_lwip_end();
		rp2350_events->closed(close_token, close_reason);
		return close_reason;
	}

	cyw43_arch_lwip_end();
	return SOCKET_DEVICE_WOULD_BLOCK;
}

static int rp2350_socket_send(socket_device_handle_t handle,
								const void *source,
								size_t length)
{
	rp2350_client_t *client;
	size_t attempt;
	u16_t available;
	err_t err;
	socket_device_token_t close_token = SOCKET_DEVICE_INVALID_TOKEN;
	int close_reason;

	if (length == 0U)
	{
		return 0;
	}
	if (!source)
	{
		return SOCKET_DEVICE_INVALID;
	}

	cyw43_arch_lwip_begin();
	client = rp2350_find_client(handle);
	if (!client || client->token == SOCKET_DEVICE_INVALID_TOKEN)
	{
		cyw43_arch_lwip_end();
		return SOCKET_DEVICE_INVALID;
	}

	if (!client->pcb)
	{
		close_token = client->token;
		close_reason = client->close_pending ?
			client->close_reason : SOCKET_DEVICE_CLOSED;
		rp2350_release_client(client);
		cyw43_arch_lwip_end();
		rp2350_events->closed(close_token, close_reason);
		return close_reason;
	}

	available = tcp_sndbuf(client->pcb);
	if (available == 0U)
	{
		client->want_write = true;
		client->writable_pending = false;
		cyw43_arch_lwip_end();
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	attempt = length;
	if (attempt > available)
	{
		attempt = available;
	}

	err = tcp_write(client->pcb,
					(const uint8_t *)source,
					(u16_t)attempt,
					TCP_WRITE_FLAG_COPY);
	if (err == ERR_MEM)
	{
		client->want_write = true;
		client->writable_pending = false;
		cyw43_arch_lwip_end();
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	if (err != ERR_OK)
	{
		close_token = client->token;
		close_reason = rp2350_fatal_result(err);
		rp2350_release_client(client);
		cyw43_arch_lwip_end();
		rp2350_events->closed(close_token, close_reason);
		return close_reason;
	}

	/* Partial success also arms a later writable transition. */
	client->want_write = attempt < length;
	client->writable_pending = false;

	/* tcp_output() is non-blocking. tcp_write() already accepted the data. */
	(void)tcp_output(client->pcb);
	cyw43_arch_lwip_end();
	return (int)attempt;
}

static int rp2350_socket_close(socket_device_handle_t handle)
{
	rp2350_listener_t *listener;
	rp2350_client_t *client;

	cyw43_arch_lwip_begin();
	listener = rp2350_find_listener(handle);
	if (listener)
	{
		rp2350_release_listener(listener);
		cyw43_arch_lwip_end();
		return SOCKET_DEVICE_OK;
	}

	client = rp2350_find_client(handle);
	if (client)
	{
		/* Local closure deliberately emits no events->closed() callback. */
		rp2350_release_client(client);
		cyw43_arch_lwip_end();
		return SOCKET_DEVICE_OK;
	}

	cyw43_arch_lwip_end();
	return SOCKET_DEVICE_INVALID;
}

static void rp2350_socket_poll(uint16_t budget)
{
	uint16_t emitted = 0U;

	/* Poll-mode Pico SDK builds require explicit CYW43/lwIP progress. */
#if defined(PICO_CYW43_ARCH_POLL) && PICO_CYW43_ARCH_POLL
	cyw43_arch_poll();
#endif

	while (emitted < budget)
	{
		rp2350_event_kind_t event = RP2350_EVENT_NONE;
		rp2350_client_t *selected = NULL;
		socket_device_handle_t client_handle = SOCKET_DEVICE_INVALID_HANDLE;
		socket_device_handle_t listener_handle = SOCKET_DEVICE_INVALID_HANDLE;
		socket_device_token_t token = SOCKET_DEVICE_INVALID_TOKEN;
		int reason = SOCKET_DEVICE_INVALID;

		/* Find at most one event in round-robin order. */
		cyw43_arch_lwip_begin();
		for (uint16_t checked = 0U;
			 checked < SOCKET_MAX_CONNECTIONS;
			 ++checked)
		{
			uint16_t index = rp2350_poll_cursor;
			rp2350_client_t *client = &rp2350_clients[index];

			rp2350_poll_cursor =
				(uint16_t)((rp2350_poll_cursor + 1U) %
						   SOCKET_MAX_CONNECTIONS);

			if (!client->in_use)
			{
				continue;
			}

			if (client->accept_pending)
			{
				client->accept_pending = false;
				event = RP2350_EVENT_ACCEPTED;
				selected = client;
				client_handle = client->handle;
				listener_handle = client->listener_handle;
				break;
			}

			if (client->token == SOCKET_DEVICE_INVALID_TOKEN)
			{
				continue;
			}

			/* Final payload always precedes remote/fatal closure. */
			if (client->rx_count > 0U && client->readable_pending)
			{
				client->readable_pending = false;
				event = RP2350_EVENT_READABLE;
				token = client->token;
				break;
			}

			if (client->close_pending && client->rx_count == 0U)
			{
				event = RP2350_EVENT_CLOSED;
				token = client->token;
				reason = client->close_reason;
				rp2350_release_client(client);
				break;
			}

			if (!client->close_pending && client->writable_pending)
			{
				client->writable_pending = false;
				event = RP2350_EVENT_WRITABLE;
				token = client->token;
				break;
			}
		}
		cyw43_arch_lwip_end();

		if (event == RP2350_EVENT_NONE)
		{
			break;
		}

		switch (event)
		{
		case RP2350_EVENT_ACCEPTED:
		{
			socket_device_token_t accepted_token =
				rp2350_events->accepted(listener_handle, client_handle);

			/* Store the token before allowing any later event to be emitted. */
			cyw43_arch_lwip_begin();
			if (selected->in_use && selected->handle == client_handle &&
				selected->token == SOCKET_DEVICE_INVALID_TOKEN)
			{
				if (accepted_token == SOCKET_DEVICE_INVALID_TOKEN)
				{
					/* Core rejection: local release, never emit closed(). */
					rp2350_release_client(selected);
				}
				else
				{
					selected->token = accepted_token;
					if (selected->rx_count > 0U)
					{
						selected->readable_pending = true;
					}
				}
			}
			cyw43_arch_lwip_end();
			break;
		}

		case RP2350_EVENT_READABLE:
			rp2350_events->readable(token);
			break;

		case RP2350_EVENT_WRITABLE:
			rp2350_events->writable(token);
			break;

		case RP2350_EVENT_CLOSED:
			rp2350_events->closed(token, reason);
			break;

		default:
			break;
		}

		++emitted;
	}
}

/* Public backend object registered by the RP2350 network initialization. */
socket_device_t wifi_socket = {
	.init = rp2350_socket_init,
	.listen = rp2350_socket_listen,
	.recv = rp2350_socket_recv,
	.send = rp2350_socket_send,
	.close = rp2350_socket_close,
	.poll = rp2350_socket_poll};

#ifdef __cplusplus
}
#endif

#endif /* MCU == MCU_RP2350 */
