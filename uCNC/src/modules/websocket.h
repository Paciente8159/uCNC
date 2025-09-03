// /*
// 	Name: websocket.h
// 	Description: Websocket for µCNC.

// 	Copyright: Copyright (c) João Martins
// 	Author: João Martins
// 	Date: 29-01-2024

// 	µCNC is free software: you can redistribute it and/or modify
// 	it under the terms of the GNU General Public License as published by
// 	the Free Software Foundation, either version 3 of the License, or
// 	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

// 	µCNC is distributed WITHOUT ANY WARRANTY;
// 	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// 	See the	GNU General Public License for more details.
// */

// #ifndef WEBSOCKET_H
// #define WEBSOCKET_H

// #ifdef __cplusplus
// extern "C"
// {
// #endif

// #include "../cnc.h"
// #include <stddef.h>

// #define WS_EVENT_ERROR 0
// #define WS_EVENT_DISCONNECTED 1
// #define WS_EVENT_CONNECTED 2
// #define WS_EVENT_TEXT 3
// #define WS_EVENT_BIN 4
// #define WS_EVENT_FRAGMENT_TEXT_START 5
// #define WS_EVENT_FRAGMENT_BIN_START 6
// #define WS_EVENT_FRAGMENT 7
// #define WS_EVENT_FRAGMENT_FIN 8
// #define WS_EVENT_PING 9
// #define WS_EVENT_PONG 10

// #define WS_SEND_TXT 1
// #define WS_SEND_BIN 2
// #define WS_SEND_PING 4
// #define WS_SEND_TYPE (WS_SEND_TXT | WS_SEND_BIN | WS_SEND_PING)
// #define WS_SEND_BROADCAST 128

// 	typedef struct websocket_client_
// 	{
// 		uint8_t id;
// 		uint32_t ip;
// 	} websocket_client_t;

// 	typedef struct websocket_event_
// 	{
// 		uint8_t id;
// 		uint32_t ip;
// 		uint8_t event;
// 		uint8_t *data;
// 		size_t length;
// 	} websocket_event_t;

// 	/*These must be implemented by the MCU HAL*/
// 	DECL_EVENT_HANDLER(websocket_client_connected);
// 	DECL_EVENT_HANDLER(websocket_client_disconnected);
// 	DECL_EVENT_HANDLER(websocket_client_error);
// 	DECL_EVENT_HANDLER(websocket_client_receive);
// 	void __attribute__((weak)) websocket_send(uint8_t clientid, uint8_t *data, size_t length, uint8_t flags);

// #ifdef __cplusplus
// }
// #endif

// #endif