/*
	Name: trigger_control.c
	Description: The input control unit for uCNC.
        This is responsible to check all limit switches (both hardware and software), control switches,
        and probe.

		TODO:
			-implement generic inputs
			
	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 07/12/2019

	uCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	uCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "config.h"
#include "grbl_interface.h"
#include "settings.h"
#include "mcudefs.h"
#include "mcu.h"
#include "mcumap.h"
#include "trigger_control.h"
#include "cnc.h"

static volatile bool tc_homing;
static uint8_t tc_limits;
static uint8_t tc_controls;
static uint8_t tc_probe;

void tc_init()
{
	uint8_t l = (g_settings.hard_limits_enabled) ? (mcu_get_limits() ^ g_settings.limits_invert_mask) : 0;
	tc_limits = l & ~PROBE_MASK; 
	tc_controls = mcu_get_controls() ^ g_settings.control_invert_mask;
	tc_probe = l & PROBE_MASK;
}

void tc_limits_isr(uint8_t limits)
{	
	limits ^= g_settings.limits_invert_mask;
	
	if(g_settings.hard_limits_enabled)
	{
		tc_limits = limits & ~PROBE_MASK;
		tc_probe = limits & PROBE_MASK;
		//limits &= tc_limitsmask;
		if(tc_limits)
		{
			cnc_stop();
			cnc_alarm(EXEC_ALARM_HARD_LIMIT);
		}
	}
}

void tc_controls_isr(uint8_t controls)
{
	tc_controls = controls ^ g_settings.control_invert_mask;
	
	if(tc_controls & ESTOP_MASK)
	{
		cnc_exec_rt_command(RT_CMD_RESET);
	}
	
	if(tc_controls & SAFETY_DOOR_MASK)
	{
		cnc_exec_rt_command(RT_CMD_SAFETY_DOOR);
	}
	
	if(tc_controls & FHOLD_MASK)
	{
		cnc_exec_rt_command(RT_CMD_FEED_HOLD);
	}
}

bool tc_check_boundaries(float* axis)
{
	if(!g_settings.soft_limits_enabled)
	{
		return true;
	}
	
	if(cnc_is_homed())
	{
		#ifdef AXIS_X
		if(axis[AXIS_X]<0 || axis[AXIS_X]>g_settings.max_distance[AXIS_X])
		{
			//tc_limits |= LIMIT_X_MASK;
			return false;
		}	
		#endif
		#ifdef AXIS_Y
		if(axis[AXIS_Y]<0 || axis[AXIS_Y]>g_settings.max_distance[AXIS_Y])
		{
			//tc_limits |= LIMIT_Y_MASK;
			return false;
		}
		#endif
		#ifdef AXIS_Z
		if(axis[AXIS_Z]<0 || axis[AXIS_Z]>g_settings.max_distance[AXIS_Z])
		{
			//tc_limits |= LIMIT_Z_MASK;
			return false;
		}
		#endif
		#ifdef AXIS_A
		if(axis[AXIS_A]<0 || axis[AXIS_A]>g_settings.max_distance[AXIS_A])
		{
			//tc_limits |= LIMIT_A_MASK;
			return false;
		}
		#endif
		#ifdef AXIS_B
		if(axis[AXIS_B]<0 || axis[AXIS_B]>g_settings.max_distance[AXIS_B])
		{
			//tc_limits |= LIMIT_B_MASK;
			return false;
		}
		#endif
		#ifdef AXIS_C
		if(axis[AXIS_C]<0 || axis[AXIS_C]>g_settings.max_distance[AXIS_C])
		{
			//tc_limits |= LIMIT_C_MASK;
			return false;
		}
		#endif
	}
	else
	{
		#ifdef AXIS_X
		if(axis[AXIS_X]<-g_settings.max_distance[AXIS_X] || axis[AXIS_X]>g_settings.max_distance[AXIS_X])
		{
			//tc_limits |= LIMIT_X_MASK;
			return false;
		}	
		#endif
		#ifdef AXIS_Y
		if(axis[AXIS_Y]<-g_settings.max_distance[AXIS_Y] || axis[AXIS_Y]>g_settings.max_distance[AXIS_Y])
		{
			//tc_limits |= LIMIT_Y_MASK;
			return false;
		}
		#endif
		#ifdef AXIS_Z
		if(axis[AXIS_Z]<-g_settings.max_distance[AXIS_Z] || axis[AXIS_Z]>g_settings.max_distance[AXIS_Z])
		{
			//tc_limits |= LIMIT_Z_MASK;
			return false;
		}
		#endif
		#ifdef AXIS_A
		if(axis[AXIS_A]<-g_settings.max_distance[AXIS_A] || axis[AXIS_A]>g_settings.max_distance[AXIS_A])
		{
			//tc_limits |= LIMIT_A_MASK;
			return false;
		}
		#endif
		#ifdef AXIS_B
		if(axis[AXIS_B]<-g_settings.max_distance[AXIS_B] || axis[AXIS_B]>g_settings.max_distance[AXIS_B])
		{
			//tc_limits |= LIMIT_B_MASK;
			return false;
		}
		#endif
		#ifdef AXIS_C
		if(axis[AXIS_C]<-g_settings.max_distance[AXIS_C] || axis[AXIS_C]>g_settings.max_distance[AXIS_C])
		{
			//tc_limits |= LIMIT_C_MASK;
			return false;
		}
		#endif
	}

	return true;
}

uint8_t tc_get_limits(uint8_t limitmask)
{
	return (tc_limits & limitmask);
}

uint8_t tc_get_controls(uint8_t controlmask)
{
	return (tc_controls & controlmask);
}

bool tc_get_probe()
{
	return tc_probe;
}
