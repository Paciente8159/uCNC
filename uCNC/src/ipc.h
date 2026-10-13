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
   * IPC ACK handling
   *
   * The RT side always replies to a request and the reply starts with the
   * echoed request packet type (this single byte is the ACK). ipc_receive()
   * waits for that byte: while the received byte does not match the expected
   * type it retries (up to IPC_MAX_TRIES attempts) and it aborts the wait after
   * IPC_ACK_TIMEOUT milliseconds.
   */
#ifndef IPC_MAX_TRIES
#define IPC_MAX_TRIES 8
#endif
#ifndef IPC_ACK_TIMEOUT
#define IPC_ACK_TIMEOUT 50 // ms
#endif

  typedef enum ipc_packet_type_
  {
    ITP_IS_FULL = 0,
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
    IO_SET_STEPS,
    IO_TOGGLE_STEPS,
    IO_SET_DIRS,
    IO_GET_STEPS_POS,
    IO_ENABLE_STEPPERS
  } ipc_packet_type_t;

  typedef struct ipc_packet_
  {
    ipc_packet_type_t type;
    size_t len;
    uint8_t *payload;
  } __attribute__((packed, aligned(1))) ipc_packet_t;

  void ipc_init(void);
  void ipc_send(ipc_packet_type_t type, const void *data, size_t len); // overridable request send (used by ipc_exec)
  // overridable ACK/response receive (used by ipc_exec). Waits for the reply whose
  // first byte equals type (the ACK), retrying on any other byte up to
  // IPC_MAX_TRIES and giving up after IPC_ACK_TIMEOUT ms. On a matching ACK it
  // copies up to len payload bytes into data (data may be NULL for a bare ACK).
  // Returns the number of bytes received including the ACK byte (>= 1) or 0 if no
  // ACK was received.
  size_t ipc_receive(ipc_packet_type_t type, void *data, size_t len);
  void ipc_exec(ipc_packet_type_t type, const void *data_out, size_t len_out,
                void *data_in, size_t len_in); // makes an ipc execution request (NonRT side to RT side) and waits for the ACK
  void ipc_dotasks(void);                      // run IPC tasks (runs on the RT side to process requests)
  // gets the current core ID. Overridable: the default implementation in ipc.c
  // returns IPC_RT_CORE. The declaration is weak too, because a weak definition
  // only satisfies a weak reference on some toolchains (MinGW/PE); on the real
  // targets either form works.
  uint8_t __attribute__((weak)) ipc_get_current_core(void);

#ifdef __cplusplus
}
#endif

#endif
