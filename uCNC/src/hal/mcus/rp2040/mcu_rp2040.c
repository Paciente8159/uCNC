/*
	Name: mcu_rpi_pico.c
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

#if (MCU == MCU_RP2040)
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

static volatile bool rp2040_global_isr_enabled;

extern void rp2040_uart_init(int baud);
extern void rp2040_uart_flush(void);
extern void rp2040_uart_write(char c);
extern bool rp2040_uart_rx_ready(void);
extern bool rp2040_uart_tx_ready(void);
extern void rp2040_uart_process(void);

extern void rp2040_eeprom_init(int size);
extern uint8_t rp2040_eeprom_read(uint16_t address);
extern void rp2040_eeprom_write(uint16_t address, uint8_t value);
extern void rp2040_eeprom_flush(void);

#ifdef MCU_HAS_I2C
extern void rp2040_i2c_init(uint32_t freq);
extern void rp2040_spi_config(uint32_t freq);
extern uint8_t rp2040_i2c_write(uint8_t data, bool send_start, bool send_stop);
uint8_t rp2040_i2c_read(bool with_ack, bool send_stop);
#endif

uint8_t rp2040_pwm[16];

void mcu_din_isr(void)
{
	mcu_inputs_changed_cb();
}

void mcu_probe_isr(void)
{
	mcu_probe_changed_cb();
}

void mcu_limits_isr(void)
{
	mcu_limits_changed_cb();
}

void mcu_controls_isr(void)
{
	mcu_controls_changed_cb();
}

// RTC, ONESHOT, and SERVO alarms
// slow rate alarms all share a single timer
typedef struct rp2040_alarm_
{
	uint32_t timeout;
	void (*alarm_cb)(void);
	struct rp2040_alarm_ *next;
} rp2040_alarm_t;

volatile rp2040_alarm_t *mcu_alarms;
// the alarm isr processes all executes all pending RTC, ONESHOT and SERVO isr's
void mcu_alarm_isr(void)
{
	hw_clear_bits(&timer_hw->intr, (1U << ALARM_TIMER));
	if (mcu_alarms)
	{
		while (mcu_alarms->timeout < (uint32_t)timer_hw->timerawl)
		{
			rp2040_alarm_t *alarm = (rp2040_alarm_t *)mcu_alarms;
			// advance
			mcu_alarms = mcu_alarms->next;
			// dequeue
			alarm->next = NULL;
			if (alarm->alarm_cb)
			{
				alarm->alarm_cb();
			}

			// no more alarms
			if (!mcu_alarms)
			{
				return;
			}
		}

		timer_hw->alarm[ALARM_TIMER] = mcu_alarms->timeout;
	}
	else
	{
		// just re-arm
		timer_hw->alarm[ALARM_TIMER] = 0xFFFFFFFF;
	}
}

// initializes the alarm isr
void mcu_alarms_init(void)
{
	hw_set_bits(&timer_hw->inte, 1u << ALARM_TIMER);
	// Set irq handler for alarm irq
	irq_set_exclusive_handler(ALARM_TIMER_IRQ, mcu_alarm_isr);
	// Enable the alarm irq
	irq_set_enabled(ALARM_TIMER_IRQ, true);

	// just re-arm
	timer_hw->alarm[ALARM_TIMER] = 0xFFFFFFFF;
}

// enqueues an alarm for execution
void mcu_enqueue_alarm(rp2040_alarm_t *a, uint32_t timeout_us)
{
	uint64_t target = timer_hw->timerawl + timeout_us;
	a->timeout = (uint32_t)target;
	a->next = NULL;

	__ATOMIC__
	{
		rp2040_alarm_t *ptr = (rp2040_alarm_t *)mcu_alarms;
		// is the only
		if (!ptr)
		{
			mcu_alarms = a;
			// adjust alarm to next event
			timer_hw->alarm[ALARM_TIMER] = mcu_alarms->timeout;
		}
		else
		{
			while (ptr)
			{
				// comes before first alarm in queue
				if (ptr->timeout < target && ptr->timeout < (uint32_t)target && ptr == mcu_alarms)
				{
					a->next = (rp2040_alarm_t *)mcu_alarms;
					mcu_alarms = a;
					break;
				}

				// will be the last in queue
				if (!ptr->next)
				{
					ptr->next = a;
					break;
				}

				// insert mid queue
				if (ptr->next->timeout > target)
				{
					a->next = ptr->next;
					ptr->next = a;
					break;
				}

				ptr = ptr->next;
			}
		}
	}
}

#if SERVOS_MASK > 0

static uint8_t mcu_servos[6];
static rp2040_alarm_t servo_alarm;
#define servo_start_timeout(X) mcu_enqueue_alarm(&servo_alarm, (X))

static void mcu_clear_servos(void)
{
#if ASSERT_PIN(SERVO0)
	mcu_clear_output(SERVO0);
#endif
#if ASSERT_PIN(SERVO1)
	mcu_clear_output(SERVO1);
#endif
#if ASSERT_PIN(SERVO2)
	mcu_clear_output(SERVO2);
#endif
#if ASSERT_PIN(SERVO3)
	mcu_clear_output(SERVO3);
#endif
#if ASSERT_PIN(SERVO4)
	mcu_clear_output(SERVO4);
#endif
#if ASSERT_PIN(SERVO5)
	mcu_clear_output(SERVO5);
#endif
}
#endif

static rp2040_alarm_t rtc_alarm;

void mcu_rtc_isr(void)
{
	// enqueue alarm again
	mcu_enqueue_alarm(&rtc_alarm, 1000UL);

	// counts to 20 and reloads
#if SERVOS_MASK > 0
	static uint8_t ms_servo_counter = 0;
	uint8_t servo_counter = ms_servo_counter;

	switch (servo_counter)
	{
#if ASSERT_PIN(SERVO0)
	case SERVO0_FRAME:
		servo_start_timeout(mcu_servos[0]);
		mcu_set_output(SERVO0);
		break;
#endif
#if ASSERT_PIN(SERVO1)
	case SERVO1_FRAME:
		mcu_set_output(SERVO1);
		servo_start_timeout(mcu_servos[1]);
		break;
#endif
#if ASSERT_PIN(SERVO2)
	case SERVO2_FRAME:
		mcu_set_output(SERVO2);
		servo_start_timeout(mcu_servos[2]);
		break;
#endif
#if ASSERT_PIN(SERVO3)
	case SERVO3_FRAME:
		mcu_set_output(SERVO3);
		servo_start_timeout(mcu_servos[3]);
		break;
#endif
#if ASSERT_PIN(SERVO4)
	case SERVO4_FRAME:
		mcu_set_output(SERVO4);
		servo_start_timeout(mcu_servos[4]);
		break;
#endif
#if ASSERT_PIN(SERVO5)
	case SERVO5_FRAME:
		mcu_set_output(SERVO5);
		servo_start_timeout(mcu_servos[5]);
		break;
#endif
	}

	servo_counter++;
	ms_servo_counter = (servo_counter != 20) ? servo_counter : 0;

#endif
	mcu_rtc_cb(millis());
}

static void mcu_usart_init(void)
{
	rp2040_uart_init(BAUDRATE);
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

	pinMode(LED_BUILTIN, OUTPUT);
	// init rtc, oneshot and servo alarms
	mcu_alarms_init();
	rtc_alarm.alarm_cb = &mcu_rtc_isr;
	mcu_enqueue_alarm(&rtc_alarm, 500000UL);

#if SERVOS_MASK > 0
	servo_alarm.alarm_cb = &mcu_clear_servos;
#endif

#ifndef RAM_ONLY_SETTINGS
	rp2040_eeprom_init(1024); // 1K Emulated EEPROM
#endif
#ifdef MCU_HAS_SPI
	rp2040_spi_init(SPI_FREQ, SPI_MODE);
#endif

#ifdef MCU_HAS_I2C
	mcu_i2c_config(I2C_FREQ);
#endif

#ifdef MCU_HAS_ONESHOT
	// // Set irq handler for alarm irq
	// irq_set_exclusive_handler(ONESHOT_IRQ, mcu_oneshot_isr);
	// // Enable the alarm irq
	// irq_set_enabled(ITP_TIMER_IRQ, true);
#endif
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

// ISR
/**
 * enables global interrupts on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_enable_global_isr
void mcu_enable_global_isr(void)
{
	// ets_intr_unlock();
	rp2040_global_isr_enabled = true;
}
#endif

/**
 * disables global interrupts on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_disable_global_isr
void mcu_disable_global_isr(void)
{
	rp2040_global_isr_enabled = false;
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
	return rp2040_global_isr_enabled;
}
#endif

// Step interpolator

static uint32_t mcu_step_counter;
static uint32_t mcu_step_reload;
static void mcu_itp_isr(void)
{
	static bool resetstep = false;

	mcu_disable_global_isr();
	// Clear the alarm irq
	hw_clear_bits(&timer_hw->intr, (1U << ITP_TIMER));
	uint32_t target = (uint32_t)timer_hw->timerawl + mcu_step_reload;
	timer_hw->alarm[ITP_TIMER] = target;

	if (!resetstep)
	{
		mcu_step_cb();
	}

	else
	{
		mcu_step_reset_cb();
	}

	resetstep = !resetstep;
	mcu_enable_global_isr();
}

/**
 * convert step rate to clock cycles
 * */
void mcu_freq_to_clocks(float frequency, uint16_t *ticks, uint16_t *prescaller)
{
	// up and down counter (generates half the step rate at each event)
	uint32_t totalticks = (uint32_t)((float)(1000000UL >> 1) / frequency);
	*prescaller = 1;
	while (totalticks > 0xFFFF)
	{
		(*prescaller) += 1;
		totalticks >>= 1;
	}

	*ticks = (uint16_t)totalticks;
}

float mcu_clocks_to_freq(uint16_t ticks, uint16_t prescaller)
{
	return ((float)(1000000UL >> 1) / (float)(((uint32_t)ticks) << prescaller));
}

/**
 * starts the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_start_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	hw_set_bits(&timer_hw->inte, 1u << ITP_TIMER);
	// Set irq handler for alarm irq
	irq_set_exclusive_handler(ITP_TIMER_IRQ, mcu_itp_isr);
	// Enable the alarm irq
	irq_set_enabled(ITP_TIMER_IRQ, true);
	// Enable interrupt in block and at processor

	// Alarm is only 32 bits so if trying to delay more
	// than that need to be careful and keep track of the upper
	// bits
	uint32_t target = (((uint32_t)ticks) << prescaller);
	mcu_step_reload = target;

	target += (uint32_t)timer_hw->timerawl;

	// Write the lower 32 bits of the target time to the alarm which
	// will arm it
	timer_hw->alarm[ITP_TIMER] = target;
}

/**
 * changes the step rate of the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_change_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	uint32_t target = (((uint32_t)ticks) << prescaller);
	mcu_step_reload = target;

	target += (uint32_t)timer_hw->timerawl;

	// Write the lower 32 bits of the target time to the alarm which
	// will arm it
	timer_hw->alarm[ITP_TIMER] = target;
}

/**
 * stops the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_stop_itp_isr(void)
{
	irq_set_enabled(ITP_TIMER_IRQ, false);
	hw_clear_bits(&timer_hw->inte, 1u << ITP_TIMER);
}

/**
 * gets the MCU running time in milliseconds.
 * the time counting is controled by the internal RTC
 * */
uint32_t mcu_millis()
{
	return millis();
}

/**
 * provides a delay in us (micro seconds)
 * the maximum allowed delay is 255 us
 * */
uint32_t mcu_micros()
{
	return micros();
}

/**
 * runs all internal tasks of the MCU.
 * for the moment these are:
 *   - if USB is enabled and MCU uses tinyUSB framework run tinyUSB tud_task
 * */
void mcu_dotasks(void)
{
	rp2040_uart_process();
}

// Non volatile memory
/**
 * gets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
 * */
uint8_t mcu_eeprom_getc(uint16_t address)
{
#ifndef RAM_ONLY_SETTINGS
	return rp2040_eeprom_read(address);
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
	rp2040_eeprom_write(address, value);
#endif
}

/**
 * flushes all recorded registers into the eeprom.
 * */
void mcu_eeprom_flush(void)
{
#ifndef RAM_ONLY_SETTINGS
	rp2040_eeprom_flush();
#endif
}

#ifdef MCU_HAS_ONESHOT_TIMER
/**
 * configures a single shot timeout in us
 * */

static uint32_t rp2040_oneshot_reload;
static rp2040_alarm_t oneshot_alarm;
static void mcu_oneshot_isr(void)
{
	if (mcu_timeout_cb)
	{
		mcu_timeout_cb();
	}
}

#ifndef mcu_config_timeout
void mcu_config_timeout(mcu_timeout_delgate fp, uint32_t timeout)
{
	// mcu_timeout_cb = fp;
	oneshot_alarm.alarm_cb = &mcu_oneshot_isr;
	rp2040_oneshot_reload = (1000000UL / timeout);
	mcu_timeout_cb = fp;
}
#endif

/**
 * starts the timeout. Once hit the the respective callback is called
 * */
#ifndef mcu_start_timeout
void mcu_start_timeout()
{
	// hw_set_bits(&timer_hw->inte, 1u << ONESHOT_TIMER);
	// // Enable interrupt in block and at processor
	// // Alarm is only 32 bits so if trying to delay more
	// // than that need to be careful and keep track of the upper
	// // bits
	// uint32_t target = (((uint32_t)ticks) << prescaller);
	// rp2040_oneshot_reload = target;

	// target += (uint32_t)timer_hw->timerawl;

	// // Write the lower 32 bits of the target time to the alarm which
	// // will arm it
	// timer_hw->alarm[ONESHOT_TIMER] = target;
	mcu_enqueue_alarm(&oneshot_alarm, rp2040_oneshot_reload);
}
#endif
#endif

#endif
