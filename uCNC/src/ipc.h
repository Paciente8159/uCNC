/*
        Name: ipc.h
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

#ifndef IPC_H
#define IPC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

  /**
   * Inter-process comunication (IPC)
   */

  /**
   * IPC mode flags
   * Examples:
   * AVR - IPC_SINGLE_CORE
   * ESP32 - IPC_MULTICORE | IPC_SHARED_MEM
   * RP2040/2350 - IPC_MULTICORE | IPC_SHARED_MEM or just IPC_MULTICORE where pico/util/queue.h can be used for inter core communication instead of SPI
   * ESP8266 - IPC_MULTICORE_NONRT + AVR - IPC_MULTICORE_RT where the ESP8266 runs non RT code only and the AVR runs RT code only
   *
   */
#define IPC_SINGLE_CORE 0                                      // single core
#define IPC_MULTICORE_NONRT 1                                  // multicore (Non RT mode only)
#define IPC_MULTICORE_RT 2                                     // multicore (RT mode only)
#define IPC_MULTICORE (IPC_MULTICORE_NONRT | IPC_MULTICORE_RT) // multicore (both modes)
#define IPC_SHARED_MEM 128                                     // multicore (shared memory mode) (still deciding on what to do with this mode)

#ifndef IPC_MODE
#define IPC_MODE IPC_SINGLE_CORE
#endif
#ifndef IPC_NON_RT_CORE
#define IPC_NON_RT_CORE 0
#endif
#ifndef IPC_RT_CORE
#define IPC_RT_CORE 0
#endif

/**
 * Which side of the split a block of code belongs to.
 *
 * There are four questions here, and they are not the same one.
 *
 * ASSERT_IPC_HAS_NONRT() asks whether this build contains the NonRT side at all.
 * Per side setup code needs that answer: the NonRT side owns the streams, the
 * settings and the network stack and has to initialize them whenever it is
 * present, no matter how it reaches the RT side.
 *
 * ASSERT_IPC_NONRT() asks whether the NonRT side has to hand a command to the RT
 * side over the transport. It guards the forwarding path. It is narrower than
 * the question above, because when the two sides reach each other directly - the
 * single core, or IPC_SHARED_MEM - the NonRT side is present but has nothing to
 * forward.
 *
 * ASSERT_IPC_RT() asks whether the real body of a split function runs here. That
 * body runs locally, on the core that owns the state: the RT side, the single
 * core, or a shared memory build. There is no separate "contains the RT side"
 * macro because this already is that test.
 *
 * ASSERT_IPC_MULTICORE() asks whether both sides are in this image, so that the
 * core the code runs on is what tells them apart. Only then are the two core ids
 * different values and only then is the runtime check worth making. The answer
 * is a compile time constant, so a build with one side carries none of the
 * check, and the question is named once here instead of being respelled at every
 * seam.
 * */
#define ASSERT_IPC_HAS_NONRT() ((IPC_MODE == IPC_SINGLE_CORE) || (IPC_MODE & (IPC_MULTICORE_NONRT | IPC_SHARED_MEM)))
#define ASSERT_IPC_NONRT() ((IPC_MODE & IPC_MULTICORE_NONRT) && !(IPC_MODE & IPC_SHARED_MEM))
#define ASSERT_IPC_RT() ((IPC_MODE == IPC_SINGLE_CORE) || (IPC_MODE & (IPC_MULTICORE_RT | IPC_SHARED_MEM)))
#define ASSERT_IPC_MULTICORE() ((IPC_MODE & IPC_MULTICORE) == IPC_MULTICORE)

  /**
   * IPC request/reply matching
   *
   * Every message carries a sequence number and a reply status. A reply is
   * matched by both type and sequence, so a delayed reply to an older request
   * cannot be taken as the answer to a newer one. The RT side caches the last
   * reply; if the same sequence arrives again because the NonRT side retried,
   * the cached reply is sent back without executing the request twice.
   *
   * ipc_receive() waits one IPC_ACK_TIMEOUT for a matching reply. ipc_exec()
   * owns the retry policy: it resends the same sequence up to IPC_MAX_TRIES
   * times, so a lost request or reply is recovered without duplicating work.
   */
#ifndef IPC_MAX_TRIES
#define IPC_MAX_TRIES 8
#endif
#ifndef IPC_ACK_TIMEOUT
  /**
   * 50 ms is several times the time a 256 byte frame takes at 115200 baud, so a
   * live RT side always answers well inside it.
   * */
#define IPC_ACK_TIMEOUT 50 // ms
#endif

  // Reply status. Positive means the request was accepted and executed.
  // Negative values identify why it was not.
#define IPC_STATUS_OK 1
#define IPC_STATUS_ERROR -1
#define IPC_STATUS_TIMEOUT -2
#define IPC_STATUS_OVERSIZE -3
#define IPC_STATUS_IO -4
#define IPC_STATUS_BAD_REPLY -5
#define IPC_STATUS_BUSY -6
#define IPC_STATUS_INVALID -7

  /**
   * Startup handshake
   *
   * The NonRT side cannot do anything before the RT side is online, since every
   * command it issues is forwarded and waits for an ACK. So it pings the RT side
   * at startup and only gives up after this total time budget has elapsed. This
   * is deliberately not the ACK timeout above: that one bounds a single request
   * while the machine is running, while the RT board may take a long time to
   * boot.
   */
#ifndef IPC_BOOT_TIMEOUT
#define IPC_BOOT_TIMEOUT 5000 // ms
#endif

  /**
   * Size of the request and reply buffers. A transport frames a message as its
   * type, its length and that many payload bytes, so the payload a message can
   * carry is this minus the packet header.
   */
#ifndef IPC_BUFFER_SIZE
#define IPC_BUFFER_SIZE 256
#endif

  typedef enum ipc_packet_type_
  {
    // startup handshake. The NonRT side sends this and waits for the reply
    // carrying the echoed type (the ACK) to confirm the RT side is online and
    // servicing requests before forwarding anything else to it.
    IPC_PING = 0x5A,
    // Interpolator functions
    ITP_IS_FULL,
    ITP_PUSH_BLK,
    ITP_PUSH_SGM,
    ITP_STEP_RATE_CONVERT,
    ITP_START,
    ITP_CLEAR,
    ITP_STOP,
    ITP_GET_RT_POSITION,
    ITP_RESET_RT_POSITION,
    ITP_SYNC_RT_POSITION,
    ITP_GET_RT_FEED,
    // IO functions owned by the RT side (see io_control.c). The NonRT side
    // forwards these requests to the RT core, that executes them.
    IO_LOCK_LIMITS,
    IO_ENABLE_LIMITS,
    IO_DISABLE_LIMITS,
    IO_INVERT_LIMITS,
    IO_GET_RAW_LIMITS,
    IO_GET_LIMITS,
    IO_ENABLE_PROBE,
    IO_DISABLE_PROBE,
    IO_GET_PROBE,
    // position (in steps) recorded by the RT side when the probe triggered.
    // IO_GET_PROBE_POSITION reads it back, IO_SYNC_PROBE_POSITION makes the RT
    // side capture the current position into it.
    IO_GET_PROBE_POSITION,
    IO_SYNC_PROBE_POSITION,
    IO_SET_STEPS,
    IO_TOGGLE_STEPS,
    IO_SET_DIRS,
    IO_GET_STEPS_POS,
    IO_ENABLE_STEPPERS,
    // generic pin value access owned by the RT side. IO_SET_PINVALUE carries the
    // pin and the new value in its payload. IO_GET_PINVALUE carries the pin and
    // replies with the current value (see io_set_pinvalue and io_get_pinvalue).
    IO_SET_PINVALUE,
    IO_GET_PINVALUE,
    // cnc_state_rt.exec_state is owned by the RT side (see cnc.c). The NonRT side
    // forwards these requests to the RT core, that executes them.
    CNC_GET_EXEC_STATE,
    CNC_SET_EXEC_STATE_RT,
    CNC_CLEAR_EXEC_STATE_RT,
    // The tool output is owned by the RT side (see tool.c). The NonRT side
    // forwards these requests to the RT core, that executes them.
    TOOL_SET_SPEED,
    TOOL_SET_COOLANT,
    // The tool mode is owned by the NonRT side and mirrored to the RT side, so
    // the NonRT side pushes the new value to the RT core (see tool_set_mode).
    TOOL_SET_MODE,
    // The machine settings are owned by the NonRT side and mirrored to the RT
    // side, so the NonRT side pushes the whole settings block to the RT core
    // (see settings_sync_rt).
    SETTINGS_SYNC
  } ipc_packet_type_t;

  /**
   * The in-memory packet. It is never transferred as a whole: every transport
   * frames type/seq/status/len and the payload bytes itself (the byte layout in
   * ipc_serial.h, the pipe header in virtual_windows.c, the FIFO word packing in
   * mcu_rp2040.c), so the member order and the padding a compiler inserts do not
   * have to match on the two sides. Nothing serializes this struct.
   *
   * What does have to hold is that the payload region right after it stays
   * aligned: the handlers cast it to uint16_t, int32_t and float, and Cortex-M0+
   * takes a HardFault on an unaligned access. A packed layout broke that by
   * putting the payload at offset 11. Without packed, sizeof() is always a
   * multiple of the struct alignment, so the payload lands aligned; the offset
   * below makes that explicit instead of leaving it to the compiler's padding.
   * */
  typedef struct ipc_packet_
  {
    ipc_packet_type_t type;
    uint8_t seq;
    int8_t status;
    size_t len;
    uint8_t *payload;
  } ipc_packet_t;

  /**
   * Where the payload starts inside the transport's buffer, rounded up so that
   * it is aligned whatever the compiler made of the header above (the enum, the
   * size_t and the pointer are all target dependent). Every transport uses this
   * instead of sizeof(ipc_packet_t) so the alignment does not rest on how a
   * given compiler padded the struct.
   * */
#define IPC_PAYLOAD_OFFSET (((sizeof(ipc_packet_t) + 7u) & ~(size_t)7u))
  typedef char ipc_packet_payload_aligned[(IPC_PAYLOAD_OFFSET % 8) == 0 ? 1 : -1];

  /**
   * The header is at most this wide on any target this runs on (8 on AVR, 16 on
   * the 32bit targets, 24 with 64bit pointers). Reserving the same for all of
   * them keeps the capacity a constant of the protocol instead of a value that
   * moves with the pointer width, which is what lets two builds made by
   * different compilers agree on how large a message may be.
   * */
#define IPC_PACKET_HEADER_MAX 24
  typedef char ipc_packet_header_fits[(IPC_PAYLOAD_OFFSET <= IPC_PACKET_HEADER_MAX) ? 1 : -1];

  // the payload a message can carry: the buffer minus the reserved header
#define IPC_MAX_PAYLOAD (IPC_BUFFER_SIZE - IPC_PACKET_HEADER_MAX)

  // the request buffer ipc_dotasks() reads. The transport fills it and sets
  // g_ipc_data_available when a request is pending. Aligned so that the payload
  // right after the header is aligned too: the handlers cast it to uint16_t,
  // int32_t and float, and Cortex-M0+ faults on those when unaligned.
  extern uint8_t g_ipc_buffer[IPC_BUFFER_SIZE] __attribute__((aligned(8)));
  extern bool g_ipc_data_available;

  // confirms the RT side is online and answering before the NonRT side starts
  // forwarding commands to it. Blocks up to IPC_BOOT_TIMEOUT ms.
  // Returns true when the RT side acknowledged the ping, or when no handshake is
  // needed: single core and shared memory builds, and the RT side itself.
  bool ipc_init(void);
  // Sends one complete packet. Returns IPC_STATUS_OK on success, or a negative
  // status if the payload does not fit or the transport failed.
  int8_t ipc_send(const ipc_packet_t *packet);
  // Waits for the reply that matches packet->type and packet->seq, copies up to
  // packet->len payload bytes into packet->payload, and returns the reply status.
  // Returns IPC_STATUS_TIMEOUT if no matching reply arrives in IPC_ACK_TIMEOUT,
  // or IPC_STATUS_BAD_REPLY if a frame is malformed: a declared payload longer
  // than IPC_MAX_PAYLOAD, or a frame that stops arriving part way through. A
  // transport that can drop a bad frame and resynchronize the stream (the byte
  // stream) keeps waiting instead, so there it surfaces as a timeout.
  int8_t ipc_receive(ipc_packet_t *packet);
  // overridable. Fetches one pending request from the transport into
  // g_ipc_buffer and sets g_ipc_data_available. The default does nothing, which
  // is what a build without a transport needs, and what the NonRT side needs:
  // it never receives requests, it waits for the reply inside ipc_receive.
  // Called by ipc_dotasks before it looks at g_ipc_data_available.
  void ipc_poll(void);
  // Makes a request and waits for the reply. Returns the reply status:
  // IPC_STATUS_OK when the RT side accepted and executed it, a negative error
  // code otherwise.
  int8_t ipc_exec(ipc_packet_type_t type, const void *data_out, size_t len_out,
                  void *data_in, size_t len_in);
  void ipc_dotasks(void); // run IPC tasks (runs on the RT side to process requests)
  // true after a request timed out or the transport failed. The NonRT loop
  // turns this into a safe alarm instead of silently using stale state.
  bool ipc_has_fault(void);
  void ipc_clear_fault(void);
  // gets the current core ID. An MCU that has real cores provides its own and
  // defines IPC_CURRENT_CORE_OVERRIDE; every other build gets the default in
  // ipc.c.
  //
  // The override is a build flag rather than a weak symbol on purpose. With
  // MinGW/PE a strong reference does not resolve against a weak definition, and
  // marking this declaration weak as well leaves two weak definitions for the
  // linker to choose between, so the override silently stops working.
  uint8_t ipc_get_current_core(void);

#ifdef __cplusplus
}
#endif

#endif
