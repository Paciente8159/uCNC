/*
	Name: multiboard_protocol.h
	Description: µCNC implementation of a multiboard send-response protocol

	This is loselly based on min-protocol <https://github.com/min-protocol/min>

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 02/06/2023

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef MULTIBOARD_PROTOCOL_H
#define MULTIBOARD_PROTOCOL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#ifndef MULTIBOARD_PROTOCOL_TIMEOUT_MS
#define MULTIBOARD_PROTOCOL_TIMEOUT_MS 100
#endif
#ifndef MULTIBOARD_PROTOCOL_RETRIES
#define MULTIBOARD_PROTOCOL_RETRIES 5
#endif
#ifndef MULTIBOARD_BUFFER_SIZE
#define MULTIBOARD_BUFFER_SIZE 48
#endif

#ifdef MULTIBOARD_PROTOCOL_DEBUG
#define MULTIBOARD_DEBUG_MSG(X) protocol_send_string(X)
#else
#define MULTIBOARD_DEBUG_MSG(X)
#endif

	typedef union multiboard_data_
	{
		uint8_t rawdata[MULTIBOARD_BUFFER_SIZE];
		struct
		{
			uint8_t : 1; // always 0
			uint8_t msgid : 3;
			uint8_t ack : 4;
			union
			{
				uint8_t rawcmd;
				struct
				{
					uint8_t : 1; // always 0
					uint8_t from_slave : 1;
					uint8_t cmd : 6;
				} cmd_regs;
			};
			uint8_t : 2; // always 0
			uint8_t length : 6;
			uint8_t data[MULTIBOARD_BUFFER_SIZE - 3];
		} multiboard_frame;
	} multiboard_data_t;

	typedef union slave_board_io_
	{
		uint32_t slave_io_reg;
		struct
		{
			uint8_t state;			 // internal slave state
			uint8_t probe : 1;		 // probe
			uint8_t limits2 : 3;	 // dual limits
			uint8_t controls : 4;	 // controls
			uint8_t limits;			 // limits
			uint8_t onchange_inputs; // interruptable inputs
		} slave_io_bits;
	} slave_board_io_t;

	typedef struct slave_board_state_
	{
		slave_board_io_t io;			   // slave io
		int32_t itp_rt_pos[STEPPER_COUNT]; // slave rt position
	} slave_board_state_t;

	extern slave_board_state_t g_multiboard_slave;

#define MULTIBOARD_PROTOCOL_OK 0
#define MULTIBOARD_PROTOCOL_TIMEOUT 1
#define MULTIBOARD_PROTOCOL_ERROR 2
#define MULTIBOARD_PROTOCOL_UNEXPECTED_MESSAGE 3

#define MULTIBOARD_PROTOCOL_SOF 0xAA
#define MULTIBOARD_PROTOCOL_EOF 0x55
#define MULTIBOARD_PROTOCOL_ACK 0xFE  // ACK
#define MULTIBOARD_PROTOCOL_NACK 0xFD // NACK

// these codes might not be used
// since this will work on a send+response base ensure command order will not be necessary
// #define MULTIBOARD_PROTOCOL_SYNC 0xF846      // SYNC+CRC
// #define MULTIBOARD_PROTOCOL_SYNC_LOST 0xF94F // SYNC_LOST+CRC

// list of commands

// Master to slave commands
#define MULTIBOARD_CMD_SET_STATE 0x01
#define MULTIBOARD_CMD_CLEAR_STATE 0x02
#define MULTIBOARD_CMD_ITP_BLOCK 0x03
#define MULTIBOARD_CMD_ITP_SEGMENT 0x04
#define MULTIBOARD_CMD_ITP_BLOCK_WRITE 0x05
#define MULTIBOARD_CMD_ITP_START 0x06
#define MULTIBOARD_CMD_ITP_POS_RESET 0x07
#define MULTIBOARD_CMD_SET_OUTPUT 0x08

// Master to slave request commands
#define MULTIBOARD_CMD_GET_ITP_POS 0x10
#define MULTIBOARD_CMD_GET_PIN 0x11

// Slave to master messages
#define MULTIBOARD_SLAVE_BASE_CMD 0x40
#define MULTIBOARD_SLAVE_IO_CHANGED (MULTIBOARD_SLAVE_BASE_CMD + 1)
#define MULTIBOARD_SLAVE_ITP_POS (MULTIBOARD_CMD_GET_ITP_POS + 1)
#define MULTIBOARD_SLAVE_GET_PIN (MULTIBOARD_CMD_GET_PIN + 1)

#if defined(ENABLE_MULTIBOARD) && defined(IS_SLAVE_BOARD)
	void multiboard_slave_dotasks(void);
#elif defined(ENABLE_MULTIBOARD) && defined(IS_MASTER_BOARD)
void multiboard_master_send_command(uint8_t command, uint8_t *data, uint8_t len);
#endif

#ifdef __cplusplus
}
#endif

#endif