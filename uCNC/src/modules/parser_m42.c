/*
    Name: parser_m42.c
    Description: Implements a parser extension for Marlin M42 for µCNC.
        This is only a partial implementation. Only the state S of the pin will be definable

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 18/01/2022

    µCNC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version. Please see <http://www.gnu.org/licenses/>

    µCNC is distributed WITHOUT ANY WARRANTY;
    Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the	GNU General Public License for more details.
*/

#include "../cnc.h"
#include <stdbool.h>

#ifdef ENABLE_PARSER_MODULES
// this ID must be unique for each code
#define M42 1042

uint8_t m42_parse(unsigned char c, uint8_t word, uint8_t error, float value, parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
uint8_t m42_exec(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);

CREATE_LISTENER(gcode_parse_delegate, m42_parse);
CREATE_LISTENER(gcode_exec_delegate, m42_exec);

// this just parses and acceps the code
uint8_t m42_parse(unsigned char word, uint8_t code, uint8_t error, float value, parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
    if (word == 'M' && code == 42)
    {
        if (cmd->group_extended != 0)
        {
            // there is a collision of custom gcode commands (only one per line can be processed)
            return STATUS_GCODE_MODAL_GROUP_VIOLATION;
        }
        // tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
        cmd->group_extended = M42;
        return STATUS_OK;
    }

    // if this is not catched by this parser, just send back the error so other extenders can process it
    return error;
}

// if all conventions changes this must be updated
#define PWM0_ID 24
#define DOUT0_ID 46
#define DOUT31_ID 71

// this actually performs 2 steps in 1 (validation and execution)
uint8_t m42_exec(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
    if (cmd->group_extended == M42)
    {
        if (CHECKFLAG(cmd->words, (GCODE_WORD_S | GCODE_WORD_P)) != (GCODE_WORD_S | GCODE_WORD_P))
        {
            return STATUS_GCODE_VALUE_WORD_MISSING;
        }

        if (words->p >= PWM0_ID && words->p <= DOUT31_ID)
        {
            if (words->p >= DOUT0_ID)
            {
                io_set_output(words->p, (words->s != 0));
            }
            else
            {
                io_set_pwm(words->p, (uint8_t)CLAMP(0, words->s, 255));
            }
        }

        return STATUS_OK;
    }

    return STATUS_GCODE_EXTENDED_UNSUPPORTED;
}

#endif
