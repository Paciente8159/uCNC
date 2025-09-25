/*
	Name: boardmap_mega_shield_v3.h
	Description: Contains all MCU and PIN definitions for Arduino Mega with the Shield V3 to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 17/07/2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_MEGA_SHIELD_V3_H
#define BOARDMAP_MEGA_SHIELD_V3_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MCU
#define MCU MCU_AVR
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "MEGA SHIELD V3"
#endif

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define AVR6

#define PCINT0_PORT B
#define PCINT1_PORT J
#define PCINT2_PORT K

#define PCINT0_PORT B
#define PCINT1_PORT J
#define PCINT2_PORT K
#define UART_PORT 0
#define ONESHOT_TIMER 4
#define IC74HC595_COUNT 0
#define IC74HC165_COUNT 0
#define STEP0_BIT 4
#define STEP0_PORT E
#define STEP1_BIT 5
#define STEP1_PORT E
#define STEP2_BIT 5
#define STEP2_PORT G
#define DIR0_BIT 3
#define DIR0_PORT E
#define DIR1_BIT 3
#define DIR1_PORT H
#define DIR2_BIT 4
#define DIR2_PORT H
#define STEP0_EN_BIT 5
#define STEP0_EN_PORT H
#define PWM0_BIT 6
#define PWM0_PORT B
#define PWM0_CHANNEL B
#define PWM0_TIMER 0
#define DOUT0_BIT 7
#define DOUT0_PORT B
#define LIMIT_X_BIT 6
#define LIMIT_X_PORT H
#define LIMIT_Y_BIT 4
#define LIMIT_Y_PORT B
#define LIMIT_Z_BIT 5
#define LIMIT_Z_PORT B
#define PROBE_BIT 5
#define PROBE_PORT F
#define ESTOP_BIT 0
#define ESTOP_PORT F
#define FHOLD_BIT 1
#define FHOLD_PORT F
#define CS_RES_BIT 2
#define CS_RES_PORT F
#define TX_BIT 1
#define TX_PORT E
#define RX_BIT 0
#define RX_PORT E
#define RX_PULLUP

#define ONESHOT_TIMER 4
#define RTC_TIMER 2

#define ENABLE_RT_PROBE_CHECKING
#define ENABLE_RT_LIMITS_CHECKING


#ifdef __cplusplus
}
#endif

#endif
