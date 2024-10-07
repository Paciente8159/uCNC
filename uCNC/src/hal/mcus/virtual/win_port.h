#ifndef WIN_PORT_H
#define WIN_PORT_H

#include "../../../cnc.h"
#if (MCU == MCU_VIRTUAL_WIN)

#include <stdint.h>
#include <stdbool.h>


typedef int(__attribute__((__stdcall__)) * available_delegate)(struct win_port_ *);
typedef int(__attribute__((__stdcall__)) * read_delegate)(struct win_port_ *, uint8_t *buffer, size_t len);
typedef int(__attribute__((__stdcall__)) * write_delegate)(struct win_port_ *, uint8_t *buffer, size_t len);

typedef struct win_port_
{
    char portname[32];
    void* handle;
    bool isclient;
    void* client;
    bool connected;
    available_delegate available_cb;
    read_delegate read_cb;
    write_delegate write_cb;
} win_port_t;

void namedpipe_init(win_port_t *port);
void console_init(win_port_t *port);
void socket_init(win_port_t *port);
void uart_init(win_port_t *port);

static int port_available(win_port_t *port)
{
    if (!port->available_cb)
    {
        return 0;
    }
    return port->available_cb(port);
}

static int port_write(win_port_t *port, uint8_t *buffer, int len)
{
    if (!port->write_cb)
    {
        return 0;
    }
    return port->write_cb(port, buffer, len);
}

static int port_read(win_port_t *port, uint8_t *buffer, int len)
{
    if (!port->read_cb)
    {
        return 0;
    }
    return port->read_cb(port, buffer, len);
}

#endif

#endif