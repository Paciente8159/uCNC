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

/*
 * Static transmit storage reserved for each Telnet client.
 *
 * The buffer must be at least three bytes so an IAC negotiation reply can be
 * queued atomically. Applications may override this before including telnet.h.
 * A larger value absorbs longer application bursts at the cost of static RAM.
 */
#ifndef TELNET_TX_BUFFER_SIZE
#define TELNET_TX_BUFFER_SIZE 64
#endif

#if TELNET_TX_BUFFER_SIZE < 3U
#error "TELNET_TX_BUFFER_SIZE must be at least 3"
#endif

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
	telnet_parse_state_t parse_state;
	uint8_t pending_command;
	uint8_t tx_buffer[TELNET_TX_BUFFER_SIZE];
	size_t tx_offset;
	size_t tx_length;
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
 * Copies application bytes into one client's persistent transmit queue.
 *
 * Contract:
 * - data may be NULL only when data_len is zero;
 * - the source may be transient: accepted bytes are copied before return;
 * - this function is nonblocking and performs at most one socket_send attempt;
 * - only the protocol writable callback resumes a partial backend send.
 *
 * Result:
 * - returns the number of payload bytes accepted into the queue;
 * - returns SOCKET_DEVICE_WOULD_BLOCK when the client is connected but no queue
 *   space is available;
 * - returns another negative socket error for invalid/disconnected clients.
 * A short positive result is valid; retry only the unaccepted suffix later.
 */
int telnet_send(telnet_protocol_t *telnet,
				uint8_t client_idx,
				const void *data,
				size_t data_len);

/*
 * Independently queues the same payload for every connected Telnet client.
 *
 * Result:
 * - returns the smallest number of payload bytes accepted by any target;
 * - returns 0 when there are no connected clients;
 * - may return SOCKET_DEVICE_WOULD_BLOCK if at least one target cannot accept a
 *   byte. Faster clients can still have accepted data, so callers must not
 *   blindly rebroadcast the complete payload after a short/blocked result.
 */
int telnet_broadcast(telnet_protocol_t *telnet,
					 const void *data,
					 size_t data_len);

#ifdef __cplusplus
}
#endif

#endif /* TELNET_H */
