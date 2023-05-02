/*
	Name: system_menu.h
	Description: System menus for displays for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 20-04-2023

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef SYSTEM_MENU_H
#define SYSTEM_MENU_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../cnc.h"
#include "system_languages.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifndef SYSTEM_MENU_MAX_STR_LEN
#define SYSTEM_MENU_MAX_STR_LEN 32
#endif

#ifndef SYSTEM_MENU_GO_IDLE_MS
#define SYSTEM_MENU_GO_IDLE_MS 10000
#endif

#ifndef SYSTEM_MENU_REDRAW_STARTUP_MS
#define SYSTEM_MENU_REDRAW_STARTUP_MS 5000
#endif

#ifndef SYSTEM_MENU_REDRAW_IDLE_MS
#define SYSTEM_MENU_REDRAW_IDLE_MS 200
#endif

// render flags
#define SYSTEM_MENU_MODE_MODIFY 8
#define SYSTEM_MENU_MODE_EDIT 4
#define SYSTEM_MENU_MODE_SELECT 2
#define SYSTEM_MENU_MODE_REDRAW 1
#define SYSTEM_MENU_MODE_DISPLAY 0

#define SYSTEM_MENU_ACTION_NONE 0
#define SYSTEM_MENU_ACTION_SELECT 1
#define SYSTEM_MENU_ACTION_NEXT 2
#define SYSTEM_MENU_ACTION_PREV 3

#define VAR_TYPE_BOOLEAN 1
#define VAR_TYPE_UINT8 2
#define VAR_TYPE_INT8 3
#define VAR_TYPE_UINT16 4
#define VAR_TYPE_INT16 5
#define VAR_TYPE_UINT32 6
#define VAR_TYPE_INT32 7
#define VAR_TYPE_FLOAT 8

#define CONST_VARG(X) ((void *)X)
#if (__SIZEOF_POINTER__ == 2)
#define VARG_CONST(X) ((uint16_t)X)
#elif (__SIZEOF_POINTER__ == 4)
#define VARG_CONST(X) ((uint32_t)X)
#else
#define VARG_CONST(X) (X)
#endif

	// anonymous struct that is defined later
	typedef struct system_menu_item_ system_menu_item_t;
	typedef void (*system_menu_page_render_cb)(void);
	typedef bool (*system_menu_page_action_cb)(uint8_t);
	typedef void (*system_menu_item_render_cb)(uint8_t, system_menu_item_t *);
	typedef bool (*system_menu_item_action_cb)(uint8_t, void *);

	struct system_menu_item_
	{
		char *label;
		void *argptr;
		system_menu_item_render_cb item_render;
		void *render_arg;
		system_menu_item_action_cb item_action;
		void *action_arg;
	};

	typedef struct system_menu_index_
	{
		const system_menu_item_t *menu_item;
		struct system_menu_index_ *next;
	} system_menu_index_t;

	typedef struct system_menu_page_
	{
		uint8_t menu_id;
		uint8_t parent_id;
		const char *page_label;
		system_menu_page_render_cb page_render;
		system_menu_page_action_cb page_action;
		system_menu_index_t *items_index;
		struct system_menu_page_ *extended;
	} system_menu_page_t;

	typedef struct system_menu_
	{
		uint8_t flags;
		uint8_t current_menu;
		int16_t current_index;
		int8_t current_multiplier;
		uint8_t total_items;
		system_menu_page_t *menu_entry;
		// uint32_t next_redraw;
		uint32_t go_idle;
	} system_menu_t;

#define MENU_ENTRY(name) ((system_menu_item_t *)&name)
#define DECL_MENU_ENTRY(menu_id, name, strvalue, arg_ptr, display_cb, display_cb_arg, action_cb, action_cb_arg)                                                                                                   \
	static const system_menu_item_t name##_item __rom__ = {.label = strvalue, .argptr = arg_ptr, .item_render = display_cb, .render_arg = display_cb_arg, .item_action = action_cb, .action_arg = action_cb_arg}; \
	static system_menu_index_t name = {.menu_item = &name##_item, .next = NULL};                                                                                                                                  \
	system_menu_append_item(menu_id, &name)

/**
 * Helper macros
 * **/
#define DECL_MENU_LABEL(menu_id, name, strvalue) DECL_MENU_ENTRY(menu_id, name, strvalue, NULL, NULL, NULL, NULL, NULL)
#define DECL_MENU_GOTO(menu_id, name, strvalue, menu) DECL_MENU_ENTRY(menu_id, name, strvalue, NULL, NULL, "->", system_menu_action_goto, menu)
#define DECL_MENU_VAR(menu_id, name, strvalue, varptr, vartype, render_cb) DECL_MENU_ENTRY(menu_id, name, strvalue, varptr, render_cb, varptr, system_menu_action_edit, CONST_VARG(vartype))
#define DECL_MENU_ACTION(menu_id, name, strvalue, action_cb, action_cb_arg) DECL_MENU_ENTRY(menu_id, name, strvalue, NULL, NULL, NULL, action_cb, action_cb_arg)

#define DECL_MENU(id, parentid, label)                                                                                                                                                      \
	static const char m##id##_label[] __rom__ = label;                                                                                                                                      \
	static system_menu_page_t m##id = {.menu_id = id, .parent_id = parentid, .page_label = m##id##_label, .page_render = NULL, .page_action = NULL, .items_index = NULL, .extended = NULL}; \
	system_menu_append(&m##id)
#define DECL_DYNAMIC_MENU(id, parentid, render_cb, action_cb) static system_menu_page_t m##id = {.menu_id = id, .parent_id = parentid, .page_label = NULL, .page_render = render_cb, .page_action = action_cb, .items_index = NULL, .extended = NULL}
#define MENU(id) (&m##id)

#define MENU_LOOP(page, item) for (system_menu_page_t *item = page; item != NULL; item = item->extended)

	extern system_menu_t g_system_menu;

	DECL_MODULE(system_menu);
	void system_menu_reset(void);
	void system_menu_go_idle(void);
	void system_menu_action(uint8_t action);
	void system_menu_render(void);

	void system_menu_append(system_menu_page_t *newpage);
	void system_menu_append_item(uint8_t menu_id, system_menu_index_t *newitem);

	const system_menu_page_t *system_menu_get_current(void);
	const system_menu_item_t *system_menu_get_current_item(void);

	/**
	 * Overridable functions to be implemented for the display to render the system menu
	 * **/
	void system_menu_render_header(const char *__s);
	bool system_menu_render_menu_item_filter(uint8_t item_index);
	void system_menu_render_menu_item(uint8_t render_flags, const system_menu_item_t *item);
	void system_menu_render_nav_back(bool is_hover);
	void system_menu_render_footer(void);
	void system_menu_render_startup(void);
	void system_menu_render_idle(void);
	void system_menu_render_alarm(void);

	/**
	 * Helper µCNC action callbacks
	 * **/
	bool system_menu_action_goto(uint8_t action, void *cmd);
	bool system_menu_action_rt_cmd(uint8_t action, void *cmd);
	bool system_menu_action_serial_cmd(uint8_t action, void *cmd);
	bool system_menu_action_edit(uint8_t action, void *cmd);
	bool system_menu_action_nav_back(uint8_t action, void *cmd);

	/**
	 * Helper µCNC render callbacks
	 * **/
	void system_menu_item_render_label(uint8_t render_flags, const char *label);
	void system_menu_item_render_arg(uint8_t render_flags, const char *label);
	void system_menu_item_render_str_arg(uint8_t render_flags, system_menu_item_t *item);
	void system_menu_item_render_uint32_arg(uint8_t render_flags, system_menu_item_t *item);
	void system_menu_item_render_uint16_arg(uint8_t render_flags, system_menu_item_t *item);
	void system_menu_item_render_uint8_arg(uint8_t render_flags, system_menu_item_t *item);
	void system_menu_item_render_bool_arg(uint8_t render_flags, system_menu_item_t *item);
	void system_menu_item_render_flt_arg(uint8_t render_flags, system_menu_item_t *item);

	/**
	 * Helper µCNC to display variables
	 * **/

	extern char *system_menu_var_to_str_set_buffer_ptr;
	void system_menu_var_to_str_set_buffer(char *ptr);
	void system_menu_var_to_str(unsigned char c);

#define system_menu_int_to_str(buf_ptr, var)    \
	system_menu_var_to_str_set_buffer(buf_ptr); \
	print_int(system_menu_var_to_str, (uint32_t)var)
#define system_menu_flt_to_str(buf_ptr, var)    \
	system_menu_var_to_str_set_buffer(buf_ptr); \
	print_flt(system_menu_var_to_str, (float)var)

#ifdef __cplusplus
}
#endif

#endif
