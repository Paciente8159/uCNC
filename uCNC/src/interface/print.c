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

#define HEX_NONE 0
#define HEX_PREFIX 128
#define HEX_UPPER 64
#define VAR_BYTE 1
#define VAR_WORD 2
#define VAR_DWORD 4
#define HEX_SIZE_MASK (VAR_DWORD | VAR_WORD | VAR_BYTE)
#define HEX_SIZE(X) ((X & HEX_SIZE_MASK) >> 1)

#ifndef printf_getc
#define printf_getc(fmt) pgm_read_byte(fmt)
#endif

void print_putc(print_putc_cb cb, char **buffer_ref, char c)
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

void print_str(print_putc_cb cb, char **buffer_ref, const char *__s)
{
	while (*__s)
	{
		print_putc(cb, buffer_ref, *__s++);
	}
}

void print_romstr(print_putc_cb cb, char **buffer_ref, const char *__s)
{
	char c = pgm_read_byte(__s);
	while (c)
	{
		print_putc(cb, buffer_ref, c);
		__s++;
		c = pgm_read_byte(__s);
	}
}

void print_byte(print_putc_cb cb, char **buffer_ref, const uint8_t *data, uint8_t flags)
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

void print_bytes(print_putc_cb cb, char **buffer_ref, const uint8_t *data, uint8_t count, uint8_t flags)
{
	print_byte(cb, buffer_ref, data, flags);
	while (--count)
	{
		print_putc(cb, buffer_ref, ' ');
		print_byte(cb, buffer_ref, data, flags);
	}
	while (--count)
		;
}

void print_int(print_putc_cb cb, char **buffer_ref, int32_t num)
{
	if (num == 0)
	{
		print_putc(cb, buffer_ref, '0');
		return;
	}

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

	do
	{
		i--;
		print_putc(cb, buffer_ref, '0' + buffer[i]);
	} while (i);
}

void print_flt(print_putc_cb cb, char **buffer_ref, float num)
{
	if (num < 0)
	{
		print_putc(cb, buffer_ref, '-');
		num = -num;
	}

	int32_t interger = (uint32_t)floorf(num);
	num -= interger;
	uint32_t mult = (!g_settings.report_inches) ? 1000 : 10000;
	num *= mult;
	uint32_t digits = (uint32_t)lroundf(num);
	if (digits == mult)
	{
		interger++;
		digits = 0;
	}

	print_int(cb, buffer_ref, interger);
	print_putc(cb, buffer_ref, '.');
	if (g_settings.report_inches)
	{
		if (digits < 1000)
		{
			print_putc(cb, buffer_ref, '0');
		}
	}

	if (digits < 100)
	{
		print_putc(cb, buffer_ref, '0');
	}

	if (digits < 10)
	{
		print_putc(cb, buffer_ref, '0');
	}

	print_int(cb, buffer_ref, digits);
}

void print_ip(print_putc_cb cb, char **buffer_ref, uint32_t ip)
{
	uint8_t *ptr = (uint8_t *)&ip;
	print_int(cb, buffer_ref, (int32_t)ptr[3]);
	print_putc(cb, buffer_ref, '.');
	print_int(cb, buffer_ref, (int32_t)ptr[2]);
	print_putc(cb, buffer_ref, '.');
	print_int(cb, buffer_ref, (int32_t)ptr[1]);
	print_putc(cb, buffer_ref, '.');
	print_int(cb, buffer_ref, (int32_t)ptr[0]);
}

void print_fmtva(print_putc_cb cb, char *buffer, const char *fmt, va_list *args)
{
	char c = 0, cval = 0, *s;
	uint8_t lcount = 0;
	bool hexflags = HEX_NONE;
	int32_t val = 0;
	float fval = 0;
	char **buffer_ref = NULL;
	char *ptr = buffer;
	if (ptr)
	{
		buffer_ref = &ptr;
	}

	do
	{
		c = printf_getc(fmt);
		fmt++;
		if (c == '%')
		{
			c = printf_getc(fmt);
			fmt++;
			switch (c)
			{
			case '#':
				hexflags = HEX_PREFIX;
				__FALL_THROUGH__
			case '-':
			case '+':
			case '0':
				while (c >= '0' && c <= '9' && c)
				{
					c = printf_getc(fmt);
					fmt++;
				}
				__FALL_THROUGH__
			case '.':
				if (c == '.')
				{
					do
					{
						c = printf_getc(fmt);
						fmt++;
					} while (c >= '0' && c <= '9' && c);
				}
				break;
			}

			while (c == 'l' || c == 'h')
			{
				if (c == 'l')
				{
					lcount++;
				}
				c = printf_getc(fmt);
				fmt++;
			}

			while (c == 'l' || c == 'h')
			{
				if (c == 'l')
				{
					lcount++;
				}
				c = printf_getc(fmt);
				fmt++;
			}

			switch (c)
			{
			case 'c':
				cval = (float)va_arg(*args, char);
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
			case 'd':
			case 'i':
				switch (lcount)
				{
				case 0:
					val = (uint32_t)va_arg(*args, int8_t);
					break;
				case 1:
					val = (uint32_t)va_arg(*args, int16_t);
					break;
				default:
					val = (uint32_t)va_arg(*args, int32_t);
					break;
				}
				print_int(cb, buffer_ref, val);
				/* code */
				break;
			case 'M':
				lcount = 4;
				__FALL_THROUGH__
			case 'X':
				hexflags |= HEX_UPPER;
				__FALL_THROUGH__
			case 'u':
			case 'x':
				switch (lcount)
				{
				case 0:
					val = (uint32_t)va_arg(*args, uint8_t);
					lcount = VAR_BYTE;
					break;
				case 1:
					val = (uint32_t)va_arg(*args, uint16_t);
					lcount = VAR_WORD;
					break;
				default:
					val = (uint32_t)va_arg(*args, uint32_t);
					lcount = VAR_DWORD;
					break;
				}
				if (c == 'u')
				{
					print_int(cb, buffer_ref, val);
				}
				else if (c == 'M')
				{
					print_ip(cb, buffer_ref, val);
				}
				else
				{
					print_byte(cb, buffer_ref, (const uint8_t *)&val, (hexflags | lcount));
				}
				/* code */
				break;
			case 'A':
				hexflags |= HEX_UPPER;
				__FALL_THROUGH__
			case 'f':
			case 'F':
			case 'e':
			case 'E':
			case 'a':
			case 'g':
			case 'G':
				fval = (float)va_arg(*args, float);
				if (c != 'a' && c != 'A')
				{
					print_flt(cb, buffer_ref, fval);
				}
				else
				{
					print_byte(cb, buffer_ref, (const uint8_t *)&fval, (hexflags | VAR_DWORD));
				}
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
	} while (c);
}

void print_fmt(print_putc_cb cb, char *buffer, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	print_fmtva(cb, buffer, fmt, &args);
	va_end(args);
}