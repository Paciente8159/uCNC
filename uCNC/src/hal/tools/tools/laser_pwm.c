/*
	Name: laser_pwm.c
	Description: Defines a laser tool using PWM0 for µCNC.
				 Defines a coolant output using DOUT1 (can be used for air assist).

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 17/12/2021

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
#include <math.h>

/**
 * This configures a simple spindle control with a pwm assigned to PWM0 and dir invert assigned to DOUT0
 * This spindle also has a coolant pin assigned to DOUT1
 *
 * */

// give names to the pins (easier to identify)
#ifndef LASER_PWM
#define LASER_PWM PWM0
#endif

#ifndef LASER_FREQ
#define LASER_FREQ 8000
#endif

// #define ENABLE_COOLANT
#ifdef ENABLE_COOLANT
#ifndef LASER_PWM_AIR_ASSIST
#define LASER_PWM_AIR_ASSIST DOUT2
#endif
#endif

// this sets the minimum power (laser will never fully turn off during engraving and prevents power up delays)
#define PWM_MIN_VALUE 2

static bool previous_mode;

static void startup_code(void)
{
// force laser mode
#if ASSERT_PIN(LASER_PWM)
	io_config_pwm(LASER_PWM, LASER_FREQ);
	io_set_pwm(LASER_PWM, 0);
#else
	io_set_pwm(LASER_PWM, 0);
#endif
	previous_mode = g_settings.laser_mode;
	g_settings.laser_mode = LASER_PWM_MODE;
}

static void shutdown_code(void)
{
	// restore laser mode
	g_settings.laser_mode = previous_mode;
}

static void set_speed(int16_t value)
{
// easy macro to execute the same code as below
// SET_LASER(LASER_PWM, value, invert);

// speed optimized version (in AVR it's 24 instruction cycles)
#if ASSERT_PIN(LASER_PWM)
	io_set_pwm(LASER_PWM, (uint8_t)ABS(value));
#else
	io_set_pwm(LASER_PWM, (uint8_t)ABS(value));
#endif
}

static int16_t range_speed(int16_t value)
{
	// converts core tool speed to laser power (PWM)
	value = (int16_t)(PWM_MIN_VALUE + ((255.0f - PWM_MIN_VALUE) * (((float)value) / g_settings.spindle_max_rpm)));
	return value;
}

static void set_coolant(uint8_t value)
{
// easy macro
#ifdef ENABLE_COOLANT
	SET_COOLANT(LASER_PWM_AIR_ASSIST, UNDEF_PIN, value);
#endif
}

static uint16_t get_speed(void)
{
#if ASSERT_PIN(LASER_PWM)
	float laser = (float)io_get_pwm(LASER_PWM) * g_settings.spindle_max_rpm * UINT8_MAX_INV;
	return (uint16_t)lroundf(laser);
#else
	return 0;
#endif
}

const tool_t laser_pwm = {
	.startup_code = &startup_code,
	.shutdown_code = &shutdown_code,
#if PID_CONTROLLERS > 0
	.pid_update = NULL,
	.pid_error = NULL,
#endif
	.range_speed = &range_speed,
	.get_speed = &get_speed,
	.set_speed = &set_speed,
	.set_coolant = &set_coolant};
