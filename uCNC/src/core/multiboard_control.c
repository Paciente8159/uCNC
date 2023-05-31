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

__attribute__((weak)) uint8_t master_send_command(uint8_t address, uint8_t command, uint8_t *data, uint8_t datalen)
{
	return MULTIBOARD_CONTROL_OK;
}

__attribute__((weak)) uint8_t master_get_response(uint8_t address, uint8_t command, uint8_t *data, uint8_t datalen, uint32_t timeout)
{
	return MULTIBOARD_CONTROL_OK;
}

uint8_t master_get_combined_responses(uint8_t command, uint8_t *data, uint8_t datalen, uint32_t timeout)
{
	for (uint8_t slaveid = SLAVE_BOARDS_ADDRESS_OFFSET; slaveid < (SLAVE_BOARDS_ADDRESS_OFFSET + SLAVE_BOARDS_COUNT); slaveid++)
	{
		uint8_t buffer[datalen + 1];
		if (master_get_response(slaveid, command, buffer, datalen, timeout) != MULTIBOARD_CONTROL_OK)
		{
			memset(data, 0xFF, datalen);
		}

		for (uint8_t i = 0; i < datalen; i++)
		{
			data[i] |= buffer[i + 1];
		}
	}

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
	case MULTIBOARD_CMD_SET_CNC_STATE:
		cnc_set_exec_state(data[1]);
		break;
	case MULTIBOARD_CMD_CLEAR_CNC_STATE:
		cnc_clear_exec_state(data[1]);
		break;
	case MULTIBOARD_CMD_ITPBLOCK:
		itp_add_block(ptr);
		break;
	case MULTIBOARD_CMD_ITPBLOCK_WRITE:
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
	case MULTIBOARD_CMD_CNCSETTINGS:
		memcpy(&g_settings, ptr, sizeof(settings_t));
		break;
	}
}

void slave_rqst_cb(uint8_t *data, uint8_t *datalen)
{
	// the CRC can be checked here if needed
	switch (data[0])
	{
	case MULTIBOARD_CMD_REQUEST_LIMITS:
		*datalen = 2;
		data[1] = io_get_limits();
		break;
	case MULTIBOARD_CMD_REQUEST_LIMITS_DUAL:
		*datalen = 2;
		data[1] = io_get_limits_dual();
		break;
	case MULTIBOARD_CMD_REQUEST_CONTROLS:
		*datalen = 2;
		data[1] = io_get_controls();
		break;
	}
}
#endif

#endif