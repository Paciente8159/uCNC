/*
	Copyright (c) 2026 - uCNC

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/
#include "../../cnc.h"
#ifdef ENABLE_SOCKETS

#include "telnet.h"
#include <limits.h>
#include <string.h>

static const uint8_t telnet_welcome[] = {
	TELNET_IAC, TELNET_WILL, 0x01U,
	TELNET_IAC, TELNET_WILL, 0x03U,
	TELNET_IAC, TELNET_WONT, 0x22U};

static void telnet_client_reset(telnet_client_t *client)
{
	if (client)
	{
		memset(client, 0, sizeof(*client));
		client->parse_state = TELNET_PARSE_DATA;
	}
}

/*
 * Makes exactly one nonblocking backend-send attempt for a client's queued
 * bytes. The unsent suffix always remains in persistent client storage.
 *
 * This is intentionally the only Telnet helper that calls socket_send().
 * It is safe to call from ordinary API code and from the writable callback;
 * it does not loop or wait for progress.
 */
static int telnet_flush(telnet_protocol_t *telnet, uint8_t client_idx)
{
	telnet_client_t *client;
	int sent;

	if (!telnet || !telnet->telnet_socket || client_idx >= SOCKET_MAX_CLIENTS)
		return SOCKET_DEVICE_INVALID;

	client = &telnet->clients[client_idx];
	if (client->tx_length == 0U)
	{
		client->tx_offset = 0U;
		return 0;
	}

	sent = socket_send(telnet->telnet_socket,
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

	return sent;
}

/*
 * Copies as much payload as possible into persistent storage, then gives the
 * backend one opportunity to drain it. Accepted bytes remain owned by Telnet
 * even if the backend reports a partial send or WOULD_BLOCK.
 */
static int telnet_queue(telnet_protocol_t *telnet,
						uint8_t client_idx,
						const uint8_t *data,
						size_t data_len)
{
	telnet_client_t *client;
	size_t free_space;
	size_t accepted;
	bool was_pending;

	if (!telnet || !telnet->telnet_socket || client_idx >= SOCKET_MAX_CLIENTS ||
		(!data && data_len != 0U))
		return SOCKET_DEVICE_INVALID;
	if (!socket_client_is_connected(telnet->telnet_socket, client_idx))
		return SOCKET_DEVICE_CLOSED;
	if (data_len == 0U)
		return 0;

	client = &telnet->clients[client_idx];
	was_pending = client->tx_length != 0U;
	if (client->tx_offset != 0U &&
		client->tx_offset + client->tx_length + data_len > sizeof(client->tx_buffer))
	{
		memmove(client->tx_buffer,
				&client->tx_buffer[client->tx_offset],
				client->tx_length);
		client->tx_offset = 0U;
	}

	free_space = sizeof(client->tx_buffer) - client->tx_offset - client->tx_length;
	accepted = data_len < free_space ? data_len : free_space;
	if (accepted == 0U)
		return SOCKET_DEVICE_WOULD_BLOCK;

	memcpy(&client->tx_buffer[client->tx_offset + client->tx_length], data, accepted);
	client->tx_length += accepted;
	if (!was_pending)
		(void)telnet_flush(telnet, client_idx);

	return accepted > (size_t)INT_MAX ? INT_MAX : (int)accepted;
}

/*
 * Queues a complete three-byte Telnet negotiation response atomically. The
 * command is omitted when three bytes are not available; it is never emitted
 * partially because a truncated IAC command would corrupt the byte stream.
 */
static void telnet_refuse_option(telnet_protocol_t *telnet,
								 uint8_t client_idx,
								 uint8_t command,
								 uint8_t option)
{
	telnet_client_t *client;
	uint8_t reply[3];
	size_t free_space;

	if (!telnet || client_idx >= SOCKET_MAX_CLIENTS)
		return;

	client = &telnet->clients[client_idx];
	if (client->tx_offset != 0U &&
		client->tx_offset + client->tx_length + sizeof(reply) > sizeof(client->tx_buffer))
	{
		memmove(client->tx_buffer,
				&client->tx_buffer[client->tx_offset],
				client->tx_length);
		client->tx_offset = 0U;
	}
	free_space = sizeof(client->tx_buffer) - client->tx_offset - client->tx_length;
	if (free_space < sizeof(reply))
		return;

	reply[0] = TELNET_IAC;
	reply[1] = (command == TELNET_DO || command == TELNET_DONT) ? TELNET_WONT : TELNET_DONT;
	reply[2] = option;
	(void)telnet_queue(telnet, client_idx, reply, sizeof(reply));
}

/*
 * Incremental Telnet decoder. Parser state is kept per client, so IAC commands
 * and subnegotiation sequences may be split across arbitrary TCP callbacks.
 * Application data is delivered as immutable one-byte spans; no RX buffer is
 * modified in place.
 */
static void telnet_on_data(uint8_t client_idx,
						   const uint8_t *data,
						   size_t data_len,
						   void *protocol)
{
	telnet_protocol_t *telnet = (telnet_protocol_t *)protocol;
	telnet_client_t *client;
	size_t i;

	if (!telnet || client_idx >= SOCKET_MAX_CLIENTS || (!data && data_len != 0U))
		return;
	client = &telnet->clients[client_idx];

	for (i = 0U; i < data_len; ++i)
	{
		uint8_t byte = data[i];
		switch (client->parse_state)
		{
		case TELNET_PARSE_DATA:
			if (byte == TELNET_IAC)
				client->parse_state = TELNET_PARSE_IAC;
			else if (telnet->telnet_data)
				telnet->telnet_data(client_idx, &data[i], 1U);
			break;

		case TELNET_PARSE_IAC:
			if (byte == TELNET_IAC)
			{
				if (telnet->telnet_data)
					telnet->telnet_data(client_idx, &data[i], 1U);
				client->parse_state = TELNET_PARSE_DATA;
			}
			else if (byte == TELNET_DO || byte == TELNET_DONT ||
					 byte == TELNET_WILL || byte == TELNET_WONT)
			{
				client->pending_command = byte;
				client->parse_state = TELNET_PARSE_OPTION;
			}
			else if (byte == TELNET_SB)
				client->parse_state = TELNET_PARSE_SUBNEGOTIATION;
			else
				client->parse_state = TELNET_PARSE_DATA;
			break;

		case TELNET_PARSE_OPTION:
			telnet_refuse_option(telnet, client_idx, client->pending_command, byte);
			client->parse_state = TELNET_PARSE_DATA;
			break;

		case TELNET_PARSE_SUBNEGOTIATION:
			if (byte == TELNET_IAC)
				client->parse_state = TELNET_PARSE_SUBNEGOTIATION_IAC;
			break;

		case TELNET_PARSE_SUBNEGOTIATION_IAC:
			client->parse_state = (byte == TELNET_SE) ? TELNET_PARSE_DATA
													  : TELNET_PARSE_SUBNEGOTIATION;
			break;

		default:
			client->parse_state = TELNET_PARSE_DATA;
			break;
		}
	}
}

static void telnet_on_connected(uint8_t client_idx, void *protocol)
{
	telnet_protocol_t *telnet = (telnet_protocol_t *)protocol;

	if (!telnet || client_idx >= SOCKET_MAX_CLIENTS)
		return;
	telnet_client_reset(&telnet->clients[client_idx]);
	(void)telnet_queue(telnet, client_idx, telnet_welcome, sizeof(telnet_welcome));
}

/*
 * A writable notification means a prior partial/blocked send may make progress.
 * It is a continuation signal, not permission to regenerate application data.
 */
static void telnet_on_writable(uint8_t client_idx, void *protocol)
{
	(void)telnet_flush((telnet_protocol_t *)protocol, client_idx);
}

static void telnet_on_disconnected(uint8_t client_idx, int reason, void *protocol)
{
	telnet_protocol_t *telnet = (telnet_protocol_t *)protocol;
	(void)reason;

	if (telnet && client_idx < SOCKET_MAX_CLIENTS)
		telnet_client_reset(&telnet->clients[client_idx]);
}

socket_if_t *telnet_start(telnet_protocol_t *telnet,
						  uint16_t port,
						  telnet_data_callback_t callback)
{
	socket_if_t *socket;
	uint8_t i;

	if (!telnet || !callback)
		return NULL;

	memset(telnet, 0, sizeof(*telnet));
	for (i = 0U; i < SOCKET_MAX_CLIENTS; ++i)
		telnet_client_reset(&telnet->clients[i]);

	socket = socket_start(IP_ANY, port);
	if (!socket)
		return NULL;

	telnet->telnet_socket = socket;
	telnet->telnet_data = callback;
	socket_set_protocol(socket, telnet);
	socket_add_ondata_handler(socket, telnet_on_data);
	socket_add_onconnected_handler(socket, telnet_on_connected);
	socket_add_onwritable_handler(socket, telnet_on_writable);
	socket_add_ondisconnected_handler(socket, telnet_on_disconnected);
	return socket;
}

void telnet_stop(telnet_protocol_t *telnet)
{
	uint8_t i;

	if (!telnet)
		return;
	if (telnet->telnet_socket)
		socket_stop(telnet->telnet_socket);
	for (i = 0U; i < SOCKET_MAX_CLIENTS; ++i)
		telnet_client_reset(&telnet->clients[i]);
	telnet->telnet_socket = NULL;
	telnet->telnet_data = NULL;
}

bool telnet_hasclients(const telnet_protocol_t *telnet)
{
	uint8_t i;

	if (!telnet || !telnet->telnet_socket)
		return false;
	for (i = 0U; i < SOCKET_MAX_CLIENTS; ++i)
	{
		if (socket_client_is_connected(telnet->telnet_socket, i))
			return true;
	}
	return false;
}

int telnet_send(telnet_protocol_t *telnet,
				uint8_t client_idx,
				const void *data,
				size_t data_len)
{
	return telnet_queue(telnet, client_idx, (const uint8_t *)data, data_len);
}

int telnet_broadcast(telnet_protocol_t *telnet,
					 const void *data,
					 size_t data_len)
{
	int minimum = INT_MAX;
	bool found = false;
	uint8_t i;

	if (!telnet || !telnet->telnet_socket || (!data && data_len != 0U))
		return SOCKET_DEVICE_INVALID;
	for (i = 0U; i < SOCKET_MAX_CLIENTS; ++i)
	{
		int accepted;
		if (!socket_client_is_connected(telnet->telnet_socket, i))
			continue;
		found = true;
		accepted = telnet_queue(telnet, i, (const uint8_t *)data, data_len);
		if (accepted < minimum)
			minimum = accepted;
	}
	return found ? minimum : 0;
}

#endif
