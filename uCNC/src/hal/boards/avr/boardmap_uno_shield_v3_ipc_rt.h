/*
	Name: boardmap_uno_shield_v3.h
	Description: Contains all MCU and PIN definitions for Arduino UNO similar to Grbl 1.1+ to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 05/01/2023

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_UNO_SHIELD_V3_IPC_RT_H
#define BOARDMAP_UNO_SHIELD_V3_IPC_RT_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MCU
#define MCU MCU_AVR
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "UNO SHIELD V3 IPC RT"
#endif

#include "boardmap_uno.h"

// swap limit z and replace pwm by spin enable pin
#undef LIMIT_Z_BIT
#undef LIMIT_Z_PORT
#undef LIMIT_Z_ISR
#undef PWM0_BIT
#undef PWM0_PORT
#undef PWM0_CHANNEL
#undef PWM0_TIMER
#undef ESTOP_BIT
#undef FHOLD_BIT
#undef CS_RES_BIT
#undef ESTOP_PORT
#undef FHOLD_PORT
#undef CS_RES_PORT
#undef ESTOP_ISR
#undef FHOLD_ISR
#undef CS_RES_ISR
#undef RX_BIT
#undef TX_BIT
#undef RX_PORT
#undef TX_PORT
#undef RX_PULLUP
#undef UART_PORT
#undef DISABLE_MULTISTREAM_SERIAL
#undef DISABLE_RTC_CODE
#undef DISABLE_SAFE_SETTINGS
#undef DISABLE_ITP_STEP_GEN_OPTIMIZATIONS

// Setup com pins
#define RX2_BIT 0
#define TX2_BIT 1
#define RX2_PORT D
#define TX2_PORT D
#define RX2_PULLUP
// only uncomment this if other port other then 0 is used
#define UART2_PORT 0
#define BAUDRATE2 1000000

// Grbl 0.8 limit z
#define LIMIT_Z_BIT 3	 // assigns LIMIT_Z pin
#define LIMIT_Z_PORT B // assigns LIMIT_Z port
#define LIMIT_Z_ISR 3	 // assigns LIMIT_Z ISR

// spindle en
#define DOUT1_BIT 4
#define DOUT1_PORT B

#define IPC_MODE IPC_MULTICORE_RT
#define IPC_USES_UART2

#ifdef __cplusplus
}
#endif

#endif
