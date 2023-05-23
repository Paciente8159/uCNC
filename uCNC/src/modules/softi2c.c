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

static uint8_t softi2c_clock_stretch(softi2c_port_t *port)
{
	uint8_t timeout = 3;
	while (!port->get_scl() && timeout--)
	{
		softi2c_delay(port->i2cdelay);
	}

	return timeout;
}

static void softi2c_stop(softi2c_port_t *port)
{
	port->sda(false);
	softi2c_delay(port->i2cdelay);
	port->scl(true);
	softi2c_clock_stretch(port);
	port->sda(true);
	softi2c_delay(port->i2cdelay);
}

static uint8_t softi2c_write(softi2c_port_t *port, uint8_t c, bool send_start, bool send_stop)
{
	if (!port)
	{
#ifdef MCU_HAS_I2C
		return mcu_i2c_write(c, send_start, send_stop);
#else
		return 0;
#endif
	}

	uint8_t ack = 0;

	if (send_start)
	{
		// init
		port->sda(true);
		port->scl(true);
		if (!softi2c_clock_stretch(port))
		{
			return 0;
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
		port->scl(true);
		softi2c_delay(port->i2cdelay);
		port->scl(false); // write the most-significant bit
		c <<= 1;
	}

	// read ack
	port->sda(true);
	softi2c_delay(port->i2cdelay);
	port->scl(true);
	softi2c_clock_stretch(port);
	ack = !port->get_sda();
	port->scl(false);

	if (send_stop || !ack)
	{
		softi2c_stop(port);
	}

	return ack;
}

static uint8_t softi2c_read(softi2c_port_t *port, bool with_ack, bool send_stop)
{
	if (!port)
	{
#ifdef MCU_HAS_I2C
		return mcu_i2c_read(with_ack, send_stop);
#else
		return 0;
#endif
	}

	uint8_t c = 0;
	uint8_t i = 8;
	do
	{
		c <<= 1;
		c |= (uint8_t)port->get_sda();
	} while (!--i);

	port->sda(!with_ack);
	softi2c_delay(port->i2cdelay);
	port->scl(true);
	softi2c_delay(port->i2cdelay);
	port->scl(false);

	if (send_stop)
	{
		softi2c_stop(port);
	}

	return c;
}

uint8_t softi2c_send(softi2c_port_t *port, uint8_t address, uint8_t *data, uint8_t len)
{
	if (len)
	{
		len--;
		if (softi2c_write(port, address << 1, true, false)) // start, send address, write
		{
			// send data, stop
			for (uint8_t i = 0; i < len; i++)
			{
				if (!softi2c_write(port, data[i], false, false))
				{
					return 0;
				}
			}

			return softi2c_write(port, data[len], false, true);
		}
	}
	return 0;
}

uint8_t softi2c_receive(softi2c_port_t *port, uint8_t address, uint8_t *data, uint8_t len)
{
	if (len)
	{
		len--;
		if (softi2c_write(port, (address << 1) | 0x01, true, false)) // start, send address, write
		{
			for (uint8_t i = 0; i < len; i++)
			{
				data[i] = softi2c_read(port, true, false);
			}

			data[len] = softi2c_read(port, false, true);
		}
	}

	return 0;
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
