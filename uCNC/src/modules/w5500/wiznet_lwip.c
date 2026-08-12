/*
 * C99 socket_device_t backend for WIZnet W5100/W5200/W5500 hardware sockets.
 * WIZnet chip contains the TCP/IP stack and exposes hardware sockets.
 */
#include "wiznet_ethernet.h"
#include "../net/socket.h"

#include <string.h>

enum {
    WIZ_SNIR_CON = 0x01,
    WIZ_SNIR_DISCON = 0x02,
    WIZ_SNIR_RECV = 0x04,
    WIZ_SNIR_TIMEOUT = 0x08,
    WIZ_SNIR_SEND_OK = 0x10
};

enum {
    WIZ_SNSR_CLOSED = 0x00,
    WIZ_SNSR_LISTEN = 0x14,
    WIZ_SNSR_ESTABLISHED = 0x17,
    WIZ_SNSR_FIN_WAIT = 0x18,
    WIZ_SNSR_CLOSING = 0x1A,
    WIZ_SNSR_TIME_WAIT = 0x1B,
    WIZ_SNSR_CLOSE_WAIT = 0x1C,
    WIZ_SNSR_LAST_ACK = 0x1D
};

typedef enum {
    WIZ_ROLE_FREE = 0,
    WIZ_ROLE_LISTENER,
    WIZ_ROLE_CLIENT
} wiz_socket_role_t;

typedef struct {
    wiz_socket_role_t role;
    uint8_t listener_index;
    uint8_t socket_number;
    bool tx_pending;
} wiz_hw_slot_t;

typedef struct {
    bool used;
    uint16_t port;
    int domain;
    int type;
    int protocol;
} wiz_listener_t;

static const socket_device_events_t *backend_events;
static wiz_hw_slot_t hw_slots[WIZNET_MAX_HW_SOCKETS];
static wiz_listener_t listeners[MAX_SOCKETS];
static uint8_t service_cursor;
static uint8_t listener_cursor;
static uint8_t last_socket_status[WIZNET_MAX_HW_SOCKETS];
static char receive_buffer[SOCKET_MAX_DATA_SIZE + 1U];

static socket_handle_t listener_handle(uint8_t index)
{
    return (socket_handle_t)(uintptr_t)&listeners[index];
}

static socket_handle_t client_handle(uint8_t socket_number)
{
    return (socket_handle_t)(uintptr_t)&hw_slots[socket_number];
}

static int listener_index_from_handle(socket_handle_t handle)
{
    uint8_t i;
    for (i = 0U; i < MAX_SOCKETS; ++i) {
        if (handle == listener_handle(i) && listeners[i].used) {
            return (int)i;
        }
    }
    return -1;
}

static int client_index_from_handle(socket_handle_t handle)
{
    uint8_t i;
    uint8_t count = wiznet_hw_socket_count();
    for (i = 0U; i < count; ++i) {
        if (handle == client_handle(i) &&
            hw_slots[i].role == WIZ_ROLE_CLIENT) {
            return (int)i;
        }
    }
    return -1;
}

static int find_free_hw_socket(void)
{
    uint8_t i;
    uint8_t count = wiznet_hw_socket_count();
    for (i = 0U; i < count; ++i) {
        if (hw_slots[i].role == WIZ_ROLE_FREE) {
            return (int)i;
        }
    }
    return -1;
}

static bool listener_has_hw_socket(uint8_t listener_index)
{
    uint8_t i;
    uint8_t count = wiznet_hw_socket_count();
    for (i = 0U; i < count; ++i) {
        if (hw_slots[i].role == WIZ_ROLE_LISTENER &&
            hw_slots[i].listener_index == listener_index) {
            return true;
        }
    }
    return false;
}

static bool open_listener_socket(uint8_t listener_index)
{
    int socket_number;

    if (!listeners[listener_index].used ||
        listener_has_hw_socket(listener_index)) {
        return true;
    }

    socket_number = find_free_hw_socket();
    if (socket_number < 0) {
        WIZDGB("WIZnet backend: no free hardware socket for listener %u port %u\n",
               (unsigned int)listener_index,
               (unsigned int)listeners[listener_index].port);
        return false;
    }

    if (!wiznet_hw_socket_open_tcp_server((uint8_t)socket_number,
                                           listeners[listener_index].port)) {
        WIZDGB("WIZnet backend: failed to open hardware socket %u for listener %u\n",
               (unsigned int)socket_number,
               (unsigned int)listener_index);
        return false;
    }

    hw_slots[socket_number].role = WIZ_ROLE_LISTENER;
    hw_slots[socket_number].listener_index = listener_index;
    hw_slots[socket_number].tx_pending = false;
    WIZDGB("WIZnet backend: listener %u assigned hardware socket %u\n",
           (unsigned int)listener_index, (unsigned int)socket_number);
    return true;
}

static void release_hw_socket(uint8_t socket_number)
{
    WIZDGB("WIZnet backend: releasing hardware socket %u, role=%u\n",
           (unsigned int)socket_number,
           (unsigned int)hw_slots[socket_number].role);
    wiznet_hw_socket_close(socket_number);
    hw_slots[socket_number].role = WIZ_ROLE_FREE;
    hw_slots[socket_number].listener_index = 0U;
    hw_slots[socket_number].tx_pending = false;
}

static void disconnect_client(uint8_t socket_number, int reason)
{
    socket_handle_t handle = client_handle(socket_number);

    WIZDGB("WIZnet backend: client on socket %u disconnected, reason=%d\n",
           (unsigned int)socket_number, reason);
    release_hw_socket(socket_number);
    if (backend_events != NULL && backend_events->disconnected != NULL) {
        backend_events->disconnected(handle, reason);
    }
}

static int wiz_backend_init(const socket_device_events_t *events)
{
    uint8_t i;

    if (!wiznet_is_ready() || events == NULL) {
        WIZDGB("WIZnet backend: initialization rejected, ready=%u events=%p\n",
               wiznet_is_ready() ? 1U : 0U, (const void *)events);
        return SOCKET_DEVICE_ERROR;
    }

    backend_events = events;
    memset(listeners, 0, sizeof(listeners));
    for (i = 0U; i < WIZNET_MAX_HW_SOCKETS; ++i) {
        hw_slots[i].role = WIZ_ROLE_FREE;
        hw_slots[i].listener_index = 0U;
        hw_slots[i].socket_number = i;
        hw_slots[i].tx_pending = false;
        last_socket_status[i] = 0xFFU;
    }
    service_cursor = 0U;
    listener_cursor = 0U;
    WIZDGB("WIZnet backend: initialized with %u hardware sockets\n",
           (unsigned int)wiznet_hw_socket_count());
    return SOCKET_DEVICE_OK;
}

static socket_handle_t wiz_backend_listen(uint32_t ip_listen, uint16_t port,
                                           int domain, int type, int protocol,
                                           uint8_t backlog)
{
    uint8_t i;
    ipv4_address_t local_ip = wiznet_get_ip();

    (void)backlog; /* Capacity is limited by the physical hardware sockets. */

    WIZDGB("WIZnet backend: listen request IP=0x%08lX port=%u domain=%d type=%d protocol=%d backlog=%u\n",
           (unsigned long)ip_listen, (unsigned int)port, domain, type,
           protocol, (unsigned int)backlog);

    if (!wiznet_is_ready() || port == 0U || domain != AF_INET ||
        type != SOCK_STREAM || (protocol != 0 && protocol != 6) ||
        (ip_listen != IP_ANY && ip_listen != local_ip.ip)) {
        WIZDGB("WIZnet backend: listen request rejected; ready=%u local IP=%u.%u.%u.%u\n",
               wiznet_is_ready() ? 1U : 0U,
               (unsigned int)local_ip.octets[0],
               (unsigned int)local_ip.octets[1],
               (unsigned int)local_ip.octets[2],
               (unsigned int)local_ip.octets[3]);
        return SOCKET_INVALID_HANDLE;
    }

    for (i = 0U; i < MAX_SOCKETS; ++i) {
        if (!listeners[i].used) {
            listeners[i].used = true;
            listeners[i].port = port;
            listeners[i].domain = domain;
            listeners[i].type = type;
            listeners[i].protocol = protocol;
            if (!open_listener_socket(i)) {
                WIZDGB("WIZnet backend: listener %u creation failed\n",
                       (unsigned int)i);
                memset(&listeners[i], 0, sizeof(listeners[i]));
                return SOCKET_INVALID_HANDLE;
            }
            WIZDGB("WIZnet backend: listener %u created on port %u\n",
                   (unsigned int)i, (unsigned int)port);
            return listener_handle(i);
        }
    }
    WIZDGB("WIZnet backend: listen rejected; logical listener table full\n");
    return SOCKET_INVALID_HANDLE;
}

static int wiz_backend_send(socket_handle_t handle, const void *data,
                            size_t length, int flags)
{
    int index;
    uint8_t status;
    int result;

    (void)flags;
    if (data == NULL || length == 0U) {
        WIZDGB("WIZnet backend: invalid send data=%p length=%lu\n", data,
               (unsigned long)length);
        return (length == 0U) ? 0 : SOCKET_DEVICE_INVALID;
    }

    index = client_index_from_handle(handle);
    if (index < 0) {
        WIZDGB("WIZnet backend: send rejected for unknown client handle\n");
        return SOCKET_DEVICE_INVALID;
    }
    if (hw_slots[index].tx_pending) {
        WIZDGB("WIZnet backend: socket %d send would block; SEND_OK pending\n",
               index);
        return SOCKET_DEVICE_WOULD_BLOCK;
    }

    status = wiznet_hw_socket_status((uint8_t)index);
    if (status != WIZ_SNSR_ESTABLISHED && status != WIZ_SNSR_CLOSE_WAIT) {
        WIZDGB("WIZnet backend: socket %d send rejected, status=0x%02X\n",
               index, (unsigned int)status);
        return SOCKET_DEVICE_CLOSED;
    }

    result = wiznet_hw_socket_send((uint8_t)index, (const uint8_t *)data,
                                   length);
    if (result == 0) {
        WIZDGB("WIZnet backend: socket %d send would block in hardware\n",
               index);
        return SOCKET_DEVICE_WOULD_BLOCK;
    }
    if (result < 0) {
        WIZDGB("WIZnet backend: socket %d send failed\n", index);
        return SOCKET_DEVICE_ERROR;
    }
    hw_slots[index].tx_pending = true;
    WIZDGB("WIZnet backend: socket %d queued %d TX bytes\n", index, result);
    return result;
}

static int wiz_backend_close(socket_handle_t handle)
{
    int index = client_index_from_handle(handle);
    uint8_t i;

    if (index >= 0) {
        WIZDGB("WIZnet backend: local close of client socket %d\n", index);
        release_hw_socket((uint8_t)index);
        return SOCKET_DEVICE_OK;
    }

    index = listener_index_from_handle(handle);
    if (index < 0) {
        WIZDGB("WIZnet backend: close rejected for unknown handle\n");
        return SOCKET_DEVICE_INVALID;
    }

    listeners[index].used = false;
    WIZDGB("WIZnet backend: closing listener %d port %u\n", index,
           (unsigned int)listeners[index].port);
    for (i = 0U; i < wiznet_hw_socket_count(); ++i) {
        if (hw_slots[i].role == WIZ_ROLE_LISTENER &&
            hw_slots[i].listener_index == (uint8_t)index) {
            release_hw_socket(i);
        }
    }
    memset(&listeners[index], 0, sizeof(listeners[index]));
    return SOCKET_DEVICE_OK;
}

static void service_listener(uint8_t socket_number, uint8_t status)
{
    uint8_t index = hw_slots[socket_number].listener_index;

    if (!listeners[index].used) {
        WIZDGB("WIZnet backend: socket %u belongs to stale listener %u\n",
               (unsigned int)socket_number, (unsigned int)index);
        release_hw_socket(socket_number);
        return;
    }

    if (status == WIZ_SNSR_ESTABLISHED || status == WIZ_SNSR_CLOSE_WAIT) {
        bool accepted = false;
        socket_handle_t handle = client_handle(socket_number);

        hw_slots[socket_number].role = WIZ_ROLE_CLIENT;
        hw_slots[socket_number].tx_pending = false;
        wiznet_hw_socket_clear_interrupt(socket_number, WIZ_SNIR_CON);
        if (backend_events != NULL && backend_events->connected != NULL) {
            accepted = backend_events->connected(listener_handle(index), handle);
        }
        WIZDGB("WIZnet backend: connection on socket %u for listener %u %s\n",
               (unsigned int)socket_number, (unsigned int)index,
               accepted ? "accepted" : "rejected");
        if (!accepted) {
            release_hw_socket(socket_number);
        }
        (void)open_listener_socket(index);
    } else if (status != WIZ_SNSR_LISTEN) {
        WIZDGB("WIZnet backend: listener socket %u left LISTEN, status=0x%02X; reopening\n",
               (unsigned int)socket_number, (unsigned int)status);
        release_hw_socket(socket_number);
        (void)open_listener_socket(index);
    }
}

static void service_client(uint8_t socket_number, uint8_t status)
{
    uint8_t interrupts = wiznet_hw_socket_interrupt(socket_number);

    if ((interrupts & WIZ_SNIR_TIMEOUT) != 0U) {
        WIZDGB("WIZnet backend: socket %u timeout interrupt, IR=0x%02X\n",
               (unsigned int)socket_number, (unsigned int)interrupts);
        wiznet_hw_socket_clear_interrupt(socket_number, WIZ_SNIR_TIMEOUT);
        disconnect_client(socket_number, SOCKET_DEVICE_ERROR);
        return;
    }

    if (status == WIZ_SNSR_CLOSED || status == WIZ_SNSR_FIN_WAIT ||
        status == WIZ_SNSR_CLOSING || status == WIZ_SNSR_TIME_WAIT ||
        status == WIZ_SNSR_LAST_ACK) {
        WIZDGB("WIZnet backend: socket %u entered closing status 0x%02X\n",
               (unsigned int)socket_number, (unsigned int)status);
        disconnect_client(socket_number, 0);
        return;
    }

    if ((interrupts & WIZ_SNIR_SEND_OK) != 0U) {
        WIZDGB("WIZnet backend: socket %u SEND_OK, IR=0x%02X\n",
               (unsigned int)socket_number, (unsigned int)interrupts);
        wiznet_hw_socket_clear_interrupt(socket_number, WIZ_SNIR_SEND_OK);
        if (hw_slots[socket_number].tx_pending) {
            hw_slots[socket_number].tx_pending = false;
            if (backend_events != NULL && backend_events->writable != NULL) {
                backend_events->writable(client_handle(socket_number));
            }
            if (hw_slots[socket_number].role != WIZ_ROLE_CLIENT) {
                return;
            }
        }
    }

    if (status == WIZ_SNSR_ESTABLISHED || status == WIZ_SNSR_CLOSE_WAIT) {
        int received = wiznet_hw_socket_receive(
            socket_number, (uint8_t *)receive_buffer, SOCKET_MAX_DATA_SIZE);
        if (received > 0) {
            receive_buffer[received] = '\0';
            wiznet_hw_socket_clear_interrupt(socket_number, WIZ_SNIR_RECV);
            if (backend_events != NULL && backend_events->data != NULL) {
                WIZDGB("WIZnet backend: delivering %d RX bytes from socket %u\n",
                       received, (unsigned int)socket_number);
                backend_events->data(client_handle(socket_number),
                                     receive_buffer, (size_t)received);
            }
            return;
        }
        if (received < 0) {
            WIZDGB("WIZnet backend: socket %u receive failed\n",
                   (unsigned int)socket_number);
            disconnect_client(socket_number, SOCKET_DEVICE_ERROR);
            return;
        }
    }

    if (status == WIZ_SNSR_CLOSE_WAIT ||
        (interrupts & WIZ_SNIR_DISCON) != 0U) {
        WIZDGB("WIZnet backend: socket %u remote close, status=0x%02X IR=0x%02X\n",
               (unsigned int)socket_number, (unsigned int)status,
               (unsigned int)interrupts);
        if ((interrupts & WIZ_SNIR_DISCON) != 0U) {
            wiznet_hw_socket_clear_interrupt(socket_number, WIZ_SNIR_DISCON);
        }
        disconnect_client(socket_number, 0);
    }
}

static void wiz_backend_service(void)
{
    uint8_t count;
    uint8_t socket_number;

    if (!wiznet_is_ready()) {
        return;
    }

    count = wiznet_hw_socket_count();
    if (count == 0U) {
        return;
    }

    socket_number = service_cursor;
    service_cursor = (uint8_t)((service_cursor + 1U) % count);

    if (hw_slots[socket_number].role != WIZ_ROLE_FREE) {
        uint8_t status = wiznet_hw_socket_status(socket_number);
        if (status != last_socket_status[socket_number]) {
            WIZDGB("WIZnet backend: socket %u status 0x%02X -> 0x%02X, role=%u\n",
                   (unsigned int)socket_number,
                   (unsigned int)last_socket_status[socket_number],
                   (unsigned int)status,
                   (unsigned int)hw_slots[socket_number].role);
            last_socket_status[socket_number] = status;
        }
        if (hw_slots[socket_number].role == WIZ_ROLE_LISTENER) {
            service_listener(socket_number, status);
        } else if (hw_slots[socket_number].role == WIZ_ROLE_CLIENT) {
            service_client(socket_number, status);
        }
    }

    /* At most one missing listening socket is recreated per invocation. */
    if (listeners[listener_cursor].used) {
        (void)open_listener_socket(listener_cursor);
    }
    listener_cursor =
        (uint8_t)((listener_cursor + 1U) < MAX_SOCKETS
                      ? (listener_cursor + 1U)
                      : 0U);
}

static socket_device_t wiznet_socket_device = {
    .init = wiz_backend_init,
    .listen = wiz_backend_listen,
    .send = wiz_backend_send,
    .close = wiz_backend_close,
    .service = wiz_backend_service
};

bool wiznet_socket_backend_register(void)
{
    bool registered = socket_register_device(&wiznet_socket_device);
    WIZDGB("WIZnet backend: socket_register_device returned %u\n",
           registered ? 1U : 0U);
    return registered;
}
