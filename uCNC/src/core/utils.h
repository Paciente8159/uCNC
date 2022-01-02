/*
	Name: utils.h
	Description: Some useful constants and macros.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 19/09/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

#ifndef BYTE_OPS
#define SETBIT(x, y) ((x) |= (1 << (y)))	/* Set bit y in byte x*/
#define CLEARBIT(x, y) ((x) &= ~(1 << (y))) /* Clear bit y in byte x*/
#define CHECKBIT(x, y) ((x) & (1 << (y)))	/* Check bit y in byte x*/
#define TOGGLEBIT(x, y) ((x) ^= (1 << (y))) /* Toggle bit y in byte x*/

#define SETFLAG(x, y) ((x) |= (y))	  /* Set byte y in byte x*/
#define CLEARFLAG(x, y) ((x) &= ~(y)) /* Clear byte y in byte x*/
#define CHECKFLAG(x, y) ((x) & (y))	  /* Check byte y in byte x*/
#define TOGGLEFLAG(x, y) ((x) ^= (y)) /* Toggle byte y in byte x*/
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef ABS
#define ABS(a) (((a) > 0) ? (a) : -(a))
#endif

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define __UINT32_R0__ 0
#define __UINT32_R1__ 1
#define __UINT32_R2__ 2
#define __UINT32_R3__ 3
#else
#define __UINT32_R0__ 3
#define __UINT32_R1__ 2
#define __UINT32_R2__ 1
#define __UINT32_R3__ 0
#endif

#if (defined(ENABLE_FAST_MATH) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ && __SIZEOF_FLOAT__ == 4)
	typedef union
	{
		float f;
		int32_t i;
		struct
		{
			uint16_t w0;
			int16_t w1;
		};
		struct
		{
			uint8_t b0;
			uint8_t b1;
			uint8_t b2;
			int8_t b3;
		};
		struct
		{
			int32_t mant : 23;
			int32_t expn : 8;
			int32_t sign : 1;
		};
	} flt_t;
//performs direct float manipulation and bit shifting. At the very end spectrum of the float (to infinity and to 0) makes aproximation either to 0 or infinity
//div2 takes about 13 clock cycles on AVR instead of 144 if multiply by 0.5f (x11 faster)
//div4 takes about 9 clock cycles on AVR instead of 144 if multiply by 0.25f (x16 faster)
//mul2 takes about 11 clock cycles on AVR instead of 144 if multiply by 0.25f (x13 faster)
//mul4 takes about 7 clock cycles on AVR instead of 144 if multiply by 0.25f (x20 faster)
#define fast_flt_div2(x) (       \
	{                            \
		flt_t res;               \
		res.f = (x);             \
		if (res.w1 & 0x7f80)     \
			res.i -= 0x00800000; \
		else                     \
			res.i = 0;           \
		res.f;                   \
	})
#define fast_flt_div4(x) (       \
	{                            \
		flt_t res;               \
		res.f = (x);             \
		if (res.b3 & 0x7f)       \
			res.i -= 0x01000000; \
		else                     \
			res.i = 0;           \
		res.f;                   \
	})
#define fast_flt_mul2(x) (               \
	{                                    \
		flt_t res;                       \
		res.f = (x);                     \
		if ((res.w1 & 0x7f80) != 0x7f80) \
			res.i += 0x00800000;         \
		res.f;                           \
	})
#define fast_flt_mul4(x) (           \
	{                                \
		flt_t res;                   \
		res.f = (x);                 \
		if ((res.b3 & 0x7f) != 0x7f) \
			res.i += 0x01000000;     \
		res.f;                       \
	})
//Quake III based fast sqrt calculation
//fast_flt_sqrt takes about 19 clock cycles on AVR instead of +/-482 if using normal sqrt (x25 faster). The error of this shortcut should be under 4~5%.
#define fast_flt_sqrt(x) (                       \
	{                                            \
		flt_t res;                               \
		res.f = (x);                             \
		if (res.i)                               \
			res.i = (0x1fbeecc0 + (res.i >> 1)); \
		res.f;                                   \
	})
//fast_flt_invsqrt takes about 18 clock cycles on AVR instead of +/-960 if using normal 1/sqrt (x53 faster). The error of this shortcut should be under 4~5%.
#define fast_flt_invsqrt(x) (                \
	{                                        \
		flt_t res;                           \
		res.f = (x);                         \
		res.i = (0x5f3759df - (res.i >> 1)); \
		res.f;                               \
	})
//fast_flt_pow2 takes about 25 clock cycles on AVR instead of 144 if using normal pow or muliply by itself (x~5.5 faster). The error of this shortcut should be under 4~5%.
#define fast_flt_pow2(x) (                       \
	{                                            \
		flt_t res;                               \
		res.f = (x);                             \
		if (!(res.b3 & 0x20))                    \
			res.i = ((res.i << 1) - 0x3f7adaba); \
		else                                     \
			res.f = INFINITY;                    \
		((!res.sign) ? res.f : 0);               \
	})
//mul10 takes about 26 clock cycles on AVR instead of 77 on 32bit integer multiply by 10 (x~3 faster). Can be customized for each MCU
#ifndef fast_int_mul10
#define fast_int_mul10(x) ((((x) << 2) + (x)) << 1)
#endif
#else
#define fast_flt_div2(x) ((x)*0.5f)
#define fast_flt_div4(x) ((x)*0.25f)
#define fast_flt_mul2(x) ((x)*2.0f)
#define fast_flt_mul4(x) ((x)*4.0f)
#define fast_flt_sqrt(x) (sqrtf(x))
#define fast_flt_invsqrt(x) (1.0f / sqrtf(x))
#define fast_flt_pow2(x) (x * x)
#ifndef fast_int_mul10
#define fast_int_mul10(x) (x * 10)
#endif
#endif

#define MM_INCH_MULT 0.0393700787401574803f
#define INCH_MM_MULT 25.4f
#define MIN_SEC_MULT 0.0166666666666666667f
#define UINT8_MAX_INV 0.0039215686274509804f
#ifndef LOG2
#define LOG2 0.3010299956639811952f
#endif

#define FORCEINLINE __attribute__((always_inline)) inline

	static FORCEINLINE uint8_t __atomic_in(void)
	{
		mcu_disable_global_isr();
		return 1;
	}

	static FORCEINLINE uint8_t __atomic_out(bool s)
	{
		if (s)
		{
			mcu_enable_global_isr();
		}
	}

#define __ATOMIC__ for (bool __restore_atomic__ __attribute__((__cleanup__(__atomic_out))) = mcu_get_global_isr(), __AtomLock = __atomic_in(); __AtomLock; __AtomLock = 0)

#ifdef __cplusplus
}
#endif

#endif
