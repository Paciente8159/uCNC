/*
	Name: mcu_virtual.c
	Description: Simulates and MCU that runs on a PC. This is mainly used to test/simulate µCNC.
		For now it's only working/tested on Windows.
		Besides all the functions declared in the mcu.h it also implements the code responsible
		for handling:
			interpolator.h
				void itp_step_isr();
				void itp_step_reset_isr();
			serial.h
				void serial_rx_isr(char c);
				char serial_tx_isr();
			trigger_control.h
				void dio_limits_isr(uint8_t limits);
				void io_controls_isr(uint8_t controls);
				
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

#include "cnc.h"
#if (MCU == MCU_VIRTUAL)
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>

#include "interface/settings.h"
#include "virtualtimer.h"
#include "virtualserial.h"
#include "interface/serial.h"
#include "core/interpolator.h"
#include "core/io_control.h"

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#ifndef BAUDRATE
#define BAUDRATE 115200
#endif

#ifndef COM_BUFFER_SIZE
#define COM_BUFFER_SIZE 50
#endif

virtports_t virtualports;

static VIRTUAL_MAP virtualmap;
/**/

//UART communication
uint8_t g_mcu_combuffer[COM_BUFFER_SIZE];
uint8_t g_mcu_bufferhead;
uint8_t g_mcu_buffertail;
uint8_t g_mcu_buffercount;
char *mcu_tx_buffer;
volatile bool mcu_tx_ready;
/*
#ifdef __DEBUG__
#define MAX(A,B) if(B>A) A=B
volatile uint8_t g_mcu_perfCounterOffset = 0;
volatile PERFORMANCE_METER mcu_performacecounters;
#endif
*/
uint32_t _previnputs = 0;

volatile bool global_isr_enabled = false;
volatile unsigned long isr_flags = 0;

#define ISR_PULSE 1
#define ISR_PULSERESET 2
#define ISR_COMRX 4
#define ISR_COMTX 8
#define ISR_INPUT 16

volatile unsigned long g_cpu_freq = 0;
volatile unsigned long pulse_interval = 0;
volatile unsigned long resetpulse_interval = 0;
volatile unsigned long pulse_counter = 0;
volatile unsigned long *pulse_counter_ptr;
volatile unsigned long integrator_counter = 0;
volatile bool pulse_enabled = false;
volatile bool send_char = false;
volatile unsigned char uart_char;

pthread_t thread_id;
pthread_t thread_idout;
pthread_t thread_timer_id;
pthread_t thread_step_id;

//emulates uart RX
void *comsimul(void)
{
#ifndef USECONSOLE

#ifndef ENABLE_SYNC_TX
	virtualserial_open(&serial_tx_isr, &serial_rx_isr);
#else
	virtualserial_open(NULL, &serial_rx_isr);
#endif

#else
	for (;;)
	{
		unsigned char c = getch();
		if (c != 0)
		{
			uart_char = c;
			while (!serial_rx_is_empty())
			{
			}
			serial_rx_isr(c);
			if (c == '\n' | c == '\r')
			{
			}
		}
	}
#endif
}

//emulates uart TX
void *comoutsimul(void)
{
	unsigned char combuffer[128];
	static uint8_t i = 0;
	for (;;)
	{
		if (mcu_tx_ready)
		{
			serial_tx_isr();
		}
	}
}

//simulates internal clock (1Kz limited by windows timer)
volatile static uint32_t mcu_runtime = 0;

void *stepsimul(void)
{
	static uint16_t tick_counter = 0;
	static uint16_t timer_counter = 0;
	unsigned long lasttime = getTickCounter();
	unsigned long acumm = 0;
	while (1)
	{

		unsigned long time = getTickCounter();
		unsigned long elapsed = time - lasttime;
		acumm += elapsed;
		elapsed *= F_CPU;
		elapsed /= g_cpu_freq;
		elapsed = (elapsed < 100) ? elapsed : 100;

		while (acumm > (F_CPU / 1000))
		{
			acumm -= (F_CPU / 1000);
			mcu_runtime++;
		}

		while (elapsed--)
		{
			if (pulse_interval && resetpulse_interval && pulse_enabled)
			{
				tick_counter++;
			}
			else
			{
				tick_counter = 0;
				break;
			}

			if (tick_counter == pulse_interval)
			{
				isr_flags |= ISR_PULSE; //flags step isr
			}

			if (tick_counter >= resetpulse_interval)
			{
				isr_flags |= ISR_PULSERESET; //flags step isr
				tick_counter = 0;
			}

			if (global_isr_enabled)
			{
				bool isr = global_isr_enabled;
				global_isr_enabled = false;

				if (isr_flags & ISR_INPUT)
				{
					//serial_rx_isr(uart_char);
					io_limits_isr();
					io_controls_isr();
					isr_flags &= ~ISR_INPUT;
				}

				if (pulse_enabled)
				{
					if (isr_flags & ISR_PULSE)
					{
						itp_step_isr();
						isr_flags &= ~ISR_PULSE;
					}

					if (isr_flags & ISR_PULSERESET)
					{
						itp_step_reset_isr();
						isr_flags &= ~ISR_PULSERESET;
					}
				}

				global_isr_enabled = isr;
			}

			lasttime = time;
		}
	}
}

void ticksimul(void)
{

	static VIRTUAL_MAP initials = {0};
	/*
	FILE *infile = fopen("inputs.txt", "r");
	char inputs[255];

	if (infile != NULL) //checks input file
	{
		fscanf(infile, "%lX", &(virtualports->inputs));
		fclose(infile);

		uint32_t diff = virtualports->inputs ^ initials.inputs;
		initials.inputs = virtualports->inputs;

		if (diff)
		{
			isr_flags |= ISR_INPUT; //flags input isr
		}
	}*/

	mcu_runtime++;
}

uint32_t mcu_millis()
{
	return mcu_runtime;
}

void mcu_init(void)
{
	send_char = false;
	virtualports = &virtualmap;
	FILE *infile = fopen("inputs.txt", "r");
	if (infile != NULL)
	{
		fscanf(infile, "%lX", &(virtualports->inputs));
		fclose(infile);
	}
	else
	{
		infile = fopen("inputs.txt", "w+");
		if (infile != NULL)
		{
			fprintf(infile, "%lX", virtualports->inputs);
			fflush(infile);
			fclose(infile);
		}
		else
		{
			printf("Failed to open input file");
		}
	}
	g_cpu_freq = getCPUFreq();
	//start_timer(1, &ticksimul);
	pthread_create(&thread_id, NULL, &comsimul, NULL);
#ifdef USECONSOLE
	pthread_create(&thread_idout, NULL, &comoutsimul, NULL);
#endif
	pthread_create(&thread_step_id, NULL, &stepsimul, NULL);
	mcu_tx_ready = false;
	g_mcu_buffercount = 0;
	pulse_counter_ptr = &pulse_counter;
	mcu_enable_global_isr();
}

//IO functions
void mcu_enable_probe_isr(void)
{
}
void mcu_disable_probe_isr(void)
{
}

uint8_t mcu_get_analog(uint8_t channel)
{
	return 0;
}

//Outputs
void mcu_set_pwm(uint8_t pwm, uint8_t value)
{
}

uint8_t mcu_get_pwm(uint8_t pwm)
{
	return 0;
}

//Communication functions
//sends a packet
void mcu_enable_tx_isr(void)
{
#ifndef USECONSOLE
	serial_tx_isr();
#endif
	mcu_tx_ready = true;
}

void mcu_disable_tx_isr(void)
{
	mcu_tx_ready = false;
}

void mcu_putc(char c)
{
#ifdef USECONSOLE
	putchar(c);
#else
	virtualserial_putc(c);
	putchar(c);
#endif
}

char mcu_getc(void)
{
	char c = 0;
	if (g_mcu_buffertail != g_mcu_bufferhead)
	{
		c = g_mcu_combuffer[g_mcu_buffertail];
		if (++g_mcu_buffertail == COM_BUFFER_SIZE)
		{
			g_mcu_buffertail = 0;
		}

		if (c == '\n')
		{
			g_mcu_buffercount--;
		}
	}

	return c;
}

char mcu_peek(void)
{
	if (g_mcu_buffercount == 0)
		return 0;
	return g_mcu_combuffer[g_mcu_buffertail];
}

void mcu_bufferClear(void)
{
	memset(&g_mcu_combuffer, 0, sizeof(char) * COM_BUFFER_SIZE);
	g_mcu_buffertail = 0;
	g_mcu_bufferhead = 0;
}

//RealTime
void mcu_freq_to_clocks(float frequency, uint16_t *ticks, uint16_t *tick_reps)
{
	if (frequency < F_STEP_MIN)
		frequency = F_STEP_MIN;
	if (frequency > F_STEP_MAX)
		frequency = F_STEP_MAX;

	*ticks = (uint16_t)floorf((F_CPU / frequency));
	*tick_reps = 1;
}

//enables all interrupts on the mcu. Must be called to enable all IRS functions
void mcu_enable_global_isr(void)
{
	global_isr_enabled = true;
}
//disables all ISR functions
void mcu_disable_global_isr(void)
{
	global_isr_enabled = false;
}

//starts a constant rate pulse at a given frequency. This triggers to ISR handles with an offset of MIN_PULSE_WIDTH useconds
void mcu_start_itp_isr(uint16_t clocks_speed, uint16_t prescaller)
{
	resetpulse_interval = clocks_speed;
	pulse_interval = resetpulse_interval >> 1;
	(*pulse_counter_ptr) = 0;
	pulse_enabled = true;
}

void mcu_change_itp_isr(uint16_t clocks_speed, uint16_t prescaller)
{
	pulse_enabled = false;
	resetpulse_interval = clocks_speed;
	pulse_interval = resetpulse_interval >> 1;
	(*pulse_counter_ptr) = 0;
	pulse_enabled = true;
}
//stops the pulse
void mcu_stop_itp_isr(void)
{
	pulse_enabled = false;
}

void mcu_printfp(const char *__fmt, ...)
{
	char buffer[50];
	char *newfmt = strcpy((char *)&buffer, __fmt);
	va_list __ap;
	va_start(__ap, __fmt);
	vprintf(newfmt, __ap);
	va_end(__ap);
}

void mcu_loadDummyPayload(const char *__fmt, ...)
{
	char buffer[30];
	char payload[50];
	char *newfmt = strcpy((char *)&buffer, __fmt);
	va_list __ap;
	va_start(__ap, __fmt);
	vsprintf((char *)&payload, newfmt, __ap);
	va_end(__ap);
	g_mcu_bufferhead = strlen(payload);
	memset(&g_mcu_combuffer, 0, g_mcu_bufferhead);
	strcpy((char *)&g_mcu_combuffer, payload);
	g_mcu_buffertail = 0;
	g_mcu_buffercount++;
}

uint8_t mcu_eeprom_getc(uint16_t address)
{
	FILE *fp = fopen("virtualeeprom", "rb");
	uint8_t c = 0;

	if (fp != NULL)
	{
		if (!fseek(fp, address, SEEK_SET))
		{
			c = getc(fp);
			fclose(fp);
		}
	}

	return c;
}

void mcu_eeprom_putc(uint16_t address, uint8_t value)
{
	FILE *src = fopen("virtualeeprom", "rb+");

	if (!src)
	{
		FILE *dest = fopen("virtualeeprom", "wb");
		fclose(dest);
		src = fopen("virtualeeprom", "rb+");
	}

	/*for(int i = 0; i < address; i++)
	{
		getc(src);
	}*/

	fseek(src, address, SEEK_SET);
	putc((int)value, src);

	fflush(src);
	fclose(src);
}
/*
void mcu_eeprom_erase(uint16_t address)
{
	FILE* src = fopen("virtualeeprom", "rb+");
	
	if(!src)
	{
		FILE* dest = fopen("virtualeeprom", "wb");
		fclose(dest);
		src = fopen("virtualeeprom", "rb+");
	}

	fseek(src, address, SEEK_SET);	
	putc(EOF, src);
	
	fflush(src);
	fclose(src);
}*/

/*uint8_t mcu_eeprom_putc(uint16_t address, uint8_t value)
{
	FILE* src = fopen("virtualeeprom", "r");
	FILE* dest = fopen("newvirtualeeprom", "w");
	
	for(int i = 0; i < address; i++)
	{
		if(src!=NULL)
			putc(getc(src), dest);
		else
			putc(0, dest);
	}
	
	if(src!=NULL)
		getc(src);
	putc(value, dest);
	
	for(int i = address + 1; i < 1024; i++)
	{
		if(src!=NULL)
			putc(getc(src), dest);
		else
			putc(0, dest);
	}
	
	if(src!=NULL)
		fclose(src);
	fflush(dest);
	fclose(dest);
	remove("virtualeeprom");
	rename("newvirtualeeprom", "virtualeeprom");
	
	return value;
}*/

void mcu_startPerfCounter(void)
{
	startCycleCounter();
}

uint16_t mcu_stopPerfCounter(void)
{
	return (uint16_t)stopCycleCounter();
}

void mcu_dotasks(void)
{
#ifdef ENABLE_SYNC_RX
	while (mcu_read_available())
	{
		unsigned char c = mcu_getc();
		serial_rx_isr(c);
	}
#endif
}

#endif
