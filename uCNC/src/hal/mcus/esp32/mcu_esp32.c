/*
	Name: mcu_esp32.c
	Description: Implements the µCNC HAL for ESP8266.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 05-02-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

#if (MCU == MCU_ESP32)
#include "esp_timer.h"
#include "esp_task_wdt.h"
#include "esp_ipc.h"
#include "driver/uart.h"
#include "driver/timer.h"
#include "soc/i2s_struct.h"
#ifdef MCU_HAS_I2C
#include "driver/i2c.h"
#endif
#ifdef MCU_HAS_SPI
#include "hal/spi_types.h"
#include "driver/spi_master.h"
#endif
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

static volatile bool esp32_global_isr_enabled;
static volatile bool mcu_itp_timer_running;
#ifdef IC74HC595_CUSTOM_SHIFT_IO
uint32_t ic74hc595_i2s_pins;
#endif
hw_timer_t *esp32_step_timer;

void esp32_wifi_bt_init(void);
void esp32_wifi_bt_flush(uint8_t *buffer);
void esp32_wifi_bt_process(void);

#ifndef FLASH_EEPROM_SIZE
#define FLASH_EEPROM_SIZE 1024
#endif

#if !defined(RAM_ONLY_SETTINGS) && !defined(USE_ARDUINO_EEPROM_LIBRARY)
#include <nvs.h>
#include <esp_partition.h>
// Non volatile memory
typedef struct
{
	nvs_handle_t nvs_handle;
	size_t size;
	bool dirty;
	uint8_t data[FLASH_EEPROM_SIZE];
} flash_eeprom_t;

static flash_eeprom_t mcu_eeprom;
#endif

MCU_CALLBACK void mcu_itp_isr(void *arg);
MCU_CALLBACK void ic74hc595_shift_io_pins(void);
MCU_CALLBACK void esp32_io_updater(void *arg);
MCU_CALLBACK void mcu_gpio_isr(void *type);

/**
 * IO 74HC595 expander via I2S
 * **/
#ifdef IC74HC595_CUSTOM_SHIFT_IO
#if IC74HC595_COUNT != 4
#error "IC74HC595_COUNT must be 4(bytes) to use ESP32 I2S mode for IO shifting"
#endif
#include "driver/i2s.h"
#define USES_BUFFERED_MOTION

#ifdef USES_BUFFERED_MOTION
#define I2S_SAMPLES_PER_BUFFER 256
#define I2S_BUFFER_COUNT 5
#define I2S_SAMPLE_US 5
static uint32_t i2s_dma_buffer[I2S_SAMPLES_PER_BUFFER];
// i2s writer queue
static QueueHandle_t i2s_dma_queue;
#endif

static uint32_t i2s_step_mode;

static uint32_t i2s_itp_timer_reload;
static int32_t i2s_itp_timer_counter;

// implements the custom step mode function to switch between buffered stepping and realtime stepping
void itp_set_step_mode(uint8_t mode)
{
	itp_sync();
	i2s_step_mode = mode;
}

// direct I2S write
MCU_CALLBACK void ic74hc595_shift_io_pins(void)
{
	if (i2s_step_mode == ITP_STEP_MODE_REALTIME)
	{
		I2SREG.conf_single_data = __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED);
	}
}

void esp32_i2s_stream_task(void *param)
{
	uint8_t available_buffers = I2S_BUFFER_COUNT;
	i2s_event_t evt;
	portTickType xLastWakeTimeUpload = xTaskGetTickCount();

	i2s_stop(IC74HC595_I2S_PORT);

	for (;;)
	{
		if (xQueueReceive(i2s_dma_queue, &evt, 0) == pdPASS)
		{
			if (evt.type == I2S_EVENT_TX_DONE)
			{
				available_buffers++;
				if (available_buffers == I2S_BUFFER_COUNT && i2s_step_mode == ITP_STEP_MODE_REALTIME)
				{
					i2s_stop(IC74HC595_I2S_PORT);
					i2s_zero_dma_buffer(IC74HC595_I2S_PORT);
				}
			}
		}

		if (i2s_step_mode == ITP_STEP_MODE_DEFAULT)
		{
			while (available_buffers && itp_get_rt_segment() != NULL)
			{
				for (uint32_t t = 0; t < I2S_SAMPLES_PER_BUFFER; t++)
				{
					// generate steps
					mcu_itp_isr(NULL);
					// updated software PWM pins
					io_soft_pwm_update();
					// write to buffer
					i2s_dma_buffer[t] = ic74hc595_i2s_pins;
				}

				uint32_t w = 0;
				uint32_t i = 0;
				while (i2s_write(IC74HC595_I2S_PORT, &i2s_dma_buffer[i], I2S_SAMPLES_PER_BUFFER - i, &w, 1) != ESP_OK)
				{
					i = w;
					vTaskDelayUntil(&xLastWakeTimeUpload, (1 / portTICK_RATE_MS));
				}

				if (available_buffers == I2S_BUFFER_COUNT)
				{
					i2s_start(IC74HC595_I2S_PORT);
				}

				available_buffers--;
			}
		}

		vTaskDelayUntil(&xLastWakeTimeUpload, (1 / portTICK_RATE_MS));
	}
}

static FORCEINLINE void esp32_i2s_extender_init(void)
{
	i2s_config_t i2s_config = {
		.mode = I2S_MODE_MASTER | I2S_MODE_TX,			// Only TX
		.sample_rate = 1000UL * I2S_SAMPLES_PER_BUFFER, // sample rate to do 2ms per buffer
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

	i2s_zero_dma_buffer(IC74HC595_I2S_PORT);
	i2s_driver_install(IC74HC595_I2S_PORT, &i2s_config, 0, &i2s_dma_queue);
	i2s_set_pin(IC74HC595_I2S_PORT, &pin_config);

	I2SREG.clkm_conf.clka_en = 0;	   // Use PLL/2 as reference
	I2SREG.clkm_conf.clkm_div_num = 2; // reset value of 4
	I2SREG.clkm_conf.clkm_div_a = 1;   // 0 at reset, what about divide by 0?
	I2SREG.clkm_conf.clkm_div_b = 0;   // 0 at reset

	//
	I2SREG.fifo_conf.tx_fifo_mod = 3; // 32 bits single channel data
	I2SREG.conf_chan.tx_chan_mod = 3; //
	I2SREG.sample_rate_conf.tx_bits_mod = 32;

	I2SREG.conf_single_data = 0;

	// Use normal clock format, (WS is aligned with the last bit)
	I2SREG.conf.tx_msb_shift = 0;
	I2SREG.conf.rx_msb_shift = 0;

	// Disable TX interrupts
	I2SREG.int_ena.out_eof = 0;
	I2SREG.int_ena.out_dscr_err = 0;

	xTaskCreatePinnedToCore(esp32_i2s_stream_task, "esp32I2Supdate", 1024, NULL, 7, NULL, CONFIG_ARDUINO_RUNNING_CORE);
}
#endif

#ifdef IC74HC595_HAS_PWMS
#if SERVOS_MASK > 0
// also run servo pin signals
static uint32_t servo_tick_counter = 0;
static uint32_t servo_tick_alarm = 0;
#endif

void servo_update(void);
void servo_reset(void *p);
// this IO updated will run @128KHz
MCU_CALLBACK void esp32_io_updater(void *arg)
{
	// // updated software PWM pins
	io_soft_pwm_update();

#if SERVOS_MASK > 0
	// also run servo pin signals
	uint32_t counter = servo_tick_counter;

	// updated next servo output
	if (!(counter & 0x7F))
	{
		servo_update();
	}

	// reached set tick alarm and resets all servo outputs
	if (counter == servo_tick_alarm)
	{
		servo_reset(NULL);
	}

	// resets every 3ms
	servo_tick_counter = ++counter;
	// servo_tick_counter = (++counter != 384) ? counter : 0;
#endif

	ic74hc595_shift_io_pins();

#ifdef IC74HC595_CUSTOM_SHIFT_IO
	if (i2s_step_mode == ITP_STEP_MODE_DEFAULT)
	{
		return;
	}
#endif

	timer_group_clr_intr_status_in_isr(SERVO_TIMER_TG, SERVO_TIMER_IDX);
	/* After the alarm has been triggered
	  we need enable it again, so it is triggered the next time */
	timer_group_enable_alarm_in_isr(SERVO_TIMER_TG, SERVO_TIMER_IDX);
}
#endif

#if SERVOS_MASK > 0
static uint8_t mcu_servos[6];
MCU_CALLBACK void servo_reset(void *p)
{
#ifndef IC74HC595_HAS_PWMS
	timer_pause(SERVO_TIMER_TG, SERVO_TIMER_IDX);
	timer_group_clr_intr_status_in_isr(SERVO_TIMER_TG, SERVO_TIMER_IDX);
#endif
#if ASSERT_PIN(SERVO0)
	io_clear_output(SERVO0);
#endif
#if ASSERT_PIN(SERVO1)
	io_clear_output(SERVO1);
#endif
#if ASSERT_PIN(SERVO2)
	io_clear_output(SERVO2);
#endif
#if ASSERT_PIN(SERVO3)
	io_clear_output(SERVO3);
#endif
#if ASSERT_PIN(SERVO4)
	io_clear_output(SERVO4);
#endif
#if ASSERT_PIN(SERVO5)
	io_clear_output(SERVO5);
#endif
#ifndef IC74HC595_HAS_PWMS
#ifdef IC74HC595_HAS_SERVOS
	ic74hc595_shift_io_pins();
#endif
#endif
}

void start_servo_timeout(uint8_t timeout);
MCU_CALLBACK void servo_update(void)
{
	static uint8_t servo_counter = 0;
#ifdef IC74HC595_HAS_SERVOS
	uint8_t servomask = 0;
#endif
	switch (servo_counter)
	{
#if ASSERT_PIN(SERVO0)
	case SERVO0_FRAME:
		io_set_output(SERVO0);
		start_servo_timeout(mcu_servos[0]);
#ifdef IC74HC595_HAS_SERVOS
		servomask = SERVO0_MASK;
#endif
		break;
#endif
#if ASSERT_PIN(SERVO1)
	case SERVO1_FRAME:
		io_set_output(SERVO1);
		start_servo_timeout(mcu_servos[1]);
#ifdef IC74HC595_HAS_SERVOS
		servomask = SERVO1_MASK;
#endif
		break;
#endif
#if ASSERT_PIN(SERVO2)
	case SERVO2_FRAME:
		io_set_output(SERVO2);
		start_servo_timeout(mcu_servos[2]);
#ifdef IC74HC595_HAS_SERVOS
		servomask = SERVO2_MASK;
#endif
		break;
#endif
#if ASSERT_PIN(SERVO3)
	case SERVO3_FRAME:
		io_set_output(SERVO3);
		start_servo_timeout(mcu_servos[3]);
#ifdef IC74HC595_HAS_SERVOS
		servomask = SERVO3_MASK;
#endif
		break;
#endif
#if ASSERT_PIN(SERVO4)
	case SERVO4_FRAME:
		io_set_output(SERVO4);
		start_servo_timeout(mcu_servos[4]);
#ifdef IC74HC595_HAS_SERVOS
		servomask = SERVO4_MASK;
#endif
		break;
#endif
#if ASSERT_PIN(SERVO5)
	case SERVO5_FRAME:
		io_set_output(SERVO5);
		start_servo_timeout(mcu_servos[5]);
#ifdef IC74HC595_HAS_SERVOS
		servomask = SERVO5_MASK;
#endif
		break;
#endif
	}

#ifndef IC74HC595_HAS_PWMS
#ifdef IC74HC595_HAS_SERVOS
	ic74hc595_shift_io_pins();
#endif
#endif

	servo_counter++;
	servo_counter = (servo_counter != 20) ? servo_counter : 0;
}

MCU_CALLBACK void start_servo_timeout(uint8_t timeout)
{
#ifndef IC74HC595_HAS_PWMS
	timer_config_t config = {0};
	config.divider = getApbFrequency() / 1000000UL; // 1us per count
	config.counter_dir = TIMER_COUNT_UP;
	config.counter_en = TIMER_PAUSE;
	config.alarm_en = TIMER_ALARM_EN;
	config.auto_reload = true;
	timer_init(SERVO_TIMER_TG, SERVO_TIMER_IDX, &config);

	// should be 500ms but this takes in account the execution code
	// tunned with oscilloscope
	uint64_t us_pulse = 470 + ((2000 * timeout) >> 8);

	/* Timer's counter will initially start from value below.
	   Also, if auto_reload is set, this value will be automatically reload on alarm */
	timer_set_counter_value(SERVO_TIMER_TG, SERVO_TIMER_IDX, 0x00000000ULL);

	/* Configure the alarm value and the interrupt on alarm. */
	timer_set_alarm_value(SERVO_TIMER_TG, SERVO_TIMER_IDX, (uint64_t)us_pulse);
	timer_enable_intr(SERVO_TIMER_TG, SERVO_TIMER_IDX);
	timer_isr_register(SERVO_TIMER_TG, SERVO_TIMER_IDX, servo_reset, NULL, 0, NULL);
	timer_start(SERVO_TIMER_TG, SERVO_TIMER_IDX);
#else
	servo_tick_alarm = servo_tick_counter + timeout + 64 /*0.5ms*/;
#endif
}
#endif

MCU_CALLBACK void mcu_gpio_isr(void *type)
{
	// read the address and not the pointer value because we are passing a literal integer
	// reading the pointer value would try to read an invalid memory address
	switch ((int)type)
	{
	case 0:
		mcu_controls_changed_cb();
		break;
	case 1:
		mcu_limits_changed_cb();
		break;
	case 2:
		mcu_probe_changed_cb();
		break;
	case 3:
		mcu_inputs_changed_cb();
		break;
	default:
		break;
	}
}

#ifdef IC74HC595_HAS_PWMS
uint8_t mcu_softpwm_freq_config(uint16_t freq)
{
	// keeps 8 bit resolution up to 500Hz
	// reduces bit resolution for higher frequencies

	// determines the bit resolution (7 - esp32_pwm_res);
	uint8_t res = (uint8_t)MAX((int8_t)ceilf(log2(freq * 0.002f)), 0);
	return res;
}
#endif

void mcu_core0_tasks_init(void *arg)
{
#ifdef MCU_HAS_UART
	// install UART driver handler
	uart_driver_install(UART_PORT, RX_BUFFER_CAPACITY * 2, 0, 0, NULL, 0);
#endif
#ifdef MCU_HAS_UART2
	// install UART driver handler
	uart_driver_install(UART2_PORT, RX_BUFFER_CAPACITY * 2, 0, 0, NULL, 0);
#endif
}

void mcu_rtc_task(void *arg)
{
	portTickType xLastWakeTimeUpload = xTaskGetTickCount();
	for (;;)
	{
#ifndef IC74HC595_HAS_PWMS
#if SERVOS_MASK > 0
		servo_update();
#endif
#endif
		mcu_rtc_cb(mcu_millis());
		vTaskDelayUntil(&xLastWakeTimeUpload, (1 / portTICK_RATE_MS));
	}
}

MCU_CALLBACK void mcu_itp_isr(void *arg)
{
	static bool resetstep = false;

#ifdef IC74HC595_CUSTOM_SHIFT_IO
	if (i2s_step_mode == ITP_STEP_MODE_DEFAULT)
	{
		if (!mcu_itp_timer_running)
		{
			return;
		}

		int32_t t = i2s_itp_timer_counter;
		t -= I2S_SAMPLE_US;
		if (t > 0)
		{
			i2s_itp_timer_counter = t;
			// exit
			return;
		}

		i2s_itp_timer_counter = i2s_itp_timer_reload + t;
	}
#endif

	if (!resetstep)
	{
		mcu_step_cb();
	}
	else
	{
		mcu_step_reset_cb();
	}
	resetstep = !resetstep;

#ifdef IC74HC595_CUSTOM_SHIFT_IO
	if (i2s_step_mode == ITP_STEP_MODE_DEFAULT)
	{
		return;
	}
#endif

	timer_group_clr_intr_status_in_isr(ITP_TIMER_TG, ITP_TIMER_IDX);
	/* After the alarm has been triggered
	  we need enable it again, so it is triggered the next time */
	timer_group_enable_alarm_in_isr(ITP_TIMER_TG, ITP_TIMER_IDX);
}

/**
 * initializes the mcu
 * this function needs to:
 *   - configure all IO pins (digital IO, PWM, Analog, etc...)
 *   - configure all interrupts
 *   - configure uart or usb
 *   - start the internal RTC
 * */
void mcu_init(void)
{
#if (defined(LIMIT_X_ISR) || defined(LIMIT_Y_ISR) || defined(LIMIT_Z_ISR) || defined(LIMIT_X2_ISR) || defined(LIMIT_Y2_ISR) || defined(LIMIT_Z2_ISR) || defined(LIMIT_A_ISR) || defined(LIMIT_B_ISR) || defined(LIMIT_C_ISR) || defined(PROBE_ISR) || defined(ESTOP_ISR) || defined(SAFETY_DOOR_ISR) || defined(FHOLD_ISR) || defined(CS_RES_ISR) || defined(DIN0_ISR) || defined(DIN1_ISR) || defined(DIN2_ISR) || defined(DIN3_ISR) || defined(DIN4_ISR) || defined(DIN5_ISR) || defined(DIN6_ISR) || defined(DIN7_ISR))
	gpio_install_isr_service(0);
#endif

	mcu_io_init();

	// starts EEPROM before UART to enable WiFi and BT settings
#if !defined(RAM_ONLY_SETTINGS) && !defined(USE_ARDUINO_EEPROM_LIBRARY)
	// esp32_eeprom_init(FLASH_EEPROM_SIZE); // 1K Emulated EEPROM

	// starts nvs
	mcu_eeprom.size = 0;
	memset(mcu_eeprom.data, 0, FLASH_EEPROM_SIZE);
	if (nvs_open("eeprom", NVS_READWRITE, &mcu_eeprom.nvs_handle) == ESP_OK)
	{
		// determines the maximum sector size of NVS that can be read/write
		nvs_get_blob(mcu_eeprom.nvs_handle, "eeprom", NULL, &mcu_eeprom.size);
		if (FLASH_EEPROM_SIZE > mcu_eeprom.size)
		{
			log_e("eeprom does not have enough space");
			mcu_eeprom.size = 0;
		}

		nvs_get_blob(mcu_eeprom.nvs_handle, "eeprom", mcu_eeprom.data, &mcu_eeprom.size);
	}
	else
	{
		log_e("eeprom failed to open");
	}
#else
	extern void esp32_eeprom_init(int size);
	esp32_eeprom_init(FLASH_EEPROM_SIZE);
#endif

#ifdef MCU_HAS_UART
	// initialize UART
	const uart_config_t uartconfig = {
		.baud_rate = BAUDRATE,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_APB};
	// We won't use a buffer for sending data.
	uart_param_config(UART_PORT, &uartconfig);
	uart_set_pin(UART_PORT, TX_BIT, RX_BIT, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
#endif

#ifdef MCU_HAS_UART2
	// initialize UART
	const uart_config_t uart2config = {
		.baud_rate = BAUDRATE2,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_APB};
	// We won't use a buffer for sending data.
	uart_param_config(UART2_PORT, &uart2config);
	uart_set_pin(UART2_PORT, TX2_BIT, RX2_BIT, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
#endif

#ifdef IC74HC595_CUSTOM_SHIFT_IO
	esp32_i2s_extender_init();
#endif

#ifdef IC74HC595_HAS_PWMS
	/* Select and initialize basic parameters of the timer */
	timer_config_t pwmconfig = {0};
	pwmconfig.divider = getApbFrequency() / 640000UL; // 1.5625us per pulse
	pwmconfig.counter_dir = TIMER_COUNT_UP;
	pwmconfig.counter_en = TIMER_PAUSE;
	pwmconfig.intr_type = TIMER_INTR_MAX;
	pwmconfig.alarm_en = TIMER_ALARM_EN;
	pwmconfig.auto_reload = true;
	timer_init(SERVO_TIMER_TG, SERVO_TIMER_IDX, &pwmconfig);

	/* Timer's counter will initially start from value below.
	   Also, if auto_reload is set, this value will be automatically reload on alarm */
	timer_set_counter_value(SERVO_TIMER_TG, SERVO_TIMER_IDX, 0x00000000ULL);
	/* Configure the alarm value and the interrupt on alarm. */
	timer_set_alarm_value(SERVO_TIMER_TG, SERVO_TIMER_IDX, (uint64_t)5);
	// register PWM isr
	timer_isr_register(SERVO_TIMER_TG, SERVO_TIMER_IDX, esp32_io_updater, NULL, 0, NULL);
	timer_enable_intr(SERVO_TIMER_TG, SERVO_TIMER_IDX);
	timer_start(SERVO_TIMER_TG, SERVO_TIMER_IDX);
#endif

	// inititialize ITP timer
	timer_config_t itpconfig = {0};
	itpconfig.divider = getApbFrequency() / 1000000UL; // 1us per pulse
	itpconfig.counter_dir = TIMER_COUNT_UP;
	itpconfig.counter_en = TIMER_PAUSE;
	itpconfig.intr_type = TIMER_INTR_MAX;
	itpconfig.alarm_en = TIMER_ALARM_EN;
	itpconfig.auto_reload = true;
	timer_init(ITP_TIMER_TG, ITP_TIMER_IDX, &itpconfig);

	// initialize rtc timer (currently on core 1)
	xTaskCreatePinnedToCore(mcu_rtc_task, "rtcTask", 1024, NULL, 7, NULL, CONFIG_ARDUINO_RUNNING_CORE);

	// launches isr tasks that will run on core 0
	// currently it's running PWM and UART on core 0
	// Arduino Bluetooth also runs on core 0
	// Arduino WiFi ???
	esp_ipc_call_blocking(0, mcu_core0_tasks_init, NULL);

#ifdef MCU_HAS_SPI
	spi_bus_config_t spiconf = {
		.miso_io_num = SPI_SDI_BIT,
		.mosi_io_num = SPI_SDO_BIT,
		.sclk_io_num = SPI_CLK_BIT,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.data4_io_num = -1,
		.data5_io_num = -1,
		.data6_io_num = -1,
		.data7_io_num = -1,
		.max_transfer_sz = SOC_SPI_MAXIMUM_BUFFER_SIZE,
		.flags = 0,
		.intr_flags = 0};
	// Initialize the SPI bus
	spi_bus_initialize(SPI_PORT, &spiconf, SPI_DMA_DISABLED);
	mcu_spi_config(SPI_MODE, SPI_FREQ);
#endif

#ifdef MCU_HAS_I2C
	mcu_i2c_config(I2C_FREQ);
#endif

	esp32_wifi_bt_init();
	mcu_enable_global_isr();
}

/**
 * enables the pin probe mcu isr on change
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_enable_probe_isr
void mcu_enable_probe_isr(void)
{
}
#endif

/**
 * disables the pin probe mcu isr on change
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_disable_probe_isr
void mcu_disable_probe_isr(void)
{
}
#endif

/**
 * gets the voltage value of a built-in ADC pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_analog
uint16_t mcu_get_analog(uint8_t channel)
{
	return 0;
}
#endif

/**
 * sets the pwm value of a built-in pwm pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_set_pwm
void mcu_set_pwm(uint8_t pwm, uint8_t value)
{
}
#endif

/**
 * gets the configured pwm value of a built-in pwm pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_pwm
uint8_t mcu_get_pwm(uint8_t pwm)
{
	return 0;
}
#endif

/*UART*/

/**
 * sends a uint8_t either via uart (hardware, software or USB virtual COM port)
 * can be defined either as a function or a macro call
 * */
#ifdef MCU_HAS_UART
#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE 64
#endif
DECL_BUFFER(uint8_t, uart_rx, RX_BUFFER_SIZE);
DECL_BUFFER(uint8_t, uart_tx, UART_TX_BUFFER_SIZE);
uint8_t mcu_uart_getc(void)
{
	uint8_t c = 0;
	BUFFER_DEQUEUE(uart_rx, &c);
	return c;
}

uint8_t mcu_uart_available(void)
{
	return BUFFER_READ_AVAILABLE(uart_rx);
}

void mcu_uart_clear(void)
{
	BUFFER_CLEAR(uart_rx);
}

void mcu_uart_putc(uint8_t c)
{
	while (BUFFER_FULL(uart_tx))
	{
		mcu_uart_flush();
	}
	BUFFER_ENQUEUE(uart_tx, &c);
}

void mcu_uart_flush(void)
{
	while (!BUFFER_EMPTY(uart_tx))
	{
		uint8_t tmp[UART_TX_BUFFER_SIZE + 1];
		memset(tmp, 0, sizeof(tmp));
		uint8_t r;

		BUFFER_READ(uart_tx, tmp, UART_TX_BUFFER_SIZE, r);
		uart_write_bytes(UART_PORT, tmp, r);
	}
}
#endif

#ifdef MCU_HAS_UART2
#ifndef UART2_TX_BUFFER_SIZE
#define UART2_TX_BUFFER_SIZE 64
#endif
DECL_BUFFER(uint8_t, uart2_rx, RX_BUFFER_SIZE);
DECL_BUFFER(uint8_t, uart2_tx, UART2_TX_BUFFER_SIZE);

uint8_t mcu_uart2_getc(void)
{
	uint8_t c = 0;
	BUFFER_DEQUEUE(uart2_rx, &c);
	return c;
}

uint8_t mcu_uart2_available(void)
{
	return BUFFER_READ_AVAILABLE(uart2_rx);
}

void mcu_uart2_clear(void)
{
	BUFFER_CLEAR(uart2_rx);
}

void mcu_uart2_putc(uint8_t c)
{
	while (BUFFER_FULL(uart2_tx))
	{
		mcu_uart2_flush();
	}
	BUFFER_ENQUEUE(uart2_tx, &c);
}

void mcu_uart2_flush(void)
{
	while (!BUFFER_EMPTY(uart2_tx))
	{
		uint8_t tmp[UART2_TX_BUFFER_SIZE + 1];
		memset(tmp, 0, sizeof(tmp));
		uint8_t r;

		BUFFER_READ(uart2_tx, tmp, UART2_TX_BUFFER_SIZE, r);
		uart_write_bytes(UART2_PORT, tmp, r);
	}
}
#endif

// ISR
/**
 * enables global interrupts on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_enable_global_isr
void mcu_enable_global_isr(void)
{
	// ets_intr_unlock();
	esp32_global_isr_enabled = true;
}
#endif

/**
 * disables global interrupts on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_disable_global_isr
void mcu_disable_global_isr(void)
{
	esp32_global_isr_enabled = false;
	// ets_intr_lock();
}
#endif

/**
 * gets global interrupts state on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_global_isr
bool mcu_get_global_isr(void)
{
	return esp32_global_isr_enabled;
}
#endif

// Step interpolator
/**
 * convert step rate to clock cycles
 * */
void mcu_freq_to_clocks(float frequency, uint16_t *ticks, uint16_t *prescaller)
{
	frequency = CLAMP((float)F_STEP_MIN, frequency, (float)F_STEP_MAX);
	// up and down counter (generates half the step rate at each event)
	uint32_t totalticks = (uint32_t)(500000.0f / frequency);
	*prescaller = 1;
	while (totalticks > 0xFFFF)
	{
		(*prescaller) <<= 1;
		totalticks >>= 1;
	}

	*ticks = (uint16_t)totalticks;
}

float mcu_clocks_to_freq(uint16_t ticks, uint16_t prescaller)
{
	return (500000.0f / ((float)ticks * (float)prescaller));
}

/**
 * starts the timer interrupt that generates the step pulses for the interpolator
 * */

void mcu_start_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	if (!mcu_itp_timer_running)
	{
#ifdef IC74HC595_CUSTOM_SHIFT_IO
		if (i2s_step_mode == ITP_STEP_MODE_DEFAULT)
		{
			i2s_itp_timer_reload = ticks * prescaller;
			mcu_itp_timer_running = true;
			return;
		}
#endif
		/* Timer's counter will initially start from value below.
		   Also, if auto_reload is set, this value will be automatically reload on alarm */
		timer_set_counter_value(ITP_TIMER_TG, ITP_TIMER_IDX, 0x00000000ULL);

		/* Configure the alarm value and the interrupt on alarm. */
		timer_set_alarm_value(ITP_TIMER_TG, ITP_TIMER_IDX, (uint64_t)ticks * prescaller);
		timer_enable_intr(ITP_TIMER_TG, ITP_TIMER_IDX);
		timer_isr_register(ITP_TIMER_TG, ITP_TIMER_IDX, mcu_itp_isr, NULL, 0, NULL);

		timer_start(ITP_TIMER_TG, ITP_TIMER_IDX);
		mcu_itp_timer_running = true;
	}
	else
	{
		mcu_change_itp_isr(ticks, prescaller);
	}
}

/**
 * changes the step rate of the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_change_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	if (mcu_itp_timer_running)
	{
#ifdef IC74HC595_CUSTOM_SHIFT_IO
		if (i2s_step_mode == ITP_STEP_MODE_DEFAULT)
		{
			i2s_itp_timer_reload = ticks * prescaller;
			return;
		}
#endif

		timer_pause(ITP_TIMER_TG, ITP_TIMER_IDX);
		timer_set_alarm_value(ITP_TIMER_TG, ITP_TIMER_IDX, (uint64_t)ticks * prescaller);
		timer_start(ITP_TIMER_TG, ITP_TIMER_IDX);
	}
	else
	{
		mcu_start_itp_isr(ticks, prescaller);
	}
}

/**
 * stops the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_stop_itp_isr(void)
{
	if (mcu_itp_timer_running)
	{
#ifdef IC74HC595_CUSTOM_SHIFT_IO
		if (i2s_step_mode == ITP_STEP_MODE_DEFAULT)
		{
			mcu_itp_timer_running = false;
			return;
		}
#endif

		timer_pause(ITP_TIMER_TG, ITP_TIMER_IDX);
		timer_disable_intr(ITP_TIMER_TG, ITP_TIMER_IDX);
		mcu_itp_timer_running = false;
	}
}

/**
 * gets the MCU running time in milliseconds.
 * the time counting is controled by the internal RTC
 * */
extern int64_t esp_system_get_time(void);
uint32_t mcu_millis()
{
	return (uint32_t)(esp_system_get_time() / 1000);
}

uint32_t mcu_micros()
{
	return (uint32_t)esp_system_get_time();
}

void esp32_delay_us(uint16_t delay)
{
	int64_t time = esp_system_get_time() + delay - 1;
	while (time > esp_system_get_time())
		;
}

/**
 * runs all internal tasks of the MCU.
 * for the moment these are:
 *   - if USB is enabled and MCU uses tinyUSB framework run tinyUSB tud_task
 * */
void mcu_dotasks(void)
{
	// reset WDT
	esp_task_wdt_reset();

	// loop through received data
	uint8_t rxdata[RX_BUFFER_SIZE];
	int rxlen, i;
#ifdef MCU_HAS_UART
	rxlen = uart_read_bytes(UART_PORT, rxdata, RX_BUFFER_CAPACITY, 0);
	for (i = 0; i < rxlen; i++)
	{
		uint8_t c = (uint8_t)rxdata[i];
		if (mcu_com_rx_cb(c))
		{
			if (BUFFER_FULL(uart_rx))
			{
				c = OVF;
			}

			*(BUFFER_NEXT_FREE(uart_rx)) = c;
			BUFFER_STORE(uart_rx);
		}
	}
#endif
#if defined(MCU_HAS_UART2)
	rxlen = uart_read_bytes(UART2_PORT, rxdata, RX_BUFFER_CAPACITY, 0);
#if !defined(DETACH_UART2_FROM_MAIN_PROTOCOL)
	for (i = 0; i < rxlen; i++)
	{
		uint8_t c = (uint8_t)rxdata[i];
		if (mcu_com_rx_cb(c))
		{
			if (BUFFER_FULL(uart2_rx))
			{
				c = OVF;
			}

			*(BUFFER_NEXT_FREE(uart2_rx)) = c;
			BUFFER_STORE(uart2_rx);
		}
	}
#else
	for (i = 0; i < rxlen; i++)
	{
		mcu_uart_rx_cb((uint8_t)rxdata[i]);
	}
#endif
#endif

	esp32_wifi_bt_process();
}

// Non volatile memory
/**
 * gets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
 * */
#if !defined(RAM_ONLY_SETTINGS) && !defined(USE_ARDUINO_EEPROM_LIBRARY)
uint8_t mcu_eeprom_getc(uint16_t address)
{
#ifndef RAM_ONLY_SETTINGS
	// return esp32_eeprom_read(address);
	size_t size = mcu_eeprom.size;
	if (size)
	{
		return mcu_eeprom.data[address];
	}
#endif
	return 0;
}

/**
 * sets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
 * */
void mcu_eeprom_putc(uint16_t address, uint8_t value)
{
#ifndef RAM_ONLY_SETTINGS
	// esp32_eeprom_write(address, value);
	size_t size = mcu_eeprom.size;
	if (size)
	{
		mcu_eeprom.dirty |= (mcu_eeprom.data[address] != value);
		mcu_eeprom.data[address] = value;
	}
#endif
}

/**
 * flushes all recorded registers into the eeprom.
 * */
void mcu_eeprom_flush(void)
{
#ifndef RAM_ONLY_SETTINGS
	// esp32_eeprom_flush();
	// esp32_eeprom_write(address, value);
	if (mcu_eeprom.size && mcu_eeprom.dirty)
	{
		nvs_set_blob(mcu_eeprom.nvs_handle, "eeprom", mcu_eeprom.data, mcu_eeprom.size);
		nvs_commit(mcu_eeprom.nvs_handle);
	}
#endif
}
#endif

#ifdef MCU_HAS_ONESHOT_TIMER

MCU_CALLBACK void mcu_oneshot_isr(void *arg)
{
	timer_pause(ONESHOT_TIMER_TG, ONESHOT_TIMER_IDX);
	timer_group_clr_intr_status_in_isr(ONESHOT_TIMER_TG, ONESHOT_TIMER_IDX);

	if (mcu_timeout_cb)
	{
		mcu_timeout_cb();
	}
}

/**
 * configures a single shot timeout in us
 * */
#ifndef mcu_config_timeout
void mcu_config_timeout(mcu_timeout_delgate fp, uint32_t timeout)
{
	mcu_timeout_cb = fp;
	timer_config_t config = {0};
	config.divider = getApbFrequency() / 1000000UL; // 1us per count
	config.counter_dir = TIMER_COUNT_UP;
	config.counter_en = TIMER_PAUSE;
	config.alarm_en = TIMER_ALARM_EN;
	config.auto_reload = true;
	timer_init(ONESHOT_TIMER_TG, ONESHOT_TIMER_IDX, &config);

	/* Timer's counter will initially start from value below.
	   Also, if auto_reload is set, this value will be automatically reload on alarm */
	timer_set_counter_value(ONESHOT_TIMER_TG, ONESHOT_TIMER_IDX, 0x00000000ULL);

	/* Configure the alarm value and the interrupt on alarm. */
	timer_set_alarm_value(ONESHOT_TIMER_TG, ONESHOT_TIMER_IDX, (uint64_t)timeout);
	timer_enable_intr(ONESHOT_TIMER_TG, ONESHOT_TIMER_IDX);
	timer_isr_register(ONESHOT_TIMER_TG, ONESHOT_TIMER_IDX, mcu_oneshot_isr, NULL, 0, NULL);
}
#endif

/**
 * starts the timeout. Once hit the the respective callback is called
 * */
#ifndef mcu_start_timeout
void mcu_start_timeout()
{
	timer_start(ONESHOT_TIMER_TG, ONESHOT_TIMER_IDX);
}
#endif
#endif

#if defined(MCU_HAS_SPI) && !defined(USE_ARDUINO_SPI_LIBRARY)

static spi_device_handle_t mcu_spi_handle;

#ifndef mcu_spi_xmit
uint8_t mcu_spi_xmit(uint8_t data)
{
	uint8_t rxdata = 0xFF;
	spi_transaction_t spi_trans = {0};
	spi_trans.length = 8; // Number of bits NOT number of bytes.
	spi_trans.tx_buffer = &data;
	spi_trans.rx_buffer = &rxdata;

	spi_device_transmit(mcu_spi_handle, &spi_trans);

	return rxdata;
}
#endif

#ifndef mcu_spi_config
void mcu_spi_config(uint8_t mode, uint32_t frequency)
{
	spi_bus_remove_device(mcu_spi_handle);
	spi_device_interface_config_t mcu_spi_conf = {0};
	mcu_spi_conf.clock_speed_hz = frequency;
	mcu_spi_conf.spics_io_num = -1;
	mcu_spi_conf.queue_size = 1;

	spi_bus_add_device(SPI_PORT, &mcu_spi_conf, &mcu_spi_handle);
}
#endif
#endif

/*IO functions*/
// IO functions
void mcu_set_servo(uint8_t servo, uint8_t value)
{
#if SERVOS_MASK > 0
	mcu_servos[servo - SERVO_PINS_OFFSET] = value;
#endif
}

/**
 * gets the pwm for a servo (50Hz with tON between 1~2ms)
 * can be defined either as a function or a macro call
 * */
uint8_t mcu_get_servo(uint8_t servo)
{
#if SERVOS_MASK > 0
	uint8_t offset = servo - SERVO_PINS_OFFSET;

	if ((1U << offset) & SERVOS_MASK)
	{
		return mcu_servos[offset];
	}
#endif
	return 0;
}

#endif
