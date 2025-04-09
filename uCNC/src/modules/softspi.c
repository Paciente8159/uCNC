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

void softspi_config(softspi_port_t *port, spi_config_t config, uint32_t frequency)
{
	port->spiconfig = config;
	port->spifreq = frequency;

	if (!port)
	{
#ifdef MCU_HAS_SPI
		mcu_spi_config(config, frequency);
#endif
		return;
	}

	if (port->config)
	{
		// if port with custom method execute it
		port->config(config, frequency);
		return;
	}

#ifdef MCU_HAS_SPI
	if (port->spiport == MCU_SPI)
	{
		mcu_spi_config(config, frequency);
		return;
	}
#endif

#ifdef MCU_HAS_SPI2
	if (port->spiport == MCU_SPI2)
	{
		mcu_spi2_config(config, frequency);
	}
#endif
}

uint8_t softspi_xmit(softspi_port_t *port, uint8_t c)
{
	// if no port is defined defaults to SPI hardware if available
	if (!port)
	{
#ifdef MCU_HAS_SPI
		return mcu_spi_xmit(c);
#else
		return 0;
#endif
	}

	// if port with custom method execute it
	if (port->spiport)
	{
		return port->spiport->xmit(c);
	}

	bool clk = (bool)(port->spiconfig.mode & 0x2);
	bool on_down = (bool)(port->spiconfig.mode & 0x1);
	uint16_t delay = (uint16_t)SPI_DELAY(port->spifreq);

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
		mcu_delay_us(delay);
		clk = !clk;
		port->clk(clk);
		c |= port->miso();

		mcu_delay_us(delay);
		if (!on_down)
		{
			clk = !clk;
			port->clk(clk);
		}
	} while (--counter);

	return c;
}

uint16_t softspi_xmit16(softspi_port_t *port, uint16_t c)
{
	// if no port is defined defaults to SPI hardware if available
	if (!port)
	{
#ifdef MCU_HAS_SPI
		uint16_t res = mcu_spi_xmit((uint8_t)(c >> 8));
		res <<= 8;
		return (res | mcu_spi_xmit((uint8_t)(0xFF & c)));
#else
		return 0;
#endif
	}

	// if port with custom method execute it
	if (port->spiport)
	{
		uint16_t res = port->spiport->xmit((uint8_t)(c >> 8));
		res <<= 8;
		return (res | port->spiport->xmit((uint8_t)(0xFF & c)));
	}

	bool clk = (bool)(port->spiconfig.mode & 0x2);
	bool on_down = (bool)(port->spiconfig.mode & 0x1);
	uint16_t delay = (uint16_t)SPI_DELAY(port->spifreq);

	port->clk(clk);

	uint8_t counter = 16;
	do
	{
		if (on_down)
		{
			clk = !clk;
			port->clk(clk);
		}
		port->mosi((bool)(c & 0x8000));
		c <<= 1;
		// sample
		mcu_delay_us(delay);
		clk = !clk;
		port->clk(clk);
		c |= port->miso();

		mcu_delay_us(delay);
		if (!on_down)
		{
			clk = !clk;
			port->clk(clk);
		}
	} while (--counter);

	return c;
}

void softspi_bulk_xmit(softspi_port_t *port, const uint8_t *out, uint8_t *in, uint16_t len)
{

#ifndef SPI_BULK_LEGACY_MODE_ENABLED
	// if no port is defined defaults to SPI hardware if available
	if (!port)
	{
#ifdef MCU_HAS_SPI
		while (mcu_spi_bulk_transfer(out, in, len))
		{
			cnc_yield();
		}
#endif
		return;
	}

	// if port with custom method execute it
	if (port->spiport)
	{
		while (port->spiport->bulk_xmit(out, in, len))
		{
			cnc_yield();
		}
		return;
	}
#endif

	uint32_t timeout = BULK_SPI_TIMEOUT + mcu_millis();
	while (len--)
	{
		uint8_t c = softspi_xmit(port, *out++);
		if (in)
		{
			*in++ = c;
		}

		if (timeout < mcu_millis())
		{
			timeout = BULK_SPI_TIMEOUT + mcu_millis();
			cnc_yield();
		}
	}
}

void softspi_start(softspi_port_t *port)
{
	if (!port)
	{
#ifdef MCU_HAS_SPI
		MODULE_LOCK_ENABLE(LISTENER_HWSPI_LOCK);
#endif
		return;
	}

	// if port with custom method execute it
	// usually HW ports
	if (port->spiport)
	{
		if (port->spiport == MCU_SPI)
		{
			MODULE_LOCK_ENABLE(LISTENER_HWSPI_LOCK);
		}
		else
		{
			MODULE_LOCK_ENABLE(LISTENER_HWSPI2_LOCK);
		}
		port->spiport->start(port->spiconfig, port->spifreq);
		return;
	}

#ifdef SOFTSPI_LOCKGUARD_ENABLED
	MODULE_LOCK_ENABLE(LISTENER_SWSPI_LOCK);
#endif
	softspi_config(port, port->spiconfig, port->spifreq);
}

void softspi_stop(softspi_port_t *port)
{
	if (!port)
	{
#ifdef MCU_HAS_SPI
		MODULE_LOCK_DISABLE(LISTENER_HWSPI_LOCK);
#endif
		return;
	}

	// if port with custom method execute it
	if (port->spiport)
	{
		port->spiport->stop();
		if (port->spiport == MCU_SPI)
		{
			MODULE_LOCK_DISABLE(LISTENER_HWSPI_LOCK);
		}
		else
		{
			MODULE_LOCK_DISABLE(LISTENER_HWSPI2_LOCK);
		}
	}
	// unlocks resource
#ifdef SOFTSPI_LOCKGUARD_ENABLED
	MODULE_LOCK_DISABLE(LISTENER_SWSPI_LOCK);
#endif
}
