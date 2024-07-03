/*
	Name: softspi.h
	Description: A software based SPI library for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 23-03-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef SOFTSPI_H
#define SOFTSPI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../cnc.h"
#include <stdint.h>
#include <stdbool.h>

	/**
	 * The new softspi port structure allow to create software SPI ports with different configurations.
	 * It also allows to take advantage of the Arduino library to allow creation of new HW SPI ports.
	 *
	 * For example to create a new SPI port using Arduino for ESP32 you can do something like this
	 * #ifdef ARDUINO
	 * #include <SPI.h>
	 * #include <softspi.h>
	 * SPIClass* myspiport;
	 *
	 * extern "C" void myspiport_config(uint8_t mode, uint32_t frequency){
	 *  if(!myspiport){myspiport->end();	myspiport = NULL;}
	 *  myspiport = new SPIClass(VSPI);
	 * 	myspiport->begin(sckpin, misopin, mosipin, sspin);
	 * }
	 *
	 * extern "C" void myspiport_start(uint8_t mode, uint32_t frequency){
	 * 	myspiport->beginTransaction(SPISettings(uint32_t frequency, SPI_MSBFIRST, uint8_t mode));
	 * }
	 *
	 * extern "C" uint8_t myspiport_xmit(uint8_t c){
	 * 	return myspiport->transfer(c);
	 * }
	 *
	 * extern "C" void myspiport_stop(void){
	 * 	myspiport.myspiport->endTransaction();
	 * }
	 *
	 * extern "C" softspi_port_t __attribute__((used)) ARDUINO_SPI = {.spimode = 0, .spifreq = 20000000UL, .clk = NULL, .mosi = NULL, .miso = NULL, .config = myspiport_config, .start = myspiport_start, .xmit = myspiport_xmit, .stop = myspiport_stop};
	 * #endif
	 *
	 */

	typedef struct softspi_port_
	{
		uint8_t spimode;
		uint32_t spifreq;
		void (*clk)(bool);
		void (*mosi)(bool);
		bool (*miso)(void);
		void (*config)(uint8_t, uint32_t);
		void (*start)(uint8_t, uint32_t);
		uint8_t (*xmit)(uint8_t);
		void (*stop)(void);
	} softspi_port_t;

#define SPI_DELAY(FREQ) (CLAMP(1, (2500000UL / FREQ), 0xFFFF) - 1)

#define SOFTSPI(NAME, FREQ, MODE, MOSIPIN, MISOPIN, CLKPIN) \
	void NAME##_config(uint8_t mode, uint32_t frequency)      \
	{                                                         \
		io_config_output(CLKPIN);                               \
		io_config_output(MOSIPIN);                              \
		io_config_input(MISOPIN);                               \
	}                                                         \
	void NAME##_clk(bool state)                               \
	{                                                         \
		if (state)                                              \
		{                                                       \
			io_set_output(CLKPIN);                                \
		}                                                       \
		else                                                    \
		{                                                       \
			io_clear_output(CLKPIN);                              \
		}                                                       \
	}                                                         \
	void NAME##_mosi(bool state)                              \
	{                                                         \
		if (state)                                              \
		{                                                       \
			io_set_output(MOSIPIN);                               \
		}                                                       \
		else                                                    \
		{                                                       \
			io_clear_output(MOSIPIN);                             \
		}                                                       \
	}                                                         \
	bool NAME##_miso(void) { return io_get_input(MISOPIN); }  \
	__attribute__((used)) softspi_port_t NAME = {.spimode = MODE, .spifreq = FREQ, .clk = &NAME##_clk, .mosi = &NAME##_mosi, .miso = &NAME##_miso, .config = &NAME##_config, .start = NULL, .xmit = NULL, .stop = NULL};

	void softspi_config(softspi_port_t *port, uint8_t mode, uint32_t frequency);
	void softspi_start(softspi_port_t *port);
	uint8_t softspi_xmit(softspi_port_t *port, uint8_t c);
	uint16_t softspi_xmit16(softspi_port_t *port, uint16_t c);
	void softspi_stop(softspi_port_t *port);

#ifdef MCU_HAS_SPI
	extern softspi_port_t MCU_SPI_PORT;
#define MCU_SPI (&MCU_SPI_PORT)
#else
#define MCU_SPI NULL
#endif

#ifdef __cplusplus
}
#endif

#endif