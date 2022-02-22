/*
	Name: module.c
	Description: Module extensions for µCNC.
	All entry points for extending µCNC core functionalities are declared in this module

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 21-02-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "cnc.h"

#define MODULE_CALLBACK __attribute__((weak))

// this is the place to declare all parser extension registration calls
#ifdef ENABLE_PARSER_MODULES
uint8_t MODULE_CALLBACK mod_gcode_parse_hook(unsigned char word, uint8_t code, uint8_t error, float value, parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
	EVENT_TYPE(gcode_parse_delegate) *ptr = gcode_parse_event;
	while ((error == STATUS_GCODE_UNSUPPORTED_COMMAND || error == STATUS_GCODE_UNUSED_WORDS) && (ptr != NULL))
	{
		if (ptr->fptr != NULL)
		{
			error = ptr->fptr(word, code, error, value, new_state, words, cmd);
		}

		ptr = ptr->next;
	}

	return error;
}

uint8_t MODULE_CALLBACK mod_gcode_exec_hook(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
	EVENT_TYPE(gcode_exec_delegate) *ptr = gcode_exec_event;
	uint8_t error = STATUS_GOCDE_EXTENDED_UNSUPPORTED;
	while ((cmd->group_extended != 0) && (ptr != NULL))
	{
		if (ptr->fptr != NULL)
		{
			error = ptr->fptr(new_state, words, cmd);
		}

		// checks if function catched the extended code
		if (error != STATUS_GOCDE_EXTENDED_UNSUPPORTED)
		{
			break;
		}

		ptr = ptr->next;
	}

	if ((cmd->group_extended != 0))
	{
		return error;
	}
}
#endif

#ifdef ENABLE_SCHEDULER_LOOP_MODULES
void MODULE_CALLBACK mod_rtc_tick_hook(void)
{
}
#endif

#ifdef ENABLE_IO_MODULES
void MODULE_CALLBACK mod_input_change_hook(void)
{
}
#endif

void mod_init(void)
{
	// initializes PID module
#if PID_CONTROLLERS > 0
	pid_init(); // pid
#endif

// initializes parser extension modules
#ifdef ENABLE_PARSER_MODULES
	ADD_LISTENER(gcode_parse_delegate, m42_parse, gcode_parse_event);
	ADD_LISTENER(gcode_exec_delegate, m42_exec, gcode_exec_event);
#endif
}
