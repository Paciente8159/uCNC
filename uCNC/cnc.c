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
#include "cnc.h"

cnc_state_t g_cnc_state;

void cnc_init()
{
	//initializes cnc state
	memset(&g_cnc_state, 0, sizeof(cnc_state_t));
	
	mcu_init();
	protocol_init();
	if(!settings_init())
	{
		settings_reset();
		protocol_printf(MSG_ERROR, STATUS_SETTING_READ_FAIL);
	}
	tc_init();
	parser_init();
	kinematics_init();
	mc_init();
	planner_init();
	interpolator_init();
}

void cnc_print_status_report()
{
	static uint8_t report_count = 0;
	float axis[AXIS_COUNT];
	static uint8_t report_limit = 30;
	if(g_cnc_state.exec_state & EXEC_RUN)
	{
		report_limit = 10;
	}
	
	kinematics_apply_forward(g_cnc_state.rt_position, axis);
	
	if(g_cnc_state.exec_state & EXEC_SLEEP)
	{
		protocol_append(__romstr__("<Sleep"));
	}
	else if(g_cnc_state.exec_state & EXEC_ALARM)
	{
		protocol_append(__romstr__("<Alarm"));
	}
	else if(g_cnc_state.exec_state & EXEC_HOLD)
	{
		if((g_cnc_state.exec_state & EXEC_RUN))
		{
			protocol_append(__romstr__("<Hold:1"));
		}
		else
		{
			protocol_append(__romstr__("<Hold:0"));
		}
	}
	else if(g_cnc_state.exec_state & EXEC_HOMING)
	{
		protocol_append(__romstr__("<Home"));
	}
	else if(g_cnc_state.exec_state & EXEC_JOG)
	{
		protocol_append(__romstr__("<Jog"));
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
	
	/*g_cnc_state.controls = mcu_getControls();
	g_cnc_state.limits = mcu_getLimits();
	g_cnc_state.limits |= mcu_getProbe();*/
	
	if((g_cnc_state.controls & (ESTOP_MASK | DOOR_OPEN_MASK | FHOLD_MASK)) | (g_cnc_state.limits & LIMITS_MASK))
	{
		protocol_append(__romstr__("|Pn:"));
		
		if(g_cnc_state.controls)
		{
			if(g_cnc_state.controls & ESTOP_MASK)
			{
				protocol_append(__romstr__("R"));
			}
		}
		
		if(g_cnc_state.controls)
		{
			if(g_cnc_state.controls & DOOR_OPEN_MASK)
			{
				protocol_append(__romstr__("D"));
			}
		}
		
		if(g_cnc_state.controls)
		{
			if(g_cnc_state.controls & FHOLD_MASK)
			{
				protocol_append(__romstr__("H"));
			}
		}
		
		if(g_cnc_state.limits & PROBE_MASK)
		{
			protocol_append(__romstr__("P"));
		}
		
		if(g_cnc_state.limits & LIMIT_X_MASK)
		{
			protocol_append(__romstr__("X"));
		}
		
		if(g_cnc_state.limits & LIMIT_Y_MASK)
		{
			protocol_append(__romstr__("Y"));
		}
		
		if(g_cnc_state.limits & LIMIT_Z_MASK)
		{
			protocol_append(__romstr__("Z"));
		}
		
		if(g_cnc_state.limits & LIMIT_A_MASK)
		{
			protocol_append(__romstr__("A"));
		}
		
		if(g_cnc_state.limits & LIMIT_B_MASK)
		{
			protocol_append(__romstr__("B"));
		}
		
		if(g_cnc_state.limits & LIMIT_C_MASK)
		{
			protocol_append(__romstr__("C"));
		}
	}
		
	if(report_count<report_limit)
	{
		protocol_puts(__romstr__(">\r\n\0"));
		report_count++;
	}
	else
	{
		protocol_puts(__romstr__("|WCO:0.000,0.000,0.000>\r\n\0"));
		report_count = 0;
	}
}

void cnc_reset()
{
	//clear all systems
	interpolator_clear();
	planner_clear();
	protocol_clear();

	protocol_printf(MSG_STARTUP, VERSION_NUMBER_HIGH, VERSION_NUMBER_LOW, REVISION_NUMBER);

	//initial state (ALL IS LOCKED)
	if(g_settings.homing_enabled && !g_cnc_state.is_homed)
	{
		g_cnc_state.exec_state = EXEC_ALARM;
		g_cnc_state.halt = true;
		protocol_puts(MSG_FEEDBACK_2);
	}
	else
	{
		if(g_cnc_state.controls & ESTOP_MASK)
		{
			cnc_alarm(EXEC_ALARM_ABORT_CYCLE);
			return;
		}
		g_cnc_state.exec_state = EXEC_IDLE;
	}
	g_cnc_state.rt_cmd = 0;
}

void cnc_run()
{
	cnc_reset();
	tc_home();
	for(;;)
	{
		//process gcode commands
		if(protocol_received_cmd())
		{
			uint8_t error = 0;

			if(protocol_peek() == '$') //settings command
			{	
				error = parser_grbl_command();
			}
			else if((g_cnc_state.exec_state < EXEC_JOG))
			{
				error = parser_gcode_command();
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

static void cnc_rt_commands()
{
	//reset command
	if((g_cnc_state.rt_cmd & RT_CMD_RESET))
	{
		g_cnc_state.rt_cmd &= RT_CLEAR_RESET; //erases/ignores all other rt commands
		g_cnc_state.halt = true;
		cnc_reset();
		return;
	}

	//activates hold flag
	if((g_cnc_state.rt_cmd & RT_CMD_FEED_HOLD))
	{
		if((g_cnc_state.exec_state < EXEC_HOMING)) //if not in idle run or jog ignores
		{
			g_cnc_state.rt_cmd &= RT_CLEAR_FEED_HOLD; //erases/ignores all other rt commands
			g_cnc_state.exec_state |= EXEC_HOLD; //activates hold
			return;
		}
	}
	
	//activates hold flag
	if((g_cnc_state.rt_cmd & RT_CMD_JOG_CANCEL))
	{
		if((g_cnc_state.exec_state & EXEC_JOG) && !(g_cnc_state.exec_state & EXEC_JOGCANCEL)) //if not in jog mode and cancel is not already in active
		{
			g_cnc_state.rt_cmd &= RT_CLEAR_JOG_CANCEL; //erases/ignores all other rt commands
			g_cnc_state.exec_state |= (EXEC_HOLD | EXEC_JOGCANCEL); //activates hold and jog cancel
			return;
		}
	}
	
	//activates cycle start flag
	if((g_cnc_state.rt_cmd & RT_CMD_CYCLE_START))
	{
		if(!(g_cnc_state.exec_state & EXEC_RUN))
		{
			g_cnc_state.rt_cmd &= RT_CLEAR_CYCLE_START; //erases/ignores all other rt commands
			//clears active hold
			g_cnc_state.exec_state &= ~EXEC_HOLD;
			return;
		}
	}
	
	//remaining realtime commands
	if((g_cnc_state.rt_cmd & RT_CMD_REPORT))
	{
		cnc_print_status_report();
		g_cnc_state.rt_cmd &= RT_CLEAR_REPORT;
	}
}

void cnc_doevents()
{
	cnc_rt_commands();
	
	//exit if in alarm mode and ignores all other realtime commands and exit
	if(g_cnc_state.exec_state & EXEC_ALARM)
	{
		if(!(g_cnc_state.exec_state & EXEC_HOMING) && g_cnc_state.alarm) //in homing motions all alarm messages are canceled
		{
			protocol_printf(MSG_ALARM, g_cnc_state.alarm);
			g_cnc_state.alarm = 0;
		}
		return;
	}
	
	//if not on hold state or still running or is idle
	if(!(g_cnc_state.exec_state & EXEC_HOLD) || (g_cnc_state.exec_state & EXEC_RUN) || (g_cnc_state.exec_state == EXEC_IDLE))
	{
		interpolator_execute();
	}
	
	//if motion stoped
	if(!(g_cnc_state.exec_state & EXEC_RUN))
	{
		//if in jog mode
		if((g_cnc_state.exec_state & EXEC_JOG))
		{
			//discards planner and interpolator if jog cancel was issued
			if((g_cnc_state.exec_state & EXEC_JOGCANCEL))
			{
				interpolator_clear();
				planner_clear();
				g_cnc_state.exec_state &= ~EXEC_JOGCANCEL;
			}
			
			g_cnc_state.exec_state &= ~EXEC_JOG;
			protocol_clear();
		}
	}
}

void cnc_home()
{
	g_cnc_state.exec_state |= EXEC_HOMING;
	kinematics_home();
}

void cnc_alarm(uint8_t code)
{
	g_cnc_state.exec_state |= EXEC_ALARM;
	g_cnc_state.halt = true;
	g_cnc_state.alarm = code;
}

void cnc_kill()
{
	
}

void cnc_unlock()
{
	//if emergeny stop is pressed it can't unlock
	if(g_cnc_state.controls & ESTOP_MASK)
	{
		g_cnc_state.halt = true;
		return;
	}
	
	if(g_cnc_state.exec_state & EXEC_ALARM)
	{
		g_cnc_state.exec_state &= ~EXEC_ALARM;
		g_cnc_state.halt = false;
		g_cnc_state.unlocked = false;
		protocol_puts(MSG_FEEDBACK_3);
	}
	
	if(g_cnc_state.exec_state & EXEC_HOMING)
	{
		g_cnc_state.halt = false;
	}
}
