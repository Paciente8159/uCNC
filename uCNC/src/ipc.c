/*
        Name: ipc.c
        Description: Inter-Process Communication for µCNC.

        Copyright: Copyright (c) João Martins
        Author: João Martins
        Date: 09/10/2026

        µCNC is free software: you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version. Please see
   <http://www.gnu.org/licenses/>

        µCNC is distributed WITHOUT ANY WARRANTY;
        Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A
   PARTICULAR PURPOSE. See the	GNU General Public License for more details.
*/

#include "cnc.h"

void ipc_init(void) { /*TODO*/ }

/**overridable send and receive methods - depends on the transport layer (SPI,
 * ethernet, etc..) */
void __attribute__((weak)) ipc_send(ipc_packet_type_t type, const void *data,
                                    size_t len) {}
size_t __attribute__((weak)) ipc_receive(ipc_packet_type_t type, void *data,
                                         size_t len) {}

/**
 * master to slave communication messages (orchestrator to executor)
 * If data_out is NULL then no data will be transmitted besides the message type
 * (to be executed by the slave)
 *
 *  */
void ipc_exec(ipc_packet_type_t type, const void *data_out, size_t len_out,
              void *data_in, size_t len_in) {
  ipc_send(type, data_out, len_out);
  if (data_in) {
    ipc_receive(type, data_in, len_in);
  }
}

/**
 * tasks to be executed. On the slave side this means processing a buffer of
 * incomming requests and execute and reply to requests.
 *
 * The slave uses a generic buffer. Requests may contain a payload or not
 * (variable length payload), that needs processing. A request may expect a
 * response or not. In any case an ACK is required in order for the master to
 * know the message has been received.
 *
 * Only one request at time will be received and processed (no backpressure).
 */
void ipc_dotasks(void) { /*TODO*/ }