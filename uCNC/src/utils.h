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
#include <math.h>

#ifndef BYTE_OPS
#define BYTE_OPS
#define SETBIT(x, y) ((x) |= (1U << (y)))	 /* Set bit y in byte x*/
#define CLEARBIT(x, y) ((x) &= ~(1U << (y))) /* Clear bit y in byte x*/
#define CHECKBIT(x, y) ((x) & (1U << (y)))	 /* Check bit y in byte x*/
#define TOGGLEBIT(x, y) ((x) ^= (1U << (y))) /* Toggle bit y in byte x*/

#define SETFLAG(x, y) ((x) |= (y))	  /* Set byte y in byte x*/
#define CLEARFLAG(x, y) ((x) &= ~(y)) /* Clear byte y in byte x*/
#define CHECKFLAG(x, y) ((x) & (y))	  /* Check byte y in byte x*/
#define TOGGLEFLAG(x, y) ((x) ^= (y)) /* Toggle byte y in byte x*/
#endif

#ifndef MAX
#define MAX(a, b) (((a) >= (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) (((a) <= (b)) ? (a) : (b))
#endif
#ifndef CLAMP
#define CLAMP(a, b, c) (MIN((c), MAX((a), (b))))
#endif
#ifndef ABS
#define ABS(a) (((a) >= 0) ? (a) : -(a))
#endif

#ifndef CLAMP
#define CLAMP(a, b, c) (MIN((c), MAX((a), (b))))
#endif

#if (defined(ENABLE_FAST_MATH) && __SIZEOF_FLOAT__ == 4)
	// performs direct float manipulation and bit shifting. At the very end spectrum of the float (to infinity and to 0) makes aproximation either to 0 or infinity
	// div2 takes about 13 clock cycles on AVR instead of 144 if multiply by 0.5f (x11 faster)
	// div4 takes about 9 clock cycles on AVR instead of 144 if multiply by 0.25f (x16 faster)
	// mul2 takes about 11 clock cycles on AVR instead of 144 if multiply by 0.25f (x13 faster)
	// mul4 takes about 7 clock cycles on AVR instead of 144 if multiply by 0.25f (x20 faster)
	typedef union
	{
		float f;
		int32_t i;
	} flt_t;

#define fast_flt_div2(x)         \
	({                           \
		flt_t res;               \
		res.f = (x);             \
		if (res.i & 0x7f800000)  \
			res.i -= 0x00800000; \
		else                     \
			res.i = 0;           \
		res.f;                   \
	})
#define fast_flt_div4(x)         \
	({                           \
		flt_t res;               \
		res.f = (x);             \
		if (res.i & 0x7f000000)  \
			res.i -= 0x01000000; \
		else                     \
			res.i = 0;           \
		res.f;                   \
	})
#define fast_flt_mul2(x)                        \
	({                                          \
		flt_t res;                              \
		res.f = (x);                            \
		if ((res.i & 0x7f800000) != 0x7f800000) \
			res.i += 0x00800000;                \
		res.f;                                  \
	})
#define fast_flt_mul4(x)                        \
	({                                          \
		flt_t res;                              \
		res.f = (x);                            \
		if ((res.i & 0x7f000000) != 0x7f000000) \
			res.i += 0x01000000;                \
		res.f;                                  \
	})
// Quake III based fast sqrt calculation
// fast_flt_sqrt takes about 19 clock cycles on AVR instead of +/-482 if using normal sqrt (x25 faster). The error of this shortcut should be under 4~5%.
#define fast_flt_sqrt(x)                         \
	({                                           \
		flt_t res;                               \
		res.f = (x);                             \
		if (res.i)                               \
			res.i = (0x1fbeecc0 + (res.i >> 1)); \
		res.f;                                   \
	})
// fast_flt_invsqrt takes about 18 clock cycles on AVR instead of +/-960 if using normal 1/sqrt (x53 faster). The error of this shortcut should be under 4~5%.
#define fast_flt_invsqrt(x)                  \
	({                                       \
		flt_t res;                           \
		res.f = (x);                         \
		res.i = (0x5f3759df - (res.i >> 1)); \
		res.f;                               \
	})
// fast_flt_pow2 takes about 25 clock cycles on AVR instead of 144 if using normal pow or muliply by itself (x~5.5 faster). The error of this shortcut should be under 4~5%.
#define fast_flt_pow2(x)                         \
	({                                           \
		flt_t res;                               \
		res.f = ABS((x));                        \
		if (res.f != 0)                          \
		{                                        \
			res.i = ((res.i << 1) - 0x3f7adaba); \
			if (res.i < 0)                       \
				res.i = 0;                       \
			res.f;                               \
		}                                        \
		0;                                       \
	})
// mul10 takes about 26 clock cycles on AVR instead of 77 on 32bit integer multiply by 10 (x~3 faster). Can be customized for each MCU
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

#define DEG_RAD_MULT 0.0174532925199432958f
#define MM_INCH_MULT 0.0393700787401574803f
#define INCH_MM_MULT 25.4f
#define MIN_SEC_MULT 0.0166666666666666667f
#define UINT8_MAX_INV 0.0039215686274509804f
#ifndef LOG2
#define LOG2 0.3010299956639811952f
#endif
#ifndef M_PI
#define M_PI 3.1415926535897932385f
#endif
#ifndef M_PI_INV
#define M_PI_INV 0.3183098861837906715f
#endif
#ifndef M_COS_TAYLOR_1
#define M_COS_TAYLOR_1 0.1666666666666666667f
#endif

#ifndef FORCEINLINE
#define FORCEINLINE __attribute__((always_inline)) inline
#endif

	static FORCEINLINE uint8_t __atomic_in(void)
	{
		mcu_disable_global_isr();
		return 1;
	}

	static FORCEINLINE void __atomic_out(uint8_t *s)
	{
		if (*s != 0)
		{
			mcu_enable_global_isr();
		}
	}

	static FORCEINLINE void __atomic_out_on(uint8_t *s)
	{
		mcu_enable_global_isr();
	}

#define __ATOMIC__ for (uint8_t __restore_atomic__ __attribute__((__cleanup__(__atomic_out))) = mcu_get_global_isr(), __AtomLock = __atomic_in(); __AtomLock; __AtomLock = 0)
#define __ATOMIC_FORCEON__ for (uint8_t __restore_atomic__ __attribute__((__cleanup__(__atomic_out_on))) = 1, __AtomLock = __atomic_in(); __AtomLock; __AtomLock = 0)

#define __STRGIFY__(s) #s
#define STRGIFY(s) __STRGIFY__(s)

#ifndef CRC_WITHOUT_LOOKUP_TABLE
const uint8_t __rom__ crc7_table[256] =
	{
		0x00, 0x09, 0x12, 0x1b, 0x24, 0x2d, 0x36, 0x3f,
		0x48, 0x41, 0x5a, 0x53, 0x6c, 0x65, 0x7e, 0x77,
		0x19, 0x10, 0x0b, 0x02, 0x3d, 0x34, 0x2f, 0x26,
		0x51, 0x58, 0x43, 0x4a, 0x75, 0x7c, 0x67, 0x6e,
		0x32, 0x3b, 0x20, 0x29, 0x16, 0x1f, 0x04, 0x0d,
		0x7a, 0x73, 0x68, 0x61, 0x5e, 0x57, 0x4c, 0x45,
		0x2b, 0x22, 0x39, 0x30, 0x0f, 0x06, 0x1d, 0x14,
		0x63, 0x6a, 0x71, 0x78, 0x47, 0x4e, 0x55, 0x5c,
		0x64, 0x6d, 0x76, 0x7f, 0x40, 0x49, 0x52, 0x5b,
		0x2c, 0x25, 0x3e, 0x37, 0x08, 0x01, 0x1a, 0x13,
		0x7d, 0x74, 0x6f, 0x66, 0x59, 0x50, 0x4b, 0x42,
		0x35, 0x3c, 0x27, 0x2e, 0x11, 0x18, 0x03, 0x0a,
		0x56, 0x5f, 0x44, 0x4d, 0x72, 0x7b, 0x60, 0x69,
		0x1e, 0x17, 0x0c, 0x05, 0x3a, 0x33, 0x28, 0x21,
		0x4f, 0x46, 0x5d, 0x54, 0x6b, 0x62, 0x79, 0x70,
		0x07, 0x0e, 0x15, 0x1c, 0x23, 0x2a, 0x31, 0x38,
		0x41, 0x48, 0x53, 0x5a, 0x65, 0x6c, 0x77, 0x7e,
		0x09, 0x00, 0x1b, 0x12, 0x2d, 0x24, 0x3f, 0x36,
		0x58, 0x51, 0x4a, 0x43, 0x7c, 0x75, 0x6e, 0x67,
		0x10, 0x19, 0x02, 0x0b, 0x34, 0x3d, 0x26, 0x2f,
		0x73, 0x7a, 0x61, 0x68, 0x57, 0x5e, 0x45, 0x4c,
		0x3b, 0x32, 0x29, 0x20, 0x1f, 0x16, 0x0d, 0x04,
		0x6a, 0x63, 0x78, 0x71, 0x4e, 0x47, 0x5c, 0x55,
		0x22, 0x2b, 0x30, 0x39, 0x06, 0x0f, 0x14, 0x1d,
		0x25, 0x2c, 0x37, 0x3e, 0x01, 0x08, 0x13, 0x1a,
		0x6d, 0x64, 0x7f, 0x76, 0x49, 0x40, 0x5b, 0x52,
		0x3c, 0x35, 0x2e, 0x27, 0x18, 0x11, 0x0a, 0x03,
		0x74, 0x7d, 0x66, 0x6f, 0x50, 0x59, 0x42, 0x4b,
		0x17, 0x1e, 0x05, 0x0c, 0x33, 0x3a, 0x21, 0x28,
		0x5f, 0x56, 0x4d, 0x44, 0x7b, 0x72, 0x69, 0x60,
		0x0e, 0x07, 0x1c, 0x15, 0x2a, 0x23, 0x38, 0x31,
		0x46, 0x4f, 0x54, 0x5d, 0x62, 0x6b, 0x70, 0x79};

#define crc7(x, y) rom_read_byte(&crc7_table[x ^ y])
#else
uint8_t crc7(uint8_t c, uint8_t crc);
#endif

#ifdef __cplusplus
}
#endif

#endif
