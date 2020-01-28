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
#include "io_control.h"
#include "parser.h"
#include "kinematics.h"
#include "planner.h"
#include "cnc.h"
#include "mcu.h"
#include "protocol.h"

static void procotol_send_newline()
{
    serial_putc('\r');
    serial_putc('\n');
}

void protocol_send_ok()
{
    serial_print_str(__romstr__("ok"));
    procotol_send_newline();
}

void protocol_send_error(uint8_t error)
{
    serial_print_str(__romstr__("error:"));
    serial_print_int(error);
    procotol_send_newline();
}

void protocol_send_alarm(uint8_t alarm)
{
    serial_print_str(__romstr__("ALARM:"));
    serial_print_int(alarm);
    procotol_send_newline();
}

void protocol_send_string(const unsigned char* __s)
{
    serial_print_str(__s);
}

static uint8_t protocol_get_tools()
{
    uint8_t modalgroups[9];
    uint16_t feed;
    uint16_t spindle;

    parser_get_modes(modalgroups, &feed, &spindle);

    uint8_t result = 0;
#ifdef USE_COOLANT
    result = 9 - modalgroups[6];
#endif
#ifdef USE_SPINDLE
    if(modalgroups[5] != 5)
    {
        result |= ((modalgroups[5] == 3) ? 4 : 8);
    }
#endif
    return result;
}

static void protocol_send_status_tail()
{
    float axis[AXIS_COUNT];
    if(parser_get_wco(axis))
    {
        serial_print_str(__romstr__("|WCO:"));
        serial_print_fltarr(axis, AXIS_COUNT);
        return;
    }

    uint8_t ovr[3];
    if(planner_get_overflows(ovr))
    {
        serial_print_str(__romstr__("|Ov:"));
        serial_print_int(ovr[0]);
        serial_putc(',');
        serial_print_int(ovr[1]);
        serial_putc(',');
        serial_print_int(ovr[2]);
        uint8_t tools = protocol_get_tools();
        if(tools)
        {
            serial_print_str(__romstr__("|A:"));
            if(CHECKFLAG(tools, 4))
            {
                serial_putc('S');
            }
            if(CHECKFLAG(tools, 8))
            {
                serial_putc('C');
            }
            if(CHECKFLAG(tools, 1))
            {
                serial_putc('F');
            }

            if(CHECKFLAG(tools, 2))
            {
                serial_putc('M');
            }
        }
        return;
    }
}

void protocol_send_status()
{
    float axis[AXIS_COUNT];

    //only send report when buffer is empty
    //this prevents locks and stack overflow of the cnc_doevents()
    if(!serial_tx_is_empty())
    {
    	return;
    }

    itp_get_rt_position((float*)&axis);
    kinematics_apply_reverse_transform((float*)&axis);
    float feed = itp_get_rt_feed() * 60.0f; //convert from mm/s to mm/m
    float spindle = planner_update_spindle(false);

    uint8_t state = cnc_get_exec_state(0xFF);
    uint8_t filter = 0x80;
    while(!(state & filter) && filter)
    {
        filter >>= 1;
    }

    state &= filter;

    serial_putc('<');
    switch(state)
    {
        case EXEC_ABORT:
            serial_print_str(__romstr__("Abort"));
            break;
        case EXEC_DOOR:
            serial_print_str(__romstr__("Door:"));
            if(io_get_controls(SAFETY_DOOR_MASK))
            {

                if(cnc_get_exec_state(EXEC_RUN))
                {
                    serial_putc('2');
                }
                else
                {
                    serial_putc('1');
                }
            }
            else
            {
                if(cnc_get_exec_state(EXEC_RUN))
                {
                    serial_putc('3');
                }
                else
                {
                    serial_putc('0');
                }
            }
            break;
        case EXEC_NOHOME:
            serial_print_str(__romstr__("Alarm"));
            break;
        case EXEC_HOLD:
            serial_print_str(__romstr__("Hold:"));
            if(cnc_get_exec_state(EXEC_RUN))
            {
                serial_putc('1');
            }
            else
            {
                serial_putc('0');
            }
            break;
        case EXEC_HOMING:
            serial_print_str(__romstr__("Home"));
            break;
        case EXEC_JOG:
            serial_print_str(__romstr__("Jog"));
            break;
        case EXEC_RUN:
            serial_print_str(__romstr__("Run"));
            break;
        default:
            serial_print_str(__romstr__("Idle"));
            break;
    }

    serial_print_str(__romstr__("|MPos:"));
    serial_print_fltarr(axis, AXIS_COUNT);

#ifdef USE_SPINDLE
    serial_print_str(__romstr__("|FS:"));
#else
    serial_print_str(__romstr__("|F:"));
#endif
    serial_print_int((uint16_t)feed);
#ifdef USE_SPINDLE
    serial_putc(',');
    serial_print_int((uint16_t)spindle);
#endif

    if(io_get_controls(ESTOP_MASK | SAFETY_DOOR_MASK | FHOLD_MASK) | io_get_limits(LIMITS_MASK))
    {
        serial_print_str(__romstr__("|Pn:"));

        if(io_get_controls(ESTOP_MASK))
        {
            serial_putc('R');
        }

        if(io_get_controls(SAFETY_DOOR_MASK))
        {
            serial_putc('D');
        }

        if(io_get_controls(FHOLD_MASK))
        {
            serial_putc('H');
        }

        if(io_get_probe())
        {
            serial_putc('P');
        }

        if(io_get_limits(LIMIT_X_MASK))
        {
            serial_putc('X');
        }

        if(io_get_limits(LIMIT_Y_MASK))
        {
            serial_putc('Y');
        }

        if(io_get_limits(LIMIT_Z_MASK))
        {
            serial_putc('Z');
        }

        if(io_get_limits(LIMIT_A_MASK))
        {
            serial_putc('A');
        }

        if(io_get_limits(LIMIT_B_MASK))
        {
            serial_putc('B');
        }

        if(io_get_limits(LIMIT_C_MASK))
        {
            serial_putc('C');
        }
    }

    protocol_send_status_tail();
    /*
    #ifdef __PERFSTATS__
    uint16_t stepclocks = mcu_get_step_clocks();
    uint16_t stepresetclocks = mcu_get_step_reset_clocks();
    protocol_printf(__romstr__("|Perf:%d,%d"), stepclocks, stepresetclocks);
    #endif
    */
    serial_putc('>');
    procotol_send_newline();
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
        serial_putc(']');
        procotol_send_newline();
    }

    for(uint8_t i = 6; i < COORD_SYS_COUNT; i++)
    {
        serial_print_int(i - 5);
        serial_putc(':');
        serial_print_fltarr(parser_get_coordsys(i), AXIS_COUNT);
        serial_putc(']');
        procotol_send_newline();
    }

    serial_print_str(__romstr__("[G28:"));
    serial_print_fltarr(parser_get_coordsys(28), AXIS_COUNT);
    serial_putc(']');
    procotol_send_newline();

    serial_print_str(__romstr__("[G30:"));
    serial_print_fltarr(parser_get_coordsys(30), AXIS_COUNT);
    serial_putc(']');
    procotol_send_newline();

    serial_print_str(__romstr__("[G92:"));
    serial_print_fltarr(parser_get_coordsys(92), AXIS_COUNT);
    serial_putc(']');
    procotol_send_newline();

    serial_print_str(__romstr__("[PRB:"));
    serial_print_fltarr(parser_get_coordsys(255), AXIS_COUNT);
    serial_putc(':');
    serial_putc('0' + parser_get_probe_result());
    serial_putc(']');
    procotol_send_newline();
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
        serial_print_int((int16_t)modalgroups[i]);
        serial_putc(' ');
    }

    for(uint8_t i = 5; i < 8; i++)
    {
        serial_putc('M');
        serial_print_int((int16_t)((i==6 && modalgroups[i]==6) ? 7 : modalgroups[i]));
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

    serial_putc(']');
    procotol_send_newline();
}

static void protocol_send_gcode_setting_line_int(uint8_t setting, uint16_t value)
{
    serial_putc('$');
    serial_print_int(setting);
    serial_putc('=');
    serial_print_int(value);
    procotol_send_newline();
}

static void protocol_send_gcode_setting_line_flt(uint8_t setting, float value)
{
    serial_putc('$');
    serial_print_int(setting);
    serial_putc('=');
    serial_print_flt(value);
    procotol_send_newline();
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
