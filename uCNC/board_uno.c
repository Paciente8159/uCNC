#include "config.h"

#if(BOARD == BOARD_UNO)
#include <math.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "pins.h"
#include "board.h"

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

#ifdef DEBUGMODE
#define MAX(A,B) if(B>A) A=B
volatile uint8_t g_board_perfCounterOffset = 0;
volatile PERFORMANCE_METER board_performacecounters;
#endif

//UART communication
//MACHINE_COMMAND g_board_rxbuffer;
uint8_t g_board_combuffer[COM_BUFFER_SIZE];
uint8_t g_board_bufferhead;
uint8_t g_board_buffertail;

//IO Registers
IO_REGISTER g_board_dirRegister;
volatile INPUT_REGISTER g_board_inputs;
volatile OUTPUT_REGISTER g_board_outputs;

ISRTIMER g_board_pulseCallback = NULL;
ISRTIMER g_board_pulseResetCallback = NULL;
ISRPINCHANGE g_board_pinChangeCallback = NULL;

ISR(TIMER0_COMPA_vect) // timer compare uint8_terrupt service routine
{
	#ifdef DEBUGMODE
	board_startPerfCounter();
	#endif
    if(g_board_pulseCallback!=NULL)
    	g_board_pulseCallback();
    #ifdef DEBUGMODE
	uint16_t counter = board_stopPerfCounter();
	MAX(board_performacecounters.pulseCounter, counter);
	#endif	
}

ISR(TIMER0_COMPB_vect) // timer compare uint8_terrupt service routine
{
	#ifdef DEBUGMODE
	board_startPerfCounter();
	#endif
    if(g_board_pulseResetCallback!=NULL)
    	g_board_pulseResetCallback();
    #ifdef DEBUGMODE
	uint16_t counter = board_stopPerfCounter();
	MAX(board_performacecounters.resetPulseCounter, counter);
	#endif
}

/*ISR(TIMER1_OVF_vect)
{
	g_board_perfcountoverflows++;
}*/

#ifdef PORTISR0
	ISR(PCINT0_vect) // input pin on change service routine
	{
		#ifdef DEBUGMODE
		board_startPerfCounter();
		#endif
	    (*(inreg)).r0 = PORTRD0;
	    if(g_board_pinChangeCallback!=NULL)
	    	g_board_pinChangeCallback(&g_board_inputs);
	    #ifdef DEBUGMODE
		uint16_t counter = board_stopPerfCounter();
		MAX(board_performacecounters.pinChangeCounter, counter);
		#endif	
	}
#endif

#ifdef PORTISR1
	ISR(PCINT1_vect) // input pin on change service routine
	{
		#ifdef DEBUGMODE
		board_startPerfCounter();
		#endif
	    (*(inreg)).r1 = PORTRD1;
	    if(g_board_pinChangeCallback!=NULL)
	    	g_board_pinChangeCallback(&g_board_inputs);
	    #ifdef DEBUGMODE
		uint16_t counter = board_stopPerfCounter();
		MAX(board_performacecounters.pinChangeCounter, counter);
		#endif
	}
#endif

#ifdef PORTISR2
	ISR(PCINT2_vect) // input pin on change service routine
	{
		#ifdef DEBUGMODE
		board_startPerfCounter();
		#endif
	    g_board_inputs.critical_inputs = (PORTRD2 >> 2);
	    if(g_board_pinChangeCallback!=NULL)
	    	g_board_pinChangeCallback(&g_board_inputs);
	    #ifdef DEBUGMODE
		uint16_t counter = board_stopPerfCounter();
		MAX(board_performacecounters.pinChangeCounter, counter);
		#endif
	}
#endif

#ifdef PORTISR3
	ISR(PCINT3_vect) // input pin on change service routine
	{
		#ifdef DEBUGMODE
		board_startPerfCounter();
		#endif
	    (*(inreg)).r3 = PORTRD3;
	    if(g_board_pinChangeCallback!=NULL)
	    	g_board_pinChangeCallback(&g_board_inputs);
	    #ifdef DEBUGMODE
		uint16_t counter = board_stopPerfCounter();
		MAX(board_performacecounters.pinChangeCounter, counter);
		#endif
	}
#endif

#if(IN10 >= 0)
	ISR(USART_RXC_vect)
	{
		g_board_combuffer[g_board_buffertail] = UDR0;
		if(++g_board_buffertail==COM_BUFFER_SIZE)
			g_board_buffertail = 0;
	}
#endif

void board_setup()
{
    IO_REGISTER reg = {};
    
    g_board_dirRegister.r = OUTPUT_PINS;
    
    //sets all outputs (all other pins will be inputs)
    #ifdef PORTDIR0
        PORTDIR0 = g_board_dirRegister.r0;
    #endif
    #ifdef PORTDIR1
        PORTDIR1 = g_board_dirRegister.r1;
    #endif
    #ifdef PORTDIR2
        PORTDIR2 = g_board_dirRegister.r2;
    #endif
    #ifdef PORTDIR3
        PORTDIR3 = g_board_dirRegister.r3;
    #endif

    //sets default output logic states
    //also sets the pull-ups if active
    board_setOutputs(0);
    /* unused 
    #ifdef INPUT_PULLUP_MASK
        //if pull-ups are active add them
        reg.r = INPUT_PULLUP_MASK;
        #ifdef PORTWR0
        	PORTWR0 |= reg.r0;
	    #endif
	    #ifdef PORTWR1
	        PORTWR1 |= reg.r1;
	    #endif
	    #ifdef PORTWR2
	        PORTWR2 |= reg.r2;
	    #endif
	    #ifdef PORTWR3
	        PORTWR3 |= reg.r3;
	    #endif
    #endif*/
    
    //initializes input internal register
    g_board_inputs.reg32in = 0;
    #ifdef PORTRD0
        g_board_inputs.r0 = PORTRD0;
    #endif
    #ifdef PORTRD1
        g_board_inputs.r1 = PORTRD1;
    #endif
    #ifdef PORTRD2
        g_board_inputs.critical_inputs = (PORTRD2>>2);
    #endif
    #ifdef PORTRD3
        g_board_inputs.r3 = PORTRD3;
    #endif
    
    //activates interrupt on all inputs except COM_RX
    reg.r = ~g_board_dirRegister.r;
    #if (RX >= 0)
    	reg.r &= ~(BITVALPIN(RX));
    #endif
    #ifdef PCMSK0
        PCMSK0 = reg.r0;
        PCICR |= _BV(0);
    #endif
    #ifdef PCMSK1
        PCMSK1 = reg.r1;
        PCICR |= _BV(1);
    #endif
    #ifdef PCMSK2
        PCMSK2 = reg.r2;
        PCICR |= _BV(2);
    #endif
    #ifdef PCMSK3
        PCMSK3 = reg.r3;
        PCICR |= _BV(3);
    #endif

    //set serial port
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
    
    #ifdef DEBUGMODE
    //calculate performance offset
    board_startPerfCounter();
    g_board_perfCounterOffset = board_stopPerfCounter();
    #endif

}

uint16_t board_getAnalog(uint8_t pin)
{
    return 0;
}

//IO functions    
//Inputs  
uint16_t board_getInputs()
{
	return g_board_inputs.inputs;	
}

uint8_t board_getCriticalInputs()
{
	return g_board_inputs.critical_inputs;
}

//outputs
void board_setStepDirs(uint16_t value)
{
	g_board_outputs.dirstep_0_3 = (volatile uint8_t)value;
	//g_board_outputs.dirstep_4 = (value>>8);
	PORTWR1 = (volatile uint8_t)value;
}

void board_setOutputs(uint16_t value)
{

}

void board_enableInterrupts()
{
	sei();
}
void board_disableInterrupts()
{
	cli();
}

void board_attachOnInputChange(ISRPINCHANGE handler)
{
	g_board_pinChangeCallback = handler;
}

void board_uart_putchar(char c)
{
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
}

char board_uart_getchar()
{
    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}

uint8_t board_comPacketReady()
{//
	//if(g_board_tail != sizeof(g_board_rxbuffer))
	{
		return 0;
	}
}

//UART communications
void board_comSendPacket(uint8_t *ptr, uint8_t length)
{
    do
    {
        board_uart_putchar(*ptr);
        ptr++;
    } while(--length);
}

uint8_t board_comGetPacket(uint8_t *ptr, uint8_t length)
{
    uint8_t avail = g_board_buffertail - g_board_bufferhead;
    
    if(g_board_bufferhead > g_board_buffertail)
    {
    	avail += COM_BUFFER_SIZE;
	}
	
	if(avail < length | length==0) //can't read packet (not complete)
	{
		return 0;
	}
	
	do
	{
		*ptr = g_board_combuffer[g_board_bufferhead];
		if(++g_board_bufferhead==COM_BUFFER_SIZE)
			g_board_bufferhead = 0;
	} while(--length);
	

    return 1;
}

int16_t board_comPeek()
{
	if(g_board_buffertail==g_board_bufferhead)
	{
		return -1;
	}
	else
	{
		return g_board_combuffer[g_board_bufferhead];
	}	
}

void board_comClear()
{
	g_board_bufferhead = 0;
	g_board_buffertail = 0;
}

#ifdef DEBUGMODE
//measures performance up to 65536 clock cycles
void board_startPerfCounter()
{
    TCCR1A = 0; // set entire TCCR1A register to 0
    TCCR1B = 0; // same for TCCR1B
    TCNT1 = 0;  //initialize counter value to 0
    TIFR1 = 0;
    //TIMSK1 |= (1 << TOIE1);
    cli();
    TCCR1B = 1;
}

uint16_t board_stopPerfCounter()
{
    uint16_t ticks = TCNT1;
    if(TIFR1 & 0x01)//checks for overflow
    {
    	ticks = 0xFFFF;
	}
    TCCR1B = 0;
    sei();
    return (ticks - g_board_perfCounterOffset);
}
#endif

//RealTime
void board_startPulse(uint32_t frequency)
{
    uint16_t prescaler = 1;
    if (frequency < 62993)
        prescaler = 8;
    if (frequency < 7875)
        prescaler = 64;
    if (frequency < 985)
        prescaler = 256;
    if (frequency < 124)
        prescaler = 1024;

    //set timer1 uint8_terrupt at 1Hz
    TCCR0A = 0; // set entire TCCR1A register to 0
    TCCR0B = 0; // same for TCCR1B
    TCNT0 = 0;  //initialize counter value to 0
    // set compare match register for 1hz increments
    OCR0B = F_CPU / (prescaler * frequency) - 1; // = (16*10^6) / (1*1024) - 1 (must be <65536)
    
    //uses the second OCR register to generate te pulse interrupt with the same timer offset
    uint8_t pulseoffset = OCR0B - (uint8_t)(MIN_PULSE_WIDTH_US * F_CPU / 1000000UL);
    if(pulseoffset<1)
    	pulseoffset = 1;
    OCR0A = OCR0B - pulseoffset;
    
    
    // turn on CTC mode
    //TCCR1B |= (1 << WGM12);
    switch (prescaler)
    {
    case 1:
        TCCR0B |= (0 << CS12) | (0 << CS11) | (1 << CS10);
        break;
    case 8:
        TCCR0B |= (0 << CS12) | (1 << CS11) | (0 << CS10);
        break;
    case 64:
        TCCR0B |= (0 << CS12) | (1 << CS11) | (1 << CS10);
        break;
    case 256:
        TCCR0B |= (1 << CS12) | (0 << CS11) | (0 << CS10);
        break;
    case 1024:
        TCCR0B |= (1 << CS12) | (0 << CS11) | (1 << CS10);
        break;
    }

    // enable timer compare uint8_terrupt
    TIMSK0 |= (1 << OCIE0B) | (1 << OCIE0A);
}

void board_stopPulse()
{
	TCCR0B &=~((1 << CS12) | (1 << CS11) | (1 << CS10));
    TIMSK0 &= ~((1 << OCIE0B) | (1 << OCIE0A));
}

void board_attachOnPulse(ISRTIMER handler)
{
	g_board_pulseCallback = handler;
}

void board_attachOnPulseReset(ISRTIMER handler)
{
	g_board_pulseResetCallback = handler;
}

uint8_t board_readProMemByte(uint8_t* src)
{
	return pgm_read_byte(src);
}
/*
void board_startTimer1ISR(uint32_t frequency)
{
    uint16_t prescaler = 1;
    if (frequency < 245)
        prescaler = 8;
    if (frequency < 31)
        prescaler = 64;
    if (frequency < 4)
        prescaler = 256;
    if (frequency < 1)
        prescaler = 1024;

    //set timer1 uint8_terrupt at 1Hz
    TCCR1A = 0; // set entire TCCR1A register to 0
    TCCR1B = 0; // same for TCCR1B
    TCNT1 = 0;  //initialize counter value to 0
    // set compare match register for 1hz increments
    OCR1A = F_CPU / (prescaler * frequency) - 1; // = (16*10^6) / (1*1024) - 1 (must be <65536)
    // turn on CTC mode
    TCCR1B |= (1 << WGM12);
    switch (prescaler)
    {
    case 1:
        TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);
        break;
    case 8:
        TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10);
        break;
    case 64:
        TCCR1B |= (0 << CS12) | (1 << CS11) | (1 << CS10);
        break;
    case 256:
        TCCR1B |= (1 << CS12) | (0 << CS11) | (0 << CS10);
        break;
    case 1024:
        TCCR1B |= (1 << CS12) | (0 << CS11) | (1 << CS10);
        break;
    }

    // enable timer compare uint8_terrupt
    TIMSK1 |= (1 << OCIE1A);
}

void board_pauseTimer1ISR()
{
    // disable timer and compare uint8_terrupt
    TCCR1B &=~((1 << CS12) | (1 << CS11) | (1 << CS10));
    TIMSK1 &= ~(1 << OCIE1A);
}*/

#endif
