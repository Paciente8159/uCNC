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

#ifndef MULTIBOARD_RESPONSE_TIMEOUT
#define MULTIBOARD_RESPONSE_TIMEOUT 10
#endif

#define MULTIBOARD_CMD_SET_CNC_STATE 0xB1
#define MULTIBOARD_CMD_CLEAR_CNC_STATE 0xB2
#define MULTIBOARD_CMD_SLAVE_IO 0xB3
#define MULTIBOARD_CMD_CNCSETTINGS 0xC1
#define MULTIBOARD_CMD_ITPBLOCK 0xD1
#define MULTIBOARD_CMD_ITPBLOCK_WRITE 0xD2
#define MULTIBOARD_CMD_ITPSEGMENT 0xD3
#define MULTIBOARD_CMD_ITPRUN 0xD4
#define MULTIBOARD_CMD_ITPPOS_RESET 0xD5

// commands that expect a response from the slave
#define MULTIBOARD_CMDS_REQUEST_BASE 0xE0

#define MULTIBOARD_CMD_REQUEST_LIMITS (MULTIBOARD_CMDS_REQUEST_BASE + 0)
#define MULTIBOARD_CMD_REQUEST_LIMITS_DUAL (MULTIBOARD_CMDS_REQUEST_BASE + 1)
#define MULTIBOARD_CMD_REQUEST_CONTROLS (MULTIBOARD_CMDS_REQUEST_BASE + 2)
#define MULTIBOARD_CMD_REQUEST_PROBE (MULTIBOARD_CMDS_REQUEST_BASE + 3)

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

	uint8_t master_send_command(uint8_t address, uint8_t command, uint8_t *data, uint8_t datalen);
	uint8_t master_get_response(uint8_t address, uint8_t command, uint8_t *data, uint8_t datalen, uint32_t timeout);
	uint8_t master_get_combined_responses(uint8_t command, uint8_t *data, uint8_t datalen, uint32_t timeout);
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

#define MULTIBOARD_SLAVE_SET_STATE(statemask) master_send_command(0, MULTIBOARD_CMD_SET_CNC_STATE, (uint8_t *)&statemask, 1)
#define MULTIBOARD_SLAVE_CLEAR_STATE(statemask) master_send_command(0, MULTIBOARD_CMD_CLEAR_CNC_STATE, (uint8_t *)&statemask, 1)
#define MULTIBOARD_SLAVE_SET_SETTINGS() master_send_command(0, MULTIBOARD_CMD_CNCSETTINGS, (uint8_t *)&g_settings, sizeof(settings_t))
#define MULTIBOARD_SLAVE_SET_ITPBLOCK(block, blocklen) master_send_command(0, MULTIBOARD_CMD_ITPBLOCK, (uint8_t *)block, blocklen);
#define MULTIBOARD_SLAVE_SET_ITPSEGMENT(segm, segmlen) master_send_command(0, MULTIBOARD_CMD_ITPSEGMENT, (uint8_t *)segm, segmlen);

#define MULTIBOARD_SYNC_IOOUTPUT(output, inputval)                                    \
	{                                                                                 \
		uint8_t packet[2] = {output, inputval};                                       \
		master_send_command(0, MULTIBOARD_CMD_PIN, (uint8_t *)output, 0, &packet, 2); \
	}

#define MULTIBOARD_SLAVE_GET_LIMITS(limits) master_get_combined_responses(MULTIBOARD_CMD_REQUEST_LIMITS, (uint8_t *)limits, 1, MULTIBOARD_RESPONSE_TIMEOUT);
#define MULTIBOARD_SLAVE_GET_LIMITS_DUAL(limits) master_get_combined_responses(MULTIBOARD_CMD_REQUEST_LIMITS_DUAL, (uint8_t *)limits, 1, MULTIBOARD_RESPONSE_TIMEOUT);
#define MULTIBOARD_SLAVE_GET_CONTROLS(controls) master_get_combined_responses(MULTIBOARD_CMD_REQUEST_CONTROLS, (uint8_t *)controls, 1, MULTIBOARD_RESPONSE_TIMEOUT);
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