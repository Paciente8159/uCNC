/*
	Name: print.h
	Description: Print utilities for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 18/09/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef PRINT_H
#define PRINT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#define ITOF_NUMBER_UNDEF 0
#define ITOF_NUMBER_OK 0x20
#define ITOF_NUMBER_ISFLOAT 0x40
#define ITOF_NUMBER_ISNEGATIVE 0x80

	// printing utils
	typedef void (*print_putc_cb)(char);
	void print_fmtva(print_putc_cb cb, char *buffer, const char *fmt, va_list *args);
	void print_fmt(print_putc_cb cb, char *buffer, const char *fmt, ...);
	// scaning utilities
	typedef char(*print_read_input_cb)(bool);
	uint8_t print_itof(print_read_input_cb cb, const char **buffer, float *value);

// string helper functions
#define str_sprintf(buffer, fmt, ...) print_fmt(NULL, buffer, fmt, __VA_ARGS__)
#define str_itof(buffer, var) print_itof(NULL, buffer, var)

#ifdef __cplusplus
}
#endif

#endif