/*
	Name: softi2c.h
	Description: A software based I2C library for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 15-05-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef SOFTI2C_H
#define SOFTI2C_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../cnc.h"
#include <stdint.h>
#include <stdbool.h>

	typedef struct softi2c_port_
	{
		void (*wait)(void);
		void (*scl)(bool);
		void (*sda)(bool);
		bool (*get_sda)(void);
	} softi2c_port_t;

#define SOFTI2C(NAME, FREQ, SCLPIN, SDAPIN)                      \
	void NAME##_scl(bool state)                                  \
	{                                                            \
		if (state)                                               \
		{                                                        \
			mcu_config_input(SCLPIN);                            \
		}                                                        \
		else                                                     \
		{                                                        \
			mcu_clear_output(SCLPIN);                            \
			mcu_config_output(SCLPIN);                           \
		}                                                        \
	}                                                            \
	void NAME##_sda(bool state)                                  \
	{                                                            \
		if (state)                                               \
		{                                                        \
			mcu_config_input(SDAPIN);                            \
		}                                                        \
		else                                                     \
		{                                                        \
			mcu_clear_output(SDAPIN);                            \
			mcu_config_output(SDAPIN);                           \
		}                                                        \
	}                                                            \
	bool NAME##_get_sda(void)                                    \
	{                                                            \
		mcu_config_input(SDAPIN);                                \
		return mcu_get_input(SDAPIN);                            \
	}                                                            \
	void NAME##_wait(void) { mcu_delay_us((1000000UL / FREQ)); } \
	softi2c_port_t NAME = {.wait = &NAME##_wait, .scl = &NAME##_scl, .sda = &NAME##_sda, .get_sda = &NAME##_get_sda};

	uint8_t softi2c_write(softi2c_port_t *port, uint8_t c, bool start, bool stop);
	uint8_t softi2c_read(softi2c_port_t *port, bool ack, bool stop);
	uint8_t softi2c_write_byte(softi2c_port_t *port, uint8_t address, uint8_t c);
	uint8_t softi2c_read_byte(softi2c_port_t *port, uint8_t address);
	uint8_t softi2c_write_reg(softi2c_port_t *port, uint8_t address, uint8_t reg, uint8_t c);
	uint8_t softi2c_read_reg(softi2c_port_t *port, uint8_t address, uint8_t reg);

#ifdef __cplusplus
}
#endif

#endif