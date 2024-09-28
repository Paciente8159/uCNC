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
#define HEX_SIZE(X) (X & HEX_SIZE_MASK)

#ifdef PRINTF_FTM_CUSTOM_PRECISION
#if ((PRINTF_FTM_CUSTOM_PRECISION < 0) || (PRINTF_FTM_CUSTOM_PRECISION > 9))
#error "Invalid custom precison value. Must be a value between 0 and 9"
#endif
#endif

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

#ifndef PRINT_FTM_MINIMAL
void print_byte(print_putc_cb cb, char **buffer_ref, const uint8_t *data, uint8_t flags)
{
	bool prefix = (flags && HEX_PREFIX);
	char hexchar = (flags & HEX_UPPER) ? 'A' : 'a';
	uint8_t size = HEX_SIZE(flags);
	if (prefix)
	{
		print_putc(cb, buffer_ref, '0');
		print_putc(cb, buffer_ref, 'x');
	}
	for (uint8_t i = 0; i < size; i++)
	{
		uint8_t up = data[i] >> 4;
		uint8_t c = (up > 9) ? (hexchar + up - 10) : ('0' + up);
		print_putc(cb, buffer_ref, c);
		up = data[i] & 0x0F;
		c = (up > 9) ? (hexchar + up - 10) : ('0' + up);
		print_putc(cb, buffer_ref, c);
	}
}
#endif

void print_int(print_putc_cb cb, char **buffer_ref, uint32_t num, uint8_t padding)
{
	uint8_t buffer[11];
	uint8_t i = 0;

	if (num == 0)
	{
		padding = MAX(1, padding);
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

	while (i--)
	{
		print_putc(cb, buffer_ref, '0' + buffer[i]);
	}
}

void print_flt(print_putc_cb cb, char **buffer_ref, float num, uint8_t precision)
{
	if (num < 0)
	{
		print_putc(cb, buffer_ref, '-');
		num = -num;
	}

#ifndef PRINT_FTM_MINIMAL
	if (num == INFINITY)
	{
		print_putc(cb, buffer_ref, 'I');
		print_putc(cb, buffer_ref, 'n');
		print_putc(cb, buffer_ref, 'f');
	}

	if (num == NAN)
	{
		print_putc(cb, buffer_ref, 'N');
		print_putc(cb, buffer_ref, 'a');
		print_putc(cb, buffer_ref, 'N');
	}
#endif

	uint32_t interger = floorf(num);
	num -= interger;
#ifndef PRINT_FTM_MINIMAL
	uint32_t mult = pow(10, precision);
#else
	uint32_t mult = (!g_settings.report_inches) ? 1000 : 10000;
#endif
	num *= mult;
	uint32_t digits = (uint32_t)lroundf(num);
	if (digits == mult)
	{
		interger++;
		digits = 0;
	}

	print_int(cb, buffer_ref, interger, 0);
	print_putc(cb, buffer_ref, '.');
	print_int(cb, buffer_ref, digits, precision);
}

#ifndef PRINT_FTM_MINIMAL
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

void print_fmtva(print_putc_cb cb, char *buffer, const char *fmt, va_list *args)
{
	char c = 0, cval = 0;
	uint8_t lcount = 2;
#ifndef PRINT_FTM_MINIMAL
	const char *s;
	bool hexflags = HEX_NONE;
	void *pt = NULL;
#endif
	int32_t li = 0;
	float f, *f_ptr = NULL;

	char *ptr = buffer;
	char **buffer_ref = (!ptr) ? NULL : &ptr;
	uint8_t elems = 0;
#ifndef PRINTF_FTM_CUSTOM_PRECISION
	uint8_t precision = (!g_settings.report_inches) ? 3 : 5;
#else
	uint8_t precision = PRINTF_FTM_CUSTOM_PRECISION;
#endif

	do
	{
		c = printf_getc(fmt++);
		if (c == '%')
		{
			c = printf_getc(fmt++);
			switch (c)
			{
#ifndef PRINT_FTM_MINIMAL
			case '#':
				hexflags = HEX_PREFIX;
				__FALL_THROUGH__
			case '0':
				// ignores zero padding
				__FALL_THROUGH__
			case '.':
				__FALL_THROUGH__
#endif
			default:
				if (c == '.' || (c >= '1' && c <= '9'))
				{
					fmt--;
					cval = print_atof(NULL, (const char **)&fmt, &f);
					if (cval != ATOF_NUMBER_UNDEF)
					{
						elems = (uint8_t)f;
#ifndef PRINT_FTM_MINIMAL
						if (cval & ATOF_NUMBER_ISFLOAT)
						{
							precision = (roundf((f - elems) * 10));
						}
#endif
					}
					c = printf_getc(fmt++);
				}
				break;
			}

			// ll is the same as l and hh is the same as h
			while (c == 'l' || c == 'h')
			{
				lcount = ((c == 'l') ? 4 : 1);
				c = printf_getc(fmt++);
			}

			switch (c)
			{
			case 'c':
				cval = (char)va_arg(*args, int);
				print_putc(cb, buffer_ref, cval);
				/* code */
				break;
#ifndef PRINT_FTM_MINIMAL
			case 's':
			case 'S':
				s = (const char *)va_arg(*args, const char *);
				for (;;)
				{
					cval = (c == 's') ? *s++ : rom_read_byte(s++);
					if (!cval)
					{
						break;
					}
					print_putc(cb, buffer_ref, cval);
				}
				break;
			case 'I':
				lcount = 4;
				__FALL_THROUGH__
			case 'X':
				hexflags |= HEX_UPPER;
				__FALL_THROUGH__
			case 'x':
#endif
			case 'd':
			case 'i':
				cval = 1;
				__FALL_THROUGH__
			case 'u':
#ifndef PRINT_FTM_MINIMAL
				if (elems)
				{
					pt = va_arg(*args, void *);
				}
				else
				{
#endif
					switch (lcount)
					{
					case 4:
						// mask |= 0xffff0000;
						li = va_arg(*args, int32_t);
						break;
					default:
						li = va_arg(*args, int);
						break;
					}
#ifndef PRINT_FTM_MINIMAL
					elems = 1;
				}
				do
				{
					switch (c)
					{
					case 'x':
					case 'X':
						print_byte(cb, buffer_ref, (const uint8_t *)&li, (hexflags | lcount));
						break;
					case 'M':
						print_ip(cb, buffer_ref, li);
						break;
					default:
#endif
						if (cval && (li < 0))
						{
							print_putc(cb, buffer_ref, '-');
							li = -li;
						}
						li &= (0xffffffff >> ((4 - lcount) << 3));
						print_int(cb, buffer_ref, (uint32_t)li, 0);
#ifndef PRINT_FTM_MINIMAL
						break;
					}
					if (elems && --elems)
					{
						print_putc(cb, buffer_ref, ',');
					}
					pt += (1 << lcount);
				} while (elems);
#endif
				/* code */
				break;
#ifndef PRINT_FTM_MINIMAL
			case 'A':
				hexflags |= HEX_UPPER;
				__FALL_THROUGH__
			case 'a':
			case 'e':
			case 'E':
			case 'g':
			case 'G':
#endif
			case 'f':
			case 'F':
				if (elems)
				{
					f_ptr = va_arg(*args, float *);
				}
				else
				{
					f = (float)va_arg(*args, double);
					f_ptr = &f;
					elems = 1;
				}
				do
				{
#ifndef PRINT_FTM_MINIMAL
					switch (c)
					{

					case 'a':
					case 'A':
						print_byte(cb, buffer_ref, (const uint8_t *)f_ptr, (hexflags | VAR_DWORD));
						break;
					default:
						print_flt(cb, buffer_ref, *f_ptr++, precision);
						break;
					}
#else
					print_flt(cb, buffer_ref, *f_ptr++, precision);
#endif
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
		else if (c)
		{
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
		else if (c == (uint8_t)('.' - 48) && !(result & ATOF_NUMBER_ISFLOAT))
		{
			result |= ATOF_NUMBER_ISFLOAT;
		}
		else if (result & ATOF_NUMBER_OK)
		{
			rhs = (float)intval;
			while (fpcount--)
			{
				rhs *= 0.1f;
			}

			*value = (result & ATOF_NUMBER_ISNEGATIVE) ? -rhs : rhs;
			return result;
		}
		else
		{
			return ATOF_NUMBER_UNDEF;
		}

		atof_get(cb, buffer);
		c = (uint8_t)atof_peek(cb, buffer);
	}

	return ATOF_NUMBER_UNDEF;
}
