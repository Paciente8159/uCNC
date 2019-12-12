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
	parser_init();
	kinematics_init();
	mc_init();
	planner_init();
	interpolator_init();
	cnc_unhome();
}

void cnc_reset()
{
	protocol_clear();
	protocol_printf(MSG_STARTUP, VERSION_NUMBER_HIGH, VERSION_NUMBER_LOW, REVISION_NUMBER);
	//initial state (ALL IS LOCKED)
	if(g_settings.homing_enabled)
	{
		g_cnc_state.exec_state = EXEC_ALARM;
		g_cnc_state.halt = true;
		protocol_puts(MSG_FEEDBACK_2);
	}
	else
	{
		g_cnc_state.exec_state = EXEC_IDLE;
	}
	g_cnc_state.rt_cmd = 0;
}

void cnc_run()
{
	cnc_reset();

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
			else if(!CHECKFLAG(g_cnc_state.exec_state, EXEC_ALARM))
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

void cnc_doevents()
{
	//reset command
	if(CHECKFLAG(g_cnc_state.rt_cmd, RT_CMD_RESET))
	{
		cnc_reset();
		return;
	}
	
	//exit if in alarm mode and ignores all other realtime commands
	if(!CHECKFLAG(g_cnc_state.exec_state, EXEC_ALARM))
	{
		return;
	}
	
	//remaining realtime commands
	if(CHECKFLAG(g_cnc_state.rt_cmd, RT_CMD_REPORT))
	{
		CLEARFLAG(g_cnc_state.rt_cmd, RT_CMD_REPORT);
		parser_print_states();
	}
	
	//cycle execution instructions
	if(CHECKFLAG(g_cnc_state.exec_state, EXEC_RUN))
	{
		interpolator_execute();
	}
}

void cnc_home()
{
	g_cnc_state.exec_state |= EXEC_HOMING;
	kinematics_home();
}

//resets homing state (clears flag and sets step position to middle)
void cnc_unhome()
{
	g_cnc_state.is_homed = false;
	for(uint8_t i = AXIS_COUNT; i !=0;)
	{
		i--;
		g_cnc_state.rt_position[i] = (UINT32_MAX >> 1);
	}
}

void cnc_alarm(uint8_t code)
{
	g_cnc_state.exec_state = EXEC_ALARM;
	protocol_printf(MSG_ALARM, code);
}

void cnc_kill()
{
	
}

void cnc_unlock()
{
	g_cnc_state.exec_state = EXEC_IDLE;
	g_cnc_state.halt = false;
	g_cnc_state.unlocked = false;
	protocol_puts(MSG_FEEDBACK_3);
}
