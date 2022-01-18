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

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

	//33bytes in total
	typedef struct
	{
		//1byte
		uint8_t motion : 5;
		uint8_t coord_system : 3;
		//1byte
		uint8_t nonmodal : 4; //reset to 0 in every line (non persistent)
		uint8_t plane : 2;
		uint8_t path_mode : 2;
		//1byte
		uint8_t cutter_radius_compensation : 2;
		uint8_t distance_mode : 1;
		uint8_t feedrate_mode : 1;
		uint8_t units : 1;
		uint8_t tlo_mode : 1;
		uint8_t return_mode : 1;
		uint8_t feed_speed_override : 1;
		//1byte
#if TOOL_COUNT > 1
		uint8_t tool_change : 5;
#else
	uint8_t tool_change : 1;
#endif
		uint8_t stopping : 3;
#ifdef USE_SPINDLE
		uint8_t spindle_turning : 2;
#else
	uint8_t : 2; //unused
#endif
#ifdef USE_COOLANT
		uint8_t coolant : 2;
#else
	uint8_t : 2; //unused
#endif
		uint8_t : 4; //unused
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
#ifdef GCODE_PROCESS_LINE_NUMBERS
		uint32_t n;
#endif
#ifdef USE_SPINDLE
		int16_t s;
#endif
		int8_t t;
		uint8_t l;
	} parser_words_t;

	typedef struct
	{
		uint16_t groups;
		uint16_t words;
		uint8_t group_0_1_useaxis : 1;
#ifdef ENABLE_PARSER_EXTENSIONS
		uint8_t group_extended : 7;
#endif
	} parser_cmd_explicit_t;

	typedef struct
	{
		parser_groups_t groups;
		float feedrate;
#if TOOL_COUNT > 0
		uint8_t tool_index;
#endif
#ifdef USE_SPINDLE
		int16_t spindle;
#endif
#ifdef GCODE_PROCESS_LINE_NUMBERS
		uint32_t line;
#endif
	} parser_state_t;

#ifdef ENABLE_PARSER_EXTENSIONS
	typedef uint8_t (*parser_extender_callback)(unsigned char, uint8_t, uint8_t, float, parser_state_t *, parser_words_t *, parser_cmd_explicit_t *, void *);
	typedef uint8_t (*parser_extender_exec_callback)(parser_state_t *, parser_words_t *, parser_cmd_explicit_t *, void *);
	typedef struct parser_extender_
	{
		parser_extender_callback parse_word;
		parser_extender_exec_callback execute;
		struct parser_extender_ *next;
	} parser_extender_t;

#endif

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
	uint8_t parser_exec_command(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
//parser extender functions
#ifdef ENABLE_PARSER_EXTENSIONS
	void parser_register_extender(parser_extender_t *new_extender);
#endif

#ifdef __cplusplus
}
#endif

#endif
