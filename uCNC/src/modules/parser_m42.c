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

#ifdef ENABLE_PARSER_EXTENSIONS
//this ID must be unique for each code
#define M42 1

static uint8_t m42_parse(unsigned char c, uint8_t word, uint8_t error, float value, parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
static uint8_t m42_exec(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd);
parser_extender_t m42_extender = {&m42_parse, &m42_exec, NULL};

void m42_register(void)
{
    parser_register_extender(&m42_extender);
}

//this just parses and acceps the code
static uint8_t m42_parse(unsigned char c, uint8_t word, uint8_t error, float value, parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
    if (c == 'M' && word == 42)
    {
        if (cmd->group_extended != 0)
        {
            //there is a collision of custom gcode commands (only one per line can be processed)
            return STATUS_GCODE_MODAL_GROUP_VIOLATION;
        }
        //tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
        cmd->group_extended = M42;
        return STATUS_OK;
    }

    //if this is not catched by this parser, just send back the error so other extenders can process it
    return error;
}

//this actually performs 2 steps in 1 (validation and execution)
static uint8_t m42_exec(parser_state_t *new_state, parser_words_t *words, parser_cmd_explicit_t *cmd)
{
    if (cmd->group_extended == M42)
    {
        if (CHECKFLAG(cmd->words, (GCODE_WORD_S | GCODE_WORD_P)) != (GCODE_WORD_S | GCODE_WORD_P))
        {
            return STATUS_GCODE_VALUE_WORD_MISSING;
        }

        if (words->p >= 20 && words->p <= 51)
        {
            if (words->p >= 36)
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

    return STATUS_GOCDE_EXTENDED_UNSUPPORTED;
}

#endif