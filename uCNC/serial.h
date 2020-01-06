/*
	Name: serial.h
	Description: Serial communication basic read/write functions uCNC
	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 30/12/2019

	uCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	uCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef SERIAL_H
#define SERIAL_H

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

void serial_init();

bool serial_rx_is_empty();
char serial_getc();
char serial_peek();
void serial_inject_cmd(const char* __s);
void serial_discard_cmd();

bool serial_tx_is_empty();
void serial_putc(char c);
void serial_print_str(const char* __s);
void serial_print_int(uint16_t num);
void serial_print_flt(float num);
void serial_print_intarr(uint16_t* arr, int count);
void serial_print_fltarr(float* arr, int count);
void serial_flush();

//ISR
void serial_rx_isr(char c);
char serial_tx_isr();

#endif
