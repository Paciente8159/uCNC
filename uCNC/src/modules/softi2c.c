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
#include "softi2c.h"

void softi2c_delay(uint8_t loops_100ns)
{
	while (loops_100ns--)
	{
		mcu_delay_100ns();
	}
}

static void softi2c_stop(softi2c_port_t *port)
{
	port->sda(false);
	softi2c_delay(port->i2cdelay);
	port->scl(true);
	softi2c_delay(port->i2cdelay);
	port->sda(true);
}

static uint8_t softi2c_clock_stretch(softi2c_port_t *port)
{
	// releases the clock then monitors the clock line to see if it's available or not
	// if not released under 50ms issues a timeout error
	uint8_t timeout = 50;
	port->scl(true);
	while (!port->get_scl())
	{
		if (timeout--)
		{
			cnc_delay_ms(1);
		}
		else
		{
			// sent stop condition and exit
			softi2c_stop(port);
			return I2C_NOTOK;
		}
	}

	return I2C_OK;
}

static uint8_t softi2c_write(softi2c_port_t *port, uint8_t c, bool send_start, bool send_stop)
{
	uint8_t ack = 0;

	if (send_start)
	{
		// init
		port->sda(true);
		uint8_t timeout = 50;
		while (!port->get_sda())
		{
			if (timeout--)
			{
				cnc_delay_ms(1);
			}
			else
			{
				// unable to generate start condition
				return I2C_NOTOK;
			}
		}

		if (softi2c_clock_stretch(port) != I2C_OK)
		{
			return I2C_NOTOK;
		}

		port->sda(false);
		softi2c_delay(port->i2cdelay);
		port->scl(false);
		softi2c_delay(port->i2cdelay);
	}

	for (uint8_t i = 0; i < 8; i++)
	{
		port->sda((c & 0x80));
		softi2c_delay(port->i2cdelay);
		if (softi2c_clock_stretch(port) != I2C_OK)
		{
			return I2C_NOTOK;
		}
		softi2c_delay(port->i2cdelay);
		port->scl(false); // write the most-significant bit
		c <<= 1;
	}

	// read ack
	port->sda(true);
	softi2c_delay(port->i2cdelay);
	if (softi2c_clock_stretch(port) != I2C_OK)
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

static uint8_t softi2c_read(softi2c_port_t *port, bool with_ack, bool send_stop)
{
	uint8_t c = 0xFF;
	uint8_t i = 8;
	do
	{
		softi2c_delay(port->i2cdelay);
		if (softi2c_clock_stretch(port) != I2C_OK)
		{
			return I2C_NOTOK;
		}
		c <<= 1;
		c |= (uint8_t)port->get_sda();
		port->scl(false);

	} while (!--i);

	port->sda(!with_ack);
	softi2c_delay(port->i2cdelay);
	if (softi2c_clock_stretch(port) != I2C_OK)
	{
		return I2C_NOTOK;
	}
	softi2c_delay(port->i2cdelay);
	port->scl(false);

	if (send_stop)
	{
		softi2c_stop(port);
	}

	return c;
}

uint8_t softi2c_send(softi2c_port_t *port, uint8_t address, uint8_t *data, uint8_t len, bool release)
{
	if (!port)
	{
#ifdef MCU_HAS_I2C
		return mcu_i2c_send(address, data, len, release);
#else
		return I2C_NOTOK;
#endif
	}

	if (len)
	{
		len--;
		if (softi2c_write(port, address << 1, true, false) == I2C_OK) // start, send address, write
		{
			// send data, stop
			for (uint8_t i = 0; i < len; i++)
			{
				if (softi2c_write(port, data[i], false, false) != I2C_OK)
				{
					return I2C_NOTOK;
				}
			}

			return softi2c_write(port, data[len], false, release);
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
		if (softi2c_write(port, (address << 1) | 0x01, true, false) == I2C_OK) // start, send address, write
		{
			for (uint8_t i = 0; i < len; i++)
			{
				data[i] = softi2c_read(port, true, false);
			}

			data[len] = softi2c_read(port, false, true);
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
}
