/*
	Name: multiboard_protocol.c
	Description: µCNC implementation of a multiboard send-response protocol over UART2 port
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

#include "../cnc.h"

#ifdef ENABLE_MULTIBOARD

slave_board_state_t g_multiboard_slave;

// small ring buffer
DECL_BUFFER(multiboard_data_t, multiboard_ring_buffer, 3);

// defaults to UART2 if undefined
#ifndef MULTIBOARD_RX_CB
#define MULTIBOARD_RX_CB mcu_uart2_rx_cb
#endif
#ifndef MULTIBOARD_TX
#define MULTIBOARD_TX mcu_uart2_putc
#endif
#ifndef MULTIBOARD_FLUSH
#define MULTIBOARD_FLUSH mcu_uart2_flush
#endif

void MULTIBOARD_RX_CB(unsigned char c)
{
	static int8_t framebytes = -3;

	int8_t current_bytes = framebytes;
	multiboard_data_t *ringbuffer = (multiboard_data_t *)BUFFER_NEXT_FREE(multiboard_ring_buffer);

	switch (c)
	{
	case MULTIBOARD_PROTOCOL_SOF:
		if (current_bytes < 0)
		{
			current_bytes++;
			framebytes = current_bytes;
			if (!current_bytes)
			{
				memset(ringbuffer, 0, sizeof(ringbuffer));
			}
			return;
		}
		break;
	case MULTIBOARD_PROTOCOL_EOF: // received a EOF (eval if is part of the message, it's EOF or error)
		// EOF found at the expected location (id + cmd + message length + 1 (crc))
		if (ringbuffer->multiboard_frame.length == current_bytes - 1)
		{
			BUFFER_STORE(multiboard_ring_buffer); // advances the buffer if possible
			framebytes = -3;					  // reset protocol internal state
			return;
		}
		if (ringbuffer->multiboard_frame.length < current_bytes - 1) // beyond expected message length
		{
			framebytes = -3; // reset protocol internal state
			return;
		}
		break;
	default: // received other character but not yet inside the knowned packet format

		if (current_bytes < 0)
		{
			current_bytes = -3; // reset protocol internal state
			return;
		}
	}

	ringbuffer->rawdata[current_bytes++] = c;
	framebytes = current_bytes;
}

void multiboard_tx_data(multiboard_data_t *msg)
{
	uint8_t crc = 0;
	uint8_t len = msg->multiboard_frame.length;
	MULTIBOARD_TX(MULTIBOARD_PROTOCOL_SOF); // SOF
	MULTIBOARD_TX(MULTIBOARD_PROTOCOL_SOF); // SOF
	MULTIBOARD_TX(MULTIBOARD_PROTOCOL_SOF); // SOF
	for (uint8_t i = 0; i < len; i++)
	{
		uint8_t c = msg->rawdata[i];
		MULTIBOARD_TX(c);
		crc = crc7(c, crc);
	}
	msg->rawdata[len] = crc;
	MULTIBOARD_TX(crc);
	MULTIBOARD_TX(MULTIBOARD_PROTOCOL_EOF); // EOF
	MULTIBOARD_FLUSH();
}

static bool multiboard_check_crc(multiboard_data_t *packet)
{
	uint8_t len = packet->multiboard_frame.length, crc = 0;

	if (packet->multiboard_frame.length > MULTIBOARD_BUFFER_SIZE)
	{
		return false;
	}

	for (uint8_t i = 0; i < len; i++)
	{
		crc = crc7(packet->rawdata[i], crc);
	}

	return (crc == packet->rawdata[len]);
}

#ifdef IS_SLAVE_BOARD
static void multiboard_slave_send_response(multiboard_data_t *msg)
{
	multiboard_tx_data(msg);

	// discards the remaining buffer
	// this protocol works on a message/response base
	// is a response is to be formulated the command was accepted and all possible retransmitions can be discarded
	BUFFER_CLEAR(multiboard_ring_buffer);
}

// all commands are executed in the main loop
void multiboard_slave_dotasks(void)
{
	static uint8_t last_id = 0;
	// there is an available command when the crc is set
	// crc will never be 0
	while (!BUFFER_EMPTY(multiboard_ring_buffer))
	{
		multiboard_data_t msg;
		BUFFER_PEEK(multiboard_ring_buffer, &msg);
		// check message CRC
		if (multiboard_check_crc(&msg))
		{
			if (last_id != msg.multiboard_frame.msgid) // msg not processed yet
			{
				last_id = msg.multiboard_frame.msgid;
				uint8_t command = msg.multiboard_frame.rawcmd;
				// run actions
				switch (command)
				{
				case MULTIBOARD_CMD_SET_STATE:
					cnc_set_exec_state(msg.multiboard_frame.data[0]);
					break;
				case MULTIBOARD_CMD_CLEAR_STATE:
					cnc_clear_exec_state(msg.multiboard_frame.data[0]);
					break;
				case MULTIBOARD_CMD_ITP_BLOCK:
					itp_add_block(msg.multiboard_frame.data);
					break;
				case MULTIBOARD_CMD_ITP_SEGMENT:
					itp_add_segment(msg.multiboard_frame.data);
					break;
				case MULTIBOARD_CMD_ITP_BLOCK_WRITE:
					itp_blk_buffer_write();
					break;
				case MULTIBOARD_CMD_ITP_START:
					itp_start((msg.multiboard_frame.data[0] != 0));
					break;
				case MULTIBOARD_CMD_ITP_POS_RESET:
					itp_reset_rt_position((float *)msg.multiboard_frame.data);
					break;
				case MULTIBOARD_CMD_SET_OUTPUT:
					io_set_pinvalue(msg.multiboard_frame.data[0], msg.multiboard_frame.data[1]);
					break;
					// request commands
				case MULTIBOARD_CMD_GET_ITP_POS:
					itp_get_rt_position((int32_t *)msg.multiboard_frame.data);
					msg.multiboard_frame.length = sizeof(int32_t) * STEPPER_COUNT + 3;
					msg.multiboard_frame.cmd_regs.from_slave = 1;
					break;
				case MULTIBOARD_CMD_GET_PIN:
					*((int16_t *)&msg.multiboard_frame.data[0]) = io_get_pinvalue(msg.multiboard_frame.data[0]);
					msg.multiboard_frame.length = 5;
					msg.multiboard_frame.cmd_regs.from_slave = 1;
					return;
				default:
					// command unknowned. can be use for eventual command extensions
					// send ACK anyway
					break;
				}
			}
			msg.multiboard_frame.ack = 1;
		}

		multiboard_slave_send_response(&msg);
	}

	static slave_board_io_t prev_io = {0};
	slave_board_io_t io;
	io.slave_io_bits.state = cnc_get_exec_state(EXEC_ALLACTIVE);
	io.slave_io_bits.limits = io_get_limits();
	io.slave_io_bits.controls = io_get_controls();
	io.slave_io_bits.probe = io_get_probe();

	if (prev_io.slave_io_reg ^ io.slave_io_reg)
	{
		// slave io changed
		multiboard_data_t tmp;
		tmp.multiboard_frame.rawcmd = MULTIBOARD_SLAVE_IO_CHANGED;
		tmp.multiboard_frame.length = 4 + 3/*header*/;
		memcpy(tmp.multiboard_frame.data, &io, sizeof(slave_board_io_t));
		multiboard_slave_send_response(&tmp);
		prev_io.slave_io_reg = io.slave_io_reg;
	}
}

#elif defined(ENABLE_MULTIBOARD) && defined(IS_MASTER_BOARD)
static uint8_t multiboard_master_process_slave_message(uint8_t command, multiboard_data_t *msg)
{
	// checks the msg crc
	if (!multiboard_check_crc(msg))
	{
		return MULTIBOARD_PROTOCOL_ERROR;
	}

	switch (msg->multiboard_frame.rawcmd)
	{
		// handle master requests
	case MULTIBOARD_SLAVE_ITP_POS:
		memcpy(g_multiboard_slave.itp_rt_pos, msg->multiboard_frame.data, sizeof(g_multiboard_slave.itp_rt_pos));
		break;
	case MULTIBOARD_SLAVE_GET_PIN:
		break;
	// slave spontaneous messages
	case MULTIBOARD_SLAVE_IO_CHANGED:
		memcpy(&g_multiboard_slave.io, msg->multiboard_frame.data, sizeof(slave_board_io_t));
	default:
		// unrecognized message
		return MULTIBOARD_PROTOCOL_ERROR;
	}

	return (command == msg->multiboard_frame.cmd_regs.cmd) ? MULTIBOARD_PROTOCOL_OK : MULTIBOARD_PROTOCOL_UNEXPECTED_MESSAGE;
}

static uint8_t multiboard_master_check_ack(uint8_t command, uint32_t timeout)
{
	timeout += mcu_millis();
	while (true)
	{
		mcu_dotasks();
		// received something
		while (!BUFFER_EMPTY(multiboard_ring_buffer))
		{
			mcu_dotasks();
			// gets
			multiboard_data_t msg;
			BUFFER_PEEK(multiboard_ring_buffer, &msg);
			// ACK for matched command (don't bother check the CRC)
			if (msg.multiboard_frame.rawcmd == command && msg.multiboard_frame.ack)
			{
				return MULTIBOARD_PROTOCOL_OK;
			}
			else
			{
				// other messages like master requests. If not the response does not match the command continue to process messages
				if (multiboard_master_process_slave_message(command, &msg) == MULTIBOARD_PROTOCOL_OK)
				{
					return MULTIBOARD_PROTOCOL_OK;
				}
			}

			BUFFER_REMOVE(multiboard_ring_buffer);
		}

		// cnc_io_dotasks();
		if (timeout < mcu_millis())
		{
			return MULTIBOARD_PROTOCOL_TIMEOUT;
		}
	}

	// never reaches here
	return MULTIBOARD_PROTOCOL_ERROR;
}

void multiboard_master_send_command(uint8_t command, uint8_t *data, uint8_t len)
{
	uint8_t tries = MULTIBOARD_PROTOCOL_RETRIES;
	static uint8_t msg_id = 0;

	// prevents reentrancy
	if (cnc_get_alarm() == EXEC_ALARM_MULTIBOARD_CRITICAL_ERROR)
	{
		return;
	}

	// increments the id
	// the id prevents the slave to process a retransmitted message that was already processed
	multiboard_data_t msg = {0};
	msg.multiboard_frame.msgid = msg_id + 1;
	msg_id = msg.multiboard_frame.msgid;
	msg.multiboard_frame.rawcmd = command;
	msg.multiboard_frame.length = len + 3/*header*/;
	memcpy(msg.multiboard_frame.data, data, len);

	do
	{
		multiboard_tx_data(&msg);
		if (!--tries)
		{
			cnc_alarm(EXEC_ALARM_MULTIBOARD_CRITICAL_ERROR);
			return;
		}
		mcu_dotasks();
	} while (multiboard_master_check_ack(command, MULTIBOARD_PROTOCOL_TIMEOUT_MS) != MULTIBOARD_PROTOCOL_OK);
}
#endif
#endif
