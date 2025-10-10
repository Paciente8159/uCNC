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

#if (MCU == MCU_RP2350)
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

static volatile bool rp2350_global_isr_enabled;

extern void rp2350_uart_init(int baud);
extern void rp2350_uart_process(void);

extern void rp2350_eeprom_init(int size);
extern uint8_t rp2350_eeprom_read(uint16_t address);
extern void rp2350_eeprom_write(uint16_t address, uint8_t value);
extern void rp2350_eeprom_flush(void);

uint8_t rp2350_pwm[16];

#ifdef IC74HC595_CUSTOM_SHIFT_IO

#ifndef IC74HC595_PIO_FREQ
#define IC74HC595_PIO_FREQ 20000000UL
#endif

#if IC74HC595_COUNT != 4
#error "IC74HC595_COUNT must be 4 to use ESP32 I2S mode for IO shifting"
#endif

#ifdef IC74HC595_HAS_PWMS
#warning "IC74HC595 on RP2350 does not support soft PWM yet!"
#endif

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "ic74hc595.pio.h"

static PIO pio_ic74hc595;
static uint sm_ic74hc595;

void ic74hc595_pio_init()
{
	pio_ic74hc595 = pio0;
	sm_ic74hc595 = 0;
	uint offset = pio_add_program(pio_ic74hc595, &ic74hc595_program);
	ic74hc595_program_init(pio_ic74hc595, sm_ic74hc595, offset, IC74HC595_PIO_DATA, IC74HC595_PIO_CLK, IC74HC595_PIO_LATCH, IC74HC595_PIO_FREQ);
}

// disable this function
// IO will be updated at a fixed rate
MCU_CALLBACK void shift_register_io_pins(void)
{
	ic74hc595_program_write(pio_ic74hc595, sm_ic74hc595, *((volatile uint32_t *)&ic74hc595_io_pins[0]));
}

#endif

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
typedef struct rp2350_alarm_
{
	uint32_t timeout;
	void (*alarm_cb)(void);
	struct rp2350_alarm_ *next;
} rp2350_alarm_t;

volatile rp2350_alarm_t *mcu_alarms;
// the alarm isr processes all executes all pending RTC, ONESHOT and SERVO isr's
void mcu_alarm_isr(void)
{
	hw_clear_bits(&timer_hw->intr, (1U << ALARM_TIMER));
	if (mcu_alarms)
	{
		while (mcu_alarms->timeout < (uint32_t)timer_hw->timerawl)
		{
			rp2350_alarm_t *alarm = (rp2350_alarm_t *)mcu_alarms;
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
void mcu_enqueue_alarm(rp2350_alarm_t *a, uint32_t timeout_us)
{
	uint64_t target = timer_hw->timerawl + timeout_us;
	a->timeout = (uint32_t)target;
	a->next = NULL;

	__ATOMIC__
	{
		rp2350_alarm_t *ptr = (rp2350_alarm_t *)mcu_alarms;
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
					a->next = (rp2350_alarm_t *)mcu_alarms;
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
static rp2350_alarm_t servo_alarm;
#define servo_start_timeout(X) mcu_enqueue_alarm(&servo_alarm, (X))

static void mcu_clear_servos(void)
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
#endif

static rp2350_alarm_t rtc_alarm;

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
		io_set_output(SERVO0);
		break;
#endif
#if ASSERT_PIN(SERVO1)
	case SERVO1_FRAME:
		io_set_output(SERVO1);
		servo_start_timeout(mcu_servos[1]);
		break;
#endif
#if ASSERT_PIN(SERVO2)
	case SERVO2_FRAME:
		io_set_output(SERVO2);
		servo_start_timeout(mcu_servos[2]);
		break;
#endif
#if ASSERT_PIN(SERVO3)
	case SERVO3_FRAME:
		io_set_output(SERVO3);
		servo_start_timeout(mcu_servos[3]);
		break;
#endif
#if ASSERT_PIN(SERVO4)
	case SERVO4_FRAME:
		io_set_output(SERVO4);
		servo_start_timeout(mcu_servos[4]);
		break;
#endif
#if ASSERT_PIN(SERVO5)
	case SERVO5_FRAME:
		io_set_output(SERVO5);
		servo_start_timeout(mcu_servos[5]);
		break;
#endif
	}

	servo_counter++;
	ms_servo_counter = (servo_counter != 20) ? servo_counter : 0;

#endif
	mcu_rtc_cb(millis());
}

/**
 * Multicore code
 * **/
void rp2350_core0_loop()
{
	rp2350_uart_process();
}

void setup1()
{
}

void loop1()
{
	for (;;)
	{
		cnc_run();
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
	mcu_io_init();

#ifdef IC74HC595_CUSTOM_SHIFT_IO
	ic74hc595_pio_init();
#endif
#ifndef RAM_ONLY_SETTINGS
	rp2350_eeprom_init(NVM_STORAGE_SIZE); // 2K Emulated EEPROM
#endif

	rp2350_uart_init(BAUDRATE);

	pinMode(LED_BUILTIN, OUTPUT);
	// init rtc, oneshot and servo alarms
	mcu_alarms_init();
	rtc_alarm.alarm_cb = &mcu_rtc_isr;
	mcu_enqueue_alarm(&rtc_alarm, 500000UL);

#if SERVOS_MASK > 0
	servo_alarm.alarm_cb = &mcu_clear_servos;
#endif

#ifdef MCU_HAS_SPI
	spi_config_t spi_conf = {0};
	spi_conf.mode = SPI_MODE;
	mcu_spi_config(spi_conf, SPI_FREQ);
#endif

#ifdef MCU_HAS_SPI2
	spi_config_t spi2_conf = {0};
	spi2_conf.mode = SPI2_MODE;
	mcu_spi2_config(spi2_conf, SPI2_FREQ);
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

/**
 * sets the pwm for a servo (50Hz with tON between 1~2ms)
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_set_servo
void mcu_set_servo(uint8_t servo, uint8_t value)
{
	#if SERVOS_MASK > 0
	mcu_servos[servo - SERVO_PINS_OFFSET] = (((2000UL * value) >> 8) + 500); // quick aproximation should be divided by 255 but it's a faste quick approach
	#else
	(void)servo;
	(void)value;
	#endif
}
#endif

/**
 * gets the pwm for a servo (50Hz with tON between 1~2ms)
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_servo
uint8_t mcu_get_servo(uint8_t servo)
{
	#if SERVOS_MASK > 0
	return (((mcu_servos[servo - SERVO_PINS_OFFSET] - 500) << 8) / 2000);
	#else
	(void)servo;
	return 0;
	#endif
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
	rp2350_global_isr_enabled = true;
}
#endif

/**
 * disables global interrupts on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_disable_global_isr
void mcu_disable_global_isr(void)
{
	rp2350_global_isr_enabled = false;
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
	return rp2350_global_isr_enabled;
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
	frequency = CLAMP((float)F_STEP_MIN, frequency, (float)F_STEP_MAX);
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
 * runs all internal tasks of the MCU.
 * for the moment these are:
 *   - if USB is enabled and MCU uses tinyUSB framework run tinyUSB tud_task
 * */
void mcu_dotasks(void)
{
#ifndef RP2350_RUN_MULTICORE
	rp2350_uart_process();
#endif
}

// Non volatile memory
/**
 * gets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
 * */
uint8_t mcu_eeprom_getc(uint16_t address)
{
	if (NVM_STORAGE_SIZE <= address)
	{
		DBGMSG("EEPROM invalid address @ %u", address);
		return 0;
	}
#ifndef RAM_ONLY_SETTINGS
	return rp2350_eeprom_read(address);
#else
	return 0;
#endif
}

/**
 * sets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
 * */
void mcu_eeprom_putc(uint16_t address, uint8_t value)
{
	if (NVM_STORAGE_SIZE <= address)
	{
		DBGMSG("EEPROM invalid address @ %u", address);
	}
#ifndef RAM_ONLY_SETTINGS
	rp2350_eeprom_write(address, value);
#endif
}

/**
 * flushes all recorded registers into the eeprom.
 * */
void mcu_eeprom_flush(void)
{
#ifndef RAM_ONLY_SETTINGS
	rp2350_eeprom_flush();
#endif
}

#ifdef MCU_HAS_ONESHOT_TIMER
/**
 * configures a single shot timeout in us
 * */

static uint32_t rp2350_oneshot_reload;
static rp2350_alarm_t oneshot_alarm;
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
	rp2350_oneshot_reload = (1000000UL / timeout);
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
	// rp2350_oneshot_reload = target;

	// target += (uint32_t)timer_hw->timerawl;

	// // Write the lower 32 bits of the target time to the alarm which
	// // will arm it
	// timer_hw->alarm[ONESHOT_TIMER] = target;
	mcu_enqueue_alarm(&oneshot_alarm, rp2350_oneshot_reload);
}
#endif
#endif

/**
 *
 * This handles SPI communications
 *
 * **/

#if defined(MCU_HAS_SPI) && !defined(USE_ARDUINO_SPI_LIBRARY)
#include <hardware/spi.h>
#include <hardware/dma.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"

static spi_config_t rp2350_spi_config;

void mcu_spi_init(void)
{
	spi_init(SPI_HW, SPI_FREQ);
	// Enable SPI 0 at 1 MHz and connect to GPIOs
	gpio_set_function(SPI_CLK_BIT, GPIO_FUNC_SPI);
	gpio_set_function(SPI_SDO_BIT, GPIO_FUNC_SPI);
	gpio_set_function(SPI_SDI_BIT, GPIO_FUNC_SPI);
#ifdef SPI_CS_BIT
	gpio_init(SPI_CS_BIT);
#endif
}

void mcu_spi_config(spi_config_t config, uint32_t frequency)
{
	rp2350_spi_config = config;
	spi_deinit(SPI_HW);
	spi_set_baudrate(SPI_HW, frequency);
	spi_set_format(SPI_HW, 8, ((config.mode >> 1) & 0x01), (config.mode & 0x01), SPI_MSB_FIRST);
	mcu_spi_init();
}

uint8_t mcu_spi_xmit(uint8_t data)
{
	spi_get_hw(SPI_HW)->dr = data;
	while ((spi_get_hw(SPI_HW)->sr & SPI_SSPSR_BSY_BITS))
		;
	return (spi_get_hw(SPI_HW)->dr & 0xFF);
}

void mcu_spi_start(spi_config_t config, uint32_t frequency)
{
	mcu_spi_config(config, frequency);
}

void mcu_spi_stop(void)
{
}

#ifndef BULK_SPI_TIMEOUT
#define BULK_SPI_TIMEOUT (1000 / INTERPOLATOR_FREQ)
#endif

bool mcu_spi_bulk_transfer(const uint8_t *out, uint8_t *in, uint16_t len)
{
	static bool transmitting = false;
	// Grab some unused dma channels
	static uint dma_tx = 0;
	static uint dma_rx = 0;

	if (rp2350_spi_config.enable_dma)
	{
		if (!transmitting)
		{
			uint32_t startmask = (1u << dma_tx);
			// We set the outbound DMA to transfer from a memory buffer to the SPI transmit FIFO paced by the SPI TX FIFO DREQ
			// The default is for the read address to increment every element (in this case 1 byte = DMA_SIZE_8)
			// and for the write address to remain unchanged.
			dma_tx = dma_claim_unused_channel(true);

			dma_channel_config c = dma_channel_get_default_config(dma_tx);
			channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
			channel_config_set_dreq(&c, spi_get_dreq(spi_default, true));
			dma_channel_configure(dma_tx, &c,
														&spi_get_hw(SPI_HW)->dr, // write address
														out,										 // read address
														len,										 // element count (each element is of size transfer_data_size)
														false);									 // don't start yet

			if (in)
			{
				// We set the inbound DMA to transfer from the SPI receive FIFO to a memory buffer paced by the SPI RX FIFO DREQ
				// We configure the read address to remain unchanged for each element, but the write
				// address to increment (so data is written throughout the buffer)
				dma_rx = dma_claim_unused_channel(true);
				c = dma_channel_get_default_config(dma_rx);
				channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
				channel_config_set_dreq(&c, spi_get_dreq(spi_default, false));
				channel_config_set_read_increment(&c, false);
				channel_config_set_write_increment(&c, true);
				dma_channel_configure(dma_rx, &c,
															in,											 // write address
															&spi_get_hw(SPI_HW)->dr, // read address
															len,										 // element count (each element is of size transfer_data_size)
															false);									 // don't start yet

				startmask |= (1u << dma_rx);
			}

			// start the DMA transmission
			dma_start_channel_mask(startmask);
			transmitting = true;
		}
		else
		{
			if (!dma_channel_is_busy(dma_tx) && !dma_channel_is_busy(dma_rx))
			{
				dma_channel_unclaim(dma_tx);
				dma_channel_unclaim(dma_rx);
				transmitting = false;
			}
		}
	}
	else
	{
		transmitting = false;
		uint32_t timeout = BULK_SPI_TIMEOUT + mcu_millis();
		while (len--)
		{
			uint8_t c = mcu_spi_xmit(*out++);
			if (in)
			{
				*in++ = c;
			}

			if (timeout < mcu_millis())
			{
				timeout = BULK_SPI_TIMEOUT + mcu_millis();
				cnc_dotasks();
			}
		}

		return false;
	}

	return transmitting;
}
#endif

#if defined(MCU_HAS_SPI2) && !defined(USE_ARDUINO_SPI_LIBRARY)
#include <hardware/spi.h>
#include <hardware/dma.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"

static spi_config_t rp2350_spi2_config;

void mcu_spi2_init(void)
{
	spi_init(SPI2_HW, SPI2_FREQ);
	// Enable SPI 0 at 1 MHz and connect to GPIOs
	gpio_set_function(SPI2_CLK_BIT, GPIO_FUNC_SPI);
	gpio_set_function(SPI2_SDO_BIT, GPIO_FUNC_SPI);
	gpio_set_function(SPI2_SDI_BIT, GPIO_FUNC_SPI);
#ifdef SPI2_CS_BIT
	gpio_init(SPI2_CS_BIT);
#endif
}

void mcu_spi2_config(spi_config_t config, uint32_t frequency)
{
	rp2350_spi2_config = config;
	spi_deinit(SPI2_HW);
	spi_set_baudrate(SPI2_HW, frequency);
	spi_set_format(SPI2_HW, 8, ((config.mode >> 1) & 0x01), (config.mode & 0x01), SPI_MSB_FIRST);
	mcu_spi2_init();
}

uint8_t mcu_spi2_xmit(uint8_t data)
{
	spi_get_hw(SPI2_HW)->dr = data;
	while ((spi_get_hw(SPI2_HW)->sr & SPI_SSPSR_BSY_BITS))
		;
	return (spi_get_hw(SPI2_HW)->dr & 0xFF);
}

void mcu_spi2_start(spi_config_t config, uint32_t frequency)
{
	mcu_spi2_config(config, frequency);
}

void mcu_spi2_stop(void)
{
}

#ifndef BULK_SPI2_TIMEOUT
#define BULK_SPI2_TIMEOUT (1000 / INTERPOLATOR_FREQ)
#endif

bool mcu_spi2_bulk_transfer(const uint8_t *out, uint8_t *in, uint16_t len)
{
	static bool transmitting = false;
	// Grab some unused dma channels
	static uint dma_tx = 0;
	static uint dma_rx = 0;

	if (rp2350_spi2_config.enable_dma)
	{
		if (!transmitting)
		{
			uint32_t startmask = (1u << dma_tx);
			// We set the outbound DMA to transfer from a memory buffer to the SPI transmit FIFO paced by the SPI TX FIFO DREQ
			// The default is for the read address to increment every element (in this case 1 byte = DMA_SIZE_8)
			// and for the write address to remain unchanged.
			dma_tx = dma_claim_unused_channel(true);

			dma_channel_config c = dma_channel_get_default_config(dma_tx);
			channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
			channel_config_set_dreq(&c, spi_get_dreq(spi_default, true));
			dma_channel_configure(dma_tx, &c,
														&spi_get_hw(SPI2_HW)->dr, // write address
														out,										 // read address
														len,										 // element count (each element is of size transfer_data_size)
														false);									 // don't start yet

			if (in)
			{
				// We set the inbound DMA to transfer from the SPI receive FIFO to a memory buffer paced by the SPI RX FIFO DREQ
				// We configure the read address to remain unchanged for each element, but the write
				// address to increment (so data is written throughout the buffer)
				dma_rx = dma_claim_unused_channel(true);
				c = dma_channel_get_default_config(dma_rx);
				channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
				channel_config_set_dreq(&c, spi_get_dreq(spi_default, false));
				channel_config_set_read_increment(&c, false);
				channel_config_set_write_increment(&c, true);
				dma_channel_configure(dma_rx, &c,
															in,											 // write address
															&spi_get_hw(SPI2_HW)->dr, // read address
															len,										 // element count (each element is of size transfer_data_size)
															false);									 // don't start yet

				startmask |= (1u << dma_rx);
			}

			// start the DMA transmission
			dma_start_channel_mask(startmask);
			transmitting = true;
		}
		else
		{
			if (!dma_channel_is_busy(dma_tx) && !dma_channel_is_busy(dma_rx))
			{
				dma_channel_unclaim(dma_tx);
				dma_channel_unclaim(dma_rx);
				transmitting = false;
			}
		}
	}
	else
	{
		transmitting = false;
		uint32_t timeout = BULK_SPI2_TIMEOUT + mcu_millis();
		while (len--)
		{
			uint8_t c = mcu_spi2_xmit(*out++);
			if (in)
			{
				*in++ = c;
			}

			if (timeout < mcu_millis())
			{
				timeout = BULK_SPI2_TIMEOUT + mcu_millis();
				cnc_dotasks();
			}
		}

		return false;
	}

	return transmitting;
}
#endif

#endif
