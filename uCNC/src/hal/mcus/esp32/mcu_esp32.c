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
#include <driver/timer.h>
#ifdef MCU_HAS_I2C
#include <driver/i2c.h>
#endif
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

static volatile bool esp32_global_isr_enabled;
static volatile uint32_t mcu_runtime_ms;

void esp32_uart_init(int baud);
char esp32_uart_read(void);
void esp32_uart_write(char c);
bool esp32_uart_rx_ready(void);
bool esp32_uart_tx_ready(void);
void esp32_uart_flush(void);
void esp32_uart_process(void);

#ifndef RAM_ONLY_SETTINGS
void esp32_eeprom_init(int size);
uint8_t esp32_eeprom_read(uint16_t address);
void esp32_eeprom_write(uint16_t address, uint8_t value);
void esp32_eeprom_flush(void);
#endif

hw_timer_t *esp32_rtc_timer;
hw_timer_t *esp32_step_timer;

uint8_t esp32_pwm[16];
static IRAM_ATTR void mcu_gen_pwm(void)
{
	static uint8_t pwm_counter = 0;
	// software PWM
	if (++pwm_counter < 127)
	{
#if !(PWM0 < 0)
		if (pwm_counter > esp32_pwm[0])
		{
			mcu_clear_output(PWM0);
		}
#endif
#if !(PWM1 < 0)
		if (pwm_counter > esp32_pwm[1])
		{
			mcu_clear_output(PWM1);
		}
#endif
#if !(PWM2 < 0)
		if (pwm_counter > esp32_pwm[2])
		{
			mcu_clear_output(PWM2);
		}
#endif
#if !(PWM3 < 0)
		if (pwm_counter > esp32_pwm[3])
		{
			mcu_clear_output(PWM3);
		}
#endif
#if !(PWM4 < 0)
		if (pwm_counter > esp32_pwm[4])
		{
			mcu_clear_output(PWM4);
		}
#endif
#if !(PWM5 < 0)
		if (pwm_counter > esp32_pwm[5])
		{
			mcu_clear_output(PWM5);
		}
#endif
#if !(PWM6 < 0)
		if (pwm_counter > esp32_pwm[6])
		{
			mcu_clear_output(PWM6);
		}
#endif
#if !(PWM7 < 0)
		if (pwm_counter > esp32_pwm[7])
		{
			mcu_clear_output(PWM7);
		}
#endif
#if !(PWM8 < 0)
		if (pwm_counter > esp32_pwm[8])
		{
			mcu_clear_output(PWM8);
		}
#endif
#if !(PWM9 < 0)
		if (pwm_counter > esp32_pwm[9])
		{
			mcu_clear_output(PWM9);
		}
#endif
#if !(PWM10 < 0)
		if (pwm_counter > esp32_pwm[10])
		{
			mcu_clear_output(PWM10);
		}
#endif
#if !(PWM11 < 0)
		if (pwm_counter > esp32_pwm[11])
		{
			mcu_clear_output(PWM11);
		}
#endif
#if !(PWM12 < 0)
		if (pwm_counter > esp32_pwm[12])
		{
			mcu_clear_output(PWM12);
		}
#endif
#if !(PWM13 < 0)
		if (pwm_counter > esp32_pwm[13])
		{
			mcu_clear_output(PWM13);
		}
#endif
#if !(PWM14 < 0)
		if (pwm_counter > esp32_pwm[14])
		{
			mcu_clear_output(PWM14);
		}
#endif
#if !(PWM15 < 0)
		if (pwm_counter > esp32_pwm[15])
		{
			mcu_clear_output(PWM15);
		}
#endif
	}
	else
	{
		pwm_counter = 0;
#if !(PWM0 < 0)
		if (esp32_pwm[0])
		{
			mcu_set_output(PWM0);
		}
#endif
#if !(PWM1 < 0)
		if (esp32_pwm[1])
		{
			mcu_set_output(PWM1);
		}
#endif
#if !(PWM2 < 0)
		if (esp32_pwm[2])
		{
			mcu_set_output(PWM2);
		}
#endif
#if !(PWM3 < 0)
		if (esp32_pwm[3])
		{
			mcu_set_output(PWM3);
		}
#endif
#if !(PWM4 < 0)
		if (esp32_pwm[4])
		{
			mcu_set_output(PWM4);
		}
#endif
#if !(PWM5 < 0)
		if (esp32_pwm[5])
		{
			mcu_set_output(PWM5);
		}
#endif
#if !(PWM6 < 0)
		if (esp32_pwm[6])
		{
			mcu_set_output(PWM6);
		}
#endif
#if !(PWM7 < 0)
		if (esp32_pwm[7])
		{
			mcu_set_output(PWM7);
		}
#endif
#if !(PWM8 < 0)
		if (esp32_pwm[8])
		{
			mcu_set_output(PWM8);
		}
#endif
#if !(PWM9 < 0)
		if (esp32_pwm[9])
		{
			mcu_set_output(PWM9);
		}
#endif
#if !(PWM10 < 0)
		if (esp32_pwm[10])
		{
			mcu_set_output(PWM10);
		}
#endif
#if !(PWM11 < 0)
		if (esp32_pwm[11])
		{
			mcu_set_output(PWM11);
		}
#endif
#if !(PWM12 < 0)
		if (esp32_pwm[12])
		{
			mcu_set_output(PWM12);
		}
#endif
#if !(PWM13 < 0)
		if (esp32_pwm[13])
		{
			mcu_set_output(PWM13);
		}
#endif
#if !(PWM14 < 0)
		if (esp32_pwm[14])
		{
			mcu_set_output(PWM14);
		}
#endif
#if !(PWM15 < 0)
		if (esp32_pwm[15])
		{
			mcu_set_output(PWM15);
		}
#endif
	}
}

IRAM_ATTR void mcu_din_isr(void)
{
	mcu_inputs_changed_cb();
}

IRAM_ATTR void mcu_probe_isr(void)
{
	mcu_probe_changed_cb();
}

IRAM_ATTR void mcu_limits_isr(void)
{
	mcu_limits_changed_cb();
}

IRAM_ATTR void mcu_controls_isr(void)
{
	mcu_controls_changed_cb();
}

IRAM_ATTR void mcu_rtc_isr(void *arg)
{
	static uint8_t rtc_counter = 0;

	mcu_gen_pwm();
	rtc_counter++;
	if (rtc_counter == 128)
	{
		mcu_runtime_ms++;
		mcu_rtc_cb(mcu_runtime_ms);
		rtc_counter = 0;
	}

	timer_group_clr_intr_status_in_isr(RTC_TIMER_TG, RTC_TIMER_IDX);
	/* After the alarm has been triggered
	  we need enable it again, so it is triggered the next time */
	timer_group_enable_alarm_in_isr(RTC_TIMER_TG, RTC_TIMER_IDX);
}

IRAM_ATTR void mcu_itp_isr(void *arg)
{
	static bool resetstep = false;

	if (!resetstep)
		mcu_step_cb();
	else
		mcu_step_reset_cb();
	resetstep = !resetstep;

	timer_group_clr_intr_status_in_isr(ITP_TIMER_TG, ITP_TIMER_IDX);
	/* After the alarm has been triggered
	  we need enable it again, so it is triggered the next time */
	timer_group_enable_alarm_in_isr(ITP_TIMER_TG, ITP_TIMER_IDX);
}

static void mcu_usart_init(void)
{
	esp32_uart_init(BAUDRATE);
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
	mcu_io_init();
	mcu_usart_init();

	// initialize rtc timer
	/* Select and initialize basic parameters of the timer */
	timer_config_t config = {
		.divider = 5,
		.counter_dir = TIMER_COUNT_UP,
		.counter_en = TIMER_PAUSE,
		.alarm_en = TIMER_ALARM_EN,
		.auto_reload = true,
	}; // default clock source is APB
	timer_init(RTC_TIMER_TG, RTC_TIMER_IDX, &config);

	/* Timer's counter will initially start from value below.
	   Also, if auto_reload is set, this value will be automatically reload on alarm */
	timer_set_counter_value(RTC_TIMER_TG, RTC_TIMER_IDX, 0x00000000ULL);

	/* Configure the alarm value and the interrupt on alarm. */
	timer_set_alarm_value(RTC_TIMER_TG, RTC_TIMER_IDX, (uint64_t)125);
	timer_enable_intr(RTC_TIMER_TG, RTC_TIMER_IDX);
	timer_isr_register(RTC_TIMER_TG, RTC_TIMER_IDX, mcu_rtc_isr, NULL, 0, NULL);

	timer_start(RTC_TIMER_TG, RTC_TIMER_IDX);

	/*uint16_t timerdiv = (uint16_t)(getApbFrequency() / 128000UL);
	esp32_rtc_timer = timerBegin(RTC_TIMER, timerdiv, true);
	timerAttachInterrupt(esp32_rtc_timer, &mcu_rtc_isr, true);
	timerAlarmWrite(esp32_rtc_timer, 1, true);
	timerAlarmEnable(esp32_rtc_timer);

	// initialize stepper timer
	timerdiv = (uint16_t)(getApbFrequency() / (F_STEP_MAX << 1));
	esp32_step_timer = timerBegin(ITP_TIMER, timerdiv, true);*/

#ifndef RAM_ONLY_SETTINGS
	esp32_eeprom_init(1024); // 1K Emulated EEPROM
#endif

#ifdef MCU_HAS_SPI
	esp32_spi_init(SPI_FREQ, SPI_MODE, SPI_CLK, SPI_SDI, SPI_SDO);
#endif

#ifdef MCU_HAS_I2C
	i2c_config_t conf = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = I2C_SDA_BIT, // select GPIO specific to your project
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_io_num = I2C_SCL_BIT, // select GPIO specific to your project
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = I2C_FREQ, // select frequency specific to your project
		.clk_flags = 0,				  // you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here
	};
	i2c_param_config((i2c_port_t)I2C_PORT, &conf);
	i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0);
#endif

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
uint8_t mcu_get_analog(uint8_t channel)
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

/**
 * checks if the serial hardware of the MCU is ready do send the next char
 * */
#ifndef mcu_tx_ready
bool mcu_tx_ready(void)
{
	return esp32_uart_tx_ready();
}
#endif

/**
 * checks if the serial hardware of the MCU has a new char ready to be read
 * */
#ifndef mcu_rx_ready
bool mcu_rx_ready(void)
{
	return esp32_uart_rx_ready();
}
#endif

/**
 * sends a char either via uart (hardware, software or USB virtual COM port)
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_putc

void mcu_putc(char c)
{
#if !(LED < 0)
	mcu_toggle_output(LED);
#endif
#ifdef ENABLE_SYNC_TX
	while (!mcu_tx_ready())
		;
#endif

	esp32_uart_write(c);
}
#endif

/**
 * gets a char either via uart (hardware, software or USB virtual COM port)
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_getc
char mcu_getc(void)
{
#if !(LED < 0)
	mcu_toggle_output(LED);
#endif
#ifdef ENABLE_SYNC_RX
	while (!mcu_rx_ready())
		;
#endif

	return esp32_uart_read();
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

/**
 * starts the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_start_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	/*timerAttachInterrupt(esp32_step_timer, &mcu_itp_isr, true);
	timerAlarmWrite(esp32_step_timer, (uint32_t)ticks * (uint32_t)prescaller, true);
	timerAlarmEnable(esp32_step_timer); // Just Enable*/
	timer_config_t config = {
		.divider = 80,
		.counter_dir = TIMER_COUNT_UP,
		.counter_en = TIMER_PAUSE,
		.alarm_en = TIMER_ALARM_EN,
		.auto_reload = true,
	}; // default clock source is APB
	timer_init(ITP_TIMER_TG, ITP_TIMER_IDX, &config);

	/* Timer's counter will initially start from value below.
	   Also, if auto_reload is set, this value will be automatically reload on alarm */
	timer_set_counter_value(ITP_TIMER_TG, ITP_TIMER_IDX, 0x00000000ULL);

	/* Configure the alarm value and the interrupt on alarm. */
	timer_set_alarm_value(ITP_TIMER_TG, ITP_TIMER_IDX, (uint64_t)ticks * prescaller);
	timer_enable_intr(ITP_TIMER_TG, ITP_TIMER_IDX);
	timer_isr_register(ITP_TIMER_TG, ITP_TIMER_IDX, mcu_itp_isr, NULL, 0, NULL);

	timer_start(ITP_TIMER_TG, ITP_TIMER_IDX);
}

/**
 * changes the step rate of the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_change_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	timer_pause(ITP_TIMER_TG, ITP_TIMER_IDX);
	timer_set_alarm_value(ITP_TIMER_TG, ITP_TIMER_IDX, (uint64_t)ticks * prescaller);
	timer_start(ITP_TIMER_TG, ITP_TIMER_IDX);
}

/**
 * stops the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_stop_itp_isr(void)
{
	// timerAlarmDisable(esp32_step_timer);
	timer_pause(ITP_TIMER_TG, ITP_TIMER_IDX);
	timer_disable_intr(ITP_TIMER_TG, ITP_TIMER_IDX);
}

/**
 * gets the MCU running time in milliseconds.
 * the time counting is controled by the internal RTC
 * */
uint32_t mcu_millis()
{
	return mcu_runtime_ms;
}

#ifndef mcu_delay_us
void mcu_delay_us(uint8_t delay)
{
	int64_t time = esp_timer_get_time() + delay;
	while (time > esp_timer_get_time())
		;
}
#endif

/**
 * runs all internal tasks of the MCU.
 * for the moment these are:
 *   - if USB is enabled and MCU uses tinyUSB framework run tinyUSB tud_task
 *   - if ENABLE_SYNC_RX is enabled check if there are any chars in the rx transmitter (or the tinyUSB buffer) and read them to the serial_rx_isr
 *   - if ENABLE_SYNC_TX is enabled check if serial_tx_empty is false and run serial_tx_isr
 * */
void mcu_dotasks(void)
{
	// reset WDT
	esp_task_wdt_reset();
	esp32_uart_process();
}

// Non volatile memory
/**
 * gets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
 * */
uint8_t mcu_eeprom_getc(uint16_t address)
{
#ifndef RAM_ONLY_SETTINGS
	return esp32_eeprom_read(address);
#else
	return 0;
#endif
}

/**
 * sets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
 * */
void mcu_eeprom_putc(uint16_t address, uint8_t value)
{
#ifndef RAM_ONLY_SETTINGS
	esp32_eeprom_write(address, value);
#endif
}

/**
 * flushes all recorded registers into the eeprom.
 * */
void mcu_eeprom_flush(void)
{
#ifndef RAM_ONLY_SETTINGS
	esp32_eeprom_flush();
#endif
}

#ifdef MCU_HAS_I2C
#ifndef mcu_i2c_write
i2c_cmd_handle_t esp32_i2c_cmd;
uint8_t esp32_i2c_cmd_buff[I2C_LINK_RECOMMENDED_SIZE(1)];
uint8_t mcu_i2c_write(uint8_t data, bool send_start, bool send_stop)
{
	esp_err_t ret = ESP_FAIL;

	if (send_start)
	{
		// init
		if (esp32_i2c_cmd != NULL)
		{
			i2c_cmd_link_delete_static(esp32_i2c_cmd);
		}
		memset(esp32_i2c_cmd_buff, 0, I2C_LINK_RECOMMENDED_SIZE(1));
		esp32_i2c_cmd = i2c_cmd_link_create_static(esp32_i2c_cmd_buff, I2C_LINK_RECOMMENDED_SIZE(1));
		ret = i2c_master_start(esp32_i2c_cmd);
		if (ret != ESP_OK)
		{
			return 0;
		}
	}

	ret = i2c_master_write_byte(esp32_i2c_cmd, data, true);
	if (ret != ESP_OK)
	{
		return 0;
	}

	if (send_stop)
	{
		ret = i2c_master_stop(esp32_i2c_cmd);
		if (ret != ESP_OK)
		{
			return 0;
		}

		ret = i2c_master_cmd_begin(I2C_PORT, esp32_i2c_cmd, 100);
		if (ret != ESP_OK)
		{
			return 0;
		}
	}

	return 1;
}
#endif

#ifndef mcu_i2c_read
uint8_t mcu_i2c_read(bool with_ack, bool send_stop)
{
	uint8_t c = 0;
	esp_err_t ret = ESP_FAIL;

	ret = i2c_master_read_byte(esp32_i2c_cmd, &c, (!with_ack) ? I2C_MASTER_ACK : I2C_MASTER_NACK);
	if (ret != ESP_OK)
	{
		return 0;
	}

	if (send_stop)
	{
		ret = i2c_master_stop(esp32_i2c_cmd);
		if (ret != ESP_OK)
		{
			return 0;
		}

		ret = i2c_master_cmd_begin(I2C_PORT, esp32_i2c_cmd, 100);
		if (ret != ESP_OK)
		{
			return 0;
		}
	}

	return c;
}
#endif
#endif

#endif
