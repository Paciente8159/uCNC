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

#define MULTIBOARD_PROTOCOL_SOF 0xAA
#define MULTIBOARD_PROTOCOL_EOF 0x55
#define MULTIBOARD_PROTOCOL_ACK 0xFE70       // ACK+CRC
#define MULTIBOARD_PROTOCOL_NACK 0xFD6B      // NACK+CRC

// these codes might not be used
// since this will work on a send+response base ensure command order will not be necessary
#define MULTIBOARD_PROTOCOL_SYNC 0xF846      // SYNC+CRC
#define MULTIBOARD_PROTOCOL_SYNC_LOST 0xF94F // SYNC_LOST+CRC

// list of commands

// Master to slave
#define MULTIBOARD_CMD_SET_STATE
#define MULTIBOARD_CMD_CLEAR_STATE
#define MULTIBOARD_CMD_ITP_BLOCK
#define MULTIBOARD_CMD_ITP_SEGMENT
#define MULTIBOARD_CMD_ITP_NEWBLOCK
#define MULTIBOARD_CMD_ITP_RUN

    void multiboard_slave_process_command(uint8_t command);

#ifdef __cplusplus
}
#endif

#endif