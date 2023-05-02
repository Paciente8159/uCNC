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
#include <math.h>

system_menu_t g_system_menu;

static void system_menu_go_idle_timeout(uint32_t delay);
static uint8_t system_menu_get_item_count(uint8_t menu_id);
static void system_menu_go_idle_timeout(uint32_t delay);
static uint8_t system_menu_action_settings_cmd(uint8_t action, void *cmd);

// declarate startup screen
static void system_menu_startup(void)
{
	system_menu_render_startup();
}

// declarate idle screen
static void system_menu_idle(void)
{
	system_menu_render_idle();
	system_menu_go_idle_timeout(SYSTEM_MENU_REDRAW_IDLE_MS);
}
static uint8_t system_menu_main_open(uint8_t action)
{
	return system_menu_action_goto(SYSTEM_MENU_ACTION_SELECT, CONST_VARG(1));
}

DECL_MODULE(system_menu)
{
	// entry menu to startup screen
	DECL_DYNAMIC_MENU(255, 0, system_menu_startup, NULL);
	system_menu_append(MENU(255));

	// append idle menu
	DECL_DYNAMIC_MENU(0, 0, system_menu_idle, system_menu_main_open);
	system_menu_append(MENU(0));

	// append main
	DECL_MENU(1, 0, "Main menu");

	// main menu entries
	DECL_MENU_ACTION(1, hold, "Hold", &system_menu_action_rt_cmd, CONST_VARG(CMD_CODE_FEED_HOLD));
	DECL_MENU_ACTION(1, resume, "Resume", &system_menu_action_rt_cmd, CONST_VARG(CMD_CODE_CYCLE_START));
	DECL_MENU_ACTION(1, unlock, "Unlock", &system_menu_action_serial_cmd, "$X");
	DECL_MENU_ACTION(1, home, "Home", &system_menu_action_serial_cmd, "$H");
	DECL_MENU_GOTO(1, settings, "Settings", CONST_VARG(2));

	// append settings menu
	DECL_MENU(2, 1, "Settings");

	// settings menu
	DECL_MENU_ACTION(2, set_load, "Load settings", system_menu_action_settings_cmd, CONST_VARG(0));
	DECL_MENU_ACTION(2, set_save, "Save settings", system_menu_action_settings_cmd, CONST_VARG(1));
	DECL_MENU_ACTION(2, set_reset, "Reset settings", system_menu_action_settings_cmd, CONST_VARG(2));
	DECL_MENU_GOTO(2, ioconfig, "IO config", CONST_VARG(6));
	DECL_MENU_VAR(2, s11, "G64 fact:", &g_settings.g64_angle_factor, VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(2, s12, "Arc tol:", &g_settings.arc_tolerance, VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_GOTO(2, gohome, "Homing", CONST_VARG(3));
#if (AXIS_COUNT > 0)
	DECL_MENU_GOTO(2, goaxis, "Axis", CONST_VARG(4));
#endif
#if (defined(ENABLE_SKEW_COMPENSATION) || (KINEMATIC == KINEMATIC_LINEAR_DELTA) || (KINEMATIC == KINEMATIC_DELTA))
	DECL_MENU_GOTO(2, goaxis, "Kinematics", CONST_VARG(5));
#endif

	DECL_MENU(6, 2, "IO config");
	DECL_MENU_VAR(6, s2, "Step inv:", &g_settings.dir_invert_mask, VAR_TYPE_UINT8, system_menu_item_render_uint8_arg);
	DECL_MENU_VAR(6, s3, "Dir inv:", &g_settings.dir_invert_mask, VAR_TYPE_UINT8, system_menu_item_render_uint8_arg);
	DECL_MENU_VAR(6, s4, "Enable inv:", &g_settings.step_enable_invert, VAR_TYPE_UINT8, system_menu_item_render_uint8_arg);
	DECL_MENU_VAR(6, s5, "Limits inv:", &g_settings.limits_invert_mask, VAR_TYPE_UINT8, system_menu_item_render_uint8_arg);
	DECL_MENU_VAR(6, s6, "Probe inv:", &g_settings.probe_invert_mask, VAR_TYPE_BOOLEAN, system_menu_item_render_bool_arg);
	DECL_MENU_VAR(6, s7, "Control inv:", &g_settings.control_invert_mask, VAR_TYPE_UINT8, system_menu_item_render_uint8_arg);
#if ENCODERS > 0
	DECL_MENU_VAR(6, s8, "Step inv:", &g_settings.encoders_pulse_invert_mask, VAR_TYPE_UINT8, system_menu_item_render_uint8_arg);
	DECL_MENU_VAR(6, s9, "Step inv:", &g_settings.encoders_dir_invert_mask, VAR_TYPE_UINT8, system_menu_item_render_uint8_arg);
#endif

	// append homing settings menu
	DECL_MENU(3, 2, "Homing");

	DECL_MENU_VAR(3, s20, "Soft-limits:", &g_settings.soft_limits_enabled, VAR_TYPE_BOOLEAN, system_menu_item_render_bool_arg);
	DECL_MENU_VAR(3, s21, "Hard-limits:", &g_settings.hard_limits_enabled, VAR_TYPE_BOOLEAN, system_menu_item_render_bool_arg);
	DECL_MENU_VAR(3, s22, "Enable homing:", &g_settings.homing_enabled, VAR_TYPE_BOOLEAN, system_menu_item_render_bool_arg);
	DECL_MENU_VAR(3, s23, "Dir inv mask:", &g_settings.homing_dir_invert_mask, VAR_TYPE_BOOLEAN, system_menu_item_render_bool_arg);
	DECL_MENU_VAR(3, s24, "Slow feed:", &g_settings.homing_slow_feed_rate, VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(3, s25, "Fast feed:", &g_settings.homing_fast_feed_rate, VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(3, s26, "Debounce(ms):", &g_settings.debounce_ms, VAR_TYPE_BOOLEAN, system_menu_item_render_bool_arg);
	DECL_MENU_VAR(3, s27, "Offset:", &g_settings.homing_offset, VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);

// append steppers settings menu
#if (AXIS_COUNT > 0)
	DECL_MENU(4, 2, "Axis");
	DECL_MENU_VAR(4, s100, "X step/mm:", &g_settings.step_per_mm[0], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(4, s110, "X v-max:", &g_settings.max_feed_rate[0], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(4, s120, "X accel:", &g_settings.acceleration[0], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(4, s130, "X max dist:", &g_settings.max_distance[0], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
#ifdef ENABLE_BACKLASH_COMPENSATION
	DECL_MENU_VAR(4, s140, " X backlash:", &g_settings.backlash_steps[0], VAR_TYPE_UINT16, system_menu_item_render_uint16_arg);
#endif

#if (AXIS_COUNT > 1)
	DECL_MENU_VAR(4, s101, "Y step/mm:", &g_settings.step_per_mm[1], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(4, s111, "Y v-max:", &g_settings.max_feed_rate[1], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(4, s121, "Y accel:", &g_settings.acceleration[1], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(4, s131, "Y max dist:", &g_settings.max_distance[1], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
#ifdef ENABLE_BACKLASH_COMPENSATION
	DECL_MENU_VAR(4, s141, " Y backlash:", &g_settings.backlash_steps[1], VAR_TYPE_UINT16, system_menu_item_render_uint16_arg);
#endif
#endif
#if (AXIS_COUNT > 2)
	DECL_MENU_VAR(4, s102, "Z step/mm:", &g_settings.step_per_mm[2], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(4, s112, "Z v-max:", &g_settings.max_feed_rate[2], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(4, s122, "Z accel:", &g_settings.acceleration[2], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(4, s132, "Z max dist:", &g_settings.max_distance[2], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
#ifdef ENABLE_BACKLASH_COMPENSATION
	DECL_MENU_VAR(4, s142, " Z backlash:", &g_settings.backlash_steps[2], VAR_TYPE_UINT16, system_menu_item_render_uint16_arg);
#endif
#endif
#if (AXIS_COUNT > 3)
	DECL_MENU_VAR(4, s103, "A step/mm:", &g_settings.step_per_mm[3], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(4, s113, "A v-max:", &g_settings.max_feed_rate[3], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(4, s123, "A accel:", &g_settings.acceleration[3], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(4, s133, "A max dist:", &g_settings.max_distance[3], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
#ifdef ENABLE_BACKLASH_COMPENSATION
	DECL_MENU_VAR(4, s143, " A backlash:", &g_settings.backlash_steps[3], VAR_TYPE_UINT16, system_menu_item_render_uint16_arg);
#endif
#endif
#if (AXIS_COUNT > 4)
	DECL_MENU_VAR(4, s104, "B step/mm:", &g_settings.step_per_mm[4], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(4, s114, "B v-max:", &g_settings.max_feed_rate[4], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(4, s124, "B accel:", &g_settings.acceleration[4], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(4, s134, "B max dist:", &g_settings.max_distance[4], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
#ifdef ENABLE_BACKLASH_COMPENSATION
	DECL_MENU_VAR(4, s144, " B backlash:", &g_settings.backlash_steps[4], VAR_TYPE_UINT16, system_menu_item_render_uint16_arg);
#endif
#endif
#if (AXIS_COUNT > 5)
	DECL_MENU_VAR(4, s105, "C step/mm:", &g_settings.step_per_mm[5], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(4, s115, "C v-max:", &g_settings.max_feed_rate[5], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(4, s125, "C accel:", &g_settings.acceleration[5], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(4, s135, "C max dist:", &g_settings.max_distance[5], VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
#ifdef ENABLE_BACKLASH_COMPENSATION
	DECL_MENU_VAR(4, s145, " C backlash:", &g_settings.backlash_steps[5], VAR_TYPE_UINT16, system_menu_item_render_uint16_arg);
#endif
#endif
#endif

#if (defined(ENABLE_SKEW_COMPENSATION) || (KINEMATIC == KINEMATIC_LINEAR_DELTA) || (KINEMATIC == KINEMATIC_DELTA))
	DECL_MENU(5, 2, "Kinematics");
#ifdef ENABLE_SKEW_COMPENSATION
	DECL_MENU_VAR(5, s37, "XY factor:", &g_settings.skew_xy_factor, VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
#ifndef SKEW_COMPENSATION_XY_ONLY
	DECL_MENU_VAR(5, s38, "XZ factor:", &g_settings.skew_xz_factor, VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(5, s39, "YZ factor:", &g_settings.skew_yz_factor, VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
#endif
#endif
#if (KINEMATIC == KINEMATIC_LINEAR_DELTA)
	DECL_MENU_VAR(5, s106, "Arm len:", &g_settings.delta_arm_length, VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(5, s107, "Base rad:", &g_settings.delta_armbase_radius, VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
#elif (KINEMATIC == KINEMATIC_DELTA)
	DECL_MENU_VAR(5, s106, "Base rad:", &g_settings.delta_base_radius, VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(5, s107, "Eff rad:", &g_settings.delta_effector_radius, VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(5, s108, "Bicep len:", &g_settings.delta_bicep_length, VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(5, s109, "F-arm len:", &g_settings.delta_forearm_length, VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
	DECL_MENU_VAR(5, s28, "Home ang:", &g_settings.delta_bicep_homing_angle, VAR_TYPE_FLOAT, system_menu_item_render_flt_arg);
#endif
#endif

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
		// idle timeout occurred
		if (g_system_menu.go_idle < mcu_millis())
		{
			// system_menu_go_idle();
			currentmenu = g_system_menu.current_menu = 0;
			currentindex = g_system_menu.current_index = 0;
			g_system_menu.flags |= SYSTEM_MENU_MODE_REDRAW;
			system_menu_go_idle_timeout(SYSTEM_MENU_REDRAW_IDLE_MS);
			// g_system_menu.next_redraw = 0;
		}
		return;
	}

	// startup screen, alarm or other special (128...255)
	// ignore actions
	if (currentmenu < 0)
	{
		return;
	}

	const system_menu_page_t *menupage = system_menu_get_current();
	const system_menu_item_t *menuitem_ptr = (system_menu_item_t *)system_menu_get_current_item();

	if (menupage)
	{
		// forces imediate render
		g_system_menu.flags |= SYSTEM_MENU_MODE_REDRAW;
		system_menu_go_idle_timeout(SYSTEM_MENU_GO_IDLE_MS);

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

		// if it's over the nav back element
		if (currentindex < 0 || g_system_menu.current_multiplier < 0)
		{
			if (!system_menu_action_nav_back(action, NULL))
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
	uint8_t render_flags = g_system_menu.flags;
	uint8_t cur_index = g_system_menu.current_index;
	// checks if it's time to redraw
	if (render_flags & SYSTEM_MENU_MODE_REDRAW)
	{
		g_system_menu.flags &= ~SYSTEM_MENU_MODE_REDRAW;
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
					const system_menu_item_t *item = system_menu_get_current_item();
					system_menu_render_menu_item(render_flags, item);
				}
				else
				{
					// runs througn each item
					system_menu_index_t *item = menu_page->items_index;
					while (item)
					{
						if (system_menu_render_menu_item_filter(item_index))
						{
							system_menu_render_menu_item(render_flags | ((cur_index == item_index) ? SYSTEM_MENU_MODE_SELECT : 0), item->menu_item);
						}
						item = item->next;
						item_index++;
					}
				}
			}
		}

		system_menu_render_nav_back((g_system_menu.current_index < 0 || g_system_menu.current_multiplier < 0));
		system_menu_render_footer();
		return;
	}
}

static void system_menu_go_idle_timeout(uint32_t delay)
{
	g_system_menu.go_idle = delay + mcu_millis();
}

const system_menu_page_t *system_menu_get_current(void)
{
	uint8_t menu = g_system_menu.current_menu;
	MENU_LOOP(g_system_menu.menu_entry, menu_page)
	{
		if (menu_page->menu_id == menu)
		{
			return menu_page;
		}
	}

	// could not find
	// return empty item
	return NULL;
}

const system_menu_item_t *system_menu_get_current_item(void)
{
	int8_t menu = (int8_t)g_system_menu.current_menu;
	int16_t index = g_system_menu.current_index;
	if ((menu > 0) && (index >= 0))
	{
		MENU_LOOP(g_system_menu.menu_entry, menu_page)
		{
			if (menu_page->menu_id == menu)
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

	if (!ptr)
	{
		g_system_menu.menu_entry = newpage;
		return;
	}

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

	g_system_menu.current_multiplier = 0;
	// forces imediate render
	g_system_menu.flags = SYSTEM_MENU_MODE_REDRAW;
	system_menu_go_idle_timeout(SYSTEM_MENU_REDRAW_STARTUP_MS);
}

void system_menu_go_idle(void)
{
	// idle menu
	g_system_menu.current_menu = 0;
	g_system_menu.current_index = 0;
	g_system_menu.total_items = 0;
	g_system_menu.current_multiplier = 0;
	// forces imediate render
	g_system_menu.flags = SYSTEM_MENU_MODE_REDRAW;
	system_menu_go_idle_timeout(SYSTEM_MENU_GO_IDLE_MS);
}

/**
 * Helper µCNC commands callbacks
 * **/

// calls a new menu
uint8_t system_menu_action_goto(uint8_t action, void *cmd)
{
	if (action == SYSTEM_MENU_ACTION_SELECT)
	{
		uint8_t menu_id = (uint8_t)VARG_CONST(cmd);
		g_system_menu.current_menu = menu_id;
		g_system_menu.current_index = 0;
		g_system_menu.current_multiplier = 0;
		g_system_menu.flags &= ~(SYSTEM_MENU_MODE_EDIT | SYSTEM_MENU_MODE_MODIFY);
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
		cnc_call_rt_command((uint8_t)VARG_CONST(cmd));
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

static uint8_t system_menu_action_settings_cmd(uint8_t action, void *cmd)
{
	if (action == SYSTEM_MENU_ACTION_SELECT)
	{
		uint8_t settings_action = (uint8_t)VARG_CONST(cmd);
		switch (settings_action)
		{
		case 0:
			settings_init();
			break;
		case 1:
			settings_save(SETTINGS_ADDRESS_OFFSET, (uint8_t *)&g_settings, (uint8_t)sizeof(settings_t));
			break;
		case 2:
			settings_reset(false);
			break;
		default:
			break;
		}
		return 0;
	}
	return 1;
}

uint8_t system_menu_action_nav_back(uint8_t action, void *cmd)
{
	if (action == SYSTEM_MENU_ACTION_SELECT)
	{
		// direct to page nav back action
		if (cmd)
		{
			return system_menu_action_goto(action, cmd);
		}

		if (g_system_menu.current_multiplier < 0)
		{
			g_system_menu.current_multiplier = 0;
			g_system_menu.flags &= ~(SYSTEM_MENU_MODE_EDIT | SYSTEM_MENU_MODE_MODIFY);
			return 0;
		}

		if (g_system_menu.current_index < 0)
		{
			const system_menu_page_t *menu = system_menu_get_current();
			if (menu)
			{
				return system_menu_action_goto(action, CONST_VARG(VARG_CONST(menu->parent_id)));
			}
		}

		system_menu_go_idle();
		return system_menu_action_goto(action, CONST_VARG(0));
	}
	return 1;
}

uint8_t system_menu_action_edit(uint8_t action, void *cmd)
{
	uint8_t vartype = (uint8_t)VARG_CONST(cmd);
	uint8_t flags = g_system_menu.flags;
	int8_t currentmult = g_system_menu.current_multiplier;
	const system_menu_item_t *itmptr = NULL;
	system_menu_item_t item = {0};
	float modifier = 0;

	itmptr = system_menu_get_current_item();
	if (!itmptr)
	{
		system_menu_go_idle();
		return 0;
	}

	rom_memcpy(&item, itmptr, sizeof(system_menu_item_t));

	switch (action)
	{
	case SYSTEM_MENU_ACTION_SELECT:
		if (flags & SYSTEM_MENU_MODE_EDIT)
		{
			// toogle modify mode
			g_system_menu.flags ^= SYSTEM_MENU_MODE_MODIFY;
		}
		g_system_menu.flags |= SYSTEM_MENU_MODE_EDIT;
		break;
	case SYSTEM_MENU_ACTION_PREV:
	case SYSTEM_MENU_ACTION_NEXT:
		if (flags & SYSTEM_MENU_MODE_MODIFY)
		{
			// increment var by multiplier
			if (!item.argptr)
			{
				// passthrough action
				return 1;
			}

			if (vartype == VAR_TYPE_FLOAT)
			{
				modifier = (action == SYSTEM_MENU_ACTION_NEXT) ? powf(10.0f, (currentmult - 3)) : -powf(10.0f, (currentmult - 3));
			}
			else
			{
				modifier = (action == SYSTEM_MENU_ACTION_NEXT) ? powf(10.0f, currentmult) : -powf(10.0f, currentmult);
			}
		}
		else if (flags & SYSTEM_MENU_MODE_EDIT)
		{
			currentmult += (action == SYSTEM_MENU_ACTION_NEXT) ? 1 : -1;
		}
		else
		{
			// passthrough action
			return 1;
		}
		break;
	}

	// modify mode enabled
	if (flags & SYSTEM_MENU_MODE_MODIFY)
	{
		// adds the multiplier
		switch (vartype)
		{
		case VAR_TYPE_BOOLEAN:
			(*(bool *)item.argptr) = (modifier > 0) ? 1 : 0;
			break;
		case VAR_TYPE_INT8:
		case VAR_TYPE_UINT8:
			(*(uint8_t *)item.argptr) += (uint8_t)modifier;
			(*(uint8_t *)item.argptr) = CLAMP(0, (*(uint8_t *)item.argptr), 0xFF);
			break;
		case VAR_TYPE_INT16:
		case VAR_TYPE_UINT16:
			(*(uint16_t *)item.argptr) += (uint16_t)modifier;
			(*(uint16_t *)item.argptr) = CLAMP(0, (*(uint16_t *)item.argptr), 0xFFFF);
			break;
		case VAR_TYPE_INT32:
		case VAR_TYPE_UINT32:
			(*(uint32_t *)item.argptr) += (uint32_t)modifier;
			(*(uint32_t *)item.argptr) = CLAMP(0, (*(uint32_t *)item.argptr), 0xFFFFFFFF);
			break;
		case VAR_TYPE_FLOAT:
			(*(float *)item.argptr) += modifier;
			(*(float *)item.argptr) = CLAMP(__FLT_MIN__, (*(float *)item.argptr), __FLT_MAX__);
			break;
		}
	}
	else
	{
		// clamps the multiplier
		switch (vartype)
		{
		case VAR_TYPE_BOOLEAN:
			g_system_menu.current_multiplier = CLAMP(-1, currentmult, 0);
			break;
		case VAR_TYPE_INT8:
		case VAR_TYPE_UINT8:
			g_system_menu.current_multiplier = CLAMP(-1, currentmult, 2);
			break;
		case VAR_TYPE_INT16:
		case VAR_TYPE_UINT16:
			g_system_menu.current_multiplier = CLAMP(-1, currentmult, 4);
		case VAR_TYPE_INT32:
		case VAR_TYPE_UINT32:
		case VAR_TYPE_FLOAT:
			g_system_menu.current_multiplier = CLAMP(-1, currentmult, 9);
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

void __attribute__((weak)) system_menu_render_nav_back(bool is_hover)
{
	// render the nav back element
}

bool __attribute__((weak)) system_menu_render_menu_item_filter(uint8_t item_index)
{
	// filters if the menu item in an item page is to be printed (true) or not (false)
	return true;
}

void __attribute__((weak)) system_menu_render_menu_item(uint8_t render_flags, const system_menu_item_t *item)
{
	// this is the default rendering of a menu item
	// prints a label
	system_menu_item_t menuitem = {0};
	rom_memcpy(&menuitem, item, sizeof(system_menu_item_t));

	// render item label
	system_menu_item_render_label(render_flags, menuitem.label);

	// menu item has custom render method
	if (menuitem.item_render)
	{
		menuitem.item_render(render_flags, &menuitem);
	}
	else
	{
		// defaults to render the arg as a string
		system_menu_item_render_arg(render_flags, menuitem.render_arg);
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

void __attribute__((weak)) system_menu_item_render_arg(uint8_t render_flags, const char *label)
{
	// this is were the display renders the item variable
}

void __attribute__((weak)) system_menu_item_render_str_arg(uint8_t render_flags, system_menu_item_t *item)
{
	system_menu_item_render_arg(render_flags, (const char *)item->argptr);
}

void system_menu_item_render_uint32_arg(uint8_t render_flags, system_menu_item_t *item)
{
	char buffer[SYSTEM_MENU_MAX_STR_LEN];
	system_menu_int_to_str(buffer, *((uint32_t *)item->argptr));
	system_menu_item_render_arg(render_flags, (const char *)buffer);
}

void system_menu_item_render_uint16_arg(uint8_t render_flags, system_menu_item_t *item)
{
	char buffer[SYSTEM_MENU_MAX_STR_LEN];
	system_menu_int_to_str(buffer, (uint32_t) * ((uint16_t *)item->argptr));
	system_menu_item_render_arg(render_flags, (const char *)buffer);
}

void system_menu_item_render_uint8_arg(uint8_t render_flags, system_menu_item_t *item)
{
	char buffer[SYSTEM_MENU_MAX_STR_LEN];
	system_menu_int_to_str(buffer, (uint32_t) * ((uint8_t *)item->argptr));
	system_menu_item_render_arg(render_flags, (const char *)buffer);
}

void system_menu_item_render_bool_arg(uint8_t render_flags, system_menu_item_t *item)
{
	char *buffer = (*((bool *)item->argptr)) ? "1" : "0";
	system_menu_item_render_arg(render_flags, (const char *)buffer);
}

void system_menu_item_render_flt_arg(uint8_t render_flags, system_menu_item_t *item)
{
	char buffer[SYSTEM_MENU_MAX_STR_LEN];
	system_menu_flt_to_str(buffer, *((float *)item->argptr));
	system_menu_item_render_arg(render_flags, (const char *)buffer);
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