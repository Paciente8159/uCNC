/*
	Name: vfd_huanyang.c
	Description: Defines a Huanyang VFD controlled tool for µCNC.
				 Defines a coolant output using DOUT1.

	Copyright: Copyright (c) João Martins
	Author: James Harton
	Date: 1/5/2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"
#include "../../../modules/softuart.h"
#include "../../../modules/modbus.h"
static uint8_t spindle_speed;

#define VFD_TX_PIN DOUT31
#define VFD_RX_PIN DIN31
#define VFD_BAUDRATE 9600

SOFTUART(vfd_uart, VFD_BAUDRATE, VFD_TX_PIN, VFD_RX_PIN)

/*VFD settings*/
#define VFD_ADDRESS 1
#define VFD_RPM_HZ 60
#define VFD_RUNSTOP_CMD 8192
#define VFD_SETFREQ_CMD 8193
#define VFD_GETFREQ_CMD 8451
#define VFD_CW_CMD 18
#define VFD_CCW_CMD 34
#define VFD_STOP_CMD 1
#define VFD_IN_MULT 50
#define VFD_IN_DIV 60
#define VFD_OUT_MULT 60
#define VFD_OUT_DIV 100

void vfd_startup()
{

}

void vfd_shutdown()
{
#if !(SPINDLE_POWER_RELAY < 0)
	mcu_clear_output(SPINDLE_POWER_RELAY);
#endif
}

void vfd_set_speed(uint8_t value, bool invert)
{

	if (!value || invert)
	{
#if !(SPINDLE_SERVO < 0)
		mcu_set_servo(SPINDLE_SERVO, THROTTLE_DOWN);
#endif
	}
	else
	{
#if !(SPINDLE_SERVO < 0)
		uint16_t scale = value * THROTTLE_RANGE;
		uint8_t new_val = (0xFF & (scale >> 8)) + THROTTLE_DOWN;
		mcu_set_servo(SPINDLE_SERVO, new_val);
#endif
	}

	spindle_speed = (invert) ? 0 : value;
}

void vfd_set_coolant(uint8_t value)
{
	SET_COOLANT(COOLANT_FLOOD_EN, COOLANT_MIST_EN, value);
}

uint16_t vfd_get_speed(void)
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

const tool_t __rom__ vfd = {
	.startup_code = &vfd_startup,
	.shutdown_code = &vfd_shutdown,
	.set_speed = &vfd_set_speed,
	.set_coolant = &vfd_set_coolant,
#if PID_CONTROLLERS > 0
	.pid_update = NULL,
	.pid_error = NULL,
#endif
	.get_speed = &vfd_get_speed};
