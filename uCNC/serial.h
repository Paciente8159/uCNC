/*
	Name: serial.h
	Description: Serial communication basic read/write functions uCNC.

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

#define EOL '\0'
#define OVF 0xFF //overflow char
#define RX_BUFFER_SIZE 129 //buffer sizes
#define TX_BUFFER_SIZE 129 //buffer sizes

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

void serial_init();

bool serial_rx_is_empty();
unsigned char serial_getc();
unsigned char serial_peek();
void serial_inject_cmd(const unsigned char* __s);
void serial_restore_line();
//void serial_rx_clear();

bool serial_tx_is_empty();
void serial_putc(unsigned char c);
void serial_print_str(const unsigned char* __s);
void serial_print_int(uint16_t num);
void serial_print_flt(float num);
void serial_print_intarr(uint16_t* arr, uint8_t count);
void serial_print_fltarr(float* arr, uint8_t count);
void serial_flush();

//ISR
void serial_rx_isr(unsigned char c);
void serial_tx_isr();

#endif
