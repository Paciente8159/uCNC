/*
	Name: mcu_avr.c
	Description: Implements mcu interface on AVR.
		Besides all the functions declared in the mcu.h it also implements the code responsible
		for handling:
			interpolator.h
				void itp_step_isr();
				void itp_step_reset_isr();
			serial.h
				void serial_rx_isr(char c);
				char serial_tx_isr();
			trigger_control.h
				void io_limits_isr(uint8_t limits);
				void io_controls_isr(uint8_t controls);
				
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
#include "../../config.h"
#include "../../mcudefs.h"
#ifdef __MCU_AVR__
#include "../../mcumap.h"
#include "../../mcu.h"
#include "../../utils.h"
#include "../../serial.h"
#include "../../interpolator.h"
#include "../../io_control.h"

#include <math.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
//#include <avr/delay.h>
#include <avr/eeprom.h>

#ifdef ESTOP_PULLUP
#define ESTOP_PULLUP_MASK ESTOP_MASK
#else
#define ESTOP_PULLUP_MASK 0
#endif
#ifdef FHOLD_PULLUP
#define FHOLD_PULLUP_MASK FHOLD_MASK
#else
#define FHOLD_PULLUP_MASK 0
#endif
#ifdef PROBE_PULLUP
#define PROBE_PULLUP_MASK PROBE_MASK
#else
#define PROBE_PULLUP_MASK 0
#endif
#ifdef LIMIT_X_PULLUP
#define LIMIT_X_PULLUP_MASK LIMIT_X_MASK
#else
#define LIMIT_X_PULLUP_MASK 0
#endif
#ifdef LIMIT_Y_PULLUP
#define LIMIT_Y_PULLUP_MASK LIMIT_Y_MASK
#else
#define LIMIT_Y_PULLUP_MASK 0
#endif
#ifdef LIMIT_Z_PULLUP
#define LIMIT_Z_PULLUP_MASK LIMIT_Z_MASK
#else
#define LIMIT_Z_PULLUP_MASK 0
#endif
#ifdef LIMIT_A_PULLUP
#define LIMIT_A_PULLUP_MASK LIMIT_A_MASK
#else
#define LIMIT_A_PULLUP_MASK 0
#endif
#ifdef LIMIT_B_PULLUP
#define LIMIT_B_PULLUP_MASK LIMIT_B_MASK
#else
#define LIMIT_B_PULLUP_MASK 0
#endif
#ifdef LIMIT_C_PULLUP
#define LIMIT_C_PULLUP_MASK LIMIT_C_MASK
#else
#define LIMIT_C_PULLUP_MASK 0
#endif
#ifdef CS_RES_PULLUP
#define CS_RES_PULLUP_MASK CS_RES_MASK
#else
#define CS_RES_PULLUP_MASK 0
#endif
#ifdef SAFETY_DOOR_PULLUP
#define SAFETY_DOOR_PULLUP_MASK SAFETY_DOOR_MASK
#else
#define SAFETY_DOOR_PULLUP_MASK 0
#endif

#define CONTROLS_PULLUP_MASK (ESTOP_PULLUP_MASK | FHOLD_PULLUP_MASK | CS_RES_PULLUP_MASK | SAFETY_DOOR_PULLUP_MASK)
#define LIMITS_PULLUP_MASK (LIMIT_X_PULLUP_MASK | LIMIT_Y_PULLUP_MASK | LIMIT_Z_PULLUP_MASK | LIMIT_A_PULLUP_MASK | LIMIT_B_PULLUP_MASK | LIMIT_C_PULLUP_MASK)

#ifdef DIN0_PULLUP
#define DIN0_PULLUP_MASK (DIN0_MASK>>0)
#else
#define DIN0_PULLUP_MASK 0
#endif
#ifdef DIN1_PULLUP
#define DIN1_PULLUP_MASK (DIN1_MASK>>0)
#else
#define DIN1_PULLUP_MASK 0
#endif
#ifdef DIN2_PULLUP
#define DIN2_PULLUP_MASK (DIN2_MASK>>0)
#else
#define DIN2_PULLUP_MASK 0
#endif
#ifdef DIN3_PULLUP
#define DIN3_PULLUP_MASK (DIN3_MASK>>0)
#else
#define DIN3_PULLUP_MASK 0
#endif
#ifdef DIN4_PULLUP
#define DIN4_PULLUP_MASK (DIN4_MASK>>0)
#else
#define DIN4_PULLUP_MASK 0
#endif
#ifdef DIN5_PULLUP
#define DIN5_PULLUP_MASK (DIN5_MASK>>0)
#else
#define DIN5_PULLUP_MASK 0
#endif
#ifdef DIN6_PULLUP
#define DIN6_PULLUP_MASK (DIN6_MASK>>0)
#else
#define DIN6_PULLUP_MASK 0
#endif
#ifdef DIN7_PULLUP
#define DIN7_PULLUP_MASK (DIN7_MASK>>0)
#else
#define DIN7_PULLUP_MASK 0
#endif
#ifdef DIN8_PULLUP
#define DIN8_PULLUP_MASK (DIN8_MASK>>8)
#else
#define DIN8_PULLUP_MASK 0
#endif
#ifdef DIN9_PULLUP
#define DIN9_PULLUP_MASK (DIN9_MASK>>8)
#else
#define DIN9_PULLUP_MASK 0
#endif
#ifdef DIN10_PULLUP
#define DIN10_PULLUP_MASK (DIN10_MASK>>8)
#else
#define DIN10_PULLUP_MASK 0
#endif
#ifdef DIN11_PULLUP
#define DIN11_PULLUP_MASK (DIN11_MASK>>8)
#else
#define DIN11_PULLUP_MASK 0
#endif
#ifdef DIN12_PULLUP
#define DIN12_PULLUP_MASK (DIN12_MASK>>8)
#else
#define DIN12_PULLUP_MASK 0
#endif
#ifdef DIN13_PULLUP
#define DIN13_PULLUP_MASK (DIN13_MASK>>8)
#else
#define DIN13_PULLUP_MASK 0
#endif
#ifdef DIN14_PULLUP
#define DIN14_PULLUP_MASK (DIN14_MASK>>8)
#else
#define DIN14_PULLUP_MASK 0
#endif
#ifdef DIN15_PULLUP
#define DIN15_PULLUP_MASK (DIN15_MASK>>8)
#else
#define DIN15_PULLUP_MASK 0
#endif
#ifdef DIN16_PULLUP
#define DIN16_PULLUP_MASK (DIN16_MASK>>16)
#else
#define DIN16_PULLUP_MASK 0
#endif
#ifdef DIN17_PULLUP
#define DIN17_PULLUP_MASK (DIN17_MASK>>16)
#else
#define DIN17_PULLUP_MASK 0
#endif
#ifdef DIN18_PULLUP
#define DIN18_PULLUP_MASK (DIN18_MASK>>16)
#else
#define DIN18_PULLUP_MASK 0
#endif
#ifdef DIN19_PULLUP
#define DIN19_PULLUP_MASK (DIN19_MASK>>16)
#else
#define DIN19_PULLUP_MASK 0
#endif
#ifdef DIN20_PULLUP
#define DIN20_PULLUP_MASK (DIN20_MASK>>16)
#else
#define DIN20_PULLUP_MASK 0
#endif
#ifdef DIN21_PULLUP
#define DIN21_PULLUP_MASK (DIN21_MASK>>16)
#else
#define DIN21_PULLUP_MASK 0
#endif
#ifdef DIN22_PULLUP
#define DIN22_PULLUP_MASK (DIN22_MASK>>16)
#else
#define DIN22_PULLUP_MASK 0
#endif
#ifdef DIN23_PULLUP
#define DIN23_PULLUP_MASK (DIN23_MASK>>16)
#else
#define DIN23_PULLUP_MASK 0
#endif
#ifdef DIN24_PULLUP
#define DIN24_PULLUP_MASK (DIN24_MASK>>24)
#else
#define DIN24_PULLUP_MASK 0
#endif
#ifdef DIN25_PULLUP
#define DIN25_PULLUP_MASK (DIN25_MASK>>24)
#else
#define DIN25_PULLUP_MASK 0
#endif
#ifdef DIN26_PULLUP
#define DIN26_PULLUP_MASK (DIN26_MASK>>24)
#else
#define DIN26_PULLUP_MASK 0
#endif
#ifdef DIN27_PULLUP
#define DIN27_PULLUP_MASK (DIN27_MASK>>24)
#else
#define DIN27_PULLUP_MASK 0
#endif
#ifdef DIN28_PULLUP
#define DIN28_PULLUP_MASK (DIN28_MASK>>24)
#else
#define DIN28_PULLUP_MASK 0
#endif
#ifdef DIN29_PULLUP
#define DIN29_PULLUP_MASK (DIN29_MASK>>24)
#else
#define DIN29_PULLUP_MASK 0
#endif
#ifdef DIN30_PULLUP
#define DIN30_PULLUP_MASK (DIN30_MASK>>24)
#else
#define DIN30_PULLUP_MASK 0
#endif
#ifdef DIN31_PULLUP
#define DIN31_PULLUP_MASK (DIN31_MASK>>24)
#else
#define DIN31_PULLUP_MASK 0
#endif

#define DINS_R0_PULLUP_MASK (DIN0_PULLUP_MASK | DIN1_PULLUP_MASK | DIN2_PULLUP_MASK | DIN3_PULLUP_MASK | DIN4_PULLUP_MASK | DIN5_PULLUP_MASK | DIN6_PULLUP_MASK | DIN7_PULLUP_MASK)
#define DINS_R1_PULLUP_MASK (DIN8_PULLUP_MASK | DIN9_PULLUP_MASK | DIN10_PULLUP_MASK | DIN11_PULLUP_MASK | DIN12_PULLUP_MASK | DIN13_PULLUP_MASK | DIN14_PULLUP_MASK | DIN15_PULLUP_MASK)
#define DINS_R2_PULLUP_MASK (DIN16_PULLUP_MASK | DIN17_PULLUP_MASK | DIN18_PULLUP_MASK | DIN19_PULLUP_MASK | DIN20_PULLUP_MASK | DIN21_PULLUP_MASK | DIN22_PULLUP_MASK | DIN23_PULLUP_MASK)
#define DINS_R3_PULLUP_MASK (DIN24_PULLUP_MASK | DIN25_PULLUP_MASK | DIN26_PULLUP_MASK | DIN27_PULLUP_MASK | DIN28_PULLUP_MASK | DIN29_PULLUP_MASK | DIN30_PULLUP_MASK | DIN31_PULLUP_MASK)

//Helper macros
#define VARNAME(prefix,id,suffix) prefix##id##suffix
#define VAREVAL(prefix,id,suffix) VARNAME(prefix,id,suffix)

//Timer registers
#define TIMER_COMPB_vect VAREVAL(TIMER, STEP_TIMER_ID, _COMPB_vect)
#define TIMER_COMPA_vect VAREVAL(TIMER, STEP_TIMER_ID, _COMPA_vect)
#define TCNT VAREVAL(TCNT, STEP_TIMER_ID, )
#define TCNT VAREVAL(TCNT, STEP_TIMER_ID, )
#define TCCRA VAREVAL(TCCR, STEP_TIMER_ID, A)
#define TCCRB VAREVAL(TCCR, STEP_TIMER_ID, B)
#define OCRA VAREVAL(OCR, STEP_TIMER_ID, A)
#define OCRB VAREVAL(OCR, STEP_TIMER_ID, B)
#define TIFR VAREVAL(TIFR, STEP_TIMER_ID, )
#define TIMSK VAREVAL(TIMSK, STEP_TIMER_ID, )
#define OCIEB VAREVAL(OCIE, STEP_TIMER_ID, B)
#define OCIEA VAREVAL(OCIE, STEP_TIMER_ID, A)

//COM registers
#ifdef COM_ID
#define COM_RX_vect VAREVAL(USART, COM_ID, _RX_vect)
#define COM_TX_vect VAREVAL(USART, COM_ID, _UDRE_vect)
#else
#define COM_RX_vect USART_RX_vect
#define COM_TX_vect USART_UDRE_vect
#define COM_ID 0
#endif
#define UCSRB VAREVAL(UCSR, COM_ID, B)
#define UCSRA VAREVAL(UCSR, COM_ID, A)
#define UDRIE VAREVAL(UDRIE, COM_ID, )
#define U2X VAREVAL(U2X, COM_ID, )
#define UBRRH VAREVAL(UBRR, COM_ID, H)
#define UBRRL VAREVAL(UBRR, COM_ID, L)
#define RXEN VAREVAL(RXEN, COM_ID, )
#define TXEN VAREVAL(TXEN, COM_ID, )
#define RXCIE VAREVAL(RXCIE, COM_ID, )
#define UDRE VAREVAL(UDRE, COM_ID, )
#define RXC VAREVAL(RXC, COM_ID, )

//Pin change ISR Registers
#define LIMITS_ISRREG VAREVAL(PCMSK, LIMITS_ISR_ID, )
#define CONTROLS_ISRREG VAREVAL(PCMSK, CONTROLS_ISR_ID, )
#define PROBE_ISRREG VAREVAL(PCMSK, PROBE_ISR_ID, )

#ifdef PWM0
#ifndef PWM0_TIMER_ID
#error Undefined PWM0 Timer
#endif
#if(PWM0_TIMER_ID==STEP_TIMER_ID)
#error PWM0 timer and Step timer must be different
#endif
#ifndef PWM0_OCR_ID
#error Undefined PWM0 OCR Register
#endif
#define PWM0_TMRAREG VAREVAL(TCCR, PWM0_TIMER_ID, A)
#define PWM0_TMRBREG VAREVAL(TCCR, PWM0_TIMER_ID, B)
#define PWM0_CNTREG VAREVAL(OCR, PWM0_TIMER_ID, A)
#if(PWM0_OCR_ID == A)
#define PWM0_ENABLE_MASK 0x80
#elif(PWM0_OCR_ID == B)
#define PWM0_ENABLE_MASK 0x20
#elif(PWM0_OCR_ID == C)
#define PWM0_ENABLE_MASK 0x08
#endif
#if(PWM0_TIMER_ID==0 || PWM0_TIMER_ID==2)
#define PWM0_MODE 0x03
#define PWM0_PRESCALLER 0x04
#else
#define PWM0_MODE 0x05
#define PWM0_PRESCALLER 0x03
#endif
#endif

#ifdef PWM1
#ifndef PWM1_TIMER_ID
#error Undefined PWM_1 Timer
#endif
#if(PWM1_TIMER_ID==STEP_TIMER_ID)
#error PWM0 timer and Step timer must be different
#endif
#ifndef PWM1_OCR_ID
#error Undefined PWM_1 OCR Register
#endif
#define PWM1_TMRAREG VAREVAL(TCCR, PWM1_TIMER_ID, A)
#define PWM1_TMRBREG VAREVAL(TCCR, PWM1_TIMER_ID, B)
#define PWM1_CNTREG VAREVAL(OCR, PWM1_TIMER_ID, A)
#if(PWM0_OCR_ID == A)
#define PWM0_ENABLE_MASK 0x80
#elif(PWM0_OCR_ID == B)
#define PWM0_ENABLE_MASK 0x20
#elif(PWM0_OCR_ID == C)
#define PWM0_ENABLE_MASK 0x08
#endif
#if(PWM1_TIMER_ID==0 || PWM1_TIMER_ID==2)
#define PWM1_MODE 0x03
#define PWM1_PRESCALLER 0x04
#else
#define PWM1_MODE 0x05
#define PWM1_PRESCALLER 0x03
#endif
#endif

#ifdef PWM2
#ifndef PWM2_TIMER_ID
#error Undefined PWM2 Timer
#endif
#if(PWM2_TIMER_ID==STEP_TIMER_ID)
#error PWM0 timer and Step timer must be different
#endif
#ifndef PWM2_OCR_ID
#error Undefined PWM2 OCR Register
#endif
#define PWM2_TMRAREG VAREVAL(TCCR, PWM2_TIMER_ID, A)
#define PWM2_TMRBREG VAREVAL(TCCR, PWM2_TIMER_ID, B)
#define PWM2_CNTREG VAREVAL(OCR, PWM2_TIMER_ID, A)
#if(PWM2_OCR_ID == A)
#define PWM2_ENABLE_MASK 0x80
#elif(PWM2_OCR_ID == B)
#define PWM2_ENABLE_MASK 0x20
#elif(PWM2_OCR_ID == C)
#define PWM2_ENABLE_MASK 0x08
#endif
#if(PWM2_TIMER_ID==0 || PWM2_TIMER_ID==2)
#define PWM2_MODE 0x03
#define PWM2_PRESCALLER 0x04
#else
#define PWM2_MODE 0x05
#define PWM2_PRESCALLER 0x03
#endif
#endif

#ifdef PWM3
#ifndef PWM3_TIMER_ID
#error Undefined PWM0 Timer
#endif
#if(PWM3_TIMER_ID==STEP_TIMER_ID)
#error PWM0 timer and Step timer must be different
#endif
#ifndef PWM0_OCR_ID
#error Undefined PWM0 OCR Register
#endif
#define PWM3_TMRAREG VAREVAL(TCCR, PWM3_TIMER_ID, A)
#define PWM3_TMRBREG VAREVAL(TCCR, PWM3_TIMER_ID, B)
#define PWM3_CNTREG VAREVAL(OCR, PWM3_TIMER_ID, A)
#if(PWM3_OCR_ID == A)
#define PWM3_ENABLE_MASK 0x80
#elif(PWM3_OCR_ID == B)
#define PWM3_ENABLE_MASK 0x20
#elif(PWM3_OCR_ID == C)
#define PWM3_ENABLE_MASK 0x08
#endif
#if(PWM3_TIMER_ID==0 || PWM3_TIMER_ID==2)
#define PWM3_MODE 0x03
#define PWM3_PRESCALLER 0x04
#else
#define PWM3_MODE 0x05
#define PWM3_PRESCALLER 0x03
#endif
#endif

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#ifndef BAUD
#define BAUD 115200
#endif


#ifdef __PERFSTATS__
volatile uint16_t mcu_perf_step;
volatile uint16_t mcu_perf_step_reset;

uint16_t mcu_get_step_clocks()
{
	uint16_t res = mcu_perf_step;
	return res;
}
uint16_t mcu_get_step_reset_clocks()
{
	uint16_t res = mcu_perf_step_reset;
	return res;
}
#endif

ISR(TIMER_COMPA_vect, ISR_BLOCK)
{
	#ifdef __PERFSTATS__
	uint16_t clocks = TCNT;
	#endif
	itp_step_reset_isr();
	
	#ifdef __PERFSTATS__
    uint16_t clocks2 = TCNT;
    clocks2 -= clocks;
	mcu_perf_step_reset = MAX(mcu_perf_step_reset, clocks2);
	#endif
}

ISR(TIMER_COMPB_vect, ISR_BLOCK)
{
	#ifdef __PERFSTATS__
	uint16_t clocks = TCNT;
	#endif
    itp_step_isr();
    #ifdef __PERFSTATS__
    uint16_t clocks2 = TCNT;
    clocks2 -= clocks;
	mcu_perf_step = MAX(mcu_perf_step, clocks2);
	#endif
}

ISR(PCINT0_vect, ISR_BLOCK) // input pin on change service routine
{
	static uint8_t prev_value = 0;
	uint8_t value = PCMASK0_INREG;
	uint8_t diff = prev_value ^ value;
	prev_value = value;
	
	#if(LIMITS_ISR_ID==0)
	if(diff & LIMITS_MASK)
	{
		io_limits_isr(value & LIMITS_MASK);
	}
	#endif
	
	#if(CONTROLS_ISR_ID==0)
	if(diff & CONTROLS_MASK)
	{
		io_controls_isr(value & CONTROLS_MASK);
	}
	#endif
	
	#if(PROBE_ISR_ID==0)
	if(diff & PROBE_MASK)
	{
		io_probe_isr(value & PROBE_MASK);
	}
	#endif		
}

ISR(PCINT1_vect, ISR_BLOCK) // input pin on change service routine
{
	static uint8_t prev_value = 0;
	uint8_t value = PCMASK1_INREG;
	uint8_t diff = prev_value ^ value;
	prev_value = value;
	
	#if(LIMITS_ISR_ID==1)
	if(diff & LIMITS_MASK)
	{
		io_limits_isr(value & LIMITS_MASK);
	}
	#endif
	
	#if(CONTROLS_ISR_ID==1)
	if(diff & CONTROLS_MASK)
	{
		io_controls_isr((value & CONTROLS_MASK));
	}
	#endif
	
	#if(PROBE_ISR_ID==1)
	if(diff & PROBE_MASK)
	{
		io_probe_isr(value & PROBE_MASK);
	}
	#endif
}

ISR(PCINT2_vect, ISR_BLOCK) // input pin on change service routine
{
    static uint8_t prev_value = 0;
	uint8_t value = PCMASK2_INREG;
	uint8_t diff = prev_value ^ value;
	prev_value = value;
	
	#if(LIMITS_ISR_ID==2)
	if(diff & LIMITS_MASK)
	{
		io_limits_isr(value & LIMITS_MASK);
	}
	#endif
	
	#if(CONTROLS_ISR_ID==2)
	if(diff & CONTROLS_MASK)
	{
		io_controls_isr(value & CONTROLS_MASK);
	}
	#endif
	
	#if(PROBE_ISR_ID==2)
	if(diff & PROBE_MASK)
	{
		io_probe_isr(value & PROBE_MASK);
	}
	#endif
}

ISR(COM_RX_vect, ISR_BLOCK)
{
	serial_rx_isr(COM_INREG);
}

ISR(COM_TX_vect, ISR_BLOCK)
{
	serial_tx_isr();
	/*{
		UCSRB &= ~(1<<UDRIE);
	}*/
}

void mcu_init()
{
	//disable WDT
	wdt_reset();
    MCUSR &= ~(1<<WDRF);
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	WDTCSR = 0x00;

    //sets all outputs and inputs
    //inputs
    #ifdef CONTROLS_DIRREG
        CONTROLS_DIRREG = 0;
    #endif
    
	#ifdef LIMITS_DIRREG
        LIMITS_DIRREG = 0;
    #endif
    
    #ifdef PROBE_DIRREG
        PROBE_DIRREG = 0;
    #endif
    
	#ifdef COM_DIRREG
        COM_DIRREG = 0;
    #endif
    
    #ifdef DINS_R0_DIRREG
        DINS_R0_DIRREG = 0;
    #endif
    
    #ifdef DINS_R1_DIRREG
        DINS_R1_DIRREG = 0;
    #endif
    
    #ifdef DINS_R2_DIRREG
        DINS_R2_DIRREG = 0;
    #endif
    
    #ifdef DINS_R3_DIRREG
        DINS_R3_DIRREG = 0;
    #endif

	#ifdef ANALOG_DIRREG
		ANALOG_DIRREG = 0;
	#endif
    
    //pull-ups
    #ifdef CONTROLS_PULLUPREG
        CONTROLS_PULLUPREG |= CONTROLS_PULLUP_MASK;
    #endif
    
	#ifdef LIMITS_PULLUPREG
        LIMITS_PULLUPREG |= LIMITS_PULLUP_MASK;
    #endif
    
    #ifdef PROBE_PULLUPREG
        PROBE_PULLUPREG |= PROBE_PULLUP_MASK;
    #endif

    #ifdef DINS_R0_PULLUPREG
        DINS_R0_PULLUPREG |= DINS_R0_PULLUP_MASK;
    #endif
    
    #ifdef DINS_R1_PULLUPREG
        DINS_R1_PULLUPREG |= DINS_R1_PULLUP_MASK;
    #endif
    
    #ifdef DINS_R2_PULLUPREG
        DINS_R2_PULLUPREG |= DINS_R2_PULLUP_MASK;
    #endif
    
    #ifdef DINS_R3_PULLUPREG
        DINS_R3_PULLUPREG |= DINS_R3_PULLUP_MASK;
    #endif
    
    //outputs
    #ifdef STEPS_DIRREG
        STEPS_DIRREG |= STEPS_MASK;
    #endif
    
	#ifdef DIRS_DIRREG
        DIRS_DIRREG |= DIRS_MASK;
    #endif
    
    #ifdef COM_DIRREG
        COM_DIRREG |= TX_MASK;
    #endif
    
	#ifdef DOUTS_R0_DIRREG
        DOUTS_R0_DIRREG |= DOUTS_R0_MASK;
    #endif
    
    #ifdef DOUTS_R1_DIRREG
        DOUTS_R1_DIRREG |= DOUTS_R1_MASK;
    #endif
    
    #ifdef DOUTS_R2_DIRREG
        DOUTS_R2_DIRREG |= DOUTS_R2_MASK;
    #endif
    
    #ifdef DOUTS_R3_DIRREG
        DOUTS_R3_DIRREG |= DOUTS_R3_MASK;
    #endif

    //activate Pin on change interrupt
    #ifdef LIMITS_ISR_ID
    PCICR |= (1<<LIMITS_ISR_ID);
    #endif
    #ifdef CONTROLS_ISR_ID
	PCICR |= (1<<CONTROLS_ISR_ID);
	#endif
	#ifdef PROBE_ISR_ID
	PCICR |= (1<<PROBE_ISR_ID);
	#endif
    
    #ifdef LIMITS_ISRREG
    	LIMITS_ISRREG |= LIMITS_MASK;
    #endif
    #ifdef CONTROLS_ISRREG
    	CONTROLS_ISRREG |= CONTROLS_MASK;
    #endif
    #ifdef PROBE_ISRREG //probe is disabled at start
    	PROBE_ISRREG &= ~PROBE_MASK;
    #endif

    //stdout = &g_mcu_streamout;

	//PWM's
	//TCCRXA Mode 3/5 - Fast PWM 
	//TCCRXB Prescaller 1/64
	#ifdef PWM0
		PWM0_DIRREG |= PWM0_MASK;
		PWM0_TMRAREG = PWM0_MODE;
		PWM0_TMRBREG = PWM0_PRESCALLER;
		PWM0_CNTREG = 0;
	#endif
	#ifdef PWM1
		PWM1_DIRREG |= PWM1_MASK;
		PWM1_TMRAREG = PWM1_MODE;
		PWM1_TMRBREG = PWM1_PRESCALLER;
		PWM1_CNTREG = 0;
	#endif
	#ifdef PWM2
		PWM2_DIRREG |= PWM2_MASK;
		PWM2_TMRAREG = PWM2_MODE;
		PWM2_TMRBREG = PWM2_PRESCALLER;
		PWM2_CNTREG = 0;
	#endif
	#ifdef PWM3
		PWM3_DIRREG |= PWM3_MASK;
		PWM3_TMRAREG = PWM3_MODE;
		PWM3_TMRBREG = PWM3_PRESCALLER;
		PWM3_CNTREG = 0;
	#endif

    // Set baud rate
	uint16_t UBRR_value;
    #if BAUD < 57600
      UBRR_value = ((F_CPU / (8L * BAUD)) - 1)/2 ;
      UCSRA &= ~(1 << U2X); // baud doubler off  - Only needed on Uno XXX
    #else
      UBRR_value = ((F_CPU / (4L * BAUD)) - 1)/2;
      UCSRA |= (1 << U2X);  // baud doubler on for high baud rates, i.e. 115200
    #endif
    UBRRH = UBRR_value >> 8;
    UBRRL = UBRR_value;
  
    // enable rx, tx, and interrupt on complete reception of a byte and UDR empty
    UCSRB |= (1<<RXEN | 1<<TXEN | 1<<RXCIE);
    
	//enable interrupts
	sei();
}

//IO functions    
#ifdef PROBE
void mcu_enable_probe_isr()
{
	#ifdef PROBE_ISRREG
    	PROBE_ISRREG |= PROBE_MASK;
    #endif
}

void mcu_disable_probe_isr()
{
	#ifdef PROBE_ISRREG
    	PROBE_ISRREG &= ~PROBE_MASK;
    #endif
}
#endif

uint8_t mcu_get_analog(uint8_t channel)
{
	ADMUX = (0x42 | channel); //VRef = Vcc with reading left aligned
	ADCSRA = 0xC7; //Start read with ADC with 128 prescaller
	do
	{
	}while(CHECKBIT(ADCSRA,ADSC));
	uint8_t result = ADCH;
	ADCSRA = 0; //switch adc off
	ADMUX = 0; //switch adc off

	return result;
}

void mcu_set_pwm(uint8_t pwm, uint8_t value)
{
	switch(pwm)
	{
		case 0:
			#ifdef PWM0
			PWM0_CNTREG = value;
			if(value != 0)
			{
				SETFLAG(PWM0_TMRAREG,PWM0_ENABLE_MASK);
			}
			else
			{
				CLEARFLAG(PWM0_TMRAREG,PWM0_ENABLE_MASK);
			}
			#endif
			break;
		case 1:
			#ifdef PWM1
			PWM1_CNTREG = value;
			if(value != 0)
			{
				SETFLAG(PWM1_TMRAREG,PWM1_ENABLE_MASK);
			}
			else
			{
				CLEARFLAG(PWM1_TMRAREG,PWM1_ENABLE_MASK);
			}
			#endif
			break;
		case 2:
			#ifdef PWM2
			PWM2_CNTREG = value;
			if(value != 0)
			{
				SETFLAG(PWM2_TMRAREG,PWM2_ENABLE_MASK);
			}
			else
			{
				CLEARFLAG(PWM2_TMRAREG,PWM2_ENABLE_MASK);
			}
			#endif
			break;
		case 3:
			#ifdef PWM3
			PWM3_CNTREG = value;
			if(value != 0)
			{
				SETFLAG(PWM3_TMRAREG,PWM3_ENABLE_MASK);
			}
			else
			{
				CLEARFLAG(PWM3_TMRAREG,PWM3_ENABLE_MASK);
			}
			#endif
			break;
	}
}

uint8_t mcu_get_pwm(uint8_t pwm)
{
	switch(pwm)
	{
		case 0:
			#ifdef PWM0
			return PWM0_CNTREG;
			#endif
			break;
		case 1:
			#ifdef PWM1
			return PWM1_CNTREG;
			#endif
			break;
		case 2:
			#ifdef PWM2
			return PWM2_CNTREG;
			#endif
			break;
		case 3:
			#ifdef PWM3
			return PWM3_CNTREG;
			#endif
			break;
	}
	
	return 0;
}

void mcu_enable_interrupts()
{
	sei();
}
void mcu_disable_interrupts()
{
	cli();
}
/*
//internal redirect of stdout
int mcu_putchar(char c, FILE* stream)
{
	mcu_putc(c);
	return c;
}*/

void mcu_start_send()
{
	SETBIT(UCSRB,UDRIE);
}

void mcu_stop_send()
{
	CLEARBIT(UCSRB,UDRIE);
}

void mcu_putc(char c)
{
	loop_until_bit_is_set(UCSRA, UDRE);
	COM_OUTREG = c;
}

bool mcu_is_tx_ready()
{
	return CHECKBIT(UCSRA, UDRE);
}

char mcu_getc()
{
	loop_until_bit_is_set(UCSRA, RXC);
    return COM_INREG;
}

//RealTime
void mcu_freq_to_clocks(float frequency, uint16_t* ticks, uint8_t* prescaller)
{
	if(frequency < F_STEP_MIN)
		frequency = F_STEP_MIN;
	if(frequency > F_STEP_MAX)
		frequency = F_STEP_MAX;
		
	float clockcounter = F_CPU;
		
	if(frequency >= 245)
	{
		*prescaller = 9;
	}
	else if(frequency >= 31)
	{
		*prescaller = 10;
		clockcounter *= 0.125;
	}
	else if(frequency >= 4)
	{
		*prescaller = 11;
		clockcounter *= 0.015625;		
	}
	else if(frequency >= 1)
	{
		*prescaller = 12;
		clockcounter *= 0.00390625;
	}
	else
	{
		*prescaller = 13;
		clockcounter *= 0.0009765625;
	}

	*ticks = floorf((clockcounter/frequency)) - 1;
}
/*
	initializes the pulse ISR
	In Arduino this is done in TIMER1
	The frequency range is from 4Hz to F_PULSE
*/
void mcu_start_step_ISR(uint16_t clocks_speed, uint8_t prescaller)
{
	//stops timer
	TCCRB = 0;
	//CTC mode
    TCCRA = 0;
    //resets counter
    TCNT = 0;
    //set step clock
    OCRA = clocks_speed;
	//sets OCR0B to half
	//this will allways fire step_reset between pulses
    OCRB = OCRA>>1;
	TIFR = 0;
	// enable timer interrupts on both match registers
    TIMSK |= (1 << OCIEB) | (1 << OCIEA);
    
    //start timer in CTC mode with the correct prescaler
    TCCRB = prescaller;
}

// se implementar amass deixo de necessitar de prescaler
void mcu_change_step_ISR(uint16_t clocks_speed, uint8_t prescaller)
{
	//stops timer
	//TCCRB = 0;
	OCRB = clocks_speed>>1;
	OCRA = clocks_speed;
	//sets OCR0B to half
	//this will allways fire step_reset between pulses
    
	//reset timer
    //TCNT = 0;
	//start timer in CTC mode with the correct prescaler
    TCCRB = prescaller;
}

void mcu_step_stop_ISR()
{
	TCCRB = 0;
    TIMSK &= ~((1 << OCIEB) | (1 << OCIEA));
}

/*#define MCU_1MS_LOOP F_CPU/1000000
static __attribute__((always_inline)) void mcu_delay_1ms() 
{
	uint16_t loop = MCU_1MS_LOOP;
	do{
	}while(--loop);
}*/

/*void mcu_delay_ms(uint16_t miliseconds)
{
	do{
		_delay_ms(1);
	}while(--miliseconds);
	
}*/


//This was copied from grbl
#ifndef EEPE
		#define EEPE  EEWE  //!< EEPROM program/write enable.
		#define EEMPE EEMWE //!< EEPROM master program/write enable.
#endif

#ifndef SELFPRGEN
	#define SELFPRGEN SPMEN
#endif

/* These two are unfortunately not defined in the device include files. */
#define EEPM1 5 //!< EEPROM Programming Mode Bit 1.
#define EEPM0 4 //!< EEPROM Programming Mode Bit 0.

uint8_t mcu_eeprom_getc(uint16_t address)
{
	do {} while( EECR & (1<<EEPE) ); // Wait for completion of previous write.
	EEAR = address; // Set EEPROM address register.
	EECR = (1<<EERE); // Start EEPROM read operation.
	return EEDR; // Return the byte read from EEPROM.
}

uint8_t mcu_eeprom_putc(uint16_t address, uint8_t value)
{
	char old_value; // Old EEPROM value.
	char diff_mask; // Difference mask, i.e. old value XOR new value.

	cli(); // Ensure atomic operation for the write operation.
	
	do {} while( EECR & (1<<EEPE) ); // Wait for completion of previous write
	do {} while( SPMCSR & (1<<SELFPRGEN) ); // Wait for completion of SPM.
	
	EEAR = address; // Set EEPROM address register.
	EECR = (1<<EERE); // Start EEPROM read operation.
	old_value = EEDR; // Get old EEPROM value.
	diff_mask = old_value ^ value; // Get bit differences.
	
	// Check if any bits are changed to '1' in the new value.
	if( diff_mask & value ) {
		// Now we know that _some_ bits need to be erased to '1'.
		
		// Check if any bits in the new value are '0'.
		if( value != 0xff ) {
			// Now we know that some bits need to be programmed to '0' also.
			
			EEDR = value; // Set EEPROM data register.
			EECR = (1<<EEMPE) | // Set Master Write Enable bit...
			       (0<<EEPM1) | (0<<EEPM0); // ...and Erase+Write mode.
			EECR |= (1<<EEPE);  // Start Erase+Write operation.
		} else {
			// Now we know that all bits should be erased.

			EECR = (1<<EEMPE) | // Set Master Write Enable bit...
			       (1<<EEPM0);  // ...and Erase-only mode.
			EECR |= (1<<EEPE);  // Start Erase-only operation.
		}
	} else {
		// Now we know that _no_ bits need to be erased to '1'.
		
		// Check if any bits are changed from '1' in the old value.
		if( diff_mask ) {
			// Now we know that _some_ bits need to the programmed to '0'.
			
			EEDR = value;   // Set EEPROM data register.
			EECR = (1<<EEMPE) | // Set Master Write Enable bit...
			       (1<<EEPM1);  // ...and Write-only mode.
			EECR |= (1<<EEPE);  // Start Write-only operation.
		}
	}
	
	sei(); // Restore interrupt flag state.
}

void mcu_eeprom_erase(uint16_t address)
{
	cli(); // Ensure atomic operation for the write operation.
	
	do {} while( EECR & (1<<EEPE) ); // Wait for completion of previous write
	do {} while( SPMCSR & (1<<SELFPRGEN) ); // Wait for completion of SPM.
	
	EEAR = address; // Set EEPROM address register.

	EECR = (1<<EEMPE) | // Set Master Write Enable bit...
			(1<<EEPM0);  // ...and Erase-only mode.
	EECR |= (1<<EEPE);  // Start Erase-only operation.

	sei(); // Restore interrupt flag state.
}

#endif
