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
#include "esp_private/gdma.h"

#define BCK_OUT_IDX __helper__(I2S, I2S_PORT, O_BCK_OUT_IDX)	// resolves to I2S<I2S_PORT>O_BCK_OUT_IDX
#define WS_OUT_IDX __helper__(I2S, I2S_PORT, O_WS_OUT_IDX)		// resolves to I2S<I2S_PORT>O_WS_OUT_IDX
#define DATA_OUT_IDX __helper__(I2S, I2S_PORT, O_SD_OUT_IDX)	// resolves to I2S<I2S_PORT>O_SD_OUT_IDX
#define PERIPH_I2S __helper__(PERIPH_I2S, I2S_PORT, _MODULE)	// resolves to I2S<I2S_PORT>_MODULE
#define I2S_ITR_SRC __helper__(ETS_I2S, I2S_PORT, _INTR_SOURCE) // resolves to I2S<I2S_PORT>_INTR_SOURCE
#define GDMA_I2S_PERIPH (SOC_GDMA_TRIG_PERIPH_I2S0 + I2S_PORT)	// resolves to SOC_GDMA_TRIG_PERIPH_I2S0 or SOC_GDMA_TRIG_PERIPH_I2S1

#ifdef IC74HC595_CUSTOM_SHIFT_IO
#if IC74HC595_COUNT != 4
#error "IC74HC595_COUNT must be 4(bytes) to use ESP32 I2S mode for IO shifting"
#endif

#ifndef I2S_REG
#define I2S_REG __helper__(I2S, I2S_PORT, ) // resolves to I2S<I2S_PORT>
#endif
#ifndef I2S_PERIF
#define I2S_PERIF __helper__(PERIPH_I2S, I2S_PORT, _MODULE) // resolves to I2S<I2S_PORT>_MODULE
#endif

#define I2S_ITR_FLAGS (GDMA_LL_EVENT_TX_DONE)

#ifndef I2S_SAMPLE_RATE
#define I2S_SAMPLE_RATE 500000 // max 500000
#endif

#define ITP_TICK_US (1000000 / (I2S_SAMPLE_RATE << 1)) // calculates the time it takes to send a sample
// for I2S_SAMPLE_RATE 250000 ITP_TICK_US is 4us
// for I2S_SAMPLE_RATE 500000 ITP_TICK_US is 2us

#define DMA_BUFFER_SIZE 64

volatile uint32_t ic74hc595_i2s_pins;
volatile DRAM_ATTR uint32_t i2s_mode;

static uint32_t DMA_ATTR buf[2][DMA_BUFFER_SIZE];
static dma_descriptor_t DMA_ATTR desc[2];

typedef struct
{
	i2s_hal_context_t ctx;
	int gdma_channel;
	gdma_channel_handle_t dma_chan;
	intr_handle_t intr_handle;
} i2s_hal_t;

static i2s_hal_t i2s_hal;

MCU_CALLBACK void mcu_itp_isr(void *arg);
MCU_CALLBACK void mcu_gen_pwm(void);
MCU_CALLBACK void mcu_gen_servo(void);
MCU_CALLBACK void mcu_gen_step(void);
MCU_CALLBACK void mcu_gpio_isr(void *type);
MCU_CALLBACK void mcu_gen_oneshot(void);

IRAM_ATTR void i2s_write_word(uint32_t value)
{
	I2S_REG.conf_single_data = value;
}

/**
 * Try to fill the buffer (saturation)
 */

static FORCEINLINE uint32_t i2s_signal_loop()
{
	signal_timer.us_step = ITP_TICK_US;
	mcu_gen_step();
	mcu_gen_pwm();
	mcu_gen_servo();

#if defined(MCU_HAS_ONESHOT_TIMER) && defined(ENABLE_RT_SYNC_MOTIONS)
	mcu_gen_oneshot();
#endif

	return __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED);
}
static void IRAM_ATTR i2s_fifo_fill_words(uint32_t *buf, uint32_t mode)
{
	for (int i = 0; i < DMA_BUFFER_SIZE; i++)
	{
		*buf++ = i2s_signal_loop();
	}
}

static void IRAM_ATTR i2s_dma_event_handler(void *arg)
{
	uint32_t irq = gdma_ll_tx_get_interrupt_status(&GDMA, i2s_hal.gdma_channel);
	dma_descriptor_t *finished = (dma_descriptor_t *)gdma_ll_tx_get_eof_desc_addr(&GDMA, i2s_hal.gdma_channel);
	uint32_t mode = I2S_MODE;
	uint32_t *buffer = ((finished == &desc[0]) ? &buf[0][0] : &buf[1][0]);
	memset(finished, 0, sizeof(dma_descriptor_t));

	switch (mode)
	{
	case ITP_STEP_MODE_DEFAULT:
		finished->dw0.suc_eof = 1;
		finished->dw0.length = DMA_BUFFER_SIZE * sizeof(uint32_t);
		finished->dw0.size = DMA_BUFFER_SIZE * sizeof(uint32_t);
		finished->buffer = buffer;
		finished->next = (dma_descriptor_t *)((finished == &desc[1]) ? &desc[0] : &desc[1]);
		i2s_fifo_fill_words(buffer, mode);
		finished->dw0.owner = 1;
		break;
	default:
		I2S_REG.tx_conf.tx_stop_en = 1;
		break;
	}

	gdma_ll_tx_clear_interrupt_status(&GDMA, i2s_hal.gdma_channel, irq);
}

static void IRAM_ATTR i2s_enter_realtime_mode(void)
{
	I2S_REG.tx_conf.tx_stop_en = 0;
	config.mode = I2S_MODE_MASTER | I2S_MODE_TX;
	config.sample_rate = I2S_SAMPLE_RATE;
	config.comm_fmt = I2S_COMM_FORMAT_STAND_I2S | I2S_COMM_FORMAT_STAND_MSB;
	config.chan_fmt = I2S_CHANNEL_FMT_ONLY_LEFT;
	config.sample_bits = I2S_BITS_PER_SAMPLE_32BIT;
	config.chan_bits = I2S_BITS_PER_SAMPLE_32BIT;
	config.active_chan = 0;
	config.total_chan = 2;

	i2s_hal_config_param(&i2s_hal.ctx, &config);
	i2s_ll_tx_set_active_chan_mask(&I2S_REG, 0);
	// I2S_REG.tx_timing.tx_ws_out_dm = 1;
	I2S_REG.tx_conf.tx_chan_equal = 0;
	I2S_REG.conf_single_data = __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED);
	timer_set_counter_value(ITP_TIMER_TG, ITP_TIMER_IDX, 0x00000000ULL);
	timer_start(ITP_TIMER_TG, ITP_TIMER_IDX);
}

/**
 * default mode transition
 */
static void i2s_enter_mode_glitch_free(uint32_t mode)
{
	// on realtime mode switchwait for GDMA to flush, else continue
	while (!I2S_REG.state.tx_idle && (mode & ITP_STEP_MODE_REALTIME))
	{
		ets_delay_us(1);
	}

	// Reset I2S
	gdma_ll_tx_stop(&GDMA, i2s_hal.gdma_channel);
	I2S_REG.tx_conf.tx_stop_en = 0;
	i2s_ll_tx_reset(&I2S_REG);
	i2s_ll_tx_reset_fifo(&I2S_REG);

	// switch mode
	__atomic_store_n((uint32_t *)&i2s_mode, mode, __ATOMIC_RELAXED);

	i2s_hal_config_t config;
	switch (mode)
	{
	case ITP_STEP_MODE_DEFAULT:
		// Prefill
		timer_pause(ITP_TIMER_TG, ITP_TIMER_IDX);
		config.mode = I2S_MODE_MASTER | I2S_MODE_TX;
		config.sample_rate = I2S_SAMPLE_RATE;
		config.comm_fmt = I2S_COMM_FORMAT_STAND_I2S | I2S_COMM_FORMAT_STAND_MSB;
		config.chan_fmt = I2S_CHANNEL_FMT_ALL_LEFT;
		config.sample_bits = I2S_BITS_PER_SAMPLE_32BIT;
		config.chan_bits = I2S_BITS_PER_SAMPLE_32BIT;
		config.active_chan = 1;
		config.total_chan = 2;

		i2s_hal_config_param(&i2s_hal.ctx, &config);
		i2s_ll_tx_set_active_chan_mask(&I2S_REG, 1);
		I2S_REG.tx_timing.tx_ws_out_dm = 1;
		// gdma_ll_tx_reset_channel(&GDMA, i2s_hal.gdma_channel);
		memset(&desc[0], 0, sizeof(dma_descriptor_t));
		memset(&desc[1], 0, sizeof(dma_descriptor_t));
		desc[0].dw0.owner = 1;
		desc[0].dw0.suc_eof = 1;
		desc[0].dw0.length = DMA_BUFFER_SIZE * sizeof(uint32_t);
		desc[0].dw0.size = DMA_BUFFER_SIZE * sizeof(uint32_t);
		desc[0].buffer = &buf[0][0];
		desc[0].next = (dma_descriptor_t *)(&desc[1]);
		desc[1].dw0.owner = 1;
		desc[1].dw0.suc_eof = 1;
		desc[1].dw0.length = DMA_BUFFER_SIZE * sizeof(uint32_t);
		desc[1].dw0.size = DMA_BUFFER_SIZE * sizeof(uint32_t);
		desc[1].buffer = &buf[1][0];
		desc[1].next = (dma_descriptor_t *)(&desc[0]);
		i2s_fifo_fill_words(buf[0], mode);
		i2s_fifo_fill_words(buf[1], mode);

		// Start
		gdma_ll_tx_reset_channel(&GDMA, i2s_hal.gdma_channel);
		gdma_ll_tx_set_desc_addr(&GDMA, i2s_hal.gdma_channel, (uint32_t)(&desc[0]));
		gdma_ll_tx_clear_interrupt_status(&GDMA, i2s_hal.gdma_channel, gdma_ll_tx_get_interrupt_status(&GDMA, i2s_hal.gdma_channel));
		gdma_ll_tx_enable_interrupt(&GDMA, i2s_hal.gdma_channel, GDMA_LL_EVENT_TX_DONE | GDMA_LL_EVENT_TX_TOTAL_EOF, true);
		break;
	case ITP_STEP_MODE_REALTIME:
		i2s_enter_realtime_mode();
		break;
	}

	I2S_REG.tx_conf.tx_update = 1;
	while (I2S_REG.tx_conf.tx_update)
		;

	//    i2s_ll_tx_start(&I2S_REG); // Fails?
	// if (mode == ITP_STEP_MODE_DEFAULT)
	gdma_ll_tx_start(&GDMA, i2s_hal.gdma_channel);

	I2S_REG.tx_conf.tx_start = 1;
}

/**
 * Switch mode and entre transition function
 */
uint8_t itp_set_step_mode(uint8_t mode)
{
	uint8_t last_mode = I2S_MODE;
	if (mode != last_mode)
	{
		itp_sync();
#ifdef USE_I2S_REALTIME_MODE_ONLY
		__atomic_store_n((uint32_t *)&i2s_mode, (ITP_STEP_MODE_REALTIME), __ATOMIC_RELAXED);
#else
		__atomic_store_n((uint32_t *)&i2s_mode, (ITP_STEP_MODE_SYNC | mode), __ATOMIC_RELAXED);
#endif
		i2s_enter_mode_glitch_free(mode);
	}
	return last_mode;
}

/**
 * Initializes I2S
 */

static int32_t allocate_dma_channel(void)
{
	uint32_t ch = SOC_GDMA_PAIRS_PER_GROUP;

	do
	{
		if (GDMA.channel[--ch].out.link.addr == 0)
			return ch;
	} while (ch);

	return -1;
}

static void i2s_tx_base_config(void)
{

	if ((i2s_hal.gdma_channel = allocate_dma_channel()) == -1)
		return;

	// To make sure hardware is enabled before any hardware register operations.
	periph_module_reset(PERIPH_I2S);
	periph_module_enable(PERIPH_I2S);
	//    periph_module_reset(PERIPH_GDMA_MODULE);
	//    periph_module_enable(PERIPH_GDMA_MODULE);

	if (REG_GET_BIT(SYSTEM_PERIP_CLK_EN1_REG, SYSTEM_DMA_CLK_EN) == 0)
	{
		REG_CLR_BIT(SYSTEM_PERIP_CLK_EN1_REG, SYSTEM_DMA_CLK_EN);
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

	i2s_ll_tx_stop(&I2S_REG);
	gdma_ll_enable_clock(&GDMA, 1);
	gdma_ll_tx_reset_channel(&GDMA, i2s_hal.gdma_channel);
	gdma_ll_tx_enable_interrupt(&GDMA, i2s_hal.gdma_channel, GDMA_LL_TX_EVENT_MASK, false);
	gdma_ll_tx_clear_interrupt_status(&GDMA, i2s_hal.gdma_channel, GDMA_LL_TX_EVENT_MASK);
	gdma_ll_tx_connect_to_periph(&GDMA, i2s_hal.gdma_channel, GDMA_TRIG_PERIPH_I2S, GDMA_I2S_PERIPH);
	gdma_ll_tx_set_eof_mode(&GDMA, i2s_hal.gdma_channel, 0);

	i2s_hal.ctx.dev = &I2S_REG;
	i2s_hal_config_t config = {
		.mode = I2S_MODE_MASTER | I2S_MODE_TX,
		.sample_rate = I2S_SAMPLE_RATE,
		.comm_fmt = I2S_COMM_FORMAT_STAND_I2S | I2S_COMM_FORMAT_STAND_MSB,
		.chan_fmt = I2S_CHANNEL_FMT_ALL_LEFT,
		.sample_bits = I2S_BITS_PER_SAMPLE_32BIT,
		.chan_bits = I2S_BITS_PER_SAMPLE_32BIT,
		.active_chan = 1,
		.total_chan = 2};

	i2s_hal_config_param(&i2s_hal.ctx, &config);
	i2s_ll_tx_enable_pdm(&I2S_REG, false); // Enables TDM
	i2s_ll_tx_set_active_chan_mask(&I2S_REG, 1);
	I2S_REG.tx_timing.tx_ws_out_dm = 1;

#if I2S_SAMPLE_RATE == 500000
	i2s_ll_mclk_div_t mclk_set = {.mclk_div = 2, .a = 32, .b = 16};
#elif I2S_SAMPLE_RATE == 250000
	i2s_ll_mclk_div_t mclk_set = {.mclk_div = 5, .a = 0, .b = 0};
#else
	float mult = 16000000.0f / (float)(I2S_SAMPLE_RATE * 2 * 64);
	uint16_t div = (uint16_t)mult;
	float rem = (mult - div);
	uint16_t b = (uint16_t)(128.0f * rem);
	i2s_ll_mclk_div_t mclk_set;
	if (rem != 0)
	{
		mclk_set.mclk_div = div;
		mclk_set.a = 128;
		mclk_set.b = b;
	}
	else
	{
		mclk_set.mclk_div = div;
		mclk_set.a = 0;
		mclk_set.b = 0;
	};
	ESP_LOGD("i2s", "clock %u, %u, %u", mclk_set.mclk_div, mclk_set.a, mclk_set.b);
#endif

	//
	// i2s_set_clk
	//
	i2s_ll_tx_set_ws_idle_pol(&I2S_REG, 1);
	i2s_ll_tx_clk_set_src(&I2S_REG, I2S_CLK_D2CLK); // Set I2S_CLK_D2CLK as default
	i2s_ll_mclk_use_tx_clk(&I2S_REG);
	i2s_ll_tx_set_clk(&I2S_REG, &mclk_set);
	i2s_ll_tx_set_bck_div_num(&I2S_REG, 2);
	i2s_ll_tx_enable_clock(&I2S_REG);
	i2s_ll_tx_reset(&I2S_REG);
	i2s_ll_tx_reset_fifo(&I2S_REG);

	// Allocate and enable the I2S DMA interrupt
	esp_intr_alloc(gdma_periph_signals.groups[0].pairs[i2s_hal.gdma_channel].tx_irq_id, ESP_INTR_FLAG_IRAM, i2s_dma_event_handler, NULL, &i2s_hal.intr_handle);
}

void mcu_i2s_extender_init(void)
{
	i2s_tx_base_config();

#ifndef USE_I2S_REALTIME_MODE_ONLY
	itp_set_step_mode(ITP_STEP_MODE_DEFAULT);
#else
	itp_set_step_mode(ITP_STEP_MODE_REALTIME);
	i2s_enter_realtime_mode();
#endif
}

#endif
#endif