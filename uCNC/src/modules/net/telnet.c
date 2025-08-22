/*
	Name: socket.c
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

#include "../../cnc.h"
#include "telnet.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifndef TELNET_PORT
#define TELNET_PORT 23
#endif

static socket_if_t *telnet_srv = NULL;

/* Send a Telnet command using the abstraction layer */
static void telnet_send_option(int client_index, uint8_t cmd, uint8_t option)
{
	uint8_t buf[3] = {TELNET_IAC, cmd, option};
	socket_send(telnet_srv, client_index, (char *)buf, sizeof(buf), 0);
}

static void telnet_negotiate(int client_index)
{
	/* Refuse all options for simplicity */
	telnet_send_option(client_index, TELNET_WILL, 0);
	telnet_send_option(client_index, TELNET_WONT, 0);
	telnet_send_option(client_index, TELNET_DO, 0);
	telnet_send_option(client_index, TELNET_DONT, 0);
}

CREATE_HOOK(telnet_onrecv);

/* Telnet payload handler */
static void telnet_data_handler(int client_index, void *data, size_t data_len)
{
	uint8_t *bytes = (uint8_t *)data;
	size_t outlen = 0;

	for (size_t i = 0; i < data_len;)
	{
		if ((uint8_t)bytes[i] == TELNET_IAC)
		{
			if (i + 2 < data_len)
			{
				i += 3; // Skip IAC sequence
			}
			else
			{
				break; // Incomplete IAC sequence
			}
		}
		else
		{
			char ch = bytes[i++];
			bytes[outlen++] = ch;
		}
	}

	if (outlen > 0)
	{
		// Invoke the hook with modified buffer and adjusted length
		HOOK_INVOKE(telnet_onrecv, data, outlen);
	}
}

static void telnet_new_client_handler(int client)
{
	telnet_negotiate(client);
	const char welcome[] = "Welcome to Embedded Telnet\r\n> ";
	socket_send(telnet_srv, client, (char *)welcome, sizeof(welcome) - 1, 0);
}

int telnet_hasclients(void)
{
	return socket_server_hasclients(telnet_srv);
}

DECL_MODULE(telnet_server)
{
	telnet_srv = socket_start(IP_ANY, TELNET_PORT, 2 /*AF_INET*/, 1 /*SOCK_STREAM*/, 0);
	if (!telnet_srv)
		return;

	socket_add_ondata_handler(telnet_srv, telnet_data_handler);
	socket_add_onconnected_handler(telnet_srv, telnet_new_client_handler);
}

void telnet_server_run(void)
{
	socket_server_run(telnet_srv);
}

// sends data to a specific socket interface to a client
int telnet_send(int client, char *data, size_t data_len, int flags)
{
	return socket_send(telnet_srv, client, data, data_len, flags);
}
// sends data to a specific socket interface to all clients
int telnet_broadcast(char *data, size_t data_len, int flags)
{
	return socket_broadcast(telnet_srv, data, data_len, flags);
}
