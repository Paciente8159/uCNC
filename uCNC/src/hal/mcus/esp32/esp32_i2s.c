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

#if (CONFIG_IDF_TARGET_ESP32)
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
#include <rom/lldesc.h>

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

#define DMA_BUFFER_SIZE 32

volatile uint32_t ic74hc595_i2s_pins;
volatile uint32_t i2s_mode;

typedef void (*i2s_refill_cb_t)(uint32_t *buf, size_t len);

typedef struct
{
	i2s_hal_context_t ctx;
	uint32_t *buf_a;
	uint32_t *buf_b;
	lldesc_t desc_a;
	lldesc_t desc_b;
	i2s_refill_cb_t refill_cb;
	intr_handle_t isr_handle;
} i2s_hal_t;

static i2s_hal_t i2s_hal;

MCU_CALLBACK void mcu_itp_isr(void *arg);
MCU_CALLBACK void mcu_gen_pwm(void);
MCU_CALLBACK void mcu_gen_servo(void);
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
static void IRAM_ATTR i2s_fifo_fill_words(uint32_t *buf, size_t len)
{
	for (int i = 0; i < len; i++)
	{
		signal_timer.us_step = 4;

		mcu_gen_step();
		mcu_gen_pwm();
		mcu_gen_servo();

#if defined(MCU_HAS_ONESHOT_TIMER) && defined(ENABLE_RT_SYNC_MOTIONS)
		mcu_gen_oneshot();
#endif
		// WRITE_PERI_REG(I2S_FIFO_WR_REG(I2S_PORT), 0xaaaaaaaa);
		buf[i] = __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED);
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
	uint32_t int_st = i2s_hal_get_intr_status(&i2s_hal.ctx);
	uint8_t mode = I2S_MODE;

	// Stage A: DMA descriptor finished
	if (int_st & I2S_OUT_EOF_INT_ST)
	{
		lldesc_t *finished = NULL;
		i2s_hal_get_out_eof_des_addr(&i2s_hal.ctx, (uint32_t *)&finished);

		// Normal ping-pong refill if NOT switching
		switch (mode)
		{
		case ITP_STEP_MODE_DEFAULT:
			mcu_clear_output(DOUT49);
			if (finished == &i2s_hal.desc_a)
			{
				i2s_hal.refill_cb(i2s_hal.buf_a, DMA_BUFFER_SIZE);
				i2s_hal.desc_a.owner = 1;
			}
			else if (finished == &i2s_hal.desc_b)
			{
				i2s_hal.refill_cb(i2s_hal.buf_b, DMA_BUFFER_SIZE);
				i2s_hal.desc_b.owner = 1;
			}
			break;
		case (ITP_STEP_MODE_REALTIME | ITP_STEP_MODE_SYNC):
		mcu_set_output(DOUT49);
			i2s_hal_stop_tx_link(&i2s_hal.ctx);
			__atomic_store_n((uint32_t *)&i2s_mode, ITP_STEP_MODE_REALTIME, __ATOMIC_RELAXED);
			break;
		case ITP_STEP_MODE_REALTIME:
			mcu_clear_output(DOUT49);
			I2S_REG.conf.tx_start = 0;
			I2S_REG.conf.tx_reset = 1;
			I2S_REG.conf.tx_reset = 0;
			I2S_REG.conf.rx_fifo_reset = 1;
			I2S_REG.conf.rx_fifo_reset = 0;
			// modify registers for realtime usage
			I2S_REG.out_link.stop = 1;
			I2S_REG.fifo_conf.dscr_en = 0;
			I2S_REG.conf.tx_start = 0;
			I2S_REG.int_clr.val = 0xFFFFFFFF;
			I2S_REG.clkm_conf.clka_en = 0;			// Use PLL/2 as reference
			I2S_REG.clkm_conf.clkm_div_num = 2; // reset value of 4
			I2S_REG.clkm_conf.clkm_div_a = 1;		// 0 at reset, what about divide by 0?
			I2S_REG.clkm_conf.clkm_div_b = 0;		// 0 at reset
			I2S_REG.fifo_conf.tx_fifo_mod = 3;	// 32 bits single channel data
			I2S_REG.conf_chan.tx_chan_mod = 3;	//
			I2S_REG.sample_rate_conf.tx_bits_mod = 32;
			I2S_REG.conf.tx_msb_shift = 0;
			I2S_REG.conf.rx_msb_shift = 0;
			I2S_REG.int_ena.out_eof = 0;
			I2S_REG.int_ena.out_dscr_err = 0;
			I2S_REG.conf_single_data = __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED);
			I2S_REG.conf1.tx_stop_en = 0;
			I2S_REG.int_ena.val = 0;
			I2S_REG.fifo_conf.dscr_en = 1;
			I2S_REG.int_clr.val = 0xFFFFFFFF;
			I2S_REG.out_link.start = 1;
			I2S_REG.conf.tx_start = 1;
			// Start TX and kick the motion timer
			i2s_hal_start_tx(&i2s_hal.ctx);
			timer_set_counter_value(ITP_TIMER_TG, ITP_TIMER_IDX, 0x00000000ULL);
			timer_group_clr_intr_status_in_isr(ITP_TIMER_TG, ITP_TIMER_IDX);
			timer_enable_intr(ITP_TIMER_TG, ITP_TIMER_IDX);
			timer_start(ITP_TIMER_TG, ITP_TIMER_IDX);
			break;
		}
	}

	i2s_hal_clear_intr_status(&i2s_hal.ctx, int_st);
}

/**
 * default mode transition
 */
static void IRAM_ATTR i2s_enter_default_mode_glitch_free(void)
{
	// pause I2S and the pulse timer
	timer_pause(ITP_TIMER_TG, ITP_TIMER_IDX);
	timer_disable_intr(ITP_TIMER_TG, ITP_TIMER_IDX);
	i2s_hal_stop_tx(&i2s_hal.ctx);
	i2s_hal_reset_tx(&i2s_hal.ctx);
	i2s_hal_reset_rx(&i2s_hal.ctx);
	i2s_hal_reset_tx_fifo(&i2s_hal.ctx);
	i2s_hal_reset_rx_fifo(&i2s_hal.ctx);

	// Prefill 64 words
	i2s_fifo_fill_words(i2s_hal.buf_a, DMA_BUFFER_SIZE);
	i2s_fifo_fill_words(i2s_hal.buf_b, DMA_BUFFER_SIZE);
	// init the addres descriptor and enable ISR
	i2s_hal_start_tx_link(&i2s_hal.ctx, (uint32_t)&i2s_hal.desc_a);
	i2s_hal_enable_tx_dma(&i2s_hal.ctx);
	i2s_hal_enable_tx_intr(&i2s_hal.ctx);

	// Start TX
	i2s_hal_start_tx(&i2s_hal.ctx);
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
		mcu_toggle_output(DOUT49);
		// Apply immediately: glitch-free path
		switch (mode)
		{
		case ITP_STEP_MODE_DEFAULT:
			i2s_enter_default_mode_glitch_free();
			// Clear sync flag
			__atomic_store_n((uint32_t *)&i2s_mode, ITP_STEP_MODE_DEFAULT, __ATOMIC_RELAXED);
			break;
		}
		mcu_toggle_output(DOUT49);
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
#define I2S_ITR_SRC __helper__(ETS_I2S, I2S_PORT, _INTR_SOURCE)

static void i2s_tx_base_config(void)
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

	i2s_hal.ctx.dev = &I2S_REG;
	i2s_hal.ctx.version = I2S_PORT;
	// Stop and reset
	i2s_hal_stop_tx_link(&i2s_hal.ctx);
	i2s_hal_stop_tx(&i2s_hal.ctx);
	i2s_hal_reset_tx(&i2s_hal.ctx);
	i2s_hal_reset_rx(&i2s_hal.ctx);
	i2s_hal_reset_tx_fifo(&i2s_hal.ctx);
	i2s_hal_reset_rx_fifo(&i2s_hal.ctx);

	// 32-bit frames, mono-like usage, MSB-right placement compatible with 74HC595 stream
	i2s_hal_config_t config = {
			.mode = I2S_MODE_MASTER | I2S_MODE_TX,
			.sample_rate = I2S_SAMPLE_RATE,
			.comm_fmt = I2S_COMM_FORMAT_STAND_I2S | I2S_COMM_FORMAT_STAND_MSB,
			.chan_fmt = I2S_CHANNEL_FMT_ONLY_LEFT,
			.sample_bits = I2S_BITS_PER_SAMPLE_32BIT,
			.chan_bits = I2S_BITS_PER_SAMPLE_32BIT,
			.active_chan = 1,
			.total_chan = 2};

	// i2s_hal_clock_cfg_t clk_conf;
	// 		i2s_hal_mclk_div_decimal_cal(&clk_conf, &clock);
	i2s_hal_config_param(&i2s_hal.ctx, &config);
	i2s_hal_clock_cfg_t clock = {160000000, 64000000, 32000000, 2, 2}; // 1us per word
																																		 // i2s_hal_clock_cfg_t clock = {160000000, 160000000,160000000, 1, 5 }; // 2us per word
																																		 // i2s_hal_clock_cfg_t clock = {160000000, 32000000,16000000, 5, 2 }; // 2us per word also

	i2s_hal_tx_clock_config(&i2s_hal.ctx, &clock);

#ifndef USE_I2S_REALTIME_MODE_ONLY
	i2s_hal_enable_tx_dma(&i2s_hal.ctx);
	i2s_hal_enable_tx_intr(&i2s_hal.ctx);
	i2s_hal_disable_rx_intr(&i2s_hal.ctx);
	i2s_hal.refill_cb = i2s_fifo_fill_words;

	// Allocate DMA-capable buffers
	i2s_hal.buf_a = heap_caps_malloc(DMA_BUFFER_SIZE * sizeof(uint32_t), MALLOC_CAP_DMA);
	i2s_hal.buf_b = heap_caps_malloc(DMA_BUFFER_SIZE * sizeof(uint32_t), MALLOC_CAP_DMA);

	if (!i2s_hal.buf_a || !i2s_hal.buf_b)
		return;

	// Setup descriptors
	i2s_hal.desc_a.length = DMA_BUFFER_SIZE * sizeof(uint32_t);
	i2s_hal.desc_a.size = i2s_hal.desc_a.length;
	i2s_hal.desc_a.owner = 1;
	i2s_hal.desc_a.buf = (uint8_t *)i2s_hal.buf_a;
	i2s_hal.desc_a.eof = 1;
	i2s_hal.desc_a.qe.stqe_next = &i2s_hal.desc_b;

	i2s_hal.desc_b.length = DMA_BUFFER_SIZE * sizeof(uint32_t);
	i2s_hal.desc_b.size = i2s_hal.desc_b.length;
	i2s_hal.desc_b.owner = 1;
	i2s_hal.desc_b.buf = (uint8_t *)i2s_hal.buf_b;
	i2s_hal.desc_b.eof = 1;
	i2s_hal.desc_b.qe.stqe_next = &i2s_hal.desc_a;

	i2s_hal_start_tx_link(&i2s_hal.ctx, (uint32_t)&i2s_hal.desc_a);

	// Hook ISR
	esp_intr_alloc_intrstatus(I2S_ITR_SRC, ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL3, (uint32_t)i2s_ll_get_intr_status_reg(&I2S_REG), (I2S_OUT_EOF_INT_ST | I2S_TX_REMPTY_INT_ST),
														i2s_tx_isr, NULL, &i2s_hal.isr_handle);
#else
	i2s_ll_tx_disable_intr(&I2S_REG);
#endif
}

void mcu_i2s_extender_init(void)
{
	i2s_tx_base_config();

#ifndef USE_I2S_REALTIME_MODE_ONLY
	itp_set_step_mode(ITP_STEP_MODE_DEFAULT);
#else
	itp_set_step_mode(ITP_STEP_MODE_REALTIME);
	timer_enable_intr(ITP_TIMER_TG, ITP_TIMER_IDX);
	timer_start(ITP_TIMER_TG, ITP_TIMER_IDX);
#endif
}

#endif
#endif