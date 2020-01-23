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
#include "../../../config.h"
#if(MCU == MCU_ATMEGA328P)
#include "../../../mcudefs.h"
#include "../../../mcumap.h"
#include "../../../mcu.h"
#include "../../../utils.h"
#include "../../../serial.h"
#include "../../../interpolator.h"
#include "../../../io_control.h"

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
#include <avr/delay.h>
#include <avr/eeprom.h>

#define PORTMASK (OUTPUT_INVERT_MASK|INPUT_PULLUP_MASK)
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#ifndef BAUD
#define BAUD 115200
#endif

#ifndef COM_BUFFER_SIZE
#define COM_BUFFER_SIZE 50
#endif

#define PULSE_RESET_DELAY MIN_PULSE_WIDTH_US * F_CPU / 1000000

//USART communication
/*int mcu_putchar(char c, FILE* stream);
FILE g_mcu_streamout = FDEV_SETUP_STREAM(mcu_putchar, NULL, _FDEV_SETUP_WRITE);*/

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

ISR(TIMER1_COMPA_vect, ISR_BLOCK)
{
	#ifdef __PERFSTATS__
	uint16_t clocks = TCNT1;
	#endif
	itp_step_reset_isr();
	
	#ifdef __PERFSTATS__
    uint16_t clocks2 = TCNT1;
    clocks2 -= clocks;
	mcu_perf_step_reset = MAX(mcu_perf_step_reset, clocks2);
	#endif
}

ISR(TIMER1_COMPB_vect, ISR_BLOCK)
{
	#ifdef __PERFSTATS__
	uint16_t clocks = TCNT1;
	#endif
    itp_step_isr();
    #ifdef __PERFSTATS__
    uint16_t clocks2 = TCNT1;
    clocks2 -= clocks;
	mcu_perf_step = MAX(mcu_perf_step, clocks2);
	#endif
}

ISR(PCINT0_vect, ISR_BLOCK) // input pin on change service routine
{
	static uint8_t prev_value = 0;
	uint8_t value = PINB;
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
	uint8_t value = PINC;
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
	uint8_t value = PIND;
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


ISR(USART_RX_vect, ISR_BLOCK)
{
	unsigned char c = UDR0;
	serial_rx_isr(c);
}

ISR(USART_UDRE_vect, ISR_BLOCK)
{
	if(serial_tx_is_empty())
	{
		UCSR0B &= ~(1<<UDRIE0);
		return;
	}
	
	UDR0 = serial_tx_isr();
}

void mcu_init()
{
    //IO_REGISTER reg = {};
    
    #ifdef __PERFSTATS__
	mcu_perf_step = 0;
	mcu_perf_step_reset = 0;
	#endif
	
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
	//TCCRXA Mode 3 - Fast PWM 
	//TCCRXB Prescaller 1/64
	#ifdef PWM0
		PWM0_DIRREG |= PWM0_MASK;
		PWM0_TMRAREG = 0x03; //Mode 3
		PWM0_TMRBREG = PWM0_PRESCMASK; //Prescaller 1/64
		PWM0_CNTREG = 0;
	#endif
	#ifdef PWM1
		PWM1_DIRREG |= PWM1_MASK;
		PWM1_TMRAREG = 0x03; //Mode 3
		PWM1_TMRBREG = PWM1_PRESCMASK; //Prescaller 1/64
		PWM1_CNTREG = 0;
	#endif
	#ifdef PWM2
		PWM2_DIRREG |= PWM2_MASK;
		PWM2_TMRAREG = 0x03; //Mode 3
		PWM2_TMRBREG = PWM2_PRESCMASK; //Prescaller 1/64
		PWM2_CNTREG = 0;
	#endif
	#ifdef PWM3
		PWM3_DIRREG |= PWM3_MASK;
		PWM3_TMRAREG = 0x03; //Mode 3
		PWM3_TMRBREG = PWM3_PRESCMASK; //Prescaller 1/64
		PWM3_CNTREG = 0;
	#endif

    // Set baud rate
    #if BAUD < 57600
      uint16_t UBRR0_value = ((F_CPU / (8L * BAUD)) - 1)/2 ;
      UCSR0A &= ~(1 << U2X0); // baud doubler off  - Only needed on Uno XXX
    #else
      uint16_t UBRR0_value = ((F_CPU / (4L * BAUD)) - 1)/2;
      UCSR0A |= (1 << U2X0);  // baud doubler on for high baud rates, i.e. 115200
    #endif
    UBRR0H = UBRR0_value >> 8;
    UBRR0L = UBRR0_value;
  
    // enable rx, tx, and interrupt on complete reception of a byte and UDR empty
    UCSR0B |= (1<<RXEN0 | 1<<TXEN0 | 1<<RXCIE0);
    
	//enable interrupts
	sei();
}

//IO functions    
//Inputs  
/*uint32_t mcu_get_inputs()
{
	uint32_t result;
	uint8_t* reg = &result;
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

uint8_t mcu_get_controls()
{
	return (CONTROLS_INREG & CONTROLS_MASK);
}

uint8_t mcu_get_limits()
{
	return (LIMITS_INREG & LIMITS_MASK);
}*/

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

//outputs
/*
//sets all step pins
void mcu_set_steps(uint8_t value)
{
	STEPS_OUTREG = (~STEPS_MASK & STEPS_OUTREG) | value;
}
//sets all dir pins
void mcu_set_dirs(uint8_t value)
{
	DIRS_OUTREG = (~DIRS_MASK & DIRS_OUTREG) | value;
}

void mcu_set_outputs(uint32_t value)
{
	uint8_t* reg = &value;
	
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

uint32_t mcu_get_outputs()
{
	uint32_t result;
	uint8_t* reg = &result;

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
}*/

void mcu_set_pwm(uint8_t pwm, uint8_t value)
{
	switch(pwm)
	{
		case 0:
			#ifdef PWM0
			PWM0_CNTREG = value;
			if(value != 0)
			{
				#if(PWM0_OCREG==A)
					SETFLAG(PWM0_TMRAREG,0x80);
				#elif(PWM0_OCREG==B)
					SETFLAG(PWM0_TMRAREG,0x20);
				#endif
			}
			else
			{
				#if(PWM0_OCREG==A)
					CLEARFLAG(PWM0_TMRAREG,0x80);
				#elif(PWM0_OCREG==B)
					CLEARFLAG(PWM0_TMRAREG,0x20);
				#endif
			}
			#endif
			break;
		case 1:
			#ifdef PWM1
			PWM1_CNTREG = value;
			if(value != 0)
			{
				#if(PWM1_OCREG==A)
					SETFLAG(PWM1_TMRAREG,0x80);
				#elif(PWM1_OCREG==B)
					SETFLAG(PWM1_TMRAREG,0x20);
				#endif
			}
			else
			{
				#if(PWM1_OCREG==A)
					CLEARFLAG(PWM1_TMRAREG,0x80);
				#elif(PWM1_OCREG==B)
					CLEARFLAG(PWM1_TMRAREG,0x20);
				#endif
			}
			#endif
			break;
		case 2:
			#ifdef PWM2
			PWM2_CNTREG = value;
			if(value != 0)
			{
				#if(PWM2_OCREG==A)
					SETFLAG(PWM2_TMRAREG,0x80);
				#elif(PWM2_OCREG==B)
					SETFLAG(PWM2_TMRAREG,0x20);
				#endif
			}
			else
			{
				#if(PWM2_OCREG==A)
					CLEARFLAG(PWM2_TMRAREG,0x80);
				#elif(PWM2_OCREG==B)
					CLEARFLAG(PWM2_TMRAREG,0x20);
				#endif
			}
			#endif
			break;
		case 3:
			#ifdef PWM3
			PWM3_CNTREG = value;
			if(value != 0)
			{
				#if(PWM3_OCREG==A)
					SETFLAG(PWM3_TMRAREG,0x80);
				#elif(PWM3_OCREG==B)
					SETFLAG(PWM3_TMRAREG,0x20);
				#endif
			}
			else
			{
				#if(PWM3_OCREG==A)
					CLEARFLAG(PWM3_TMRAREG,0x80);
				#elif(PWM3_OCREG==B)
					CLEARFLAG(PWM3_TMRAREG,0x20);
				#endif
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
	SETBIT(UCSR0B,UDRIE0);
}

void mcu_putc(char c)
{
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
}

bool mcu_is_tx_ready()
{
	return CHECKBIT(UCSR0A, UDRE0);
}

char mcu_getc()
{
	loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
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
	TCCR1B = 0;
	//CTC mode
    TCCR1A = 0;
    //resets counter
    TCNT1 = 0;
    //set step clock
    OCR1A = clocks_speed;
	//sets OCR0B to half
	//this will allways fire step_reset between pulses
    OCR1B = OCR1A>>1;
	TIFR1 = 0;
	// enable timer interrupts on both match registers
    TIMSK1 |= (1 << OCIE1B) | (1 << OCIE1A);
    
    //start timer in CTC mode with the correct prescaler
    TCCR1B = prescaller;
}

// se implementar amass deixo de necessitar de prescaler
void mcu_change_step_ISR(uint16_t clocks_speed, uint8_t prescaller)
{
	//stops timer
	//TCCR1B = 0;
	OCR1B = clocks_speed>>1;
	OCR1A = clocks_speed;
	//sets OCR0B to half
	//this will allways fire step_reset between pulses
    
	//reset timer
    //TCNT1 = 0;
	//start timer in CTC mode with the correct prescaler
    TCCR1B = prescaller;
}

void mcu_step_stop_ISR()
{
	TCCR1B = 0;
    TIMSK1 &= ~((1 << OCIE1B) | (1 << OCIE1A));
}

/*#define MCU_1MS_LOOP F_CPU/1000000
static __attribute__((always_inline)) void mcu_delay_1ms() 
{
	uint16_t loop = MCU_1MS_LOOP;
	do{
	}while(--loop);
}*/

void mcu_delay_ms(uint16_t miliseconds)
{
	do{
		_delay_ms(1);
	}while(--miliseconds);
	
}

#ifndef EEPE
		#define EEPE  EEWE  //!< EEPROM program/write enable.
		#define EEMPE EEMWE //!< EEPROM master program/write enable.
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

//taken from grbl
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

#endif
