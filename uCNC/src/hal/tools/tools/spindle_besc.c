/*
	Name: spindle_relay.c
	Description: Defines a spindle tool using a brushless motor and a ESC controller

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 05/05/2022

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
 * Configures a simple spindle control with digital pins for tool forward (`M3`) and reverse (`M4`).
 * There is also provision for digital pins for coolant mist (`M7`) and flood (`M8`).
 *
 * */

// Spindle enable pins.  You can set these to the same pin if required.
#ifndef SPINDLE_BESC_SERVO
#define SPINDLE_BESC_SERVO SERVO0
#endif
#ifndef SPINDLE_BESC_POWER_RELAY
#define SPINDLE_BESC_POWER_RELAY DOUT0
#endif

#ifdef ENABLE_COOLANT
#ifndef SPINDLE_BESC_COOLANT_FLOOD
#define SPINDLE_BESC_COOLANT_FLOOD DOUT2
#endif
#ifndef SPINDLE_BESC_COOLANT_MIST
#define SPINDLE_BESC_COOLANT_MIST DOUT3
#endif
#endif

#define THROTTLE_DOWN 50
#define THROTTLE_NEUTRAL 127
#define THROTTLE_FULL 255
#define THROTTLE_RANGE (THROTTLE_FULL - THROTTLE_DOWN)

// #define HAS_RPM_COUNTER
#ifdef HAS_RPM_COUNTER
#define RPM_ENCODER 0
#endif

static uint8_t spindle_speed;

static void startup_code(void)
{
// do whatever routine you need to do here to arm the ESC
#if !(SPINDLE_BESC_POWER_RELAY < 0)
#if !(SPINDLE_BESC_SERVO < 0)
	mcu_set_servo(SPINDLE_BESC_SERVO, THROTTLE_NEUTRAL);
#endif
	mcu_set_output(SPINDLE_BESC_POWER_RELAY);
	cnc_delay_ms(1000);
#if !(SPINDLE_BESC_SERVO < 0)
	mcu_set_servo(SPINDLE_BESC_SERVO, THROTTLE_DOWN);
#endif
	cnc_delay_ms(2000);
#endif
}

static void shutdown_code(void)
{
#if !(SPINDLE_BESC_POWER_RELAY < 0)
	mcu_clear_output(SPINDLE_BESC_POWER_RELAY);
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
#if !(SPINDLE_BESC_SERVO < 0)
		mcu_set_servo(SPINDLE_BESC_SERVO, THROTTLE_DOWN);
#endif
	}
	else
	{
#if !(SPINDLE_BESC_SERVO < 0)
		mcu_set_servo(SPINDLE_BESC_SERVO, (uint8_t)value);
#endif
	}

	spindle_speed = (value <= 0) ? 0 : value;
}

static void set_coolant(uint8_t value)
{
#ifdef ENABLE_COOLANT
	SET_COOLANT(SPINDLE_BESC_COOLANT_FLOOD, SPINDLE_BESC_COOLANT_MIST, value);
#endif
}

static uint16_t get_speed(void)
{

	// this show how to use an encoder (in this case encoder 0) configured as a counter
	// to take real RPM readings of the spindle
	// the reading is updated every 5 seconds

#if (defined(HAS_RPM_COUNTER) && (ENCODERS > RPM_ENCODER))
	extern int32_t encoder_get_position(uint8_t i);
	extern void encoder_reset_position(uint8_t i, int32_t position);
	static uint32_t last_time = 0;
	static uint16_t lastrpm = 0;
	uint16_t rpm = lastrpm;

	uint32_t elapsed = (mcu_millis() - last_time);
	int32_t read = encoder_get_position(0);

	// updates speed read every 5s
	if (read > 0)
	{
		float timefact = 60000.f / (float)elapsed;
		float newrpm = timefact * (float)read;
		last_time = mcu_millis();
		encoder_reset_position(0, 0);
		rpm = (uint16_t)newrpm;
		lastrpm = rpm;
	}
	else if (elapsed > 60000)
	{
		last_time = mcu_millis();
		rpm = 0;
		lastrpm = 0;
	}

	return rpm;
#else
	return spindle_speed;
#endif
}

const tool_t spindle_besc = {
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
