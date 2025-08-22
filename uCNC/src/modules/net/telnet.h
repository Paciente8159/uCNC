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

	DECL_MODULE(telnet_server);
	void telnet_server_run(void);
	// fires when new data is available
	DECL_HOOK(telnet_onrecv, void *data, size_t data_len);
	// gets how many clients are connected to the telnet server
	int telnet_hasclients(void);
	// sends data to a specific socket interface to a client
	int telnet_send(int client, char *data, size_t data_len, int flags);
	// sends data to a specific socket interface to all clients
	int telnet_broadcast(char *data, size_t data_len, int flags);

#ifdef __cplusplus
}
#endif

#endif
