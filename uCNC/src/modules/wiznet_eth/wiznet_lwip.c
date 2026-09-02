/*
 * C99 socket_device_t backend for WIZnet W5100/W5200/W5500 hardware sockets.
 * WIZnet chip contains the TCP/IP stack and exposes hardware sockets.
 */
#include "wiznet_ethernet.h"
#include "../net/socket.h"

#include <string.h>

enum
{
    WIZ_SNIR_CON = 0x01,
    WIZ_SNIR_DISCON = 0x02,
    WIZ_SNIR_RECV = 0x04,
    WIZ_SNIR_TIMEOUT = 0x08,
    WIZ_SNIR_SEND_OK = 0x10
};

enum
{
    WIZ_SNSR_CLOSED = 0x00,
    WIZ_SNSR_LISTEN = 0x14,
    WIZ_SNSR_SYNRECV = 0x16,
    WIZ_SNSR_ESTABLISHED = 0x17,
    WIZ_SNSR_FIN_WAIT = 0x18,
    WIZ_SNSR_CLOSING = 0x1A,
    WIZ_SNSR_TIME_WAIT = 0x1B,
    WIZ_SNSR_CLOSE_WAIT = 0x1C,
    WIZ_SNSR_LAST_ACK = 0x1D
};

typedef enum
{
    WIZ_ROLE_FREE = 0,
    WIZ_ROLE_LISTENER,
    WIZ_ROLE_CLIENT
} wiz_socket_role_t;

typedef struct
{
    wiz_socket_role_t role;
    uint8_t listener_index;
    bool tx_pending;
    socket_device_token_t token;
} wiz_hw_slot_t;

typedef struct
{
    bool used;
    uint16_t port;
} wiz_listener_t;

static const socket_device_events_t *backend_events;
static wiz_hw_slot_t hw_slots[WIZNET_MAX_HW_SOCKETS];
static wiz_listener_t listeners[MAX_SOCKETS];
static uint8_t service_cursor;
static uint8_t listener_cursor;

static socket_device_handle_t listener_handle(uint8_t index)
{
    return (socket_device_handle_t)(uintptr_t)&listeners[index];
}

static socket_device_handle_t client_handle(uint8_t socket_number)
{
    return (socket_device_handle_t)(uintptr_t)&hw_slots[socket_number];
}

static int listener_index_from_handle(socket_device_handle_t handle)
{
    uint8_t i;
    for (i = 0U; i < MAX_SOCKETS; ++i)
    {
        if (handle == listener_handle(i) && listeners[i].used)
        {
            return (int)i;
        }
    }
    return -1;
}

static int client_index_from_handle(socket_device_handle_t handle)
{
    uint8_t i;
    uint8_t count = wiznet_hw_socket_count();
    for (i = 0U; i < count; ++i)
    {
        if (handle == client_handle(i) &&
            hw_slots[i].role == WIZ_ROLE_CLIENT)
        {
            return (int)i;
        }
    }
    return -1;
}

static int find_free_hw_socket(void)
{
    uint8_t i;
    uint8_t count = wiznet_hw_socket_count();
    for (i = 0U; i < count; ++i)
    {
        if (hw_slots[i].role == WIZ_ROLE_FREE)
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
    for (i = 0U; i < count; ++i)
    {
        if (hw_slots[i].role == WIZ_ROLE_LISTENER &&
            hw_slots[i].listener_index == listener_index)
        {
            return true;
        }
    }
    return false;
}


static void release_hw_socket(uint8_t socket_number)
{
    wiznet_hw_socket_close(socket_number);
    memset(&hw_slots[socket_number], 0, sizeof(hw_slots[socket_number]));
}

static int disconnect_client(uint8_t socket_number, int reason)
{
    socket_device_token_t token = hw_slots[socket_number].token;
    release_hw_socket(socket_number);
    if (token != SOCKET_DEVICE_INVALID_TOKEN)
    {
        backend_events->closed(token, reason);
    }
    return reason;
}

static bool open_listener_socket(uint8_t index)
{
    int socket_number;
    if (!listeners[index].used || listener_has_hw_socket(index))
    {
        return true;
    }
    socket_number = find_free_hw_socket();
    if (socket_number < 0)
    {
        return false;
    }
    if (!wiznet_hw_socket_open_tcp_server((uint8_t)socket_number,
                                         listeners[index].port))
    {
        release_hw_socket((uint8_t)socket_number);
        return false;
    }
    hw_slots[socket_number].role = WIZ_ROLE_LISTENER;
    hw_slots[socket_number].listener_index = index;
    return true;
}

static int wiz_backend_init(const socket_device_events_t *events)
{
    if (!wiznet_is_ready() || events == NULL || events->accepted == NULL ||
        events->readable == NULL || events->closed == NULL)
    {
        return SOCKET_DEVICE_ERROR;
    }
    backend_events = events;
    memset(listeners, 0, sizeof(listeners));
    memset(hw_slots, 0, sizeof(hw_slots));
    service_cursor = 0U;
    listener_cursor = 0U;
    return SOCKET_DEVICE_OK;
}

static socket_device_handle_t wiz_backend_listen(
    const socket_device_endpoint_t *endpoint, uint8_t backlog)
{
    uint8_t i;
    ipv4_address_t local_ip = wiznet_get_ip();
    /* Configuration stores wire-order octets; endpoints are host-order values. */
    uint32_t address = ((uint32_t)local_ip.octets[0] << 24) |
                       ((uint32_t)local_ip.octets[1] << 16) |
                       ((uint32_t)local_ip.octets[2] << 8) |
                       local_ip.octets[3];

    /* Each logical listener gets one pending hardware socket, including when
     * backlog is zero. Established clients use separate hardware slots. */
    (void)backlog;
    if (!wiznet_is_ready() || endpoint == NULL || endpoint->port == 0U ||
        (endpoint->address != IP_ANY && endpoint->address != address))
    {
        return SOCKET_DEVICE_INVALID_HANDLE;
    }
    for (i = 0U; i < MAX_SOCKETS; ++i)
    {
        if (listeners[i].used && listeners[i].port == endpoint->port)
        {
            return SOCKET_DEVICE_INVALID_HANDLE;
        }
    }
    for (i = 0U; i < MAX_SOCKETS; ++i)
    {
        if (!listeners[i].used)
        {
            listeners[i].used = true;
            listeners[i].port = endpoint->port;
            if (!open_listener_socket(i))
            {
                memset(&listeners[i], 0, sizeof(listeners[i]));
                return SOCKET_DEVICE_INVALID_HANDLE;
            }
            return listener_handle(i);
        }
    }
    return SOCKET_DEVICE_INVALID_HANDLE;
}

/* Used only in owner context. The recv/send contract requires synchronous
 * closed notification on fatal discovery, even outside poll(). */
static int client_status(uint8_t socket_number, uint8_t *status)
{
    uint8_t interrupts = wiznet_hw_socket_interrupt(socket_number);
    *status = wiznet_hw_socket_status(socket_number);
    if ((interrupts & WIZ_SNIR_TIMEOUT) != 0U)
    {
        return disconnect_client(socket_number, SOCKET_DEVICE_TIMEOUT);
    }
    if (*status != WIZ_SNSR_ESTABLISHED && *status != WIZ_SNSR_CLOSE_WAIT)
    {
        return disconnect_client(socket_number, SOCKET_DEVICE_CLOSED);
    }
    if ((interrupts & WIZ_SNIR_SEND_OK) != 0U)
    {
        wiznet_hw_socket_clear_interrupt(socket_number, WIZ_SNIR_SEND_OK);
        hw_slots[socket_number].tx_pending = false;
    }
    return SOCKET_DEVICE_OK;
}

static int wiz_backend_recv(socket_device_handle_t handle, void *destination,
                            size_t capacity)
{
    int index;
    int result;
    uint8_t status;
    if (capacity == 0U)
    {
        return 0;
    }
    index = client_index_from_handle(handle);
    if (index < 0 || destination == NULL)
    {
        return SOCKET_DEVICE_INVALID;
    }
    result = client_status((uint8_t)index, &status);
    if (result < 0)
    {
        return result;
    }
    result = wiznet_hw_socket_receive((uint8_t)index, destination, capacity);
    if (result < 0)
    {
        return disconnect_client((uint8_t)index, SOCKET_DEVICE_ERROR);
    }
    if (result > 0)
    {
        return result;
    }
    /* Drain payload preceding FIN before reporting EOF. */
    if (status == WIZ_SNSR_CLOSE_WAIT)
    {
        return disconnect_client((uint8_t)index, SOCKET_DEVICE_CLOSED);
    }
    return SOCKET_DEVICE_WOULD_BLOCK;
}

static int wiz_backend_send(socket_device_handle_t handle, const void *source,
                            size_t length)
{
    int index;
    int result;
    uint8_t status;
    if (length == 0U)
    {
        return 0;
    }
    index = client_index_from_handle(handle);
    if (index < 0 || source == NULL)
    {
        return SOCKET_DEVICE_INVALID;
    }
    result = client_status((uint8_t)index, &status);
    if (result < 0)
    {
        return result;
    }
    if (hw_slots[index].tx_pending)
    {
        return SOCKET_DEVICE_WOULD_BLOCK;
    }
    /* Copy directly into the native TCP TX ring. Never wait for SEND_OK. */
    result = wiznet_hw_socket_send((uint8_t)index, source, length);
    if (result < 0)
    {
        return disconnect_client((uint8_t)index, SOCKET_DEVICE_ERROR);
    }
    if (result == 0)
    {
        return SOCKET_DEVICE_WOULD_BLOCK;
    }
    hw_slots[index].tx_pending = true;
    return result;
}

static int wiz_backend_close(socket_device_handle_t handle)
{
    int index = client_index_from_handle(handle);
    uint8_t i;
    if (index >= 0)
    {
        release_hw_socket((uint8_t)index);
        return SOCKET_DEVICE_OK;
    }
    index = listener_index_from_handle(handle);
    if (index < 0)
    {
        return SOCKET_DEVICE_INVALID;
    }
    listeners[index].used = false;
    for (i = 0U; i < wiznet_hw_socket_count(); ++i)
    {
        if (hw_slots[i].role == WIZ_ROLE_LISTENER &&
            hw_slots[i].listener_index == (uint8_t)index)
        {
            release_hw_socket(i);
        }
    }
    memset(&listeners[index], 0, sizeof(listeners[index]));
    return SOCKET_DEVICE_OK;
}

/* Returns the number of normalized events emitted (at most one). */
static uint8_t service_listener(uint8_t socket_number, uint8_t status)
{
    uint8_t index = hw_slots[socket_number].listener_index;
    if (!listeners[index].used)
    {
        release_hw_socket(socket_number);
        return 0U;
    }
    if (status == WIZ_SNSR_ESTABLISHED || status == WIZ_SNSR_CLOSE_WAIT)
    {
        socket_device_token_t token;
        hw_slots[socket_number].role = WIZ_ROLE_CLIENT;
        wiznet_hw_socket_clear_interrupt(socket_number, WIZ_SNIR_CON);
        token = backend_events->accepted(listener_handle(index),
                                         client_handle(socket_number));
        if (token == SOCKET_DEVICE_INVALID_TOKEN)
        {
            release_hw_socket(socket_number);
        }
        else
        {
            hw_slots[socket_number].token = token;
        }
        return 1U;
    }
    if (status != WIZ_SNSR_LISTEN && status != WIZ_SNSR_SYNRECV)
    {
        release_hw_socket(socket_number);
    }
    return 0U;
}

static uint8_t service_client(uint8_t socket_number)
{
    uint8_t status;
    if (client_status(socket_number, &status) < 0)
    {
        return 1U;
    }
    /* Readiness only: leave bytes and RX_RD untouched until recv(). */
    if (wiznet_hw_socket_available(socket_number) != 0U)
    {
        backend_events->readable(hw_slots[socket_number].token);
        return 1U;
    }
    if (status == WIZ_SNSR_CLOSE_WAIT)
    {
        disconnect_client(socket_number, SOCKET_DEVICE_CLOSED);
        return 1U;
    }
    return 0U;
}

static void wiz_backend_poll(uint16_t budget)
{
    uint8_t count = wiznet_hw_socket_count();
    uint8_t visited;
    if (count == 0U || backend_events == NULL)
    {
        return;
    }
    for (visited = 0U; visited < count && budget != 0U; ++visited)
    {
        uint8_t socket_number = service_cursor;
        service_cursor = (uint8_t)((service_cursor + 1U) % count);
        if (hw_slots[socket_number].role == WIZ_ROLE_LISTENER)
        {
            budget -= service_listener(socket_number,
                                       wiznet_hw_socket_status(socket_number));
        }
        else if (hw_slots[socket_number].role == WIZ_ROLE_CLIENT)
        {
            budget -= service_client(socket_number);
        }
    }
    /* Bounded housekeeping, including for a zero event budget. */
    (void)open_listener_socket(listener_cursor);
    listener_cursor = (uint8_t)((listener_cursor + 1U) % MAX_SOCKETS);
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
