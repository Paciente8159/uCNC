/*
	Name: spindle_pwm_encoder.c
	Description: Defines a spindle tool using PWM0-speed and DOUT0-dir for µCNC.
				 Defines a coolant output using DOUT1 and DOUT2.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 02/12/2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

#include <math.h>

/**
 * This configures a simple spindle control with a pwm assigned to PWM0 and dir invert assigned to DOUT0
 * This spindle also has a coolant pin assigned to DOUT1
 *
 * */

// give names to the pins (easier to identify)
#ifndef SPINDLE_PWM_ENC
#define SPINDLE_PWM_ENC PWM0
#endif
#ifndef SPINDLE_PWM_ENC_DIR
#define SPINDLE_PWM_ENC_DIR DOUT0
#endif

#ifdef ENABLE_COOLANT
#ifndef SPINDLE_PWM_ENC_COOLANT_FLOOD
#define SPINDLE_PWM_ENC_COOLANT_FLOOD DOUT2
#endif
#ifndef SPINDLE_PWM_ENC_COOLANT_MIST
#define SPINDLE_PWM_ENC_COOLANT_MIST DOUT3
#endif
#endif

#ifndef SPINDLE_PWM_ENC_FEEDBACK
#define SPINDLE_PWM_ENC_FEEDBACK ENC0
#endif

static uint8_t speed;

static void speed_encoder_read(uint8_t inputs, uint8_t diff)
{
}

static void startup_code(void)
{
// force pwm mode
#if !(SPINDLE_PWM_ENC < 0)
	mcu_config_pwm(SPINDLE_PWM_ENC, 1000);
#endif

	io_inputs_changed_cb = &speed_encoder_read;
}

static void shutdown_code(void)
{
	io_inputs_changed_cb = NULL;
}

static void set_speed(int16_t value)
{
	// easy macro to execute the same code as below
	// SET_SPINDLE(SPINDLE_PWM_ENC, SPINDLE_PWM_ENC_DIR, value, invert);
	speed = (uint8_t)ABS(value);
// speed optimized version (in AVR it's 24 instruction cycles)
#if !(SPINDLE_PWM_ENC_DIR < 0)
	if ((value <= 0))
	{
		mcu_clear_output(SPINDLE_PWM_ENC_DIR);
	}
	else
	{
		mcu_set_output(SPINDLE_PWM_ENC_DIR);
	}
#endif

#if !(SPINDLE_PWM_ENC < 0)
	mcu_set_pwm(SPINDLE_PWM_ENC, (uint8_t)ABS(value));
#else
	io_set_pwm(SPINDLE_PWM_ENC, (uint8_t)ABS(value));
#endif
}

static void set_coolant(uint8_t value)
{
// easy macro
#ifdef ENABLE_COOLANT
	SET_COOLANT(SPINDLE_PWM_ENC_COOLANT_FLOOD, SPINDLE_PWM_ENC_COOLANT_MIST, value);
#endif
}

static int16_t range_speed(int16_t value)
{
	value = (int16_t)((255.0f) * (((float)value) / g_settings.spindle_max_rpm));
	return value;
}

static uint16_t get_speed(void)
{
#if SPINDLE_PWM_ENC >= 0
	float spindle = (float)mcu_get_pwm(SPINDLE_PWM_ENC) * g_settings.spindle_max_rpm * UINT8_MAX_INV;
	return (uint16_t)lroundf(spindle);
#else
	return 0;
#endif
}

#if PID_CONTROLLERS > 0
static void pid_update(int16_t value)
{
	if (speed != 0)
	{
		uint8_t newval = CLAMP(0, mcu_get_pwm(SPINDLE_PWM_ENC) + value, 255);
#if !(SPINDLE_PWM_ENC < 0)
		mcu_set_pwm(SPINDLE_PWM_ENC, newval);
#else
		io_set_pwm(SPINDLE_PWM_ENC, newval);
#endif
	}
}

static int16_t pid_error(void)
{
#if (!(SPINDLE_FEEDBACK < 0) && !(SPINDLE_PWM_ENC < 0))
	uint8_t reader = mcu_get_analog(ANALOG0);
	return (speed - reader);
#else
	return 0;
#endif
}
#endif

const tool_t spindle_pwm_enc = {
	.startup_code = &startup_code,
	.shutdown_code = &shutdown_code,
#if PID_CONTROLLERS > 0
	.pid_update = &pid_update,
	.pid_error = &pid_error,
#endif
	.range_speed = &range_speed,
	.get_speed = &get_speed,
	.set_speed = &set_speed,
	.set_coolant = &set_coolant};
