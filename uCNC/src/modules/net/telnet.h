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
#ifndef TELNET_H
#define TELNET_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <stdint.h>

#include "socket.h"

#define TELNET_SE 240U
#define TELNET_SB 250U
#define TELNET_WILL 251U
#define TELNET_WONT 252U
#define TELNET_DO 253U
#define TELNET_DONT 254U
#define TELNET_IAC 255U

typedef enum
{
	TELNET_PARSE_DATA = 0,
	TELNET_PARSE_IAC,
	TELNET_PARSE_OPTION,
	TELNET_PARSE_SUBNEGOTIATION,
	TELNET_PARSE_SUBNEGOTIATION_IAC
} telnet_parse_state_t;

typedef struct
{
	uint8_t parse_state;
	uint8_t pending_command;
} telnet_client_t;

typedef void (*telnet_data_callback_t)(uint8_t client_idx, const uint8_t *data, size_t data_len);

typedef struct
{
	socket_if_t *telnet_socket;
	telnet_client_t clients[SOCKET_MAX_CLIENTS];
	telnet_data_callback_t telnet_data;
} telnet_protocol_t;

/*
 * Starts a Telnet listener and installs all required socket callbacks.
 *
 * Contract:
 * - telnet and callback must remain valid until telnet_stop() returns.
 * - port is in host byte order.
 * - no dynamic memory is allocated.
 *
 * Result:
 * - returns the listener on success;
 * - returns NULL when an argument is invalid or the socket listener cannot be
 *   created. On failure telnet is left stopped.
 */
socket_if_t *telnet_start(telnet_protocol_t *telnet,
						  uint16_t port,
						  telnet_data_callback_t callback);

/*
 * Stops accepting clients and closes all clients owned by telnet.
 * Calling this with NULL, or on an already stopped instance, has no effect.
 * Pending bytes are discarded because socket_stop() closes clients.
 */
void telnet_stop(telnet_protocol_t *telnet);

/*
 * Returns true when at least one client is currently connected.
 * A NULL or stopped instance returns false.
 */
bool telnet_hasclients(const telnet_protocol_t *telnet);

/*
 * Sends application bytes to one Telnet client synchronously.
 *
 * No protocol TX buffer is retained. Blocking socket_send() returns exactly
 * data_len on success or a negative socket_device_result_t on invalid state,
 * timeout, disconnect, or transport failure.
 */
int telnet_send(telnet_protocol_t *telnet,
				uint8_t client_idx,
				const void *data,
				size_t data_len);

/*
 * Sends the payload to every client connected when the call starts. Each pass
 * gives every unfinished client a nonblocking socket_send() attempt and retains
 * only a stack-local per-client offset. The backend is polled between passes.
 * Clients still incomplete after timeout_ms are closed locally.
 */
void telnet_broadcast(telnet_protocol_t *telnet,
					  const void *data,
					  size_t data_len,
					  uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* TELNET_H */
