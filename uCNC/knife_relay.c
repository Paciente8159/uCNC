/*
		Name: knife_relay.c
		Description: Control a single pin for a simple relay. (Such as used for a knife tool)

		Copyright: Copyright (c) Elizabeth Cray
		Author: Elizabeth Cray
		Date: 28/09/2024

		µCNC is free software: you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation, either version 3 of the License, or
		(at your option) any later version. Please see
	 <http://www.gnu.org/licenses/>

		µCNC is distributed WITHOUT ANY WARRANTY;
		Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A
	 PARTICULAR PURPOSE. See the	GNU General Public License for more details.
*/

#include "src/cnc.h"

#ifndef KNIFE_RELAY
#define KNIFE_RELAY DOUT0
#endif

// Control a relay with M3/M4/M5
static void knife_relay_set_speed(int16_t value)
{
	if (value <= 0)
	{
		// set Relay to up (off)
		io_set_pinvalue(KNIFE_RELAY, 0);
	}
	else
	{
		// set Relay to down (on)
		io_set_pinvalue(KNIFE_RELAY, 1);
	}
}

static void knife_relay_shutdown(void)
{
	// set Relay to up (off)
	io_set_pinvalue(KNIFE_RELAY, 0);
}

const tool_t knife_relay = {
	.startup_code = NULL,                        // not used (empty pointer)
	.shutdown_code = &knife_relay_shutdown,      // pointer to shutdown function
	.pid_update = NULL,	                         // not used (empty pointer)
	.range_speed = NULL,	                     // not used (empty pointer)
	.get_speed = NULL,		                     // not used (empty pointer)
	.set_speed = &knife_relay_set_speed,         // pointer to set_speed function
	.set_coolant = NULL						     // not used (empty pointer)
};