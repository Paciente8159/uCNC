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
#define VFD_RETRY_DELAY_MS 100

SOFTUART(vfd_uart, VFD_BAUDRATE, VFD_TX_PIN, VFD_RX_PIN)

/*VFD settings*/
#define VFD_ADDRESS 1
#define VFD_RPM_HZ 60
#define VFD_RUNSTOP_CMD 8192
#define VFD_SETRPM_CMD                          \
	{                                           \
		4, MODBUS_FORCE_SINGLE_COIL, 0x01, 0, 0 \
	}
#define VFD_GETRPM_CMD                                   \
	{                                                    \
		5, MODBUS_READ_INPUT_REGISTERS, 0x03, 0x01, 0, 0 \
	}
#define VFD_RPM_HZ_CMD                               \
	{                                                \
		5, MODBUS_READ_COIL_STATUS, 0x03, 0x90, 0, 0 \
	}
#define VFD_CW_CMD                                         \
	{                                                      \
		3, MODBUS_READ_HOLDING_REGISTERS, 0x01, 0x01, 0, 0 \
	}
#define VFD_CCW_CMD                                        \
	{                                                      \
		3, MODBUS_READ_HOLDING_REGISTERS, 0x01, 0x02, 0, 0 \
	}
#define VFD_STOP_CMD                                       \
	{                                                      \
		3, MODBUS_READ_HOLDING_REGISTERS, 0x01, 0x08, 0, 0 \
	}
#define VFD_IN_MULT 50
#define VFD_IN_DIV 60
#define VFD_OUT_MULT 60
#define VFD_OUT_DIV 100
#define VFD_MAX_COMMAND_RETRIES 1

typedef struct vfd_state
{
	uint8_t connected : 1;
	uint8_t ccw : 1;
	uint8_t needs_update : 1;
	float rpm;
	float rpm_hz;
} vfd_state_t;

static vfd_state_t vfd_state;

static bool modvfd_command(uint8_t *cmd, modbus_response_t *response)
{
	modbus_request_t request = {0};
	memset(response, 0, sizeof(modbus_response_t));
	request.address = VFD_ADDRESS;
	uint8_t *data = &(request.fcode);
	for (uint8_t i = 0; i < cmd[0]; i++)
	{
		data[i] = cmd[i + 1];
	}

	uint8_t retries = VFD_MAX_COMMAND_RETRIES;
	vfd_state.needs_update = false;
	while (retries--)
	{
		send_request(request, &vfd_uart);
		if (read_response(&response, &vfd_uart))
		{
			return true;
		}
		protocol_send_error(__romstr__("Communication with VFD failed!"));
		cnc_delay_ms(VFD_RETRY_DELAY_MS);
	}
}

static void vfd_rpm_hz(void)
{
	modbus_response_t response = {0};
	uint8_t *cmd = VFD_RPM_HZ_CMD;

	if (modvfd_command(cmd, &response))
	{
		vfd_state.rpm_hz = ((float)((((uint16_t)response.data[1]) << 8) | response.data[2]));
	}
}

static void vfd_get_rpm(bool truerpm)
{
	if (truerpm)
	{
		modbus_response_t response = {0};
		uint8_t *cmd = VFD_GETRPM_CMD;

		if (modvfd_command(cmd, &response))
		{
			return ((float)((((uint16_t)response.data[1]) << 8) | response.data[2])) * VFD_IN_MULT / VFD_IN_DIV;
		}
		return -1;
	}

	return vfd_state.rpm;
}

static void vfd_update_rpm(void)
{
	modbus_response_t response = {0};
	uint8_t *cmd = VFD_STOP_CMD;
	if (vfd_state.rpm != 0)
	{
		uint16_t hz = vfd_state.rpm * VFD_OUT_MULT / VFD_OUT_DIV;
	}

	modvfd_command(cmd, &response);
}

static void vfd_cw(void)
{
	if (vfd_state.ccw)
	{
		modbus_response_t response = {0};
		uint8_t *cmd = VFD_CW_CMD;

		if (modvfd_command(cmd, &response))
		{
			vfd_state.ccw = 0;
		}
	}
}

static void vfd_ccw(void)
{
	if (!vfd_state.ccw)
	{
		modbus_response_t response = {0};
		uint8_t *cmd = VFD_CCW_CMD;

		if (modvfd_command(cmd, &response))
		{
			vfd_state.ccw = 1;
		}
	}
}

void vfd_startup()
{
	vfd_rpm_hz();
	vfd_state.rpm = 0;
	vfd_update_rpm();
}

void vfd_shutdown()
{
	vfd_state.rpm = 0;
	vfd_update_rpm();
}

void vfd_set_speed(uint8_t value, bool invert)
{
	if (!value)
	{
		vfd_state.rpm = 0;
		vfd_update_rpm();
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
