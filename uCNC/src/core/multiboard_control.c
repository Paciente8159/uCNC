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

#ifdef IS_MASTER_BOARD
uint8_t multiboard_get_byte(uint8_t cmd, uint8_t *var, uint8_t error_val, void *arg, uint8_t arglen)
{
	//mcu_i2c_send(0, &cmd, 1, true);

	// for (uint8_t slaveid = 1; slaveid <= SLAVE_BOARDS_COUNT; slaveid++)
	// {
	// 	uint8_t val = error_val;
	// 	//master_send_command(0, cmd, &val, 1);
	// 	/*if (master_send_command(slaveid, cmd, arg, arglen) == I2C_OK)
	// 	{
	// 		if (master_get_response(slaveid, &val, 1, 2) == I2C_OK)
	// 		{
	// 			*var |= val;
	// 		}
	// 		else
	// 		{
	// 			return MULTIBOARD_CONTROL_RESPONSE_ERROR;
	// 		}
	// 	}
	// 	else
	// 	{
	// 		return MULTIBOARD_CONTROL_CMD_ERROR;
	// 	}*/
	// }

	return MULTIBOARD_CONTROL_OK;
}
#endif

#if MULTIBOARD_IPC == IPC_I2C

#ifdef IS_MASTER_BOARD
#ifndef I2C_BUFFER_SIZE
#define I2C_BUFFER_SIZE 48
#endif

uint8_t master_send_command(uint8_t address, uint8_t command, void *data, uint8_t datalen)
{
	uint8_t buffer[I2C_BUFFER_SIZE];
	buffer[0] = command;
	if (data && datalen)
	{
		memcpy(&buffer[1], data, datalen);
	}

	return mcu_i2c_send(address, buffer, datalen + 1, true);
}

uint8_t master_get_response(uint8_t address, uint8_t *data, uint8_t datalen, uint32_t timeout)
{
	return 0;//mcu_i2c_receive(address, data, datalen, timeout);
}
#else

// overrides the I2C slave callback
MCU_IO_CALLBACK void mcu_i2c_slave_cb(uint8_t *data, uint8_t datalen)
{
	// all data request commands only have the command id and the crc
	bool isrequest = (datalen == 2);
	// the CRC can be checked here if needed
	switch (data[0])
	{
		// sync states
	case MULTIBOARD_CMD_CNCSTATE:
		if (isrequest)
		{
			data[0] = cnc_get_exec_state(EXEC_ALLACTIVE);
		}
		else
		{
			cnc_set_exec_state(data[1]);
		}
		break;
	}
}

#endif
#endif

#endif