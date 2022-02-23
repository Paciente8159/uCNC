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

// this is the place to declare all parser extension registration calls
#ifdef ENABLE_PARSER_MODULES
// overridable handler
uint8_t __attribute__((weak)) mod_gcode_parse_hook(unsigned char word, uint8_t code, uint8_t error, float value, parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
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

// overridable handler
uint8_t __attribute__((weak)) mod_gcode_exec_hook(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
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

#ifdef ENABLE_INTERPOLATOR_MODULES

// declares the handler hook to be called inside the parser core
void __attribute__((weak)) mod_itp_reset_rt_position_hook(float *origin)
{
	EVENT_TYPE(itp_reset_rt_position_delegate) *ptr = itp_reset_rt_position_event;
	while (ptr != NULL)
	{
		if (ptr->fptr != NULL)
		{
			ptr->fptr(origin);
		}
		ptr = ptr->next;
	}
}
#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
// mod_cnc_reset_hook
WEAK_HOOK(cnc_reset)
{
	// for now only encoder module uses this hook and overrides it
	// DEFAULT_HANDLER(cnc_reset);
}

// mod_rtc_tick_hook
WEAK_HOOK(rtc_tick)
{
	// for now only pid module uses this hook and overrides it
	// DEFAULT_HANDLER(rtc_tick);
}

// mod_cnc_dotasks_hook
WEAK_HOOK(cnc_dotasks)
{
	// for now this is not used
	//  DEFAULT_HANDLER(cnc_dotasks);
}

// mod_cnc_stop_hook
WEAK_HOOK(cnc_stop)
{
	// for now only pid module uses this hook and overrides it
	// DEFAULT_HANDLER(cnc_stop);
}

#endif

#ifdef ENABLE_SETTINGS_MODULES
// mod_settings_change_hook
WEAK_HOOK(settings_change)
{
	// for now only pid module uses this hook and overrides it
	// DEFAULT_HANDLER(settings_change);
}
#endif

#ifdef ENABLE_PROTOCOL_MODULES
// mod_send_pins_states_hook
WEAK_HOOK(send_pins_states)
{
	// for now only encoder module uses this hook and overrides it
	// it actually overrides the mcu callback to be faster
	// DEFAULT_HANDLER(input_change);
}
#endif

#ifdef ENABLE_IO_MODULES
// mod_input_change_hook
WEAK_HOOK(input_change)
{
	// for now only encoder module uses this hook and overrides it
	// it actually overrides the mcu callback to be faster
	// DEFAULT_HANDLER(input_change);
}

// mod_probe_enable_hook
WEAK_HOOK(probe_enable)
{
	// for now this is not used
}

// mod_probe_disable_hook
WEAK_HOOK(probe_disable)
{
	// for now this is not used
}
#endif

#if PID_CONTROLLERS > 0
extern void pid_init(void);
#endif

void mod_init(void)
{
	// initializes PID module
#if PID_CONTROLLERS > 0
	pid_init(); // pid
#endif

// add listeners here
#ifdef ENABLE_PARSER_MODULES
	ADD_LISTENER(gcode_parse_delegate, m42_parse, gcode_parse_event);
	ADD_LISTENER(gcode_exec_delegate, m42_exec, gcode_exec_event);
#endif
}
