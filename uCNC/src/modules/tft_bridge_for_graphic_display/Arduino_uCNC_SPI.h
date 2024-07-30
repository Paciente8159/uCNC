/*
	Name: Arduino_uCNC_SPI.cpp
	Description: Defines a DataBus for Arduino GFX that is supported on µCNC internal SPI port.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 16-06-2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/
#ifndef ARDUINO_UCNC_SPI_H
#define ARDUINO_UCNC_SPI_H

#include <stdint.h>
#include "Arduino_DataBus.h"

class Arduino_uCNC_SPI : public Arduino_DataBus
{
public:
	Arduino_uCNC_SPI(void *spi, uint8_t dc, uint8_t cs, bool is_shared_interface);
	bool begin(int32_t speed, int8_t dataMode) override;
	void beginWrite() override;
	void endWrite() override;
	void writeCommand(uint8_t c) override;
	void writeCommand16(uint16_t c) override;
	void writeCommandBytes(uint8_t *data, uint32_t len) override;
	void write(uint8_t c) override;
	void write16(uint16_t c) override;
	void writeRepeat(uint16_t p, uint32_t len) override;
	void writeBytes(uint8_t *data, uint32_t len) override;
	void writePixels(uint16_t *data, uint32_t len) override;

private:
	void clearDC(bool dc_val);
	void setDC(void);
	void convertTo9bit(const uint8_t* data, uint8_t* buffer, uint16_t len, bool bit9val);
	void *_spi;
	uint8_t _cs;
	bool _is_shared_interface;
	uint8_t _dc;
	uint32_t _speed;
	uint8_t _dataMode;
};
#endif
