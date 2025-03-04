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

// if no action go to idle screen after 10s
#ifndef SYSTEM_MENU_GO_IDLE_MS
#define SYSTEM_MENU_GO_IDLE_MS 10000
#endif

// show startup screen for 5s
#ifndef SYSTEM_MENU_REDRAW_STARTUP_MS
#define SYSTEM_MENU_REDRAW_STARTUP_MS 5000
#endif

// show modal popup screen for 2s
#ifndef SYSTEM_MENU_MODAL_POPUP_MS
#define SYSTEM_MENU_MODAL_POPUP_MS 2000
#endif

// refresh idle screen every 0.2s
#ifndef SYSTEM_MENU_REDRAW_IDLE_MS
#define SYSTEM_MENU_REDRAW_IDLE_MS 200
#endif

// render flags
#define SYSTEM_MENU_MODE_MODAL_POPUP 64
#define SYSTEM_MENU_MODE_DELAYED_REDRAW 32
#define SYSTEM_MENU_MODE_MODIFY 16
#define SYSTEM_MENU_MODE_EDIT 8
#define SYSTEM_MENU_MODE_SIMPLE_EDIT 4
#define SYSTEM_MENU_MODE_SELECT 2
#define SYSTEM_MENU_MODE_REDRAW 1
#define SYSTEM_MENU_MODE_NONE 0

// System menu IDs
#define SYSTEM_MENU_ID_STARTUP 255
#define SYSTEM_MENU_ID_IDLE 0
#define SYSTEM_MENU_ID_MAIN_MENU 1
#define SYSTEM_MENU_ID_SETTINGS 2
#define SYSTEM_MENU_ID_JOG 7
#define SYSTEM_MENU_ID_OVERRIDES 8

#define SYSTEM_MENU_ACTION_NONE 0
#define SYSTEM_MENU_ACTION_SELECT 1
#define SYSTEM_MENU_ACTION_NEXT 2
#define SYSTEM_MENU_ACTION_PREV 3

// Characters given to this macro should be in range of printable ASCII characters (32-127)
#define SYSTEM_MENU_ACTION_CHAR_INPUT(c) (c)
// Action ID range from 128 - 137
#define SYSTEM_MENU_ACTION_SIDE_BUTTON(b) ((b) + 128)
// Action ID range from 138 - 147
#define SYSTEM_MENU_ACTION_SPECIAL_BUTTON(b) ((b) + 138)

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
	typedef void (*system_menu_page_render_cb)(uint8_t);
	typedef bool (*system_menu_page_action_cb)(uint8_t);
	typedef void (*system_menu_item_render_cb)(uint8_t, system_menu_item_t *);
	typedef bool (*system_menu_item_action_cb)(uint8_t, system_menu_item_t *);

	struct system_menu_item_
	{
		const char *label;
		void *argptr;
		system_menu_item_render_cb item_render;
		const void *render_arg;
		system_menu_item_action_cb item_action;
		const void *action_arg;
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
		uint32_t action_timeout;
	} system_menu_t;

#define MENU_ENTRY(name) ((system_menu_item_t *)&name)
#define DECL_MENU_ENTRY(menu_id, name, strvalue, arg_ptr, display_cb, display_cb_arg, action_cb, action_cb_arg)                                                                                                 \
	static const system_menu_item_t name##_item __rom__ = {.label = strvalue, .argptr = arg_ptr, .item_render = display_cb, .render_arg = display_cb_arg, .item_action = action_cb, .action_arg = action_cb_arg}; \
	static system_menu_index_t name = {.menu_item = &name##_item, .next = NULL};                                                                                                                                  \
	system_menu_append_item(menu_id, &name)

/**
 * Helper macros
 * **/
#define DECL_MENU_LABEL(menu_id, name, strvalue) DECL_MENU_ENTRY(menu_id, name, strvalue, NULL, NULL, NULL, NULL, NULL)
#define DECL_MENU_GOTO(menu_id, name, strvalue, menu) DECL_MENU_ENTRY(menu_id, name, strvalue, NULL, NULL, "->", system_menu_action_goto, menu)
#define DECL_MENU_VAR(menu_id, name, strvalue, varptr, vartype) DECL_MENU_ENTRY(menu_id, name, strvalue, varptr, system_menu_item_render_var_arg, CONST_VARG(vartype), system_menu_action_edit, CONST_VARG(vartype))
#define DECL_MENU_VAR_SIMPLE(menu_id, name, strvalue, varptr, vartype) DECL_MENU_ENTRY(menu_id, name, strvalue, varptr, system_menu_item_render_var_arg, CONST_VARG(vartype), system_menu_action_edit_simple, CONST_VARG(vartype))
#define DECL_MENU_VAR_CUSTOM_EDIT(menu_id, name, strvalue, varptr, vartype, actioncb, actioncb_arg) DECL_MENU_ENTRY(menu_id, name, strvalue, varptr, system_menu_item_render_var_arg, CONST_VARG(vartype), actioncb, actioncb_arg)
#define DECL_MENU_ACTION(menu_id, name, strvalue, action_cb, action_cb_arg) DECL_MENU_ENTRY(menu_id, name, strvalue, NULL, NULL, NULL, action_cb, action_cb_arg)

#define DECL_MENU(id, parentid, label)                                                                                                                                                    \
	static const char m##id##_label[] __rom__ = label;                                                                                                                                      \
	static system_menu_page_t m##id = {.menu_id = id, .parent_id = parentid, .page_label = m##id##_label, .page_render = NULL, .page_action = NULL, .items_index = NULL, .extended = NULL}; \
	system_menu_append(&m##id)
#define DECL_DYNAMIC_MENU(id, parentid, render_cb, action_cb)                                                                                                                              \
	static system_menu_page_t m##id = {.menu_id = id, .parent_id = parentid, .page_label = NULL, .page_render = render_cb, .page_action = action_cb, .items_index = NULL, .extended = NULL}; \
	system_menu_append(&m##id)
#define MENU(id) (&m##id)

#define MENU_LOOP(page, item) for (system_menu_page_t *item = page; item != NULL; item = item->extended)

	extern system_menu_t g_system_menu;

	extern float g_system_menu_jog_distance;
	extern float g_system_menu_jog_feed;

	DECL_MODULE(system_menu);
	void system_menu_reset(void);
	void system_menu_go_idle(void);
	void system_menu_action(uint8_t action);
	void system_menu_render(void);
	void system_menu_show_modal_popup(uint32_t timeout, const char *__s);
	void system_menu_action_timeout(uint32_t delay);
	void system_menu_goto(uint8_t id);

	void system_menu_set_render_callback(uint8_t menu_id, system_menu_page_render_cb callback);
	void system_menu_set_action_callback(uint8_t menu_id, system_menu_page_action_cb callback);

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
	void system_menu_render_modal_popup(const char *__s);
	// this needs to be implemented using a serial stream
	uint8_t system_menu_send_cmd(const char *__s);

	/**
	 * Overridable system menu actions to be implemented for the user input system
	 * **/

	void system_menu_action_custom_code(uint8_t action);

	/**
	 * Helper µCNC action callbacks
	 * **/
	bool system_menu_action_goto(uint8_t action, system_menu_item_t *item);
	bool system_menu_action_rt_cmd(uint8_t action, system_menu_item_t *item);
	bool system_menu_action_serial_cmd(uint8_t action, system_menu_item_t *item);
	bool system_menu_action_edit(uint8_t action, system_menu_item_t *item);
	bool system_menu_action_edit_simple(uint8_t action, system_menu_item_t *item);

	/**
	 * Helper µCNC render callbacks
	 * **/
	// these should be implemented on the display side
	// one to render label and the other to render the label variable argument
	void system_menu_item_render_label(uint8_t render_flags, const char *label);
	void system_menu_item_render_arg(uint8_t render_flags, const char *label);
	// generic menu item argument renderer
	void system_menu_item_render_var_arg(uint8_t render_flags, system_menu_item_t *item);

	/**
	 * Helper µCNC to display variables
	 * **/
#define system_menu_int_to_str(buf_ptr, var) ({ char* _ptr = (buf_ptr); char** ptr_ptr = &_ptr; prt_int(ptr_ptr, PRINT_MAX, (uint32_t)var, 0); })
#define system_menu_flt_to_str(buf_ptr, var) ({ char* _ptr = (buf_ptr); char** ptr_ptr = &_ptr; prt_flt(ptr_ptr, PRINT_MAX, (float)var, ((!g_settings.report_inches) ? 3 : 5)); })

#ifdef __cplusplus
}
#endif

#endif
