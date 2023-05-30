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
void multiboard_set_slave_boards_io(void)
{
	master_send_command(0, MULTIBOARD_CMD_SLAVE_IO, (uint8_t *)&g_slaves_io, sizeof(slave_board_io_t));
}

void multiboard_get_slave_boards_io(void)
{
	slave_board_io_t slaves;

	for (uint8_t slaveid = SLAVE_BOARDS_ADDRESS_OFFSET; slaveid < (SLAVE_BOARDS_ADDRESS_OFFSET + SLAVE_BOARDS_COUNT); slaveid++)
	{
		slave_board_io_t slave_data;
		if (master_get_response(slaveid, MULTIBOARD_REQUEST_CMD_SLAVE_IO, (uint8_t *)&slave_data, sizeof(slave_board_io_t), 2) != I2C_OK)
		{
			slave_data.slave_io_reg = 0xFFFFFFFF;
		}

		slaves.slave_io_reg |= slave_data.slave_io_reg;
	}

	g_slaves_io.slave_io_reg = slaves.slave_io_reg;
}

__attribute__((weak)) uint8_t master_send_command(uint8_t address, uint8_t command, uint8_t *data, uint8_t datalen)
{
	return MULTIBOARD_CONTROL_OK;
}

__attribute__((weak)) uint8_t master_get_response(uint8_t address, uint8_t command, uint8_t *data, uint8_t datalen, uint32_t timeout)
{
	return MULTIBOARD_CONTROL_OK;
}

#else
void slave_rcv_cb(uint8_t *data, uint8_t *datalen)
{
	// the CRC can be checked here if needed
	uint8_t cmd = data[0];
	uint8_t *ptr = &data[1];
	switch (cmd)
	{
	case MULTIBOARD_CMD_SLAVE_IO:
		cnc_set_exec_state(((slave_board_io_t *)ptr)->slave_io_bits.state);
		cnc_clear_exec_state(~((slave_board_io_t *)ptr)->slave_io_bits.state);
		g_slaves_io.slave_io_reg = ((slave_board_io_t *)ptr)->slave_io_reg;
		break;
	case MULTIBOARD_CMD_ITPBLOCK:
		itp_add_block(ptr);
		break;
	case MULTIBOARD_CMD_ITPBLOCK_ADVANCE:
		itp_blk_buffer_write();
		break;
	case MULTIBOARD_CMD_ITPSEGMENT:
		itp_add_segment(ptr);
		break;
	case MULTIBOARD_CMD_ITPRUN:
		itp_start(*ptr);
		break;
	case MULTIBOARD_CMD_ITPPOS_RESET:
		itp_reset_rt_position((float *)ptr);
		break;
	}
}

void slave_rqst_cb(uint8_t *data, uint8_t *datalen)
{
	// the CRC can be checked here if needed
	switch (data[0])
	{
	case MULTIBOARD_REQUEST_CMD_SLAVE_IO:
		*datalen = sizeof(slave_board_io_t);
		memcpy(data, &g_slaves_io.slave_io_reg, sizeof(slave_board_io_t));
		break;
	}
}
#endif

#endif