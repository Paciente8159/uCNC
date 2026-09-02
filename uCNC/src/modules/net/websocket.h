/*
	Name: websocket.h
	Description: Small, allocation-free WebSocket server for uCNC.

	Copyright: Copyright (c) Joao Martins
	Author: Joao Martins

	uCNC is free software: you can redistribute it and/or modify it under the
	terms of the GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later
	version.
*/
#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "socket.h"
#include "../../module.h"
#include "utils/http_request.h"

#define WS_OPCODE_ERROR 0U
#define WS_OPCODE_DISCONNECTED 1U
#define WS_OPCODE_CONNECTED 2U
#define WS_OPCODE_TEXT 4U
#define WS_OPCODE_BIN 8U
#define WS_OPCODE_FRAGMENT_START 16U
#define WS_OPCODE_FRAGMENT 32U
#define WS_OPCODE_FRAGMENT_FIN 64U
#define WS_OPCODE_PING 128U
#define WS_OPCODE_PONG (WS_OPCODE_PING | WS_OPCODE_FRAGMENT_FIN)

#define WS_SEND_TXT 1U
#define WS_SEND_BIN 2U
#define WS_SEND_PING 4U
#define WS_SEND_PONG 8U
#define WS_SEND_CLOSE 16U
#define WS_SEND_TYPE (WS_SEND_TXT | WS_SEND_BIN | WS_SEND_PING | WS_SEND_PONG | WS_SEND_CLOSE)
#define WS_SEND_BROADCAST 128U

#ifndef WEBSOCKET_MAX_CHUNK
#define WEBSOCKET_MAX_CHUNK SOCKET_MAX_DATA_SIZE
#endif

typedef enum
{
	WS_S_HANDSHAKE = 0,
	WS_S_OPEN,
	WS_S_CLOSING
} ws_status_t;

typedef struct ws_handshake_state_
{
	request_ctx_t req;
	request_header_t header;
	ws_handshake_t handshake;
} ws_handshake_state_t;

typedef struct ws_frame_state_
{
	uint8_t hdr[14];
	uint8_t hdr_have;
	uint8_t hdr_need;
	uint8_t fin;
	uint8_t frame_opcode;
	uint8_t message_opcode;
	uint8_t masked;
	uint8_t mask[4];
	uint8_t mask_i;
	uint64_t payload_len;
	uint64_t payload_rem;
	uint8_t control_buf[125];
	uint8_t control_len;
} ws_frame_state_t;

typedef struct ws_client_state_
{
	/* Handshake state and frame-parser state never coexist. */
	union
	{
		ws_handshake_state_t hs;
		ws_frame_state_t frame;
	} state;

	uint16_t close_code;
	uint8_t status;
	bool active;
	bool close_notified;
} ws_client_state_t;

/*
 * Called synchronously while an unmasked payload chunk is borrowed.
 *
 * data points to data_len immutable bytes and is valid only during the call.
 * TCP and WebSocket message boundaries need not match callback boundaries.
 * flags identifies text/binary data and includes WS_OPCODE_FRAGMENT_FIN on the
 * final chunk of a complete message. data may be NULL only when data_len is 0.
 * The callback must copy any bytes it needs after return.
 */
typedef void (*websocket_onrecv_delegate_t)(uint8_t client_idx,
												const void *data,
												size_t data_len,
												uint8_t flags);

/*
 * Called once after the complete HTTP 101 response has entered the transport.
 * websocket_send() may be called from this callback and is synchronous.
 */
typedef void (*websocket_onopen_delegate_t)(uint8_t client_idx);

/*
 * Called at most once for an opened connection. code is the peer close code,
 * the locally requested/protocol-error code, or 1001 for an unannounced TCP
 * disconnect. The socket slot may already be invalid when this callback runs.
 */
typedef void (*websocket_onclose_delegate_t)(uint8_t client_idx, uint16_t code);

typedef struct websocket_protocol_
{
	socket_if_t *ws_socket;
	ws_client_state_t ws_clients[SOCKET_MAX_CLIENTS];
	websocket_onrecv_delegate_t ws_onrecv_cb;
	websocket_onopen_delegate_t ws_onopen_cb;
	websocket_onclose_delegate_t ws_onclose_cb;
} websocket_protocol_t;

/*
 * Starts a WebSocket listener and registers all socket callbacks.
 *
 * Contract:
 * - ws and its callback members must remain valid until websocket_stop();
 * - set callback members before calling; any member may be NULL;
 * - port is in host byte order and no dynamic memory is allocated.
 *
 * Returns the listener on success or NULL on invalid input/listen failure.
 * On failure ws->ws_socket is NULL.
 */
socket_if_t *websocket_start_listen(websocket_protocol_t *ws, uint16_t port);

/*
 * Stops the listener and schedules closure of all owned clients.
 * NULL and already-stopped instances are harmless. Pending TX is discarded.
 */
void websocket_stop(websocket_protocol_t *ws);

/*
 * Sends one complete unmasked server frame without retaining a TX copy.
 *
 * The small frame header is built on the stack, then header and payload are sent
 * synchronously with blocking socket_send(). Application data therefore remains
 * owned by the caller and is valid only for the duration of this call.
 *
 * Exactly one WS_SEND_TXT/BIN/PING/PONG/CLOSE type must be selected;
 * WS_SEND_BROADCAST may be ORed with it. Application payload is limited to
 * WEBSOCKET_MAX_CHUNK and control payloads to 125 bytes.
 *
 * Returns complete frame wire bytes on success, 0 for an empty broadcast target
 * set, or a negative socket_device_result_t on invalid state/timeout/transport
 * failure. A failed partial frame causes that TCP client to be closed.
 */
int websocket_send(websocket_protocol_t *ws,
				   uint8_t client_idx,
				   const void *data,
				   size_t len,
				   uint8_t send_code);

#ifdef __cplusplus
}
#endif

#endif /* WEBSOCKET_H */
