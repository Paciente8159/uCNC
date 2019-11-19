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

#define PORTMASK (OUTPUT_INVERT_MASK|INPUT_PULLUP_MASK)
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#ifndef BAUD
#define BAUD 9600
#endif

#ifndef COM_BUFFER_SIZE
#define COM_BUFFER_SIZE 50
#endif

#define PULSE_RESET_DELAY MIN_PULSE_WIDTH_US * F_CPU / 1000000
//#define INTEGRATOR_BASE_FREQ F_CPU>>8

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

//UART communication
//MACHINE_COMMAND g_mcu_rxbuffer;
uint8_t g_mcu_rxbuffer[COM_BUFFER_SIZE];
uint8_t g_mcu_rxhead;
uint8_t g_mcu_rxtail;
volatile uint8_t g_mcu_rxcount;
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

ISRPINCHANGE g_mcu_pinChangeCallback;
ISRCOMRX g_mcu_rs232RxCallback;

ISR(TIMER1_COMPA_vect, ISR_BLOCK)
{
    if(g_mcu_stepCallback!=NULL)
	{
    	g_mcu_stepCallback();
	}
	//interpolator_step();
}

ISR(TIMER1_COMPB_vect, ISR_BLOCK)
{
    if(g_mcu_stepResetCallback!=NULL)
	{
    	g_mcu_stepResetCallback();
	}
	//interpolator_stepReset();
}

/*ISR(TIMER0_OVF_vect, ISR_BLOCK)
{
	if(g_mcu_integratorCallback!=NULL)
	{
		g_mcu_integratorCallback();
	}
	interpolator_rt_integrator();
}*/

#ifdef PORTISR0
	ISR(PCINT0_vect, ISR_NOBLOCK) // input pin on change service routine
	{
		/*#ifdef __DEBUG__
		uint32_t tickcount = mcu_getCycles();
		#endif*/
	    g_mcu_inputs.r0 = PORTRD0;
	    if(g_mcu_pinChangeCallback!=NULL)
	    	g_mcu_pinChangeCallback(&g_mcu_inputs.r);
	    /*#ifdef __DEBUG__
		tickcount = mcu_getElapsedCycles(tickcount);
		MAX(mcu_performacecounters.pinChangeCounter, counter);
		#endif*/	
	}
#endif

#ifdef PORTISR1
	ISR(PCINT1_vect, ISR_NOBLOCK) // input pin on change service routine
	{
		/*#ifdef __DEBUG__
		uint32_t tickcount = mcu_getCycles();
		#endif*/
	    g_mcu_inputs.r1 = PORTRD1;
	    if(g_mcu_pinChangeCallback!=NULL)
	    	g_mcu_pinChangeCallback(&g_mcu_inputs.r);
	    /*#ifdef __DEBUG__
		tickcount = mcu_getElapsedCycles(tickcount);
		MAX(mcu_performacecounters.pinChangeCounter, counter);
		#endif*/
	}
#endif

#ifdef PORTISR2
	ISR(PCINT2_vect, ISR_NOBLOCK) // input pin on change service routine
	{
		/*#ifdef __DEBUG__
		uint32_t tickcount = mcu_getCycles();
		#endif*/
		g_mcu_inputs.r2 = PORTRD2;
	    //g_mcu_inputs.critical_inputs = (PORTRD2 >> 2);
	    if(g_mcu_pinChangeCallback!=NULL)
	    	g_mcu_pinChangeCallback(&g_mcu_inputs.r);
	    /*#ifdef __DEBUG__
		tickcount = mcu_getElapsedCycles(tickcount);
		MAX(mcu_performacecounters.pinChangeCounter, counter);
		#endif*/
	}
#endif

#ifdef PORTISR3
	ISR(PCINT3_vect, ISR_NOBLOCK) // input pin on change service routine
	{
		/*#ifdef __DEBUG__
		uint32_t tickcount = mcu_getCycles();
		#endif*/
		g_mcu_inputs.r3 = PORTRD3;
	    //g_mcu_inputs.critical_inputs = (PORTRD2 >> 2);
	    if(g_mcu_pinChangeCallback!=NULL)
	    	g_mcu_pinChangeCallback(&g_mcu_inputs.r);
	    /*#ifdef __DEBUG__
		tickcount = mcu_getElapsedCycles(tickcount);
		MAX(mcu_performacecounters.pinChangeCounter, counter);
		#endif*/
	}
#endif

#if(RX >= 0)
	ISR(USART_RX_vect, ISR_NOBLOCK)
	{
        volatile char c = UDR0;
        g_mcu_rxbuffer[g_mcu_rxhead] = c;
        if(++g_mcu_rxhead==COM_BUFFER_SIZE)
        {
        	g_mcu_rxhead = 0;
		}
            
    	if((c == '\n') | (c == '\r'))
    	{
    		g_mcu_rxcount++;
		}

        if(g_mcu_rs232RxCallback!=NULL)
        {
            g_mcu_rs232RxCallback(c);
        }
	}
#endif

void mcu_init()
{
    IO_REGISTER reg = {};
    
    g_mcu_stepCallback = NULL;
	g_mcu_stepResetCallback = NULL;
	g_mcu_pinChangeCallback = NULL;
	g_mcu_rs232RxCallback = NULL;
	
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
    g_mcu_rxhead = 0;
    g_mcu_rxtail = 0;
    g_mcu_rxcount = 0;
    
    stdout = &g_mcu_streamout;
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
  
    // enable rx, tx, and interrupt on complete reception of a byte
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

uint16_t mcu_getAnalog(uint8_t pin)
{
    return 0;
}

//IO functions    
//Inputs  
uint16_t mcu_getInputs()
{
	
	return 0;//g_mcu_inputs.inputs;	
}

uint8_t mcu_getControls()
{
	uint8_t val = CONTROLS_INREG & CONTROLS_MASK;
	return val;
}

uint8_t mcu_getLimits()
{
	uint8_t val = LIMITS_INREG & LIMITS_MASK;
	return val;
}

uint8_t mcu_getProbe()
{
	uint8_t val = PROBE_INREG & PROBE_MASK;
	return val;
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

void mcu_attachOnInputChange(ISRPINCHANGE handler)
{
	g_mcu_pinChangeCallback = handler;
}

//internal redirect of stdout
int mcu_putchar(char c, FILE* stream)
{
	mcu_putc(c);
	return c;
}

void mcu_putc(char c)
{
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
}

char mcu_getc()
{
	char c = 0;

    if(g_mcu_rxtail!=g_mcu_rxhead)
    {
    	c = g_mcu_rxbuffer[g_mcu_rxtail++];
		if(g_mcu_rxtail>=COM_BUFFER_SIZE)
		{
			g_mcu_rxtail = 0;
		}
		
		if(c=='\n')
		{
			g_mcu_rxcount--;
		}
		
        return c;
    }

    return 0;
}

char mcu_peek()
{
	if(g_mcu_rxcount==0)
	{
		return 0;
	}
    
	return g_mcu_rxbuffer[g_mcu_rxtail];
}

void mcu_bufferClear()
{
    g_mcu_rxtail = 0;
    g_mcu_rxhead = 0;
    g_mcu_rxbuffer[0] = 0;
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

void mcu_stopPulse()
{
	TCCR1B = 0;
    TIMSK1 &= ~((1 << OCIE1B) | (1 << OCIE1A));
}

void mcu_attachOnStep(ISRTIMER handler)
{
	g_mcu_stepCallback = handler;
}

void mcu_attachOnStepReset(ISRTIMER handler)
{
	g_mcu_stepResetCallback = handler;
}
/*
//starts a constant rate integrator
//timer0 overflow with prescaller at 256
//frequency of integrator = F_CPU/256
void mcu_startIntegrator()
{
	//stops timer
	TCCR0B = 0;
	//Normal mode
    TCCR0A = 0;
    //resets counter
    TCNT0 = 0;
    
    //float pulse_dur = 1.0f/frequency;
    //g_mcu_tmr0_value = (uint16_t)((float)(INTEGRATOR_BASE_FREQ)*pulse_dur) - 1;
    //g_mcu_tmr0_counter = g_mcu_tmr0_value;
    
    TIFR0 = 0;
    TIMSK0 |= (1 << TOIE0);
	//start timer in Normal mode 256 prescaler
	TCCR0B = 4;
}

void mcu_pauseIntegrator()
{
	TCCR0B = 0;
}

void mcu_resumeIntegrator()
{
	TCCR0B = 4;
}

//stops the pulse 
void mcu_stopIntegrator()
{
	TIMSK0 &= ~(1 << TOIE0);
	//start timer in Normal mode without prescaler
	TCCR0B = 0;
}
//attaches a function handle to the integrator ISR
void mcu_attachOnIntegrator(ISRTIMER handler)
{
	g_mcu_integratorCallback = handler;
}

void mcu_detachOnIntegrator()
{
	g_mcu_integratorCallback = NULL;
}
*/
void mcu_printp(const char *__fmt)
{
	char buffer[50];
	char* newfmt = strcpy_P((char*)&buffer, __fmt);
	printf(newfmt);
}

void mcu_printfp(const char *__fmt, ...)
{
	char buffer[100];
	char* newfmt = strcpy_P((char*)&buffer, __fmt);
	va_list __ap;
 	va_start(__ap,__fmt);
 	vprintf(newfmt,__ap);
 	va_end(__ap);
}

uint8_t mcu_readProMemByte(uint8_t* src)
{
	return pgm_read_byte(src);
}

void mcu_loadDummyPayload(const char *__fmt, ...)
{
	char buffer[50];
	//erase old string
	memset((char*)&g_mcu_rxbuffer, 0, COM_BUFFER_SIZE);
	
	//print formated string to buffer;
	char* newfmt = strcpy_P((char*)&buffer, __fmt);
	va_list __ap;
 	va_start(__ap,__fmt);
 	vsprintf((char*)&g_mcu_rxbuffer, newfmt,__ap);
 	va_end(__ap);
	
	g_mcu_rxhead = strlen((const char*)&g_mcu_rxbuffer);
	g_mcu_rxtail = 0;
	
	//signal read
	g_mcu_rxcount++;
}

#endif
