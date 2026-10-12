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
   * ESP32 - IPC_MULTI_CORE | IPC_SHARED_MEM
   * RP2040/2350 - IPC_MULTI_CORE | IPC_SHARED_MEM or just IPC_MULTI_CORE | IPC_NON_RT_CORE | IPC_RT_CORE where pico/util/queue.h can be used for inter core communication instead of SPI
   * ESP8266 - IPC_MULTI_CORE | IPC_NON_RT_CORE + AVR - IPC_MULTI_CORE | IPC_RT_CORE
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
    ITP_GET_RT_FEED
  } ipc_packet_type_t;

  typedef struct ipc_packet_
  {
    ipc_packet_type_t type;
    uint8_t *payload;
    size_t len;
  } ipc_packet_t;

  void ipc_init(void);
  void ipc_send(ipc_packet_type_t type, const void *data, size_t len); // overridable request send (used by ipc_exec)
  size_t ipc_receive(ipc_packet_type_t type, void *data, size_t len);  // overridable response receive (used by ipc_exec)
  void ipc_exec(ipc_packet_type_t type, const void *data_out, size_t len_out,
                void *data_in, size_t len_in); // makes an ipc execution request (NonRT side to RT side)
  void ipc_dotasks(void);                      // run IPC tasks (runs on the RT side to process requests)
  uint8_t ipc_get_current_core(void);          // gets the current core ID

#ifdef __cplusplus
}
#endif

#endif
