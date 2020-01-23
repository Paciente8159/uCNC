/*
	Name: mcu.h
	Description: Contains all the function declarations necessary to interact with the MCU.
        This provides a opac intenterface between the uCNC and the MCU unit used to power the uCNC.

	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 01/11/2019

	uCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	uCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef MCU_H
#define MCU_H

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "config.h"
#include "utils.h"
#include "mcumap.h"

/*IO functions*/
static inline uint32_t mcu_get_inputs()
{
	uint32_t result;
	uint8_t* reg = (uint8_t*)&result;
	#ifdef DINS_R0_INREG
	reg[__UINT32_R0__] = (DINS_R0_INREG & DINS_R0_MASK);
	#endif
	#ifdef DINS_R1_INREG
	reg[__UINT32_R1__] = (DINS_R1_INREG & DINS_R1_MASK);
	#endif
	#ifdef DINS_R2_INREG
	reg[__UINT32_R2__] = (DINS_R2_INREG & DINS_R2_MASK);
	#endif
	#ifdef DINS_R3_INREG
	reg[__UINT32_R3__] = (DINS_R3_INREG & DINS_R3_MASK);
	#endif
	return result;	
}

static inline uint8_t mcu_get_controls()
{
	#ifdef CONTROLS_INREG
	return (CONTROLS_INREG & CONTROLS_MASK);
	#else
	return 0;
	#endif
}

static inline uint8_t mcu_get_limits()
{
	#ifdef LIMITS_INREG
	return (LIMITS_INREG & LIMITS_MASK);
	#else
	return 0;
	#endif
}

static inline bool mcu_get_probe()
{
	#ifdef PROBE_INREG
	return (PROBE_INREG && PROBE_MASK);
	#else
	return false;
	#endif
}

static inline void mcu_set_steps(uint8_t value)
{
	STEPS_OUTREG = (~STEPS_MASK & STEPS_OUTREG) | value;
}

static inline void mcu_set_dirs(uint8_t value)
{
	DIRS_OUTREG = (~DIRS_MASK & DIRS_OUTREG) | value;
}

static inline void mcu_set_outputs(uint32_t value)
{
	uint8_t* reg = (uint8_t*)&value;
	
	#ifdef DOUTS_R0_OUTREG
		DOUTS_R0_OUTREG = ((~DOUTS_R0_MASK & DOUTS_R0_OUTREG) | reg[__UINT32_R0__]);
	#endif
	#ifdef DOUTS_R1_OUTREG
		DOUTS_R1_OUTREG = ((~DOUTS_R1_MASK & DOUTS_R1_OUTREG) | reg[__UINT32_R1__]);
	#endif
	#ifdef DOUTS_R2_OUTREG
		DOUTS_R2_OUTREG = ((~DOUTS_R2_MASK & DOUTS_R2_OUTREG) | reg[__UINT32_R2__]);
	#endif
	#ifdef DOUTS_R3_OUTREG
		DOUTS_R3_OUTREG = ((~DOUTS_R3_MASK & DOUTS_R3_OUTREG) | reg[__UINT32_R3__]);
	#endif
}

static inline uint32_t mcu_get_outputs()
{
	uint32_t result;
	uint8_t* reg = (uint8_t*)&result;

	#ifdef DOUTS_R0_OUTREG
		reg[__UINT32_R0__] = DOUTS_R0_OUTREG;
	#endif
	#ifdef DOUTS_R1_OUTREG
		reg[__UINT32_R1__] = DOUTS_R1_OUTREG;
	#endif
	#ifdef DOUTS_R2_OUTREG
		reg[__UINT32_R2__] = DOUTS_R2_OUTREG;
	#endif
	#ifdef DOUTS_R3_OUTREG
		reg[__UINT32_R3__] = DOUTS_R3_OUTREG;
	#endif
	
	return (result & DOUTS_MASK);
}

void mcu_init();

#ifdef PROBE
void mcu_enable_probe_isr();
void mcu_disable_probe_isr();
#endif

//Analog input
uint8_t mcu_get_analog(uint8_t channel);

//PWM
void mcu_set_pwm(uint8_t pwm, uint8_t value);
uint8_t mcu_get_pwm(uint8_t pwm);

//Communication functions
void mcu_start_send(); //Start async send
void mcu_putc(char c);
char mcu_getc();

//ISR
//enables all interrupts on the mcu. Must be called to enable all IRS functions
void mcu_enable_interrupts();
//disables all ISR functions
void mcu_disable_interrupts();

//Timers
//convert step rate to clock cycles
void mcu_freq_to_clocks(float frequency, uint16_t* ticks, uint8_t* prescaller);
//starts a constant rate pulse at a given frequency.
void mcu_start_step_ISR(uint16_t ticks, uint8_t prescaller);
//modifies the pulse frequency
void mcu_change_step_ISR(uint16_t ticks, uint8_t prescaller);
//stops the pulse 
void mcu_step_stop_ISR();

//Custom delay function
//void mcu_delay_ms(uint16_t miliseconds);

//Non volatile memory
uint8_t mcu_eeprom_getc(uint16_t address);
uint8_t mcu_eeprom_putc(uint16_t address, uint8_t value);

/*
#ifdef __PERFSTATS__
uint16_t mcu_get_step_clocks();
uint16_t mcu_get_step_reset_clocks();
#endif
*/
#endif
