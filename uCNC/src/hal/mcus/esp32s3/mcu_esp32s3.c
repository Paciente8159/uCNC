/*
	Name: mcu_esp32s.c
	Description: Implements the µCNC HAL for ESP32S.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 13-03-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

#if (MCU == MCU_ESP32S3)
#include "esp_timer.h"
#include "esp_task_wdt.h"
#include "esp_ipc.h"
#include "driver/timer.h"
#include "soc/i2s_struct.h"
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "../esp32common/esp32_common.h"

#ifndef ITP_SAMPLE_RATE
#define ITP_SAMPLE_RATE (F_STEP_MAX * 2)
#endif

static volatile bool esp32_global_isr_enabled;
static volatile uint32_t mcu_itp_timer_reload;
static volatile bool mcu_itp_timer_running;
hw_timer_t *esp32_step_timer;
MCU_CALLBACK void mcu_itp_isr(void *arg);
MCU_CALLBACK void mcu_gen_pwm_and_servo(void);
MCU_CALLBACK void mcu_gen_step(void);
MCU_CALLBACK void mcu_gpio_isr(void *type);

#if SERVOS_MASK > 0
// also run servo pin signals
static uint32_t servo_tick_counter = 0;
static uint32_t servo_tick_alarm = 0;
static uint8_t mcu_servos[6];
static FORCEINLINE void servo_reset(void)
{
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
}

#define start_servo_timeout(timeout)                      \
	{                                                       \
		servo_tick_alarm = servo_tick_counter + timeout + 64; \
	}

static FORCEINLINE void servo_update(void)
{
	static uint8_t servo_counter = 0;

	switch (servo_counter)
	{
#if ASSERT_PIN(SERVO0)
	case SERVO0_FRAME:
		io_set_output(SERVO0);
		start_servo_timeout(mcu_servos[0]);
		break;
#endif
#if ASSERT_PIN(SERVO1)
	case SERVO1_FRAME:
		io_set_output(SERVO1);
		start_servo_timeout(mcu_servos[1]);
		break;
#endif
#if ASSERT_PIN(SERVO2)
	case SERVO2_FRAME:
		io_set_output(SERVO2);
		start_servo_timeout(mcu_servos[2]);
		break;
#endif
#if ASSERT_PIN(SERVO3)
	case SERVO3_FRAME:
		io_set_output(SERVO3);
		start_servo_timeout(mcu_servos[3]);
		break;
#endif
#if ASSERT_PIN(SERVO4)
	case SERVO4_FRAME:
		io_set_output(SERVO4);
		start_servo_timeout(mcu_servos[4]);
		break;
#endif
#if ASSERT_PIN(SERVO5)
	case SERVO5_FRAME:
		io_set_output(SERVO5);
		start_servo_timeout(mcu_servos[5]);
		break;
#endif
	}

	servo_counter++;
	servo_counter = (servo_counter != 20) ? servo_counter : 0;
}
#endif

MCU_CALLBACK void mcu_gen_pwm_and_servo(void)
{
	static int16_t mcu_soft_io_counter;
	int16_t t = mcu_soft_io_counter;
	t--;
	if (t <= 0)
	{
// updated software PWM pins
#if defined(IC74HC595_HAS_PWMS) || defined(MCU_HAS_SOFT_PWM_TIMER)
		io_soft_pwm_update();
#endif

		// update servo pins
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
			servo_reset();
		}

		// resets every 3ms
		servo_tick_counter = ++counter;
#endif
		mcu_soft_io_counter = (int16_t)roundf((float)ITP_SAMPLE_RATE / 128000.0f);
	}
	else
	{
		mcu_soft_io_counter = t;
	}
}

static volatile uint32_t mcu_itp_timer_reload;
static volatile bool mcu_itp_timer_running;
MCU_CALLBACK void mcu_gen_step(void)
{
	static bool step_reset = true;
	static int32_t mcu_itp_timer_counter;

	// generate steps
	if (mcu_itp_timer_running)
	{
		// stream mode tick
		int32_t t = mcu_itp_timer_counter;
		bool reset = step_reset;
		t -= (int32_t)roundf(1000000.0f / (float)ITP_SAMPLE_RATE);
		if (t <= 0)
		{
			if (!reset)
			{
				mcu_step_cb();
			}
			else
			{
				mcu_step_reset_cb();
			}
			step_reset = !reset;
			mcu_itp_timer_counter = mcu_itp_timer_reload + t;
		}
		else
		{
			mcu_itp_timer_counter = t;
		}
	}
}

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
	uint8_t res = (uint8_t)MAX((int8_t)ceilf(LN(freq * 0.002f)), 0);
	return res;
}
#endif

void mcu_core0_wiredcoms_init(void *arg)
{
	mcu_uart_start();
	mcu_uart2_start();
}

void mcu_core0_wirelesscoms_init(void *arg)
{
	mcu_wifi_init();
	mcu_bt_init();
}

MCU_CALLBACK void mcu_itp_isr(void *arg)
{
#ifdef IC74HC595_CUSTOM_SHIFT_IO
	uint32_t mode = I2S_MODE;
#if defined(IC74HC595_HAS_STEPS) || defined(IC74HC595_HAS_DIRS)
	if (mode == ITP_STEP_MODE_REALTIME)
#endif
#endif
	{
		mcu_gen_step();
	}
#ifdef IC74HC595_CUSTOM_SHIFT_IO
#if defined(IC74HC595_HAS_PWMS) || defined(IC74HC595_HAS_SERVOS)
	if (mode == ITP_STEP_MODE_REALTIME)
#endif
#endif
	{
		mcu_gen_pwm_and_servo();
	}
#if defined(MCU_HAS_ONESHOT_TIMER) && defined(ENABLE_RT_SYNC_MOTIONS)
	mcu_gen_oneshot();
#endif
#ifdef IC74HC595_CUSTOM_SHIFT_IO
#if defined(IC74HC595_HAS_STEPS) || defined(IC74HC595_HAS_DIRS) || defined(IC74HC595_HAS_PWMS) || defined(IC74HC595_HAS_SERVOS)
	// this is where the IO update happens in RT mode
	// this prevents multiple
	if (mode == ITP_STEP_MODE_REALTIME)
	{
		I2SREG.conf_single_data = __atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED);
	}
#endif
#endif

	timer_group_clr_intr_status_in_isr(ITP_TIMER_TG, ITP_TIMER_IDX);
	/* After the alarm has been triggered
		we need enable it again, so it is triggered the next time */
	timer_group_enable_alarm_in_isr(ITP_TIMER_TG, ITP_TIMER_IDX);
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

	mcu_uart_init();
	mcu_uart2_init();
	mcu_usb_init();
	esp_ipc_call_blocking(0, mcu_core0_wiredcoms_init, NULL);

	/**
	 * EEPROM config
	 */

	// starts EEPROM before UART to enable WiFi and BT settings
#if !defined(RAM_ONLY_SETTINGS)
	mcu_eeprom_init(NVM_STORAGE_SIZE);
#endif

	mcu_wifi_init();

	/**
	 * Timers config
	 */

	// inititialize ITP timer
	timer_config_t itpconfig = {0};
	itpconfig.divider = 2;
	itpconfig.counter_dir = TIMER_COUNT_UP;
	itpconfig.counter_en = TIMER_PAUSE;
	itpconfig.intr_type = TIMER_INTR_MAX;
	itpconfig.alarm_en = TIMER_ALARM_EN;
	itpconfig.auto_reload = true;
	timer_init(ITP_TIMER_TG, ITP_TIMER_IDX, &itpconfig);
	/* Timer's counter will initially start from value below.
		 Also, if auto_reload is set, this value will be automatically reload on alarm */
	timer_set_counter_value(ITP_TIMER_TG, ITP_TIMER_IDX, 0x00000000ULL);
	/* Configure the alarm value and the interrupt on alarm. */
	timer_set_alarm_value(ITP_TIMER_TG, ITP_TIMER_IDX, (uint64_t)(getApbFrequency() / (ITP_SAMPLE_RATE * 2)));
	// register PWM isr
	timer_isr_register(ITP_TIMER_TG, ITP_TIMER_IDX, mcu_itp_isr, NULL, 0, NULL);
	timer_enable_intr(ITP_TIMER_TG, ITP_TIMER_IDX);
	timer_start(ITP_TIMER_TG, ITP_TIMER_IDX);

#ifdef IC74HC595_CUSTOM_SHIFT_IO
	// esp32_i2s_extender_init();
#endif

	// initialize rtc timer (currently on core 1)
	xTaskCreatePinnedToCore(mcu_rtc_task, "rtcTask", 8192, NULL, 7, NULL, CONFIG_ARDUINO_RUNNING_CORE);
	mcu_enable_global_isr();
}

// ISR
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
// bool mcu_get_global_isr(void)
// {
// 	return esp32_global_isr_enabled;
// }
bool mcu_get_global_isr(void)
{
	if (xPortInIsrContext())
	{
		return false;
	}
	
	uint32_t ps;
	__asm__ volatile("rsr.ps %0" : "=a"(ps));
	// INTLEVEL is bits [3:0] of PS
	return ((ps & 0xF) < 2);
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
	if (!mcu_itp_timer_running)
	{
		mcu_itp_timer_reload = ticks * prescaller;
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
		mcu_itp_timer_reload = ticks * prescaller;
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
	// mcu_wifi_dotasks();
	// esp_task_wdt_reset();
	// mcu_bt_dotasks();
}

#endif
