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
#include "../cnc.h"
#include "softi2c.h"

static void softi2c_stop(softi2c_port_t *port)
{
	port->sda(false);
	mcu_delay_us(port->i2cdelay);
	port->scl(true);
	mcu_delay_us(port->i2cdelay);
	port->sda(true);
}

static uint8_t softi2c_clock_stretch(softi2c_port_t *port, uint32_t ms_timeout)
{
	// releases the clock then monitors the clock line to see if it's available or not
	// if not released under 50ms issues a timeout error
	port->scl(true);
	__TIMEOUT_MS__(ms_timeout)
	{
		if (port->get_scl())
		{
			return I2C_OK;
		}
		// if not in ISR run main loop
		if (mcu_get_global_isr())
		{
			cnc_dotasks();
		}
	}

	softi2c_stop(port);
	return I2C_NOTOK;
}

static uint8_t softi2c_write(softi2c_port_t *port, uint8_t c, bool send_start, bool send_stop, uint32_t ms_timeout)
{
	uint8_t ack = 0;
	cnc_dotasks();
	if (send_start)
	{
		// init
		port->sda(true);
		port->scl(true);
		uint32_t timeout = ms_timeout;
		__TIMEOUT_MS__(timeout)
		{
			if (port->get_sda())
			{
				break;
			}
			// if not in ISR run main loop
			if (mcu_get_global_isr())
			{
				cnc_dotasks();
			}
		}
		__TIMEOUT_ASSERT__(timeout)
		{
			return I2C_NOTOK;
		}

		if (softi2c_clock_stretch(port, ms_timeout) != I2C_OK)
		{
			return I2C_NOTOK;
		}

		port->sda(false);
		mcu_delay_us(port->i2cdelay);
		port->scl(false);
		mcu_delay_us(port->i2cdelay);
	}

	for (uint8_t i = 0; i < 8; i++)
	{
		port->sda((c & 0x80));
		mcu_delay_us(port->i2cdelay);
		if (softi2c_clock_stretch(port, ms_timeout) != I2C_OK)
		{
			return I2C_NOTOK;
		}
		mcu_delay_us(port->i2cdelay);
		port->scl(false); // write the most-significant bit
		c <<= 1;
	}

	// read ack
	port->sda(true);
	mcu_delay_us(port->i2cdelay);
	if (softi2c_clock_stretch(port, ms_timeout) != I2C_OK)
	{
		return I2C_NOTOK;
	}
	ack = !port->get_sda();
	port->scl(false);

	if (send_stop || !ack)
	{
		softi2c_stop(port);
	}

	return ((ack) ? I2C_OK : I2C_NOTOK);
}

static uint8_t softi2c_read(softi2c_port_t *port, bool with_ack, bool send_stop, uint32_t ms_timeout)
{
	uint8_t c = 0;
	cnc_dotasks();
	for (uint8_t i = 0; i < 8; i++)
	{
		mcu_delay_us(port->i2cdelay);
		if (softi2c_clock_stretch(port, ms_timeout) != I2C_OK)
		{
			return I2C_NOTOK;
		}
		c <<= 1;
		c |= (uint8_t)port->get_sda();
		port->scl(false);
	}

	port->sda(!with_ack);
	mcu_delay_us(port->i2cdelay);
	if (softi2c_clock_stretch(port, ms_timeout) != I2C_OK)
	{
		return I2C_NOTOK;
	}
	mcu_delay_us(port->i2cdelay);
	port->scl(false);

	if (send_stop)
	{
		softi2c_stop(port);
	}

	return c;
}

uint8_t softi2c_send(softi2c_port_t *port, uint8_t address, uint8_t *data, uint8_t len, bool release, uint32_t ms_timeout)
{
	if (!port)
	{
#ifdef MCU_HAS_I2C
		return mcu_i2c_send(address, data, len, release, ms_timeout);
#else
		return I2C_NOTOK;
#endif
	}

	if (len)
	{
		len--;
		if (softi2c_write(port, address << 1, true, false, ms_timeout) == I2C_OK) // start, send address, write
		{
			// send data, stop
			for (uint8_t i = 0; i < len; i++)
			{
				if (softi2c_write(port, data[i], false, false, ms_timeout) != I2C_OK)
				{
					return I2C_NOTOK;
				}
			}

			return softi2c_write(port, data[len], false, release, ms_timeout);
		}
	}
	return I2C_NOTOK;
}

uint8_t softi2c_receive(softi2c_port_t *port, uint8_t address, uint8_t *data, uint8_t len, uint32_t ms_timeout)
{
	if (!port)
	{
#ifdef MCU_HAS_I2C
		return mcu_i2c_receive(address, data, len, ms_timeout);
#else
		return I2C_NOTOK;
#endif
	}

	if (len)
	{
		len--;
		if (softi2c_write(port, (address << 1) | 0x01, true, false, ms_timeout) == I2C_OK) // start, send address, write
		{
			for (uint8_t i = 0; i < len; i++)
			{
				data[i] = softi2c_read(port, true, false, ms_timeout);
			}

			data[len] = softi2c_read(port, false, true, ms_timeout);
			return I2C_OK;
		}
	}

	return I2C_NOTOK;
}

void softi2c_config(softi2c_port_t *port, uint32_t frequency)
{
	if (!port)
	{
#ifdef MCU_HAS_I2C
		mcu_i2c_config(frequency);
#endif
		return;
	}

	port->i2cdelay = I2C_DELAY(frequency);
	softi2c_stop(port);
}
