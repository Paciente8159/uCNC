/*
	Name: mcu_esp32.c
	Description: Implements the µCNC HAL for ESP32.

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

#if (MCU == MCU_ESP32C3)
#include "esp_timer.h"
#include "esp_task_wdt.h"
#include "esp_ipc.h"
#include "driver/uart.h"
#include "driver/timer.h"
#include "soc/i2s_struct.h"
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

static volatile bool esp32_global_isr_enabled;
#ifdef IC74HC595_CUSTOM_SHIFT_IO
volatile uint32_t ic74hc595_i2s_pins;
#endif
hw_timer_t *esp32_step_timer;

MCU_CALLBACK void mcu_itp_isr(void *arg);
MCU_CALLBACK void mcu_gpio_isr(void *type);

#ifndef ITP_SAMPLE_RATE
#define ITP_SAMPLE_RATE (F_STEP_MAX * 2)
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

void mcu_coms_dotasks(void *arg)
{
	// loop through received data
	for (;;)
	{
		mcu_uart_dotasks();
		taskYIELD();
		mcu_uart2_dotasks();
		taskYIELD();
		mcu_usb_dotasks();
		taskYIELD();
		mcu_wifi_dotasks();
		taskYIELD();
		mcu_bt_dotasks();
	}
}

void mcu_rtc_task(void *arg)
{
	portTickType xLastWakeTimeUpload = xTaskGetTickCount();
	for (;;)
	{
		mcu_rtc_cb(mcu_millis());
		vTaskDelayUntil(&xLastWakeTimeUpload, (1 / portTICK_RATE_MS));
	}
}

MCU_CALLBACK void mcu_itp_isr(void *arg)
{
#ifdef IC74HC595_CUSTOM_SHIFT_IO
	if (I2S_MODE == ITP_STEP_MODE_REALTIME)
#endif
	{
		signal_timer.us_step = (1000000 / (ITP_SAMPLE_RATE >> 1));
		// run twice per timer isr (step up and step down at limit speed)
		mcu_gen_step();
		mcu_gen_pwm();
		mcu_gen_servo();
#if defined(MCU_HAS_ONESHOT_TIMER) && defined(ENABLE_RT_SYNC_MOTIONS)
		mcu_gen_oneshot();
#endif
#ifdef IC74HC595_CUSTOM_SHIFT_IO
		i2s_write_word(__atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED));
#endif
	}

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

	/**
	 * IO conficuration
	 */

	mcu_io_init();

#ifdef MCU_HAS_SPI
	spi_config_t spi_conf = {0};
	spi_conf.mode = SPI_MODE;
	mcu_spi_init();
	mcu_spi_config(spi_conf, SPI_FREQ);
#endif

#ifdef MCU_HAS_SPI2
	spi_config_t spi2_conf = {0};
	spi2_conf.mode = SPI2_MODE;
	mcu_spi2_init();
	mcu_spi2_config(spi2_conf, SPI2_FREQ);
#endif

#ifdef MCU_HAS_I2C
	mcu_i2c_config(I2C_FREQ);
#endif

	/**
	 * Wired Communications config
	 */
	mcu_uart_init();
	mcu_uart_start();
	mcu_uart2_init();
	mcu_uart2_start();
	mcu_usb_init();

	/**
	 * EEPROM config
	 */

	// starts EEPROM before UART to enable WiFi and BT settings
#if !defined(RAM_ONLY_SETTINGS)
	mcu_eeprom_init(NVM_STORAGE_SIZE);
#endif

	/**
	 * Wireless Communications config
	 */

	mcu_wifi_init();
	mcu_bt_init();

	// initialize rtc timer (currently on core 1)
	// xTaskCreate(mcu_coms_dotasks, "comsTask", 8192, NULL, 7, NULL);

	/**
	 * Timers config
	 */

	// inititialize ITP timer
	timer_config_t itpconfig = {0};
	itpconfig.clk_src = TIMER_SRC_CLK_APB;
	itpconfig.divider = 2;
	itpconfig.counter_dir = TIMER_COUNT_UP;
	itpconfig.counter_en = TIMER_PAUSE;
	itpconfig.intr_type = TIMER_INTR_LEVEL;
	itpconfig.alarm_en = TIMER_ALARM_EN;
	itpconfig.auto_reload = true;
	timer_init(ITP_TIMER_TG, ITP_TIMER_IDX, &itpconfig);
	/* Timer's counter will initially start from value below.
		 Also, if auto_reload is set, this value will be automatically reload on alarm */
	timer_set_counter_value(ITP_TIMER_TG, ITP_TIMER_IDX, 0x00000000ULL);
	/* Configure the alarm value and the interrupt on alarm. */
	timer_set_alarm_value(ITP_TIMER_TG, ITP_TIMER_IDX, (uint64_t)(getApbFrequency() / ITP_SAMPLE_RATE));
	timer_set_alarm(ITP_TIMER_TG, ITP_TIMER_IDX, TIMER_ALARM_EN);
	timer_isr_register(ITP_TIMER_TG, ITP_TIMER_IDX, mcu_itp_isr, NULL, ESP_INTR_FLAG_IRAM, NULL);
	timer_enable_intr(ITP_TIMER_TG, ITP_TIMER_IDX);

#ifdef IC74HC595_CUSTOM_SHIFT_IO
	mcu_i2s_extender_init();
#else
	timer_start(ITP_TIMER_TG, ITP_TIMER_IDX);
#endif

	// initialize rtc timer (currently on core 1)
	xTaskCreatePinnedToCore(mcu_rtc_task, "rtcTask", 8192, NULL, 7, NULL, CONFIG_ARDUINO_RUNNING_CORE);

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
	if (xPortInIsrContext())
	{
		return false;
	}

	uint32_t mstatus;
	asm volatile("csrr %0, mstatus" : "=r"(mstatus));

	// MIE (Machine Interrupt Enable) is bit 3 of mstatus
	return (mstatus & (1 << 3)) != 0;
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
	uint32_t totalticks = (uint32_t)((500000.0f) / frequency);
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
	uint32_t totalticks = (uint32_t)ticks * prescaller;
	return 500000.0f / ((float)totalticks);
}

/**
 * starts the timer interrupt that generates the step pulses for the interpolator
 * */

void mcu_start_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	if (!signal_timer.step_alarm_en)
	{
		signal_timer.itp_reload = ticks * prescaller;
		signal_timer.step_alarm_en = true;
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
	if (signal_timer.step_alarm_en)
	{
		signal_timer.itp_reload = ticks * prescaller;
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
	if (signal_timer.step_alarm_en)
	{
		signal_timer.step_alarm_en = false;
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

uint32_t mcu_free_micros()
{
	return (uint32_t)(esp_system_get_time() % 1000);
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
	mcu_uart_dotasks();
	esp_task_wdt_reset();
	mcu_uart2_dotasks();
	esp_task_wdt_reset();
	mcu_usb_dotasks();
	esp_task_wdt_reset();
	mcu_wifi_dotasks();
	esp_task_wdt_reset();
	mcu_bt_dotasks();
}

#ifdef MCU_HAS_ONESHOT_TIMER

uint32_t esp32_oneshot_counter;
uint32_t esp32_oneshot_reload;

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
#if defined(MCU_HAS_ONESHOT_TIMER) && defined(ENABLE_RT_SYNC_MOTIONS)
	esp32_oneshot_reload = ((ITP_SAMPLE_RATE >> 1) / timeout);
#elif defined(MCU_HAS_ONESHOT_TIMER)
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
#endif
}
#endif

/**
 * starts the timeout. Once hit the the respective callback is called
 * */
#ifndef mcu_start_timeout
MCU_CALLBACK void mcu_start_timeout()
{
#if defined(MCU_HAS_ONESHOT_TIMER) && defined(ENABLE_RT_SYNC_MOTIONS)
	esp32_oneshot_counter = esp32_oneshot_reload;
#elif defined(MCU_HAS_ONESHOT_TIMER)
	timer_start(ONESHOT_TIMER_TG, ONESHOT_TIMER_IDX);
#endif
}
#endif

// software generated oneshot for RT steps like laser PPI
#if defined(MCU_HAS_ONESHOT_TIMER) && defined(ENABLE_RT_SYNC_MOTIONS)
MCU_CALLBACK void mcu_gen_oneshot(void)
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
