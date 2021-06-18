/*
	Name: mcu.h
	Description: Contains all the function declarations necessary to interact with the MCU.
        This provides a opac intenterface between the µCNC and the MCU unit used to power the µCNC.

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

#include "ucnc_config.h"
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/*IO functions*/
#ifndef mcu_get_input
	uint8_t mcu_get_input(uint8_t pin);
#endif

#ifndef mcu_get_output
	uint8_t mcu_get_output(uint8_t pin);
#endif

#ifndef mcu_set_output
	void mcu_set_output(uint8_t pin);
#endif

#ifndef mcu_clear_output
	void mcu_clear_output(uint8_t pin);
#endif

#ifndef mcu_toggle_output
	void mcu_toggle_output(uint8_t pin);
#endif

	void mcu_init(void);
#ifndef mcu_enable_probe_isr
	void mcu_enable_probe_isr(void);
#endif
#ifndef mcu_disable_probe_isr
	void mcu_disable_probe_isr(void);
#endif

//Analog input
#ifndef mcu_get_analog
	uint8_t mcu_get_analog(uint8_t channel);
#endif

//PWM
#ifndef mcu_set_pwm
	void mcu_set_pwm(uint8_t pwm, uint8_t value);
#endif

#ifndef mcu_get_pwm
	uint8_t mcu_get_pwm(uint8_t pwm);
#endif

//Communication functions
#ifndef mcu_start_send
	void mcu_start_send(void); //Start async send
#endif
#ifndef mcu_stop_send
	void mcu_stop_send(void); //Stop async send
#endif

#ifndef mcu_putc
	void mcu_putc(char c);
#endif

#ifndef mcu_getc
	char mcu_getc(void);
#endif

//ISR
//enables all interrupts on the mcu. Must be called to enable all IRS functions
#ifndef mcu_enable_interrupts
	void mcu_enable_interrupts(void);
#endif
//disables all ISR functions
#ifndef mcu_disable_interrupts
	void mcu_disable_interrupts(void);
#endif

	//Timers
	//convert step rate to clock cycles
	void mcu_freq_to_clocks(float frequency, uint16_t *ticks, uint16_t *prescaller);
	//starts a constant rate pulse at a given frequency.
	void mcu_start_step_ISR(uint16_t ticks, uint16_t prescaller);
	//modifies the pulse frequency
	void mcu_change_step_ISR(uint16_t ticks, uint16_t prescaller);
	//stops the pulse
	void mcu_step_stop_ISR(void);

//Custom delay function
#ifdef RTC_ENABLE
	//gets the mcu running time in ms
	uint32_t mcu_millis();
#endif

#ifndef mcu_delay_ms
	//Custom delay function
	void mcu_delay_ms(uint32_t miliseconds);
#endif

	//Non volatile memory
	uint8_t mcu_eeprom_getc(uint16_t address);
	void mcu_eeprom_putc(uint16_t address, uint8_t value);

#ifdef __PERFSTATS__
	uint16_t mcu_get_step_clocks(void);
	uint16_t mcu_get_step_reset_clocks(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
