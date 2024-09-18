/*
	Name: str_helper.h
	Description: String helpers for µCNC.

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

#ifndef STR_HELPER_H
#define STR_HELPER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include "print.h"

#define sh_printf(buffer, fmt, ...)                  \
	{                                                 \
		char *ptr = buffer;                             \
		char **ptr_ref = &ptr;                          \
		print_fmt(str_putc, ptr_ref, fmt, __VA_ARGS__); \
	}

#define sh_flprintf(buffer, fmt, ...)                  \
	{                                                 \
		const char *ptr = buffer;                             \
		const char **ptr_ref = &ptr;                          \
		print_fmt(str_putc, ptr_ref, fmt, __VA_ARGS__); \
	}

#ifdef __cplusplus
}
#endif

#endif