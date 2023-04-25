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

DECL_MENU_ACTION(hold, "Hold", system_menu_action_rt_cmd, CONST_VARG(CMD_CODE_FEED_HOLD));
DECL_MENU_ACTION(resume, "Resume", system_menu_action_rt_cmd, CONST_VARG(RT_CMD_CYCLE_START));
DECL_MENU_ACTION(home, "Home", system_menu_action_serial_cmd, "$H");
// DECL_MENU_GOTO(settings, "Settings", &settings_menu);
DECL_MENU(1, 0, "Main menu", 3, &hold, &resume, &home);
DECL_MENU(2, 1, "Settings menu", 0, NULL);

system_menu_t g_system_menu;

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
	g_system_menu.flags = SYSTEM_MENU_STARTUP | SYSTEM_MENU_RENDER;
	g_system_menu.next_redraw = mcu_millis() + SYSTEM_MENU_IDLE_TIMEOUT_MS;
}

CREATE_EVENT_LISTENER(cnc_dotasks, system_menu_render);
void system_menu_render(void)
{
	uint8_t renderflags = g_system_menu.flags;
	g_system_menu.flags &= ~SYSTEM_MENU_RENDER;

	if (g_system_menu.next_redraw < mcu_millis())
	{
		// the render flag is active
		if (renderflags & SYSTEM_MENU_RENDER)
		{
			if (renderflags & SYSTEM_MENU_ALARM)
			{
				system_menu_render_startup();
				return;
			}

			if (renderflags & SYSTEM_MENU_STARTUP)
			{
				system_menu_render_startup();
				return;
			}

			if (renderflags & SYSTEM_MENU_IDLE)
			{
				system_menu_render_idle();
				return;
			}

			if (!g_system_menu.current_menu)
			{
				// nothing to render
				return;
			}
			else
			{
				uint8_t item_index = 0;
				MENU_LOOP(g_system_menu.menu_entry, menu_item)
				{
					if (menu_item->menu_id == g_system_menu.current_menu)
					{
						if (!item_index)
						{
							system_menu_render_header(menu_item->label);
						}

						for (uint8_t i = 0; i < menu_item->item_count; i++)
						{
							if (menu_item->items[i]->render)
							{
								menu_item->items[i]->render(menu_item->items[i]->render_arg);
							}
							else
							{
								system_menu_render_content(item_index + i, menu_item->items[i]);
							}
						}
						item_index += menu_item->item_count;
					}
				}
			}
		}

		g_system_menu.next_redraw = mcu_millis() + SYSTEM_MENU_REDRAW_MS;
	}
}

void system_menu_action(uint8_t action)
{
	static uint32_t last_action_time = 0;
	uint32_t current_time = mcu_millis();

	bool is_idle = ((current_time - last_action_time) > SYSTEM_MENU_IDLE_TIMEOUT_MS);

	if (action)
	{
		is_idle = false;
		last_action_time = current_time;
	}
	else if (is_idle)
	{
		g_system_menu.flags = SYSTEM_MENU_IDLE;
		return;
	}

	switch (action)
	{
	case SYSTEM_MENU_ACTION_SELECT:
		if (!g_system_menu.current_menu)
		{
			// enter main menu
			g_system_menu.current_menu = 1;
			g_system_menu.current_index = 0;
		}
		else
		{
			// if inside a menu get the arg
			const system_menu_item_t *next = system_menu_get_item(g_system_menu.current_menu, g_system_menu.current_index);
			if (next)
			{
				system_menu_item_t item = {0};
				rom_memcpy(&item, next, sizeof(system_menu_item_t));
				// found custom action and execute
				if (item.action)
				{
					item.action(item.action_arg);
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
		g_system_menu.current_index++;
		break;
	case SYSTEM_MENU_ACTION_PREV:
		g_system_menu.current_index--;
		break;
	default:
		// no new action don't render
		return;
	}

	g_system_menu.flags |= SYSTEM_MENU_RENDER;
}

/**
 * Helper µCNC commands callbacks
 * **/

// calls a new menu
void system_menu_action_goto(void *cmd)
{
	g_system_menu.current_menu = (uint8_t)cmd;
	g_system_menu.current_index = 0;
	g_system_menu.flags |= SYSTEM_MENU_RENDER;
}

void system_menu_action_rt_cmd(void *cmd)
{
	cnc_call_rt_command((uint8_t)cmd);
}

void system_menu_action_serial_cmd(void *cmd)
{
	serial_inject_cmd((const char *)cmd, false);
}

void __attribute__((weak)) system_menu_render_header(const char *__s)
{
	// render the menu header
}

void __attribute__((weak)) system_menu_render_content(uint8_t item_index, const system_menu_item_t *item)
{
	// render item
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

void system_menu_init(void)
{
	g_system_menu.current_menu = 0;
	g_system_menu.current_index = 0;
	g_system_menu.menu_entry = NULL;
	g_system_menu.flags = SYSTEM_MENU_RENDER;
	system_menu_append(MENU(1));
	system_menu_append(MENU(2));

#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_dotasks, system_menu_render);
	// ADD_EVENT_LISTENER(cnc_reset, system_menu_render);
#endif
}

#endif
