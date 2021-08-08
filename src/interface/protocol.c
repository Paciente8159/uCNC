/*
	Name: protocol.h
	Description: µCNC implementation of a Grbl compatible send-response protocol
	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 19/09/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifdef __cplusplus
extern "C"
{
#endif

#include "cnc.h"
#include "interface/settings.h"
#include "interface/serial.h"
#include "core/interpolator.h"
#include "core/io_control.h"
#include "core/motion_control.h"
#include "core/parser.h"
#include "core/planner.h"
#include "protocol.h"
#include "interface/grbl_interface.h"

    static bool protocol_busy;

    bool protocol_is_busy(void)
    {
        return protocol_busy;
    }

    static void procotol_send_newline(void)
    {
        serial_print_str(MSG_EOL);
    }

    void protocol_send_ok(void)
    {
        protocol_busy = true;
        serial_print_str(MSG_OK);
        procotol_send_newline();
        protocol_busy = false;
    }

    void protocol_send_error(uint8_t error)
    {
        protocol_busy = true;
        serial_print_str(MSG_ERROR);
        serial_print_int(error);
        procotol_send_newline();
        protocol_busy = false;
    }

    void protocol_send_alarm(int8_t alarm)
    {
        protocol_busy = true;
        serial_print_str(MSG_ALARM);
        serial_print_int(alarm);
        procotol_send_newline();
        protocol_busy = false;
    }

    void protocol_send_string(const unsigned char *__s)
    {
        protocol_busy = true;
        serial_print_str(__s);
        protocol_busy = false;
    }

    void protocol_send_feedback(const unsigned char *__s)
    {
        protocol_busy = true;
        serial_print_str(MSG_START);
        serial_print_str(__s);
        serial_print_str(MSG_END);
        protocol_busy = false;
    }

    static uint8_t protocol_get_tools(void)
    {
        uint8_t modalgroups[12];
        uint16_t feed;
        uint16_t spindle;
        uint8_t coolant;

        parser_get_modes(modalgroups, &feed, &spindle, &coolant);

#ifdef USE_SPINDLE
        if (modalgroups[8] != 5)
        {
            coolant |= ((modalgroups[8] == 3) ? 4 : 8);
        }
#endif
        return coolant;
    }

    static void protocol_send_status_tail(void)
    {
        float axis[MAX(AXIS_COUNT, 3)];
        if (parser_get_wco(axis))
        {
            serial_print_str(MSG_STATUS_WCO);
            serial_print_fltarr(axis, AXIS_COUNT);
            return;
        }

        uint8_t ovr[3];
        if (planner_get_overflows(ovr))
        {
            serial_print_str(MSG_STATUS_OVR);
            serial_print_int(ovr[0]);
            serial_putc(',');
            serial_print_int(ovr[1]);
            serial_putc(',');
            serial_print_int(ovr[2]);
            uint8_t tools = protocol_get_tools();
            if (tools)
            {
                serial_print_str(MSG_STATUS_TOOL);
                if (CHECKFLAG(tools, 4))
                {
                    serial_putc('S');
                }
                if (CHECKFLAG(tools, 8))
                {
                    serial_putc('C');
                }
                if (CHECKFLAG(tools, COOLANT_MASK))
                {
                    serial_putc('F');
                }

                if (CHECKFLAG(tools, MIST_MASK))
                {
                    serial_putc('M');
                }
            }
            return;
        }
    }

    void protocol_send_status(void)
    {
        protocol_busy = true;
        float axis[MAX(AXIS_COUNT, 3)];

        uint32_t steppos[STEPPER_COUNT];
        itp_get_rt_position(steppos);
        kinematics_apply_forward(steppos, axis);
        kinematics_apply_reverse_transform(axis);
        float feed = itp_get_rt_feed(); //convert from mm/s to mm/m
#ifdef USE_SPINDLE
        uint16_t spindle = itp_get_rt_spindle();
#endif
        uint8_t controls = io_get_controls();
        uint8_t limits = io_get_limits();
        bool probe = io_get_probe();
        uint8_t state = cnc_get_exec_state(0xFF);
        uint8_t filter = 0x80;
        while (!(state & filter) && filter)
        {
            filter >>= 1;
        }

        state &= filter;

        serial_putc('<');
        if (!mc_get_checkmode() || cnc_get_exec_state(EXEC_ALARM))
        {
            switch (state)
            {
            case EXEC_KILL:
                serial_print_str(MSG_STATUS_ALARM);
                break;
            case EXEC_DOOR:
                serial_print_str(MSG_STATUS_DOOR);
                if (CHECKFLAG(controls, SAFETY_DOOR_MASK))
                {

                    if (cnc_get_exec_state(EXEC_RUN))
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
                    if (cnc_get_exec_state(EXEC_RUN))
                    {
                        serial_putc('3');
                    }
                    else
                    {
                        serial_putc('0');
                    }
                }
                break;
            case EXEC_HALT:
                serial_print_str(MSG_STATUS_ALARM);
                break;
            case EXEC_HOLD:
                serial_print_str(MSG_STATUS_HOLD);
                if (cnc_get_exec_state(EXEC_RUN))
                {
                    serial_putc('1');
                }
                else
                {
                    serial_putc('0');
                }
                break;
            case EXEC_HOMING:
                serial_print_str(MSG_STATUS_HOME);
                break;
            case EXEC_JOG:
                serial_print_str(MSG_STATUS_JOG);
                break;
            case EXEC_RESUMING:
            case EXEC_RUN:
                serial_print_str(MSG_STATUS_RUN);
                break;
            default:
                serial_print_str(MSG_STATUS_IDLE);
                break;
            }
        }
        else
        {
            serial_print_str(MSG_STATUS_CHECK);
        }

        serial_print_str(MSG_STATUS_MPOS);
        serial_print_fltarr(axis, AXIS_COUNT);

#ifdef USE_SPINDLE
        serial_print_str(MSG_STATUS_FS);
#else
    serial_print_str(MSG_STATUS_F);
#endif
        serial_print_fltunits(feed);
#ifdef USE_SPINDLE
        serial_putc(',');
        serial_print_int(spindle);
#endif

#ifdef GCODE_PROCESS_LINE_NUMBERS
        serial_print_str(MSG_STATUS_LINE);
        serial_print_long(itp_get_rt_line_number());
#endif

        if (CHECKFLAG(controls, (ESTOP_MASK | SAFETY_DOOR_MASK | FHOLD_MASK)) || CHECKFLAG(limits, LIMITS_MASK) || probe)
        {
            serial_print_str(MSG_STATUS_PIN);

            if (CHECKFLAG(controls, ESTOP_MASK))
            {
                serial_putc('R');
            }

            if (CHECKFLAG(controls, SAFETY_DOOR_MASK))
            {
                serial_putc('D');
            }

            if (CHECKFLAG(controls, FHOLD_MASK))
            {
                serial_putc('H');
            }

            if (probe)
            {
                serial_putc('P');
            }

            if (CHECKFLAG(limits, LIMIT_X_MASK))
            {
                serial_putc('X');
            }

            if (CHECKFLAG(limits, LIMIT_Y_MASK))
            {
                serial_putc('Y');
            }

            if (CHECKFLAG(limits, LIMIT_Z_MASK))
            {
                serial_putc('Z');
            }

            if (CHECKFLAG(limits, LIMIT_A_MASK))
            {
                serial_putc('A');
            }

            if (CHECKFLAG(limits, LIMIT_B_MASK))
            {
                serial_putc('B');
            }

            if (CHECKFLAG(limits, LIMIT_C_MASK))
            {
                serial_putc('C');
            }
        }

        protocol_send_status_tail();

        serial_putc('>');
        procotol_send_newline();
        protocol_busy = false;
    }

    void protocol_send_gcode_coordsys(void)
    {
        protocol_busy = true;
        float axis[MAX(AXIS_COUNT, 3)];
        uint8_t coordlimit = MIN(6, COORD_SYS_COUNT);
        for (uint8_t i = 0; i < coordlimit; i++)
        {
            parser_get_coordsys(i, axis);
            serial_print_str(__romstr__("[G"));
            serial_print_int(i + 54);
            serial_putc(':');
            serial_print_fltarr(axis, AXIS_COUNT);
            serial_putc(']');
            procotol_send_newline();
        }

        for (uint8_t i = 6; i < COORD_SYS_COUNT; i++)
        {
            serial_print_int(i - 5);
            serial_putc(':');
            parser_get_coordsys(i, axis);
            serial_print_fltarr(axis, AXIS_COUNT);
            serial_putc(']');
            procotol_send_newline();
        }

        serial_print_str(__romstr__("[G28:"));
        parser_get_coordsys(28, axis);
        serial_print_fltarr(axis, AXIS_COUNT);
        serial_putc(']');
        procotol_send_newline();

        serial_print_str(__romstr__("[G30:"));
        parser_get_coordsys(30, axis);
        serial_print_fltarr(axis, AXIS_COUNT);
        serial_putc(']');
        procotol_send_newline();

        serial_print_str(__romstr__("[G92:"));
        parser_get_coordsys(92, axis);
        serial_print_fltarr(axis, AXIS_COUNT);
        serial_putc(']');
        procotol_send_newline();

#ifdef AXIS_TOOL
        serial_print_str(__romstr__("[TLO:"));
        parser_get_coordsys(254, axis);
        serial_print_flt(axis[0]);
        serial_putc(']');
        procotol_send_newline();
#endif

        serial_print_str(__romstr__("[PRB:"));
        parser_get_coordsys(255, axis);
        serial_print_fltarr(axis, AXIS_COUNT);
        serial_putc(':');
        serial_putc('0' + parser_get_probe_result());
        serial_putc(']');
        procotol_send_newline();
        protocol_busy = false;
    }

    static void protocol_send_parser_modalstate(unsigned char word, uint8_t val, uint8_t mantissa)
    {
        serial_putc(word);
        serial_print_int(val);
        if (mantissa)
        {
            serial_putc('.');
            serial_print_int(mantissa);
        }
        serial_putc(' ');
    }

    void protocol_send_gcode_modes(void)
    {
        uint8_t modalgroups[12];
        uint16_t feed;
        uint16_t spindle;
        uint8_t coolant;

        protocol_busy = true;
        parser_get_modes(modalgroups, &feed, &spindle, &coolant);

        serial_print_str(__romstr__("[GC:"));

        for (uint8_t i = 0; i < 7; i++)
        {
            protocol_send_parser_modalstate('G', modalgroups[i], 0);
        }

        if (modalgroups[7] == 62)
        {
            protocol_send_parser_modalstate('G', 61, 1);
        }
        else
        {
            protocol_send_parser_modalstate('G', modalgroups[7], 0);
        }

        for (uint8_t i = 8; i < 11; i++)
        {
            protocol_send_parser_modalstate('M', modalgroups[i], 0);
        }

        serial_putc('T');
        serial_print_int(modalgroups[11]);
        serial_putc(' ');

        serial_putc('F');
        serial_print_fltunits(feed);
        serial_putc(' ');

        serial_putc('S');
        serial_print_int(spindle);

        serial_putc(']');
        procotol_send_newline();
        protocol_busy = false;
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

    void protocol_send_start_blocks(void)
    {
        protocol_busy = true;
        unsigned char c = 0;
        uint16_t address = STARTUP_BLOCK0_ADDRESS_OFFSET;
        serial_print_str(__romstr__("$N0="));
        for (;;)
        {
            settings_load(address++, &c, 1);
            if (c > 0 && c < 128)
            {
                serial_putc(c);
            }
            else
            {
                procotol_send_newline();
                break;
            }
        }

        address = STARTUP_BLOCK1_ADDRESS_OFFSET;
        serial_print_str(__romstr__("$N1="));
        for (;;)
        {
            settings_load(address++, &c, 1);
            if (c > 0 && c < 128)
            {
                serial_putc(c);
            }
            else
            {
                procotol_send_newline();
                break;
            }
        }
        protocol_busy = false;
    }

    void protocol_send_cnc_settings(void)
    {
        protocol_busy = true;
        protocol_send_gcode_setting_line_flt(0, g_settings.max_step_rate);
#ifdef EMULATE_GRBL_STARTUP
        // just adds this for compatibility
        // this setting is not used
        protocol_send_gcode_setting_line_int(1, 0);
#endif
        protocol_send_gcode_setting_line_int(2, g_settings.step_invert_mask);
        protocol_send_gcode_setting_line_int(3, g_settings.dir_invert_mask);
        protocol_send_gcode_setting_line_int(4, g_settings.step_enable_invert);
        protocol_send_gcode_setting_line_int(5, g_settings.limits_invert_mask);
        protocol_send_gcode_setting_line_int(6, g_settings.probe_invert_mask);
        protocol_send_gcode_setting_line_int(7, g_settings.control_invert_mask);
        protocol_send_gcode_setting_line_int(10, g_settings.status_report_mask);
        protocol_send_gcode_setting_line_flt(11, g_settings.g64_angle_factor);
        protocol_send_gcode_setting_line_flt(12, g_settings.arc_tolerance);
        protocol_send_gcode_setting_line_int(13, g_settings.report_inches);
        protocol_send_gcode_setting_line_int(20, g_settings.soft_limits_enabled);
        protocol_send_gcode_setting_line_int(21, g_settings.hard_limits_enabled);
        protocol_send_gcode_setting_line_int(22, g_settings.homing_enabled);
        protocol_send_gcode_setting_line_int(23, g_settings.homing_dir_invert_mask);
        protocol_send_gcode_setting_line_flt(24, g_settings.homing_slow_feed_rate);
        protocol_send_gcode_setting_line_flt(25, g_settings.homing_fast_feed_rate);
        protocol_send_gcode_setting_line_int(26, g_settings.debounce_ms);
        protocol_send_gcode_setting_line_flt(27, g_settings.homing_offset);
        protocol_send_gcode_setting_line_flt(30, g_settings.spindle_max_rpm);
        protocol_send_gcode_setting_line_flt(31, g_settings.spindle_min_rpm);
#ifdef LASER_MODE
        protocol_send_gcode_setting_line_int(32, g_settings.laser_mode);
#endif

#ifdef ENABLE_SKEW_COMPENSATION
        protocol_send_gcode_setting_line_flt(37, g_settings.skew_xy_factor);
#ifndef SKEW_COMPENSATION_XY_ONLY
        protocol_send_gcode_setting_line_flt(38, g_settings.skew_xz_factor);
        protocol_send_gcode_setting_line_flt(39, g_settings.skew_yz_factor);
#endif
#endif

        for (uint8_t i = 0; i < STEPPER_COUNT; i++)
        {
            protocol_send_gcode_setting_line_flt(100 + i, g_settings.step_per_mm[i]);
        }

        for (uint8_t i = 0; i < STEPPER_COUNT; i++)
        {
            protocol_send_gcode_setting_line_flt(110 + i, g_settings.max_feed_rate[i]);
        }

        for (uint8_t i = 0; i < STEPPER_COUNT; i++)
        {
            protocol_send_gcode_setting_line_flt(120 + i, g_settings.acceleration[i]);
        }

        for (uint8_t i = 0; i < AXIS_COUNT; i++)
        {
            protocol_send_gcode_setting_line_flt(130 + i, g_settings.max_distance[i]);
        }

#ifdef ENABLE_BACKLASH_COMPENSATION
        for (uint8_t i = 0; i < STEPPER_COUNT; i++)
        {
            protocol_send_gcode_setting_line_int(140 + i, g_settings.backlash_steps[i]);
        }
#endif

#if PID_CONTROLLERS > 0
        for (uint8_t i = 0; i < PID_CONTROLLERS; i++)
        {
            protocol_send_gcode_setting_line_flt(200 + fast_int_mul10(i), g_settings.pid_gain[i][0]);
            protocol_send_gcode_setting_line_flt(201 + fast_int_mul10(i), g_settings.pid_gain[i][1]);
            protocol_send_gcode_setting_line_flt(202 + fast_int_mul10(i), g_settings.pid_gain[i][2]);
        }
#endif
        protocol_busy = false;
    }

#ifdef __cplusplus
}
#endif
