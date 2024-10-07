/*
	Name: grbl_print.h
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

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#define ATOF_NUMBER_UNDEF 0
#define ATOF_NUMBER_OK 0x20
#define ATOF_NUMBER_ISFLOAT 0x40
#define ATOF_NUMBER_ISNEGATIVE 0x80

#define PRINT_CALLBACK SIZE_MAX
#define PRINT_MAX (SIZE_MAX - 1)

	// printing utils
	typedef void (*prt_putc_cb)(char);
	size_t prt_byte(void *out, size_t maxlen, const uint8_t *data, uint8_t flags);
	size_t prt_int(void *out, size_t maxlen, uint32_t num, uint8_t padding);
	size_t prt_flt(void *out, size_t maxlen, float num, uint8_t precision);
	size_t prt_ip(void *out, size_t maxlen, uint32_t ip);
	size_t prt_fmtva(void *out, size_t maxlen, const char *fmt, va_list *args);
	size_t prt_fmt(void *out, size_t maxlen, const char *fmt, ...);
	// scaning utilities
	typedef unsigned char (*prt_getc_cb)(bool);
	typedef unsigned char (*prt_read_rom_byte)(const char *);
	uint8_t prt_atof(void *cb, const char **buffer, float *value);

// string helper functions
#define str_sprintf(buffer, fmt, ...) prt_fmt(buffer, PRINT_MAX, fmt, __VA_ARGS__)
#define str_snprintf(buffer, n, fmt, ...) prt_fmt(buffer, n, fmt, __VA_ARGS__)
#define str_atof(buffer, var) prt_atof(NULL, buffer, PRINT_MAX, var)

#ifdef __cplusplus
}
#endif

#endif