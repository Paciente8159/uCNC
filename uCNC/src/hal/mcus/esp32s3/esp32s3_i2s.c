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
	int gdma_channel;
	gdma_channel_handle_t dma_chan;
	intr_handle_t intr_handle;
} i2s_hal_t;

static i2s_hal_t i2s_hal;

extern void mcu_itp_isr(void *arg);
extern void mcu_gen_pwm(void);
extern void mcu_gen_servo(void);
extern void mcu_gen_step(void);
extern void mcu_gpio_isr(void *type);

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
		*buf++ = __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED);
		// uint32_t data = __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED);
		// gdma_ll_tx_push_data(&GDMA, i2s_hal.gdma_channel, data);
	}
}

static void IRAM_ATTR i2s_dma_event_handler(void *arg)
{
	uint32_t irq = gdma_ll_tx_get_interrupt_status(&GDMA, i2s_hal.gdma_channel);
	dma_descriptor_t *finished = (dma_descriptor_t *)gdma_ll_tx_get_eof_desc_addr(&GDMA, i2s_hal.gdma_channel);
	mcu_toggle_output(DOUT49);

	memset(finished, 0, sizeof(dma_descriptor_t));
	finished->dw0.suc_eof = 1;
	finished->dw0.length = DMA_BUFFER_SIZE * 4;
	finished->dw0.size = DMA_BUFFER_SIZE * 4;
	uint32_t *buffer = ((finished == &desc[1]) ? &buf[0][0] : &buf[1][0]);
	finished->buffer = buffer;
	finished->next = (dma_descriptor_t *)((finished == &desc[1]) ? &desc[0] : &desc[1]);
	i2s_fifo_fill_words(buffer, DMA_BUFFER_SIZE);
	finished->dw0.owner = 1;

	gdma_ll_tx_clear_interrupt_status(&GDMA, i2s_hal.gdma_channel, irq);
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
	gdma_ll_tx_stop(&GDMA, i2s_hal.gdma_channel);

	i2s_ll_tx_reset(&I2S0);
	i2s_ll_tx_reset_fifo(&I2S0);

	// Prefill
	// gdma_ll_tx_reset_channel(&GDMA, i2s_hal.gdma_channel);
	memset(&desc[0], 0, sizeof(dma_descriptor_t));
	memset(&desc[1], 0, sizeof(dma_descriptor_t));
	desc[0].dw0.owner = 1;
	desc[0].dw0.suc_eof = 1;
	desc[0].dw0.length = DMA_BUFFER_SIZE * 4;
	desc[0].dw0.size = DMA_BUFFER_SIZE * 4;
	desc[0].buffer = &buf[0][0];
	desc[0].next = (dma_descriptor_t *)(&desc[1]);
	desc[1].dw0.owner = 1;
	desc[1].dw0.suc_eof = 1;
	desc[1].dw0.length = DMA_BUFFER_SIZE * 4;
	desc[1].dw0.size = DMA_BUFFER_SIZE * 4;
	desc[1].buffer = &buf[1][0];
	desc[1].next = (dma_descriptor_t *)(&desc[0]);
	i2s_fifo_fill_words(buf[0], DMA_BUFFER_SIZE);
	i2s_fifo_fill_words(buf[1], DMA_BUFFER_SIZE);

	// Start
	gdma_ll_tx_reset_channel(&GDMA, i2s_hal.gdma_channel);
	gdma_ll_tx_set_desc_addr(&GDMA, i2s_hal.gdma_channel, (uint32_t)(&desc[0]));
	//    gdma_ll_tx_enable_data_burst(&GDMA, i2s_sr.dma.channel, true);
	//    gdma_ll_tx_enable_descriptor_burst(&GDMA, i2s_sr.dma.channel, true);
	gdma_ll_tx_clear_interrupt_status(&GDMA, i2s_hal.gdma_channel, gdma_ll_tx_get_interrupt_status(&GDMA, i2s_hal.gdma_channel));
	gdma_ll_tx_enable_interrupt(&GDMA, i2s_hal.gdma_channel, GDMA_LL_EVENT_TX_DONE | GDMA_LL_EVENT_TX_TOTAL_EOF, true);

	//    I2S0.tx_conf.tx_stop_en = 1;  // BCK and WCK are suppressed while FIFO is empty - no ll func!

	I2S0.tx_conf.tx_update = 1;
	while (I2S0.tx_conf.tx_update)
		;

	//    i2s_ll_tx_start(&I2S0); // Fails?
	gdma_ll_tx_start(&GDMA, i2s_hal.gdma_channel);

	I2S0.tx_conf.tx_start = 1;
	//    I2S0.tx_conf.tx_start = 1;
	// Wait for the first FIFO data to prevent the unintentional generation of 0 data
	ets_delay_us(20);

	I2S0.tx_conf.tx_stop_en = 1; // BCK and WCK are suppressed while FIFO is empty - no ll func!
	ESP_LOGD("i2s", "start gdma");
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

static inline void gpio_matrix_out_check(uint8_t gpio, uint32_t signal_idx, bool out_inv, bool oen_inv)
{
	if (gpio != 255)
	{
		PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[gpio], PIN_FUNC_GPIO);
		gpio_set_direction((gpio_num_t)gpio, GPIO_MODE_OUTPUT);
		gpio_matrix_out(gpio, signal_idx, out_inv, oen_inv);
	}
}

static void IRAM_ATTR i2s_out_gpio_attach(uint8_t ws, uint8_t bck, uint8_t data)
{
	// Route the i2s pins to the appropriate GPIO
	gpio_matrix_out_check(data, I2S0O_SD_OUT_IDX, false, false);
	gpio_matrix_out_check(bck, I2S0O_BCK_OUT_IDX, false, false);
	gpio_matrix_out_check(ws, I2S0O_WS_OUT_IDX, false, false);
}

void i2sOutTask(void *arg)
{
	for (;;)
	{
		signal_timer.us_step = 4;
		ESP_LOGD("i2s", "loop");
		mcu_gen_step();
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}

static void i2s_tx_base_config(void)
{

	if ((i2s_hal.gdma_channel = allocate_dma_channel()) == -1)
		return;

	// To make sure hardware is enabled before any hardware register operations.
	periph_module_reset(PERIPH_I2S0_MODULE);
	periph_module_enable(PERIPH_I2S0_MODULE);
	//    periph_module_reset(PERIPH_GDMA_MODULE);
	//    periph_module_enable(PERIPH_GDMA_MODULE);

	if (REG_GET_BIT(SYSTEM_PERIP_CLK_EN1_REG, SYSTEM_DMA_CLK_EN) == 0)
	{
		REG_CLR_BIT(SYSTEM_PERIP_CLK_EN1_REG, SYSTEM_DMA_CLK_EN);
		REG_SET_BIT(SYSTEM_PERIP_CLK_EN1_REG, SYSTEM_DMA_CLK_EN);
		REG_SET_BIT(SYSTEM_PERIP_RST_EN1_REG, SYSTEM_DMA_RST);
		REG_CLR_BIT(SYSTEM_PERIP_RST_EN1_REG, SYSTEM_DMA_RST);
	}

	// Route the i2s pins to the appropriate GPIO
	i2s_out_gpio_attach(IC74HC595_I2S_WS, IC74HC595_I2S_CLK, IC74HC595_I2S_DATA);
	// i2s_clear_o_dma_buffers(ic74hc595_i2s_pins);
	i2s_ll_tx_stop(&I2S0);
	gdma_ll_enable_clock(&GDMA, 1);
	gdma_ll_tx_reset_channel(&GDMA, i2s_hal.gdma_channel);
	gdma_ll_tx_enable_interrupt(&GDMA, i2s_hal.gdma_channel, GDMA_LL_TX_EVENT_MASK, false);
	gdma_ll_tx_clear_interrupt_status(&GDMA, i2s_hal.gdma_channel, GDMA_LL_TX_EVENT_MASK);
	gdma_ll_tx_connect_to_periph(&GDMA, i2s_hal.gdma_channel, GDMA_TRIG_PERIPH_I2S, SOC_GDMA_TRIG_PERIPH_I2S0);
	gdma_ll_tx_set_eof_mode(&GDMA, i2s_hal.gdma_channel, 0);

	i2s_ll_mclk_div_t clk_ = {
		.mclk_div = 2,
		.a = 32,
		.b = 16};

	i2s_ll_tx_enable_pdm(&I2S0, false); // Enables TDM
	i2s_ll_rx_set_active_chan_mask(&I2S0, 1);
	i2s_ll_tx_enable_msb_shift(&I2S0, 0);
	i2s_ll_tx_set_sample_bit(&I2S0, 32, 32); // ?
	i2s_ll_tx_set_ws_width(&I2S0, 1);
	I2S0.tx_timing.tx_ws_out_dm = 1;

	//
	// i2s_set_clk
	//
	i2s_ll_tx_set_ws_idle_pol(&I2S0, 1);
	i2s_ll_tx_clk_set_src(&I2S0, I2S_CLK_D2CLK); // Set I2S_CLK_D2CLK as default
	i2s_ll_mclk_use_tx_clk(&I2S0);
	i2s_ll_tx_set_clk(&I2S0, &clk_);
	i2s_ll_tx_enable_clock(&I2S0);
	i2s_ll_tx_reset(&I2S0);
	i2s_ll_tx_reset_fifo(&I2S0);
	I2S0.tx_conf.tx_stop_en = 1;

	ESP_LOGD("i2s", "address a %u", (uint32_t)&desc[0]);
	ESP_LOGD("i2s", "address b %u", (uint32_t)&desc[1]);

	// Create the task that will feed the buffer
	// xTaskCreatePinnedToCore(i2sOutTask, "I2SOutTask", 4096, NULL, 7, NULL, ARDUINO_RUNNING_CORE);

	// Allocate and enable the I2S DMA interrupt

	esp_intr_alloc(gdma_periph_signals.groups[0].pairs[i2s_hal.gdma_channel].tx_irq_id, ESP_INTR_FLAG_IRAM, i2s_dma_event_handler, NULL, &i2s_hal.intr_handle);

	ESP_LOGD("i2s", "intr_handle %u", i2s_hal.intr_handle);
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