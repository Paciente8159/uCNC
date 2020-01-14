/*
	Name: cnc.c
	Description: uCNC main unit.
	
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
#include "mcumap.h"
#include "mcu.h"
#include "grbl_interface.h"
#include "serial.h"
#include "protocol.h"
#include "parser.h"
#include "kinematics.h"
#include "motion_control.h"
#include "planner.h"
#include "interpolator.h"
#include "dio_control.h"
#include "cnc.h"

typedef struct
{
    uint8_t system_state;		//signals if CNC is system_state and gcode can run
    volatile uint8_t exec_state;
    uint8_t active_alarm;
    uint8_t rt_cmd;
} cnc_state_t;

static cnc_state_t cnc_state;

void cnc_init()
{
	//initializes cnc state
	memset(&cnc_state, 0, sizeof(cnc_state_t));
	
	//initializes all systems
	//mcu
	mcu_init();
	serial_init();
	settings_init();
	mc_init();
	parser_init();
	planner_init();
	//dio_init();
	itp_init();
}

void cnc_reset()
{
	cnc_state.system_state = SYS_OK;
	//clear all systems
	itp_clear();
	planner_clear();
	serial_flush();		
	protocol_send_string(MSG_STARTUP);

	//initial state (ALL IS LOCKED)
	if(g_settings.homing_enabled /*&& cnc_state.homed != HOME_OK*/)
	{
		cnc_set_exec_state(EXEC_ALARM);
		cnc_state.system_state = SYS_LOCKED;
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
					if((cnc_state.exec_state < EXEC_JOG) && !cnc_state.system_state)
					{
						error = parser_gcode_command();
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
	}while(!CHECKFLAG(cnc_state.system_state, SYS_RESET));
}

void cnc_call_rt_command(uint8_t command)
{
	switch(command)
	{
		case RT_CMD_REPORT:
			if(!cnc_state.rt_cmd)
			{
				cnc_state.rt_cmd = command;
			}
			break;
		default:
			cnc_state.rt_cmd = command;
			break;
	}
}

void cnc_exec_rt_command(uint8_t command)
{
	switch(command)
	{
		case RT_CMD_REPORT:
			protocol_send_status();
			break;
		case RT_CMD_RESET:
			if(cnc_get_exec_state(EXEC_HOMING)) //if homing
			{
				cnc_clear_exec_state(EXEC_HOMING); //clear homing flag that supress alarms
				cnc_alarm(EXEC_ALARM_HOMING_FAIL_RESET);
			}
			//imediatly calls killing process
			cnc_kill();
			cnc_state.system_state = SYS_RESET;			
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
		case RT_CMD_FEED_100:
			planner_feed_ovr_reset();
			break;
		case RT_CMD_FEED_INC_COARSE:
			planner_feed_ovr_inc(FEED_OVR_COARSE);
			break;
		case RT_CMD_FEED_DEC_COARSE:
			planner_feed_ovr_inc(-FEED_OVR_COARSE);
			break;
		case RT_CMD_FEED_INC_FINE:
			planner_feed_ovr_inc(FEED_OVR_FINE);
			break;
		case RT_CMD_FEED_DEC_FINE:
			planner_feed_ovr_inc(-FEED_OVR_FINE);
			break;
		case RT_CMD_RAPIDFEED_100:
			planner_rapid_feed_ovr_reset();
			break;
		case RT_CMD_RAPIDFEED_OVR1:
			planner_rapid_feed_ovr(RAPID_FEED_OVR1);
			break;
		case RT_CMD_RAPIDFEED_OVR2:
			planner_rapid_feed_ovr(RAPID_FEED_OVR2);
			break;
		#ifdef USE_SPINDLE
		case RT_CMD_SPINDLE_100:
			planner_spindle_ovr_reset();
			break;
		case RT_CMD_SPINDLE_INC_COARSE:
			planner_spindle_ovr_inc(SPINDLE_OVR_COARSE);
			break;
		case RT_CMD_SPINDLE_DEC_COARSE:
			planner_spindle_ovr_inc(-SPINDLE_OVR_COARSE);
			break;
		case RT_CMD_SPINDLE_INC_FINE:
			planner_spindle_ovr_inc(SPINDLE_OVR_FINE);
			break;
		case RT_CMD_SPINDLE_DEC_FINE:
			planner_spindle_ovr_inc(-SPINDLE_OVR_FINE);
			break;
		case RT_CMD_SPINDLE_TOGGLE:
			if(cnc_get_exec_state(EXEC_HOLD))
			{
				//toogle state
				if(dio_get_pwm(SPINDLE_PWM_CHANNEL))
				{
					dio_set_pwm(SPINDLE_PWM_CHANNEL, 0);
				}
				else
				{
					uint8_t pwm = 0;
					bool ccw = false;
					planner_get_spindle_speed(&pwm, &ccw);
					dio_set_pwm(SPINDLE_PWM_CHANNEL, pwm);
				}
			}
			break;
		#endif
		#ifdef USE_COOLANT
		case RT_CMD_COOL_FLD_TOGGLE:
			if(cnc_get_exec_state(EXEC_RUN | EXEC_HOLD) || cnc_state.exec_state == EXEC_IDLE)
			{
				dio_toogle_outputs(COOLANT_FLOOD_PIN);
			}
			break;
		case RT_CMD_COOL_MST_TOGGLE:
			if(cnc_get_exec_state(EXEC_RUN | EXEC_HOLD) || cnc_state.exec_state == EXEC_IDLE)
			{
				dio_toogle_outputs(COOLANT_MIST_PIN);
			}
			break;
		#endif
	}
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

	//check if RT commands are pending execution
	if(cnc_state.rt_cmd)
	{
		cnc_exec_rt_command(cnc_state.rt_cmd);
		cnc_state.rt_cmd = 0;
	}
	
	
	if(cnc_state.system_state) //cnc in not ok
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
				itp_stop();
				itp_clear();
				planner_clear();
				//can clears any active hold, homing or jog
				cnc_clear_exec_state(EXEC_HOLD | EXEC_HOMING | EXEC_JOG);
			}
		}
		
		return; //leaves the interpolator dry
	}
	
	itp_run();
}

void cnc_home()
{
	cnc_set_exec_state(EXEC_HOMING);
	uint8_t error = kinematics_home();
	if(error)
	{
		//disables homing and reenables alarm messages
		cnc_clear_exec_state(EXEC_HOMING);
		cnc_alarm(error);
		return;
	}

	float target[AXIS_COUNT];
	planner_block_data_t block_data;
	planner_get_position(target);
	
	for(uint8_t i = AXIS_COUNT; i != 0;)
	{
		i--;
		target[i] += ((g_settings.homing_dir_invert_mask & (1<<i)) ? -g_settings.homing_offset : g_settings.homing_offset);
	}
	
	block_data.feed = g_settings.homing_fast_feed_rate * MIN_SEC_MULT;
	block_data.coolant = 0;
	block_data.spindle = 0;
	block_data.dwell = 0;
	//starts offset and waits to finnish
	planner_add_line((float*)&target, block_data);
	do{
		cnc_doevents();
	} while(cnc_get_exec_state(EXEC_RUN));

	//reset position
	itp_reset_rt_position();
	planner_resync_position();
}

void cnc_alarm(uint8_t code)
{
	if(!cnc_get_exec_state(EXEC_HOMING)) //while homing supress all alarms
	{
		cnc_set_exec_state(EXEC_ALARM);
		cnc_state.active_alarm = code;
	}
	
	//locks the system
	cnc_state.system_state = false;
}

void cnc_kill()
{
	//kills motion and flushes all buffers
	cnc_stop();
	//stop tools
	#ifdef USE_SPINDLE
	dio_set_pwm(SPINDLE_PWM_CHANNEL, 0);
	#endif
	#ifdef USE_COOLANT
	dio_clear_outputs(COOLANT_FLOOD_PIN | COOLANT_MIST_PIN);
	#endif
	itp_clear();
	planner_clear();
	cnc_state.system_state = SYS_RESET;
}

void cnc_stop()
{
	//halt is active and was running flags it lost home position
	if(cnc_get_exec_state(EXEC_RUN) & cnc_state.system_state)
	{
		itp_stop();
	}
}

void cnc_unlock()
{
	if(cnc_state.system_state)
	{
		return;
	}
	
	//if emergeny stop is pressed it can't unlock
	if(dio_get_controls(ESTOP_MASK | SAFETY_DOOR_MASK))
	{
		cnc_state.system_state = SYS_LOCKED;
		return;
	}
	
	if(cnc_get_exec_state(EXEC_ALARM))
	{
		protocol_send_string(MSG_FEEDBACK_3);
	}
	
	cnc_clear_exec_state(EXEC_ALARM);
	cnc_state.system_state = SYS_OK;
}

uint8_t cnc_get_exec_state(uint8_t statemask)
{
	return CHECKFLAG(cnc_state.exec_state, statemask);
}

void cnc_set_exec_state(uint8_t statemask)
{
	SETFLAG(cnc_state.exec_state,statemask);
}

void cnc_clear_exec_state(uint8_t statemask)
{
	CLEARFLAG(cnc_state.exec_state,statemask);
}


