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
#include "hal/i2s_hal.h"
#include "hal/i2s_ll.h"
#include <soc/i2s_struct.h>

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
		if (i2s_ll_get_intr_status(&I2S_REG) & I2S_TX_WFULL_INT_ST)
		{
			i2s_ll_clear_intr_status(&I2S_REG, I2S_TX_WFULL_INT_CLR);
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
		// WRITE_PERI_REG(I2S_FIFO_WR_REG(I2S_PORT), 0xaaaaaaaa);
		I2S_REG.fifo_wr = __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED);
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
	// mcu_toggle_output(DOUT49);
	uint32_t st = i2s_ll_get_intr_status(&I2S_REG);

	// Default mode: periodic refill

	if (st & I2S_TX_PUT_DATA_INT_ST)
	{
		if ((i2s_mode & ~ITP_STEP_MODE_SYNC) == ITP_STEP_MODE_DEFAULT)
		{
			i2s_fifo_fill_words();
		}
	}

	// Realtime transition: when draining, detect FIFO empty and flip
	if (st & I2S_TX_REMPTY_INT_ST)
	{
		if ((i2s_mode & ~ITP_STEP_MODE_SYNC) == ITP_STEP_MODE_REALTIME)
		{
			i2s_ll_enable_intr(&I2S_REG, I2S_TX_REMPTY_INT_ENA, 0);
			i2s_ll_tx_stop(&I2S_REG);
		}
	}

	i2s_ll_clear_intr_status(&I2S_REG, 0x0000ffff);
}

static intr_handle_t i2s_intr_handle = NULL;
#ifndef I2S_INTR_SRC
#define I2S_INTR_SRC __helper__(ETS_I2S, I2S_PORT, _INTR_SOURCE)
#endif

static void attach_i2s_isr(void)
{
	if (!i2s_intr_handle)
	{
		I2S_REG.fifo_conf.tx_data_num = I2S_REFILL_CHUNK_WORDS;
		esp_intr_alloc_intrstatus(I2S_INTR_SRC,
															ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL3,
															(uint32_t)i2s_ll_get_intr_status_reg(&I2S_REG),
															(I2S_TX_PUT_DATA_INT_ST | I2S_TX_REMPTY_INT_ST),
															&i2s_tx_isr,
															NULL,
															&i2s_intr_handle);
	}
}

static void IRAM_ATTR i2s_tx_base_config(void)
{
	// Stop and reset
	i2s_ll_tx_stop_link(&I2S_REG);
	i2s_ll_tx_stop(&I2S_REG);
	i2s_ll_tx_reset(&I2S_REG);
	i2s_ll_rx_reset(&I2S_REG);
	i2s_ll_tx_reset_fifo(&I2S_REG);
	i2s_ll_rx_reset_fifo(&I2S_REG);

	// Disable DMA, PDM, camera/LCD
	i2s_ll_enable_dma(&I2S_REG, false);
	i2s_ll_tx_disable_intr(&I2S_REG);
	i2s_ll_rx_disable_intr(&I2S_REG);
#ifdef SOC_I2S_SUPPORTS_PDM_TX
	i2s_ll_tx_enable_pdm(&I2S_REG, false);
#endif
	i2s_ll_enable_lcd(&I2S_REG, false);
	i2s_ll_enable_camera(&I2S_REG, false);

	// 32-bit frames, mono-like usage, MSB-right placement compatible with 74HC595 stream
	// i2s_ll_tx_set_chan_mod(&I2S_REG, 0);
	i2s_ll_tx_set_chan_mod(&I2S_REG, I2S_CHANNEL_FMT_ONLY_RIGHT);
	i2s_ll_tx_set_sample_bit(&I2S_REG, I2S_BITS_PER_SAMPLE_32BIT, I2S_BITS_PER_SAMPLE_32BIT);
	// I2S_REG.fifo_conf.tx_fifo_mod = 3;
	i2s_ll_tx_enable_mono_mode(&I2S_REG, true);
	i2s_ll_tx_enable_msb_right(&I2S_REG, false);
	i2s_ll_tx_enable_right_first(&I2S_REG, false);
	i2s_ll_tx_enable_msb_shift(&I2S_REG, false);
	// i2s_ll_rx_enable_msb_shift(&I2S_REG, false);
	i2s_ll_tx_force_enable_fifo_mod(&I2S_REG, true);
	i2s_ll_tx_set_slave_mod(&I2S_REG, false);
	i2s_ll_tx_set_ws_width(&I2S_REG, 0);
#ifdef CONFIG_IDF_TARGET_ESP32
	i2s_ll_tx_clk_set_src(&I2S_REG, I2S_CLK_D2CLK); // 80 MHz typical
#endif

	// Frame rate selection
	i2s_ll_mclk_div_t div = {2, 32, 16}; // default 500 kHz @ 32-bit when bck_div=2
	i2s_ll_tx_set_clk(&I2S_REG, &div);
	i2s_ll_tx_set_bck_div_num(&I2S_REG, 2);

#ifndef USE_I2S_REALTIME_MODE_ONLY
	attach_i2s_isr();
	// i2s_ll_tx_enable_intr(&I2S_REG);
#else
	i2s_ll_tx_disable_intr(&I2S_REG);
#endif
}

/**
 * Program LC FIFO timeout so ISR cadence ≈ I2S_SAMPLE_RATE / 32
 */
static void IRAM_ATTR i2s_enable_periodic_isr(void)
{
	i2s_ll_clear_intr_status(&I2S_REG, 0x0000ffff);
	i2s_ll_enable_intr(&I2S_REG, (I2S_TX_PUT_DATA_INT_ENA | I2S_TX_REMPTY_INT_ENA), 1);
}

/**
 * default mode transition
 */
static void IRAM_ATTR i2s_enter_default_mode_glitch_free(void)
{
	// Prefill 64 words
	i2s_fifo_fill_words();

	// pause I2S and the pulse timer
	timer_pause(ITP_TIMER_TG, ITP_TIMER_IDX);
	i2s_ll_tx_stop(&I2S_REG);
	
	// Enable periodic ISR (~every 32 samples)
	i2s_enable_periodic_isr();

	// set the adequate mode (from realtime to FIFO)
	i2s_ll_tx_set_chan_mod(&I2S_REG, 0);

	// Start TX
	i2s_ll_tx_start(&I2S_REG);
}

/**
 * realtime mode transition
 */
static void IRAM_ATTR i2s_enter_realtime_mode_glitch_free(void)
{
	// prevent the refill from happening
	i2s_ll_enable_intr(&I2S_REG, I2S_TX_PUT_DATA_INT_ENA, 0);

	// wait for the fifo to empty and the I2S stop
	while (I2S_REG.conf.tx_start)
	{
		ets_delay_us(1);
	}

	i2s_ll_tx_set_chan_mod(&I2S_REG, 3);
	I2S_REG.conf_single_data = __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED);

	// Restart TX and the timer
	i2s_ll_tx_start(&I2S_REG);
	timer_start(ITP_TIMER_TG, ITP_TIMER_IDX);
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

#include <soc/gpio_periph.h>
#include <soc/gpio_sig_map.h>
#define BCK_OUT_IDX __helper__(I2S, I2S_PORT, O_BCK_OUT_IDX)
#define WS_OUT_IDX __helper__(I2S, I2S_PORT, O_WS_OUT_IDX)
#define DATA_OUT_IDX __helper__(I2S, I2S_PORT, O_DATA_OUT23_IDX)
#define PERIPH_I2S __helper__(PERIPH_I2S, I2S_PORT, _MODULE)
void mcu_i2s_extender_init(void)

{
	periph_module_reset(PERIPH_I2S);
	periph_module_enable(PERIPH_I2S);

	// Route pins via GPIO matrix
	gpio_set_direction(IC74HC595_I2S_CLK, GPIO_MODE_OUTPUT);
	gpio_matrix_out(IC74HC595_I2S_CLK, BCK_OUT_IDX, false, false);

	gpio_set_direction(IC74HC595_I2S_WS, GPIO_MODE_OUTPUT);
	gpio_matrix_out(IC74HC595_I2S_WS, WS_OUT_IDX, false, false);

	gpio_set_direction(IC74HC595_I2S_DATA, GPIO_MODE_OUTPUT);
	gpio_matrix_out(IC74HC595_I2S_DATA, DATA_OUT_IDX, false, false);

	i2s_tx_base_config();

#ifndef USE_I2S_REALTIME_MODE_ONLY
	itp_set_step_mode(ITP_STEP_MODE_DEFAULT);
#else
	itp_set_step_mode(ITP_STEP_MODE_REALTIME);
#endif
}

#endif
#endif