/*
	Name: modbus.c
	Description: An modbus library for for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 16/08/2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../cnc.h"
#include "modbus.h"
#include <stdint.h>

#define MODBUS_UARTBAUD 38400

FORCEINLINE static uint16_t crc16(uint8_t *data, uint8_t len)
{
	uint16_t crc = 0xFFFF;
	uint8_t pos;

	for (pos = 0; pos < len; pos++)
	{
		crc ^= (uint16_t)data[pos]; // XOR byte into least sig. byte of crc
		for (uint8_t i = 8; i != 0; i--)
		{
			if ((crc & 0x0001) != 0)
			{
				crc >>= 1;
				crc ^= 0xA001;
			}
			else
				crc >>= 1;
		}
	}

	return crc;
}

void send_request(modbus_request_t request, softuart_port_t *port)
{
	uint8_t *data = (uint8_t *)&request;
	uint8_t len = !request.datalen ? 8 : (8 + 1 + request.datalen);
	request.crc = crc16(data, len);

	while (len--)
	{
		softuart_putc(port, *data);
		data++;
	}

	softuart_putc(port, ((uint8_t *)request.crc)[0]);
	softuart_putc(port, ((uint8_t *)request.crc)[1]);
}

bool read_response(modbus_response_t *response, softuart_port_t *port)
{
	int16_t c = 0;
	uint8_t len;
	uint8_t *data = (uint8_t*)response;
	do
	{
		c = softuart_getc(port);
		*data = (uint8_t)(0xFF & softuart_getc(port));
		data++;
		len++;
	} while (c >= 0);

	if (len < 6)
	{
		return false;
	}

	data-=2;
	response->crc = *((uint16_t*)data);
	return true;
}
