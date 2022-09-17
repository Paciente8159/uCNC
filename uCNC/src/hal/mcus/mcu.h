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

#ifndef MCU_TX_CALLBACK
#define MCU_TX_CALLBACK MCU_CALLBACK
#endif

#ifndef MCU_RX_CALLBACK
#define MCU_RX_CALLBACK MCU_CALLBACK
#endif

#ifndef MCU_IO_CALLBACK
#define MCU_IO_CALLBACK MCU_CALLBACK
#endif

	// the extern is not necessary
	// this explicit declaration just serves to reeinforce the idea that these callbacks are implemented on other µCNC core code translation units
	// these callbacks provide a transparent way for the mcu to call them when the ISR/IRQ is triggered

	MCU_CALLBACK void mcu_step_cb(void);
	MCU_CALLBACK void mcu_step_reset_cb(void);
	MCU_RX_CALLBACK void mcu_com_rx_cb(unsigned char c);
	MCU_TX_CALLBACK void mcu_com_tx_cb();
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
	uint8_t mcu_get_analog(uint8_t channel);
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
#define SERVO0_UCNC_INTERNAL_PIN 40
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

/**
 * checks if the serial hardware of the MCU is ready do send the next char
 * */
#ifndef mcu_tx_ready
	bool mcu_tx_ready(void); // Start async send
#endif

/**
 * checks if the serial hardware of the MCU has a new char ready to be read
 * */
#ifndef mcu_rx_ready
	bool mcu_rx_ready(void); // Stop async send
#endif

/**
 * sends a char either via uart (hardware, software or USB virtual COM port)
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_putc
	void mcu_putc(char c);
#endif

/**
 * gets a char either via uart (hardware, software or USB virtual COM port)
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_getc
	char mcu_getc(void);
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
	 * convert step rate to clock cycles
	 * */
	void mcu_freq_to_clocks(float frequency, uint16_t *ticks, uint16_t *prescaller);

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
	uint32_t mcu_millis();

	/**
	 * provides a delay in us (micro seconds)
	 * the maximum allowed delay is 255 us
	 * */
#ifndef mcu_delay_us
	void mcu_delay_us(uint16_t delay);
#endif

	/**
	 * provides a delay in us (micro seconds)
	 * the maximum allowed delay is 255 us
	 * */
#ifndef mcu_delay_100ns
	void mcu_delay_100ns();
#endif

#ifndef mcu_nop
#define mcu_nop() asm volatile("nop\n\t")
#endif

#if (F_CPU<20000000UL)
#define MCU_100NS_LOOPS 1
#endif
#if (F_CPU>=20000000UL && F_CPU<30000000UL)
#define MCU_100NS_LOOPS 2
#endif
#if (F_CPU>=30000000UL && F_CPU<40000000UL)
#define MCU_100NS_LOOPS 3
#endif
#if (F_CPU>=40000000UL && F_CPU<50000000UL)
#define MCU_100NS_LOOPS 4
#endif
#if (F_CPU>=50000000UL && F_CPU<60000000UL)
#define MCU_100NS_LOOPS 5
#endif
#if (F_CPU>=60000000UL && F_CPU<70000000UL)
#define MCU_100NS_LOOPS 6
#endif
#if (F_CPU>=70000000UL && F_CPU<80000000UL)
#define MCU_100NS_LOOPS 7
#endif
#if (F_CPU>=80000000UL && F_CPU<90000000UL)
#define MCU_100NS_LOOPS 8
#endif
#if (F_CPU>=90000000UL && F_CPU<100000000UL)
#define MCU_100NS_LOOPS 9
#endif
#if (F_CPU>=100000000UL && F_CPU<110000000UL)
#define MCU_100NS_LOOPS 10
#endif
#if (F_CPU>=110000000UL && F_CPU<120000000UL)
#define MCU_100NS_LOOPS 11
#endif
#if (F_CPU>=120000000UL && F_CPU<130000000UL)
#define MCU_100NS_LOOPS 12
#endif
#if (F_CPU>=130000000UL && F_CPU<140000000UL)
#define MCU_100NS_LOOPS 13
#endif
#if (F_CPU>=140000000UL && F_CPU<150000000UL)
#define MCU_100NS_LOOPS 14
#endif
#if (F_CPU>=150000000UL && F_CPU<160000000UL)
#define MCU_100NS_LOOPS 15
#endif
#if (F_CPU>=160000000UL && F_CPU<170000000UL)
#define MCU_100NS_LOOPS 16
#endif
#if (F_CPU>=170000000UL && F_CPU<180000000UL)
#define MCU_100NS_LOOPS 17
#endif
#if (F_CPU>=180000000UL && F_CPU<190000000UL)
#define MCU_100NS_LOOPS 18
#endif
#if (F_CPU>=190000000UL && F_CPU<200000000UL)
#define MCU_100NS_LOOPS 19
#endif
#if (F_CPU>=200000000UL && F_CPU<210000000UL)
#define MCU_100NS_LOOPS 20
#endif
#if (F_CPU>=210000000UL && F_CPU<220000000UL)
#define MCU_100NS_LOOPS 21
#endif
#if (F_CPU>=220000000UL && F_CPU<230000000UL)
#define MCU_100NS_LOOPS 22
#endif
#if (F_CPU>=230000000UL && F_CPU<240000000UL)
#define MCU_100NS_LOOPS 23
#endif
#if (F_CPU>=240000000UL && F_CPU<250000000UL)
#define MCU_100NS_LOOPS 24
#endif
#if (F_CPU>=250000000UL && F_CPU<260000000UL)
#define MCU_100NS_LOOPS 25
#endif
#if (F_CPU>=260000000UL && F_CPU<270000000UL)
#define MCU_100NS_LOOPS 26
#endif
#if (F_CPU>=270000000UL && F_CPU<280000000UL)
#define MCU_100NS_LOOPS 27
#endif
#if (F_CPU>=280000000UL && F_CPU<290000000UL)
#define MCU_100NS_LOOPS 28
#endif
#if (F_CPU>=290000000UL && F_CPU<300000000UL)
#define MCU_100NS_LOOPS 29
#endif
#if (F_CPU>=300000000UL && F_CPU<310000000UL)
#define MCU_100NS_LOOPS 30
#endif
#if (F_CPU>=310000000UL && F_CPU<320000000UL)
#define MCU_100NS_LOOPS 31
#endif
#if (F_CPU>=320000000UL && F_CPU<330000000UL)
#define MCU_100NS_LOOPS 32
#endif
#if (F_CPU>=330000000UL && F_CPU<340000000UL)
#define MCU_100NS_LOOPS 33
#endif
#if (F_CPU>=340000000UL && F_CPU<350000000UL)
#define MCU_100NS_LOOPS 34
#endif
#if (F_CPU>=350000000UL && F_CPU<360000000UL)
#define MCU_100NS_LOOPS 35
#endif
#if (F_CPU>=360000000UL && F_CPU<370000000UL)
#define MCU_100NS_LOOPS 36
#endif
#if (F_CPU>=370000000UL && F_CPU<380000000UL)
#define MCU_100NS_LOOPS 37
#endif
#if (F_CPU>=380000000UL && F_CPU<390000000UL)
#define MCU_100NS_LOOPS 38
#endif
#if (F_CPU>=390000000UL && F_CPU<400000000UL)
#define MCU_100NS_LOOPS 39
#endif
#if (F_CPU>=400000000UL && F_CPU<410000000UL)
#define MCU_100NS_LOOPS 40
#endif
#if (F_CPU>=410000000UL && F_CPU<420000000UL)
#define MCU_100NS_LOOPS 41
#endif
#if (F_CPU>=420000000UL && F_CPU<430000000UL)
#define MCU_100NS_LOOPS 42
#endif
#if (F_CPU>=430000000UL && F_CPU<440000000UL)
#define MCU_100NS_LOOPS 43
#endif
#if (F_CPU>=440000000UL && F_CPU<450000000UL)
#define MCU_100NS_LOOPS 44
#endif
#if (F_CPU>=450000000UL && F_CPU<460000000UL)
#define MCU_100NS_LOOPS 45
#endif
#if (F_CPU>=460000000UL && F_CPU<470000000UL)
#define MCU_100NS_LOOPS 46
#endif
#if (F_CPU>=470000000UL && F_CPU<480000000UL)
#define MCU_100NS_LOOPS 47
#endif
#if (F_CPU>=480000000UL && F_CPU<490000000UL)
#define MCU_100NS_LOOPS 48
#endif
#if (F_CPU>=490000000UL && F_CPU<500000000UL)
#define MCU_100NS_LOOPS 49
#endif

#define _MCU_DELAY_CYCLE_X1 mcu_nop()
#define _MCU_DELAY_CYCLE_X2 _MCU_DELAY_CYCLE_X1;mcu_nop()
#define _MCU_DELAY_CYCLE_X3 _MCU_DELAY_CYCLE_X2;mcu_nop()
#define _MCU_DELAY_CYCLE_X4 _MCU_DELAY_CYCLE_X3;mcu_nop()
#define _MCU_DELAY_CYCLE_X5 _MCU_DELAY_CYCLE_X4;mcu_nop()
#define _MCU_DELAY_CYCLE_X6 _MCU_DELAY_CYCLE_X5;mcu_nop()
#define _MCU_DELAY_CYCLE_X7 _MCU_DELAY_CYCLE_X6;mcu_nop()
#define _MCU_DELAY_CYCLE_X8 _MCU_DELAY_CYCLE_X7;mcu_nop()
#define _MCU_DELAY_CYCLE_X9 _MCU_DELAY_CYCLE_X8;mcu_nop()
#define _MCU_DELAY_CYCLE_X10 _MCU_DELAY_CYCLE_X9;mcu_nop()
#define _MCU_DELAY_CYCLE_X11 _MCU_DELAY_CYCLE_X10;mcu_nop()
#define _MCU_DELAY_CYCLE_X12 _MCU_DELAY_CYCLE_X11;mcu_nop()
#define _MCU_DELAY_CYCLE_X13 _MCU_DELAY_CYCLE_X12;mcu_nop()
#define _MCU_DELAY_CYCLE_X14 _MCU_DELAY_CYCLE_X13;mcu_nop()
#define _MCU_DELAY_CYCLE_X15 _MCU_DELAY_CYCLE_X14;mcu_nop()
#define _MCU_DELAY_CYCLE_X16 _MCU_DELAY_CYCLE_X15;mcu_nop()
#define _MCU_DELAY_CYCLE_X17 _MCU_DELAY_CYCLE_X16;mcu_nop()
#define _MCU_DELAY_CYCLE_X18 _MCU_DELAY_CYCLE_X17;mcu_nop()
#define _MCU_DELAY_CYCLE_X19 _MCU_DELAY_CYCLE_X18;mcu_nop()
#define _MCU_DELAY_CYCLE_X20 _MCU_DELAY_CYCLE_X19;mcu_nop()
#define _MCU_DELAY_CYCLE_X21 _MCU_DELAY_CYCLE_X20;mcu_nop()
#define _MCU_DELAY_CYCLE_X22 _MCU_DELAY_CYCLE_X21;mcu_nop()
#define _MCU_DELAY_CYCLE_X23 _MCU_DELAY_CYCLE_X22;mcu_nop()
#define _MCU_DELAY_CYCLE_X24 _MCU_DELAY_CYCLE_X23;mcu_nop()
#define _MCU_DELAY_CYCLE_X25 _MCU_DELAY_CYCLE_X24;mcu_nop()
#define _MCU_DELAY_CYCLE_X26 _MCU_DELAY_CYCLE_X25;mcu_nop()
#define _MCU_DELAY_CYCLE_X27 _MCU_DELAY_CYCLE_X26;mcu_nop()
#define _MCU_DELAY_CYCLE_X28 _MCU_DELAY_CYCLE_X27;mcu_nop()
#define _MCU_DELAY_CYCLE_X29 _MCU_DELAY_CYCLE_X28;mcu_nop()
#define _MCU_DELAY_CYCLE_X30 _MCU_DELAY_CYCLE_X29;mcu_nop()
#define _MCU_DELAY_CYCLE_X31 _MCU_DELAY_CYCLE_X30;mcu_nop()
#define _MCU_DELAY_CYCLE_X32 _MCU_DELAY_CYCLE_X31;mcu_nop()
#define _MCU_DELAY_CYCLE_X33 _MCU_DELAY_CYCLE_X32;mcu_nop()
#define _MCU_DELAY_CYCLE_X34 _MCU_DELAY_CYCLE_X33;mcu_nop()
#define _MCU_DELAY_CYCLE_X35 _MCU_DELAY_CYCLE_X34;mcu_nop()
#define _MCU_DELAY_CYCLE_X36 _MCU_DELAY_CYCLE_X35;mcu_nop()
#define _MCU_DELAY_CYCLE_X37 _MCU_DELAY_CYCLE_X36;mcu_nop()
#define _MCU_DELAY_CYCLE_X38 _MCU_DELAY_CYCLE_X37;mcu_nop()
#define _MCU_DELAY_CYCLE_X39 _MCU_DELAY_CYCLE_X38;mcu_nop()
#define _MCU_DELAY_CYCLE_X40 _MCU_DELAY_CYCLE_X39;mcu_nop()
#define _MCU_DELAY_CYCLE_X41 _MCU_DELAY_CYCLE_X40;mcu_nop()
#define _MCU_DELAY_CYCLE_X42 _MCU_DELAY_CYCLE_X41;mcu_nop()
#define _MCU_DELAY_CYCLE_X43 _MCU_DELAY_CYCLE_X42;mcu_nop()
#define _MCU_DELAY_CYCLE_X44 _MCU_DELAY_CYCLE_X43;mcu_nop()
#define _MCU_DELAY_CYCLE_X45 _MCU_DELAY_CYCLE_X44;mcu_nop()
#define _MCU_DELAY_CYCLE_X46 _MCU_DELAY_CYCLE_X45;mcu_nop()
#define _MCU_DELAY_CYCLE_X47 _MCU_DELAY_CYCLE_X46;mcu_nop()
#define _MCU_DELAY_CYCLE_X48 _MCU_DELAY_CYCLE_X47;mcu_nop()
#define _MCU_DELAY_CYCLE_X49 _MCU_DELAY_CYCLE_X48;mcu_nop()
#define _MCU_DELAY_CYCLE_X(LOOPS) _MCU_DELAY_CYCLE_X##LOOPS
#define MCU_DELAY_CYCLE_X(LOOPS) _MCU_DELAY_CYCLE_X(LOOPS)

#define mcu_delay_100ns() MCU_DELAY_CYCLE_X(MCU_100NS_LOOPS)

	/**
	 * runs all internal tasks of the MCU.
	 * for the moment these are:
	 *   - if USB is enabled and MCU uses tinyUSB framework run tinyUSB tud_task
	 *   - if ENABLE_SYNC_RX is enabled check if there are any chars in the rx transmitter (or the tinyUSB buffer) and read them to the mcu_com_rx_cb
	 *   - if ENABLE_SYNC_TX is enabled check if serial_tx_empty is false and run mcu_com_tx_cb
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

#ifdef MCU_HAS_SPI
#ifndef mcu_spi_xmit
	uint8_t mcu_spi_xmit(uint8_t data);
#endif

#ifndef mcu_spi_config
	void mcu_spi_config(uint8_t mode, uint32_t frequency);
#endif
#endif

#ifdef MCU_HAS_I2C
#ifndef mcu_i2c_write
		uint8_t mcu_i2c_write(uint8_t data, bool send_start, bool send_stop);
#endif

#ifndef mcu_i2c_read
	uint8_t mcu_i2c_read(bool with_ack, bool send_stop);
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
