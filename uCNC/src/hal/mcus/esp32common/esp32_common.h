/*
	Name: mcu32_common.h
	Description: Common MCU implementations for ESP32 MCUS in µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 11-09-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef ESP32_COMMON_H
#define ESP32_COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>

#ifdef ESP32
	extern volatile uint32_t i2s_mode;
#define I2S_MODE __atomic_load_n((uint32_t *)&i2s_mode, __ATOMIC_RELAXED)

	void mcu_uart_start(void);
	void mcu_uart_dotasks(void);
	void mcu_uart2_start(void);
	void mcu_uart2_dotasks(void);
	void mcu_eeprom_init(int size);

	void mcu_usb_dotasks(void);
	void mcu_wifi_init(void);
	void mcu_wifi_dotasks(void);
	void mcu_bt_dotasks(void);
#ifdef IC74HC595_CUSTOM_SHIFT_IO
	extern volatile uint32_t ic74hc595_i2s_pins;
	void mcu_i2s_extender_init(void);
	void i2s_write_word(uint32_t value);
#endif
	void mcu_gen_pwm(void);
	void mcu_gen_servo(void);
	void mcu_gen_step(void);
#if defined(MCU_HAS_ONESHOT_TIMER)
	void mcu_gen_oneshot(void);
#endif
#endif

	void esp32_modules_init(void);
#define cnc_modules_dotasks() \
	RUNONCE                   \
	{                         \
		RUNONCE_COMPLETE();   \
		esp32_modules_init(); \
	}

// atomic operations
#define mcu_in_isr_context() xPortInIsrContext()
#define TASK_YIELD()          \
	if (!xPortInIsrContext()) \
	vPortYield()

#define __FREERTOS_BIN_SEMPH_TAKE__(mutex, timeout) ((xPortInIsrContext()) ? (xSemaphoreTakeFromISR(mutex, NULL)) : (xSemaphoreTake(mutex, timeout)))
#define __FREERTOS_BIN_SEMPH_GIVE__(mutex) ((xPortInIsrContext()) ? (xSemaphoreGiveFromISR(mutex, NULL)) : (xSemaphoreGive(mutex)))

#define DECL_MUTEX(name) SemaphoreHandle_t name##_semph_lock = NULL
#define ATOMIC_TYPE SemaphoreHandle_t
#define BIN_SEMPH_UNDEF (NULL)
#define BIN_SEMPH_LOCKED true
#define BIN_SEMPH_UNLOCKED false

#define BIN_SEMPH_INIT(name, locked)                        \
	if (name##_semph_lock == BIN_SEMPH_UNDEF)               \
	{                                                       \
		name##_semph_lock = xSemaphoreCreateBinary();       \
		if ((locked))                                       \
		{                                                   \
			__FREERTOS_BIN_SEMPH_GIVE__(name##_semph_lock); \
		}                                                   \
	}

#define BIN_SEMPH_UNLOCK(name)                          \
	if (name##_semph_lock)                              \
	{                                                   \
		__FREERTOS_BIN_SEMPH_GIVE__(name##_semph_lock); \
	}

#define BIN_SEMPH_TIMEDLOCK(name, timeout_ms) __FREERTOS_BIN_SEMPH_TAKE__(name##_semph_lock, timeout_ms)
#define BIN_SEMPH_LOCK(name) BIN_SEMPH_TIMEDLOCK(name, portMAX_DELAY)
#define BIN_SEMPH_TRYLOCK(name) BIN_SEMPH_TIMEDLOCK(name, 0)

#ifndef FORCEINLINE
#define FORCEINLINE __attribute__((always_inline)) inline
#endif

	/**
	 * Atomic operations
	 */
#define ATOMIC_LOAD_N(src, mode) __atomic_load_n((src), mode)
#define ATOMIC_STORE_N(dst, val, mode) __atomic_store_n((dst), (val), mode)
#define ATOMIC_COMPARE_EXCHANGE_N(dst, cmp, des, sucmode, failmode) __atomic_compare_exchange_n((dst), (cmp), (des), false, sucmode, failmode)
#define ATOMIC_FETCH_OR(dst, val, mode) __atomic_fetch_or((dst), (val), mode)
#define ATOMIC_FETCH_AND(dst, val, mode) __atomic_fetch_and((dst), (val), mode)
#define ATOMIC_FETCH_ADD(dst, val, mode) __atomic_fetch_add((dst), (val), mode)
#define ATOMIC_FETCH_SUB(dst, val, mode) __atomic_fetch_sub((dst), (val), mode)
#define ATOMIC_FETCH_XOR(dst, val, mode) __atomic_fetch_xor((dst), (val), mode)
#define ATOMIC_SPIN()         \
	if (xPortInIsrContext())  \
	{                         \
		portYIELD_FROM_ISR(); \
	}                         \
	else                      \
	{                         \
		portYIELD();          \
	}

#ifdef __cplusplus
}
#endif
#endif