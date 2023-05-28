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

#ifdef __cplusplus
extern "C"
{
#endif

#include "../cnc.h"

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
#define MULTIBOARD_CMD_CNCSETTINGS 0xC1
#define MULTIBOARD_CMD_ITPBLOCK 0xD1
#define MULTIBOARD_CMD_ITPSEGMENT 0xD2
#define MULTIBOARD_CMD_ITPRUN 0xD3

/**
 * Multiboard communication is based on these functions and different communication protocols can be used/defined to allow multiboard support
 * The protocol assumes the following structure:
 * - there is a master device and one or more slave boards
 * - each slave board as an address and all addresses are consecutive (starting from 1 to N slave boards)
 * - all communications are started by the master
 *
 * Communications from slave to master are only possible under two circumstances:
 * - if the master requested information to that slave and the slave must return information
 * - if a broadcast command (with address 0) is sent, each slave sends an ACK in order of their address.
 * - for example slave 2 will send the ACK after the full message as been received and the ACK from slave 1 as been sent to the bus
 * - in the end a broadcast will only be successfull if all ACK are received within the designated maximum timeout
 *
 * Each of the 4 multiboard function can be implemented using any type of communication port (UART, I2C, SPI or other), either via harware drivers or software
 * For better performance the number of slave boards should be kept to a minimum and fast harware communication dirvers should be used
 * Each function is explained so custom implementations could be developed with minimal efford
 *
 * **/

/**
 * Master send command sends a command with an address and an optional data payload to the slave
 * This assumes
 * - Data payload is optional
 * - The command must sends a CRC-7 byte at the end
 * - The command expects either a slave response with a given timeout (usually not more then a millisecond depending on the comunication topology and number of slaves)
 *
 * Sending the command with an address of 0 is the same as sending a sync/broadcast command directed to all slaves in the bus
 * **/
#ifdef IS_MASTER_BOARD
	uint8_t master_send_command(uint8_t address, uint8_t command, void *data, uint8_t datalen);
	uint8_t master_get_response(uint8_t address, uint8_t *data, uint8_t datalen, uint32_t timeout);
#else
	uint8_t slave_ack(void);
	uint8_t slave_response(void *data, uint8_t datalen);
#endif

	/**
	 *
	 * Sync/Broadcast commands
	 * Sync/Broadcast commands send commands with address 0 and all slaves must listen to them
	 *
	 * **/

#define MULTIBOARD_SYNC_CNCSTATE(state) master_send_command(0, MULTIBOARD_CMD_CNCSTATE, &state, 1)
#define MULTIBOARD_SYNC_CNCALARM(alarm) master_send_command(0, MULTIBOARD_CMD_CNCALARM, &alarm, 1)
#define MULTIBOARD_SYNC_CNCSETTINGS() master_send_command(0, MULTIBOARD_CMD_CNCSETTINGS, &g_settings, sizeof(settings_t))
#define MULTIBOARD_SYNC_ITPBLOCK(block, blocklen) master_send_command(0, MULTIBOARD_CMD_ITPBLOCK, &block, blocklen);
#define MULTIBOARD_SYNC_ITPSEGMENT(segm, segmlen) master_send_command(0, MULTIBOARD_CMD_ITPSEGMENT, &segm, segmlen);

#define MULTIBOARD_SYNC_IOOUTPUT(output, inputval)                         \
	{                                                                      \
		uint8_t packet[2] = {output, inputval};                            \
		master_send_command(0, MULTIBOARD_CMD_PIN, output, 0, &packet, 2); \
	}

	uint8_t multiboard_get_byte(uint8_t cmd, uint8_t *var, uint8_t error_val, void *arg, uint8_t arglen);

#define MULTIBOARD_GET_CNCSTATE(state) multiboard_get_byte(MULTIBOARD_CMD_CNCSTATE, state, EXEC_ALLACTIVE, NULL, 0)
#define MULTIBOARD_GET_IOCONTROLS(controls) multiboard_get_byte(MULTIBOARD_CMD_CONTROLS, controls, 0, NULL, 0)
#define MULTIBOARD_GET_IOLIMITS(limits) multiboard_get_byte(MULTIBOARD_CMD_LIMITS, limits, 0, NULL, 0)
#define MULTIBOARD_GET_IOPROBE(probe) multiboard_get_byte(MULTIBOARD_CMD_PROBE, probe, 0, NULL, 0)
#define MULTIBOARD_GET_IOINPUT(input, inputval) multiboard_get_byte(MULTIBOARD_CMD_PIN, inputval, 0, &input, 1)

#endif

#ifdef __cplusplus
}
#endif