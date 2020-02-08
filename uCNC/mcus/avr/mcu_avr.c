/*
	Name: mcu_avr.c
	Description: Implements mcu interface on AVR.
		Besides all the functions declared in the mcu.h it also implements the code responsible
		for handling:
			interpolator.h
				void itp_step_isr();
				void itp_step_reset_isr();
			serial.h
				void serial_rx_isr(unsinged char c);
				char serial_tx_isr();
			trigger_control.h
				void io_limits_isr();
				void io_controls_isr();
                void io_probe_isr(uint8_t probe);

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 01/11/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/
#include "../../config.h"
#include "../../mcudefs.h"
#include "../../utils.h"
#include "mcumap_avr.h"
#include "../../mcu.h"
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
#include <avr/cpufunc.h>

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

#if(PCINTA_MASK==1)
ISR(INT0_vect, ISR_BLOCK) // input pin on change service routine
{
#if(PCINTA_LIMITS_MASK==1)
    io_limits_isr();
#endif
#if(PCINTA_CONTROLS_MASK==1)
    io_controls_isr();
#endif
#if(PROBE_ISRA==1)
    io_probe_isr();
#endif
}
#endif
#if(PCINTA_MASK==4)
ISR(INT1_vect, ISR_BLOCK) // input pin on change service routine
{
#if(PCINTA_LIMITS_MASK==4)
    io_limits_isr();
#endif
#if(PCINTA_CONTROLS_MASK==4)
    io_controls_isr();
#endif
#if(PROBE_ISRA==4)
    io_probe_isr();
#endif
}
#endif
#if(PCINTA_MASK==16)
ISR(INT2_vect, ISR_BLOCK) // input pin on change service routine
{
#if(PCINTA_LIMITS_MASK==16)
    io_limits_isr();
#endif
#if(PCINTA_CONTROLS_MASK==16)
    io_controls_isr();
#endif
#if(PROBE_ISRA==16)
    io_probe_isr();
#endif
}
#endif
#if(PCINTA_MASK==64)
ISR(INT3_vect, ISR_BLOCK) // input pin on change service routine
{
#if(PCINTA_LIMITS_MASK==64)
    io_limits_isr();
#endif
#if(PCINTA_CONTROLS_MASK==64)
    io_controls_isr();
#endif
#if(PROBE_ISRA==64)
    io_probe_isr();
#endif
}
#endif
#if(PCINTB_MASK==1)
ISR(INT4_vect, ISR_BLOCK) // input pin on change service routine
{
#if(PCINTB_LIMITS_MASK==1)
    io_limits_isr();
#endif
#if(PCINTB_CONTROLS_MASK==1)
    io_controls_isr();
#endif
#if(PROBE_ISRB==1)
    io_probe_isr();
#endif
}
#endif
#if(PCINTB_MASK==4)
ISR(INT5_vect, ISR_BLOCK) // input pin on change service routine
{
#if(PCINTB_LIMITS_MASK==4)
    io_limits_isr();
#endif
#if(PCINTB_CONTROLS_MASK==4)
    io_controls_isr();
#endif
#if(PROBE_ISRB==4)
    io_probe_isr();
#endif
}
#endif
#if(PCINTB_MASK==16)
ISR(INT6_vect, ISR_BLOCK) // input pin on change service routine
{
#if(PCINTB_LIMITS_MASK==16)
    io_limits_isr();
#endif
#if(PCINTB_CONTROLS_MASK==16)
    io_controls_isr();
#endif
#if(PROBE_ISRB==16)
    io_probe_isr();
#endif
}
#endif
#if(PCINTB_MASK==64)
ISR(INT7_vect, ISR_BLOCK) // input pin on change service routine
{
#if(PCINTB_LIMITS_MASK==64)
    io_limits_isr();
#endif
#if(PCINTB_CONTROLS_MASK==64)
    io_controls_isr();
#endif
#if(PROBE_ISRB==64)
    io_probe_isr();
#endif
}
#endif

#if(PCINT0_MASK!=0)
ISR(PCINT0_vect, ISR_BLOCK) // input pin on change service routine
{
    static uint8_t prev_value = 0;
    uint8_t value = PCINT0_INREG;
    uint8_t diff = prev_value ^ value;
    prev_value = value;

#if(PCINT0_LIMITS_MASK!=0)
    if(diff & PCINT0_LIMITS_MASK)
    {
        io_limits_isr();
    }
#endif
#if(PCINT0_CONTROLS_MASK!=0)
    if(diff & PCINT0_CONTROLS_MASK)
    {
        io_controls_isr();
    }
#endif

#if(PROBE_ISR0!=0)
    if(CHECKBIT(diff,PROBE_BIT))
    {
        io_probe_isr();
    }
#endif
}
#endif

#if(PCINT1_MASK!=0)
ISR(PCINT1_vect, ISR_BLOCK) // input pin on change service routine
{
    static uint8_t prev_value = 0;
    uint8_t value = PCINT1_INREG;
    uint8_t diff = prev_value ^ value;
    prev_value = value;

#if(PCINT1_LIMITS_MASK!=0)
    if(diff & PCINT1_LIMITS_MASK)
    {
        io_limits_isr();
    }
#endif
#if(PCINT1_CONTROLS_MASK!=0)
    if(diff & PCINT1_CONTROLS_MASK)
    {
        io_controls_isr();
    }
#endif

#if(PROBE_ISR1!=0)
    if(CHECKBIT(diff,PROBE_BIT))
    {
        io_probe_isr();
    }
#endif
}
#endif

#if(PCINT2_MASK!=0)
ISR(PCINT2_vect, ISR_BLOCK) // input pin on change service routine
{
    static uint8_t prev_value = 0;
    uint8_t value = PCINT2_INREG;
    uint8_t diff = prev_value ^ value;
    prev_value = value;

#if(PCINT2_LIMITS_MASK!=0)
    if(diff & PCINT2_LIMITS_MASK)
    {
        io_limits_isr();
    }
#endif
#if(PCINT2_CONTROLS_MASK!=0)
    if(diff & PCINT2_CONTROLS_MASK)
    {
        io_controls_isr();
    }
#endif

#if(PROBE_ISR2!=0)
    if(CHECKBIT(diff,PROBE_BIT))
    {
        io_probe_isr();
    }
#endif
}
#endif

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

    //configure all pins
    //autogenerated
#ifdef STEP0
    SETBIT(STEP0_DIRREG, STEP0_BIT);
#endif
#ifdef STEP1
    SETBIT(STEP1_DIRREG, STEP1_BIT);
#endif
#ifdef STEP2
    SETBIT(STEP2_DIRREG, STEP2_BIT);
#endif
#ifdef STEP3
    SETBIT(STEP3_DIRREG, STEP3_BIT);
#endif
#ifdef STEP4
    SETBIT(STEP4_DIRREG, STEP4_BIT);
#endif
#ifdef STEP5
    SETBIT(STEP5_DIRREG, STEP5_BIT);
#endif
#ifdef STEP6
    SETBIT(STEP6_DIRREG, STEP6_BIT);
#endif
#ifdef STEP7
    SETBIT(STEP7_DIRREG, STEP7_BIT);
#endif
#ifdef STEP0_EN
    SETBIT(STEP0_EN_DIRREG, STEP0_EN_BIT);
#endif
#ifdef STEP1_EN
    SETBIT(STEP1_EN_DIRREG, STEP1_EN_BIT);
#endif
#ifdef STEP2_EN
    SETBIT(STEP2_EN_DIRREG, STEP2_EN_BIT);
#endif
#ifdef STEP3_EN
    SETBIT(STEP3_EN_DIRREG, STEP3_EN_BIT);
#endif
#ifdef STEP4_EN
    SETBIT(STEP4_EN_DIRREG, STEP4_EN_BIT);
#endif
#ifdef STEP5_EN
    SETBIT(STEP5_EN_DIRREG, STEP5_EN_BIT);
#endif
#ifdef DIR0
    SETBIT(DIR0_DIRREG, DIR0_BIT);
#endif
#ifdef DIR1
    SETBIT(DIR1_DIRREG, DIR1_BIT);
#endif
#ifdef DIR2
    SETBIT(DIR2_DIRREG, DIR2_BIT);
#endif
#ifdef DIR3
    SETBIT(DIR3_DIRREG, DIR3_BIT);
#endif
#ifdef DIR4
    SETBIT(DIR4_DIRREG, DIR4_BIT);
#endif
#ifdef DIR5
    SETBIT(DIR5_DIRREG, DIR5_BIT);
#endif
#ifdef PWM0
    SETBIT(PWM0_DIRREG, PWM0_BIT);
    PWM0_TMRAREG = PWM0_MODE;
    PWM0_TMRBREG = PWM0_PRESCALLER;
    PWM0_OCRREG = 0;
#endif
#ifdef PWM1
    SETBIT(PWM1_DIRREG, PWM1_BIT);
    PWM1_TMRAREG = PWM1_MODE;
    PWM1_TMRBREG = PWM1_PRESCALLER;
    PWM1_OCRREG = 0;
#endif
#ifdef PWM2
    SETBIT(PWM2_DIRREG, PWM2_BIT);
    PWM2_TMRAREG = PWM2_MODE;
    PWM2_TMRBREG = PWM2_PRESCALLER;
    PWM2_OCRREG = 0;
#endif
#ifdef PWM3
    SETBIT(PWM3_DIRREG, PWM3_BIT);
    PWM3_TMRAREG = PWM3_MODE;
    PWM3_TMRBREG = PWM3_PRESCALLER;
    PWM3_OCRREG = 0;
#endif
#ifdef PWM4
    SETBIT(PWM4_DIRREG, PWM4_BIT);
    PWM4_TMRAREG = PWM4_MODE;
    PWM4_TMRBREG = PWM4_PRESCALLER;
    PWM4_OCRREG = 0;
#endif
#ifdef PWM5
    SETBIT(PWM5_DIRREG, PWM5_BIT);
    PWM5_TMRAREG = PWM5_MODE;
    PWM5_TMRBREG = PWM5_PRESCALLER;
    PWM5_OCRREG = 0;
#endif
#ifdef PWM6
    SETBIT(PWM6_DIRREG, PWM6_BIT);
    PWM6_TMRAREG = PWM6_MODE;
    PWM6_TMRBREG = PWM6_PRESCALLER;
    PWM6_OCRREG = 0;
#endif
#ifdef PWM7
    SETBIT(PWM7_DIRREG, PWM7_BIT);
    PWM7_TMRAREG = PWM7_MODE;
    PWM7_TMRBREG = PWM7_PRESCALLER;
    PWM7_OCRREG = 0;
#endif
#ifdef PWM8
    SETBIT(PWM8_DIRREG, PWM8_BIT);
    PWM8_TMRAREG = PWM8_MODE;
    PWM8_TMRBREG = PWM8_PRESCALLER;
    PWM8_OCRREG = 0;
#endif
#ifdef PWM9
    SETBIT(PWM9_DIRREG, PWM9_BIT);
    PWM9_TMRAREG = PWM9_MODE;
    PWM9_TMRBREG = PWM9_PRESCALLER;
    PWM9_OCRREG = 0;
#endif
#ifdef PWM10
    SETBIT(PWM10_DIRREG, PWM10_BIT);
    PWM10_TMRAREG = PWM10_MODE;
    PWM10_TMRBREG = PWM10_PRESCALLER;
    PWM10_OCRREG = 0;
#endif
#ifdef PWM11
    SETBIT(PWM11_DIRREG, PWM11_BIT);
    PWM11_TMRAREG = PWM11_MODE;
    PWM11_TMRBREG = PWM11_PRESCALLER;
    PWM11_OCRREG = 0;
#endif
#ifdef PWM12
    SETBIT(PWM12_DIRREG, PWM12_BIT);
    PWM12_TMRAREG = PWM12_MODE;
    PWM12_TMRBREG = PWM12_PRESCALLER;
    PWM12_OCRREG = 0;
#endif
#ifdef PWM13
    SETBIT(PWM13_DIRREG, PWM13_BIT);
    PWM13_TMRAREG = PWM13_MODE;
    PWM13_TMRBREG = PWM13_PRESCALLER;
    PWM13_OCRREG = 0;
#endif
#ifdef PWM14
    SETBIT(PWM14_DIRREG, PWM14_BIT);
    PWM14_TMRAREG = PWM14_MODE;
    PWM14_TMRBREG = PWM14_PRESCALLER;
    PWM14_OCRREG = 0;
#endif
#ifdef PWM15
    SETBIT(PWM15_DIRREG, PWM15_BIT);
    PWM15_TMRAREG = PWM15_MODE;
    PWM15_TMRBREG = PWM15_PRESCALLER;
    PWM15_OCRREG = 0;
#endif
#ifdef TX
    SETBIT(TX_DIRREG, TX_BIT);
#endif
#ifdef DOUT0
    SETBIT(DOUT0_DIRREG, DOUT0_BIT);
#endif
#ifdef DOUT1
    SETBIT(DOUT1_DIRREG, DOUT1_BIT);
#endif
#ifdef DOUT2
    SETBIT(DOUT2_DIRREG, DOUT2_BIT);
#endif
#ifdef DOUT3
    SETBIT(DOUT3_DIRREG, DOUT3_BIT);
#endif
#ifdef DOUT4
    SETBIT(DOUT4_DIRREG, DOUT4_BIT);
#endif
#ifdef DOUT5
    SETBIT(DOUT5_DIRREG, DOUT5_BIT);
#endif
#ifdef DOUT6
    SETBIT(DOUT6_DIRREG, DOUT6_BIT);
#endif
#ifdef DOUT7
    SETBIT(DOUT7_DIRREG, DOUT7_BIT);
#endif
#ifdef DOUT8
    SETBIT(DOUT8_DIRREG, DOUT8_BIT);
#endif
#ifdef DOUT9
    SETBIT(DOUT9_DIRREG, DOUT9_BIT);
#endif
#ifdef DOUT10
    SETBIT(DOUT10_DIRREG, DOUT10_BIT);
#endif
#ifdef DOUT11
    SETBIT(DOUT11_DIRREG, DOUT11_BIT);
#endif
#ifdef DOUT12
    SETBIT(DOUT12_DIRREG, DOUT12_BIT);
#endif
#ifdef DOUT13
    SETBIT(DOUT13_DIRREG, DOUT13_BIT);
#endif
#ifdef DOUT14
    SETBIT(DOUT14_DIRREG, DOUT14_BIT);
#endif
#ifdef DOUT15
    SETBIT(DOUT15_DIRREG, DOUT15_BIT);
#endif
#ifdef LIMIT_X
CLEARBIT(LIMIT_X_DIRREG, LIMIT_X_BIT);
#ifdef LIMIT_X_PULLUP
SETBIT(LIMIT_X_PORTREG, LIMIT_X_BIT);
#endif
#ifdef LIMIT_X_ISR
SETFLAG(LIMIT_X_ISRREG, LIMIT_X_ISR_MASK);
#endif
#endif
#ifdef LIMIT_Y
CLEARBIT(LIMIT_Y_DIRREG, LIMIT_Y_BIT);
#ifdef LIMIT_Y_PULLUP
SETBIT(LIMIT_Y_PORTREG, LIMIT_Y_BIT);
#endif
#ifdef LIMIT_Y_ISR
SETFLAG(LIMIT_Y_ISRREG, LIMIT_Y_ISR_MASK);
#endif
#endif
#ifdef LIMIT_Z
CLEARBIT(LIMIT_Z_DIRREG, LIMIT_Z_BIT);
#ifdef LIMIT_Z_PULLUP
SETBIT(LIMIT_Z_PORTREG, LIMIT_Z_BIT);
#endif
#ifdef LIMIT_Z_ISR
SETFLAG(LIMIT_Z_ISRREG, LIMIT_Z_ISR_MASK);
#endif
#endif
#ifdef LIMIT_X2
CLEARBIT(LIMIT_X2_DIRREG, LIMIT_X2_BIT);
#ifdef LIMIT_X2_PULLUP
SETBIT(LIMIT_X2_PORTREG, LIMIT_X2_BIT);
#endif
#ifdef LIMIT_X2_ISR
SETFLAG(LIMIT_X2_ISRREG, LIMIT_X2_ISR_MASK);
#endif
#endif
#ifdef LIMIT_Y2
CLEARBIT(LIMIT_Y2_DIRREG, LIMIT_Y2_BIT);
#ifdef LIMIT_Y2_PULLUP
SETBIT(LIMIT_Y2_PORTREG, LIMIT_Y2_BIT);
#endif
#ifdef LIMIT_Y2_ISR
SETFLAG(LIMIT_Y2_ISRREG, LIMIT_Y2_ISR_MASK);
#endif
#endif
#ifdef LIMIT_Z2
CLEARBIT(LIMIT_Z2_DIRREG, LIMIT_Z2_BIT);
#ifdef LIMIT_Z2_PULLUP
SETBIT(LIMIT_Z2_PORTREG, LIMIT_Z2_BIT);
#endif
#ifdef LIMIT_Z2_ISR
SETFLAG(LIMIT_Z2_ISRREG, LIMIT_Z2_ISR_MASK);
#endif
#endif
#ifdef LIMIT_A
CLEARBIT(LIMIT_A_DIRREG, LIMIT_A_BIT);
#ifdef LIMIT_A_PULLUP
SETBIT(LIMIT_A_PORTREG, LIMIT_A_BIT);
#endif
#ifdef LIMIT_A_ISR
SETFLAG(LIMIT_A_ISRREG, LIMIT_A_ISR_MASK);
#endif
#endif
#ifdef LIMIT_B
CLEARBIT(LIMIT_B_DIRREG, LIMIT_B_BIT);
#ifdef LIMIT_B_PULLUP
SETBIT(LIMIT_B_PORTREG, LIMIT_B_BIT);
#endif
#ifdef LIMIT_B_ISR
SETFLAG(LIMIT_B_ISRREG, LIMIT_B_ISR_MASK);
#endif
#endif
#ifdef LIMIT_C
CLEARBIT(LIMIT_C_DIRREG, LIMIT_C_BIT);
#ifdef LIMIT_C_PULLUP
SETBIT(LIMIT_C_PORTREG, LIMIT_C_BIT);
#endif
#ifdef LIMIT_C_ISR
SETFLAG(LIMIT_C_ISRREG, LIMIT_C_ISR_MASK);
#endif
#endif
#ifdef PROBE
CLEARBIT(PROBE_DIRREG, PROBE_BIT);
#ifdef PROBE_PULLUP
SETBIT(PROBE_PORTREG, PROBE_BIT);
#endif
#ifdef PROBE_ISR
SETFLAG(PROBE_ISRREG, PROBE_ISR_MASK);
#endif
#endif
#ifdef ESTOP
CLEARBIT(ESTOP_DIRREG, ESTOP_BIT);
#ifdef ESTOP_PULLUP
SETBIT(ESTOP_PORTREG, ESTOP_BIT);
#endif
#ifdef ESTOP_ISR
SETFLAG(ESTOP_ISRREG, ESTOP_ISR_MASK);
#endif
#endif
#ifdef SAFETY_DOOR
CLEARBIT(SAFETY_DOOR_DIRREG, SAFETY_DOOR_BIT);
#ifdef SAFETY_DOOR_PULLUP
SETBIT(SAFETY_DOOR_PORTREG, SAFETY_DOOR_BIT);
#endif
#ifdef SAFETY_DOOR_ISR
SETFLAG(SAFETY_DOOR_ISRREG, SAFETY_DOOR_ISR_MASK);
#endif
#endif
#ifdef FHOLD
CLEARBIT(FHOLD_DIRREG, FHOLD_BIT);
#ifdef FHOLD_PULLUP
SETBIT(FHOLD_PORTREG, FHOLD_BIT);
#endif
#ifdef FHOLD_ISR
SETFLAG(FHOLD_ISRREG, FHOLD_ISR_MASK);
#endif
#endif
#ifdef CS_RES
CLEARBIT(CS_RES_DIRREG, CS_RES_BIT);
#ifdef CS_RES_PULLUP
SETBIT(CS_RES_PORTREG, CS_RES_BIT);
#endif
#ifdef CS_RES_ISR
SETFLAG(CS_RES_ISRREG, CS_RES_ISR_MASK);
#endif
#endif

#ifdef ANALOG0
    CLEARBIT(ANALOG0_DIRREG, ANALOG0_BIT);
#endif
#ifdef ANALOG1
    CLEARBIT(ANALOG1_DIRREG, ANALOG1_BIT);
#endif
#ifdef ANALOG2
    CLEARBIT(ANALOG2_DIRREG, ANALOG2_BIT);
#endif
#ifdef ANALOG3
    CLEARBIT(ANALOG3_DIRREG, ANALOG3_BIT);
#endif
#ifdef ANALOG4
    CLEARBIT(ANALOG4_DIRREG, ANALOG4_BIT);
#endif
#ifdef ANALOG5
    CLEARBIT(ANALOG5_DIRREG, ANALOG5_BIT);
#endif
#ifdef ANALOG6
    CLEARBIT(ANALOG6_DIRREG, ANALOG6_BIT);
#endif
#ifdef ANALOG7
    CLEARBIT(ANALOG7_DIRREG, ANALOG7_BIT);
#endif
#ifdef ANALOG8
    CLEARBIT(ANALOG8_DIRREG, ANALOG8_BIT);
#endif
#ifdef ANALOG9
    CLEARBIT(ANALOG9_DIRREG, ANALOG9_BIT);
#endif
#ifdef ANALOG10
    CLEARBIT(ANALOG10_DIRREG, ANALOG10_BIT);
#endif
#ifdef ANALOG11
    CLEARBIT(ANALOG11_DIRREG, ANALOG11_BIT);
#endif
#ifdef ANALOG12
    CLEARBIT(ANALOG12_DIRREG, ANALOG12_BIT);
#endif
#ifdef ANALOG13
    CLEARBIT(ANALOG13_DIRREG, ANALOG13_BIT);
#endif
#ifdef ANALOG14
    CLEARBIT(ANALOG14_DIRREG, ANALOG14_BIT);
#endif
#ifdef ANALOG15
    CLEARBIT(ANALOG15_DIRREG, ANALOG15_BIT);
#endif
#ifdef RX
    CLEARBIT(RX_DIRREG, RX_BIT);
#endif
#ifdef DIN0
    CLEARBIT(DIN0_DIRREG, DIN0_BIT);
#ifdef DIN0_PULLUP
    SETBIT(DIN0_PORTREG, DIN0_BIT);
#endif
#endif
#ifdef DIN1
    CLEARBIT(DIN1_DIRREG, DIN1_BIT);
#ifdef DIN1_PULLUP
    SETBIT(DIN1_PORTREG, DIN1_BIT);
#endif
#endif
#ifdef DIN2
    CLEARBIT(DIN2_DIRREG, DIN2_BIT);
#ifdef DIN2_PULLUP
    SETBIT(DIN2_PORTREG, DIN2_BIT);
#endif
#endif
#ifdef DIN3
    CLEARBIT(DIN3_DIRREG, DIN3_BIT);
#ifdef DIN3_PULLUP
    SETBIT(DIN3_PORTREG, DIN3_BIT);
#endif
#endif
#ifdef DIN4
    CLEARBIT(DIN4_DIRREG, DIN4_BIT);
#ifdef DIN4_PULLUP
    SETBIT(DIN4_PORTREG, DIN4_BIT);
#endif
#endif
#ifdef DIN5
    CLEARBIT(DIN5_DIRREG, DIN5_BIT);
#ifdef DIN5_PULLUP
    SETBIT(DIN5_PORTREG, DIN5_BIT);
#endif
#endif
#ifdef DIN6
    CLEARBIT(DIN6_DIRREG, DIN6_BIT);
#ifdef DIN6_PULLUP
    SETBIT(DIN6_PORTREG, DIN6_BIT);
#endif
#endif
#ifdef DIN7
    CLEARBIT(DIN7_DIRREG, DIN7_BIT);
#ifdef DIN7_PULLUP
    SETBIT(DIN7_PORTREG, DIN7_BIT);
#endif
#endif
#ifdef DIN8
    CLEARBIT(DIN8_DIRREG, DIN8_BIT);
#ifdef DIN8_PULLUP
    SETBIT(DIN8_PORTREG, DIN8_BIT);
#endif
#endif
#ifdef DIN9
    CLEARBIT(DIN9_DIRREG, DIN9_BIT);
#ifdef DIN9_PULLUP
    SETBIT(DIN9_PORTREG, DIN9_BIT);
#endif
#endif
#ifdef DIN10
    CLEARBIT(DIN10_DIRREG, DIN10_BIT);
#ifdef DIN10_PULLUP
    SETBIT(DIN10_PORTREG, DIN10_BIT);
#endif
#endif
#ifdef DIN11
    CLEARBIT(DIN11_DIRREG, DIN11_BIT);
#ifdef DIN11_PULLUP
    SETBIT(DIN11_PORTREG, DIN11_BIT);
#endif
#endif
#ifdef DIN12
    CLEARBIT(DIN12_DIRREG, DIN12_BIT);
#ifdef DIN12_PULLUP
    SETBIT(DIN12_PORTREG, DIN12_BIT);
#endif
#endif
#ifdef DIN13
    CLEARBIT(DIN13_DIRREG, DIN13_BIT);
#ifdef DIN13_PULLUP
    SETBIT(DIN13_PORTREG, DIN13_BIT);
#endif
#endif
#ifdef DIN14
    CLEARBIT(DIN14_DIRREG, DIN14_BIT);
#ifdef DIN14_PULLUP
    SETBIT(DIN14_PORTREG, DIN14_BIT);
#endif
#endif
#ifdef DIN15
    CLEARBIT(DIN15_DIRREG, DIN15_BIT);
#ifdef DIN15_PULLUP
    SETBIT(DIN15_PORTREG, DIN15_BIT);
#endif
#endif

    //Set COM port
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

    //enable interrupts on pin changes
    #if((PCINT0_LIMITS_MASK | PCINT0_CONTROLS_MASK | PROBE_ISR0) != 0)
    SETBIT(PCICR, PCIE0);
    #else
    CLEARBIT(PCICR, PCIE0);
    #endif
    #if((PCINT1_LIMITS_MASK | PCINT1_CONTROLS_MASK | PROBE_ISR1) != 0)
    SETBIT(PCICR, PCIE1);
    #else
    CLEARBIT(PCICR, PCIE1);
    #endif
    #if((PCINT2_LIMITS_MASK | PCINT2_CONTROLS_MASK | PROBE_ISR2) != 0)
    SETBIT(PCICR, PCIE2);
    #else
    CLEARBIT(PCICR, PCIE2);
    #endif

    //enable interrupts
    sei();
}

//IO functions
/*
#ifdef PROBE
void mcu_enable_probe_isr()
{
#ifdef PROBE_ISRREG
    SETBIT(PROBE_ISRREG, PROBE_BIT);
#endif
}

void mcu_disable_probe_isr()
{
#ifdef PROBE_ISRREG
    CLEARBIT(PROBE_ISRREG, PROBE_BIT);
#endif
}
#endif*/

uint8_t mcu_get_analog(uint8_t channel)
{
    ADMUX = (0x42 | channel); //VRef = Vcc with reading left aligned
    ADCSRA = 0xC7; //Start read with ADC with 128 prescaller
    do
    {
    }
    while(CHECKBIT(ADCSRA,ADSC));
    uint8_t result = ADCH;
    ADCSRA = 0; //switch adc off
    ADMUX = 0; //switch adc off

    return result;
}

void mcu_enable_interrupts()
{
    sei();
}
void mcu_disable_interrupts()
{
    cli();
}

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
    do
    {

    }
    while(EECR & (1<<EEPE));   // Wait for completion of previous write.
    EEAR = address; // Set EEPROM address register.
    EECR = (1<<EERE); // Start EEPROM read operation.
    return EEDR; // Return the byte read from EEPROM.
}

void mcu_eeprom_putc(uint16_t address, uint8_t value)
{
    uint8_t old_value; // Old EEPROM value.
    uint8_t diff_mask; // Difference mask, i.e. old value XOR new value.

    cli(); // Ensure atomic operation for the write operation.

    do
    {
    }
    while(EECR & (1<<EEPE));   // Wait for completion of previous write.

    do
    {
    }
    while(SPMCSR & (1<<SELFPRGEN));   // Wait for completion of SPM.

    EEAR = address; // Set EEPROM address register.
    EECR = (1<<EERE); // Start EEPROM read operation.
    old_value = EEDR; // Get old EEPROM value.
    diff_mask = old_value ^ value; // Get bit differences.
    // Check if any bits are changed to '1' in the new value.
    if(diff_mask & value)
    {
        // Now we know that _some_ bits need to be erased to '1'.
        // Check if any bits in the new value are '0'.
        if( value != 0xff )
        {
            // Now we know that some bits need to be programmed to '0' also.
            EEDR = value; // Set EEPROM data register.
            EECR = ((1<<EEMPE) | (0<<EEPM1) | (0<<EEPM0)); //Erase+Write mode.
            EECR |= (1<<EEPE);  // Start Erase+Write operation.
        }
        else
        {
            // Now we know that all bits should be erased.
            EECR = ((1<<EEMPE) | (1<<EEPM0));  // Erase-only mode.
            EECR |= (1<<EEPE);  // Start Erase-only operation.
        }
    }
    else
    {
        // Now we know that _no_ bits need to be erased to '1'.
        // Check if any bits are changed from '1' in the old value.
        if(diff_mask)
        {
            // Now we know that _some_ bits need to the programmed to '0'.
            EEDR = value;   // Set EEPROM data register.
            EECR = ((1<<EEMPE) | (1<<EEPM1));  // Write-only mode.
            EECR |= (1<<EEPE);  // Start Write-only operation.
        }
    }

    do
    {
    }
    while(EECR & (1<<EEPE));   // Wait for completion of previous write before enabling interrupts.

    sei(); // Restore interrupt flag state.
}
