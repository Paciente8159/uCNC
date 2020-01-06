/*
	Name: cnc.c
	Description: uCNC main unit
	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 17/09/2019

	uCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	uCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "config.h"
#include "utils.h"
#include "settings.h"
#include "mcudefs.h"
#include "mcu.h"
#include "grbl_interface.h"
#include "serial.h"
#include "protocol.h"
#include "parser.h"
#include "kinematics.h"
#include "motion_control.h"
#include "planner.h"
#include "interpolator.h"
#include "trigger_control.h"
#include "cnc.h"

typedef struct
{
    bool unlocked;		//signals if CNC is unlocked and gcode can run
    uint8_t homed;		//saves homing state
    volatile uint8_t exec_state;
    uint8_t cancel_state;
    uint8_t active_alarm;
    bool send_report;
    bool reset;
} cnc_state_t;

cnc_state_t cnc_state;

void cnc_init()
{
	//initializes cnc state
	memset(&cnc_state, 0, sizeof(cnc_state_t));
	
	//initializes all systems
	//mcu
	mcu_init();
	serial_init();
	if(!settings_init())
	{
		settings_reset();
		protocol_send_error(STATUS_SETTING_READ_FAIL);
	}
	
	mc_init();
	if(!parser_init())
	{
		parser_parameters_reset();
		protocol_send_error(STATUS_SETTING_READ_FAIL);
	}
	
	kinematics_init();
	planner_init();
	tc_init();
	interpolator_init();
	cnc_state.unlocked = false;
}

void cnc_reset()
{
	cnc_state.reset = false;
	//clear all systems
	interpolator_clear();
	planner_clear();
	serial_flush();		
	protocol_send_string(MSG_STARTUP);

	//initial state (ALL IS LOCKED)
	if(g_settings.homing_enabled && cnc_state.homed != HOME_OK)
	{
		cnc_set_exec_state(EXEC_ALARM);
		cnc_state.unlocked = false;
		protocol_send_string(MSG_FEEDBACK_2);
	}
	else
	{
		cnc_unlock();
	}

	//mcu_set_pwm(0,64);
}

void cnc_run()
{
	cnc_reset();
	
	do
	{
		//process gcode commands
		if(!serial_rx_is_empty())
		{
			uint8_t error = 0;
			//protocol_echo();
			uint8_t c = serial_peek();
			switch(c)
			{
				case '\n':
					serial_getc();
					break;
				case '$':
					serial_getc();
					error = parser_grbl_command();
					break;
				default:
					if((cnc_state.exec_state < EXEC_JOG) && cnc_state.unlocked)
					{
						error = parser_gcode_command(false);
					}
					else
					{
						error = STATUS_SYSTEM_GC_LOCK;
					}
					break;
			}
			
			if(!error)
			{
				protocol_send_ok();
			}
			else
			{
				protocol_send_error(error);
				serial_discard_cmd();//flushes the rest of the command
			}
		}
		
		cnc_doevents();
	}while(!cnc_state.reset);
}

void cnc_exec_rt_command(uint8_t command)
{
	switch(command)
	{
		case RT_CMD_REPORT:
			cnc_state.send_report = true;
			break;
		case RT_CMD_RESET:
			if(cnc_get_exec_state(EXEC_HOMING)) //if homing
			{
				cnc_clear_exec_state(EXEC_HOMING); //clear homing flag that supress alarms
				cnc_alarm(EXEC_ALARM_HOMING_FAIL_RESET);
			}
			//imediatly calls killing process
			cnc_kill();
			cnc_state.reset = true;			
			break;
		case RT_CMD_SAFETY_DOOR:
			cnc_set_exec_state(EXEC_DOOR);
			if(cnc_get_exec_state(EXEC_HOMING)) //if homing
			{
				cnc_clear_exec_state(EXEC_HOMING); //clear homing flag that supress alarms
				cnc_kill();
				cnc_alarm(EXEC_ALARM_HOMING_FAIL_DOOR);
			}
			else
			{
				cnc_set_exec_state(EXEC_HOLD);
			}
			break;
		case RT_CMD_JOG_CANCEL:
			if(cnc_get_exec_state(EXEC_JOG)) //if not jog mode ignores
			{
				cnc_set_exec_state(EXEC_HOLD); //activates hold
			}
			break;
		case RT_CMD_FEED_HOLD:
			if((cnc_state.exec_state < EXEC_HOMING)) //if not in idle, run or jog ignores
			{
				cnc_set_exec_state(EXEC_HOLD); //activates hold
			}
			break;
		case RT_CMD_CYCLE_START:
			if(cnc_get_exec_state(EXEC_DOOR | EXEC_ALARM | EXEC_SLEEP))
			{
				return;
			}
			
			if(!cnc_get_exec_state(EXEC_RUN))
			{
				//clears active hold
				cnc_clear_exec_state(EXEC_HOLD);
			}
			break;	
	}
	
	//cnc_state.rt_cmd = 0;
}

void cnc_doevents()
{
	//exit if in alarm mode and ignores all other realtime commands and exit
	if(cnc_get_exec_state(EXEC_ALARM))
	{
		if(cnc_state.active_alarm) //active alarm message
		{
			protocol_send_alarm(cnc_state.active_alarm);
			cnc_state.active_alarm = 0;
			return;
		}
	}

	if(cnc_state.send_report)
	{
		protocol_send_status();
	}
	cnc_state.send_report = false;
	
	if(!cnc_state.unlocked) //cnc is locked
	{
		return; //leaves the interpolator dry
	}
	
	if(cnc_get_exec_state(EXEC_HOLD))//there is an active hold
	{
		//hold has come to a full stop
		if(!cnc_get_exec_state(EXEC_RUN) && cnc_get_exec_state(EXEC_HOLD))
		{
			//if homing or jog flushes buffers
			if(cnc_get_exec_state(EXEC_HOMING) || cnc_get_exec_state(EXEC_JOG))
			{
				interpolator_stop();
				interpolator_clear();
				planner_clear();
				//can clears any active hold, homing or jog
				cnc_clear_exec_state(EXEC_HOLD | EXEC_HOMING | EXEC_JOG);
			}
		}
		
		return; //leaves the interpolator dry
	}
	
	interpolator_run();

}

void cnc_home()
{
	cnc_set_exec_state(EXEC_HOMING);
	kinematics_home();
}

void cnc_alarm(uint8_t code)
{
	if(!cnc_get_exec_state(EXEC_HOMING)) //while homing supress all alarms
	{
		cnc_set_exec_state(EXEC_ALARM);
		cnc_state.active_alarm = code;
	}
	
	//locks the system
	cnc_state.unlocked = false;
}

void cnc_kill()
{
	//kills motion and flushes all buffers
	cnc_stop();
	interpolator_clear();
	planner_clear();
	cnc_state.unlocked = false;
}

void cnc_stop()
{
	//halt is active and was running flags it lost home position
	if(cnc_get_exec_state(EXEC_RUN) & !cnc_state.unlocked)
	{
		interpolator_stop();
		cnc_state.homed |= HOME_NEEDS_REHOME;
	}
}

void cnc_unlock()
{
	if(cnc_state.unlocked)
	{
		return;
	}
	
	//if emergeny stop is pressed it can't unlock
	if(tc_get_controls(ESTOP_MASK))
	{
		cnc_state.unlocked = false;
		return;
	}
	
	if(cnc_get_exec_state(EXEC_ALARM))
	{
		protocol_send_string(MSG_FEEDBACK_3);
	}
	
	cnc_clear_exec_state(EXEC_ALARM);
	cnc_state.unlocked = true;
}

uint8_t cnc_get_exec_state(uint8_t statemask)
{
	return (cnc_state.exec_state & statemask);
}

void cnc_set_exec_state(uint8_t statemask)
{
	cnc_state.exec_state |= statemask;
}

void cnc_clear_exec_state(uint8_t statemask)
{
	cnc_state.exec_state &= ~statemask;
}

uint8_t cnc_is_homed()
{
	return cnc_state.homed;
}
/*moved to motion control
uint8_t cnc_home_axis(uint8_t axis, uint8_t axis_limit)
{
	float target[AXIS_COUNT];
	uint8_t axis_mask = (1<<axis);
	
	memcpy(&target, planner_get_position(), sizeof(target));

	//unlock the cnc
	cnc_unlock();

	//if HOLD or ALARM are still active or any limit switch is not cleared fails to home
	if(cnc_get_exec_state(EXEC_HOLD | EXEC_ALARM) || tc_get_limits(LIMITS_MASK))
	{
		return EXEC_ALARM_HOMING_FAIL_LIMIT_ACTIVE;
	}
	
	float max_home_dist = -g_settings.max_distance[axis] * 1.5f;
	//checks homing dir
	if(g_settings.homing_dir_invert_mask & axis_mask)
	{
		max_home_dist = -max_home_dist;
	}
	
	target[axis] += max_home_dist;
	cnc_set_exec_state(EXEC_HOMING);
	planner_add_line((float*)&target, g_settings.homing_fast_feed_rate * 0.0166666667f);
	do{
		cnc_doevents();
	} whilecnc_get_exec_state(EXEC_RUN);
	
	//flushes buffers
	interpolator_stop();
	interpolator_clear();
	planner_clear();
	
	//if limit was not triggered 
	if(!tc_get_limits(axis_limit))
	{
		return EXEC_ALARM_HOMING_FAIL_APPROACH;
	}
	
	cnc_unlock();
	//zero's the planner
	memcpy(&target, planner_get_position(), sizeof(target));
	max_home_dist = g_settings.homing_offset * 5.0f;
	
	//checks homing dir
	if(g_settings.homing_dir_invert_mask & axis_mask)
	{
		max_home_dist = -max_home_dist;
	}
	
	target[axis] += max_home_dist;

	planner_add_line((float*)&target, g_settings.homing_slow_feed_rate * 0.0166666667f);

	do {
		cnc_doevents();
		//activates hold (single time) if limit is free
		if(!tc_get_limits(axis_limit) && !cnc_get_exec_state(EXEC_HOLD))
		{
			cnc_set_exec_state(EXEC_HOLD); 
		}
	} whilecnc_get_exec_state(EXEC_RUN);
	
	//stops, flushes buffers and clears the hold if active
	cnc_stop();
	interpolator_clear();
	planner_clear();
	cnc_clear_exec_state(EXEC_HOLD);
	
	if(tc_get_limits(axis_limit))
	{
		return EXEC_ALARM_HOMING_FAIL_APPROACH;
	}
	
	return 0;
}
*/
void cnc_offset_home()
{
	float target[AXIS_COUNT];
	
	memcpy(&target, planner_get_position(), sizeof(target));
	
	for(uint8_t i = AXIS_COUNT; i != 0;)
	{
		i--;
		if(g_settings.homing_dir_invert_mask & (1<<i))
		{
			target[i] -= g_settings.homing_offset;
		}
		else
		{
			target[i] += g_settings.homing_offset;
		}
		//target[i] += ((g_settings.homing_dir_invert_mask & axis_mask) ? -g_settings.homing_offset : g_settings.homing_offset);
	}
	
	planner_add_line((float*)&target, g_settings.homing_fast_feed_rate * 0.0166666667f);
	do{
		cnc_doevents();
	} while(cnc_get_exec_state(EXEC_RUN));
}

void cnc_reset_position()
{
	interpolator_reset_rt_position();
	planner_resync_position();
}

