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
        if (crc != data[7])
        {
            return 0;
        }
        result = data[3];
        result <<= 8;
        result |= data[4];
        result <<= 8;
        result |= data[5];
        result <<= 8;
        result |= data[6];
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
        data[6] = (uint8_t)(val & 0xFF);
        val >>= 8;
        data[5] = (uint8_t)(val & 0xFF);
        val >>= 8;
        data[4] = (uint8_t)(val & 0xFF);
        val >>= 8;
        data[3] = (uint8_t)(val & 0xFF);
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

    // disable pdn
    uint32_t gconf = tmc_read_register(driver, GCONF);
    gconf |= (1UL << 6);
    tmc_write_register(driver, GCONF, gconf);
}

float tmc_get_current(tmc_driver_t *driver, float rsense)
{
    uint32_t iholdrun = tmc_read_register(driver, IHOLD_IRUN);
    uint32_t chopconf = tmc_read_register(driver, CHOPCONF);
    uint8_t irun = ((iholdrun >> 8) & 0x1F);

    return (float)(irun + 1) / 32.0 * ((chopconf & (1UL << 17)) ? 0.180 : 0.325) / (rsense + 0.02) / 1.41421 * 1000;
}

void tmc_set_current(tmc_driver_t *driver, float current, float rsense, float ihold_mul)
{
    uint8_t currentsense = 32.0 * 1.41421 * current / 1000.0 * (rsense + 0.02) / 0.325 - 1;
    // If Current Scale is too low, turn on high sensitivity R_sense and calculate again
    uint32_t chopconf = tmc_read_register(driver, CHOPCONF);

    if (currentsense < 16)
    {
        // enable vsense
        chopconf |= (1UL << 17);
        tmc_write_register(driver, CHOPCONF, chopconf);
        currentsense = 32.0 * 1.41421 * current / 1000.0 * (rsense + 0.02) / 0.180 - 1;
    }
    else if (chopconf & (1UL << 17)) // check if VSENSE is enabled
    {                                // If CS >= 16, turn off high_sense_r if it's currently ON
        // disable vsense
        //  enable vsense
        chopconf &= ~(1UL << 17);
        tmc_write_register(driver, CHOPCONF, chopconf);
    }

    uint32_t iholdrun = tmc_read_register(driver, IHOLD_IRUN);

    // rms current
    iholdrun &= ~(0x1F1FUL);
    iholdrun |= (currentsense & 0x1FUL);
    // hold current
    currentsense = (uint8_t)(currentsense * ihold_mul);
    iholdrun |= ((currentsense << 8) & 0x1F00UL);
    tmc_write_register(driver, IHOLD_IRUN, iholdrun);
}

int32_t tmc_get_microstep(tmc_driver_t *driver)
{
    uint32_t chopconf = tmc_read_register(driver, CHOPCONF);
    uint8_t ms = (uint8_t)((chopconf >> 24) & 0x0F);

    switch (ms)
    {
    case 0:
        return 256L;
    case 1:
        return 128L;
    case 2:
        return 64L;
    case 3:
        return 32L;
    case 4:
        return 16L;
    case 5:
        return 8L;
    case 6:
        return 4L;
    case 7:
        return 2L;
    case 8:
        return 0L;
    }
    return -1L;
}

void tmc_set_microstep(tmc_driver_t *driver, uint16_t ms)
{
    uint32_t gconf = tmc_read_register(driver, GCONF);
    uint32_t chopconf = tmc_read_register(driver, CHOPCONF);

    chopconf &= ~(0xFUL << 24);

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
        break;
    }

    chopconf |= (((uint32_t)ms) << 24);
    tmc_write_register(driver, CHOPCONF, chopconf);
    gconf |= (1UL << 7);
    tmc_write_register(driver, GCONF, gconf);
}

uint8_t tmc_get_stepinterpol(tmc_driver_t *driver)
{
    uint32_t chopconf = tmc_read_register(driver, CHOPCONF);
    return (chopconf & (1UL << 28)) ? 1 : 0;
}

void tmc_set_stepinterpol(tmc_driver_t *driver, uint8_t enable)
{
    uint32_t chopconf = tmc_read_register(driver, CHOPCONF);

    if (enable)
    {
        chopconf |= (1UL << 28);
    }
    else
    {
        chopconf &= ~(1UL << 28);
    }

    tmc_write_register(driver, CHOPCONF, chopconf);
}

uint32_t tmc_get_stealthshop(tmc_driver_t *driver)
{
    return tmc_read_register(driver, TPWMTHRS);
}

void tmc_set_stealthshop(tmc_driver_t *driver, uint32_t value)
{
    uint32_t gconf = tmc_read_register(driver, GCONF);
    uint32_t chopconf = tmc_read_register(driver, CHOPCONF);

    if (!value)
    {
        chopconf |= (0x1UL << 2);
        gconf |= (1UL << 2);
    }
    else
    {
        chopconf &= ~(0x1UL << 2);
        gconf &= ~(1UL << 2);
    }

    tmc_write_register(driver, TPWMTHRS, value);
    tmc_write_register(driver, CHOPCONF, chopconf);
    tmc_write_register(driver, GCONF, gconf);
}

uint32_t tmc_get_status(tmc_driver_t *driver)
{
    return tmc_read_register(driver, DRV_STATUS);
}
