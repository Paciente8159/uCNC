/*
	Name: mcu_esp8266.c
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

#if (MCU == MCU_ESP8266)

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <c_types.h>
#include "ets_sys.h"
#include "os_type.h"
#include "c_types.h"
#include "gpio.h"
#include "eagle_soc.h"
#include "osapi.h"
#include <user_interface.h>
#ifdef MCU_HAS_I2C
#include "twi.h"
#endif

volatile uint32_t esp8266_global_isr;
static volatile uint32_t mcu_runtime_ms;

#ifndef TIMER_IO_SAMPLE_RATE
#define TIMER_IO_SAMPLE_RATE (F_STEP_MAX * 2)
#endif

/**
 * Buffered outputs
 *
 * ESP8266 is a low IO count MCU. It has only 17 GPIO pins which allows tracking with an uint32_t var
 * It also does not have hardware PWM drivers so all PWM and Servo signals need to be generated using one of the 2 available timers, in this case the ITP timer
 *
 * To minimize the time spent inside the ISR all IO pins values are calculated and buffered in memory (about 10ms worth of motion)
 * The timer ISR simply pulls one value of the buffer and updates the GPIO pins
 *
 * The mcu_dotasks needs to be called frequently to keep a minimal amount of data in the buffer before the timer ISR starves the buffer
 * To ensure that this happens there is also a second call to the buffer feeding function from the RTC timer ISR that computes at least 2ms worth of motion
 *
 * In realtime mode the MCU switches to realtime mode and lowers the ITP step rate to a value that allows for slow stepping rates at realtime.
 * Glitches in PWM and servos might occur but realtime motions should mainly occur when tools are off (like probing, homing, etc...)
 *
 * In the future one possibility that can be experimented is using an emulated encoder to perform the step counting in the ISR with simple code
 */
volatile uint32_t esp8266_io_out;
#if (IC74HC595_COUNT != 0)
#if defined(IC74HC595_HAS_STEPS) || defined(IC74HC595_HAS_DIRS) || defined(IC74HC595_HAS_PWMS) || defined(IC74HC595_HAS_SERVOS)
volatile uint32_t ic74hc595_io_out;
#endif
extern volatile uint8_t ic74hc595_io_pins[IC74HC595_COUNT];
#endif
#if (IC74HC165_COUNT != 0)
extern volatile uint8_t ic74hc165_io_pins[IC74HC165_COUNT];
#endif

static volatile uint8_t esp8266_step_mode;

typedef struct esp8266_io_out_
{
	uint32_t io;
#if defined(IC74HC595_HAS_STEPS) || defined(IC74HC595_HAS_DIRS) || defined(IC74HC595_HAS_PWMS) || defined(IC74HC595_HAS_SERVOS)
	uint32_t ic74hc595;
#endif
} esp8266_io_out_t;

#if (IC74HC595_COUNT >= IC74HC165_COUNT)
#define SHIFT_REGISTER_BYTES IC74HC595_COUNT
#else
#define SHIFT_REGISTER_BYTES IC74HC165_COUNT
#endif

#ifndef SHIFT_REGISTER_SDO
#define SHIFT_REGISTER_SDO DOUT8
#endif

#ifndef SHIFT_REGISTER_SDI
#define SHIFT_REGISTER_SDI DIN8
#endif

#ifndef SHIFT_REGISTER_CLK
#define SHIFT_REGISTER_CLK DOUT9
#endif

#ifndef IC74HC595_LATCH
#define IC74HC595_LATCH DOUT10
#endif

#ifndef IC74HC595_LATCH
#define IC74HC595_LATCH DOUT10
#endif

#ifndef IC74HC165_LOAD
#define IC74HC165_LOAD DOUT11
#endif

#ifndef SHIFT_REGISTER_DELAY_CYCLES
#define SHIFT_REGISTER_DELAY_CYCLES 0
#endif

#define shift_register_delay() mcu_delay_cycles(SHIFT_REGISTER_DELAY_CYCLES)

#ifdef SHIFT_REGISTER_CUSTOM_CALLBACK
DECL_MUTEX(shifter_running);
// custom implementation of the shift register using the SPI port
MCU_CALLBACK void spi_shift_register_io_pins(void)
{
	MUTEX_INIT(shifter_running);

	MUTEX_TAKE(shifter_running)
	{
#if (IC74HC165_COUNT > 0)
		mcu_set_output_gpio(IC74HC165_LOAD);
#endif
#if (IC74HC595_COUNT > 0)
		mcu_clear_output_gpio(IC74HC595_LATCH);
#endif

#if (IC74HC595_COUNT != 0) || (IC74HC165_COUNT != 0)
#if defined(IC74HC595_HAS_STEPS) || defined(IC74HC595_HAS_DIRS) || defined(IC74HC595_HAS_PWMS) || defined(IC74HC595_HAS_SERVOS)
		memcpy((void *)((volatile uint32_t *)(0x60000000 + (0x140))) /*SPI1W0*/, (const void *)ic74hc595_io_pins, IC74HC595_COUNT);
#endif
		SPI1CMD |= SPIBUSY;
#if defined(IC74HC165_HAS_LIMITS) || defined(IC74HC165_HAS_PROBE)
		while (SPI1CMD & SPIBUSY)
			;
		memcpy((void *)ic74hc165_io_pins, (const void *)((volatile uint32_t *)(0x60000000 + (0x140))), IC74HC165_COUNT); // reads the previous SPI value
#endif
#endif
#if (IC74HC165_COUNT > 0)
		mcu_clear_output_gpio(IC74HC165_LOAD);
#endif
#if (IC74HC595_COUNT > 0)
		mcu_set_output_gpio(IC74HC595_LATCH);
#endif
	}
}
#else
#if (IC74HC595_COUNT != 0) || (IC74HC165_COUNT != 0)
// use direct gpio
MCU_CALLBACK void shift_register_io_pins(void)
{
	uint8_t pins[SHIFT_REGISTER_BYTES];

#if (IC74HC165_COUNT > 0)
	memset(pins, 0, IC74HC165_COUNT);
	mcu_set_output_gpio(IC74HC165_LOAD);
#endif
#if (IC74HC595_COUNT > 0)
	memcpy(pins, (const void *)ic74hc595_io_pins, IC74HC595_COUNT);
	mcu_clear_output_gpio(IC74HC595_LATCH);
#endif
	/**
	 * shift bytes
	 */
	for (uint8_t i = SHIFT_REGISTER_BYTES; i != 0;)
	{
		i--;
		asm volatile("": : :"memory");
#if (defined(SHIFT_REGISTER_USE_HW_SPI) && defined(MCU_HAS_SPI))
		pins[i] = mcu_spi_xmit(pins[i]);
#else
		uint8_t pinbyte = pins[i];
		for (uint8_t j = 0x80; j != 0; j >>= 1)
		{
#if (SHIFT_REGISTER_DELAY_CYCLES)
			shift_register_delay();
#endif
			mcu_clear_output_gpio(SHIFT_REGISTER_CLK);
#if (IC74HC595_COUNT > 0)
			// write
			if (pinbyte & j)
			{
				mcu_set_output_gpio(SHIFT_REGISTER_SDO);
			}
			else
			{
				mcu_clear_output_gpio(SHIFT_REGISTER_SDO);
			}
#endif
// read
#if (IC74HC165_COUNT > 0)
			if (mcu_get_input(SHIFT_REGISTER_SDI))
			{
				pinbyte |= j;
			}
			else
			{
				pinbyte &= ~j;
			}
#endif
			mcu_set_output_gpio(SHIFT_REGISTER_CLK);
		}

#if (IC74HC165_COUNT > 0)
		pins[i] = pinbyte;
#endif

#if (SHIFT_REGISTER_DELAY_CYCLES)
		shift_register_delay();
#endif
		mcu_set_output_gpio(SHIFT_REGISTER_CLK);

#endif
	}

#if (IC74HC165_COUNT > 0)
	memcpy((void *)ic74hc165_io_pins, (const void *)pins, IC74HC165_COUNT);
	mcu_clear_output_gpio(IC74HC165_LOAD); // allow a new load
#endif
#if (IC74HC595_COUNT > 0)
	mcu_set_output_gpio(IC74HC595_LATCH);
#endif
}
#endif
#endif

#define OUT_IO_BUFFER_SIZE (TIMER_IO_SAMPLE_RATE * 10 / 1000)		// (10 / 1000) => 10ms
#define OUT_IO_BUFFER_MINIMAL (TIMER_IO_SAMPLE_RATE * 2 / 1000) // (2 / 1000) => 2ms

/**
 * IO buffer/queue implementation
 * Implements a simple circular buffer
 * */
esp8266_io_out_t out_io_buffer[OUT_IO_BUFFER_SIZE];
static volatile uint16_t out_io_tail;
static volatile uint16_t out_io_head;

static void FORCEINLINE out_io_reset(void)
{
	out_io_tail = 0;
	out_io_head = 0;
	memset(out_io_buffer, 0, sizeof(out_io_buffer));
}

static bool FORCEINLINE out_io_full()
{
	uint16_t h = out_io_head + 1;
	h = (h < OUT_IO_BUFFER_SIZE) ? h : 0;
	return (h == out_io_tail);
}

static bool FORCEINLINE out_io_empty()
{
	return (out_io_head == out_io_tail);
}

static void FORCEINLINE out_io_push(esp8266_io_out_t val)
{
	uint16_t h = out_io_head;
	out_io_buffer[h] = val;
	h++;
	h = (h < OUT_IO_BUFFER_SIZE) ? h : 0;
	__ATOMIC__
	{
		out_io_head = h;
	}
}

static esp8266_io_out_t FORCEINLINE out_io_pull(void)
{
	uint16_t t = out_io_tail;
	esp8266_io_out_t val = out_io_buffer[t];

	if (!out_io_empty())
	{
		t++;
		t = (t < OUT_IO_BUFFER_SIZE) ? t : 0;
		out_io_tail = t;
	}
	else
	{
		val.io = esp8266_io_out;
#if defined(IC74HC595_HAS_STEPS) || defined(IC74HC595_HAS_DIRS) || defined(IC74HC595_HAS_PWMS) || defined(IC74HC595_HAS_SERVOS)
		val.ic74hc595 = ic74hc595_io_out;
#endif
	}
	return val;
}

#ifdef MCU_HAS_WIFI
extern void esp8266_wifi_init(void);
extern void esp8266_wifi_dotasks(void);
#endif

ETSTimer esp8266_rtc_timer;

#ifdef MCU_HAS_ONESHOT_TIMER
static uint32_t esp8266_oneshot_counter;
static uint32_t esp8266_oneshot_reload;
static FORCEINLINE void mcu_gen_oneshot(void)
{
	if (esp8266_oneshot_counter)
	{
		esp8266_oneshot_counter--;
		if (!esp8266_oneshot_counter)
		{
			if (mcu_timeout_cb)
			{
				mcu_timeout_cb();
			}
		}
	}
}
#endif

#if SERVOS_MASK > 0
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

static FORCEINLINE void mcu_gen_pwm_and_servo(void)
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
		mcu_soft_io_counter = (int16_t)roundf((float)TIMER_IO_SAMPLE_RATE / 128000.0f);
	}
	else
	{
		mcu_soft_io_counter = t;
	}
}

static volatile uint32_t mcu_itp_timer_reload;
static volatile bool mcu_itp_timer_running;
static FORCEINLINE void mcu_gen_step(void)
{
	static bool step_reset = true;
	static int32_t mcu_itp_timer_counter;

	// generate steps
	if (mcu_itp_timer_running)
	{
		// stream mode tick
		int32_t t = mcu_itp_timer_counter;
		bool reset = step_reset;
		t -= (int32_t)roundf(1000000.0f / (float)TIMER_IO_SAMPLE_RATE);
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

IRAM_ATTR void mcu_itp_isr(void)
{
	if (esp8266_step_mode == ITP_STEP_MODE_REALTIME)
	{
		mcu_gen_step();
		mcu_gen_pwm_and_servo();
		mcu_gen_oneshot();
	}

	esp8266_io_out_t outputs = out_io_pull();
	GPO = (outputs.io & 0xFFFF);
	if (outputs.io & 0x10000)
	{
		GP16O |= 1;
	}
	else
	{
		GP16O &= ~1;
	}

#ifdef SHIFT_REGISTER_CUSTOM_CALLBACK
	memcpy((void *)ic74hc595_io_pins, (const void *)&(outputs.ic74hc595), IC74HC595_COUNT);
	spi_shift_register_io_pins();
#elif (IC74HC595_COUNT != 0) || (IC74HC165_COUNT != 0)
	memcpy((void *)ic74hc595_io_pins, (const void *)&(outputs.ic74hc595), IC74HC595_COUNT);
	shift_register_io_pins();
#endif
}

#undef DBGMSG
#define DBGMSG(fmt, ...)                                       \
	prt_fmt(&mcu_uart_putc, PRINT_CALLBACK, fmt, ##__VA_ARGS__); \
	mcu_uart_flush()

void itp_buffer_dotasks(uint16_t limit)
{
	static volatile bool running = false;
	// __ATOMIC__
	{
		if (running)
		{
			return;
		}
		running = true;
	}

	uint8_t mode = esp8266_step_mode;
	// updates the working mode
	if (mode & ITP_STEP_MODE_SYNC)
	{
		// let the buffer flush out
		if (!out_io_empty())
		{
			running = false;
			return;
		}

		timer1_disable();
		out_io_reset();
#ifdef SHIFT_REGISTER_CUSTOM_CALLBACK
		spi_config_t conf = {0};
		mcu_spi_config(conf, 20000000);
		SPI1U1 = (((SHIFT_REGISTER_BYTES * 8) - 1) << SPILMOSI) | (((SHIFT_REGISTER_BYTES * 8) - 1) << SPILMISO);
#endif
		timer1_isr_init();
		timer1_attachInterrupt(mcu_itp_isr);

		switch (mode & ~ITP_STEP_MODE_SYNC)
		{
		case ITP_STEP_MODE_DEFAULT:
			timer1_write((APB_CLK_FREQ / TIMER_IO_SAMPLE_RATE));
			timer1_enable(TIM_DIV1, TIM_EDGE, TIM_LOOP);
			break;
		case ITP_STEP_MODE_REALTIME:
			timer1_write((APB_CLK_FREQ / (TIMER_IO_SAMPLE_RATE >> 2)));
			timer1_enable(TIM_DIV1, TIM_EDGE, TIM_LOOP);
			break;
		}

		// clear sync flag
		// __ATOMIC__
		{
			esp8266_step_mode &= ~ITP_STEP_MODE_SYNC;
		}
	}

	// fill the buffer in buffered mode
	while (mode == ITP_STEP_MODE_DEFAULT && !out_io_full() && --limit)
	{
		mcu_gen_step();
		mcu_gen_pwm_and_servo();
		mcu_gen_oneshot();
		esp8266_io_out_t outputs;
		outputs.io = esp8266_io_out;
#if defined(IC74HC595_HAS_STEPS) || defined(IC74HC595_HAS_DIRS) || defined(IC74HC595_HAS_PWMS) || defined(IC74HC595_HAS_SERVOS)
		outputs.ic74hc595 = ic74hc595_io_out;
#endif
		out_io_push(outputs);
	}

	// static uint32_t next_print;
	// if (next_print < mcu_millis())
	// {
	// 	next_print = mcu_millis() + 3000;
	// 	DBGMSG("mode: %hd, full?: %hd, buffer: %d\n", mode, (uint8_t)out_io_full(), out_io_head);
	// }

	running = false;
}

IRAM_ATTR void mcu_rtc_isr(void)
{
	mcu_isr_context_enter();
	mcu_runtime_ms++;
	mcu_rtc_cb(mcu_runtime_ms);
	itp_buffer_dotasks(OUT_IO_BUFFER_MINIMAL); // process at most 2ms of motion
	uint32_t stamp = esp_get_cycle_count() + (ESP8266_CLOCK / 1000);
	timer0_write(stamp);
}

// modifies the step generation mode
uint8_t itp_set_step_mode(uint8_t mode)
{
	uint8_t last_mode = esp8266_step_mode;
	itp_sync();
#ifdef USE_I2S_REALTIME_MODE_ONLY
	esp8266_step_mode = (ITP_STEP_MODE_SYNC | ITP_STEP_MODE_REALTIME);
#else
	esp8266_step_mode = (ITP_STEP_MODE_SYNC | mode);
#endif
	cnc_delay_ms(20);
	return last_mode;
}

/**
 * initializes the mcu
 * this function needs to:
 *   - configure all IO pins (digital IO, PWM, Analog, etc...)
 *   - configure all interrupts
 *   - configure uart or usb
 *   - start the internal RTC
 * */

extern void mcu_uart_dotasks(void);
extern void mcu_uart_init(void);
#ifndef RAM_ONLY_SETTINGS
extern void mcu_eeprom_init(void);
#endif
extern void mcu_spi_init();

void mcu_init(void)
{
	mcu_io_init();
	mcu_uart_init();

#ifndef RAM_ONLY_SETTINGS
	mcu_eeprom_init(); // Emulated EEPROM
#endif

#ifdef MCU_HAS_WIFI
	esp8266_wifi_init();
#endif

	esp8266_step_mode = (ITP_STEP_MODE_DEFAULT | ITP_STEP_MODE_SYNC);

#ifdef MCU_HAS_SPI
	mcu_spi_init();
#endif

#ifdef MCU_HAS_I2C
	i2c_master_gpio_init();
	i2c_master_init();
#endif

	// kick start the RTC
	// the RTC will start the ITP timer
	uint32_t stamp = esp_get_cycle_count() + (ESP8266_CLOCK / 1000);
	timer0_isr_init();
	timer0_attachInterrupt(mcu_rtc_isr);
	timer0_write(stamp);
}

/**
 * runs all internal tasks of the MCU.
 * for the moment these are:
 *   - if USB is enabled and MCU uses tinyUSB framework run tinyUSB tud_task
 * */
void mcu_dotasks(void)
{
	// reset WDT
	system_soft_wdt_feed();
	mcu_uart_dotasks();
#ifdef MCU_HAS_WIFI
	esp8266_wifi_dotasks();
#endif
	itp_buffer_dotasks(OUT_IO_BUFFER_MINIMAL);
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
uint32_t mcu_millis()
{
	return mcu_runtime_ms;
}

uint32_t mcu_micros()
{
	return (uint32_t)system_get_time();
}

void esp8266_delay_us(uint16_t delay)
{
	uint32_t time = system_get_time() + delay - 1;
	while (time > system_get_time())
		;
}

uint32_t mcu_free_micros()
{
	return (uint32_t)(system_get_time() % 1000);
}

// #ifdef MCU_HAS_I2C
// #ifndef mcu_i2c_write
// uint8_t mcu_i2c_write(uint8_t data, bool send_start, bool send_stop)
// {
// 	if (send_start)
// 	{
// 		// init
// 		i2c_master_start();
// 		if (!i2c_master_checkAck())
// 		{
// 			i2c_master_stop();
// 			return 0;
// 		}
// 	}

// 	i2c_master_writeByte(data);

// 	if (!i2c_master_checkAck())
// 	{
// 		i2c_master_stop();
// 		return 0;
// 	}

// 	if (send_stop)
// 	{
// 		i2c_master_stop();
// 	}

// 	return 1;
// }
// #endif

// #ifndef mcu_i2c_read
// uint8_t mcu_i2c_read(bool with_ack, bool send_stop)
// {
// 	uint8_t c = 0;

// 	if (with_ack)
// 	{
// 		i2c_master_send_ack();
// 	}
// 	else
// 	{
// 		i2c_master_send_nack();
// 	}
// 	c = i2c_master_readByte();

// 	if (send_stop)
// 	{
// 		i2c_master_stop();
// 	}

// 	return c;
// }
// #endif
// #endif

#ifdef MCU_HAS_ONESHOT_TIMER
/**
 * configures a single shot timeout in us
 * */

#ifndef mcu_config_timeout
void mcu_config_timeout(mcu_timeout_delgate fp, uint32_t timeout)
{
	mcu_timeout_cb = fp;
	esp8266_oneshot_reload = (TIMER_IO_SAMPLE_RATE / timeout);
}
#endif

/**
 * starts the timeout. Once hit the the respective callback is called
 * */
#ifndef mcu_start_timeout
void mcu_start_timeout()
{
	esp8266_oneshot_counter = esp8266_oneshot_reload;
}
#endif
#endif

#endif
