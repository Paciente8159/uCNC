/*
	Name: bsd_socket.c
	Description: Implements a simple Telnet Server based on BSD/POSIX Sockets for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 20-08-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../cnc.h"
#include "bsd_socket.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Telnet protocol constants */
#define TELNET_IAC  255
#define TELNET_WILL 251
#define TELNET_WONT 252
#define TELNET_DO   253
#define TELNET_DONT 254

#define TELNET_PORT 23

static socket_if_t *telnet_srv = NULL;

/* Send a Telnet command using the abstraction layer */
static void telnet_send_option(uint8_t cmd, uint8_t option) {
    uint8_t buf[3] = { TELNET_IAC, cmd, option };
    socket_send(telnet_srv, (char*)buf, sizeof(buf), 0);
}

static void telnet_negotiate(void) {
    /* Refuse all options for simplicity */
    telnet_send_option(TELNET_WILL, 0);
    telnet_send_option(TELNET_WONT, 0);
    telnet_send_option(TELNET_DO,   0);
    telnet_send_option(TELNET_DONT, 0);
}

/* Telnet payload handler */
static void telnet_data_handler(void* data, size_t len) {
    char *bytes = (char*)data;
    char outbuf[SOCKET_MAX_DATA_SIZE];
    size_t outlen = 0;

    for (size_t i = 0; i < len; ) {
        if ((uint8_t)bytes[i] == TELNET_IAC) {
            if (i + 2 < len) {
                i += 3; /* skip IAC sequence */
            } else break;
        } else {
            char ch = bytes[i++];
            if (isprint((unsigned char)ch) || ch == '\r' || ch == '\n') {
                if (outlen < sizeof(outbuf)) {
                    outbuf[outlen++] = ch;
                }
            }
        }
    }

    if (outlen > 0) {
        /* Echo back the received text */
        socket_send(telnet_srv, outbuf, outlen, 0);
        /* Optional: prompt */
        const char prompt[] = "\r\n> ";
        socket_send(telnet_srv, (char*)prompt, sizeof(prompt)-1, 0);
    }
}

int telnet_server_start(void) {
    socket_server_int();

    telnet_srv = socket_start(2 /*AF_INET*/, 1 /*SOCK_STREAM*/, 0, telnet_data_handler);
    if (!telnet_srv) return -1;

    struct bsd_sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = 2; /* AF_INET */
    addr.sin_port   = bsd_htons(TELNET_PORT);
    addr.sin_addr   = bsd_htonl(0); /* listen on all IFs */

    if (bsd_bind(telnet_srv->socket_if, &addr, sizeof(addr)) < 0) return -1;
    if (bsd_listen(telnet_srv->socket_if, SOCKET_MAX_CLIENTS) < 0) return -1;

    return 0;
}

void telnet_server_run(void) {
    socker_server_run();

    /* Negotiate with current client if needed */
    if (telnet_srv->current_client >= 0) {
        telnet_negotiate();
        const char welcome[] = "Welcome to Embedded Telnet\r\n> ";
        socket_send(telnet_srv, (char*)welcome, sizeof(welcome)-1, 0);
    }
}
