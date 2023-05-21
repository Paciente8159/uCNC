/*
	Name: multiboard_protocol.c
	Description: This defines the µCNC multiboard protocol.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 21-05-2023

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

/**
 * This implements the multiboard control communication protocol using I2C
 *
 * **/

#include "../cnc.h"
#include <math.h>

#ifdef ENABLE_MULTIBOARD

uint8_t multiboard_get_byte(uint8_t cmd, uint8_t *var, uint8_t error_val, void *arg, uint8_t arglen)
{
	for (uint8_t slaveid = 1; slaveid <= SLAVE_BOARDS_COUNT; slaveid++)
	{
		uint8_t val = error_val;
		if (!master_send_command(slaveid, cmd, arg, arglen))
		{
			if (!master_get_response(&val, 1, 2))
			{
				*var |= val;
			}
			else
			{
				return MULTIBOARD_CONTROL_RESPONSE_ERROR;
			}
		}
		else
		{
			return MULTIBOARD_CONTROL_CMD_ERROR;
		}
	}

	return MULTIBOARD_CONTROL_OK;
}

#define PROPAGATION_TIME_IN_US ceilf(SLAVE_BOARDS_COUNT * 1000000.0f / BAUDRATE2)
#define PROTOCOL_TIMEOUT_IN_MS ceilf(PROPAGATION_TIME_IN_US / 1000.0f)

#ifdef IS_MASTER_BOARD

uint8_t master_send_command(uint8_t address, uint8_t command, void *data, uint8_t datalen)
{
	// the average time the message takes to propagate through the serial bus with passthrough
	uint8_t crc = 0;
	mcu_i2c_write(address, true, false);
	crc = crc7(address, crc);
	mcu_i2c_write(command, false, false);
	crc = crc7(command, crc);
	if (data && datalen)
	{
		uint8_t *ptr = data;
		do
		{
			mcu_i2c_write(*ptr, false, false);
			crc = crc7(*ptr, crc);
		} while (--datalen);
	}
	mcu_i2c_write(crc, false, true);

	if (address)
	{
		// mcu
	}
	else
	{
		// check slave ack for each slave
		for (uint8_t id = 1; id <= SLAVE_BOARDS_COUNT; id++)
		{
			//mcu_i2c_read()
		}
	}
}
uint8_t master_get_response(void *data, uint8_t datalen, uint32_t timeout)
{
}
#else
uint8_t slave_ack(void);
uint8_t slave_response(void *data, uint8_t datalen);
#endif

#endif