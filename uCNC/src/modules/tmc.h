/*
	Name: tmc.h
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

#ifndef TMC_H
#define TMC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define TMC_READ_ERROR 0xFFFFFFFFUL
#define TMC_WRITE_ERROR 0xFFFFFFFFUL
#define GCONF 0x00
#define IFCNT 0x02
#define IHOLD_IRUN 0x10
#define TPWMTHRS 0X13
#define TCOOLTHRS 0x14
#define SGTHRS 0x40
#define SG_RESULT 0x41
#define CHOPCONF 0x6C
#define COOLCONF 0x6D
#define DRV_STATUS 0x6F
#define PWMCONF 0x70

	typedef void (*tmc_rw)(uint8_t *, uint8_t, uint8_t);
	typedef void (*tmc_startup)(void);

	typedef struct
	{
		uint8_t ifcnt;
		uint32_t ihold_irun;
		uint32_t tpwmthrs;
		uint32_t tcoolthrs;
		uint32_t sgthrs_coolconf;
	} tmc_driver_reg_t;

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
		// internal driver registers
		tmc_driver_reg_t reg;
	} tmc_driver_t;

	void tmc_init(tmc_driver_t *driver);
	float tmc_get_current(tmc_driver_t *driver, float rsense);
	void tmc_set_current(tmc_driver_t *driver, float current, float rsense, float ihold_mul, uint8_t ihold_delay);
	int32_t tmc_get_microstep(tmc_driver_t *driver);
	void tmc_set_microstep(tmc_driver_t *driver, int16_t ms);
	uint8_t tmc_get_stepinterpol(tmc_driver_t *driver);
	void tmc_set_stepinterpol(tmc_driver_t *driver, uint8_t enable);
	int32_t tmc_get_stealthchop(tmc_driver_t *driver);
	void tmc_set_stealthchop(tmc_driver_t *driver, int32_t value);
	int32_t tmc_get_stallguard(tmc_driver_t *driver);
	void tmc_set_stallguard(tmc_driver_t *driver, int32_t value);
	uint32_t tmc_get_status(tmc_driver_t *driver);
	uint32_t tmc_read_register(tmc_driver_t *driver, uint8_t address);
	uint32_t tmc_write_register(tmc_driver_t *driver, uint8_t address, uint32_t val);

#ifdef __cplusplus
}
#endif

#endif
