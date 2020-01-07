/*
	Name: protocol.h
	Description: uCNC implementation of a Grbl compatible send-response protocol
	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 19/09/2019

	uCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	uCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "config.h"
#include "utils.h"
#include "settings.h"
#include "serial.h"
#include "interpolator.h"
#include "dio_control.h"
#include "parser.h"
#include "cnc.h"
#include "mcu.h"
#include "protocol.h"

void protocol_send_ok()
{
	serial_print_str(__romstr__("ok\r\n"));
}

void protocol_send_error(uint8_t error)
{
	serial_print_str(__romstr__("error:"));
	serial_print_int(error);
	serial_print_str(__romstr__("\r\n"));
}

void protocol_send_alarm(uint8_t alarm)
{
	serial_print_str(__romstr__("ALARM:"));
	serial_print_int(alarm);
	serial_print_str(__romstr__("\r\n"));
}

void protocol_send_string(const char* __s)
{
	serial_print_str(__s);
}

void protocol_send_status()
{
	static uint8_t report_count = 0;
	float axis[AXIS_COUNT];
	static uint8_t report_limit = 30;

	//only send report when buffer is empty
	//this prevents locks and stack overflow of the cnc_doevents()
	if(!serial_tx_is_empty())
	{
		return;
	}

	if(cnc_get_exec_state(EXEC_RUN))
	{
		report_limit = 10;
	}
	
	interpolator_get_rt_position((float*)&axis);
	float feed = interpolator_get_rt_feed() * 60.0f; //convert from mm/s to mm/m
	
	uint8_t state = cnc_get_exec_state(0xFF);
	uint8_t filter = EXEC_SLEEP;
	while(!(state & filter) && filter)
	{
		filter >>= 1;
	}
	
	state &= filter;
	
	switch(state)
	{
		case EXEC_SLEEP:
			serial_print_str(__romstr__("<Sleep"));
			break;
		case EXEC_DOOR:
			if(dio_get_controls(SAFETY_DOOR_MASK))
			{
				if(cnc_get_exec_state(EXEC_RUN))
				{
					serial_print_str(__romstr__("<Door:2"));
				}
				else
				{
					serial_print_str(__romstr__("<Door:1"));
				}
			}
			else
			{
				if(cnc_get_exec_state(EXEC_RUN))
				{
					serial_print_str(__romstr__("<Door:3"));
				}
				else
				{
					serial_print_str(__romstr__("<Door:0"));
				}
			}
			break;
		case EXEC_ALARM:
			serial_print_str(__romstr__("<Alarm"));
			break;
		case EXEC_HOLD:
			if(cnc_get_exec_state(EXEC_RUN))
			{
				serial_print_str(__romstr__("<Hold:1"));
			}
			else
			{
				serial_print_str(__romstr__("<Hold:0"));
			}
			break;
		case EXEC_HOMING:
			serial_print_str(__romstr__("<Home"));
			break;
		case EXEC_JOG:
			serial_print_str(__romstr__("<Jog"));
			break;
		case EXEC_RUN:
			serial_print_str(__romstr__("<Run"));
			break;
		default:
			serial_print_str(__romstr__("<Idle"));
			break;
	}
	/*
	if(cnc_get_exec_state(EXEC_SLEEP))
	{
		serial_print_str(__romstr__("<Sleep"));
	}
	else if(cnc_get_exec_state(EXEC_ALARM))
	{
		serial_print_str(__romstr__("<Alarm"));
	}
	else if(cnc_get_exec_state(EXEC_DOOR))
	{
		if(dio_get_controls(SAFETY_DOOR_MASK))
		{
			if(cnc_get_exec_state(EXEC_RUN))
			{
				serial_print_str(__romstr__("<Door:2"));
			}
			else
			{
				serial_print_str(__romstr__("<Door:1"));
			}
		}
		else
		{
			if(cnc_get_exec_state(EXEC_RUN))
			{
				serial_print_str(__romstr__("<Door:3"));
			}
			else
			{
				serial_print_str(__romstr__("<Door:0"));
			}
		}
	}
	else if(cnc_get_exec_state(EXEC_HOMING))
	{
		serial_print_str(__romstr__("<Home"));
	}
	else if(cnc_get_exec_state(EXEC_JOG))
	{
		serial_print_str(__romstr__("<Jog"));
	}
	else if(cnc_get_exec_state(EXEC_HOLD))
	{
		if(cnc_get_exec_state(EXEC_RUN))
		{
			serial_print_str(__romstr__("<Hold:1"));
		}
		else
		{
			serial_print_str(__romstr__("<Hold:0"));
		}
	}
	else if(cnc_get_exec_state(EXEC_RUN))
	{
		serial_print_str(__romstr__("<Run"));
	}
	else
	{
		serial_print_str(__romstr__("<Idle"));
	}
	*/	
	serial_print_str(__romstr__("|MPos:"));
	serial_print_fltarr(axis, AXIS_COUNT);
	
	serial_print_str(__romstr__("|FS:"));
	serial_print_int((uint16_t)feed);
	serial_putc(',');
	serial_putc('0');

	if(dio_get_controls(ESTOP_MASK | SAFETY_DOOR_MASK | FHOLD_MASK) | dio_get_limits(LIMITS_MASK))
	{
		serial_print_str(__romstr__("|Pn:"));
		
		if(dio_get_controls(ESTOP_MASK))
		{
			serial_putc('R');
		}
		
		if(dio_get_controls(SAFETY_DOOR_MASK))
		{
			serial_putc('D');
		}
		
		if(dio_get_controls(FHOLD_MASK))
		{
			serial_putc('H');
		}
		
		if(dio_get_probe())
		{
			serial_putc('P');
		}
		
		if(dio_get_limits(LIMIT_X_MASK))
		{
			serial_putc('X');
		}
		
		if(dio_get_limits(LIMIT_Y_MASK))
		{
			serial_putc('Y');
		}
		
		if(dio_get_limits(LIMIT_Z_MASK))
		{
			serial_putc('Z');
		}
		
		if(dio_get_limits(LIMIT_A_MASK))
		{
			serial_putc('A');
		}
		
		if(dio_get_limits(LIMIT_B_MASK))
		{
			serial_putc('B');
		}
		
		if(dio_get_limits(LIMIT_C_MASK))
		{
			serial_putc('C');
		}
	}
	/*	
	if(report_count>report_limit)
	{
		parser_get_wco(axis);
		serial_print_string(__romstr__("|WCO:"));
		for(uint8_t i = 0; i < AXIS_COUNT-1; i++)
		{
			protocol_printf(__romstr__("%0.3f,"), axis[i]);
		}
		
		protocol_printf(__romstr__("%0.3f"), axis[AXIS_COUNT-1]);
		report_count = 0;
	}
	
	#ifdef __PERFSTATS__
	uint16_t stepclocks = mcu_get_step_clocks();
	uint16_t stepresetclocks = mcu_get_step_reset_clocks();
	protocol_printf(__romstr__("|Perf:%d,%d"), stepclocks, stepresetclocks);
	#endif
	*/
	serial_print_str(__romstr__(">\r\n"));
	report_count++;
}

void protocol_send_gcode_coordsys()
{
	uint8_t coordlimit = MIN(6, COORD_SYS_COUNT);
	for(uint8_t i = 0; i < coordlimit; i++)
	{
		float* axis = parser_get_coordsys(i);
		serial_print_str(__romstr__("[G"));
		serial_print_int(i + 54);
		serial_putc(':');
		serial_print_fltarr(parser_get_coordsys(i), AXIS_COUNT);
		serial_print_str(__romstr__("]\r\n"));
	}
	
	for(uint8_t i = 6; i < COORD_SYS_COUNT; i++)
	{
		serial_print_int(i - 5);
		serial_putc(':');
		serial_print_fltarr(parser_get_coordsys(i), AXIS_COUNT);
		serial_print_str(__romstr__("]\r\n"));
	}
	
	serial_print_str(__romstr__("[G28:"));
	serial_print_fltarr(parser_get_coordsys(28), AXIS_COUNT);
	serial_print_str(__romstr__("]\r\n"));
	
	serial_print_str(__romstr__("[G30:"));
	serial_print_fltarr(parser_get_coordsys(30), AXIS_COUNT);
	serial_print_str(__romstr__("]\r\n"));
	
	serial_print_str(__romstr__("[G92:"));
	serial_print_fltarr(parser_get_coordsys(92), AXIS_COUNT);
	serial_print_str(__romstr__("]\r\n"));
}

void protocol_send_gcode_modes()
{
	uint8_t modalgroups[9];
	uint16_t feed;
	uint16_t spindle;
	
	parser_get_modes(modalgroups, &feed, &spindle);
	
	serial_print_str(__romstr__("[GC:"));
	
	for(uint8_t i = 0; i < 5; i++)
	{
		serial_putc('G');
		serial_print_int(modalgroups[i]);
		serial_putc(' ');
	}
	
	for(uint8_t i = 5; i < 8; i++)
	{
		serial_putc('M');
		serial_print_int(modalgroups[i]);
		serial_putc(' ');
	}
	
	serial_putc('T');
	serial_print_int(modalgroups[8]);
	serial_putc(' ');
	
	serial_putc('F');
	serial_print_int(feed);
	serial_putc(' ');
	
	serial_putc('S');
	serial_print_int(spindle);

	serial_print_str(__romstr__("]\r\n"));
}

static void protocol_send_gcode_setting_line_int(uint8_t setting, uint16_t value)
{
	serial_putc('$');
	serial_print_int(setting);
	serial_putc('=');
	serial_print_int(value);
	serial_putc('\r');
	serial_putc('\n');
}

static void protocol_send_gcode_setting_line_flt(uint8_t setting, float value)
{
	serial_putc('$');
	serial_print_int(setting);
	serial_putc('=');
	serial_print_flt(value);
	serial_putc('\r');
	serial_putc('\n');
}

void protocol_send_gcode_settings()
{
	protocol_send_gcode_setting_line_int(0, g_settings.max_step_rate);
	protocol_send_gcode_setting_line_int(2, g_settings.step_invert_mask);
	protocol_send_gcode_setting_line_int(3, g_settings.dir_invert_mask);
	protocol_send_gcode_setting_line_int(4, g_settings.step_enable_invert);
	protocol_send_gcode_setting_line_int(5, g_settings.limits_invert_mask);
	protocol_send_gcode_setting_line_int(7, g_settings.control_invert_mask);
	protocol_send_gcode_setting_line_int(10, g_settings.status_report_mask);
	protocol_send_gcode_setting_line_flt(12, g_settings.arc_tolerance);
	protocol_send_gcode_setting_line_int(20, g_settings.soft_limits_enabled);
	protocol_send_gcode_setting_line_int(21, g_settings.hard_limits_enabled);
	protocol_send_gcode_setting_line_int(22, g_settings.homing_enabled);
	protocol_send_gcode_setting_line_int(23, g_settings.homing_dir_invert_mask);
	protocol_send_gcode_setting_line_flt(24, g_settings.homing_slow_feed_rate);
	protocol_send_gcode_setting_line_flt(25, g_settings.homing_fast_feed_rate);
	protocol_send_gcode_setting_line_flt(27, g_settings.homing_offset);
	protocol_send_gcode_setting_line_flt(30, g_settings.spindle_max_rpm);
	protocol_send_gcode_setting_line_flt(31, g_settings.spindle_min_rpm);
	
	for(uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		protocol_send_gcode_setting_line_flt(100 + i , g_settings.step_per_mm[i]);
	}
	
	for(uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		protocol_send_gcode_setting_line_flt(110 + i , g_settings.max_feed_rate[i]);
	}
	
	for(uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		protocol_send_gcode_setting_line_flt(120 + i , g_settings.acceleration[i]);
	}
	
	for(uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		protocol_send_gcode_setting_line_flt(130 + i , g_settings.max_distance[i]);
	}
}
