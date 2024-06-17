/*
	Name: graphic_display.h
	Description: Defines the graphic_display interface.

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

#include "Arduino_uCNC_SPI.h"
#include "../../cnc.h"

bool Arduino_uCNC_SPI::begin(int32_t speed = 1000000UL, int8_t dataMode = 0)
{
	this->_speed = (uint32_t)speed;
	this->_dataMode = (uint8_t)dataMode;
	softspi_config(this->_spi, this->_dataMode, this->_speed);
	io_set_pinvalue(this->_cs, 1);
	return true;
}

void Arduino_uCNC_SPI::beginWrite()
{
	if (this->_is_shared_interface)
	{
		// SPI_BEGIN_TRANSACTION();
	}

	io_set_pinvalue(this->_dc, 1);
	io_set_pinvalue(this->_cs, 0);
}
void Arduino_uCNC_SPI::endWrite()
{
	io_set_pinvalue(this->_cs, 1);
	if (this->_is_shared_interface)
	{
		// SPI_BEGIN_TRANSACTION();
	}
}
void Arduino_uCNC_SPI::writeCommand(uint8_t c)
{
	io_set_pinvalue(this->_dc, 0);
	softspi_xmit(this->_spi, c);
	io_set_pinvalue(this->_dc, 1);
}

void Arduino_uCNC_SPI::writeCommand16(uint16_t c)
{
	io_set_pinvalue(this->_dc, 0);
	softspi_xmit(this->_spi, (uint8_t)(c >> 8));
	softspi_xmit(this->_spi, (uint8_t)(c & 0xff));
	io_set_pinvalue(this->_dc, 1);
}

void Arduino_uCNC_SPI::writeCommandBytes(uint8_t *data, uint32_t len)
{
	io_set_pinvalue(this->_dc, 0);
	while (len--)
	{
		softspi_xmit(this->_spi, *data++);
	}
	io_set_pinvalue(this->_dc, 1);
}

void Arduino_uCNC_SPI::write(uint8_t c)
{
	softspi_xmit(this->_spi, c);
}

void Arduino_uCNC_SPI::write16(uint16_t c)
{
	softspi_xmit(this->_spi, (uint8_t)(c >> 8));
	softspi_xmit(this->_spi, (uint8_t)(c & 0xff));
}

void Arduino_uCNC_SPI::writeRepeat(uint16_t p, uint32_t len)
{
	while (len--)
	{
		softspi_xmit(this->_spi, (uint8_t)(p >> 8));
		softspi_xmit(this->_spi, (uint8_t)(p & 0xff));
	}
}

void Arduino_uCNC_SPI::writeBytes(uint8_t *data, uint32_t len)
{
	while (len--)
	{
		softspi_xmit(this->_spi, *data++);
	}
}

void Arduino_uCNC_SPI::writePixels(uint16_t *data, uint32_t len)
{
	while (len--)
	{
		uint16_t p = *data++;
		softspi_xmit(this->_spi, (uint8_t)(p >> 8));
		softspi_xmit(this->_spi, (uint8_t)(p & 0xff));
	}
}
