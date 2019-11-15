#include "config.h"

#if(MCU == MCU_VIRTUAL)
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h> 
#include <math.h>

typedef struct virtual_map_t
{
	uint8_t steps;
	uint8_t dirs;
	uint8_t controls;
	uint8_t limits;
	uint8_t probe;
}VIRTUAL_MAP;

#include "mcumap.h"
#include "mcu.h"
#include "util/timer.h"
#include "interpolator.h"

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#ifndef BAUD
#define BAUD 115200
#endif

#ifndef COM_BUFFER_SIZE
#define COM_BUFFER_SIZE 50
#endif

virtports_t virtualports;

static VIRTUAL_MAP virtualmap;
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
uint8_t g_mcu_combuffer[COM_BUFFER_SIZE];
uint8_t g_mcu_bufferhead;
uint8_t g_mcu_buffertail;
uint8_t g_mcu_buffercount;

#ifdef __DEBUG__
#define MAX(A,B) if(B>A) A=B
volatile uint8_t g_mcu_perfCounterOffset = 0;
volatile PERFORMANCE_METER mcu_performacecounters;
#endif

uint32_t _previnputs = 0;

volatile bool global_irq_enabled = false;

unsigned long g_cpu_freq = 0;
volatile bool pulse_enabled = false;
volatile bool integrator_enabled = false;
volatile unsigned long pulse_interval = 0;
volatile unsigned long resetpulse_interval = 0;
volatile unsigned long integrator_interval = F_CPU/F_INTEGRATOR;
volatile unsigned long pulse_counter = 0;
unsigned long *pulse_counter_ptr;
volatile unsigned long integrator_counter = 0;

ISRTIMER g_mcu_pulseCallback = NULL;
ISRTIMER g_mcu_pulseResetCallback = NULL;
ISRTIMER g_mcu_integratorCallback = NULL;
ISRPINCHANGE g_mcu_pinChangeCallback = NULL;
ISRCOMRX g_mcu_charReceived = NULL;

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
		//unsigned long newticks = ticks+1;//getTickCounter();
		//unsigned long diff = (newticks>=ticks) ? newticks - ticks : newticks - ticks + (unsigned long)-1;
		//diff = F_CPU * diff / freq; 
		
		if(global_irq_enabled)
		{
			if(pulse_enabled)
				(*pulse_counter_ptr)++;
		
			/*if(integrator_enabled)
				integrator_counter++;*/
	
			if((*pulse_counter_ptr)==pulse_interval && pulse_enabled )
			{
				(*pulse_counter_ptr) = 0;
				/*if(g_mcu_pulseCallback!=NULL)
				{
					g_mcu_pulseCallback();
				}*/
				
				interpolator_step();
			}
			
			if((*pulse_counter_ptr)==resetpulse_interval && pulse_enabled )
			{
				/*if(g_mcu_pulseResetCallback!=NULL)
				{
					g_mcu_pulseResetCallback();
				}*/
				
				interpolator_stepReset();
			}
			
			/*if(integrator_counter==integrator_interval && integrator_enabled)
			{
				integrator_counter = 0;
				if(g_mcu_integratorCallback!=NULL)
				{
					g_mcu_integratorCallback();
				}
			}*/
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
				if(g_mcu_bufferhead!=g_mcu_buffertail)
					g_mcu_bufferhead--;
				g_mcu_combuffer[g_mcu_bufferhead] = '\0';
				g_mcu_bufferhead--;
				break;
			case '\r':
			case '\n':
				c = '\n';
				g_mcu_combuffer[g_mcu_bufferhead] = c;
				g_mcu_buffercount++;
			default:
				g_mcu_combuffer[g_mcu_bufferhead] = c;
		}
		
		putchar(c);
		if(++g_mcu_bufferhead == COM_BUFFER_SIZE)
		{
			g_mcu_bufferhead = 0;
		}
		
		if(g_mcu_charReceived != NULL)
		{
			g_mcu_charReceived(c);
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
		fscanf(infile, "%uX %uX", &(virtualports->controls), &(virtualports->limits));
		fclose(infile);
	}
	
	/*if(tick_enabled)
	{
		tick_counter++;
		
		if(pulse_interval != 0)
		{
			if(g_mcu_pulseCallback!=NULL)
			{
				if(tick_counter%pulse_interval==0)
				{
					g_mcu_pulseCallback();
				}
			}
		    		
		    if(g_mcu_pulseResetCallback!=NULL)
		    {
		    	if(tick_counter%(pulse_interval + MIN_PULSE_WIDTH_US)==0)
				{
					g_mcu_pulseResetCallback();
				}
			}
		}
				
	    if(g_mcu_pinChangeCallback!=NULL)
	    {
	    	if(_previnputs != g_mcu_inputs.reg32in)
	    	{
	    		_previnputs = g_mcu_inputs.reg32in;
	    		g_mcu_pinChangeCallback(&(g_mcu_inputs.reg32in));
			}	
		}
	}*/
}

void mcu_init()
{
	virtualports = &virtualmap;
	FILE *infile = fopen("inputs.txt", "w+");
	if(infile!=NULL)
	{
		fprintf(infile, "%X %X", virtualports->controls, virtualports->limits);
		fflush(infile);
		fclose(infile);
	}
	else
	{
		printf("Failed to open input file");
	}
	
	g_cpu_freq = getCPUFreq();
	
	//start_timer(1, &ticksimul);
	pthread_create(&thread_id, NULL, &inputsimul, NULL); 
	pthread_create(&thread_timer_id, NULL, &timersimul, NULL); 
	g_mcu_buffercount = 0;
	pulse_counter_ptr = &pulse_counter;
	mcu_enableInterrupts();
}

//IO functions    
//Inputs  
//returns the value of the input pins
uint16_t mcu_getInputs()
{
	return 0;	
}
//returns the value of the critical input pins
uint8_t mcu_getControls()
{
	return virtualports->controls;
}

uint8_t mcu_getLimits()
{
	return virtualports->limits;
}

uint8_t mcu_getProbe()
{
	return virtualports->probe;
}

//attaches a function handle to the input pin changed ISR
void mcu_attachOnInputChange(ISRPINCHANGE handler)
{
	g_mcu_pinChangeCallback = handler;
}
//detaches the input pin changed ISR
void mcu_detachOnInputChange()
{
	g_mcu_pinChangeCallback = NULL;
}

//outputs
//sets all step and dir pins
void mcu_setSteps(uint8_t value)
{	
	virtualports->steps = value;
}

void mcu_setDirs(uint8_t value)
{	
	virtualports->dirs = value;
}

//sets all digital outputs pins
void mcu_setOutputs(uint16_t value)
{
	//g_mcu_outputs.outputs = value;
}

//Communication functions
//sends a packet
void mcu_putc(char c)
{
	putchar(c);
}

char mcu_getc()
{
	char c = 0;
	if(g_mcu_buffertail!=g_mcu_bufferhead)
	{
		c = g_mcu_combuffer[g_mcu_buffertail];
		if(++g_mcu_buffertail==COM_BUFFER_SIZE)
		{
			g_mcu_buffertail = 0;
		}
		
		if(c=='\n')
		{
			g_mcu_buffercount--;
		}
	}
	
	return c;
}

char mcu_peek()
{
	if(g_mcu_buffercount==0)
		return 0;
	return g_mcu_combuffer[g_mcu_buffertail];
}

void mcu_bufferClear()
{
	memset(&g_mcu_combuffer, 0, sizeof(char)*COM_BUFFER_SIZE);
	g_mcu_buffertail = 0;
	g_mcu_bufferhead = 0;
}

//RealTime
void mcu_freq2clocks(float frequency, uint16_t* ticks, uint8_t* tick_reps)
{
	if(frequency < F_PULSE_MIN)
		frequency = F_PULSE_MIN;
	if(frequency > F_PULSE_MAX)
		frequency = F_PULSE_MAX;

	*ticks = (uint16_t)floorf((F_CPU/frequency)) - 1;
	*tick_reps = 1;
}

//enables all interrupts on the mcu. Must be called to enable all IRS functions
void mcu_enableInterrupts()
{
	global_irq_enabled = true;
}
//disables all ISR functions
void mcu_disableInterrupts()
{
	global_irq_enabled = false;
}

//starts a constant rate pulse at a given frequency. This triggers to ISR handles with an offset of MIN_PULSE_WIDTH useconds
void mcu_startStepISR(uint16_t clocks_speed, uint8_t prescaller)
{
	pulse_interval = clocks_speed;
	resetpulse_interval = clocks_speed>>1;
	(*pulse_counter_ptr) = 0;
	pulse_enabled = true;
}

void mcu_changeStepISR(uint16_t clocks_speed, uint8_t prescaller)
{
	pulse_enabled = false;
	pulse_interval = clocks_speed;
	resetpulse_interval = clocks_speed>>1;
	if((*pulse_counter_ptr)>=pulse_interval)
	{
		(*pulse_counter_ptr) = resetpulse_interval - 2;
	}
	pulse_enabled = true;
}
//stops the pulse 
void mcu_stopPulse()
{
	pulse_enabled = false;
}
//attaches a function handle to the pulse ISR
void mcu_attachOnPulse(ISRTIMER handler)
{
	g_mcu_pulseCallback = handler;
}
void mcu_detachOnPulse()
{
	g_mcu_pulseCallback = NULL;
}
//attaches a function handle to the reset pulse ISR. This is fired MIN_PULSE_WIDTH useconds after pulse ISR
void mcu_attachOnPulseReset(ISRTIMER handler)
{
	g_mcu_pulseResetCallback = handler;
}

void mcu_detachOnPulseReset()
{
	g_mcu_pulseCallback = NULL;
}

//starts a constant rate pulse at a given frequency. This triggers to ISR handles with an offset of MIN_PULSE_WIDTH useconds
void mcu_startIntegrator()
{
	integrator_interval = F_CPU/F_INTEGRATOR;
	integrator_counter = 0;
	integrator_enabled = true;
}

void mcu_resumeIntegrator()
{
	integrator_enabled = true;
}

void mcu_pauseIntegrator()
{
	integrator_enabled = false;
}
	
//stops the pulse 
void mcu_stopIntegrator()
{
	integrator_enabled = false;
}
//attaches a function handle to the pulse ISR
void mcu_attachOnIntegrator(ISRTIMER handler)
{
	g_mcu_integratorCallback = handler;
}
void mcu_detachOnIntegrator()
{
	g_mcu_integratorCallback = NULL;
}

void mcu_printfp(const char* __fmt, ...)
{
	char buffer[50];
	char* newfmt = strcpy((char*)&buffer, __fmt);
	va_list __ap;
 	va_start(__ap,__fmt);
 	vprintf(newfmt,__ap);
 	va_end(__ap);
}

void mcu_loadDummyPayload(const char* __fmt, ...)
{
	char buffer[30];
	char payload[50];
	char* newfmt = strcpy((char*)&buffer, __fmt);
	va_list __ap;
 	va_start(__ap,__fmt);
 	vsprintf((char*)&payload, newfmt,__ap);
 	va_end(__ap);
	g_mcu_bufferhead = strlen(payload);
	memset(&g_mcu_combuffer, 0, g_mcu_bufferhead);
	strcpy((char*)&g_mcu_combuffer, payload);
	g_mcu_buffertail = 0;
	g_mcu_buffercount++;
}


uint8_t mcu_readProMemByte(uint8_t* src)
{
	return *src;
}

void mcu_startPerfCounter()
{
	startCycleCounter();
}

uint16_t mcu_stopPerfCounter()
{
    return (uint16_t)stopCycleCounter();
}

#endif
