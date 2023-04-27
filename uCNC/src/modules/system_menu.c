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

DECL_MENU_ACTION(hold, "Hold", &system_menu_action_rt_cmd, CONST_VARG(CMD_CODE_FEED_HOLD));
DECL_MENU_ACTION(resume, "Resume", &system_menu_action_rt_cmd, CONST_VARG(CMD_CODE_CYCLE_START));
DECL_MENU_ACTION(home, "Home", &system_menu_action_serial_cmd, "$H");
DECL_MENU_GOTO(settings, "Settings", CONST_VARG(2));
DECL_MENU(1, 0, "Main menu", 4, &hold, &resume, &home, &settings);

// Settings menu will be dynamic
// this allow more complex generation with preprocessor conditions
void system_menu_settings_page_render(void)
{
	system_menu_render_header(__romstr__("Settings"));
	uint8_t index = 0;
	if (system_menu_render_menu_item_filter(index))
	{
		system_menu_item_render_label(index, "X st/mm");
		char buffer[SYSTEM_MENU_MAX_STR_LEN];
		// system_menu_item_render_var(index, )
	}
}
void system_menu_settings_page_action(void)
{
}

DECL_DYNAMIC_MENU(2, 1, system_menu_settings_page_render, system_menu_settings_page_action);

system_menu_t g_system_menu;

static void system_menu_enqueue_redraw(uint32_t delay)
{
	g_system_menu.next_redraw = delay + mcu_millis();
}

static const system_menu_item_t *system_menu_get_item(uint8_t menu_id, int16_t index)
{
	MENU_LOOP(g_system_menu.menu_entry, menu_item)
	{
		if (menu_item->menu_id == menu_id)
		{
			// in the item group
			if (index < menu_item->item_count)
			{
				return menu_item->items[index];
			}
			else
			{
				// must be in one extended menu
				index -= menu_item->item_count;
				// get the next set of extended menu entries
			}
		}
	}

	// could not find
	// return empty item
	return NULL;
}

static uint8_t system_menu_get_itemcount(uint8_t menu_id)
{
	uint8_t item_count = 0;
	MENU_LOOP(g_system_menu.menu_entry, menu_item)
	{
		if (menu_item->menu_id == menu_id)
		{
			item_count += menu_item->item_count;
		}
	}

	// could not find
	// return empty item
	return item_count;
}

void system_menu_append(system_menu_page_t *extended_menu)
{
	system_menu_page_t *ptr = g_system_menu.menu_entry;

	while (ptr->extended != NULL)
	{
		ptr = ptr->extended;
	}

	ptr->extended = extended_menu;
}

void system_menu_reset(void)
{
	g_system_menu.current_menu = 0;
	g_system_menu.current_index = 0;
	g_system_menu.total_items = 0;
	g_system_menu.flags = SYSTEM_MENU_STARTUP;
	// forces imediate render
	g_system_menu.next_redraw = 0;
}

void system_menu_render(void)
{
	uint8_t renderflags = g_system_menu.flags;

	if (g_system_menu.next_redraw < mcu_millis())
	{
		g_system_menu.flags = SYSTEM_MENU_IDLE;

		if (renderflags & SYSTEM_MENU_ALARM)
		{
			system_menu_render_alarm();
			return;
		}

		if (renderflags & SYSTEM_MENU_STARTUP)
		{
			system_menu_render_startup();
			system_menu_enqueue_redraw(SYSTEM_MENU_REDRAW_STARTUP_MS);
			return;
		}

		if (renderflags & SYSTEM_MENU_ACTIVE)
		{
			uint8_t item_index = 0;
			MENU_LOOP(g_system_menu.menu_entry, menu_item)
			{
				if (menu_item->menu_id == g_system_menu.current_menu)
				{
					if (!menu_item->page_render)
					{
						if (!item_index)
						{
							system_menu_render_header(menu_item->page_label);
						}

						for (uint8_t i = 0; i < menu_item->item_count; i++, item_index++)
						{
							if (system_menu_render_menu_item_filter(item_index))
							{
								system_menu_render_menu_item(item_index, menu_item->items[i]);
							}
						}
					}
					else
					{
						// custom page render
						menu_item->page_render();
					}
				}
			}

			system_menu_render_footer();
			system_menu_enqueue_redraw(SYSTEM_MENU_ACTIVE_REDRAW_MS);
			return;
		}

		// idle or nothing to render
		// reset menus
		g_system_menu.current_menu = 0;
		g_system_menu.current_index = 0;
		system_menu_render_idle();
		system_menu_enqueue_redraw(SYSTEM_MENU_REDRAW_IDLE_MS);
	}
}

bool system_menu_is_item_active(uint8_t item_index)
{
	return (g_system_menu.current_index == item_index);
}

void system_menu_action(uint8_t action)
{
	system_menu_item_t *next = NULL;

	switch (action)
	{
	case SYSTEM_MENU_ACTION_SELECT:
		g_system_menu.flags |= SYSTEM_MENU_ACTIVE;
		if (!g_system_menu.current_menu)
		{
			// enter main menu
			g_system_menu.current_menu = 1;
			g_system_menu.total_items = system_menu_get_itemcount(1);
			g_system_menu.current_index = 0;
		}
		else
		{
			// if inside a menu get the arg
			next = (system_menu_item_t *)system_menu_get_item(g_system_menu.current_menu, g_system_menu.current_index);
			if (next)
			{
				system_menu_item_t item = {0};
				rom_memcpy(&item, next, sizeof(system_menu_item_t));
				// found custom action and execute
				if (item.action)
				{
					item.action(item.action_arg);
					g_system_menu.next_redraw = 0;
					return;
				}
			}
			else
			{
				// something went wrong (menu not found)
				// return to home screen
				g_system_menu.current_menu = 0;
				g_system_menu.current_index = 0;
			}
		}
		break;
	case SYSTEM_MENU_ACTION_NEXT:
		if (g_system_menu.current_menu)
		{
			g_system_menu.flags |= SYSTEM_MENU_ACTIVE;
			if ((g_system_menu.total_items - 1) > g_system_menu.current_index)
			{
				g_system_menu.current_index++;
			}
		}
		break;
	case SYSTEM_MENU_ACTION_PREV:
		if (g_system_menu.current_menu)
		{
			g_system_menu.flags |= SYSTEM_MENU_ACTIVE;
			if (g_system_menu.current_index)
			{
				g_system_menu.current_index--;
			}
		}
		break;
	default:
		// no new action don't render
		return;
	}

	// forces imediate render
	g_system_menu.next_redraw = 0;
}

/**
 * Helper µCNC commands callbacks
 * **/

// calls a new menu
void system_menu_action_goto(void *cmd)
{
	g_system_menu.current_menu = (uint8_t)VARG_UINT(cmd);
	g_system_menu.current_index = 0;
	g_system_menu.total_items = system_menu_get_itemcount((uint8_t)VARG_UINT(cmd));
	g_system_menu.flags |= SYSTEM_MENU_ACTIVE;
	// forces imediate render
	g_system_menu.next_redraw = 0;
}

void system_menu_action_rt_cmd(void *cmd)
{
	cnc_call_rt_command((uint8_t)VARG_UINT(cmd));
	g_system_menu.flags |= SYSTEM_MENU_ACTIVE;
	// forces imediate render
	g_system_menu.next_redraw = 0;
}

void system_menu_action_serial_cmd(void *cmd)
{
	serial_inject_cmd((const char *)cmd);
	g_system_menu.flags |= SYSTEM_MENU_ACTIVE;
	// forces imediate render
	g_system_menu.next_redraw = 0;
}

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
	return true;
}

void __attribute__((weak)) system_menu_render_menu_item(uint8_t item_index, const system_menu_item_t *item)
{
	system_menu_item_t menuitem = {0};
	rom_memcpy(&menuitem, item, sizeof(system_menu_item_t));

	// menu item has custom render method
	if (menuitem.render)
	{
		menuitem.render(menuitem.render_arg);
	}
	else
	{
		// render item
		system_menu_item_render_label(item_index, menuitem.label);
		if (menuitem.argptr)
		{
		}
	}
}

void __attribute__((weak)) system_menu_render_startup(void)
{
	// render startup screen
	system_menu_render_header(__romstr__("µCNC"));
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
}

void __attribute__((weak)) system_menu_item_render_value(uint8_t item_index, const char *label)
{
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

DECL_MODULE(system_menu)
{
	// entry menu
	g_system_menu.menu_entry = MENU(1);
	// append subsequent menus
	system_menu_append(MENU(2));
	system_menu_reset();
}

#endif
