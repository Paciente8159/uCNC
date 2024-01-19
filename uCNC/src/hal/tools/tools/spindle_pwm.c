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

#if defined(ENABLE_TOOL_PID_CONTROLLER) && !defined(DISABLE_SPINDLE_PWM_PID)
#ifndef SPINDLE_PWM_PID_SAMPLE_RATE_HZ
#define SPINDLE_PWM_PID_SAMPLE_RATE_HZ 125
#endif
#define SPINDLE_PWM_PID_SETTING_ID 300
#include <stdbool.h>
#include "../../../modules/pid.h"
static pid_data_t spindle_pwm_pid;
DECL_EXTENDED_SETTING(SPINDLE_PWM_PID_SETTING_ID, spindle_pwm_pid.k, float, 3, protocol_send_gcode_setting_line_flt);
#endif

static void
startup_code(void)
{
// force pwm mode
#if ASSERT_PIN(SPINDLE_PWM)
	io_config_pwm(SPINDLE_PWM, 1000);
#endif

#if defined(ENABLE_TOOL_PID_CONTROLLER) && !defined(DISABLE_SPINDLE_PWM_PID)
	EXTENDED_SETTING_INIT(SPINDLE_PWM_PID_SETTING_ID, spindle_pwm_pid.k);
	settings_load(EXTENDED_SETTING_ADDRESS(SPINDLE_PWM_PID_SETTING_ID), (uint8_t *)spindle_pwm_pid.k, sizeof(spindle_pwm_pid.k));
	spindle_pwm_pid.max = g_settings.spindle_max_rpm;
	spindle_pwm_pid.min = g_settings.spindle_min_rpm;
#endif
}

static void set_speed(int16_t value)
{
	// easy macro to execute the same code as below
	// SET_SPINDLE(SPINDLE_PWM, SPINDLE_PWM_DIR, value, invert);
// speed optimized version (in AVR it's 24 instruction cycles)
#if ASSERT_PIN(SPINDLE_PWM_DIR)
	if ((value <= 0))
	{
		io_clear_output(SPINDLE_PWM_DIR);
	}
	else
	{
		io_set_output(SPINDLE_PWM_DIR);
	}
#endif

#if ASSERT_PIN(SPINDLE_PWM)
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

static int16_t range_speed(int16_t value, uint8_t conv)
{
	// converts core tool speed to laser power (PWM)
	if (!conv)
	{
		value = (int16_t)((255.0f) * (((float)value) / g_settings.spindle_max_rpm));
	}
	else
	{
		value = (int16_t)roundf((1.0f / 255.0f) * value * g_settings.spindle_max_rpm);
	}
	return value;
}

static uint16_t get_speed(void)
{
#ifdef SPINDLE_PWM_HAS_RPM_ENCODER
	return encoder_get_rpm();
#else
#if ASSERT_PIN(SPINDLE_PWM)
	return tool_get_setpoint();
#else
	return 0;
#endif
#endif
}

#if defined(ENABLE_TOOL_PID_CONTROLLER) && !defined(DISABLE_SPINDLE_PWM_PID)
static void pid_update(void)
{
	float output = tool_get_setpoint();

	if (output != 0)
	{
		if (pid_compute(&spindle_pwm_pid, &output, output, get_speed(), HZ_TO_MS(SPINDLE_PWM_PID_SAMPLE_RATE_HZ)))
		{
			io_set_pwm(SPINDLE_PWM, range_speed((int16_t)output));
		}
	}
}

#endif

const tool_t spindle_pwm = {
	.startup_code = &startup_code,
	.shutdown_code = NULL,
#if defined(ENABLE_TOOL_PID_CONTROLLER) && !defined(DISABLE_SPINDLE_PWM_PID)
	.pid_update = &pid_update,
#else
	.pid_update = NULL,
#endif
	.range_speed = &range_speed,
	.get_speed = &get_speed,
	.set_speed = &set_speed,
	.set_coolant = &set_coolant};
