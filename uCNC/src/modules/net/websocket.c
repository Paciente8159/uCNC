/*
	Name: websocket.c
	Description: Small, allocation-free WebSocket server for uCNC.

	Copyright: Copyright (c) Joao Martins
	Author: Joao Martins

	uCNC is free software: you can redistribute it and/or modify it under the
	terms of the GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later
	version.
*/
#include "../../cnc.h"
#ifdef ENABLE_SOCKETS

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "websocket.h"
#include "utils/base64.h"
#include "utils/sha1.h"

/* Magic GUID defined by RFC 6455 section 4.2.2. */
static const char ws_guid[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

#ifndef WS_STREAM_CHUNK
#define WS_STREAM_CHUNK 256U
#endif

#if WS_STREAM_CHUNK == 0U
#error "WS_STREAM_CHUNK must be greater than zero"
#endif

/* A full control frame uses two header bytes plus at most 125 payload bytes. */
static void ws_reset_client(ws_client_state_t *client)
{
	if (client)
		memset(client, 0, sizeof(*client));
}

static void ws_frame_reset(ws_client_state_t *client)
{
	if (!client)
		return;
	client->state.frame.hdr_have = 0U;
	client->state.frame.hdr_need = 0U;
	client->state.frame.fin = 0U;
	client->state.frame.frame_opcode = 0U;
	client->state.frame.masked = 0U;
	client->state.frame.mask_i = 0U;
	client->state.frame.payload_len = 0U;
	client->state.frame.payload_rem = 0U;
	client->state.frame.control_len = 0U;
	memset(client->state.frame.hdr, 0, sizeof(client->state.frame.hdr));
	/* message_opcode deliberately survives between fragmented data frames. */
}

static void ws_notify_close(websocket_protocol_t *ws,
							uint8_t client_idx,
							ws_client_state_t *client,
							uint16_t code)
{
	if (!ws || !client || client->close_notified)
		return;
	client->close_notified = true;
	client->close_code = code;
	if (ws->ws_onclose_cb)
		ws->ws_onclose_cb(client_idx, code);
}

static int ws_send_handshake(websocket_protocol_t *ws,
							 uint8_t client_idx,
							 const void *data,
							 size_t data_len,
							 bool open_after)
{
	ws_client_state_t *client;
	int sent;

	if (!ws || !ws->ws_socket || client_idx >= SOCKET_MAX_CLIENTS ||
		(!data && data_len != 0U))
		return SOCKET_DEVICE_INVALID;
	if (!socket_client_is_connected(ws->ws_socket, client_idx))
		return SOCKET_DEVICE_CLOSED;

	client = &ws->ws_clients[client_idx];
	sent = socket_send(ws->ws_socket, client_idx, data, data_len, false);
	if (sent < 0)
	{
		if (socket_client_is_connected(ws->ws_socket, client_idx))
			(void)socket_close(ws->ws_socket, client_idx);
		return sent;
	}

	if (open_after)
	{
		memset(&client->state.frame, 0, sizeof(client->state.frame));
		client->status = WS_S_OPEN;
		if (ws->ws_onopen_cb)
			ws->ws_onopen_cb(client_idx);
	}
	else
	{
		client->status = WS_S_CLOSING;
		(void)socket_close(ws->ws_socket, client_idx);
	}
	return sent;
}

static size_t ws_frame_header(uint8_t *header, uint8_t opcode, size_t payload_len)
{
	if (payload_len <= 125U)
	{
		header[0] = (uint8_t)(0x80U | (opcode & 0x0FU));
		header[1] = (uint8_t)payload_len;
		return 2U;
	}
	if (payload_len <= 0xFFFFU)
	{
		header[0] = (uint8_t)(0x80U | (opcode & 0x0FU));
		header[1] = 126U;
		header[2] = (uint8_t)(payload_len >> 8);
		header[3] = (uint8_t)payload_len;
		return 4U;
	}
	else
	{
		uint64_t value = (uint64_t)payload_len;
		header[0] = (uint8_t)(0x80U | (opcode & 0x0FU));
		header[1] = 127U;
		header[2] = (uint8_t)(value >> 56);
		header[3] = (uint8_t)(value >> 48);
		header[4] = (uint8_t)(value >> 40);
		header[5] = (uint8_t)(value >> 32);
		header[6] = (uint8_t)(value >> 24);
		header[7] = (uint8_t)(value >> 16);
		header[8] = (uint8_t)(value >> 8);
		header[9] = (uint8_t)value;
		return 10U;
	}
}

/* Send one complete frame. A partial-frame failure makes the stream unusable. */
static int ws_send_frame(websocket_protocol_t *ws,
						 uint8_t client_idx,
						 uint8_t opcode,
						 const uint8_t *payload,
						 size_t payload_len,
						 bool mark_close)
{
	ws_client_state_t *client;
	uint8_t header[10];
	size_t header_len;
	int sent;

	if (!ws || !ws->ws_socket || client_idx >= SOCKET_MAX_CLIENTS ||
		(!payload && payload_len != 0U) || payload_len > (size_t)INT_MAX - 10U)
		return SOCKET_DEVICE_INVALID;
	if (!socket_client_is_connected(ws->ws_socket, client_idx))
		return SOCKET_DEVICE_CLOSED;
	if ((opcode & 0x08U) != 0U && payload_len > 125U)
		return SOCKET_DEVICE_INVALID;

	client = &ws->ws_clients[client_idx];
	if (!client->active || client->status != WS_S_OPEN)
		return SOCKET_DEVICE_CLOSED;

	header_len = ws_frame_header(header, opcode, payload_len);
	sent = socket_send(ws->ws_socket, client_idx, header, header_len, false);
	if (sent < 0)
		return sent;
	if (payload_len != 0U)
	{
		sent = socket_send(ws->ws_socket, client_idx, payload, payload_len, false);
		if (sent < 0)
		{
			if (socket_client_is_connected(ws->ws_socket, client_idx))
				(void)socket_close(ws->ws_socket, client_idx);
			return sent;
		}
	}

	if (mark_close)
	{
		client->status = WS_S_CLOSING;
		(void)socket_close(ws->ws_socket, client_idx);
	}
	return (int)(header_len + payload_len);
}

/*
 * Parses a complete accumulated frame header.
 * Returns 0 when ready, 1 when more header bytes are needed, or -1 for an RFC
 * protocol error. Control opcodes never overwrite fragmented-message state.
 */
static int ws_parse_header(ws_client_state_t *client)
{
	uint8_t b0;
	uint8_t b1;
	uint8_t opcode;
	uint8_t fin;
	uint64_t payload_len;
	size_t index;
	size_t need;
	bool control;

	if (!client || client->state.frame.hdr_have < 2U)
		return 1;
	b0 = client->state.frame.hdr[0];
	b1 = client->state.frame.hdr[1];
	fin = (uint8_t)(b0 >> 7);
	opcode = (uint8_t)(b0 & 0x0FU);
	payload_len = (uint64_t)(b1 & 0x7FU);
	if ((b0 & 0x70U) != 0U || (b1 & 0x80U) == 0U)
		return -1;
	if (!(opcode == 0x0U || opcode == 0x1U || opcode == 0x2U ||
		  opcode == 0x8U || opcode == 0x9U || opcode == 0xAU))
		return -1;

	need = 2U;
	if (payload_len == 126U)
		need += 2U;
	else if (payload_len == 127U)
		need += 8U;
	need += 4U; /* client-to-server masking key is mandatory */
	client->state.frame.hdr_need = (uint8_t)need;
	if (client->state.frame.hdr_have < client->state.frame.hdr_need)
		return 1;

	index = 2U;
	if (payload_len == 126U)
	{
		payload_len = ((uint64_t)client->state.frame.hdr[index] << 8) |
					  client->state.frame.hdr[index + 1U];
		if (payload_len < 126U)
			return -1;
		index += 2U;
	}
	else if (payload_len == 127U)
	{
		unsigned int i;
		if ((client->state.frame.hdr[index] & 0x80U) != 0U)
			return -1;
		payload_len = 0U;
		for (i = 0U; i < 8U; ++i)
			payload_len = (payload_len << 8) | client->state.frame.hdr[index + i];
		if (payload_len <= 0xFFFFU)
			return -1;
		index += 8U;
	}

	control = (opcode & 0x08U) != 0U;
	if (control && (!fin || payload_len > 125U))
		return -1;
	if (opcode == 0x0U)
	{
		if (client->state.frame.message_opcode == 0U)
			return -1;
	}
	else if (!control)
	{
		if (client->state.frame.message_opcode != 0U)
			return -1;
		if (!fin)
			client->state.frame.message_opcode = opcode;
	}

	client->state.frame.fin = fin;
	client->state.frame.frame_opcode = opcode;
	client->state.frame.masked = 1U;
	client->state.frame.mask[0] = client->state.frame.hdr[index];
	client->state.frame.mask[1] = client->state.frame.hdr[index + 1U];
	client->state.frame.mask[2] = client->state.frame.hdr[index + 2U];
	client->state.frame.mask[3] = client->state.frame.hdr[index + 3U];
	client->state.frame.mask_i = 0U;
	client->state.frame.payload_len = payload_len;
	client->state.frame.payload_rem = payload_len;
	return 0;
}

static void ws_dispatch_payload(websocket_protocol_t *ws,
								ws_client_state_t *client,
								uint8_t client_idx,
								const uint8_t *data,
								size_t data_len,
								bool final_chunk)
{
	uint8_t effective_opcode;
	uint8_t flags = 0U;

	if (!ws || !client || !ws->ws_onrecv_cb)
		return;
	effective_opcode = client->state.frame.frame_opcode == 0U ? client->state.frame.message_opcode
												  : client->state.frame.frame_opcode;
	flags = effective_opcode == 0x1U ? WS_OPCODE_TEXT : WS_OPCODE_BIN;
	if (client->state.frame.frame_opcode == 0U)
		flags |= WS_OPCODE_FRAGMENT;
	else if (!client->state.frame.fin)
		flags |= WS_OPCODE_FRAGMENT_START;
	if (final_chunk && client->state.frame.fin)
		flags |= WS_OPCODE_FRAGMENT_FIN;
	ws->ws_onrecv_cb(client_idx, data, data_len, flags);
}

static void ws_protocol_error_close(websocket_protocol_t *ws,
									uint8_t client_idx,
									ws_client_state_t *client,
									uint16_t code)
{
	uint8_t payload[2];
	int queued;

	if (!ws || !client)
		return;
	payload[0] = (uint8_t)(code >> 8);
	payload[1] = (uint8_t)code;
	client->close_code = code;
	queued = ws_send_frame(ws, client_idx, 0x8U, payload, sizeof(payload), true);
	if (queued < 0)
	{
		client->status = WS_S_CLOSING;
		(void)socket_close(ws->ws_socket, client_idx);
	}
}

static void ws_handle_control_complete(websocket_protocol_t *ws,
									   ws_client_state_t *client,
									   uint8_t client_idx)
{
	int queued;

	if (client->state.frame.frame_opcode == 0x9U)
	{
		queued = ws_send_frame(ws, client_idx, 0xAU,
								client->state.frame.control_buf, client->state.frame.control_len, false);
		if (queued < 0)
			(void)socket_close(ws->ws_socket, client_idx);
	}
	else if (client->state.frame.frame_opcode == 0x8U)
	{
		uint16_t code = 1000U;
		if (client->state.frame.control_len == 1U)
		{
			ws_protocol_error_close(ws, client_idx, client, 1002U);
			return;
		}
		if (client->state.frame.control_len >= 2U)
			code = (uint16_t)(((uint16_t)client->state.frame.control_buf[0] << 8) |
							  client->state.frame.control_buf[1]);
		ws_notify_close(ws, client_idx, client, code);
		client->close_code = code;
		queued = ws_send_frame(ws, client_idx, 0x8U,
								client->state.frame.control_buf, client->state.frame.control_len, true);
		if (queued < 0)
		{
			client->status = WS_S_CLOSING;
			(void)socket_close(ws->ws_socket, client_idx);
		}
	}
	client->state.frame.control_len = 0U;
}

static void ws_on_connected(uint8_t client_idx, void *protocol)
{
	websocket_protocol_t *ws = (websocket_protocol_t *)protocol;
	ws_client_state_t *client;

	if (!ws || client_idx >= SOCKET_MAX_CLIENTS)
		return;
	client = &ws->ws_clients[client_idx];
	ws_reset_client(client);
	client->active = true;
	client->status = WS_S_HANDSHAKE;
}

static void ws_on_disconnected(uint8_t client_idx, int reason, void *protocol)
{
	websocket_protocol_t *ws = (websocket_protocol_t *)protocol;
	ws_client_state_t *client;
	(void)reason;

	if (!ws || client_idx >= SOCKET_MAX_CLIENTS)
		return;
	client = &ws->ws_clients[client_idx];
	if (!client->active)
		return;
	if (client->status == WS_S_OPEN || client->status == WS_S_CLOSING)
		ws_notify_close(ws, client_idx, client,
						client->close_code != 0U ? client->close_code : 1001U);
	ws_reset_client(client);
}

static void ws_handle_handshake(websocket_protocol_t *ws,
								ws_client_state_t *client,
								uint8_t client_idx,
								const uint8_t *input,
								size_t input_len)
{
	/* The request parser advances the pointer but must treat pointed bytes as read-only. */
	char *cursor = (char *)(uintptr_t)input;
	size_t remaining = input_len;

	while (remaining != 0U && client->status == WS_S_HANDSHAKE)
	{
		if (client->state.hs.req.status != REQ_START_FINISHED)
		{
			http_request_parse_start(&client->state.hs.req, &cursor, &remaining);
			memset(&client->state.hs.header, 0, sizeof(client->state.hs.header));
		}
		if (client->state.hs.req.status == REQ_START_FINISHED)
		{
			http_request_parse_header(&client->state.hs.header, &cursor, &remaining);
			if (client->state.hs.header.status == REQ_HEAD_FINISHED)
			{
				if (client->state.hs.header.name[0] == '\0')
				{
					if (client->state.hs.handshake.hs_got_upgrade &&
						client->state.hs.handshake.hs_got_connection &&
						client->state.hs.handshake.hs_got_key &&
						client->state.hs.handshake.hs_got_version)
					{
						uint8_t digest[20];
						sha1_ctx sha1;
						char accept[64];
						char response[256];
						int length;

						sha1_init(&sha1);
						sha1_update(&sha1, (const uint8_t *)client->state.hs.handshake.hs_key,
									strlen(client->state.hs.handshake.hs_key));
						sha1_update(&sha1, (const uint8_t *)ws_guid, strlen(ws_guid));
						sha1_final(&sha1, digest);
						base64_encode(digest, sizeof(digest), accept, sizeof(accept));
						length = str_snprintf(response, sizeof(response),
											  "HTTP/1.1 101 Switching Protocols\r\n"
											  "Upgrade: websocket\r\n"
											  "Connection: Upgrade\r\n"
											  "Sec-WebSocket-Accept: %s\r\n\r\n",
											  accept);
						if (length <= 0 || (size_t)length >= sizeof(response) ||
							ws_send_handshake(ws, client_idx, response,
											  (size_t)length, true) < 0)
							(void)socket_close(ws->ws_socket, client_idx);
					}
					else
					{
						static const char bad_request[] =
							"HTTP/1.1 400 Bad Request\r\n"
							"Connection: close\r\n"
							"Content-Length: 0\r\n\r\n";
						if (ws_send_handshake(ws, client_idx, bad_request,
											  sizeof(bad_request) - 1U, false) < 0)
							(void)socket_close(ws->ws_socket, client_idx);
					}
					return;
				}
				http_request_ws_handshake(&client->state.hs.handshake, &client->state.hs.header);
				memset(&client->state.hs.header, 0, sizeof(client->state.hs.header));
			}
		}
	}
}

static void ws_on_data(uint8_t client_idx,
					   const uint8_t *data,
					   size_t data_len,
					   void *protocol)
{
	websocket_protocol_t *ws = (websocket_protocol_t *)protocol;
	ws_client_state_t *client;
	const uint8_t *cursor = data;
	size_t remaining = data_len;

	if (!ws || client_idx >= SOCKET_MAX_CLIENTS || (!data && data_len != 0U))
		return;
	client = &ws->ws_clients[client_idx];
	if (!client->active)
		return;
	if (client->status == WS_S_HANDSHAKE)
	{
		ws_handle_handshake(ws, client, client_idx, data, data_len);
		return;
	}
	if (client->status != WS_S_OPEN)
		return;

	while (remaining != 0U && client->status == WS_S_OPEN)
	{
		if (client->state.frame.hdr_need == 0U || client->state.frame.hdr_have < client->state.frame.hdr_need)
		{
			if (client->state.frame.hdr_have < 2U)
			{
				size_t needed = 2U - client->state.frame.hdr_have;
				size_t take = remaining < needed ? remaining : needed;
				memcpy(&client->state.frame.hdr[client->state.frame.hdr_have], cursor, take);
				client->state.frame.hdr_have += (uint8_t)take;
				cursor += take;
				remaining -= take;
				if (client->state.frame.hdr_have < 2U)
					break;
			}
			if (client->state.frame.hdr_need == 0U)
			{
				size_t needed = 2U;
				uint8_t short_len = (uint8_t)(client->state.frame.hdr[1] & 0x7FU);
				if (short_len == 126U)
					needed += 2U;
				else if (short_len == 127U)
					needed += 8U;
				if ((client->state.frame.hdr[1] & 0x80U) != 0U)
					needed += 4U;
				client->state.frame.hdr_need = (uint8_t)needed;
			}
			if (client->state.frame.hdr_have < client->state.frame.hdr_need)
			{
				size_t needed = client->state.frame.hdr_need - client->state.frame.hdr_have;
				size_t take = remaining < needed ? remaining : needed;
				memcpy(&client->state.frame.hdr[client->state.frame.hdr_have], cursor, take);
				client->state.frame.hdr_have += (uint8_t)take;
				cursor += take;
				remaining -= take;
				if (client->state.frame.hdr_have < client->state.frame.hdr_need)
					break;
			}
			if (ws_parse_header(client) < 0)
			{
				ws_protocol_error_close(ws, client_idx, client, 1002U);
				return;
			}
		}

		if (client->state.frame.payload_rem != 0U)
		{
			size_t available = client->state.frame.payload_rem < (uint64_t)remaining
								   ? (size_t)client->state.frame.payload_rem
								   : remaining;
			while (available != 0U)
			{
				uint8_t unmasked[WS_STREAM_CHUNK];
				size_t chunk = available > WS_STREAM_CHUNK ? WS_STREAM_CHUNK : available;
				size_t i;
				for (i = 0U; i < chunk; ++i)
					unmasked[i] = (uint8_t)(cursor[i] ^ client->state.frame.mask[(client->state.frame.mask_i++) & 3U]);

				if ((client->state.frame.frame_opcode & 0x08U) != 0U)
				{
					if (chunk > sizeof(client->state.frame.control_buf) - client->state.frame.control_len)
					{
						ws_protocol_error_close(ws, client_idx, client, 1002U);
						return;
					}
					memcpy(&client->state.frame.control_buf[client->state.frame.control_len], unmasked, chunk);
					client->state.frame.control_len += (uint8_t)chunk;
				}
				else
				{
					bool final_chunk = (uint64_t)chunk == client->state.frame.payload_rem;
					ws_dispatch_payload(ws, client, client_idx, unmasked, chunk, final_chunk);
				}
				cursor += chunk;
				remaining -= chunk;
				available -= chunk;
				client->state.frame.payload_rem -= chunk;
			}
		}

		if (client->state.frame.payload_rem == 0U)
		{
			if ((client->state.frame.frame_opcode & 0x08U) != 0U)
				ws_handle_control_complete(ws, client, client_idx);
			else
			{
				if (client->state.frame.payload_len == 0U)
					ws_dispatch_payload(ws, client, client_idx, NULL, 0U, true);
				if (client->state.frame.fin && client->state.frame.message_opcode != 0U)
					client->state.frame.message_opcode = 0U;
			}
			ws_frame_reset(client);
		}
	}
}

static int ws_send_one(websocket_protocol_t *ws,
					   uint8_t client_idx,
					   const uint8_t *data,
					   size_t len,
					   uint8_t type)
{
	uint8_t opcode;
	bool close_frame = false;
	uint8_t default_close[2] = {0x03U, 0xE8U};
	uint16_t close_code = 0U;
	int queued;
	if (!ws || client_idx >= SOCKET_MAX_CLIENTS)
		return SOCKET_DEVICE_INVALID;

	if (type == WS_SEND_CLOSE)
	{
		opcode = 0x8U;
		close_frame = true;
		if (len < 2U)
		{
			data = default_close;
			len = sizeof(default_close);
		}
		else if (len > 125U)
			return SOCKET_DEVICE_INVALID;
		close_code = (uint16_t)(((uint16_t)data[0] << 8) | data[1]);
	}
	else if (type == WS_SEND_PING || type == WS_SEND_PONG)
	{
		if (len > 125U)
			return SOCKET_DEVICE_INVALID;
		opcode = type == WS_SEND_PING ? 0x9U : 0xAU;
	}
	else
	{
		if (len > WEBSOCKET_MAX_CHUNK)
			return SOCKET_DEVICE_INVALID;
		opcode = type == WS_SEND_BIN ? 0x2U : 0x1U;
	}
	queued = ws_send_frame(ws, client_idx, opcode, data, len, close_frame);
	if (queued >= 0 && close_frame)
		ws->ws_clients[client_idx].close_code = close_code;
	return queued;
}

int websocket_send(websocket_protocol_t *ws,
				   uint8_t client_idx,
				   const void *data,
				   size_t len,
				   uint8_t send_code)
{
	uint8_t type = (uint8_t)(send_code & WS_SEND_TYPE);
	bool broadcast = (send_code & WS_SEND_BROADCAST) != 0U;
	int minimum = INT_MAX;
	bool found = false;
	uint8_t i;

	if (!ws || !ws->ws_socket || (!data && len != 0U) || type == 0U ||
		(type & (uint8_t)(type - 1U)) != 0U)
		return SOCKET_DEVICE_INVALID;
	if (!broadcast)
		return ws_send_one(ws, client_idx, (const uint8_t *)data, len, type);

	for (i = 0U; i < SOCKET_MAX_CLIENTS; ++i)
	{
		int queued;
		if (!socket_client_is_connected(ws->ws_socket, i) ||
			!ws->ws_clients[i].active || ws->ws_clients[i].status != WS_S_OPEN)
			continue;
		found = true;
		queued = ws_send_one(ws, i, (const uint8_t *)data, len, type);
		if (queued < minimum)
			minimum = queued;
	}
	return found ? minimum : 0;
}

socket_if_t *websocket_start_listen(websocket_protocol_t *ws, uint16_t port)
{
	socket_if_t *socket;
	uint8_t i;

	if (!ws)
		return NULL;
	LOAD_MODULE(socket_server);
	ws->ws_socket = NULL;
	for (i = 0U; i < SOCKET_MAX_CLIENTS; ++i)
		ws_reset_client(&ws->ws_clients[i]);

	socket = socket_start(IP_ANY, port);
	if (!socket)
		return NULL;
	ws->ws_socket = socket;
	socket_set_protocol(socket, ws);
	socket_add_ondata_handler(socket, ws_on_data);
	socket_add_onconnected_handler(socket, ws_on_connected);
	socket_add_ondisconnected_handler(socket, ws_on_disconnected);
	return socket;
}

void websocket_stop(websocket_protocol_t *ws)
{
	if (!ws || !ws->ws_socket)
		return;
	socket_stop(ws->ws_socket);
	ws->ws_socket = NULL;
}

#endif /* ENABLE_SOCKETS */
