/*
	Name: io_control.c
	Description: The input control unit for µCNC.
        This is responsible to check all limit switches (both hardware and software), control switches,
        and probe.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 07/12/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "config.h"
#include "grbl_interface.h"
#include "settings.h"
#include "mcudefs.h"
#include "mcu.h"
#include "mcumap.h"
#include "io_control.h"
#include "parser.h"
#include "interpolator.h"
#include "cnc.h"

void io_limits_isr(uint8_t limits)
{
    limits ^= g_settings.limits_invert_mask;

    if(g_settings.hard_limits_enabled)
    {
        if(limits)
        {
            if(cnc_get_exec_state(EXEC_RUN))
            {
                cnc_set_exec_state(EXEC_NOHOME); //if motions was executing flags home position lost
            }
            itp_stop();
            cnc_set_exec_state(EXEC_LIMITS);
            if(!cnc_get_exec_state(EXEC_HOMING)) //if not in a homing motion triggers an alarm
            {
                cnc_alarm(EXEC_ALARM_HARD_LIMIT);
            }
        }
    }
}

void io_controls_isr(uint8_t controls)
{
    controls ^= g_settings.control_invert_mask;

#ifdef ESTOP
    if(controls & ESTOP_MASK)
    {
        cnc_call_rt_command(CMD_CODE_RESET);
        return; //forces exit
    }
#endif
#ifdef SAFETY_DOOR
    if(controls & SAFETY_DOOR_MASK)
    {
        //safety door activates hold simultaneously to start the controlled stop
        cnc_call_rt_command(CMD_CODE_SAFETY_DOOR);
    }
#endif
#ifdef FHOLD
    if(controls & FHOLD_MASK)
    {
        cnc_call_rt_command(CMD_CODE_FEED_HOLD);
    }
#endif
#ifdef CS_RES
    if(controls & CS_RES_MASK)
    {
        cnc_call_rt_command(CMD_CODE_CYCLE_START);
    }
#endif
}

void io_probe_isr(uint8_t probe)
{
    //on hit enables hold (directly)
    cnc_set_exec_state(EXEC_HOLD);
    //stores rt position
    parser_sync_probe();
}

bool io_check_boundaries(float* axis)
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

uint8_t io_get_limits(uint8_t limitmask)
{
    return ((mcu_get_limits() ^ g_settings.limits_invert_mask) & limitmask);
}

uint8_t io_get_controls(uint8_t controlmask)
{
    return ((mcu_get_controls() ^ g_settings.control_invert_mask) & controlmask);
}

void io_enable_probe()
{
    mcu_enable_probe_isr();
}

void io_disable_probe()
{
    mcu_disable_probe_isr();
}

bool io_get_probe()
{
    bool probe = (mcu_get_limits() & PROBE_MASK);
    return (!g_settings.probe_invert_mask) ? probe : !probe;
}

uint32_t io_get_inputs()
{
    return mcu_get_inputs();
}

void io_set_outputs(uint32_t mask)
{
    mcu_set_outputs(mcu_get_outputs() | mask);
}

void io_clear_outputs(uint32_t mask)
{
    mcu_set_outputs(mcu_get_outputs() & ~mask);
}

void io_toogle_outputs(uint32_t mask)
{
    mcu_set_outputs(mcu_get_outputs() ^ mask);
}

uint32_t io_get_outputs()
{
    return mcu_get_outputs();
}

uint8_t io_get_analog(uint8_t channel)
{
    return mcu_get_analog(channel);
}

void io_set_pwm(uint8_t channel, uint8_t value)
{
    mcu_set_pwm(channel, value);
}

uint8_t io_get_pwm(uint8_t channel)
{
    return mcu_get_pwm(channel);
}
