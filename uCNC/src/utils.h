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
#include <string.h>
#include <math.h>

#ifndef BYTE_OPS
#define BYTE_OPS
#define SETBIT(x, y) ((x) |= (1U << (y)))		 /* Set bit y in byte x*/
#define CLEARBIT(x, y) ((x) &= ~(1U << (y))) /* Clear bit y in byte x*/
#define CHECKBIT(x, y) ((x) & (1U << (y)))	 /* Check bit y in byte x*/
#define TOGGLEBIT(x, y) ((x) ^= (1U << (y))) /* Toggle bit y in byte x*/

#define SETFLAG(x, y) ((x) |= (y))		/* Set byte y in byte x*/
#define CLEARFLAG(x, y) ((x) &= ~(y)) /* Clear byte y in byte x*/
#define CHECKFLAG(x, y) ((x) & (y))		/* Check byte y in byte x*/
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

#define fast_flt_div2(x)    \
	({                        \
		flt_t res;              \
		res.f = (x);            \
		if (res.i & 0x7f800000) \
			res.i -= 0x00800000;  \
		else                    \
			res.i = 0;            \
		res.f;                  \
	})
#define fast_flt_div4(x)    \
	({                        \
		flt_t res;              \
		res.f = (x);            \
		if (res.i & 0x7f000000) \
			res.i -= 0x01000000;  \
		else                    \
			res.i = 0;            \
		res.f;                  \
	})
#define fast_flt_mul2(x)                    \
	({                                        \
		flt_t res;                              \
		res.f = (x);                            \
		if ((res.i & 0x7f800000) != 0x7f800000) \
			res.i += 0x00800000;                  \
		res.f;                                  \
	})
#define fast_flt_mul4(x)                    \
	({                                        \
		flt_t res;                              \
		res.f = (x);                            \
		if ((res.i & 0x7f000000) != 0x7f000000) \
			res.i += 0x01000000;                  \
		res.f;                                  \
	})
// Quake III based fast sqrt calculation
// fast_flt_sqrt takes about 19 clock cycles on AVR instead of +/-482 if using normal sqrt (x25 faster). The error of this shortcut should be under 4~5%.
#define fast_flt_sqrt(x)                   \
	({                                       \
		flt_t res;                             \
		res.f = (x);                           \
		if (res.i)                             \
			res.i = ((res.i >> 1) - 0xe041a9fb); \
		res.f;                                 \
	})
// fast_flt_invsqrt takes about 18 clock cycles on AVR instead of +/-960 if using normal 1/sqrt (x53 faster). The error of this shortcut should be under 4~5%.
#define fast_flt_invsqrt(x)              \
	({                                     \
		flt_t res;                           \
		res.f = (x);                         \
		res.i = (0x5f3759df - (res.i >> 1)); \
		res.f;                               \
	})
// fast_flt_pow2 takes about 25 clock cycles on AVR instead of 144 if using normal pow or muliply by itself (x~5.5 faster). The error of this shortcut should be under 4~5%.
#define fast_flt_pow2(x)                   \
	({                                       \
		flt_t res;                             \
		res.f = ABS((x));                      \
		if (res.f != 0)                        \
		{                                      \
			res.i = ((res.i << 1) + 0xc0858106); \
			if (res.i < 0)                       \
				res.i = 0;                         \
		}                                      \
		res.f;                                 \
	})

#define fast_flt_inv(x)           \
	({                              \
		flt_t res;                    \
		res.f = (x);                  \
		res.i = (0x7EF0624D - res.i); \
		res.f;                        \
	})

#define fast_flt_div(y, x)        \
	({                              \
		flt_t res;                    \
		res.f = (x);                  \
		res.i = (0x7EF0624D - res.i); \
		(y) * res.f;                  \
	})
// mul10 takes about 26 clock cycles on AVR instead of 77 on 32bit integer multiply by 10 (x~3 faster). Can be customized for each MCU
#ifndef fast_int_mul10
#define fast_int_mul10(x) ((((x) << 2) + (x)) << 1)
#endif
#else
#define fast_flt_div2(x) ((x) * 0.5f)
#define fast_flt_div4(x) ((x) * 0.25f)
#define fast_flt_mul2(x) ((x) * 2.0f)
#define fast_flt_mul4(x) ((x) * 4.0f)
#define fast_flt_sqrt(x) (sqrtf(x))
#define fast_flt_invsqrt(x) (1.0f / sqrtf(x))
#define fast_flt_pow2(x) ((x) * (x))
#define fast_flt_inv(x) (1.0f / (x))
#define fast_flt_div(y, x) ((y) / (x))
#ifndef fast_int_mul10
#define fast_int_mul10(x) (x * 10)
#endif
#endif

#define DEG_RAD_MULT 0.0174532925199432958f
#define RAD_DEG_MULT 57.295779513082320877f
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
#ifndef M_LN2
#define M_LN2 0.693147180559945309417f
#endif

// some math.h libraries do not define log of base 2
#ifndef LN
#define LN(x) log(x) / M_LN2
#endif

#define __TIMEOUT_US__(timeout) for (int32_t elap_us = 0, curr_us = mcu_free_micros(); timeout > 0; ({elap_us = mcu_free_micros(); int32_t tmp = elap_us-curr_us; curr_us = elap_us; timeout -= ((tmp >= 0) ? tmp : 1000 + tmp); }))
#define __TIMEOUT_MS__(timeout)                                                          \
	timeout = (((uint32_t)timeout) < (INT32_MAX / 1000)) ? (timeout * 1000) : INT32_MAX; \
	__TIMEOUT_US__(timeout)
#define __TIMEOUT_ASSERT__(timeout) if (timeout == 0)

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

#ifndef __ATOMIC__
#define __ATOMIC__ for (uint8_t __restore_atomic__ __attribute__((__cleanup__(__atomic_out))) = mcu_get_global_isr(), __AtomLock = __atomic_in(); __AtomLock; __AtomLock = 0)
#endif
#ifndef __ATOMIC_FORCEON__
#define __ATOMIC_FORCEON__ for (uint8_t __restore_atomic__ __attribute__((__cleanup__(__atomic_out_on))) = 1, __AtomLock = __atomic_in(); __AtomLock; __AtomLock = 0)
#endif

#ifndef DECL_MUTEX
#define MUTEX_CLEANUP(name)                    \
	static void name##_mutex_cleanup(uint8_t *m) \
	{                                            \
		if (!*m /*can unlock*/)                    \
		{                                          \
			name##_mutex_lock = 0;                   \
		}                                          \
	}
#define DECL_MUTEX(name)                     \
	static volatile uint8_t name##_mutex_lock; \
	MUTEX_CLEANUP(name)
#define MUTEX_INIT(name) uint8_t __attribute__((__cleanup__(name##_mutex_cleanup))) name##_mutex_temp = 0
#define MUTEX_RELEASE(name)                \
	if (!name##_mutex_temp /*has the lock*/) \
	name##_mutex_lock = 0
#define MUTEX_TAKE(name)                   \
	__ATOMIC__                               \
	{                                        \
		name##_mutex_temp = name##_mutex_lock; \
		if (!name##_mutex_temp)                \
		{                                      \
			name##_mutex_lock = 1;               \
		}                                      \
	}                                        \
	if (!name##_mutex_temp /*the lock was aquired*/)

#define MUTEX_WAIT(name, timeout_ms)         \
	__TIMEOUT_MS__(timeout_us)                 \
	{                                          \
		__ATOMIC__                               \
		{                                        \
			name##_mutex_temp = name##_mutex_lock; \
			if (!name##_mutex_temp)                \
			{                                      \
				name##_mutex_lock = 1;               \
				break;                               \
			}                                      \
		}                                        \
	}                                          \
	if (!name##_mutex_temp && timeout_us != 0 /*the lock was aquired in time*/)
#endif

#define __STRGIFY__(s) #s
#define STRGIFY(s) __STRGIFY__(s)

	/**
	 * RING BUFFER UTILS
	 * **/

	typedef struct ring_buffer_
	{
		volatile uint8_t count;
		volatile uint8_t head;
		volatile uint8_t tail;
		uint8_t *data;
		const uint8_t size;
		const uint8_t elem_size;
	} ring_buffer_t;

#ifndef USE_MACRO_BUFFER
#ifndef USE_CUSTOM_BUFFER_IMPLEMENTATION
#define DECL_BUFFER(type, name, size)  \
	static type name##_bufferdata[size]; \
	ring_buffer_t name = {0, 0, 0, name##_bufferdata, size, sizeof(type)}

	uint8_t buffer_write_available(ring_buffer_t *buffer);
	uint8_t buffer_read_available(ring_buffer_t *buffer);
	bool buffer_empty(ring_buffer_t *buffer);
	bool buffer_full(ring_buffer_t *buffer);
	void buffer_peek(ring_buffer_t *buffer, void *ptr);
	void buffer_dequeue(ring_buffer_t *buffer, void *ptr);
	void buffer_enqueue(ring_buffer_t *buffer, void *ptr);
	void buffer_write(ring_buffer_t *buffer, void *ptr, uint8_t len, uint8_t *written);
	void buffer_read(ring_buffer_t *buffer, void *ptr, uint8_t len, uint8_t *read);
	void buffer_clear(ring_buffer_t *buffer);

#define BUFFER_INIT(type, buffer, size)
#define BUFFER_WRITE_AVAILABLE(buffer) buffer_write_available(&buffer)
#define BUFFER_READ_AVAILABLE(buffer) buffer_read_available(&buffer)
#define BUFFER_EMPTY(buffer) buffer_empty(&buffer)
#define BUFFER_FULL(buffer) buffer_full(&buffer)
#define BUFFER_PEEK(buffer, ptr) buffer_peek(&buffer, ptr)
#define BUFFER_DEQUEUE(buffer, ptr) buffer_dequeue(&buffer, ptr)
#define BUFFER_ENQUEUE(buffer, ptr) buffer_enqueue(&buffer, ptr)
#define BUFFER_WRITE(buffer, ptr, len, written) buffer_write(&buffer, ptr, len, &written)
#define BUFFER_READ(buffer, ptr, len, read) buffer_read(&buffer, ptr, len, &read)
#define BUFFER_CLEAR(buffer) buffer_clear(&buffer)
#endif
#else
#define DECL_BUFFER(type, name, size)      \
	static type name##_bufferdata[size];     \
	static const uint8_t name##_size = size; \
	ring_buffer_t name

#define BUFFER_INIT(type, buffer, size)
#define BUFFER_WRITE_AVAILABLE(buffer) (buffer##_size - buffer.count)
#define BUFFER_READ_AVAILABLE(buffer) (buffer.count)
#define BUFFER_EMPTY(buffer) (!buffer.count)
#define BUFFER_FULL(buffer) (buffer.count == buffer##_size)
#define BUFFER_PEEK(buffer) (buffer##_bufferdata[buffer.tail])

#define BUFFER_DEQUEUE(buffer, ptr)                                            \
	{                                                                            \
		if (!BUFFER_EMPTY(buffer))                                                 \
		{                                                                          \
			uint8_t tail;                                                            \
			__ATOMIC__                                                               \
			{                                                                        \
				tail = buffer.tail;                                                    \
			}                                                                        \
			memcpy(ptr, &buffer##_bufferdata[tail], sizeof(buffer##_bufferdata[0])); \
			tail++;                                                                  \
			if (tail >= buffer##_size)                                               \
			{                                                                        \
				tail = 0;                                                              \
			}                                                                        \
			__ATOMIC__                                                               \
			{                                                                        \
				buffer.tail = tail;                                                    \
				buffer.count--;                                                        \
			}                                                                        \
		}                                                                          \
	}

#define BUFFER_ENQUEUE(buffer, ptr)                                            \
	{                                                                            \
		if (!BUFFER_FULL(buffer))                                                  \
		{                                                                          \
			uint8_t head;                                                            \
			__ATOMIC__                                                               \
			{                                                                        \
				head = buffer.head;                                                    \
			}                                                                        \
			memcpy(&buffer##_bufferdata[head], ptr, sizeof(buffer##_bufferdata[0])); \
			head++;                                                                  \
			if (head >= buffer##_size)                                               \
			{                                                                        \
				head = 0;                                                              \
			}                                                                        \
			__ATOMIC__                                                               \
			{                                                                        \
				buffer.head = head;                                                    \
				buffer.count++;                                                        \
			}                                                                        \
		}                                                                          \
	}

#define BUFFER_WRITE(buffer, ptr, len, written) ({                                             \
	uint8_t count, head;                                                                         \
	__ATOMIC__                                                                                   \
	{                                                                                            \
		head = buffer.head;                                                                        \
		count = buffer.count;                                                                      \
	}                                                                                            \
	count = MIN(buffer##_size - count, len);                                                     \
	written = 0;                                                                                 \
	if (count)                                                                                   \
	{                                                                                            \
		uint8_t avail = (buffer##_size - head);                                                    \
		if (avail < count && avail)                                                                \
		{                                                                                          \
			memcpy(&buffer##_bufferdata[head], ptr, avail * sizeof(buffer##_bufferdata[0]));         \
			written = avail;                                                                         \
			count -= avail;                                                                          \
			head = 0;                                                                                \
		}                                                                                          \
		else                                                                                       \
		{                                                                                          \
			avail = 0;                                                                               \
		}                                                                                          \
		if (count)                                                                                 \
		{                                                                                          \
			memcpy(&buffer##_bufferdata[head], &ptr[avail], count * sizeof(buffer##_bufferdata[0])); \
			written += count;                                                                        \
			__ATOMIC__                                                                               \
			{                                                                                        \
				head += count;                                                                         \
				if (head == buffer##_size)                                                             \
				{                                                                                      \
					head = 0;                                                                            \
				}                                                                                      \
				buffer.head = head;                                                                    \
				buffer.count += written;                                                               \
			}                                                                                        \
		}                                                                                          \
	}                                                                                            \
})

#define BUFFER_READ(buffer, ptr, len, read) ({                                                 \
	uint8_t count, tail;                                                                         \
	__ATOMIC__                                                                                   \
	{                                                                                            \
		tail = buffer.tail;                                                                        \
		count = buffer.count;                                                                      \
	}                                                                                            \
	if (count > len)                                                                             \
	{                                                                                            \
		count = len;                                                                               \
	}                                                                                            \
	read = 0;                                                                                    \
	if (count)                                                                                   \
	{                                                                                            \
		uint8_t avail = buffer##_size - tail;                                                      \
		if (avail < count && avail)                                                                \
		{                                                                                          \
			memcpy(ptr, &buffer##_bufferdata[tail], avail * sizeof(buffer##_bufferdata[0]));         \
			read = avail;                                                                            \
			count -= avail;                                                                          \
			tail = 0;                                                                                \
		}                                                                                          \
		else                                                                                       \
		{                                                                                          \
			avail = 0;                                                                               \
		}                                                                                          \
		if (count)                                                                                 \
		{                                                                                          \
			memcpy(&ptr[avail], &buffer##_bufferdata[tail], count * sizeof(buffer##_bufferdata[0])); \
			read += count;                                                                           \
			__ATOMIC__                                                                               \
			{                                                                                        \
				tail += count;                                                                         \
				if (tail == buffer##_size)                                                             \
				{                                                                                      \
					tail = 0;                                                                            \
				}                                                                                      \
				buffer.tail = tail;                                                                    \
				buffer.count -= read;                                                                  \
			}                                                                                        \
		}                                                                                          \
	}                                                                                            \
})

#define BUFFER_CLEAR(buffer)          \
	{                                   \
			__ATOMIC__{                     \
					buffer##_bufferdata[0] = 0; \
	buffer.tail = 0;                    \
	buffer.head = 0;                    \
	buffer.count = 0;                   \
	}                                   \
	}
#endif

#if defined(__GNUC__) && __GNUC__ >= 7
#define __FALL_THROUGH__ __attribute__((fallthrough));
#else
#define __FALL_THROUGH__
#endif /* __GNUC__ >= 7 */

#define TOUPPER(c) ((c >= 'a' && c <= 'z') ? (c - 32) : c)

#ifdef __cplusplus
}
#endif

#endif
