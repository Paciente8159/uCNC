/*
	Name: vfd_pwm.c
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
#ifndef VFD_PWM
#define VFD_PWM PWM0
#endif
#ifndef VFD_PWM_DIR
#define VFD_PWM_DIR DOUT0
#endif

#ifdef ENABLE_COOLANT
#ifndef VFD_PWM_COOLANT_FLOOD
#define VFD_PWM_COOLANT_FLOOD DOUT2
#endif
#ifndef VFD_PWM_COOLANT_MIST
#define VFD_PWM_COOLANT_MIST DOUT3
#endif
#endif

#ifndef VFD_PWM_ANALOG_FEEDBACK
#define VFD_PWM_ANALOG_FEEDBACK ANALOG0
#endif

static uint8_t speed;

static void startup_code(void)
{
// force pwm mode
#if ASSERT_PIN(VFD_PWM)
	io_config_pwm(VFD_PWM, 1000);
#endif
}

static void set_speed(int16_t value)
{
	// easy macro to execute the same code as below
	// SET_SPINDLE(VFD_PWM, VFD_PWM_DIR, value, invert);
	speed = (uint8_t)ABS(value);
// speed optimized version (in AVR it's 24 instruction cycles)
#if ASSERT_PIN(VFD_PWM_DIR)
	if ((value <= 0))
	{
		io_clear_output(VFD_PWM_DIR);
	}
	else
	{
		io_set_output(VFD_PWM_DIR);
	}
#endif

#if ASSERT_PIN(VFD_PWM)
	io_set_pwm(VFD_PWM, (uint8_t)ABS(value));
#else
	io_set_pwm(VFD_PWM, (uint8_t)ABS(value));
#endif
}

static void set_coolant(uint8_t value)
{
// easy macro
#ifdef ENABLE_COOLANT
	SET_COOLANT(VFD_PWM_COOLANT_FLOOD, VFD_PWM_COOLANT_MIST, value);
#endif
}

static int16_t range_speed(int16_t value)
{
	value = (int16_t)((255.0f) * (((float)value) / g_settings.spindle_max_rpm));
	return value;
}

static uint16_t get_speed(void)
{
#if ASSERT_PIN(VFD_PWM_ANALOG_FEEDBACK)
	float spindle = (float)io_get_analog(VFD_PWM_ANALOG_FEEDBACK) * g_settings.spindle_max_rpm * UINT8_MAX_INV;
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
		uint8_t newval = CLAMP(0, io_get_pwm(VFD_PWM) + value, 255);
#if ASSERT_PIN(VFD_PWM)
		io_set_pwm(VFD_PWM, newval);
#else
		io_set_pwm(VFD_PWM, newval);
#endif
	}
}

static int16_t pid_error(void)
{
#if (ASSERT_PIN(VFD_PWM_ANALOG_FEEDBACK) && ASSERT_PIN(VFD_PWM))
	uint8_t reader = io_get_analog(VFD_PWM_ANALOG_FEEDBACK);
	return (speed - reader);
#else
	return 0;
#endif
}
#endif

const tool_t vfd_pwm = {
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
