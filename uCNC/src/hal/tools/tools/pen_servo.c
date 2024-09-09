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

#ifndef PEN_SERVO_LOW
#define PEN_SERVO_LOW 50
#endif
#ifndef PEN_SERVO_HIGH
#define PEN_SERVO_HIGH 255
#endif
#define PEN_SERVO_RANGE ((PEN_SERVO_HIGH) - (PEN_SERVO_LOW))

static void startup_code(void)
{
#if ASSERT_PIN(PEN_SERVO)
	io_set_pwm(PEN_SERVO, PEN_SERVO_LOW);
#endif
}

static void shutdown_code(void)
{
#if ASSERT_PIN(PEN_SERVO)
	io_set_pwm(PEN_SERVO, PEN_SERVO_LOW);
#endif
}

static int16_t range_speed(int16_t value, uint8_t conv)
{
	if (value == 0)
	{
		return 0;
	}

	if (!conv)
	{
		value = (int16_t)(((float)PEN_SERVO_RANGE) * ((float)value) / ((float)g_settings.spindle_max_rpm) + PEN_SERVO_LOW);
	}
	else{
		value = (int16_t)((((float)value) - ((float)PEN_SERVO_LOW)) * ((float)g_settings.spindle_max_rpm) / ((float)PEN_SERVO_RANGE));
	}

	return value;
}

static void set_speed(int16_t value)
{
  value = CLAMP(PEN_SERVO_LOW, value, PEN_SERVO_HIGH);

#if ASSERT_PIN(PEN_SERVO)
  io_set_pwm(PEN_SERVO, (uint8_t)value);
#endif
}

const tool_t pen_servo = {
	.startup_code = &startup_code,
	.shutdown_code = &shutdown_code,
	.pid_update = NULL,
	.range_speed = &range_speed,
	.get_speed = NULL,
	.set_speed = &set_speed,
	.set_coolant = NULL};
