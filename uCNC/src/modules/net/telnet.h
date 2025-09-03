/*
	Name: telnet.h
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

#ifndef TELNET_H
#define TELNET_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdint.h>
#include "socket.h"
#include "../../module.h"

/* Telnet protocol constants */
#define TELNET_IAC 255
#define TELNET_WILL 251
#define TELNET_WONT 252
#define TELNET_DO 253
#define TELNET_DONT 254

	typedef void (*telnet_onrecv_delegate_t)(uint8_t client_idx, void *data, size_t data_len);
	typedef struct telnet_protocol_
	{
		socket_if_t* telnet_socket;
		telnet_onrecv_delegate_t telnet_onrecv_cb;
	} telnet_protocol_t;

	socket_if_t *telnet_start_listen(telnet_protocol_t *telnet_protocol, int port);
	void telnet_stop(telnet_protocol_t *telnet_protocol);
	// gets how many clients are connected to the telnet server
	int telnet_hasclients(telnet_protocol_t* telnet);
	// sends data to a specific socket interface to a client
	int telnet_send(telnet_protocol_t *telnet, uint8_t client_idx, uint8_t *data, size_t data_len, int flags);
	// sends data to a specific socket interface to all clients
	int telnet_broadcast(telnet_protocol_t *telnet, uint8_t *data, size_t data_len, int flags);

#ifdef __cplusplus
}
#endif

#endif
