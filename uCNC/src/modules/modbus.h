/*
	Name: modbus.h
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
#include <stdint.h>

#ifndef MODBUS_DATA_MAX_LEN
#define MODBUS_DATA_MAX_LEN 10
#endif

typedef struct modbus_request
{
	uint8_t address;
	uint8_t fcode;
	uint16_t startaddress;
	uint16_t value;
	uint8_t datalen;
	uint8_t data[MODBUS_DATA_MAX_LEN];
	uint16_t crc;
} modbus_request_t;

typedef struct modbus_response
{
	uint8_t address;
	uint8_t fcode;
	uint8_t len;
	uint16_t value;
	uint8_t data[MODBUS_DATA_MAX_LEN];
	uint16_t crc;
} modbus_response_t;

void send_request(modbus_request_t request);