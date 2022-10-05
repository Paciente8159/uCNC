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
#ifndef SPINDLE_DIR
#define SPINDLE_DIR DOUT0
#endif

#ifdef ENABLE_COOLANT
#ifndef COOLANT_FLOOD
#define COOLANT_FLOOD DOUT2
#endif
#ifndef COOLANT_MIST
#define COOLANT_MIST DOUT3
#endif
#endif

#define SPINDLE_FEEDBACK ANALOG0

static uint8_t spindle_pwm_speed;

void spindle_pwm_startup_code(void)
{
// force pwm mode
#if !(SPINDLE_PWM < 0)
	mcu_config_pwm(SPINDLE_PWM, 1000);
#endif
}

void spindle_pwm_set_speed(int16_t value)
{
	// easy macro to execute the same code as below
	// SET_SPINDLE(SPINDLE_PWM, SPINDLE_DIR, value, invert);
	spindle_pwm_speed = (uint8_t)ABS(value);
// speed optimized version (in AVR it's 24 instruction cycles)
#if !(SPINDLE_DIR < 0)
	if ((value <= 0))
	{
		mcu_clear_output(SPINDLE_DIR);
	}
	else
	{
		mcu_set_output(SPINDLE_DIR);
	}
#endif

#if !(SPINDLE_PWM < 0)
	mcu_set_pwm(SPINDLE_PWM, (uint8_t)ABS(value));
#endif
}

void spindle_pwm_set_coolant(uint8_t value)
{
// easy macro
#ifdef ENABLE_COOLANT
	SET_COOLANT(COOLANT_FLOOD, COOLANT_MIST, value);
#endif
}

int16_t spindle_pwm_range_speed(float value)
{
	value = (255.0f) * (value / g_settings.spindle_max_rpm);
	return ((int16_t)value);
}

uint16_t spindle_pwm_get_speed(void)
{
#if SPINDLE_PWM >= 0
	float spindle = (float)mcu_get_pwm(SPINDLE_PWM) * g_settings.spindle_max_rpm * UINT8_MAX_INV;
	return (uint16_t)roundf(spindle);
#else
	return 0;
#endif
}

#if PID_CONTROLLERS > 0
void spindle_pwm_pid_update(int16_t value)
{

#if !(SPINDLE_PWM < 0)
	if (spindle_pwm_speed != 0)
	{
		uint8_t newval = CLAMP(0, mcu_get_pwm(SPINDLE_PWM) + value, 255);
		mcu_set_pwm(SPINDLE_PWM, newval);
	}
#endif
}

int16_t spindle_pwm_pid_error(void)
{
#if (!(SPINDLE_FEEDBACK < 0) && !(SPINDLE_PWM < 0))
	uint8_t reader = mcu_get_analog(ANALOG0);
	return (spindle_pwm_speed - reader);
#else
	return 0;
#endif
}
#endif

const tool_t __rom__ spindle_pwm = {
	.startup_code = &spindle_pwm_startup_code,
	.shutdown_code = NULL,
#if PID_CONTROLLERS > 0
	.pid_update = &spindle_pwm_pid_update,
	.pid_error = &spindle_pwm_pid_error,
#endif
	.range_speed = &spindle_pwm_range_speed,
	.get_speed = &spindle_pwm_get_speed,
	.set_speed = &spindle_pwm_set_speed,
	.set_coolant = &spindle_pwm_set_coolant};
