/*
	Name: module.h
	Description: Module extensions for µCNC.
	All entry points for extending µCNC core functionalities are declared in this module.

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

#ifndef MODULE_H
#define MODULE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../cnc_config.h"
#include "core/parser.h"
#include <stdint.h>
#include <stdbool.h>

#define UCNC_MODULE_VERSION_1_5_0_PLUS

#define EVENT_CONTINUE false
#define EVENT_HANDLED true

#define DECL_MODULE(modulename) void modulename##_init(void)
#define LOAD_MODULE(modulename)          \
	extern void modulename##_init(void); \
	modulename##_init()

// definitions to create events and event listeners
#define EVENT(delegate)                 \
	typedef struct delegate##_event_    \
	{                                   \
		delegate fptr;                  \
		struct delegate##_event_ *next; \
	} delegate##_event_t;               \
	delegate##_event_t *
#define EVENT_TYPE(delegate) delegate##_event_t
#define EVENT_INVOKE(event, args) mod_##event##_hook(args)
#define CREATE_LISTENER(delegate, handler) __attribute__((used)) delegate##_event_t delegate##_##handler = {&handler, NULL}
#define ADD_LISTENER(delegate, handler, event)          \
	{                                                   \
		extern delegate##_event_t delegate##_##handler; \
		if (event == NULL)                              \
		{                                               \
			event = &delegate##_##handler;              \
			event->next = NULL;                         \
		}                                               \
		else                                            \
		{                                               \
			delegate##_event_t *p = event;              \
			while (p->next != NULL)                     \
			{                                           \
				p = p->next;                            \
			}                                           \
			p->next = &delegate##_##handler;            \
			p->next->next = NULL;                       \
		}                                               \
	}

	// definitions to create overridable default handlers for functions void-void hooks
	// the resulting handles is named mod_<hookname>_hook and can be placed inside any point in the core code
	// for example DECL_HOOK(do_stuff) will create a function declaration equivalent to void mod_do_stuff_hook(void)
	// mod_do_stuff_hook can then be placed inside the core code to run the hook code

#define DECL_HOOK(hook)                                 \
	typedef uint8_t (*hook##_delegate)(void *, bool *); \
	EVENT(hook##_delegate)                              \
	hook##_event;                                       \
	uint8_t mod_##hook##_hook(void *args)
#define WEAK_HOOK(hook) uint8_t __attribute__((weak)) mod_##hook##_hook(void *args)
#define DEFAULT_HANDLER(hook)                              \
	{                                                      \
		EVENT_TYPE(hook##_delegate) *ptr = hook##_event;   \
		bool handled = EVENT_CONTINUE;                     \
		uint8_t result = 0;                                \
		while (ptr != NULL && (handled == EVENT_CONTINUE)) \
		{                                                  \
			if (ptr->fptr != NULL)                         \
			{                                              \
				result = ptr->fptr(args, &handled);        \
			}                                              \
			ptr = ptr->next;                               \
		}                                                  \
                                                           \
		return result;                                     \
	}

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
	// mod_gcode_parse_hook
	DECL_HOOK(gcode_parse);

	typedef struct gcode_exec_args_
	{
		parser_state_t *new_state;
		parser_words_t *words;
		parser_cmd_explicit_t *cmd;
	} gcode_exec_args_t;
	// mod_gcode_exec_hook
	DECL_HOOK(gcode_exec);
#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
	// generates a default delegate, event and handler hook
	// mod_cnc_reset_hook
	DECL_HOOK(cnc_reset);
	// mod_rtc_tick_hook
	DECL_HOOK(rtc_tick);
	// mod_cnc_dotasks_hook
	DECL_HOOK(cnc_dotasks);
	// mod_cnc_stop_hook
	DECL_HOOK(cnc_stop);
#endif

#ifdef ENABLE_INTERPOLATOR_MODULES
	// mod_itp_reset_rt_position_hook
	DECL_HOOK(itp_reset_rt_position);
#endif

#ifdef ENABLE_SETTINGS_MODULES
	// mod_settings_change_hook
	DECL_HOOK(settings_change);
#endif

#ifdef ENABLE_PROTOCOL_MODULES
	// mod_send_pins_states_hook
	DECL_HOOK(send_pins_states);
#endif

#ifdef ENABLE_IO_MODULES
	// mod_input_change_hook
	DECL_HOOK(input_change);
#endif

#ifdef ENABLE_MOTION_MODULES
	// mod_probe_enable_hook
	DECL_HOOK(probe_enable);
	// mod_probe_disable_hook
	DECL_HOOK(probe_disable);
#endif

	void mod_init(void);

#ifdef __cplusplus
}
#endif

#endif
