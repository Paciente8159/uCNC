// #include <stdint.h>
// #include <stddef.h>
// #include <string.h>
// #include "websocket.h"
// #include "utils/base64.h"
// #include "utils/sha1.h"

// /*  Internal state  */

// #ifndef INVALID_SOCKET
// #define INVALID_SOCKET (-1)
// #endif

// #define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

// typedef enum
// {
// 	WS_S_HANDSHAKE = 0,
// 	WS_S_OPEN = 1,
// 	WS_S_CLOSING = 2
// } ws_phase_t;

// typedef struct
// {
// 	int active;
// 	ws_phase_t phase;

// 	// Handshake accumulation
// 	char hs_buf[WEBSOCKET_MAX_HEADER_BUF];
// 	size_t hs_len;

// 	// Frame parsing
// 	uint8_t hdr[14];	/* At most 14 bytes header */
// 	uint8_t hdr_have; /* bytes filled in hdr[] */
// 	uint8_t hdr_need; /* total header length once known */
// 	uint8_t fin;
// 	uint8_t opcode;
// 	uint8_t masked;
// 	uint8_t mask[4];
// 	uint8_t mask_i;
// 	uint64_t payload_len;
// 	uint64_t payload_rem;
// } ws_client_state_t;

// // Server instance
// static socket_if_t *ws_srv = NULL;
// static ws_client_state_t ws_clients[WEBSOCKET_MAX_CLIENTS];

// // User callbacks
// static ws_on_text_cb cb_text = NULL;
// static ws_on_binary_cb cb_binary = NULL;
// static ws_on_open_cb cb_open = NULL;
// static ws_on_close_cb cb_close = NULL;

// // Utilities

// static int find_slot_by_fd(int fd)
// {
// 	if (!ws_srv)
// 		return -1;
// 	for (int i = 0; i < SOCKET_MAX_CLIENTS; i++)
// 	{
// 		if (ws_srv->socket_clients[i] == fd)
// 			return i;
// 	}
// 	return -1;
// }

// // Case-insensitive starts-with
// static int istarts_with(const char *s, const char *pfx)
// {
// 	while (*pfx && *s)
// 	{
// 		char a = *s, b = *pfx;
// 		if (a >= 'A' && a <= 'Z')
// 			a += 32;
// 		if (b >= 'A' && b <= 'Z')
// 			b += 32;
// 		if (a != b)
// 			return 0;
// 		s++;
// 		pfx++;
// 	}
// 	return *pfx == '\0';
// }

// // Find CRLFCRLF in buffer, return index of the byte just after the sequence, or -1
// static int find_hdr_end(const char *buf, size_t len)
// {
// 	if (len < 4)
// 		return -1;
// 	for (size_t i = 3; i < len; i++)
// 	{
// 		if (buf[i - 3] == '\r' && buf[i - 2] == '\n' && buf[i - 1] == '\r' && buf[i] == '\n')
// 		{
// 			return (int)(i + 1);
// 		}
// 	}
// 	return -1;
// }

// // Handshake

// static void ws_reset_client(ws_client_state_t *st)
// {
// 	memset(st, 0, sizeof(*st));
// 	st->active = 1;
// 	st->phase = WS_S_HANDSHAKE;
// }

// static int ws_send_upgrade_response(int client_fd, const char *sec_key, size_t sec_key_len)
// {
// 	/* SHA1( key + GUID ), then Base64 */
// 	sha1_ctx ctx;
// 	uint8_t digest[20];
// 	char accept_key[32]; /* 28 chars + safety */
// 	size_t accept_len;

// 	sha1_init(&ctx);
// 	sha1_update(&ctx, sec_key, sec_key_len);
// 	sha1_update(&ctx, WS_GUID, strlen(WS_GUID));
// 	sha1_final(&ctx, digest);

// 	accept_len = base64_encode(digest, 20, accept_key, 32);

// 	static const char resp1[] =
// 			"HTTP/1.1 101 Switching Protocols\r\n"
// 			"Upgrade: websocket\r\n"
// 			"Connection: Upgrade\r\n"
// 			"Sec-WebSocket-Accept: ";
// 	static const char resp2[] = "\r\n\r\n";

// 	int r = 0;
// 	if (socket_send(ws_srv, client_fd, (char *)resp1, sizeof(resp1) - 1, 0) < 0)
// 		r = -1;
// 	if (r == 0 && socket_send(ws_srv, client_fd, accept_key, accept_len, 0) < 0)
// 		r = -1;
// 	if (r == 0 && socket_send(ws_srv, client_fd, (char *)resp2, sizeof(resp2) - 1, 0) < 0)
// 		r = -1;
// 	return r;
// }

// static int ws_do_handshake(int slot)
// {
// 	ws_client_state_t *st = &ws_clients[slot];
// 	/* Must contain CRLFCRLF by the time we call this */
// 	int end = find_hdr_end(st->hs_buf, st->hs_len);
// 	if (end < 0)
// 		return 0;

// 	/* Very small, forgiving header parse */
// 	const char *h = st->hs_buf;
// 	size_t hlen = (size_t)end;

// 	/* Check "GET" line minimality */
// 	if (!(hlen >= 4 && h[0] == 'G' && h[1] == 'E' && h[2] == 'T' && h[3] == ' '))
// 	{
// 		return -1;
// 	}

// 	/* Find Sec-WebSocket-Key header (case-insensitive) */
// 	const char *p = h;
// 	const char *key_start = NULL;
// 	size_t key_len = 0;

// 	while (p < h + hlen)
// 	{
// 		/* Find line end */
// 		const char *line_end = p;
// 		while (line_end < h + hlen && !(line_end[0] == '\r' && (line_end + 1) < h + hlen && line_end[1] == '\n'))
// 		{
// 			line_end++;
// 		}
// 		size_t line_len = (size_t)(line_end - p);
// 		if (line_len == 0)
// 			break; /* end of headers */

// 		/* Match header name */
// 		if (line_len > 18 && istarts_with(p, "Sec-WebSocket-Key:"))
// 		{
// 			const char *v = p + 18;
// 			/* Skip spaces */
// 			while (v < line_end && (*v == ' ' || *v == '\t'))
// 				v++;
// 			key_start = v;
// 			/* Trim trailing spaces */
// 			const char *ve = line_end;
// 			while (ve > v && (ve[-1] == ' ' || ve[-1] == '\t'))
// 				ve--;
// 			key_len = (size_t)(ve - v);
// 		}

// 		/* Next line */
// 		p = line_end + 2;
// 	}

// 	if (!key_start || key_len == 0)
// 	{
// 		return -1;
// 	}

// 	int client_fd = ws_srv->socket_clients[slot];
// 	if (ws_send_upgrade_response(client_fd, key_start, key_len) < 0)
// 	{
// 		return -1;
// 	}

// 	st->phase = WS_S_OPEN;
// 	st->hs_len = 0; /* free the buffer */
// 	if (cb_open)
// 		cb_open(client_fd);
// 	return 1;
// }

// /*  Frame I/O  */

// static int ws_send_frame_header(int client_fd, uint8_t opcode, size_t len)
// {
// 	/* Server-to-client: FIN=1, no mask */
// 	uint8_t hdr[10];
// 	size_t hlen = 0;

// 	hdr[hlen++] = 0x80 | (opcode & 0x0F);
// 	if (len <= 125)
// 	{
// 		hdr[hlen++] = (uint8_t)(len & 0x7F);
// 	}
// 	else if (len <= 0xFFFF)
// 	{
// 		hdr[hlen++] = 126;
// 		uint16_t n = (uint16_t)len;
// 		hdr[hlen++] = (uint8_t)((n >> 8) & 0xFF);
// 		hdr[hlen++] = (uint8_t)(n & 0xFF);
// 	}
// 	else
// 	{
// 		hdr[hlen++] = 127;
// 		uint64_t v = (uint64_t)len;
// 		/* network order 8 bytes */
// 		for (int i = 7; i >= 0; i--)
// 		{
// 			hdr[hlen++] = (uint8_t)((v >> (i * 8)) & 0xFF);
// 		}
// 	}

// 	return socket_send(ws_srv, client_fd, (char *)hdr, hlen, 0);
// }

// int websocket_send_text(int client_fd, const char *data, size_t len)
// {
// 	if (!ws_srv)
// 		return -1;
// 	if (ws_send_frame_header(client_fd, WS_OPCODE_TEXT, len) < 0)
// 		return -1;
// 	return socket_send(ws_srv, client_fd, (char *)data, len, 0);
// }

// int websocket_send_binary(int client_fd, const uint8_t *data, size_t len)
// {
// 	if (!ws_srv)
// 		return -1;
// 	if (ws_send_frame_header(client_fd, WS_OPCODE_BINARY, len) < 0)
// 		return -1;
// 	return socket_send(ws_srv, client_fd, (char *)data, len, 0);
// }

// int websocket_send_pong(int client_fd, const uint8_t *data, size_t len)
// {
// 	if (!ws_srv)
// 		return -1;
// 	if (ws_send_frame_header(client_fd, WS_OPCODE_PONG, len) < 0)
// 		return -1;
// 	return socket_send(ws_srv, client_fd, (char *)data, len, 0);
// }

// int websocket_send_close(int client_fd, uint16_t code)
// {
// 	if (!ws_srv)
// 		return -1;
// 	uint8_t payload[2];
// 	payload[0] = (uint8_t)((code >> 8) & 0xFF);
// 	payload[1] = (uint8_t)(code & 0xFF);
// 	if (ws_send_frame_header(client_fd, WS_OPCODE_CLOSE, sizeof(payload)) < 0)
// 		return -1;
// 	return socket_send(ws_srv, client_fd, (char *)payload, sizeof(payload), 0);
// }

// /* Parse frame header incrementally */
// static int ws_parse_header(ws_client_state_t *st)
// {
// 	if (st->hdr_have < 2)
// 		return 0;

// 	uint8_t b0 = st->hdr[0];
// 	uint8_t b1 = st->hdr[1];
// 	st->fin = (b0 >> 7) & 1u;
// 	st->opcode = (uint8_t)(b0 & 0x0F);
// 	st->masked = (uint8_t)((b1 >> 7) & 1u);

// 	size_t need = 2;
// 	uint8_t len7 = (uint8_t)(b1 & 0x7F);
// 	if (len7 == 126)
// 		need += 2;
// 	else if (len7 == 127)
// 		need += 8;
// 	if (st->masked)
// 		need += 4;

// 	st->hdr_need = (uint8_t)need;
// 	if (st->hdr_have < st->hdr_need)
// 		return 0;

// 	/* Length */
// 	size_t o = 2;
// 	uint64_t plen = 0;
// 	if (len7 < 126)
// 	{
// 		plen = len7;
// 	}
// 	else if (len7 == 126)
// 	{
// 		plen = ((uint64_t)st->hdr[o] << 8) | (uint64_t)st->hdr[o + 1];
// 		o += 2;
// 	}
// 	else
// 	{
// 		/* 64-bit length */
// 		for (int i = 0; i < 8; i++)
// 		{
// 			plen = (plen << 8) | st->hdr[o + i];
// 		}
// 		o += 8;
// 	}

// 	if (!st->masked)
// 	{
// 		/* Per RFC, client-to-server frames MUST be masked. */
// 		st->payload_len = 0;
// 		st->payload_rem = 0;
// 		return -2; /* protocol error */
// 	}

// 	/* Mask key */
// 	st->mask[0] = st->hdr[o + 0];
// 	st->mask[1] = st->hdr[o + 1];
// 	st->mask[2] = st->hdr[o + 2];
// 	st->mask[3] = st->hdr[o + 3];
// 	st->mask_i = 0;

// 	st->payload_len = plen;
// 	st->payload_rem = plen;

// 	return 1; /* header complete */
// }

// /* Process payload chunk in-place (unmask), deliver to callbacks immediately */
// static void ws_deliver_payload(int client_fd, ws_client_state_t *st, uint8_t *p, size_t n)
// {
// 	/* Unmask in-place */
// 	for (size_t i = 0; i < n; i++)
// 	{
// 		p[i] ^= st->mask[st->mask_i & 3];
// 		st->mask_i++;
// 	}

// 	int fin = (st->payload_rem == n) ? (st->fin ? 1 : 0) : 0;

// 	if (st->opcode == WS_OPCODE_TEXT || st->opcode == WS_OPCODE_CONT)
// 	{
// 		if (cb_text)
// 			cb_text(client_fd, (const char *)p, n, fin);
// 		else
// 		{
// 			/* Default behavior: echo text frames */
// 			websocket_send_text(client_fd, (const char *)p, n);
// 		}
// 	}
// 	else if (st->opcode == WS_OPCODE_BINARY)
// 	{
// 		if (cb_binary)
// 			cb_binary(client_fd, (const uint8_t *)p, n, fin);
// 	}
// }

// /* Main per-chunk processor (after handshake) */
// static void ws_process_data_open(int client_fd, int slot, uint8_t *data, size_t len)
// {
// 	ws_client_state_t *st = &ws_clients[slot];
// 	size_t i = 0;

// 	while (i < len)
// 	{
// 		/* Need header? */
// 		if (st->payload_rem == 0)
// 		{
// 			/* Fill header */
// 			while (st->hdr_have < 2 && i < len)
// 			{
// 				st->hdr[st->hdr_have++] = data[i++];
// 			}
// 			if (st->hdr_have >= 2)
// 			{
// 				int ph = ws_parse_header(st);
// 				if (ph < 0)
// 				{
// 					/* Protocol error -> try to send close, mark closing */
// 					websocket_send_close(client_fd, 1002); /* Protocol error */
// 					st->phase = WS_S_CLOSING;
// 					return;
// 				}
// 				/* If not enough header bytes yet, continue filling */
// 				while (st->hdr_have < st->hdr_need && i < len)
// 				{
// 					st->hdr[st->hdr_have++] = data[i++];
// 				}
// 				if (st->hdr_have >= st->hdr_need)
// 				{
// 					/* Header complete */
// 					if (st->opcode == WS_OPCODE_PING)
// 					{
// 						/* Echo pong with same payload (we will read payload next loop) */
// 						/* Fall through to read payload, then send pong */
// 					}
// 					else if (st->opcode == WS_OPCODE_PONG)
// 					{
// 						/* Ignore payload */
// 					}
// 					else if (st->opcode == WS_OPCODE_CLOSE)
// 					{
// 						/* Read optional payload (2-byte code), then respond close */
// 						/* We'll consume payload and then send close */
// 					}
// 				}
// 				else
// 				{
// 					/* Wait for more data */
// 					return;
// 				}
// 			}
// 			else
// 			{
// 				/* Need more data for header */
// 				return;
// 			}
// 		}

// 		/* Consume payload */
// 		if (st->payload_rem > 0)
// 		{
// 			size_t take = (size_t)((len - i) < st->payload_rem ? (len - i) : st->payload_rem);

// 			if (st->opcode == WS_OPCODE_PING)
// 			{
// 				/* Unmask into a small temp if needed? We can unmask in-place then reply. */
// 				ws_deliver_payload(client_fd, st, &data[i], take); /* delivers to default or user (won't for ping) */
// 				/* For Ping, the “deliver” above is a no-op for user; now respond */
// 				websocket_send_pong(client_fd, &data[i], take);
// 			}
// 			else if (st->opcode == WS_OPCODE_CLOSE)
// 			{
// 				/* Optionally parse status code if present in first 2 bytes */
// 				uint16_t code = 1000;
// 				if (st->payload_len >= 2 && st->payload_rem == st->payload_len && take >= 2)
// 				{
// 					uint8_t tmp[2];
// 					/* Unmask first two bytes only to get code */
// 					tmp[0] = (uint8_t)(data[i] ^ st->mask[st->mask_i & 3]);
// 					st->mask_i++;
// 					tmp[1] = (uint8_t)(data[i + 1] ^ st->mask[st->mask_i & 3]);
// 					st->mask_i++;
// 					code = ((uint16_t)tmp[0] << 8) | (uint16_t)tmp[1];
// 					/* Unmask the rest (if any) without using it */
// 					for (size_t k = 2; k < take; k++)
// 					{
// 						(void)(data[i + k] ^= st->mask[st->mask_i & 3]);
// 						st->mask_i++;
// 					}
// 				}
// 				else
// 				{
// 					for (size_t k = 0; k < take; k++)
// 					{
// 						(void)(data[i + k] ^= st->mask[st->mask_i & 3]);
// 						st->mask_i++;
// 					}
// 				}
// 				if (cb_close)
// 					cb_close(client_fd, code);
// 				websocket_send_close(client_fd, code);
// 			}
// 			else if (st->opcode == WS_OPCODE_PONG)
// 			{
// 				/* Unmask-and-ignore */
// 				for (size_t k = 0; k < take; k++)
// 				{
// 					(void)(data[i + k] ^= st->mask[st->mask_i & 3]);
// 					st->mask_i++;
// 				}
// 			}
// 			else
// 			{
// 				/* Text / Binary / Continuation payload delivery */
// 				ws_deliver_payload(client_fd, st, &data[i], take);
// 			}

// 			i += take;
// 			st->payload_rem -= take;

// 			if (st->payload_rem == 0)
// 			{
// 				/* Reset for next frame */
// 				st->hdr_have = 0;
// 				st->hdr_need = 0;
// 				st->mask_i = 0;
// 				/* If we were closing, stop processing further */
// 				if (st->opcode == WS_OPCODE_CLOSE)
// 				{
// 					st->phase = WS_S_CLOSING;
// 					return;
// 				}
// 			}
// 		}
// 	}
// }

// /*  Socket callbacks  */

// static void ws_on_data(int client_fd, void *buf, size_t len)
// {
// 	int slot = find_slot_by_fd(client_fd);
// 	if (slot < 0)
// 		return;
// 	ws_client_state_t *st = &ws_clients[slot];
// 	uint8_t *data = (uint8_t *)buf;

// 	if (!st->active)
// 		ws_reset_client(st);

// 	if (st->phase == WS_S_HANDSHAKE)
// 	{
// 		/* Accumulate headers until CRLFCRLF */
// 		if (st->hs_len < sizeof(st->hs_buf))
// 		{
// 			size_t space = sizeof(st->hs_buf) - st->hs_len;
// 			size_t cp = (len < space) ? len : space;
// 			memcpy(&st->hs_buf[st->hs_len], data, cp);
// 			st->hs_len += cp;

// 			int res = ws_do_handshake(slot);
// 			if (res < 0)
// 			{
// 				/* Bad handshake: ignore or optionally close connection */
// 				/* No active close API: leave it to peer */
// 				return;
// 			}
// 			else if (res > 0)
// 			{
// 				/* Handshake done; if there was overflow (more bytes beyond headers) we ignore.
// 					 Subsequent recv calls will deliver frames. */
// 			}
// 		}
// 		return;
// 	}

// 	if (st->phase == WS_S_OPEN)
// 	{
// 		ws_process_data_open(client_fd, slot, data, len);
// 		return;
// 	}

// 	/* WS_S_CLOSING: ignore further payload; rely on peer to close */
// }

// static void ws_on_connected(int client_fd)
// {
// 	int slot = find_slot_by_fd(client_fd);
// 	if (slot >= 0)
// 		ws_reset_client(&ws_clients[slot]);
// }

// static void ws_on_disconnected(int client_fd)
// {
// 	int slot = find_slot_by_fd(client_fd);
// 	if (slot >= 0)
// 	{
// 		memset(&ws_clients[slot], 0, sizeof(ws_clients[slot]));
// 	}
// }

// /*  Public callbacks registration  */

// void websocket_set_on_text(ws_on_text_cb cb) { cb_text = cb; }
// void websocket_set_on_binary(ws_on_binary_cb cb) { cb_binary = cb; }
// void websocket_set_on_open(ws_on_open_cb cb) { cb_open = cb; }
// void websocket_set_on_close(ws_on_close_cb cb) { cb_close = cb; }

// /*  Module glue  */

// #ifndef WEBSOCKET_PORT
// #define WEBSOCKET_PORT 80 /* change if needed */
// #endif

// DECL_MODULE(websocket_server)
// {
// 	ws_srv = socket_start(IP_ANY, WEBSOCKET_PORT, 2 /*AF_INET*/, 1 /*SOCK_STREAM*/, 0);
// 	if (!ws_srv)
// 		return;

// 	for (int i = 0; i < WEBSOCKET_MAX_CLIENTS; i++)
// 	{
// 		memset(&ws_clients[i], 0, sizeof(ws_clients[i]));
// 	}

// 	socket_add_ondata_handler(ws_srv, ws_on_data);
// 	socket_add_onconnected_handler(ws_srv, ws_on_connected);
// 	socket_add_ondisconnected_handler(ws_srv, ws_on_disconnected);

// 	/* Optional default behavior: welcome message after handshake (done in cb_open). */
// 	if (!cb_open)
// 	{
// 		/* Install a tiny default open-callback that sends a hello on first text frame echo. */
// 		cb_open = NULL; /* keep default no-op */
// 	}
// }

// void websocket_server_run(void)
// {
// 	socket_server_run(ws_srv);
// }
