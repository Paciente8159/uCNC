/*
	Name: spindle_pwm.c
	Description: Defines a spindle tool using PWM0-speed and DOUT0-dir for µCNC.
				 Defines a coolant output using DOUT1 and DOUT2.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 16/12/2021

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
#ifndef SPINDLE_PWM
#define SPINDLE_PWM PWM0
#endif
#ifndef SPINDLE_PWM_DIR
#define SPINDLE_PWM_DIR DOUT0
#endif

#ifdef ENABLE_COOLANT
#ifndef SPINDLE_PWM_COOLANT_FLOOD
#define SPINDLE_PWM_COOLANT_FLOOD DOUT2
#endif
#ifndef SPINDLE_PWM_COOLANT_MIST
#define SPINDLE_PWM_COOLANT_MIST DOUT3
#endif
#endif

#ifdef SPINDLE_PWM_HAS_RPM_ENCODER
#ifndef ENABLE_ENCODER_RPM
#error "To use RPM encoder you must enable ENABLE_ENCODER_RPM in the HAL"
#endif
#endif

static uint8_t speed;

static void startup_code(void)
{
// force pwm mode
#if ASSERT_PIN(SPINDLE_PWM)
	mcu_config_pwm(SPINDLE_PWM, 1000);
#endif
}

static void set_speed(int16_t value)
{
	// easy macro to execute the same code as below
	// SET_SPINDLE(SPINDLE_PWM, SPINDLE_PWM_DIR, value, invert);
	speed = (uint8_t)ABS(value);
// speed optimized version (in AVR it's 24 instruction cycles)
#if ASSERT_PIN(SPINDLE_PWM_DIR)
	if ((value <= 0))
	{
		mcu_clear_output(SPINDLE_PWM_DIR);
	}
	else
	{
		mcu_set_output(SPINDLE_PWM_DIR);
	}
#endif

#if ASSERT_PIN(SPINDLE_PWM)
	mcu_set_pwm(SPINDLE_PWM, (uint8_t)ABS(value));
#else
	io_set_pwm(SPINDLE_PWM, (uint8_t)ABS(value));
#endif
}

static void set_coolant(uint8_t value)
{
// easy macro
#ifdef ENABLE_COOLANT
	SET_COOLANT(SPINDLE_PWM_COOLANT_FLOOD, SPINDLE_PWM_COOLANT_MIST, value);
#endif
}

static int16_t range_speed(int16_t value)
{
	value = (int16_t)((255.0f) * (((float)value) / g_settings.spindle_max_rpm));
	return value;
}

static uint16_t get_speed(void)
{
#ifdef SPINDLE_PWM_HAS_RPM_ENCODER
	return encoder_get_rpm();
#else
#if SPINDLE_PWM >= 0
	float spindle = (float)speed * g_settings.spindle_max_rpm * UINT8_MAX_INV;
	return (uint16_t)lroundf(spindle);
#else
	return 0;
#endif
#endif
}

#if PID_CONTROLLERS > 0
static void pid_update(int16_t value)
{
	if (speed != 0)
	{
		uint8_t newval = CLAMP(0, mcu_get_pwm(SPINDLE_PWM) + value, 255);
#if ASSERT_PIN(SPINDLE_PWM)
		mcu_set_pwm(SPINDLE_PWM, newval);
#else
		io_set_pwm(SPINDLE_PWM, newval);
#endif
	}
}

static int16_t pid_error(void)
{
#if (ASSERT_PIN(SPINDLE_FEEDBACK) && ASSERT_PIN(SPINDLE_PWM))
	uint8_t reader = mcu_get_analog(ANALOG0);
	return (speed - reader);
#else
	return 0;
#endif
}
#endif

const tool_t spindle_pwm = {
	.startup_code = &startup_code,
	.shutdown_code = NULL,
#if PID_CONTROLLERS > 0
	.pid_update = &pid_update,
	.pid_error = &pid_error,
#endif
	.range_speed = &range_speed,
	.get_speed = &get_speed,
	.set_speed = &set_speed,
	.set_coolant = &set_coolant};
