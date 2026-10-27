/*
        Name: ipc_serial.c
        Description: IPC transport over SERIAL for µCNC.

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
 * Asking for this transport without a SERIAL to run it on would leave the weak
 * no-op transport in ipc.c in place, so every forwarded request would silently
 * go nowhere and ipc_init() would just take IPC_BOOT_TIMEOUT to fail. Trap it
 * at build time instead.
 * */
#if defined(IPC_USES_SERIAL) && !defined(IPC_SERIAL_PORT)
#error "IPC_USES_SERIAL needs a MCU serial port to be assigned"
#endif

#if defined(IPC_USES_SERIAL) && defined(IPC_SERIAL_PORT)

#include "ipc_serial.h"

#if (IPC_SERIAL_PORT == IPC_SERIAL_UART)
#define ipc_serial_available mcu_uart_available
#define ipc_serial_getc mcu_uart_getc
#define ipc_serial_putc mcu_uart_putc
#define ipc_serial_flush mcu_uart_flush
#ifndef MCU_HAS_UART
#error "IPC_SERIAL_UART requires UART port to be defined"
#endif
#ifndef DETACH_UART_FROM_MAIN_PROTOCOL
#error "IPC_SERIAL_UART requires DETACH_UART_FROM_MAIN_PROTOCOL"
#endif
#elif (IPC_SERIAL_PORT == IPC_SERIAL_UART2)
#define ipc_serial_available mcu_uart2_available
#define ipc_serial_getc mcu_uart2_getc
#define ipc_serial_putc mcu_uart2_putc
#define ipc_serial_flush mcu_uart2_flush
#ifndef MCU_HAS_UART2
#error "IPC_SERIAL_UART2 requires UART2 port to be defined"
#endif
#ifndef DETACH_UART2_FROM_MAIN_PROTOCOL
#error "IPC_SERIAL_UART2 requires DETACH_UART2_FROM_MAIN_PROTOCOL"
#endif
#elif (IPC_SERIAL_PORT == IPC_SERIAL_USB)
#define ipc_serial_available mcu_usb_available
#define ipc_serial_getc mcu_usb_getc
#define ipc_serial_putc mcu_usb_putc
#define ipc_serial_flush mcu_usb_flush
#ifndef MCU_HAS_USB
#error "IPC_SERIAL_USB requires USB port to be defined"
#endif
#ifndef DETACH_USB_FROM_MAIN_PROTOCOL
#error "IPC_SERIAL_USB requires DETACH_USB_FROM_MAIN_PROTOCOL"
#endif
#else
#error "Invalid IPC_SERIAL_PORT value"
#endif

/**
 * Debug trace for the SERIAL IPC transport. A build that wants the trace defines
 * IPCSERIALDBG(...) as a printf-like sink before this header is reached; without
 * it the calls in ipc_serial.c compile away. See the SETTINGSDBG pattern in
 * grbl_settings.c.
 * */
#if defined(IPC_SERIAL_DEBUG) && !defined(IPCSERIALDBG)
#include <stdio.h>
#define IPCSERIALDBG(...)      \
  do                           \
  {                            \
    printf(__VA_ARGS__);       \
    printf("\n");              \
    fflush(stdout);            \
  } while (0)
#endif
#ifndef IPCSERIALDBG
#define IPCSERIALDBG(...) ((void)0)
#endif

/**
 * The frame being rebuilt out of the byte stream, and how far it got.
 *
 * The HAL already has a receive ring, but the ring is as large as one gcode
 * line (RX_BUFFER_SIZE) and a frame can be larger than that, so the frame is
 * assembled here instead: the ring only has to cover the time between two
 * calls to ipc_serial_drain(), and this buffer has to cover a whole message.
 * */
static uint8_t ipc_serial_rx_frame[IPC_SERIAL_FRAME_MAX];
static uint16_t ipc_serial_rx_len;   // bytes of the frame stored so far
static uint16_t ipc_serial_rx_total; // whole frame length, 0 until LEN is in
static bool ipc_serial_rx_ready;     // a checked frame is waiting to be handed over
static uint32_t ipc_serial_rx_last_byte;

/**
 * CRC16-CCITT over a block of bytes, in the shape the frame layout asks for
 * (see ipc_serial.h). Feeding it the frame header first and the payload after
 * gives the same value as one pass over both, so a sender that streams the
 * payload never has to hold the whole frame.
 * */
static uint16_t ipc_serial_crc16(uint16_t crc, const void *data, size_t len)
{
  const uint8_t *bytes = (const uint8_t *)data;

  while (len--)
  {
    crc ^= (uint16_t)((uint16_t)(*bytes++) << 8);

    for (uint8_t bit = 0; bit < 8; bit++)
    {
      crc = (uint16_t)((crc & 0x8000u) ? ((crc << 1) ^ IPC_SERIAL_CRC_POLY) : ((uint16_t)(crc << 1)));
    }
  }

  return crc;
}

// the payload length a frame in the assembler declares
static uint16_t ipc_serial_frame_len(void)
{
  return (uint16_t)((uint16_t)ipc_serial_rx_frame[IPC_SERIAL_OFF_LEN] |
                    ((uint16_t)ipc_serial_rx_frame[IPC_SERIAL_OFF_LEN + 1] << 8));
}

// drops whatever the assembler holds, so the next byte starts a new frame
static void ipc_serial_reset(void)
{
  ipc_serial_rx_len = 0;
  ipc_serial_rx_total = 0;
  ipc_serial_rx_ready = false;
}

/**
 * Checks a whole frame that is in the assembler and keeps it if it is sound.
 * This is where a corrupted message dies: the trailer has to close the frame
 * and the CRC has to match what the frame carries, or the frame is dropped and
 * never reaches the caller.
 * */
static void ipc_serial_check(void)
{
  uint16_t payload = ipc_serial_frame_len();
  uint16_t total = ipc_serial_rx_total;
  // TYPE, LEN and PAYLOAD, which is everything between START and the CRC
  uint16_t crc = ipc_serial_crc16(IPC_SERIAL_CRC_INIT, &ipc_serial_rx_frame[IPC_SERIAL_OFF_TYPE],
                                  (size_t)((IPC_SERIAL_OFF_DATA - 1u) + payload));
  uint16_t frame_crc = (uint16_t)((uint16_t)ipc_serial_rx_frame[total - 3] |
                                  ((uint16_t)ipc_serial_rx_frame[total - 2] << 8));

  if ((ipc_serial_rx_frame[total - 1] != IPC_SERIAL_END) || (crc != frame_crc))
  {
    IPCSERIALDBG("ipc_serial: reject frame type %u len %u (end 0x%02X, crc 0x%04X want 0x%04X)",
                 (unsigned)ipc_serial_rx_frame[IPC_SERIAL_OFF_TYPE], (unsigned)payload,
                 (unsigned)ipc_serial_rx_frame[total - 1], (unsigned)crc, (unsigned)frame_crc);
    ipc_serial_reset();
    return;
  }

  IPCSERIALDBG("ipc_serial: frame ok type %u len %u",
               (unsigned)ipc_serial_rx_frame[IPC_SERIAL_OFF_TYPE], (unsigned)payload);
  ipc_serial_rx_ready = true;
}

// takes one byte of the stream into the frame being rebuilt
static void ipc_serial_feed(uint8_t c)
{
  if (ipc_serial_rx_ready)
  {
    /**
     * A checked frame is still waiting to be handed over, so there is nowhere
     * for this byte to go. Dropping it with the frame is what keeps the two
     * sides in step: the caller reads a whole message before another one is
     * sent, so anything arriving here belongs to a frame that has nowhere to
     * be kept.
     * */
    IPCSERIALDBG("ipc_serial: byte 0x%02X dropped, frame waiting", (unsigned)c);
    return;
  }

  if (ipc_serial_rx_len == 0)
  {
    // hunting for the start. Everything before it is line noise or the tail of
    // a frame that was already lost
    if (c != IPC_SERIAL_START)
    {
      return;
    }

    ipc_serial_rx_frame[0] = c;
    ipc_serial_rx_len = 1;
    ipc_serial_rx_total = 0;
    IPCSERIALDBG("ipc_serial: start byte, hunting frame");
    return;
  }

  if (ipc_serial_rx_len >= IPC_SERIAL_FRAME_MAX)
  {
    /**
     * Unreachable while LEN is checked as soon as it is known, which is what
     * the block below does. Kept as a floor so that a frame can never be built
     * past the end of the buffer.
     * */
    IPCSERIALDBG("ipc_serial: frame over %u bytes, assembler overflow", (unsigned)IPC_SERIAL_FRAME_MAX);
    ipc_serial_reset();
    return;
  }

  ipc_serial_rx_frame[ipc_serial_rx_len++] = c;

  if (ipc_serial_rx_len == IPC_SERIAL_OFF_DATA)
  {
    /**
     * The header is complete, so the length of the whole frame is known and can
     * be refused before another byte of it is stored. A peer built with a wider
     * buffer than this one would otherwise run the assembler past its end.
     * */
    uint16_t payload = ipc_serial_frame_len();

    if (payload > IPC_MAX_PAYLOAD)
    {
      IPCSERIALDBG("ipc_serial: payload %u over limit %u", (unsigned)payload, (unsigned)IPC_MAX_PAYLOAD);
      ipc_serial_reset();
      return;
    }

    IPCSERIALDBG("ipc_serial: header type %u payload %u", (unsigned)ipc_serial_rx_frame[IPC_SERIAL_OFF_TYPE], (unsigned)payload);
    ipc_serial_rx_total = (uint16_t)(payload + IPC_SERIAL_OVERHEAD);
  }

  if ((ipc_serial_rx_total != 0) && (ipc_serial_rx_len == ipc_serial_rx_total))
  {
    ipc_serial_check();
  }
}

/**
 * Moves everything the HAL has buffered into the assembler, and drops a frame
 * that has stopped arriving.
 *
 * The resync is the important part. A frame the line ate leaves the assembler
 * waiting for bytes that never come, and the next frame then supplies them: the
 * wait would swallow that frame whole before the byte count came out and the
 * check failed. A gap longer than IPC_SERIAL_RESYNC_TIMEOUT cannot happen inside
 * a frame - the sender emits one back to back - so it means the frame is gone
 * and the assembler has to start over.
 * */
static void ipc_serial_drain(void)
{
  while (ipc_serial_available())
  {
    ipc_serial_rx_last_byte = mcu_millis();
    ipc_serial_feed(ipc_serial_getc());
  }

  if (!ipc_serial_rx_ready && (ipc_serial_rx_len > 0) &&
      ((uint32_t)(mcu_millis() - ipc_serial_rx_last_byte) >= IPC_SERIAL_RESYNC_TIMEOUT))
  {
    IPCSERIALDBG("ipc_serial: no byte for %u ms, frame lost, resync", (unsigned)IPC_SERIAL_RESYNC_TIMEOUT);
    ipc_serial_reset();
  }
}

/**
 * Sends one packet as a frame.
 *
 * The payload is streamed straight out of the caller's buffer, so nothing here
 * depends on how large a payload is; the header and the trailer are the only
 * bytes this function has to hold, and they are smaller than a frame. An
 * oversized payload is rejected instead of being truncated, so a partial
 * settings or motion block can never look like a valid frame.
 * */
int8_t ipc_send(const ipc_packet_t *packet)
{
  uint8_t frame[IPC_SERIAL_HDR_SIZE + IPC_SERIAL_TAIL_SIZE];
  uint16_t payload;
  uint16_t crc;
  size_t i;

  if (!packet || (packet->len > IPC_MAX_PAYLOAD))
  {
    return IPC_STATUS_OVERSIZE;
  }

  payload = (uint16_t)packet->len;

  IPCSERIALDBG("ipc_serial: send type %u seq %u status %d len %u",
               (unsigned)packet->type, (unsigned)packet->seq,
               (int)packet->status, (unsigned)payload);

  frame[0] = IPC_SERIAL_START;
  frame[1] = (uint8_t)packet->type;
  frame[2] = packet->seq;
  frame[3] = (uint8_t)packet->status;
  frame[4] = (uint8_t)(payload & 0xFF);
  frame[5] = (uint8_t)(payload >> 8);

  crc = ipc_serial_crc16(IPC_SERIAL_CRC_INIT, &frame[IPC_SERIAL_OFF_TYPE], (size_t)(IPC_SERIAL_OFF_DATA - 1u));
  if (payload)
  {
    crc = ipc_serial_crc16(crc, packet->payload, (size_t)payload);
  }

  frame[IPC_SERIAL_HDR_SIZE] = (uint8_t)(crc & 0xFF);
  frame[IPC_SERIAL_HDR_SIZE + 1] = (uint8_t)(crc >> 8);
  frame[IPC_SERIAL_HDR_SIZE + 2] = IPC_SERIAL_END;

  for (i = 0; i < IPC_SERIAL_HDR_SIZE; i++)
  {
    ipc_serial_putc(frame[i]);
  }

  for (i = 0; i < (size_t)payload; i++)
  {
    ipc_serial_putc(((const uint8_t *)packet->payload)[i]);
  }

  for (i = IPC_SERIAL_HDR_SIZE; i < (IPC_SERIAL_HDR_SIZE + IPC_SERIAL_TAIL_SIZE); i++)
  {
    ipc_serial_putc(frame[i]);
  }

  // hands the frame over, so the caller only waits for the reply once all of it
  // is out
  ipc_serial_flush();
  return IPC_STATUS_OK;
}

/**
 * Waits for the reply to a request, and gives up after IPC_ACK_TIMEOUT.
 *
 * The reply is matched by both type and sequence. A frame whose type or
 * sequence does not match is stale and is dropped, so a delayed reply to an
 * older request cannot be mistaken for this one. Only the side that forwards
 * requests waits for one; the RT side only answers, so this is empty there.
 * */
int8_t ipc_receive(ipc_packet_t *packet)
{
#if !ASSERT_IPC_NONRT()
  (void)packet;
  return IPC_STATUS_OK;
#else
  uint32_t deadline = mcu_millis();
#ifdef PIO_UNIT_TESTING
  // only the build whose clock cannot move on its own needs this (see below)
  uint32_t stuck = 0;
#endif

  for (;;)
  {
    ipc_serial_drain();

    if (ipc_serial_rx_ready)
    {
      if ((ipc_serial_rx_frame[IPC_SERIAL_OFF_TYPE] == (uint8_t)packet->type) &&
          (ipc_serial_rx_frame[IPC_SERIAL_OFF_SEQ] == packet->seq))
      {
        uint16_t payload = ipc_serial_frame_len();
        uint16_t copied = (uint16_t)MIN((size_t)payload, packet->len);

        if (packet->payload && copied)
        {
          memcpy(packet->payload, &ipc_serial_rx_frame[IPC_SERIAL_OFF_DATA], (size_t)copied);
        }

        packet->status = (int8_t)ipc_serial_rx_frame[IPC_SERIAL_OFF_STATUS];
        ipc_serial_reset();

        IPCSERIALDBG("ipc_serial: reply type %u seq %u status %d len %u",
                     (unsigned)packet->type, (unsigned)packet->seq,
                     (int)packet->status, (unsigned)payload);

        return packet->status;
      }

      // a reply to something else. It is dropped and the wait goes on
      IPCSERIALDBG("ipc_serial: unexpected reply type %u seq %u (want %u/%u)",
                   (unsigned)ipc_serial_rx_frame[IPC_SERIAL_OFF_TYPE],
                   (unsigned)ipc_serial_rx_frame[IPC_SERIAL_OFF_SEQ],
                   (unsigned)packet->type, (unsigned)packet->seq);
      ipc_serial_reset();
#ifdef PIO_UNIT_TESTING
      stuck = 0;
#endif
      continue;
    }

    if ((uint32_t)(mcu_millis() - deadline) >= IPC_ACK_TIMEOUT)
    {
      IPCSERIALDBG("ipc_serial: reply timeout");
      return IPC_STATUS_TIMEOUT;
    }

#ifdef PIO_UNIT_TESTING
    /**
     * Bound for a build whose clock cannot move on its own (see
     * IPC_SERIAL_RX_WAIT_LIMIT). Only the host unit test build is that build: its
     * clock advances from cnc_dotasks(), which this wait deliberately does not
     * run, so the timeout above could never be reached there.
     *
     * Every other build has a clock of its own and must not be bounded by this.
     * A clock that advances is not one that advances on every pass: the emulator
     * moves its clock in a single step per EMULATION_MS_TICK, so a spin count
     * reaches any limit inside one step and the wait would give up on a reply
     * that had arrived but had not been read yet.
     * */
    if (mcu_millis() != deadline)
    {
      stuck = 0;
    }
    else if (++stuck >= IPC_SERIAL_RX_WAIT_LIMIT)
    {
      return IPC_STATUS_TIMEOUT;
    }
#endif
  }
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

  ipc_serial_drain();

  if (!ipc_serial_rx_ready)
  {
    return;
  }

  ipc_packet_t *packet = (ipc_packet_t *)g_ipc_buffer;
  uint16_t payload = ipc_serial_frame_len();

  packet->type = (ipc_packet_type_t)ipc_serial_rx_frame[IPC_SERIAL_OFF_TYPE];
  packet->seq = ipc_serial_rx_frame[IPC_SERIAL_OFF_SEQ];
  packet->status = (int8_t)ipc_serial_rx_frame[IPC_SERIAL_OFF_STATUS];
  packet->len = payload;
  packet->payload = g_ipc_buffer + IPC_PAYLOAD_OFFSET;

  if (payload)
  {
    memcpy(packet->payload, &ipc_serial_rx_frame[IPC_SERIAL_OFF_DATA], (size_t)payload);
  }

  IPCSERIALDBG("ipc_serial: request type %u seq %u len %u", (unsigned)packet->type,
               (unsigned)packet->seq, (unsigned)payload);
  ipc_serial_reset();
  g_ipc_data_available = true;
#endif
}

#endif /* IPC_USES_SERIAL */
