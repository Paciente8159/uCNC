/*
	Name: parser.h
	Description: Parses Grbl system commands and RS274NGC (GCode) commands
		The RS274NGC parser tries to follow the standard document version 3 as close as possible.
		The parsing is done in 3 steps:
			- Tockenization; Converts the command string to a structure with GCode parameters
			- Validation; Validates the command by checking all the parameters (part 3.5 - 3.7 of the document)
			- Execution; Executes the command by the orther set in part 3.8 of the document.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 07/12/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef PARSER_H
#define PARSER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../module.h"
#include "motion_control.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define EXTENDED_GCODE_BASE 0
#define EXTENDED_MCODE_BASE 1000
#define EXTENDED_MCODE(X) (EXTENDED_MCODE_BASE + X)
#define EXTENDED_GCODE(X) (EXTENDED_GCODE_BASE + X)
#define EXTENDED_MOTION_GCODE(X) (-EXTENDED_GCODE(X))

// group masks
#define GCODE_GROUP_MOTION 0x0001
#define GCODE_GROUP_PLANE 0x0002
#define GCODE_GROUP_DISTANCE 0x0004
#define GCODE_GROUP_FEEDRATE 0x0008
#define GCODE_GROUP_UNITS 0x0010
#define GCODE_GROUP_CUTTERRAD 0x0020
#define GCODE_GROUP_TOOLLENGTH 0x0040
#define GCODE_GROUP_RETURNMODE 0x0080
#define GCODE_GROUP_COORDSYS 0x0100
#define GCODE_GROUP_PATH 0x0200
#define GCODE_GROUP_STOPPING 0x0400
#define GCODE_GROUP_TOOLCHANGE 0x0800
#define GCODE_GROUP_SPINDLE 0x1000
#define GCODE_GROUP_COOLANT 0x2000
#define GCODE_GROUP_ENABLEOVER 0x4000
#define GCODE_GROUP_NONMODAL 0x8000

// word masks
#define GCODE_WORD_X 0x0001
#define GCODE_WORD_Y 0x0002
#define GCODE_WORD_Z 0x0004
#define GCODE_WORD_A 0x0008
#define GCODE_WORD_B 0x0010
#define GCODE_WORD_C 0x0020
#define GCODE_WORD_D 0x0040
#define GCODE_WORD_F 0x0080
#define GCODE_WORD_I 0x0100 // matches X axis bit
#define GCODE_WORD_J 0x0200 // matches Y axis bit
#define GCODE_WORD_K 0x0400 // matches Z axis bit
#define GCODE_WORD_L 0x0800
#define GCODE_WORD_P 0x1000
#define GCODE_WORD_R 0x2000
#define GCODE_WORD_S 0x4000
#define GCODE_WORD_T 0x8000

// only used in canned cycles and splines for now
// can overlap same memory position
#define GCODE_WORD_Q GCODE_WORD_D

#define GCODE_WORD_TOOL (1 << AXIS_TOOL)

	// H and Q are related to unsupported commands

	// #if (defined(AXIS_B) | defined(AXIS_C) | defined(GCODE_PROCESS_LINE_NUMBERS))
	// #define GCODE_WORDS_EXTENDED
	// #endif

#define GCODE_JOG_INVALID_WORDS (GCODE_WORD_I | GCODE_WORD_J | GCODE_WORD_K | GCODE_WORD_D | GCODE_WORD_L | GCODE_WORD_P | GCODE_WORD_R | GCODE_WORD_T | GCODE_WORD_S)
#define GCODE_ALL_AXIS (GCODE_WORD_X | GCODE_WORD_Y | GCODE_WORD_Z | GCODE_WORD_A | GCODE_WORD_B | GCODE_WORD_C)
#define GCODE_XYPLANE_AXIS (GCODE_WORD_X | GCODE_WORD_Y)
#define GCODE_XZPLANE_AXIS (GCODE_WORD_X | GCODE_WORD_Z)
#define GCODE_YZPLANE_AXIS (GCODE_WORD_Y | GCODE_WORD_Z)
#define GCODE_IJPLANE_AXIS (GCODE_XYPLANE_AXIS << 8)
#define GCODE_IKPLANE_AXIS (GCODE_XZPLANE_AXIS << 8)
#define GCODE_JKPLANE_AXIS (GCODE_YZPLANE_AXIS << 8)
#define GCODE_IJK_AXIS (GCODE_WORD_I | GCODE_WORD_J | GCODE_WORD_K)

#define LASER_PWM_MODE 1
#define LASER_PPI_MODE 2
#define LASER_PPI_VARPOWER_MODE 4

	// 34bytes in total
	typedef struct
	{
		// 1byte
		uint8_t motion;
		// 1byte
		uint8_t motion_mantissa : 3;
		uint8_t coord_system : 3;
		uint8_t : 2; // unused
		// 1byte
		uint8_t nonmodal : 4; // reset to 0 in every line (non persistent)
		uint8_t plane : 2;
		uint8_t path_mode : 2;
		// 1byte
		uint8_t cutter_radius_compensation : 2;
		uint8_t distance_mode : 1;
		uint8_t feedrate_mode : 1;
		uint8_t units : 1;
		uint8_t tlo_mode : 1;
		uint8_t return_mode : 1;
		uint8_t feed_speed_override : 1;
		// 2byte or 1byte
		uint8_t stopping : 3;
#if TOOL_COUNT == 1
		uint8_t tool_change : 1;
		uint8_t spindle_turning : 2;
		uint8_t coolant : 2;
#elif TOOL_COUNT > 1
	uint8_t tool_change : 5;
	uint8_t spindle_turning : 2;
	uint8_t coolant : 2;
	uint8_t : 4; // unused
#else
	uint8_t : 5; // unused
#endif
	} parser_groups_t;

	typedef struct
	{
		float tool_length_offset;
		uint8_t coord_system_index;
		float coord_system_offset[AXIS_COUNT];
		float g92_offset[AXIS_COUNT];
		float last_probe_position[AXIS_COUNT];
		uint8_t last_probe_ok;
	} parser_parameters_t;

	typedef struct
	{
		float xyzabc[AXIS_COUNT];
		float ijk[3];
		float d;
		float f;
		float p;
		float r;
		float s;
		int8_t t;
		uint8_t l;
#ifdef GCODE_PROCESS_LINE_NUMBERS
		uint32_t n;
#endif
	} parser_words_t;

	typedef struct
	{
		uint16_t groups;
		uint16_t words;
		int16_t group_extended : 15;
		uint8_t group_0_1_useaxis : 1;
	} parser_cmd_explicit_t;

	typedef struct
	{
		parser_groups_t groups;
		float feedrate;
#if TOOL_COUNT > 0
		uint8_t tool_index;
		uint16_t spindle;
#endif
#ifdef GCODE_PROCESS_LINE_NUMBERS
		uint32_t line;
#endif
	} parser_state_t;

	void parser_init(void);
	uint8_t parser_read_command(void);
	void parser_get_modes(uint8_t *modalgroups, uint16_t *feed, uint16_t *spindle, uint8_t *coolant);
	void parser_get_coordsys(uint8_t system_num, float *axis);
	bool parser_get_wco(float *axis);
	void parser_sync_probe(void);
	void parser_update_probe_pos(void);
	uint8_t parser_get_probe_result(void);
	void parser_parameters_load(void);
	void parser_parameters_reset(void);
	void parser_parameters_save(void);
	void parser_sync_position(void);
	void parser_reset(void);
	void parser_machine_to_work(float *axis);
	uint8_t parser_get_float(float *value);
	uint8_t parser_exec_command(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
#ifdef ENABLE_LASER_PPI
	void parser_config_ppi(void);
#endif

#ifdef ENABLE_PARSER_MODULES
	// generates a default delegate, event and handler hook
	typedef struct gcode_parse_args_
	{
		unsigned char word;
		uint8_t code;
		uint8_t error;
		float value;
		parser_state_t *new_state;
		parser_words_t *words;
		parser_cmd_explicit_t *cmd;
	} gcode_parse_args_t;
	// event_gcode_parse_handler
	DECL_EVENT_HANDLER(gcode_parse);

	typedef struct gcode_exec_args_
	{
		parser_state_t *new_state;
		parser_words_t *words;
		parser_cmd_explicit_t *cmd;
		float *target;
		motion_data_t *block_data;
	} gcode_exec_args_t;
	// event_gcode_exec_handler
	DECL_EVENT_HANDLER(gcode_exec);

	// event_gcode_exec_modifier_handler
	DECL_EVENT_HANDLER(gcode_exec_modifier);

	// event_grbl_cmd_handler
	typedef struct grbl_cmd_args_
	{
		unsigned char *cmd;
		uint8_t len;
		char next_char;
	} grbl_cmd_args_t;
	DECL_EVENT_HANDLER(grbl_cmd);

	// event_parse_token_handler
	DECL_EVENT_HANDLER(parse_token);
#endif

#ifdef __cplusplus
}
#endif

#endif
