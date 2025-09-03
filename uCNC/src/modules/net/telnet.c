/*
	Name: telnet.c
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

/* Send a Telnet command using the abstraction layer */
static void telnet_send_option(telnet_protocol_t* telnet, int client_fd, uint8_t cmd, uint8_t option)
{
	uint8_t buf[3] = {TELNET_IAC, cmd, option};
	socket_send(telnet->telnet_socket, client_fd, (char *)buf, sizeof(buf), 0);
}

static void telnet_negotiate(telnet_protocol_t* telnet, int client_fd)
{
	/* Refuse all options for simplicity */
	telnet_send_option(telnet, client_fd, TELNET_WILL, 0);
	telnet_send_option(telnet, client_fd, TELNET_WONT, 0);
	telnet_send_option(telnet, client_fd, TELNET_DO, 0);
	telnet_send_option(telnet, client_fd, TELNET_DONT, 0);
}

/* Telnet payload handler */
static void telnet_data_handler(uint8_t client_idx, char *data, size_t data_len, void* protocol)
{
	telnet_protocol_t* telnet = (telnet_protocol_t*) protocol;
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
		if(telnet && telnet->telnet_onrecv_cb){
			telnet->telnet_onrecv_cb(client_idx, data, outlen);
		}
	}
}

static void telnet_new_client_handler(int client, void* protocol)
{
	telnet_protocol_t* telnet = (telnet_protocol_t*) protocol;
	telnet_negotiate(telnet, client);
	const char welcome[] = "uCNC Telnet\r\n> ";
	socket_send(telnet->telnet_socket, client, (char *)welcome, sizeof(welcome) - 1, 0);
}

int telnet_hasclients(telnet_protocol_t* telnet)
{
	return socket_server_hasclients(telnet->telnet_socket);
}

socket_if_t *telnet_start_listen(telnet_protocol_t *telnet_protocol, int port)
{
	LOAD_MODULE(socket_server);
	socket_if_t *socket = socket_start_listen(IP_ANY, port, 2 /*AF_INET*/, 1 /*SOCK_STREAM*/, 0);
	if (!socket)
		return;

	socket_add_ondata_handler(socket, telnet_data_handler);
	socket_add_onconnected_handler(socket, telnet_new_client_handler);

	// binds the socket to the prototol
	socket->protocol = telnet_protocol;
	telnet_protocol->telnet_socket = socket;
	return socket;
}

void telnet_stop(telnet_protocol_t *telnet_protocol){
	socket_stop_listening(telnet_protocol->telnet_socket);
	telnet_protocol->telnet_socket = NULL;
}

// sends data to a specific socket interface to a client
int telnet_send(telnet_protocol_t *telnet, uint8_t client_idx, uint8_t *data, size_t data_len, int flags)
{
	return socket_send(telnet->telnet_socket, client_idx, data, data_len, flags);
}
// sends data to a specific socket interface to all clients
int telnet_broadcast(telnet_protocol_t *telnet, uint8_t *data, size_t data_len, int flags)
{
	return socket_broadcast(telnet->telnet_socket, data, data_len, flags);
}
