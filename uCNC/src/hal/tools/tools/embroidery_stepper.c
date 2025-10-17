/*
	Name: embroidery_stepper.c
	Description: Defines a embroidery tool where the needle mechanism is controlled with a stepper motor for µCNC.


	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 15-10-2025

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

#ifndef EMBD_STEP
#define EMBD_STEP DOUT0
#endif

#ifndef EMBD_DIR
#define EMBD_DIR DOUT1
#endif

#ifndef EMBD_FWD_INV
#define EMBD_FWD_INV 0
#endif

#ifndef INT_MATH_SHIFT
#define INT_MATH_SHIFT 8
#endif

#ifndef MCU_HAS_ONESHOT
#warning "Embroidery stepper tool requires the oneshot timer to be configured"
#endif

static uint32_t embd_steps_per_rev;
static uint32_t embd_steps_target_us;
static volatile uint32_t embd_steps_curr_us;
static uint32_t embd_steps_count;
static float embd_accel;
static int16_t previous_rpm;
static bool embd_stop_on_target;
static uint32_t embd_update_steps;
/**
 *
 * Motion and interpolator related stuff
 *
 * **/

static FORCEINLINE float update_timeout(void)
{
	uint32_t current_us = embd_steps_curr_us;
	uint32_t steps = embd_update_steps;
	uint32_t next;

	if (current_us > embd_steps_target_us)
	{
		steps++;
		embd_update_steps = steps;
		next = (uint32_t)((uint64_t)current_us << 1) / ((steps << 2) + 1);
		if (next == 0)
			next = 1;
		// recurrence relation for constant accel
		current_us -= next;
		// current_us -= (uint32_t)(next >> 9);
		if (current_us <= embd_steps_target_us)
		{
			current_us = embd_steps_target_us;
		} // fix overshoot and resets counters
	}
	else if (current_us < embd_steps_target_us && (embd_steps_per_rev - embd_steps_count) <= steps)
	{
		uint32_t steps_mult = (steps << 2);
		next = (uint32_t)((uint64_t)current_us  * (steps_mult + 1)) / (steps_mult - 1);// apply the reverse equation
		// recurrence relation for constant deaccel
		current_us = next;
		// current_us -= (next >> 10);
		if (current_us >= embd_steps_target_us || !steps)
		{
			current_us = embd_steps_target_us;
			if (embd_stop_on_target)
			{
				return 0;
			}
		} // fix overshoot and resets counters
		steps--;
		embd_update_steps = steps;
	}

	return current_us;
}

MCU_CALLBACK void embd_isr_cb(void)
{
#if ASSERT_PIN(EMBD_STEP)
	io_toggle_output(EMBD_STEP);
#endif
	uint32_t steps = embd_steps_count;
	uint32_t current_us = embd_steps_curr_us;
	steps++;
	if (steps > embd_steps_per_rev)
	{

		steps = 0;
		//		mcu_toggle_output(DOUT1); /*for test purposes*/
	}

	// if (!(steps & 0x01))
	{
		current_us = update_timeout();
		embd_steps_curr_us = current_us;
	}
	embd_steps_count = steps;
	if (!current_us)
	{
		return; // tool stopped. prevent rearm timer
	}
	mcu_config_timeout(&embd_isr_cb, ((uint32_t)current_us >> INT_MATH_SHIFT));
	mcu_start_timeout(); // arm the timer again
}

/**
 *
 * Parser extensions
 * Parser extensions are optional since these can be controlled via tool options
 *
 * **/

#ifdef ENABLE_PARSER_MODULES

// bool laser_ppi_parser_reset(void *args)
//{
//	laser_ppi_config_parameters();
//	return EVENT_CONTINUE;
// }
//// create event listener
// CREATE_EVENT_LISTENER(parser_reset, laser_ppi_parser_reset);
//
// #define M126 EXTENDED_MCODE(126)
// #define M127 EXTENDED_MCODE(127)
// #define M128 EXTENDED_MCODE(128)
//
//// this just parses and acceps the code
// bool laser_ppi_mcodes_parse(void *args)
//{
//	gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;
//	if (ptr->word == 'M')
//	{
//		switch (ptr->code)
//		{
//		case 126:
//			if (ptr->cmd->group_extended != 0)
//			{
//				// there is a collision of custom gcode commands (only one per line can be processed)
//				*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
//			}
//			else
//			{
//				ptr->cmd->group_extended = M126;
//				*(ptr->error) = STATUS_OK;
//			}
//			return EVENT_HANDLED;
//		case 127:
//			if (ptr->cmd->group_extended != 0)
//			{
//				// there is a collision of custom gcode commands (only one per line can be processed)
//				*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
//			}
//			else
//			{
//				ptr->cmd->group_extended = M127;
//				*(ptr->error) = STATUS_OK;
//			}
//			return EVENT_HANDLED;
//		case 128:
//			if (ptr->cmd->group_extended != 0)
//			{
//				// there is a collision of custom gcode commands (only one per line can be processed)
//				*(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
//			}
//			else
//			{
//				ptr->cmd->group_extended = M128;
//				*(ptr->error) = STATUS_OK;
//			}
//			return EVENT_HANDLED;
//		}
//	}
//
//	// if this is not catched by this parser, just send back the error so other extenders can process it
//	return EVENT_CONTINUE;
// }
//
// CREATE_EVENT_LISTENER(gcode_parse, laser_ppi_mcodes_parse);
//
//// this actually performs 2 steps in 1 (validation and execution)
// bool laser_ppi_mcodes_exec(void *args)
//{
//	gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;
//	switch (ptr->cmd->group_extended)
//	{
//	case M127:
//	case M128:
//		// prevents command execution if mode disabled
//		if (!(g_settings.laser_mode & (LASER_PPI_MODE | LASER_PPI_VARPOWER_MODE)))
//		{
//			*(ptr->error) = STATUS_LASER_PPI_MODE_DISABLED;
//			return EVENT_HANDLED;
//		}
//	case M126:
//		if (CHECKFLAG(ptr->cmd->words, (GCODE_WORD_P)) != (GCODE_WORD_P))
//		{
//			*(ptr->error) = STATUS_GCODE_VALUE_WORD_MISSING;
//			return EVENT_HANDLED;
//		}
//
//		*(ptr->error) = STATUS_OK;
//		break;
//	}
//
//	switch (ptr->cmd->group_extended)
//	{
//	case M126:
//		g_settings.laser_mode &= ~(LASER_PPI_MODE | LASER_PPI_VARPOWER_MODE);
//		switch ((((uint8_t)ptr->words->p)))
//		{
//		case 1:
//			g_settings.laser_mode |= LASER_PPI_MODE;
//			break;
//		case 2:
//			g_settings.laser_mode |= LASER_PPI_VARPOWER_MODE;
//			break;
//		case 3:
//			g_settings.laser_mode |= (LASER_PPI_MODE | LASER_PPI_VARPOWER_MODE);
//			break;
//		}
//		laser_ppi_config_parameters();
//		return EVENT_HANDLED;
//	case M127:
//		g_settings.laser_ppi = (uint16_t)ptr->words->p;
//		laser_ppi_config_parameters();
//		return EVENT_HANDLED;
//	case M128:
//		g_settings.laser_ppi_uswidth = (uint16_t)ptr->words->p;
//		laser_ppi_config_parameters();
//		return EVENT_HANDLED;
//	}
//
//	return EVENT_CONTINUE;
// }
//
// CREATE_EVENT_LISTENER(gcode_exec, laser_ppi_mcodes_exec);
#endif

DECL_MODULE(embroidery_stepper)
{
	// #ifdef ENABLE_PARSER_MODULES
	//	ADD_EVENT_LISTENER(gcode_parse, laser_ppi_mcodes_parse);
	//	ADD_EVENT_LISTENER(gcode_exec, laser_ppi_mcodes_exec);
	// #else
	// #warning "Parser extensions are not enabled. M126, M127 and M128 code extensions will not work."
	// #endif
}

/**
 * Now starts the actual tool functions definitions
 * These functions will then be called by the tool HAL
 * **/
static void startup_code(void)
{
// force laser mode
#if ASSERT_PIN(EMBD_STEP)
	io_config_output(EMBD_STEP);
#endif
#if ASSERT_PIN(EMBD_DIR)
	io_config_output(EMBD_DIR);
#if EMBD_FWD_INV == 0
	io_clear_output(EMBD_DIR);
#else
	io_set_output(EMBD_DIR);
#endif
#endif

	embd_steps_per_rev = (3200 << 1);
	embd_accel = 5;
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
	if (value != previous_rpm)
	{
		uint32_t target_us = (value) ? (uint32_t)(1000000.0f / (value * embd_steps_per_rev * MIN_SEC_MULT)) : 0;

		if ((previous_rpm == 0) || (value == 0))
		{
			float dai = fast_flt_inv(2.0f * embd_accel);
			float fact = embd_accel * embd_steps_per_rev;
			uint32_t min_us = (uint32_t)(2000000.f * fast_flt_invsqrt(fact));

			if (previous_rpm == 0)
			{
				embd_steps_curr_us = min_us << INT_MATH_SHIFT;
			}
			else
			{
				target_us = min_us;
			}
		}

		if (value > previous_rpm)
		{
			embd_update_steps = 0;
		}

		embd_steps_target_us = (target_us << INT_MATH_SHIFT);
		embd_stop_on_target = (value == 0);
		previous_rpm = value;
		mcu_config_timeout(&embd_isr_cb, (embd_steps_curr_us >> INT_MATH_SHIFT));
		mcu_start_timeout();
	}
}

static uint16_t get_speed(void)
{
	float rpm = (float)((embd_steps_curr_us >> INT_MATH_SHIFT) * embd_steps_per_rev);
	return 60000000.0f / rpm;
}

const tool_t embroidery_stepper = {
		.startup_code = &startup_code,
		.shutdown_code = &shutdown_code,
		.pid_update = NULL,
		.range_speed = NULL,
		.get_speed = &get_speed,
		.set_speed = &set_speed,
		.set_coolant = &set_coolant};
