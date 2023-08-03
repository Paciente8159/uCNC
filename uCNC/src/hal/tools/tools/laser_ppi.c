/*
	Name: laser_ppi.c
	Description: Defines a laser PPI tool using PWM0 pin for µCNC.
				 Defines a coolant output using DOUT2 and DOUT3 (can be used for air assist).

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 05/10/2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

#include <stdbool.h>
#include <float.h>
#include <math.h>

#ifndef LASER_PPI
#define LASER_PPI PWM0
#endif

// #define ENABLE_COOLANT
#ifdef ENABLE_COOLANT
#ifndef LASER_PPI_AIR_ASSIST
#define LASER_PPI_AIR_ASSIST DOUT2
#endif
#endif

#ifdef ENABLE_LASER_PPI

/**
 *
 * Motion and interpolator related stuff
 *
 * **/

// turn laser off callback via ONESHOT timer
MCU_CALLBACK void laser_ppi_turnoff_cb(void)
{
#ifndef INVERT_LASER_PPI_LOGIC
	io_clear_output(LASER_PPI);
#else
	io_set_output(LASER_PPI);
#endif
}

// laser ppi pulse callback called from the step ISR
static uint16_t new_laser_ppi = 0;
MCU_CALLBACK void laser_ppi_pulse(uint8_t new_stepbits, uint8_t flags)
{
	if (g_settings.laser_mode & (LASER_PPI_MODE | LASER_PPI_VARPOWER_MODE))
	{
		if (new_stepbits & LASER_PPI_MASK)
		{
			if (new_laser_ppi)
			{
				mcu_config_timeout(&laser_ppi_turnoff_cb, new_laser_ppi);
				new_laser_ppi = 0;
			}
			mcu_start_timeout();
#ifndef INVERT_LASER_PPI_LOGIC
			io_set_output(LASER_PPI);
#else
			io_clear_output(LASER_PPI);
#endif
		}
	}
}

// configs the parameters for laser PPI
// called by each custom laser PPI MCode and on parser reset
static void laser_ppi_config_parameters(void)
{
	g_settings.acceleration[STEPPER_COUNT - 1] = FLT_MAX;
	if (g_settings.laser_mode & (LASER_PPI_MODE | LASER_PPI_VARPOWER_MODE))
	{
		// if previously disabled, reload default value
		if (!g_settings.step_per_mm[STEPPER_COUNT - 1])
		{
			g_settings.step_per_mm[STEPPER_COUNT - 1] = g_settings.laser_ppi * MM_INCH_MULT;
		}
		g_settings.max_feed_rate[STEPPER_COUNT - 1] = (60000000.0f / (g_settings.laser_ppi_uswidth * g_settings.step_per_mm[STEPPER_COUNT - 1]));
		mcu_config_timeout(&laser_ppi_turnoff_cb, g_settings.laser_ppi_uswidth);
	}
	else
	{
		g_settings.step_per_mm[STEPPER_COUNT - 1] = 0;
		g_settings.max_feed_rate[STEPPER_COUNT - 1] = FLT_MAX;
	}
}

/**
 *
 * Parser extensions
 * Parser extensions are optional since these can be controlled via tool options
 *
 * **/

#ifdef ENABLE_PARSER_MODULES

bool laser_ppi_parser_reset(void *args)
{
	laser_ppi_config_parameters();
	return EVENT_CONTINUE;
}
// create event listener
CREATE_EVENT_LISTENER(parser_reset, laser_ppi_parser_reset);

#define M126 EXTENDED_MCODE(126)
#define M127 EXTENDED_MCODE(127)
#define M128 EXTENDED_MCODE(128)

// this just parses and acceps the code
bool laser_ppi_mcodes_parse(void *args)
{
	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;
	if (ptr->word == 'M')
	{
		switch (ptr->code)
		{
		case 126:
			if (ptr->cmd->group_extended != 0)
			{
				// there is a collision of custom gcode commands (only one per line can be processed)
				*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			}
			else
			{
				ptr->cmd->group_extended = M126;
				*(ptr->error) = STATUS_OK;
			}
			return EVENT_HANDLED;
		case 127:
			if (ptr->cmd->group_extended != 0)
			{
				// there is a collision of custom gcode commands (only one per line can be processed)
				*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			}
			else
			{
				ptr->cmd->group_extended = M127;
				*(ptr->error) = STATUS_OK;
			}
			return EVENT_HANDLED;
		case 128:
			if (ptr->cmd->group_extended != 0)
			{
				// there is a collision of custom gcode commands (only one per line can be processed)
				*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
			}
			else
			{
				ptr->cmd->group_extended = M128;
				*(ptr->error) = STATUS_OK;
			}
			return EVENT_HANDLED;
		}
	}

	// if this is not catched by this parser, just send back the error so other extenders can process it
	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(gcode_parse, laser_ppi_mcodes_parse);

// this actually performs 2 steps in 1 (validation and execution)
bool laser_ppi_mcodes_exec(void *args)
{
	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;
	switch (ptr->cmd->group_extended)
	{
	case M127:
	case M128:
		// prevents command execution if mode disabled
		if (!(g_settings.laser_mode & (LASER_PPI_MODE | LASER_PPI_VARPOWER_MODE)))
		{
			*(ptr->error) = STATUS_LASER_PPI_MODE_DISABLED;
			return EVENT_HANDLED;
		}
	case M126:
		if (CHECKFLAG(ptr->cmd->words, (GCODE_WORD_P)) != (GCODE_WORD_P))
		{
			*(ptr->error) = STATUS_GCODE_VALUE_WORD_MISSING;
			return EVENT_HANDLED;
		}
		
		*(ptr->error) = STATUS_OK;
		break;
	}

	switch (ptr->cmd->group_extended)
	{
	case M126:
		g_settings.laser_mode &= ~(LASER_PPI_MODE | LASER_PPI_VARPOWER_MODE);
		switch ((((uint8_t)ptr->words->p)))
		{
		case 1:
			g_settings.laser_mode |= LASER_PPI_MODE;
			break;
		case 2:
			g_settings.laser_mode |= LASER_PPI_VARPOWER_MODE;
			break;
		case 3:
			g_settings.laser_mode |= (LASER_PPI_MODE | LASER_PPI_VARPOWER_MODE);
			break;
		}
		laser_ppi_config_parameters();
		return EVENT_HANDLED;
	case M127:
		g_settings.laser_ppi = (uint16_t)ptr->words->p;
		laser_ppi_config_parameters();
		return EVENT_HANDLED;
	case M128:
		g_settings.laser_ppi_uswidth = (uint16_t)ptr->words->p;
		laser_ppi_config_parameters();
		return EVENT_HANDLED;
	}

	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(gcode_exec, laser_ppi_mcodes_exec);
#endif

/**
 * Now starts the actual tool functions definitions
 * These functions will then be called by the tool HAL
 * **/
static void startup_code(void)
{
// force laser mode
#if ASSERT_PIN(LASER_PPI)
	io_config_output(LASER_PPI);
#ifndef INVERT_LASER_PPI_LOGIC
	io_clear_output(LASER_PPI);
#else
	io_set_output(LASER_PPI);
#endif
#endif
	g_settings.laser_mode |= LASER_PPI_MODE;
	laser_ppi_config_parameters();
	HOOK_ATTACH_CALLBACK(itp_rt_stepbits, laser_ppi_pulse);

	RUNONCE
	{
		ADD_EVENT_LISTENER(gcode_parse, laser_ppi_mcodes_parse);
		ADD_EVENT_LISTENER(gcode_exec, laser_ppi_mcodes_exec);
		RUNONCE_COMPLETE();
	}
}

static void shutdown_code(void)
{
#if ASSERT_PIN(LASER_PPI)
#ifndef INVERT_LASER_PPI_LOGIC
	io_clear_output(LASER_PPI);
#else
	io_set_output(LASER_PPI);
#endif
#endif
	// restore laser mode
	g_settings.laser_mode &= ~(LASER_PPI_MODE | LASER_PPI_VARPOWER_MODE);
	laser_ppi_config_parameters();
	HOOK_RELEASE(itp_rt_stepbits);
}

static void set_coolant(uint8_t value)
{
// easy macro
#ifdef ENABLE_COOLANT
	SET_COOLANT(LASER_PPI_AIR_ASSIST, UNDEF_PIN, value);
#endif
}

static void set_speed(int16_t value)
{
	if (g_settings.laser_mode & (LASER_PPI_MODE | LASER_PPI_VARPOWER_MODE))
	{
		new_laser_ppi = value;
	}
}

static uint16_t get_speed(void)
{
	return g_settings.step_per_mm[STEPPER_COUNT - 1];
}

const tool_t laser_ppi = {
	.startup_code = &startup_code,
	.shutdown_code = &shutdown_code,
	.pid_update = NULL,
	.range_speed = NULL,
	.get_speed = &get_speed,
	.set_speed = &set_speed,
	.set_coolant = &set_coolant};

#endif
