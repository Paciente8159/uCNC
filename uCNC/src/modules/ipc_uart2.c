/*
        Name: ipc_uart2.c
        Description: IPC transport over UART2 for µCNC.

        Copyright: Copyright (c) João Martins
        Author: João Martins
        Date: 19/10/2026

        µCNC is free software: you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version. Please see
   <http://www.gnu.org/licenses/>

        µCNC is distributed WITHOUT ANY WARRANTY;
        Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A
   PARTICULAR PURPOSE. See the	GNU General Public License for more details.
*/

#include "../cnc.h"

/**
 * Asking for this transport without a UART2 to run it on would leave the weak
 * no-op transport in ipc.c in place, so every forwarded request would silently
 * go nowhere and ipc_init() would just take IPC_BOOT_TIMEOUT to fail. Trap it
 * at build time instead.
 * */
#if defined(IPC_USES_UART2) && !defined(MCU_HAS_UART2)
#error "IPC_USES_UART2 needs a MCU that has UART2 (MCU_HAS_UART2)"
#endif

#if defined(IPC_USES_UART2) && defined(MCU_HAS_UART2)

#include "ipc_uart2.h"

/**
 * The frame being rebuilt out of the byte stream, and how far it got.
 *
 * The HAL already has a receive ring, but the ring is as large as one gcode
 * line (RX_BUFFER_SIZE) and a frame can be larger than that, so the frame is
 * assembled here instead: the ring only has to cover the time between two
 * calls to ipc_uart2_drain(), and this buffer has to cover a whole message.
 * */
static uint8_t ipc_uart2_rx_frame[IPC_UART2_FRAME_MAX];
static uint16_t ipc_uart2_rx_len;   // bytes of the frame stored so far
static uint16_t ipc_uart2_rx_total; // whole frame length, 0 until LEN is in
static bool ipc_uart2_rx_ready;     // a checked frame is waiting to be handed over
static uint32_t ipc_uart2_rx_last_byte;

/**
 * CRC16-CCITT over a block of bytes, in the shape the frame layout asks for
 * (see ipc_uart2.h). Feeding it the frame header first and the payload after
 * gives the same value as one pass over both, so a sender that streams the
 * payload never has to hold the whole frame.
 * */
static uint16_t ipc_uart2_crc16(uint16_t crc, const void *data, size_t len)
{
  const uint8_t *bytes = (const uint8_t *)data;

  while (len--)
  {
    crc ^= (uint16_t)((uint16_t)(*bytes++) << 8);

    for (uint8_t bit = 0; bit < 8; bit++)
    {
      crc = (uint16_t)((crc & 0x8000u) ? ((crc << 1) ^ IPC_UART2_CRC_POLY) : ((uint16_t)(crc << 1)));
    }
  }

  return crc;
}

// the payload length a frame in the assembler declares
static uint16_t ipc_uart2_frame_len(void)
{
  return (uint16_t)((uint16_t)ipc_uart2_rx_frame[IPC_UART2_OFF_LEN] |
                    ((uint16_t)ipc_uart2_rx_frame[IPC_UART2_OFF_LEN + 1] << 8));
}

// drops whatever the assembler holds, so the next byte starts a new frame
static void ipc_uart2_reset(void)
{
  ipc_uart2_rx_len = 0;
  ipc_uart2_rx_total = 0;
  ipc_uart2_rx_ready = false;
}

/**
 * Checks a whole frame that is in the assembler and keeps it if it is sound.
 * This is where a corrupted message dies: the trailer has to close the frame
 * and the CRC has to match what the frame carries, or the frame is dropped and
 * never reaches the caller.
 * */
static void ipc_uart2_check(void)
{
  uint16_t payload = ipc_uart2_frame_len();
  uint16_t total = ipc_uart2_rx_total;
  // TYPE, LEN and PAYLOAD, which is everything between START and the CRC
  uint16_t crc = ipc_uart2_crc16(IPC_UART2_CRC_INIT, &ipc_uart2_rx_frame[IPC_UART2_OFF_TYPE],
                                 (size_t)((IPC_UART2_OFF_DATA - 1u) + payload));
  uint16_t frame_crc = (uint16_t)((uint16_t)ipc_uart2_rx_frame[total - 3] |
                                  ((uint16_t)ipc_uart2_rx_frame[total - 2] << 8));

  if ((ipc_uart2_rx_frame[total - 1] != IPC_UART2_END) || (crc != frame_crc))
  {
    ipc_uart2_reset();
    return;
  }

  ipc_uart2_rx_ready = true;
}

// takes one byte of the stream into the frame being rebuilt
static void ipc_uart2_feed(uint8_t c)
{
  if (ipc_uart2_rx_ready)
  {
    /**
     * A checked frame is still waiting to be handed over, so there is nowhere
     * for this byte to go. Dropping it with the frame is what keeps the two
     * sides in step: the caller reads a whole message before another one is
     * sent, so anything arriving here belongs to a frame that has nowhere to
     * be kept.
     * */
    return;
  }

  if (ipc_uart2_rx_len == 0)
  {
    // hunting for the start. Everything before it is line noise or the tail of
    // a frame that was already lost
    if (c != IPC_UART2_START)
    {
      return;
    }

    ipc_uart2_rx_frame[0] = c;
    ipc_uart2_rx_len = 1;
    ipc_uart2_rx_total = 0;
    return;
  }

  if (ipc_uart2_rx_len >= IPC_UART2_FRAME_MAX)
  {
    /**
     * Unreachable while LEN is checked as soon as it is known, which is what
     * the block below does. Kept as a floor so that a frame can never be built
     * past the end of the buffer.
     * */
    ipc_uart2_reset();
    return;
  }

  ipc_uart2_rx_frame[ipc_uart2_rx_len++] = c;

  if (ipc_uart2_rx_len == IPC_UART2_OFF_DATA)
  {
    /**
     * The header is complete, so the length of the whole frame is known and can
     * be refused before another byte of it is stored. A peer built with a wider
     * buffer than this one would otherwise run the assembler past its end.
     * */
    uint16_t payload = ipc_uart2_frame_len();

    if (payload > IPC_MAX_PAYLOAD)
    {
      ipc_uart2_reset();
      return;
    }

    ipc_uart2_rx_total = (uint16_t)(payload + IPC_UART2_OVERHEAD);
  }

  if ((ipc_uart2_rx_total != 0) && (ipc_uart2_rx_len == ipc_uart2_rx_total))
  {
    ipc_uart2_check();
  }
}

/**
 * Moves everything the HAL has buffered into the assembler, and drops a frame
 * that has stopped arriving.
 *
 * The resync is the important part. A frame the line ate leaves the assembler
 * waiting for bytes that never come, and the next frame then supplies them: the
 * wait would swallow that frame whole before the byte count came out and the
 * check failed. A gap longer than IPC_UART2_RESYNC_TIMEOUT cannot happen inside
 * a frame - the sender emits one back to back - so it means the frame is gone
 * and the assembler has to start over.
 * */
static void ipc_uart2_drain(void)
{
  while (mcu_uart2_available())
  {
    ipc_uart2_rx_last_byte = mcu_millis();
    ipc_uart2_feed(mcu_uart2_getc());
  }

  if (!ipc_uart2_rx_ready && (ipc_uart2_rx_len > 0) &&
      ((uint32_t)(mcu_millis() - ipc_uart2_rx_last_byte) >= IPC_UART2_RESYNC_TIMEOUT))
  {
    ipc_uart2_reset();
  }
}

/**
 * Sends one message as a frame.
 *
 * The payload is streamed straight out of the caller's buffer, so nothing here
 * depends on how large a payload is; the header and the trailer are the only
 * bytes this function has to hold, and they are smaller than a frame. The
 * length is clamped to what this side's frame layout can describe, and the CRC
 * is computed over the clamped length so that the two always agree.
 * */
void ipc_send(ipc_packet_type_t type, const void *data, size_t len)
{
  uint8_t frame[IPC_UART2_HDR_SIZE + IPC_UART2_TAIL_SIZE];
  uint16_t payload = (uint16_t)MIN(len, (size_t)IPC_MAX_PAYLOAD);
  uint16_t crc;
  size_t i;

  frame[0] = IPC_UART2_START;
  frame[1] = (uint8_t)type;
  frame[2] = (uint8_t)(payload & 0xFF);
  frame[3] = (uint8_t)(payload >> 8);

  crc = ipc_uart2_crc16(IPC_UART2_CRC_INIT, &frame[IPC_UART2_OFF_TYPE], (size_t)(IPC_UART2_OFF_DATA - 1u));
  if (payload)
  {
    crc = ipc_uart2_crc16(crc, data, (size_t)payload);
  }

  frame[IPC_UART2_HDR_SIZE] = (uint8_t)(crc & 0xFF);
  frame[IPC_UART2_HDR_SIZE + 1] = (uint8_t)(crc >> 8);
  frame[IPC_UART2_HDR_SIZE + 2] = IPC_UART2_END;

  for (i = 0; i < IPC_UART2_HDR_SIZE; i++)
  {
    mcu_uart2_putc(frame[i]);
  }

  for (i = 0; i < (size_t)payload; i++)
  {
    mcu_uart2_putc(((const uint8_t *)data)[i]);
  }

  for (i = IPC_UART2_HDR_SIZE; i < (IPC_UART2_HDR_SIZE + IPC_UART2_TAIL_SIZE); i++)
  {
    mcu_uart2_putc(frame[i]);
  }

  // hands the frame over, so the caller only waits for the reply once all of it
  // is out
  mcu_uart2_flush();
}

/**
 * Waits for the reply to a request, and gives up after IPC_ACK_TIMEOUT.
 *
 * The reply carries the type of the request, which is the ACK: a frame whose
 * type does not match is a stale reply, so it is dropped and the wait goes on.
 * Only the side that forwards requests waits for one; the RT side only answers,
 * so this is empty there.
 * */
size_t ipc_receive(ipc_packet_type_t type, void *data, size_t len)
{
#if !ASSERT_IPC_NONRT()
  (void)type;
  (void)data;
  (void)len;
  return 0;
#else
  uint16_t tries = 0;
  uint32_t deadline = mcu_millis();
  uint32_t stuck = 0;

  while (tries < IPC_MAX_TRIES)
  {
    ipc_uart2_drain();

    if (ipc_uart2_rx_ready)
    {
      if (ipc_uart2_rx_frame[IPC_UART2_OFF_TYPE] == (uint8_t)type)
      {
        uint16_t payload = ipc_uart2_frame_len();
        uint16_t copied = (uint16_t)MIN((size_t)payload, len);

        if (data && copied)
        {
          memcpy(data, &ipc_uart2_rx_frame[IPC_UART2_OFF_DATA], (size_t)copied);
        }

        ipc_uart2_reset();

        // the ACK byte, plus the payload bytes that fit
        return (size_t)(1 + copied);
      }

      // a reply to something else. It is dropped and the wait retries
      ipc_uart2_reset();
      tries++;
      deadline = mcu_millis();
      stuck = 0;
      continue;
    }

    if ((uint32_t)(mcu_millis() - deadline) >= IPC_ACK_TIMEOUT)
    {
      tries++;
      deadline = mcu_millis();
      stuck = 0;
      continue;
    }

    /**
     * Bound for a build whose clock cannot move on its own (see
     * IPC_UART2_RX_WAIT_LIMIT). A clock that advances at all - which is every
     * build that is not the host unit test one - clears this and leaves the
     * timeout above in charge.
     * */
    if (mcu_millis() != deadline)
    {
      stuck = 0;
    }
    else if (++stuck >= IPC_UART2_RX_WAIT_LIMIT)
    {
      break;
    }
  }

  return 0;
#endif
}

/**
 * Makes a whole request available to ipc_dotasks() when one has arrived, in the
 * layout it reads. Only the RT side receives requests, so this does nothing on
 * the side that forwards them.
 * */
void ipc_poll(void)
{
#if !ASSERT_IPC_RT()
  return;
#else
  if (g_ipc_data_available)
  {
    return;
  }

  ipc_uart2_drain();

  if (!ipc_uart2_rx_ready)
  {
    return;
  }

  ipc_packet_t *packet = (ipc_packet_t *)g_ipc_buffer;
  uint16_t payload = ipc_uart2_frame_len();

  packet->type = (ipc_packet_type_t)ipc_uart2_rx_frame[IPC_UART2_OFF_TYPE];
  packet->len = payload;
  packet->payload = g_ipc_buffer + sizeof(ipc_packet_t);

  if (payload)
  {
    memcpy(packet->payload, &ipc_uart2_rx_frame[IPC_UART2_OFF_DATA], (size_t)payload);
  }

  ipc_uart2_reset();
  g_ipc_data_available = true;
#endif
}

#endif /* IPC_USES_UART2 && MCU_HAS_UART2 */
