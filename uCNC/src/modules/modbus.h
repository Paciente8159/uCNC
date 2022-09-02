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

#ifndef MODBUS_H
#define MODBUS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../cnc.h"
#include "softuart.h"
#include <stdint.h>

#ifndef MODBUS_DATA_MAX_LEN
#define MODBUS_DATA_MAX_LEN 10
#endif

#define MODBUS_READ_COIL_STATUS 0x01
#define MODBUS_READ_INPUT_STATUS 0x02
#define MODBUS_READ_HOLDING_REGISTERS 0x03
#define MODBUS_READ_INPUT_REGISTERS 0x04
#define MODBUS_FORCE_SINGLE_COIL 0x05
#define MODBUS_PRESET_SINGLE_REGISTER 0x06
#define MODBUS_FORCE_MULTIPLE_COILS 0x0F
#define MODBUS_PRESET_MULTIPLE_REGISTERS 0x10
#define MODBUS_READ_WRITE_MULTIPLE_REGISTERS 0x017

typedef struct modbus_request
{
	uint8_t address;
	uint8_t fcode;
	uint8_t startaddress[2];
	uint8_t value[2];
	uint8_t datalen;
	uint8_t data[MODBUS_DATA_MAX_LEN];
	uint16_t crc;
} modbus_request_t;

typedef struct modbus_response
{
	uint8_t address;
	uint8_t fcode;
	uint8_t data[MODBUS_DATA_MAX_LEN];
	uint16_t crc;
} modbus_response_t;

void send_request(modbus_request_t request, uint8_t len, softuart_port_t *port);
bool read_response(modbus_response_t *response, uint8_t len, softuart_port_t *port, uint32_t ms_timeout);

#ifdef __cplusplus
}
#endif

#endif