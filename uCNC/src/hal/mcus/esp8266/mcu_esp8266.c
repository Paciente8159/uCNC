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
#include "uart_register.h"
#include "osapi.h"
#ifdef MCU_HAS_I2C
#include "twi.h"
#endif

#define UART_PARITY_EN (BIT(1))
#define UART_PARITY_EN_M 0x00000001
#define UART_PARITY_EN_S 1
#define UART_PARITY (BIT(0))
#define UART_PARITY_M 0x00000001
#define UART_PARITY_S 0

static volatile bool esp8266_global_isr_enabled;
static volatile uint32_t mcu_runtime_ms;

void esp8266_uart_init(int baud);
char esp8266_uart_read(void);
void esp8266_uart_write(char c);
bool esp8266_uart_rx_ready(void);
bool esp8266_uart_tx_ready(void);
void esp8266_uart_flush(void);
void esp8266_uart_process(void);

#ifndef RAM_ONLY_SETTINGS
void esp8266_eeprom_init(int size);
uint8_t esp8266_eeprom_read(uint16_t address);
void esp8266_eeprom_write(uint16_t address, uint8_t value);
void esp8266_eeprom_flush(void);
#endif

ETSTimer esp8266_rtc_timer;

uint8_t esp8266_pwm[16];
static IRAM_ATTR void mcu_gen_pwm(void)
{
	static uint8_t pwm_counter = 0;
	// software PWM
	if (++pwm_counter < 127)
	{
#if !(PWM0 < 0)
		if (pwm_counter > esp8266_pwm[0])
		{
			mcu_clear_output(PWM0);
		}
#endif
#if !(PWM1 < 0)
		if (pwm_counter > esp8266_pwm[1])
		{
			mcu_clear_output(PWM1);
		}
#endif
#if !(PWM2 < 0)
		if (pwm_counter > esp8266_pwm[2])
		{
			mcu_clear_output(PWM2);
		}
#endif
#if !(PWM3 < 0)
		if (pwm_counter > esp8266_pwm[3])
		{
			mcu_clear_output(PWM3);
		}
#endif
#if !(PWM4 < 0)
		if (pwm_counter > esp8266_pwm[4])
		{
			mcu_clear_output(PWM4);
		}
#endif
#if !(PWM5 < 0)
		if (pwm_counter > esp8266_pwm[5])
		{
			mcu_clear_output(PWM5);
		}
#endif
#if !(PWM6 < 0)
		if (pwm_counter > esp8266_pwm[6])
		{
			mcu_clear_output(PWM6);
		}
#endif
#if !(PWM7 < 0)
		if (pwm_counter > esp8266_pwm[7])
		{
			mcu_clear_output(PWM7);
		}
#endif
#if !(PWM8 < 0)
		if (pwm_counter > esp8266_pwm[8])
		{
			mcu_clear_output(PWM8);
		}
#endif
#if !(PWM9 < 0)
		if (pwm_counter > esp8266_pwm[9])
		{
			mcu_clear_output(PWM9);
		}
#endif
#if !(PWM10 < 0)
		if (pwm_counter > esp8266_pwm[10])
		{
			mcu_clear_output(PWM10);
		}
#endif
#if !(PWM11 < 0)
		if (pwm_counter > esp8266_pwm[11])
		{
			mcu_clear_output(PWM11);
		}
#endif
#if !(PWM12 < 0)
		if (pwm_counter > esp8266_pwm[12])
		{
			mcu_clear_output(PWM12);
		}
#endif
#if !(PWM13 < 0)
		if (pwm_counter > esp8266_pwm[13])
		{
			mcu_clear_output(PWM13);
		}
#endif
#if !(PWM14 < 0)
		if (pwm_counter > esp8266_pwm[14])
		{
			mcu_clear_output(PWM14);
		}
#endif
#if !(PWM15 < 0)
		if (pwm_counter > esp8266_pwm[15])
		{
			mcu_clear_output(PWM15);
		}
#endif
	}
	else
	{
		pwm_counter = 0;
#if !(PWM0 < 0)
		if (esp8266_pwm[0])
		{
			mcu_set_output(PWM0);
		}
#endif
#if !(PWM1 < 0)
		if (esp8266_pwm[1])
		{
			mcu_set_output(PWM1);
		}
#endif
#if !(PWM2 < 0)
		if (esp8266_pwm[2])
		{
			mcu_set_output(PWM2);
		}
#endif
#if !(PWM3 < 0)
		if (esp8266_pwm[3])
		{
			mcu_set_output(PWM3);
		}
#endif
#if !(PWM4 < 0)
		if (esp8266_pwm[4])
		{
			mcu_set_output(PWM4);
		}
#endif
#if !(PWM5 < 0)
		if (esp8266_pwm[5])
		{
			mcu_set_output(PWM5);
		}
#endif
#if !(PWM6 < 0)
		if (esp8266_pwm[6])
		{
			mcu_set_output(PWM6);
		}
#endif
#if !(PWM7 < 0)
		if (esp8266_pwm[7])
		{
			mcu_set_output(PWM7);
		}
#endif
#if !(PWM8 < 0)
		if (esp8266_pwm[8])
		{
			mcu_set_output(PWM8);
		}
#endif
#if !(PWM9 < 0)
		if (esp8266_pwm[9])
		{
			mcu_set_output(PWM9);
		}
#endif
#if !(PWM10 < 0)
		if (esp8266_pwm[10])
		{
			mcu_set_output(PWM10);
		}
#endif
#if !(PWM11 < 0)
		if (esp8266_pwm[11])
		{
			mcu_set_output(PWM11);
		}
#endif
#if !(PWM12 < 0)
		if (esp8266_pwm[12])
		{
			mcu_set_output(PWM12);
		}
#endif
#if !(PWM13 < 0)
		if (esp8266_pwm[13])
		{
			mcu_set_output(PWM13);
		}
#endif
#if !(PWM14 < 0)
		if (esp8266_pwm[14])
		{
			mcu_set_output(PWM14);
		}
#endif
#if !(PWM15 < 0)
		if (esp8266_pwm[15])
		{
			mcu_set_output(PWM15);
		}
#endif
	}
}

static uint32_t mcu_step_counter;
static uint32_t mcu_step_reload;
static IRAM_ATTR void mcu_gen_step(void)
{
	if (mcu_step_reload)
	{
		if (!--mcu_step_counter)
		{
			static bool resetstep = false;
			if (!resetstep)
				mcu_step_cb();
			else
				mcu_step_reset_cb();
			resetstep = !resetstep;
			mcu_step_counter = mcu_step_reload;
		}
	}
}

IRAM_ATTR void mcu_din_isr(void)
{
	io_inputs_isr();
}

IRAM_ATTR void mcu_probe_isr(void)
{
	io_probe_isr();
}

IRAM_ATTR void mcu_limits_isr(void)
{
	io_limits_isr();
}

IRAM_ATTR void mcu_controls_isr(void)
{
	io_controls_isr();
}

IRAM_ATTR void mcu_rtc_isr(void *arg)
{
	mcu_runtime_ms++;
	mcu_rtc_cb(mcu_runtime_ms);
}

IRAM_ATTR void mcu_itp_isr(void)
{
	// mcu_disable_global_isr();
	// static bool resetstep = false;
	// if (!resetstep)
	// 	mcu_step_cb();
	// else
	// 	mcu_step_reset_cb();
	// resetstep = !resetstep;
	// mcu_enable_global_isr();
	mcu_gen_step();
	mcu_gen_pwm();
}

// static void mcu_uart_isr(void *arg)
// {
// 	/*ATTENTION:*/
// 	/*IN NON-OS VERSION SDK, DO NOT USE "ICACHE_FLASH_ATTR" FUNCTIONS IN THE WHOLE HANDLER PROCESS*/
// 	/*ALL THE FUNCTIONS CALLED IN INTERRUPT HANDLER MUST BE DECLARED IN RAM */
// 	/*IF NOT , POST AN EVENT AND PROCESS IN SYSTEM TASK */
// 	if ((READ_PERI_REG(UART_INT_ST(0)) & UART_FRM_ERR_INT_ST))
// 	{
// 		WRITE_PERI_REG(UART_INT_CLR(0), UART_FRM_ERR_INT_CLR);
// 	}
// 	else if ((READ_PERI_REG(UART_INT_ST(0)) & (UART_RXFIFO_FULL_INT_ST | UART_RXFIFO_TOUT_INT_ST)))
// 	{
// 		// disable ISR
// 		CLEAR_PERI_REG_MASK(UART_INT_ENA(0), UART_RXFIFO_FULL_INT_ENA | UART_RXFIFO_TOUT_INT_ENA);
// 		WRITE_PERI_REG(UART_INT_CLR(0), (READ_PERI_REG(UART_INT_ST(0)) & (UART_RXFIFO_FULL_INT_ST | UART_RXFIFO_TOUT_INT_ST)));
// 		uint8_t fifo_len = (READ_PERI_REG(UART_STATUS(0)) >> UART_RXFIFO_CNT_S) & UART_RXFIFO_CNT;
// 		unsigned char c = 0;

// 		for (uint8_t i = 0; i < fifo_len; i++)
// 		{
// 			c = READ_PERI_REG(UART_FIFO(0)) & 0xFF;
// 			mcu_com_rx_cb(c);
// 		}

// 		WRITE_PERI_REG(UART_INT_CLR(0), UART_RXFIFO_FULL_INT_CLR | UART_RXFIFO_TOUT_INT_CLR);
// 		// reenable ISR
// 		SET_PERI_REG_MASK(UART_INT_ENA(0), UART_RXFIFO_FULL_INT_ENA | UART_RXFIFO_TOUT_INT_ENA);
// 	}
// 	else if (UART_TXFIFO_EMPTY_INT_ST == (READ_PERI_REG(UART_INT_ST(0)) & UART_TXFIFO_EMPTY_INT_ST))
// 	{
// 		/* to output uart data from uart buffer directly in empty interrupt handler*/
// 		/*instead of processing in system event, in order not to wait for current task/function to quit */
// 		/*ATTENTION:*/
// 		/*IN NON-OS VERSION SDK, DO NOT USE "ICACHE_FLASH_ATTR" FUNCTIONS IN THE WHOLE HANDLER PROCESS*/
// 		/*ALL THE FUNCTIONS CALLED IN INTERRUPT HANDLER MUST BE DECLARED IN RAM */
// 		CLEAR_PERI_REG_MASK(UART_INT_ENA(0), UART_TXFIFO_EMPTY_INT_ENA);
// 		mcu_com_tx_cb();
// 		// system_os_post(uart_recvTaskPrio, 1, 0);
// 		WRITE_PERI_REG(UART_INT_CLR(0), UART_TXFIFO_EMPTY_INT_CLR);
// 	}
// 	else if (UART_RXFIFO_OVF_INT_ST == (READ_PERI_REG(UART_INT_ST(0)) & UART_RXFIFO_OVF_INT_ST))
// 	{
// 		WRITE_PERI_REG(UART_INT_CLR(0), UART_RXFIFO_OVF_INT_CLR);
// 	}
// }

static void mcu_usart_init(void)
{
	esp8266_uart_init(BAUDRATE);
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

	// init rtc
	os_timer_setfn(&esp8266_rtc_timer, (os_timer_func_t *)&mcu_rtc_isr, NULL);
	os_timer_arm(&esp8266_rtc_timer, 1, true);

	// init timer1
	timer1_isr_init();
	timer1_attachInterrupt(mcu_itp_isr);
	timer1_enable(TIM_DIV1, TIM_EDGE, TIM_LOOP);
	timer1_write(625);

#ifndef RAM_ONLY_SETTINGS
	esp8266_eeprom_init(1024); // 1K Emulated EEPROM
#endif
#ifdef MCU_HAS_SPI
	esp8266_spi_init(SPI_FREQ, SPI_MODE);
#endif

#ifdef MCU_HAS_I2C
	i2c_master_gpio_init();
	i2c_master_init();
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

/**
 * checks if the serial hardware of the MCU is ready do send the next char
 * */
#ifndef mcu_tx_ready
bool mcu_tx_ready(void)
{
	return esp8266_uart_tx_ready();
}
#endif

/**
 * checks if the serial hardware of the MCU has a new char ready to be read
 * */
#ifndef mcu_rx_ready
bool mcu_rx_ready(void)
{
	return esp8266_uart_rx_ready();
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

	esp8266_uart_write(c);
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

	return esp8266_uart_read();
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
	esp8266_global_isr_enabled = true;
}
#endif

/**
 * disables global interrupts on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_disable_global_isr
void mcu_disable_global_isr(void)
{
	esp8266_global_isr_enabled = false;
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
	return esp8266_global_isr_enabled;
}
#endif

// Step interpolator
/**
 * convert step rate to clock cycles
 * */
void mcu_freq_to_clocks(float frequency, uint16_t *ticks, uint16_t *prescaller)
{
	// up and down counter (generates half the step rate at each event)
	uint32_t totalticks = (uint32_t)((float)(128000UL >> 1) / frequency);
	*prescaller = 0;
	while (totalticks > 0xFFFF)
	{
		(*prescaller) += 1;
		totalticks >>= 1;
	}

	*ticks = (uint16_t)totalticks;
}

/**
 * starts the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_start_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	mcu_step_reload = (((uint32_t)ticks) << prescaller);
	mcu_step_counter = mcu_step_reload;
}

/**
 * changes the step rate of the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_change_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	mcu_step_reload = (((uint32_t)ticks) << prescaller);
	mcu_step_counter = mcu_step_reload;
}

/**
 * stops the timer interrupt that generates the step pulses for the interpolator
 * */
void mcu_stop_itp_isr(void)
{
	mcu_step_reload = 0;
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
	uint32_t time = system_get_time() + delay;
	while (time > system_get_time())
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
	system_soft_wdt_feed();
	esp8266_uart_process();
}

// Non volatile memory
/**
 * gets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
 * */
uint8_t mcu_eeprom_getc(uint16_t address)
{
#ifndef RAM_ONLY_SETTINGS
	return esp8266_eeprom_read(address);
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
	esp8266_eeprom_write(address, value);
#endif
}

/**
 * flushes all recorded registers into the eeprom.
 * */
void mcu_eeprom_flush(void)
{
#ifndef RAM_ONLY_SETTINGS
	esp8266_eeprom_flush();
#endif
}

#ifdef MCU_HAS_I2C
#ifndef mcu_i2c_write
uint8_t mcu_i2c_write(uint8_t data, bool send_start, bool send_stop)
{
	if (send_start)
	{
		// init
		i2c_master_start();
		if (!i2c_master_checkAck())
		{
			i2c_master_stop();
			return 0;
		}
	}

	i2c_master_writeByte(data);

	if (!i2c_master_checkAck())
	{
		i2c_master_stop();
		return 0;
	}

	if (send_stop)
	{
		i2c_master_stop();
	}

	return 1;
}
#endif

#ifndef mcu_i2c_read
uint8_t mcu_i2c_read(bool with_ack, bool send_stop)
{
	uint8_t c = 0;

	if (with_nack)
	{
		i2c_master_send_ack();
	}
	else
	{
		i2c_master_send_nack();
	}
	c = i2c_master_readByte();

	if (send_stop)
	{
		i2c_master_stop();
	}

	return c;
}
#endif
#endif

#endif
