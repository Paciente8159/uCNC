/*
	Name: dio_control.c
	Description: The input control unit for uCNC.
        This is responsible to check all limit switches (both hardware and software), control switches,
        and probe.

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
#include "dio_control.h"
#include "cnc.h"

static volatile bool dio_homing;
static uint8_t dio_limits;
static uint8_t dio_controls;
static uint8_t dio_probe;
static uint16_t dio_outputs;

void dio_init()
{
	uint8_t l = (g_settings.hard_limits_enabled) ? (mcu_get_limits() ^ g_settings.limits_invert_mask) : 0;
	dio_limits = l & ~PROBE_MASK; 
	dio_controls = mcu_get_controls() ^ g_settings.control_invert_mask;
	dio_probe = l & PROBE_MASK;
	dio_outputs = 0; //set outputs to default state
	mcu_set_outputs(dio_outputs);
}

void dio_limits_isr(uint8_t limits)
{	
	limits ^= g_settings.limits_invert_mask;
	
	if(g_settings.hard_limits_enabled)
	{
		dio_limits = limits & ~PROBE_MASK;
		dio_probe = limits & PROBE_MASK;
		//limits &= dio_limitsmask;
		if(dio_limits)
		{
			cnc_stop();
			cnc_alarm(EXEC_ALARM_HARD_LIMIT);
		}
	}
}

void dio_controls_isr(uint8_t controls)
{
	dio_controls = controls ^ g_settings.control_invert_mask;
	
	if(dio_controls & ESTOP_MASK)
	{
		cnc_exec_rt_command(RT_CMD_RESET);
	}
	
	if(dio_controls & SAFETY_DOOR_MASK)
	{
		cnc_exec_rt_command(RT_CMD_SAFETY_DOOR);
	}
	
	if(dio_controls & FHOLD_MASK)
	{
		cnc_exec_rt_command(RT_CMD_FEED_HOLD);
	}
}

bool dio_check_boundaries(float* axis)
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
			//dio_limits |= LIMIT_X_MASK;
			return false;
		}	
		#endif
		#ifdef AXIS_Y
		if(axis[AXIS_Y]<0 || axis[AXIS_Y]>g_settings.max_distance[AXIS_Y])
		{
			//dio_limits |= LIMIT_Y_MASK;
			return false;
		}
		#endif
		#ifdef AXIS_Z
		if(axis[AXIS_Z]<0 || axis[AXIS_Z]>g_settings.max_distance[AXIS_Z])
		{
			//dio_limits |= LIMIT_Z_MASK;
			return false;
		}
		#endif
		#ifdef AXIS_A
		if(axis[AXIS_A]<0 || axis[AXIS_A]>g_settings.max_distance[AXIS_A])
		{
			//dio_limits |= LIMIT_A_MASK;
			return false;
		}
		#endif
		#ifdef AXIS_B
		if(axis[AXIS_B]<0 || axis[AXIS_B]>g_settings.max_distance[AXIS_B])
		{
			//dio_limits |= LIMIT_B_MASK;
			return false;
		}
		#endif
		#ifdef AXIS_C
		if(axis[AXIS_C]<0 || axis[AXIS_C]>g_settings.max_distance[AXIS_C])
		{
			//dio_limits |= LIMIT_C_MASK;
			return false;
		}
		#endif
	}
	else
	{
		#ifdef AXIS_X
		if(axis[AXIS_X]<-g_settings.max_distance[AXIS_X] || axis[AXIS_X]>g_settings.max_distance[AXIS_X])
		{
			//dio_limits |= LIMIT_X_MASK;
			return false;
		}	
		#endif
		#ifdef AXIS_Y
		if(axis[AXIS_Y]<-g_settings.max_distance[AXIS_Y] || axis[AXIS_Y]>g_settings.max_distance[AXIS_Y])
		{
			//dio_limits |= LIMIT_Y_MASK;
			return false;
		}
		#endif
		#ifdef AXIS_Z
		if(axis[AXIS_Z]<-g_settings.max_distance[AXIS_Z] || axis[AXIS_Z]>g_settings.max_distance[AXIS_Z])
		{
			//dio_limits |= LIMIT_Z_MASK;
			return false;
		}
		#endif
		#ifdef AXIS_A
		if(axis[AXIS_A]<-g_settings.max_distance[AXIS_A] || axis[AXIS_A]>g_settings.max_distance[AXIS_A])
		{
			//dio_limits |= LIMIT_A_MASK;
			return false;
		}
		#endif
		#ifdef AXIS_B
		if(axis[AXIS_B]<-g_settings.max_distance[AXIS_B] || axis[AXIS_B]>g_settings.max_distance[AXIS_B])
		{
			//dio_limits |= LIMIT_B_MASK;
			return false;
		}
		#endif
		#ifdef AXIS_C
		if(axis[AXIS_C]<-g_settings.max_distance[AXIS_C] || axis[AXIS_C]>g_settings.max_distance[AXIS_C])
		{
			//dio_limits |= LIMIT_C_MASK;
			return false;
		}
		#endif
	}

	return true;
}

uint8_t dio_get_limits(uint8_t limitmask)
{
	return (dio_limits & limitmask);
}

uint8_t dio_get_controls(uint8_t controlmask)
{
	return (dio_controls & controlmask);
}

bool dio_get_probe()
{
	return dio_probe;
}

uint16_t dio_get_inputs()
{
	return mcu_get_inputs();
}

void dio_set_outputs(uint16_t mask)
{
	dio_outputs |= mask;
	mcu_set_outputs(dio_outputs);
}

void dio_clear_outputs(uint16_t mask)
{
	dio_outputs &= ~mask;
	mcu_set_outputs(dio_outputs);
}

uint8_t dio_get_analog(uint8_t channel)
{
	return mcu_get_analog(channel);
}

void dio_set_pwm(uint8_t channel, uint8_t value)
{
	mcu_set_pwm(channel, value);
}
