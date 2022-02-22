/*
    Name: module.h
    Description: Module extensions for µCNC.
    All entry points for extending µCNC core functionalities are declared in this module

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 21-02-2022

    µCNC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version. Please see <http://www.gnu.org/licenses/>

    µCNC is distributed WITHOUT ANY WARRANTY;
    Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the	GNU General Public License for more details.
*/

#ifndef MODULE_H
#define MODULE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../cnc_config.h"
#include "core/parser.h"

#define EVENT(delegate)              \
    typedef struct delegate##_event_ \
    {                                \
        delegate fptr;               \
        struct delegate##_ *next     \
    } delegate##_event_t;            \
    delegate##_event_t *
#define EVENT_TYPE(delegate) delegate##_event_t
#define CREATE_LISTENER(delegate, handler) __attribute__((used)) delegate##_event_t delegate##_##handler = {&handler, NULL}
#define ADD_LISTENER(delegate, handler, event) ({   \
    extern delegate##_event_t delegate##_##handler; \
    delegate##_event_t **p = &event;                \
    while ((*p) != NULL)                            \
    {                                               \
        (*p) = (*p)->next;                          \
    }                                               \
    (*p) = &delegate##_##handler;                   \
    (*p)->next = NULL;                              \
})

#ifdef ENABLE_PARSER_MODULES
    // defines a delegate function for the gcode parser handler
    typedef uint8_t (*gcode_parse_delegate)(unsigned char, uint8_t, uint8_t, float, parser_state_t *, parser_words_t *, parser_cmd_explicit_t *);
    // creates an event for the gcode_parse
    EVENT(gcode_parse_delegate)
    gcode_parse_event;
    uint8_t mod_gcode_parse_hook(unsigned char word, uint8_t code, uint8_t error, float value, parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);

    // defines a callback for the gcode exec handler
    typedef uint8_t (*gcode_exec_delegate)(parser_state_t *, parser_words_t *, parser_cmd_explicit_t *);
    // creates an event for the gcode_exec
    EVENT(gcode_exec_delegate)
    gcode_exec_event;
    uint8_t mod_gcode_exec_hook(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
#endif

#ifdef ENABLE_SCHEDULER_LOOP_MODULES
    typedef void (*rtc_tick_delegate)(void);
    EVENT(rtc_tick_delegate)
    rtc_tick_event;
    void mod_rtc_tick_hook(void);
#endif

#ifdef ENABLE_IO_MODULES
    void mod_input_change_hook(void);
    void mod_probe_enable_hook(void);
    void mod_probe_disable_hook(void);
#endif

    void mod_init(void);

#ifdef __cplusplus
}
#endif

#endif
