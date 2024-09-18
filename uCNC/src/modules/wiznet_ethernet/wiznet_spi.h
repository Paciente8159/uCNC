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
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/**
 * 
 * wiznet_spi.c
 * 
 */
void wiznet_init(void);
void wiznet_critical_section_enter(void);
void wiznet_critical_section_exit(void);
void wiznet_cs_select(void);
void wiznet_cs_deselect(void);
uint8_t wiznet_getc(void);
void wiznet_putc(uint8_t c);
void wiznet_read(uint8_t *buff, uint16_t len);
void wiznet_write(uint8_t *buff, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif