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
#define MULTIBOARD_PROTOCOL_TIMEOUT_MS 10
#endif
#ifndef MULTIBOARD_PROTOCOL_RETRIES
#define MULTIBOARD_PROTOCOL_RETRIES 5
#endif
#ifndef MULTIBOARD_BUFFER_SIZE
#define MULTIBOARD_BUFFER_SIZE 48
#endif

    typedef union multiboard_data_
    {
        uint8_t rawdata[MULTIBOARD_BUFFER_SIZE + 3];
        struct
        {
            uint8_t command;
            uint8_t length;
            uint8_t content[MULTIBOARD_BUFFER_SIZE];
            uint8_t crc;
        } multiboard_frame;
    } multiboard_data_t;

    extern multiboard_data_t g_multiboard_data;

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
    extern slave_board_io_t g_slave_io;

#define MULTIBOARD_PROTOCOL_OK 0
#define MULTIBOARD_PROTOCOL_TIMEOUT 1
#define MULTIBOARD_PROTOCOL_ERROR 2

#define MULTIBOARD_PROTOCOL_SOF 0xAA
#define MULTIBOARD_PROTOCOL_EOF 0x55
#define MULTIBOARD_PROTOCOL_ACK 0xFE70  // ACK+CRC
#define MULTIBOARD_PROTOCOL_NACK 0xFD6B // NACK+CRC

// these codes might not be used
// since this will work on a send+response base ensure command order will not be necessary
#define MULTIBOARD_PROTOCOL_SYNC 0xF846      // SYNC+CRC
#define MULTIBOARD_PROTOCOL_SYNC_LOST 0xF94F // SYNC_LOST+CRC

// list of commands

// Master to slave
#define MULTIBOARD_CMD_SET_STATE 0x81
#define MULTIBOARD_CMD_CLEAR_STATE 0x82
#define MULTIBOARD_CMD_ITP_BLOCK 0x83
#define MULTIBOARD_CMD_ITP_SEGMENT 0x84
#define MULTIBOARD_CMD_ITP_NEWBLOCK 0x85
#define MULTIBOARD_CMD_ITP_RUN 0x86
#define MULTIBOARD_CMD_ITP_POS_RESET 0x87

    void multiboard_rcv_byte_cb(unsigned char c);
#ifndef IS_MASTER_BOARD
    void multiboard_slave_dotasks(void);
#else
void multiboard_master_send_command(uint8_t command, uint8_t *data, uint8_t len);
#endif

#ifdef __cplusplus
}
#endif

#endif