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
#include "../../../modules/modbus.h"
#include <math.h>
#include <stdbool.h>

#define COOLANT_FLOOD DOUT1
#define COOLANT_MIST DOUT2

#define VFD_TX_PIN DOUT26
#define VFD_RX_PIN DIN26
#define VFD_BAUDRATE 9600
#define VFD_RETRY_DELAY_MS 100

#if !(VFD_TX_PIN < 0) && !(VFD_RX_PIN < 0)
SOFTUART(vfd_uart, VFD_BAUDRATE, VFD_TX_PIN, VFD_RX_PIN)

typedef struct vfd_state
{
	uint8_t loaded : 1;
	uint8_t connected : 1;
	uint8_t ccw : 1;
	volatile uint8_t needs_update : 1;
	volatile int16_t rpm;
	float rpm_hz;
} vfd_state_t;

static vfd_state_t vfd_state;

/*VFD settings*/
#define VFD_ADDRESS 1
#define VFD_MAX_COMMAND_RETRIES 1

// uncomment the right type of VFD used
#define VFD_HUANYANG_TYPE1
// #define VFD_HUANYANG_TYPE2
// #define VFD_YL620

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
#ifdef VFD_HUANYANG_TYPE1
#define VFD_SETRPM_CMD                                         \
	{                                                          \
		7, 6, MODBUS_FORCE_SINGLE_COIL, 0x02, 0x00, 0x00, 0x00 \
	}
#define VFD_GETRPM_CMD                                            \
	{                                                             \
		8, 8, MODBUS_READ_INPUT_REGISTERS, 0x03, 0x01, 0x00, 0x00 \
	}
#define VFD_RPM_HZ_CMD                                        \
	{                                                         \
		8, 8, MODBUS_READ_COIL_STATUS, 0x03, 0x90, 0x00, 0x00 \
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
#define VFD_IN_MULT vfd_state.rpm_hz
#define VFD_IN_DIV 5000.0f
#define VFD_OUT_MULT 5000.0f
#define VFD_OUT_DIV vfd_state.rpm_hz
#endif

#ifdef VFD_HUANYANG_TYPE2
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
#define VFD_IN_MULT vfd_state.rpm_hz
#define VFD_IN_DIV 5000.0f
#define VFD_OUT_MULT 5000.0f
#define VFD_OUT_DIV vfd_state.rpm_hz
#endif

#ifdef VFD_YL620
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

static bool modvfd_command(uint8_t *cmd, modbus_response_t *response)
{
	// checks if is dummy command
	if (!cmd[0])
	{
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
		if (read_response(response, cmd[1], &vfd_uart))
		{
			return true;
		}
		cnc_delay_ms(VFD_RETRY_DELAY_MS);
	}

	vfd_state.connected = 0;
	return false;
}

static uint16_t vfd_rpm_hz(void)
{
	modbus_response_t response = {0};
	uint8_t cmd[7] = VFD_RPM_HZ_CMD;

	if (modvfd_command(cmd, &response))
	{
		return ((float)((((uint16_t)response.data[2]) << 8) | response.data[3]));
	}

	return -1;
}

static uint16_t vfd_get_rpm(bool truerpm)
{
	if (!vfd_state.loaded)
	{
		return -1;
	}

	if (truerpm)
	{
		modbus_response_t response = {0};
		uint8_t cmd[7] = VFD_GETRPM_CMD;

		if (modvfd_command(cmd, &response))
		{
			return (uint16_t)(((float)((((uint16_t)response.data[2]) << 8) | response.data[3])) * VFD_IN_MULT / VFD_IN_DIV);
		}
		return -1;
	}

	return (uint16_t)vfd_state.rpm;
}

static bool vfd_update_rpm(void)
{
	modbus_response_t response = {0};
	if (vfd_state.rpm != 0)
	{
		uint8_t cmd[7] = VFD_SETRPM_CMD;
		uint16_t hz = (uint16_t)roundf((float)ABS(vfd_state.rpm) * VFD_OUT_MULT / VFD_OUT_DIV);
		uint8_t i = cmd[0] - 4;
		cmd[i] = (uint8_t)(hz >> 8);
		i++;
		cmd[i] = (uint8_t)(hz & 0xFF);
		return modvfd_command(cmd, &response);
	}
	else
	{
		uint8_t cmd[7] = VFD_STOP_CMD;
		return modvfd_command(cmd, &response);
	}
}

static void vfd_cw(void)
{
	if (vfd_state.ccw)
	{
		modbus_response_t response = {0};
		uint8_t cmd[7] = VFD_CW_CMD;

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
		uint8_t cmd[7] = VFD_CCW_CMD;

		if (modvfd_command(cmd, &response))
		{
			vfd_state.ccw = 1;
		}
	}
}

/**
 *
 * Module callbacks
 * Vfd update will not happen in the ISR due to the slow speed communication
 * It will be signaled to be updated in the main loop
 *
 * */

#ifdef ENABLE_MAIN_LOOP_MODULES
uint8_t vfd_update(void *args, bool *handled)
{
	if (!vfd_state.connected)
	{
		vfd_state.rpm_hz = vfd_rpm_hz();
		// was able do communicate via modbus
		if (vfd_state.rpm_hz >= 0)
		{
			vfd_state.connected = 1;
		}
		else
		{
			protocol_send_error(STATUS_VFD_COMMUNICATION_FAILED);
			return STATUS_VFD_COMMUNICATION_FAILED;
		}
	}

	if (vfd_state.needs_update)
	{
		if (vfd_state.rpm < 0)
		{
			vfd_ccw();
		}
		else
		{
			vfd_cw();
		}

		vfd_state.needs_update = 0;
		if (!vfd_update_rpm())
		{
			protocol_send_error(STATUS_VFD_COMMUNICATION_FAILED);
			return STATUS_VFD_COMMUNICATION_FAILED;
		}
	}

	return STATUS_OK;
}

CREATE_EVENT_LISTENER(cnc_dotasks, vfd_update);
#endif

DECL_MODULE(vfd)
{
#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_dotasks, vfd_update);
#else
#warning "VFD tool requires ENABLE_MAIN_LOOP_MODULES option."
#endif
}

/**
 *
 * Tool callbacks
 *
 * */

void vfd_startup()
{
	if (!vfd_state.loaded)
	{
		vfd_init();
		vfd_state.loaded = 1;
	}

	if (!vfd_state.connected)
	{
		vfd_rpm_hz();
		// was able do communicate via modbus
		if (vfd_state.rpm_hz >= 0)
		{
			vfd_state.connected = 1;
		}
	}

	vfd_state.rpm = 0;
	vfd_state.needs_update = 1;
}

void vfd_shutdown()
{
	vfd_state.rpm = 0;
	vfd_state.needs_update = 1;
	vfd_state.connected = 0;
	vfd_update_rpm();
}

void vfd_set_speed(int16_t value)
{
	if (vfd_state.rpm != value)
	{
		vfd_state.rpm = value;
		vfd_state.needs_update = 1;
	}
}

void vfd_set_coolant(uint8_t value)
{
	SET_COOLANT(COOLANT_FLOOD, COOLANT_MIST, value);
}

uint16_t vfd_get_speed(void)
{
	return vfd_get_rpm(true);
}

const tool_t __rom__ vfd = {
	.startup_code = &vfd_startup,
	.shutdown_code = &vfd_shutdown,
#if PID_CONTROLLERS > 0
	.pid_update = NULL,
	.pid_error = NULL,
#endif
	.range_speed = NULL,
	.get_speed = &vfd_get_speed,
	.set_speed = &vfd_set_speed,
	.set_coolant = &vfd_set_coolant};

#endif
