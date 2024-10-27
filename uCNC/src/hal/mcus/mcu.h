/*
	Name: mcu.h
	Description: Contains all the function declarations necessary to interact with the MCU.
		This provides an intenterface between the µCNC and the MCU unit used to power the µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 01/11/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef MCU_H
#define MCU_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef MCU_CALLBACK
#define MCU_CALLBACK
#endif

#ifndef MCU_RX_CALLBACK
#define MCU_RX_CALLBACK MCU_CALLBACK
#endif

#ifndef MCU_IO_CALLBACK
#define MCU_IO_CALLBACK MCU_CALLBACK
#endif

#ifndef F_STEP_MAX
#define F_STEP_MAX 30000
#endif

#define STREAM_UART 1
#define STREAM_UART2 2
#define STREAM_USB 4
#define STREAM_WIFI 8
#define STREAM_BTH 16
#define STREAM_BOARDCAST 255

// defines special mcu to access flash strings and arrays
#ifndef __rom__
#define __rom__
#endif
#ifndef __romstr__
#define __romstr__
#endif
#ifndef __romarr__
#define __romarr__ const uint8_t
#endif
#ifndef rom_strptr
#define rom_strptr *
#endif
#ifndef rom_strcpy
#define rom_strcpy strcpy
#endif
#ifndef rom_strncpy
#define rom_strncpy strncpy
#endif
#ifndef rom_memcpy
#define rom_memcpy memcpy
#endif
#ifndef rom_read_byte
#define rom_read_byte *
#endif
#ifndef rom_strcmp
#define rom_strcmp strcmp
#endif

	// the extern is not necessary
	// this explicit declaration just serves to reeinforce the idea that these callbacks are implemented on other µCNC core code translation units
	// these callbacks provide a transparent way for the mcu to call them when the ISR/IRQ is triggered

	MCU_CALLBACK void mcu_step_cb(void);
	MCU_CALLBACK void mcu_step_reset_cb(void);
	MCU_RX_CALLBACK bool mcu_com_rx_cb(uint8_t c);
	MCU_CALLBACK void mcu_rtc_cb(uint32_t millis);
	MCU_IO_CALLBACK void mcu_controls_changed_cb(void);
	MCU_IO_CALLBACK void mcu_limits_changed_cb(void);
	MCU_IO_CALLBACK void mcu_probe_changed_cb(void);
	MCU_IO_CALLBACK void mcu_inputs_changed_cb(void);

/*IO functions*/

/**
 * config a pin in input mode
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_config_input
	void mcu_config_input(uint8_t pin);
#endif

/**
 * config pullup
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_config_pullup
	void mcu_config_pullup(uint8_t pin);
#endif

/**
 * config a pin in output mode
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_config_output
	void mcu_config_output(uint8_t pin);
#endif

/**
 * get the value of a digital input pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_input
	uint8_t mcu_get_input(uint8_t pin);
#endif

/**
 * gets the value of a digital output pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_output
	uint8_t mcu_get_output(uint8_t pin);
#endif

/**
 * sets the value of a digital output pin to logical 1
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_set_output
	void mcu_set_output(uint8_t pin);
#endif

/**
 * sets the value of a digital output pin to logical 0
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_clear_output
	void mcu_clear_output(uint8_t pin);
#endif

/**
 * toggles the value of a digital output pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_toggle_output
	void mcu_toggle_output(uint8_t pin);
#endif

	/**
	 *
	 * This is used has by the generic mcu functions has generic (overridable) IO initializer
	 *
	 * */
	void mcu_io_init(void);

	/**
	 * This can be used to set the defaults state of IO pins on reset. (overridable)
	 * */
#ifndef mcu_io_reset
	void mcu_io_reset(void);
#endif

	/**
	 * initializes the mcu
	 * this function needs to:
	 *   - configure all IO pins (digital IO, PWM, Analog, etc...)
	 *   - configure all interrupts
	 *   - configure uart or usb
	 *   - start the internal RTC
	 * */
	void mcu_init(void);

/**
 * enables the pin probe mcu isr on change
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_enable_probe_isr
	void mcu_enable_probe_isr(void);
#endif

/**
 * disables the pin probe mcu isr on change
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_disable_probe_isr
	void mcu_disable_probe_isr(void);
#endif

/**
 * gets the voltage value of a built-in ADC pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_analog
	uint16_t mcu_get_analog(uint8_t channel);
#endif

/**
 * configs the pwm pin and output frequency
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_config_pwm
	void mcu_config_pwm(uint8_t pin, uint16_t freq);
#endif

/**
 * sets the pwm value of a built-in pwm pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_set_pwm
	void mcu_set_pwm(uint8_t pwm, uint8_t value);
#endif

/**
 * gets the configured pwm value of a built-in pwm pin
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_pwm
	uint8_t mcu_get_pwm(uint8_t pwm);
#endif

/**
 * sets the pwm for a servo (50Hz with tON between 1~2ms)
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_set_servo
	void mcu_set_servo(uint8_t servo, uint8_t value);
#endif

/**
 * gets the pwm for a servo (50Hz with tON between 1~2ms)
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_servo
	uint8_t mcu_get_servo(uint8_t servo);
#endif

// ISR
/**
 * enables global interrupts on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_enable_global_isr
	void mcu_enable_global_isr(void);
#endif

/**
 * disables global interrupts on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_disable_global_isr
	void mcu_disable_global_isr(void);
#endif

/**
 * gets global interrupts state on the MCU
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_get_global_isr
	bool mcu_get_global_isr(void);
#endif

	// Step interpolator
	/**
	 * convert step rate/frequency to timer ticks and prescaller
	 * */
	void mcu_freq_to_clocks(float frequency, uint16_t *ticks, uint16_t *prescaller);

	/**
	 * convert timer ticks and prescaller to step rate/frequency
	 * */
	float mcu_clocks_to_freq(uint16_t ticks, uint16_t prescaller);

	/**
	 * starts the timer interrupt that generates the step pulses for the interpolator
	 * */
	void mcu_start_itp_isr(uint16_t ticks, uint16_t prescaller);

	/**
	 * changes the step rate of the timer interrupt that generates the step pulses for the interpolator
	 * */
	void mcu_change_itp_isr(uint16_t ticks, uint16_t prescaller);

	/**
	 * stops the timer interrupt that generates the step pulses for the interpolator
	 * */
	void mcu_stop_itp_isr(void);

/**
 * gets the MCU running time in milliseconds.
 * the time counting is controled by the internal RTC
 * */
#ifndef mcu_millis
	uint32_t mcu_millis(void);
#endif

/**
 * gets the MCU running time in microseconds.
 * the time counting is controled by the internal RTC
 * */
#ifndef mcu_micros
	uint32_t mcu_micros(void);
#endif

/**
 * gets the microsecond portion of the free RTC clock counter (from 0 to 1000).
 * this free runner is always running even during an ISR or atomic operation
 * */
#ifndef mcu_free_micros
	uint32_t mcu_free_micros(void);
#endif

#ifndef mcu_nop
#define mcu_nop() asm volatile("nop\n\t")
#endif

	void mcu_delay_loop(uint16_t loops);

#ifndef mcu_delay_cycles
// set per MCU
#ifndef MCU_CLOCKS_PER_CYCLE
#error "MCU_CLOCKS_PER_CYCLE not defined for this MCU"
#endif
#ifndef MCU_CYCLES_PER_LOOP_OVERHEAD
#error "MCU_CYCLES_PER_LOOP_OVERHEAD not defined for this MCU"
#endif
#ifndef MCU_CYCLES_PER_LOOP
#error "MCU_CYCLES_PER_LOOP not defined for this MCU"
#endif
#ifndef MCU_CYCLES_PER_LOOP_OVERHEAD
#error "MCU_CYCLES_PER_LOOP_OVERHEAD not defined for this MCU"
#endif

#define mcu_delay_cycles(X)                                                                                                               \
	{                                                                                                                                       \
		if (X > (MCU_CYCLES_PER_LOOP + MCU_CYCLES_PER_LOOP_OVERHEAD))                                                                         \
		{                                                                                                                                     \
			mcu_delay_loop((uint16_t)((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP));                                               \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 0)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 1)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 2)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 3)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 4)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 5)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 6)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 7)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 8)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 9)  \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) - (((X - MCU_CYCLES_PER_LOOP_OVERHEAD) / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 10) \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
		}                                                                                                                                     \
		else                                                                                                                                  \
		{                                                                                                                                     \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 0)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 1)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 2)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 3)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 4)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 5)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 6)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 7)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 8)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 9)                                                                    \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
			if ((X - ((X / MCU_CYCLES_PER_LOOP) * MCU_CYCLES_PER_LOOP)) > 10)                                                                   \
			{                                                                                                                                   \
				mcu_nop();                                                                                                                        \
			}                                                                                                                                   \
		}                                                                                                                                     \
	}
#endif

#ifndef mcu_delay_100ns
#define mcu_delay_100ns() mcu_delay_cycles((F_CPU / MCU_CLOCKS_PER_CYCLE / 10000000UL))
#endif

/**
 * provides a delay in us (micro seconds)
 * the maximum allowed delay is 255 us
 * */
#ifndef mcu_delay_us
#define mcu_delay_us(X) mcu_delay_cycles(F_CPU / MCU_CLOCKS_PER_CYCLE / 1000000UL * X)
#endif

#ifdef MCU_HAS_ONESHOT_TIMER
	typedef void (*mcu_timeout_delgate)(void);
	extern MCU_CALLBACK mcu_timeout_delgate mcu_timeout_cb;
/**
 * configures a single shot timeout in us
 * */
#ifndef mcu_config_timeout
	void mcu_config_timeout(mcu_timeout_delgate fp, uint32_t timeout);
#endif

/**
 * starts the timeout. Once hit the the respective callback is called
 * */
#ifndef mcu_start_timeout
	void mcu_start_timeout();
#endif
#endif

	/**
	 * runs all internal tasks of the MCU.
	 * for the moment these are:
	 *   - if USB is enabled and MCU uses tinyUSB framework run tinyUSB tud_task
	 * */
	void mcu_dotasks(void);

	// Non volatile memory
	/**
	 * gets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
	 * */
	uint8_t mcu_eeprom_getc(uint16_t address);

	/**
	 * sets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
	 * */
	void mcu_eeprom_putc(uint16_t address, uint8_t value);

	/**
	 * flushes all recorded registers into the eeprom.
	 * */
	void mcu_eeprom_flush(void);

	typedef union
	{
		uint8_t flags;
		struct
		{
			uint8_t mode : 3;
			uint8_t : 1; // reserved for bit order
			uint8_t enable_dma : 1;
			uint8_t : 3; // reserved
		};
	} spi_config_t;

	// hardware port function calls
	typedef struct spi_port_
	{
		bool isbusy;
		void (*start)(spi_config_t, uint32_t);
		uint8_t (*xmit)(uint8_t);
		bool (*bulk_xmit)(const uint8_t *, uint8_t *, uint16_t);
		void (*stop)(void);
	} spi_port_t;

#ifdef MCU_HAS_SPI
#ifndef mcu_spi_xmit
	uint8_t mcu_spi_xmit(uint8_t data);
#endif

#ifndef mcu_spi_bulk_transfer
	bool mcu_spi_bulk_transfer(const uint8_t *out, uint8_t *in, uint16_t len);
#endif

#ifndef mcu_spi_start
	void mcu_spi_start(spi_config_t config, uint32_t frequency);
#endif

#ifndef mcu_spi_stop
	void mcu_spi_stop(void);
#endif

#ifndef mcu_spi_config
	void mcu_spi_config(spi_config_t config, uint32_t frequency);
#endif

extern spi_port_t mcu_spi_port;
#define MCU_SPI (&mcu_spi_port)
#else
#define MCU_SPI NULL
#endif

#ifdef MCU_HAS_SPI2
#ifndef mcu_spi2_xmit
	uint8_t mcu_spi2_xmit(uint8_t data);
#endif

#ifndef mcu_spi2_bulk_transfer
	bool mcu_spi2_bulk_transfer(const uint8_t *out, uint8_t *in, uint16_t len);
#endif

#ifndef mcu_spi2_start
	void mcu_spi2_start(spi_config_t config, uint32_t frequency);
#endif

#ifndef mcu_spi2_stop
	void mcu_spi2_stop(void);
#endif

#ifndef mcu_spi2_config
	void mcu_spi2_config(spi_config_t config, uint32_t frequency);
#endif

extern spi_port_t mcu_spi2_port;
#define MCU_SPI2 (&mcu_spi2_port)
#else
#define MCU_SPI2 NULL
#endif

#ifdef MCU_HAS_I2C
#ifndef I2C_OK
#define I2C_OK 0
#endif
#ifndef I2C_NOTOK
#define I2C_NOTOK 1
#endif

#ifndef mcu_i2c_send
	// master sends command to slave
	uint8_t mcu_i2c_send(uint8_t address, uint8_t *data, uint8_t datalen, bool release, uint32_t ms_timeout);
#endif
#ifndef mcu_i2c_receive
	// master receive response from slave
	uint8_t mcu_i2c_receive(uint8_t address, uint8_t *data, uint8_t datalen, uint32_t ms_timeout);
#endif

#if defined(MCU_SUPPORTS_I2C_SLAVE) && (I2C_ADDRESS != 0)
#ifndef I2C_SLAVE_BUFFER_SIZE
#define I2C_SLAVE_BUFFER_SIZE 48
#endif
#ifndef mcu_i2c_slave_cb
	MCU_IO_CALLBACK void mcu_i2c_slave_cb(uint8_t *data, uint8_t *datalen);
#endif
#endif

#ifndef mcu_i2c_config
	void mcu_i2c_config(uint32_t frequency);
#endif

#endif

	/**
	 * sends a uint8_t either via uart (hardware, software USB CDC, Wifi or BT)
	 * can be defined either as a function or a macro call
	 * */

#ifdef MCU_HAS_USB
	uint8_t mcu_usb_getc(void);
	uint8_t mcu_usb_available(void);
	void mcu_usb_clear(void);
	void mcu_usb_putc(uint8_t c);
	void mcu_usb_flush(void);
#ifdef DETACH_USB_FROM_MAIN_PROTOCOL
	MCU_RX_CALLBACK void mcu_usb_rx_cb(uint8_t c);
#endif
#endif

#ifdef MCU_HAS_UART
	uint8_t mcu_uart_getc(void);
	uint8_t mcu_uart_available(void);
	void mcu_uart_clear(void);
	void mcu_uart_putc(uint8_t c);
	void mcu_uart_flush(void);
#ifdef DETACH_UART_FROM_MAIN_PROTOCOL
	MCU_RX_CALLBACK void mcu_uart_rx_cb(uint8_t c);
#endif
#endif

#ifdef MCU_HAS_UART2
	uint8_t mcu_uart2_getc(void);
	uint8_t mcu_uart2_available(void);
	void mcu_uart2_clear(void);
	void mcu_uart2_putc(uint8_t c);
	void mcu_uart2_flush(void);
#ifdef DETACH_UART2_FROM_MAIN_PROTOCOL
	MCU_RX_CALLBACK void mcu_uart2_rx_cb(uint8_t c);
#endif
#endif

#ifdef MCU_HAS_WIFI
	uint8_t mcu_wifi_getc(void);
	uint8_t mcu_wifi_available(void);
	void mcu_wifi_clear(void);
	void mcu_wifi_putc(uint8_t c);
	void mcu_wifi_flush(void);
#ifdef DETACH_WIFI_FROM_MAIN_PROTOCOL
	MCU_RX_CALLBACK void mcu_wifi_rx_cb(uint8_t c);
#endif
#endif

#ifdef MCU_HAS_BLUETOOTH
	uint8_t mcu_bt_getc(void);
	uint8_t mcu_bt_available(void);
	void mcu_bt_clear(void);
	void mcu_bt_putc(uint8_t c);
	void mcu_bt_flush(void);
#ifdef DETACH_BLUETOOTH_FROM_MAIN_PROTOCOL
	MCU_RX_CALLBACK void mcu_bt_rx_cb(uint8_t c);
#endif
#endif

#ifndef mcu_getc
#define mcu_getc (&mcu_uart_getc)
#endif
#ifndef mcu_available
#define mcu_available (&mcu_uart_available)
#endif
#ifndef mcu_clear
#define mcu_clear (&mcu_uart_clear)
#endif
#ifndef mcu_putc
#define mcu_putc (&mcu_uart_putc)
#endif
#ifndef mcu_flush
#define mcu_flush (&mcu_uart_flush)
#endif

#ifdef __cplusplus
}
#endif

#endif
