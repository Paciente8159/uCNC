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

#if (CONFIG_IDF_TARGET_ESP32S3)
#include "driver/uart.h"
#include "hal/i2s_hal.h"
#include "hal/gdma_types.h"
#include "driver/gpio.h"
#include "soc/gpio_sig_map.h"
#include "esp_heap_caps.h"
#include <driver/periph_ctrl.h>
#include <rom/lldesc.h>
#include <soc/i2s_struct.h>
#include <soc/gdma_struct.h>
#include <soc/gdma_channel.h>
#include "hal/i2s_ll.h"
#include "hal/gdma_ll.h"
#include "hal/dma_types.h"
#include "hal/interrupt_controller_hal.h"
#include <freertos/queue.h>
#include "esp_intr_alloc.h"
#include "soc/gdma_periph.h"
#include "soc/system_reg.h"
#include <stdatomic.h>
#include <soc/gpio_periph.h>

#define BCK_OUT_IDX __helper__(I2S, I2S_PORT, O_BCK_OUT_IDX)
#define WS_OUT_IDX __helper__(I2S, I2S_PORT, O_WS_OUT_IDX)
#define DATA_OUT_IDX __helper__(I2S, I2S_PORT, O_SD_OUT_IDX)
#define PERIPH_I2S __helper__(PERIPH_I2S, I2S_PORT, _MODULE)
#define I2S_ITR_SRC __helper__(ETS_I2S, I2S_PORT, _INTR_SOURCE)
#define GDMA_I2S_PERIPH (SOC_GDMA_TRIG_PERIPH_I2S0 + I2S_PORT)

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

#define I2S_ITR_FLAGS (GDMA_LL_EVENT_TX_DONE | GDMA_LL_EVENT_TX_TOTAL_EOF)

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
	int gdma_channel;
	uint32_t *buf_a;
	uint32_t *buf_b;
	dma_descriptor_t desc_a;
	dma_descriptor_t desc_b;
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
	static uint32_t counter;
	uint32_t int_st = gdma_ll_tx_get_interrupt_status(&GDMA, i2s_hal.gdma_channel);
	uint8_t mode = I2S_MODE;

	char str[64];
	sprintf(str, "%lu %u %lu\r\n\0", int_st, mode, counter++);
	uart_write_bytes(0, str, strlen(str));

	// Stage A: DMA descriptor finished
	if (int_st & I2S_ITR_FLAGS)
	{
		dma_descriptor_t *finished = NULL;
		finished = (dma_descriptor_t *)gdma_ll_tx_get_eof_desc_addr(&GDMA, i2s_hal.gdma_channel);
		
		// Normal ping-pong refill if NOT switching
		switch (mode)
		{
		case (ITP_STEP_MODE_DEFAULT | ITP_STEP_MODE_SYNC):
		case ITP_STEP_MODE_DEFAULT:
			mcu_clear_output(DOUT49);
			sprintf(str, "address none\r\n\0");
			if (finished == &i2s_hal.desc_a)
			{
				sprintf(str, "address a %lu\r\n\0", (uint32_t)i2s_hal.desc_a.next);
	
				i2s_hal.refill_cb(i2s_hal.buf_a, DMA_BUFFER_SIZE);
				i2s_hal.desc_a.dw0.owner = 1;
				i2s_hal.desc_a.dw0.suc_eof = 1;
			}
			else if (finished == &i2s_hal.desc_b)
			{
				sprintf(str, "address b %lu\r\n\0", (uint32_t)i2s_hal.desc_b.next);
				i2s_hal.refill_cb(i2s_hal.buf_b, DMA_BUFFER_SIZE);
				i2s_hal.desc_b.dw0.owner = 1;
				i2s_hal.desc_b.dw0.suc_eof = 1;
			}
			uart_write_bytes(0, str, strlen(str));
			break;
		case (ITP_STEP_MODE_REALTIME | ITP_STEP_MODE_SYNC):
			mcu_set_output(DOUT49);
			finished->next = NULL;
			__atomic_store_n((uint32_t *)&i2s_mode, ITP_STEP_MODE_REALTIME, __ATOMIC_RELAXED);
			break;
		case ITP_STEP_MODE_REALTIME:
			if (int_st & GDMA_LL_EVENT_TX_TOTAL_EOF)
			{
				mcu_clear_output(DOUT49);
				i2s_hal_stop_tx(&i2s_hal.ctx);
				i2s_hal_reset_tx(&i2s_hal.ctx);
				i2s_hal_reset_tx_fifo(&i2s_hal.ctx);
				gdma_ll_tx_stop(&GDMA, i2s_hal.gdma_channel);

				I2S_REG.conf_single_data = __atomic_load_n(&ic74hc595_i2s_pins, __ATOMIC_RELAXED);
				// Start TX and kick the motion timer
				i2s_hal_start_tx(&i2s_hal.ctx);
				timer_set_counter_value(ITP_TIMER_TG, ITP_TIMER_IDX, 0x00000000ULL);
				timer_group_clr_intr_status_in_isr(ITP_TIMER_TG, ITP_TIMER_IDX);
				timer_enable_intr(ITP_TIMER_TG, ITP_TIMER_IDX);
				timer_start(ITP_TIMER_TG, ITP_TIMER_IDX);
			}
			break;
		}
	}

	gdma_ll_tx_clear_interrupt_status(&GDMA, i2s_hal.gdma_channel, int_st);
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
	gdma_ll_tx_enable_interrupt(&GDMA, i2s_hal.gdma_channel, I2S_ITR_FLAGS, false);
	i2s_hal_reset_tx(&i2s_hal.ctx);
	i2s_hal_reset_rx(&i2s_hal.ctx);
	i2s_hal_reset_tx_fifo(&i2s_hal.ctx);
	i2s_hal_reset_rx_fifo(&i2s_hal.ctx);

	// Prefill 64 words
	i2s_fifo_fill_words(i2s_hal.buf_a, DMA_BUFFER_SIZE);
	i2s_fifo_fill_words(i2s_hal.buf_b, DMA_BUFFER_SIZE);

	i2s_hal.desc_a.dw0.owner = 1;
	i2s_hal.desc_a.dw0.suc_eof = 1;

	i2s_hal.desc_b.dw0.owner = 1;
	i2s_hal.desc_b.dw0.suc_eof = 1;
	// init the addres descriptor and enable ISR
	gdma_ll_tx_reset_channel(&GDMA, i2s_hal.gdma_channel);
	gdma_ll_tx_set_desc_addr(&GDMA, i2s_hal.gdma_channel, (uint32_t)&i2s_hal.desc_a);
	gdma_ll_tx_connect_to_periph(&GDMA, i2s_hal.gdma_channel, GDMA_TRIG_PERIPH_I2S, GDMA_I2S_PERIPH);
	gdma_ll_tx_clear_interrupt_status(&GDMA, i2s_hal.gdma_channel, 0xFF);
	gdma_ll_tx_enable_interrupt(&GDMA, i2s_hal.gdma_channel, I2S_ITR_FLAGS, true);

	// Start TX
	gdma_ll_tx_start(&GDMA, i2s_hal.gdma_channel);
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

static void i2s_tx_base_config(void)
{
	periph_module_reset(PERIPH_I2S);
	periph_module_enable(PERIPH_I2S);

	if (REG_GET_BIT(SYSTEM_PERIP_CLK_EN1_REG, SYSTEM_DMA_CLK_EN) == 0)
	{
		REG_SET_BIT(SYSTEM_PERIP_CLK_EN1_REG, SYSTEM_DMA_CLK_EN);
		REG_SET_BIT(SYSTEM_PERIP_RST_EN1_REG, SYSTEM_DMA_RST);
		REG_CLR_BIT(SYSTEM_PERIP_RST_EN1_REG, SYSTEM_DMA_RST);
	}

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

	i2s_ll_tx_set_sample_bit(&I2S_REG, 32, 32);
	i2s_ll_tx_set_ws_idle_pol(&I2S_REG, 1);
	i2s_ll_tx_clk_set_src(&I2S_REG, I2S_CLK_D2CLK);
	i2s_hal_config_param(&i2s_hal.ctx, &config);
	i2s_hal_clock_cfg_t clock = {160000000, 64000000, 32000000, 2, 2}; // 1us per word
																																		 // i2s_hal_clock_cfg_t clock = {160000000, 160000000,160000000, 1, 5 }; // 2us per word
																																		 // i2s_hal_clock_cfg_t clock = {160000000, 32000000,16000000, 5, 2 }; // 2us per word also

	i2s_hal_tx_clock_config(&i2s_hal.ctx, &clock);

#ifndef USE_I2S_REALTIME_MODE_ONLY
	// Allocate GDMA TX channel
	// Allocate DMA buffers
	i2s_hal.buf_a = heap_caps_malloc(DMA_BUFFER_SIZE * sizeof(uint32_t), MALLOC_CAP_DMA);
	i2s_hal.buf_b = heap_caps_malloc(DMA_BUFFER_SIZE * sizeof(uint32_t), MALLOC_CAP_DMA);

	// Setup descriptors
	i2s_hal.desc_a.dw0.owner = 1;
	i2s_hal.desc_a.dw0.suc_eof = 1;
	i2s_hal.desc_a.dw0.length = DMA_BUFFER_SIZE * sizeof(uint32_t);
	i2s_hal.desc_a.dw0.size = i2s_hal.desc_a.dw0.length;
	i2s_hal.desc_a.buffer = (uint8_t *)i2s_hal.buf_a;
	i2s_hal.desc_a.next = &i2s_hal.desc_b;

	i2s_hal.desc_b.dw0.owner = 1;
	i2s_hal.desc_b.dw0.suc_eof = 1;
	i2s_hal.desc_b.dw0.length = DMA_BUFFER_SIZE * sizeof(uint32_t);
	i2s_hal.desc_b.dw0.size = i2s_hal.desc_b.dw0.length;
	i2s_hal.desc_b.buffer = (uint8_t *)i2s_hal.buf_b;
	i2s_hal.desc_b.next = &i2s_hal.desc_a;

	char str[64];
	sprintf(str, "address a %lu\r\n\0", (uint32_t)&i2s_hal.desc_a);
	uart_write_bytes(0, str, strlen(str));

	sprintf(str, "address b %lu\r\n\0", (uint32_t)&i2s_hal.desc_b);
	uart_write_bytes(0, str, strlen(str));

	i2s_hal.refill_cb = i2s_fifo_fill_words;

	int ch = -1;
	for (int i = SOC_GDMA_PAIRS_PER_GROUP - 1; i >= 0; --i)
	{
		if (GDMA.channel[i].out.link.addr == 0)
		{
			ch = i;
			break;
		}
	}
	if (ch < 0)
	{ /* handle error */
	}
	i2s_hal.gdma_channel = ch;

	// Allocate GDMA channel
	gdma_ll_enable_clock(&GDMA, true);
	gdma_ll_tx_reset_channel(&GDMA, i2s_hal.gdma_channel);
	gdma_ll_tx_connect_to_periph(&GDMA, i2s_hal.gdma_channel, GDMA_TRIG_PERIPH_I2S, GDMA_I2S_PERIPH);
	gdma_ll_tx_set_desc_addr(&GDMA, i2s_hal.gdma_channel, (uint32_t)&i2s_hal.desc_a);
	gdma_ll_tx_enable_interrupt(&GDMA, i2s_hal.gdma_channel, I2S_ITR_FLAGS, true);

	// Hook ISR
	esp_intr_alloc(gdma_periph_signals.groups[0].pairs[i2s_hal.gdma_channel].tx_irq_id,
								 ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL3,
								 i2s_tx_isr, NULL, &i2s_hal.isr_handle);

	// esp_intr_alloc_intrstatus(I2S_ITR_SRC, ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL3, (uint32_t)GDMA., I2S_ITR_FLAGS,
	// 													i2s_tx_isr, NULL, &i2s_hal.isr_handle);
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