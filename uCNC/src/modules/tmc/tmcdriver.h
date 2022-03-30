/*
    Name: tmcdriver.h
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

#ifndef TMCDRIVER_H
#define TMCDRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define TMC_READ_ERROR 0xFFFFFFFFUL
#define GCONF 0x00
#define IHOLD_IRUN 0x10
#define CHOPCONF 0x6C
#define TPWMTHRS 0X13
#define DRV_STATUS 0x6F

    typedef void (*tmc_rw)(uint8_t *, uint8_t, uint8_t);
    typedef void (*tmc_startup)(void);

    typedef struct
    {
        // identifies the type of driver (2208, 2209, 2225, etc...)
        uint16_t type;
        // if the driver has a slave ID this should be set here
        uint8_t slave;
        // Callback for the UART/SPI interface initialization if needed
        tmc_startup init;
        // Callback for RW
        tmc_rw rw;
    } tmc_driver_t;

    void tmc_init(tmc_driver_t *driver);
    float tmc_get_current(tmc_driver_t *driver, float rsense);
    void tmc_set_current(tmc_driver_t *driver, float current, float rsense, float ihold_mul);
    int32_t tmc_get_microstep(tmc_driver_t *driver);
    void tmc_set_microstep(tmc_driver_t *driver, int16_t ms);
    uint8_t tmc_get_stepinterpol(tmc_driver_t *driver);
    void tmc_set_stepinterpol(tmc_driver_t *driver, uint8_t enable);
    uint32_t tmc_get_stealthshop(tmc_driver_t *driver);
    void tmc_set_stealthchop(tmc_driver_t *driver, uint32_t value);
    uint32_t tmc_get_status(tmc_driver_t *driver);
    uint32_t tmc_read_register(tmc_driver_t *driver, uint8_t address);
    uint32_t tmc_write_register(tmc_driver_t *driver, uint8_t address, uint32_t val);

#ifdef __cplusplus
}
#endif

#endif
