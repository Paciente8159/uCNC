#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "config.h"
#include "utils.h"
#include "settings.h"
#include "mcu.h"
#include "grbl_interface.h"
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
	protocol_init();
	if(!settings_init())
	{
		settings_reset();
		protocol_printf(MSG_ERROR, STATUS_SETTING_READ_FAIL);
	}
	
	mc_init();
	if(!parser_init())
	{
		parser_parameters_reset();
		protocol_printf(MSG_ERROR, STATUS_SETTING_READ_FAIL);
	}
	
	kinematics_init();
	planner_init();
	tc_init();
	interpolator_init();
	cnc_state.unlocked = false;
}

void cnc_print_status_report()
{
	static uint8_t report_count = 0;
	float axis[AXIS_COUNT];
	static uint8_t report_limit = 30;
	
	if(cnc_state.exec_state & EXEC_RUN)
	{
		report_limit = 10;
	}
	
	interpolator_get_rt_position((float*)&axis);
	
	if(cnc_state.exec_state & EXEC_SLEEP)
	{
		protocol_append(__romstr__("<Sleep"));
	}
	else if(cnc_state.exec_state & EXEC_ALARM)
	{
		protocol_append(__romstr__("<Alarm"));
	}
	else if(cnc_state.exec_state & EXEC_DOOR)
	{
		if(tc_get_controls(SAFETY_DOOR_MASK))
		{
			if((cnc_state.exec_state & EXEC_RUN))
			{
				protocol_append(__romstr__("<Door:2"));
			}
			else
			{
				protocol_append(__romstr__("<Door:1"));
			}
		}
		else
		{
			if((cnc_state.exec_state & EXEC_RUN))
			{
				protocol_append(__romstr__("<Door:3"));
			}
			else
			{
				protocol_append(__romstr__("<Door:0"));
			}
		}
	}
	else if(cnc_state.exec_state & EXEC_HOMING)
	{
		protocol_append(__romstr__("<Home"));
	}
	else if(cnc_state.exec_state & EXEC_JOG)
	{
		protocol_append(__romstr__("<Jog"));
	}
	else if(cnc_state.exec_state & EXEC_HOLD)
	{
		if((cnc_state.exec_state & EXEC_RUN))
		{
			protocol_append(__romstr__("<Hold:1"));
		}
		else
		{
			protocol_append(__romstr__("<Hold:0"));
		}
	}
	else if(cnc_state.exec_state & EXEC_RUN)
	{
		protocol_append(__romstr__("<Run"));
	}
	else
	{
		protocol_append(__romstr__("<Idle"));
	}
		
	protocol_append(__romstr__("|MPos:"));
	for(uint8_t i = 0; i < AXIS_COUNT -1; i++)
	{
		protocol_appendf(__romstr__("%0.3f,"), axis[i]);
	}
	
	protocol_appendf(__romstr__("%0.3f"), axis[AXIS_COUNT -1]);
	
	protocol_append(__romstr__("|FS:0,0"));
	
	/*cnc_state.controls = mcu_getControls();
	cnc_state.limits = mcu_getLimits();
	cnc_state.limits |= mcu_getProbe();*/
	
	if(tc_get_controls(ESTOP_MASK | SAFETY_DOOR_MASK | FHOLD_MASK) | tc_get_limits(LIMITS_MASK))
	{
		protocol_append(__romstr__("|Pn:"));
		
		if(tc_get_controls(ESTOP_MASK))
		{
			protocol_append(__romstr__("R"));
		}
		
		if(tc_get_controls(SAFETY_DOOR_MASK))
		{
			protocol_append(__romstr__("D"));
		}
		
		if(tc_get_controls(FHOLD_MASK))
		{
			protocol_append(__romstr__("H"));
		}
		
		if(tc_get_probe())
		{
			protocol_append(__romstr__("P"));
		}
		
		if(tc_get_limits(LIMIT_X_MASK))
		{
			protocol_append(__romstr__("X"));
		}
		
		if(tc_get_limits(LIMIT_Y_MASK))
		{
			protocol_append(__romstr__("Y"));
		}
		
		if(tc_get_limits(LIMIT_Z_MASK))
		{
			protocol_append(__romstr__("Z"));
		}
		
		if(tc_get_limits(LIMIT_A_MASK))
		{
			protocol_append(__romstr__("A"));
		}
		
		if(tc_get_limits(LIMIT_B_MASK))
		{
			protocol_append(__romstr__("B"));
		}
		
		if(tc_get_limits(LIMIT_C_MASK))
		{
			protocol_append(__romstr__("C"));
		}
	}
		
	if(report_count>report_limit)
	{
		parser_get_wco(axis);
		protocol_append(__romstr__("|WCO:"));
		for(uint8_t i = 0; i < AXIS_COUNT -1; i++)
		{
			protocol_appendf(__romstr__("%0.3f,"), axis[i]);
		}
		
		protocol_appendf(__romstr__("%0.3f"), axis[AXIS_COUNT -1]);
		report_count = 0;
	}
	
	protocol_puts(__romstr__(">\r\n\0"));
	report_count++;
}

void cnc_reset()
{
	//clear all systems
	interpolator_clear();
	planner_clear();
	protocol_clear();
			
	protocol_printf(MSG_STARTUP, VERSION_NUMBER_HIGH, VERSION_NUMBER_LOW, REVISION_NUMBER);

	//initial state (ALL IS LOCKED)
	if(g_settings.homing_enabled && cnc_state.homed != HOME_OK )
	{
		cnc_state.exec_state = EXEC_ALARM;
		cnc_state.unlocked = false;
		protocol_puts(MSG_FEEDBACK_2);
	}
	else
	{
		cnc_unlock();
	}

	cnc_state.reset = false;
}

void cnc_run()
{
	cnc_reset();
	
	//protocol_inject_cmd(__romstr__("$J=G91F100X1"));
	for(;;)
	{
		if(cnc_state.reset)
		{
			return;
		}
		
		//process gcode commands
		if(protocol_received_cmd())
		{
			uint8_t error = 0;

			if(protocol_peek() == '$') //settings command
			{	
				if(!(cnc_state.exec_state & EXEC_RUN) || (cnc_state.exec_state & EXEC_JOG)) //if it is not running or is in jog otherwise ignore
				{
					error = parser_grbl_command();
				}
			}
			else if((cnc_state.exec_state < EXEC_JOG) && cnc_state.unlocked)
			{
				error = parser_gcode_command(false);
			}
			else
			{
				error = STATUS_SYSTEM_GC_LOCK;
			}
			
			//clear buffer remaining buffer chars
			protocol_clear();
			
			if(!error)
			{
				protocol_puts(MSG_OK);
			}
			else
			{
				
				protocol_printf(MSG_ERROR, error);
			}
		}
		
		cnc_doevents();
	}
}

void cnc_exec_rt_command(uint8_t command)
{
	switch(command)
	{
		case RT_CMD_RESET:
			if(cnc_state.exec_state & EXEC_HOMING) //if homing
			{
				cnc_state.exec_state &= ~EXEC_HOMING; //clear homing flag that supress alarms
				cnc_alarm(EXEC_ALARM_HOMING_FAIL_RESET);
			}
			//imediatly calls killing process
			cnc_kill();
			cnc_state.reset = true;			
			break;
		case RT_CMD_SAFETY_DOOR:
			cnc_state.exec_state |= EXEC_DOOR;
			if(cnc_state.exec_state & EXEC_HOMING) //if homing
			{
				cnc_state.exec_state &= ~EXEC_HOMING; //clear homing flag that supress alarms
				cnc_kill();
				cnc_alarm(EXEC_ALARM_HOMING_FAIL_DOOR);
			}
			else
			{
				cnc_state.exec_state |= EXEC_HOLD;
			}
			break;
		case RT_CMD_JOG_CANCEL:
			if((cnc_state.exec_state & EXEC_JOG)) //if not jog mode ignores
			{
				cnc_state.exec_state |= EXEC_HOLD; //activates hold
			}
			break;
		case RT_CMD_FEED_HOLD:
			if((cnc_state.exec_state < EXEC_HOMING)) //if not in idle, run or jog ignores
			{
				cnc_state.exec_state |= EXEC_HOLD; //activates hold
			}
			break;
		case RT_CMD_CYCLE_START:
			if(cnc_state.exec_state > EXEC_HOLD)
			{
				return;
			}
			
			if(!(cnc_state.exec_state & EXEC_RUN))
			{
				//clears active hold
				cnc_state.exec_state &= ~EXEC_HOLD;
			}
			break;
		case RT_CMD_REPORT:
			cnc_state.send_report = true;
			break;	
	}
	
	//cnc_state.rt_cmd = 0;
}

void cnc_doevents()
{
	//send report if asked
	if(cnc_state.send_report && protocol_sent_resp())
	{
		cnc_print_status_report();
		cnc_state.send_report = false;
	}
	
	//exit if in alarm mode and ignores all other realtime commands and exit
	if(cnc_state.exec_state & EXEC_ALARM)
	{
		if(cnc_state.active_alarm) //active alarm message
		{
			protocol_printf(MSG_ALARM, cnc_state.active_alarm);
			cnc_state.active_alarm = 0;
		}
		return;
	}
	
	if(!cnc_state.unlocked) //cnc is locked
	{
		return; //leaves the interpolator dry
	}
	
	if(cnc_state.exec_state & EXEC_HOLD) //there is an active hold
	{
		//hold has come to a full stop
		if(!(cnc_state.exec_state & EXEC_RUN) && (cnc_state.exec_state & EXEC_HOLD))
		{
			//if homing or jog flushes buffers
			if((cnc_state.exec_state & EXEC_HOMING) || (cnc_state.exec_state & EXEC_JOG))
			{
				interpolator_stop();
				interpolator_clear();
				planner_clear();
				//can clears any active hold, homing or jog
				cnc_state.exec_state &= ~(EXEC_HOLD | EXEC_HOMING | EXEC_JOG);
			}
		}
		
		return; //leaves the interpolator dry
	}
	
	interpolator_run();
}

void cnc_home()
{
	cnc_state.exec_state |= EXEC_HOMING;
	kinematics_home();
}

void cnc_alarm(uint8_t code)
{
	if(!cnc_get_exec_state(EXEC_HOMING)) //while homing supress all alarms
	{
		cnc_state.exec_state = EXEC_ALARM;
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
	interpolator_stop();
	//halt is active and was running flags it lost home position
	if((cnc_state.exec_state & EXEC_RUN) & !cnc_state.unlocked)
	{
		cnc_state.homed |= HOME_NEEDS_REHOME;
	}
	cnc_state.exec_state &= ~EXEC_RUN; //signals all motions have stoped
}

void cnc_safe_stop()
{
	interpolator_stop();
	//halt is active and was running flags it lost home position
	if((cnc_state.exec_state & EXEC_RUN) & !cnc_state.unlocked)
	{
		cnc_state.homed |= HOME_NEEDS_REHOME;
	}
	cnc_state.exec_state &= ~EXEC_RUN; //signals all motions have stoped
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
	
	if(cnc_state.exec_state & EXEC_ALARM)
	{
		protocol_puts(MSG_FEEDBACK_3);
	}
	
	cnc_state.exec_state = EXEC_IDLE;
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
	} while(cnc_state.exec_state & EXEC_RUN);
	
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
		if(!tc_get_limits(axis_limit) && !(cnc_state.exec_state & EXEC_HOLD))
		{
			cnc_state.exec_state |= EXEC_HOLD; 
		}
	} while(cnc_state.exec_state & EXEC_RUN);
	
	//stops, flushes buffers and clears the hold if active
	cnc_stop();
	interpolator_clear();
	planner_clear();
	cnc_state.exec_state &= ~EXEC_HOLD;
	
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
	} while(cnc_state.exec_state & EXEC_RUN);
}

void cnc_reset_position()
{
	interpolator_reset_rt_position();
	planner_resync_position();
}

