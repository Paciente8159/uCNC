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

static void softi2c_stop(softi2c_port_t *port)
{
	port->sda(false);
	port->wait();
	port->scl(true);
	port->wait();
	port->sda(true);
	port->wait();
}

uint8_t softi2c_write(softi2c_port_t *port, uint8_t c, bool send_start, bool send_stop)
{
	uint8_t ack = 0;

	if (send_start)
	{
		// init
		port->sda(true);
		port->scl(true);
		port->wait();
		port->sda(false);
		port->wait();
		port->scl(false);
		port->wait();
	}

	for (uint8_t i = 0; i < 8; i++)
	{
		port->sda((c & 0x80));
		port->wait();
		port->scl(true);
		port->wait();
		port->scl(false); // write the most-significant bit
		c <<= 1;
	}

	// read ack
	port->sda(true);
	port->wait();
	port->scl(true);
	port->wait();
	ack = !port->get_sda();
	port->scl(false);

	if (send_stop)
	{
		softi2c_stop(port);
	}

	return ack;
}

uint8_t softi2c_read(softi2c_port_t *port, bool ack, bool send_stop)
{
	uint8_t c = 0;
	uint8_t i = 8;
	do
	{
		c <<= 1;
		c |= (uint8_t)port->get_sda();
	} while (!--i);

	port->sda(!ack);
	port->wait();
	port->scl(true);
	port->wait();
	port->scl(false);

	if (send_stop)
	{
		softi2c_stop(port);
	}

	return c;
}

uint8_t softi2c_write_byte(softi2c_port_t *port, uint8_t address, uint8_t c)
{
	if (softi2c_write(port, (address << 1), true, false)) // start, send address, write
	{
		// send data, stop
		if (softi2c_write(port, c, false, true))
			return 1;
	}

	softi2c_stop(port); // make sure to impose a stop if NAK'd
	return 0;
}

uint8_t softi2c_read_byte(softi2c_port_t *port, uint8_t address)
{
	if (softi2c_write(port, (address << 1) | 0x01, true, false)) // start, send address, read
	{
		return softi2c_read(port, false, true);
	}

	return 0; // return zero if NAK'd
}

uint8_t softi2c_write_reg(softi2c_port_t *port, uint8_t address, uint8_t reg, uint8_t c)
{
	if (softi2c_write(port, address << 1, true, false)) // start, send address, write
	{
		// send data, stop
		if (softi2c_write(port, reg, false, false))
		{
			return softi2c_write(port, c, false, true);
		}
	}

	softi2c_stop(port); // make sure to impose a stop if NAK'd
	return 0;
}

uint8_t softi2c_read_reg(softi2c_port_t *port, uint8_t address, uint8_t reg)
{
	if (softi2c_write(port, address << 1, true, false)) // start, send address, write
	{
		// send data, stop
		if (softi2c_write(port, reg, false, false))
		{
			if (softi2c_write(port, (address << 1) | 0x01, true, false)) // start, send address, read
			{
				return softi2c_read(port, false, true);
			}
		}
	}

	softi2c_stop(port); // make sure to impose a stop if NAK'd
	return 0;
}
