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

#define DECL_MODULE(name) void name##_init(void)
#define LOAD_MODULE(name)          \
	extern void name##_init(void); \
	name##_init()

// definitions to create events and event listeners
#define EVENT(name)                          \
	typedef struct name##_delegate_event_    \
	{                                        \
		name##_delegate fptr;                \
		struct name##_delegate_event_ *next; \
	} name##_delegate_event_t;               \
	name##_delegate_event_t *
// #define EVENT_TYPE(name) name##_delegate_event_t
#define EVENT_INVOKE(name, args) event_##name##_handler(args)
#define CREATE_EVENT_LISTENER(name, handler) __attribute__((used)) name##_delegate_event_t name##_delegate_##handler = {&handler, NULL}
#define ADD_EVENT_LISTENER(name, handler)                    \
	{                                                             \
		extern name##_delegate_event_t name##_delegate_##handler; \
		if (name##_event == NULL)                                 \
		{                                                         \
			name##_event = &name##_delegate_##handler;            \
			name##_event->next = NULL;                            \
		}                                                         \
		else                                                      \
		{                                                         \
			name##_delegate_event_t *p = name##_event;            \
			while (p->next != NULL)                               \
			{                                                     \
				p = p->next;                                      \
			}                                                     \
			p->next = &name##_delegate_##handler;                 \
			p->next->next = NULL;                                 \
		}                                                         \
	}

	// definitions to create overridable default handlers for functions void-void hooks
	// the resulting handles is named mod_<hookname>_hook and can be placed inside any point in the core code
	// for example DECL_EVENT_HANDLER(do_stuff) will create a function declaration equivalent to void mod_do_stuff_hook(void)
	// mod_do_stuff_hook can then be placed inside the core code to run the hook code

#define DECL_EVENT_HANDLER(name)                                 \
	typedef uint8_t (*name##_delegate)(void *, bool *); \
	EVENT(name)                              \
	name##_event;                                       \
	uint8_t event_##name##_handler(void *args)
#define WEAK_EVENT_HANDLER(name) uint8_t __attribute__((weak)) event_##name##_handler(void *args)
#define DEFAULT_EVENT_HANDLER(name)                              \
	{                                                      \
		name##_delegate_event_t *ptr = name##_event;   \
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
	DECL_EVENT_HANDLER(gcode_parse);

	typedef struct gcode_exec_args_
	{
		parser_state_t *new_state;
		parser_words_t *words;
		parser_cmd_explicit_t *cmd;
	} gcode_exec_args_t;
	// mod_gcode_exec_hook
	DECL_EVENT_HANDLER(gcode_exec);
#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
	// generates a default delegate, event and handler hook
	// mod_cnc_reset_hook
	DECL_EVENT_HANDLER(cnc_reset);
	// mod_rtc_tick_hook
	DECL_EVENT_HANDLER(rtc_tick);
	// mod_cnc_dotasks_hook
	DECL_EVENT_HANDLER(cnc_dotasks);
	// mod_cnc_stop_hook
	DECL_EVENT_HANDLER(cnc_stop);
#endif

#ifdef ENABLE_INTERPOLATOR_MODULES
	// mod_itp_reset_rt_position_hook
	DECL_EVENT_HANDLER(itp_reset_rt_position);
#endif

#ifdef ENABLE_SETTINGS_MODULES
	// mod_settings_change_hook
	DECL_EVENT_HANDLER(settings_change);
#endif

#ifdef ENABLE_PROTOCOL_MODULES
	// mod_send_pins_states_hook
	DECL_EVENT_HANDLER(send_pins_states);
#endif

#ifdef ENABLE_IO_MODULES
	// mod_input_change_hook
	DECL_EVENT_HANDLER(input_change);
#endif

#ifdef ENABLE_MOTION_MODULES
	// mod_probe_enable_hook
	DECL_EVENT_HANDLER(probe_enable);
	// mod_probe_disable_hook
	DECL_EVENT_HANDLER(probe_disable);
#endif

	void mod_init(void);

#ifdef __cplusplus
}
#endif

#endif
