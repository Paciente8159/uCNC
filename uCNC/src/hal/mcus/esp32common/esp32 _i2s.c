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
#include "driver/i2s.h"
#include "../esp32common/esp32_common.h"

#ifdef IC74HC595_CUSTOM_SHIFT_IO
#if IC74HC595_COUNT != 4
#error "IC74HC595_COUNT must be 4(bytes) to use ESP32 I2S mode for IO shifting"
#endif

#ifndef I2S_SAMPLE_RATE
#define I2S_SAMPLE_RATE (F_STEP_MAX * 2)
#endif

#define I2S_SAMPLES_PER_BUFFER ((I2S_SAMPLE_RATE / 1000) < 8 ? 8 : (I2S_SAMPLE_RATE / 1000)) // number of samples per 1ms (0.001/1 = 1/1000)
#define I2S_BUFFER_COUNT 10																																	 // DMA buffer size 10 * 1ms = 10ms stored motions (can be adjusted but may cause to much or too little latency)
#define I2S_SAMPLE_US (1000000UL / I2S_SAMPLE_RATE)																					 // (1s/250KHz = 0.000004s = 4us)

#if I2S_BUFFER_COUNT < 2 || I2S_BUFFER_COUNT > 128
#error "I2S_BUFFER_COUNT must be between 2 and 128"
#endif

#if I2S_SAMPLES_PER_BUFFER < 8
#error "I2S_SAMPLES_PER_BUFFER must be >= 8"
#endif

#ifdef ITP_SAMPLE_RATE
#undef ITP_SAMPLE_RATE
#endif
#define ITP_SAMPLE_RATE (I2S_SAMPLE_RATE)

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

// implements the custom step mode function to switch between buffered stepping and realtime stepping
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
		cnc_delay_ms(20);
	}
	return last_mode;
}

#if defined(ESP32S3) || defined(ESP32C3)
static i2s_config_t i2s_config;
static i2s_pin_config_t pin_config;
static QueueHandle_t i2s_dma_queue;

static void IRAM_ATTR esp32_i2s_stream_task(void *param)
{
	int8_t available_buffers = I2S_BUFFER_COUNT;
	i2s_event_t evt;
	portTickType xLastWakeTimeUpload = xTaskGetTickCount();

	for (;;)
	{
		uint32_t mode = I2S_MODE;

		// Track DMA buffer usage
		if (available_buffers < I2S_BUFFER_COUNT)
		{
			while (xQueueReceive(i2s_dma_queue, &evt, 0) == pdPASS)
			{
				if (evt.type == I2S_EVENT_TX_DONE)
				{
					available_buffers++;
				}
			}
		}

		// Handle sync mode (reset driver cleanly)
		if (mode & ITP_STEP_MODE_SYNC)
		{
			while (available_buffers < I2S_BUFFER_COUNT)
			{
				while (xQueueReceive(i2s_dma_queue, &evt, 0) == pdPASS)
				{
					if (evt.type == I2S_EVENT_TX_DONE)
					{
						available_buffers++;
					}
				}
				vTaskDelayUntil(&xLastWakeTimeUpload, (20 / portTICK_RATE_MS));
			}

			// Reinstall driver to clear state
			i2s_driver_uninstall(IC74HC595_I2S_PORT);
			i2s_driver_install(IC74HC595_I2S_PORT, &i2s_config, I2S_BUFFER_COUNT, &i2s_dma_queue);
			i2s_set_pin(IC74HC595_I2S_PORT, &pin_config);

			available_buffers = I2S_BUFFER_COUNT;
			__atomic_fetch_and((uint32_t *)&i2s_mode, ~ITP_STEP_MODE_SYNC, __ATOMIC_RELAXED);
		}

		// Default buffered mode
		while ((mode & ~ITP_STEP_MODE_SYNC) == ITP_STEP_MODE_DEFAULT && available_buffers > 0)
		{
			uint32_t i2s_dma_buffer[I2S_SAMPLES_PER_BUFFER];

			for (uint32_t t = 0; t < I2S_SAMPLES_PER_BUFFER; t++)
			{
#if defined(IC74HC595_HAS_STEPS) || defined(IC74HC595_HAS_DIRS)
				mcu_gen_step();
#endif
#if defined(IC74HC595_HAS_PWMS) || defined(IC74HC595_HAS_SERVOS)
				mcu_gen_pwm_and_servo();
#endif
#if defined(MCU_HAS_ONESHOT_TIMER) && defined(ENABLE_RT_SYNC_MOTIONS)
				mcu_gen_oneshot();
#endif

				i2s_dma_buffer[t] = __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED);
			}

			size_t written = 0;
			i2s_write(IC74HC595_I2S_PORT, i2s_dma_buffer,
								I2S_SAMPLES_PER_BUFFER * sizeof(uint32_t),
								&written, portMAX_DELAY);

			available_buffers--;
		}

		// Realtime mode fallback: just stream one sample at a time
		if ((mode & ~ITP_STEP_MODE_SYNC) == ITP_STEP_MODE_REALTIME)
		{
			uint32_t sample = __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED);

			size_t written = 0;
			i2s_write(IC74HC595_I2S_PORT, &sample, sizeof(sample), &written, 0);

			// ets_delay_us(I2S_SAMPLE_US > 2 ? (I2S_SAMPLE_US - 2) : 0);
		}

		vTaskDelayUntil(&xLastWakeTimeUpload, (5 / portTICK_RATE_MS));
	}
}

#else

static void IRAM_ATTR esp32_i2s_stream_task(void *param)
{
	int8_t available_buffers = I2S_BUFFER_COUNT;
	i2s_event_t evt;
	portTickType xLastWakeTimeUpload = xTaskGetTickCount();
	i2s_config_t i2s_config = {
			.mode = I2S_MODE_MASTER | I2S_MODE_TX, // Only TX
			.sample_rate = I2S_SAMPLE_RATE,
			.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
			.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // 1-channels
			.communication_format = I2S_COMM_FORMAT_STAND_I2S | I2S_COMM_FORMAT_STAND_MSB,
			.dma_buf_count = I2S_BUFFER_COUNT,
			.dma_buf_len = I2S_SAMPLES_PER_BUFFER,
			.use_apll = false,
			.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Interrupt level 1
			.tx_desc_auto_clear = false,
			.fixed_mclk = 0};

	i2s_pin_config_t pin_config = {
			.bck_io_num = IC74HC595_I2S_CLK,
			.ws_io_num = IC74HC595_I2S_WS,
			.data_out_num = IC74HC595_I2S_DATA,
			.data_in_num = -1 // Not used
	};
	QueueHandle_t i2s_dma_queue;

	i2s_driver_install(IC74HC595_I2S_PORT, &i2s_config, I2S_BUFFER_COUNT, &i2s_dma_queue);
	i2s_set_pin(IC74HC595_I2S_PORT, &pin_config);

	for (;;)
	{
		uint32_t mode = I2S_MODE;

		// tracks DMA buffer usage
		if (available_buffers < I2S_BUFFER_COUNT)
		{
			while (xQueueReceive(i2s_dma_queue, &evt, 0) == pdPASS)
			{
				if (evt.type == I2S_EVENT_TX_DONE)
				{
					available_buffers++;
				}
			}
		}

		// updates the working mode
		if (mode & ITP_STEP_MODE_SYNC)
		{
			// wait for DMA to output content
			while (available_buffers < I2S_BUFFER_COUNT)
			{
				// tracks DMA buffer usage
				while (xQueueReceive(i2s_dma_queue, &evt, 0) == pdPASS)
				{
					if (evt.type == I2S_EVENT_TX_DONE)
					{
						available_buffers++;
					}
				}
				vTaskDelayUntil(&xLastWakeTimeUpload, (20 / portTICK_RATE_MS));
			}

			switch (mode & ~ITP_STEP_MODE_SYNC)
			{
			case ITP_STEP_MODE_DEFAULT:
				// timer_pause(ITP_TIMER_TG, ITP_TIMER_IDX);
				// timer_disable_intr(ITP_TIMER_TG, ITP_TIMER_IDX);
				I2SREG.conf.tx_start = 0;
				I2SREG.conf.tx_reset = 1;
				I2SREG.conf.tx_reset = 0;
				I2SREG.conf.rx_fifo_reset = 1;
				I2SREG.conf.rx_fifo_reset = 0;
				available_buffers = I2S_BUFFER_COUNT;
				i2s_driver_uninstall(IC74HC595_I2S_PORT);
				i2s_driver_install(IC74HC595_I2S_PORT, &i2s_config, I2S_BUFFER_COUNT, &i2s_dma_queue);
				i2s_set_pin(IC74HC595_I2S_PORT, &pin_config);
				break;
			case ITP_STEP_MODE_REALTIME:
				I2SREG.conf.tx_start = 0;
				I2SREG.conf.tx_reset = 1;
				I2SREG.conf.tx_reset = 0;
				I2SREG.conf.rx_fifo_reset = 1;
				I2SREG.conf.rx_fifo_reset = 0;
				available_buffers = I2S_BUFFER_COUNT;
				// modify registers for realtime usage
				I2SREG.out_link.stop = 1;
				I2SREG.fifo_conf.dscr_en = 0;
				I2SREG.conf.tx_start = 0;
				I2SREG.int_clr.val = 0xFFFFFFFF;
				I2SREG.clkm_conf.clka_en = 0;			 // Use PLL/2 as reference
				I2SREG.clkm_conf.clkm_div_num = 2; // reset value of 4
				I2SREG.clkm_conf.clkm_div_a = 1;	 // 0 at reset, what about divide by 0?
				I2SREG.clkm_conf.clkm_div_b = 0;	 // 0 at reset
				I2SREG.fifo_conf.tx_fifo_mod = 3;	 // 32 bits single channel data
				I2SREG.conf_chan.tx_chan_mod = 3;	 //
				I2SREG.sample_rate_conf.tx_bits_mod = 32;
				I2SREG.conf.tx_msb_shift = 0;
				I2SREG.conf.rx_msb_shift = 0;
				I2SREG.int_ena.out_eof = 0;
				I2SREG.int_ena.out_dscr_err = 0;
				I2SREG.conf_single_data = __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED);
				I2SREG.conf1.tx_stop_en = 0;
				I2SREG.int_ena.val = 0;
				I2SREG.fifo_conf.dscr_en = 1;
				I2SREG.int_clr.val = 0xFFFFFFFF;
				I2SREG.out_link.start = 1;
				I2SREG.conf.tx_start = 1;
				ets_delay_us(20);
				break;
			}

			// clear sync flag
			__atomic_fetch_and((uint32_t *)&i2s_mode, ~ITP_STEP_MODE_SYNC, __ATOMIC_RELAXED);
		}

		while (mode == ITP_STEP_MODE_DEFAULT && available_buffers > 0)
		{
			uint32_t i2s_dma_buffer[I2S_SAMPLES_PER_BUFFER];

			for (uint32_t t = 0; t < I2S_SAMPLES_PER_BUFFER; t++)
			{
#if defined(IC74HC595_HAS_STEPS) || defined(IC74HC595_HAS_DIRS)
				mcu_gen_step();
#endif
#if defined(IC74HC595_HAS_PWMS) || defined(IC74HC595_HAS_SERVOS)
				mcu_gen_pwm_and_servo();
#endif
#if defined(MCU_HAS_ONESHOT_TIMER) && defined(ENABLE_RT_SYNC_MOTIONS)
				mcu_gen_oneshot();
#endif
				// write to buffer
				i2s_dma_buffer[t] = __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED);
			}

			uint32_t w = 0;

			i2s_write(IC74HC595_I2S_PORT, &i2s_dma_buffer[0], I2S_SAMPLES_PER_BUFFER * 4, &w, portMAX_DELAY);
			available_buffers--;
		}

		vTaskDelayUntil(&xLastWakeTimeUpload, (5 / portTICK_RATE_MS));
	}
}
#endif

void mcu_i2s_extender_init(void)
{
#if defined(ESP32S3) || defined(ESP32C3)
	i2s_config.mode = I2S_MODE_MASTER | I2S_MODE_TX;
	i2s_config.sample_rate = I2S_SAMPLE_RATE;
	i2s_config.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
	i2s_config.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
	i2s_config.communication_format = I2S_COMM_FORMAT_STAND_I2S | I2S_COMM_FORMAT_STAND_MSB;
	i2s_config.dma_buf_count = I2S_BUFFER_COUNT;
	i2s_config.dma_buf_len = I2S_SAMPLES_PER_BUFFER;
	i2s_config.use_apll = false;
	i2s_config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
	i2s_config.tx_desc_auto_clear = true;
	i2s_config.fixed_mclk = 0;

	pin_config.bck_io_num = IC74HC595_I2S_CLK;
	pin_config.ws_io_num = IC74HC595_I2S_WS;
	pin_config.data_out_num = IC74HC595_I2S_DATA;
	pin_config.data_in_num = -1;

	i2s_driver_install(IC74HC595_I2S_PORT, &i2s_config, I2S_BUFFER_COUNT, &i2s_dma_queue);
	i2s_set_pin(IC74HC595_I2S_PORT, &pin_config);
#endif

#ifdef USE_I2S_REALTIME_MODE_ONLY
	itp_set_step_mode(ITP_STEP_MODE_REALTIME);
#else
	itp_set_step_mode(ITP_STEP_MODE_DEFAULT);
#endif
	xTaskCreatePinnedToCore(esp32_i2s_stream_task, "esp32I2Supdate", 8192, NULL, 7, NULL, CONFIG_ARDUINO_RUNNING_CORE);
}

#endif
#endif