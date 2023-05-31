/*
	Name: multiboard_protocol.h
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

#ifndef MULTIBOARD_CONTROL_H
#define MULTIBOARD_CONTROL_H
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef ENABLE_MULTIBOARD

#define MULTIBOARD_CONTROL_OK 0
#define MULTIBOARD_CONTROL_CMD_ERROR 1
#define MULTIBOARD_CONTROL_RESPONSE_ERROR 2

#define MULTIBOARD_CMD_CNCSTATE 0xB1
#define MULTIBOARD_CMD_CNCALARM 0xB2
#define MULTIBOARD_CMD_CONTROLS 0xB3
#define MULTIBOARD_CMD_LIMITS 0xB4
#define MULTIBOARD_CMD_PROBE 0xB5
#define MULTIBOARD_CMD_PIN 0xB6
#define MULTIBOARD_CMD_SLAVE_IO 0xB7
#define MULTIBOARD_CMD_CNCSETTINGS 0xC1
#define MULTIBOARD_CMD_ITPBLOCK 0xD1
#define MULTIBOARD_CMD_ITPBLOCK_ADVANCE 0xD2
#define MULTIBOARD_CMD_ITPSEGMENT 0xD3
#define MULTIBOARD_CMD_ITPRUN 0xD4
#define MULTIBOARD_CMD_ITPPOS_RESET 0xD5

// commands that expect a response from the slave
#define MULTIBOARD_REQUEST_CMDS_BASE 0xE0

#define MULTIBOARD_REQUEST_CMD_SLAVE_IO (MULTIBOARD_REQUEST_CMDS_BASE + 1)

#define MULTIBOARD_CMD_CONFIRM_CRC 0xA3
#define MULTIBOARD_CMD_SLAVE_ACK 0xA5

	/**
	 * Multiboard communication is based on these functions and different communication protocols can be used/defined to allow multiboard support
	 * The protocol assumes the following structure:
	 * - there is a master device and one or more slave boards
	 * - each slave board as an address and all addresses are consecutive (starting from 1 to N slave boards)
	 * - all communications are started by the master
	 *
	 *
	 * **/

	/**
	 * Master send command sends a command with an address and an optional data payload to the slave
	 * This assumes
	 * - Data payload is optional
	 * - The command expects either a slave response with a given timeout (usually not more then a millisecond depending on the comunication topology and number of slaves)
	 *
	 * Sending the command with an address of 0 is the same as sending a sync/broadcast command directed to all slaves in the bus
	 * **/

	typedef union slave_board_io_
	{
		uint32_t slave_io_reg;
		struct
		{
			uint8_t state;
			uint8_t probe : 1;
			uint8_t limits2 : 3;
			uint8_t controls : 4;
			uint8_t limits;
			uint8_t onchange_inputs;
		} slave_io_bits;
	} slave_board_io_t;
	extern slave_board_io_t g_slaves_io;

	uint8_t master_send_command(uint8_t address, uint8_t command, uint8_t *data, uint8_t datalen);
	uint8_t master_get_response(uint8_t address, uint8_t command, uint8_t *data, uint8_t datalen, uint32_t timeout);
	// master to slave data received callback
	void slave_rcv_cb(uint8_t *data, uint8_t *datalen);
	// slave to master data request callback (request always comes from master)
	void slave_rqst_cb(uint8_t *data, uint8_t *datalen);

	/**
	 *
	 * Sync/Broadcast commands
	 * Sync/Broadcast commands send commands with address 0 and all slaves must listen to them
	 *
	 * **/

// #define MULTIBOARD_SYNC_CNCSTATE(state) master_send_command(0, MULTIBOARD_CMD_CNCSTATE, &state, 1)
// #define MULTIBOARD_SYNC_CNCALARM(alarm) master_send_command(0, MULTIBOARD_CMD_CNCALARM, &alarm, 1)
#define MULTIBOARD_SYNC_CNCSETTINGS() master_send_command(0, MULTIBOARD_CMD_CNCSETTINGS, (uint8_t*)&g_settings, sizeof(settings_t))
// #define MULTIBOARD_SYNC_ITPBLOCK(block, blocklen) master_send_command(0, MULTIBOARD_CMD_ITPBLOCK, &block, blocklen);
// #define MULTIBOARD_SYNC_ITPSEGMENT(segm, segmlen) master_send_command(0, MULTIBOARD_CMD_ITPSEGMENT, &segm, segmlen);

#define MULTIBOARD_SYNC_IOOUTPUT(output, inputval)                         \
	{                                                                      \
		uint8_t packet[2] = {output, inputval};                            \
		master_send_command(0, MULTIBOARD_CMD_PIN, (uint8_t*)output, 0, &packet, 2); \
	}

	void multiboard_set_slave_boards_io(void);
	void multiboard_get_slave_boards_io(void);

	// #define MULTIBOARD_GET_SLAVESTATE(state) multiboard_get_data(MULTIBOARD_CMD_CNCSTATE, state, EXEC_ALLACTIVE, NULL, 0)
	// queries slave boards for the limits, controls and probe pins
	// #define MULTIBOARD_GET_SPECIAL_INPUTS(sp_inputs) multiboard_get_data(MULTIBOARD_CMD_CONTROLS, sp_inputs, 0xFFFF, NULL, 0)
	// #define MULTIBOARD_GET_IOLIMITS(limits) multiboard_get_byte(MULTIBOARD_CMD_LIMITS, limits, 0, NULL, 0)
	// #define MULTIBOARD_GET_IOPROBE(probe) multiboard_get_byte(MULTIBOARD_CMD_PROBE, probe, 0, NULL, 0)
	// #define MULTIBOARD_GET_IOINPUT(input, inputval) multiboard_get_byte(MULTIBOARD_CMD_PIN, inputval, 0, &input, 1)
	

#endif

#ifdef __cplusplus
}
#endif