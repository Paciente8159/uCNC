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

#ifdef FORCEINLINE
#undef FORCEINLINE
#endif
#define FORCEINLINE __attribute__((always_inline, gnu_inline)) inline

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
	extern const uint8_t __rom__ crc7_table[256];
#define crc7(x, y) rom_read_byte(&crc7_table[x ^ y])
#else
uint8_t crc7(uint8_t c, uint8_t crc);
#endif

/**
 * RING BUFFER UTILS
 * **/
#define DECL_BUFFER_TYPE(T, SIZE) \
	typedef struct T##_buffer_    \
	{                             \
		volatile uint8_t count;   \
		volatile uint8_t head;    \
		uint8_t tail;             \
		T data[SIZE];             \
	} T##_buffer_t

#define DECL_BUFFER(T, NAME, SIZE)           \
	DECL_BUFFER_TYPE(T, SIZE);               \
	static const uint8_t NAME##_size = SIZE; \
	T##_buffer_t NAME

#define DECL_STATIC_BUFFER(T, NAME, SIZE)    \
	DECL_BUFFER_TYPE(T, SIZE);               \
	static const uint8_t NAME##_size = SIZE; \
	static T##_buffer_t NAME

#define BUFFER_EMPTY(buffer) (!buffer.count)
#define BUFFER_FULL(buffer) (buffer.count == buffer##_size)
#define BUFFER_PEEK(buffer) (&buffer.data[buffer.tail])
#define BUFFER_READ(buffer)                   \
	{                                         \
		uint8_t i = buffer.tail++;            \
		if (!BUFFER_EMPTY(buffer))            \
		{                                     \
			buffer.tail++;                    \
			if (buffer.tail >= buffer##_size) \
			{                                 \
				buffer.tail = 0;              \
			}                                 \
			buffer.count--;                   \
		}                                     \
		&buffer.data[i];                      \
	}

#define BUFFER_POP(buffer, ptr)                                                 \
	{                                                                           \
		if (!BUFFER_EMPTY(buffer))                                              \
		{                                                                       \
			if (ptr != NULL)                                                    \
			{                                                                   \
				memcpy(ptr, &buffer.data[buffer.tail], sizeof(buffer.data[0])); \
			}                                                                   \
			buffer.tail++;                                                      \
			if (buffer.tail >= buffer##_size)                                   \
			{                                                                   \
				buffer.tail = 0;                                                \
			}                                                                   \
			buffer.count--;                                                     \
		}                                                                       \
	}
#define BUFFER_WRITE(buffer)             \
	{                                    \
		if (!BUFFER_FULL(buffer))        \
		{                                \
			uint8_t r = buffer.head + 1; \
			if (r >= buffer##_size)      \
			{                            \
				r = 0;                   \
			}                            \
			buffer.head = r;             \
			buffer.count++;              \
		}                                \
	}

#define BUFFER_PUSH(buffer, ptr)                                      \
	{                                                                 \
		if (!BUFFER_FULL(buffer))                                     \
		{                                                             \
			uint8_t r = buffer.head + 1;                              \
			if (r >= buffer##_size)                                   \
			{                                                         \
				r = 0;                                                \
			}                                                         \
			buffer.head = r;                                          \
			buffer.count++;                                           \
			if (ptr != NULL)                                          \
			{                                                         \
				memcpy(ptr, &buffer.data[r], sizeof(buffer.data[0])); \
			}                                                         \
		}                                                             \
	}
#define BUFFER_NEXT_SLOT(buffer) (&buffer.data[buffer.head])

#ifdef __cplusplus
}
#endif

#endif
