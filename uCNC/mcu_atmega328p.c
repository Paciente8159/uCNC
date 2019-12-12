#include "config.h"

#if(MCU == MCU_ATMEGA328P)
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

#include "mcudefs.h"
#include "mcumap.h"
#include "mcu.h"
#include "utils.h"
#include "protocol.h"

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

typedef union{
    uint32_t r; // occupies 4 bytes
#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    struct
    {
        uint16_t rl;
        uint16_t rh;
    };
    struct
    {
        uint8_t r0;
        uint8_t r1;
        uint8_t r2;
        uint8_t r3;
    };
#else
    struct
    {
        uint16_t rh;
        uint16_t rl;
    };
    struct
    {
        uint8_t r3;
        uint8_t r2;
        uint8_t r1;
        uint8_t r0;
    }
    ;
#endif
} IO_REGISTER;

//USART communication
char *mcu_tx_buffer;
volatile bool mcu_tx_ready;
int mcu_putchar(char c, FILE* stream);
FILE g_mcu_streamout = FDEV_SETUP_STREAM(mcu_putchar, NULL, _FDEV_SETUP_WRITE);

//IO Registers
//IO_REGISTER g_mcu_dirRegister;
volatile IO_REGISTER g_mcu_inputs;
//volatile INPUT_REGISTER g_mcu_inputs;
//volatile OUTPUT_REGISTER g_mcu_outputs;
volatile IO_REGISTER g_mcu_pouts;

ISRVOID g_mcu_stepCallback;
ISRVOID g_mcu_stepResetCallback;
//ISRTIMER g_mcu_integratorCallback;
volatile uint16_t g_mcu_tmr0_counter;
volatile uint16_t g_mcu_tmr0_value;

ISRCOMRX g_mcu_rs232RxCallback;
ISRVOID g_mcu_rs232TxCallback;

ISRPORTCHANGE mcu_limit_trigger_callback;
static uint8_t mcu_prev_limits;
ISRPORTCHANGE mcu_control_trigger_callback;
uint8_t mcu_prev_controls;

ISR(TIMER1_COMPA_vect, ISR_NOBLOCK)
{
    if(g_mcu_stepCallback!=NULL)
	{
    	g_mcu_stepCallback();
	}
}

ISR(TIMER1_COMPB_vect, ISR_NOBLOCK)
{
    if(g_mcu_stepResetCallback!=NULL)
	{
    	g_mcu_stepResetCallback();
	}
}

/*
	Fazer modificação
	ISR apenas para limites e controls
	criar static e comparar com valor anterior e so disparar callback caso tenha alterado
*/

#ifdef PORTISR0
	ISR(PCINT0_vect, ISR_NOBLOCK) // input pin on change service routine
	{
		#ifdef LIMITS_INREG
	    if(mcu_limit_trigger_callback != NULL && (LIMITS_INREG == PORTRD0))
	    {
	    	mcu_limit_trigger_callback((LIMITS_INREG & LIMITS_MASK));
		}
		#endif		
	}
#endif

#ifdef PORTISR1
	ISR(PCINT1_vect, ISR_NOBLOCK) // input pin on change service routine
	{
	    #ifdef LIMITS_INREG
	    if(mcu_limit_trigger_callback != NULL && (LIMITS_INREG == PORTRD1))
	    {
	    	mcu_limit_trigger_callback((LIMITS_INREG & LIMITS_MASK));
		}
		#endif
	}
#endif

#ifdef PORTISR2
	ISR(PCINT2_vect, ISR_NOBLOCK) // input pin on change service routine
	{
	    #ifdef LIMITS_INREG
	    if(mcu_limit_trigger_callback != NULL && (LIMITS_INREG == PORTRD2))
	    {
	    	mcu_limit_trigger_callback((LIMITS_INREG & LIMITS_MASK));
		}
		#endif
	}
#endif

#ifdef PORTISR3
	ISR(PCINT3_vect, ISR_NOBLOCK) // input pin on change service routine
	{
	    #ifdef LIMITS_INREG
	    if(mcu_limit_trigger_callback != NULL && (LIMITS_INREG == PORTRD3))
	    {
	    	mcu_limit_trigger_callback((LIMITS_INREG & LIMITS_MASK));
		}
		#endif
	}
#endif

#ifdef RX
	ISR(USART_RX_vect, ISR_BLOCK)
	{
        if(g_mcu_rs232RxCallback!=NULL)
		{
	    	g_mcu_rs232RxCallback(UDR0);
		}
	}
#endif

#ifdef TX
	ISR(USART_UDRE_vect, ISR_BLOCK)
	{
		static bool eol = false;
		char c = *mcu_tx_buffer++;
		
		if(!eol)
		{
			UDR0 = c;
			if(c=='\n')
			{
				eol = true;
			}
		}
		else
		{
			eol = false;
			mcu_tx_ready = true;
			UCSR0B &= ~(1<<UDRIE0);
		}
	}
#endif

void mcu_init()
{
    IO_REGISTER reg = {};
    
    g_mcu_stepCallback = NULL;
	g_mcu_stepResetCallback = NULL;
	mcu_limit_trigger_callback = NULL;
	g_mcu_rs232RxCallback = NULL;
	g_mcu_rs232TxCallback = NULL;
	
	//disable WDT
	wdt_reset();
    MCUSR &= ~(1<<WDRF);
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	WDTCSR = 0x00;
    //g_mcu_dirRegister.r = OUTPUT_PINS;
    
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
    
    #ifdef DINS_LOW_DIRREG
        DINS_LOW_DIRREG = 0;
    #endif
    
    #ifdef DINS_HIGH_DIRREG
        DINS_HIGH_DIRREG = 0;
    #endif
    
    //pull-ups
    #ifdef CONTROLS_PULLUPREG
        CONTROLS_PULLUPREG = CONTROLS_PULLUP_MASK;
    #endif
    
	#ifdef LIMITS_PULLUPREG
        LIMITS_PULLUPREG = LIMITS_PULLUP_MASK;
    #endif
    
    #ifdef PROBE_PULLUPREG
        PROBE_PULLUPREG = PROBE_PULLUP_MASK;
    #endif

    #ifdef DINS_LOW_PULLUPREG
        DINS_LOW_PULLUPREG = DINS_LOW_PULLUP_MASK;
    #endif
    
    #ifdef DINS_HIGH_PULLUPREG
        DINS_HIGH_PULLUPREG = DINS_HIGH_PULLUP_MASK;
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
    
	#ifdef DOUTS_LOW_DIRREG
        DOUTS_LOW_DIRREG |= DOUTS_LOW_MASK;
    #endif
    
    #ifdef DOUTS_HIGH_DIRREG
        DOUTS_HIGH_DIRREG |= DOUTS_HIGH_MASK;
    #endif
    
    //initializes input internal register
    g_mcu_inputs.r = 0;
    #ifdef PORTRD0
        g_mcu_inputs.r0 = PORTRD0;
    #endif
    #ifdef PORTRD1
        g_mcu_inputs.r1 = PORTRD1;
    #endif
    #ifdef PORTRD2
        g_mcu_inputs.r2 = PORTRD2;
    #endif
    #ifdef PORTRD3
        g_mcu_inputs.r3 = PORTRD3;
    #endif
    
    //initializes interrupts on all input pins
    //first read direction registers on all ports (inputs are set to 0)
    reg.r = 0;
    #ifdef PORTRD0
        reg.r0 = PORTDIR0;
    #endif
    #ifdef PORTRD1
        reg.r1 = PORTDIR1;
    #endif
    #ifdef PORTRD2
        reg.r2 = PORTDIR2;
    #endif
    #ifdef PORTRD3
        reg.r3 = PORTDIR3;
    #endif
    
      /*  
    //invers values to apply interrupt masks
    reg.r = ~reg.r;
    #ifdef PCMSK0
    	PCMSK0 = reg.r0;
    #endif
	#ifdef PCMSK1
    	PCMSK1 = reg.r1;
    #endif
    #ifdef PCMSK2
    	PCMSK2 = reg.r2;
    #endif
    #ifdef PCMSK3
    	PCMSK3 = reg.r3;
    #endif
    //enable interrupts on all inputs
    PCICR = 0xFF;*/
    
    //set serial port
    /*g_mcu_rxhead = 0;
    g_mcu_rxtail = 0;
    g_mcu_rxcount = 0;*/
    
    stdout = &g_mcu_streamout;
    mcu_tx_ready = true;
    /*#ifdef __DEBUG__
    stdin = &g_mcu_streamin;
    #endif
	*/

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
	
	/*#ifdef __PROF__
	mcu_startTickCounter();
	g_mcu_perfCounterOffset = 0;
    //calculate performance offset
    //uint32_t tickcount = mcu_getCycles();
    
    uint32_t tickcount = mcu_getCycles();
    g_mcu_perfCounterOffset = (uint8_t)mcu_getElapsedCycles(tickcount);
    //g_mcu_perfCounterOffset = mcu_stopPerfCounter();
    #endif*/
}

//IO functions    
//Inputs  
uint16_t mcu_getInputs()
{
	IO_REGISTER reg;
	reg.r = 0;
	#ifdef DINS_LOW
	reg.r0 = (DINS_LOW & DINS_LOW_MASK);
	#endif
	#ifdef DINS_HIGH
	reg.r1 = (DINS_HIGH & DINS_HIGH_MASK);
	#endif
	return reg.rl;	
}

uint8_t mcu_getControls()
{
	return (CONTROLS_INREG & CONTROLS_MASK);
}

uint8_t mcu_getLimits()
{
	return (LIMITS_INREG & LIMITS_MASK);
}

uint8_t mcu_getProbe()
{
	return (PROBE_INREG & PROBE_MASK);
}

//outputs

//sets all step pins
void mcu_setSteps(uint8_t value)
{
	STEPS_OUTREG = (~STEPS_MASK & STEPS_OUTREG) | value;
}
//sets all dir pins
void mcu_setDirs(uint8_t value)
{
	DIRS_OUTREG = (~DIRS_MASK & DIRS_OUTREG) | value;
}

void mcu_setOutputs(uint16_t value)
{
	IO_REGISTER reg = {};
	reg.rl = value;
	
	#ifdef DOUTS_LOW_OUTREG
		DOUTS_LOW_OUTREG = (~DOUTS_LOW_MASK & DOUTS_LOW_OUTREG) | reg.r0;
	#endif
	#ifdef DOUTS_HIGH_OUTREG
		DOUTS_HIGH_OUTREG = (~DOUTS_HIGH_MASK & DOUTS_HIGH_OUTREG) | reg.r1;
	#endif
}

void mcu_enableInterrupts()
{
	sei();
}
void mcu_disableInterrupts()
{
	cli();
}

void mcu_attachOnLimitTrigger(ISRPORTCHANGE handler)
{
	mcu_limit_trigger_callback = handler;
}

//internal redirect of stdout
int mcu_putchar(char c, FILE* stream)
{
	mcu_putc(c);
	return c;
}

void mcu_putc(char c)
{
	while(!mcu_tx_ready);
	mcu_tx_ready = false;
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	mcu_tx_ready = true;
    
}

void mcu_puts(const char* __str)
{
	while(!mcu_tx_ready);
	mcu_tx_ready = false;
	mcu_tx_buffer = __str;
	UCSR0B |= (1<<UDRIE0);
}

bool mcu_is_txready()
{
	return mcu_tx_ready;
}

char mcu_getc()
{
	loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}

void mcu_attachOnReadChar(ISRCOMRX handler)
{
    g_mcu_rs232RxCallback = handler;
}

void mcu_detachOnReadChar()
{
    g_mcu_rs232RxCallback = NULL;
}

#ifdef __PROF__
/*ISR(TIMER2_OVF_vect)
{
	g_mcu_perfOvfCounter++;
}

//will count up to 16777215 (24 bits)
uint32_t mcu_getCycles()
{
	uint8_t ticks = (uint8_t)TCNT2;
    uint16_t highticks = (uint16_t)g_mcu_perfOvfCounter;
    uint32_t result = highticks;
	result <<= 8;
	if(ticks != 255)
    	result |= ticks;
    
    return result;
}

uint32_t mcu_getElapsedCycles(uint32_t cycle_ref)
{
	uint32_t result = mcu_getCycles();
	result -= g_mcu_perfCounterOffset;
	if(result < cycle_ref)
		result += 1677216;
	return result - cycle_ref;
}

void mcu_startTickCounter()
{
    TCCR2A = 0;
    TCCR2B = 0;
    TCNT2 = 0;  //initialize counter value to 0
    TIFR2 = 0;
    TIMSK2 |= (1 << TOIE2);
    g_mcu_perfOvfCounter = 0;
    TCCR2B = 1;
}
*/
/*

void mcu_startPerfCounter()
{
    TCCR2A = 0;
    TCCR2B = 0;
    TCNT2 = 0;  //initialize counter value to 0
    TIFR2 = 0;
    TIMSK2 |= (1 << TOIE2);
    g_mcu_perfOvfCounter = 0;
    TCCR2B = 1;
}

uint16_t mcu_stopPerfCounter()
{
    uint8_t ticks = TCNT2;
    TCCR2B = 0;
    TIMSK2 &= ~(1 << TOIE2);
    uint16_t res = g_mcu_perfOvfCounter;
    res *= 256;
    res += ticks;
    return res;
}
*/
#endif

//RealTime
void mcu_freq2clocks(float frequency, uint16_t* ticks, uint8_t* prescaller)
{
	if(frequency < F_PULSE_MIN)
		frequency = F_PULSE_MIN;
	if(frequency > F_PULSE_MAX)
		frequency = F_PULSE_MAX;
		
	float clockcounter = F_CPU;
		
	if(frequency >= 245)
	{
		*prescaller = 9;
		
	}
	else if(frequency >= 31)
	{
		*prescaller = 10;
		clockcounter *= 0.125f;
	}
	else if(frequency >= 4)
	{
		*prescaller = 11;
		clockcounter *= 0.015625f;
		
	}
	else if(frequency >= 1)
	{
		*prescaller = 12;
		clockcounter *= 0.00390625f;
	}
	else
	{
		*prescaller = 13;
		clockcounter *= 0.0009765625f;
	}

	*ticks = floorf((clockcounter/frequency)) - 1;
}
/*
	initializes the pulse ISR
	In Arduino this is done in TIMER1
	The frequency range is from 4Hz to F_PULSE
*/
void mcu_startStepISR(uint16_t clocks_speed, uint8_t prescaller)
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
void mcu_changeStepISR(uint16_t clocks_speed, uint8_t prescaller)
{
	//stops timer
	TCCR1B = 0;
	OCR1A = clocks_speed;
	//sets OCR0B to half
	//this will allways fire step_reset between pulses
    OCR1B = OCR1A>>1;
	//reset timer
    TCNT1 = 0;
	//start timer in CTC mode with the correct prescaler
    TCCR1B = prescaller;
}

void mcu_stopStepISR()
{
	TCCR1B = 0;
    TIMSK1 &= ~((1 << OCIE1B) | (1 << OCIE1A));
}

void mcu_attachOnStep(ISRVOID handler)
{
	g_mcu_stepCallback = handler;
}

void mcu_attachOnStepReset(ISRVOID handler)
{
	g_mcu_stepResetCallback = handler;
}

uint8_t mcu_eeprom_getc(uint16_t address)
{
}

uint8_t mcu_eeprom_putc(uint16_t address, uint8_t value)
{
}

#endif
