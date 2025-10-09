/*
	Name: esp32_i2s.c
	Description: Implements the µCNC custom ESP32 I2S IO Shifter.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 15-09-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

#if (ESP32)
#include "soc/i2s_struct.h"
#include "soc/soc.h"
#include "soc/interrupts.h"
#include "driver/i2s.h"
#include "soc/i2s_reg.h"
#include "driver/i2s.h"
#include "../esp32common/esp32_common.h"

#ifdef IC74HC595_CUSTOM_SHIFT_IO
#if IC74HC595_COUNT != 4
#error "IC74HC595_COUNT must be 4(bytes) to use ESP32 I2S mode for IO shifting"
#endif

#ifndef I2S_REG
#define I2S_REG __helper__(I2S, I2S_PORT, )
#endif
#ifndef I2S_PERIF
#define I2S_PERIF __helper__(PERIPH_I2S, I2S_PORT, _MODULE)
#endif
#ifndef I2S_INTR_SRC
#define I2S_INTR_SRC __helper__(ETS_I2S, I2S_PORT, _INTR_SOURCE)
#endif

#ifndef I2S_SAMPLE_RATE
#define I2S_SAMPLE_RATE (500000)
#endif

#define I2S_REFILL_CHUNK_WORDS 32
#define I2S_FIFO_WORDS 64

volatile uint32_t ic74hc595_i2s_pins;
volatile uint32_t i2s_mode;

MCU_CALLBACK void mcu_itp_isr(void *arg);
MCU_CALLBACK void mcu_gen_pwm_and_servo(void);
MCU_CALLBACK void mcu_gen_step(void);
MCU_CALLBACK void mcu_gpio_isr(void *type);

// software generated oneshot for RT steps like laser PPI
#if defined(MCU_HAS_ONESHOT_TIMER) && defined(ENABLE_RT_SYNC_MOTIONS)
static uint32_t esp32_oneshot_counter;
static uint32_t esp32_oneshot_reload;
static FORCEINLINE void mcu_gen_oneshot(void)
{
	if (esp32_oneshot_counter)
	{
		esp32_oneshot_counter--;
		if (!esp32_oneshot_counter)
		{
			if (mcu_timeout_cb)
			{
				mcu_timeout_cb();
			}
		}
	}
}
#endif

static inline void IRAM_ATTR i2s_write_single_word(uint32_t sample)
{
	REG_WRITE(I2S_CONF_SIGLE_DATA_REG(I2S_PORT), sample);
}

/**
 * Try to fill the buffer (saturation)
 */
static void IRAM_ATTR i2s_fifo_fill_words(void)
{
	for (int i = 0; i < I2S_FIFO_WORDS; ++i)
	{
		// Check FIFO full flag
		if (I2S_REG.int_st.tx_wfull)
		{
			break; // stop writing to prevent overflow
		}
#if defined(IC74HC595_HAS_STEPS) || defined(IC74HC595_HAS_DIRS)
		mcu_gen_step();
#endif
#if defined(IC74HC595_HAS_PWMS) || defined(IC74HC595_HAS_SERVOS)
		mcu_gen_pwm_and_servo();
#endif
#if defined(MCU_HAS_ONESHOT_TIMER) && defined(ENABLE_RT_SYNC_MOTIONS)
		mcu_gen_oneshot();
#endif
		// WRITE_PERI_REG(I2S_FIFO_WR_REG(I2S_PORT), __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED));
		WRITE_PERI_REG(I2S_OUTFIFO_PUSH_REG(I2S_PORT), __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED));
	}
}

/**
 * The I2S FIFO ISR
 * This should keep the buffer saturated on default mode
 * If running at 500KHz it should kick retain about 128us worth of motion
 * ISR interrupt is designed to enter when the buffer is about half way
 * A 64us latency should be managable even with latency introduced by FreeRTOS
 */
static void IRAM_ATTR i2s_tx_isr(void *arg)
{
	uint32_t st = I2S_REG.int_st.val;

	// Default mode: periodic refill
	if ((i2s_mode & ~ITP_STEP_MODE_SYNC) == ITP_STEP_MODE_DEFAULT)
	{
		if (st & I2S_TX_HUNG_INT_ST)
		{
			i2s_fifo_fill_words();
			I2S_REG.int_clr.tx_hung = 1;
		}
	}

	// Realtime transition: when draining, detect FIFO empty and flip
	if ((i2s_mode & ~ITP_STEP_MODE_SYNC) == ITP_STEP_MODE_REALTIME)
	{
		if (st & I2S_TX_REMPTY_INT_ST)
		{
			I2S_REG.int_clr.tx_rempty = 1;
			I2S_REG.conf_single_data = __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED);
			I2S_REG.int_ena.tx_rempty = 0; // stop further empty interrupts
		}
	}
}

static intr_handle_t i2s_intr_handle = NULL;

static void attach_i2s_isr(void)
{
	if (!i2s_intr_handle)
	{
		esp_intr_alloc(I2S_INTR_SRC, ESP_INTR_FLAG_IRAM, i2s_tx_isr, NULL, &i2s_intr_handle);
	}
}

static void IRAM_ATTR i2s_tx_base_config(void)
{
	I2S_REG.conf.tx_start = 0;
	I2S_REG.conf.tx_reset = 1;
	I2S_REG.conf.tx_reset = 0;
	I2S_REG.conf.tx_fifo_reset = 1;
	I2S_REG.conf.tx_fifo_reset = 0;

	// Disable DMA
	I2S_REG.out_link.stop = 1;
	I2S_REG.fifo_conf.dscr_en = 0;

	// Mono 32-bit stream
	I2S_REG.fifo_conf.tx_fifo_mod = 3;
	I2S_REG.conf_chan.tx_chan_mod = 3;
	I2S_REG.conf.tx_msb_shift = 0;
	I2S_REG.conf.rx_msb_shift = 0;
	I2S_REG.conf1.tx_stop_en = 0;

	// Clear interrupts
	I2S_REG.int_ena.val = 0;
	I2S_REG.int_clr.val = 0xFFFFFFFF;

	uint32_t f_apb = getApbFrequency(); // usually 80 MHz on ESP32 classic
	uint32_t target_ws = I2S_SAMPLE_RATE;

	// Fixed 32-bit mono
	uint32_t bits = 32;

	// Compute combined divider
	uint32_t div_total = f_apb / (target_ws * bits);

	// Split into clkm_div and bck_div
	uint32_t clkm_div = div_total; // simplest: put all into clkm_div
	uint32_t bck_div = 1;

	// Program registers
	I2S_REG.clkm_conf.clka_en = 0;
	I2S_REG.clkm_conf.clk_en = 1;
	I2S_REG.clkm_conf.clkm_div_a = 1;
	I2S_REG.clkm_conf.clkm_div_b = 0;
	I2S_REG.clkm_conf.clkm_div_num = clkm_div;

	I2S_REG.sample_rate_conf.tx_bck_div_num = bck_div;
	I2S_REG.sample_rate_conf.tx_bits_mod = bits - 1; // 31 for 32-bit
}

/**
 * Program LC FIFO timeout so ISR cadence ≈ I2S_SAMPLE_RATE / 32
 */
static void IRAM_ATTR i2s_enable_periodic_isr(void)
{
	float f_isr = (float)I2S_SAMPLE_RATE / (float)I2S_REFILL_CHUNK_WORDS;
	uint32_t best_shift = 0, best_timeout = 1;
	for (uint32_t shift = 0; shift <= 7; ++shift)
	{
		float tickrate = 88000.0f / (float)(1u << shift);
		uint32_t timeout = (uint32_t)((tickrate / f_isr) + 0.5f);
		if (timeout >= 1 && timeout <= 255)
		{
			best_shift = shift;
			best_timeout = timeout;
			break;
		}
	}
	I2S_REG.lc_hung_conf.fifo_timeout_shift = best_shift;
	I2S_REG.lc_hung_conf.fifo_timeout = best_timeout;
	I2S_REG.lc_hung_conf.fifo_timeout_ena = 1;
	I2S_REG.int_clr.tx_hung = 1;
	I2S_REG.int_ena.tx_hung = 1;

	attach_i2s_isr();
}

/**
 * default mode transition
 */
static void IRAM_ATTR i2s_enter_default_mode_glitch_free(void)
{
	// configures fifo mode
	i2s_tx_base_config();

	// Prefill 64 words
	i2s_fifo_fill_words();

	// Enable periodic ISR (~every 32 samples)
	i2s_enable_periodic_isr();

	// Start TX
	I2S_REG.conf.tx_start = 1;
}

/**
 * realtime mode transition
 */
static void IRAM_ATTR i2s_enter_realtime_mode_glitch_free(void)
{
	// Stop periodic refills
	I2S_REG.int_ena.tx_hung = 0;
	I2S_REG.int_clr.tx_hung = 1;

	// Arm FIFO empty interrupt for clean handover
	I2S_REG.int_clr.tx_rempty = 1;
	I2S_REG.int_ena.tx_rempty = 1;
	attach_i2s_isr();

	// Ensure TX is running so the FIFO drains
	I2S_REG.conf.tx_start = 1;
}

/**
 * Switch mode and entre transition function
 */
uint8_t itp_set_step_mode(uint8_t mode)
{
	uint8_t last_mode = I2S_MODE;
	if (mode)
	{
		itp_sync();

#ifdef USE_I2S_REALTIME_MODE_ONLY
		__atomic_store_n((uint32_t *)&i2s_mode, (ITP_STEP_MODE_SYNC | ITP_STEP_MODE_REALTIME), __ATOMIC_RELAXED);
#else
		__atomic_store_n((uint32_t *)&i2s_mode, (ITP_STEP_MODE_SYNC | mode), __ATOMIC_RELAXED);
#endif

		// Apply immediately: glitch-free path
		switch (mode)
		{
		case ITP_STEP_MODE_DEFAULT:
			i2s_enter_default_mode_glitch_free();
			break;
		case ITP_STEP_MODE_REALTIME:
			i2s_enter_realtime_mode_glitch_free();
			break;
		}

		// Clear sync flag
		__atomic_fetch_and((uint32_t *)&i2s_mode, ~ITP_STEP_MODE_SYNC, __ATOMIC_RELAXED);

		// Small settle if desired (optional)
		// cnc_delay_ms(1);
	}
	return last_mode;
}

/**
 * Initializes I2S
 */
void mcu_i2s_extender_init(void)
{
	periph_module_enable(PERIPH_I2S0_MODULE);

	// Route pins via GPIO matrix
	gpio_set_direction(IC74HC595_I2S_CLK, GPIO_MODE_OUTPUT);
	gpio_matrix_out(IC74HC595_I2S_CLK, I2S0O_BCK_OUT_IDX, false, false);

	gpio_set_direction(IC74HC595_I2S_WS, GPIO_MODE_OUTPUT);
	gpio_matrix_out(IC74HC595_I2S_WS, I2S0O_WS_OUT_IDX, false, false);

	gpio_set_direction(IC74HC595_I2S_DATA, GPIO_MODE_OUTPUT);
	gpio_matrix_out(IC74HC595_I2S_DATA, I2S0O_DATA_OUT0_IDX, false, false);

#ifndef USE_I2S_REALTIME_MODE_ONLY
	itp_set_step_mode(ITP_STEP_MODE_DEFAULT);
#else
	itp_set_step_mode(ITP_STEP_MODE_REALTIME);
#endif
}

#endif
#endif