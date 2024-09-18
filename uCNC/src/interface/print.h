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

	// printing utils
	typedef void* (*print_cb)(void*, char);
	void print_str(print_cb cb, void* arg, const char *__s);
	void print_bytes(print_cb cb, void* arg, const uint8_t *data, uint8_t count);
	void print_int(print_cb cb, void* arg, int32_t num);
	void print_flt(print_cb cb, void* arg, float num);
	void print_ip(print_cb cb, void* arg, uint32_t ip);
	void print_fmt(print_cb cb, void *arg, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif