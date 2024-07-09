/*
	Name: wiznet_ethernet.h
	Description: Implements a Wiznet Ethernet interface for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 09-07-2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef WIZNET_ETHERNET_H
#define WIZNET_ETHERNET_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../cnc.h"
#include "../softspi.h"
#include "../../modules/endpoint.h"
#include "../../modules/websocket.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifndef WIZNET_DRIVER
#define WIZNET_DRIVER W5500
#define _WIZCHIP_ WIZNET_DRIVER
#endif

#define WIZNET_HW_SPI 1
#define WIZNET_SW_SPI 2

#ifndef WIZNET_BUS
#define WIZNET_BUS WIZNET_HW_SPI
#endif

#ifndef WIZNET_CS
#define WIZNET_CS DOUT28
#endif

#if (WIZNET_BUS == WIZNET_HW_SPI)
#define WIZNET_SPI MCU_SPI
#endif

#ifdef __cplusplus
}
#endif

#endif