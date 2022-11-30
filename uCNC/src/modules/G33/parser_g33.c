/*
	Name: parser_g33.c
	Description: Implements a parser extension for LinuxCNC G33 for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 25/11/2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef ENABLE_SETTINGS_MODULES
static float tool_at_speed_tolerance;
#endif

#ifdef ENABLE_PARSER_MODULES

#ifndef UCNC_MODULE_VERSION_1_5_0_PLUS
#error "This module is not compatible with the current version of µCNC"
#endif

// this ID must be unique for each code
#define G33 33

uint8_t g33_parse(void *args, bool *handled);
uint8_t g33_exec(void *args, bool *handled);

CREATE_EVENT_LISTENER(gcode_parse, g33_parse);
CREATE_EVENT_LISTENER(gcode_exec, g33_exec);

// this just parses and accepts the code
uint8_t g33_parse(void *args, bool *handled)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;
	if (ptr->word == 'G' && ptr->code == 33)
	{
		// stops event propagation
		*handled = true;
		if (ptr->cmd->group_extended != 0 || CHECKFLAG(ptr->cmd->groups, GCODE_GROUP_MOTION))
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			return STATUS_GCODE_MODAL_GROUP_VIOLATION;
		}
		// checks if it's G5 or G5.1
		// check mantissa
		uint8_t mantissa = (uint8_t)lroundf(((ptr->value - ptr->code) * 100.0f));

		if (mantissa != 0)
		{
			return STATUS_GCODE_UNSUPPORTED_COMMAND;
		}

		ptr->new_state->groups.motion = G33;
		ptr->new_state->groups.motion_mantissa = 0;
		SETFLAG(ptr->cmd->groups, GCODE_GROUP_MOTION);
		ptr->cmd->group_extended = EXTENDED_MOTION_GCODE(33);
		return STATUS_OK;
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return ptr->error;
}

// this actually performs 2 steps in 1 (validation and execution)
uint8_t g33_exec(void *args, bool *handled)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;
	if (ptr->cmd->group_extended == EXTENDED_MOTION_GCODE(33))
	{
		// stops event propagation
		*handled = true;

		if (!CHECKFLAG(ptr->cmd->words, GCODE_XYZ_AXIS))
		{
			// it's an error no axis word is specified
			return STATUS_GCODE_NO_AXIS_WORDS;
		}

		if (!CHECKFLAG(ptr->cmd->words, GCODE_WORD_K))
		{
			// it's an error no distance per rev word is specified
			return STATUS_GCODE_VALUE_WORD_MISSING;
		}

		// syncs motions and sets spindle
		if (mc_update_tools(ptr->block_data) != STATUS_OK)
		{
			return STATUS_CRITICAL_FAIL;
		}

		// wait for spindle to reach the desired speed
		uint16_t programmed_speed = planner_get_spindle_speed(1);
		uint16_t at_speed_threshold = lroundf(tool_at_speed_tolerance * programmed_speed);

		// wait for tool at speed
		while (ABS(programmed_speed - tool_get_speed()) > at_speed_threshold)
			;

		ptr->block_data->motion_flags.bit.synched = 1;

		// queue motion in planner
		if (mc_line(ptr->target, ptr->block_data) != STATUS_OK)
		{
			return STATUS_CRITICAL_FAIL;
		}

		while (itp_sync() == STATUS_OK)
		{
			
		}
	}

	return STATUS_GCODE_EXTENDED_UNSUPPORTED;
}

#endif

DECL_MODULE(g33)
{
#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(gcode_parse, g33_parse);
	ADD_EVENT_LISTENER(gcode_exec, g33_exec);
#else
#error "Parser extensions are not enabled. G33 code extension will not work."
#endif
#ifndef ENABLE_IO_MODULES
#error "IO extensions are not enabled. G33 code extension will not work."
#endif
#ifndef ENABLE_MAIN_LOOP_MODULES
#error "Main loop extensions are not enabled. G33 code extension will not work."
#endif
}
