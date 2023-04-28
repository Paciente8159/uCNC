/*
	Name: system_menu.c
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

#include "system_menu.h"

#ifdef ENABLE_SYSTEM_MENU

system_menu_t g_system_menu;

static void system_menu_redraw_timeout(uint32_t delay);
static const system_menu_page_t *system_menu_get(uint8_t menu_id);
static const system_menu_item_t *system_menu_get_item(uint8_t menu_id, int16_t index);
static uint8_t system_menu_get_item_count(uint8_t menu_id);
static void system_menu_redraw_timeout(uint32_t delay);

// declarate startup screen
static void system_menu_startup(void)
{
	system_menu_render_startup();
}

DECL_DYNAMIC_MENU(255, 0, system_menu_startup, NULL);

// declarate idle screen
static void system_menu_idle(void)
{
	system_menu_render_idle();
	system_menu_redraw_timeout(SYSTEM_MENU_REDRAW_IDLE_MS);
}
static uint8_t system_menu_main_open(uint8_t action)
{
	return system_menu_action_goto(SYSTEM_MENU_ACTION_SELECT, CONST_VARG(1));
}

DECL_DYNAMIC_MENU(0, 0, system_menu_idle, system_menu_main_open);

// Declarate remaining menus
DECL_MENU(1, 0, "Main menu");
DECL_MENU(2, 1, "Settings");

DECL_MODULE(system_menu)
{
	// entry menu to startup screen
	g_system_menu.menu_entry = MENU(255);

	// append idle menu
	system_menu_append(MENU(0));

	// append main
	system_menu_append(MENU(1));

	// main menu entries
	DECL_MENU_ACTION(1, hold, "Hold", &system_menu_action_rt_cmd, CONST_VARG(CMD_CODE_FEED_HOLD));
	DECL_MENU_ACTION(1, resume, "Resume", &system_menu_action_rt_cmd, CONST_VARG(CMD_CODE_CYCLE_START));
	DECL_MENU_ACTION(1, home, "Home", &system_menu_action_serial_cmd, "$H");
	DECL_MENU_GOTO(1, settings, "Settings", CONST_VARG(2));

	// append settings menu
	system_menu_append(MENU(2));

	// settings menu
	DECL_MENU_VAR(2, s3, "Dir inv:", &g_settings.dir_invert_mask, system_menu_item_render_uint8_arg);
	DECL_MENU_VAR(2, s4, "Enable inv:", &g_settings.step_enable_invert, system_menu_item_render_uint8_arg);
	DECL_MENU_VAR(2, s5, "Limits inv:", &g_settings.limits_invert_mask, system_menu_item_render_uint8_arg);
	DECL_MENU_VAR(2, s6, "Probe inv:", &g_settings.probe_invert_mask, system_menu_item_render_bool_arg);
	DECL_MENU_VAR(2, s7, "Control inv:", &g_settings.control_invert_mask, system_menu_item_render_uint8_arg);
	DECL_MENU_VAR(2, s20, "Soft-limits:", &g_settings.soft_limits_enabled, system_menu_item_render_bool_arg);
	DECL_MENU_VAR(2, s2, "Steps/mm X:", &g_settings.step_per_mm[0], system_menu_item_render_flt_arg);

	// reset system menu
	system_menu_reset();
}

/**
 *
 * system_menu_action and system_menu_render are the two primary functions to be executed in the display's loop
 * always call system_menu_action with the user action (or no action) followed by system_menu_render to update the
 * display if needed
 *
 * **/
void system_menu_action(uint8_t action)
{
	int8_t currentmenu = (int8_t)g_system_menu.current_menu;
	int16_t currentindex = g_system_menu.current_index;

	if (action == SYSTEM_MENU_ACTION_NONE)
	{
		if (currentmenu && (g_system_menu.go_idle < mcu_millis()))
		{
			currentmenu = g_system_menu.current_menu = 0;
			currentindex = g_system_menu.current_index = 0;
			g_system_menu.flags = SYSTEM_MENU_MODE_DISPLAY;
			g_system_menu.next_redraw = 0;
		}
		return;
	}

	// startup screen, alarm or other special (128...255)
	if (currentmenu < 0)
	{
		return;
	}

	const system_menu_page_t *menupage = system_menu_get(currentmenu);
	const system_menu_item_t *menuitem_ptr = (system_menu_item_t *)system_menu_get_item(currentmenu, currentindex);
	g_system_menu.go_idle = mcu_millis() + SYSTEM_MENU_GO_IDLE_MS;

	if (menupage)
	{
		// forces imediate render
		g_system_menu.next_redraw = 0;

		// checks if the menu has a custom action callback
		if (menupage->page_action)
		{
			// if the custom action callback returns 0 exit
			// else continue
			if (!menupage->page_action(action))
			{
				return;
			}
		}

		// if the item exists
		if (menuitem_ptr)
		{
			system_menu_item_t menuitem = {0};
			rom_memcpy(&menuitem, menuitem_ptr, sizeof(system_menu_item_t));
			// checks if the menu item has a custom action callback
			if (menuitem.item_action)
			{
				if (!menuitem.item_action(action, menuitem.action_arg))
				{
					return;
				}
			}
		}

		// executes the default system_menu actions
		switch (action)
		{
		case SYSTEM_MENU_ACTION_SELECT:
			// index -1 is return to previous menu
			if (currentindex < 0)
			{
				if (!system_menu_action_goto(action, CONST_VARG(VARG_UINT(menupage->parent_id))))
				{
					return;
				}
			}
			break;
		case SYSTEM_MENU_ACTION_NEXT:
			if (currentmenu)
			{
				if ((g_system_menu.total_items - 1) > currentindex)
				{
					g_system_menu.current_index++;
				}
			}
			break;
		case SYSTEM_MENU_ACTION_PREV:
			if (currentmenu)
			{
				if (currentindex > -1)
				{
					g_system_menu.current_index--;
				}
			}
			break;
		default:
			// no new action
			return;
		}
	}
}

void system_menu_render(void)
{
	// checks if it's time to redraw
	if (g_system_menu.next_redraw < mcu_millis())
	{
		uint8_t item_index = 0;
		MENU_LOOP(g_system_menu.menu_entry, menu_page)
		{
			if (menu_page->menu_id == g_system_menu.current_menu)
			{
				// if menu has custom render
				if (menu_page->page_render)
				{
					menu_page->page_render();
					return;
				}

				// renders header
				if (!item_index)
				{
					system_menu_render_header(menu_page->page_label);
				}

				if (g_system_menu.flags & SYSTEM_MENU_MODE_EDIT)
				{
					const system_menu_item_t *item = system_menu_get_item(g_system_menu.current_menu, g_system_menu.current_index);
					system_menu_render_menu_item(item_index, item);
				}
				else
				{
					// runs througn each item
					system_menu_index_t *item = menu_page->items_index;
					while (item)
					{
						if (system_menu_render_menu_item_filter(item_index))
						{
							system_menu_render_menu_item(item_index, item->menu_item);
						}
						item = item->next;
						item_index++;
					}
				}
			}
		}

		system_menu_render_footer();
		return;
	}
}

static void system_menu_redraw_timeout(uint32_t delay)
{
	g_system_menu.next_redraw = delay + mcu_millis();
}

static const system_menu_page_t *system_menu_get(uint8_t menu_id)
{
	MENU_LOOP(g_system_menu.menu_entry, menu_page)
	{
		if (menu_page->menu_id == menu_id)
		{
			return menu_page;
		}
	}

	// could not find
	// return empty item
	return NULL;
}

static const system_menu_item_t *system_menu_get_item(uint8_t menu_id, int16_t index)
{
	if (menu_id && (index >= 0))
	{
		MENU_LOOP(g_system_menu.menu_entry, menu_page)
		{
			if (menu_page->menu_id == menu_id)
			{
				if (!menu_page->items_index)
				{
					return NULL;
				}
				// in the item group
				system_menu_index_t *item = menu_page->items_index;
				while (index && item->next)
				{
					item = item->next;
					index--;
				}

				return (!index) ? item->menu_item : NULL;
			}
		}
	}

	// could not find
	// return empty item
	return NULL;
}

static uint8_t system_menu_get_item_count(uint8_t menu_id)
{
	uint8_t item_count = 0;
	MENU_LOOP(g_system_menu.menu_entry, menu_page)
	{
		if (menu_page->menu_id == menu_id)
		{
			if (!(menu_page->items_index))
			{
				return item_count;
			}
			system_menu_index_t *item = menu_page->items_index;
			item_count++;
			while (item->next)
			{
				item = item->next;
				item_count++;
			}

			return item_count;
		}
	}

	// could not find
	// return empty item
	return item_count;
}

void system_menu_append_item(uint8_t menu_id, system_menu_index_t *newitem)
{
	MENU_LOOP(g_system_menu.menu_entry, menu_page)
	{
		if (menu_page->menu_id == menu_id)
		{
			if (!menu_page->items_index)
			{
				menu_page->items_index = newitem;
				return;
			}
			system_menu_index_t *item = menu_page->items_index;
			while (item->next)
			{
				item = item->next;
			}

			item->next = newitem;
		}
	}
}

void system_menu_append(system_menu_page_t *newpage)
{
	system_menu_page_t *ptr = g_system_menu.menu_entry;

	while (ptr->extended != NULL)
	{
		ptr = ptr->extended;
	}

	ptr->extended = newpage;
}

void system_menu_reset(void)
{
	// startup menu
	g_system_menu.current_menu = 255;
	g_system_menu.current_index = 0;
	g_system_menu.total_items = 0;
	g_system_menu.flags = 0;
	// forces imediate render
	g_system_menu.next_redraw = 0;
	g_system_menu.go_idle = SYSTEM_MENU_REDRAW_STARTUP_MS + mcu_millis();
}

/**
 * Helper µCNC commands callbacks
 * **/

// calls a new menu
uint8_t system_menu_action_goto(uint8_t action, void *cmd)
{
	if (action == SYSTEM_MENU_ACTION_SELECT)
	{
		uint8_t menu_id = (uint8_t)VARG_UINT(cmd);
		g_system_menu.current_menu = menu_id;
		g_system_menu.current_index = 0;
		// menu 0 goes to idle screen
		if (menu_id)
		{
			g_system_menu.total_items = system_menu_get_item_count(menu_id);
		}
		return 0;
	}
	return 1;
}

uint8_t system_menu_action_rt_cmd(uint8_t action, void *cmd)
{
	if (action == SYSTEM_MENU_ACTION_SELECT)
	{
		cnc_call_rt_command((uint8_t)VARG_UINT(cmd));
		return 0;
	}
	return 1;
}

uint8_t system_menu_action_serial_cmd(uint8_t action, void *cmd)
{
	if (action == SYSTEM_MENU_ACTION_SELECT)
	{
		serial_inject_cmd((const char *)cmd);
		return 0;
	}
	return 1;
}

uint8_t system_menu_edit_var(uint8_t action, void *cmd)
{
	if (g_system_menu.flags & SYSTEM_MENU_MODE_EDIT)
	{
		g_system_menu.flags ^= SYSTEM_MENU_MODE_MODIFY;
	}
	g_system_menu.flags |= SYSTEM_MENU_MODE_EDIT;

	if (cmd)
	{
		switch (*((uint8_t *)cmd))
		{
		case SYSTEM_MENU_ACTION_SELECT:
			/* code */
			break;
		case SYSTEM_MENU_ACTION_PREV:
			/* code */
			break;
		case SYSTEM_MENU_ACTION_NEXT:
			/* code */
			break;
		default:
			break;
		}
	}

	// stop action propagation
	return 0;
}

/**
 * Helper µCNC render callbacks
 * These can be overriten by the display to perform the rendering of the menu content
 * **/

void __attribute__((weak)) system_menu_render_header(const char *__s)
{
	// render the menu header
}

void __attribute__((weak)) system_menu_render_footer(void)
{
	// render the menu footer
}

bool __attribute__((weak)) system_menu_render_menu_item_filter(uint8_t item_index)
{
	// filters if the menu item in an item page is to be printed (true) or not (false)
	return true;
}

void __attribute__((weak)) system_menu_render_menu_item(uint8_t item_index, const system_menu_item_t *item)
{
	// this is the default rendering of a menu item
	// prints a label
	system_menu_item_t menuitem = {0};
	rom_memcpy(&menuitem, item, sizeof(system_menu_item_t));

	// render item label
	system_menu_item_render_label(item_index, menuitem.label);

	// menu item has custom render method
	if (menuitem.item_render)
	{
		menuitem.item_render(item_index, &menuitem);
	}
	else
	{
		// defaults to render the arg as a string
		system_menu_item_render_str_arg(item_index, &menuitem);
	}
}

void __attribute__((weak)) system_menu_render_startup(void)
{
	// render startup screen
}

void __attribute__((weak)) system_menu_render_idle(void)
{
	// render idle screen
	// this is usually the screen showing the position and status of the machine
}

void __attribute__((weak)) system_menu_render_alarm(void)
{
	// render alarm screen
}

/**
 * Helper µCNC render callbacks
 * **/
void __attribute__((weak)) system_menu_item_render_label(uint8_t item_index, const char *label)
{
	// this is were the display renders the item label
}

void __attribute__((weak)) system_menu_item_render_arg(uint8_t item_index, const char *label)
{
	// this is were the display renders the item variable
}

void __attribute__((weak)) system_menu_item_render_str_arg(uint8_t item_index, system_menu_item_t *item)
{
	system_menu_item_render_arg(item_index, (const char *)item->argptr);
}

void system_menu_item_render_uint32_arg(uint8_t item_index, system_menu_item_t *item)
{
	char buffer[SYSTEM_MENU_MAX_STR_LEN];
	system_menu_int_to_str(buffer, *((uint32_t *)item->argptr));
	system_menu_item_render_arg(item_index, (const char *)buffer);
}

void system_menu_item_render_uint16_arg(uint8_t item_index, system_menu_item_t *item)
{
	char buffer[SYSTEM_MENU_MAX_STR_LEN];
	system_menu_int_to_str(buffer, (uint32_t) * ((uint16_t *)item->argptr));
	system_menu_item_render_arg(item_index, (const char *)buffer);
}

void system_menu_item_render_uint8_arg(uint8_t item_index, system_menu_item_t *item)
{
	char buffer[SYSTEM_MENU_MAX_STR_LEN];
	system_menu_int_to_str(buffer, (uint32_t) * ((uint8_t *)item->argptr));
	system_menu_item_render_arg(item_index, (const char *)buffer);
}

void system_menu_item_render_bool_arg(uint8_t item_index, system_menu_item_t *item)
{
	char *buffer = (*((bool *)item->argptr)) ? "1" : "0";
	system_menu_item_render_arg(item_index, (const char *)buffer);
}

void system_menu_item_render_flt_arg(uint8_t item_index, system_menu_item_t *item)
{
	char buffer[SYSTEM_MENU_MAX_STR_LEN];
	system_menu_flt_to_str(buffer, *((float *)item->argptr));
	system_menu_item_render_arg(item_index, (const char *)buffer);
}

/**
 * Helper µCNC to display variables
 * **/

char *system_menu_var_to_str_set_buffer_ptr;
void system_menu_var_to_str_set_buffer(char *ptr)
{
	system_menu_var_to_str_set_buffer_ptr = ptr;
}

void system_menu_var_to_str(unsigned char c)
{
	*system_menu_var_to_str_set_buffer_ptr = c;
	*(++system_menu_var_to_str_set_buffer_ptr) = 0;
}

#endif
