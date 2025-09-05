/*
	Name: websocket.c
	Description: Implements a simple Websocket Server based on BSD/POSIX Sockets for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 03-09-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "websocket.h"

#ifdef ENABLE_SOCKETS
#include "utils/sha1.h"
#include "utils/base64.h"

#ifndef WEBSOCKET_PORT
#define WEBSOCKET_PORT 8080
#endif

// Magic GUID for Sec-WebSocket-Accept
static const char WS_GUID[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

// Max size of streaming unmask buffer per step
#ifndef WS_STREAM_CHUNK
#define WS_STREAM_CHUNK 256
#endif

#ifndef WEBSOCKET_MAX_CLIENTS
#define WEBSOCKET_MAX_CLIENTS SOCKET_MAX_CLIENTS
#endif

#ifndef WEBSOCKET_MAX_IDLE_TIMEOUT
#define WEBSOCKET_MAX_IDLE_TIMEOUT 10000
#endif

static void ws_on_connected(uint8_t client_idx, void *protocol);
static void ws_on_disconnected(uint8_t client_idx, void *protocol);
static void ws_on_data(uint8_t client_idx, char *data, size_t data_len, void *protocol);
static void ws_frame_reset(ws_client_state_t *c);
static int ws_parse_header(ws_client_state_t *c);
static void ws_dispatch_payload(websocket_protocol_t *ws, ws_client_state_t *c, uint8_t client_idx, const uint8_t *buf, size_t n, bool final_for_chunk);
static int ws_send_frame(websocket_protocol_t *ws, uint8_t client_idx, uint8_t opcode, const uint8_t *payload, size_t len, bool broadcast);
static void ws_protocol_error_close(websocket_protocol_t *ws, uint8_t client_idx, ws_client_state_t *c, uint16_t code);

static void ws_reset_client(ws_client_state_t *c)
{
	memset(c, 0, sizeof(*c));
}

static void ws_frame_reset(ws_client_state_t *c)
{
	c->hdr_have = 0;
	c->hdr_need = 0;
	c->fin = 0;
	// Do not clear c->opcode here; it is cleared when a message FIN is seen
	c->masked = 0;
	c->mask_i = 0;
	c->payload_len = 0;
	c->payload_rem = 0;
	c->control_len = 0;
	memset(c->hdr, 0, sizeof(c->hdr));
}

// Parse frame header into client state; returns 0 when ready, 1 when need more, -1 on protocol error
static int ws_parse_header(ws_client_state_t *c)
{
	if (c->hdr_have < 2)
		return 1;

	const uint8_t b0 = c->hdr[0];
	const uint8_t b1 = c->hdr[1];

	const uint8_t fin = (uint8_t)((b0 >> 7) & 1u);
	const uint8_t rsv = (uint8_t)((b0 >> 4) & 0x7u);
	const uint8_t opcode = (uint8_t)(b0 & 0x0Fu);
	const uint8_t masked = (uint8_t)((b1 >> 7) & 1u);
	uint64_t payload_len = (uint8_t)(b1 & 0x7Fu);

	// RSV must be 0 unless extensions negotiated
	if (rsv != 0)
		return -1;

	size_t need = 2;
	if (payload_len == 126)
		need += 2;
	else if (payload_len == 127)
		need += 8;
	if (masked)
		need += 4;

	c->hdr_need = (uint8_t)need;
	if (c->hdr_have < c->hdr_need)
		return 1;

	size_t idx = 2;
	if (payload_len == 126)
	{
		payload_len = ((uint64_t)c->hdr[idx] << 8) | (uint64_t)c->hdr[idx + 1];
		idx += 2;
	}
	else if (payload_len == 127)
	{
		payload_len =
				((uint64_t)c->hdr[idx + 0] << 56) |
				((uint64_t)c->hdr[idx + 1] << 48) |
				((uint64_t)c->hdr[idx + 2] << 40) |
				((uint64_t)c->hdr[idx + 3] << 32) |
				((uint64_t)c->hdr[idx + 4] << 24) |
				((uint64_t)c->hdr[idx + 5] << 16) |
				((uint64_t)c->hdr[idx + 6] << 8) |
				((uint64_t)c->hdr[idx + 7]);
		idx += 8;
	}

	uint8_t mask[4] = {0, 0, 0, 0};
	if (masked)
	{
		mask[0] = c->hdr[idx + 0];
		mask[1] = c->hdr[idx + 1];
		mask[2] = c->hdr[idx + 2];
		mask[3] = c->hdr[idx + 3];
		idx += 4;
	}

	// Client->server frames must be masked
	if (!masked)
		return -1;

	// Control frames: FIN must be 1 and payload <= 125
	const bool is_control = ((opcode & 0x08) != 0);
	if (is_control)
	{
		if (!fin || payload_len > 125)
			return -1;
	}

	c->fin = fin;
	if (opcode != 0x0)
	{
		// New message starts (non-continuation)
		// Valid if no previous message is in progress
		// (We use c->opcode == 0 to indicate no message in progress)
		if (!is_control && c->opcode != 0)
		{
			// A new data message while a previous message is fragmented is invalid
			return -1;
		}
		c->opcode = opcode;
	}
	else
	{
		// Continuation: must have a data message in progress
		if (!is_control && c->opcode == 0)
		{
			return -1;
		}
	}

	c->masked = masked;
	c->mask[0] = mask[0];
	c->mask[1] = mask[1];
	c->mask[2] = mask[2];
	c->mask[3] = mask[3];
	c->mask_i = 0;
	c->payload_len = payload_len;
	c->payload_rem = payload_len;

	return 0;
}

static void ws_dispatch_payload(websocket_protocol_t *ws, ws_client_state_t *c, uint8_t client_idx, const uint8_t *buf, size_t n, bool final_for_chunk)
{
	uint8_t eff_opcode = c->opcode;
	uint8_t flags = 0;
	switch (eff_opcode)
	{
	case 0x1:
		flags |= WS_OPCODE_TEXT;
		break;
	case 0x2:
		flags |= WS_OPCODE_BIN;
		break;
	default:
		break;
	}
	if (final_for_chunk)
		flags |= WS_OPCODE_FRAGMENT_FIN;

	if (ws && ws->ws_onrecv_cb && n)
	{
		ws->ws_onrecv_cb(client_idx, (void *)buf, n, flags);
	}
}

// Encode and send a single frame (server->client frames are not masked)
static int ws_send_frame(websocket_protocol_t *ws, uint8_t client_idx, uint8_t opcode, const uint8_t *payload, size_t len, bool broadcast)
{
	if ((opcode & 0x08) && len > 125)
		return -1; // control frames <=125
	uint8_t hdr[14];
	size_t hlen = 0;

	hdr[0] = (uint8_t)(0x80u | (opcode & 0x0Fu)); // FIN=1
	if (len <= 125)
	{
		hdr[1] = (uint8_t)len; // MASK=0
		hlen = 2;
	}
	else if (len <= 0xFFFFu)
	{
		hdr[1] = 126;
		hdr[2] = (uint8_t)((len >> 8) & 0xFF);
		hdr[3] = (uint8_t)(len & 0xFF);
		hlen = 4;
	}
	else
	{
		uint64_t L = (uint64_t)len;
		hdr[1] = 127;
		hdr[2] = (uint8_t)((L >> 56) & 0xFF);
		hdr[3] = (uint8_t)((L >> 48) & 0xFF);
		hdr[4] = (uint8_t)((L >> 40) & 0xFF);
		hdr[5] = (uint8_t)((L >> 32) & 0xFF);
		hdr[6] = (uint8_t)((L >> 24) & 0xFF);
		hdr[7] = (uint8_t)((L >> 16) & 0xFF);
		hdr[8] = (uint8_t)((L >> 8) & 0xFF);
		hdr[9] = (uint8_t)(L & 0xFF);
		hlen = 10;
	}

	if (broadcast)
	{
		socket_broadcast(ws->ws_socket, (char *)hdr, hlen, 0);
		if (len)
			socket_broadcast(ws->ws_socket, (char *)payload, len, 0);
	}
	else
	{
		socket_send(ws->ws_socket, client_idx, (char *)hdr, hlen, 0);
		if (len)
			socket_send(ws->ws_socket, client_idx, (char *)payload, len, 0);
	}
	return (int)(hlen + len);
}

static void ws_protocol_error_close(websocket_protocol_t *ws, uint8_t client_idx, ws_client_state_t *c, uint16_t code)
{
	uint8_t payload[2];
	payload[0] = (uint8_t)((code >> 8) & 0xFF);
	payload[1] = (uint8_t)(code & 0xFF);
	ws_send_frame(ws, client_idx, 0x8, payload, 2, false);
	c->status = WS_S_CLOSING;
}

static void ws_on_connected(uint8_t client_idx, void *protocol)
{
	websocket_protocol_t *ws = (websocket_protocol_t *)protocol;
	ws_client_state_t *c = &ws->ws_clients[client_idx];
	ws_reset_client(c);
	c->active = 1;
	c->status = WS_S_HANDSHAKE;
}

static void ws_on_disconnected(uint8_t client_idx, void *protocol)
{
	websocket_protocol_t *ws = (websocket_protocol_t *)protocol;
	ws_client_state_t *c = &ws->ws_clients[client_idx];
	if (!c->active)
		return;

	if (c->status == WS_S_OPEN || c->status == WS_S_CLOSING)
	{
		if (ws && ws->ws_onclose_cb)
			ws->ws_onclose_cb(client_idx, (uint16_t)1001);
	}
	ws_reset_client(c);
}

static void ws_handle_control_complete(websocket_protocol_t *ws, ws_client_state_t *c, uint8_t client_idx, uint8_t cur_opcode)
{
	if (cur_opcode == 0x9)
	{
		// PING -> echo PONG with same payload
		ws_send_frame(ws, client_idx, 0xA, c->control_buf, c->control_len, false);
	}
	else if (cur_opcode == 0xA)
	{
		// PONG -> ignore or notify upper layer if needed
	}
	else if (cur_opcode == 0x8)
	{
		// CLOSE -> mirror payload, notify, and enter closing
		uint16_t code = 1000;
		if (c->control_len >= 2)
		{
			code = (uint16_t)((c->control_buf[0] << 8) | c->control_buf[1]);
		}
		if (ws && ws->ws_onclose_cb)
			ws->ws_onclose_cb(client_idx, code);
		ws_send_frame(ws, client_idx, 0x8, c->control_buf, c->control_len, false);
		c->status = WS_S_CLOSING;
	}
	c->control_len = 0;
}

static void ws_finish_message_if_needed(ws_client_state_t *c)
{
	if (c->fin)
	{
		// End-of-message for data frames
		if ((c->opcode & 0x08) == 0)
		{
			// Clear message opcode only for data messages (not control)
			c->opcode = 0;
		}
	}
}

// on idle checks for any connection waiting to be closed
static void ws_on_idle(uint8_t client_idx, uint32_t idle_ms, void *protocol)
{
	websocket_protocol_t *ws = (websocket_protocol_t *)protocol;
	if (ws->ws_clients[client_idx].status == WS_S_CLOSING)
	{
		socket_free(ws->ws_socket, client_idx);
		ws_reset_client(&(ws->ws_clients[client_idx]));
	}
}

static void ws_on_data(uint8_t client_idx, char *data, size_t data_len, void *protocol)
{
	websocket_protocol_t *ws = (websocket_protocol_t *)protocol;
	ws_client_state_t *c = &ws->ws_clients[client_idx];
	if (!c->active)
		return;

	uint8_t *p = (uint8_t *)data;

	if (c->status == WS_S_HANDSHAKE)
	{
		for (;;)
		{
			if (c->req.status != REQ_START_FINISHED)
			{
				http_request_parse_start(&c->req, (char **)&data, &data_len);
				memset(&c->header, 0, sizeof(c->header));
			}
			if (c->req.status == REQ_START_FINISHED)
			{
				http_request_parse_header(&c->header, (char **)&data, &data_len);
				if (c->header.status == REQ_HEAD_FINISHED)
				{
					if (c->header.name[0] == 0)
					{
						// headers complete, craft handshake response
						if (c->handshake.hs_got_upgrade && c->handshake.hs_got_connection &&
								c->handshake.hs_got_key && c->handshake.hs_got_version)
						{
							// Compute accept and reply
							uint8_t digest[20];
							sha1_ctx ctx;
							sha1_init(&ctx);
							sha1_update(&ctx, (const uint8_t *)c->handshake.hs_key, strlen(c->handshake.hs_key));
							sha1_update(&ctx, (const uint8_t *)WS_GUID, strlen(WS_GUID));
							sha1_final(&ctx, digest);
							char accept_b64[64];
							base64_encode(digest, 20, accept_b64, sizeof(accept_b64));
							char resp[256];
							int n = str_snprintf(resp, sizeof(resp),
																	 "HTTP/1.1 101 Switching Protocols\r\n"
																	 "Upgrade: websocket\r\n"
																	 "Connection: Upgrade\r\n"
																	 "Sec-WebSocket-Accept: %s\r\n"
																	 "\r\n",
																	 accept_b64);
							socket_send(ws->ws_socket, client_idx, resp, (size_t)n, 0);
							c->status = WS_S_OPEN;
							if (ws && ws->ws_onopen_cb)
								ws->ws_onopen_cb(client_idx);
						}
						else
						{
							static const char bad[] =
									"HTTP/1.1 400 Bad Request\r\n"
									"Connection: close\r\n"
									"Content-Length: 0\r\n"
									"\r\n";
							socket_send(ws->ws_socket, client_idx, bad, sizeof(bad) - 1, 0);
							c->status = WS_S_CLOSING;
						}
						return;
					}
					http_request_ws_handshake(&c->handshake, &c->header);
					memset(&c->header, 0, sizeof(c->header));
				}
			}
			if (!data_len)
				return;
		}
	}

	if (c->status == WS_S_CLOSING)
	{
		// Ignore further data; waiting for TCP close
		return;
	}

	size_t nleft = data_len;

	// Phase: OPEN — streaming frame parsing
	while (nleft > 0)
	{
		// 1) Accumulate header (14 bytes max) incrementally
		if (c->hdr_need == 0 || c->hdr_have < c->hdr_need)
		{
			// First ensure the first 2 bytes
			if (c->hdr_have < 2)
			{
				size_t need2 = 2 - c->hdr_have;
				size_t take = (nleft < need2) ? nleft : need2;
				memcpy(c->hdr + c->hdr_have, p, take);
				c->hdr_have += (uint8_t)take;
				p += take;
				nleft -= take;
				if (c->hdr_have < 2)
					break; // need more
			}

			// Compute full header length once first 2 bytes are available
			if (c->hdr_need == 0)
			{
				uint8_t b1 = c->hdr[1] & 0x7Fu;
				size_t need = 2;
				if (b1 == 126)
					need += 2;
				else if (b1 == 127)
					need += 8;
				if (c->hdr[1] & 0x80u)
					need += 4; // mask
				c->hdr_need = (uint8_t)need;
			}

			// Pull the rest of the header if available
			if (c->hdr_have < c->hdr_need)
			{
				size_t need_more = c->hdr_need - c->hdr_have;
				size_t take = (nleft < need_more) ? nleft : need_more;
				memcpy(c->hdr + c->hdr_have, p, take);
				c->hdr_have += (uint8_t)take;
				p += take;
				nleft -= take;
				if (c->hdr_have < c->hdr_need)
					break; // need more
			}

			// Parse header now that we have it all
			int pr = ws_parse_header(c);
			if (pr < 0)
			{
				ws_protocol_error_close(ws, client_idx, c, 1002);
				return;
			}
		}

		// 2) Payload: stream in <= WS_STREAM_CHUNK pieces, unmasking on the fly
		uint8_t cur_frame_opcode = (uint8_t)(c->hdr[0] & 0x0Fu);
		bool is_control = ((cur_frame_opcode & 0x08) != 0);

		// Take up to payload_rem and what remains in this TCP chunk
		size_t take_total = (nleft < c->payload_rem) ? nleft : (size_t)c->payload_rem;

		while (take_total > 0)
		{
			size_t chunk = (take_total > WS_STREAM_CHUNK) ? WS_STREAM_CHUNK : take_total;
			uint8_t tmp[WS_STREAM_CHUNK];

			// Unmask into tmp
			for (size_t i = 0; i < chunk; ++i)
			{
				uint8_t v = p[i];
				if (c->masked)
					v ^= c->mask[(c->mask_i++) & 3u];
				tmp[i] = v;
			}

			if (is_control)
			{
				// Buffer control bytes until full payload (<=125 total)
				size_t room = sizeof(c->control_buf) - c->control_len;
				size_t copy = (chunk < room) ? chunk : room;
				if (copy != chunk)
				{
					// control payload overflow -> protocol error
					ws_protocol_error_close(ws, client_idx, c, 1002);
					return;
				}
				memcpy(c->control_buf + c->control_len, tmp, chunk);
				c->control_len += (uint8_t)chunk;
			}
			else
			{
				// Data frames: stream out immediately
				bool final_for_chunk = (chunk == c->payload_rem) && (c->fin != 0);
				ws_dispatch_payload(ws, c, client_idx, tmp, chunk, final_for_chunk);
			}

			p += chunk;
			nleft -= chunk;
			take_total -= chunk;
			c->payload_rem -= chunk;
		}

		// 3) If frame payload complete, act on it
		if (c->payload_rem == 0)
		{
			if (is_control)
			{
				ws_handle_control_complete(ws, c, client_idx, cur_frame_opcode);
				if (c->status == WS_S_CLOSING)
				{
					// After responding to CLOSE, we’re closing
					ws_frame_reset(c);
					return;
				}
			}
			else
			{
				ws_finish_message_if_needed(c);
			}
			ws_frame_reset(c);
		}
	}
}

int websocket_send(websocket_protocol_t *ws, uint8_t client_idx, char *data, size_t len, uint8_t send_code)
{
	bool broadcast = (send_code & WS_SEND_BROADCAST) != 0;
	uint8_t type = send_code & WS_SEND_TYPE;
	if (type == 0)
		return -1;

	uint8_t opcode = 0x1; // default text
	const uint8_t *payload = (const uint8_t *)data;
	size_t payload_len = len;

	if (send_code & WS_SEND_CLOSE)
	{
		opcode = 0x8;
		uint8_t closebuf[2 + 123];
		size_t clen = 0;
		if (len >= 2)
		{
			size_t reason_len = len - 2;
			if (reason_len > 123)
				reason_len = 123;
			memcpy(closebuf, data, 2 + reason_len);
			payload = closebuf;
			payload_len = 2 + reason_len;
			clen = payload_len;
		}
		else
		{
			// default 1000
			closebuf[0] = 0x03;
			closebuf[1] = 0xE8;
			payload = closebuf;
			payload_len = 2;
			clen = 2;
		}
		int sent = ws_send_frame(ws, client_idx, opcode, payload, clen, broadcast);

		// Mark closing locally
		if (broadcast)
		{
			for (int i = 0; i < (int)WEBSOCKET_MAX_CLIENTS; ++i)
			{
				if (ws->ws_clients[i].active && ws->ws_clients[i].status == WS_S_OPEN)
				{
					ws->ws_clients[i].status = WS_S_CLOSING;
				}
			}
		}
		else if (ws->ws_socket->socket_clients[client_idx] >= 0)
		{
			ws->ws_clients[client_idx].status = WS_S_CLOSING;
		}
		return sent;
	}

	if (send_code & WS_SEND_PING)
	{
		opcode = 0x9;
		if (payload_len > 125)
			payload_len = 125;
		return ws_send_frame(ws, client_idx, opcode, payload, payload_len, broadcast);
	}

	if (send_code & WS_SEND_PONG)
	{
		opcode = 0xA;
		if (payload_len > 125)
			payload_len = 125;
		return ws_send_frame(ws, client_idx, opcode, payload, payload_len, broadcast);
	}

	if (send_code & WS_SEND_BIN)
		opcode = 0x2;
	else
		opcode = 0x1;
	return ws_send_frame(ws, client_idx, opcode, payload, payload_len, broadcast);
}

socket_if_t *websocket_start_listen(websocket_protocol_t *ws, int port)
{
	LOAD_MODULE(socket_server);
	socket_if_t *socket = socket_start_listen(IP_ANY, port, 2 /*AF_INET*/, 1 /*SOCK_STREAM*/, 0);

	for (int i = 0; i < (int)WEBSOCKET_MAX_CLIENTS; ++i)
	{
		ws_reset_client(&(ws->ws_clients[i]));
	}
	socket_add_ondata_handler(socket, ws_on_data);
	socket_add_onidle_handler(socket, ws_on_idle);
	socket_add_onconnected_handler(socket, ws_on_connected);
	socket_add_ondisconnected_handler(socket, ws_on_disconnected);

	socket->protocol = ws;
	ws->ws_socket = socket;
	return socket;
}

#endif
