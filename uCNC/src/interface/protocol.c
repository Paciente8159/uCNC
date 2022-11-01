/*
	Name: protocol.c
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

#include "../cnc.h"

#ifdef ECHO_CMD
static bool protocol_busy;
#endif

#ifdef ENABLE_IO_MODULES
// event_protocol_send_pins_states_handler
WEAK_EVENT_HANDLER(protocol_send_pins_states)
{
	DEFAULT_EVENT_HANDLER(protocol_send_pins_states);
}
#endif

#ifdef ENABLE_SETTINGS_MODULES
// event_protocol_send_cnc_settings_handler
WEAK_EVENT_HANDLER(protocol_send_cnc_settings)
{
	DEFAULT_EVENT_HANDLER(protocol_send_cnc_settings);
}
#endif

static void procotol_send_newline(void)
{
	protocol_send_string(MSG_EOL);
}

void protocol_send_ok(void)
{
#ifdef ECHO_CMD
	protocol_busy = true;
#endif
	protocol_send_string(MSG_OK);
	procotol_send_newline();
#ifdef ECHO_CMD
	protocol_busy = false;
#endif
}

void protocol_send_error(uint8_t error)
{
#ifdef ECHO_CMD
	protocol_busy = true;
#endif
	protocol_send_string(MSG_ERROR);
	serial_print_int(error);
	procotol_send_newline();
#ifdef ECHO_CMD
	protocol_busy = false;
#endif
}

void protocol_send_alarm(int8_t alarm)
{
#ifdef ECHO_CMD
	protocol_busy = true;
#endif
	protocol_send_string(MSG_ALARM);
	serial_print_int(alarm);
	procotol_send_newline();
#ifdef ECHO_CMD
	protocol_busy = false;
#endif
}

void protocol_send_string(const char *__s)
{
#ifdef ECHO_CMD
	protocol_busy = true;
#endif
	unsigned char c = (unsigned char)rom_strptr(__s++);
	do
	{
		serial_putc(c);
		c = (unsigned char)rom_strptr(__s++);
	} while (c != 0);
#ifdef ECHO_CMD
	protocol_busy = false;
#endif
}

void protocol_send_feedback(const char *__s)
{
#ifdef ECHO_CMD
	protocol_busy = true;
#endif
	protocol_send_string(MSG_START);
	protocol_send_string(__s);
	protocol_send_string(MSG_END);
#ifdef ECHO_CMD
	protocol_busy = false;
#endif
}

static uint8_t protocol_get_tools(void)
{
	uint8_t modalgroups[12];
	uint16_t feed;
	uint16_t spindle;
	uint8_t coolant;

	parser_get_modes(modalgroups, &feed, &spindle, &coolant);

#if TOOL_COUNT > 0
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
		protocol_send_string(MSG_STATUS_WCO);
		serial_print_fltarr(axis, AXIS_COUNT);
		return;
	}

	uint8_t ovr[3];
	if (planner_get_overflows(ovr))
	{
		protocol_send_string(MSG_STATUS_OVR);
		serial_print_int(ovr[0]);
		serial_putc(',');
		serial_print_int(ovr[1]);
		serial_putc(',');
		serial_print_int(ovr[2]);
		uint8_t tools = protocol_get_tools();
		if (tools)
		{
			protocol_send_string(MSG_STATUS_TOOL);
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
#ifdef ECHO_CMD
	if (protocol_busy)
	{
		return;
	}
#endif

#ifdef ECHO_CMD
	protocol_busy = true;
#endif
	float axis[MAX(AXIS_COUNT, 3)];

	int32_t steppos[AXIS_TO_STEPPERS];
	itp_get_rt_position(steppos);
	kinematics_apply_forward(steppos, axis);
	kinematics_apply_reverse_transform(axis);
	float feed = itp_get_rt_feed(); // convert from mm/s to mm/m
#if TOOL_COUNT > 0
	uint16_t spindle = tool_get_speed();
#else
	uint16_t spindle = 0;
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
	if (cnc_has_alarm())
	{
		protocol_send_string(MSG_STATUS_ALARM);
	}
	else if (mc_get_checkmode())
	{
		protocol_send_string(MSG_STATUS_CHECK);
	}
	else
	{
		switch (state)
		{
		case EXEC_DOOR:
			protocol_send_string(MSG_STATUS_DOOR);
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
			protocol_send_string(MSG_STATUS_ALARM);
			break;
		case EXEC_HOLD:
			protocol_send_string(MSG_STATUS_HOLD);
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
			protocol_send_string(MSG_STATUS_HOME);
			break;
		case EXEC_JOG:
			protocol_send_string(MSG_STATUS_JOG);
			break;
		case EXEC_RESUMING:
		case EXEC_RUN:
			protocol_send_string(MSG_STATUS_RUN);
			break;
		default:
			protocol_send_string(MSG_STATUS_IDLE);
			break;
		}
	}

	if ((g_settings.status_report_mask & 1))
	{
		protocol_send_string(MSG_STATUS_MPOS);
	}
	else
	{
		protocol_send_string(MSG_STATUS_WPOS);
		parser_machine_to_work(axis);
	}

	serial_print_fltarr(axis, AXIS_COUNT);

#if TOOL_COUNT > 0
	protocol_send_string(MSG_STATUS_FS);
#else
	protocol_send_string(MSG_STATUS_F);
#endif
	serial_print_fltunits(feed);
#if TOOL_COUNT > 0
	serial_putc(',');
	serial_print_int(spindle);
#endif

#ifdef GCODE_PROCESS_LINE_NUMBERS
	protocol_send_string(MSG_STATUS_LINE);
	serial_print_int(itp_get_rt_line_number());
#endif

	if (CHECKFLAG(controls, (ESTOP_MASK | SAFETY_DOOR_MASK | FHOLD_MASK)) || CHECKFLAG(limits, LIMITS_MASK) || probe)
	{
		protocol_send_string(MSG_STATUS_PIN);

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

	if ((g_settings.status_report_mask & 2))
	{
		protocol_send_string(MSG_STATUS_BUF);
		serial_print_int((uint32_t)planner_get_buffer_freeblocks());
		serial_putc(',');
		serial_print_int((uint32_t)serial_get_rx_freebytes());
	}

	serial_putc('>');
	procotol_send_newline();
#ifdef ECHO_CMD
	protocol_busy = false;
#endif
}

void protocol_send_gcode_coordsys(void)
{
#ifdef ECHO_CMD
	protocol_busy = true;
#endif
	float axis[MAX(AXIS_COUNT, 3)];
	uint8_t coordlimit = MIN(6, COORD_SYS_COUNT);
	for (uint8_t i = 0; i < coordlimit; i++)
	{
		parser_get_coordsys(i, axis);
		protocol_send_string(__romstr__("[G"));
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

	protocol_send_string(__romstr__("[G28:"));
	parser_get_coordsys(28, axis);
	serial_print_fltarr(axis, AXIS_COUNT);
	serial_putc(']');
	procotol_send_newline();

	protocol_send_string(__romstr__("[G30:"));
	parser_get_coordsys(30, axis);
	serial_print_fltarr(axis, AXIS_COUNT);
	serial_putc(']');
	procotol_send_newline();

	protocol_send_string(__romstr__("[G92:"));
	parser_get_coordsys(92, axis);
	serial_print_fltarr(axis, AXIS_COUNT);
	serial_putc(']');
	procotol_send_newline();

#ifdef AXIS_TOOL
	protocol_send_string(__romstr__("[TLO:"));
	parser_get_coordsys(254, axis);
	serial_print_flt(axis[0]);
	serial_putc(']');
	procotol_send_newline();
#endif
	protocol_send_probe_result(parser_get_probe_result());

#ifdef ECHO_CMD
	protocol_busy = false;
#endif
}

void protocol_send_probe_result(uint8_t val)
{
	float axis[MAX(AXIS_COUNT, 3)];
#ifdef ECHO_CMD
	protocol_busy = true;
#endif
	protocol_send_string(__romstr__("[PRB:"));
	parser_get_coordsys(255, axis);
	serial_print_fltarr(axis, AXIS_COUNT);
	serial_putc(':');
	serial_putc('0' + val);
	serial_putc(']');
	procotol_send_newline();
#ifdef ECHO_CMD
	protocol_busy = false;
#endif
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

#ifdef ECHO_CMD
	protocol_busy = true;
#endif
	parser_get_modes(modalgroups, &feed, &spindle, &coolant);

	protocol_send_string(__romstr__("[GC:"));

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
#ifdef ECHO_CMD
	protocol_busy = false;
#endif
}

void protocol_send_gcode_setting_line_int(setting_offset_t setting, uint16_t value)
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
#ifdef ECHO_CMD
	protocol_busy = true;
#endif
	unsigned char c = 0;
	uint16_t address = STARTUP_BLOCK0_ADDRESS_OFFSET;
	protocol_send_string(__romstr__("$N0="));
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
	protocol_send_string(__romstr__("$N1="));
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
#ifdef ECHO_CMD
	protocol_busy = false;
#endif
}

void protocol_send_cnc_settings(void)
{
#ifdef ECHO_CMD
	protocol_busy = true;
#endif
	protocol_send_gcode_setting_line_flt(0, (1000000.0f / g_settings.max_step_rate));
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
#if ENCODERS > 0
	protocol_send_gcode_setting_line_int(8, g_settings.encoders_pulse_invert_mask);
	protocol_send_gcode_setting_line_int(9, g_settings.encoders_dir_invert_mask);
#endif
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
	protocol_send_gcode_setting_line_int(30, g_settings.spindle_max_rpm);
	protocol_send_gcode_setting_line_int(31, g_settings.spindle_min_rpm);
	protocol_send_gcode_setting_line_int(32, g_settings.laser_mode);
#ifdef ENABLE_LASER_PPI
	protocol_send_gcode_setting_line_int(33, g_settings.laser_ppi);
	protocol_send_gcode_setting_line_int(34, g_settings.laser_ppi_uswidth);
	protocol_send_gcode_setting_line_flt(35, g_settings.laser_ppi_mixmode_ppi);
	protocol_send_gcode_setting_line_flt(36, g_settings.laser_ppi_mixmode_uswidth);
#endif
#ifdef ENABLE_SKEW_COMPENSATION
	protocol_send_gcode_setting_line_flt(37, g_settings.skew_xy_factor);
#ifndef SKEW_COMPENSATION_XY_ONLY
	protocol_send_gcode_setting_line_flt(38, g_settings.skew_xz_factor);
	protocol_send_gcode_setting_line_flt(39, g_settings.skew_yz_factor);
#endif
#endif

#if PID_CONTROLLERS > 0
	for (uint8_t i = 0; i < PID_CONTROLLERS; i++)
	{
		protocol_send_gcode_setting_line_flt(40 + 4 * i, g_settings.pid_gain[i][0]);
		protocol_send_gcode_setting_line_flt(41 + 4 * i, g_settings.pid_gain[i][1]);
		protocol_send_gcode_setting_line_flt(42 + 4 * i, g_settings.pid_gain[i][2]);
	}
#endif

#if TOOL_COUNT > 0
	protocol_send_gcode_setting_line_int(80, g_settings.default_tool);
	for (uint8_t i = 0; i < TOOL_COUNT; i++)
	{
		protocol_send_gcode_setting_line_flt(81 + i, g_settings.tool_length_offset[i]);
	}
#endif

	for (uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		protocol_send_gcode_setting_line_flt(100 + i, g_settings.step_per_mm[i]);
	}

#if (KINEMATIC == KINEMATIC_LINEAR_DELTA)
	protocol_send_gcode_setting_line_flt(106, g_settings.delta_arm_length);
	protocol_send_gcode_setting_line_flt(107, g_settings.delta_armbase_radius);
	// protocol_send_gcode_setting_line_int(108, g_settings.delta_efector_height);
#endif

	for (uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		protocol_send_gcode_setting_line_flt(110 + i, g_settings.max_feed_rate[i]);
	}

	for (uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		protocol_send_gcode_setting_line_flt(120 + i, g_settings.acceleration[i]);
	}

	for (uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		protocol_send_gcode_setting_line_flt(130 + i, g_settings.max_distance[i]);
	}

#ifdef ENABLE_BACKLASH_COMPENSATION
	for (uint8_t i = 0; i < AXIS_TO_STEPPERS; i++)
	{
		protocol_send_gcode_setting_line_int(140 + i, g_settings.backlash_steps[i]);
	}
#endif

#ifdef ENABLE_SETTINGS_MODULES
	EVENT_INVOKE(protocol_send_cnc_settings, NULL);
#endif
#ifdef ECHO_CMD
	protocol_busy = false;
#endif
}

#ifdef ENABLE_EXTRA_SYSTEM_CMDS
void protocol_send_pins_states(void)
{
#ifdef ECHO_CMD
	protocol_busy = true;
#endif
	for (uint8_t i = 0; i < 161; i++)
	{
		int16_t val = io_get_pinvalue(i);
		if (val >= 0)
		{
			if (i < 100)
			{
				if (i < 24)
				{
					protocol_send_string(__romstr__("[SO:"));
				}
				else if (i < 40)
				{
					protocol_send_string(__romstr__("[P:"));
				}
				else if (i < 46)
				{
					protocol_send_string(__romstr__("[SV:"));
				}
				else if (i < 78)
				{
					protocol_send_string(__romstr__("[O:"));
				}

				else
				{
					i = 100; // jumps to inputs
				}
			}

			if (i >= 100)
			{
				if (i < 114)
				{
					protocol_send_string(__romstr__("[SI:"));
				}
				else if (i < 130)
				{
					protocol_send_string(__romstr__("[A:"));
				}
				else
				{
					protocol_send_string(__romstr__("[I:"));
				}
			}
			serial_print_int(i);
			serial_putc(':');
			serial_print_int(val);
			protocol_send_string(MSG_END);
		}
	}

	int32_t steps[STEPPER_COUNT];
	itp_get_rt_position(steps);
	protocol_send_string(__romstr__("[STEPS:"));
	serial_print_intarr(steps, AXIS_TO_STEPPERS);
	protocol_send_string(MSG_END);

#if ENCODERS > 0
	encoder_print_values();
#endif

#ifdef ENABLE_IO_MODULES
	EVENT_INVOKE(protocol_send_pins_states, NULL);
#endif

	protocol_send_string(__romstr__("[RUNTIME:"));
	serial_print_int(mcu_millis());
	protocol_send_string(MSG_END);

#ifdef ECHO_CMD
	protocol_busy = false;
#endif
}
#endif

#ifdef ENABLE_SYSTEM_INFO
#if (KINEMATIC == KINEMATIC_CARTESIAN)
#define KINEMATIC_INFO "C" STRGIFY(AXIS_COUNT) ","
#elif (KINEMATIC == KINEMATIC_COREXY)
#define KINEMATIC_INFO "XY" STRGIFY(AXIS_COUNT) ","
#elif (KINEMATIC == KINEMATIC_LINEAR_DELTA)
#define KINEMATIC_INFO "D" STRGIFY(AXIS_COUNT) ","
#else
#define KINEMATIC_INFO ""
#endif
#define TOOLS_INFO "T" STRGIFY(TOOL_COUNT) ","

#ifdef GCODE_PROCESS_LINE_NUMBERS
#define LINES_INFO "N,"
#else
#define LINES_INFO ""
#endif

#ifdef BRESENHAM_16BIT
#define BRESENHAM_INFO "16B,"
#else
#define BRESENHAM_INFO ""
#endif

#ifdef ENABLE_S_CURVE_ACCELERATION
#define DYNACCEL_INFO "S,"
#else
#define DYNACCEL_INFO ""
#endif

#ifndef USE_LEGACY_STEP_INTERPOLATOR
#define ACCELALG_INFO "NI,"
#else
#define ACCELALG_INFO ""
#endif

#ifdef INVERT_EMERGENCY_STOP
#define INVESTOP_INFO "IE,"
#else
#define INVESTOP_INFO ""
#endif

#ifdef DISABLE_ALL_CONTROLS
#define CONTROLS_INFO "DC,"
#else
#define CONTROLS_INFO ""
#endif

#ifdef DISABLE_ALL_LIMITS
#define LIMITS_INFO "DL,"
#else
#define LIMITS_INFO ""
#endif

#ifdef DISABLE_PROBE
#define PROBE_INFO "DP,"
#else
#define PROBE_INFO ""
#endif

#ifdef ENABLE_EXTRA_SYSTEM_CMDS
#define EXTRACMD_INFO "XC,"
#else
#define EXTRACMD_INFO ""
#endif

#ifdef ENABLE_FAST_MATH
#define FASTMATH_INFO "F,"
#else
#define FASTMATH_INFO ""
#endif

#ifdef ENABLE_LINACT_PLANNER
#define LINPLAN_INFO "LP,"
#else
#define LINPLAN_INFO ""
#endif

#ifdef ENABLE_SKEW_COMPENSATION
#ifdef SKEW_COMPENSATION_XY_ONLY
#define SKEW_INFO "SKXY,"
#else
#define SKEW_INFO "SK,"
#endif
#else
#define SKEW_INFO ""
#endif

#ifdef ENABLE_LASER_PPI
#define PPI_INFO "PPI,"
#else
#define PPI_INFO ""
#endif

#define DSS_INFO "DSS" STRGIFY(DSS_MAX_OVERSAMPLING) "_" STRGIFY(DSS_CUTOFF_FREQ) ","
#define PLANNER_INFO             \
	STRGIFY(PLANNER_BUFFER_SIZE) \
	","

#define SERIAL_INFO STRGIFY(RX_BUFFER_CAPACITY)

#ifndef BOARD_NAME
#define BOARD_NAME "Generic board"
#endif

#define OPT_INFO __romstr__("[OPT:" KINEMATIC_INFO LINES_INFO BRESENHAM_INFO DSS_INFO DYNACCEL_INFO ACCELALG_INFO SKEW_INFO LINPLAN_INFO PPI_INFO INVESTOP_INFO CONTROLS_INFO LIMITS_INFO PROBE_INFO EXTRACMD_INFO FASTMATH_INFO)
#define VER_INFO __romstr__("[VER: uCNC " CNC_VERSION " - " BOARD_NAME "]" STR_EOL)

WEAK_EVENT_HANDLER(protocol_send_cnc_info)
{
	//custom handler
	protocol_send_cnc_info_delegate_event_t *ptr = protocol_send_cnc_info_event;
	while (ptr != NULL)
	{
		if (ptr->fptr != NULL)
		{
			ptr->fptr(args, NULL);
			serial_putc(',');
		}
		ptr = ptr->next;
	}

	return 0;
}

void protocol_send_cnc_info(void)
{
#ifdef ECHO_CMD
	protocol_busy = true;
#endif
	protocol_send_string(VER_INFO);
	protocol_send_string(OPT_INFO);
	EVENT_INVOKE(protocol_send_cnc_info, NULL);
	protocol_send_string(__romstr__(PLANNER_INFO SERIAL_INFO "]" STR_EOL));
#ifdef ECHO_CMD
	protocol_busy = false;
#endif
}
#endif
