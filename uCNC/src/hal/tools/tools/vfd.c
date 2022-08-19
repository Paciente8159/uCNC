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

#define COOLANT_FLOOD DOUT1
#define COOLANT_MIST DOUT2

#define VFD_TX_PIN DOUT26
#define VFD_RX_PIN DIN26
#define VFD_BAUDRATE 9600
#define VFD_RETRY_DELAY_MS 100

SOFTUART(vfd_uart, VFD_BAUDRATE, VFD_TX_PIN, VFD_RX_PIN)

/*VFD settings*/
#define VFD_ADDRESS 1
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
	uint8_t loaded : 1;
	uint8_t ccw : 1;
	volatile uint8_t needs_update : 1;
	volatile int16_t rpm;
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
		if (read_response(response, &vfd_uart))
		{
			return true;
		}
		protocol_send_error(STATUS_VFD_COMMUNICATION_FAILED);
		cnc_delay_ms(VFD_RETRY_DELAY_MS);
	}

	return false;
}

static void vfd_rpm_hz(void)
{
	modbus_response_t response = {0};
	uint8_t cmd[6] = VFD_RPM_HZ_CMD;

	if (modvfd_command(cmd, &response))
	{
		vfd_state.rpm_hz = ((float)((((uint16_t)response.data[1]) << 8) | response.data[2]));
	}

	vfd_state.rpm_hz = -1;
}

static uint16_t vfd_get_rpm(bool truerpm)
{
	if (truerpm)
	{
		modbus_response_t response = {0};
		uint8_t cmd[6] = VFD_GETRPM_CMD;

		if (modvfd_command(cmd, &response))
		{
			return (uint16_t)(((float)((((uint16_t)response.data[1]) << 8) | response.data[2])) * VFD_IN_MULT / VFD_IN_DIV);
		}
		return -1;
	}

	return (uint16_t)vfd_state.rpm;
}

static void vfd_update_rpm(void)
{
	modbus_response_t response = {0};
	if (vfd_state.rpm != 0)
	{
		uint8_t cmd[6] = VFD_SETRPM_CMD;
		uint16_t hz = vfd_state.rpm * VFD_OUT_MULT / VFD_OUT_DIV;
		cmd[3] = (uint8_t)(hz >> 8);
		cmd[4] = (uint8_t)(hz & 0xFF);
		modvfd_command(cmd, &response);
	}
	else
	{
		uint8_t cmd[6] = VFD_STOP_CMD;
		modvfd_command(cmd, &response);
	}
}

static void vfd_cw(void)
{
	if (vfd_state.ccw)
	{
		modbus_response_t response = {0};
		uint8_t cmd[6] = VFD_CW_CMD;

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
		uint8_t cmd[6] = VFD_CCW_CMD;

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

		vfd_update_rpm();
	}

	vfd_state.needs_update = 0;
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
		vfd_rpm_hz();
		// was able do communicate via modbus
		if (vfd_state.rpm_hz >= 0)
		{
			vfd_init();
			vfd_state.loaded = 1;
		}
	}

	vfd_state.rpm = 0;
	vfd_state.needs_update = 1;
}

void vfd_shutdown()
{
	vfd_state.rpm = 0;
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
