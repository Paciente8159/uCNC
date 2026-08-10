/*
 * Name: wiznet5500_socket_device.c
 * Description: WIZnet W5500 backend for the event-driven µCNC socket_device_t API.
 *
 * Assumptions:
 *  - WIZnet ioLibrary_Driver is present and already initialized at the hardware
 *    level (SPI callbacks, W5500 memory configuration, MAC/IP/DHCP, etc.).
 *  - This backend owns the W5500 hardware sockets selected by
 *    UCNC_W5500_SOCKET_MASK. Do not let another ioLibrary user allocate those
 *    same socket numbers concurrently.
 *  - TCP/IPv4 server sockets only. UDP/TCP-client support is intentionally
 *    outside the current socket_device_t contract.
 */

#include "wiznet5500_socket_device.h"

/*
 * Adjust this include path to match the ioLibrary_Driver location in the
 * PlatformIO project. The important point is that this is WIZnet's
 * Ethernet/socket.h, not µCNC's modules/net/socket.h.
 */
#include "driver/socket.h"

#include <limits.h>
#include <string.h>

#if (_WIZCHIP_ != W5500)
#error "wiznet5500_socket_device.c requires ioLibrary_Driver configured for W5500"
#endif

#ifndef UCNC_W5500_SOCKET_MASK
#define UCNC_W5500_SOCKET_MASK ((1UL << _WIZCHIP_SOCK_NUM_) - 1UL)
#endif

#ifndef UCNC_W5500_MAX_LISTENERS
#define UCNC_W5500_MAX_LISTENERS _WIZCHIP_SOCK_NUM_
#endif

#ifndef UCNC_W5500_RX_CHUNK_SIZE
#ifdef SOCKET_MAX_DATA_SIZE
#define UCNC_W5500_RX_CHUNK_SIZE SOCKET_MAX_DATA_SIZE
#else
#define UCNC_W5500_RX_CHUNK_SIZE 1024U
#endif
#endif

#if (UCNC_W5500_RX_CHUNK_SIZE > UINT16_MAX)
#error "UCNC_W5500_RX_CHUNK_SIZE must fit in uint16_t"
#endif

#define W5500_SN_INVALID ((uint8_t)0xFFU)
#define W5500_LISTENER_HANDLE_BASE ((socket_handle_t)0x100U)

typedef enum
{
	W5500_HW_FREE = 0,
	W5500_HW_LISTENER,
	W5500_HW_CLIENT
} w5500_hw_kind_t;

typedef struct
{
	bool in_use;
	socket_handle_t handle;
	uint16_t port;
	uint8_t max_clients;
	uint8_t active_clients;
	uint8_t listen_sn;
} w5500_listener_t;

typedef struct
{
	w5500_hw_kind_t kind;
	socket_handle_t listener_handle;
	bool write_blocked;
	bool disconnect_pending;
	int disconnect_reason;
} w5500_hw_state_t;

static const socket_device_events_t *w5500_events;
static w5500_listener_t w5500_listeners[UCNC_W5500_MAX_LISTENERS];
static w5500_hw_state_t w5500_hw[_WIZCHIP_SOCK_NUM_];
static uint8_t w5500_service_cursor;
static uint8_t w5500_listener_cursor;
static socket_handle_t w5500_next_listener_handle = W5500_LISTENER_HANDLE_BASE;

/*
 * Static RX buffer keeps the cooperative socket service from consuming a large
 * amount of task/ISR stack. It is valid only while events->data() executes,
 * exactly as required by socket_device_t.
 */
static uint8_t w5500_rx_buffer[UCNC_W5500_RX_CHUNK_SIZE + 1U];

static bool w5500_sn_allowed(uint8_t sn)
{
	return (sn < _WIZCHIP_SOCK_NUM_) &&
		   ((UCNC_W5500_SOCKET_MASK & (1UL << sn)) != 0U);
}

static void w5500_hw_clear(uint8_t sn)
{
	if (sn >= _WIZCHIP_SOCK_NUM_)
		return;

	memset(&w5500_hw[sn], 0, sizeof(w5500_hw[sn]));
	w5500_hw[sn].kind = W5500_HW_FREE;
	w5500_hw[sn].listener_handle = SOCKET_INVALID_HANDLE;
}

static int w5500_find_listener_slot(socket_handle_t handle)
{
	for (uint8_t i = 0; i < UCNC_W5500_MAX_LISTENERS; i++)
	{
		if (w5500_listeners[i].in_use && w5500_listeners[i].handle == handle)
			return (int)i;
	}

	return -1;
}

static int w5500_find_free_listener_slot(void)
{
	for (uint8_t i = 0; i < UCNC_W5500_MAX_LISTENERS; i++)
	{
		if (!w5500_listeners[i].in_use)
			return (int)i;
	}

	return -1;
}

static socket_handle_t w5500_alloc_listener_handle(void)
{
	/*
	 * Listener handles are synthetic and deliberately outside the W5500
	 * hardware socket-number range (0..7).
	 */
	for (;;)
	{
		socket_handle_t handle = w5500_next_listener_handle++;

		if (w5500_next_listener_handle == SOCKET_INVALID_HANDLE)
			w5500_next_listener_handle = W5500_LISTENER_HANDLE_BASE;

		if (handle != SOCKET_INVALID_HANDLE &&
			handle >= W5500_LISTENER_HANDLE_BASE &&
			w5500_find_listener_slot(handle) < 0)
		{
			return handle;
		}
	}
}

static int w5500_alloc_hw_socket(void)
{
	for (uint8_t sn = 0; sn < _WIZCHIP_SOCK_NUM_; sn++)
	{
		if (!w5500_sn_allowed(sn))
			continue;

		if (w5500_hw[sn].kind != W5500_HW_FREE)
			continue;

		/*
		 * The configured mask is expected to be exclusively owned by this
		 * backend. The status check is still useful protection against stale
		 * or externally-opened sockets.
		 */
		if (getSn_SR(sn) != SOCK_CLOSED)
			continue;

		return (int)sn;
	}

	return -1;
}

static void w5500_listener_client_inc(socket_handle_t listener_handle)
{
	int idx = w5500_find_listener_slot(listener_handle);

	if (idx >= 0 && w5500_listeners[idx].active_clients < UINT8_MAX)
		w5500_listeners[idx].active_clients++;
}

static void w5500_listener_client_dec(socket_handle_t listener_handle)
{
	int idx = w5500_find_listener_slot(listener_handle);

	if (idx >= 0 && w5500_listeners[idx].active_clients > 0U)
		w5500_listeners[idx].active_clients--;
}

static int w5500_open_listener_hw(uint8_t listener_idx)
{
	w5500_listener_t *listener;
	int sn;
	int8_t rc;

	if (listener_idx >= UCNC_W5500_MAX_LISTENERS)
		return SOCKET_DEVICE_INVALID;

	listener = &w5500_listeners[listener_idx];

	if (!listener->in_use)
		return SOCKET_DEVICE_INVALID;

	if (listener->listen_sn != W5500_SN_INVALID)
		return SOCKET_DEVICE_OK;

	if (listener->active_clients >= listener->max_clients)
		return SOCKET_DEVICE_WOULD_BLOCK;

	sn = w5500_alloc_hw_socket();
	if (sn < 0)
		return SOCKET_DEVICE_WOULD_BLOCK;

	/*
	 * ioLibrary's SF_IO_NONBLOCK is essential. send()/recv() must return
	 * SOCK_BUSY rather than waiting for network progress.
	 */
	rc = socket((uint8_t)sn, Sn_MR_TCP, listener->port, SF_IO_NONBLOCK);
	if (rc < 0)
	{
		(void)close((uint8_t)sn);
		return SOCKET_DEVICE_ERROR;
	}

	w5500_hw_clear((uint8_t)sn);
	w5500_hw[sn].kind = W5500_HW_LISTENER;
	w5500_hw[sn].listener_handle = listener->handle;
	listener->listen_sn = (uint8_t)sn;

	rc = listen((uint8_t)sn);
	if (rc != SOCK_OK)
	{
		(void)close((uint8_t)sn);
		w5500_hw_clear((uint8_t)sn);
		listener->listen_sn = W5500_SN_INVALID;
		return SOCKET_DEVICE_ERROR;
	}

	return SOCKET_DEVICE_OK;
}

static void w5500_replenish_one_listener(void)
{
	for (uint8_t n = 0; n < UCNC_W5500_MAX_LISTENERS; n++)
	{
		uint8_t idx = (uint8_t)((w5500_listener_cursor + n) %
								UCNC_W5500_MAX_LISTENERS);
		w5500_listener_t *listener = &w5500_listeners[idx];

		if (!listener->in_use)
			continue;

		if (listener->listen_sn != W5500_SN_INVALID)
			continue;

		if (listener->active_clients >= listener->max_clients)
			continue;

		w5500_listener_cursor =
			(uint8_t)((idx + 1U) % UCNC_W5500_MAX_LISTENERS);

		/*
		 * Attempt at most one listener creation per service() invocation.
		 * Failure due to no free hardware socket is not fatal; another attempt
		 * will be made when service() runs again.
		 */
		(void)w5500_open_listener_hw(idx);
		return;
	}
}

static int w5500_map_io_error(int32_t rc)
{
	if (rc == SOCK_BUSY)
		return SOCKET_DEVICE_WOULD_BLOCK;

	switch (rc)
	{
	case SOCKERR_SOCKCLOSED:
	case SOCKERR_SOCKSTATUS:
	case SOCKERR_TIMEOUT:
		return SOCKET_DEVICE_CLOSED;

	case SOCKERR_SOCKNUM:
	case SOCKERR_SOCKMODE:
	case SOCKERR_SOCKFLAG:
	case SOCKERR_DATALEN:
	case SOCKERR_ARG:
		return SOCKET_DEVICE_INVALID;

	default:
		return SOCKET_DEVICE_ERROR;
	}
}

static void w5500_mark_disconnect(uint8_t sn, int reason)
{
	if (sn >= _WIZCHIP_SOCK_NUM_ ||
		w5500_hw[sn].kind != W5500_HW_CLIENT)
	{
		return;
	}

	w5500_hw[sn].disconnect_pending = true;
	w5500_hw[sn].disconnect_reason = reason;
}

static void w5500_release_client(uint8_t sn, int reason, bool notify)
{
	socket_handle_t listener_handle;
	socket_handle_t client_handle;

	if (sn >= _WIZCHIP_SOCK_NUM_ ||
		w5500_hw[sn].kind != W5500_HW_CLIENT)
	{
		return;
	}

	client_handle = (socket_handle_t)sn;
	listener_handle = w5500_hw[sn].listener_handle;

	/*
	 * The socket_device_t contract requires native resources to be released
	 * before the disconnected event is delivered.
	 */
	(void)close(sn);
	w5500_hw_clear(sn);
	w5500_listener_client_dec(listener_handle);

	if (notify && w5500_events && w5500_events->disconnected)
		w5500_events->disconnected(client_handle, reason);
}

static void w5500_promote_listener_to_client(uint8_t sn)
{
	socket_handle_t listener_handle;
	int listener_idx;
	bool accepted = false;

	if (sn >= _WIZCHIP_SOCK_NUM_ ||
		w5500_hw[sn].kind != W5500_HW_LISTENER)
	{
		return;
	}

	listener_handle = w5500_hw[sn].listener_handle;
	listener_idx = w5500_find_listener_slot(listener_handle);

	if (listener_idx < 0)
	{
		(void)close(sn);
		w5500_hw_clear(sn);
		return;
	}

	/*
	 * The hardware socket is no longer a listener after W5500 accepts the
	 * connection. Detach it from the logical listener before calling µCNC.
	 */
	if (w5500_listeners[listener_idx].listen_sn == sn)
		w5500_listeners[listener_idx].listen_sn = W5500_SN_INVALID;

	w5500_hw[sn].kind = W5500_HW_CLIENT;
	w5500_hw[sn].write_blocked = false;
	w5500_hw[sn].disconnect_pending = false;

	/* Clear the W5500 connection interrupt, if asserted. */
#ifdef Sn_IR_CON
	if ((getSn_IR(sn) & Sn_IR_CON) != 0U)
		setSn_IR(sn, Sn_IR_CON);
#endif

	if (w5500_events && w5500_events->connected)
		accepted = w5500_events->connected(listener_handle,
										   (socket_handle_t)sn);

	if (!accepted)
	{
		/*
		 * Core has no client slot. Reclaim this hardware socket immediately.
		 */
		if (w5500_hw[sn].kind == W5500_HW_CLIENT)
		{
			(void)close(sn);
			w5500_hw_clear(sn);
		}
	}
	else
	{
		/*
		 * connected() may invoke application code which can synchronously
		 * close the client. Only count it if the hardware slot still belongs
		 * to this client after the callback returns.
		 */
		if (w5500_hw[sn].kind == W5500_HW_CLIENT)
			w5500_listener_client_inc(listener_handle);
	}

	/*
	 * Do not force listener recreation here. service() will attempt one
	 * bounded replenishment on its next invocation.
	 */
}

static int w5500_socket_device_init(const socket_device_events_t *events)
{
	if (!events)
		return SOCKET_DEVICE_INVALID;

	w5500_events = events;
	w5500_service_cursor = 0U;
	w5500_listener_cursor = 0U;
	w5500_next_listener_handle = W5500_LISTENER_HANDLE_BASE;

	memset(w5500_listeners, 0, sizeof(w5500_listeners));

	for (uint8_t sn = 0; sn < _WIZCHIP_SOCK_NUM_; sn++)
		w5500_hw_clear(sn);

	/*
	 * Intentionally does NOT call wizchip_init(), reg_wizchip_*(), or
	 * wizchip_setnetinfo(). Those operations depend on the target MCU's SPI,
	 * reset, interrupt, static-IP/DHCP, and buffer-memory policy and belong in
	 * the hardware/platform initialization layer.
	 */
	return SOCKET_DEVICE_OK;
}

static socket_handle_t w5500_socket_device_listen(uint32_t ip_listen,
												   uint16_t port,
												   int domain,
												   int type,
												   int protocol,
												   uint8_t backlog)
{
	int idx;
	w5500_listener_t *listener;

	(void)ip_listen;
	(void)protocol;

	/*
	 * W5500 has one global source IPv4 address, not a per-socket local-address
	 * bind. IP_ANY and the configured local interface address are therefore
	 * equivalent for this backend.
	 */
	if (domain != AF_INET || type != SOCK_STREAM || port == 0U)
		return SOCKET_INVALID_HANDLE;

	idx = w5500_find_free_listener_slot();
	if (idx < 0)
		return SOCKET_INVALID_HANDLE;

	listener = &w5500_listeners[idx];
	memset(listener, 0, sizeof(*listener));

	listener->in_use = true;
	listener->handle = w5500_alloc_listener_handle();
	listener->port = port;
	listener->max_clients = backlog ? backlog : 1U;
	if (listener->max_clients > _WIZCHIP_SOCK_NUM_)
		listener->max_clients = _WIZCHIP_SOCK_NUM_;
	listener->active_clients = 0U;
	listener->listen_sn = W5500_SN_INVALID;

	if (w5500_open_listener_hw((uint8_t)idx) != SOCKET_DEVICE_OK)
	{
		memset(listener, 0, sizeof(*listener));
		return SOCKET_INVALID_HANDLE;
	}

	return listener->handle;
}

static int w5500_socket_device_send(socket_handle_t client,
									const void *data,
									size_t len,
									int flags)
{
	uint8_t sn;
	uint8_t status;
	uint16_t free_size;
	size_t chunk;
	int32_t rc;

	(void)flags;

	if (!data)
		return SOCKET_DEVICE_INVALID;

	if (len == 0U)
		return 0;

	if (client >= (socket_handle_t)_WIZCHIP_SOCK_NUM_)
		return SOCKET_DEVICE_INVALID;

	sn = (uint8_t)client;

	if (!w5500_sn_allowed(sn) ||
		w5500_hw[sn].kind != W5500_HW_CLIENT)
	{
		return SOCKET_DEVICE_INVALID;
	}

	if (w5500_hw[sn].disconnect_pending)
		return SOCKET_DEVICE_CLOSED;

	status = getSn_SR(sn);
	if (status != SOCK_ESTABLISHED && status != SOCK_CLOSE_WAIT)
	{
		w5500_mark_disconnect(sn, SOCKET_DEVICE_CLOSED);
		return SOCKET_DEVICE_CLOSED;
	}

	/*
	 * Avoid calling ioLibrary send() when no hardware TX space exists.
	 * send() was opened with SF_IO_NONBLOCK, so a send already in progress
	 * will also return SOCK_BUSY rather than waiting for network progress.
	 */
	free_size = getSn_TX_FSR(sn);
	if (free_size == 0U)
	{
		w5500_hw[sn].write_blocked = true;
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	chunk = len;
	if (chunk > (size_t)free_size)
		chunk = (size_t)free_size;
	if (chunk > (size_t)UINT16_MAX)
		chunk = (size_t)UINT16_MAX;

	rc = send(sn, (uint8_t *)(uintptr_t)data, (uint16_t)chunk);

	if (rc > 0)
		return (int)rc;

	if (rc == SOCK_BUSY)
	{
		w5500_hw[sn].write_blocked = true;
		return SOCKET_DEVICE_WOULD_BLOCK;
	}

	{
		int mapped = w5500_map_io_error(rc);

		if (mapped == SOCKET_DEVICE_CLOSED)
			w5500_mark_disconnect(sn, mapped);

		return mapped;
	}
}

static int w5500_socket_device_close(socket_handle_t handle)
{
	int listener_idx;

	/* Synthetic logical listener handle. */
	listener_idx = w5500_find_listener_slot(handle);
	if (listener_idx >= 0)
	{
		w5500_listener_t *listener = &w5500_listeners[listener_idx];
		uint8_t sn = listener->listen_sn;

		/*
		 * Mark it dead first so re-entrant/core activity cannot cause a new
		 * hardware listener to be replenished.
		 */
		listener->in_use = false;
		listener->listen_sn = W5500_SN_INVALID;

		if (sn != W5500_SN_INVALID &&
			sn < _WIZCHIP_SOCK_NUM_ &&
			w5500_hw[sn].kind == W5500_HW_LISTENER &&
			w5500_hw[sn].listener_handle == handle)
		{
			(void)close(sn);
			w5500_hw_clear(sn);
		}

		memset(listener, 0, sizeof(*listener));
		return SOCKET_DEVICE_OK;
	}

	/* Client handles map directly to W5500 hardware socket numbers. */
	if (handle < (socket_handle_t)_WIZCHIP_SOCK_NUM_)
	{
		uint8_t sn = (uint8_t)handle;

		if (w5500_hw[sn].kind != W5500_HW_CLIENT)
			return SOCKET_DEVICE_INVALID;

		/*
		 * Explicit/local close must not generate disconnected(), per the
		 * socket_device_t contract.
		 */
		w5500_release_client(sn, 0, false);
		return SOCKET_DEVICE_OK;
	}

	return SOCKET_DEVICE_INVALID;
}

static void w5500_socket_device_service(void)
{
	/*
	 * Keep one standby hardware listener for each logical listener while it
	 * has capacity and a W5500 hardware socket is available.
	 *
	 * At most one listener-open attempt is made per service() invocation.
	 */
	w5500_replenish_one_listener();

	/*
	 * Inspect at most _WIZCHIP_SOCK_NUM_ hardware sockets and dispatch at most
	 * one µCNC event per call. This keeps service() cooperative and bounded.
	 */
	for (uint8_t n = 0; n < _WIZCHIP_SOCK_NUM_; n++)
	{
		uint8_t sn =
			(uint8_t)((w5500_service_cursor + n) % _WIZCHIP_SOCK_NUM_);
		w5500_hw_state_t *hw = &w5500_hw[sn];
		uint8_t status;

		if (hw->kind == W5500_HW_FREE)
			continue;

		w5500_service_cursor =
			(uint8_t)((sn + 1U) % _WIZCHIP_SOCK_NUM_);

		status = getSn_SR(sn);

		if (hw->kind == W5500_HW_LISTENER)
		{
			if (status == SOCK_ESTABLISHED || status == SOCK_CLOSE_WAIT)
			{
				w5500_promote_listener_to_client(sn);
				return;
			}

			if (status == SOCK_CLOSED)
			{
				socket_handle_t listener_handle = hw->listener_handle;
				int listener_idx =
					w5500_find_listener_slot(listener_handle);

				w5500_hw_clear(sn);

				if (listener_idx >= 0 &&
					w5500_listeners[listener_idx].listen_sn == sn)
				{
					w5500_listeners[listener_idx].listen_sn =
						W5500_SN_INVALID;
				}

				return;
			}

			continue;
		}

		/* Client */
		if (hw->disconnect_pending)
		{
			int reason = hw->disconnect_reason;
			w5500_release_client(sn, reason, true);
			return;
		}

#ifdef Sn_IR_TIMEOUT
		if ((getSn_IR(sn) & Sn_IR_TIMEOUT) != 0U)
		{
			w5500_release_client(sn, SOCKET_DEVICE_ERROR, true);
			return;
		}
#endif

		if (status == SOCK_ESTABLISHED || status == SOCK_CLOSE_WAIT)
		{
			uint16_t rx_size = getSn_RX_RSR(sn);

			if (rx_size > 0U)
			{
				uint16_t chunk = rx_size;
				int32_t rc;

				if (chunk > (uint16_t)UCNC_W5500_RX_CHUNK_SIZE)
					chunk = (uint16_t)UCNC_W5500_RX_CHUNK_SIZE;

				rc = recv(sn, w5500_rx_buffer, chunk);

				if (rc > 0)
				{
					size_t len = (size_t)rc;
					w5500_rx_buffer[len] = '\0';

					if (w5500_events && w5500_events->data)
					{
						w5500_events->data((socket_handle_t)sn,
										  (char *)w5500_rx_buffer,
										  len);
					}
					return;
				}

				if (rc != SOCK_BUSY)
				{
					int mapped = w5500_map_io_error(rc);
					w5500_release_client(
						sn,
						(mapped == SOCKET_DEVICE_CLOSED)
							? mapped
							: SOCKET_DEVICE_ERROR,
						true);
					return;
				}
			}

			/*
			 * Remote TCP FIN. Drain RX data first; once CLOSE_WAIT has no
			 * unread payload, release the W5500 socket and report an orderly
			 * disconnect.
			 */
			if (status == SOCK_CLOSE_WAIT && getSn_RX_RSR(sn) == 0U)
			{
				w5500_release_client(sn, 0, true);
				return;
			}

			/*
			 * Do not clear SENDOK here. ioLibrary send() uses SENDOK to retire
			 * its internal "send in progress" state on the next send attempt.
			 */
#ifdef Sn_IR_SENDOK
			if (hw->write_blocked &&
				((getSn_IR(sn) & Sn_IR_SENDOK) != 0U))
			{
				hw->write_blocked = false;

				if (w5500_events && w5500_events->writable)
					w5500_events->writable((socket_handle_t)sn);

				return;
			}
#endif

			continue;
		}

		if (status == SOCK_CLOSED)
		{
			w5500_release_client(sn, SOCKET_DEVICE_CLOSED, true);
			return;
		}

		/*
		 * Any other TCP state (FIN_WAIT, CLOSING, LAST_ACK, etc.) is allowed
		 * to progress in hardware. It will eventually become CLOSE_WAIT/CLOSED
		 * or return to a state handled above.
		 */
	}
}

socket_device_t wiznet5500_socket_device =
{
	.init = w5500_socket_device_init,
	.listen = w5500_socket_device_listen,
	.send = w5500_socket_device_send,
	.close = w5500_socket_device_close,
	.service = w5500_socket_device_service
};
