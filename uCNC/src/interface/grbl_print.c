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
#include "grbl_print.h"
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

#ifndef prtf_getc
#define prtf_getc(fmt) rom_read_byte(fmt)
#endif

static inline unsigned char str_read_romchar(const char *buffer)
{
	return rom_read_byte(buffer);
}

#ifndef PRINT_FTM_MINIMAL
static __attribute__((noinline)) void prt_memc(void *out, char c)
{
	char **mem = ((char **)out);
	**mem = c;
	*mem += 1;
}
#endif

static size_t prt_putc(void *out, size_t maxlen, char c)
{
	if (maxlen == PRINT_CALLBACK)
	{
		((prt_putc_cb)out)(c);
		return PRINT_CALLBACK;
	}
#ifndef PRINT_FTM_MINIMAL
	if (maxlen)
	{
		prt_memc(out, c);
		return --maxlen;
	}
#endif
	return 0;
}

#ifndef PRINT_FTM_MINIMAL
size_t prt_byte(void *out, size_t maxlen, const uint8_t *data, uint8_t flags)
{
	bool prefix = (flags && HEX_PREFIX);
	char hexchar = (flags & HEX_UPPER) ? 'A' : 'a';
	uint8_t size = HEX_SIZE(flags);
	if (prefix)
	{
		maxlen = prt_putc(out, maxlen, '0');
		maxlen = prt_putc(out, maxlen, 'x');
	}
	for (uint8_t i = size; i !=0;)
	{
		i--;
		uint8_t up = data[i] >> 4;
		uint8_t c = (up > 9) ? (hexchar + up - 10) : ('0' + up);
		maxlen = prt_putc(out, maxlen, c);
		up = data[i] & 0x0F;
		c = (up > 9) ? (hexchar + up - 10) : ('0' + up);
		maxlen = prt_putc(out, maxlen, c);
	}

	return maxlen;
}
#endif

size_t prt_int(void *out, size_t maxlen, uint32_t num, uint8_t padding)
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
		maxlen = prt_putc(out, maxlen, '0');
	}

	while (i--)
	{
		maxlen = prt_putc(out, maxlen, '0' + buffer[i]);
	}

	return maxlen;
}

size_t prt_flt(void *out, size_t maxlen, float num, uint8_t precision)
{
#ifndef PRINT_FTM_MINIMAL
	if (num == INFINITY)
	{
		maxlen = prt_putc(out, maxlen, 'I');
		maxlen = prt_putc(out, maxlen, 'n');
		maxlen = prt_putc(out, maxlen, 'f');
		return maxlen;
	}

	if (num == NAN)
	{
		maxlen = prt_putc(out, maxlen, 'N');
		maxlen = prt_putc(out, maxlen, 'a');
		maxlen = prt_putc(out, maxlen, 'N');
		return maxlen;
	}
#endif

	if (num < 0)
	{
		maxlen = prt_putc(out, maxlen, '-');
		num = -num;
	}

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

	maxlen = prt_int(out, maxlen, interger, 0);
	maxlen = prt_putc(out, maxlen, '.');
	maxlen = prt_int(out, maxlen, digits, precision);

	return maxlen;
}

#ifndef PRINT_FTM_MINIMAL
size_t prt_ip(void *out, size_t maxlen, uint32_t ip)
{
	uint8_t *ptr = (uint8_t *)&ip;
	maxlen = prt_int(out, maxlen, (int32_t)ptr[3], 0);
	maxlen = prt_putc(out, maxlen, '.');
	maxlen = prt_int(out, maxlen, (int32_t)ptr[2], 0);
	maxlen = prt_putc(out, maxlen, '.');
	maxlen = prt_int(out, maxlen, (int32_t)ptr[1], 0);
	maxlen = prt_putc(out, maxlen, '.');
	maxlen = prt_int(out, maxlen, (int32_t)ptr[0], 0);

	return maxlen;
}
#endif

size_t prt_fmtva(void *out, size_t maxlen, const char *fmt, va_list *args)
{
	char *ptr = (char *)out;
	char **memref;
	if (maxlen != PRINT_CALLBACK)
	{
		memref = &ptr;
		out = memref;
	}

	char c = 0;

	do
	{
		char cval = 0;
		uint8_t lcount = 2;
#ifndef PRINT_FTM_MINIMAL
		const char *s;
		uint8_t hexflags = HEX_NONE;
		void *pt = NULL;
#endif
		int32_t li = 0;
		float f, *f_ptr = NULL;
		uint8_t elems = 0;
#ifndef PRINTF_FTM_CUSTOM_PRECISION
		uint8_t precision = (!g_settings.report_inches) ? 3 : 5;
#else
		uint8_t precision = PRINTF_FTM_CUSTOM_PRECISION;
#endif

		c = prtf_getc(fmt++);
		if (c == '%')
		{
			c = prtf_getc(fmt++);
			switch (c)
			{
#ifndef PRINT_FTM_MINIMAL
			case '#':
				hexflags |= HEX_PREFIX;
				c = prtf_getc(fmt++);
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
					cval = prt_atof((void *)str_read_romchar, (const char **)&fmt, &f);
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
					c = prtf_getc(fmt++);
				}
				break;
			}

			// ll is the same as l and hh is the same as h
			while (c == 'l' || c == 'h')
			{
				lcount = ((c == 'l') ? 4 : 1);
				c = prtf_getc(fmt++);
			}

			switch (c)
			{
			case 'c':
				cval = (char)va_arg(*args, int);
				maxlen = prt_putc(out, maxlen, cval);
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
					maxlen = prt_putc(out, maxlen, cval);
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
					pt = &li;
					elems = 1;
				}
				do
				{
					switch (c)
					{
					case 'x':
					case 'X':
#ifndef PRINT_FTM_MINIMAL
						maxlen = prt_byte(out, maxlen, (const uint8_t *)pt, (hexflags | lcount));
#else
						maxlen = prt_byte(out, maxlen, (const uint8_t *)&li, (hexflags | lcount));
#endif
						break;
					case 'I':
						maxlen = prt_ip(out, maxlen, li);
						break;
					default:
#endif
						if (cval && (li < 0))
						{
							maxlen = prt_putc(out, maxlen, '-');
							li = -li;
						}
						li &= (0xffffffff >> ((4 - lcount) << 3));
						maxlen = prt_int(out, maxlen, (uint32_t)li, 0);
#ifndef PRINT_FTM_MINIMAL
						break;
					}
					if (elems && --elems)
					{
						maxlen = prt_putc(out, maxlen, ',');
					}
					pt += (1 << (lcount - 1));
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
						maxlen = prt_byte(out, maxlen, (const uint8_t *)f_ptr, (hexflags | VAR_DWORD));
						break;
					default:
						maxlen = prt_flt(out, maxlen, *f_ptr++, precision);
						break;
					}
#else
					maxlen = prt_flt(out, maxlen, *f_ptr++, precision);
#endif
					if (elems && --elems)
					{
						maxlen = prt_putc(out, maxlen, ',');
					}
				} while (elems);
				/* code */
				break;
			case '%':
				maxlen = prt_putc(out, maxlen, '%');
				break;
			default:
				maxlen = prt_putc(out, maxlen, '%');
				maxlen = prt_putc(out, maxlen, c);
				break;
			}
		}
		else if (c)
		{
			maxlen = prt_putc(out, maxlen, c);
		}
	} while (c);

	return maxlen;
}

size_t prt_fmt(void *out, size_t maxlen, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	maxlen = prt_fmtva(out, maxlen, fmt, &args);
	va_end(args);
	return maxlen;
}

#define atof_peek(cb, buffer) ((!buffer) ? ((prt_getc_cb)cb)(true) : ((cb) ? ((prt_read_rom_byte)cb)(*buffer) : **buffer))
#define atof_get(cb, buffer) ((!buffer) ? ((prt_getc_cb)cb)(false) : ({ *buffer += 1; 0; }))

uint8_t prt_atof(void *cb, const char **buffer, float *value)
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
