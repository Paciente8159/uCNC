/*
	Name: print.c
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
#include "print.h"
#include "../cnc.h"

#define PRINT_ENABLE_MINIMAL
#ifdef PRINT_ENABLE_MINIMAL
#define PRINT_DISABLE_FMT_PADDING
#define PRINT_DISABLE_FMT_HEX
#define PRINT_DISABLE_FMT_IP
#define PRINT_ENABLE_FMT_FLOAT_FONLY
#endif

#define HEX_NONE 0
#define HEX_PREFIX 128
#define HEX_UPPER 64
#define VAR_BYTE 1
#define VAR_WORD 2
#define VAR_DWORD 4
#define HEX_SIZE_MASK (VAR_DWORD | VAR_WORD | VAR_BYTE)
#define HEX_SIZE(X) ((X & HEX_SIZE_MASK) >> 1)

#ifndef printf_getc
#define printf_getc(fmt) rom_read_byte(fmt)
#endif

static void print_putc(print_putc_cb cb, char **buffer_ref, char c)
{
	if (cb)
	{
		cb(c);
	}
	else if (buffer_ref)
	{
		**buffer_ref = c;
		*buffer_ref += 1;
	}
}

static void print_str(print_putc_cb cb, char **buffer_ref, const char *__s)
{
	while (*__s)
	{
		print_putc(cb, buffer_ref, *__s++);
	}
}

static void print_romstr(print_putc_cb cb, char **buffer_ref, const char *__s)
{
	char c = rom_read_byte(__s);
	while (c)
	{
		print_putc(cb, buffer_ref, c);
		__s++;
		c = rom_read_byte(__s);
	}
}

#ifndef PRINT_DISABLE_FMT_HEX
static void print_byte(print_putc_cb cb, char **buffer_ref, const uint8_t *data, uint8_t flags)
{
	bool prefix = (flags && HEX_PREFIX);
	char hexchar = (flags & HEX_UPPER) ? 'A' : 'a';
	uint8_t size = HEX_SIZE(flags);
	do
	{
		if (prefix)
		{
			print_putc(cb, buffer_ref, '0');
			print_putc(cb, buffer_ref, 'x');
		}
		print_putc(cb, buffer_ref, ' ');
		uint8_t up = *data >> 4;
		uint8_t c = (up > 9) ? (hexchar + up - 10) : ('0' + up);
		print_putc(cb, buffer_ref, c);
		up = *data & 0x0F;
		c = (up > 9) ? (hexchar + up - 10) : ('0' + up);
		print_putc(cb, buffer_ref, c);
		data++;
	} while (--size);
}
#endif

static void print_int(print_putc_cb cb, char **buffer_ref, int32_t num, uint8_t padding)
{
	uint8_t buffer[11];
	uint8_t i = 0;

	if (num < 0)
	{
		print_putc(cb, buffer_ref, '-');
		num = -num;
	}

	while (num > 0)
	{
		uint8_t digit = num % 10;
		num = (uint32_t)truncf((float)num * 0.1f);
		buffer[i++] = digit;
	}

	while (i < padding--)
	{
		print_putc(cb, buffer_ref, '0');
	}

	do
	{
		i--;
		print_putc(cb, buffer_ref, '0' + buffer[i]);
	} while (i);
}

static void FORCEINLINE print_flt(print_putc_cb cb, char **buffer_ref, float num)
{
	int32_t interger = (int32_t)floorf(num);
	num -= interger;
	uint32_t mult = (!g_settings.report_inches) ? 1000 : 10000;
	num *= mult;
	uint32_t digits = (uint32_t)lroundf(num);
	if (digits == mult)
	{
		interger++;
		digits = 0;
	}

	print_int(cb, buffer_ref, interger, 0);
	print_putc(cb, buffer_ref, '.');
	print_int(cb, buffer_ref, digits, (!g_settings.report_inches) ? 3 : 5);
}

#ifndef PRINT_DISABLE_FMT_IP
void print_ip(print_putc_cb cb, char **buffer_ref, uint32_t ip)
{
	uint8_t *ptr = (uint8_t *)&ip;
	print_int(cb, buffer_ref, (int32_t)ptr[3], 0);
	print_putc(cb, buffer_ref, '.');
	print_int(cb, buffer_ref, (int32_t)ptr[2], 0);
	print_putc(cb, buffer_ref, '.');
	print_int(cb, buffer_ref, (int32_t)ptr[1], 0);
	print_putc(cb, buffer_ref, '.');
	print_int(cb, buffer_ref, (int32_t)ptr[0], 0);
}
#endif

// static char itof_getc_dummy(bool peek)
// {
// 	return 0;
// }

void print_fmtva(print_putc_cb cb, char *buffer, const char *fmt, va_list *args)
{
	char c = 0, cval = 0;
	const char *s;
	uint8_t lcount = 0;
#ifndef PRINT_DISABLE_FMT_HEX
	bool hexflags = HEX_NONE;
#endif
	void *pt = NULL;
	int32_t i = 0;
	float f, *f_ptr = NULL;
	char **buffer_ref = NULL;
	char *ptr = buffer;
	uint8_t elems = 0;

	if (ptr)
	{
		buffer_ref = &ptr;
	}

	do
	{
		c = printf_getc(fmt++);
		if (c == '%')
		{
			c = printf_getc(fmt++);
			switch (c)
			{
#ifndef PRINT_DISABLE_FMT_PADDING
			case '#':
				hexflags = HEX_PREFIX;
				__FALL_THROUGH__
			case '-':
			case '+':
				fmt++;
				__FALL_THROUGH__
			case '0':
				while (c >= '0' && c <= '9' && c)
				{
					c = printf_getc(fmt++);
				}
				__FALL_THROUGH__
			case '.':
				fmt++;
				__FALL_THROUGH__
#endif
			default:
				if (c == '.' || (c >= '1' && c <= '9'))
				{
					if (print_atof(NULL/*itof_getc_dummy*/, (const char **)&fmt, &f))
					{
						elems = (uint8_t)f;
					}
					c = printf_getc(fmt++);
				}
				break;
			}

			while (c == 'l' || c == 'h')
			{
				if (c == 'l')
				{
					lcount++;
				}
				c = printf_getc(fmt++);
			}

			switch (c)
			{
			case 'c':
				cval = (char)va_arg(*args, int);
				print_putc(cb, buffer_ref, cval);
				/* code */
				break;
			case 's':
				s = (char *)va_arg(*args, char *);
				print_str(cb, buffer_ref, s);
				break;
			case 'S':
				s = (const char *)va_arg(*args, const char *);
				print_romstr(cb, buffer_ref, s);
				break;
#ifndef PRINT_DISABLE_FMT_IP
			case 'M':
				lcount = 4;
				__FALL_THROUGH__
#endif
#ifndef PRINT_DISABLE_FMT_HEX
			case 'X':
				hexflags |= HEX_UPPER;
				__FALL_THROUGH__
			case 'x':
#endif
			case 'd':
			case 'i':
			case 'u':
				if (elems)
				{
					pt = va_arg(*args, void *);
				}
				else
				{
					switch (lcount)
					{
					case 0:
						i = (int32_t)va_arg(*args, int);
						break;
					case 1:
						i = (int32_t)va_arg(*args, uint16_t);
						break;
					default:
						i = (int32_t)va_arg(*args, uint32_t);
						break;
					}
					elems=1;
				}
				do
				{
					switch (c)
					{
					case 'u':
						i = ABS(i);
						__FALL_THROUGH__
					case 'd':
					case 'i':
						print_int(cb, buffer_ref, i, 0);
						break;
#ifndef PRINT_DISABLE_FMT_HEX
					case 'x':
					case 'X':
						print_byte(cb, buffer_ref, (const uint8_t *)&i, (hexflags | lcount));
						break;
#endif
#ifndef PRINT_DISABLE_FMT_IP
					case 'M':
						print_ip(cb, buffer_ref, i);
						break;
#endif
					}
					if (elems && --elems)
					{
						print_putc(cb, buffer_ref, ',');
					}
					pt += (1 << lcount);
				} while (elems);
				/* code */
				break;
#ifndef PRINT_DISABLE_FMT_HEX
			case 'A':
				hexflags |= HEX_UPPER;
				__FALL_THROUGH__
			case 'a':
#endif
			case 'f':
			case 'F':
#ifndef PRINT_ENABLE_FMT_FLOAT_FONLY
			case 'e':
			case 'E':
			case 'g':
			case 'G':
#endif
				if (elems)
				{
					f_ptr = va_arg(*args, float *);
				}
				else
				{
					f = (float)va_arg(*args, double);
					f_ptr = &f;
					elems=1;
				}
				do
				{
					switch (c)
					{
#ifndef PRINT_DISABLE_FMT_HEX
					case 'a':
					case 'A':
						print_byte(cb, buffer_ref, (const uint8_t *)f_ptr, (hexflags | VAR_DWORD));
						break;
#endif
					default:
						print_flt(cb, buffer_ref, *f_ptr++);
						break;
					}
					if (elems && --elems)
					{
						print_putc(cb, buffer_ref, ',');
					}
				} while (elems);
				/* code */
				break;
			case '%':
				print_putc(cb, buffer_ref, '%');
				break;
			default:
				print_putc(cb, buffer_ref, '%');
				print_putc(cb, buffer_ref, c);
				break;
			}
		}
		else if(c){
			cb(c);
		}
	} while (c);
}

void print_fmt(print_putc_cb cb, char *buffer, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	print_fmtva(cb, buffer, fmt, &args);
	va_end(args);
}

#define atof_peek(cb, buffer) ((!buffer) ? cb(true) : rom_read_byte(*buffer)) /*((cb) ? rom_read_byte(*buffer) : **buffer))*/
#define atof_get(cb, buffer) ((!buffer) ? cb(false) : ({ *buffer += 1; 0; }))

uint8_t print_atof(print_read_input_cb cb, const char **buffer, float *value)
{
	uint32_t intval = 0;
	uint8_t fpcount = 0;
	uint8_t result = ATOF_NUMBER_UNDEF;
	float rhs = 0;

	uint8_t c = (uint8_t)atof_peek(cb, buffer);

	if (c == '-' || c == '+')
	{
		if (c == '-')
		{
			result |= ATOF_NUMBER_ISNEGATIVE;
		}
		atof_get(cb, buffer);
		c = (uint8_t)atof_peek(cb, buffer);
	}

	for (;;)
	{
		switch (c)
		{
		case '.':
			result |= ATOF_NUMBER_ISFLOAT;
			break;
		default:
			c -= 48;
			if (c <= 9)
			{
				intval = fast_int_mul10(intval) + c;
				if (result & ATOF_NUMBER_ISFLOAT)
				{
					fpcount++;
				}

				result |= ATOF_NUMBER_OK;
			}
			else
			{
				if ((result & ATOF_NUMBER_OK))
				{
					rhs = (float)intval;
					while (fpcount--)
					{
						rhs *= 0.1f;
					}

					*value = (result & ATOF_NUMBER_ISNEGATIVE) ? -rhs : rhs;
					return result;
				}
			}
			break;
		}
	}

	return ATOF_NUMBER_UNDEF;
}
