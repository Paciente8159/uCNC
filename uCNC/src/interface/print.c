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

void print_str(print_cb cb, void* arg, const char *__s)
{
	while (*__s)
	{
		cb(arg, *__s++);
	}
}

void print_bytes(print_cb cb, void *arg, const uint8_t *data, uint8_t count)
{
	do
	{
		cb(arg, ' ');
		uint8_t up = *data >> 4;
		uint8_t c = (up > 9) ? ('a' + up - 10) : ('0' + up);
		cb(arg, c);
		up = *data & 0x0F;
		c = (up > 9) ? ('a' + up - 10) : ('0' + up);
		cb(arg, c);
		data++;
	} while (--count);
}

void print_int(print_cb cb, void *arg, int32_t num)
{
	if (num == 0)
	{
		cb(arg, '0');
		return;
	}

	uint8_t buffer[11];
	uint8_t i = 0;

	if (num < 0)
	{
		cb(arg, '-');
		num = -num;
	}

	while (num > 0)
	{
		uint8_t digit = num % 10;
		num = (uint32_t)truncf((float)num * 0.1f);
		buffer[i++] = digit;
	}

	do
	{
		i--;
		cb(arg, '0' + buffer[i]);
	} while (i);
}

void print_flt(print_cb cb, void *arg, float num)
{
	if (num < 0)
	{
		cb(arg, '-');
		num = -num;
	}

	uint32_t interger = (uint32_t)floorf(num);
	num -= interger;
	uint32_t mult = (!g_settings.report_inches) ? 1000 : 10000;
	num *= mult;
	uint32_t digits = (uint32_t)lroundf(num);
	if (digits == mult)
	{
		interger++;
		digits = 0;
	}

	print_int(cb, arg, interger);
	cb(arg, '.');
	if (g_settings.report_inches)
	{
		if (digits < 1000)
		{
			cb(arg, '0');
		}
	}

	if (digits < 100)
	{
		cb(arg, '0');
	}

	if (digits < 10)
	{
		cb(arg, '0');
	}

	print_int(cb, arg, digits);
}

void print_ip(print_cb cb, void *arg, uint32_t ip)
{
	uint8_t *ptr = &ip;
	print_int(cb, arg, (int32_t)ptr[3]);
	cb(arg, '.');
	print_int(cb, arg, (int32_t)ptr[2]);
	cb(arg, '.');
	print_int(cb, arg, (int32_t)ptr[1]);
	cb(arg, '.');
	print_int(cb, arg, (int32_t)ptr[0]);
}

void print_fmt(print_cb cb, void *arg, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char c = 0, cval = 0, *s;
	uint8_t lcount = 0;
	int32_t val = 0;
	float fval = 0;
	do
	{
		c = *fmt++;
		if (c == '%')
		{
			c = *fmt++;
			switch (c)
			{
			case '-':
			case '+':
			case '#':
			case '0':
				while (c >= '0' && c <= '9' && c)
				{
					c = *fmt++;
				}
				__FALL_THROUGH__
			case '.':
				if (c == '.')
				{
					do
					{
						c = *fmt++;
					} while (c >= '0' && c <= '9' && c);
				}
			}

			while (c == 'l' || c == 'h')
			{
				if (c == 'l')
				{
					lcount++;
				}
				c = *fmt++;
			}

			switch (c)
			{
			case 'c':
				cval = (float)va_arg(args, char);
				cb(arg, cval);
				/* code */
				break;
			case 's':
				s = (char *)va_arg(args, char *);
				print_str(cb, arg, s);
				break;
			case 'd':
			case 'i':
				switch (lcount)
				{
				case 0:
					val = (uint32_t)va_arg(args, int8_t);
					break;
				case 1:
					val = (uint32_t)va_arg(args, int16_t);
					break;
				default:
					val = (uint32_t)va_arg(args, int32_t);
					break;
				}
				print_int(cb, arg, val);
				/* code */
				break;
			case 'M':
				lcount = 4;
				__FALL_THROUGH__
			case 'u':
			case 'x':
			case 'X':
				switch (lcount)
				{
				case 0:
					val = (uint32_t)va_arg(args, uint8_t);
					lcount = 1;
					break;
				case 1:
					val = (uint32_t)va_arg(args, uint16_t);
					lcount = 2;
					break;
				default:
					val = (uint32_t)va_arg(args, uint32_t);
					lcount = 4;
					break;
				}
				if (c == 'u')
				{
					print_int(cb, arg, val);
				}
				else if (c == 'M')
				{
					print_ip(cb, arg, val);
				}
				else
				{
					print_bytes(cb, arg, &val, lcount);
				}
				/* code */
				break;
			case 'f':
			case 'F':
			case 'e':
			case 'E':
			case 'a':
			case 'A':
			case 'g':
			case 'G':
				fval = (float)va_arg(args, float);
				if (c != 'a' && c != 'A')
				{
					print_flt(cb, arg, fval);
				}
				else
				{
					print_bytes(cb, arg, &fval, sizeof(float));
				}
				/* code */
				break;
			case '%':
				cb(arg, '%');
				break;
			default:
				cb(arg, '%');
				cb(arg, c);
				break;
			}
		}
	} while (c);
	va_end(args);
}