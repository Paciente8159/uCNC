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

#include <stdint.h>
#include <stdbool.h>

#define UCNC_MODULE_VERSION 010800

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
	extern name##_delegate_event_t *
// #define EVENT_TYPE(name) name##_delegate_event_t
#define EVENT_INVOKE(name, args) event_##name##_handler(args)
#define CREATE_EVENT_LISTENER(name, handler) __attribute__((used)) name##_delegate_event_t name##_delegate_##handler = {&handler, NULL}
#define ADD_EVENT_LISTENER(name, handler)                         \
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

	// definitions to create overridable default handlers for functions with a declaration like uint8_t (*function)(void *, bool *);
	// the resulting handles is named event_<event name>_handler and can be placed inside any point in the core code
	// for example DECL_EVENT_HANDLER(<event name>) will create a function declaration equivalent to uint8_t event_<event name>_handler(void* args)
	// event_<event name>_handler can then be placed inside the core code to run the hook code

#define DECL_EVENT_HANDLER(name)             \
	typedef bool (*name##_delegate)(void *); \
	EVENT(name)                              \
	name##_event;                            \
	bool event_##name##_handler(void *args)
#define WEAK_EVENT_HANDLER(name)           \
	name##_delegate_event_t *name##_event; \
	bool __attribute__((weak)) event_##name##_handler(void *args)
#define OVERRIDE_EVENT_HANDLER(name) bool event_##name##_handler(void *args)
#define DEFAULT_EVENT_HANDLER(name)                  \
	{                                                \
		name##_delegate_event_t *ptr = name##_event; \
		while (ptr != NULL)                          \
		{                                            \
			if (ptr->fptr != NULL)                   \
			{                                        \
				if (ptr->fptr(args))                 \
				{                                    \
					return true;                     \
				}                                    \
			}                                        \
			ptr = ptr->next;                         \
		}                                            \
		return false;                                \
	}

	void mod_init(void);

#ifdef __cplusplus
}
#endif

#endif
