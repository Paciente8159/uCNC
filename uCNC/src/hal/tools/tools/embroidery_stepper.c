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

#ifdef ENABLE_EMBROIDERY

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
static uint32_t embd_steps_needle_down;
static uint32_t embd_steps_target_us;
static volatile uint32_t embd_steps_curr_us;
static volatile uint32_t embd_steps_count;
static float embd_accel;
static int16_t previous_rpm;
static bool embd_stop_on_target;
static uint32_t embd_update_steps;
static uint32_t embd_max_steps;
static uint32_t embd_down_steps;

#ifdef ENABLE_SETTINGS_MODULES
#undef EMBRODERY_STEPPER_STP_PER_REVS
#define EMBRODERY_STEPPER_STP_PER_REVS 310
DECL_EXTENDED_SETTING(EMBRODERY_STEPPER_STP_PER_REVS, &embd_steps_per_rev, uint32_t, 1, proto_gcode_setting_line_int);
#define EMBRODERY_STEPPER_STP_NEEDLE_DOWN 311
DECL_EXTENDED_SETTING(EMBRODERY_STEPPER_STP_NEEDLE_DOWN, &embd_steps_needle_down, uint32_t, 1, proto_gcode_setting_line_int);
#else
#ifndef EMBRODERY_STEPPER_STP_PER_REVS
#define EMBRODERY_STEPPER_STP_PER_REVS 3200 // if settings are disable set the steps per revolution directly
#endif
#ifndef EMBRODERY_STEPPER_STP_NEEDLE_DOWN
#define EMBRODERY_STEPPER_STP_NEEDLE_DOWN 2600 // if settings are disable set the steps at which the needle enters the fabric and not motion can occur directly
#endif
#endif

bool needle_is_down()
{
	return (embd_steps_count >= embd_down_steps);
}

/**
 *
 * Motion and interpolator related stuff
 *
 * **/

static FORCEINLINE uint32_t update_timeout(void)
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
			// mcu_set_output(DOUT1);
		} // fix overshoot and resets counters
	}
	else if (current_us < embd_steps_target_us && (embd_steps_per_rev - embd_steps_count) <= steps)
	{
		// mcu_clear_output(DOUT1);
		uint32_t steps_mult = (steps << 2);
		next = (uint32_t)((uint64_t)current_us * (steps_mult + 1)) / (steps_mult - 1); // apply the reverse equation
		// recurrence relation for constant deaccel
		current_us = next;
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
	mcu_isr_context_enter();
	
#if ASSERT_PIN(EMBD_STEP)
	io_toggle_output(EMBD_STEP);
#endif
	uint32_t steps = embd_steps_count;
	uint32_t current_us = embd_steps_curr_us;

	steps++;
	if (steps > embd_max_steps)
	{

		steps = 0;
		// mcu_toggle_output(DOUT1); /*for test purposes*/
		itp_inc_block_id();
	}

	current_us = update_timeout();
	embd_steps_curr_us = current_us;

	embd_steps_count = steps;
	if (!current_us)
	{
		if (!embd_stop_on_target)
		{
#ifdef ENABLE_RT_SYNC_MOTIONS
			itp_set_block_mode(ITP_BLOCK_CONTINUOUS); // switch to continuous mode
#else
#warning "ENABLE_RT_SYNC_MOTIONS not enabled embroidery tool will not work correctly"
#endif
			return; // tool stopped. prevent rearm timer
		}
		mcu_config_timeout(&embd_isr_cb, ((uint32_t)embd_steps_target_us >> INT_MATH_SHIFT));
		embd_stop_on_target = false;
	}
	else
	{
		mcu_config_timeout(&embd_isr_cb, ((uint32_t)current_us >> INT_MATH_SHIFT));
	}

	mcu_start_timeout(); // arm the timer again
}

/**
 *
 * Parser extensions
 * Parser extensions are optional since these can be controlled via tool options
 *
 * **/

#ifdef ENABLE_PARSER_MODULES

/**
 * If necessary future custom M/G codes can be added here
 */

// DECL_MODULE(embroidery_stepper)
// {
// 	// #ifdef ENABLE_PARSER_MODULES
// 	//	ADD_EVENT_LISTENER(gcode_parse, laser_ppi_mcodes_parse);
// 	//	ADD_EVENT_LISTENER(gcode_exec, laser_ppi_mcodes_exec);
// 	// #else
// 	// #warning "Parser extensions are not enabled. M126, M127 and M128 code extensions will not work."
// 	// #endif
// }

#endif

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

#ifdef ENABLE_SETTINGS_MODULES
	EXTENDED_SETTING_INIT(EMBRODERY_STEPPER_STP_PER_REVS, embd_steps_per_rev);
	settings_load(EXTENDED_SETTING_ADDRESS(EMBRODERY_STEPPER_STP_PER_REVS), (uint8_t *)&embd_steps_per_rev, sizeof(embd_steps_per_rev));
	EXTENDED_SETTING_INIT(EMBRODERY_STEPPER_STP_NEEDLE_DOWN, embd_steps_needle_down);
	settings_load(EXTENDED_SETTING_ADDRESS(EMBRODERY_STEPPER_STP_NEEDLE_DOWN), (uint8_t *)&embd_steps_needle_down, sizeof(embd_steps_needle_down));
#else
	embd_steps_per_rev = (EMBRODERY_STEPPER_STP_PER_REVS);
	embd_steps_needle_down = (EMBRODERY_STEPPER_STP_NEEDLE_DOWN);
#endif
	embd_max_steps = embd_steps_per_rev << 1;
	embd_down_steps = embd_steps_needle_down << 1;

#if defined(RT_STEP_PREVENT_CONDITION) && defined(ENABLE_RT_SYNC_MOTIONS)
	itp_rt_step_prevent_cb = &needle_is_down;
#else
#warning "RT_STEP_PREVENT_CONDITION and ENABLE_RT_SYNC_MOTIONS is not set and needle down condition will not be detected!!"
#endif
	embd_accel = 5;
	g_settings.tool_mode = EMBROIDERY_MODE;
}

static void shutdown_code(void)
{
#if ASSERT_PIN(EMBD_DIR)
#if EMBD_FWD_INV == 0
	io_clear_output(EMBD_DIR);
#else
	io_set_output(EMBD_DIR);
#endif
#endif
#if ASSERT_PIN(EMBD_STEP)
	io_clear_output(EMBD_STEP);
#endif
	g_settings.tool_mode = UNDEF_MODE;
#ifdef ENABLE_RT_SYNC_MOTIONS
	itp_rt_step_prevent_cb = NULL;
#else
#warning "ENABLE_RT_SYNC_MOTIONS not enabled embroidery tool will not work correctly"
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
	value = ABS(value);

	if (value != previous_rpm)
	{
		uint32_t target_us = (value) ? (uint32_t)(1000000.0f / (value * embd_max_steps * MIN_SEC_MULT)) : 0;

#ifdef ENABLE_RT_SYNC_MOTIONS
		itp_set_block_mode(ITP_BLOCK_SINGLE);
#else
#warning "ENABLE_RT_SYNC_MOTIONS not enabled embroidery tool will not work correctly"
#endif
		if ((previous_rpm == 0) || (value == 0))
		{
			float dai = fast_flt_inv(2.0f * embd_accel);
			float fact = embd_accel * embd_max_steps;
			uint32_t min_us = (uint32_t)(2000000.f * fast_flt_invsqrt(fact));

			if (previous_rpm == 0)
			{
				embd_steps_curr_us = (min_us << INT_MATH_SHIFT);
			}
			else
			{
				target_us = min_us;
			}
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
	float rpm = (float)((embd_steps_curr_us >> INT_MATH_SHIFT) * embd_max_steps);
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

#endif
