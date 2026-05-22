/*
    Name: parser_g96_g97.c
    Description: Implements a parser extension for LinuxCNC G7-G8 for µCNC.

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 04/01/2023

    µCNC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version. Please see <http://www.gnu.org/licenses/>

    µCNC is distributed WITHOUT ANY WARRANTY;
    Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the	GNU General Public License for more details.
*/

#include "../../cnc.h"
#include <stdint.h>
#include <stdbool.h>
#include <float.h>

#if defined(ENABLE_PARSER_MODULES) && defined(ENABLE_MOTION_CONTROL_MODULES)

#if (UCNC_MODULE_VERSION < 11605 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

static uint8_t spindle_css_mode;

// this ID must be unique for each code
#define G96 96
#define G97 97

bool g96_g97_parse(void *args);
bool g96_g97_reset(void *args);
bool g96_g97_modifier(void *args);
bool g96_g97_send_gcode_modes(void *args);
bool g96_g97_mc_line_calc_segments(void *args);
bool g96_g97_mc_line_segment_pre(void *args);

CREATE_EVENT_LISTENER(gcode_parse, g96_g97_parse);
CREATE_EVENT_LISTENER(parser_reset, g96_g97_reset);
CREATE_EVENT_LISTENER(gcode_exec_modifier, g96_g97_modifier);
CREATE_EVENT_LISTENER(protocol_send_gcode_modes, g96_g97_send_gcode_modes);
CREATE_EVENT_LISTENER(mc_line_calc_segments, g96_g97_mc_line_calc_segments);
CREATE_EVENT_LISTENER(mc_line_segment_pre, g96_g97_mc_line_segment_pre);

// this just parses and accepts the code
bool g96_g97_parse(void *args)
{
    gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;
    if (ptr->word == 'G')
    {
        // stops event propagation
        bool ok = true;

        switch (ptr->code)
        {
        case G96:
            spindle_css_mode = 1;
            break;
        case G97:
            /* code */
            spindle_css_mode = 0;
            break;
        default:
            // not able to catch this G code
            ok = false;
            break;
        }

        if (ok)
        {
            // stops event propagation
            *(ptr->error) = STATUS_OK;
            return EVENT_HANDLED;
        }
    }

    // if this is not catched by this parser, just send back the error so other extenders can process it
    return EVENT_CONTINUE;
}

bool g96_g97_reset(void *args)
{
    spindle_css_mode = 0; // default
    return EVENT_CONTINUE;
}

bool g96_g97_modifier(void *args)
{
    // TODO
    // store D parameter if available
    // permanently store G96/G97 state
    return EVENT_CONTINUE;
}

bool g96_g97_send_gcode_modes(void *args)
{
    serial_putc('G');
    serial_putc('9');
    if (spindle_css_mode)
    {
        serial_putc('6');
    }
    else
    {
        serial_putc('6');
    }
    serial_putc(' ');
    return EVENT_CONTINUE;
}

bool g96_g97_mc_line_calc_segments(void *args){
    // TODO
    // estimate the amount of line segments needed based on the distance and direction travelled
}

bool g96_g97_mc_line_segment_pre(void *args){
    // TODO
    // calculated the correct RPM based on the distance to the machine center
}

#endif

DECL_MODULE(g96_g97)
{
#ifdef ENABLE_PARSER_MODULES
    ADD_EVENT_LISTENER(gcode_parse, g96_g97_parse);
    ADD_EVENT_LISTENER(parser_reset, g96_g97_reset);
    ADD_EVENT_LISTENER(protocol_send_gcode_modes, g96_g97_send_gcode_modes);
    ADD_EVENT_LISTENER(gcode_exec_modifier, g96_g97_modifier);
#else
#error "Parser extensions are not enabled. G96-G97 code extension will not work."
#endif

#ifdef ENABLE_MOTION_CONTROL_MODULES
    ADD_EVENT_LISTENER(mc_line_calc_segments, g96_g97_mc_line_calc_segments);
    ADD_EVENT_LISTENER(mc_line_segment_pre, g96_g97_mc_line_segment_pre);
#else
#error "Motion control extensions are not enabled. G96-G97 code extension will not work."
#endif
}