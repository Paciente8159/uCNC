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

uint8_t g_ipc_buffer[IPC_BUFFER_SIZE] __attribute__((aligned(8)));
// backing storage for the reply/ACK payload built by ipc_dotasks
static uint8_t g_ipc_response[IPC_BUFFER_SIZE] __attribute__((aligned(8)));
bool g_ipc_data_available;

/**
 * The last request sequence executed on the RT side, and the reply it produced.
 * A retransmission repeats the same sequence, so the cached reply is sent back
 * instead of running a non-idempotent command twice.
 * */
typedef struct
{
  bool valid;
  ipc_packet_type_t type;
  uint8_t seq;
  int8_t status;
  uint16_t len;
  uint8_t payload[IPC_MAX_PAYLOAD];
} ipc_reply_cache_t;

static ipc_reply_cache_t g_ipc_last_reply;
static volatile bool g_ipc_fault;

// the settings the RT side reads are pushed in a single message, so that block
// must fit the request buffer
typedef char ipc_settings_block_fits[(sizeof(rt_settings_t) <= IPC_MAX_PAYLOAD) ? 1 : -1];

bool ipc_init(void)
{
#if ASSERT_IPC_NONRT()
#if ASSERT_IPC_MULTICORE()
  if (ipc_get_current_core() != IPC_NON_RT_CORE)
  {
    return true; // the RT side answers the ping, it does not send one
  }
#endif
  ipc_packet_t ping = {0};
  ping.type = IPC_PING;
  ping.seq = 0;
  ping.status = 0;
  uint32_t start = mcu_millis();
  while ((mcu_millis() - start) < IPC_BOOT_TIMEOUT)
  {
    if (ipc_send(&ping) == IPC_STATUS_OK && ipc_receive(&ping) == IPC_STATUS_OK)
    {
      ipc_clear_fault();
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
int8_t __attribute__((weak)) ipc_send(const ipc_packet_t *packet)
{
  (void)packet;
  return IPC_STATUS_IO;
}
int8_t __attribute__((weak)) ipc_receive(ipc_packet_t *packet)
{
  (void)packet;
  return IPC_STATUS_IO;
}

void __attribute__((weak)) ipc_poll(void) {}

bool ipc_has_fault(void)
{
  return g_ipc_fault;
}

void ipc_clear_fault(void)
{
  g_ipc_fault = false;
}

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
 * The same sequence is reused when a request is resent, so the RT side can send
 * its cached reply instead of executing the command twice.
 *  */
int8_t ipc_exec(ipc_packet_type_t type, const void *data_out, size_t len_out,
                void *data_in, size_t len_in)
{
#if ASSERT_IPC_NONRT()
  static uint8_t next_seq;
  ipc_packet_t request = {0};
  ipc_packet_t reply = {0};

  request.type = type;
  request.seq = (uint8_t)++next_seq;
  request.status = 0;
  request.len = len_out;
  request.payload = (uint8_t *)data_out;

  reply.type = type;
  reply.seq = request.seq;
  reply.status = IPC_STATUS_TIMEOUT;
  reply.len = len_in;
  reply.payload = (uint8_t *)data_in;

  for (uint16_t tries = 0; tries < IPC_MAX_TRIES; tries++)
  {
    int8_t status = ipc_send(&request);
    if (status != IPC_STATUS_OK)
    {
      g_ipc_fault = true;
      return status;
    }

    status = ipc_receive(&reply);
    if (status != IPC_STATUS_TIMEOUT && status != IPC_STATUS_IO && status != IPC_STATUS_BAD_REPLY)
    {
      return status;
    }
  }

  g_ipc_fault = true;
  return IPC_STATUS_TIMEOUT;
#else
  (void)type;
  (void)data_out;
  (void)len_out;
  (void)data_in;
  (void)len_in;
  return IPC_STATUS_OK;
#endif
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

  /**
   * A retried request repeats its sequence. The command has already run, so
   * send the cached reply instead of executing it again.
   * */
  if (g_ipc_last_reply.valid && (ipc_data->seq == g_ipc_last_reply.seq) &&
      (ipc_data->type == g_ipc_last_reply.type))
  {
    ipc_packet_t cached = {0};
    cached.type = g_ipc_last_reply.type;
    cached.seq = g_ipc_last_reply.seq;
    cached.status = g_ipc_last_reply.status;
    cached.len = g_ipc_last_reply.len;
    cached.payload = g_ipc_last_reply.payload;
    ipc_send(&cached);
    g_ipc_data_available = false;
    return;
  }

  ipc_packet_t response = {0};
  response.type = ipc_data->type;
  response.seq = ipc_data->seq;
  response.status = IPC_STATUS_OK;
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
    /**
     * The reply reports the buffer's state once the segment is in it. The
     * NonRT side pushes the segments and the RT side consumes them, so a
     * buffer reported as not full here stays not full until the NonRT side
     * adds to it: carrying the answer back saves the feed loop a request of
     * its own to ask for it (see itp_run).
     * */
    response.payload[0] = (itp_is_full() ? 1 : 0);
    response.len = 1;
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
  case IO_SET_PINVALUE:
    io_set_pinvalue(ipc_data->payload[0], ipc_data->payload[1]);
    break;
  case IO_GET_PINVALUE:
    *(int16_t *)response.payload = io_get_pinvalue(ipc_data->payload[0]);
    response.len = sizeof(int16_t);
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
    response.status = IPC_STATUS_ERROR;
    break;
  }

  // always reply. The master waits for the reply even when the command has no
  // response payload.
  ipc_send(&response);

  // remember the reply so a retransmitted request is answered, not re-executed
  g_ipc_last_reply.valid = true;
  g_ipc_last_reply.type = response.type;
  g_ipc_last_reply.seq = response.seq;
  g_ipc_last_reply.status = response.status;
  g_ipc_last_reply.len = (uint16_t)MIN(response.len, (size_t)IPC_MAX_PAYLOAD);
  if (g_ipc_last_reply.len)
  {
    memcpy(g_ipc_last_reply.payload, response.payload, g_ipc_last_reply.len);
  }

  g_ipc_data_available = false;
}
