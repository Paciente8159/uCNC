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

#ifndef BOARDMAP_UNO_SHIELD_V3_H
#define BOARDMAP_UNO_SHIELD_V3_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MCU
#define MCU MCU_AVR
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "UNO SHIELD V3"
#endif

#include "boardmap_uno.h"

// swap limit z and replace pwm by spin enable pin
#ifdef LIMIT_Z_BIT
#undef LIMIT_Z_BIT
#endif
#ifdef LIMIT_Z_PORT
#undef LIMIT_Z_PORT
#endif
#ifdef PWM0_BIT
#undef PWM0_BIT
#endif
#ifdef PWM0_PORT
#undef PWM0_PORT
#endif
#ifdef PWM0_CHANNEL
#undef PWM0_CHANNEL
#endif
#ifdef PWM0_TIMER
#undef PWM0_TIMER
#endif

// Grbl 0.8 limit z
#define LIMIT_Z_BIT 3	 // assigns LIMIT_Z pin
#define LIMIT_Z_PORT B // assigns LIMIT_Z port
#define LIMIT_Z_ISR 0	 // assigns LIMIT_Z ISR

// spindle en
#define DOUT1_BIT 4
#define DOUT1_PORT B

#ifdef __cplusplus
}
#endif

#endif
