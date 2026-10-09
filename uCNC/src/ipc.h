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
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Inter-process comunication (IPC)
 */

#define IPC_MONOLITHIC 0
#define IPC_SHARED_MEM 1
#define IPC_ORCHESTRATOR 2
#define IPC_EXECUTER 3

#ifndef IPC_MODE
#define IPC_MODE IPC_MONOLITHIC
#endif

typedef enum ipc_packet_type_ {
  ITP_SGM_IS_FULL = 0,
  ITP_BLK_WRITE,
  ITP_SGM_WRITE,
  ITP_STEP_RATE,
  ITP_START
} ipc_packet_type_t;

typedef struct ipc_packet_ {
  ipc_packet_type_t type;
  uint8_t *payload;
  size_t len;
} ipc_packet_t;

void ipc_init(void);
void ipc_send(ipc_packet_type_t type, const void *data, size_t len);
size_t ipc_receive(ipc_packet_type_t type, void *data, size_t len);
void ipc_exec(ipc_packet_type_t type, const void *data_out, size_t len_out,
              void *data_in, size_t len_in);
void ipc_dotasks(void);

#ifdef __cplusplus
}
#endif

#endif
