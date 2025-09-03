/*
	Name: websocket.h
	Description: Implements a WebSocket Server based on BSD/POSIX Sockets for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 28-08-2025

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

#include <stdlib.h>
#include <stdint.h>
#include "socket.h"
#include "../../module.h"
#include "utils/sha1.h"
#include "utils/base64.h"
#include "utils/http_request.h"

#define WS_OPCODE_ERROR 0
#define WS_OPCODE_DISCONNECTED 1
#define WS_OPCODE_CONNECTED 2
#define WS_OPCODE_TEXT 4
#define WS_OPCODE_BIN 8
#define WS_OPCODE_FRAGMENT_START 16
#define WS_OPCODE_FRAGMENT 32
#define WS_OPCODE_FRAGMENT_FIN 64
#define WS_OPCODE_PING 128
#define WS_OPCODE_PONG (WS_OPCODE_PING | WS_OPCODE_FRAGMENT_FIN)

#define WS_SEND_TXT 1
#define WS_SEND_BIN 2
#define WS_SEND_PING 4
#define WS_SEND_PONG 8
#define WS_SEND_CLOSE 16
#define WS_SEND_TYPE (WS_SEND_TXT | WS_SEND_BIN | WS_SEND_PING)
#define WS_SEND_BROADCAST 128

#ifndef WEBSOCKET_MAX_HEADER_BUF
#define WEBSOCKET_MAX_HEADER_BUF SOCKET_MAX_DATA_SIZE
#endif

#ifndef WEBSOCKET_MAX_CHUNK
#define WEBSOCKET_MAX_CHUNK SOCKET_MAX_DATA_SIZE
#endif

	// Websocket structs
	typedef enum
	{
		WS_S_HANDSHAKE = 0,
		WS_S_OPEN = 1,
		WS_S_CLOSING = 2
	} ws_status_t;

	typedef struct ws_client_state_
	{
		int active;
		ws_status_t status;

		// Handshake accumulation
    request_ctx_t req;
		request_header_t header;
		ws_handshake_t handshake;

		// Frame parsing
		uint8_t hdr[14];	/* At most 14 bytes header */
		uint8_t hdr_have; /* bytes filled in hdr[] */
		uint8_t hdr_need; /* total header length once known */
		uint8_t fin;
		uint8_t opcode;
		uint8_t masked;
		uint8_t mask[4];
		uint8_t mask_i;
		uint64_t payload_len;
		uint64_t payload_rem;

		// NEW: control frame buffering (<=125 bytes)
    uint8_t control_buf[125];
    uint8_t control_len;
		uint32_t timeout;
	} ws_client_state_t;

	typedef void (*websocket_onrecv_delegate_t)(uint8_t client_idx, void *data, size_t data_len, uint8_t flags);
	typedef void (*websocket_onopen_delegate_t)(uint8_t client_idx);
	typedef void (*websocket_onclose_delegate_t)(uint8_t client_idx, uint16_t code);
	typedef struct websocket_protocol_
	{
		socket_if_t* ws_socket;
		ws_client_state_t ws_clients[SOCKET_MAX_CLIENTS];
		websocket_onrecv_delegate_t ws_onrecv_cb;
		websocket_onopen_delegate_t ws_onopen_cb;
		websocket_onclose_delegate_t ws_onclose_cb;
	} websocket_protocol_t;

	socket_if_t *websocket_start_listen(websocket_protocol_t *ws, int port);
	int websocket_send(websocket_protocol_t *ws, uint8_t client_idx, const char *data, size_t len, uint8_t send_code);

#ifdef __cplusplus
}
#endif

#endif
