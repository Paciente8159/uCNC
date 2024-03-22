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
#ifndef SPINDLE_RELAY_FWD
#define SPINDLE_RELAY_FWD DOUT0
#endif
#ifndef SPINDLE_RELAY_REV
#define SPINDLE_RELAY_REV DOUT1
#endif

#ifdef ENABLE_COOLANT
#ifndef SPINDLE_RELAY_COOLANT_FLOOD
#define SPINDLE_RELAY_COOLANT_FLOOD DOUT2
#endif
#ifndef SPINDLE_RELAY_COOLANT_MIST
#define SPINDLE_RELAY_COOLANT_MIST DOUT3
#endif
#endif

void set_speed(int16_t value)
{

	if (value == 0)
	{
#if ASSERT_PIN(SPINDLE_RELAY_FWD)
		io_clear_output(SPINDLE_RELAY_FWD);
#endif
#if ASSERT_PIN(SPINDLE_RELAY_REV)
		io_clear_output(SPINDLE_RELAY_REV);
#endif
	}
	else if (value < 0)
	{
#if ASSERT_PIN(SPINDLE_RELAY_FWD)
		io_clear_output(SPINDLE_RELAY_FWD);
#endif
#if ASSERT_PIN(SPINDLE_RELAY_REV)
		io_set_output(SPINDLE_RELAY_REV);
#endif
	}
	else
	{
#if ASSERT_PIN(SPINDLE_RELAY_REV)
		io_clear_output(SPINDLE_RELAY_REV);
#endif
#if ASSERT_PIN(SPINDLE_RELAY_FWD)
		io_set_output(SPINDLE_RELAY_FWD);
#endif
	}
}

void set_coolant(uint8_t value)
{
#ifdef ENABLE_COOLANT
	SET_COOLANT(SPINDLE_RELAY_COOLANT_FLOOD, SPINDLE_RELAY_COOLANT_MIST, value);
#endif
}

const tool_t spindle_relay = {
	.startup_code = NULL,
	.shutdown_code = NULL,
	.pid_update = NULL,
	.range_speed = NULL,
	.get_speed = NULL,
	.set_speed = &set_speed,
	.set_coolant = &set_coolant};
