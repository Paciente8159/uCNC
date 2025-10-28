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
#define I2S_SAMPLE_RATE (500000)
#endif

#define DMA_BUFFER_SIZE 64

volatile uint32_t ic74hc595_i2s_pins;
volatile DRAM_ATTR uint32_t i2s_mode;

typedef void (*i2s_refill_cb_t)(uint32_t *buf, size_t len);

static uint32_t DMA_ATTR buf[2][DMA_BUFFER_SIZE];
static dma_descriptor_t DMA_ATTR desc[2];
static xQueueHandle queue;

typedef struct
{
	bool running;
	i2s_hal_context_t ctx;
	// int gdma_channel;
	gdma_channel_handle_t dma_chan;
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

static bool IRAM_ATTR i2s_dma_event_handler(gdma_channel_handle_t dma_chan, gdma_event_data_t *event_data, void *user_data)
{

	dma_descriptor_t *finished = (dma_descriptor_t *)event_data->tx_eof_desc_addr;
	ESP_LOGD("i2s", "fin %u", (uint32_t)finished);
	// portBASE_TYPE wake = pdFALSE;
	// xQueueSendFromISR(queue, &finished, &wake);

	// return wake;
	// uint8_t mode = I2S_MODE;

	// switch (mode)
	// {
	// case (ITP_STEP_MODE_DEFAULT | ITP_STEP_MODE_SYNC):
	// case ITP_STEP_MODE_DEFAULT:

	// 	// Refill finished buffer; keep ring intact
	// 	if (finished)
	// 	{
	i2s_fifo_fill_words((uint32_t *)finished->buffer, DMA_BUFFER_SIZE);
	finished->dw0.suc_eof = 1;
	finished->dw0.length = DMA_BUFFER_SIZE * sizeof(uint32_t);
	finished->dw0.size = DMA_BUFFER_SIZE * sizeof(uint32_t);
	finished->next = ((finished == &desc[0]) ? &desc[1] : &desc[0]);
	finished->dw0.owner = 1;
	// }

	ESP_LOGD("i2s", "start gdma %u", gdma_start(i2s_hal.dma_chan, (intptr_t)finished->next));
	//
	// 	break;
	// }

	return false;
}

/**
 * default mode transition
 */
static void i2s_enter_default_mode_glitch_free(void)
{
	// Pause motion timer
	timer_pause(ITP_TIMER_TG, ITP_TIMER_IDX);
	timer_disable_intr(ITP_TIMER_TG, ITP_TIMER_IDX);

	// Reset I2S
	i2s_hal_stop_tx(&i2s_hal.ctx);
	i2s_hal_reset_tx(&i2s_hal.ctx);
	i2s_hal_reset_rx(&i2s_hal.ctx);
	i2s_hal_reset_tx_fifo(&i2s_hal.ctx);
	i2s_hal_reset_rx_fifo(&i2s_hal.ctx);

	// Prefill
	i2s_fifo_fill_words(buf[0], DMA_BUFFER_SIZE);
	i2s_fifo_fill_words(buf[1], DMA_BUFFER_SIZE);
	desc[0].dw0.owner = 1;
	desc[0].dw0.suc_eof = 1;
	desc[1].dw0.owner = 1;
	desc[1].dw0.suc_eof = 1;

	// Start
	ESP_LOGD("i2s", "start gdma %u", gdma_start(i2s_hal.dma_chan, (intptr_t)&desc[0]));
	i2s_hal.running = true;
	i2s_hal_start_tx(&i2s_hal.ctx);
}

static void i2s_enter_realtime_mode_glitch_free(void)
{
	while (I2S_MODE & ITP_STEP_MODE_SYNC)
	{
		vPortYield();
	}
	ets_delay_us(2 * DMA_BUFFER_SIZE); // us per word * last buffer size
	i2s_hal_stop_tx(&i2s_hal.ctx);
	i2s_hal_reset_tx(&i2s_hal.ctx);
	i2s_hal_reset_tx_fifo(&i2s_hal.ctx);
	// Enter realtime
	I2S_REG.conf_single_data = __atomic_load_n(&ic74hc595_i2s_pins, __ATOMIC_RELAXED);
	i2s_hal_start_tx(&i2s_hal.ctx);
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

	i2s_hal_init(&i2s_hal.ctx, I2S_PORT);
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
		.chan_fmt = I2S_CHANNEL_FMT_ALL_LEFT,
		.sample_bits = I2S_BITS_PER_SAMPLE_32BIT,
		.chan_bits = I2S_BITS_PER_SAMPLE_32BIT,
		.active_chan = 1,
		.total_chan = 2};

	i2s_ll_tx_set_sample_bit(&I2S_REG, 32, 32);
	i2s_ll_tx_set_ws_idle_pol(&I2S_REG, 1);
	i2s_ll_tx_clk_set_src(&I2S_REG, I2S_CLK_D2CLK);
	i2s_hal_clock_cfg_t clock = {160000000, 64000000, 32000000, 2, 2}; // 1us per word
																	   // i2s_hal_clock_cfg_t clock = {160000000, 160000000,160000000, 1, 5 }; // 2us per word
																	   // i2s_hal_clock_cfg_t clock = {160000000, 32000000,16000000, 5, 2 }; // 2us per word also

	i2s_hal_tx_clock_config(&i2s_hal.ctx, &clock);
	i2s_hal_enable_module_clock(&i2s_hal.ctx);
	i2s_ll_tx_enable_clock(&I2S_REG);

	i2s_hal_config_param(&i2s_hal.ctx, &config);
	I2S_REG.int_ena.tx_done = 1;
	I2S_REG.int_ena.tx_hung = 1;

#ifndef USE_I2S_REALTIME_MODE_ONLY

	gdma_channel_alloc_config_t dma_cfg = {.direction = GDMA_CHANNEL_DIRECTION_TX};
#if I2S_PORT == 0
	gdma_trigger_t trig = GDMA_MAKE_TRIGGER(GDMA_TRIG_PERIPH_I2S, 0);
#else
	gdma_trigger_t trig = GDMA_MAKE_TRIGGER(GDMA_TRIG_PERIPH_I2S, 1);
#endif
	ESP_LOGD("i2s", "new channel %u", gdma_new_channel(&dma_cfg, &i2s_hal.dma_chan));
	ESP_LOGD("i2s", "connect %u", gdma_connect(i2s_hal.dma_chan, trig));
	gdma_tx_event_callbacks_t cb = {.on_trans_eof = i2s_dma_event_handler};
	ESP_LOGD("i2s", "reg cb %u", gdma_register_tx_event_callbacks(i2s_hal.dma_chan, &cb, &i2s_hal));

	// queue = xQueueCreate(DMA_BUFFER_SIZE * 2, sizeof(uint32_t *));

	// Setup descriptors
	desc[0].dw0.owner = 1;
	desc[0].dw0.suc_eof = 1;
	desc[0].dw0.length = DMA_BUFFER_SIZE * sizeof(uint32_t);
	desc[0].dw0.size = DMA_BUFFER_SIZE * sizeof(uint32_t);
	desc[0].buffer = (uint8_t *)&buf[0];
	desc[0].next = &desc[1];

	desc[1].dw0.owner = 1;
	desc[1].dw0.suc_eof = 1;
	desc[1].dw0.length = DMA_BUFFER_SIZE * sizeof(uint32_t);
	desc[1].dw0.size = DMA_BUFFER_SIZE * sizeof(uint32_t);
	desc[1].buffer = (uint8_t *)&buf[1];
	desc[1].next = &desc[0];

	ESP_LOGD("i2s", "address a %u", (uint32_t)&desc[0]);
	ESP_LOGD("i2s", "address b %u", (uint32_t)&desc[1]);
#else
	i2s_ll_tx_disable_intr(&I2S_REG);
#endif
}

// void i2stask(void *arg)
// {
// 	dma_descriptor_t *finished;
// 	for (;;)
// 	{
// 		ESP_LOGD("i2s", "wait for queue");
// 		xQueueReceive(queue, &finished, portMAX_DELAY);
// 		i2s_fifo_fill_words(finished->buffer, DMA_BUFFER_SIZE);
// 	}
// }

void mcu_i2s_extender_init(void)
{
	i2s_tx_base_config();
	// xTaskCreatePinnedToCore(i2stask, "i2stask", 8192, NULL, 7, NULL, CONFIG_ARDUINO_RUNNING_CORE);

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