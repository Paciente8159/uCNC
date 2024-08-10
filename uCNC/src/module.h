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

#define UCNC_MODULE_VERSION 10990

#define EVENT_CONTINUE false
#define EVENT_HANDLED true

// for now 8bits are enough. Might be expanded
#define LISTENER_NO_LOCK 0x00
#define LISTENER_RUNNING_LOCK 0x01
#define LISTENER_HWSPI_LOCK 0x02 // prevent multiple accesses to HWSPI by different modules
#define LISTENER_HWI2C_LOCK 0x04 // for future use
#define LISTENER_SWSPI_LOCK 0x20 // prevent multiple accesses to any software emulated SPI by different modules
#define LISTENER_SWI2C_LOCK 0x40 // prevent multiple accesses to any software emulated I2C by different modules
	// other locks might be added latter

	/// @brief this global variable keeps the global lock guards to shared resources
	extern uint8_t g_module_lockguard;

#define MODULE_LOCK_ENABLE(X) SETFLAG(g_module_lockguard, (X & ~LISTENER_RUNNING_LOCK))
#define MODULE_LOCK_DISABLE(X) CLEARFLAG(g_module_lockguard, (X & ~LISTENER_RUNNING_LOCK))

#define DECL_MODULE(name) void name##_init(void)
#define LOAD_MODULE(name)        \
	extern void name##_init(void); \
	name##_init()

// definitions to create events and event listeners
#define EVENT(name)                      \
	typedef struct name##_delegate_event_  \
	{                                      \
		name##_delegate fptr;                \
		uint8_t fplock;                      \
		struct name##_delegate_event_ *next; \
	} name##_delegate_event_t;             \
	extern name##_delegate_event_t *
#define EVENT_HANDLER_NAME(name) event_##name##_handler
#define EVENT_INVOKE(name, args) EVENT_HANDLER_NAME(name)(args)
#define CREATE_EVENT_LISTENER(name, handler) __attribute__((used)) name##_delegate_event_t name##_delegate_##handler = {&handler, LISTENER_NO_LOCK, NULL}
#define CREATE_EVENT_LISTENER_WITHLOCK(name, handler, lock_flags) __attribute__((used)) name##_delegate_event_t name##_delegate_##handler = {&handler, (lock_flags & (~LISTENER_RUNNING_LOCK)), NULL}
#define ADD_EVENT_LISTENER(name, handler)                     \
	{                                                           \
		extern name##_delegate_event_t name##_delegate_##handler; \
		if (name##_event == NULL)                                 \
		{                                                         \
			name##_event = &name##_delegate_##handler;              \
			name##_event->next = NULL;                              \
		}                                                         \
		else                                                      \
		{                                                         \
			name##_delegate_event_t *p = name##_event;              \
			while (p->next != NULL)                                 \
			{                                                       \
				p = p->next;                                          \
			}                                                       \
			p->next = &name##_delegate_##handler;                   \
			p->next->next = NULL;                                   \
		}                                                         \
	}

	// definitions to create overridable default handlers for functions with a declaration like uint8_t (*function)(void *, bool *);
	// the resulting handles is named event_<event name>_handler and can be placed inside any point in the core code
	// for example DECL_EVENT_HANDLER(<event name>) will create a function declaration equivalent to uint8_t event_<event name>_handler(void* args)
	// event_<event name>_handler can then be placed inside the core code to run the hook code
#define DECL_EVENT_HANDLER(name)           \
	typedef bool (*name##_delegate)(void *); \
	EVENT(name)                              \
	name##_event;                            \
	bool EVENT_HANDLER_NAME(name)(void *args)
#define WEAK_EVENT_HANDLER(name)         \
	name##_delegate_event_t *name##_event; \
	bool __attribute__((weak)) EVENT_HANDLER_NAME(name)(void *args)
#define OVERRIDE_EVENT_HANDLER(name) bool EVENT_HANDLER_NAME(name)(void *args)
// MODULE_DEBUG_ENABLED can be added to allow debuggin the default handler code
// this comes with a small performance penalty but also makes code about 1k~2k smaller by using a generic function
#ifndef MODULE_DEBUG_ENABLED
#define DEFAULT_EVENT_HANDLER(name)                                                                   \
	{                                                                                                   \
		static name##_delegate_event_t *start = NULL;                                                     \
		name##_delegate_event_t *ptr = start;                                                             \
		if (!ptr)                                                                                         \
		{                                                                                                 \
			ptr = name##_event;                                                                             \
		}                                                                                                 \
		while (ptr != NULL)                                                                               \
		{                                                                                                 \
			start = ptr->next;                                                                              \
			if (ptr->fptr != NULL && !CHECKFLAG(ptr->fplock, (g_module_lockguard | LISTENER_RUNNING_LOCK))) \
			{                                                                                               \
				SETFLAG(ptr->fplock, LISTENER_RUNNING_LOCK);                                                  \
				if (ptr->fptr(args))                                                                          \
				{                                                                                             \
					CLEARFLAG(ptr->fplock, LISTENER_RUNNING_LOCK);                                              \
					start = name##_event; /*handled. restart.*/                                                 \
					return EVENT_HANDLED;                                                                       \
				}                                                                                             \
				CLEARFLAG(ptr->fplock, LISTENER_RUNNING_LOCK);                                                \
			}                                                                                               \
			ptr = ptr->next;                                                                                \
		}                                                                                                 \
		return EVENT_CONTINUE;                                                                            \
	}
#else
typedef bool (*mod_delegate)(void *args);
typedef struct mod_delegate_event_
{
	mod_delegate fptr;
	uint8_t fplock;
	struct mod_delegate_event_ *next;
} mod_delegate_event_t;

bool mod_event_default_handler(mod_delegate_event_t **event, mod_delegate_event_t **last, void **args);

#define DEFAULT_EVENT_HANDLER(name)                                                                                               \
	{                                                                                                                               \
		static name##_delegate_event_t *last = NULL;                                                                                  \
		return mod_event_default_handler((mod_delegate_event_t **)(&name##_event), (mod_delegate_event_t **)(&last), (void **)&args); \
	}
#endif
	void mod_init(void);

// uses VARADIC MACRO available since C99
#define DECL_HOOK(name, ...)                      \
	typedef void (*name##_delegate_t)(__VA_ARGS__); \
	extern name##_delegate_t name##_cb
#define CREATE_HOOK(name) name##_delegate_t name##_cb
#define HOOK_ATTACH_CALLBACK(name, cb) name##_cb = &cb
#define HOOK_RELEASE(name) name##_cb = NULL

#define HOOK_INVOKE(name, ...) \
	if (name##_cb)               \
	{                            \
		name##_cb(__VA_ARGS__);    \
	}

#define RUNONCE                \
	static bool runonce = false; \
	if (!runonce)

#define RUNONCE_COMPLETE() runonce = true

#ifdef __cplusplus
}
#endif

#endif
