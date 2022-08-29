/*
	Name: digipot.c
	Description: Digital potenciometer module for µCNC.

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
#include "softspi.h"

#ifdef STEPPER_CURR_DIGIPOT
SOFTSPI(digipotspi, 1000000UL, 0, STEPPER_DIGIPOT_DO, STEPPER_DIGIPOT_DI, STEPPER_DIGIPOT_CLK)

/*custom gcode commands*/
#if defined(ENABLE_PARSER_MODULES)

// this ID must be unique for each code
#define M907 EXTENDED_MCODE(907)

uint8_t m907_parse(void *args, bool *handled);
uint8_t m907_exec(void *args, bool *handled);

CREATE_EVENT_LISTENER(gcode_parse, m907_parse);
CREATE_EVENT_LISTENER(gcode_exec, m907_exec);

// this just parses and acceps the code
uint8_t m907_parse(void *args, bool *handled)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;

	if (ptr->word == 'M' && ptr->value == 907.0f)
	{
		if (ptr->cmd->group_extended != 0)
		{
			// there is a collision of custom gcode commands (only one per line can be processed)
			return STATUS_GCODE_MODAL_GROUP_VIOLATION;
		}
		// tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
		ptr->cmd->group_extended = M907;
		return STATUS_OK;
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return ptr->error;
}

// this actually performs 2 steps in 1 (validation and execution)
uint8_t m907_exec(void* args, bool* handled)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;

	if (ptr->cmd->group_extended == M907)
	{
		*handled = true;
		
		itp_sync();
		if (!ptr->cmd->words)
		{
			return STATUS_GCODE_NO_AXIS_WORDS;
		}

		mcu_clear_output(STEPPER_DIGIPOT_SS);

		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_X))
		{
#if STEPPER0_DIGIPOT_CHANNEL > 0
			softspi_xmit(&digipotspi, STEPPER0_DIGIPOT_CHANNEL);
			softspi_xmit(&digipotspi, (uint8_t)ptr->words->xyzabc[0]);
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Y))
		{
#if STEPPER1_DIGIPOT_CHANNEL > 0
			softspi_xmit(&digipotspi, STEPPER1_DIGIPOT_CHANNEL);
			softspi_xmit(&digipotspi, (uint8_t)ptr->words->xyzabc[1]);
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_Z))
		{
#if STEPPER2_DIGIPOT_CHANNEL > 0
			softspi_xmit(&digipotspi, STEPPER2_DIGIPOT_CHANNEL);
			softspi_xmit(&digipotspi, (uint8_t)ptr->words->xyzabc[2]);
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_A))
		{
#if STEPPER3_DIGIPOT_CHANNEL > 0
			softspi_xmit(&digipotspi, STEPPER3_DIGIPOT_CHANNEL);
			softspi_xmit(&digipotspi, (uint8_t)ptr->words->xyzabc[3]);
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_B))
		{
#if STEPPER4_DIGIPOT_CHANNEL > 0
			softspi_xmit(&digipotspi, STEPPER4_DIGIPOT_CHANNEL);
			softspi_xmit(&digipotspi, (uint8_t)ptr->words->xyzabc[4]);
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_C))
		{
#if STEPPER5_DIGIPOT_CHANNEL > 0
			softspi_xmit(&digipotspi, STEPPER5_DIGIPOT_CHANNEL);
			softspi_xmit(&digipotspi, (uint8_t)ptr->words->xyzabc[5]);
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_I))
		{
#if STEPPER6_DIGIPOT_CHANNEL > 0
			softspi_xmit(&digipotspi, STEPPER6_DIGIPOT_CHANNEL);
			softspi_xmit(&digipotspi, (uint8_t)ptr->words->ijk[0]);
#endif
		}
		if (CHECKFLAG(ptr->cmd->words, GCODE_WORD_J))
		{
#if STEPPER7_DIGIPOT_CHANNEL > 0
			softspi_xmit(&digipotspi, STEPPER7_DIGIPOT_CHANNEL);
			softspi_xmit(&digipotspi, (uint8_t)ptr->words->ijk[1]);
#endif
		}

		mcu_set_output(STEPPER_DIGIPOT_SS);

		return STATUS_OK;
	}

	return STATUS_GCODE_EXTENDED_UNSUPPORTED;
}

#endif

uint8_t digipot_config(void *args, bool *handled)
{
	// Digipot for stepper motors
#ifdef STEPPER_CURR_DIGIPOT
	mcu_clear_output(STEPPER_DIGIPOT_SS);
#if STEPPER0_DIGIPOT_CHANNEL > 0
	softspi_xmit(&digipotspi, STEPPER0_DIGIPOT_CHANNEL);
	softspi_xmit(&digipotspi, STEPPER0_DIGIPOT_VALUE);
#endif
#if STEPPER1_DIGIPOT_CHANNEL > 0
	softspi_xmit(&digipotspi, STEPPER1_DIGIPOT_CHANNEL);
	softspi_xmit(&digipotspi, STEPPER1_DIGIPOT_VALUE);
#endif
#if STEPPER2_DIGIPOT_CHANNEL > 0
	softspi_xmit(&digipotspi, STEPPER2_DIGIPOT_CHANNEL);
	softspi_xmit(&digipotspi, STEPPER2_DIGIPOT_VALUE);
#endif
#if STEPPER3_DIGIPOT_CHANNEL > 0
	softspi_xmit(&digipotspi, STEPPER3_DIGIPOT_CHANNEL);
	softspi_xmit(&digipotspi, STEPPER3_DIGIPOT_VALUE);
#endif
#if STEPPER4_DIGIPOT_CHANNEL > 0
	softspi_xmit(&digipotspi, STEPPER4_DIGIPOT_CHANNEL);
	softspi_xmit(&digipotspi, STEPPER4_DIGIPOT_VALUE);
#endif
#if STEPPER5_DIGIPOT_CHANNEL > 0
	softspi_xmit(&digipotspi, STEPPER5_DIGIPOT_CHANNEL);
	softspi_xmit(&digipotspi, STEPPER5_DIGIPOT_VALUE);
#endif
#if STEPPER6_DIGIPOT_CHANNEL > 0
	softspi_xmit(&digipotspi, STEPPER6_DIGIPOT_CHANNEL);
	softspi_xmit(&digipotspi, STEPPER6_DIGIPOT_VALUE);
#endif
#if STEPPER7_DIGIPOT_CHANNEL > 0
	softspi_xmit(&digipotspi, STEPPER7_DIGIPOT_CHANNEL);
	softspi_xmit(&digipotspi, STEPPER7_DIGIPOT_VALUE);
#endif
	mcu_set_output(STEPPER_DIGIPOT_SS);
#endif

	return 0;
}

#ifdef ENABLE_MAIN_LOOP_MODULES
CREATE_EVENT_LISTENER(cnc_reset, digipot_config);
#endif

DECL_MODULE(digipot)
{
	// initialize slave select pins
#ifdef STEPPER_CURR_DIGIPOT
	mcu_set_output(STEPPER_DIGIPOT_SS);
#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_reset, digipot_config);
#endif
#ifdef ENABLE_PARSER_MODULES
	ADD_EVENT_LISTENER(gcode_parse, m907_parse);
	ADD_EVENT_LISTENER(gcode_exec, m907_exec);
#endif
}

#endif
