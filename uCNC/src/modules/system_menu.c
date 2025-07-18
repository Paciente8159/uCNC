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

#include "../cnc.h"
#include "system_menu.h"
#include <math.h>

system_menu_t g_system_menu;

float g_system_menu_jog_distance, g_system_menu_jog_feed;

static uint8_t system_menu_get_item_count(uint8_t menu_id);
static bool system_menu_action_settings_cmd(uint8_t action, system_menu_item_t *item);
static bool system_menu_action_jog(uint8_t action, system_menu_item_t *item);
static bool system_menu_action_overrides(uint8_t action, system_menu_item_t *item);
static void system_menu_render_axis_position(uint8_t render_flags, system_menu_item_t *item);
static bool system_menu_action_nav_back(uint8_t action, const system_menu_page_t *item);

void system_menu_goto(uint8_t id)
{
	g_system_menu.current_menu = id;
	g_system_menu.current_index = 0;
	g_system_menu.current_multiplier = 0;
	g_system_menu.flags &= ~(SYSTEM_MENU_MODE_EDIT | SYSTEM_MENU_MODE_MODIFY);
	// menu 0 goes to idle screen
	if (id)
	{
		g_system_menu.total_items = system_menu_get_item_count(id);
	}

	// Set correct flags in case go to is called from outside of this module
	g_system_menu.flags |= SYSTEM_MENU_MODE_REDRAW;
	system_menu_action_timeout(SYSTEM_MENU_GO_IDLE_MS);
}

// declarate startup screen
static void system_menu_startup(uint8_t render_flags)
{
	system_menu_render_startup();
}

// declarate idle screen
static void system_menu_idle(uint8_t render_flags)
{
	system_menu_render_idle();
	system_menu_action_timeout(SYSTEM_MENU_REDRAW_IDLE_MS);
}
static bool system_menu_main_open(uint8_t action)
{
	system_menu_goto(SYSTEM_MENU_ID_MAIN_MENU);
	return true;
}

DECL_MODULE(system_menu)
{
	// this prevents reloading the module
	// can be initialized anywere
	static bool loaded = false;
	if (loaded)
	{
		return;
	}
	loaded = true;

	g_system_menu_jog_distance = 1.0f;
	g_system_menu_jog_feed = 100.0f;

	// entry menu to startup screen
	DECL_DYNAMIC_MENU(SYSTEM_MENU_ID_STARTUP, 0, system_menu_startup, NULL);

	// append idle menu
	DECL_DYNAMIC_MENU(SYSTEM_MENU_ID_IDLE, 0, system_menu_idle, system_menu_main_open);

	// append main
	DECL_MENU(SYSTEM_MENU_ID_MAIN_MENU, SYSTEM_MENU_ID_IDLE, STR_MAIN_MENU);

	// main menu entries
	DECL_MENU_ACTION(SYSTEM_MENU_ID_MAIN_MENU, hold, STR_HOLD, system_menu_action_rt_cmd, CONST_VARG(CMD_CODE_FEED_HOLD));
	DECL_MENU_ACTION(SYSTEM_MENU_ID_MAIN_MENU, resume, STR_RESUME, system_menu_action_rt_cmd, CONST_VARG(CMD_CODE_CYCLE_START));
	DECL_MENU_ACTION(SYSTEM_MENU_ID_MAIN_MENU, unlock, STR_UNLOCK, system_menu_action_serial_cmd, "$X\r");
	DECL_MENU_ACTION(SYSTEM_MENU_ID_MAIN_MENU, home, STR_HOME, system_menu_action_serial_cmd, "$H\r");
	DECL_MENU_GOTO(SYSTEM_MENU_ID_MAIN_MENU, jog, STR_JOG, CONST_VARG(SYSTEM_MENU_ID_JOG));
	DECL_MENU_GOTO(SYSTEM_MENU_ID_MAIN_MENU, overrides, STR_OVERRIDES, CONST_VARG(SYSTEM_MENU_ID_OVERRIDES));
	DECL_MENU_GOTO(SYSTEM_MENU_ID_MAIN_MENU, settings, STR_SETTINGS, CONST_VARG(SYSTEM_MENU_ID_SETTINGS));

	DECL_MENU(SYSTEM_MENU_ID_OVERRIDES, SYSTEM_MENU_ID_MAIN_MENU, STR_OVERRIDES);
	DECL_MENU_VAR_CUSTOM_EDIT(SYSTEM_MENU_ID_OVERRIDES, ovf, STR_FEED_OVR, &g_planner_state.feed_override, VAR_TYPE_UINT8, system_menu_action_overrides, CONST_VARG('f'));
	DECL_MENU_ACTION(SYSTEM_MENU_ID_OVERRIDES, ovf_100, STR_FEED_100, system_menu_action_rt_cmd, CONST_VARG(CMD_CODE_FEED_100));
#if (TOOL_COUNT > 0)
	DECL_MENU_VAR_CUSTOM_EDIT(SYSTEM_MENU_ID_OVERRIDES, ovt, STR_TOOL_OVR, &g_planner_state.spindle_speed_override, VAR_TYPE_UINT8, system_menu_action_overrides, CONST_VARG('s'));
#endif
	DECL_MENU_ACTION(SYSTEM_MENU_ID_OVERRIDES, ovt_100, STR_TOOL_100, system_menu_action_rt_cmd, CONST_VARG(CMD_CODE_SPINDLE_100));

	// append Jog menu
	// default initial distance
	DECL_MENU(SYSTEM_MENU_ID_JOG, SYSTEM_MENU_ID_MAIN_MENU, STR_JOG);
	DECL_MENU_ENTRY(SYSTEM_MENU_ID_JOG, jogx, STR_JOG_AXIS("X"), NULL, system_menu_render_axis_position, NULL, system_menu_action_jog, "X");
#if (AXIS_COUNT > 1)
	DECL_MENU_ENTRY(SYSTEM_MENU_ID_JOG, jogy, STR_JOG_AXIS("Y"), NULL, system_menu_render_axis_position, NULL, system_menu_action_jog, "Y");
#endif
#if (AXIS_COUNT > 2)
	DECL_MENU_ENTRY(SYSTEM_MENU_ID_JOG, jogz, STR_JOG_AXIS("Z"), NULL, system_menu_render_axis_position, NULL, system_menu_action_jog, "Z");
#endif
#if (AXIS_COUNT > 3)
	DECL_MENU_ENTRY(SYSTEM_MENU_ID_JOG, joga, STR_JOG_AXIS("A"), NULL, system_menu_render_axis_position, NULL, system_menu_action_jog, "A");
#endif
#if (AXIS_COUNT > 4)
	DECL_MENU_ENTRY(SYSTEM_MENU_ID_JOG, jogb, STR_JOG_AXIS("B"), NULL, system_menu_render_axis_position, NULL, system_menu_action_jog, "B");
#endif
#if (AXIS_COUNT > 5)
	DECL_MENU_ENTRY(SYSTEM_MENU_ID_JOG, jogc, STR_JOG_AXIS("C"), NULL, system_menu_render_axis_position, NULL, system_menu_action_jog, "C");
#endif
	DECL_MENU_VAR(SYSTEM_MENU_ID_JOG, jogdist, STR_JOG_DIST, &g_system_menu_jog_distance, VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_JOG, jogfeed, STR_JOG_FEED, &g_system_menu_jog_feed, VAR_TYPE_FLOAT);

	// append settings menu
	DECL_MENU(SYSTEM_MENU_ID_SETTINGS, SYSTEM_MENU_ID_MAIN_MENU, STR_SETTINGS);

	// settings menu
	DECL_MENU_GOTO(SYSTEM_MENU_ID_SETTINGS, ioconfig, STR_IO_CONFIG, CONST_VARG(SYSTEM_MENU_ID_IO_SETTIGNS));
	DECL_MENU_GOTO(SYSTEM_MENU_ID_SETTINGS, gohome, STR_HOMING, CONST_VARG(SYSTEM_MENU_ID_HOMING));
#if (AXIS_COUNT > 0)
	DECL_MENU_GOTO(SYSTEM_MENU_ID_SETTINGS, goaxis, STR_AXIS, CONST_VARG(SYSTEM_MENU_ID_STEPPER_SETTINGS));
#endif
#if (defined(ENABLE_SKEW_COMPENSATION) || (KINEMATIC == KINEMATIC_LINEAR_DELTA) || (KINEMATIC == KINEMATIC_DELTA))
	DECL_MENU_GOTO(SYSTEM_MENU_ID_SETTINGS, kinemats, STR_KINEMATICS, CONST_VARG(SYSTEM_MENU_ID_KINEMATIC_SETTINGS));
#endif
	DECL_MENU_GOTO(SYSTEM_MENU_ID_SETTINGS, other_config, STR_OTHER, CONST_VARG(SYSTEM_MENU_ID_MISC_SETTINGS));
	DECL_MENU_ACTION(SYSTEM_MENU_ID_SETTINGS, set_load, STR_LOAD_SETTINGS, system_menu_action_settings_cmd, CONST_VARG(0));
	DECL_MENU_ACTION(SYSTEM_MENU_ID_SETTINGS, set_save, STR_SAVE_SETTINGS, system_menu_action_settings_cmd, CONST_VARG(1));
	DECL_MENU_ACTION(SYSTEM_MENU_ID_SETTINGS, set_reset, STR_RESET_SETTINGS, system_menu_action_settings_cmd, CONST_VARG(2));

	DECL_MENU(SYSTEM_MENU_ID_MISC_SETTINGS, SYSTEM_MENU_ID_SETTINGS, STR_OTHER);
	DECL_MENU_VAR(SYSTEM_MENU_ID_MISC_SETTINGS, s11, STR_G64_FACT, &g_settings.g64_angle_factor, VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_MISC_SETTINGS, s12, STR_ARC_TOL, &g_settings.arc_tolerance, VAR_TYPE_FLOAT);

#ifdef H_MAPPING_EEPROM_STORE_ENABLED
	DECL_MENU_VAR(SYSTEM_MENU_ID_MISC_SETTINGS, s215, STR_ARC_TOL, &g_settings.hmap_x, VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_MISC_SETTINGS, s216, STR_ARC_TOL, &g_settings.hmap_y, VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_MISC_SETTINGS, s217, STR_ARC_TOL, &g_settings.hmap_x_offset, VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_MISC_SETTINGS, s218, STR_ARC_TOL, &g_settings.hmap_y_offset, VAR_TYPE_FLOAT);
	// TODO allow edit each offset individually
#endif

	DECL_MENU(SYSTEM_MENU_ID_IO_SETTIGNS, SYSTEM_MENU_ID_SETTINGS, STR_IO_CONFIG);
	DECL_MENU_VAR(SYSTEM_MENU_ID_IO_SETTIGNS, s2, STR_STEP_INV, &g_settings.dir_invert_mask, VAR_TYPE_UINT8);
	DECL_MENU_VAR(SYSTEM_MENU_ID_IO_SETTIGNS, s3, STR_DIR_INV, &g_settings.dir_invert_mask, VAR_TYPE_UINT8);
	DECL_MENU_VAR(SYSTEM_MENU_ID_IO_SETTIGNS, s4, STR_ENABLE_INV, &g_settings.step_enable_invert, VAR_TYPE_UINT8);
	DECL_MENU_VAR(SYSTEM_MENU_ID_IO_SETTIGNS, s5, STR_LIMITS_INV, &g_settings.limits_invert_mask, VAR_TYPE_UINT8);
	DECL_MENU_VAR(SYSTEM_MENU_ID_IO_SETTIGNS, s6, STR_PROBE_INV, &g_settings.probe_invert_mask, VAR_TYPE_BOOLEAN);
	DECL_MENU_VAR(SYSTEM_MENU_ID_IO_SETTIGNS, s7, STR_CONTROL_INV, &g_settings.control_invert_mask, VAR_TYPE_UINT8);
#if ENCODERS > 0
	DECL_MENU_VAR(SYSTEM_MENU_ID_IO_SETTIGNS, s8, STR_ENC_P_INV, &g_settings.encoders_pulse_invert_mask, VAR_TYPE_UINT8);
	DECL_MENU_VAR(SYSTEM_MENU_ID_IO_SETTIGNS, s9, STR_ENC_D_INV, &g_settings.encoders_dir_invert_mask, VAR_TYPE_UINT8);
#endif
#ifdef ENABLE_STEPPERS_DISABLE_TIMEOUT
	DECL_MENU_VAR(SYSTEM_MENU_ID_IO_SETTIGNS, s10, STR_STEPPER_TIMEOUT, &g_settings.step_disable_timeout, VAR_TYPE_UINT16);
#endif

	// append homing settings menu
	DECL_MENU(SYSTEM_MENU_ID_HOMING, SYSTEM_MENU_ID_SETTINGS, STR_HOMING);

	DECL_MENU_VAR(SYSTEM_MENU_ID_HOMING, s20, STR_SOFTLIMITS, &g_settings.soft_limits_enabled, VAR_TYPE_BOOLEAN);
	DECL_MENU_VAR(SYSTEM_MENU_ID_HOMING, s21, STR_HARDLIMITS, &g_settings.hard_limits_enabled, VAR_TYPE_BOOLEAN);
	DECL_MENU_VAR(SYSTEM_MENU_ID_HOMING, s22, STR_ENABLE_HOMING, &g_settings.homing_enabled, VAR_TYPE_BOOLEAN);
	DECL_MENU_VAR(SYSTEM_MENU_ID_HOMING, s23, STR_DIR_INV_MASK, &g_settings.homing_dir_invert_mask, VAR_TYPE_UINT8);
	DECL_MENU_VAR(SYSTEM_MENU_ID_HOMING, s24, STR_SLOW_FEED, &g_settings.homing_slow_feed_rate, VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_HOMING, s25, STR_FAST_FEED, &g_settings.homing_fast_feed_rate, VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_HOMING, s26, STR_DEBOUNCEMS, &g_settings.debounce_ms, VAR_TYPE_UINT16);
	DECL_MENU_VAR(SYSTEM_MENU_ID_HOMING, s27, STR_OFFSET, &g_settings.homing_offset, VAR_TYPE_FLOAT);

	// append steppers settings menu
	DECL_MENU(SYSTEM_MENU_ID_STEPPER_SETTINGS, SYSTEM_MENU_ID_SETTINGS, STR_AXIS);
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s100, STR_STEPMM("X"), &g_settings.step_per_mm[0], VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s110, STR_VMAX("X"), &g_settings.max_feed_rate[0], VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s120, STR_ACCEL("X"), &g_settings.acceleration[0], VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s130, STR_MAX_DIST("X"), &g_settings.max_distance[0], VAR_TYPE_FLOAT);
#ifdef ENABLE_BACKLASH_COMPENSATION
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s140, STR_BACKLASH("X"), &g_settings.backlash_steps[0], VAR_TYPE_UINT16);
#endif

#if (AXIS_COUNT > 1)
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s101, STR_STEPMM("Y"), &g_settings.step_per_mm[1], VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s111, STR_VMAX("Y"), &g_settings.max_feed_rate[1], VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s121, STR_ACCEL("Y"), &g_settings.acceleration[1], VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s131, STR_MAX_DIST("Y"), &g_settings.max_distance[1], VAR_TYPE_FLOAT);
#ifdef ENABLE_BACKLASH_COMPENSATION
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s141, STR_BACKLASH("Y"), &g_settings.backlash_steps[1], VAR_TYPE_UINT16);
#endif
#endif
#if (AXIS_COUNT > 2)
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s102, STR_STEPMM("Z"), &g_settings.step_per_mm[2], VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s112, STR_VMAX("Z"), &g_settings.max_feed_rate[2], VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s122, STR_ACCEL("Z"), &g_settings.acceleration[2], VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s132, STR_MAX_DIST("Z"), &g_settings.max_distance[2], VAR_TYPE_FLOAT);
#ifdef ENABLE_BACKLASH_COMPENSATION
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s142, STR_BACKLASH("Z"), &g_settings.backlash_steps[2], VAR_TYPE_UINT16);
#endif
#endif
#if (AXIS_COUNT > 3)
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s103, STR_STEPMM("A"), &g_settings.step_per_mm[3], VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s113, STR_VMAX("A"), &g_settings.max_feed_rate[3], VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s123, STR_ACCEL("A"), &g_settings.acceleration[3], VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s133, STR_MAX_DIST("A"), &g_settings.max_distance[3], VAR_TYPE_FLOAT);
#ifdef ENABLE_BACKLASH_COMPENSATION
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s143, STR_BACKLASH("A"), &g_settings.backlash_steps[3], VAR_TYPE_UINT16);
#endif
#endif
#if (AXIS_COUNT > 4)
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s104, STR_STEPMM("B"), &g_settings.step_per_mm[4], VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s114, STR_VMAX("B"), &g_settings.max_feed_rate[4], VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s124, STR_ACCEL("B"), &g_settings.acceleration[4], VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s134, STR_MAX_DIST("B"), &g_settings.max_distance[4], VAR_TYPE_FLOAT);
#ifdef ENABLE_BACKLASH_COMPENSATION
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s144, STR_BACKLASH("B"), &g_settings.backlash_steps[4], VAR_TYPE_UINT16);
#endif
#endif
#if (AXIS_COUNT > 5)
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s105, STR_STEPMM("C"), &g_settings.step_per_mm[5], VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s115, STR_VMAX("C"), &g_settings.max_feed_rate[5], VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s125, STR_ACCEL("C"), &g_settings.acceleration[5], VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s135, STR_MAX_DIST("C"), &g_settings.max_distance[5], VAR_TYPE_FLOAT);
#ifdef ENABLE_BACKLASH_COMENSATION
	DECL_MENU_VAR(SYSTEM_MENU_ID_STEPPER_SETTINGS, s145, STR_BACKLASH("C"), &g_settings.backlash_steps[5], VAR_TYPE_UINT16);
#endif
#endif

#if (defined(ENABLE_SKEW_COMPENSATION) || defined(KINEMATICS_VARS_SYSTEM_MENU_INIT))
	DECL_MENU(SYSTEM_MENU_ID_KINEMATIC_SETTINGS, SYSTEM_MENU_ID_SETTINGS, STR_KINEMATICS);
#ifdef ENABLE_SKEW_COMPENSATION
	DECL_MENU_VAR(SYSTEM_MENU_ID_KINEMATIC_SETTINGS, s37, STR_SKEW_FACTOR("XY"), &g_settings.skew_xy_factor, VAR_TYPE_FLOAT);
#ifndef SKEW_COMPENSATION_XY_ONLY
	DECL_MENU_VAR(SYSTEM_MENU_ID_KINEMATIC_SETTINGS, s38, STR_SKEW_FACTOR("XZ"), &g_settings.skew_xz_factor, VAR_TYPE_FLOAT);
	DECL_MENU_VAR(SYSTEM_MENU_ID_KINEMATIC_SETTINGS, s39, STR_SKEW_FACTOR("YZ"), &g_settings.skew_yz_factor, VAR_TYPE_FLOAT);
#endif
#endif
#ifdef KINEMATICS_VARS_SYSTEM_MENU_INIT
	KINEMATICS_VARS_SYSTEM_MENU_INIT
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

	// kill alarm is active
	if (cnc_get_exec_state(EXEC_INTERLOCKING_FAIL) || cnc_has_alarm())
	{
		// never go idle
		g_system_menu.action_timeout = mcu_millis() + SYSTEM_MENU_MODAL_POPUP_MS;
		g_system_menu.flags |= SYSTEM_MENU_MODE_REDRAW;
		// leave. ignore all actions
		return;
	}

	uint32_t timestamp = mcu_millis();

	// forces a second redraw after flushing all commands
	if (g_system_menu.flags & SYSTEM_MENU_MODE_DELAYED_REDRAW)
	{
		if (g_system_menu.action_timeout < timestamp)
		{
			g_system_menu.flags &= ~SYSTEM_MENU_MODE_DELAYED_REDRAW;
			g_system_menu.flags |= SYSTEM_MENU_MODE_REDRAW;
			system_menu_action_timeout(SYSTEM_MENU_GO_IDLE_MS);
		}
	}

	// with a modal popup active actions will be locked
	// after the popup show time is over return to normal flow
	if (g_system_menu.flags & SYSTEM_MENU_MODE_MODAL_POPUP)
	{
		// popup timeout occurred
		if (g_system_menu.action_timeout < timestamp)
		{
			g_system_menu.flags &= ~SYSTEM_MENU_MODE_MODAL_POPUP;
			g_system_menu.flags |= SYSTEM_MENU_MODE_REDRAW;
			system_menu_action_timeout(SYSTEM_MENU_GO_IDLE_MS);
		}
		else
		{
			// prevent redraw
			g_system_menu.flags &= ~SYSTEM_MENU_MODE_REDRAW;
		}
		return;
	}

	if (action == SYSTEM_MENU_ACTION_NONE)
	{
		// idle timeout occurred
		if (g_system_menu.action_timeout < timestamp)
		{
			// system_menu_go_idle();
			currentmenu = g_system_menu.current_menu = SYSTEM_MENU_ID_IDLE;
			currentindex = g_system_menu.current_index = 0;
			g_system_menu.flags = SYSTEM_MENU_MODE_REDRAW;
			system_menu_action_timeout(SYSTEM_MENU_REDRAW_IDLE_MS);
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
		system_menu_action_timeout(SYSTEM_MENU_GO_IDLE_MS);

		// checks if the menu has a custom action callback
		if (menupage->page_action)
		{
			// if the custom action callback returns 1 it was handled
			// else continue
			if (menupage->page_action(action))
			{
				return;
			}
		}

		// if it's over the nav back element
		if (currentindex < 0 || g_system_menu.current_multiplier < 0)
		{
			if (system_menu_action_nav_back(action, menupage))
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
				if (menuitem.item_action(action, &menuitem))
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
			// pass the code to a custom action implementation
			system_menu_action_custom_code(action);
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

		if (cnc_get_exec_state(EXEC_INTERLOCKING_FAIL) || cnc_has_alarm())
		{
			system_menu_render_alarm();
			return;
		}

		MENU_LOOP(g_system_menu.menu_entry, menu_page)
		{
			if (menu_page->menu_id == g_system_menu.current_menu)
			{
				// if menu has custom render
				if (menu_page->page_render)
				{
					menu_page->page_render(render_flags);
					return;
				}

				// renders header
				if (!item_index)
				{
					char buffer[SYSTEM_MENU_MAX_STR_LEN];
					memset(buffer, 0, sizeof(buffer));
					rom_strcpy((char *)buffer, (const char *)menu_page->page_label);
					system_menu_render_header(buffer);
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

void system_menu_show_modal_popup(uint32_t timeout, const char *__s)
{
	// prevents redraw
	g_system_menu.flags &= ~(SYSTEM_MENU_MODE_REDRAW | SYSTEM_MENU_MODE_DELAYED_REDRAW);
	// renders the popup
	system_menu_render_modal_popup(__s);
	// locks the popup action
	g_system_menu.flags |= SYSTEM_MENU_MODE_MODAL_POPUP;
	system_menu_action_timeout(timeout);
}

void system_menu_action_timeout(uint32_t delay)
{
	// if the arg is 0 then the update is done right away
	g_system_menu.action_timeout = (delay) ? (delay + mcu_millis()) : 0;
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
	g_system_menu.current_menu = SYSTEM_MENU_ID_STARTUP;
	g_system_menu.current_index = 0;
	g_system_menu.total_items = 0;

	g_system_menu.current_multiplier = 0;
	// forces imediate render
	g_system_menu.flags = SYSTEM_MENU_MODE_REDRAW;
	system_menu_action_timeout(SYSTEM_MENU_REDRAW_STARTUP_MS);
}

void system_menu_go_idle(void)
{
	// idle menu
	g_system_menu.current_menu = SYSTEM_MENU_ID_IDLE;
	g_system_menu.current_index = 0;
	g_system_menu.total_items = 0;
	g_system_menu.current_multiplier = 0;
	// forces imediate render
	g_system_menu.flags = SYSTEM_MENU_MODE_REDRAW;
	system_menu_action_timeout(SYSTEM_MENU_GO_IDLE_MS);
}

/**
 * Helper µCNC commands callbacks
 * **/

// calls a new menu
bool system_menu_action_goto(uint8_t action, system_menu_item_t *item)
{
	if (action == SYSTEM_MENU_ACTION_SELECT && item)
	{
		system_menu_goto((uint8_t)VARG_CONST(item->action_arg));
		return true;
	}
	return false;
}

bool system_menu_action_rt_cmd(uint8_t action, system_menu_item_t *item)
{
	if (action == SYSTEM_MENU_ACTION_SELECT && item)
	{
		cnc_call_rt_command((uint8_t)VARG_CONST(item->action_arg));
		char buffer[SYSTEM_MENU_MAX_STR_LEN];
		memset(buffer, 0, sizeof(buffer));
		rom_strcpy((char *)buffer, __romstr__(STR_RT_CMD_SENT));
		system_menu_show_modal_popup(SYSTEM_MENU_MODAL_POPUP_MS, buffer);
		return true;
	}
	return false;
}

bool system_menu_action_serial_cmd(uint8_t action, system_menu_item_t *item)
{
	if (action == SYSTEM_MENU_ACTION_SELECT && item)
	{
		if (grbl_stream_write_available() > 20)
		{
			char buffer[SYSTEM_MENU_MAX_STR_LEN];
			memset(buffer, 0, sizeof(buffer));
			if (system_menu_send_cmd((const char *)item->action_arg) == STATUS_OK)
			{
				rom_strcpy((char *)buffer, __romstr__(STR_CMD_SENT));
			}
			else
			{
				rom_strcpy((char *)buffer, __romstr__(STR_CMD_NOTSENT));
			}

			system_menu_show_modal_popup(SYSTEM_MENU_MODAL_POPUP_MS, buffer);
		}
		return true;
	}
	return false;
}

static bool system_menu_action_overrides(uint8_t action, system_menu_item_t *item)
{
	if (!item)
	{
		return false;
	}

	if (action == SYSTEM_MENU_ACTION_SELECT)
	{
		g_system_menu.flags ^= SYSTEM_MENU_MODE_SIMPLE_EDIT;
		return true;
	}
	else if (g_system_menu.flags & SYSTEM_MENU_MODE_SIMPLE_EDIT)
	{
		uint8_t override = (uint8_t)VARG_CONST(item->action_arg);
		switch (action)
		{
		case SYSTEM_MENU_ACTION_NEXT:
			switch (override)
			{
			case 'f':
				cnc_call_rt_command(CMD_CODE_FEED_INC_FINE);
				break;
			case 's':
				cnc_call_rt_command(CMD_CODE_SPINDLE_INC_FINE);
				break;
			}
			break;
		case SYSTEM_MENU_ACTION_PREV:
			switch (override)
			{
			case 'f':
				cnc_call_rt_command(CMD_CODE_FEED_DEC_FINE);
				break;
			case 's':
				cnc_call_rt_command(CMD_CODE_SPINDLE_DEC_FINE);
				break;
			}
			break;
		default:
			// allow to propagate
			return false;
		}

		return true;
	}
	return false;
}

static bool system_menu_action_jog(uint8_t action, system_menu_item_t *item)
{
	if (!item)
	{
		return false;
	}

	if (action == SYSTEM_MENU_ACTION_SELECT)
	{
		if (cnc_get_exec_state(EXEC_JOG))
		{
			cnc_call_rt_command(CMD_CODE_JOG_CANCEL);
		}
		g_system_menu.flags ^= SYSTEM_MENU_MODE_SIMPLE_EDIT;
		return true;
	}
	else if (g_system_menu.flags & SYSTEM_MENU_MODE_SIMPLE_EDIT)
	{
		// one jog command at time
		if (grbl_stream_write_available() > 32)
		{
			char buffer[SYSTEM_MENU_MAX_STR_LEN];
			memset(buffer, 0, sizeof(buffer));
			rom_strcpy((char *)buffer, __romstr__("$J=G91"));
			char *ptr = buffer;
			// search for the end of string
			while (*++ptr)
				;
			// replaces the axis letter
			*ptr++ = *((uint8_t *)item->action_arg);
			switch (action)
			{
			case SYSTEM_MENU_ACTION_NEXT:
				system_menu_flt_to_str(ptr, g_system_menu_jog_distance);
				break;
			case SYSTEM_MENU_ACTION_PREV:
				system_menu_flt_to_str(ptr, -g_system_menu_jog_distance);
				break;
			default:
				// allow to propagate
				return false;
			}
			// search for the end of string
			while (*++ptr)
				;
			*ptr++ = 'F';
			system_menu_flt_to_str(ptr, g_system_menu_jog_feed);
			while (*++ptr)
				;
			*ptr++ = '\r';
			DBGMSG("%s", buffer);
			if (system_menu_send_cmd(buffer) != STATUS_OK)
			{
				rom_strcpy((char *)buffer, __romstr__(STR_CMD_NOTSENT));
				system_menu_show_modal_popup(SYSTEM_MENU_MODAL_POPUP_MS, buffer);
			}
		}
		return true;
	}
	return false;
}

static bool system_menu_action_settings_cmd(uint8_t action, system_menu_item_t *item)
{
	char buffer[SYSTEM_MENU_MAX_STR_LEN];
	memset(buffer, 0, sizeof(buffer));
	if (action == SYSTEM_MENU_ACTION_SELECT)
	{
		uint8_t settings_action = (uint8_t)VARG_CONST(item->action_arg);
		switch (settings_action)
		{
		case 0:
			settings_init();
			rom_strcpy((char *)buffer, __romstr__(STR_SETTINGS_LOADED));
			break;
		case 1:
			settings_save(SETTINGS_ADDRESS_OFFSET, (uint8_t *)&g_settings, (uint8_t)sizeof(settings_t));
			rom_strcpy((char *)buffer, __romstr__(STR_SETTINGS_SAVED));
			break;
		case 2:
			settings_reset(false);
			rom_strcpy((char *)buffer, __romstr__(STR_SETTINGS_RESET));
			break;
		default:
			break;
		}
		system_menu_show_modal_popup(SYSTEM_MENU_MODAL_POPUP_MS, buffer);
		return true;
	}
	return false;
}

static bool system_menu_action_nav_back(uint8_t action, const system_menu_page_t *menu)
{
	if (action == SYSTEM_MENU_ACTION_SELECT)
	{
		// direct to page nav back action
		// if (item->action_arg)
		// {
		// 	return system_menu_action_goto(action, item);
		// }

		if (g_system_menu.current_multiplier < 0)
		{
			g_system_menu.current_multiplier = 0;
			g_system_menu.flags &= ~(SYSTEM_MENU_MODE_EDIT | SYSTEM_MENU_MODE_MODIFY);
			return true;
		}

		if (g_system_menu.current_index < 0)
		{
			if (menu)
			{
				system_menu_goto(menu->parent_id);
				return true;
			}
		}

		system_menu_go_idle();
		system_menu_goto(SYSTEM_MENU_ID_IDLE);
		return true;
	}
	return false;
}

bool system_menu_action_edit_simple(uint8_t action, system_menu_item_t *item)
{
	if (!item)
	{
		return false;
	}

	if (action == SYSTEM_MENU_ACTION_SELECT)
	{
		g_system_menu.flags ^= SYSTEM_MENU_MODE_SIMPLE_EDIT;
		return true;
	}
	else if (g_system_menu.flags & SYSTEM_MENU_MODE_SIMPLE_EDIT)
	{
		uint8_t vartype = (uint8_t)VARG_CONST(item->action_arg);

		bool inc = true;
		switch (action)
		{
		case SYSTEM_MENU_ACTION_NEXT:
			break;
		case SYSTEM_MENU_ACTION_PREV:
			inc = false;
			break;
		default:
			// allow to propagate
			return false;
		}

		switch (vartype)
		{
		case VAR_TYPE_BOOLEAN:
			(*(bool *)item->argptr) = inc;
			break;
		case VAR_TYPE_INT8:
		case VAR_TYPE_UINT8:
			(*(uint8_t *)item->argptr) += (inc) ? 1 : -1;
			(*(uint8_t *)item->argptr) = CLAMP(0, (*(uint8_t *)item->argptr), 0xFF);
			break;
		case VAR_TYPE_INT16:
		case VAR_TYPE_UINT16:
			(*(uint16_t *)item->argptr) += (inc) ? 1 : -1;
			(*(uint16_t *)item->argptr) = CLAMP(0, (*(uint16_t *)item->argptr), 0xFFFF);
			break;
		case VAR_TYPE_INT32:
		case VAR_TYPE_UINT32:
			(*(uint32_t *)item->argptr) += (inc) ? 1 : -1;
			(*(uint32_t *)item->argptr) = CLAMP(0, (*(uint32_t *)item->argptr), 0xFFFFFFFF);
			break;
		case VAR_TYPE_FLOAT:
			(*(float *)item->argptr) += (inc) ? 1 : -1;
			(*(float *)item->argptr) = CLAMP(__FLT_MIN__, (*(float *)item->argptr), __FLT_MAX__);
			break;
		}

		return true;
	}
	return false;
}

bool system_menu_action_edit(uint8_t action, system_menu_item_t *item)
{
	if (!item)
	{
		return false;
	}

	uint8_t vartype = (uint8_t)VARG_CONST(item->action_arg);
	uint8_t flags = g_system_menu.flags;
	int8_t currentmult = g_system_menu.current_multiplier;
	float modifier = 0;

	switch (action)
	{
	case SYSTEM_MENU_ACTION_SELECT:
		if (flags & SYSTEM_MENU_MODE_EDIT)
		{
			// toogle modify mode
			g_system_menu.flags ^= SYSTEM_MENU_MODE_MODIFY;
		}
		g_system_menu.flags |= SYSTEM_MENU_MODE_EDIT;
		return true;
	case SYSTEM_MENU_ACTION_PREV:
	case SYSTEM_MENU_ACTION_NEXT:
		if (flags & SYSTEM_MENU_MODE_MODIFY)
		{
			// increment var by multiplier
			if (!item->argptr)
			{
				// passthrough action
				return false;
			}

			switch (vartype)
			{
			case VAR_TYPE_BOOLEAN:
				modifier = (action == SYSTEM_MENU_ACTION_NEXT) ? 1 : 0;
				break;
			case VAR_TYPE_FLOAT:
				modifier = (action == SYSTEM_MENU_ACTION_NEXT) ? powf(10.0f, (currentmult - 3)) : -powf(10.0f, (currentmult - 3));
				break;
			default:
				modifier = (action == SYSTEM_MENU_ACTION_NEXT) ? powf(10.0f, currentmult) : -powf(10.0f, currentmult);
				break;
			}
		}
		else if (flags & SYSTEM_MENU_MODE_EDIT)
		{
			currentmult += (action == SYSTEM_MENU_ACTION_NEXT) ? 1 : -1;
		}
		else
		{
			// passthrough action
			return false;
		}
		break;
	case SYSTEM_MENU_ACTION_CHAR_INPUT('0'):
	case SYSTEM_MENU_ACTION_CHAR_INPUT('1'):
	case SYSTEM_MENU_ACTION_CHAR_INPUT('2'):
	case SYSTEM_MENU_ACTION_CHAR_INPUT('3'):
	case SYSTEM_MENU_ACTION_CHAR_INPUT('4'):
	case SYSTEM_MENU_ACTION_CHAR_INPUT('5'):
	case SYSTEM_MENU_ACTION_CHAR_INPUT('6'):
	case SYSTEM_MENU_ACTION_CHAR_INPUT('7'):
	case SYSTEM_MENU_ACTION_CHAR_INPUT('8'):
	case SYSTEM_MENU_ACTION_CHAR_INPUT('9'):
	{
		int digit = action - SYSTEM_MENU_ACTION_CHAR_INPUT('0');
		int current_digit;
		switch (vartype)
		{
		case VAR_TYPE_BOOLEAN:
			// Set boolean
			(*(bool *)item->argptr) = digit != 0;
			// Boolean has only one digit
			g_system_menu.current_multiplier = 0;
			break;
		case VAR_TYPE_FLOAT:
			// Modify digit
			current_digit = (int)((*(float *)item->argptr) / powf(10.0f, (currentmult - 3))) % 10;
			(*(float *)item->argptr) += (digit - current_digit) * powf(10.0f, (currentmult - 3));
			// Clamp
			(*(float *)item->argptr) = CLAMP(__FLT_MIN__, (*(float *)item->argptr), __FLT_MAX__);
			g_system_menu.current_multiplier = CLAMP(-1, currentmult + 1, 9);
			break;
		case VAR_TYPE_INT8:
		case VAR_TYPE_UINT8:
			// Modify digit
			current_digit = ((*(uint8_t *)item->argptr) / (int)powf(10.0f, currentmult)) % 10;
			(*(uint8_t *)item->argptr) += (digit - current_digit) * powf(10.0f, currentmult);
			// Clamp
			(*(uint8_t *)item->argptr) = CLAMP(0, (*(uint8_t *)item->argptr), 0xFF);
			g_system_menu.current_multiplier = CLAMP(-1, currentmult + 1, 2);
			break;
		case VAR_TYPE_INT16:
		case VAR_TYPE_UINT16:
			current_digit = ((*(uint16_t *)item->argptr) / (int)powf(10.0f, currentmult)) % 10;
			(*(uint16_t *)item->argptr) += (digit - current_digit) * powf(10.0f, currentmult);
			// Clamp
			(*(uint16_t *)item->argptr) = CLAMP(0, (*(uint16_t *)item->argptr), 0xFFFF);
			g_system_menu.current_multiplier = CLAMP(-1, currentmult + 1, 4);
			break;
		case VAR_TYPE_INT32:
		case VAR_TYPE_UINT32:
			// Modify digit
			current_digit = ((*(uint32_t *)item->argptr) / (int)powf(10.0f, currentmult)) % 10;
			(*(uint32_t *)item->argptr) += (digit - current_digit) * powf(10.0f, currentmult);
			// Clamp
			(*(uint32_t *)item->argptr) = CLAMP(0, (*(uint32_t *)item->argptr), 0xFFFFFFFF);
			g_system_menu.current_multiplier = CLAMP(-1, currentmult + 1, 9);
			break;
		default:
			break;
		}
	}
		return true;
	default:
		// allow to propagate
		return false;
	}

	// modify mode enabled
	if (flags & SYSTEM_MENU_MODE_MODIFY)
	{
		// adds the multiplier
		switch (vartype)
		{
		case VAR_TYPE_BOOLEAN:
			(*(bool *)item->argptr) = (modifier != 0);
			break;
		case VAR_TYPE_INT8:
		case VAR_TYPE_UINT8:
			(*(uint8_t *)item->argptr) += (uint8_t)modifier;
			(*(uint8_t *)item->argptr) = CLAMP(0, (*(uint8_t *)item->argptr), 0xFF);
			break;
		case VAR_TYPE_INT16:
		case VAR_TYPE_UINT16:
			(*(uint16_t *)item->argptr) += (uint16_t)modifier;
			(*(uint16_t *)item->argptr) = CLAMP(0, (*(uint16_t *)item->argptr), 0xFFFF);
			break;
		case VAR_TYPE_INT32:
		case VAR_TYPE_UINT32:
			(*(uint32_t *)item->argptr) += (uint32_t)modifier;
			(*(uint32_t *)item->argptr) = CLAMP(0, (*(uint32_t *)item->argptr), 0xFFFFFFFF);
			break;
		case VAR_TYPE_FLOAT:
			(*(float *)item->argptr) += modifier;
			(*(float *)item->argptr) = CLAMP(__FLT_MIN__, (*(float *)item->argptr), __FLT_MAX__);
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
			break;
		case VAR_TYPE_INT32:
		case VAR_TYPE_UINT32:
		case VAR_TYPE_FLOAT:
			g_system_menu.current_multiplier = CLAMP(-1, currentmult, 9);
			break;
		}
	}

	// stop action propagation
	return true;
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

void __attribute__((weak)) system_menu_render_modal_popup(const char *__s)
{
	// renders the modal popup message
}

// this needs to be implemented using a serial stream
uint8_t __attribute__((weak)) system_menu_send_cmd(const char *__s)
{
	return STATUS_STREAM_FAILED;
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

void system_menu_item_render_var_arg(uint8_t render_flags, system_menu_item_t *item)
{
	uint8_t vartype = (uint8_t)VARG_CONST(item->render_arg);
	char buffer[SYSTEM_MENU_MAX_STR_LEN];
	memset(buffer, 0, sizeof(buffer));
	char *buff_ptr = buffer;
	switch (vartype)
	{
	case VAR_TYPE_BOOLEAN:
		buffer[0] = (*((bool *)item->argptr)) ? '1' : '0';
		buffer[1] = 0;
		break;
	case VAR_TYPE_INT8:
	case VAR_TYPE_UINT8:
		system_menu_int_to_str(buffer, (uint32_t)*((uint8_t *)item->argptr));
		break;
	case VAR_TYPE_INT16:
	case VAR_TYPE_UINT16:
		system_menu_int_to_str(buffer, (uint32_t)*((uint16_t *)item->argptr));
		break;
	case VAR_TYPE_INT32:
	case VAR_TYPE_UINT32:
		system_menu_int_to_str(buffer, (uint32_t)*((uint32_t *)item->argptr));
		break;
	case VAR_TYPE_FLOAT:
		system_menu_flt_to_str(buffer, *((float *)item->argptr));
		break;
	default:
		buff_ptr = item->argptr;
		break;
	}

	system_menu_item_render_arg(render_flags, (const char *)buff_ptr);
}

static void system_menu_render_axis_position(uint8_t render_flags, system_menu_item_t *item)
{
	if ((render_flags & (SYSTEM_MENU_MODE_SELECT | SYSTEM_MENU_MODE_SIMPLE_EDIT)) == (SYSTEM_MENU_MODE_SELECT | SYSTEM_MENU_MODE_SIMPLE_EDIT))
	{
		// force auto render in this state
		g_system_menu.flags |= SYSTEM_MENU_MODE_DELAYED_REDRAW;
		system_menu_action_timeout(SYSTEM_MENU_REDRAW_IDLE_MS);

		float axis[MAX(AXIS_COUNT, 3)];
#if AXIS_COUNT < 3
		memset(axis, 0, sizeof(axis));
#endif
		int32_t steppos[STEPPER_COUNT];
		itp_get_rt_position(steppos);
		kinematics_steps_to_coordinates(steppos, axis);
		// X = 0
		char axis_letter = *((char *)item->action_arg);
		uint8_t axis_index = (axis_letter >= 'X') ? (axis_letter - 'X') : (3 + axis_letter - 'A');

		char buffer[SYSTEM_MENU_MAX_STR_LEN];
		memset(buffer, 0, sizeof(buffer));
		char *buff_ptr = buffer;
		system_menu_flt_to_str(buff_ptr, axis[axis_index]);

		system_menu_item_render_arg(render_flags, buffer);
		// system_menu_item_render_arg(render_flags, "x");
	}
}

/**
 * Overridable system menu actions to be implemented for the user input system
 * **/

void __attribute__((weak)) system_menu_action_custom_code(uint8_t action)
{
	// used to implement custom action codes for different user interface systems (keypads, keyboards, screens, etc..)
}

// allow modification of a page render after the page is initialized
void system_menu_set_render_callback(uint8_t menu_id, system_menu_page_render_cb callback)
{
	MENU_LOOP(g_system_menu.menu_entry, menu_page)
	{
		if (menu_page->menu_id == menu_id)
		{
			menu_page->page_render = callback;
		}
	}
}

// allow modification of a page action after the page is initialized
void system_menu_set_action_callback(uint8_t menu_id, system_menu_page_action_cb callback)
{
	MENU_LOOP(g_system_menu.menu_entry, menu_page)
	{
		if (menu_page->menu_id == menu_id)
		{
			menu_page->page_action = callback;
		}
	}
}
