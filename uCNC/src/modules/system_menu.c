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

#define MENU_RENDER 1

#ifdef ENABLE_SYSTEM_MENU
DECL_MENU_ACTION(hold, "Hold", system_menu_rt_command, CMD_CODE_FEED_HOLD);
DECL_MENU_ACTION(resume, "Resume", system_menu_rt_command, RT_CMD_CYCLE_START);
DECL_MENU_ACTION(home, "Home", system_menu_serial_command, "$H");
DECL_MENU(system_menu_main, "Main menu", NULL, 5, &hold, &resume, &home);

system_menu_t g_system_menu;
#endif

static system_menu_item_t *system_menu_get_item(system_menu_walker_t *menu, uint8_t index)
{
    MENU_WALKER(menu, menu_item)
    {
        // in the item group
        if (index < menu_item->item_count)
        {
            return menu_item->items[index];
        }
        else
        {
            // must be in one extended menu
            index - menu_item->item_count;
            // get the next set of extended menu entries
        }
    }

    // could not find
    // return empty item
    return NULL;
}

void system_menu_append(system_menu_walker_t *parent_menu, system_menu_walker_t *extended_menu)
{
    if (!parent_menu)
    {
        parent_menu = &system_menu_main;
    }

    system_menu_walker_t *ptr = parent_menu;

    while (ptr->extended != NULL)
    {
        ptr = ptr->extended;
    }

    ptr->extended = extended_menu;
}

void system_menu_reset(void)
{
    g_system_menu.active_menu = NULL;
    g_system_menu.current_index = 0;
    g_system_menu.flags = MENU_RENDER;
}

void system_menu_action(uint8_t action)
{
    switch (action)
    {
    case SYSTEM_MENU_ACTION_SELECT:
        if (!g_system_menu.active_menu)
        {
            g_system_menu.active_menu = &system_menu_main;
            g_system_menu.current_index = 0;
        }
        else
        {
            // if inside a menu get get the arg
            system_menu_item_t *next = system_menu_get_menu_item(g_system_menu.active_menu, g_system_menu.current_index);
            if (next)
            {
                // found custom
                if (next->action)
                {
                    next->action(next->action_arg);
                    return;
                }
            }
            else
            {
                // something went wrong (menu not found)
                // return to home screen
                g_system_menu.active_menu = NULL;
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
        break;
    }

    g_system_menu.flags |= MENU_RENDER;
}

/**
 * Helper µCNC commands callbacks
 * **/

// calls a new menu
void system_menu_goto(void *cmd)
{
    g_system_menu.active_menu = cmd;
    g_system_menu.current_index = 0;
    g_system_menu.flags |= MENU_RENDER;
}

void system_menu_rt_command(void *cmd)
{
    cnc_call_rt_command((uint8_t)cmd);
}

void system_menu_serial_command(void *cmd)
{
    serial_inject_cmd((const char *)cmd, false);
}