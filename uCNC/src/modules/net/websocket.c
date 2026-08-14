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
#define WS_CONTROL_FRAME_RESERVE 127U

static void ws_reset_client(ws_client_state_t *client)
{
	if (client)
		memset(client, 0, sizeof(*client));
}

static void ws_frame_reset(ws_client_state_t *client)
{
	if (!client)
		return;
	client->hdr_have = 0U;
	client->hdr_need = 0U;
	client->fin = 0U;
	client->frame_opcode = 0U;
	client->masked = 0U;
	client->mask_i = 0U;
	client->payload_len = 0U;
	client->payload_rem = 0U;
	client->control_len = 0U;
	memset(client->hdr, 0, sizeof(client->hdr));
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

/*
 * Makes exactly one nonblocking send attempt for persistent queued bytes.
 * This is the only WebSocket helper that calls socket_send(). A partial or
 * blocked suffix remains byte-for-byte stable until ws_on_writable() resumes it.
 */
static int ws_flush(websocket_protocol_t *ws, uint8_t client_idx)
{
	ws_client_state_t *client;
	int sent;

	if (!ws || !ws->ws_socket || client_idx >= SOCKET_MAX_CLIENTS)
		return SOCKET_DEVICE_INVALID;
	client = &ws->ws_clients[client_idx];

	if (client->tx_length != 0U)
	{
		sent = socket_send(ws->ws_socket,
						   client_idx,
						   &client->tx_buffer[client->tx_offset],
						   client->tx_length);
		if (sent > 0)
		{
			size_t consumed = (size_t)sent;
			if (consumed > client->tx_length)
				consumed = client->tx_length;
			client->tx_offset += consumed;
			client->tx_length -= consumed;
			if (client->tx_length == 0U)
				client->tx_offset = 0U;
		}
		else if (sent < 0 && sent != SOCKET_DEVICE_WOULD_BLOCK)
		{
			(void)socket_close(ws->ws_socket, client_idx);
			return sent;
		}
	}
	else
	{
		sent = 0;
		client->tx_offset = 0U;
	}

	if (client->tx_length == 0U)
	{
		if (client->open_after_tx)
		{
			client->open_after_tx = false;
			client->status = WS_S_OPEN;
			if (ws->ws_onopen_cb)
				ws->ws_onopen_cb(client_idx);
		}
		if (client->close_after_tx)
		{
			client->close_after_tx = false;
			(void)socket_close(ws->ws_socket, client_idx);
		}
	}
	return sent;
}

static void ws_compact_tx(ws_client_state_t *client)
{
	if (client && client->tx_offset != 0U)
	{
		memmove(client->tx_buffer,
				&client->tx_buffer[client->tx_offset],
				client->tx_length);
		client->tx_offset = 0U;
	}
}

/* Queues an HTTP handshake response atomically in persistent client storage. */
static int ws_queue_handshake(websocket_protocol_t *ws,
							  uint8_t client_idx,
							  const void *data,
							  size_t data_len,
							  bool open_after,
							  bool close_after)
{
	ws_client_state_t *client;
	size_t tail;
	bool was_pending;

	if (!ws || !ws->ws_socket || client_idx >= SOCKET_MAX_CLIENTS ||
		(!data && data_len != 0U))
		return SOCKET_DEVICE_INVALID;
	if (!socket_client_is_connected(ws->ws_socket, client_idx))
		return SOCKET_DEVICE_CLOSED;
	client = &ws->ws_clients[client_idx];
	was_pending = client->tx_length != 0U;
	ws_compact_tx(client);
	if (data_len > sizeof(client->tx_buffer) - client->tx_length)
		return SOCKET_DEVICE_WOULD_BLOCK;

	tail = client->tx_length;
	if (data_len != 0U)
		memcpy(&client->tx_buffer[tail], data, data_len);
	client->tx_length += data_len;
	client->open_after_tx = open_after;
	client->close_after_tx = close_after;
	client->status = open_after ? WS_S_HANDSHAKE_REPLY : WS_S_CLOSING;
	if (!was_pending)
		(void)ws_flush(ws, client_idx);
	return data_len > (size_t)INT_MAX ? INT_MAX : (int)data_len;
}

static size_t ws_frame_header_length(size_t payload_len)
{
	if (payload_len <= 125U)
		return 2U;
	if (payload_len <= 0xFFFFU)
		return 4U;
	return 10U;
}

/*
 * Appends a complete, unmasked server frame atomically. application_frame
 * reserves enough free space for one mandatory control response. mark_close
 * schedules TCP closure only after every prior byte and this CLOSE frame drain.
 */
static int ws_queue_frame(websocket_protocol_t *ws,
						  uint8_t client_idx,
						  uint8_t opcode,
						  const uint8_t *payload,
						  size_t payload_len,
						  bool application_frame,
						  bool mark_close)
{
	ws_client_state_t *client;
	size_t header_len;
	size_t required;
	size_t reserve = application_frame ? WS_CONTROL_FRAME_RESERVE : 0U;
	size_t tail;
	uint8_t *header;
	bool was_pending;

	if (!ws || !ws->ws_socket || client_idx >= SOCKET_MAX_CLIENTS ||
		(!payload && payload_len != 0U))
		return SOCKET_DEVICE_INVALID;
	if (!socket_client_is_connected(ws->ws_socket, client_idx))
		return SOCKET_DEVICE_CLOSED;
	if ((opcode & 0x08U) != 0U && payload_len > 125U)
		return SOCKET_DEVICE_INVALID;

	client = &ws->ws_clients[client_idx];
	if (!client->active || client->status != WS_S_OPEN || client->close_after_tx)
		return SOCKET_DEVICE_CLOSED;
	header_len = ws_frame_header_length(payload_len);
	if (payload_len > SIZE_MAX - header_len)
		return SOCKET_DEVICE_INVALID;
	required = header_len + payload_len;
	was_pending = client->tx_length != 0U;
	ws_compact_tx(client);
	if (required > sizeof(client->tx_buffer) - client->tx_length ||
		reserve > sizeof(client->tx_buffer) - client->tx_length - required)
		return SOCKET_DEVICE_WOULD_BLOCK;

	tail = client->tx_length;
	header = &client->tx_buffer[tail];
	header[0] = (uint8_t)(0x80U | (opcode & 0x0FU));
	if (payload_len <= 125U)
	{
		header[1] = (uint8_t)payload_len;
	}
	else if (payload_len <= 0xFFFFU)
	{
		header[1] = 126U;
		header[2] = (uint8_t)(payload_len >> 8);
		header[3] = (uint8_t)payload_len;
	}
	else
	{
		uint64_t value = (uint64_t)payload_len;
		header[1] = 127U;
		header[2] = (uint8_t)(value >> 56);
		header[3] = (uint8_t)(value >> 48);
		header[4] = (uint8_t)(value >> 40);
		header[5] = (uint8_t)(value >> 32);
		header[6] = (uint8_t)(value >> 24);
		header[7] = (uint8_t)(value >> 16);
		header[8] = (uint8_t)(value >> 8);
		header[9] = (uint8_t)value;
	}
	if (payload_len != 0U)
		memcpy(&header[header_len], payload, payload_len);
	client->tx_length += required;
	if (mark_close)
	{
		client->status = WS_S_CLOSING;
		client->close_after_tx = true;
	}
	if (!was_pending)
		(void)ws_flush(ws, client_idx);
	return required > (size_t)INT_MAX ? INT_MAX : (int)required;
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

	if (!client || client->hdr_have < 2U)
		return 1;
	b0 = client->hdr[0];
	b1 = client->hdr[1];
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
	client->hdr_need = (uint8_t)need;
	if (client->hdr_have < client->hdr_need)
		return 1;

	index = 2U;
	if (payload_len == 126U)
	{
		payload_len = ((uint64_t)client->hdr[index] << 8) |
					  client->hdr[index + 1U];
		if (payload_len < 126U)
			return -1;
		index += 2U;
	}
	else if (payload_len == 127U)
	{
		unsigned int i;
		if ((client->hdr[index] & 0x80U) != 0U)
			return -1;
		payload_len = 0U;
		for (i = 0U; i < 8U; ++i)
			payload_len = (payload_len << 8) | client->hdr[index + i];
		if (payload_len <= 0xFFFFU)
			return -1;
		index += 8U;
	}

	control = (opcode & 0x08U) != 0U;
	if (control && (!fin || payload_len > 125U))
		return -1;
	if (opcode == 0x0U)
	{
		if (client->message_opcode == 0U)
			return -1;
	}
	else if (!control)
	{
		if (client->message_opcode != 0U)
			return -1;
		if (!fin)
			client->message_opcode = opcode;
	}

	client->fin = fin;
	client->frame_opcode = opcode;
	client->masked = 1U;
	client->mask[0] = client->hdr[index];
	client->mask[1] = client->hdr[index + 1U];
	client->mask[2] = client->hdr[index + 2U];
	client->mask[3] = client->hdr[index + 3U];
	client->mask_i = 0U;
	client->payload_len = payload_len;
	client->payload_rem = payload_len;
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
	effective_opcode = client->frame_opcode == 0U ? client->message_opcode
												  : client->frame_opcode;
	flags = effective_opcode == 0x1U ? WS_OPCODE_TEXT : WS_OPCODE_BIN;
	if (client->frame_opcode == 0U)
		flags |= WS_OPCODE_FRAGMENT;
	else if (!client->fin)
		flags |= WS_OPCODE_FRAGMENT_START;
	if (final_chunk && client->fin)
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
	queued = ws_queue_frame(ws, client_idx, 0x8U, payload, sizeof(payload), false, true);
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

	if (client->frame_opcode == 0x9U)
	{
		queued = ws_queue_frame(ws, client_idx, 0xAU,
								client->control_buf, client->control_len, false, false);
		if (queued < 0)
			(void)socket_close(ws->ws_socket, client_idx);
	}
	else if (client->frame_opcode == 0x8U)
	{
		uint16_t code = 1000U;
		if (client->control_len == 1U)
		{
			ws_protocol_error_close(ws, client_idx, client, 1002U);
			return;
		}
		if (client->control_len >= 2U)
			code = (uint16_t)(((uint16_t)client->control_buf[0] << 8) |
							  client->control_buf[1]);
		ws_notify_close(ws, client_idx, client, code);
		client->close_code = code;
		queued = ws_queue_frame(ws, client_idx, 0x8U,
								client->control_buf, client->control_len, false, true);
		if (queued < 0)
		{
			client->status = WS_S_CLOSING;
			(void)socket_close(ws->ws_socket, client_idx);
		}
	}
	client->control_len = 0U;
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

/* Writable is a continuation hint: never rebuild a frame in this callback. */
static void ws_on_writable(uint8_t client_idx, void *protocol)
{
	(void)ws_flush((websocket_protocol_t *)protocol, client_idx);
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
		if (client->req.status != REQ_START_FINISHED)
		{
			http_request_parse_start(&client->req, &cursor, &remaining);
			memset(&client->header, 0, sizeof(client->header));
		}
		if (client->req.status == REQ_START_FINISHED)
		{
			http_request_parse_header(&client->header, &cursor, &remaining);
			if (client->header.status == REQ_HEAD_FINISHED)
			{
				if (client->header.name[0] == '\0')
				{
					if (client->handshake.hs_got_upgrade &&
						client->handshake.hs_got_connection &&
						client->handshake.hs_got_key &&
						client->handshake.hs_got_version)
					{
						uint8_t digest[20];
						sha1_ctx sha1;
						char accept[64];
						char response[256];
						int length;

						sha1_init(&sha1);
						sha1_update(&sha1, (const uint8_t *)client->handshake.hs_key,
									strlen(client->handshake.hs_key));
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
							ws_queue_handshake(ws, client_idx, response,
											   (size_t)length, true, false) < 0)
							(void)socket_close(ws->ws_socket, client_idx);
					}
					else
					{
						static const char bad_request[] =
							"HTTP/1.1 400 Bad Request\r\n"
							"Connection: close\r\n"
							"Content-Length: 0\r\n\r\n";
						if (ws_queue_handshake(ws, client_idx, bad_request,
											   sizeof(bad_request) - 1U, false, true) < 0)
							(void)socket_close(ws->ws_socket, client_idx);
					}
					return;
				}
				http_request_ws_handshake(&client->handshake, &client->header);
				memset(&client->header, 0, sizeof(client->header));
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
		if (client->hdr_need == 0U || client->hdr_have < client->hdr_need)
		{
			if (client->hdr_have < 2U)
			{
				size_t needed = 2U - client->hdr_have;
				size_t take = remaining < needed ? remaining : needed;
				memcpy(&client->hdr[client->hdr_have], cursor, take);
				client->hdr_have += (uint8_t)take;
				cursor += take;
				remaining -= take;
				if (client->hdr_have < 2U)
					break;
			}
			if (client->hdr_need == 0U)
			{
				size_t needed = 2U;
				uint8_t short_len = (uint8_t)(client->hdr[1] & 0x7FU);
				if (short_len == 126U)
					needed += 2U;
				else if (short_len == 127U)
					needed += 8U;
				if ((client->hdr[1] & 0x80U) != 0U)
					needed += 4U;
				client->hdr_need = (uint8_t)needed;
			}
			if (client->hdr_have < client->hdr_need)
			{
				size_t needed = client->hdr_need - client->hdr_have;
				size_t take = remaining < needed ? remaining : needed;
				memcpy(&client->hdr[client->hdr_have], cursor, take);
				client->hdr_have += (uint8_t)take;
				cursor += take;
				remaining -= take;
				if (client->hdr_have < client->hdr_need)
					break;
			}
			if (ws_parse_header(client) < 0)
			{
				ws_protocol_error_close(ws, client_idx, client, 1002U);
				return;
			}
		}

		if (client->payload_rem != 0U)
		{
			size_t available = client->payload_rem < (uint64_t)remaining
								   ? (size_t)client->payload_rem
								   : remaining;
			while (available != 0U)
			{
				uint8_t unmasked[WS_STREAM_CHUNK];
				size_t chunk = available > WS_STREAM_CHUNK ? WS_STREAM_CHUNK : available;
				size_t i;
				for (i = 0U; i < chunk; ++i)
					unmasked[i] = (uint8_t)(cursor[i] ^ client->mask[(client->mask_i++) & 3U]);

				if ((client->frame_opcode & 0x08U) != 0U)
				{
					if (chunk > sizeof(client->control_buf) - client->control_len)
					{
						ws_protocol_error_close(ws, client_idx, client, 1002U);
						return;
					}
					memcpy(&client->control_buf[client->control_len], unmasked, chunk);
					client->control_len += (uint8_t)chunk;
				}
				else
				{
					bool final_chunk = (uint64_t)chunk == client->payload_rem;
					ws_dispatch_payload(ws, client, client_idx, unmasked, chunk, final_chunk);
				}
				cursor += chunk;
				remaining -= chunk;
				available -= chunk;
				client->payload_rem -= chunk;
			}
		}

		if (client->payload_rem == 0U)
		{
			if ((client->frame_opcode & 0x08U) != 0U)
				ws_handle_control_complete(ws, client, client_idx);
			else
			{
				if (client->payload_len == 0U)
					ws_dispatch_payload(ws, client, client_idx, NULL, 0U, true);
				if (client->fin && client->message_opcode != 0U)
					client->message_opcode = 0U;
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
	bool application_frame;
	bool close_frame = false;
	uint8_t default_close[2] = {0x03U, 0xE8U};
	uint16_t close_code = 0U;
	int queued;
	if (!ws || client_idx >= SOCKET_MAX_CLIENTS)
		return SOCKET_DEVICE_INVALID;

	if (type == WS_SEND_CLOSE)
	{
		opcode = 0x8U;
		application_frame = false;
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
		application_frame = false;
	}
	else
	{
		if (len > WEBSOCKET_MAX_CHUNK)
			return SOCKET_DEVICE_INVALID;
		opcode = type == WS_SEND_BIN ? 0x2U : 0x1U;
		application_frame = true;
	}
	queued = ws_queue_frame(ws, client_idx, opcode, data, len,
							application_frame, close_frame);
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
	socket_add_onwritable_handler(socket, ws_on_writable);
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
