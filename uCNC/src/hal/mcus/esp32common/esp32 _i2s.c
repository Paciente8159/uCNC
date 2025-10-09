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
#include "soc/i2s_reg.h"
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

#include "soc/soc.h"
#include "driver/i2s.h"
#include "soc/i2s_reg.h"

// Assumes:
// - IC74HC595_I2S_PORT, IC74HC595_I2S_CLK, IC74HC595_I2S_WS, IC74HC595_I2S_DATA
// - I2S_SAMPLE_RATE, I2S_SAMPLES_PER_BUFFER, I2S_BUFFER_COUNT
// - ic74hc595_i2s_pins, i2s_mode, ITP_STEP_MODE_DEFAULT, ITP_STEP_MODE_REALTIME, ITP_STEP_MODE_SYNC
// - MCU callbacks: mcu_gen_step(), mcu_gen_pwm_and_servo(), mcu_gen_oneshot() (if enabled)
// - Atomic ops used as in original
// - ets_delay_us available

static void IRAM_ATTR esp32_i2s_stream_task(void *param) {
    int8_t available_buffers = I2S_BUFFER_COUNT;
    i2s_event_t evt;
    portTickType xLastWakeTimeUpload = xTaskGetTickCount();

    // Same DMA config and pin config as original
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S | I2S_COMM_FORMAT_STAND_MSB,
        .dma_buf_count = I2S_BUFFER_COUNT,
        .dma_buf_len = I2S_SAMPLES_PER_BUFFER,
        .use_apll = false,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num   = IC74HC595_I2S_CLK,
        .ws_io_num    = IC74HC595_I2S_WS,
        .data_out_num = IC74HC595_I2S_DATA,
        .data_in_num  = -1
    };

    QueueHandle_t i2s_dma_queue;
    i2s_driver_install(IC74HC595_I2S_PORT, &i2s_config, I2S_BUFFER_COUNT, &i2s_dma_queue);
    i2s_set_pin(IC74HC595_I2S_PORT, &pin_config);

    for (;;) {
        uint32_t mode = __atomic_load_n((uint32_t *)&i2s_mode, __ATOMIC_RELAXED);

        // Track DMA buffer usage (DEFAULT mode path)
        if (available_buffers < I2S_BUFFER_COUNT) {
            while (xQueueReceive(i2s_dma_queue, &evt, 0) == pdPASS) {
                if (evt.type == I2S_EVENT_TX_DONE) {
                    available_buffers++;
                }
            }
        }

        // Mode update handling (sync)
        if (mode & ITP_STEP_MODE_SYNC) {
            // Drain DMA before switching
            while (available_buffers < I2S_BUFFER_COUNT) {
                while (xQueueReceive(i2s_dma_queue, &evt, 0) == pdPASS) {
                    if (evt.type == I2S_EVENT_TX_DONE) {
                        available_buffers++;
                    }
                }
                vTaskDelayUntil(&xLastWakeTimeUpload, (20 / portTICK_RATE_MS));
            }

            switch (mode & ~ITP_STEP_MODE_SYNC) {
                case ITP_STEP_MODE_DEFAULT: {
                    // Stop TX, reset FIFOs (conf.tx_start=0, tx_reset=1->0, rx_fifo_reset=1->0)
                    // I2S_CONF bits: TX_START(4), TX_RESET(0), RX_FIFO_RESET(3)
                    // Clear then re-install driver/pins
                    // Stop TX
                    CLEAR_PERI_REG_MASK(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_TX_START);
                    // TX reset pulse
                    SET_PERI_REG_MASK(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_TX_RESET);
                    CLEAR_PERI_REG_MASK(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_TX_RESET);
                    // RX FIFO reset pulse
                    SET_PERI_REG_MASK(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_RX_FIFO_RESET);
                    CLEAR_PERI_REG_MASK(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_RX_FIFO_RESET);

                    available_buffers = I2S_BUFFER_COUNT;

                    i2s_driver_uninstall(IC74HC595_I2S_PORT);
                    i2s_driver_install(IC74HC595_I2S_PORT, &i2s_config, I2S_BUFFER_COUNT, &i2s_dma_queue);
                    i2s_set_pin(IC74HC595_I2S_PORT, &pin_config);
                    break;
                }

                case ITP_STEP_MODE_REALTIME: {
                    // Stop/Reset as above
                    CLEAR_PERI_REG_MASK(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_TX_START);
                    SET_PERI_REG_MASK(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_TX_RESET);
                    CLEAR_PERI_REG_MASK(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_TX_RESET);
                    SET_PERI_REG_MASK(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_RX_FIFO_RESET);
                    CLEAR_PERI_REG_MASK(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_RX_FIFO_RESET);

                    available_buffers = I2S_BUFFER_COUNT;

                    // REALTIME reconfiguration using “single data” and DMA out-link gated
                    // Stop out link and disable descriptor engine
                    SET_PERI_REG_MASK(I2S_OUT_LINK_REG(IC74HC595_I2S_PORT), I2S_OUTLINK_STOP);
                    CLEAR_PERI_REG_MASK(I2S_FIFO_CONF_REG(IC74HC595_I2S_PORT), I2S_DSCR_EN);

                    // Ensure TX stopped, clear all interrupts
                    CLEAR_PERI_REG_MASK(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_TX_START);
                    WRITE_PERI_REG(I2S_INT_CLR_REG(IC74HC595_I2S_PORT), 0xFFFFFFFF);

                    // Clock config: use PLL/2 reference, set clkm divs to match original intent
                    // I2S_CLKM_CONF: CLKA_ENA(21), DIV_NUM(7:0), DIV_A(19:14), DIV_B(13:8)
                    // Disable CLKA (use internal PLL/2 path), DIV_NUM = 2, DIV_A = 1, DIV_B = 0
                    CLEAR_PERI_REG_MASK(I2S_CLKM_CONF_REG(IC74HC595_I2S_PORT), I2S_CLKA_ENA);
                    // Write fields atomically: clear then set
                    uint32_t clkm = READ_PERI_REG(I2S_CLKM_CONF_REG(IC74HC595_I2S_PORT));
                    clkm &= ~(I2S_CLKM_DIV_NUM_M | I2S_CLKM_DIV_A_M | I2S_CLKM_DIV_B_M);
                    clkm |= ((2  << I2S_CLKM_DIV_NUM_S) & I2S_CLKM_DIV_NUM_M);
                    clkm |= ((1  << I2S_CLKM_DIV_A_S)   & I2S_CLKM_DIV_A_M);
                    clkm |= ((0  << I2S_CLKM_DIV_B_S)   & I2S_CLKM_DIV_B_M);
                    WRITE_PERI_REG(I2S_CLKM_CONF_REG(IC74HC595_I2S_PORT), clkm);

                    // FIFO and channel mode: 32-bit single-channel
                    // I2S_FIFO_CONF: TX_FIFO_MOD(15:13) = 3
                    uint32_t fifo_conf = READ_PERI_REG(I2S_FIFO_CONF_REG(IC74HC595_I2S_PORT));
                    fifo_conf &= ~I2S_TX_FIFO_MOD_M;
                    fifo_conf |= ((3 << I2S_TX_FIFO_MOD_S) & I2S_TX_FIFO_MOD_M);
                    WRITE_PERI_REG(I2S_FIFO_CONF_REG(IC74HC595_I2S_PORT), fifo_conf);

                    // I2S_CONF_CHAN: TX_CHAN_MOD(2:0) = 3
                    uint32_t conf_chan = READ_PERI_REG(I2S_CONF_CHAN_REG(IC74HC595_I2S_PORT));
                    conf_chan &= ~I2S_TX_CHAN_MOD_M;
                    conf_chan |= ((3 << I2S_TX_CHAN_MOD_S) & I2S_TX_CHAN_MOD_M);
                    WRITE_PERI_REG(I2S_CONF_CHAN_REG(IC74HC595_I2S_PORT), conf_chan);

                    // Sample bits: TX/RX MSB shift off, 32-bit mode
                    // I2S_CONF: TX_MSB_SHIFT(10)=0, RX_MSB_SHIFT(11)=0
                    CLEAR_PERI_REG_MASK(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_TX_MSB_SHIFT | I2S_RX_MSB_SHIFT);

                    // I2S_SAMPLE_RATE_CONF: TX_BITS_MOD(17:12)=32
                    uint32_t srate = READ_PERI_REG(I2S_SAMPLE_RATE_CONF_REG(IC74HC595_I2S_PORT));
                    srate &= ~I2S_TX_BITS_MOD_M;
                    srate |= ((32 << I2S_TX_BITS_MOD_S) & I2S_TX_BITS_MOD_M);
                    WRITE_PERI_REG(I2S_SAMPLE_RATE_CONF_REG(IC74HC595_I2S_PORT), srate);

                    // Disable specific interrupts used in DMA streaming path
                    uint32_t int_ena = READ_PERI_REG(I2S_INT_ENA_REG(IC74HC595_I2S_PORT));
                    int_ena &= ~(I2S_OUT_EOF_INT_ENA | I2S_OUT_DSCR_ERR_INT_ENA);
                    WRITE_PERI_REG(I2S_INT_ENA_REG(IC74HC595_I2S_PORT), int_ena);

                    // Push current pins to single data register
                    WRITE_PERI_REG(I2S_CONF_SIGLE_DATA_REG(IC74HC595_I2S_PORT),
                                   __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED));

                    // Ensure no auto stop
                    uint32_t conf1 = READ_PERI_REG(I2S_CONF1_REG(IC74HC595_I2S_PORT));
                    conf1 &= ~I2S_TX_STOP_EN;
                    WRITE_PERI_REG(I2S_CONF1_REG(IC74HC595_I2S_PORT), conf1);

                    // Re-enable descriptor engine and clear interrupts again
                    SET_PERI_REG_MASK(I2S_FIFO_CONF_REG(IC74HC595_I2S_PORT), I2S_DSCR_EN);
                    WRITE_PERI_REG(I2S_INT_CLR_REG(IC74HC595_I2S_PORT), 0xFFFFFFFF);

                    // Start out-link and TX
                    SET_PERI_REG_MASK(I2S_OUT_LINK_REG(IC74HC595_I2S_PORT), I2S_OUTLINK_START);
                    SET_PERI_REG_MASK(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_TX_START);

                    ets_delay_us(20);
                    break;
                }
            }

            // Clear sync flag
            __atomic_fetch_and((uint32_t *)&i2s_mode, ~ITP_STEP_MODE_SYNC, __ATOMIC_RELAXED);
        }

        // DEFAULT buffered streaming: fill buffers and write via DMA
        while (mode == ITP_STEP_MODE_DEFAULT && available_buffers > 0) {
            uint32_t i2s_dma_buffer[I2S_SAMPLES_PER_BUFFER];

            for (uint32_t t = 0; t < I2S_SAMPLES_PER_BUFFER; t++) {
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

            uint32_t w = 0;
            i2s_write(IC74HC595_I2S_PORT, &i2s_dma_buffer[0], I2S_SAMPLES_PER_BUFFER * 4, &w, portMAX_DELAY);
            available_buffers--;
        }

        vTaskDelayUntil(&xLastWakeTimeUpload, (5 / portTICK_RATE_MS));
    }
}

// static inline void i2s_tx_start(void) {
//     REG_SET_BIT(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_TX_START);
// }

// static inline void i2s_tx_stop_reset(void) {
//     // Stop TX
//     REG_CLR_BIT(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_TX_START);
//     // Reset TX FIFO
//     REG_SET_BIT(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_TX_FIFO_RESET);
//     REG_CLR_BIT(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_TX_FIFO_RESET);
//     // Reset TX core
//     REG_SET_BIT(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_TX_RESET);
//     REG_CLR_BIT(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_TX_RESET);
//     // Clear interrupts
//     REG_WRITE(I2S_INT_CLR_REG(IC74HC595_I2S_PORT), 0xFFFFFFFF);
// }

// static inline void i2s_config_32bit_mono(void) {
//     // Word size 32-bit
//     REG_SET_FIELD(I2S_SAMPLE_RATE_CONF_REG(IC74HC595_I2S_PORT), I2S_TX_BITS_MOD, 31);
//     // Channel mode mono
//     REG_SET_FIELD(I2S_CONF_CHAN_REG(IC74HC595_I2S_PORT), I2S_TX_CHAN_MOD, 3);
//     REG_SET_BIT(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_TX_MONO);
//     // MSB/right-first formatting
//     REG_CLR_BIT(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_TX_MSB_SHIFT);
//     REG_SET_BIT(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_TX_RIGHT_FIRST);
//     REG_SET_BIT(I2S_CONF_REG(IC74HC595_I2S_PORT), I2S_TX_MSB_RIGHT);
//     // FIFO mode selection and disable DMA descriptors
//     REG_SET_FIELD(I2S_FIFO_CONF_REG(IC74HC595_I2S_PORT), I2S_TX_FIFO_MOD, 3);
//     REG_CLR_BIT(I2S_FIFO_CONF_REG(IC74HC595_I2S_PORT), I2S_DSCR_EN);
//     // Enable I2S peripheral clock
//     REG_SET_BIT(I2S_CLKM_CONF_REG(IC74HC595_I2S_PORT), I2S_CLK_EN);
// }

// static inline void i2s_write_conf_single_data(uint32_t sample) {
//     // Non-DPORT register: safe with REG_WRITE
//     REG_WRITE(I2S_CONF_SIGLE_DATA_REG(IC74HC595_I2S_PORT), sample);
// }

// static inline void i2s_push_fifo_word(uint32_t word) {
//     // This register exposes WDATA bits [8:0]. For full-width streaming, rely on CONF_SIGLE_DATA (realtime) or DMA.
//     // Included for completeness; not used in realtime path.
//     uint32_t v = ((1U << I2S_OUTFIFO_PUSH_S) & I2S_OUTFIFO_PUSH_M)               // push bit
//                | ((word & I2S_OUTFIFO_WDATA_V) << I2S_OUTFIFO_WDATA_S);          // lower 9 bits
//     REG_WRITE(I2S_OUTFIFO_PUSH_REG(IC74HC595_I2S_PORT), v);
// }

// static inline void i2s_push_outfifo_word_lower9(uint32_t word) {
//     // DPORT register: must use DPORT_REG_WRITE on ESP32 classic
//     uint32_t v = I2S_OUTFIFO_PUSH_M                      // push bit (bit 16)
//                | ((word & I2S_OUTFIFO_WDATA_V)           // lower 9 bits payload
//                   << I2S_OUTFIFO_WDATA_S);
//     DPORT_REG_WRITE(I2S_OUTFIFO_PUSH_REG(IC74HC595_I2S_PORT), v);
// }

// static void IRAM_ATTR esp32_i2s_stream_task(void *param) {
//     i2s_tx_stop_reset();
//     i2s_config_32bit_mono();

//     for (;;) {
//         uint32_t mode = i2s_mode;

//         // Sync transition: stop + reset
//         if (mode & ITP_STEP_MODE_SYNC) {
//             i2s_tx_stop_reset();
//             __atomic_fetch_and(&i2s_mode, ~ITP_STEP_MODE_SYNC, __ATOMIC_RELAXED);
//         }

//         // Buffered mode: generate a small batch and (optionally) push via FIFO or re-use single word API in a loop
//         if ((mode & ~ITP_STEP_MODE_SYNC) == ITP_STEP_MODE_DEFAULT) {
//             uint32_t buf[I2S_SAMPLES_PER_BUFFER];
//             for (uint32_t t = 0; t < I2S_SAMPLES_PER_BUFFER; t++) {
//                 mcu_gen_step();
//                 mcu_gen_pwm_and_servo();
//                 buf[t] = __atomic_load_n(&ic74hc595_i2s_pins, __ATOMIC_RELAXED);
//             }

//             // For wide words without DMA, prefer single-data writes per sample; this retains deterministic timing.
//             for (uint32_t t = 0; t < I2S_SAMPLES_PER_BUFFER; t++) {
//                 i2s_write_conf_single_data(buf[t]);    // 32-bit word write
//             }
//             i2s_tx_start();
//         }

//         // // Realtime mode: write the latest latched word directly
//         // if ((mode & ~ITP_STEP_MODE_SYNC) == ITP_STEP_MODE_REALTIME) {
//         //     mcu_gen_step();
//         //     mcu_gen_pwm_and_servo();
//         //     uint32_t sample = __atomic_load_n(&ic74hc595_i2s_pins, __ATOMIC_RELAXED);
//         //     i2s_write_conf_single_data(sample);        // single word write
//         //     i2s_tx_start();
//         // }

//         // Pace the loop modestly (adjust for your scheduler)
//         vTaskDelay(pdMS_TO_TICKS(2));
//     }
// }

// static volatile uint32_t i2s_latched;
// static volatile uint32_t i2s_diff_prev;
// void mcu_i2s_write()
// {
// 	static uint32_t sample_prev = 0;
// 	static uint32_t diff = 0;
// 	uint32_t sample = __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED);
// 	diff = (sample_prev ^ sample);
// 	sample_prev = sample;
// 	uint32_t checkdiff = __atomic_fetch_or(&i2s_diff_prev, diff, __ATOMIC_ACQ_REL);
// 	while(checkdiff & diff){ // a bit is being changed a second time
// 		checkdiff = __atomic_load_n(&i2s_diff_prev, __ATOMIC_ACQUIRE);
// 	}

// 	i2s_latched = sample;
// }

// #if defined(ESP32) || defined (ESP32S3)
// void IRAM_ATTR __attribute__((naked)) i2s_fifo_isr(void) {
//     asm volatile (
//         // Load latched value
//         "l32i.n a2, a0, i2s_latched\n"     // a2 = i2s_latched

//         // Write to FIFO
//         "s32i.n a2, a0, %[fifo]\n"

//         // Clear in-flight diff bits (simple store of 0)
//         "movi.n a2, 0\n"
//         "s32i.n a2, a0, i2s_diff_prev\n"

//         // Clear interrupt
//         "movi.n a2, %[mask]\n"
//         "s32i.n a2, a0, %[int_clr]\n"

//         // Return from interrupt
//         "rsr     a0, EXCSAVE_1\n"
//         "rfi     1\n"
//         :
//         : [fifo] "i" (I2S_TX_FIFO_WR_REG(0)),
//           [int_clr] "i" (I2S_INT_CLR_REG(0)),
//           [mask] "i" (I2S_INTR_TX_REMPTY)
//         : "a2"
//     );
// }
// #elif defined (ESP32C3)
// void __attribute__((naked)) i2s_fifo_isr(void) {
//     asm volatile (
//         // Load latched value
//         "lw t0, i2s_latched\n"

//         // Write to FIFO
//         "sw t0, %[fifo]\n"

//         // Clear diff mask
//         "sw x0, i2s_diff_prev\n"

//         // Clear interrupt
//         "li t0, %[mask]\n"
//         "sw t0, %[int_clr]\n"

//         // Return from interrupt
//         "mret\n"
//         :
//         : [fifo] "i" (&I2S0.fifo_wr),
//           [int_clr] "i" (&I2S0.int_clr),
//           [mask] "i" (I2S_INTR_TX_REMPTY)
//         : "t0"
//     );
// }
// #endif

// #if defined(ESP32S3) || defined(ESP32C3)
// static i2s_config_t i2s_config;
// static i2s_pin_config_t pin_config;
// static QueueHandle_t i2s_dma_queue;

// static void IRAM_ATTR esp32_i2s_stream_task(void *param)
// {
// 	int8_t available_buffers = I2S_BUFFER_COUNT;
// 	i2s_event_t evt;
// 	portTickType xLastWakeTimeUpload = xTaskGetTickCount();

// 	for (;;)
// 	{
// 		uint32_t mode = I2S_MODE;

// 		// Track DMA buffer usage
// 		if (available_buffers < I2S_BUFFER_COUNT)
// 		{
// 			while (xQueueReceive(i2s_dma_queue, &evt, 0) == pdPASS)
// 			{
// 				if (evt.type == I2S_EVENT_TX_DONE)
// 				{
// 					available_buffers++;
// 				}
// 			}
// 		}

// 		// Handle sync mode (reset driver cleanly)
// 		if (mode & ITP_STEP_MODE_SYNC)
// 		{
// 			while (available_buffers < I2S_BUFFER_COUNT)
// 			{
// 				while (xQueueReceive(i2s_dma_queue, &evt, 0) == pdPASS)
// 				{
// 					if (evt.type == I2S_EVENT_TX_DONE)
// 					{
// 						available_buffers++;
// 					}
// 				}
// 				vTaskDelayUntil(&xLastWakeTimeUpload, (20 / portTICK_RATE_MS));
// 			}

// 			// Reinstall driver to clear state
// 			i2s_driver_uninstall(IC74HC595_I2S_PORT);
// 			i2s_driver_install(IC74HC595_I2S_PORT, &i2s_config, I2S_BUFFER_COUNT, &i2s_dma_queue);
// 			i2s_set_pin(IC74HC595_I2S_PORT, &pin_config);

// 			available_buffers = I2S_BUFFER_COUNT;
// 			__atomic_fetch_and((uint32_t *)&i2s_mode, ~ITP_STEP_MODE_SYNC, __ATOMIC_RELAXED);
// 		}

// 		// Default buffered mode
// 		while ((mode & ~ITP_STEP_MODE_SYNC) == ITP_STEP_MODE_DEFAULT && available_buffers > 0)
// 		{
// 			uint32_t i2s_dma_buffer[I2S_SAMPLES_PER_BUFFER];

// 			for (uint32_t t = 0; t < I2S_SAMPLES_PER_BUFFER; t++)
// 			{
// #if defined(IC74HC595_HAS_STEPS) || defined(IC74HC595_HAS_DIRS)
// 				mcu_gen_step();
// #endif
// #if defined(IC74HC595_HAS_PWMS) || defined(IC74HC595_HAS_SERVOS)
// 				mcu_gen_pwm_and_servo();
// #endif
// #if defined(MCU_HAS_ONESHOT_TIMER) && defined(ENABLE_RT_SYNC_MOTIONS)
// 				mcu_gen_oneshot();
// #endif

// 				i2s_dma_buffer[t] = __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED);
// 			}

// 			size_t written = 0;
// 			i2s_write(IC74HC595_I2S_PORT, i2s_dma_buffer,
// 								I2S_SAMPLES_PER_BUFFER * sizeof(uint32_t),
// 								&written, portMAX_DELAY);

// 			available_buffers--;
// 		}

// 		// // Realtime mode fallback: just stream one sample at a time
// 		// if ((mode & ~ITP_STEP_MODE_SYNC) == ITP_STEP_MODE_REALTIME)
// 		// {
// 		// 	uint32_t sample = __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED);
// 		// 	REG_WRITE(I2S_CONF_SIGLE_DATA_REG(0), value);
// 		// 	size_t written = 0;
// 		// 	i2s_write(IC74HC595_I2S_PORT, &sample, sizeof(sample), &written, 0);

// 		// 	// ets_delay_us(I2S_SAMPLE_US > 2 ? (I2S_SAMPLE_US - 2) : 0);
// 		// }

// 		vTaskDelayUntil(&xLastWakeTimeUpload, (5 / portTICK_RATE_MS));
// 	}
// }

// #else

// static void IRAM_ATTR esp32_i2s_stream_task(void *param)
// {
// 	int8_t available_buffers = I2S_BUFFER_COUNT;
// 	i2s_event_t evt;
// 	portTickType xLastWakeTimeUpload = xTaskGetTickCount();
// 	i2s_config_t i2s_config = {
// 			.mode = I2S_MODE_MASTER | I2S_MODE_TX, // Only TX
// 			.sample_rate = I2S_SAMPLE_RATE,
// 			.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
// 			.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // 1-channels
// 			.communication_format = I2S_COMM_FORMAT_STAND_I2S | I2S_COMM_FORMAT_STAND_MSB,
// 			.dma_buf_count = I2S_BUFFER_COUNT,
// 			.dma_buf_len = I2S_SAMPLES_PER_BUFFER,
// 			.use_apll = false,
// 			.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Interrupt level 1
// 			.tx_desc_auto_clear = false,
// 			.fixed_mclk = 0};

// 	i2s_pin_config_t pin_config = {
// 			.bck_io_num = IC74HC595_I2S_CLK,
// 			.ws_io_num = IC74HC595_I2S_WS,
// 			.data_out_num = IC74HC595_I2S_DATA,
// 			.data_in_num = -1 // Not used
// 	};
// 	QueueHandle_t i2s_dma_queue;

// 	i2s_driver_install(IC74HC595_I2S_PORT, &i2s_config, I2S_BUFFER_COUNT, &i2s_dma_queue);
// 	i2s_set_pin(IC74HC595_I2S_PORT, &pin_config);

// 	for (;;)
// 	{
// 		uint32_t mode = I2S_MODE;

// 		// tracks DMA buffer usage
// 		if (available_buffers < I2S_BUFFER_COUNT)
// 		{
// 			while (xQueueReceive(i2s_dma_queue, &evt, 0) == pdPASS)
// 			{
// 				if (evt.type == I2S_EVENT_TX_DONE)
// 				{
// 					available_buffers++;
// 				}
// 			}
// 		}

// 		// updates the working mode
// 		if (mode & ITP_STEP_MODE_SYNC)
// 		{
// 			// wait for DMA to output content
// 			while (available_buffers < I2S_BUFFER_COUNT)
// 			{
// 				// tracks DMA buffer usage
// 				while (xQueueReceive(i2s_dma_queue, &evt, 0) == pdPASS)
// 				{
// 					if (evt.type == I2S_EVENT_TX_DONE)
// 					{
// 						available_buffers++;
// 					}
// 				}
// 				vTaskDelayUntil(&xLastWakeTimeUpload, (20 / portTICK_RATE_MS));
// 			}

// 			switch (mode & ~ITP_STEP_MODE_SYNC)
// 			{
// 			case ITP_STEP_MODE_DEFAULT:
// 				// timer_pause(ITP_TIMER_TG, ITP_TIMER_IDX);
// 				// timer_disable_intr(ITP_TIMER_TG, ITP_TIMER_IDX);
// 				I2SREG.conf.tx_start = 0;
// 				I2SREG.conf.tx_reset = 1;
// 				I2SREG.conf.tx_reset = 0;
// 				I2SREG.conf.rx_fifo_reset = 1;
// 				I2SREG.conf.rx_fifo_reset = 0;
// 				available_buffers = I2S_BUFFER_COUNT;
// 				i2s_driver_uninstall(IC74HC595_I2S_PORT);
// 				i2s_driver_install(IC74HC595_I2S_PORT, &i2s_config, I2S_BUFFER_COUNT, &i2s_dma_queue);
// 				i2s_set_pin(IC74HC595_I2S_PORT, &pin_config);
// 				break;
// 			case ITP_STEP_MODE_REALTIME:
// 				I2SREG.conf.tx_start = 0;
// 				I2SREG.conf.tx_reset = 1;
// 				I2SREG.conf.tx_reset = 0;
// 				I2SREG.conf.rx_fifo_reset = 1;
// 				I2SREG.conf.rx_fifo_reset = 0;
// 				available_buffers = I2S_BUFFER_COUNT;
// 				// modify registers for realtime usage
// 				I2SREG.out_link.stop = 1;
// 				I2SREG.fifo_conf.dscr_en = 0;
// 				I2SREG.conf.tx_start = 0;
// 				I2SREG.int_clr.val = 0xFFFFFFFF;
// 				I2SREG.clkm_conf.clka_en = 0;			 // Use PLL/2 as reference
// 				I2SREG.clkm_conf.clkm_div_num = 2; // reset value of 4
// 				I2SREG.clkm_conf.clkm_div_a = 1;	 // 0 at reset, what about divide by 0?
// 				I2SREG.clkm_conf.clkm_div_b = 0;	 // 0 at reset
// 				I2SREG.fifo_conf.tx_fifo_mod = 3;	 // 32 bits single channel data
// 				I2SREG.conf_chan.tx_chan_mod = 3;	 //
// 				I2SREG.sample_rate_conf.tx_bits_mod = 32;
// 				I2SREG.conf.tx_msb_shift = 0;
// 				I2SREG.conf.rx_msb_shift = 0;
// 				I2SREG.int_ena.out_eof = 0;
// 				I2SREG.int_ena.out_dscr_err = 0;
// 				I2SREG.conf_single_data = __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED);
// 				I2SREG.conf1.tx_stop_en = 0;
// 				I2SREG.int_ena.val = 0;
// 				I2SREG.fifo_conf.dscr_en = 1;
// 				I2SREG.int_clr.val = 0xFFFFFFFF;
// 				I2SREG.out_link.start = 1;
// 				I2SREG.conf.tx_start = 1;
// 				ets_delay_us(20);
// 				break;
// 			}

// 			// clear sync flag
// 			__atomic_fetch_and((uint32_t *)&i2s_mode, ~ITP_STEP_MODE_SYNC, __ATOMIC_RELAXED);
// 		}

// 		while (mode == ITP_STEP_MODE_DEFAULT && available_buffers > 0)
// 		{
// 			uint32_t i2s_dma_buffer[I2S_SAMPLES_PER_BUFFER];

// 			for (uint32_t t = 0; t < I2S_SAMPLES_PER_BUFFER; t++)
// 			{
// #if defined(IC74HC595_HAS_STEPS) || defined(IC74HC595_HAS_DIRS)
// 				mcu_gen_step();
// #endif
// #if defined(IC74HC595_HAS_PWMS) || defined(IC74HC595_HAS_SERVOS)
// 				mcu_gen_pwm_and_servo();
// #endif
// #if defined(MCU_HAS_ONESHOT_TIMER) && defined(ENABLE_RT_SYNC_MOTIONS)
// 				mcu_gen_oneshot();
// #endif
// 				// write to buffer
// 				i2s_dma_buffer[t] = __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED);
// 			}

// 			uint32_t w = 0;

// 			i2s_write(IC74HC595_I2S_PORT, &i2s_dma_buffer[0], I2S_SAMPLES_PER_BUFFER * 4, &w, portMAX_DELAY);
// 			available_buffers--;
// 		}

// 		vTaskDelayUntil(&xLastWakeTimeUpload, (5 / portTICK_RATE_MS));
// 	}
// }
// #endif

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