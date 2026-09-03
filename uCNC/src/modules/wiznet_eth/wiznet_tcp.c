/*
 * Allocation-free socket_device_t backend for WIZnet W5100/W5200/W5500.
 *
 * Historical note: this file keeps its existing wiznet_lwip.c name for build
 * compatibility, but it does not use lwIP.  TCP is implemented by the WIZnet
 * hardware engine and this file is only a thin adapter to uCNC's current
 * allocation-free socket_device_t contract.
 *
 * A WIZnet TCP listening socket becomes the connected socket after passive
 * open.  A logical listener is therefore a small static record.  When its
 * hardware socket accepts, that hardware slot is promoted to a client and
 * poll() later opens one replacement listening socket when a free hardware
 * slot exists.
 *
 * TX policy is intentionally minimal:
 *  - no application TX queue exists;
 *  - send() performs at most one hardware SEND submission;
 *  - once a SEND command is outstanding, later send() calls return
 *    SOCKET_DEVICE_WOULD_BLOCK until SEND_OK is observed;
 *  - poll() (or a later send()) clears that native in-flight state, but no
 *    writable event exists in the current uCNC contract.
 *
 * The WIZnet RX/TX rings are the only payload storage.  RX bytes are copied
 * directly from the hardware ring into the core's borrowed destination and
 * committed only after the copy.  Remote closure is retained until the
 * hardware RX ring has drained, so final payload precedes closed().
 *
 * The driver is synchronous SPI and has no native ISR/callback producer.
 * uCNC's socket core is the only owner, so no backend lock or event queue is
 * required.  The low-level WIZnet helpers perform only bounded register/SPI
 * operations; this backend never waits for RX, TX capacity, SEND_OK, or a peer
 * acknowledgement.
 */

#include "wiznet_ethernet.h"
#include "../net/socket.h"

#include <limits.h>
#include <string.h>

#if MAX_SOCKETS < WIZNET_MAX_HW_SOCKETS
#define WIZNET_MAX_LISTENERS MAX_SOCKETS
#else
#define WIZNET_MAX_LISTENERS WIZNET_MAX_HW_SOCKETS
#endif

#if WIZNET_MAX_HW_SOCKETS != 8U
#error "WIZnet backend handle encoding assumes exactly eight hardware sockets"
#endif

/*
 * Native socket interrupt bits shared by W5100/W5200/W5500.
 */
enum
{
	WIZ_SNIR_CON = 0x01U,
	WIZ_SNIR_DISCON = 0x02U,
	WIZ_SNIR_RECV = 0x04U,
	WIZ_SNIR_TIMEOUT = 0x08U,
	WIZ_SNIR_SEND_OK = 0x10U
};

/*
 * TCP socket states shared by W5100/W5200/W5500.
 */
enum
{
	WIZ_SNSR_CLOSED = 0x00U,
	WIZ_SNSR_LISTEN = 0x14U,
	WIZ_SNSR_SYNRECV = 0x16U,
	WIZ_SNSR_ESTABLISHED = 0x17U,
	WIZ_SNSR_FIN_WAIT = 0x18U,
	WIZ_SNSR_CLOSING = 0x1AU,
	WIZ_SNSR_TIME_WAIT = 0x1BU,
	WIZ_SNSR_CLOSE_WAIT = 0x1CU,
	WIZ_SNSR_LAST_ACK = 0x1DU
};

/*
 * role occupies the low two state bits.  The remaining bits are independent
 * flags.  Packing them into one byte keeps one hardware record at eight bytes
 * on normal 32-bit ABIs (token 4 + generation 2 + two state bytes).
 */
enum
{
	WIZ_ROLE_FREE = 0U,
	WIZ_ROLE_LISTENER = 1U,
	WIZ_ROLE_CLIENT = 2U,
	WIZ_ROLE_MASK = 0x03U,

	/* A SEND command owns the socket; another SEND is illegal before SEND_OK. */
	WIZ_FLAG_TX_PENDING = 1U << 2,
	/* readable() has been emitted; core keeps draining until recv() blocks. */
	WIZ_FLAG_RX_REPORTED = 1U << 3,
	/* FIN/fatal closure is retained until the hardware RX ring is empty. */
	WIZ_FLAG_CLOSE_PENDING = 1U << 4,
	/* CLOSE_PENDING is fatal rather than an orderly peer close. */
	WIZ_FLAG_CLOSE_ERROR = 1U << 5
};

/*
 * Handles are values, not pointers:
 *   bits 0..2 : hardware/listener table index
 *   bit  3    : 0 client, 1 logical listener
 *   bits 4..  : allocation generation
 *
 * The generation makes a locally closed handle fail after the hardware/table
 * slot is reused.  uint16_t generation keeps the record compact.  On 16-bit
 * pointer targets the top representable generation is skipped so
 * SOCKET_DEVICE_INVALID_HANDLE (UINTPTR_MAX) can never be generated.
 */
#define WIZ_HANDLE_INDEX_MASK ((uintptr_t)0x07U)
#define WIZ_HANDLE_LISTENER_BIT ((uintptr_t)0x08U)
#define WIZ_HANDLE_GENERATION_SHIFT 4U

typedef struct wiz_hw_slot_
{
	socket_device_token_t token;
	uint16_t generation;
	uint8_t listener_index;
	uint8_t state;
} wiz_hw_slot_t;

/*
 * port==0 marks a free listener.  Port zero is invalid for listen(), so no
 * separate used flag is required.  Generation persists while the record is
 * free and changes on every new allocation.
 */
typedef struct wiz_listener_
{
	uint16_t port;
	uint16_t generation;
} wiz_listener_t;

static const socket_device_events_t *backend_events;
static wiz_hw_slot_t hw_slots[WIZNET_MAX_HW_SOCKETS];
static wiz_listener_t listeners[WIZNET_MAX_LISTENERS];
static uint8_t service_cursor;
static uint8_t listener_cursor;

static uint8_t slot_role(const wiz_hw_slot_t *slot)
{
	return (uint8_t)(slot->state & WIZ_ROLE_MASK);
}

static void slot_set_role(wiz_hw_slot_t *slot, uint8_t role)
{
	slot->state = (uint8_t)((slot->state & (uint8_t)~WIZ_ROLE_MASK) |
						   (role & WIZ_ROLE_MASK));
}

static uint16_t next_handle_generation(uint16_t generation)
{
	uintptr_t native_limit = UINTPTR_MAX >> WIZ_HANDLE_GENERATION_SHIFT;
	uint16_t limit;

	/*
	 * Reserve generation zero and the largest representable generation.  The
	 * latter prevents an index/type nibble of 0xF from ever producing
	 * SOCKET_DEVICE_INVALID_HANDLE on a 16-bit uintptr_t target.
	 */
	if (native_limit > (uintptr_t)UINT16_MAX)
	{
		limit = UINT16_MAX;
	}
	else
	{
		limit = (uint16_t)native_limit;
	}

	++generation;
	if (generation == 0U || generation >= limit)
	{
		generation = 1U;
	}
	return generation;
}

static socket_device_handle_t make_handle(uint8_t index,
										  bool listener,
										  uint16_t generation)
{
	uintptr_t value = ((uintptr_t)generation << WIZ_HANDLE_GENERATION_SHIFT) |
					  (uintptr_t)(index & (uint8_t)WIZ_HANDLE_INDEX_MASK);

	if (listener)
	{
		value |= WIZ_HANDLE_LISTENER_BIT;
	}
	return (socket_device_handle_t)value;
}

static bool decode_handle(socket_device_handle_t handle,
						  bool listener,
						  uint8_t *index,
						  uint16_t *generation)
{
	uintptr_t value = (uintptr_t)handle;
	uintptr_t native_generation;

	if (handle == SOCKET_DEVICE_INVALID_HANDLE)
	{
		return false;
	}
	if (((value & WIZ_HANDLE_LISTENER_BIT) != 0U) != listener)
	{
		return false;
	}

	native_generation = value >> WIZ_HANDLE_GENERATION_SHIFT;
	if (native_generation == 0U ||
		native_generation > (uintptr_t)UINT16_MAX)
	{
		return false;
	}

	if (index != NULL)
	{
		*index = (uint8_t)(value & WIZ_HANDLE_INDEX_MASK);
	}
	if (generation != NULL)
	{
		*generation = (uint16_t)native_generation;
	}
	return true;
}

static socket_device_handle_t listener_handle(uint8_t index)
{
	return make_handle(index, true, listeners[index].generation);
}

static socket_device_handle_t client_handle(uint8_t socket_number)
{
	return make_handle(socket_number, false, hw_slots[socket_number].generation);
}

static int listener_index_from_handle(socket_device_handle_t handle)
{
	uint8_t index;
	uint16_t generation;

	if (!decode_handle(handle, true, &index, &generation) ||
		index >= WIZNET_MAX_LISTENERS ||
		listeners[index].port == 0U ||
		listeners[index].generation != generation)
	{
		return -1;
	}
	return (int)index;
}

static int client_index_from_handle(socket_device_handle_t handle)
{
	uint8_t index;
	uint16_t generation;
	uint8_t count;

	if (!decode_handle(handle, false, &index, &generation))
	{
		return -1;
	}

	count = wiznet_hw_socket_count();
	if (count > WIZNET_MAX_HW_SOCKETS)
	{
		count = WIZNET_MAX_HW_SOCKETS;
	}
	if (index >= count ||
		slot_role(&hw_slots[index]) != WIZ_ROLE_CLIENT ||
		hw_slots[index].token == SOCKET_DEVICE_INVALID_TOKEN ||
		hw_slots[index].generation != generation)
	{
		return -1;
	}
	return (int)index;
}

static bool status_can_send(uint8_t status)
{
	/*
	 * CLOSE_WAIT is TCP half-close: the peer has sent FIN but WIZnet still
	 * permits local transmission until the host closes the socket.
	 */
	return status == WIZ_SNSR_ESTABLISHED ||
		   status == WIZ_SNSR_CLOSE_WAIT;
}

static bool status_is_closing(uint8_t status)
{
	return status == WIZ_SNSR_CLOSED ||
		   status == WIZ_SNSR_FIN_WAIT ||
		   status == WIZ_SNSR_CLOSING ||
		   status == WIZ_SNSR_TIME_WAIT ||
		   status == WIZ_SNSR_CLOSE_WAIT ||
		   status == WIZ_SNSR_LAST_ACK;
}

static bool status_is_known_client(uint8_t status)
{
	return status_can_send(status) || status_is_closing(status);
}

static void reset_hw_slot(uint8_t socket_number)
{
	uint16_t generation = hw_slots[socket_number].generation;

	memset(&hw_slots[socket_number], 0, sizeof(hw_slots[socket_number]));
	hw_slots[socket_number].generation = generation;
	hw_slots[socket_number].token = SOCKET_DEVICE_INVALID_TOKEN;
	hw_slots[socket_number].listener_index = UINT8_MAX;
	slot_set_role(&hw_slots[socket_number], WIZ_ROLE_FREE);
}

static int find_free_hw_socket(void)
{
	uint8_t i;
	uint8_t count = wiznet_hw_socket_count();

	if (count > WIZNET_MAX_HW_SOCKETS)
	{
		count = WIZNET_MAX_HW_SOCKETS;
	}
	for (i = 0U; i < count; ++i)
	{
		if (slot_role(&hw_slots[i]) == WIZ_ROLE_FREE)
		{
			return (int)i;
		}
	}
	return -1;
}

static bool listener_has_hw_socket(uint8_t listener_index)
{
	uint8_t i;
	uint8_t count = wiznet_hw_socket_count();

	if (count > WIZNET_MAX_HW_SOCKETS)
	{
		count = WIZNET_MAX_HW_SOCKETS;
	}
	for (i = 0U; i < count; ++i)
	{
		if (slot_role(&hw_slots[i]) == WIZ_ROLE_LISTENER &&
			hw_slots[i].listener_index == listener_index)
		{
			return true;
		}
	}
	return false;
}

/*
 * Close and invalidate a hardware slot without emitting a core event.  State is
 * invalidated before touching the chip so a nested/stale owner call cannot
 * resolve the old handle even if the bounded CLOSE transaction takes time.
 */
static void release_hw_socket(uint8_t socket_number)
{
	reset_hw_slot(socket_number);
	wiznet_hw_socket_close(socket_number);
}

/*
 * Remote/fatal close notification.  The backend releases the native socket
 * before invalidating the core token, as required by socket_device_t.
 */
static void notify_client_closed(uint8_t socket_number, int reason)
{
	socket_device_token_t token = hw_slots[socket_number].token;

	release_hw_socket(socket_number);
	if (token != SOCKET_DEVICE_INVALID_TOKEN)
	{
		backend_events->closed(token, reason);
	}
}

static void mark_client_close(uint8_t socket_number, int reason)
{
	hw_slots[socket_number].state |= WIZ_FLAG_CLOSE_PENDING;
	if (reason != SOCKET_DEVICE_CLOSED)
	{
		/* A fatal condition always wins over an earlier orderly FIN marker. */
		hw_slots[socket_number].state |= WIZ_FLAG_CLOSE_ERROR;
	}
}

static int pending_close_reason(const wiz_hw_slot_t *slot)
{
	return (slot->state & WIZ_FLAG_CLOSE_ERROR) != 0U
			   ? SOCKET_DEVICE_ERROR
			   : SOCKET_DEVICE_CLOSED;
}

/*
 * Observe interrupt/status state without emitting an event.  This helper is
 * safe from poll(), recv() and send(), all of which execute in the core owner
 * context.  SEND_OK only frees the native one-command-at-a-time gate.
 */
static void observe_client_state(uint8_t socket_number,
								 uint8_t status,
								 uint8_t interrupts)
{
	wiz_hw_slot_t *slot = &hw_slots[socket_number];

	if ((interrupts & WIZ_SNIR_TIMEOUT) != 0U)
	{
		/*
		 * Log before clearing Sn_IR: after a WIZnet TCP timeout the chip can
		 * already have moved Sn_SR to CLOSED, so the later low-level CLOSE log
		 * alone cannot distinguish a native timeout from local cleanup.
		 */
		WIZDGB("WIZnet: socket %u native TIMEOUT, status=0x%X ir=0x%X tx_pending=%u\n",
			   (unsigned int)socket_number, (unsigned int)status,
			   (unsigned int)interrupts,
			   (slot->state & WIZ_FLAG_TX_PENDING) != 0U ? 1U : 0U);
		wiznet_hw_socket_clear_interrupt(socket_number, WIZ_SNIR_TIMEOUT);
		mark_client_close(socket_number, SOCKET_DEVICE_ERROR);
	}

	if ((interrupts & WIZ_SNIR_DISCON) != 0U)
	{
		WIZDGB("WIZnet: socket %u peer DISCON, status=0x%X ir=0x%X tx_pending=%u\n",
			   (unsigned int)socket_number, (unsigned int)status,
			   (unsigned int)interrupts,
			   (slot->state & WIZ_FLAG_TX_PENDING) != 0U ? 1U : 0U);
		wiznet_hw_socket_clear_interrupt(socket_number, WIZ_SNIR_DISCON);
		mark_client_close(socket_number, SOCKET_DEVICE_CLOSED);
	}

	if (status_is_closing(status))
	{
		if (status == WIZ_SNSR_CLOSED &&
			(interrupts & (WIZ_SNIR_TIMEOUT | WIZ_SNIR_DISCON)) == 0U &&
			(slot->state & WIZ_FLAG_CLOSE_PENDING) == 0U)
		{
			WIZDGB("WIZnet: socket %u became CLOSED without TIMEOUT/DISCON, ir=0x%X tx_pending=%u\n",
				   (unsigned int)socket_number, (unsigned int)interrupts,
				   (slot->state & WIZ_FLAG_TX_PENDING) != 0U ? 1U : 0U);
		}
		mark_client_close(socket_number, SOCKET_DEVICE_CLOSED);
	}
	else if (!status_is_known_client(status))
	{
		WIZDGB("WIZnet: socket %u invalid client status=0x%X ir=0x%X tx_pending=%u\n",
			   (unsigned int)socket_number, (unsigned int)status,
			   (unsigned int)interrupts,
			   (slot->state & WIZ_FLAG_TX_PENDING) != 0U ? 1U : 0U);
		mark_client_close(socket_number, SOCKET_DEVICE_ERROR);
	}

	if ((interrupts & WIZ_SNIR_SEND_OK) != 0U)
	{
		/*
		 * W5100/W5500 completion is just SEND_OK.  W5200 needs the vendor's
		 * extra completion check/re-SEND workaround, implemented in the
		 * low-level helper without waiting for another network event.
		 */
		if ((slot->state & WIZ_FLAG_TX_PENDING) != 0U &&
			(slot->state & WIZ_FLAG_CLOSE_ERROR) == 0U &&
			status_can_send(status))
		{
			int progress = wiznet_hw_socket_send_progress(socket_number);

			if (progress > 0)
			{
				WIZDGB("WIZnet: socket %u SEND_OK, TX command complete\n",
					   (unsigned int)socket_number);
				slot->state &= (uint8_t)~WIZ_FLAG_TX_PENDING;
			}
			else if (progress < 0 && progress != WIZNET_HW_RETRY)
			{
				WIZDGB("WIZnet: socket %u SEND completion housekeeping failed (%d)\n",
					   (unsigned int)socket_number, progress);
				mark_client_close(socket_number, SOCKET_DEVICE_ERROR);
			}
		}
		else
		{
			/* No live SEND owns this edge, or the connection is already fatal. */
			wiznet_hw_socket_clear_interrupt(socket_number, WIZ_SNIR_SEND_OK);
		}
	}
}

/*
 * Finish a previously observed close only after RX is known to be empty.
 *
 * Returns:
 *   SOCKET_DEVICE_WOULD_BLOCK while retained RX may still exist;
 *   SOCKET_DEVICE_CLOSED / SOCKET_DEVICE_ERROR after emitting closed().
 *
 * An incoherent volatile RX_RSR sample is backpressure, not proof of emptiness.
 */
static int finish_close_if_rx_drained(uint8_t socket_number)
{
	wiz_hw_slot_t *slot = &hw_slots[socket_number];
	int available;
	int reason;

	if ((slot->state & WIZ_FLAG_CLOSE_PENDING) == 0U)
	{
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	available = wiznet_hw_socket_rx_available(socket_number);
	if (available > 0 || available == WIZNET_HW_RETRY)
	{
		return SOCKET_DEVICE_WOULD_BLOCK;
	}
	if (available < 0)
	{
		slot->state |= WIZ_FLAG_CLOSE_ERROR;
	}

	reason = pending_close_reason(slot);
	notify_client_closed(socket_number, reason);
	return reason;
}

static bool open_listener_socket(uint8_t listener_index)
{
	int socket_number;

	if (listener_index >= WIZNET_MAX_LISTENERS ||
		listeners[listener_index].port == 0U ||
		listener_has_hw_socket(listener_index))
	{
		return true;
	}

	socket_number = find_free_hw_socket();
	if (socket_number < 0)
	{
		return false;
	}

	/*
	 * The low-level open path starts by closing the chosen hardware slot and
	 * clearing all stale Sn_IR bits.  Only publish backend ownership after the
	 * complete OPEN/LISTEN sequence succeeds.
	 */
	if (!wiznet_hw_socket_open_tcp_server((uint8_t)socket_number,
										 listeners[listener_index].port))
	{
		reset_hw_slot((uint8_t)socket_number);
		return false;
	}

	hw_slots[socket_number].token = SOCKET_DEVICE_INVALID_TOKEN;
	hw_slots[socket_number].listener_index = listener_index;
	hw_slots[socket_number].state = WIZ_ROLE_LISTENER;
	return true;
}

static int wiz_backend_init(const socket_device_events_t *events)
{
	uint8_t i;
	uint8_t count;

	if (!wiznet_is_ready() ||
		events == NULL ||
		events->accepted == NULL ||
		events->readable == NULL ||
		events->closed == NULL)
	{
		return SOCKET_DEVICE_INVALID;
	}

	memset(listeners, 0, sizeof(listeners));
	memset(hw_slots, 0, sizeof(hw_slots));
	for (i = 0U; i < WIZNET_MAX_HW_SOCKETS; ++i)
	{
		reset_hw_slot(i);
	}

	count = wiznet_hw_socket_count();
	if (count == 0U || count > WIZNET_MAX_HW_SOCKETS)
	{
		return SOCKET_DEVICE_ERROR;
	}

	/*
	 * wiznet_init() owns chip/network initialization.  Backend init only
	 * normalizes socket resources to CLOSED and clears stale interrupt state.
	 */
	for (i = 0U; i < count; ++i)
	{
		wiznet_hw_socket_close(i);
	}

	service_cursor = 0U;
	listener_cursor = 0U;
	backend_events = events;
	return SOCKET_DEVICE_OK;
}

static uint32_t ipv4_to_host_order(ipv4_address_t address)
{
	return ((uint32_t)address.octets[0] << 24) |
		   ((uint32_t)address.octets[1] << 16) |
		   ((uint32_t)address.octets[2] << 8) |
		   (uint32_t)address.octets[3];
}

static socket_device_handle_t wiz_backend_listen(
	const socket_device_endpoint_t *endpoint,
	uint8_t backlog)
{
	ipv4_address_t local_ip;
	uint8_t i;

	/*
	 * WIZnet passive open has no software accept queue: one hardware LISTEN
	 * socket accepts one peer and becomes that client.  The backend then opens a
	 * replacement listener in another free hardware slot.  Thus the immediate
	 * native backlog is one; backlog==0 is still treated as one bounded slot.
	 */
	(void)backlog;

	if (!wiznet_is_ready() ||
		endpoint == NULL ||
		endpoint->port == 0U)
	{
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	local_ip = wiznet_get_ip();
	if (endpoint->address != 0U && endpoint->address != ipv4_to_host_order(local_ip))
	{
		return SOCKET_DEVICE_INVALID_HANDLE;
	}

	/* Match bind semantics: one logical listener owns a TCP port. */
	for (i = 0U; i < WIZNET_MAX_LISTENERS; ++i)
	{
		if (listeners[i].port == endpoint->port)
		{
			return SOCKET_DEVICE_INVALID_HANDLE;
		}
	}

	for (i = 0U; i < WIZNET_MAX_LISTENERS; ++i)
	{
		if (listeners[i].port == 0U)
		{
			listeners[i].generation =
				next_handle_generation(listeners[i].generation);
			listeners[i].port = endpoint->port;

			if (!open_listener_socket(i))
			{
				/* Keep generation advanced so a failed/stale value cannot be
				 * revived by the next successful allocation. */
				listeners[i].port = 0U;
				return SOCKET_DEVICE_INVALID_HANDLE;
			}
			return listener_handle(i);
		}
	}

	return SOCKET_DEVICE_INVALID_HANDLE;
}

static int wiz_backend_recv(socket_device_handle_t handle,
							void *destination,
							size_t capacity)
{
	int index = client_index_from_handle(handle);
	uint8_t socket_number;
	uint8_t status;
	uint8_t interrupts;
	int received;
	int commit_result;

	if (index < 0)
	{
		return SOCKET_DEVICE_INVALID;
	}
	if (capacity == 0U)
	{
		return 0;
	}
	if (destination == NULL)
	{
		return SOCKET_DEVICE_INVALID;
	}

	socket_number = (uint8_t)index;
	status = wiznet_hw_socket_status(socket_number);
	interrupts = wiznet_hw_socket_interrupt(socket_number);
	observe_client_state(socket_number, status, interrupts);

	/*
	 * Peek directly into the core-owned destination.  No backend RX copy exists.
	 * The hardware read pointer is advanced only after the bytes have been
	 * copied, so consumption exactly matches the bytes returned to the core.
	 */
	received = wiznet_hw_socket_receive_peek(socket_number,
											(uint8_t *)destination,
											capacity);
	if (received > 0)
	{
		commit_result =
			wiznet_hw_socket_receive_commit(socket_number, (size_t)received);
		if (commit_result < 0)
		{
			/*
			 * The payload is already in the caller's buffer, but the native RX
			 * commit failed and future stream position is no longer trustworthy.
			 * Close the native socket to prevent duplicate delivery.  The current
			 * core recv dispatch still delivers this positive chunk before its
			 * next turn observes the queued close.
			 */
			mark_client_close(socket_number, SOCKET_DEVICE_ERROR);
			notify_client_closed(socket_number, SOCKET_DEVICE_ERROR);
			return received;
		}

		/* RX_RSR, not the edge-triggered RECV bit, is authoritative. */
		wiznet_hw_socket_clear_interrupt(socket_number, WIZ_SNIR_RECV);
		return received;
	}

	/*
	 * Returning WOULD_BLOCK causes the core to clear its readable state.  Rearm
	 * the backend hint so a later RX arrival can report readable() again.
	 */
	hw_slots[socket_number].state &= (uint8_t)~WIZ_FLAG_RX_REPORTED;

	if (received == WIZNET_HW_RETRY)
	{
		return SOCKET_DEVICE_WOULD_BLOCK;
	}
	if (received < 0)
	{
		mark_client_close(socket_number, SOCKET_DEVICE_ERROR);
		return finish_close_if_rx_drained(socket_number);
	}

	if ((hw_slots[socket_number].state & WIZ_FLAG_CLOSE_PENDING) != 0U)
	{
		return finish_close_if_rx_drained(socket_number);
	}

	return SOCKET_DEVICE_WOULD_BLOCK;
}

static int wiz_backend_send(socket_device_handle_t handle,
							const void *source,
							size_t length)
{
	int index = client_index_from_handle(handle);
	uint8_t socket_number;
	uint8_t status;
	uint8_t interrupts;
	int free_size;
	int sent;

	if (index < 0)
	{
		return SOCKET_DEVICE_INVALID;
	}
	if (length == 0U)
	{
		return 0;
	}
	if (source == NULL)
	{
		return SOCKET_DEVICE_INVALID;
	}

	socket_number = (uint8_t)index;
	status = wiznet_hw_socket_status(socket_number);
	interrupts = wiznet_hw_socket_interrupt(socket_number);
	observe_client_state(socket_number, status, interrupts);

	/*
	 * A native timeout is fatal.  Do not submit more bytes even if Sn_SR has not
	 * caught up yet.  If final RX is still retained, defer closed() until recv()
	 * can drain it.
	 */
	if ((hw_slots[socket_number].state & WIZ_FLAG_CLOSE_ERROR) != 0U)
	{
		return finish_close_if_rx_drained(socket_number);
	}

	/*
	 * CLOSE_WAIT is an orderly TCP half-close and WIZnet still permits sending.
	 * Other closing states are not sendable; report closure only after final RX
	 * has drained.
	 */
	if (!status_can_send(status))
	{
		if ((hw_slots[socket_number].state & WIZ_FLAG_CLOSE_PENDING) == 0U)
		{
			mark_client_close(socket_number, SOCKET_DEVICE_ERROR);
		}
		return finish_close_if_rx_drained(socket_number);
	}

	/*
	 * WIZnet requires the next SEND command to wait for SEND_OK from the prior
	 * one.  This is native command state, not a generic TX continuation queue.
	 */
	if ((hw_slots[socket_number].state & WIZ_FLAG_TX_PENDING) != 0U)
	{
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	free_size = wiznet_hw_socket_tx_free(socket_number);
	if (free_size == WIZNET_HW_RETRY || free_size == 0)
	{
		return SOCKET_DEVICE_WOULD_BLOCK;
	}
	if (free_size < 0)
	{
		mark_client_close(socket_number, SOCKET_DEVICE_ERROR);
		return finish_close_if_rx_drained(socket_number);
	}

	/*
	 * Perform exactly one native acceptance attempt.  The low-level helper
	 * copies at most the currently free chip TX memory, advances Sn_TX_WR and
	 * issues one SEND command.  The caller's source pointer is never retained.
	 */
	if (length > (size_t)free_size)
	{
		length = (size_t)free_size;
	}
	sent = wiznet_hw_socket_send(socket_number,
								(const uint8_t *)source,
								length);
	if (sent > 0)
	{
		if ((size_t)sent > length)
		{
			mark_client_close(socket_number, SOCKET_DEVICE_ERROR);
			notify_client_closed(socket_number, SOCKET_DEVICE_ERROR);
			return SOCKET_DEVICE_ERROR;
		}
		hw_slots[socket_number].state |= WIZ_FLAG_TX_PENDING;
		return sent;
	}

	if (sent == 0)
	{
		/*
		 * The second volatile TX_FSR sample inside the low-level helper can become
		 * temporarily incoherent even after our first stable sample.  Re-check
		 * connection state so genuine closure is not mislabeled as backpressure.
		 */
		status = wiznet_hw_socket_status(socket_number);
		interrupts = wiznet_hw_socket_interrupt(socket_number);
		observe_client_state(socket_number, status, interrupts);
		if ((hw_slots[socket_number].state & WIZ_FLAG_CLOSE_ERROR) != 0U ||
			!status_can_send(status))
		{
			return finish_close_if_rx_drained(socket_number);
		}
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	/*
	 * The low-level SEND command failed after touching native TX state.  Stream
	 * acceptance is ambiguous, so retrying could duplicate bytes.  Close the
	 * connection rather than pretend this is temporary backpressure.
	 */
	mark_client_close(socket_number, SOCKET_DEVICE_ERROR);
	notify_client_closed(socket_number, SOCKET_DEVICE_ERROR);
	return SOCKET_DEVICE_ERROR;
}

static int wiz_backend_close(socket_device_handle_t handle)
{
	int index = client_index_from_handle(handle);
	uint8_t i;
	uint8_t count;

	if (index >= 0)
	{
		/* Local close is synchronous with ownership and emits no closed() event. */
		release_hw_socket((uint8_t)index);
		return SOCKET_DEVICE_OK;
	}

	index = listener_index_from_handle(handle);
	if (index < 0)
	{
		return SOCKET_DEVICE_INVALID;
	}

	/*
	 * Invalidate the logical listener first.  Accepted clients are independent
	 * hardware sockets and socket_stop() closes them separately through their
	 * client handles.
	 */
	listeners[index].port = 0U;
	count = wiznet_hw_socket_count();
	if (count > WIZNET_MAX_HW_SOCKETS)
	{
		count = WIZNET_MAX_HW_SOCKETS;
	}
	for (i = 0U; i < count; ++i)
	{
		if (slot_role(&hw_slots[i]) == WIZ_ROLE_LISTENER &&
			hw_slots[i].listener_index == (uint8_t)index)
		{
			release_hw_socket(i);
		}
	}
	return SOCKET_DEVICE_OK;
}

/* Returns one only when accepted() consumed one normalized event budget unit. */
static uint8_t service_listener(uint8_t socket_number,
								uint8_t status,
								bool may_emit)
{
	uint8_t listener_index = hw_slots[socket_number].listener_index;

	if (listener_index >= WIZNET_MAX_LISTENERS ||
		listeners[listener_index].port == 0U)
	{
		release_hw_socket(socket_number);
		return 0U;
	}

	if (status == WIZ_SNSR_ESTABLISHED || status == WIZ_SNSR_CLOSE_WAIT)
	{
		socket_device_token_t token;

		if (!may_emit)
		{
			return 0U;
		}

		/*
		 * Promotion is complete before accepted().  Incrementing the generation
		 * here guarantees that a stale client handle from an earlier use of this
		 * same hardware socket cannot resolve to the new client.
		 */
		hw_slots[socket_number].generation =
			next_handle_generation(hw_slots[socket_number].generation);
		hw_slots[socket_number].token = SOCKET_DEVICE_INVALID_TOKEN;
		hw_slots[socket_number].listener_index = listener_index;
		hw_slots[socket_number].state = WIZ_ROLE_CLIENT;
		wiznet_hw_socket_clear_interrupt(socket_number, WIZ_SNIR_CON);

		token = backend_events->accepted(listener_handle(listener_index),
										 client_handle(socket_number));
		if (token == SOCKET_DEVICE_INVALID_TOKEN)
		{
			/* Core rejected capacity; it never owned a token for this client. */
			release_hw_socket(socket_number);
		}
		else
		{
			hw_slots[socket_number].token = token;
			if (status == WIZ_SNSR_CLOSE_WAIT)
			{
				mark_client_close(socket_number, SOCKET_DEVICE_CLOSED);
			}
		}
		return 1U;
	}

	if (status == WIZ_SNSR_SYNRECV || status == WIZ_SNSR_LISTEN)
	{
		return 0U;
	}

	/*
	 * LISTEN fell into an unusable state before acceptance.  Release it; bounded
	 * listener housekeeping will try to recreate it on a later poll pass.
	 */
	release_hw_socket(socket_number);
	return 0U;
}

/*
 * Returns one only when one readable/closed normalized event is emitted.
 * SEND_OK/TIMEOUT handling is native housekeeping and consumes no uCNC event
 * budget unless it ultimately emits closed().
 */
static uint8_t service_client(uint8_t socket_number,
							  uint8_t status,
							  bool may_emit)
{
	wiz_hw_slot_t *slot = &hw_slots[socket_number];
	uint8_t interrupts = wiznet_hw_socket_interrupt(socket_number);
	int available;

	observe_client_state(socket_number, status, interrupts);

	available = wiznet_hw_socket_rx_available(socket_number);
	if (available > 0)
	{
		wiznet_hw_socket_clear_interrupt(socket_number, WIZ_SNIR_RECV);
		if (may_emit && (slot->state & WIZ_FLAG_RX_REPORTED) == 0U)
		{
			socket_device_token_t token = slot->token;

			slot->state |= WIZ_FLAG_RX_REPORTED;
			backend_events->readable(token);
			/* readable() does not synchronously call application code, and this
			 * function emits no second event for the slot in this pass. */
			return 1U;
		}
		return 0U;
	}

	if (available < 0 && available != WIZNET_HW_RETRY)
	{
		mark_client_close(socket_number, SOCKET_DEVICE_ERROR);
	}

	if (available == 0 &&
		(slot->state & WIZ_FLAG_CLOSE_PENDING) != 0U &&
		may_emit)
	{
		int reason = pending_close_reason(slot);

		notify_client_closed(socket_number, reason);
		return 1U;
	}

	/*
	 * available==WIZNET_HW_RETRY is deliberately not treated as empty.  A
	 * volatile RX_RSR sample that failed to stabilize may still represent final
	 * unread payload, so closure stays deferred.
	 */
	return 0U;
}

static void reopen_one_missing_listener(void)
{
	uint8_t checked;

	for (checked = 0U; checked < WIZNET_MAX_LISTENERS; ++checked)
	{
		uint8_t index = listener_cursor;

		listener_cursor = (uint8_t)((listener_cursor + 1U) % WIZNET_MAX_LISTENERS);
		if (listeners[index].port != 0U &&
			!listener_has_hw_socket(index))
		{
			(void)open_listener_socket(index);
			return;
		}
	}
}

static void wiz_backend_poll(uint16_t budget)
{
	uint8_t count;
	uint8_t checked;
	uint16_t emitted = 0U;

	if (!wiznet_is_ready())
	{
		return;
	}

	count = wiznet_hw_socket_count();
	if (count == 0U || count > WIZNET_MAX_HW_SOCKETS)
	{
		return;
	}

	/*
	 * Scan every fixed hardware slot at most once.  The rotating starting point
	 * prevents a small event budget from starving high-numbered sockets.
	 */
	for (checked = 0U; checked < count; ++checked)
	{
		uint8_t socket_number =
			(uint8_t)((service_cursor + checked) % count);
		uint8_t role = slot_role(&hw_slots[socket_number]);
		uint8_t status;
		bool may_emit = emitted < budget;

		if (role == WIZ_ROLE_FREE)
		{
			continue;
		}

		status = wiznet_hw_socket_status(socket_number);
		if (role == WIZ_ROLE_LISTENER)
		{
			emitted = (uint16_t)(emitted +
				service_listener(socket_number, status, may_emit));
		}
		else if (role == WIZ_ROLE_CLIENT)
		{
			emitted = (uint16_t)(emitted +
				service_client(socket_number, status, may_emit));
		}
	}

	service_cursor = (uint8_t)((service_cursor + 1U) % count);

	/*
	 * Open at most one missing passive socket.  This is bounded native
	 * housekeeping and emits no normalized event, so it does not consume budget.
	 */
	reopen_one_missing_listener();
}

static socket_device_t wiznet_socket_device = {
	.init = wiz_backend_init,
	.listen = wiz_backend_listen,
	.recv = wiz_backend_recv,
	.send = wiz_backend_send,
	.close = wiz_backend_close,
	.poll = wiz_backend_poll};

bool wiznet_socket_backend_register(void)
{
	return socket_register_device(&wiznet_socket_device);
}
