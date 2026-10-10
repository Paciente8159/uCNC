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

#ifndef IPC_BUFFER_SIZE
#define IPC_BUFFER_SIZE 256
#endif
uint8_t g_ipc_buffer[IPC_BUFFER_SIZE];
bool g_ipc_data_available;

void ipc_init(void) { /*TODO*/ }

/**overridable send and receive methods - depends on the transport layer (SPI,
 * ethernet, etc..) */
void __attribute__((weak)) ipc_send(ipc_packet_type_t type, const void *data,
                                    size_t len) {}
size_t __attribute__((weak)) ipc_receive(ipc_packet_type_t type, void *data,
                                         size_t len)
{
  return 0;
}

/**
 * master to slave communication messages (orchestrator to executor)
 * If data_out is NULL then no data will be transmitted besides the message type
 * (to be executed by the slave)
 *
 *  */
void ipc_exec(ipc_packet_type_t type, const void *data_out, size_t len_out,
              void *data_in, size_t len_in)
{
  ipc_send(type, data_out, len_out);
  if (data_in)
  {
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
void ipc_dotasks(void)
{
  if (g_ipc_data_available)
  {
    ipc_packet_t *ipc_data = (ipc_packet_t *)g_ipc_buffer;
    ipc_packet_t response = {0};
    response.type = ipc_data->type;
    switch (ipc_data->type)
    {
    case ITP_SGM_IS_FULL:
      response.payload[0] = (itp_is_full() ? 1 : 0);
      response.len = 1;
      break;
    case ITP_BLK_WRITE:
      itp_push_blk((itp_block_t *)ipc_data->payload); // advance the writer
      break;
    case ITP_SGM_WRITE:
      itp_push_sgm((bool)ipc_data->payload[0], (itp_segment_t *)(&ipc_data->payload[1])); // advance the writer
      break;
    case ITP_STEP_RATE:
      // itp_step_rate_convert();
      break;
    case ITP_START:
      itp_start(((bool *)ipc_data->payload)[0]);
      break;
    default:
      break;
    }
  }
}