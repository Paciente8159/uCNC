/*
	Name: softspi.c
	Description: A software based UART library for µCNC.

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
#include "softspi.h"

FORCEINLINE static void softspi_delay(uint8_t loops_100ns)
{
	while (loops_100ns--)
	{
		mcu_delay_100ns();
	}
}

void softspi_config(softspi_port_t *port, uint8_t mode, uint32_t frequency)
{
	if (!port)
	{
#ifdef MCU_HAS_SPI
		mcu_spi_config(mode, frequency);
#endif
	}
	else
	{
		port->spimode = mode;
		port->spidelay = (uint8_t)SPI_DELAY(frequency);
	}
}

uint8_t softspi_xmit(softspi_port_t *port, uint8_t c)
{
	if (!port)
	{
#ifdef MCU_HAS_SPI
		return mcu_spi_xmit(c);
#else
		return 0;
#endif
	}

	bool clk = (bool)(port->spimode & 0x2);
	bool on_down = (bool)(port->spimode & 0x1);

	port->clk(clk);

	uint8_t counter = 8;
	do
	{
		if (on_down)
		{
			clk = !clk;
			port->clk(clk);
		}
		port->mosi((bool)(c & 0x80));
		c <<= 1;
		// sample
		softspi_delay(port->spidelay);
		clk = !clk;
		port->clk(clk);
		c |= port->miso();

		softspi_delay(port->spidelay);
		if (!on_down)
		{
			clk = !clk;
			port->clk(clk);
		}
	} while (--counter);

	return c;
}
