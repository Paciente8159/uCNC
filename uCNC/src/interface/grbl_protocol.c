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

#if defined(ENABLE_EXTRA_SYSTEM_CMDS) && defined(ENABLE_PIN_TRANSLATIONS)
const char pin_name_1[] __rom__ = "STEP0";
const char pin_name_2[] __rom__ = "STEP1";
const char pin_name_3[] __rom__ = "STEP2";
const char pin_name_4[] __rom__ = "STEP3";
const char pin_name_5[] __rom__ = "STEP4";
const char pin_name_6[] __rom__ = "STEP5";
const char pin_name_7[] __rom__ = "STEP6";
const char pin_name_8[] __rom__ = "STEP7";
const char pin_name_9[] __rom__ = "DIR0";
const char pin_name_10[] __rom__ = "DIR1";
const char pin_name_11[] __rom__ = "DIR2";
const char pin_name_12[] __rom__ = "DIR3";
const char pin_name_13[] __rom__ = "DIR4";
const char pin_name_14[] __rom__ = "DIR5";
const char pin_name_15[] __rom__ = "DIR6";
const char pin_name_16[] __rom__ = "DIR7";
const char pin_name_17[] __rom__ = "STEP0_EN";
const char pin_name_18[] __rom__ = "STEP1_EN";
const char pin_name_19[] __rom__ = "STEP2_EN";
const char pin_name_20[] __rom__ = "STEP3_EN";
const char pin_name_21[] __rom__ = "STEP4_EN";
const char pin_name_22[] __rom__ = "STEP5_EN";
const char pin_name_23[] __rom__ = "STEP6_EN";
const char pin_name_24[] __rom__ = "STEP7_EN";
const char pin_name_25[] __rom__ = "PWM0";
const char pin_name_26[] __rom__ = "PWM1";
const char pin_name_27[] __rom__ = "PWM2";
const char pin_name_28[] __rom__ = "PWM3";
const char pin_name_29[] __rom__ = "PWM4";
const char pin_name_30[] __rom__ = "PWM5";
const char pin_name_31[] __rom__ = "PWM6";
const char pin_name_32[] __rom__ = "PWM7";
const char pin_name_33[] __rom__ = "PWM8";
const char pin_name_34[] __rom__ = "PWM9";
const char pin_name_35[] __rom__ = "PWM10";
const char pin_name_36[] __rom__ = "PWM11";
const char pin_name_37[] __rom__ = "PWM12";
const char pin_name_38[] __rom__ = "PWM13";
const char pin_name_39[] __rom__ = "PWM14";
const char pin_name_40[] __rom__ = "PWM15";
const char pin_name_41[] __rom__ = "SERVO0";
const char pin_name_42[] __rom__ = "SERVO1";
const char pin_name_43[] __rom__ = "SERVO2";
const char pin_name_44[] __rom__ = "SERVO3";
const char pin_name_45[] __rom__ = "SERVO4";
const char pin_name_46[] __rom__ = "SERVO5";
const char pin_name_47[] __rom__ = "DOUT0";
const char pin_name_48[] __rom__ = "DOUT1";
const char pin_name_49[] __rom__ = "DOUT2";
const char pin_name_50[] __rom__ = "DOUT3";
const char pin_name_51[] __rom__ = "DOUT4";
const char pin_name_52[] __rom__ = "DOUT5";
const char pin_name_53[] __rom__ = "DOUT6";
const char pin_name_54[] __rom__ = "DOUT7";
const char pin_name_55[] __rom__ = "DOUT8";
const char pin_name_56[] __rom__ = "DOUT9";
const char pin_name_57[] __rom__ = "DOUT10";
const char pin_name_58[] __rom__ = "DOUT11";
const char pin_name_59[] __rom__ = "DOUT12";
const char pin_name_60[] __rom__ = "DOUT13";
const char pin_name_61[] __rom__ = "DOUT14";
const char pin_name_62[] __rom__ = "DOUT15";
const char pin_name_63[] __rom__ = "DOUT16";
const char pin_name_64[] __rom__ = "DOUT17";
const char pin_name_65[] __rom__ = "DOUT18";
const char pin_name_66[] __rom__ = "DOUT19";
const char pin_name_67[] __rom__ = "DOUT20";
const char pin_name_68[] __rom__ = "DOUT21";
const char pin_name_69[] __rom__ = "DOUT22";
const char pin_name_70[] __rom__ = "DOUT23";
const char pin_name_71[] __rom__ = "DOUT24";
const char pin_name_72[] __rom__ = "DOUT25";
const char pin_name_73[] __rom__ = "DOUT26";
const char pin_name_74[] __rom__ = "DOUT27";
const char pin_name_75[] __rom__ = "DOUT28";
const char pin_name_76[] __rom__ = "DOUT29";
const char pin_name_77[] __rom__ = "DOUT30";
const char pin_name_78[] __rom__ = "DOUT31";
const char pin_name_79[] __rom__ = "DOUT32";
const char pin_name_80[] __rom__ = "DOUT33";
const char pin_name_81[] __rom__ = "DOUT34";
const char pin_name_82[] __rom__ = "DOUT35";
const char pin_name_83[] __rom__ = "DOUT36";
const char pin_name_84[] __rom__ = "DOUT37";
const char pin_name_85[] __rom__ = "DOUT38";
const char pin_name_86[] __rom__ = "DOUT39";
const char pin_name_87[] __rom__ = "DOUT40";
const char pin_name_88[] __rom__ = "DOUT41";
const char pin_name_89[] __rom__ = "DOUT42";
const char pin_name_90[] __rom__ = "DOUT43";
const char pin_name_91[] __rom__ = "DOUT44";
const char pin_name_92[] __rom__ = "DOUT45";
const char pin_name_93[] __rom__ = "DOUT46";
const char pin_name_94[] __rom__ = "DOUT47";
const char pin_name_95[] __rom__ = "DOUT48";
const char pin_name_96[] __rom__ = "DOUT49";
const char pin_name_100[] __rom__ = "LIMIT_X";
const char pin_name_101[] __rom__ = "LIMIT_Y";
const char pin_name_102[] __rom__ = "LIMIT_Z";
const char pin_name_103[] __rom__ = "LIMIT_X2";
const char pin_name_104[] __rom__ = "LIMIT_Y2";
const char pin_name_105[] __rom__ = "LIMIT_Z2";
const char pin_name_106[] __rom__ = "LIMIT_A";
const char pin_name_107[] __rom__ = "LIMIT_B";
const char pin_name_108[] __rom__ = "LIMIT_C";
const char pin_name_109[] __rom__ = "PROBE";
const char pin_name_110[] __rom__ = "ESTOP";
const char pin_name_111[] __rom__ = "SAFETY_DOOR";
const char pin_name_112[] __rom__ = "FHOLD";
const char pin_name_113[] __rom__ = "CS_RES";
const char pin_name_114[] __rom__ = "ANALOG0";
const char pin_name_115[] __rom__ = "ANALOG1";
const char pin_name_116[] __rom__ = "ANALOG2";
const char pin_name_117[] __rom__ = "ANALOG3";
const char pin_name_118[] __rom__ = "ANALOG4";
const char pin_name_119[] __rom__ = "ANALOG5";
const char pin_name_120[] __rom__ = "ANALOG6";
const char pin_name_121[] __rom__ = "ANALOG7";
const char pin_name_122[] __rom__ = "ANALOG8";
const char pin_name_123[] __rom__ = "ANALOG9";
const char pin_name_124[] __rom__ = "ANALOG10";
const char pin_name_125[] __rom__ = "ANALOG11";
const char pin_name_126[] __rom__ = "ANALOG12";
const char pin_name_127[] __rom__ = "ANALOG13";
const char pin_name_128[] __rom__ = "ANALOG14";
const char pin_name_129[] __rom__ = "ANALOG15";
const char pin_name_130[] __rom__ = "DIN0";
const char pin_name_131[] __rom__ = "DIN1";
const char pin_name_132[] __rom__ = "DIN2";
const char pin_name_133[] __rom__ = "DIN3";
const char pin_name_134[] __rom__ = "DIN4";
const char pin_name_135[] __rom__ = "DIN5";
const char pin_name_136[] __rom__ = "DIN6";
const char pin_name_137[] __rom__ = "DIN7";
const char pin_name_138[] __rom__ = "DIN8";
const char pin_name_139[] __rom__ = "DIN9";
const char pin_name_140[] __rom__ = "DIN10";
const char pin_name_141[] __rom__ = "DIN11";
const char pin_name_142[] __rom__ = "DIN12";
const char pin_name_143[] __rom__ = "DIN13";
const char pin_name_144[] __rom__ = "DIN14";
const char pin_name_145[] __rom__ = "DIN15";
const char pin_name_146[] __rom__ = "DIN16";
const char pin_name_147[] __rom__ = "DIN17";
const char pin_name_148[] __rom__ = "DIN18";
const char pin_name_149[] __rom__ = "DIN19";
const char pin_name_150[] __rom__ = "DIN20";
const char pin_name_151[] __rom__ = "DIN21";
const char pin_name_152[] __rom__ = "DIN22";
const char pin_name_153[] __rom__ = "DIN23";
const char pin_name_154[] __rom__ = "DIN24";
const char pin_name_155[] __rom__ = "DIN25";
const char pin_name_156[] __rom__ = "DIN26";
const char pin_name_157[] __rom__ = "DIN27";
const char pin_name_158[] __rom__ = "DIN28";
const char pin_name_159[] __rom__ = "DIN29";
const char pin_name_160[] __rom__ = "DIN30";
const char pin_name_161[] __rom__ = "DIN31";
const char pin_name_162[] __rom__ = "DIN32";
const char pin_name_163[] __rom__ = "DIN33";
const char pin_name_164[] __rom__ = "DIN34";
const char pin_name_165[] __rom__ = "DIN35";
const char pin_name_166[] __rom__ = "DIN36";
const char pin_name_167[] __rom__ = "DIN37";
const char pin_name_168[] __rom__ = "DIN38";
const char pin_name_169[] __rom__ = "DIN39";
const char pin_name_170[] __rom__ = "DIN40";
const char pin_name_171[] __rom__ = "DIN41";
const char pin_name_172[] __rom__ = "DIN42";
const char pin_name_173[] __rom__ = "DIN43";
const char pin_name_174[] __rom__ = "DIN44";
const char pin_name_175[] __rom__ = "DIN45";
const char pin_name_176[] __rom__ = "DIN46";
const char pin_name_177[] __rom__ = "DIN47";
const char pin_name_178[] __rom__ = "DIN48";
const char pin_name_179[] __rom__ = "DIN49";
const char pin_name_200[] __rom__ = "TX";
const char pin_name_201[] __rom__ = "RX";
const char pin_name_202[] __rom__ = "USB_DM";
const char pin_name_203[] __rom__ = "USB_DP";
const char pin_name_204[] __rom__ = "SPI_CLK";
const char pin_name_205[] __rom__ = "SPI_SDI";
const char pin_name_206[] __rom__ = "SPI_SDO";
const char pin_name_207[] __rom__ = "SPI_CS";
const char pin_name_208[] __rom__ = "I2C_SCL";
const char pin_name_209[] __rom__ = "I2C_SDA";
const char pin_name_210[] __rom__ = "TX2";
const char pin_name_211[] __rom__ = "RX2";
const char pin_name_212[] __rom__ = "SPI2_CLK";
const char pin_name_213[] __rom__ = "SPI2_SDI";
const char pin_name_214[] __rom__ = "SPI2_SDO";
const char pin_name_215[] __rom__ = "SPI2_CS";
const char *const outputpins_names[] __rom__ = {pin_name_1, pin_name_2, pin_name_3, pin_name_4, pin_name_5, pin_name_6, pin_name_7, pin_name_8, pin_name_9, pin_name_10, pin_name_11, pin_name_12, pin_name_13, pin_name_14, pin_name_15, pin_name_16, pin_name_17, pin_name_18, pin_name_19, pin_name_20, pin_name_21, pin_name_22, pin_name_23, pin_name_24, pin_name_25, pin_name_26, pin_name_27, pin_name_28, pin_name_29, pin_name_30, pin_name_31, pin_name_32, pin_name_33, pin_name_34, pin_name_35, pin_name_36, pin_name_37, pin_name_38, pin_name_39, pin_name_40, pin_name_41, pin_name_42, pin_name_43, pin_name_44, pin_name_45, pin_name_46, pin_name_47, pin_name_48, pin_name_49, pin_name_50, pin_name_51, pin_name_52, pin_name_53, pin_name_54, pin_name_55, pin_name_56, pin_name_57, pin_name_58, pin_name_59, pin_name_60, pin_name_61, pin_name_62, pin_name_63, pin_name_64, pin_name_65, pin_name_66, pin_name_67, pin_name_68, pin_name_69, pin_name_70, pin_name_71, pin_name_72, pin_name_73, pin_name_74, pin_name_75, pin_name_76, pin_name_77, pin_name_78, pin_name_79, pin_name_80, pin_name_81, pin_name_82, pin_name_83, pin_name_84, pin_name_85, pin_name_86, pin_name_87, pin_name_88, pin_name_89, pin_name_90, pin_name_91, pin_name_92, pin_name_93, pin_name_94, pin_name_95, pin_name_96};
const char *const inputpins_names[] __rom__ = {pin_name_100, pin_name_101, pin_name_102, pin_name_103, pin_name_104, pin_name_105, pin_name_106, pin_name_107, pin_name_108, pin_name_109, pin_name_110, pin_name_111, pin_name_112, pin_name_113, pin_name_114, pin_name_115, pin_name_116, pin_name_117, pin_name_118, pin_name_119, pin_name_120, pin_name_121, pin_name_122, pin_name_123, pin_name_124, pin_name_125, pin_name_126, pin_name_127, pin_name_128, pin_name_129, pin_name_130, pin_name_131, pin_name_132, pin_name_133, pin_name_134, pin_name_135, pin_name_136, pin_name_137, pin_name_138, pin_name_139, pin_name_140, pin_name_141, pin_name_142, pin_name_143, pin_name_144, pin_name_145, pin_name_146, pin_name_147, pin_name_148, pin_name_149, pin_name_150, pin_name_151, pin_name_152, pin_name_153, pin_name_154, pin_name_155, pin_name_156, pin_name_157, pin_name_158, pin_name_159, pin_name_160, pin_name_161, pin_name_162, pin_name_163, pin_name_164, pin_name_165, pin_name_166, pin_name_167, pin_name_168, pin_name_169, pin_name_170, pin_name_171, pin_name_172, pin_name_173, pin_name_174, pin_name_175, pin_name_176, pin_name_177, pin_name_178, pin_name_179, pin_name_200, pin_name_201, pin_name_202, pin_name_203, pin_name_204, pin_name_205, pin_name_206, pin_name_207, pin_name_208, pin_name_209, pin_name_210, pin_name_211, pin_name_212, pin_name_213, pin_name_214, pin_name_215};

#endif

#ifndef MAX_MODAL_GROUPS
#define MAX_MODAL_GROUPS 14
#endif

static bool protocol_busy;

#ifdef ENABLE_IO_MODULES
// event_grbl_protocol_pins_states_handler
WEAK_EVENT_HANDLER(grbl_protocol_pins_states)
{
	DEFAULT_EVENT_HANDLER(grbl_protocol_pins_states);
}
#endif

#ifdef ENABLE_SETTINGS_MODULES
// event_grbl_protocol_cnc_settings_handler
WEAK_EVENT_HANDLER(grbl_protocol_cnc_settings)
{
	DEFAULT_EVENT_HANDLER(grbl_protocol_cnc_settings);
}
#endif

#ifdef ENABLE_PARSER_MODULES
// event_grbl_protocol_gcode_modes_handler
WEAK_EVENT_HANDLER(grbl_protocol_gcode_modes)
{
	DEFAULT_EVENT_HANDLER(grbl_protocol_gcode_modes);
}
#endif

void grbl_protocol_error(uint8_t error)
{
	grbl_protocol_printf(MSG_ERROR, error);
}

void grbl_protocol_alarm(int8_t alarm)
{
	grbl_stream_broadcast(true);
	grbl_protocol_printf(MSG_ALARM, alarm);
	grbl_stream_broadcast(false);
}

void grbl_protocol_feedback_base(void *arg, uint8_t type)
{
	grbl_stream_broadcast(true);
	if (type)
	{
		grbl_protocol_printf(MSG_FEEDBACK_PRINTF, arg);
	}
	else
	{
		grbl_protocol_printf(MSG_FEEDBACK, arg);
	}
	grbl_stream_broadcast(false);
}

WEAK_EVENT_HANDLER(grbl_protocol_status)
{
	// custom handler
	grbl_protocol_status_delegate_event_t *ptr = grbl_protocol_status_event;
	while (ptr != NULL)
	{
		if (ptr->fptr != NULL)
		{
			grbl_protocol_putc('|');
			ptr->fptr(args);
		}
		ptr = ptr->next;
	}

	return EVENT_CONTINUE;
}

static FORCEINLINE void grbl_protocol_status_tail(void)
{
	float axis[MAX(AXIS_COUNT, 3)];
	if (parser_get_wco(axis))
	{
		grbl_protocol_printf(MSG_STATUS_WPOS, axis);
		return;
	}

	if (!g_planner_state.ovr_counter)
	{
		g_planner_state.ovr_counter = STATUS_WCO_REPORT_MIN_FREQUENCY;
		uint8_t tovr = 0;
#if TOOL_COUNT > 0
		tovr = g_planner_state.spindle_speed_override;
#endif
		grbl_protocol_printf(MSG_STATUS_OVR, g_planner_state.feed_override, g_planner_state.rapid_feed_override, tovr);

		uint8_t modalgroups[MAX_MODAL_GROUPS];
		uint16_t feed;
		uint16_t spindle;

		parser_get_modes(modalgroups, &feed, &spindle);
		if (modalgroups[8] != 5 || modalgroups[9])
		{
			grbl_protocol_print(MSG_STATUS_TOOL);
			if (modalgroups[8] == 3)
			{
				grbl_protocol_putc('S');
			}
			if (modalgroups[8] == 4)
			{
				grbl_protocol_putc('C');
			}
#ifdef ENABLE_COOLANT
			if (CHECKFLAG(modalgroups[9], COOLANT_MASK))
			{
				grbl_protocol_putc('F');
			}
#ifndef M7_SAME_AS_M8
			if (CHECKFLAG(modalgroups[9], MIST_MASK))
			{
				grbl_protocol_putc('M');
			}
#endif
#endif
		}
		return;
	}
	g_planner_state.ovr_counter--;
}

void grbl_protocol_status(void)
{
	if (protocol_busy || grbl_stream_busy())
	{
		return;
	}

	grbl_stream_broadcast(true);

	float axis[MAX(AXIS_COUNT, 3)];

	int32_t steppos[AXIS_TO_STEPPERS];
	io_get_steps_pos(steppos);
	kinematics_steps_to_coordinates(steppos, axis);
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

	grbl_protocol_putc('<');
	if (cnc_has_alarm())
	{
		grbl_protocol_print(MSG_STATUS_ALARM);
	}
	else if (mc_get_checkmode())
	{
		grbl_protocol_print(MSG_STATUS_CHECK);
	}
	else
	{
		switch (state)
		{
#if ASSERT_PIN(SAFETY_DOOR)
		case EXEC_DOOR:
			grbl_protocol_print(MSG_STATUS_DOOR);
			grbl_protocol_putc(':');
			if (CHECKFLAG(controls, SAFETY_DOOR_MASK))
			{
				if (cnc_get_exec_state(EXEC_RUN))
				{
					grbl_protocol_putc('2');
				}
				else
				{
					grbl_protocol_putc('1');
				}
			}
			else
			{
				if (cnc_get_exec_state(EXEC_RUN))
				{
					grbl_protocol_putc('3');
				}
				else
				{
					grbl_protocol_putc('0');
				}
			}
			break;
#endif
		case EXEC_UNHOMED:
		case EXEC_LIMITS:
			if (!cnc_get_exec_state(EXEC_HOMING))
			{
				grbl_protocol_print(MSG_STATUS_ALARM);
			}
			else
			{
				grbl_protocol_print(MSG_STATUS_HOME);
			}
			break;
		case EXEC_HOLD:
			grbl_protocol_print(MSG_STATUS_HOLD);
			grbl_protocol_putc(':');
			if (cnc_get_exec_state(EXEC_RUN))
			{
				grbl_protocol_putc('1');
			}
			else
			{
				grbl_protocol_putc('0');
			}
			break;
		case EXEC_HOMING:
			grbl_protocol_print(MSG_STATUS_HOME);
			break;
		case EXEC_JOG:
			grbl_protocol_print(MSG_STATUS_JOG);
			break;
		case EXEC_RUN:
			grbl_protocol_print(MSG_STATUS_RUN);
			break;
		default:
			grbl_protocol_print(MSG_STATUS_IDLE);
			break;
		}
	}

	if (!(g_settings.status_report_mask & 1))
	{
		grbl_protocol_printf(MSG_STATUS_MPOS, axis);
	}
	else
	{
		parser_machine_to_work(axis);
		grbl_protocol_printf(MSG_STATUS_WPOS, axis);
	}

	feed = (!g_settings.report_inches) ? feed : (feed * MM_INCH_MULT);
	grbl_protocol_printf(MSG_STATUS_FS, feed, spindle);

#ifdef GCODE_PROCESS_LINE_NUMBERS
	grbl_protocol_printf(MSG_STATUS_LINE, itp_get_rt_line_number());
#endif

	if (CHECKFLAG(controls, (ESTOP_MASK | SAFETY_DOOR_MASK | FHOLD_MASK)) || CHECKFLAG(limits, LIMITS_MASK) || probe)
	{
		grbl_protocol_print(MSG_STATUS_PIN);

		if (CHECKFLAG(controls, ESTOP_MASK))
		{
			grbl_protocol_putc('R');
		}

		if (CHECKFLAG(controls, SAFETY_DOOR_MASK))
		{
			grbl_protocol_putc('D');
		}

		if (CHECKFLAG(controls, FHOLD_MASK))
		{
			grbl_protocol_putc('H');
		}

		if (probe)
		{
			grbl_protocol_putc('P');
		}

		if (CHECKFLAG(limits, LINACT0_LIMIT_MASK))
		{
			grbl_protocol_putc('X');
		}

		if (CHECKFLAG(limits, LINACT1_LIMIT_MASK))
		{
#if ((AXIS_COUNT == 2) && defined(USE_Y_AS_Z_ALIAS))
			grbl_protocol_putc('Z');
#else
			grbl_protocol_putc('Y');
#endif
		}

		if (CHECKFLAG(limits, LINACT2_LIMIT_MASK))
		{
			grbl_protocol_putc('Z');
		}

		if (CHECKFLAG(limits, LINACT3_LIMIT_MASK))
		{
			grbl_protocol_putc('A');
		}

		if (CHECKFLAG(limits, LINACT4_LIMIT_MASK))
		{
			grbl_protocol_putc('B');
		}

		if (CHECKFLAG(limits, LINACT5_LIMIT_MASK))
		{
			grbl_protocol_putc('C');
		}
	}

	grbl_protocol_status_tail();

	EVENT_INVOKE(grbl_protocol_status, NULL);

	if ((g_settings.status_report_mask & 2))
	{
		grbl_protocol_printf(MSG_STATUS_BUF, planner_get_buffer_freeblocks(), grbl_stream_write_available());
	}

	grbl_protocol_putc('>');
	grbl_protocol_putc('\r');
	grbl_protocol_putc('\n');
}

void grbl_protocol_gcode_coordsys(void)
{
	protocol_busy = true;
	float axis[MAX(AXIS_COUNT, 3)];

	for (uint8_t i = 0; i < COORD_SYS_COUNT; i++)
	{
		parser_get_coordsys(i, axis);
		grbl_protocol_printf("G%d:" MSG_AXIS "]" MSG_EOL, (i + 54), axis);
	}
#if COORD_SYS_COUNT >= 6
	for (uint8_t i = 6; i < COORD_SYS_COUNT; i++)
	{
		parser_get_coordsys(i, axis);
		grbl_protocol_printf("G59.%d:" MSG_AXIS "]" MSG_EOL, (i - 5), axis);
	}
#endif

	parser_get_coordsys(28, axis);
	grbl_protocol_printf("G28:" MSG_AXIS "]" MSG_EOL, axis);

	parser_get_coordsys(30, axis);
	grbl_protocol_printf("G30:" MSG_AXIS "]" MSG_EOL, axis);

	parser_get_coordsys(92, axis);
	grbl_protocol_printf("G92:" MSG_AXIS "]" MSG_EOL, axis);

#ifdef AXIS_TOOL
	parser_get_coordsys(254, axis);
	grbl_protocol_printf("[TLO:%1f]" MSG_EOL, axis);
#endif
	grbl_protocol_probe_result(parser_get_probe_result());

	protocol_busy = false;
}

void grbl_protocol_probe_result(uint8_t val)
{
	float axis[MAX(AXIS_COUNT, 3)];
	parser_get_coordsys(255, axis);
	grbl_protocol_printf("PRB:" MSG_AXIS ":%d]" MSG_EOL, axis, val);
}

static void grbl_protocol_parser_modalstate(uint8_t word, uint8_t val, uint8_t mantissa)
{
	if (mantissa)
	{
		grbl_protocol_printf("%d%d.%d ", word, val, mantissa);
	}
	else
	{
		grbl_protocol_printf("%d%d ", word, val);
	}
}

void grbl_protocol_gcode_modes(void)
{
	// leave extra room for future modal groups
	uint8_t modalgroups[MAX_MODAL_GROUPS];
	uint16_t feed;
	uint16_t spindle;

	parser_get_modes(modalgroups, &feed, &spindle);

	grbl_stream_broadcast(true);

	grbl_protocol_print("[GC:");

	grbl_protocol_parser_modalstate('G', modalgroups[0], modalgroups[12]);
	for (uint8_t i = 1; i < 7; i++)
	{
		grbl_protocol_parser_modalstate('G', modalgroups[i], 0);
	}

	if (modalgroups[7] == 62)
	{
		grbl_protocol_parser_modalstate('G', 61, 1);
	}
	else
	{
		grbl_protocol_parser_modalstate('G', modalgroups[7], 0);
	}

#ifdef ENABLE_G39_H_MAPPING
	if (modalgroups[13])
	{
		grbl_protocol_parser_modalstate('G', 39, 2);
	}
	else
	{
		grbl_protocol_parser_modalstate('G', 39, 1);
	}
#endif

#ifdef ENABLE_PARSER_MODULES
	EVENT_INVOKE(grbl_protocol_gcode_modes, NULL);
#endif

	grbl_protocol_parser_modalstate('M', modalgroups[8], 0);
#ifdef ENABLE_COOLANT
	if (modalgroups[9] == M9)
	{
		grbl_protocol_print("M9 ");
	}
#ifndef M7_SAME_AS_M8
	if (modalgroups[9] & M7)
	{
		grbl_protocol_print("M7 ");
	}
#endif
	if (modalgroups[9] & M8)
	{
		grbl_protocol_print("M8 ");
	}
#else
	// permanent M9
	grbl_protocol_print("M9 ");
#endif
	grbl_protocol_printf("M%d T%d F%f S%lu]" MSG_EOL, modalgroups[10], modalgroups[11], feed, spindle);
}

void grbl_protocol_gcode_setting_line_int(setting_offset_t setting, uint16_t value)
{
	grbl_protocol_printf("$%lu=%lu" MSG_EOL, setting, value);
}

void grbl_protocol_gcode_setting_line_flt(setting_offset_t setting, float value)
{
	grbl_protocol_printf("$%lu=%f" MSG_EOL, setting, value);
}

void grbl_protocol_start_blocks(void)
{
	protocol_busy = true;
	uint8_t c = 0;

	for (uint8_t i = 0; i < STARTUP_BLOCKS_COUNT; i++)
	{
		uint16_t address = STARTUP_BLOCK0_ADDRESS_OFFSET + i * STARTUP_BLOCK_SIZE;
		grbl_protocol_printf("$N%d=", i);
		for (;;)
		{
			settings_load(address++, &c, 1);
			if (c > 0 && c < 128)
			{
				grbl_protocol_putc(c);
			}
			else
			{
				grbl_protocol_print(MSG_EOL);
				break;
			}
		}
	}

	protocol_busy = false;
}

void grbl_protocol_cnc_settings(void)
{
	protocol_busy = true;
	uint8_t count = settings_count();
	for (uint8_t i = 0; i < count; i++)
	{
		setting_id_t s = {0};
		uint8_t max = 1;
		rom_memcpy(&s, &g_settings_id_table[i], sizeof(setting_id_t));
		if (s.type & SETTING_ARRAY)
		{
			max = SETTING_ARRCNT(s.type);
		}

		for (uint8_t j = 0; j < max; j++)
		{
			uint32_t val = 0;
			switch (SETTING_TYPE_MASK(s.type))
			{
			case 1:
				val = (uint32_t) * ((bool *)s.memptr);
				grbl_protocol_gcode_setting_line_int(s.id, val);
				break;
			case 2:
				val = (uint32_t) * ((uint8_t *)s.memptr);
				grbl_protocol_gcode_setting_line_int(s.id, val);
				break;
			case 3:
				val = (uint32_t) * ((uint16_t *)s.memptr);
				grbl_protocol_gcode_setting_line_int(s.id, val);
				break;
			default:
				grbl_protocol_gcode_setting_line_flt(s.id, *((float *)s.memptr));
				break;
			}
		}
	}
	/*
	grbl_protocol_gcode_setting_line_flt(0, (1000000.0f / g_settings.max_step_rate));
#if EMULATE_GRBL_STARTUP > 0 || defined(ENABLE_STEPPERS_DISABLE_TIMEOUT)
// just adds this for compatibility
// this setting is not used
#ifdef ENABLE_STEPPERS_DISABLE_TIMEOUT
	grbl_protocol_gcode_setting_line_int(1, g_settings.step_disable_timeout);
#else
	grbl_protocol_gcode_setting_line_int(1, 0);
#endif
#endif
	grbl_protocol_gcode_setting_line_int(2, g_settings.step_invert_mask);
	grbl_protocol_gcode_setting_line_int(3, g_settings.dir_invert_mask);
	grbl_protocol_gcode_setting_line_int(4, g_settings.step_enable_invert);
	grbl_protocol_gcode_setting_line_int(5, g_settings.limits_invert_mask);
	grbl_protocol_gcode_setting_line_int(6, g_settings.probe_invert_mask);
	grbl_protocol_gcode_setting_line_int(7, g_settings.control_invert_mask);
#if ENCODERS > 0
	grbl_protocol_gcode_setting_line_int(8, g_settings.encoders_pulse_invert_mask);
	grbl_protocol_gcode_setting_line_int(9, g_settings.encoders_dir_invert_mask);
#endif
	grbl_protocol_gcode_setting_line_int(10, g_settings.status_report_mask);
	grbl_protocol_gcode_setting_line_flt(11, g_settings.g64_angle_factor);
	grbl_protocol_gcode_setting_line_flt(12, g_settings.arc_tolerance);
	grbl_protocol_gcode_setting_line_int(13, g_settings.report_inches);
#if S_CURVE_ACCELERATION_LEVEL == -1
	grbl_protocol_gcode_setting_line_int(14, g_settings.s_curve_profile);
#endif
	grbl_protocol_gcode_setting_line_int(20, g_settings.soft_limits_enabled);
	grbl_protocol_gcode_setting_line_int(21, g_settings.hard_limits_enabled);
	grbl_protocol_gcode_setting_line_int(22, g_settings.homing_enabled);
	grbl_protocol_gcode_setting_line_int(23, g_settings.homing_dir_invert_mask);
	grbl_protocol_gcode_setting_line_flt(24, g_settings.homing_slow_feed_rate);
	grbl_protocol_gcode_setting_line_flt(25, g_settings.homing_fast_feed_rate);
	grbl_protocol_gcode_setting_line_int(26, g_settings.debounce_ms);
	grbl_protocol_gcode_setting_line_flt(27, g_settings.homing_offset);
#if (KINEMATIC == KINEMATIC_DELTA)
	grbl_protocol_gcode_setting_line_flt(28, g_settings.delta_bicep_homing_angle);
#elif (KINEMATIC == KINEMATIC_SCARA)
	grbl_protocol_gcode_setting_line_flt(28, g_settings.scara_arm_homing_angle);
	grbl_protocol_gcode_setting_line_flt(29, g_settings.scara_forearm_homing_angle);
#endif
	grbl_protocol_gcode_setting_line_int(30, g_settings.spindle_max_rpm);
	grbl_protocol_gcode_setting_line_int(31, g_settings.spindle_min_rpm);
	grbl_protocol_gcode_setting_line_int(32, g_settings.laser_mode);
#ifdef ENABLE_LASER_PPI
	grbl_protocol_gcode_setting_line_int(33, g_settings.laser_ppi);
	grbl_protocol_gcode_setting_line_int(34, g_settings.laser_ppi_uswidth);
	grbl_protocol_gcode_setting_line_flt(35, g_settings.laser_ppi_mixmode_ppi);
	grbl_protocol_gcode_setting_line_flt(36, g_settings.laser_ppi_mixmode_uswidth);
#endif
#ifdef ENABLE_SKEW_COMPENSATION
	grbl_protocol_gcode_setting_line_flt(37, g_settings.skew_xy_factor);
#ifndef SKEW_COMPENSATION_XY_ONLY
	grbl_protocol_gcode_setting_line_flt(38, g_settings.skew_xz_factor);
	grbl_protocol_gcode_setting_line_flt(39, g_settings.skew_yz_factor);
#endif
#endif

#if TOOL_COUNT > 0
#if TOOL_COUNT > 1
	grbl_protocol_gcode_setting_line_int(80, g_settings.default_tool);
#endif
	for (uint8_t i = 0; i < TOOL_COUNT; i++)
	{
		grbl_protocol_gcode_setting_line_flt(81 + i, g_settings.tool_length_offset[i]);
	}
#endif

	for (uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		grbl_protocol_gcode_setting_line_flt(100 + i, g_settings.step_per_mm[i]);
	}

#if (KINEMATIC == KINEMATIC_LINEAR_DELTA)
	grbl_protocol_gcode_setting_line_flt(106, g_settings.delta_arm_length);
	grbl_protocol_gcode_setting_line_flt(107, g_settings.delta_armbase_radius);
	// grbl_protocol_gcode_setting_line_int(108, g_settings.delta_efector_height);
#elif (KINEMATIC == KINEMATIC_DELTA)
	grbl_protocol_gcode_setting_line_flt(106, g_settings.delta_base_radius);
	grbl_protocol_gcode_setting_line_flt(107, g_settings.delta_effector_radius);
	grbl_protocol_gcode_setting_line_flt(108, g_settings.delta_bicep_length);
	grbl_protocol_gcode_setting_line_flt(109, g_settings.delta_forearm_length);
#elif (KINEMATIC == KINEMATIC_SCARA)
	grbl_protocol_gcode_setting_line_flt(106, g_settings.scara_arm_length);
	grbl_protocol_gcode_setting_line_flt(107, g_settings.scara_forearm_length);
#endif

	for (uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		grbl_protocol_gcode_setting_line_flt(110 + i, g_settings.max_feed_rate[i]);
	}

	for (uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		grbl_protocol_gcode_setting_line_flt(120 + i, g_settings.acceleration[i]);
	}

	for (uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		grbl_protocol_gcode_setting_line_flt(130 + i, g_settings.max_distance[i]);
	}

#ifdef ENABLE_BACKLASH_COMPENSATION
	for (uint8_t i = 0; i < AXIS_TO_STEPPERS; i++)
	{
		grbl_protocol_gcode_setting_line_int(140 + i, g_settings.backlash_steps[i]);
	}
#endif
*/

#ifdef ENABLE_SETTINGS_MODULES
	EVENT_INVOKE(grbl_protocol_cnc_settings, NULL);
	grbl_stream_broadcast(false);
#endif
	protocol_busy = false;
}

#ifdef ENABLE_EXTRA_SYSTEM_CMDS
void grbl_protocol_pins_states(void)
{
	protocol_busy = true;
	for (uint8_t i = 0; i < (DIN_PINS_OFFSET + 50); i++)
	{
		i = (i != (DOUT_PINS_OFFSET + 50)) ? i : 100;
		int16_t val = io_get_pinvalue(i);
		if (val >= 0)
		{
			if (i < 100)
			{
				if (i < PWM_PINS_OFFSET)
				{
					grbl_protocol_print("[SO:");
				}
				else if (i < SERVO_PINS_OFFSET)
				{
					grbl_protocol_print("[P:");
				}
				else if (i < DOUT_PINS_OFFSET)
				{
					grbl_protocol_print("[SV:");
				}
				else if (i < (DOUT_PINS_OFFSET + 50))
				{
					grbl_protocol_print("[O:");
				}
#ifdef ENABLE_PIN_TRANSLATIONS
				grbl_protocol_print((const char *)rom_strptr(&outputpins_names[i - 1]));
#endif
			}

			if (i >= 100)
			{
				if (i < ANALOG_PINS_OFFSET)
				{
					grbl_protocol_print("[SI:");
				}
				else if (i < DIN_PINS_OFFSET)
				{
					grbl_protocol_print("[A:");
				}
				else
				{
					grbl_protocol_print("[I:");
				}
#ifdef ENABLE_PIN_TRANSLATIONS
				grbl_protocol_print((const char *)rom_strptr(&inputpins_names[i - 100]));
#endif
			}
#ifndef ENABLE_PIN_TRANSLATIONS
			grbl_stream_print_int(i);
#endif
			grbl_protocol_putc(':');
			grbl_stream_print_int(val);
			grbl_protocol_print(MSG_END);
		}
	}

	int32_t steps[STEPPER_COUNT];
	itp_get_rt_position(steps);
	grbl_protocol_print("[STEPS:");
	grbl_stream_print_intarr(steps, AXIS_TO_STEPPERS);
	grbl_protocol_print(MSG_END);

#if ENCODERS > 0
	encoder_print_values();
#endif

#ifdef ENABLE_IO_MODULES
	EVENT_INVOKE(grbl_protocol_pins_states, NULL);
#endif

	grbl_protocol_print("[RUNTIME:");
	grbl_stream_print_int(mcu_millis());
	grbl_protocol_print(MSG_END);
	protocol_busy = false;
}
#endif

#ifdef ENABLE_SYSTEM_INFO
#ifndef KINEMATIC_TYPE_STR
#define KINEMATIC_TYPE_STR "UK" /*undefined kynematic*/
#endif
#define KINEMATIC_INFO KINEMATIC_TYPE_STR STRGIFY(AXIS_COUNT) ","
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

#if S_CURVE_ACCELERATION_LEVEL != 0
#define DYNACCEL_INFO "S" STRGIFY(S_CURVE_ACCELERATION_LEVEL) ","
#else
#define DYNACCEL_INFO ""
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

#ifdef RAM_ONLY_SETTINGS
#define SETTINGS_INFO "RAM,"
#else
#define SETTINGS_INFO ""
#endif

#ifdef ENABLE_IO_ALARM_DEBUG
#define IODBG_INFO "IODBG,"
#else
#define IODBG_INFO ""
#endif

#ifdef FORCE_SOFT_POLLING
#define SPOLL_INFO "SP,"
#else
#define SPOLL_INFO ""
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

#ifdef ENABLE_G39_H_MAPPING
#define HMAP_INFO "HMAP,"
#else
#define HMAP_INFO ""
#endif

#define DSS_INFO "DSS" STRGIFY(DSS_MAX_OVERSAMPLING) "_" STRGIFY(DSS_CUTOFF_FREQ) ","
#define PLANNER_INFO           \
	STRGIFY(PLANNER_BUFFER_SIZE) \
	","

#define SERIAL_INFO STRGIFY(RX_BUFFER_CAPACITY)

#ifndef BOARD_NAME
#define BOARD_NAME "Generic board"
#endif

#if EMULATE_GRBL_STARTUP < 2
#define EXTENDED_OPT "[OPT:"
#define EXTENDED_VER "[VER:"
#else
#define EXTENDED_OPT "[OPT+:"
#define EXTENDED_VER "[VER+:"
#endif
#define OPT_INFO EXTENDED_OPT KINEMATIC_INFO LINES_INFO BRESENHAM_INFO DSS_INFO DYNACCEL_INFO SKEW_INFO LINPLAN_INFO HMAP_INFO PPI_INFO INVESTOP_INFO SPOLL_INFO CONTROLS_INFO LIMITS_INFO PROBE_INFO IODBG_INFO SETTINGS_INFO EXTRACMD_INFO FASTMATH_INFO
#define VER_INFO EXTENDED_VER " uCNC " CNC_VERSION " - " BOARD_NAME "]" STR_EOL

WEAK_EVENT_HANDLER(grbl_protocol_cnc_info)
{
	// custom handler
	grbl_protocol_cnc_info_delegate_event_t *ptr = grbl_protocol_cnc_info_event;
	while (ptr != NULL)
	{
		if (ptr->fptr != NULL)
		{
			ptr->fptr(args);
			grbl_protocol_putc(',');
		}
		ptr = ptr->next;
	}

	return false;
}

void grbl_protocol_cnc_info(bool extended)
{
	protocol_busy = true;
#if EMULATE_GRBL_STARTUP < 2
	grbl_protocol_print(VER_INFO);
	grbl_protocol_print(OPT_INFO);
	EVENT_INVOKE(grbl_protocol_cnc_info, NULL);
	grbl_protocol_print(PLANNER_INFO SERIAL_INFO "]" STR_EOL);
#else
	if (!extended)
	{
		grbl_protocol_print("[VER:1.1h.20190825:]" STR_EOL);
		grbl_protocol_print("[OPT:V," PLANNER_INFO SERIAL_INFO "]" STR_EOL);
	}
	else
	{
		grbl_protocol_print(VER_INFO);
		grbl_protocol_print(OPT_INFO);
		EVENT_INVOKE(grbl_protocol_cnc_info, NULL);
		grbl_protocol_print(PLANNER_INFO SERIAL_INFO "]" STR_EOL);
	}
#endif
	protocol_busy = false;
}
#endif
