/*
    Name: tmcdriver.c
    Description: Minimalistic generic driver library for Trinamic stepper drivers.

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 21-03-2022

    µCNC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version. Please see <http://www.gnu.org/licenses/>

    µCNC is distributed WITHOUT ANY WARRANTY;
    Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the	GNU General Public License for more details.
*/

#include "tmcdriver.h"

uint8_t tmc_crc8(uint8_t *data, uint8_t len)
{
    int i, j;
    uint8_t crc = 0; // CRC located in last byte of message
    uint8_t currentByte;
    for (i = 0; i < len; i++)
    {                          // Execute for all bytes of a message
        currentByte = data[i]; // Retrieve a byte to be sent from Array
        for (j = 0; j < 8; j++)
        {
            if ((crc >> 7) ^ (currentByte & 0x01)) // update CRC based result of XOR operation
            {
                crc = (crc << 1) ^ 0x07;
            }
            else
            {
                crc = (crc << 1);
            }
            currentByte = currentByte >> 1;
        } // for CRC bit
    }     // for message byte

    return crc;
}

uint32_t tmc_read_register(tmc_driver_t *driver, uint8_t address)
{
    if (!(driver->rw))
    {
        return 0;
    }

    uint8_t data[8];
    uint8_t crc = 0;
    uint32_t result = 0;
    switch (driver->type)
    {
    case 2202:
    case 2208:
    case 2225:
        driver->slave = 0;
    case 2209:
    case 2226:
        /* code */
        data[0] = 0x05;
        data[1] = driver->slave;
        data[2] = address & 0x7F;
        data[3] = tmc_crc8(data, 3);
        driver->rw(data, 4, 8);
        crc = tmc_crc8(data, 7);
        if (data[0] != 0x05)
        {
            return TMC_READ_ERROR;
        }
        if (data[1] != 0xFF)
        {
            return TMC_READ_ERROR;
        }
        if (data[2] != address)
        {
            return TMC_READ_ERROR;
        }
        if (crc != data[7])
        {
            return TMC_READ_ERROR;
        }
        result = ((uint32_t)data[3] << 24) | ((uint32_t)data[4] << 16) | (data[5] << 8) | data[6];
        break;
    case 2130:
        data[0] = address & 0x7F;
        data[1] = 0;
        data[2] = 0;
        data[3] = 0;
        data[4] = 0;
        driver->rw(data, 5, 5);
    default:
        return 0;
    }

    return result;
}

uint32_t tmc_write_register(tmc_driver_t *driver, uint8_t address, uint32_t val)
{
    if (!(driver->rw))
    {
        return 0;
    }

    uint8_t data[8];
    uint32_t result = 0;
    switch (driver->type)
    {
    case 2202:
    case 2208:
    case 2225:
        driver->slave = 0;
    case 2209:
    case 2226:
        /* code */
        data[0] = 0x05;
        data[1] = driver->slave;
        data[2] = address | 0x80;
        data[3] = (val >> 24) & 0xFF;
        data[4] = (val >> 16) & 0xFF;
        data[5] = (val >> 8) & 0xFF;
        data[6] = (val)&0xFF;
        data[7] = tmc_crc8(data, 7);
        driver->rw(data, 8, 0);
        break;
    case 2130:
        data[0] = address | 0x80;
        data[4] = (uint8_t)(val & 0xFF);
        val >>= 8;
        data[3] = (uint8_t)(val & 0xFF);
        val >>= 8;
        data[2] = (uint8_t)(val & 0xFF);
        val >>= 8;
        data[1] = (uint8_t)(val & 0xFF);
        driver->rw(data, 5, 5);
    default:
        return 0;
    }

    return result;
}

void tmc_init(tmc_driver_t *driver)
{
    if (driver->init)
    {
        driver->init();
    }

    // set initial state (spread cycle on, internal vref external resistor, shaft fwd, disable pdn, microstep from mstep, normal operation)
    tmc_write_register(driver, GCONF, ((uint32_t)0xC5));
}

float tmc_get_current(tmc_driver_t *driver, float rsense)
{
    uint32_t iholdrun = tmc_read_register(driver, IHOLD_IRUN);
    if (iholdrun == TMC_READ_ERROR)
    {
        return -1.0f;
    }

    uint32_t chopconf = tmc_read_register(driver, CHOPCONF);
    if (chopconf == TMC_READ_ERROR)
    {
        return -1.0f;
    }

    uint8_t irun = ((iholdrun >> 8) & 0x1F);
    return (float)(irun + 1) / 32.0 * ((chopconf & (1UL << 17)) ? 0.180 : 0.325) / (rsense + 0.02) / 1.41421 * 1000;
}

void tmc_set_current(tmc_driver_t *driver, float current, float rsense, float ihold_mul)
{
    uint8_t currentsense = 32.0 * 1.41421 * current / 1000.0 * (rsense + 0.02) / 0.325 - 1;
    // If Current Scale is too low, turn on high sensitivity R_sense and calculate again
    uint32_t chopconf = tmc_read_register(driver, CHOPCONF);

    if (chopconf == TMC_READ_ERROR)
    {
        return;
    }

    if (currentsense < 16)
    {
        // enable vsense
        chopconf |= (((uint32_t)1) << 17);
        tmc_write_register(driver, CHOPCONF, chopconf);
        currentsense = 32.0 * 1.41421 * current / 1000.0 * (rsense + 0.02) / 0.180 - 1;
    }
    else if (chopconf & (((uint32_t)1) << 17)) // check if VSENSE is enabled
    {                                          // If CS >= 16, turn off high_sense_r if it's currently ON
        // disable vsense
        //  enable vsense
        chopconf &= ~(((uint32_t)1) << 17);
        tmc_write_register(driver, CHOPCONF, chopconf);
    }

    uint32_t iholdrun = 0;

    // rms current
    iholdrun = (currentsense & 0x1F);
    iholdrun <<= 8;
    // hold current
    currentsense = (uint8_t)(currentsense * ihold_mul);
    iholdrun |= (currentsense & 0x1F);
    tmc_write_register(driver, IHOLD_IRUN, iholdrun);
}

int32_t tmc_get_microstep(tmc_driver_t *driver)
{
    uint32_t chopconf = tmc_read_register(driver, CHOPCONF);

    if (chopconf == TMC_READ_ERROR)
    {
        return -1;
    }

    uint8_t ms = (uint8_t)((chopconf >> 24) & 0x0F);

    switch (ms)
    {
    case 0:
        return 256;
    case 1:
        return 128;
    case 2:
        return 64;
    case 3:
        return 32;
    case 4:
        return 16;
    case 5:
        return 8;
    case 6:
        return 4;
    case 7:
        return 2;
    case 8:
        return 0;
    }
    return -1;
}

void tmc_set_microstep(tmc_driver_t *driver, int16_t ms)
{
    uint32_t gconf = tmc_read_register(driver, GCONF);
    if (gconf == TMC_READ_ERROR)
    {
        return;
    }

    uint32_t chopconf = tmc_read_register(driver, CHOPCONF);
    if (chopconf == TMC_READ_ERROR)
    {
        return;
    }

    chopconf &= ~(((uint32_t)0xF) << 24);

    switch (ms)
    {
    case 256:
        ms = 0;
        break;
    case 128:
        ms = 1;
        break;
    case 64:
        ms = 2;
        break;
    case 32:
        ms = 3;
        break;
    case 16:
        ms = 4;
        break;
    case 8:
        ms = 5;
        break;
    case 4:
        ms = 6;
        break;
    case 2:
        ms = 7;
        break;
    case 0:
        ms = 8;
        break;
    default:
        if (ms < 0)
        {
            gconf &= ~((uint32_t)0xC0);
            tmc_write_register(driver, GCONF, gconf);
            return;
        }
        break;
    }

    gconf |= ((uint32_t)0xC0);
    gconf &= 0xFF;
    tmc_write_register(driver, GCONF, gconf);
    chopconf |= (((uint32_t)ms) << 24);
    tmc_write_register(driver, CHOPCONF, chopconf);
}

uint8_t tmc_get_stepinterpol(tmc_driver_t *driver)
{
    uint32_t chopconf = tmc_read_register(driver, CHOPCONF);
    return (chopconf & (((uint32_t)1) << 28)) ? 1 : 0;
}

void tmc_set_stepinterpol(tmc_driver_t *driver, uint8_t enable)
{
    uint32_t chopconf = tmc_read_register(driver, CHOPCONF);

    if (chopconf == TMC_READ_ERROR)
    {
        return;
    }

    if (enable)
    {
        chopconf |= (((uint32_t)1) << 28);
    }
    else
    {
        chopconf &= ~(((uint32_t)1) << 28);
    }

    tmc_write_register(driver, CHOPCONF, chopconf);
}

uint32_t tmc_get_stealthshop(tmc_driver_t *driver)
{
    return tmc_read_register(driver, TPWMTHRS);
}

void tmc_set_stealthchop(tmc_driver_t *driver, uint32_t value)
{
    uint32_t gconf = tmc_read_register(driver, GCONF);
    if (gconf == TMC_READ_ERROR)
    {
        return;
    }

    uint32_t chopconf = tmc_read_register(driver, CHOPCONF);
    if (chopconf == TMC_READ_ERROR)
    {
        return;
    }

    if (!value)
    {
        gconf |= ((uint32_t)0x04);
    }
    else
    {
        gconf &= ~((uint32_t)0x04);
    }

    gconf &= 0xFF;
    tmc_write_register(driver, GCONF, gconf);
    tmc_write_register(driver, TPWMTHRS, value);
}

uint32_t tmc_get_status(tmc_driver_t *driver)
{
    return tmc_read_register(driver, DRV_STATUS);
}
