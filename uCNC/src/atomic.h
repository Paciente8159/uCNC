/*
	Name: atomic.h
	Description: Some useful macros for thread syncronzation and atomic operations.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 16/12/2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef ATOMIC_H
#define ATOMIC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define MEM_BARRIER ({ asm volatile("" ::: "memory"); })

#ifndef FORCEINLINE
#define FORCEINLINE __attribute__((always_inline)) inline
#endif
	static FORCEINLINE bool __atomic_in()
	{
		mcu_disable_global_isr();
		return true;
	}

	static FORCEINLINE void __atomic_out(bool *s)
	{
		if (!!(*s))
		{
			mcu_enable_global_isr();
		}
		else
		{
			mcu_disable_global_isr();
		}
		MEM_BARRIER;
	}

	static FORCEINLINE void __atomic_out_on(bool *s)
	{
		mcu_enable_global_isr();
		MEM_BARRIER;
		(void)s;
	}

#ifndef ATOMIC_CODEBLOCK
#define ATOMIC_CODEBLOCK for (bool __restore_atomic__ __attribute__((__cleanup__(__atomic_out))) = mcu_get_global_isr(), __AtomLock = __atomic_in(); __AtomLock; __AtomLock = false)
#endif
#ifndef ATOMIC_CODEBLOCK_NR
#define ATOMIC_CODEBLOCK_NR for (bool __restore_atomic__ __attribute__((__cleanup__(__atomic_out_on))) = mcu_get_global_isr(), __AtomLock = __atomic_in(); __AtomLock; __AtomLock = false)
#endif

	/**
	 * ATOMIC operation utilities
	 */
	// if supported use these instead
	// #define ATOMIC_LOAD(T, ptr) __atomic_load_n((ptr), __ATOMIC_ACQUIRE)
	// #define ATOMIC_STORE_N(ptr, val) __atomic_store_n((ptr), (val), __ATOMIC_RELEASE)
	// #define ATOMIC_COMPARE_EXCHANGE_N(ptr, expected, desired) __atomic_compare_exchange_n((ptr), &(expected), (desired), false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)
	// #define ATOMIC_FETCH_ADD(T, ptr, val) __atomic_fetch_add((ptr), (val), __ATOMIC_ACQ_REL)
	// #define ATOMIC_SUB(ptr, val) __atomic_fetch_sub((ptr), (val), __ATOMIC_ACQ_REL)
#ifdef __GNUC__
#ifndef ATOMIC_LOAD_N
#define ATOMIC_LOAD_N(src, mode) \
	({__typeof__(*(src)) _res;ATOMIC_CODEBLOCK { _res = *(src); }_res; })
#endif
#ifndef ATOMIC_STORE_N
#define ATOMIC_STORE_N(dst, val, mode) \
	ATOMIC_CODEBLOCK { *(dst) = (val); }
#endif
	// fetch, compare and modify operations
#ifndef ATOMIC_COMPARE_EXCHANGE_N
#define ATOMIC_COMPARE_EXCHANGE_N(dst, cmp, des, sucmode, failmode) ({bool __res = false; ATOMIC_CODEBLOCK{__res = (*(dst)==*(cmp)); if(__res){*(dst) = (des);}else{*(cmp) = *(dst);}} __res; })
#endif
// fetch and modify operations
#ifndef ATOMIC_FETCH_XOR
#define ATOMIC_FETCH_XOR(dst, val, mode) ({__typeof__(*(dst)) __tmp; ATOMIC_CODEBLOCK{__tmp = *(dst); MEM_BARRIER; *(dst) = __tmp ^ (val);} __tmp; })
#endif
#ifndef ATOMIC_FETCH_ADD
#define ATOMIC_FETCH_ADD(dst, val, mode) ({__typeof__(*(dst)) __tmp; ATOMIC_CODEBLOCK{__tmp = *(dst); MEM_BARRIER; *(dst) = __tmp + (val);} __tmp; })
#endif
#ifndef ATOMIC_FETCH_SUB
#define ATOMIC_FETCH_SUB(dst, val, mode) ({__typeof__(*(dst)) __tmp; ATOMIC_CODEBLOCK{__tmp = *(dst); MEM_BARRIER; *(dst) = __tmp - (val);} __tmp; })
#endif
#ifndef ATOMIC_FETCH_AND
#define ATOMIC_FETCH_AND(dst, val, mode) ({__typeof__(*(dst)) __tmp; ATOMIC_CODEBLOCK{__tmp = *(dst); MEM_BARRIER; *(dst) = __tmp & (val);} __tmp; })
#endif
#ifndef ATOMIC_FETCH_OR
#define ATOMIC_FETCH_OR(dst, val, mode) ({__typeof__(*(dst)) __tmp; ATOMIC_CODEBLOCK{__tmp = *(dst); MEM_BARRIER; *(dst) = __tmp | (val);} __tmp; })
#endif
#else
#warning "ATOMIC macros only support type buffer_index_t"
#ifndef ATOMIC_LOAD_N
#define ATOMIC_LOAD_N(src, mode) \
	({buffer_index_t _res;ATOMIC_CODEBLOCK { _res = *(src); }_res; })
#endif
#ifndef ATOMIC_STORE_N
#define ATOMIC_STORE_N(dst, val, mode) \
	ATOMIC_CODEBLOCK { *(dst) = (val); }
#endif
// fetch, compare and modify operations
#ifndef ATOMIC_COMPARE_EXCHANGE_N
#define ATOMIC_COMPARE_EXCHANGE_N(dst, cmp, des, sucmode, failmode) ({bool __res; ATOMIC_CODEBLOCK{__res = (*(dst)==*(cmp)); if(__res){*(dst) = (des);}else{*(cmp) = *(dst);}} __res; })
#endif
// fetch and modify operations
#ifndef ATOMIC_FETCH_XOR
#define ATOMIC_FETCH_XOR(dst, val, mode) ({buffer_index_t __tmp; ATOMIC_CODEBLOCK{__tmp = *(dst); MEM_BARRIER; *(dst) = __tmp ^ (val);} __tmp; })
#endif
#ifndef ATOMIC_FETCH_ADD
#define ATOMIC_FETCH_ADD(dst, val, mode) ({buffer_index_t __tmp; ATOMIC_CODEBLOCK{__tmp = *(dst); MEM_BARRIER; *(dst) = __tmp + (val);} __tmp; })
#endif
#ifndef ATOMIC_FETCH_SUB
#define ATOMIC_FETCH_SUB(dst, val, mode) ({buffer_index_t __tmp; ATOMIC_CODEBLOCK{__tmp = *(dst); MEM_BARRIER; *(dst) = __tmp - (val);} __tmp; })
#endif
#ifndef ATOMIC_FETCH_AND
#define ATOMIC_FETCH_AND(dst, val, mode) ({buffer_index_t __tmp; ATOMIC_CODEBLOCK{__tmp = *(dst); MEM_BARRIER; *(dst) = __tmp & (val);} __tmp; })
#endif
#ifndef ATOMIC_FETCH_OR
#define ATOMIC_FETCH_OR(dst, val, mode) ({buffer_index_t __tmp; ATOMIC_CODEBLOCK{__tmp = *(dst); MEM_BARRIER; *(dst) = __tmp | (val);} __tmp; })
#endif
#endif

#ifndef ATOMIC_SPIN
#define ATOMIC_SPIN() mcu_nop()
#endif

#ifndef TASK_YIELD
#ifndef mcu_in_isr_context
	extern bool mcu_in_isr_context(void);
#endif
	extern bool cnc_dotasks(void);
#define TASK_YIELD()           \
	if (!mcu_in_isr_context()) \
	cnc_dotasks()
#endif

#define BUFFER_GUARD_INIT(s)
#define BUFFER_GUARD

#ifndef DECL_MUTEX
#define BIN_SEMPH_UNDEF ((uint8_t)-1)
#define BIN_SEMPH_LOCKED 1
#define BIN_SEMPH_UNLOCKED 0

#ifndef ATOMIC_TYPE
#define ATOMIC_TYPE volatile uint8_t
#endif

// mutex
#define DECL_MUTEX(name) volatile ATOMIC_TYPE name = BIN_SEMPH_UNDEF
#define BIN_SEMPH_INIT(name, locked) ({int8_t name##_semph_temp = BIN_SEMPH_UNDEF; ATOMIC_COMPARE_EXCHANGE_N(&name, &name##_semph_temp, (locked), __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE); })
#define BIN_SEMPH_UNLOCK(name) ({int8_t name##_semph_temp = BIN_SEMPH_LOCKED; ATOMIC_COMPARE_EXCHANGE_N(&name, &name##_semph_temp, BIN_SEMPH_UNLOCKED, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE); })
	static FORCEINLINE bool semph_safe_lock(ATOMIC_TYPE *lock, uint32_t timeout)
	{
		// converts to us
		if (timeout > (UINT32_MAX / 1000))
		{
			timeout *= 1000;
		}

		uint32_t now = mcu_free_micros();
		for (;;)
		{
			ATOMIC_TYPE expected = BIN_SEMPH_UNLOCKED;
			if (ATOMIC_COMPARE_EXCHANGE_N(lock, &expected, BIN_SEMPH_LOCKED, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
			{
				return true;
			}
			if (!timeout)
			{
				break;
			}
			TASK_YIELD();
			uint32_t tstamp = mcu_free_micros();
			uint32_t elapsed = (tstamp > now) ? (tstamp - now) : (1000 - now + tstamp);
			if (elapsed >= timeout)
				break;
			timeout -= elapsed;
			now = tstamp;
		}

		return false;
	}
#define BIN_SEMPH_TIMEDLOCK(name, timeout_ms) semph_safe_lock(&name, timeout_ms)
#define BIN_SEMPH_LOCK(name) BIN_SEMPH_TIMEDLOCK(name, 0xFFFFFFFF)
#define BIN_SEMPH_TRYLOCK(name) BIN_SEMPH_TIMEDLOCK(name, 0)
#endif

#ifdef __cplusplus
}
#endif
#endif
