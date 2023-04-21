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

#define CONST_VARG(X) ((void *)X)

    typedef struct system_menu_entry_
    {
        char menu_name[SYSTEM_MENU_MAX_STR_LEN];
        void *argptr;
        void (*render)(void *);
        void *render_arg;
        void (*action)(void *);
        void *action_arg;
    } system_menu_item_t;

    typedef struct system_menu_walker_
    {
        uint8_t menu_id;
        uint8_t parent_id;
        const char *label;
        uint8_t item_count;
        system_menu_item_t **items;
        struct system_menu_walker_ *extended;
    } system_menu_walker_t;

    typedef struct system_menu_
    {
        uint8_t flags;
        system_menu_walker_t *active_menu;
        uint8_t current_index;
    } system_menu_t;

#define DECL_MENU_ENTRY(name, strvalue, argptr, display_cb, display_cb_arg, action_cb, action_cb_arg) static const system_menu_item_t name __rom__ = {strvalue, argptr, display_cb, display_cb_arg, action_cb, action_cb_arg}

/**
 * Helper macros
 * **/
#define DECL_MENU_LABEL(name, strvalue) DECL_MENU_ENTRY(name, strvalue, NULL, NULL, NULL, NULL, NULL)
#define DECL_MENU_GOTO(name, strvalue, menu) DECL_MENU_ENTRY(name, strvalue, NULL, NULL, NULL, system_menu_action_goto, menu)
#define DECL_MENU_ACTION(name, strvalue, action_cb, action_cb_arg) DECL_MENU_ENTRY(name, strvalue, NULL, NULL, NULL, action_cb, action_cb_arg)

#define DECL_MENU(name, id, parent_id, label, count, ...) static system_menu_walker_t name = {id, parent_id, label, count, {__VA_ARGS__}, NULL}

#define MENU_WALKER(walker, item) for (system_menu_walker_t *item = walker; item != NULL; item = item->extended)
#define MENU_ITEM_WALKER(walker, iterator) for (uint8_t iterator = 0; iterator < walker->item_count; iterator++)

#ifdef ENABLE_SYSTEM_MENU
    extern system_menu_t g_system_menu;
#endif

    void system_menu_append(system_menu_walker_t *parent_menu, system_menu_walker_t *extended_menu);
    // void system_menu_render_header(system_menu_walker_t *menu);
    // void system_menu_render_content(system_menu_walker_t *menu);
    // void system_menu_render_footer(system_menu_walker_t *menu);
    void system_menu_render(void);
    void system_menu_reset(void);
    void system_menu_action(uint8_t action);

    /**
     * Helper µCNC action callbacks
     * **/
    void system_menu_action_goto(void *cmd);
    void system_menu_action_rt_cmd(void *cmd);
    void system_menu_action_serial_cmd(void *cmd);

    /**
     * Helper µCNC render callbacks
     * **/
    void system_menu_label_var(void *cmd);
    void system_menu_action_rt_cmd(void *cmd);
    void system_menu_action_serial_cmd(void *cmd);

#ifdef __cplusplus
}
#endif

#endif
