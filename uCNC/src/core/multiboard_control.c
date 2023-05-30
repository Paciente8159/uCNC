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

slave_board_io_t g_slaves_io;

#ifdef IS_MASTER_BOARD
void multiboard_get_slave_boards_io(void)
{
	slave_board_io_t slaves;

	for (uint8_t slaveid = SLAVE_BOARDS_ADDRESS_OFFSET; slaveid < (SLAVE_BOARDS_ADDRESS_OFFSET + SLAVE_BOARDS_COUNT); slaveid++)
	{
		slave_board_io_t slave_data;
		if (master_get_response(slaveid, MULTIBOARD_CMD_SLAVE_IO, (uint8_t *)&slave_data, sizeof(slave_board_io_t), 2) != I2C_OK)
		{
			slave_data.slave_io_reg = 0xFFFFFFFF;
		}

		slaves.slave_io_reg |= slave_data.slave_io_reg;
	}

	g_slaves_io.slave_io_reg = slaves.slave_io_reg;
}

uint8_t multiboard_get_data(uint8_t cmd, uint16_t *data, uint16_t default_value, uint8_t datalen)
{
	// for (uint8_t slaveid = 1; slaveid <= SLAVE_BOARDS_COUNT; slaveid++)
	// {
	// 	*data = default_value;
	// 	if (master_send_command(slaveid, cmd, NULL, 0) == I2C_OK)
	// 	{
	// 		uint16_t val;
	// 		if (master_get_response(slaveid, (uint8_t *)&val, datalen, 2) == I2C_OK)
	// 		{
	// 			*data &= val;
	// 		}
	// 		else
	// 		{
	// 			return MULTIBOARD_CONTROL_RESPONSE_ERROR;
	// 		}
	// 	}
	// 	else
	// 	{
	// 		return MULTIBOARD_CONTROL_CMD_ERROR;
	// 	}
	// }

	return MULTIBOARD_CONTROL_OK;
}
#endif

__attribute__((weak)) uint8_t master_send_command(uint8_t address, uint8_t command, uint8_t *data, uint8_t datalen)
{
}

__attribute__((weak)) uint8_t master_get_response(uint8_t address, uint8_t command, uint8_t *data, uint8_t datalen, uint32_t timeout)
{
}

void slave_rcv_cb(uint8_t *data, uint8_t *datalen)
{
	// the CRC can be checked here if needed
	switch (data[0])
	{
		// sync states
	case MULTIBOARD_CMD_CNCSTATE:
		if (*datalen == 1)
		{
			data[0] = cnc_get_exec_state(EXEC_ALLACTIVE);
		}
		else
		{
			cnc_set_exec_state(data[1]);
			cnc_clear_exec_state(~data[1]);
		}
		break;
	case MULTIBOARD_CMD_CNCALARM:
		if (*datalen == 1)
		{
			data[0] = cnc_get_alarm();
		}
		else
		{
			cnc_alarm(data[1]);
		}
		break;
	case MULTIBOARD_CMD_SLAVE_IO:
		*datalen = sizeof(slave_board_io_t);
		memcpy(data, &g_slaves_io.slave_io_reg, sizeof(slave_board_io_t));
		break;
	}
}

#endif