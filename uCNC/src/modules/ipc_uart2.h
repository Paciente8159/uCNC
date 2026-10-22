/*
        Name: ipc_uart2.h
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

#ifndef IPC_UART2_H
#define IPC_UART2_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

     /**
      * IPC transport over UART2.
      *
      * Carries the requests the NonRT side forwards and the replies the RT side
      * returns (see ipc.h) over a plain UART. The port is taken whole: defining
      * IPC_USES_UART2 also detaches UART2 from the main protocol, because the
      * gcode stream would otherwise register the same port as another stream and
      * consume the bytes the framing below depends on.
      *
      * Size it as a bus and not as a console. The two sides of the split are
      * usually separate boards, so the link carries one request and one reply per
      * forwarded call, and a payload can be as large as the settings block.
      *
      * THE FRAME
      *
      * A UART hands over a byte stream with no message boundaries, and the link
      * runs next to the power wiring of a machine, so the frame has to rebuild
      * the boundaries and reject whatever the line corrupted. Every message -
      * request and reply alike - travels as:
      *
      *   +-------+------+--------+--------+---------------+--------+--------+-----+
      *   | START | TYPE | LEN_LO | LEN_HI |    PAYLOAD    | CRC_LO | CRC_HI | END |
      *   +-------+------+--------+--------+---------------+--------+--------+-----+
      *       1      1       1        1          LEN           1        1      1
      *
      * START (0xAA) is the byte the receiver hunts for while it has no frame in
      * progress. It is not counted in LEN, and it is what lets the receiver lock
      * onto a frame again after the stream has been lost.
      *
      * TYPE is the ipc_packet_type_t of the message, and it doubles as the ACK:
      * the reply repeats the type of the request.
      *
      * LEN is the payload length, little endian, so that the same frame layout
      * describes every message. It is two bytes rather than one so that the field
      * cannot silently wrap if IPC_BUFFER_SIZE is ever raised.
      *
      * CRC is a CRC16-CCITT (polynomial 0x1021, seeded with 0xFFFF) computed over
      * TYPE, LEN and PAYLOAD. It is what turns a corrupted payload into a dropped
      * frame instead of a command the RT side would carry out.
      *
      * END (0x55) closes the frame. With START it also catches the one case the
      * CRC cannot see by itself: a frame whose tail never arrived, because the
      * receiver is then still waiting for bytes that the next frame supplies.
      * A frame that has stopped arriving is dropped after
      * IPC_UART2_RESYNC_TIMEOUT, so a lost frame cannot swallow the one after it.
      * */

#define IPC_UART2_START 0xAAu
#define IPC_UART2_END 0x55u

// START, TYPE, LEN_LO, LEN_HI
#define IPC_UART2_HDR_SIZE 4u
// CRC_LO, CRC_HI, END
#define IPC_UART2_TAIL_SIZE 3u
#define IPC_UART2_OVERHEAD (IPC_UART2_HDR_SIZE + IPC_UART2_TAIL_SIZE)

// field offsets inside a frame
#define IPC_UART2_OFF_TYPE 1u
#define IPC_UART2_OFF_LEN 2u
#define IPC_UART2_OFF_DATA IPC_UART2_HDR_SIZE

// CRC16-CCITT, as required by the frame layout above
#define IPC_UART2_CRC_INIT 0xFFFFu
#define IPC_UART2_CRC_POLY 0x1021u

// the largest frame either side has to hold. A payload over IPC_MAX_PAYLOAD is
// rejected as soon as its length is known, so the assembler never has to be
// larger than this even if the peer is built with a wider buffer.
#define IPC_UART2_FRAME_MAX (IPC_MAX_PAYLOAD + IPC_UART2_OVERHEAD)

/**
 * How long a frame in progress may stay without a byte before it is dropped.
 * A sender emits a whole frame back to back, so a gap this long means the
 * frame was lost. It only has to be longer than the time between two bytes of
 * a frame at the slowest baud rate the link is used with; at 115200 baud a
 * byte takes under 90us, and a 256 byte frame takes under 23ms in total.
 * */
#ifndef IPC_UART2_RESYNC_TIMEOUT
#define IPC_UART2_RESYNC_TIMEOUT 5 // ms
#endif

/**
 * Bound on the iterations the reply wait may run without the clock moving.
 *
 * The wait is bounded by IPC_ACK_TIMEOUT, which is what a real board and the
 * host emulator both honour because both have a clock of their own. A host
 * build under PIO_UNIT_TESTING does not: its clock only advances from
 * cnc_dotasks(), which this wait does not run. Without this bound a peer that
 * never answers would spin there forever rather than timing out.
 * */
#ifndef IPC_UART2_RX_WAIT_LIMIT
#define IPC_UART2_RX_WAIT_LIMIT 1000000u
#endif

#ifdef __cplusplus
}
#endif

#endif
