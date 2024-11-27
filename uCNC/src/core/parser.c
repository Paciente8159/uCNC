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

#include <stdint.h>
#include <math.h>
#include <string.h>
#include <float.h>

// extended codes
#define M10 EXTENDED_MCODE(10)

static parser_state_t parser_state;
static parser_parameters_t parser_parameters;
static uint8_t parser_wco_counter;
static float g92permanentoffset[AXIS_COUNT];
static int32_t rt_probe_step_pos[STEPPER_COUNT];
static float parser_last_pos[AXIS_COUNT];

#ifndef DISABLE_HOME_SUPPORT
#define ADDITIONAL_COORDINATES 2
#else
#define ADDITIONAL_COORDINATES 0
#endif

#ifdef DISABLE_COORD_SYS_SUPPORT
#define TOTAL_COORDINATE_SYSTEMS (1 + ADDITIONAL_COORDINATES)
#else
#define TOTAL_COORDINATE_SYSTEMS (COORD_SYS_COUNT + ADDITIONAL_COORDINATES)
#endif

#ifndef DISABLE_COORDINATES_SYSTEM_RAM
static float coordinate_systems[TOTAL_COORDINATE_SYSTEMS][AXIS_COUNT];
#endif

FORCEINLINE static void parser_get_comment(uint8_t start_char);
FORCEINLINE static uint8_t parser_get_token(uint8_t *word, float *value);
FORCEINLINE static uint8_t parser_gcode_word(uint8_t code, uint8_t mantissa, parser_state_t *new_state, parser_cmd_explicit_t *cmd);
FORCEINLINE static uint8_t parser_mcode_word(uint8_t code, uint8_t mantissa, parser_state_t *new_state, parser_cmd_explicit_t *cmd);
FORCEINLINE static uint8_t parser_letter_word(uint8_t c, float value, uint8_t mantissa, parser_words_t *words, parser_cmd_explicit_t *cmd);
static uint8_t parser_grbl_exec_code(uint8_t code);
static uint8_t parser_fetch_command(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
static uint8_t parser_validate_command(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
static uint8_t parser_grbl_command(void);
FORCEINLINE static uint8_t parser_gcode_command(bool is_jogging);
static void parser_coordinate_system_load(uint8_t param, float *target);

#ifdef ENABLE_RS274NGC_EXPRESSIONS
extern char parser_backtrack;
extern bool o_code_end_subrotine(void);
extern bool o_code_returned;
extern float o_code_return_value;
extern uint8_t o_code_stack_index;
#endif

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

// event_gcode_exec_modifier_handler
WEAK_EVENT_HANDLER(gcode_exec_modifier)
{
	DEFAULT_EVENT_HANDLER(gcode_exec_modifier);
}

// event_gcode_before_motion_handler
WEAK_EVENT_HANDLER(gcode_before_motion)
{
	DEFAULT_EVENT_HANDLER(gcode_before_motion);
}

// event_gcode_after_motion_handler
WEAK_EVENT_HANDLER(gcode_after_motion)
{
	DEFAULT_EVENT_HANDLER(gcode_after_motion);
}

// event_parse_token_handler
WEAK_EVENT_HANDLER(parse_token)
{
	DEFAULT_EVENT_HANDLER(parse_token);
}

// event_parser_get_modes_handler
WEAK_EVENT_HANDLER(parser_get_modes)
{
	DEFAULT_EVENT_HANDLER(parser_get_modes);
}

// event_parser_reset_handler
WEAK_EVENT_HANDLER(parser_reset)
{
	DEFAULT_EVENT_HANDLER(parser_reset);
}
#endif

#if (defined(ENABLE_PARSER_MODULES) || defined(BOARD_HAS_CUSTOM_SYSTEM_COMMANDS))
// event_grbl_cmd_handler
WEAK_EVENT_HANDLER(grbl_cmd)
{
	DEFAULT_EVENT_HANDLER(grbl_cmd);
}

int8_t parser_get_grbl_cmd_arg(char *arg, int8_t max_len)
{
	int8_t len = 0;
	for (;;)
	{
		uint8_t c = grbl_stream_getc();
		if (!c)
		{
			return len;
		}
		if (!max_len)
		{
			return -1;
		}

		*arg++ = c;
		len++;
		max_len--;
	}
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
	parser_reset(true);
}

uint8_t parser_read_command(void)
{
	uint8_t error = STATUS_OK;
	uint8_t c = grbl_stream_peek();

	if (c == '$')
	{
		error = parser_grbl_command();

		if (error >= GRBL_SYSTEM_CMD)
		{
			if (error != GRBL_JOG_CMD)
			{
				return parser_grbl_exec_code(error);
			}
		}
		else
		{
			return error;
		}
	}

	bool is_jogging = false;
	if (error == GRBL_JOG_CMD)
	{
		is_jogging = true;
	}
	else if (cnc_get_exec_state(~(EXEC_RUN | EXEC_HOLD)) || cnc_has_alarm()) // if any other than idle, run or hold discards the command
	{
		parser_discard_command();
		return STATUS_SYSTEM_GC_LOCK;
	}

	if (cnc_get_exec_state(EXEC_JOG) && !is_jogging) // error if trying to do a normal move with jog active
	{
		return STATUS_SYSTEM_GC_LOCK;
	}

	return parser_gcode_command(is_jogging);
}

void parser_get_modes(uint8_t *modalgroups, uint16_t *feed, uint16_t *spindle)
{
	modalgroups[0] = parser_state.groups.motion;
	modalgroups[12] = parser_state.groups.motion_mantissa;
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
#ifdef ENABLE_COOLANT
	modalgroups[9] = parser_state.groups.coolant;
#else
	modalgroups[9] = 0;
#endif
#if TOOL_COUNT > 1
	modalgroups[11] = parser_state.tool_index;
#else
	modalgroups[11] = 1;
#endif
#else
	modalgroups[8] = 5;
	modalgroups[9] = 9;
	modalgroups[11] = 0;
#endif
	modalgroups[10] = 49 - parser_state.groups.feed_speed_override;
#ifdef ENABLE_G39_H_MAPPING
	modalgroups[13] = parser_state.groups.height_map_active;
#endif
// event_parser_get_modes_handler
#ifdef ENABLE_PARSER_MODULES
	EVENT_INVOKE(parser_get_modes, modalgroups);
#endif
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
		*axis = parser_parameters.tool_length_offset;
		break;
	case 253:
		memcpy(axis, parser_last_pos, sizeof(parser_last_pos));
		break;
#ifndef DISABLE_HOME_SUPPORT
	case 28:
		parser_coordinate_system_load(G28HOME, axis);
		break;
	case 30:
		parser_coordinate_system_load(G30HOME, axis);
		break;
#endif
	case 92:
		memcpy(axis, parser_parameters.g92_offset, sizeof(parser_parameters.g92_offset));
		break;
	default:
		parser_coordinate_system_load(system_num, axis);
		break;
	}
}

uint8_t parser_get_probe_result(void)
{
	return parser_parameters.last_probe_ok;
}

void parser_parameters_reset(void)
{
	// erase all parameters for G54..G59.x coordinate systems and homing positions

#ifndef DISABLE_COORDINATES_SYSTEM_RAM
	memset(coordinate_systems, 0, sizeof(coordinate_systems));
#endif

	// erase all G5x workoffsets and home G28 and G30
	for (uint8_t i = 0; i < TOTAL_COORDINATE_SYSTEMS; i++)
	{
		settings_erase(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (i * PARSER_PARAM_ADDR_OFFSET), (uint8_t *)&parser_parameters.coord_system_offset, PARSER_PARAM_SIZE);
	}

// erase G92
#ifdef G92_STORE_NONVOLATILE
	settings_erase(G92ADDRESS, (uint8_t *)&g92permanentoffset, PARSER_PARAM_SIZE);
#else
	memset(g92permanentoffset, 0, sizeof(g92permanentoffset));
#endif
}

void parser_parameters_save(void)
{
	// saves all stored parameters (only possible if parameters also exist in RAM)
#ifndef DISABLE_COORDINATES_SYSTEM_RAM
	for (uint8_t i = 0; i < TOTAL_COORDINATE_SYSTEMS; i++)
	{
		settings_save(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (i * PARSER_PARAM_ADDR_OFFSET), (uint8_t *)&coordinate_systems[i], PARSER_PARAM_SIZE);
	}
#else
	// can only store the current loaded working system
	settings_save(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (parser_parameters.coord_system_index * PARSER_PARAM_ADDR_OFFSET), (uint8_t *)&parser_parameters.coord_system_offset, PARSER_PARAM_SIZE);
#endif

// save G92
#ifdef G92_STORE_NONVOLATILE
	settings_save(G92ADDRESS, (uint8_t *)&g92permanentoffset, PARSER_PARAM_SIZE);
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

void parser_get_probe(int32_t *position)
{
	memcpy(position, rt_probe_step_pos, sizeof(rt_probe_step_pos));
}

void parser_update_probe_pos(void)
{
	kinematics_steps_to_coordinates(rt_probe_step_pos, parser_parameters.last_probe_position);
}

static uint8_t parser_grbl_command(void)
{
	grbl_stream_getc(); // eat $
	uint8_t c = grbl_stream_peek();
	uint8_t grbl_cmd_str[GRBL_CMD_MAX_LEN + 1];
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
		case 'J':
			break;
		default:
			parser_discard_command();
			return STATUS_IDLE_ERROR;
		}
	}

	do
	{
		c = grbl_stream_peek();
		// toupper
		c = TOUPPER(c);

		if (!(c >= 'A' && c <= 'Z'))
		{
			if (c < '0' || c > '9' || grbl_cmd_len) // replaces old ungetc
			{
				grbl_stream_getc();
			}
			break;
		}

		grbl_stream_getc();
		grbl_cmd_str[grbl_cmd_len++] = c;
	} while ((grbl_cmd_len < GRBL_CMD_MAX_LEN));

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
			if (grbl_stream_getc() != EOL)
			{
				return STATUS_INVALID_STATEMENT;
			}
			return GRBL_SEND_SYSTEM_SETTINGS;
		case '#':
			c = grbl_stream_peek();
			switch (c)
			{
			case EOL:
				grbl_stream_getc();
				return GRBL_SEND_COORD_SYSTEM;
#ifdef ENABLE_RS274NGC_EXPRESSIONS
			default:
				if (c == '<' || (c >= '0' && c <= '9'))
				{
					return GRBL_PRINT_PARAM;
				}
#endif
			}
			return STATUS_INVALID_STATEMENT;
		case EOL:
			return GRBL_HELP;
		default:
			if (c >= '0' && c <= '9') // settings
			{
				float val = 0;
				setting_offset_t setting_num = 0;
				// grbl_stream_ungetc();
				error = parser_get_float(&val);
				if (!error)
				{
					return STATUS_INVALID_STATEMENT;
				}
#ifndef ENABLE_SETTINGS_MODULES
				if ((error & NUMBER_ISFLOAT) || val > 255 || val < 0)
				{
					return STATUS_INVALID_STATEMENT;
				}
#else
				if ((error & NUMBER_ISFLOAT) || val > 65535 || val < 0)
				{
					return STATUS_INVALID_STATEMENT;
				}
#endif

				setting_num = (setting_offset_t)val;
				// eat '='
				if (grbl_stream_getc() != '=')
				{
					return STATUS_INVALID_STATEMENT;
				}

				val = 0;
				if (!parser_get_float(&val))
				{
#ifdef ENABLE_SETTINGS_MODULES
					return settings_change(setting_num, val);
#endif
					return STATUS_BAD_NUMBER_FORMAT;
				}

				if (grbl_stream_getc() != EOL)
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
			case EOL:
				return GRBL_SEND_STARTUP_BLOCKS;
			default:
				if (c >= '0' && c <= '9')
				{
					block_address = STARTUP_BLOCK_ADDRESS_OFFSET((uint8_t)(c - '0'));
					if (grbl_stream_getc() != '=')
					{
						return STATUS_INVALID_STATEMENT;
					}

					settings_save(block_address, NULL, UINT16_MAX);
					// run startup block
					grbl_stream_start_broadcast();
					grbl_stream_eeprom(block_address);
					// checks the command validity
					error = parser_fetch_command(&next_state, &words, &cmd);
					// if uncomment will also check if any gcode rules are violated
					// allow bad rules for now to fit UNO. Will be catched when trying to execute the line
					// if (error == STATUS_OK)
					// {
					// 	error = parser_validate_command(&next_state, &words, &cmd);
					// }

					// reset streams
					grbl_stream_change(NULL);

					if (error != STATUS_OK)
					{
						parser_discard_command();
						// the Gcode is not valid then erase the startup block
						settings_erase(block_address, NULL, 1);
					}
				}

				return error;
			}
			return STATUS_INVALID_STATEMENT;
#ifdef ENABLE_PIN_DEBUG_EXTRA_CMD
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
			if (cnc_get_exec_state(EXEC_ALLACTIVE) && !cnc_get_exec_state(EXEC_JOG)) // Jog only allowed in IDLE or JOG mode
			{
				parser_discard_command();
				return STATUS_IDLE_ERROR;
			}
			return GRBL_JOG_CMD;
		}
		break;
	default:
		switch (grbl_cmd_str[0])
		{
#if EMULATE_GRBL_STARTUP == 2
		case 'I':
			if (grbl_cmd_str[1] == 'E' && grbl_cmd_len == 2 && c == EOL)
			{
				return GRBL_SEND_SYSTEM_INFO_EXTENDED;
			}
			break;
#endif
		case 'R':
			if (grbl_cmd_str[1] == 'S' && grbl_cmd_str[2] == 'T' && c == '=' && grbl_cmd_len == 3)
			{
				grbl_cmd_str[3] = '=';
				grbl_cmd_len++;
				c = grbl_stream_getc();
				if (grbl_stream_getc() == EOL)
				{
					switch (c)
					{
					case '$':
						settings_reset(false);
						return GRBL_SEND_SETTINGS_RESET;
					case '#':
						parser_parameters_reset();
						return GRBL_SEND_SETTINGS_RESET;
					case '*':
#ifndef DISABLE_SAFE_SETTINGS
						g_settings_error = 0;
#endif
						settings_reset(true);
						parser_parameters_reset();
						return GRBL_SEND_SETTINGS_RESET;
					default:
						return STATUS_INVALID_STATEMENT;
					}
				}
			}
			break;
#ifdef ENABLE_EXTRA_SETTINGS_CMDS
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
					settings_save(SETTINGS_ADDRESS_OFFSET, (uint8_t *)&g_settings, (uint8_t)sizeof(settings_t));
					return GRBL_SETTINGS_DEFAULT;
				}
			}
			break;
#endif
		}
		break;
	}

#if (defined(ENABLE_PARSER_MODULES) || defined(BOARD_HAS_CUSTOM_SYSTEM_COMMANDS))
	grbl_cmd_args_t args = {&error, grbl_cmd_str, grbl_cmd_len, c};
	EVENT_INVOKE(grbl_cmd, &args);
#endif

	return error;
}

static uint8_t parser_grbl_exec_code(uint8_t code)
{
#ifdef ENABLE_RS274NGC_EXPRESSIONS
	uint16_t param;
	float value;
#endif
	switch (code)
	{
	case GRBL_SEND_SYSTEM_SETTINGS:
		proto_cnc_settings();
		break;
	case GRBL_SEND_COORD_SYSTEM:
		proto_gcode_coordsys();
		break;
	case GRBL_SEND_PARSER_MODES:
		proto_gcode_modes();
		break;
	case GRBL_SEND_STARTUP_BLOCKS:
		proto_start_blocks();
		break;
	case GRBL_TOGGLE_CHECKMODE:
		if (mc_toogle_checkmode())
		{
			proto_feedback(MSG_FEEDBACK_4);
		}
		else
		{
			proto_feedback(MSG_FEEDBACK_5);
			cnc_alarm(EXEC_ALARM_SOFTRESET);
		}
		break;
	case GRBL_SEND_SETTINGS_RESET:
		proto_feedback(MSG_FEEDBACK_9);
		break;
	case GRBL_UNLOCK:
		if (cnc_unlock(true) == UNLOCK_OK)
		{
#if ASSERT_PIN(SAFETY_DOOR)
			if (cnc_get_exec_state(EXEC_DOOR))
			{
				return STATUS_CHECK_DOOR;
			}
#endif
			proto_feedback(MSG_FEEDBACK_3);
		}
		break;
	case GRBL_HOME:
		if (!g_settings.homing_enabled)
		{
			return STATUS_SETTING_DISABLED;
		}

		if (cnc_unlock(true) == UNLOCK_OK)
		{
#if ASSERT_PIN(SAFETY_DOOR)
			if (cnc_get_exec_state(EXEC_DOOR))
			{
				return STATUS_CHECK_DOOR;
			}
#endif
			cnc_home();
		}
		break;
	case GRBL_HELP:
		proto_print(MSG_HELP);
		break;
#ifdef ENABLE_EXTRA_SETTINGS_CMDS
	case GRBL_SETTINGS_SAVED:
		proto_feedback(MSG_FEEDBACK_13);
		break;
	case GRBL_SETTINGS_LOADED:
		proto_feedback(MSG_FEEDBACK_14);
		break;
	case GRBL_SETTINGS_DEFAULT:
		proto_feedback(MSG_FEEDBACK_15);
		break;
#endif
#ifdef ENABLE_RS274NGC_EXPRESSIONS
	case GRBL_PRINT_PARAM:
		if (parser_get_float(&value) == NUMBER_OK
#ifdef ENABLE_NAMED_PARAMETERS
				|| parser_get_namedparam_id(&value) == NUMBER_OK
#endif
		)
		{
			param = value;
			value = parser_get_parameter(param);
		}
		else
		{
			return STATUS_INVALID_STATEMENT;
		}

		if (grbl_stream_getc() != EOL)
		{
			return STATUS_INVALID_STATEMENT;
		}
		proto_info("#%d=%f", param, value);
		break;
#endif

#ifdef ENABLE_PIN_DEBUG_EXTRA_CMD
	case GRBL_PINS_STATES:
		proto_pins_states();
		break;
#endif
#ifdef ENABLE_SYSTEM_INFO
	case GRBL_SEND_SYSTEM_INFO:
		proto_cnc_info(false);
		break;
#if EMULATE_GRBL_STARTUP == 2
	case GRBL_SEND_SYSTEM_INFO_EXTENDED:
		proto_cnc_info(true);
		break;
#endif
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

/**
 *
 *
 * STEP 1
 * Fetches the next line from the mcu communication buffer and preprocesses the string
 * In the preprocess these steps are executed
 * 	1. Whitespaces are ignored
 * 	2. Comments are parsed (nothing is done besides parsing for now)
 * 	3. All letters are upper-cased
 * 	4. Checks number formats in all words
 * 	5. Checks for modal groups and words collisions
 *
 *
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
		uint8_t word = 0;
		float value = 0;
#ifdef ENABLE_RS274NGC_EXPRESSIONS
		float assign_val = 0;
#endif

		error = parser_get_token(&word, &value);
		DBGMSG("Parser word %c", word);

		if (error)
		{
			return error;
		}
		uint8_t code = (uint8_t)truncf(value);
		// check mantissa
		uint8_t m = (uint8_t)lroundf(((value - code) * 100.0f));
		uint8_t mantissa = 0;
		switch (m)
		{
		case 50:
			mantissa++;
			__FALL_THROUGH__
		case 40:
			mantissa++;
			__FALL_THROUGH__
		case 30:
			mantissa++;
			__FALL_THROUGH__
		case 20:
			mantissa++;
			__FALL_THROUGH__
		case 10:
			mantissa++;
			__FALL_THROUGH__
		case 0:
			break;
		default:
			mantissa = 255;
			break;
		}

		switch (word)
		{
#ifdef ENABLE_RS274NGC_EXPRESSIONS
		case '#':
			if ((value < 1) || (value > RS274NGC_MAX_USER_VARS) || ((int)floorf(value) != value))
			{
				return STATUS_GCODE_MAX_VALUE_EXCEEDED;
			}
			if (!parser_get_float(&assign_val))
			{
				return STATUS_BAD_NUMBER_FORMAT;
			}
			DBGMSG("Assign #%lu=%f", (uint32_t)value, assign_val);
			if (new_state->modified_params_count >= RS274NGC_MAX_PARAMS_SET_PER_LINE)
			{
				return STATUS_MAXIMUM_PARAMS_PER_BLOCK_EXCEEDED;
			}
			new_state->modified_params[new_state->modified_params_count].id = (uint16_t)value;
			new_state->modified_params[new_state->modified_params_count].value = assign_val;
			new_state->modified_params_count++;
			break;
#ifdef ENABLE_O_CODES
		case 'O':
			error = parser_ocode_word((uint16_t)truncf(value), new_state, cmd);
			if (error != STATUS_OK)
			{
				// an error on a subrotine will cause the top subrotine to close and the stack to collapse
				while (o_code_end_subrotine())
					;
				return error;
			}
			break;
		case FILE_EOF:
			if (o_code_end_subrotine())
			{
				break;
			}
			grbl_stream_change(NULL);
			__FALL_THROUGH__
#endif
#endif
		case EOL:
#ifdef GCODE_COUNT_TEXT_LINES
			// if enabled store line number
			linecounter++;
			words->n = linecounter;
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

		DBGMSG("Parser var value %f", value);

#ifdef ENABLE_PARSER_MODULES
		if ((error == STATUS_GCODE_UNSUPPORTED_COMMAND || error == STATUS_GCODE_UNUSED_WORDS))
		{
			gcode_parse_args_t args = {word, code, &error, value, new_state, words, cmd};
			EVENT_INVOKE(gcode_parse, &args);
		}
#endif

#ifdef IGNORE_UNDEFINED_AXIS
		if (error == STATUS_GCODE_UNUSED_WORDS)
		{
			if (word <= 'C' || word >= 'X') // ignore undefined axis chars
			{
				// ignore
				error = STATUS_OK;
			}
		}
#endif
		if (error)
		{
			return error;
		}

		wordcount++;
	}
	// Never should reach
	return STATUS_CRITICAL_FAIL;
}

/**
 *
 *
 * STEP 2
 * Validadates command by checking for errors on all G/M Codes
 * 	RS274NGC v3 - 3.5 G Codes
 * 	RS274NGC v3 - 3.6 Input M Codes
 * 	RS274NGC v3 - 3.7 Other Input Codes
 *
 *
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
		case G4:
			if (!(cmd->words & (GCODE_WORD_P)))
			{
				return STATUS_GCODE_VALUE_WORD_MISSING;
			}
			// P is not between 1 and N of coord systems
			if (words->p < 0)
			{
				return STATUS_NEGATIVE_VALUE;
			}
			break;
#ifndef DISABLE_G10_SUPPORT
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
#ifndef DISABLE_HOME_SUPPORT
			if (words->p != 28 && words->p != 30)
#endif
			{
				if (words->p < 0 || words->p > COORD_SYS_COUNT)
				{
					return STATUS_GCODE_UNSUPPORTED_COORD_SYS;
				}
			}
			break;
#endif
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
	// subset of canned cycles
	if (CHECKFLAG(cmd->groups, GCODE_GROUP_MOTION))
	{
		switch (new_state->groups.motion)
		{
#ifndef IGNORE_G0_G1_MISSING_AXIS_WORDS
		case G0: // G0
		case G1: // G1
#endif
#ifndef DISABLE_PROBING_SUPPORT
		case G38: // G38.2, G38.3, G38.4, G38.5
			if (!CHECKFLAG(cmd->words, GCODE_ALL_AXIS))
			{
				return STATUS_GCODE_NO_AXIS_WORDS;
			}
			break;
#endif
#ifndef DISABLE_ARC_SUPPORT
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

#ifdef ENABLE_G39_H_MAPPING
				if (new_state->groups.height_map_active)
				{
					return STATUS_INVALID_PLANE_SELECTED;
				}
#endif
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

#ifdef ENABLE_G39_H_MAPPING
				if (new_state->groups.height_map_active)
				{
					return STATUS_INVALID_PLANE_SELECTED;
				}
#endif
				break;
			}
			break;
#endif
		case G80: // G80 and
			if (CHECKFLAG(cmd->words, GCODE_ALL_AXIS) && !cmd->group_0_1_useaxis)
			{
				return STATUS_GCODE_AXIS_WORDS_EXIST;
			}

			break;
#ifdef ENABLE_G39_H_MAPPING
		case G39:
			// G39
			if (!new_state->groups.motion_mantissa)
			{
				if (new_state->groups.plane != G17)
				{
					return STATUS_INVALID_PLANE_SELECTED;
				}
				// if I, J, Z and R are missing
				if ((cmd->words & (GCODE_WORD_I | GCODE_WORD_J | GCODE_WORD_Z | GCODE_WORD_R)) != (GCODE_WORD_I | GCODE_WORD_J | GCODE_WORD_Z | GCODE_WORD_R))
				{
					return STATUS_GCODE_VALUE_WORD_MISSING;
				}
				// if either I or J are negative
				if (words->ijk[0] < 0 || words->ijk[0] < 0)
				{
					return STATUS_NEGATIVE_VALUE;
				}
			}
			break;
#endif
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

				// Q/D is negative
				if (words->d < 0)
				{
					return STATUS_NEGATIVE_VALUE;
				}
			}

			break;
#endif
		}

		// group 5 - feed rate mode
		if (new_state->groups.motion != G0)
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

	// https://linuxcnc.org/docs/html/gcode/g-code.html#gcode:g80
	// any axis word with active G80 command is invalid
	if (CHECKFLAG(cmd->words, GCODE_ALL_AXIS) && new_state->groups.motion == G80)
	{
		return STATUS_GCODE_AXIS_WORDS_EXIST;
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
#if TOOL_COUNT > 1
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
	// extension commands can only be processed individually
	if (cmd->group_extended > 0 && cmd->groups != 0)
	{
		return STATUS_GCODE_MODAL_GROUP_VIOLATION;
	}

	switch (cmd->group_extended)
	{
	case 0: // no extended command
		break;
#if (SERVOS_MASK != 0)
	case M10:
		if (CHECKFLAG(cmd->words, (GCODE_WORD_S | GCODE_WORD_P)) != (GCODE_WORD_S | GCODE_WORD_P))
		{
			return STATUS_GCODE_VALUE_WORD_MISSING;
		}
		break;
#endif
	}

	return STATUS_OK;
}

/**
 *
 *
 * STEP 3
 * Executes the command
 * 	Follows the RS274NGC v3 - 3.8 Order of Execution
 * All coordinates are converted to machine absolute coordinates before sent to the motion controller
 *
 *
 */
static uint8_t parser_exec_command(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
	float target[AXIS_COUNT];
#ifndef DISABLE_ARC_SUPPORT
	// plane selection
	uint8_t a = 0;
	uint8_t b = 0;
	uint8_t offset_a = 0;
	uint8_t offset_b = 0;
	float radius, x, y;
#endif
#ifndef DISABLE_PROBING_SUPPORT
	uint8_t probe_flags;
#endif
	motion_data_t block_data = {0};
	uint8_t error = STATUS_OK;
	bool update_tools = false;

#ifdef ENABLE_PARSER_MODULES
	gcode_exec_args_t args = {&error, new_state, words, cmd, target, &block_data};
	EVENT_INVOKE(gcode_exec_modifier, &args);
#endif

	// stoping from previous command M2 or M30 command
	if (new_state->groups.stopping && !CHECKFLAG(cmd->groups, GCODE_GROUP_STOPPING))
	{
#ifndef DISABLE_ENDPROGRAM_LOCK
		if (new_state->groups.stopping == 3 || new_state->groups.stopping == 4)
		{
			return STATUS_PROGRAM_ENDED;
		}
#endif

		new_state->groups.stopping = 0;
	}

	// standalone extended command
	// extended commands with positive codes will run here
	// a special case (negative extended command) is reserved for additional motion commands
	if (cmd->group_extended > 0)
	{
		itp_sync();
		switch (cmd->group_extended)
		{
#if (SERVOS_MASK != 0)
		case M10:
			if (words->p < 6)
			{
				io_set_pinvalue(words->p + SERVO_PINS_OFFSET, (uint8_t)CLAMP(words->s, 0, 255));
			}
			break;
#endif

		default:
			error = STATUS_GCODE_UNSUPPORTED_COMMAND;
#ifdef ENABLE_PARSER_MODULES
			EVENT_INVOKE(gcode_exec, &args);
#endif
		}

		return error;
	}

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

#if TOOL_COUNT > 1
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
#endif

	// 7. spindle on/rev/off (M3/M4/M5)
	block_data.spindle = new_state->spindle;
	block_data.motion_flags.bit.spindle_running = new_state->groups.spindle_turning;
	update_tools = ((parser_state.spindle != new_state->spindle) | (parser_state.groups.spindle_turning != new_state->groups.spindle_turning));

	// spindle speed or direction was changed (force a safety dwell to let the spindle change speed and continue)
	if (update_tools && !g_settings.laser_mode)
	{
		mc_update_tools(&block_data);
#if (DELAY_ON_SPINDLE_SPEED_CHANGE > 0)
		block_data.dwell = (uint16_t)lroundf(DELAY_ON_SPINDLE_SPEED_CHANGE * 1000);
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

	// 10. dwell (or if any other nonmodal command except G53 requires a sync motion)
	if (new_state->groups.nonmodal != 0 && new_state->groups.nonmodal != G53)
	{
		itp_sync();
		if (new_state->groups.nonmodal == G4)
		{
			// calc dwell in milliseconds
			block_data.dwell = MAX(block_data.dwell, (uint16_t)lroundf(MIN(words->p * 1000.0f, 65535)));
			new_state->groups.nonmodal = 0;
		}
	}

	// after all spindle, overrides, coolant and dwells are set
	// execute sync if dwell is present
	if (block_data.dwell)
	{
		mc_dwell(&block_data);
	}

#ifndef DISABLE_ARC_SUPPORT
	// 11. set active plane (G17, G18, G19)
	switch (new_state->groups.plane)
	{
#if (defined(AXIS_X) && defined(AXIS_Y))
	case G17:
		a = AXIS_X;
		b = AXIS_Y;
		offset_a = AXIS_X;
		offset_b = AXIS_Y;
		break;
#endif
#if (defined(AXIS_X) && defined(AXIS_Z))
	case G18:
#ifdef ENABLE_G39_H_MAPPING
		if (new_state->groups.height_map_active)
		{
			return STATUS_INVALID_PLANE_SELECTED;
		}
#endif
		a = AXIS_Z;
		b = AXIS_X;
		offset_a = AXIS_Z;
		offset_b = AXIS_X;
		break;
#endif
#if (defined(AXIS_Y) && defined(AXIS_Z))
	case G19:
#ifdef ENABLE_G39_H_MAPPING
		if (new_state->groups.height_map_active)
		{
			return STATUS_INVALID_PLANE_SELECTED;
		}
#endif
		a = AXIS_Y;
		b = AXIS_Z;
		offset_a = AXIS_Y;
		offset_b = AXIS_Z;
		break;
#endif
	}
#endif

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
			parser_parameters.tool_length_offset = words->xyzabc[AXIS_TOOL];
			CLEARFLAG(cmd->words, GCODE_WORD_Z);
			words->xyzabc[AXIS_TOOL] = 0; // resets parameter so it it doen't do anything else
		}
		parser_wco_counter = 0;
	}
#endif
// 15. coordinate system selection (G54, G55, G56, G57, G58, G59, G59.1, G59.2, G59.3) (OK nothing to be done)
#ifndef DISABLE_COORD_SYS_SUPPORT
	if (CHECKFLAG(cmd->groups, GCODE_GROUP_COORDSYS))
	{
		itp_sync();
		parser_parameters.coord_system_index = new_state->groups.coord_system;
		parser_coordinate_system_load(parser_parameters.coord_system_index, parser_parameters.coord_system_offset);
		parser_wco_counter = 0;
	}
#endif
// 16. set path control mode (G61, G61.1, G64)
#ifndef DISABLE_PATH_MODES
	switch (new_state->groups.path_mode)
	{
	case G61_1:
		block_data.motion_mode |= PLANNER_MOTION_EXACT_STOP;
		break;
	case G64:
		block_data.motion_mode |= PLANNER_MOTION_CONTINUOUS;
		break;
	}
#endif

	// 17. set distance mode (G90, G91)
	memcpy(target, parser_last_pos, sizeof(parser_last_pos));
	// absolute distances if distance mode is G90
	bool abspos = (new_state->groups.distance_mode == G90);
	// or if any nonmodal command is active (execept G4 that auto clears itself)
	abspos |= (new_state->groups.nonmodal != 0);
	// or if any nonmodal command is active (execept G4 that auto clears itself)

	// for all not explicitly declared target retain their position or add offset
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
#ifndef AXIS_A_FORCE_RELATIVE_MODE
		target[AXIS_A] = (abspos) ? words->xyzabc[AXIS_A] : (words->xyzabc[AXIS_A] + target[AXIS_A]);
#else
		target[AXIS_A] = (words->xyzabc[AXIS_A] + target[AXIS_A]);
#endif
	}
#endif
#ifdef AXIS_B
	if (CHECKFLAG(cmd->words, GCODE_WORD_B))
	{
#ifndef AXIS_B_FORCE_RELATIVE_MODE
		target[AXIS_B] = (abspos) ? words->xyzabc[AXIS_B] : (words->xyzabc[AXIS_B] + target[AXIS_B]);
#else
		target[AXIS_B] = (words->xyzabc[AXIS_B] + target[AXIS_B]);
#endif
	}
#endif
#ifdef AXIS_C
	if (CHECKFLAG(cmd->words, GCODE_WORD_C))
	{
#ifndef AXIS_C_FORCE_RELATIVE_MODE
		target[AXIS_C] = (abspos) ? words->xyzabc[AXIS_C] : (words->xyzabc[AXIS_C] + target[AXIS_C]);
#else
		target[AXIS_C] = (words->xyzabc[AXIS_C] + target[AXIS_C]);
#endif
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
	error = STATUS_OK;
	switch (new_state->groups.nonmodal)
	{
#ifndef DISABLE_G10_SUPPORT
	case G10: // G10
		index = ((uint8_t)words->p) ? words->p : (parser_parameters.coord_system_index + 1);
		switch (index)
		{
#ifndef DISABLE_HOME_SUPPORT
		case 28:
			index = G28HOME;
			break;
		case 30:
			index = G30HOME;
			break;
#endif
		default:
			index--;
			break;
		}
#endif
		break;
	case G92:
		index = G92OFFSET;
		break;
	case G92_1: // G92.1
		memset(g92permanentoffset, 0, sizeof(g92permanentoffset));
		__FALL_THROUGH__
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
	case G53:
		index = 254;
		break;
	}

	// check from were to read the previous values for the target array
	float coords[AXIS_COUNT];
	bool relative_target = true;
	float *relative_offset = NULL;
	switch (index)
	{
	case 254: // G53 (passthrough)
		break;
	case 255: // No nonmodal
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
	case G92OFFSET:
		relative_offset = parser_parameters.coord_system_offset;
		memcpy(coords, parser_parameters.g92_offset, sizeof(coords));
		break;
#ifndef DISABLE_G10_SUPPORT
	default:
		relative_offset = parser_parameters.g92_offset;
		relative_target = (words->l == 20);
		if (index == parser_parameters.coord_system_index)
		{
			memcpy(coords, parser_parameters.coord_system_offset, sizeof(coords));
		}
		else
		{
			parser_coordinate_system_load(index, coords);
		}
		break;
#endif
	}

	if (index <= G92OFFSET)
	{
		for (uint8_t i = AXIS_COUNT; i != 0;)
		{
			i--;
			if (CHECKFLAG(cmd->words, (1 << i)))
			{
				coords[i] = (relative_target) ? -(target[i] - parser_last_pos[i] + relative_offset[i]) : target[i];
			}
		}
#ifdef AXIS_TOOL
		if (relative_target)
		{
			if (CHECKFLAG(cmd->words, (1 << AXIS_TOOL)))
			{
				coords[AXIS_TOOL] += parser_parameters.tool_length_offset;
			}
		}
#endif
	}

	// stores G10 or G92 command in the right address
	switch (index)
	{
	case 254:
	case 255:
		break;
	case G92OFFSET:
		memcpy(parser_parameters.g92_offset, coords, sizeof(parser_parameters.g92_offset));
		memcpy(g92permanentoffset, parser_parameters.g92_offset, sizeof(g92permanentoffset));
#ifdef G92_STORE_NONVOLATILE
		settings_save(G92ADDRESS, (uint8_t *)&g92permanentoffset, PARSER_PARAM_SIZE);
#endif
		parser_wco_counter = 0;
		break;
#ifndef DISABLE_G10_SUPPORT
	default:
		settings_save(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (index * PARSER_PARAM_ADDR_OFFSET), (uint8_t *)coords, PARSER_PARAM_SIZE);
#ifndef DISABLE_COORDINATES_SYSTEM_RAM
		memcpy(&coordinate_systems[index], coords, PARSER_PARAM_SIZE);
#endif
		if (index == parser_parameters.coord_system_index)
		{
			memcpy(parser_parameters.coord_system_offset, coords, PARSER_PARAM_SIZE);
		}
		parser_wco_counter = 0;
		break;
#endif
	}

	// laser disabled in nonmodal moves
	if (g_settings.laser_mode && new_state->groups.nonmodal)
	{
		block_data.spindle = 0;
	}

	// 20. perform motion (G0 to G3, G80 to G89), as modified (possibly) by G53.
	// G80 does no motion
	// G81 to G89 is executed in a separate function and uses G53,G0,G1 and G4 has building blocks
	// only if any target word was used
	if (new_state->groups.nonmodal == 0 && CHECKFLAG(cmd->words, GCODE_ALL_AXIS))
	{
#ifdef ENABLE_G39_H_MAPPING
		if (new_state->groups.height_map_active)
		{
			block_data.motion_mode |= MOTIONCONTROL_MODE_APPLY_HMAP;
		}
#endif

#ifdef ENABLE_PARSER_MODULES
		EVENT_INVOKE(gcode_before_motion, &args);
#endif

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
			__FALL_THROUGH__
		case G1:
			if (block_data.feed == 0)
			{
				return STATUS_FEED_NOT_SET;
			}
			error = mc_line(target, &block_data);
			break;
#ifndef DISABLE_ARC_SUPPORT
		case G2:
		case G3:
			if (block_data.feed == 0)
			{
				return STATUS_FEED_NOT_SET;
			}
			else
			{
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
			}
			break;
#endif
#ifndef DISABLE_PROBING_SUPPORT
		case G38: // G38.2
							// G38.3
							// G38.4
							// G38.5
			probe_flags = (new_state->groups.motion_mantissa > 3) ? 1 : 0;
			probe_flags |= (new_state->groups.motion_mantissa & 0x01) ? 2 : 0;

			error = mc_probe(target, probe_flags, &block_data);
			if (error == STATUS_PROBE_SUCCESS)
			{
				parser_parameters.last_probe_ok = 1;
				error = STATUS_OK;
			}
			else
			{
				// failed at this position
				parser_sync_probe();
				parser_parameters.last_probe_ok = 0;
			}
			// sync probe position
			parser_update_probe_pos();

			if (error == STATUS_OK)
			{
				proto_probe_result(parser_parameters.last_probe_ok);
			}

			return error;
#ifdef ENABLE_G39_H_MAPPING
		case G39:
			if (!new_state->groups.motion_mantissa)
			{
				error = mc_build_hmap(target, words->ijk, words->r, &block_data);
				if (error == STATUS_OK)
				{
					new_state->groups.height_map_active = 1;
				}
				else
				{
					// clear the map
					mc_clear_hmap();
					new_state->groups.height_map_active = 0;
				}
			}
			return error;
#endif
#endif
#ifdef ENABLE_PARSER_MODULES
		default: // other motion commands (derived from extended commands)
			args.new_state = new_state;
			args.words = words;
			args.cmd = cmd;
			EVENT_INVOKE(gcode_exec, &args);
			break;
#endif
		}

#ifdef ENABLE_PARSER_MODULES
		EVENT_INVOKE(gcode_after_motion, &args);
#endif

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
#ifndef DISABLE_ENDPROGRAM_LOCK
		mc_pause();
#else
		itp_sync();
#endif
		if (resetparser)
		{
			cnc_stop();
			proto_feedback(MSG_FEEDBACK_8);
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

/**
 *
 *
 * Parse the next gcode line available in the buffer and send it to the motion controller
 *
 *
 */
static uint8_t parser_gcode_command(bool is_jogging)
{
	uint8_t result = 0;
	// initializes new state
	parser_state_t next_state = {0};
	parser_words_t words = {0};
	parser_cmd_explicit_t cmd = {0};
	// next state will be the same as previous except for nonmodal group (is set with 0)
	memcpy(&next_state, &parser_state, sizeof(parser_state_t));
	next_state.groups.nonmodal = 0; // reset nonmodal

#ifdef ENABLE_RS274NGC_EXPRESSIONS
	// reset modified params
	next_state.modified_params_count = 0;
	memset(next_state.modified_params, 0, sizeof(next_state.modified_params));
#endif

	// fetch command
	result = parser_fetch_command(&next_state, &words, &cmd);
	if (result != STATUS_OK)
	{
		parser_discard_command();
		return result;
	}

	if (is_jogging)
	{
		cnc_set_exec_state(EXEC_JOG);
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
	if (!is_jogging)
	{
#ifdef ENABLE_RS274NGC_EXPRESSIONS
		// stores the new parameters
		for (uint8_t i = 0; i < next_state.modified_params_count; i++)
		{
			parser_set_parameter(next_state.modified_params[i].id, next_state.modified_params[i].value);
		}
#endif
		// if everything went ok updates the parser modal groups and position
		memcpy(&parser_state, &next_state, sizeof(parser_state_t));
#ifdef DISABLE_ENDPROGRAM_LOCK
		if (next_state.groups.stopping == 3 || next_state.groups.stopping == 4)
		{
			parser_reset(false);
		}
#endif
	}

	return result;
}

/**
 *
 *
 * Parses comments almost as defined in the RS274NGC
 * To be compatible with Grbl it accepts bad format comments
 * On error returns false otherwise returns true
 *
 *
 */
#define COMMENT_OK 1
#define COMMENT_NOTOK 2
#ifdef PROCESS_COMMENTS
bool g_mute_comment_output;
#endif
static void parser_get_comment(uint8_t start_char)
{
	uint8_t comment_end = 0;
#ifdef PROCESS_COMMENTS
	uint8_t msg_parser = 0;
#endif
	for (;;)
	{
		uint8_t c = grbl_stream_peek();
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
			msg_parser = (c == 'M' || c == 'm') ? 1 : 0xFF;
			break;
		case 1:
			msg_parser = (c == 'S' || c == 's') ? 2 : 0xFF;
			break;
		case 2:
			msg_parser = (c == 'G' || c == 'g') ? 3 : 0xFF;
			break;
		case 3:
			msg_parser = (c == ',') ? 4 : 0xFF;
			if (!g_mute_comment_output)
			{
				proto_print(MSG_FEEDBACK_START);
			}
			break;
		case 4:
#ifdef ENABLE_RS274NGC_EXPRESSIONS
			if (c == '#')
			{
				float f = 0;
				if (parser_get_float(&f) != NUMBER_UNDEF)
				{
					if (!g_mute_comment_output)
					{
						proto_printf("%f", f);
					}
					c = grbl_stream_peek();
					if (c == ')' || c == EOL)
					{
						comment_end = COMMENT_OK;
					}
				}
			}
#endif
			if (comment_end != COMMENT_OK)
			{
				if (!g_mute_comment_output)
				{
					proto_putc(c);
				}
			}

			break;
		}
#endif

		if (c != EOL)
		{
			grbl_stream_getc();
		}

		if (comment_end)
		{
#ifdef PROCESS_COMMENTS
			if (msg_parser == 4)
			{
				if (!g_mute_comment_output)
				{
					proto_print(MSG_FEEDBACK_END);
				}
			}
#endif
			return;
		}
	}
}

#ifdef ECHO_CMD
uint8_t echo_counter;
#endif

unsigned char parser_get_next_preprocessed(bool peek)
{
	uint8_t c = grbl_stream_peek();

	while (c == ' ' || c == '(' || c == ';')
	{
		grbl_stream_getc();
		if (c != ' ')
		{
			parser_get_comment(c);
		}
		c = grbl_stream_peek();
	}

	if (!peek)
	{
		grbl_stream_getc();
#ifdef ECHO_CMD
		if (!echo_counter++)
		{
			grbl_stream_start_broadcast();
			proto_print(MSG_ECHO);
		}
		if (c > FILE_EOF)
		{
			proto_putc(c);
		}
		else
		{
			echo_counter = 0;
			proto_print(MSG_FEEDBACK_END);
		}
#endif
	}

	return c;
}

#ifndef ENABLE_RS274NGC_EXPRESSIONS
uint8_t parser_get_float(float *value)
{
	return prt_atof((void *)parser_get_next_preprocessed, NULL, value);
}
#endif

static uint8_t parser_get_token(uint8_t *word, float *value)
{
// this flushes leading white chars and also takes care of processing comments
#ifndef ENABLE_RS274NGC_EXPRESSIONS
	uint8_t c = parser_get_next_preprocessed(false);
#else
	uint8_t c = parser_backtrack;
	parser_backtrack = 0;
	if (!c)
	{
		c = parser_get_next_preprocessed(false);
	}
#endif

	// if other uint8_t starts tokenization
	c = TOUPPER(c);

	*word = c;
	switch (c)
	{
	case EOL: // EOL
		return STATUS_OK;
	case OVF:
		return STATUS_OVERFLOW;
#ifdef ENABLE_RS274NGC_EXPRESSIONS
	case '#':
		if (parser_get_float(value) != NUMBER_OK)
		{
			return STATUS_INVALID_STATEMENT;
		}
		c = parser_get_next_preprocessed(false);
		if (c != '=')
		{
			return STATUS_INVALID_STATEMENT;
		}
		parser_backtrack = 0;
		break;
#ifdef ENABLE_O_CODES
	case FILE_EOF:
		// allow FILE_EOF to continue
		break;
#endif
#endif
	default:
		if (c >= 'A' && c <= 'Z') // invalid recognized uint8_t
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
#ifndef DISABLE_PROBING_SUPPORT
		case 38:
#ifdef ENABLE_G39_H_MAPPING
		case 39:
#endif
#endif
		case 43:
		case 59:
		case 61:
		case 92:
			break;
		default:
			return STATUS_GCODE_UNSUPPORTED_COMMAND;
		}
	}

	new_state->groups.motion_mantissa = mantissa;

	switch (code)
	{
// motion codes
#ifndef DISABLE_PROBING_SUPPORT
	case 38: // check if 38.x
		if (mantissa < 2 || mantissa > 5)
		{
			return STATUS_GCODE_UNSUPPORTED_COMMAND;
		}
		__FALL_THROUGH__
#ifdef ENABLE_G39_H_MAPPING
	case 39:
#endif
#endif
	case 0:
	case 1:
#ifndef DISABLE_ARC_SUPPORT
	case 2:
	case 3:
#endif
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

		if (cmd->group_0_1_useaxis)
		{
			return STATUS_GCODE_MODAL_GROUP_VIOLATION;
		}

		if (code != 80)
		{
			cmd->group_0_1_useaxis = 1;
		}

#ifdef ENABLE_G39_H_MAPPING
		if (code == 39)
		{
			new_state->groups.height_map_active = (mantissa == 2) ? 1 : 0;

			if (mantissa)
			{
				return STATUS_OK;
			}
		}

#endif
		new_group |= GCODE_GROUP_MOTION;
		new_state->groups.motion = code;
		break;
#ifndef DISABLE_ARC_SUPPORT
	case 17:
	case 18:
	case 19:
		code -= 17;
		new_state->groups.plane = code;
		new_group |= GCODE_GROUP_PLANE;
		break;
#endif
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
		if (mantissa > 1)
		{
			return STATUS_GCODE_UNSUPPORTED_COMMAND;
		}
		__FALL_THROUGH__
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
#ifndef DISABLE_COORD_SYS_SUPPORT
	case 55:
	case 56:
	case 57:
	case 58:
	case 59:
		if (mantissa > 3)
		{
			return STATUS_GCODE_UNSUPPORTED_COMMAND;
		}

		code -= (54 - mantissa);

		if (code > COORD_SYS_COUNT)
		{
			return STATUS_GCODE_UNSUPPORTED_COORD_SYS;
		}
		new_state->groups.coord_system = code;
#endif
		new_group |= GCODE_GROUP_COORDSYS;
		break;
#ifndef DISABLE_PATH_MODES
	case 61:
	case 64:
		code -= (61 - mantissa);
		if (mantissa > 1)
		{
			return STATUS_GCODE_UNSUPPORTED_COMMAND;
		}
		new_state->groups.path_mode = code;
		new_group |= GCODE_GROUP_PATH;
		break;
#endif
	// de following nonmodal colide with motion groupcodes
	case 92:
		if (mantissa > 3)
		{
			return STATUS_GCODE_UNSUPPORTED_COMMAND;
		}
		__FALL_THROUGH__
#ifndef DISABLE_G10_SUPPORT
	case 10:
#endif
#ifndef DISABLE_HOME_SUPPORT
	case 28:
	case 30:
#endif
		if (cmd->group_0_1_useaxis)
		{
			return STATUS_GCODE_MODAL_GROUP_VIOLATION;
		}
		cmd->group_0_1_useaxis = 1;
		__FALL_THROUGH__
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
		code += mantissa;
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
		__FALL_THROUGH__
	case 30:
		code = (code & 1) ? 5 : 3;
		__FALL_THROUGH__
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
#if TOOL_COUNT > 1
	case 6:
		new_group |= GCODE_GROUP_TOOLCHANGE;
		break;
#endif
	case 7:
	case 8:
#ifdef ENABLE_COOLANT
		cmd->groups |= GCODE_GROUP_COOLANT; // word overlapping allowed
#ifndef M7_SAME_AS_M8
		new_state->groups.coolant |= ((code == 8) ? M8 : M7);
#else
		new_state->groups.coolant |= M8;
#endif
		return STATUS_OK;
#endif
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
#if (SERVOS_MASK != 0)
	case 10:
		if (cmd->group_extended > 0)
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

static uint8_t parser_letter_word(uint8_t c, float value, uint8_t mantissa, parser_words_t *words, parser_cmd_explicit_t *cmd)
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
		return STATUS_OK;
#ifdef AXIS_X
	case 'X':
		new_words |= GCODE_WORD_X;
		words->xyzabc[AXIS_X] = value;
		break;
#endif
#ifdef AXIS_Y
	case 'Y':
#if ((AXIS_COUNT == 2) && defined(USE_Y_AS_Z_ALIAS))
	case 'Z':
#endif
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
	case 'D':
		new_words |= GCODE_WORD_D;
		words->d = value;
		break;
	case 'F':
		new_words |= GCODE_WORD_F;
		words->f = value;
		break;
	case 'I':
		new_words |= GCODE_WORD_I;
		words->ijk[0] = value;
		break;
	case 'J':
		new_words |= GCODE_WORD_J;
		words->ijk[1] = value;
		break;
	case 'K':
		new_words |= GCODE_WORD_K;
		words->ijk[2] = value;
		break;
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
#ifdef AXIS_TOOL
			new_words |= GCODE_WORD_TOOL;
			words->xyzabc[AXIS_TOOL] = g_settings.tool_length_offset[index];
#endif
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
		if (c >= 'A' && c <= 'Z') // invalid recognized uint8_t
		{
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

void parser_discard_command(void)
{
	uint8_t c;
	do
	{
		c = parser_get_next_preprocessed(false);
	} while (c > FILE_EOF);
#ifdef ECHO_CMD
	grbl_stream_start_broadcast();
	proto_printf(MSG_FEEDBACK_START "Cmd discarded" MSG_FEEDBACK_END);
#endif
}

void parser_reset(bool fullreset)
{
	// modified based on https://linuxcnc.org/docs/html/gcode/m-code.html#mcode:m2-m30

	parser_state.groups.stopping = 0;											// resets all stopping commands (M0,M1,M2,M30,M60)
	parser_state.groups.coord_system = G54;								// G54
	parser_state.groups.plane = G17;											// G17
	parser_state.groups.feed_speed_override = M48;				// M48
	parser_state.groups.cutter_radius_compensation = G40; // G40
	parser_state.groups.distance_mode = G90;							// G90
	parser_state.groups.feedrate_mode = G94;							// G94
	parser_state.groups.tlo_mode = G49;										// G49
#if TOOL_COUNT > 0
	parser_state.groups.coolant = M9;					// M9
	parser_state.groups.spindle_turning = M5; // M5
	parser_state.groups.path_mode = G61;
#endif
	parser_state.groups.motion = G1; // G1
	parser_state.groups.units = G21; // G21
	parser_wco_counter = 0;
#ifdef ENABLE_G39_H_MAPPING
	parser_state.groups.height_map_active = 0;
#endif

#ifdef ENABLE_PARSER_MODULES
	EVENT_INVOKE(parser_reset, &parser_state);
#endif

	if (!fullreset)
	{
		return;
	}

	// extra reset stuff out of the M2/M30 scope
	memset(parser_parameters.g92_offset, 0, sizeof(parser_parameters.g92_offset)); // G92.2
	parser_parameters.tool_length_offset = 0;
#if TOOL_COUNT > 0
	parser_state.groups.tool_change = 1;
#endif
#if TOOL_COUNT > 1
	parser_state.tool_index = g_settings.default_tool;
#endif
}

/**
 * loads parameters from NVM
 * loads G92 offset
 * loads G54 coordinate system
 * also checks all other coordinate systems and homing positions
 */
void parser_parameters_load(void)
{
// loads G92
#ifdef G92_STORE_NONVOLATILE
	settings_load(G92ADDRESS, (uint8_t *)&g92permanentoffset, PARSER_PARAM_SIZE);
	memcpy(parser_parameters.g92_offset, g92permanentoffset, sizeof(g92permanentoffset));
#else
	memset(parser_parameters.g92_offset, 0, PARSER_PARAM_SIZE);
	memset(g92permanentoffset, 0, PARSER_PARAM_SIZE);
#endif

	// loads all coordinate systems to check the storage integrity. This includes G28 and G30 if enabled
	for (uint8_t i = 1; i < TOTAL_COORDINATE_SYSTEMS; i++)
	{
#ifndef DISABLE_COORDINATES_SYSTEM_RAM
		settings_load(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (i * PARSER_PARAM_ADDR_OFFSET), (uint8_t *)coordinate_systems[i], PARSER_PARAM_SIZE);
#else
		settings_load(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (i * PARSER_PARAM_ADDR_OFFSET), (uint8_t *)parser_parameters.coord_system_offset, PARSER_PARAM_SIZE);
#endif
	}

	// load G54
	settings_load(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET, (uint8_t *)parser_parameters.coord_system_offset, PARSER_PARAM_SIZE);
#ifndef DISABLE_COORDINATES_SYSTEM_RAM
	memcpy(coordinate_systems[0], parser_parameters.coord_system_offset, PARSER_PARAM_SIZE);
#endif
}

void parser_sync_position(void)
{
	mc_get_position(parser_last_pos);
}

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

void parser_coordinate_system_load(uint8_t param, float *target)
{
	switch (param)
	{
	case G92OFFSET:
		memcpy(target, (uint8_t *)g92permanentoffset, PARSER_PARAM_SIZE);
		break;
	default:
#ifndef DISABLE_COORDINATES_SYSTEM_RAM
		memcpy(target, (uint8_t *)coordinate_systems[param], PARSER_PARAM_SIZE);
#else
		settings_load(SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (param * PARSER_PARAM_ADDR_OFFSET), (uint8_t *)target, PARSER_PARAM_SIZE);
#endif
		break;
	}
}

/**
 *
 *
 * Canned cycles code extensions
 *
 *
 */
#ifdef ENABLE_CANNED_CYCLES

static uint8_t sticky_mask;
static float sticky_r;
static float sticky_new;
static float sticky_old;

uint8_t parser_exec_command_block(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
	// not a canned cycle (run single command)
	if (new_state->groups.nonmodal != 0 || !CHECKFLAG(cmd->words, GCODE_ALL_AXIS) || new_state->groups.motion < G81 || new_state->groups.motion > G89)
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

	// set possible feed and spindle changes embedded in the canned command
	new_state->feedrate = canned_state.feedrate;
	new_state->spindle = canned_state.spindle;

	return error;
}
#endif

/**
 *
 *
 * RS274NGC expressions extensions
 * This includes:
 *  - numbered parameters
 *  - O Codes
 *
 *
 */
#ifdef ENABLE_RS274NGC_EXPRESSIONS

float g_parser_num_params[RS274NGC_MAX_USER_VARS];

#ifdef ENABLE_NAMED_PARAMETERS
static float parser_get_named_parameter(int param, int offset, uint8_t pos)
{
	float result = -1;
	switch (offset)
	{
	case 600:
		switch (pos)
		{
		case 1: // vmajor
			str_atof(CNC_MAJOR_MINOR_VERSION, &result);
			break;
		case 2: // vminor
			str_atof(CNC_PATCH_VERSION, &result);
			break;
#ifdef GCODE_PROCESS_LINE_NUMBERS
		case 3: // line
			break;
#endif
		}
		break;
	case 601:
		switch (pos)
		{
		case 0: // motion mode
			return parser_state.groups.motion * 10 + parser_state.groups.motion_mantissa;
		case 1: // plane
			return (17 + parser_state.groups.plane) * 10;
		case 2: // compensation mode (not implemented) always G40
			return 400;
		case 3: // metric
			return (parser_state.groups.distance_mode == G21);
		case 4: // imperial
			return (parser_state.groups.distance_mode == G20);
		case 5: // absolute
			return (parser_state.groups.distance_mode == G90);
		case 6: // incremental
			return (parser_state.groups.distance_mode == G91);
		case 7: // inverse time mode
			return (parser_state.groups.feedrate_mode == G93);
		case 8: // units per minute
			return (parser_state.groups.feedrate_mode == G94);
		case 9: // units per rev (not implemented) always 0
			return 0;
		}
		break;
	case 602:
		switch (pos)
		{
		case 0: // coordinate system
			offset = (54 + parser_state.groups.coord_system) * 10;
			switch (offset)
			{
			case 600:
				return 591;
			case 610:
				return 592;
			case 620:
				return 593;
			default:
				result = offset;
			}
			break;
		case 1: // tool offset
			return (parser_state.groups.tlo_mode == G43);
		case 2: // retract plane
			return (parser_state.groups.return_mode == G98);
		case 3: // retract old z
			return (parser_state.groups.return_mode == G99);
		case 4: // spindle rpm mode (always true)
			return 1;
		case 5: // spindle constant surface mode (not implemented)
		case 6: // lathe diam mode (not implemented)
		case 7: // lathe radius mode (not implemented)
			return 0;
		}
		break;
	case 603:
		switch (pos)
		{
		case 0: // spindle on
			return (parser_state.groups.spindle_turning != M5);
		case 1: // spindle cw
			return (parser_state.groups.spindle_turning == M3);
		case 2: // flood
			return (parser_state.groups.coolant & M7);
		case 3: // mist
			return (parser_state.groups.coolant & M8);
		}
		break;
	case 604:
		switch (pos)
		{
		case 0: // feed override
		case 1: // speed override
			return (parser_state.groups.feed_speed_override == M48);
		case 2: // adaptive feed (not implemented)
			return 0;
		case 3: // feed hold
			return cnc_get_exec_state(EXEC_HOLD);
		case 4: // flood
			return parser_state.feedrate;
		case 5: // rpm
			return parser_state.spindle;
		}
		break;
	case 605:
		if (pos < AXIS_COUNT)
		{
			return parser_last_pos[pos];
		}
		break;
	case 606:
		switch (pos)
		{
		case 1: // selected tool
			return parser_state.groups.tool_change;
		default: // current pocket and selected pocket (not implemented)
			return 0;
		}
		break;
	case 610:
		switch (pos)
		{
		case 0:
			return o_code_returned;
		case 1:
			return o_code_return_value;
		}
	case 611:
		switch (pos)
		{
		case 1:
			return o_code_stack_index;
		}
		break;
	}
	return result;
}
#endif
/**
 *
 *
 * Numbered parameters and expressions parsing
 *
 *
 */
float parser_get_parameter(uint16_t param)
{
	float result[AXIS_COUNT];
	int32_t probe_position[STEPPER_COUNT];
	float axis[AXIS_COUNT];
	uint16_t offset = param * 0.1f;
	uint8_t pos = (uint8_t)(param - (offset * 10));
	uint8_t index = 0;

	pos--;

	if (param > 0 && param <= RS274NGC_MAX_USER_VARS)
	{
		return g_parser_num_params[param - 1];
	}

	switch (offset)
	{
	case 506:
		parser_get_probe(probe_position);
		kinematics_steps_to_coordinates(probe_position, result);
		if (pos < AXIS_COUNT)
		{
			return result[pos];
		}
		break;
	case 507:
		pos++;
		if (!pos)
		{
			return parser_parameters.last_probe_ok;
		}
		break;
#ifndef DISABLE_HOME_SUPPORT
	case 516:
		parser_coordinate_system_load(G28HOME, result);
		if (pos < AXIS_COUNT)
		{
			return result[pos];
		}
		break;
	case 518:
		parser_coordinate_system_load(G30HOME, result);
		if (pos < AXIS_COUNT)
		{
			return result[pos];
		}
		break;
#endif
	case 521:
		return g92permanentoffset[pos];
	case 522:
		pos++;
		if (!pos)
		{
			return (parser_state.groups.coord_system + 1);
		}
		pos--;
		__FALL_THROUGH__
	case 524:
	case 526:
	case 528:
	case 530:
	case 532:
	case 534:
	case 536:
	case 538:
		index = ((offset - 522) >> 1);
		if (pos < AXIS_COUNT && index < COORD_SYS_COUNT)
		{
			if (index != parser_parameters.coord_system_index)
			{
				parser_coordinate_system_load(index, result);
			}
			else
			{
				return parser_parameters.coord_system_offset[pos];
			}
			return result[pos];
		}
		break;
	case 540:
		pos++;
		if (!pos)
		{
			return parser_state.groups.tool_change;
		}
		pos--;
		return g_settings.tool_length_offset[parser_state.groups.tool_change];
	case 542:
		pos++;
		if (pos < AXIS_COUNT)
		{
			parser_get_wco(axis);
			return (parser_last_pos[pos] - axis[pos]);
		}
		break;
	default:
#ifdef ENABLE_NAMED_PARAMETERS
		if (offset >= 600 && offset <= 610)
		{
			return parser_get_named_parameter(param, offset, pos + 1);
		}
#endif
	}

	return 0;
}

void parser_set_parameter(uint16_t param, float value)
{
	if (param > 0 && param <= RS274NGC_MAX_USER_VARS)
	{
		g_parser_num_params[param - 1] = value;
	}
}

#endif
