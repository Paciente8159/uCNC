/*
	Name: Arduino_uCNC_SPI.h
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

#include "Arduino_uCNC_SPI.h"
#include "../../cnc.h"
#include "../softspi.h"

void Arduino_uCNC_SPI::clearDC(bool dc_val = false)
{
	if (!this->_dc) // 9-bit SPI
	{
		((softspi_port_t *)(this->_spi))->mosi(dc_val);
		((softspi_port_t *)(this->_spi))->clk(true);
		((softspi_port_t *)(this->_spi))->clk(false);
	}
	else if (!dc_val)
	{
		io_set_pinvalue(this->_dc, dc_val);
	}
}

void Arduino_uCNC_SPI::setDC(void)
{
	if (this->_dc) // 9-bit SPI
	{
		io_set_pinvalue(this->_dc, 1);
	}
}

void Arduino_uCNC_SPI::convertTo9bit(const uint8_t *data, uint8_t *buffer, uint16_t len, bool bit9val)
{
	uint16_t mask = bit9val ? 0x100 : 0x000;
	*buffer = 0;
	for (uint32_t i = 0; i < len; i++)
	{
		uint16_t d = mask | *data++;
		uint8_t shift = (i & 0x07) + 1;

		*buffer++ |= (uint8_t)((d >> shift) & 0xFF);
		*buffer = (uint8_t)((d << (8 - shift)) & 0xFF);
		if (shift == 0x8)
		{
			buffer++;
			*buffer = 0;
		}
	}
}

Arduino_uCNC_SPI::Arduino_uCNC_SPI(void *spi = (void *)0, uint8_t dc = 0, uint8_t cs = 0, bool is_shared_interface = true)
{
	this->_dc = dc;
	this->_cs = cs;
	this->_spi = spi;
	this->_is_shared_interface = is_shared_interface;
}

bool Arduino_uCNC_SPI::begin(int32_t speed = 20000000UL, int8_t dataMode = 0)
{
	this->_speed = (uint32_t)speed;
	this->_dataMode = (uint8_t)dataMode;
	io_set_pinvalue(this->_dc, 1);
	io_set_pinvalue(this->_cs, 1);
	softspi_config_t conf = {0};
	conf.spi.mode = this->_dataMode;
	conf.spi.enable_dma = 1;
	softspi_config((softspi_port_t *)this->_spi, conf, this->_speed);
	return true;
}

void Arduino_uCNC_SPI::beginWrite()
{
	if (this->_is_shared_interface)
	{
		softspi_start((softspi_port_t *)this->_spi);
	}

	setDC();
	io_set_pinvalue(this->_cs, 0);
}
void Arduino_uCNC_SPI::endWrite()
{
	if (this->_is_shared_interface)
	{
		softspi_stop((softspi_port_t *)this->_spi);
	}
	setDC();
	io_set_pinvalue(this->_cs, 1);
}
void Arduino_uCNC_SPI::writeCommand(uint8_t c)
{
	clearDC();
	softspi_xmit((softspi_port_t *)this->_spi, c);
	setDC();
}

void Arduino_uCNC_SPI::writeCommand16(uint16_t c)
{
	clearDC();
	softspi_xmit((softspi_port_t *)this->_spi, (uint8_t)(c >> 8));
	clearDC();
	softspi_xmit((softspi_port_t *)this->_spi, (uint8_t)(c & 0xff));
	setDC();
}

void Arduino_uCNC_SPI::writeCommandBytes(uint8_t *data, uint32_t len)
{
	if (this->_dc)
	{
		clearDC();
		softspi_bulk_xmit((softspi_port_t *)this->_spi, data, NULL, len);
	}
	else
	{
		uint32_t len9bit = (uint32_t)ceilf(len * 1.125f);
		uint8_t newdata[len9bit];
		convertTo9bit(data, newdata, len, false);
		softspi_bulk_xmit((softspi_port_t *)this->_spi, newdata, NULL, len9bit);
	}
	setDC();
}

void Arduino_uCNC_SPI::write(uint8_t c)
{
	clearDC(true);
	softspi_xmit((softspi_port_t *)this->_spi, c);
	setDC();
}

void Arduino_uCNC_SPI::write16(uint16_t c)
{
	clearDC(true);
	softspi_xmit((softspi_port_t *)this->_spi, (uint8_t)(c >> 8));
	clearDC(true);
	softspi_xmit((softspi_port_t *)this->_spi, (uint8_t)(c & 0xff));
	setDC();
}

void Arduino_uCNC_SPI::writeRepeat(uint16_t p, uint32_t len)
{
	// converts the data to repeat to an array in memory for faster operations
	uint16_t chunck = (uint16_t)((len << 1) & 0x7FFF); // breaks this in operations with a max size of 32Kb
	uint8_t data[chunck];															 // buffer
	uint16_t *ptr = (uint16_t *)data;									 // fill the buffer
	uint16_t l = len;

	while (l--)
	{
		*ptr++ = p; // prefill repeated data
	}

	writeBytes((uint8_t *)data, (len << 1));
}

void Arduino_uCNC_SPI::writeBytes(uint8_t *data, uint32_t len)
{
	uint16_t chunck = (uint16_t)(len & 0x7FFF); // breaks this in operations with a max size of 32Kb
	uint32_t offset = 0;

	if (this->_dc)
	{
		clearDC(true);
		while (len)
		{
			chunck = (uint16_t)MIN(chunck, len);																					// size of chunck to send
			softspi_bulk_xmit((softspi_port_t *)this->_spi, &data[offset], NULL, chunck); // send chunck
			offset += chunck;																															// update the data offset
			len -= chunck;																																// determine how much data is yet to be sent
		}
	}
	else
	{
		// convert spi format from 8bit to 9bit
		uint16_t len9bit = (uint16_t)ceilf(chunck * 1.125f);
		uint8_t newdata[len9bit];
		while (len)
		{
			chunck = (uint16_t)MIN(chunck, len);
			convertTo9bit(&data[offset], newdata, chunck, true);										 // convert a chunck of data to 9bit-width
			len9bit = (uint16_t)ceilf(1.125f * chunck);															 // size of 9bit-chunck to send
			softspi_bulk_xmit((softspi_port_t *)this->_spi, newdata, NULL, len9bit); // send chunck
			offset += chunck;																												 // update the pointer index
			len -= chunck;																													 // determine how much data is yet to be sent
		}
	}
	setDC();
}

void Arduino_uCNC_SPI::writePixels(uint16_t *data, uint32_t len)
{
	writeBytes((uint8_t *)data, (len << 1));
}
