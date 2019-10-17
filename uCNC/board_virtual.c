#include "config.h"

#if(BOARD == BOARD_VIRTUAL)
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h> 

#include "pins.h"
#include "structures.h"
#include "board.h"
#include "util/timer.h"

#define PORTMASK (OUTPUT_INVERT_MASK|INPUT_PULLUP_MASK)
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#ifndef BAUD
#define BAUD 115200
#endif

#define POW0 1L
#define POW1 2L
#define POW2 4L
#define POW3 8L
#define POW4 16L
#define POW5 32L
#define POW6 64L
#define POW7 128L
#define POW8 256L
#define POW9 512L
#define POW10 1024L
#define POW11 2048L
#define POW12 4096L
#define POW13 8192L
#define POW14 16384L
#define POW15 32768L
#define POW16 65536L
#define POW17 131072L
#define POW18 262144L
#define POW19 524288L
#define POW20 1048576L
#define POW21 2097152L
#define POW22 4194304L
#define POW23 8388608L
#define POW24 16777216L
#define POW25 33554432L
#define POW26 67108864L
#define POW27 134217728L
#define POW28 268435456L
#define POW29 536870912L
#define POW30 1073741824L
#define POW31 2147483648L
#define _POW(X) POW##X
#define POW(X) _POW(X)

/*typedef union{
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
} IO_REGISTER;*/

//UART communication
uint8_t g_board_combuffer[COM_BUFFER_SIZE];
uint8_t g_board_bufferhead;
uint8_t g_board_buffertail;
uint8_t g_board_buffercount;

#ifdef DEBUGMODE
#define MAX(A,B) if(B>A) A=B
volatile uint8_t g_board_perfCounterOffset = 0;
volatile PERFORMANCE_METER board_performacecounters;
#endif

//IO Registers
uint32_t g_board_dirRegister;
volatile INPUT_REGISTER g_board_inputs;
volatile OUTPUT_REGISTER g_board_outputs;

uint32_t _previnputs = 0;

volatile bool global_irq_enabled = false;

unsigned long g_cpu_freq = 0;
volatile bool pulse_enabled = false;
volatile bool integrator_enabled = false;
volatile unsigned long pulse_interval = 0;
volatile unsigned long integrator_interval = 0;
volatile unsigned long pulse_counter = 0;
volatile unsigned long integrator_counter = 0;

ISRTIMER g_board_pulseCallback = NULL;
ISRTIMER g_board_pulseResetCallback = NULL;
ISRTIMER g_board_integratorCallback = NULL;
ISRPINCHANGE g_board_pinChangeCallback = NULL;
ISRCOMRX g_board_charReceived = NULL;

CMD_PACKET dummy_packet;
pthread_t thread_id;
pthread_t thread_timer_id;

void* timersimul()
{
	//unsigned long freq = getCPUFreq();
	unsigned long pulse_counter = 0;
	unsigned long integrator_counter = 0;
	unsigned long ticks = 0;//getTickCounter();
	//unsigned long pulse;
	//unsigned long integrator;
	for(;;)
	{
		//unsigned long newticks = getTickCounter();
		//unsigned long diff = (newticks>=ticks) ? newticks - ticks : newticks - ticks + (unsigned long)-1;
		
		if(global_irq_enabled)
		{
			if(pulse_enabled)
				pulse_counter++;// += diff;
		
			if(integrator_enabled)
				integrator_counter++;// += diff;
	
			if(pulse_counter>=pulse_interval && pulse_enabled )
			{
				pulse_counter = 0;
				if(g_board_pulseCallback!=NULL)
				{
					g_board_pulseCallback();
				}
			}
			
			if(integrator_counter>=integrator_interval && integrator_enabled)
			{
				integrator_counter = 0;
				if(g_board_integratorCallback!=NULL)
				{
					g_board_integratorCallback();
				}
			}
		}
		
		//ticks = newticks;
	}
}

void* inputsimul()
{
	for(;;)
	{
		char c = getch();
		
		switch(c)
		{
			case '\b':
				putchar(c);
				putchar(' ');
				if(g_board_bufferhead!=g_board_buffertail)
					g_board_bufferhead--;
				g_board_combuffer[g_board_bufferhead] = '\0';
				g_board_bufferhead--;
				break;
			case '\r':
			case '\n':
				c = '\n';
				g_board_combuffer[g_board_bufferhead] = c;
				g_board_buffercount++;
			default:
				g_board_combuffer[g_board_bufferhead] = c;
		}
		
		putchar(c);
		if(++g_board_bufferhead == COM_BUFFER_SIZE)
		{
			g_board_bufferhead = 0;
		}
		
		if(g_board_charReceived != NULL)
		{
			g_board_charReceived(c);
		}
	}
}

void ticksimul()
{
	static uint16_t tick_counter = 0;
	
	FILE *infile = fopen("inputs.txt", "r");
	char inputs[255];
	
	if(infile!=NULL)
	{
		fscanf(infile, "%uX %uX", &(g_board_inputs.critical_inputs), &(g_board_inputs.inputs));
		fclose(infile);
	}
	
	/*if(tick_enabled)
	{
		tick_counter++;
		
		if(pulse_interval != 0)
		{
			if(g_board_pulseCallback!=NULL)
			{
				if(tick_counter%pulse_interval==0)
				{
					g_board_pulseCallback();
				}
			}
		    		
		    if(g_board_pulseResetCallback!=NULL)
		    {
		    	if(tick_counter%(pulse_interval + MIN_PULSE_WIDTH_US)==0)
				{
					g_board_pulseResetCallback();
				}
			}
		}
				
	    if(g_board_pinChangeCallback!=NULL)
	    {
	    	if(_previnputs != g_board_inputs.reg32in)
	    	{
	    		_previnputs = g_board_inputs.reg32in;
	    		g_board_pinChangeCallback(&(g_board_inputs.reg32in));
			}	
		}
	}*/
}

void board_init()
{
	FILE *infile = fopen("inputs.txt", "w+");
	if(infile!=NULL)
	{
		fprintf(infile, "%X %X", g_board_inputs.critical_inputs,  g_board_inputs.inputs);
		fflush(infile);
		fclose(infile);
	}
	else
	{
		printf("Failed to open input file");
	}
	
	g_cpu_freq = getCPUFreq();
	
	memset(&dummy_packet, 0, sizeof(CMD_PACKET));
	
	//start_timer(1, &ticksimul);
	pthread_create(&thread_id, NULL, &inputsimul, NULL); 
	pthread_create(&thread_timer_id, NULL, &timersimul, NULL); 
	g_board_buffercount = 0;
}

//IO functions    
//Inputs  
//returns the value of the input pins
uint16_t board_getInputs()
{
	return g_board_inputs.inputs;	
}
//returns the value of the critical input pins
uint8_t board_getCriticalInputs()
{
	return g_board_inputs.critical_inputs;
}
//attaches a function handle to the input pin changed ISR
void board_attachOnInputChange(ISRPINCHANGE handler)
{
	g_board_pinChangeCallback = handler;
}
//detaches the input pin changed ISR
void board_detachOnInputChange()
{
	g_board_pinChangeCallback = NULL;
}

//outputs
//sets all step and dir pins
void board_setStepDirs(uint16_t value)
{	
	g_board_outputs.dirstep_0_3 = value & 0xFF;
	g_board_outputs.dirstep_4 = value>>8 & 0x0F;
}
//sets all digital outputs pins
void board_setOutputs(uint16_t value)
{
	g_board_outputs.outputs = value;
}

//Communication functions
//sends a packet
void board_putc(char c)
{
	putchar(c);
}

char board_getc()
{
	char c = 0;
	if(g_board_buffertail!=g_board_bufferhead)
	{
		c = g_board_combuffer[g_board_buffertail];
		if(++g_board_buffertail==COM_BUFFER_SIZE)
		{
			g_board_buffertail = 0;
		}
		
		if(c=='\n')
		{
			g_board_buffercount--;
		}
	}
	
	return c;
}

char board_peek()
{
	if(g_board_buffercount==0)
		return 0;
	return g_board_combuffer[g_board_buffertail];
}

void board_bufferClear()
{
	memset(&g_board_combuffer, 0, sizeof(char)*COM_BUFFER_SIZE);
	g_board_buffertail = 0;
	g_board_bufferhead = 0;
}

//RealTime
//enables all interrupts on the board. Must be called to enable all IRS functions
void board_enableInterrupts()
{
	global_irq_enabled = true;
}
//disables all ISR functions
void board_disableInterrupts()
{
	global_irq_enabled = false;
}

//starts a constant rate pulse at a given frequency. This triggers to ISR handles with an offset of MIN_PULSE_WIDTH useconds
void board_startPulse(float frequency)
{
	pulse_interval = g_cpu_freq/frequency;
	pulse_counter = 0;
	pulse_enabled = true;
}

void board_changePulse(float frequency)
{
	pulse_interval = g_cpu_freq/frequency;
	pulse_counter = 0;
}
//stops the pulse 
void board_stopPulse()
{
	pulse_enabled = false;
}
//attaches a function handle to the pulse ISR
void board_attachOnPulse(ISRTIMER handler)
{
	g_board_pulseCallback = handler;
}
void board_detachOnPulse()
{
	g_board_pulseCallback = NULL;
}
//attaches a function handle to the reset pulse ISR. This is fired MIN_PULSE_WIDTH useconds after pulse ISR
void board_attachOnPulseReset(ISRTIMER handler)
{
	g_board_pulseResetCallback = handler;
}

void board_detachOnPulseReset()
{
	g_board_pulseCallback = NULL;
}

//starts a constant rate pulse at a given frequency. This triggers to ISR handles with an offset of MIN_PULSE_WIDTH useconds
void board_startIntegrator(float frequency)
{
	integrator_interval = g_cpu_freq/frequency;
	integrator_counter = 0;
	integrator_enabled = true;
}
//stops the pulse 
void board_stopIntegrator()
{
	integrator_enabled = false;
}
//attaches a function handle to the pulse ISR
void board_attachOnIntegrator(ISRTIMER handler)
{
	g_board_integratorCallback = handler;
}
void board_detachOnIntegrator()
{
	g_board_integratorCallback = NULL;
}

void board_printfp(const char* __fmt, ...)
{
	char buffer[50];
	char* newfmt = strcpy((char*)&buffer, __fmt);
	va_list __ap;
 	va_start(__ap,__fmt);
 	vprintf(newfmt,__ap);
 	va_end(__ap);
}

void board_loadDummyPayload(const char* __fmt, ...)
{
	char buffer[30];
	char payload[50];
	char* newfmt = strcpy((char*)&buffer, __fmt);
	va_list __ap;
 	va_start(__ap,__fmt);
 	vsprintf((char*)&payload, newfmt,__ap);
 	va_end(__ap);
	g_board_bufferhead = strlen(payload);
	memset(&g_board_combuffer, 0, g_board_bufferhead);
	strcpy((char*)&g_board_combuffer, payload);
	g_board_buffertail = 0;
	g_board_buffercount++;
}


uint8_t board_readProMemByte(uint8_t* src)
{
	return *src;
}

void board_startPerfCounter()
{
	startCycleCounter();
}

uint16_t board_stopPerfCounter()
{
    return (uint16_t)stopCycleCounter();
}

#endif
