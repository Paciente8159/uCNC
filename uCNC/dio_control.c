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
#include "parser.h"
#include "cnc.h"

void dio_limits_isr(uint8_t limits)
{	
	limits ^= g_settings.limits_invert_mask;
	
	if(g_settings.hard_limits_enabled)
	{
		if(limits)
		{
			cnc_stop();
			cnc_alarm(EXEC_ALARM_HARD_LIMIT);
		}
	}
}

void dio_controls_isr(uint8_t controls)
{
	controls ^= g_settings.control_invert_mask;
	
	if(controls & ESTOP_MASK)
	{
		cnc_exec_rt_command(RT_CMD_RESET);
	}
	
	if(controls & FHOLD_MASK)
	{
		cnc_call_rt_command(RT_CMD_FEED_HOLD);
	}
}

void dio_probe_isr(uint8_t probe)
{
	//on hit enables hold (directly)
	cnc_set_exec_state(EXEC_HOLD);
	//stores rt position
	parser_sync_probe();
	
	
	/*bool hit = ((!g_settings.probe_invert_mask) ? (probe != 0) : (probe == 0));
	
	if(hit)
	{
		//on hit enables hold (directly)
		cnc_set_exec_state(EXEC_HOLD);
		//stores rt position
		itp_get_rt_position(dio_last_probe);
	}*/
}

bool dio_check_boundaries(float* axis)
{
	if(!g_settings.soft_limits_enabled)
	{
		return true;
	}

	for(uint8_t i = AXIS_COUNT; i!=0;)
	{
		i--;
		float value = (axis[i] < 0) ? -axis[i] : axis[i];
		if(value > g_settings.max_distance[i])
		{
			return false;
		}
	}

	return true;
}

uint8_t dio_get_limits(uint8_t limitmask)
{
	return (mcu_get_limits() & ~PROBE_MASK & limitmask);
}

uint8_t dio_get_controls(uint8_t controlmask)
{
	return (mcu_get_controls() & controlmask);
}

void dio_enable_probe()
{
	mcu_enable_probe_isr();
}

void dio_disable_probe()
{
	mcu_disable_probe_isr();
}

bool dio_get_probe()
{
	return (mcu_get_limits() & PROBE_MASK);
}

uint16_t dio_get_inputs()
{
	return mcu_get_inputs();
}

void dio_set_outputs(uint16_t mask)
{
	mcu_set_outputs(mcu_get_outputs() | mask);
}

void dio_clear_outputs(uint16_t mask)
{
	mcu_set_outputs(mcu_get_outputs() & ~mask);
}

void dio_toogle_outputs(uint16_t mask)
{
	mcu_set_outputs(mcu_get_outputs() ^ mask);
}

uint16_t dio_get_outputs()
{
	return mcu_get_outputs();
}

uint8_t dio_get_analog(uint8_t channel)
{
	return mcu_get_analog(channel);
}

void dio_set_pwm(uint8_t channel, uint8_t value)
{
	mcu_set_pwm(channel, value);
}

uint8_t dio_get_pwm(uint8_t channel)
{
	return mcu_get_pwm(channel);
}
