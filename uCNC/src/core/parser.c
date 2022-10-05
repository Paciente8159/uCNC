/*
	Name: parser.c (version 2)
	Description: Parses Grbl system commands and RS274NGC (GCode) commands
		The RS274NGC parser tries to follow the standard document version 3 as close as possible.
		The parsing is done in 3 steps:
			- tokenization; Converts the command string to a structure with GCode parameters
			- Validation; Validates the command by checking all the parameters (part 3.5 - 3.7 of the document)
			- Execution; Executes the command by the orther set in part 3.8 of the document.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 31/07/2020

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../cnc.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <float.h>

#define G0 0
#define G1 1
#define G2 2
#define G3 3
#define G38_2 4
#define G38_3 5
#define G38_4 6
#define G38_5 7
#define G80 8
#define G81 9
#define G82 10
#define G83 11
#define G84 12
#define G85 13
#define G86 14
#define G87 15
#define G88 16
#define G89 17
#define G17 0
#define G18 1
#define G19 2
#define G90 0
#define G91 1
#define G93 0
#define G94 1
#define G20 0
#define G21 1
#define G40 0
#define G41 1
#define G42 2
#define G43 0
#define G49 1
#define G98 0
#define G99 1
#define G54 0
#define G55 1
#define G56 2
#define G57 3
#define G58 4
#define G59 5
#define G59_1 6
#define G59_2 7
#define G59_3 8
#define G61 0
#define G61_1 1
#define G64 3
#define G4 1
#define G10 2
#define G28 3
#define G30 4
#define G53 6
#define G92 10
#define G92_1 11
#define G92_2 12
#define G92_3 13

#define M0 1
#define M1 2
#define M2 3
#define M30 4
#define M60 5
#define M3 1
#define M4 2
#define M5 0
#define M6 0
#define M7 MIST_MASK
#define M8 COOLANT_MASK
#define M9 0
#define M48 1
#define M49 0
#define M10 1010

#define PARSER_PARAM_SIZE (sizeof(float) * AXIS_COUNT)	 // parser parameters array size
#define PARSER_PARAM_ADDR_OFFSET (PARSER_PARAM_SIZE + 1) // parser parameters array size + 1 crc byte
#define G28HOME COORD_SYS_COUNT							 // G28 index
#define G30HOME COORD_SYS_COUNT + 1						 // G30 index
#define G92OFFSET COORD_SYS_COUNT + 2					 // G92 index

#define PARSER_CORDSYS_ADDRESS SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET							  // 1st coordinate system offset eeprom address (G54)
#define G28ADDRESS (SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (PARSER_PARAM_ADDR_OFFSET * G28HOME)) // G28 coordinate offset eeprom address
#define G30ADDRESS (SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (PARSER_PARAM_ADDR_OFFSET * G30HOME)) // G28 coordinate offset eeprom address
#ifdef G92_STORE_NONVOLATILE
#define G92ADDRESS (SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (PARSER_PARAM_ADDR_OFFSET * G92OFFSET)) // G92 coordinate offset eeprom address
#endif

#define NUMBER_UNDEF 0
#define NUMBER_OK 0x20
#define NUMBER_ISFLOAT 0x40
#define NUMBER_ISNEGATIVE 0x80

static parser_state_t parser_state;
static parser_parameters_t parser_parameters;
static uint8_t parser_wco_counter;
static float g92permanentoffset[AXIS_COUNT];
static int32_t rt_probe_step_pos[STEPPER_COUNT];
static float parser_last_pos[AXIS_COUNT];

static unsigned char parser_get_next_preprocessed(bool peek);
FORCEINLINE static void parser_get_comment(unsigned char start_char);
FORCEINLINE static uint8_t parser_get_token(unsigned char *word, float *value);
FORCEINLINE static uint8_t parser_gcode_word(uint8_t code, uint8_t mantissa, parser_state_t *new_state, parser_cmd_explicit_t *cmd);
FORCEINLINE static uint8_t parser_mcode_word(uint8_t code, uint8_t mantissa, parser_state_t *new_state, parser_cmd_explicit_t *cmd);
FORCEINLINE static uint8_t parser_letter_word(unsigned char c, float value, uint8_t mantissa, parser_words_t *words, parser_cmd_explicit_t *cmd);
static uint8_t parse_grbl_exec_code(uint8_t code);
static uint8_t parser_fetch_command(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
static uint8_t parser_validate_command(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
static uint8_t parser_grbl_command(void);
FORCEINLINE static uint8_t parser_gcode_command(void);
static void parser_discard_command(void);
#ifdef ENABLE_CANNED_CYCLES
uint8_t parser_exec_command_block(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
#endif

#ifdef ENABLE_PARSER_MODULES
// event_gcode_parse_handler
WEAK_EVENT_HANDLER(gcode_parse)
{
	DEFAULT_EVENT_HANDLER(gcode_parse);
}

// event_gcode_exec_handler
WEAK_EVENT_HANDLER(gcode_exec)
{
	DEFAULT_EVENT_HANDLER(gcode_exec);
}

// event_gcode_exec_handler
WEAK_EVENT_HANDLER(gcode_exec_modifier)
{
	DEFAULT_EVENT_HANDLER(gcode_exec_modifier);
}

// event_grbl_cmd_handler
WEAK_EVENT_HANDLER(grbl_cmd)
{
	DEFAULT_EVENT_HANDLER(grbl_cmd);
}

// event_parse_token_handler
WEAK_EVENT_HANDLER(parse_token)
{
	DEFAULT_EVENT_HANDLER(parse_token);
}
#endif

/*
	Initializes the gcode parser
*/
void parser_init(void)
{
#ifdef FORCE_GLOBALS_TO_0
	memset(&parser_state, 0, sizeof(parser_state_t));
	memset(&parser_parameters, 0, sizeof(parser_parameters_t));
	parser_wco_counter = 0;
	memset(g92permanentoffset, 0, sizeof(g92permanentoffset));
	memset(rt_probe_step_pos, 0, sizeof(rt_probe_step_pos));
#endif
	memset(parser_last_pos, 0, sizeof(parser_last_pos));
	parser_parameters_load();
	parser_reset();
}

uint8_t parser_read_command(void)
{
	uint8_t error = STATUS_OK;
	unsigned char c = serial_peek();

	if (c == '$')
	{
		error = parser_grbl_command();

		if (error >= GRBL_SYSTEM_CMD)
		{
			if (error != GRBL_JOG_CMD)
			{
				return parse_grbl_exec_code(error);
			}
		}
		else
		{
			return error;
		}
	}

	if (error == GRBL_JOG_CMD)
	{
		if (cnc_get_exec_state(~EXEC_JOG))
		{
			return STATUS_SYSTEM_GC_LOCK;
		}
		error = parser_gcode_command();
		itp_sync();
		cnc_clear_exec_state(EXEC_JOG);
		return error;
	}
	else if (cnc_get_exec_state(~(EXEC_RUN | EXEC_HOLD | EXEC_RESUMING)))
	{
		parser_discard_command();
		return STATUS_SYSTEM_GC_LOCK;
	}

	return parser_gcode_command();
}

void parser_get_modes(uint8_t *modalgroups, uint16_t *feed, uint16_t *spindle, uint8_t *coolant)
{
	modalgroups[0] = (parser_state.groups.motion < 8) ? parser_state.groups.motion : (72 + parser_state.groups.motion);
	modalgroups[1] = parser_state.groups.plane + 17;
	modalgroups[2] = parser_state.groups.distance_mode + 90;
	modalgroups[3] = parser_state.groups.feedrate_mode + 93;
	modalgroups[4] = parser_state.groups.units + 20;
	modalgroups[5] = ((parser_state.groups.tlo_mode == G49) ? 49 : 43);
	modalgroups[6] = parser_state.groups.coord_system + 54;
	modalgroups[7] = parser_state.groups.path_mode + 61;
#if TOOL_COUNT > 0
	modalgroups[8] = ((parser_state.groups.spindle_turning == M5) ? 5 : (2 + parser_state.groups.spindle_turning));
	*spindle = (uint16_t)ABS(parser_state.spindle);
	*coolant = parser_state.groups.coolant;
	modalgroups[9] = (parser_state.groups.coolant == M9) ? 9 : MIN(parser_state.groups.coolant + 6, 8);
	modalgroups[11] = parser_state.tool_index;
#else
	modalgroups[8] = 5;
	modalgroups[9] = 9;
	modalgroups[11] = 0;
#endif
	modalgroups[10] = 49 - parser_state.groups.feed_speed_override;
	*feed = (uint16_t)parser_state.feedrate;
}

void parser_get_coordsys(uint8_t system_num, float *axis)
{
	switch (system_num)
	{
	case 255:
		memcpy(axis, parser_parameters.last_probe_position, sizeof(parser_parameters.last_probe_position));
		break;
	case 254:
		memcpy(axis, (float *)&parser_parameters.tool_length_offset, sizeof(float));
		break;
	case 28:
		settings_load(G28ADDRESS, (uint8_t *)axis, PARSER_PARAM_SIZE);
		break;
	case 30:
		settings_load(G30ADDRESS, (uint8_t *)axis, PARSER_PARAM_SIZE);
		break;
	case 92:
		memcpy(axis, parser_parameters.g92_offset, sizeof(parser_parameters.g92_offset));
		break;
	default:
		settings_load(PARSER_CORDSYS_ADDRESS + (system_num * PARSER_PARAM_ADDR_OFFSET), (uint8_t *)axis, PARSER_PARAM_SIZE);
		break;
	}
}

uint8_t parser_get_probe_result(void)
{
	return parser_parameters.last_probe_ok;
}

void parser_parameters_reset(void)
{
	// erase all parameters for G54..G59.x coordinate systems
	memset(parser_parameters.coord_system_offset, 0, sizeof(parser_parameters.coord_system_offset));
	for (uint8_t i = 0; i < COORD_SYS_COUNT; i++)
	{
		settings_erase(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (i * PARSER_PARAM_ADDR_OFFSET), PARSER_PARAM_SIZE);
	}

// erase G92
#ifdef G92_STORE_NONVOLATILE
	settings_erase(G92ADDRESS, PARSER_PARAM_SIZE);
	memset(g92permanentoffset, 0, sizeof(g92permanentoffset));
#endif
}

bool parser_get_wco(float *axis)
{
	if (!parser_wco_counter)
	{
		for (uint8_t i = AXIS_COUNT; i != 0;)
		{
			i--;
			axis[i] = parser_parameters.g92_offset[i] + parser_parameters.coord_system_offset[i];
		}

#ifdef AXIS_TOOL
		axis[AXIS_TOOL] += parser_parameters.tool_length_offset;
#endif
		parser_wco_counter = STATUS_WCO_REPORT_MIN_FREQUENCY;
		return true;
	}

	parser_wco_counter--;
	return false;
}

void parser_sync_probe(void)
{
	itp_get_rt_position(rt_probe_step_pos);
}

void parser_update_probe_pos(void)
{
	kinematics_apply_forward(rt_probe_step_pos, parser_parameters.last_probe_position);
	kinematics_apply_reverse_transform(parser_parameters.last_probe_position);
}

static uint8_t parser_grbl_command(void)
{
	serial_getc(); // eat $
	unsigned char c = serial_getc();
	unsigned char grbl_cmd_str[32];
	uint8_t grbl_cmd_len = 0;

	// if not IDLE
	if (cnc_get_exec_state(EXEC_RUN))
	{
		switch (c)
		{
		case '#':
		case 'G':
		case 'P':
		case 'I':
			break;
		default:
			parser_discard_command();
			return STATUS_IDLE_ERROR;
		}
	}

	while ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
	{
		// toupper
		if (c >= 'a' && c <= 'z')
		{
			c -= 32;
		}
		grbl_cmd_str[grbl_cmd_len++] = c;
		c = serial_getc();
	}

	grbl_cmd_str[grbl_cmd_len] = 0;

	uint16_t block_address = STARTUP_BLOCK0_ADDRESS_OFFSET;
	uint8_t error = STATUS_INVALID_STATEMENT;

	parser_state_t next_state = {0};
	parser_words_t words = {0};
	parser_cmd_explicit_t cmd = {0};

	switch (grbl_cmd_len)
	{
	case 0:
		switch (c)
		{
		case '$':
			if (serial_getc() != EOL)
			{
				return STATUS_INVALID_STATEMENT;
			}
			return GRBL_SEND_SYSTEM_SETTINGS;
		case '#':
			if (serial_getc() != EOL)
			{
				return STATUS_INVALID_STATEMENT;
			}
			return GRBL_SEND_COORD_SYSTEM;
		case EOL:
			return GRBL_HELP;
		default:
			if (c >= '0' && c <= '9') // settings
			{
				float val = 0;
				uint8_t setting_num = 0;
				serial_ungetc();
				error = parser_get_float(&val);
				if (!error)
				{
					return STATUS_INVALID_STATEMENT;
				}

				if ((error & NUMBER_ISFLOAT) || val > 255 || val < 0)
				{
					return STATUS_INVALID_STATEMENT;
				}

				setting_num = (uint8_t)val;
				// eat '='
				if (serial_getc() != '=')
				{
					return STATUS_INVALID_STATEMENT;
				}

				val = 0;
				if (!parser_get_float(&val))
				{
					return STATUS_BAD_NUMBER_FORMAT;
				}

				if (serial_getc() != EOL)
				{
					return STATUS_INVALID_STATEMENT;
				}

				return settings_change(setting_num, val);
			}
			return STATUS_INVALID_STATEMENT;
		}
		break;
	case 1:
		if (c != EOL && grbl_cmd_str[0] != 'J' && grbl_cmd_str[0] != 'N')
		{
			return STATUS_INVALID_STATEMENT;
		}
		switch (grbl_cmd_str[0])
		{
		case 'H':
			return GRBL_HOME;
		case 'X':
			return GRBL_UNLOCK;
		case 'G':
			return GRBL_SEND_PARSER_MODES;
		case 'C':
			return GRBL_TOGGLE_CHECKMODE;
		case 'N':
			switch (c)
			{
			case '0':
			case '1':
				block_address = (!(c - '0') ? STARTUP_BLOCK0_ADDRESS_OFFSET : STARTUP_BLOCK1_ADDRESS_OFFSET);
				if (serial_getc() != '=')
				{
					return STATUS_INVALID_STATEMENT;
				}

				error = parser_fetch_command(&next_state, &words, &cmd);
				if (error)
				{
					return error;
				}
				error = parser_validate_command(&next_state, &words, &cmd);
				if (error)
				{
					return error;
				}
				// everything ok reverts string and saves it
				do
				{
					serial_ungetc();
				} while (serial_peek() != '=');
				serial_getc();
				settings_save_startup_gcode(block_address);
				return STATUS_OK;
			case EOL:
				return GRBL_SEND_STARTUP_BLOCKS;
			}
			return STATUS_INVALID_STATEMENT;
#ifdef ENABLE_EXTRA_SYSTEM_CMDS
		case 'P':
			return GRBL_PINS_STATES;
#endif
#ifdef ENABLE_SYSTEM_INFO
		case 'I':
			return GRBL_SEND_SYSTEM_INFO;
#endif
		case 'J':
			if (c != '=')
			{
				break;
			}
			if (cnc_get_exec_state(EXEC_ALLACTIVE) & !cnc_get_exec_state(EXEC_JOG)) // Jog only allowed in IDLE or JOG mode
			{
				return STATUS_IDLE_ERROR;
			}
			cnc_set_exec_state(EXEC_JOG);
			return GRBL_JOG_CMD;
		}
		break;
	default:
		switch (grbl_cmd_str[0])
		{
		case 'R':
			if (grbl_cmd_str[1] == 'S' && grbl_cmd_str[2] == 'T' && c == '=' && grbl_cmd_len == 3)
			{
				switch (serial_getc())
				{
				case '$':
					settings_reset(false);
					settings_save(SETTINGS_ADDRESS_OFFSET, (uint8_t *)&g_settings, (uint8_t)sizeof(settings_t));
					return GRBL_SEND_SETTINGS_RESET;
				case '#':
					parser_parameters_reset();
					return GRBL_SEND_SETTINGS_RESET;
				case '*':
					settings_reset(true);
					settings_save(SETTINGS_ADDRESS_OFFSET, (uint8_t *)&g_settings, (uint8_t)sizeof(settings_t));
					parser_parameters_reset();
					return GRBL_SEND_SETTINGS_RESET;
				default:
					return STATUS_INVALID_STATEMENT;
				}
			}
			break;
#ifdef ENABLE_EXTRA_SYSTEM_CMDS
		case 'S':
			// new settings command
			if (c == EOL && grbl_cmd_len == 2)
			{
				switch (grbl_cmd_str[1])
				{
				case 'S':
					settings_save(SETTINGS_ADDRESS_OFFSET, (uint8_t *)&g_settings, (uint8_t)sizeof(settings_t));
					return GRBL_SETTINGS_SAVED;
				case 'L':
					settings_init();
					return GRBL_SETTINGS_LOADED;
				case 'R':
					settings_reset(false);
					return GRBL_SETTINGS_DEFAULT;
				}
			}
			break;
#endif
		}
		break;
	}

#ifdef ENABLE_PARSER_MODULES
	grbl_cmd_args_t args = {grbl_cmd_str, grbl_cmd_len};
	uint8_t newerror = EVENT_INVOKE(grbl_cmd, &args);
	if (newerror >= GRBL_SYSTEM_CMD)
	{
		error = newerror;
	}
#endif

	return error;
}

static uint8_t parse_grbl_exec_code(uint8_t code)
{

	switch (code)
	{
	case GRBL_SEND_SYSTEM_SETTINGS:
		protocol_send_cnc_settings();
		break;
	case GRBL_SEND_COORD_SYSTEM:
		protocol_send_gcode_coordsys();
		break;
	case GRBL_SEND_PARSER_MODES:
		protocol_send_gcode_modes();
		break;
	case GRBL_SEND_STARTUP_BLOCKS:
		protocol_send_start_blocks();
		break;
	case GRBL_TOGGLE_CHECKMODE:
		if (mc_toogle_checkmode())
		{
			protocol_send_feedback(MSG_FEEDBACK_4);
		}
		else
		{
			cnc_alarm(EXEC_ALARM_SOFTRESET);
			protocol_send_feedback(MSG_FEEDBACK_5);
		}
		break;
	case GRBL_SEND_SETTINGS_RESET:
		protocol_send_feedback(MSG_FEEDBACK_9);
		break;
	case GRBL_UNLOCK:
		cnc_unlock(true);
		if (cnc_get_exec_state(EXEC_DOOR))
		{
			return STATUS_CHECK_DOOR;
		}
		protocol_send_feedback(MSG_FEEDBACK_3);
		break;
	case GRBL_HOME:
		if (!g_settings.homing_enabled)
		{
			return STATUS_SETTING_DISABLED;
		}

		cnc_unlock(true);
		if (cnc_get_exec_state(EXEC_DOOR))
		{
			return STATUS_CHECK_DOOR;
		}

		cnc_home();
		break;
	case GRBL_HELP:
		protocol_send_string(MSG_HELP);
		break;
#ifdef ENABLE_EXTRA_SYSTEM_CMDS
	case GRBL_SETTINGS_SAVED:
		protocol_send_feedback(MSG_FEEDBACK_13);
		break;
	case GRBL_SETTINGS_LOADED:
		protocol_send_feedback(MSG_FEEDBACK_14);
		break;
	case GRBL_SETTINGS_DEFAULT:
		protocol_send_feedback(MSG_FEEDBACK_15);
		break;
	case GRBL_PINS_STATES:
		protocol_send_pins_states();
		break;
#endif
#ifdef ENABLE_SYSTEM_INFO
	case GRBL_SEND_SYSTEM_INFO:
		protocol_send_cnc_info();
		break;
#endif
#ifdef ENABLE_PARSER_MODULES
	case GRBL_SYSTEM_CMD_EXTENDED:
		break;
#endif
	default:
		return code;
	}

	return STATUS_OK;
}

/*
	STEP 1
	Fetches the next line from the mcu communication buffer and preprocesses the string
	In the preprocess these steps are executed
		1. Whitespaces are ignored
		2. Comments are parsed (nothing is done besides parsing for now)
		3. All letters are upper-cased
		4. Checks number formats in all words
		5. Checks for modal groups and words collisions
*/
static uint8_t parser_fetch_command(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
#ifdef GCODE_COUNT_TEXT_LINES
	static uint32_t linecounter = 0;
#endif
	uint8_t error = STATUS_OK;
	uint8_t wordcount = 0;
	for (;;)
	{
		unsigned char word = 0;
		float value = 0;
		// this flushes leading white chars and also takes care of processing comments
		parser_get_next_preprocessed(true);
#ifdef ECHO_CMD
		if (!wordcount)
		{
			protocol_send_string(MSG_ECHO);
		}
#endif
		error = parser_get_token(&word, &value);
		if (error)
		{
			parser_discard_command();
#ifdef ECHO_CMD
			protocol_send_string(MSG_END);
#endif
			return error;
		}
		uint8_t code = (uint8_t)floorf(value);
		// check mantissa
		uint8_t mantissa = (uint8_t)roundf((value - code) * 100.0f);

		switch (word)
		{
		case EOL:
#ifdef GCODE_COUNT_TEXT_LINES
			// if enabled store line number
			linecounter++;
			words->n = linecounter;
#endif
#ifdef ECHO_CMD
			protocol_send_string(MSG_END);
#endif
			return STATUS_OK;
		case 'G':
			error = parser_gcode_word(code, mantissa, new_state, cmd);
			break;
		case 'M':
			error = parser_mcode_word(code, mantissa, new_state, cmd);
			break;
		default:
			if (word == 'N' && wordcount != 0)
			{
				error = STATUS_GCODE_INVALID_LINE_NUMBER;
				break;
			}
			error = parser_letter_word(word, value, mantissa, words, cmd);
			break;
		}

#ifdef ENABLE_PARSER_MODULES
		if ((error == STATUS_GCODE_UNSUPPORTED_COMMAND || error == STATUS_GCODE_UNUSED_WORDS))
		{
			gcode_parse_args_t args = {word, code, error, value, new_state, words, cmd};
			uint8_t newerror = EVENT_INVOKE(gcode_parse, &args);
			// is extended command
			if (cmd->group_extended != 0)
			{
				error = newerror;
			}
		}
#endif

		if (error)
		{
			parser_discard_command();
#ifdef ECHO_CMD
			protocol_send_string(MSG_END);
#endif
			return error;
		}

		wordcount++;
	}
	// Never should reach
	return STATUS_CRITICAL_FAIL;
}

/*
	STEP 2
	Validadates command by checking for errors on all G/M Codes
		RS274NGC v3 - 3.5 G Codes
		RS274NGC v3 - 3.6 Input M Codes
		RS274NGC v3 - 3.7 Other Input Codes
*/

static uint8_t parser_validate_command(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
	// only alow groups 3, 6 and modal G53
	if (cnc_get_exec_state(EXEC_JOG))
	{
		if (cmd->groups & ~(GCODE_GROUP_DISTANCE | GCODE_GROUP_UNITS | GCODE_GROUP_NONMODAL))
		{
			return STATUS_INVALID_JOG_COMMAND;
		}

		// if nonmodal other than G53
		if (new_state->groups.nonmodal != 0 && new_state->groups.nonmodal != G53)
		{
			return STATUS_INVALID_JOG_COMMAND;
		}

		if (cmd->words & GCODE_JOG_INVALID_WORDS)
		{
			return STATUS_INVALID_JOG_COMMAND;
		}

		new_state->groups.motion = 1;
		SETFLAG(cmd->groups, GCODE_GROUP_MOTION);
	}

	// RS274NGC v3 - 3.5 G Codes
	// group 0 - non modal (incomplete)
	if ((cmd->groups & GCODE_GROUP_NONMODAL))
	{
		switch (new_state->groups.nonmodal)
		{
		case G10:
			// G10
			// if no P or L is present
			if (!(cmd->words & (GCODE_WORD_P | GCODE_WORD_L)))
			{
				return STATUS_GCODE_VALUE_WORD_MISSING;
			}
			// L is not 2
			if (words->l != 2 && words->l != 20)
			{
				return STATUS_GCODE_UNSUPPORTED_COMMAND;
			}
			// P is not between 1 and N of coord systems
			if (words->p != 28 && words->p != 30)
			{
				if (words->p > COORD_SYS_COUNT)
				{
					return STATUS_GCODE_UNSUPPORTED_COORD_SYS;
				}
			}
		case G92:
			if (!CHECKFLAG(cmd->words, GCODE_ALL_AXIS))
			{
				return STATUS_GCODE_NO_AXIS_WORDS;
			}
			break;
		case G53:
			// G53
			// if no G0 or G1 not active
			if (new_state->groups.motion > G1)
			{
				return STATUS_GCODE_G53_INVALID_MOTION_MODE;
			}
			break;
		}
	}

	// group 1 - motion (incomplete)
	// TODO
	// 81...89 Canned cycles
	if (CHECKFLAG(cmd->groups, GCODE_GROUP_MOTION))
	{
		switch (new_state->groups.motion)
		{
#ifndef IGNORE_G0_G1_MISSING_AXIS_WORDS
		case G0: // G0
		case G1: // G1
#endif
		case G38_2: // G38.2
		case G38_3: // G38.3
		case G38_4: // G38.4
		case G38_5: // G38.5
			if (!CHECKFLAG(cmd->words, GCODE_ALL_AXIS))
			{
				return STATUS_GCODE_NO_AXIS_WORDS;
			}
			break;
		case G2:
		case G3:
			switch (new_state->groups.plane)
			{
			case G17:
				if (!CHECKFLAG(cmd->words, GCODE_XYPLANE_AXIS))
				{
					return STATUS_GCODE_NO_AXIS_WORDS_IN_PLANE;
				}

				if (!CHECKFLAG(cmd->words, (GCODE_IJPLANE_AXIS | GCODE_WORD_R)))
				{
					return STATUS_GCODE_NO_OFFSETS_IN_PLANE;
				}
				break;
			case G18:
				if (!CHECKFLAG(cmd->words, GCODE_XZPLANE_AXIS))
				{
					return STATUS_GCODE_NO_AXIS_WORDS_IN_PLANE;
				}

				if (!CHECKFLAG(cmd->words, (GCODE_IKPLANE_AXIS | GCODE_WORD_R)))
				{
					return STATUS_GCODE_NO_OFFSETS_IN_PLANE;
				}
				break;
			case G19:
				if (!CHECKFLAG(cmd->words, GCODE_YZPLANE_AXIS))
				{
					return STATUS_GCODE_NO_AXIS_WORDS_IN_PLANE;
				}

				if (!CHECKFLAG(cmd->words, (GCODE_JKPLANE_AXIS | GCODE_WORD_R)))
				{
					return STATUS_GCODE_NO_OFFSETS_IN_PLANE;
				}
				break;
			}
			break;
		case G80: // G80 and
			if (CHECKFLAG(cmd->words, GCODE_ALL_AXIS) && !cmd->group_0_1_useaxis)
			{
				return STATUS_GCODE_AXIS_WORDS_EXIST;
			}

			break;
#ifdef ENABLE_CANNED_CYCLES
		default: // G81..G89 canned cycles (partially implemented)
			// It is an error if:
			// X, Y, and Z words are all missing during a canned cycle,
			if (!CHECKFLAG(cmd->words, GCODE_ALL_AXIS))
			{
				return STATUS_GCODE_NO_AXIS_WORDS;
			}

			// a P number is required and a negative P number is used
			if (new_state->groups.motion == G82 || new_state->groups.motion == G86 || new_state->groups.motion == G88 || new_state->groups.motion == G89)
			{
				if (!CHECKFLAG(cmd->words, GCODE_WORD_P))
				{
					return STATUS_GCODE_CANNED_CYCLE_MISSING_P;
				}
			}

			if (new_state->groups.motion == G83)
			{
				if (!CHECKFLAG(cmd->words, GCODE_WORD_Q))
				{
					return STATUS_GCODE_CANNED_CYCLE_MISSING_Q;
				}
			}

			break;
#endif
		}

		// group 5 - feed rate mode
		if (new_state->groups.motion >= G1 && new_state->groups.motion <= G38_5)
		{
			if (!CHECKFLAG(cmd->words, GCODE_WORD_F))
			{
				if (new_state->groups.feedrate_mode == G93)
				{
					return STATUS_GCODE_UNDEFINED_FEED_RATE;
				}

				if (new_state->feedrate == 0)
				{
					return STATUS_GCODE_UNDEFINED_FEED_RATE;
				}
			}
			else
			{
				if (words->f <= 0)
				{
					return STATUS_GCODE_UNDEFINED_FEED_RATE;
				}
			}
		}
	}

	// group 2 - plane selection (nothing to be checked)
	// group 3 - distance mode (nothing to be checked)

	// group 6 - units (nothing to be checked)
	// group 7 - cutter radius compensation (not implemented yet)
	// group 8 - tool length offset
	if ((new_state->groups.tlo_mode == G43) && CHECKFLAG(cmd->groups, GCODE_GROUP_TOOLLENGTH))
	{
		if (!CHECKFLAG(cmd->words, GCODE_WORD_Z))
		{
			return STATUS_GCODE_AXIS_WORDS_EXIST;
		}

		// since G43.1 (and currently G43) support and uses word Z it can't be in the same line as a MOTION group command
		// this may be reviewed in the future
		if (CHECKFLAG(cmd->groups, GCODE_GROUP_MOTION))
		{
			return STATUS_GCODE_MODAL_GROUP_VIOLATION;
		}
	}
// group 10 - return mode in canned cycles (not implemented yet)
// group 12 - coordinate system selection (nothing to be checked)
// group 13 - path control mode (nothing to be checked)

// RS274NGC v3 - 3.6 Input M Codes
// group 4 - stopping (nothing to be checked)
// group 6 - tool change(nothing to be checked)
// group 7 - spindle turning (nothing to be checked)
// group 8 - coolant (nothing to be checked)
// group 9 - enable/disable feed and speed override switches (not implemented)

// RS274NGC v3 - 3.7 Other Input Codes
// Words S and T must be positive
#if TOOL_COUNT > 0
	if (words->s < 0 || words->t < 0)
	{
		return STATUS_NEGATIVE_VALUE;
	}

	if (words->t > TOOL_COUNT)
	{
		return STATUS_INVALID_TOOL;
	}
#endif

	// checks if an extended command was called with any other command at the same time
	// exetension commands can only be processed individually
	if (cmd->group_extended != 0 && cmd->groups != 0)
	{
		return STATUS_GCODE_MODAL_GROUP_VIOLATION;
	}

	return STATUS_OK;
}

/*
	STEP 3
	Executes the command
		Follows the RS274NGC v3 - 3.8 Order of Execution
	All coordinates are converted to machine absolute coordinates before sent to the motion controller
*/
uint8_t parser_exec_command(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
	float target[AXIS_COUNT];
	// plane selection
	uint8_t a = 0;
	uint8_t b = 0;
	uint8_t offset_a = 0;
	uint8_t offset_b = 0;
	float radius;
	motion_data_t block_data = {0};
	uint8_t error = 0;
	bool update_tools = false;

#ifdef ENABLE_PARSER_MODULES
	gcode_exec_args_t args = {new_state, words, cmd};
	EVENT_INVOKE(gcode_exec_modifier, &args);
#endif

	// stoping from previous command M2 or M30 command
	if (new_state->groups.stopping && !CHECKFLAG(cmd->groups, GCODE_GROUP_STOPPING))
	{
		if (new_state->groups.stopping == 3 || new_state->groups.stopping == 4)
		{
			return STATUS_PROGRAM_ENDED;
		}

		new_state->groups.stopping = 0;
	}

#if SERVOS_MASK != 0
	if (cmd->group_extended == M10)
	{
		if (CHECKFLAG(cmd->words, (GCODE_WORD_S | GCODE_WORD_P)) != (GCODE_WORD_S | GCODE_WORD_P))
		{
			return STATUS_GCODE_VALUE_WORD_MISSING;
		}

		if (words->p < 6)
		{
			io_set_pwm(words->p + SERVO0_UCNC_INTERNAL_PIN, (uint8_t)CLAMP(0, (uint8_t)trunc(words->s), 255));
		}

		return STATUS_OK;
	}
#endif

#ifdef ENABLE_PARSER_MODULES
	if ((cmd->group_extended != 0))
	{
		return EVENT_INVOKE(gcode_exec, &args);
	}
#endif

#ifdef GCODE_PROCESS_LINE_NUMBERS
	block_data.line = words->n;
#endif

	// RS274NGC v3 - 3.8 Order of Execution
	// 1. comment (ignored - already filtered)
	// 2. set feed mode
	block_data.motion_mode = MOTIONCONTROL_MODE_FEED; // default mode
	if (new_state->groups.feedrate_mode == G93)
	{
		block_data.motion_mode |= MOTIONCONTROL_MODE_INVERSEFEED;
	}
	// 3. set feed rate (µCNC works in units per second and not per minute)
	if (CHECKFLAG(cmd->words, GCODE_WORD_F))
	{
		new_state->feedrate = words->f; // * MIN_SEC_MULT;
	}

// 4. set spindle speed
#if TOOL_COUNT > 0
	if (CHECKFLAG(cmd->words, GCODE_WORD_S))
	{
		new_state->spindle = (uint16_t)trunc(words->s);
	}

	// 5. select tool
	if (CHECKFLAG(cmd->words, GCODE_WORD_T))
	{
		if (new_state->tool_index != words->t)
		{
			new_state->groups.tool_change = words->t;
		}
	}

	// 6. M6 change tool
	if (CHECKFLAG(cmd->groups, GCODE_GROUP_TOOLCHANGE))
	{
		itp_sync();
		// tool 0 is the same as no tool (has stated in RS274NGC v3 - 3.7.3)
		tool_change(words->t);
		new_state->tool_index = new_state->groups.tool_change;
	}

	// 7. spindle on/rev/off (M3/M4/M5)
	block_data.spindle = new_state->spindle;
	block_data.motion_flags.bit.spindle_running = new_state->groups.spindle_turning;
	update_tools = ((parser_state.spindle != new_state->spindle) | (parser_state.groups.spindle_turning != new_state->groups.spindle_turning));

	// spindle speed or direction was changed (force a safety dwell to let the spindle change speed and continue)
	if (update_tools && !g_settings.laser_mode)
	{
		mc_update_tools(&block_data);
#if (DELAY_ON_SPINDLE_SPEED_CHANGE > 0)
		block_data.dwell = (uint16_t)roundf(DELAY_ON_SPINDLE_SPEED_CHANGE * 1000);
#endif
#if (defined(TOOL_WAIT_FOR_SPEED) && (TOOL_WAIT_FOR_SPEED_MAX_ERROR != 100))
		float tool_speed_error = 0;
		float set_speed = (new_state->groups.spindle_turning) ? new_state->spindle : 0;
		do
		{
			tool_speed_error = ABS((float)tool_get_speed() - set_speed) / set_speed;
			if (!cnc_dotasks())
			{
				return STATUS_CRITICAL_FAIL;
			}
		} while (tool_speed_error > ((float)TOOL_WAIT_FOR_SPEED_MAX_ERROR * 0.01f));
#endif
	}

	// 8. coolant on/off
	update_tools |= (parser_state.groups.coolant != new_state->groups.coolant);
	block_data.motion_flags.bit.coolant = new_state->groups.coolant;
	// moving to planner
	// parser_update_coolant(new_state->groups.coolant);
#endif

	// 9. overrides
	block_data.motion_flags.bit.feed_override = new_state->groups.feed_speed_override ? 1 : 0;

	// 10. dwell
	if (new_state->groups.nonmodal == G4)
	{
		// calc dwell in milliseconds
		block_data.dwell = MAX(block_data.dwell, (uint16_t)roundf(MIN(words->p * 1000.f, 65535)));
		new_state->groups.nonmodal = 0;
	}

	// after all spindle, overrides, coolant and dwells are set
	// execute sync if dwell is present
	if (block_data.dwell)
	{
		mc_dwell(&block_data);
	}

	// 11. set active plane (G17, G18, G19)
	switch (new_state->groups.plane)
	{
#if (defined(AXIS_X) && defined(AXIS_Y))
	case 0:
		a = AXIS_X;
		b = AXIS_Y;
		offset_a = AXIS_X;
		offset_b = AXIS_Y;
		break;
#endif
#if (defined(AXIS_X) && defined(AXIS_Z))
	case 1:
		a = AXIS_Z;
		b = AXIS_X;
		offset_a = AXIS_Z;
		offset_b = AXIS_X;
		break;
#endif
#if (defined(AXIS_Y) && defined(AXIS_Z))
	case 2:
		a = AXIS_Y;
		b = AXIS_Z;
		offset_a = AXIS_Y;
		offset_b = AXIS_Z;
		break;
#endif
	}

	// 12. set length units (G20, G21).
	if (new_state->groups.units == G20) // all internal state variables must be converted to mm
	{
		for (uint8_t i = AXIS_COUNT; i != 0;)
		{
			i--;
			words->xyzabc[i] *= INCH_MM_MULT;
		}

		// check if any i, j or k words were used
		if (CHECKFLAG(cmd->words, GCODE_IJK_AXIS))
		{
			words->ijk[0] *= INCH_MM_MULT;
			words->ijk[1] *= INCH_MM_MULT;
			words->ijk[2] *= INCH_MM_MULT;
		}

		// if normal feed mode convert to mm/s
		if (CHECKFLAG(cmd->words, GCODE_WORD_F) && (new_state->groups.feedrate_mode != G93))
		{
			new_state->feedrate *= INCH_MM_MULT;
		}

		if (CHECKFLAG(cmd->words, GCODE_WORD_R))
		{
			words->r *= INCH_MM_MULT;
		}
	}

// 13. cutter radius compensation on or off (G40, G41, G42) (not implemented yet)
// 14. cutter length compensation on or off (G43.1, G49)
#ifdef AXIS_TOOL
	if (CHECKFLAG(cmd->groups, GCODE_GROUP_TOOLLENGTH))
	{
		parser_parameters.tool_length_offset = 0;
		if (new_state->groups.tlo_mode == G43)
		{
			parser_parameters.tool_length_offset = words->xyzabc[AXIS_Z];
			CLEARFLAG(cmd->words, GCODE_WORD_Z);
			words->xyzabc[AXIS_TOOL] = 0; // resets parameter so it it doen't do anything else
		}
		parser_wco_counter = 0;
	}
#endif
	// 15. coordinate system selection (G54, G55, G56, G57, G58, G59, G59.1, G59.2, G59.3) (OK nothing to be done)
	if (CHECKFLAG(cmd->groups, GCODE_GROUP_COORDSYS))
	{
		parser_parameters.coord_system_index = new_state->groups.coord_system;
		settings_load(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (parser_parameters.coord_system_index * PARSER_PARAM_ADDR_OFFSET), (uint8_t *)&parser_parameters.coord_system_offset[0], PARSER_PARAM_SIZE);
		parser_wco_counter = 0;
	}
	// 16. set path control mode (G61, G61.1, G64)
	switch (new_state->groups.path_mode)
	{
	case G61_1:
		block_data.motion_mode |= PLANNER_MOTION_EXACT_STOP;
		break;
	case G64:
		block_data.motion_mode |= PLANNER_MOTION_CONTINUOUS;
		break;
	}

	// 17. set distance mode (G90, G91)
	memcpy(target, parser_last_pos, sizeof(parser_last_pos));

	// for all not explicitly declared target retain their position or add offset
	bool abspos = (new_state->groups.distance_mode == G90) | (new_state->groups.nonmodal == G53);
#ifdef AXIS_X
	if (CHECKFLAG(cmd->words, GCODE_WORD_X))
	{
		target[AXIS_X] = (abspos) ? words->xyzabc[AXIS_X] : (words->xyzabc[AXIS_X] + target[AXIS_X]);
	}
#endif
#ifdef AXIS_Y
	if (CHECKFLAG(cmd->words, GCODE_WORD_Y))
	{
		target[AXIS_Y] = (abspos) ? words->xyzabc[AXIS_Y] : (words->xyzabc[AXIS_Y] + target[AXIS_Y]);
	}
#endif
#ifdef AXIS_Z
	if (CHECKFLAG(cmd->words, GCODE_WORD_Z))
	{
		target[AXIS_Z] = (abspos) ? words->xyzabc[AXIS_Z] : (words->xyzabc[AXIS_Z] + target[AXIS_Z]);
	}
#endif
#ifdef AXIS_A
	if (CHECKFLAG(cmd->words, GCODE_WORD_A))
	{
		target[AXIS_A] = (abspos) ? words->xyzabc[AXIS_A] : (words->xyzabc[AXIS_A] + target[AXIS_A]);
	}
#endif
#ifdef AXIS_B
	if (CHECKFLAG(cmd->words, GCODE_WORD_B))
	{
		target[AXIS_B] = (abspos) ? words->xyzabc[AXIS_B] : (words->xyzabc[AXIS_B] + target[AXIS_B]);
	}
#endif
#ifdef AXIS_C
	if (CHECKFLAG(cmd->words, GCODE_WORD_C))
	{
		target[AXIS_C] = (abspos) ? words->xyzabc[AXIS_C] : (words->xyzabc[AXIS_C] + target[AXIS_C]);
	}
#endif

	// 18. set retract mode (G98, G99)  (not implemented yet)
	// 19. home (G28, G30) or change coordinate system data (G10) or set target offsets (G92, G92.1, G92.2, G92.3)
	//	or also modifies target if G53 is active. These are executed after calculating intemediate targets (G28 ad G30)
	// set the initial feedrate to the maximum value
	block_data.feed = FLT_MAX;
	// limit feed to the maximum possible feed
	if (!CHECKFLAG(block_data.motion_mode, MOTIONCONTROL_MODE_INVERSEFEED))
	{
		block_data.feed = MIN(block_data.feed, new_state->feedrate);
	}
	else
	{
		block_data.feed = new_state->feedrate;
	}

	// if non-modal is executed
	uint8_t index = 255;
	error = 0;
	switch (new_state->groups.nonmodal)
	{
	case G10: // G10
		index = ((uint8_t)words->p) ? words->p : (parser_parameters.coord_system_index + 1);
		switch (index)
		{
		case 28:
			index = G28HOME;
			break;
		case 30:

			index = G30HOME;
			break;
		default:
			index--;
			break;
		}
		break;
	case G92_1: // G92.1
		memset(g92permanentoffset, 0, sizeof(g92permanentoffset));
		// continue
	case G92_2: // G92.2
		memset(parser_parameters.g92_offset, 0, sizeof(parser_parameters.g92_offset));
		parser_wco_counter = 0;
		new_state->groups.nonmodal = 0; // this command is compatible with motion commands
		break;
	case G92_3: // G92.3
		memcpy(parser_parameters.g92_offset, g92permanentoffset, sizeof(g92permanentoffset));
		parser_wco_counter = 0;
		new_state->groups.nonmodal = 0; // this command is compatible with motion commands
		break;
	}

	// check from were to read the previous values for the target array
	if (index == 255)
	{
		switch (new_state->groups.nonmodal)
		{
		case G53:
			// G28 and G30 make the planed motion (absolute or relative)
			//         case G28:
			//         case G30:
			break;
		default:
			if ((new_state->groups.distance_mode == G90))
			{
				for (uint8_t i = AXIS_COUNT; i != 0;)
				{
					i--;
					if (CHECKFLAG(cmd->words, (1 << i)))
					{
						target[i] += parser_parameters.coord_system_offset[i] + parser_parameters.g92_offset[i];
					}
				}
#ifdef AXIS_TOOL
				if (CHECKFLAG(cmd->words, (1 << AXIS_TOOL)))
				{
					target[AXIS_TOOL] += parser_parameters.tool_length_offset;
				}
#endif
			}
			break;
		}
	}

	// stores G10 L2 command in the right address
	if (index <= G30HOME)
	{
		float coords[AXIS_COUNT];
		if (index == parser_parameters.coord_system_index)
		{
			memcpy(coords, parser_parameters.coord_system_offset, sizeof(coords));
		}
		else
		{
			settings_load(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (index * PARSER_PARAM_ADDR_OFFSET), (uint8_t *)&coords, PARSER_PARAM_SIZE);
		}

		for (uint8_t i = AXIS_COUNT; i != 0;)
		{
			i--;
			if (CHECKFLAG(cmd->words, (1 << i)))
			{
				if (words->l == 20)
				{
					coords[i] = -(target[i] - parser_last_pos[i] - parser_parameters.g92_offset[i]);
				}
				else
				{
					coords[i] = target[i];
				}
			}
		}
#ifdef AXIS_TOOL
		if (words->l == 20)
		{
			if (CHECKFLAG(cmd->words, (1 << AXIS_TOOL)))
			{
				coords[AXIS_TOOL] += parser_parameters.tool_length_offset;
			}
		}
#endif
		settings_save(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (index * PARSER_PARAM_ADDR_OFFSET), (uint8_t *)coords, PARSER_PARAM_SIZE);
		if (index == parser_parameters.coord_system_index)
		{
			memcpy(parser_parameters.coord_system_offset, coords, sizeof(parser_parameters.coord_system_offset));
		}
		parser_wco_counter = 0;
	}

	// laser disabled in nonmodal moves
	if (g_settings.laser_mode && new_state->groups.nonmodal)
	{
		block_data.spindle = 0;
	}

	switch (new_state->groups.nonmodal)
	{
	case G28: // G28
	case G30: // G30
		block_data.feed = FLT_MAX;
		if (CHECKFLAG(cmd->words, GCODE_ALL_AXIS))
		{
			error = mc_line(target, &block_data);
			update_tools = false;
			if (error)
			{
				return error;
			}
		}

		if (new_state->groups.nonmodal == G28)
		{
			settings_load(G28ADDRESS, (uint8_t *)&target, PARSER_PARAM_SIZE);
		}
		else
		{
			settings_load(G30ADDRESS, (uint8_t *)&target, PARSER_PARAM_SIZE);
		}
		error = mc_line((float *)&target, &block_data);
		// saves position
		memcpy(parser_last_pos, target, sizeof(parser_last_pos));
		break;
	case G92: // G92
		for (uint8_t i = AXIS_COUNT; i != 0;)
		{
			i--;
			parser_parameters.g92_offset[i] = -(target[i] - parser_last_pos[i] - parser_parameters.g92_offset[i]);
		}
		memcpy(g92permanentoffset, parser_parameters.g92_offset, sizeof(g92permanentoffset));
#ifdef G92_STORE_NONVOLATILE
		settings_save(G92ADDRESS, (uint8_t *)&parser_parameters.g92_offset[0], PARSER_PARAM_SIZE);
#endif
		parser_wco_counter = 0;
		break;
	case G53:
		new_state->groups.nonmodal = 0; // this command is compatible with motion commands
		break;
	}

	float x, y;
	// 20. perform motion (G0 to G3, G80 to G89), as modified (possibly) by G53.
	// G80 does no motion
	// G81 to G89 is executed in a separate function and uses G53,G0,G1 and G4 has building blocks
	// only if any target word was used
	if (new_state->groups.nonmodal == 0 && CHECKFLAG(cmd->words, GCODE_ALL_AXIS))
	{
		uint8_t probe_flags;
		switch (new_state->groups.motion)
		{
		case G0:
			// rapid move
			block_data.feed = FLT_MAX;
			// continues to send G1 at maximum feed rate
			// laser disabled in G0
			if (g_settings.laser_mode)
			{
				block_data.spindle = 0;
			}
		case G1:
			if (block_data.feed == 0)
			{
				return STATUS_FEED_NOT_SET;
			}
			error = mc_line(target, &block_data);
			break;
		case G2:
		case G3:
			if (block_data.feed == 0)
			{
				return STATUS_FEED_NOT_SET;
			}

			// target points
			x = target[a] - parser_last_pos[a];
			y = target[b] - parser_last_pos[b];
			float center_offset_a = words->ijk[offset_a];
			float center_offset_b = words->ijk[offset_b];
			// radius mode
			if (CHECKFLAG(cmd->words, GCODE_WORD_R))
			{
				if (x == 0 && y == 0)
				{
					return STATUS_GCODE_INVALID_TARGET;
				}

				float x_sqr = x * x;
				float y_sqr = y * y;
				float c_factor = words->r * words->r;
				c_factor = fast_flt_mul4(c_factor) - x_sqr - y_sqr;

				if (c_factor < 0)
				{
					return STATUS_GCODE_ARC_RADIUS_ERROR;
				}

				c_factor = -sqrt((c_factor) / (x_sqr + y_sqr));

				if (new_state->groups.motion == G3)
				{
					c_factor = -c_factor;
				}

				radius = words->r;
				if (words->r < 0)
				{
					c_factor = -c_factor;
					radius = -radius; // Finished with r. Set to positive for mc_arc
				}

				// Complete the operation by calculating the actual center of the arc
				center_offset_a = (x - (y * c_factor));
				center_offset_b = (y + (x * c_factor));
				center_offset_a = fast_flt_div2(center_offset_a);
				center_offset_b = fast_flt_div2(center_offset_b);
			}
			else // offset mode
			{
				// calculate radius and check if center is within tolerances
				radius = sqrtf(center_offset_a * center_offset_a + center_offset_b * center_offset_b);
				// calculates the distance between the center point and the target points and compares with the center offset distance
				float x1 = x - center_offset_a;
				float y1 = y - center_offset_b;
				float r1 = sqrt(x1 * x1 + y1 * y1);
				if (fabs(radius - r1) > 0.002) // error must not exceed 0.002mm according to the NIST RS274NGC standard
				{
					return STATUS_GCODE_INVALID_TARGET;
				}
			}

			error = mc_arc(target, center_offset_a, center_offset_b, radius, a, b, (new_state->groups.motion == 2), &block_data);
			break;
		case 4: // G38.2
		case 5: // G38.3
		case 6: // G38.4
		case 7: // G38.5
			probe_flags = (new_state->groups.motion > 5) ? 1 : 0;
			probe_flags |= (new_state->groups.motion & 0x01) ? 2 : 0;

			error = mc_probe(target, probe_flags, &block_data);
			if (error == STATUS_PROBE_SUCCESS)
			{
				parser_parameters.last_probe_ok = 1;
				error = STATUS_OK;
			}
			else
			{
				parser_parameters.last_probe_ok = 0;
			}

			if (error == STATUS_OK)
			{
				protocol_send_probe_result(parser_parameters.last_probe_ok);
			}

			return error;
		}

		// tool is updated in motion
		update_tools = false;
		// saves position
		memcpy(parser_last_pos, target, sizeof(parser_last_pos));
	}

	if (error)
	{
		return error;
	}

	// stop (M0, M1, M2, M30, M60) (not implemented yet).
	bool hold = false;
	bool resetparser = false;
	switch (new_state->groups.stopping)
	{
	case 1: // M0
	case 6: // M60 (pallet change has no effect)
		hold = true;
		break;
	case 2: // M1
#ifdef M1_CONDITION_ASSERT
		hold = M1_CONDITION_ASSERT;
#endif
		break;
	case 3: // M2
	case 4: // M30 (pallet change has no effect)
		hold = true;
		resetparser = true;
		break;
	}

	if (hold && !mc_get_checkmode())
	{
		mc_pause();
		if (resetparser)
		{
			cnc_stop();
			protocol_send_feedback(MSG_FEEDBACK_8);
		}
	}

	// if reached here the execution was not intersected
	// send a spindle and coolant update if needed
	if (update_tools)
	{
		return mc_update_tools(&block_data);
	}

	return STATUS_OK;
}

/*
	Parse the next gcode line available in the buffer and send it to the motion controller
*/
static uint8_t parser_gcode_command(void)
{
	uint8_t result = 0;
	// initializes new state
	parser_state_t next_state = {0};
	parser_words_t words = {0};
	parser_cmd_explicit_t cmd = {0};
	// next state will be the same as previous except for nonmodal group (is set with 0)
	memcpy(&next_state, &parser_state, sizeof(parser_state_t));
	next_state.groups.nonmodal = 0; // reset nonmodal

	// fetch command
	result = parser_fetch_command(&next_state, &words, &cmd);
	if (result != STATUS_OK)
	{
		return result;
	}

	// validates command
	result = parser_validate_command(&next_state, &words, &cmd);
	if (result != STATUS_OK)
	{
		return result;
	}

// executes command
#ifdef ENABLE_CANNED_CYCLES
	result = parser_exec_command_block(&next_state, &words, &cmd);
#else
	result = parser_exec_command(&next_state, &words, &cmd);
#endif
	if (result != STATUS_OK)
	{
		return result;
	}

	// if is jog motion state is not preserved
	if (!cnc_get_exec_state(EXEC_JOG))
	{
		// if everything went ok updates the parser modal groups and position
		memcpy(&parser_state, &next_state, sizeof(parser_state_t));
	}

	return result;
}

/*
	Parses a string to number (real)
	If the number is an integer the isinteger flag is set
	The string pointer is also advanced to the next position
*/
uint8_t parser_get_float(float *value)
{
	uint32_t intval = 0;
	uint8_t fpcount = 0;
	uint8_t result = NUMBER_UNDEF;

	unsigned char c = parser_get_next_preprocessed(true);

	*value = 0;

	if (c == '-' || c == '+')
	{
		if (c == '-')
		{
			result |= NUMBER_ISNEGATIVE;
		}
#ifdef ECHO_CMD
		serial_putc(serial_getc());
#else
		serial_getc();

#endif
		c = parser_get_next_preprocessed(true);
	}

	for (;;)
	{
		uint8_t digit = (uint8_t)c - 48;
		if (digit <= 9)
		{
			intval = fast_int_mul10(intval) + digit;
			if (fpcount)
			{
				fpcount++;
			}

			result |= NUMBER_OK;
		}
		else if (c == '.' && !fpcount)
		{
			fpcount++;
			result |= NUMBER_ISFLOAT;
		}
		else if (c == ' ')
		{
			// ignore white chars in the middle of numbers
		}
		else
		{
			if (!(result & NUMBER_OK))
			{
				return NUMBER_UNDEF;
			}
			break;
		}

#ifdef ECHO_CMD
		serial_putc(serial_getc());
#else
		serial_getc();

#endif
		c = parser_get_next_preprocessed(true);
	}

	*value = (float)intval;
	if (fpcount)
	{
		fpcount--;
	}

	do
	{
		if (fpcount >= 2)
		{
			*value *= 0.01f;
			fpcount -= 2;
		}

		if (fpcount >= 1)
		{
			*value *= 0.1f;
			fpcount -= 1;
		}

	} while (fpcount != 0);

	if (result & NUMBER_ISNEGATIVE)
	{
		*value = -*value;
	}

	return result;
}

/*
	Parses comments almost as defined in the RS274NGC
	To be compatible with Grbl it accepts bad format comments
	On error returns false otherwise returns true
*/
#define COMMENT_OK 1
#define COMMENT_NOTOK 2
static void parser_get_comment(unsigned char start_char)
{
	uint8_t comment_end = 0;
#ifdef PROCESS_COMMENTS
	uint8_t msg_parser = 0;
#endif
	for (;;)
	{
		unsigned char c = serial_peek();
		switch (c)
		{
			// case '(':	//error under RS274NGC (commented for Grbl compatibility)
		case ')': // OK
			if (start_char == '(')
			{
				comment_end = COMMENT_OK;
			}
			break;
		case EOL: // error under RS274NGC is starts with '(' (it's ignored)
			comment_end = COMMENT_OK;
			break;
		case OVF:
			// return ((c==')') ? true : false); //under RS274NGC (commented for Grbl compatibility)
			return;
		}

#ifdef PROCESS_COMMENTS
		switch (msg_parser)
		{
		case 0:
			msg_parser = (c == 'M' | c == 'm') ? 1 : 0xFF;
			break;
		case 1:
			msg_parser = (c == 'S' | c == 's') ? 2 : 0xFF;
			break;
		case 2:
			msg_parser = (c == 'G' | c == 'g') ? 3 : 0xFF;
			break;
		case 3:
			msg_parser = (c == ',') ? 4 : 0xFF;
			protocol_send_string(MSG_START);
			break;
		case 4:
			serial_putc(c);
			break;
		}
#endif

		if (c != EOL)
		{
			serial_getc();
		}

		if (comment_end)
		{
#ifdef PROCESS_COMMENTS
			if (msg_parser == 4)
			{
				protocol_send_string(MSG_END);
			}
#endif
			return;
		}
	}
}

static uint8_t parser_get_token(unsigned char *word, float *value)
{
	unsigned char c = serial_getc();

	// if other char starts tokenization
	if (c >= 'a' && c <= 'z')
	{
		c -= 32; // uppercase
	}

	*word = c;
	switch (c)
	{
	case EOL: // EOL
		return STATUS_OK;
	case OVF:
		return STATUS_OVERFLOW;
	default:
#ifdef ECHO_CMD
		serial_putc(c);
#endif
		if (c >= 'A' && c <= 'Z') // invalid recognized char
		{
			if (!parser_get_float(value))
			{
				return STATUS_BAD_NUMBER_FORMAT;
			}

			return STATUS_OK;
		}
// event_parse_token_handler
#ifdef ENABLE_PARSER_MODULES
		if (EVENT_INVOKE(parse_token, word))
		{
			return STATUS_OK;
		}
#endif
		return STATUS_EXPECTED_COMMAND_LETTER;
	}

	return STATUS_OK;
}

static uint8_t parser_gcode_word(uint8_t code, uint8_t mantissa, parser_state_t *new_state, parser_cmd_explicit_t *cmd)
{
	uint16_t new_group = cmd->groups;

	if (mantissa)
	{
		switch (code)
		{
			// codes with possible mantissa
		case 38:
		case 43:
		case 59:
		case 61:
		case 92:
			break;
		default:
			return STATUS_GCODE_COMMAND_VALUE_NOT_INTEGER;
		}
	}
	else
	{
		mantissa = 255; // if code should not have mantissa set it to a undefined value
	}

	switch (code)
	{
		// motion codes
	case 38: // check if 38.x
	case 0:
	case 1:
	case 2:
	case 3:
	case 80:
#ifdef ENABLE_CANNED_CYCLES
	case 81:
	case 82:
	case 83:
	case 84:
	case 85:
	case 86:
	case 87:
	case 88:
	case 89:
#endif
		switch (mantissa)
		{
		case 255:
			break;
		case 20:
			code = 4;
			break;
		case 30:
			code = 5;
			break;
		case 40:
			code = 6;
			break;
		case 50:
			code = 7;
			break;
		default:
			return STATUS_GCODE_UNSUPPORTED_COMMAND;
		}
		if (code >= 80)
		{
			code -= 72;
		}
		else if (cmd->group_0_1_useaxis)
		{
			return STATUS_GCODE_MODAL_GROUP_VIOLATION;
		}
		else
		{
			cmd->group_0_1_useaxis = 1;
		}

		new_group |= GCODE_GROUP_MOTION;
		new_state->groups.motion = code;
		break;
	case 17:
	case 18:
	case 19:
		new_group |= GCODE_GROUP_PLANE;
		code -= 17;
		new_state->groups.plane = code;
		break;
	case 90:
	case 91:
		new_group |= GCODE_GROUP_DISTANCE;
		new_state->groups.distance_mode = code - 90;
		break;
	case 93:
	case 94:
		new_group |= GCODE_GROUP_FEEDRATE;
		new_state->groups.feedrate_mode = code - 93;
		break;
	case 20:
	case 21:
		new_group |= GCODE_GROUP_UNITS;
		new_state->groups.units = code - 20;
		break;
	case 40:
	case 41:
	case 42:
		new_group |= GCODE_GROUP_CUTTERRAD;
		new_state->groups.cutter_radius_compensation = code - 40;
		break;
	case 43: // doesn't support G43 but G43.1 (takes Z coordinate input has offset)
		switch (mantissa)
		{
		case 255:
		case 10:
			// G43.1 same as G43
			break;
		default:
			return STATUS_GCODE_UNSUPPORTED_COMMAND;
		}
	case 49:
		new_state->groups.tlo_mode = ((code == 49) ? G49 : G43);
		new_group |= GCODE_GROUP_TOOLLENGTH;
		break;
	case 98:
	case 99:
		new_group |= GCODE_GROUP_RETURNMODE;
		new_state->groups.return_mode = code - 98;
		break;
	case 54:
	case 55:
	case 56:
	case 57:
	case 58:
	case 59:
		new_group |= GCODE_GROUP_COORDSYS;
		code -= 54;
		switch (mantissa)
		{
		case 255:
			break;
		case 10:
			code += 1;
			break;
		case 20:
			code += 2;
			break;
		case 30:
			code += 3;
			break;
		default:
			return STATUS_GCODE_UNSUPPORTED_COMMAND;
		}

		if (code > COORD_SYS_COUNT)
		{
			return STATUS_GCODE_UNSUPPORTED_COORD_SYS;
		}
		new_state->groups.coord_system = code;
		break;
	case 61:
	case 64:
		code -= 61;
		switch (mantissa)
		{
		case 255:
			break;
		case 10:
			code += 1;
			break;
		default:
			return STATUS_GCODE_UNSUPPORTED_COMMAND;
		}
		new_group |= GCODE_GROUP_PATH;
		new_state->groups.path_mode = code;
		break;
	// de following nonmodal colide with motion groupcodes
	case 10:
	case 28:
	case 30:
	case 92:
		if (cmd->group_0_1_useaxis)
		{
			return STATUS_GCODE_MODAL_GROUP_VIOLATION;
		}
		cmd->group_0_1_useaxis = 1;
	case 4:
	case 53:

		// convert code within 4 bits without
		// 4 = 1
		// 10 = 2
		// 28 = 3
		// 30 = 4
		// 53 = 6
		// 92 = 10
		// 92.1 = 11
		// 92.2 = 12
		// 92.3 = 13
		code = (uint8_t)floorf(code * 0.10001f);
		switch (mantissa)
		{
		case 255:
			break;
		case 30:
			code++;
		case 20:
			code++;
		case 10:
			code++;
			break;
		default:
			return STATUS_GCODE_UNSUPPORTED_COMMAND;
		}

		new_group |= GCODE_GROUP_NONMODAL;
		new_state->groups.nonmodal = code + 1;
		break;
	default:
		return STATUS_GCODE_UNSUPPORTED_COMMAND;
	}

	if (new_group == cmd->groups)
	{
		return STATUS_GCODE_MODAL_GROUP_VIOLATION;
	}

	cmd->groups = new_group;
	return STATUS_OK;
}

static uint8_t parser_mcode_word(uint8_t code, uint8_t mantissa, parser_state_t *new_state, parser_cmd_explicit_t *cmd)
{
	uint16_t new_group = cmd->groups;

	if (mantissa)
	{
		return STATUS_GCODE_UNSUPPORTED_COMMAND;
	}

	switch (code)
	{
	case 60:
		code = 5;
	case 30:
		code = (code & 1) ? 5 : 3;
	case 0:
	case 1:
	case 2:
		new_group |= GCODE_GROUP_STOPPING;
		new_state->groups.stopping = code + 1;
		break;
#if TOOL_COUNT > 0
	case 3:
	case 4:
	case 5:
		new_group |= GCODE_GROUP_SPINDLE;
		code = (code == 5) ? M5 : code - 2;
		new_state->groups.spindle_turning = code;
		break;
	case 6:
		new_group |= GCODE_GROUP_TOOLCHANGE;
		break;
#if !(COOLANT_MIST < 0)
	case 7:
#endif
#ifdef M7_SAME_AS_M8
	case 7:
#endif
	case 8:
		cmd->groups |= GCODE_GROUP_COOLANT; // word overlapping allowed
#if !(COOLANT_MIST < 0)
		new_state->groups.coolant |= ((code == 8) ? M8 : M7);
#else
		new_state->groups.coolant |= M8;
#endif
		return STATUS_OK;
	case 9:
		cmd->groups |= GCODE_GROUP_COOLANT;
		new_state->groups.coolant = M9;
		return STATUS_OK;
#endif
	case 48:
	case 49:
		new_group |= GCODE_GROUP_ENABLEOVER;
		new_state->groups.feed_speed_override = (code == 48) ? M48 : M49;
		break;
#if SERVOS_MASK != 0
	case 10:
		if (cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			return STATUS_GCODE_MODAL_GROUP_VIOLATION;
		}
		// tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
		cmd->group_extended = M10;
		return STATUS_OK;
#endif
	default:
		return STATUS_GCODE_UNSUPPORTED_COMMAND;
	}

	if ((new_group & ~GCODE_GROUP_COOLANT) == (cmd->groups & ~GCODE_GROUP_COOLANT))
	{
		return STATUS_GCODE_MODAL_GROUP_VIOLATION;
	}

	cmd->groups = new_group;
	return STATUS_OK;
}

static uint8_t parser_letter_word(unsigned char c, float value, uint8_t mantissa, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
	uint16_t new_words = cmd->words;
	switch (c)
	{
	case 'N':
#ifdef GCODE_PROCESS_LINE_NUMBERS
#ifndef GCODE_COUNT_TEXT_LINES
		// if enabled store line number
		words->n = value;
#endif
#endif
		break;
#ifdef AXIS_X
	case 'X':
		new_words |= GCODE_WORD_X;
		words->xyzabc[AXIS_X] = value;
		break;
#endif
#ifdef AXIS_Y
	case 'Y':
		new_words |= GCODE_WORD_Y;
		words->xyzabc[AXIS_Y] = value;
		break;
#endif
#ifdef AXIS_Z
	case 'Z':
		new_words |= GCODE_WORD_Z;
		words->xyzabc[AXIS_Z] = value;
		break;
#endif
#ifdef AXIS_A
	case 'A':
#ifdef GCODE_ACCEPT_WORD_E
	case 'E':
#endif
		new_words |= GCODE_WORD_A;
		words->xyzabc[AXIS_A] = value;
		break;
#endif
#ifdef AXIS_B
	case 'B':
		new_words |= GCODE_WORD_B;
		words->xyzabc[AXIS_B] = value;
		break;
#endif
#ifdef AXIS_C
	case 'C':
		new_words |= GCODE_WORD_C;
		words->xyzabc[AXIS_C] = value;
		break;
#endif
	// treats Q like D since they cannot cooexist
	case 'Q':
		if (value < 0)
		{
			return STATUS_NEGATIVE_VALUE;
		}
	case 'D':
		new_words |= GCODE_WORD_D;
		words->d = value;
		break;
	case 'F':
		new_words |= GCODE_WORD_F;
		words->f = value;
		break;
#ifdef AXIS_X
	case 'I':
		new_words |= GCODE_WORD_I;
		words->ijk[AXIS_X] = value;
		break;
#endif
#ifdef AXIS_Y
	case 'J':
		new_words |= GCODE_WORD_J;
		words->ijk[AXIS_Y] = value;
		break;
#endif
#ifdef AXIS_Z
	case 'K':
		new_words |= GCODE_WORD_K;
		words->ijk[AXIS_Z] = value;
		break;
#endif
	case 'L':
		new_words |= GCODE_WORD_L;

		if (mantissa)
		{
			return STATUS_GCODE_COMMAND_VALUE_NOT_INTEGER;
		}

		words->l = (uint8_t)truncf(value);
		break;
	case 'P':
		new_words |= GCODE_WORD_P;

		if (value < 0)
		{
			return STATUS_NEGATIVE_VALUE;
		}

		words->p = value;
		break;
	case 'R':
		new_words |= GCODE_WORD_R;
		words->r = value;
		break;
#if TOOL_COUNT > 0
	case 'S':
		new_words |= GCODE_WORD_S;
		if (value < 0)
		{
			return STATUS_NEGATIVE_VALUE;
		}
		words->s = value;
		break;
	case 'T':
		new_words |= GCODE_WORD_T;
		if (mantissa)
		{
			return STATUS_GCODE_COMMAND_VALUE_NOT_INTEGER;
		}

		if (value < 0)
		{
			return STATUS_NEGATIVE_VALUE;
		}

		words->t = (uint8_t)trunc(value);
		break;
	case 'H':
		// special case for G43
		// is valid if preceded from a G43/G49
		// it get's converted to a Z word with tool length
		if (CHECKFLAG(cmd->groups, GCODE_GROUP_TOOLLENGTH))
		{
			if (mantissa)
			{
				return STATUS_GCODE_COMMAND_VALUE_NOT_INTEGER;
			}

			uint8_t index = (uint8_t)trunc(value);
			if (index < 1 || index > TOOL_COUNT)
			{
				return STATUS_INVALID_TOOL;
			}
			index--;
			new_words |= GCODE_WORD_Z;
			words->xyzabc[AXIS_Z] = g_settings.tool_length_offset[index];
		}
#else
	case 'S':
		// ignores
		break;
	case 'T':
	case 'H':
		return STATUS_GCODE_UNUSED_WORDS;
#endif
		break;
	default:
		if (c >= 'A' && c <= 'Z') // invalid recognized char
		{
#ifdef IGNORE_UNDEFINED_AXIS
			if (c <= 'C' || c >= 'X') // ignore undefined axis chars
			{
				return STATUS_OK;
			}
#endif
			return STATUS_GCODE_UNUSED_WORDS;
		}
		return STATUS_INVALID_STATEMENT;
	}

	if (new_words == cmd->words)
	{
		return STATUS_GCODE_WORD_REPEATED;
	}

	cmd->words = new_words;

	return STATUS_OK;
}

static unsigned char parser_get_next_preprocessed(bool peek)
{
	unsigned char c = serial_peek();

	while (c == ' ' || c == '(' || c == ';')
	{
		serial_getc();
		if (c != ' ')
		{
			parser_get_comment(c);
		}
		c = serial_peek();
	}

	if (!peek)
	{
		serial_getc();
	}

	return c;
}

static void parser_discard_command(void)
{
	unsigned char c = '@';
#ifdef ECHO_CMD
	serial_putc(c);
#endif
	do
	{
		c = serial_getc();
#ifdef ECHO_CMD
		serial_putc(c);
#endif
	} while (c != EOL);
}

void parser_reset(void)
{
	parser_state.groups.coord_system = G54;				  // G54
	parser_state.groups.plane = G17;					  // G17
	parser_state.groups.feed_speed_override = M48;		  // M48
	parser_state.groups.cutter_radius_compensation = G40; // G40
	parser_state.groups.distance_mode = G90;			  // G90
	parser_state.groups.feedrate_mode = G94;			  // G94
	parser_state.groups.tlo_mode = G49;					  // G49
	parser_state.groups.stopping = 0;					  // resets all stopping commands (M0,M1,M2,M30,M60)
#if TOOL_COUNT > 0
	parser_state.groups.coolant = M9;		  // M9
	parser_state.groups.spindle_turning = M5; // M5
	parser_state.groups.tool_change = 1;
	parser_state.tool_index = g_settings.default_tool;
#endif
	parser_state.groups.motion = G1;											   // G1
	parser_state.groups.units = G21;											   // G21
	memset(parser_parameters.g92_offset, 0, sizeof(parser_parameters.g92_offset)); // G92.2
	parser_parameters.tool_length_offset = 0;
	parser_wco_counter = 0;
}

// loads parameters
// loads G92 offset
// loads G54 coordinate system
// also checks all other coordinate systems and homing positions
void parser_parameters_load(void)
{
// loads G92
#ifdef G92_STORE_NONVOLATILE
	if (settings_load(G92ADDRESS, (uint8_t *)&parser_parameters.g92_offset, PARSER_PARAM_SIZE))
	{
		memset(parser_parameters.g92_offset, 0, sizeof(parser_parameters.g92_offset));
		settings_erase(G92ADDRESS, PARSER_PARAM_SIZE);
	}
	memcpy(g92permanentoffset, parser_parameters.g92_offset, sizeof(g92permanentoffset));
#else
	memset(parser_parameters.g92_offset, 0, sizeof(parser_parameters.g92_offset));
	memset(g92permanentoffset, 0, sizeof(g92permanentoffset));
#endif

	for (uint8_t i = 1; i < G92OFFSET; i++)
	{
		if (settings_load(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (i * PARSER_PARAM_ADDR_OFFSET), (uint8_t *)&parser_parameters.coord_system_offset, PARSER_PARAM_SIZE))
		{
			settings_erase(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (i * PARSER_PARAM_ADDR_OFFSET), PARSER_PARAM_SIZE);
		}
	}

	// load G54
	if (settings_load(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET, (uint8_t *)&parser_parameters.coord_system_offset, PARSER_PARAM_SIZE))
	{
		memset(parser_parameters.coord_system_offset, 0, sizeof(parser_parameters.coord_system_offset));
		settings_erase(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET, PARSER_PARAM_SIZE);
	}
}

void parser_sync_position(void)
{
	mc_get_position(parser_last_pos);
}

#ifdef ENABLE_CANNED_CYCLES

static uint8_t sticky_mask;
static float sticky_r;
static float sticky_new;
static float sticky_old;

uint8_t parser_exec_command_block(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
	// not a canned cycle (run single command)
	if (new_state->groups.nonmodal != 0 || !CHECKFLAG(cmd->words, GCODE_ALL_AXIS) || new_state->groups.motion <= 8)
	{
		sticky_mask = 0;
		return parser_exec_command(new_state, words, cmd);
	}

	if (new_state->groups.feedrate_mode == G93)
	{
		return STATUS_GCODE_CANNED_CYCLE_INVALID_FEEDMODE;
	}

	if (new_state->groups.cutter_radius_compensation != G40)
	{
		return STATUS_GCODE_CANNED_CYCLE_INVALID_RADIUSCOMPMODE;
	}

	if (new_state->feedrate == 0 && !CHECKFLAG(cmd->words, GCODE_WORD_F))
	{
		return STATUS_FEED_NOT_SET;
	}

	if (new_state->groups.motion == G86 && (new_state->spindle == 0 || new_state->groups.spindle_turning == M5))
	{
		return STATUS_SPINDLE_STOPPED;
	}

	// store previous clearence
	uint8_t plane_axis = AXIS_Z;
	uint8_t plane_mask = GCODE_XYPLANE_AXIS;
	switch (new_state->groups.plane)
	{
	case G18:
		plane_axis = AXIS_Y;
		plane_mask = GCODE_XZPLANE_AXIS;
		break;
	case G19:
		plane_axis = AXIS_X;
		plane_mask = GCODE_YZPLANE_AXIS;
		break;
	}

	uint8_t mask = sticky_mask;
	// load previous retract distance
	float r = sticky_r;
	// load previous 'Z' position (may not be Z. depends on active plane)
	float old_z = sticky_old;
	float new_z = sticky_new;

	// check if all necessary arguments are present (first call)
	if (!mask)
	{
		old_z = parser_last_pos[plane_axis];
		mask = (uint8_t)(0x07 & cmd->words);
	}

	// adds explicit arguments
	if (CHECKFLAG(cmd->words, GCODE_WORD_R))
	{
		mask |= 0x8;
		r = words->r;

		// converts retract position to machine coordinates
		if ((new_state->groups.nonmodal != G53) && (new_state->groups.distance_mode == G90))
		{
			if ((new_state->groups.distance_mode == G90))
			{
				r += parser_parameters.coord_system_offset[plane_axis] + parser_parameters.g92_offset[plane_axis];
#ifdef AXIS_TOOL
				if (CHECKFLAG(plane_axis, (1 << AXIS_TOOL)))
				{
					r += parser_parameters.tool_length_offset;
				}
#endif
			}
		}
		else if ((new_state->groups.distance_mode == G91))
		{
			r += old_z;
		}
	}

	// set new sticky 'Z' or retrieves it
	if (CHECKFLAG(cmd->words, (1U << plane_axis)))
	{
		new_z = words->xyzabc[plane_axis];
		// converts retract position to machine coordinates
		if ((new_state->groups.nonmodal != G53) && (new_state->groups.distance_mode == G90))
		{
			if ((new_state->groups.distance_mode == G90))
			{
				new_z += parser_parameters.coord_system_offset[plane_axis] + parser_parameters.g92_offset[plane_axis];
#ifdef AXIS_TOOL
				if (CHECKFLAG(plane_axis, (1 << AXIS_TOOL)))
				{
					new_z += parser_parameters.tool_length_offset;
				}
#endif
			}
		}
		else if ((new_state->groups.distance_mode == G91))
		{
			new_z += old_z;
		}
	}

	// stores arguments back
	sticky_mask = mask;
	sticky_r = r;
	sticky_old = old_z;
	sticky_new = new_z;

	// Do preliminary motion for canned cycles
	parser_state_t canned_state = {0};
	parser_words_t canned_words = {0};
	parser_cmd_explicit_t canned_cmd = {0};

	memcpy(&canned_state, new_state, sizeof(parser_state_t));
	memcpy(&canned_words, words, sizeof(parser_words_t));
	memcpy(&canned_cmd, cmd, sizeof(parser_cmd_explicit_t));

	// calculate retract position in absolute coordinates
	float new_r = r;

	// preliminary motion starts by checking the programmed distance in the working axis and the clearence
	// if distance in inferior to clearence retract to safe distance
	float clearance = new_r;
	// with absolute mode active
	if (!new_state->groups.distance_mode)
	{
		// relative lift to safe position
		clearance -= old_z;
	}
	else // with relative mode active
	{
		clearance = r;
	}

	if (new_state->groups.return_mode == G98)
	{
		r = MAX(r, old_z);
	}

	uint8_t error = STATUS_OK;
	// retract if needed
	canned_state.groups.motion = G0;

	if (clearance > 0)
	{
		// force machine coordinates
		canned_state.groups.nonmodal = G53;
		canned_words.xyzabc[plane_axis] = r;
		// mask axis motion
		canned_cmd.words &= ~GCODE_ALL_AXIS;
		canned_cmd.words |= (1U << plane_axis);
		error = parser_exec_command(&canned_state, &canned_words, &canned_cmd);
		if (error)
		{
			return error;
		}

		// reset values
		canned_words.xyzabc[plane_axis] = words->xyzabc[plane_axis];
		// restores nonmodal
		canned_state.groups.nonmodal = new_state->groups.nonmodal;
		canned_cmd.words = cmd->words;
	}

	// transverse motion parallel to plane
	if (CHECKFLAG(cmd->words, plane_mask))
	{
		canned_cmd.words &= ~GCODE_ALL_AXIS;
		canned_cmd.words |= plane_mask;
		error = parser_exec_command(&canned_state, &canned_words, &canned_cmd);
		if (error)
		{
			return error;
		}
		canned_cmd.words = cmd->words;
	}

	// transverse motion to R distance
	if (clearance < 0)
	{
		// force machine coordinates
		canned_state.groups.nonmodal = G53;
		canned_words.xyzabc[plane_axis] = r;
		// mask axis motion
		canned_cmd.words &= ~GCODE_ALL_AXIS;
		canned_cmd.words |= (1U << plane_axis);
		error = parser_exec_command(&canned_state, &canned_words, &canned_cmd);
		if (error)
		{
			return error;
		}

		// reset values
		canned_words.xyzabc[plane_axis] = words->xyzabc[plane_axis];
		// restores nonmodal
		canned_state.groups.nonmodal = new_state->groups.nonmodal;
		canned_cmd.words = cmd->words;
	}

	float loops = 1;

	if (CHECKFLAG(cmd->words, GCODE_WORD_L))
	{
		loops = words->l;
	}

	// do the canned cycle motion
	for (uint8_t l = 0; l < loops; l++)
	{
		float current_z = r;
		switch (new_state->groups.motion)
		{
		case G81:
		case G82:
		case G83:
		case G85:
		case G86:
		case G89:
			// drill
			do
			{
				canned_cmd.words &= ~GCODE_ALL_AXIS;
				canned_cmd.words |= (1U << plane_axis);
				if (new_state->groups.motion == G83)
				{
					canned_cmd.words &= ~GCODE_ALL_AXIS;
					canned_cmd.words |= (1U << plane_axis);
					canned_state.groups.motion = G0;
					canned_words.xyzabc[plane_axis] = MIN(current_z + words->d * 0.5f, r);
					error = parser_exec_command(&canned_state, &canned_words, &canned_cmd);
					if (error)
					{
						return error;
					}
					current_z -= words->d;
					current_z = MAX(current_z, new_z);
				}
				else
				{
					current_z = new_z;
				}
				canned_words.xyzabc[plane_axis] = current_z;
				canned_state.groups.motion = G1;
				canned_state.groups.nonmodal = new_state->groups.nonmodal;
				error = parser_exec_command(&canned_state, &canned_words, &canned_cmd);
				if (error)
				{
					return error;
				}

				// retract
				canned_words.xyzabc[plane_axis] = r;
				if (new_state->groups.motion == G82 || new_state->groups.motion == G89)
				{
					canned_state.groups.nonmodal = G4;
					canned_words.p = words->p;
				}

				if (new_state->groups.motion == G86)
				{
					// stops spindle
					canned_state.groups.motion = G80;
					canned_state.groups.spindle_turning = M5;
					// force spindle update
					canned_cmd.groups |= GCODE_GROUP_SPINDLE;
					error = parser_exec_command(&canned_state, &canned_words, &canned_cmd);
					if (error)
					{
						return error;
					}
					// restore
					canned_cmd.groups = cmd->groups;
				}

				// mask axis motion
				canned_cmd.words &= ~GCODE_ALL_AXIS;
				canned_cmd.words |= (1U << plane_axis);
				canned_state.groups.motion = G0;
				if (new_state->groups.motion == G85 || new_state->groups.motion == G89)
				{
					canned_state.groups.motion = G1;
				}
				error = parser_exec_command(&canned_state, &canned_words, &canned_cmd);
				if (error)
				{
					return error;
				}

				if (new_state->groups.motion == G86)
				{
					// stops spindle
					canned_state.groups.motion = G80;
					canned_state.groups.spindle_turning = new_state->groups.spindle_turning;
					// force spindle update
					canned_cmd.groups |= GCODE_GROUP_SPINDLE;
					error = parser_exec_command(&canned_state, &canned_words, &canned_cmd);
					if (error)
					{
						return error;
					}
					// restore
					canned_cmd.groups = cmd->groups;
				}
			} while (current_z > new_z);
			break;
		}
	}

	return error;
}
#endif

void parser_machine_to_work(float *axis)
{
	for (uint8_t i = 0; i < AXIS_COUNT; i++)
	{
		axis[i] -= (parser_parameters.g92_offset[i] + parser_parameters.coord_system_offset[i]);
	}

#ifdef AXIS_TOOL
	if (parser_state.groups.tlo_mode != G49)
	{
		axis[AXIS_TOOL] -= parser_parameters.tool_length_offset;
	}
#endif
}
