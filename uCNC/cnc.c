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
	settings_load();
	parser_init();
	kinematics_init();
	mc_init();
	planner_init();
	interpolator_init();
	
}

void cnc_reset()
{
	//initial state (ALL IS LOCKED)
	g_cnc_state.exec_state = EXEC_ALARM;
	g_cnc_state.halt = true;
	g_cnc_state.rt_cmd = 0;
	cnc_unhome();
}

void cnc_run_system_cmd()
{
	//eats '$' char
	protocol_getc();
	
	switch(protocol_peek())
	{
		case 'X':
			g_cnc_state.exec_state &= ~EXEC_ALARM;
			protocol_clear();
			protocol_puts(MSG_FEEDBACK_3);
			break;
	}
}

void cnc_run()
{
	protocol_printf(MSG_STARTUP, VERSION_NUMBER_HIGH, VERSION_NUMBER_LOW, REVISION_NUMBER);
	protocol_puts(MSG_FEEDBACK_2);
	
	for(;;)
	{
		//process gcode commands
		if(protocol_received_cmd())
		{
			uint8_t error = 0;
			if(protocol_peek() == '$') //settings command
			{
				cnc_run_system_cmd();
			}
			else if(!CHECKFLAG(g_cnc_state.exec_state, EXEC_ALARM))
			{
				if(parser_is_ready())
				{
					error = parser_parse_command();
					if(error)
					{
						protocol_clear();
						protocol_printf(MSG_ERROR, error);
					}
				}
			}
			else
			{
				error = STATUS_SYSTEM_GC_LOCK;
				protocol_clear();
				protocol_printf(MSG_ERROR, error);
			}
			
			if(!error)
			{
				protocol_clear();
				protocol_puts(MSG_OK);
			}
		}
		
		if(CHECKFLAG(g_cnc_state.rt_cmd, RT_CMD_RESET))
		{
			//resets everything
			cnc_reset();
			return;
		} 

		cnc_doevents();
		
	}
}

void cnc_doevents()
{
	//process realtime commands
	switch(g_cnc_state.rt_cmd)
	{
		case RT_CMD_REPORT:
			parser_print_states();
			g_cnc_state.rt_cmd &= ~RT_CMD_REPORT;
			break;
	}
	
	if(!CHECKFLAG(g_cnc_state.exec_state, EXEC_ALARM))
	{
		//cycle execution instructions
		if(g_cnc_state.exec_state && EXEC_CYCLE)
		{
			interpolator_execute();
		}
	}
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

void cnc_kill()
{
	
}
