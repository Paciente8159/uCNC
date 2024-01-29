/*
	Name: websocket.h
	Description: Websocket for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 29-01-2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../cnc.h"
#include <stddef.h>

	typedef struct websocketclient_
	{
		uint8_t clientid;
		uint32_t ip;
	} websocketclient_t;

	typedef struct websocket_event_
	{
		uint8_t id;
		uint32_t ip;
		uint8_t event;
		uint8_t *data;
		size_t length;
	} websocket_event_t;

	DECL_MODULE(websocket);
	DECL_EVENT_HANDLER(websocket_client_connected);
	DECL_EVENT_HANDLER(websocket_client_disconnected);
	DECL_EVENT_HANDLER(websocket_client_client_data);

#ifdef __cplusplus
}
#endif

#endif