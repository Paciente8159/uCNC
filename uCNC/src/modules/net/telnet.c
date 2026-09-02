/*
	Name: telnet.h
	Description: Small, allocation-free, Telnet server API for uCNC.

	Copyright: Copyright (c) Joao Martins
	Author: Joao Martins

	uCNC is free software: you can redistribute it and/or modify it under the
	terms of the GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later
	version. Please see <http://www.gnu.org/licenses/>.
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

/* Send a complete Telnet negotiation response without retaining TX state. */
static void telnet_refuse_option(telnet_protocol_t *telnet,
								 uint8_t client_idx,
								 uint8_t command,
								 uint8_t option)
{
	uint8_t reply[3];

	if (!telnet || !telnet->telnet_socket || client_idx >= SOCKET_MAX_CLIENTS)
		return;
	reply[0] = TELNET_IAC;
	reply[1] = (command == TELNET_DO || command == TELNET_DONT) ? TELNET_WONT : TELNET_DONT;
	reply[2] = option;
	if (socket_send(telnet->telnet_socket, client_idx, reply, sizeof(reply), false) < 0 &&
		socket_client_is_connected(telnet->telnet_socket, client_idx))
	{
		(void)socket_close(telnet->telnet_socket, client_idx);
	}
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
	if (socket_send(telnet->telnet_socket, client_idx, telnet_welcome,
					sizeof(telnet_welcome), false) < 0 &&
		socket_client_is_connected(telnet->telnet_socket, client_idx))
	{
		(void)socket_close(telnet->telnet_socket, client_idx);
	}
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
	if (!telnet || !telnet->telnet_socket)
		return SOCKET_DEVICE_INVALID;
	return socket_send(telnet->telnet_socket, client_idx, data, data_len, false);
}

void telnet_broadcast(telnet_protocol_t *telnet,
					  const void *data,
					  size_t data_len,
					  uint32_t timeout_ms)
{
	const uint8_t *payload = (const uint8_t *)data;
	size_t offsets[SOCKET_MAX_CLIENTS];
	const size_t inactive = (size_t)-1;
	uint32_t start_ms;
	bool any_target = false;
	uint8_t i;

	if (!telnet || !telnet->telnet_socket || (!data && data_len != 0U) ||
		data_len == 0U || data_len > (size_t)INT_MAX)
		return;

	for (i = 0U; i < SOCKET_MAX_CLIENTS; ++i)
	{
		if (socket_client_is_connected(telnet->telnet_socket, i))
		{
			offsets[i] = 0U;
			any_target = true;
		}
		else
		{
			offsets[i] = inactive;
		}
	}
	if (!any_target)
		return;

	start_ms = mcu_millis();
	for (;;)
	{
		bool pending = false;

		for (i = 0U; i < SOCKET_MAX_CLIENTS; ++i)
		{
			int sent;

			if (offsets[i] == inactive || offsets[i] >= data_len)
				continue;
			if (!socket_client_is_connected(telnet->telnet_socket, i))
			{
				offsets[i] = inactive;
				continue;
			}

			sent = socket_send(telnet->telnet_socket, i,
							   &payload[offsets[i]], data_len - offsets[i], true);
			if (sent > 0)
			{
				offsets[i] += (size_t)sent;
				if (offsets[i] < data_len)
					pending = true;
			}
			else if (sent == 0)
			{
				pending = true;
			}
			else
			{
				if (socket_client_is_connected(telnet->telnet_socket, i))
					(void)socket_close(telnet->telnet_socket, i);
				offsets[i] = inactive;
			}
		}

		if (!pending)
			return;
		if ((uint32_t)(mcu_millis() - start_ms) >= timeout_ms)
		{
			for (i = 0U; i < SOCKET_MAX_CLIENTS; ++i)
			{
				if (offsets[i] != inactive && offsets[i] < data_len &&
					socket_client_is_connected(telnet->telnet_socket, i))
				{
					(void)socket_close(telnet->telnet_socket, i);
				}
			}
			return;
		}

		socket_server_poll();
	}
}

#endif
