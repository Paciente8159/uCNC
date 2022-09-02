/*
	Name: spindle_relay.c
	Description: Defines a spindle tool using DOUT to enable a digital pin for µCNC.
				 Defines a coolant output using DOUT1.

	Copyright: Copyright (c) João Martins
	Author: James Harton
	Date: 1/5/2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

/**
 * Configures a simple spindle control with digital pins for tool forward (`M3`) and reverse (`M4`).
 * There is also provision for digital pins for coolant mist (`M7`) and flood (`M8`).
 *
 * */

// Spindle enable pins.  You can set these to the same pin if required.
#ifndef SPINDLE_FWD
#define SPINDLE_FWD DOUT0
#endif
#ifndef SPINDLE_REV
#define SPINDLE_REV DOUT1
#endif

#ifdef ENABLE_COOLANT
#ifndef COOLANT_FLOOD
#define COOLANT_FLOOD DOUT2
#endif
#ifndef COOLANT_MIST
#define COOLANT_MIST DOUT3
#endif
#endif

static int16_t spindle_speed;

void spindle_relay_set_speed(int16_t value)
{

	if (value == 0)
	{
#if !(SPINDLE_FWD < 0)
		mcu_clear_output(SPINDLE_FWD);
#endif
#if !(SPINDLE_REV < 0)
		mcu_clear_output(SPINDLE_REV);
#endif
	}
	else if (value < 0)
	{
#if !(SPINDLE_FWD < 0)
		mcu_clear_output(SPINDLE_FWD);
#endif
#if !(SPINDLE_REV < 0)
		mcu_set_output(SPINDLE_REV);
#endif
	}
	else
	{
#if !(SPINDLE_REV < 0)
		mcu_clear_output(SPINDLE_REV);
#endif
#if !(SPINDLE_FWD < 0)
		mcu_set_output(SPINDLE_FWD);
#endif
	}
}

void spindle_relay_set_coolant(uint8_t value)
{
#ifdef ENABLE_COOLANT
	SET_COOLANT(COOLANT_FLOOD, COOLANT_MIST, value);
#endif
}

uint16_t spindle_relay_get_speed(void)
{
	return ABS(spindle_speed);
}

const tool_t __rom__ spindle_relay = {
	.startup_code = NULL,
	.shutdown_code = NULL,
#if PID_CONTROLLERS > 0
	.pid_update = NULL,
	.pid_error = NULL,
#endif
	.range_speed = NULL,
	.get_speed = &spindle_relay_get_speed,
	.set_speed = &spindle_relay_set_speed,
	.set_coolant = &spindle_relay_set_coolant};
