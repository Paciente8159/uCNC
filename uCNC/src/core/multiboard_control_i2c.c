/*
    Name: multiboard_protocol_i2c.c
    Description: This defines the µCNC multiboard protocol using I2C.

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 30-05-2023

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
 * All details of the communication detail should be implemented here, including packet format and error checking
 *
 * **/

#include "../cnc.h"
#include <math.h>

#ifdef ENABLE_MULTIBOARD

#ifdef IS_MASTER_BOARD
#ifndef I2C_BUFFER_SIZE
#define I2C_BUFFER_SIZE 48
#endif
#ifndef I2C_MAX_RETRIES
#define I2C_MAX_RETRIES 3
#endif

static uint8_t master_confirm_crc(uint8_t address, uint8_t crc)
{
    uint8_t buffer[2];
    buffer[0] = MULTIBOARD_CMD_CONFIRM_CRC;
    buffer[1] = crc;
    if (mcu_i2c_send(address, buffer, 2, false) != I2C_OK)
    {
        return MULTIBOARD_CONTROL_CMD_ERROR;
    }
    if (mcu_i2c_receive(address, buffer, 1, 2) != I2C_OK)
    {
        return MULTIBOARD_CONTROL_RESPONSE_ERROR;
    }

    if (buffer[0] != crc)
    {
        return MULTIBOARD_CONTROL_RESPONSE_ERROR;
    }

    return MULTIBOARD_CONTROL_OK;
}

uint8_t master_send_command(uint8_t address, uint8_t command, uint8_t *data, uint8_t datalen)
{
    uint8_t buffer[datalen + 1];
    uint8_t crc = crc7(command, 0);
    buffer[0] = command;
    if (data && datalen)
    {
        for (uint8_t i = 0, j = 1; i < datalen; i++, j++)
        {
            buffer[j] = data[i];
            crc = crc7(data[i], crc);
        }
        datalen += 1;
    }

    // sends the command
    // retries on fail
    uint8_t tries = I2C_MAX_RETRIES;
    for (;;)
    {
        if (mcu_i2c_send(address, buffer, datalen, true) == I2C_OK)
        {
            break;
        }
        if (!tries--)
        {
            return MULTIBOARD_CONTROL_CMD_ERROR;
        }
    }

    // on broadcast command asks each slave for an ACK by asking the slave to confirm the CRC on their side
    if (!address)
    {
        for (uint8_t slaveid = SLAVE_BOARDS_ADDRESS_OFFSET; slaveid < (SLAVE_BOARDS_ADDRESS_OFFSET + SLAVE_BOARDS_COUNT); slaveid++)
        {
            uint8_t error = master_confirm_crc(slaveid, crc7(slaveid, crc));
            if (error != MULTIBOARD_CONTROL_OK)
            {
                return error;
            }
        }
    }
    else
    {
        return master_confirm_crc(address, crc);
    }

    return MULTIBOARD_CONTROL_OK;
}

uint8_t master_get_response(uint8_t address, uint8_t command, uint8_t *data, uint8_t datalen, uint32_t timeout)
{
    uint8_t tries = I2C_MAX_RETRIES;
    for (;;)
    {
        if (mcu_i2c_send(address, &command, 1, false) == I2C_OK)
        {
            break;
        }
        if (!tries--)
        {
            return MULTIBOARD_CONTROL_CMD_ERROR;
        }
    }

    if (mcu_i2c_receive(address, data, datalen, timeout) != I2C_OK)
    {
        return MULTIBOARD_CONTROL_RESPONSE_ERROR;
    }

    return MULTIBOARD_CONTROL_OK;
}
#else

// overrides the I2C slave callback
typedef struct i2c_cmd_data_
{
    uint8_t cmd;
    uint8_t first_byte;
    uint8_t datalen;
} i2c_cmd_data_t;

static i2c_cmd_data_t i2c_cmd_data;
MCU_IO_CALLBACK void mcu_i2c_slave_cb(uint8_t *data, uint8_t *datalen)
{
    static uint8_t last_crc = 0;

    // CRC checking command
    if (data[0] == MULTIBOARD_CMD_CONFIRM_CRC)
    {
        if (data[1] == last_crc)
        {
            // crc matches
            // restore data initial 2 bytes
            data[0] = i2c_cmd_data.cmd;
            data[1] = i2c_cmd_data.first_byte;
            *datalen = i2c_cmd_data.datalen;
            // now can execute the command
            slave_rcv_cb(data, datalen);
            // reply with ACK
            data[0] = MULTIBOARD_CMD_SLAVE_ACK;
            *datalen = 1;
            return;
        }
    }
    // master to slave command
    else if (data[0] < MULTIBOARD_REQUEST_CMDS_BASE)
    {
        uint8_t crc = 0;
        for (uint8_t i = 0; i < *datalen; i++)
        {
            crc = crc7(data[i], crc);
        }

        // in the end also add own ID to CRC
        // master must get ACK from addressed SLAVE
        last_crc = crc7(SLAVE_BOARD_ID, crc);
        i2c_cmd_data.cmd = data[0];
        i2c_cmd_data.first_byte = data[1];
        i2c_cmd_data.datalen = *datalen;
    }
    // slave to master request command
    else
    {
        // request command
        slave_rqst_cb(data, datalen);
    }
}

#endif

#endif