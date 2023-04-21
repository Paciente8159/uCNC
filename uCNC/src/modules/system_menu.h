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
#include <stdint.h>
#include <string.h>

#ifndef SYSTEM_MENU_MAX_STR_LEN
#define SYSTEM_MENU_MAX_STR_LEN 32
#endif

#define SYSTEM_MENU_ACTION_SELECT 1
#define SYSTEM_MENU_ACTION_NEXT 2
#define SYSTEM_MENU_ACTION_PREV 3

    typedef struct system_menu_entry_
    {
        char menu_name[SYSTEM_MENU_MAX_STR_LEN];
        void *argptr;
        void (*render)(void *);
        void *render_arg;
        void (*action)(void *);
        void *action_arg;
        struct system_menu_walker_ *parent;
    } system_menu_item_t;

    typedef struct system_menu_walker_
    {
        uint8_t item_count;
        system_menu_item_t **items;
        struct system_menu_walker_ *parent;
        struct system_menu_walker_ *extended;
    } system_menu_walker_t;

    typedef struct system_menu_
    {
        uint8_t flags;
        system_menu_walker_t *active_menu;
        uint8_t current_index;
    } system_menu_t;

#define DECL_MENU_ENTRY(name, strvalue, argptr, display_cb, display_cb_arg, action_cb, action_cb_arg, parent_menu) static const system_menu_item_t name __rom__ = {strvalue, argptr, display_cb, display_cb_arg, action_cb, action_cb_arg, parent_menu}

/**
 * Helper macros
 * **/
#define DECL_MENU_LABEL(name, strvalue) DECL_MENU_ENTRY(name, strvalue, NULL, NULL, NULL, NULL, NULL, NULL)
#define DECL_MENU_GOTO(name, strvalue, menu) DECL_MENU_ENTRY(name, strvalue, NULL, NULL, NULL, NULL, NULL, menu)
#define DECL_MENU_ACTION(name, strvalue, action_cb, action_cb_arg) DECL_MENU_ENTRY(name, strvalue, NULL, NULL, NULL, action_cb, action_cb_arg, NULL)
#define DECL_MENU(name, strvalue, parent_menu, count, ...) \
    DECL_MENU_LABEL(name##_label, strvalue);               \
    static system_menu_walker_t name = {count, {&name##_label, __VA_ARGS__}, parent_menu, NULL}

#define MENU_WALKER(walker, item) for (system_menu_walker_t *item = walker; item != NULL; item = item->extended)
#define MENU_ITEM_WALKER(walker, iterator) for (uint8_t iterator = 0; iterator < walker->item_count; iterator++)

#ifdef ENABLE_SYSTEM_MENU
    extern system_menu_t g_system_menu;
#endif

    void system_menu_append(system_menu_walker_t *parent_menu, system_menu_walker_t *extended_menu);
    void system_menu_render_header(system_menu_walker_t *menu);
    void system_menu_render_content(system_menu_walker_t *menu);
    void system_menu_render_footer(system_menu_walker_t *menu);
    void system_menu_render(void);
    void system_menu_reset(void);
    void system_menu_action(uint8_t action);

    /**
     * Helper µCNC commands callbacks
     * **/
    void system_menu_goto(void *cmd);
    void system_menu_rt_command(void *cmd);
    void system_menu_serial_command(void *cmd);

#ifdef __cplusplus
}
#endif

#endif
