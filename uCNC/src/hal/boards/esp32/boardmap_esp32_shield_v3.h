/*
	Name: boardmap_esp32_shield_v3.h
	Description: Contains all MCU and PIN definitions for Arduino UNO similar to Grbl 0.8 to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 04/01/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BOARDMAP_ESP_SHIELD_V3_H
#define BOARDMAP_ESP_SHIELD_V3_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BOARD_NAME
#define BOARD_NAME "ESP32 SHIELD V3"
#endif

#include "boardmap_wemos_d1_r32.h"

// swap limit z and replace pwm by spin enable pin
#ifdef LIMIT_Z_BIT
#undef LIMIT_Z_BIT
#endif
#ifdef PWM0_BIT
#undef PWM0_BIT
#endif

// Grbl 0.8 limit z
#define LIMIT_Z_BIT 23 // assigns LIMIT_Z pin

// Grbl 0.8 spindle en
#define PWM0_BIT 19 // assigns PWM0 pin

#ifdef __cplusplus
}
#endif

#endif
