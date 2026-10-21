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

uint8_t g_ipc_buffer[IPC_BUFFER_SIZE];
// backing storage for the reply/ACK payload built by ipc_dotasks
static uint8_t g_ipc_response[IPC_BUFFER_SIZE];
bool g_ipc_data_available;

// the settings the RT side reads are pushed in a single message, so that block
// must fit the request buffer
typedef char ipc_settings_block_fits[((sizeof(rt_settings_t) + sizeof(ipc_packet_t)) <= IPC_BUFFER_SIZE) ? 1 : -1];

bool ipc_init(void)
{
#if ASSERT_IPC_NONRT()
#if ASSERT_IPC_MULTICORE()
  if (ipc_get_current_core() != IPC_NON_RT_CORE)
  {
    return true; // the RT side answers the ping, it does not send one
  }
#endif
  uint32_t start = mcu_millis();
  while ((mcu_millis() - start) < IPC_BOOT_TIMEOUT)
  {
    ipc_send(IPC_PING, NULL, 0);
    if (ipc_receive(IPC_PING, NULL, 0))
    {
      return true;
    }
    // cooperate with the task loop. On the host test build this is also what
    // advances the simulated clock, that has no timer thread of its own.
    TASK_YIELD();
  }

  return false;
#else
  return true;
#endif
}

/**overridable send and receive methods - depends on the transport layer (SPI,
 * ethernet, etc..) */
void __attribute__((weak)) ipc_send(ipc_packet_type_t type, const void *data,
                                    size_t len) {}
size_t __attribute__((weak)) ipc_receive(ipc_packet_type_t type, void *data,
                                         size_t len)
{
  return 0;
}

void __attribute__((weak)) ipc_poll(void) {}

#ifndef IPC_CURRENT_CORE_OVERRIDE
uint8_t ipc_get_current_core(void)
{
  return IPC_RT_CORE;
}
#endif

/**
 * master to slave communication messages (orchestrator to executor)
 * If data_out is NULL then no data will be transmitted besides the message type
 * (to be executed by the slave)
 *
 * The slave always replies, so the ACK is always waited for, even when the
 * request carries no response payload (data_in is NULL).
 *
 *  */
void ipc_exec(ipc_packet_type_t type, const void *data_out, size_t len_out,
              void *data_in, size_t len_in)
{
  ipc_send(type, data_out, len_out);
  ipc_receive(type, data_in, len_in);
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
  // give the transport a chance to fetch a pending request
  ipc_poll();

  if (!g_ipc_data_available)
  {
    return;
  }

  ipc_packet_t *ipc_data = (ipc_packet_t *)g_ipc_buffer;
  ipc_packet_t response = {0};
  response.type = ipc_data->type;
  response.payload = g_ipc_response;
  switch (ipc_data->type)
  {
  case IPC_PING:
    // the reply echoes the request type, so the ACK is sent at the end
    break;
  case ITP_IS_FULL:
    response.payload[0] = (itp_is_full() ? 1 : 0);
    response.len = 1;
    break;
  case ITP_PUSH_BLK:
    itp_push_blk((itp_block_t *)ipc_data->payload); // advance the writer
    break;
  case ITP_PUSH_SGM:
    itp_push_sgm((bool)ipc_data->payload[0], (itp_segment_t *)(&ipc_data->payload[1])); // advance the writer
    break;
  case ITP_STEP_RATE_CONVERT:
  {
    uint16_t *rate = (uint16_t *)response.payload;
    itp_step_rate_convert(*(float *)ipc_data->payload, &rate[0], &rate[1]);
    response.len = (sizeof(uint16_t) * 2);
    break;
  }
  case ITP_START:
    itp_start(((bool *)ipc_data->payload)[0]);
    break;
  case ITP_CLEAR:
    itp_clear();
    break;
  case ITP_STOP:
    itp_stop();
    break;
  case ITP_GET_RT_POSITION:
    itp_get_rt_position((int32_t *)response.payload);
    response.len = (sizeof(int32_t) * STEPPER_COUNT);
    break;
  case ITP_RESET_RT_POSITION:
  {
    itp_reset_rt_position_t *packet = (itp_reset_rt_position_t *)ipc_data->payload;
#if STEPPERS_ENCODERS_MASK != 0
    itp_set_rt_position(packet->steps, packet->origin);
#else
    itp_set_rt_position(packet->steps, NULL);
#endif
    break;
  }
  case ITP_SYNC_RT_POSITION:
    itp_sync_rt_position((int32_t *)ipc_data->payload);
    break;
  case ITP_GET_RT_FEED:
    *(float *)response.payload = itp_get_rt_feed();
    response.len = sizeof(float);
    break;
  case IO_LOCK_LIMITS:
#ifdef ENABLE_MULTI_STEP_HOMING
    io_lock_limits(ipc_data->payload[0]);
#endif
    break;
  case IO_ENABLE_LIMITS:
    io_enable_limits();
    break;
  case IO_DISABLE_LIMITS:
    io_disable_limits();
    break;
  case IO_INVERT_LIMITS:
    io_invert_limits(ipc_data->payload[0]);
    break;
  case IO_GET_RAW_LIMITS:
    response.payload[0] = io_get_raw_limits();
    response.len = 1;
    break;
  case IO_GET_LIMITS:
    response.payload[0] = io_get_limits();
    response.len = 1;
    break;
  case IO_ENABLE_PROBE:
    io_enable_probe();
    break;
  case IO_DISABLE_PROBE:
    io_disable_probe();
    break;
  case IO_GET_PROBE:
    response.payload[0] = (io_get_probe() ? 1 : 0);
    response.len = 1;
    break;
  case IO_GET_PROBE_POSITION:
    io_get_probe_step_pos((int32_t *)response.payload);
    response.len = (sizeof(int32_t) * STEPPER_COUNT);
    break;
  case IO_SYNC_PROBE_POSITION:
    io_sync_probe_step_pos();
    break;
  case IO_SET_STEPS:
    io_set_steps(ipc_data->payload[0]);
    break;
  case IO_TOGGLE_STEPS:
    io_toggle_steps(ipc_data->payload[0]);
    break;
  case IO_SET_DIRS:
    io_set_dirs(ipc_data->payload[0]);
    break;
  case IO_GET_STEPS_POS:
    io_get_steps_pos((int32_t *)response.payload);
    response.len = (sizeof(int32_t) * STEPPER_COUNT);
    break;
  case IO_ENABLE_STEPPERS:
    io_enable_steppers(ipc_data->payload[0]);
    break;
  case CNC_GET_EXEC_STATE:
    *(uint16_t *)response.payload =
        cnc_get_exec_state(*(uint16_t *)ipc_data->payload);
    response.len = sizeof(uint16_t);
    break;
  case CNC_SET_EXEC_STATE_RT:
    cnc_set_exec_state_rt(*(uint16_t *)ipc_data->payload);
    *(uint16_t *)response.payload =
        cnc_get_exec_state(EXEC_ALLACTIVE);
    response.len = sizeof(uint16_t);
    break;
  case CNC_CLEAR_EXEC_STATE_RT:
    cnc_clear_exec_state_rt(*(uint16_t *)ipc_data->payload);
    *(uint16_t *)response.payload =
        cnc_get_exec_state(EXEC_ALLACTIVE);
    response.len = sizeof(uint16_t);
    break;
  case TOOL_SET_SPEED:
    tool_set_speed(*(int16_t *)ipc_data->payload);
    break;
  case TOOL_SET_COOLANT:
    tool_set_coolant(ipc_data->payload[0]);
    break;
  case TOOL_SET_MODE:
    tool_set_mode(ipc_data->payload[0]);
    break;
  case SETTINGS_SYNC:
    settings_apply_rt(ipc_data->payload,
                      (uint16_t)MIN(ipc_data->len, sizeof(settings_t)));
    break;
  default:
    break;
  }

  // always reply. The master waits for the ACK even when the command has no
  // response payload.
  ipc_send(response.type, response.payload, response.len);
  g_ipc_data_available = false;
}
