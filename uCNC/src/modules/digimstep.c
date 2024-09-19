/*
	Name: digimstep.c
	Description: Digital pin stepper settings module for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 31-03-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../cnc.h"

#ifdef ENABLE_DIGITAL_MSTEP

/*custom gcode commands*/
#if defined(ENABLE_PARSER_MODULES)
// this ID must be unique for each code
#define M351 EXTENDED_MCODE(351)

bool m351_parse(void *args);
bool m351_exec(void *args);

CREATE_EVENT_LISTENER(gcode_parse, m351_parse);
CREATE_EVENT_LISTENER(gcode_exec, m351_exec);

// this just parses and acceps the code
bool m351_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;

	if (ptr->word == 'M' && ptr->value == 351.0f)
	{
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			return EVENT_HANDLED;
		}
		// tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
		ptr->cmd->group_extended = M351;
		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool m351_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

	if (ptr->cmd->group_extended == M351)
	{
		itp_sync();
		if (!ptr->cmd->words)
		{
			int32_t val = -1;
			// if no additional args then print the
			grbl_protocol_string("[MSTEPS:");
			val = -1;
			serial_putc('X');
#if ASSERT_PIN(STEPPER0_MSTEP0)
			val = io_get_output(STEPPER0_MSTEP0) ? 1 : 0;
#endif
#if ASSERT_PIN(STEPPER0_MSTEP1)
			val = MAX(0, val);
			val |= io_get_output(STEPPER0_MSTEP1) ? 2 : 0;
#endif
			serial_print_int(val);
			serial_putc(',');
			val = -1;
			serial_putc('Y');
#if ASSERT_PIN(STEPPER1_MSTEP0)
			val = io_get_output(STEPPER1_MSTEP0) ? 1 : 0;
#endif
#if ASSERT_PIN(STEPPER1_MSTEP1)
			val = MAX(0, val);
			val |= io_get_output(STEPPER1_MSTEP1) ? 2 : 0;
#endif
			serial_print_int(val);
			serial_putc(',');
			val = -1;
			serial_putc('Z');
#if ASSERT_PIN(STEPPER2_MSTEP0)
			val = io_get_output(STEPPER2_MSTEP0) ? 1 : 0;
#endif
#if ASSERT_PIN(STEPPER2_MSTEP1)
			val = MAX(0, val);
			val |= io_get_output(STEPPER2_MSTEP1) ? 2 : 0;
#endif
			serial_print_int(val);
			serial_putc(',');
			val = -1;
			serial_putc('A');
#if ASSERT_PIN(STEPPER3_MSTEP0)
			val = io_get_output(STEPPER3_MSTEP0) ? 1 : 0;
#endif
#if ASSERT_PIN(STEPPER3_MSTEP1)
			val = MAX(0, val);
			val |= io_get_output(STEPPER3_MSTEP1) ? 2 : 0;
#endif
			serial_print_int(val);
			serial_putc(',');
			val = -1;
			serial_putc('B');
#if ASSERT_PIN(STEPPER4_MSTEP0)
			val = io_get_output(STEPPER4_MSTEP0) ? 1 : 0;
#endif
#if ASSERT_PIN(STEPPER4_MSTEP1)
			val = MAX(0, val);
			val |= io_get_output(STEPPER4_MSTEP1) ? 2 : 0;
#endif
			serial_print_int(val);
			serial_putc(',');
			val = -1;
			serial_putc('C');
#if ASSERT_PIN(STEPPER5_MSTEP0)
			val = io_get_output(STEPPER5_MSTEP0) ? 1 : 0;
#endif
#if ASSERT_PIN(STEPPER5_MSTEP1)
			val = MAX(0, val);
			val |= io_get_output(STEPPER5_MSTEP1) ? 2 : 0;
#endif
			serial_print_int(val);
			serial_putc(',');
			val = -1;
			serial_putc('I');
#if ASSERT_PIN(STEPPER6_MSTEP0)
			val = io_get_output(STEPPER6_MSTEP0) ? 1 : 0;
#endif
#if ASSERT_PIN(STEPPER6_MSTEP1)
			val = MAX(0, val);
			val |= io_get_output(STEPPER6_MSTEP1) ? 2 : 0;
#endif
			serial_print_int(val);
			serial_putc(',');
			val = -1;
			serial_putc('J');
#if ASSERT_PIN(STEPPER7_MSTEP0)
			val = io_get_output(STEPPER7_MSTEP0) ? 1 : 0;
#endif
#if ASSERT_PIN(STEPPER7_MSTEP1)
			val = MAX(0, val);
			val |= io_get_output(STEPPER7_MSTEP1) ? 2 : 0;
#endif
			serial_print_int(val);
			serial_putc(']');
			grbl_protocol_string(MSG_EOL);
		}

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_X))
		{
#if ASSERT_PIN(STEPPER0_MSTEP0)
			io_set_pinvalue(STEPPER0_MSTEP0, ((uint8_t)ptr->words->xyzabc[0] & 0x01));
#endif
#if ASSERT_PIN(STEPPER0_MSTEP1)
			io_set_pinvalue(STEPPER0_MSTEP1, ((uint8_t)ptr->words->xyzabc[0] & 0x02));
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Y))
		{
#if ASSERT_PIN(STEPPER1_MSTEP0)
			io_set_pinvalue(STEPPER1_MSTEP0, ((uint8_t)ptr->words->xyzabc[1] & 0x01));
#endif
#if ASSERT_PIN(STEPPER1_MSTEP1)
			io_set_pinvalue(STEPPER1_MSTEP1, ((uint8_t)ptr->words->xyzabc[1] & 0x02));
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Z))
		{
#if ASSERT_PIN(STEPPER2_MSTEP0)
			io_set_pinvalue(STEPPER2_MSTEP0, (((uint8_t)ptr->words->xyzabc[2]) & 0x01));
#endif
#if ASSERT_PIN(STEPPER2_MSTEP1)
			io_set_pinvalue(STEPPER2_MSTEP1, (((uint8_t)ptr->words->xyzabc[2]) & 0x02));
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_A))
		{
#if ASSERT_PIN(STEPPER3_MSTEP0)
			io_set_pinvalue(STEPPER3_MSTEP0, ((uint8_t)ptr->words->xyzabc[3] & 0x01));
#endif
#if ASSERT_PIN(STEPPER3_MSTEP1)
			io_set_pinvalue(STEPPER3_MSTEP1, ((uint8_t)ptr->words->xyzabc[3] & 0x02));
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_B))
		{
#if ASSERT_PIN(STEPPER4_MSTEP0)
			io_set_pinvalue(STEPPER4_MSTEP0, ((uint8_t)ptr->words->xyzabc[4] & 0x01));
#endif
#if ASSERT_PIN(STEPPER4_MSTEP1)
			io_set_pinvalue(STEPPER4_MSTEP1, ((uint8_t)ptr->words->xyzabc[4] & 0x02));
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_C))
		{
#if ASSERT_PIN(STEPPER5_MSTEP0)
			io_set_pinvalue(STEPPER5_MSTEP0, ((uint8_t)ptr->words->xyzabc[5] & 0x01));
#endif
#if ASSERT_PIN(STEPPER5_MSTEP1)
			io_set_pinvalue(STEPPER5_MSTEP1, ((uint8_t)ptr->words->xyzabc[5] & 0x02));
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_I))
		{
#if ASSERT_PIN(STEPPER6_MSTEP0)
			io_set_pinvalue(STEPPER6_MSTEP0, ((uint8_t)ptr->words->ijk[0] & 0x01));
#endif
#if ASSERT_PIN(STEPPER6_MSTEP1)
			io_set_pinvalue(STEPPER6_MSTEP1, ((uint8_t)ptr->words->ijk[0] & 0x02));
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_J))
		{
#if ASSERT_PIN(STEPPER7_MSTEP0)
			io_set_pinvalue(STEPPER7_MSTEP0, ((uint8_t)ptr->words->ijk[1] & 0x01));
#endif
#if ASSERT_PIN(STEPPER7_MSTEP1)
			io_set_pinvalue(STEPPER7_MSTEP1, ((uint8_t)ptr->words->ijk[1] & 0x02));
#endif
		}

		*(ptr->error) = STATUS_OK;
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

#endif

DECL_MODULE(digimstep)
{
#if ASSERT_PIN(STEPPER0_MSTEP0)
	io_set_pinvalue(STEPPER0_MSTEP0, (STEPPER0_MSTEP & 0x01));
#endif
#if ASSERT_PIN(STEPPER0_MSTEP1)
	io_set_pinvalue(STEPPER0_MSTEP1, (STEPPER0_MSTEP & 0x02));
#endif
#if ASSERT_PIN(STEPPER1_MSTEP0)
	io_set_pinvalue(STEPPER1_MSTEP0, (STEPPER1_MSTEP & 0x01));
#endif
#if ASSERT_PIN(STEPPER1_MSTEP1)
	io_set_pinvalue(STEPPER1_MSTEP1, (STEPPER1_MSTEP & 0x02));
#endif
#if ASSERT_PIN(STEPPER2_MSTEP0)
	io_set_pinvalue(STEPPER2_MSTEP0, (STEPPER2_MSTEP & 0x01));
#endif
#if ASSERT_PIN(STEPPER2_MSTEP1)
	io_set_pinvalue(STEPPER2_MSTEP1, (STEPPER2_MSTEP & 0x02));
#endif
#if ASSERT_PIN(STEPPER3_MSTEP0)
	io_set_pinvalue(STEPPER3_MSTEP0, (STEPPER3_MSTEP & 0x01));
#endif
#if ASSERT_PIN(STEPPER3_MSTEP1)
	io_set_pinvalue(STEPPER3_MSTEP1, (STEPPER3_MSTEP & 0x02));
#endif
#if ASSERT_PIN(STEPPER4_MSTEP0)
	io_set_pinvalue(STEPPER4_MSTEP0, (STEPPER4_MSTEP & 0x01));
#endif
#if ASSERT_PIN(STEPPER4_MSTEP1)
	io_set_pinvalue(STEPPER4_MSTEP1, (STEPPER4_MSTEP & 0x02));
#endif
#if ASSERT_PIN(STEPPER5_MSTEP0)
	io_set_pinvalue(STEPPER5_MSTEP0, (STEPPER5_MSTEP & 0x01));
#endif
#if ASSERT_PIN(STEPPER5_MSTEP1)
	io_set_pinvalue(STEPPER5_MSTEP1, (STEPPER5_MSTEP & 0x02));
#endif
#if ASSERT_PIN(STEPPER6_MSTEP0)
	io_set_pinvalue(STEPPER6_MSTEP0, (STEPPER6_MSTEP & 0x01));
#endif
#if ASSERT_PIN(STEPPER6_MSTEP1)
	io_set_pinvalue(STEPPER6_MSTEP1, (STEPPER6_MSTEP & 0x02));
#endif
#if ASSERT_PIN(STEPPER7_MSTEP0)
	io_set_pinvalue(STEPPER7_MSTEP0, (STEPPER7_MSTEP & 0x01));
#endif
#if ASSERT_PIN(STEPPER7_MSTEP1)
	io_set_pinvalue(STEPPER7_MSTEP1, (STEPPER7_MSTEP & 0x02));
#endif

#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(gcode_parse, m351_parse);
	ADD_EVENT_LISTENER(gcode_exec, m351_exec);
#else
#warning "Parser extensions are not enabled. M351 code extension will not work."
#endif
}
#endif
