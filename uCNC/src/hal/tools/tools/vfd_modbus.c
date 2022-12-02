/*
	Name: vfd_modbus.c
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
#include "../../../modules/modbus.h"
#include <math.h>
#include <stdbool.h>

// defines default coolant pins
#ifdef ENABLE_COOLANT
#ifndef VFD_COOLANT_FLOOD
#define VFD_COOLANT_FLOOD DOUT1
#endif
#ifndef VFD_COOLANT_MIST
#define VFD_COOLANT_MIST DOUT2
#endif
#endif

// defines VFD report mode
// if false returns programmed speed
// if true return speed read from VFD
#ifndef GET_SPINDLE_TRUE_RPM
#define GET_SPINDLE_TRUE_RPM false
#endif

// defines default softuart pins and vfd communication settings
#ifndef VFD_TX_PIN
#define VFD_TX_PIN DOUT27
#endif
#ifndef VFD_RX_PIN
#define VFD_RX_PIN DIN27
#endif
#ifndef VFD_BAUDRATE
#define VFD_BAUDRATE 9600
#endif
#ifndef VFD_TIMEOUT
#define VFD_TIMEOUT 100
#endif
#ifndef VFD_RETRY_DELAY_MS
#define VFD_RETRY_DELAY_MS 100
#endif

// comment this to override vfd communication error safety hold
// #define IGNORE_VFD_COM_ERRORS
#ifndef IGNORE_VFD_COM_ERRORS
#define VFD_HOLD_ON_ERROR
#endif

#if !(VFD_TX_PIN < 0) && !(VFD_RX_PIN < 0)
SOFTUART(vfd_uart, VFD_BAUDRATE, VFD_TX_PIN, VFD_RX_PIN)

#define VFD_STOPPED 0
#define VFD_RUN_CW 2
#define VFD_RUN_CCW 3
typedef struct vfd_state
{
	uint8_t loaded : 1;
	uint8_t connected : 1;
	uint8_t running : 2;
	volatile uint8_t needs_update : 1;
	volatile int16_t rpm;
	float rpm_hz;
} vfd_state_t;

static vfd_state_t vfd_state;

/*VFD settings*/
#define VFD_ADDRESS 8
#define VFD_MAX_COMMAND_RETRIES 2

// choose the controller type
// types available
#define VFD_HUANYANG_TYPE1 1
#define VFD_HUANYANG_TYPE2 2
#define VFD_YL620 3
#define VFD_POWTRAN8100 4

#ifndef VFD_CONTROLLER
#define VFD_CONTROLLER VFD_HUANYANG_TYPE1
#endif

/**
 *
 * VFD Commands are an array of 7 bytes that have the following information
 *
 * {tx command length, rx command length, MODBUS command type, start address high byte, start address low byte, value high byte, value low byte}
 *
 * A typical MODBUS command is usually 8 <vfd address (1byte) + MODBUS command type (1byte) + start address (2byte) + value (2byte) + CRC (2byte)>
 * A tx command length of 0 will mute the command
 *
 * Some VFD do not use the standard MODBUS message format and some times the command length is different (like the Huanyang Type1)
 *
 * */
#if (VFD_CONTROLLER == VFD_HUANYANG_TYPE1)
#define VFD_SETRPM_CMD                                         \
	{                                                          \
		7, 6, MODBUS_FORCE_SINGLE_COIL, 0x02, 0x00, 0x00, 0x00 \
	}
#define VFD_GETRPM_CMD                                            \
	{                                                             \
		8, 8, MODBUS_READ_INPUT_REGISTERS, 0x03, 0x01, 0x00, 0x00 \
	}
// sets fixed freq 400.00Hz -> 40000
#define VFD_RPM_HZ_CMD                                        \
	{                                                         \
		8, 8, MODBUS_READ_COIL_STATUS, 0x03, 0x05, 0x00, 0x00 \
	}
#define VFD_CW_CMD                                                  \
	{                                                               \
		6, 6, MODBUS_READ_HOLDING_REGISTERS, 0x01, 0x01, 0x00, 0x00 \
	}
#define VFD_CCW_CMD                                                 \
	{                                                               \
		6, 6, MODBUS_READ_HOLDING_REGISTERS, 0x01, 0x11, 0x00, 0x00 \
	}
#define VFD_STOP_CMD                                                \
	{                                                               \
		6, 6, MODBUS_READ_HOLDING_REGISTERS, 0x01, 0x08, 0x00, 0x00 \
	}
#define VFD_IN_MULT g_settings.spindle_max_rpm
#define VFD_IN_DIV vfd_state.rpm_hz
#define VFD_OUT_MULT vfd_state.rpm_hz
#define VFD_OUT_DIV g_settings.spindle_max_rpm
#endif

#if (VFD_CONTROLLER == VFD_HUANYANG_TYPE2)
#define VFD_SETRPM_CMD                                              \
	{                                                               \
		8, 8, MODBUS_PRESET_SINGLE_REGISTER, 0x10, 0x00, 0x00, 0x00 \
	}
#define VFD_GETRPM_CMD                                              \
	{                                                               \
		8, 8, MODBUS_READ_HOLDING_REGISTERS, 0x70, 0x0C, 0x00, 0x02 \
	}
#define VFD_RPM_HZ_CMD                                              \
	{                                                               \
		8, 8, MODBUS_READ_HOLDING_REGISTERS, 0xB0, 0x05, 0x00, 0x02 \
	}
#define VFD_CW_CMD                                                  \
	{                                                               \
		8, 8, MODBUS_PRESET_SINGLE_REGISTER, 0x20, 0x00, 0x00, 0x01 \
	}
#define VFD_CCW_CMD                                                 \
	{                                                               \
		8, 8, MODBUS_PRESET_SINGLE_REGISTER, 0x20, 0x00, 0x00, 0x02 \
	}
#define VFD_STOP_CMD                                                \
	{                                                               \
		8, 8, MODBUS_PRESET_SINGLE_REGISTER, 0x20, 0x00, 0x00, 0x06 \
	}
#define VFD_IN_MULT g_settings.spindle_max_rpm
#define VFD_IN_DIV vfd_state.rpm_hz
#define VFD_OUT_MULT vfd_state.rpm_hz
#define VFD_OUT_DIV g_settings.spindle_max_rpm
#endif

#if (VFD_CONTROLLER == VFD_YL620)
#define VFD_SETRPM_CMD                                              \
	{                                                               \
		8, 8, MODBUS_PRESET_SINGLE_REGISTER, 0x20, 0x01, 0x00, 0x00 \
	}
#define VFD_GETRPM_CMD                                              \
	{                                                               \
		8, 8, MODBUS_READ_HOLDING_REGISTERS, 0x20, 0x0B, 0x00, 0x01 \
	}
#define VFD_RPM_HZ_CMD                                           \
	{                                                            \
		0, MODBUS_READ_HOLDING_REGISTERS, 0xB0, 0x05, 0x00, 0x02 \
	}
#define VFD_CW_CMD                                                  \
	{                                                               \
		8, 8, MODBUS_PRESET_SINGLE_REGISTER, 0x20, 0x00, 0x00, 0x12 \
	}
#define VFD_CCW_CMD                                                 \
	{                                                               \
		8, 8, MODBUS_PRESET_SINGLE_REGISTER, 0x20, 0x00, 0x00, 0x22 \
	}
#define VFD_STOP_CMD                                                \
	{                                                               \
		8, 8, MODBUS_PRESET_SINGLE_REGISTER, 0x20, 0x00, 0x00, 0x01 \
	}
#define VFD_IN_MULT 60.0f
#define VFD_IN_DIV 10.0f
#define VFD_OUT_MULT 10.0f
#define VFD_OUT_DIV 60.0f
#endif

#if (VFD_CONTROLLER == VFD_POWTRAN8100)
#define VFD_SETRPM_CMD                                              \
	{                                                               \
		8, 8, MODBUS_PRESET_SINGLE_REGISTER, 0x00, 0x01, 0x00, 0x00 \
	}
#define VFD_GETRPM_CMD                                              \
	{                                                               \
		8, 8, MODBUS_READ_HOLDING_REGISTERS, 0x00, 0x01, 0x00, 0x00 \
	}
#define VFD_RPM_HZ_CMD                                              \
	{                                                               \
		8, 7, MODBUS_READ_HOLDING_REGISTERS, 0x00, 0x0C, 0x00, 0x01 \
	}
#define VFD_RUN_CMD                                            \
	{                                                          \
		8, 8, MODBUS_FORCE_SINGLE_COIL, 0x00, 0x00, 0xFF, 0x00 \
	}
#define VFD_CW_CMD                                             \
	{                                                          \
		8, 8, MODBUS_FORCE_SINGLE_COIL, 0x00, 0x02, 0xFF, 0x00 \
	}
#define VFD_CCW_CMD                                            \
	{                                                          \
		8, 8, MODBUS_FORCE_SINGLE_COIL, 0x00, 0x02, 0x00, 0x00 \
	}
#define VFD_STOP_CMD                                           \
	{                                                          \
		8, 8, MODBUS_FORCE_SINGLE_COIL, 0x00, 0x00, 0x00, 0x00 \
	}
#define VFD_IN_MULT g_settings.spindle_max_rpm
#define VFD_IN_DIV vfd_state.rpm_hz
#define VFD_OUT_MULT vfd_state.rpm_hz
#define VFD_OUT_DIV g_settings.spindle_max_rpm
#endif

static bool modvfd_command(uint8_t *cmd, modbus_response_t *response)
{
	// checks if is dummy command
	if (!cmd[0])
	{
		memcpy(&response, &cmd[1], cmd[1]);
		return true;
	}
	modbus_request_t request = {0};
	memset(response, 0, sizeof(modbus_response_t));
	memcpy(&request, &cmd[1], 6);
	request.address = VFD_ADDRESS;
	uint8_t retries = VFD_MAX_COMMAND_RETRIES;
	vfd_state.needs_update = false;
	while (retries--)
	{
		send_request(request, cmd[0], &vfd_uart);
		if (read_response(response, cmd[1], &vfd_uart, VFD_TIMEOUT))
		{
			return true;
		}
#if (VFD_RETRY_DELAY_MS != 0)
		cnc_delay_ms(VFD_RETRY_DELAY_MS);
#endif
	}

	vfd_state.connected = 0;
	protocol_send_string(MSG_START);
	protocol_send_string(__romstr__("VFD COMMUNICATION FAILED"));
	protocol_send_string(MSG_END);
#ifdef VFD_HOLD_ON_ERROR
	cnc_call_rt_command(CMD_CODE_FEED_HOLD);
#endif
	return false;
}

static uint16_t vfd_rpm_hz(void)
{
	modbus_response_t response = {0};
	uint8_t cmd[7] = VFD_RPM_HZ_CMD;
	uint8_t data_len = cmd[1] - 4; // len-(addres+func if+crc)

	if (modvfd_command(cmd, &response))
	{
		return ((float)((((uint16_t)response.data[data_len - 2]) << 8) | response.data[data_len - 1]));
	}

	return 0;
}

static uint16_t vfd_get_rpm(bool truerpm)
{
	if (!vfd_state.loaded)
	{
		return 0;
	}

	if (truerpm)
	{
		modbus_response_t response = {0};
		uint8_t cmd[7] = VFD_GETRPM_CMD;
		uint8_t data_len = cmd[1] - 4; // len-(addres+func if+crc)

		if (modvfd_command(cmd, &response))
		{
			return (uint16_t)(((float)((((uint16_t)response.data[data_len - 2]) << 8) | response.data[data_len - 1])) * VFD_IN_MULT / VFD_IN_DIV);
		}
		return 0;
	}

	return (uint16_t)vfd_state.rpm;
}

static bool vfd_update_rpm(void)
{
	modbus_response_t response = {0};
	uint8_t cmd[7] = VFD_SETRPM_CMD;
	uint16_t hz = (uint16_t)lroundf((float)ABS(vfd_state.rpm) * VFD_OUT_MULT / VFD_OUT_DIV);
	// cmd starts at index 1 not at 0
	uint8_t i = cmd[0] - 4 + 1;
	cmd[i] = (uint8_t)(hz >> 8);
	i++;
	cmd[i] = (uint8_t)(hz & 0xFF);
#ifndef VFD_RUN_CMD
	return modvfd_command(cmd, &response);
#else
	if (!modvfd_command(cmd, &response))
	{
		return false;
	}
	uint8_t runcmd[7] = VFD_RUN_CMD;
	return modvfd_command(runcmd, &response);
#endif
}

static bool vfd_stop(void)
{
	modbus_response_t response = {0};
	uint8_t cmd[7] = VFD_STOP_CMD;

	if (!modvfd_command(cmd, &response))
	{
		return false;
	}

	vfd_state.running = VFD_STOPPED;

	return true;
}

static bool vfd_cw(void)
{
	if (vfd_state.running != VFD_RUN_CW)
	{
		modbus_response_t response = {0};
		uint8_t cmd[7] = VFD_CW_CMD;

		if (!modvfd_command(cmd, &response))
		{
			return false;
		}

		vfd_state.running = VFD_RUN_CW;
	}

	return true;
}

static bool vfd_ccw(void)
{
	if (vfd_state.running != VFD_RUN_CCW)
	{
		modbus_response_t response = {0};
		uint8_t cmd[7] = VFD_CCW_CMD;

		if (!modvfd_command(cmd, &response))
		{
			return false;
		}

		vfd_state.running = VFD_RUN_CCW;
	}

	return true;
}

static bool vfd_connect(void)
{
	if (!vfd_state.connected)
	{
		vfd_state.rpm_hz = vfd_rpm_hz();
		// was able do communicate via modbus
		if (!vfd_state.rpm_hz)
		{
			return false;
		}
	}

	vfd_state.connected = 1;
	return true;
}

uint8_t vfd_update(void)
{
	if (vfd_connect())
	{
		if (vfd_state.rpm == 0)
		{
			vfd_stop();
		}
		else
		{
			if (vfd_state.rpm < 0)
			{
				vfd_ccw();
			}
			else
			{
				vfd_cw();
			}

			vfd_update_rpm();
		}
	}

	return STATUS_OK;
}

/**
 *
 * Tool callbacks
 *
 * */

void startup_code()
{
	// initialize soft uart tx
	vfd_uart.tx(true);
	// cnc_delay_ms(200);
	vfd_state.rpm = 0;
	vfd_update();
}

void shutdown_code()
{
	vfd_state.rpm = 0;
	vfd_stop();
	vfd_state.connected = 0;
}

void set_speed(int16_t value)
{
	vfd_state.rpm = value;
	vfd_update();
}

void set_coolant(uint8_t value)
{
	SET_COOLANT(VFD_COOLANT_FLOOD, VFD_COOLANT_MIST, value);
}

uint16_t get_speed(void)
{
	return vfd_get_rpm(GET_SPINDLE_TRUE_RPM);
}

const tool_t vfd_modbus = {
	.startup_code = &startup_code,
	.shutdown_code = &shutdown_code,
#if PID_CONTROLLERS > 0
	.pid_update = NULL,
	.pid_error = NULL,
#endif
	.range_speed = NULL,
	.get_speed = &get_speed,
	.set_speed = &set_speed,
	.set_coolant = &set_coolant};

#endif
