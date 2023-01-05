/*
	Name: pen_servo.c
	Description: Defines a pen tool using a servo motor

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 05/01/2023

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

/**
 * Configures a simple pen holder controlled via servo with digital pins for tool forward (`M3`) and reverse (`M4`).
 *
 * */

// Servo enable pins.  You can set these to the same pin if required.
#ifndef PEN_SERVO
#define PEN_SERVO SERVO0
#endif

#define THROTTLE_DOWN 50
#define THROTTLE_NEUTRAL 127
#define THROTTLE_FULL 255
#define THROTTLE_RANGE (THROTTLE_FULL - THROTTLE_DOWN)

static uint8_t speed;

static void startup_code(void)
{
#if ASSERT_PIN(PEN_SERVO)
	mcu_set_servo(PEN_SERVO, THROTTLE_DOWN);
#endif
}

static void shutdown_code(void)
{
#if ASSERT_PIN(PEN_SERVO)
	mcu_set_servo(PEN_SERVO, THROTTLE_DOWN);
#endif
}

static int16_t range_speed(int16_t value)
{
	if (value == 0)
	{
		return 0;
	}

	value = (int16_t)((THROTTLE_RANGE) * (((float)value) / g_settings.spindle_max_rpm) + THROTTLE_DOWN);
	return value;
}

static void set_speed(int16_t value)
{
	if ((value <= 0))
	{
#if ASSERT_PIN(PEN_SERVO)
		mcu_set_servo(PEN_SERVO, THROTTLE_DOWN);
#endif
	}
	else
	{
#if ASSERT_PIN(PEN_SERVO)
		mcu_set_servo(PEN_SERVO, (uint8_t)value);
#endif
	}

	speed = (value <= 0) ? 0 : value;
}


static uint16_t get_speed(void)
{
#if ASSERT_PIN(PEN_SERVO)
	float spindle = (float)speed * g_settings.spindle_max_rpm * UINT8_MAX_INV;
	return (uint16_t)lroundf(spindle);
#else
	return 0;
#endif
}

const tool_t pen_servo = {
	.startup_code = &startup_code,
	.shutdown_code = &shutdown_code,
#if PID_CONTROLLERS > 0
	.pid_update = NULL,
	.pid_error = NULL,
#endif
	.range_speed = &range_speed,
	.get_speed = &get_speed,
	.set_speed = &set_speed,
	.set_coolant = NULL};
