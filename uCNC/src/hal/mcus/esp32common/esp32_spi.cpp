/*
	Name: esp32_arduino.cpp
	Description: Implements the µCNC SPI shim for all ESP32 variants.
	Uses Arduino


	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 27-07-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#if defined(ESP32)
#include <Arduino.h>
#include <SPI.h>
#include "esp_task_wdt.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

SPIClass *esp32spi = NULL;
SPIClass *esp32spi2 = NULL;

extern "C"
{
#include "../../../cnc.h"

#if defined(MCU_HAS_SPI) && defined(USE_ARDUINO_SPI_LIBRARY)
	static uint8_t spi_mode = 0;
	static uint32_t spi_freq = 1000000UL;

	void mcu_spi_init(void)
	{
		esp32spi = new SPIClass(SPI_INSTANCE);
		esp32spi->begin(SPI_CLK_BIT, SPI_SDI_BIT, SPI_SDO_BIT, -1);
	}

	void mcu_spi_config(spi_config_t config, uint32_t freq)
	{
		spi_freq = freq;
		spi_mode = config.mode;
		esp32spi->setFrequency(freq);
		esp32spi->setDataMode(config.mode);
	}

	uint8_t mcu_spi_xmit(uint8_t data)
	{
		return esp32spi->transfer(data);
	}

	void mcu_spi_start(spi_config_t config, uint32_t frequency)
	{
		esp32spi->beginTransaction(SPISettings(frequency, MSBFIRST, config.mode));
	}

	bool mcu_spi_bulk_transfer(const uint8_t *out, uint8_t *in, uint16_t len)
	{
		esp32spi->transferBytes(out, in, len);
		return false;
	}

	void mcu_spi_stop(void)
	{
		esp32spi->endTransaction();
	}

#endif

#if defined(MCU_HAS_SPI2) && defined(USE_ARDUINO_SPI_LIBRARY)
	static uint8_t spi2_mode = 0;
	static uint32_t spi2_freq = 1000000UL;

	void mcu_spi2_init(void)
	{
		esp32spi2 = new SPIClass(SPI2_INSTANCE);
		esp32spi2->begin(SPI2_CLK_BIT, SPI2_SDI_BIT, SPI2_SDO_BIT, -1);
	}

	void mcu_spi2_config(spi_config_t config, uint32_t freq)
	{
		spi2_freq = freq;
		spi2_mode = config.mode;
		esp32spi2->setFrequency(freq);
		esp32spi2->setDataMode(config.mode);
	}

	uint8_t mcu_spi2_xmit(uint8_t data)
	{
		return esp32spi2->transfer(data);
	}

	void mcu_spi2_start(spi_config_t config, uint32_t frequency)
	{
		esp32spi2->beginTransaction(SPISettings(frequency, MSBFIRST, config.mode));
	}

	bool mcu_spi2_bulk_transfer(const uint8_t *out, uint8_t *in, uint16_t len)
	{
		esp32spi2->transferBytes(out, in, len);
		return false;
	}

	void mcu_spi2_stop(void)
	{
		esp32spi2->endTransaction();
	}

#endif
}
#endif
