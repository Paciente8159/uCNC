/*
	Name: serial.h
	Description: Serial communication basic read/write functions µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 30/12/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef SERIAL_H
#define SERIAL_H

#define EOL 0x00		   //end of line char
#define OVF 0x7F		   //overflow char
#define RX_BUFFER_SIZE 129 //buffer sizes
#define TX_BUFFER_SIZE 112 //buffer sizes

#define SERIAL_UART 0
#define SERIAL_N0 1
#define SERIAL_N1 2

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

void serial_init();

bool serial_rx_is_empty(void);
unsigned char serial_getc(void);
void serial_ungetc(void);
unsigned char serial_peek(void);
void serial_inject_cmd(const unsigned char *__s);
void serial_restore_line(void);
void serial_rx_clear(void);
void serial_select(uint8_t source);

bool serial_tx_is_empty(void);
void serial_putc(unsigned char c);
void serial_print_str(const unsigned char *__s);
void serial_print_int(uint16_t num);
#ifdef GCODE_PROCESS_LINE_NUMBERS
void serial_print_long(uint32_t num);
#endif
void serial_print_flt(float num);
void serial_print_intarr(uint16_t *arr, uint8_t count);
void serial_print_fltarr(float *arr, uint8_t count);
void serial_flush(void);

//ISR
void serial_rx_isr(unsigned char c);
void serial_tx_isr(void);

#endif
