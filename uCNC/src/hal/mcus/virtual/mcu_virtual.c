/*
	Name: mcu_virtual.c
	Description: Simulates and MCU that runs on a Windows PC. This is mainly used to test/simulate µCNC.
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

#include "../../../cnc.h"
#if (MCU == MCU_VIRTUAL_WIN)

// #include <stdio.h>
// #include <winsock2.h>
// #include <ws2tcpip.h>
// #pragma comment(lib, "ws2_32.lib") // Winsock Library
// #include <windows.h>
// #include <windows.h>
// #include <winsock2.h>
// #include <ws2tcpip.h>
// #include <iphlpapi.h>
// #include <stdio.h>

// #pragma comment(lib, "Ws2_32.lib")

// #include "win_port.h"
win_port_t uart0;
win_port_t uart2;

#ifndef WIN_INTERFACE
#define WIN_INTERFACE 0
#endif

#define _str(x) #x
#define str(x) _str(x)

// uncomment to use sockets
#if (WIN_INTERFACE == 1)
#define USESOCKETS
#ifdef USESOCKETS
#define DEFAULT_BUFLEN 127
#ifndef SOCKET_PORT
#define SOCKET_PORT 34000
#endif
#endif
#elif (WIN_INTERFACE == 0)
// uncomment to use serial port
#define USESERIAL
#ifdef USESERIAL
#ifndef WIN_COM_NAME
#define WIN_COM_NAME COM11
#endif
#endif
#elif (WIN_INTERFACE == 2)
#define USECONSOLE
#endif

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#ifndef BAUDRATE
#define BAUDRATE 115200
#endif

#ifndef COM_BUFFER_SIZE
#define COM_BUFFER_SIZE 50
#endif

/*timers*/
int start_timer(int, void (*)(void));
void stop_timer(void);
void startCycleCounter(void);
unsigned long stopCycleCounter(void);
unsigned long getCPUFreq(void);
unsigned long getTickCounter(void);

HANDLE win_timer;
void (*timer_func_handler_pntr)(void);
unsigned long perf_start;

VOID CALLBACK timer_sig_handler(PVOID, BOOLEAN);

int start_timer(int mSec, void (*timer_func_handler)(void))
{
	timer_func_handler_pntr = timer_func_handler;

	if (CreateTimerQueueTimer(&win_timer, NULL, (WAITORTIMERCALLBACK)timer_sig_handler, NULL, mSec, mSec, WT_EXECUTEINTIMERTHREAD) == 0)
	{
		printf("\nCreateTimerQueueTimer() error\n");
		return (1);
	}

	return (0);
}

VOID CALLBACK timer_sig_handler(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	timer_func_handler_pntr();
}

void stop_timer(void)
{
	DeleteTimerQueueTimer(NULL, win_timer, NULL);
	CloseHandle(win_timer);
}

double cyclesPerMicrosecond;
double cyclesPerMillisecond;
void startCycleCounter(void)
{
	if (getCPUFreq() == 0)
	{
		return;
	}

	perf_start = getTickCounter();
}

unsigned long stopCycleCounter(void)
{
	return (getTickCounter() - perf_start);
}

unsigned long getCPUFreq(void)
{
	LARGE_INTEGER perf_counter;

	if (!QueryPerformanceFrequency(&perf_counter))
	{
		printf("QueryPerformanceFrequency failed!\n");
		return 0;
	}

	cyclesPerMicrosecond = (double)perf_counter.QuadPart / 1000000.0;
	cyclesPerMillisecond = (double)perf_counter.QuadPart / 1000.0;

	return perf_counter.QuadPart;
}

unsigned long getTickCounter(void)
{
	LARGE_INTEGER perf_counter;
	QueryPerformanceCounter(&perf_counter);
	return perf_counter.QuadPart;
}

uint32_t mcu_micros(void)
{
	LARGE_INTEGER perf_counter;
	QueryPerformanceCounter(&perf_counter);
	return (uint32_t)(perf_counter.QuadPart / cyclesPerMicrosecond);
}

uint32_t mcu_millis(void)
{
	LARGE_INTEGER perf_counter;
	QueryPerformanceCounter(&perf_counter);
	return (uint32_t)(perf_counter.QuadPart / cyclesPerMillisecond);
}

/**
 * IO simulation and handling for external app
 * */
typedef struct virtual_map_t
{
	uint32_t special_outputs;
	uint32_t outputs;
	uint8_t pwm[16];
	uint8_t servos[6];
	uint32_t special_inputs;
	uint32_t inputs;
	uint8_t analog[16];
} VIRTUAL_MAP;

static volatile VIRTUAL_MAP virtualmap;

void* ioserver(void *args)
{
	HANDLE hPipe;
	BOOL fSuccess = FALSE;
	DWORD cbRead, cbToWrite, cbWritten;
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\ucncio");

	// Try to open a named pipe; wait for it, if necessary.

	while (1)
	{
		BOOL fConnected = FALSE;

		hPipe = CreateNamedPipe(
			lpszPipename,				// pipe name
			PIPE_ACCESS_DUPLEX,			// read/write access
			PIPE_TYPE_MESSAGE |			// message type pipe
				PIPE_READMODE_MESSAGE | // message-read mode
				PIPE_WAIT,				// blocking mode
			PIPE_UNLIMITED_INSTANCES,	// max. instances
			sizeof(VIRTUAL_MAP),		// output buffer size
			sizeof(VIRTUAL_MAP),		// input buffer size
			0,							// client time-out
			NULL);						// no template file

		if (hPipe == INVALID_HANDLE_VALUE)
		{
			printf("CreateNamedPipe failed, GLE=%lu.\n", GetLastError());
			return NULL;
		}

		// Wait for the client to connect; if it succeeds,
		// the function returns a nonzero value. If the function
		// returns zero, GetLastError returns ERROR_PIPE_CONNECTED.

		fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

		if (fConnected)
		{
			// Send a message to the pipe server.

			cbToWrite = sizeof(VIRTUAL_MAP);
			char lpvMessage[sizeof(VIRTUAL_MAP)];
			do
			{
				memcpy(lpvMessage, (VIRTUAL_MAP *)&virtualmap, sizeof(VIRTUAL_MAP));

				fSuccess = WriteFile(
					hPipe,		// pipe handle
					lpvMessage, // message
					cbToWrite,	// message length
					&cbWritten, // bytes written
					NULL);		// not overlapped

				if (!fSuccess)
				{
					printf("WriteFile to pipe failed. GLE=%lu\n", GetLastError());
					break;
				}

				// Read from the pipe.

				fSuccess = ReadFile(
					hPipe,		// pipe handle
					lpvMessage, // buffer to receive reply
					cbToWrite,	// size of buffer
					&cbRead,	// number of bytes read
					NULL);		// not overlapped

				if (!fSuccess && GetLastError() != ERROR_MORE_DATA)
					break;

				VIRTUAL_MAP *ptr = (VIRTUAL_MAP *)&lpvMessage;
				if (virtualmap.special_inputs != ptr->special_inputs)
				{
					uint32_t diff = virtualmap.special_inputs ^ ptr->special_inputs;
					virtualmap.special_inputs = ptr->special_inputs;

					if (diff & 0x1FFUL)
						mcu_limits_changed_cb();
					if (diff & 0x200UL)
						mcu_probe_changed_cb();
					if (diff & 0x3C00UL)
						mcu_controls_changed_cb();
				}
				if (virtualmap.inputs != ptr->inputs)
				{
					virtualmap.inputs = ptr->inputs;
					mcu_inputs_changed_cb();
				}
				memcpy((uint8_t *)virtualmap.analog, ptr->analog, 16);

			} while (fSuccess); // repeat loop if ERROR_MORE_DATA

			if (!fSuccess)
			{
				printf("ReadFile from pipe failed. GLE=%lu\n", GetLastError());
			}
		}

		CloseHandle(hPipe);
	}

	return NULL;
}

/**
 * Comunications can be done via console, sockets or serial port
 * */
#ifdef MCU_HAS_UART2
volatile bool uart2_rx_ready = false;
volatile uint8_t uart2_rx_last = 0;
void win_uart_rcv_callback(uint8_t c)
{
	uart2_rx_ready = true;
	uart2_rx_last = c;
	if (mcu_uart_rcv_cb)
	{
		mcu_uart_rcv_cb(c);
	}
}

int16_t mcu_uart_getc(uint32_t timeout)
{
	timeout += mcu_millis();
	while (!uart2_rx_ready)
	{
		if (timeout < mcu_millis())
			return -1;
	}

	return uart2_rx_last;
}
#endif

void com_init(void)
{
	uart0.io.rx.rxHandler = mcu_com_rx_cb;
#ifdef USESOCKETS
	socket_init(&uart0);
#elif defined(USESERIAL)
	uart_init(&uart0);
#elif defined(USECONSOLE)
	console_init(&uart0);
#endif

#ifdef MCU_HAS_UART2
	uart2.io.rx.rxHandler = mcu_uart_rx_cb;
	mcu_uart_rcv_cb = win_uart_rcv_callback;
	memcpy(uart2.io.portname, "34000\0", 6);
	socket_init(&uart2);
#endif
}

void com_send(char *buff, int len)
{
	port_write(&uart0, buff, len);
}

void mcu_uart_putc(uint8_t c)
{
	port_write(&uart2, (char*)&c, 1);
}

// UART communication
uint8_t g_mcu_combuffer[COM_BUFFER_SIZE];
uint8_t g_mcu_bufferhead;
uint8_t g_mcu_buffertail;
uint8_t g_mcu_buffercount;

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

uint8_t pwms[16];

pthread_t thread_io;
pthread_t thread_id;
pthread_t thread_idout;
pthread_t thread_timer_id;
pthread_t thread_step_id;

void mcu_tx_isr(void)
{
	uart0.io.tx.empty = true;
}

// simulates internal clock (1Kz limited by windows timer)
volatile static uint32_t mcu_runtime = 0;

void *stepsimul(void *args)
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
				isr_flags |= ISR_PULSE; // flags step isr
			}

			if (tick_counter >= resetpulse_interval)
			{
				isr_flags |= ISR_PULSERESET; // flags step isr
				tick_counter = 0;
			}

			if (global_isr_enabled)
			{
				bool isr = global_isr_enabled;
				global_isr_enabled = false;

				if (isr_flags & ISR_INPUT)
				{
					// serial_rx_isr(uart_char);
					mcu_limits_changed_cb();
					mcu_controls_changed_cb();
					isr_flags &= ~ISR_INPUT;
				}

				if (pulse_enabled)
				{
					if (isr_flags & ISR_PULSE)
					{
						mcu_step_cb();
						isr_flags &= ~ISR_PULSE;
					}

					if (isr_flags & ISR_PULSERESET)
					{
						mcu_step_reset_cb();
						isr_flags &= ~ISR_PULSERESET;
					}
				}

				global_isr_enabled = isr;
			}

			lasttime = time;
		}
	}
}

void rpmsimul(void)
{
	virtualmap.inputs ^= (1 << 7);
	mcu_inputs_changed_cb();
}

void ticksimul(void)
{

	// static VIRTUAL_MAP initials = {0};

	if (global_isr_enabled)
	{

		// FILE *infile = fopen("inputs.txt", "r");
		// char inputs[255];

		// if (infile != NULL) //checks input file
		// {
		// 	fscanf(infile, "%lX", &(virtualmap.inputs));
		// 	fclose(infile);

		// 	uint32_t diff = virtualmap.inputs ^ initials.inputs;
		// 	initials.inputs = virtualmap.inputs;

		// 	if (diff)
		// 	{
		// 		isr_flags |= ISR_INPUT; //flags input isr
		// 	}
		// }

		mcu_runtime++;
		mcu_disable_global_isr();
		mcu_rtc_cb(mcu_runtime);
		mcu_enable_global_isr();
	}
}

// uint32_t mcu_millis()
//{
//	return mcu_runtime;
// }

void mcu_init(void)
{
	startCycleCounter();
	virtualmap.special_outputs = 0;
	virtualmap.special_inputs = 0;
	virtualmap.inputs = 0;
	virtualmap.outputs = 0;
	com_init();
	send_char = false;
	//	FILE *infile = fopen("inputs.txt", "r");
	//	if (infile != NULL)
	//	{
	//		fscanf(infile, "%lX", &(virtualmap.inputs));
	//		fclose(infile);
	//	}
	//	else
	//	{
	//		infile = fopen("inputs.txt", "w+");
	//		if (infile != NULL)
	//		{
	//			fprintf(infile, "%lX", virtualmap.inputs);
	//			fflush(infile);txHandle
	//			fclose(infile);
	//		}
	//		else
	//		{
	//			printf("Failed to open input file");
	//		}
	//	}
	g_cpu_freq = getCPUFreq();
	start_timer(1, &ticksimul);
	start_timer(10, &rpmsimul);
	// #ifdef USECONSOLE
	//	pthread_create(&thread_idout, NULL, &comoutsimul, NULL);
	// #endif
	pthread_create(&thread_step_id, NULL, &stepsimul, NULL);
	pthread_create(&thread_io, NULL, &ioserver, NULL);
	uart0.io.tx.empty = false;
	g_mcu_buffercount = 0;
	pulse_counter_ptr = &pulse_counter;
	uart0.io.tx.empty = true;
	mcu_enable_global_isr();
}

// IO functions
void mcu_enable_probe_isr(void)
{
}
void mcu_disable_probe_isr(void)
{
}

#ifdef MCU_HAS_ONESHOT_TIMER
uint32_t mcu_timeout;
extern MCU_CALLBACK mcu_timeout_delgate mcu_timeout_cb;
HANDLE oneshot_handle;
VOID CALLBACK oneshot_handler(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	if (mcu_timeout_cb)
	{
		mcu_timeout_cb();
	}
}
/**
 * configures a single shot timeout in us
 * */
#ifndef mcu_config_timeout
void mcu_config_timeout(mcu_timeout_delgate fp, uint32_t timeout)
{
	mcu_timeout_cb = fp;
	mcu_timeout = timeout;
}
#endif

/**
 * starts the timeout. Once hit the the respective callback is called
 * */
#ifndef mcu_start_timeout
void mcu_start_timeout()
{
	CreateTimerQueueTimer(&oneshot_handle, NULL, (WAITORTIMERCALLBACK)oneshot_handler, NULL, 0, mcu_timeout, WT_EXECUTEINTIMERTHREAD | WT_EXECUTEONLYONCE);
}
#endif
#endif

uint8_t mcu_get_pin_offset(uint8_t pin)
{
	if (pin >= STEP0 && pin <= STEP7_EN)
	{
		return pin;
	}
	else if (pin >= DOUT0 && pin <= DOUT31)
	{
		return pin - DOUT0;
	}
	if (pin >= LIMIT_X && pin <= CS_RES)
	{
		return pin - LIMIT_X;
	}
	else if (pin >= DIN0 && pin <= DIN31)
	{
		return pin - DIN0;
	}

	return -1;
}

void mcu_config_input(uint8_t pin)
{
}

void mcu_config_pwm(uint8_t pin, uint16_t freq)
{
}

uint8_t mcu_get_input(uint8_t pin)
{
	uint8_t offset = mcu_get_pin_offset(pin);
	if (offset > 31)
	{
		return 0;
	}
	if (pin >= DIN0)
	{
		return (virtualmap.inputs & (1 << offset)) ? 1 : 0;
	}
	else
	{
		return (virtualmap.special_inputs & (1 << offset)) ? 1 : 0;
	}

	return 0;
}

void mcu_config_output(uint8_t pin)
{
}

/**
 * gets the value of a digital output pin
 * can be defined either as a function or a macro call
 * */
uint8_t mcu_get_output(uint8_t pin)
{
	uint8_t offset = mcu_get_pin_offset(pin);
	if (offset > 31)
	{
		return 0;
	}

	if (pin >= DOUT0)
	{
		return (virtualmap.outputs & (1 << offset)) ? 1 : 0;
	}
	else
	{
		return (virtualmap.special_outputs & (1 << offset)) ? 1 : 0;
	}

	return 0;
}

/**
 * sets the value of a digital output pin to logical 1
 * can be defined either as a function or a macro call
 * */
void mcu_set_output(uint8_t pin)
{
	uint8_t offset = mcu_get_pin_offset(pin);
	if (offset > 31)
	{
		return;
	}

	if (pin >= DOUT0)
	{
		virtualmap.outputs |= (1UL << offset);
	}
	else
	{
		virtualmap.special_outputs |= (1UL << offset);
	}
}

/**
 * sets the value of a digital output pin to logical 0
 * can be defined either as a function or a macro call
 * */
void mcu_clear_output(uint8_t pin)
{
	uint8_t offset = mcu_get_pin_offset(pin);
	if (offset > 31)
	{
		return;
	}

	if (pin >= DOUT0)
	{
		virtualmap.outputs &= ~(1UL << offset);
	}
	else
	{
		virtualmap.special_outputs &= ~(1UL << offset);
	}
}

/**
 * toggles the value of a digital output pin
 * can be defined either as a function or a macro call
 * */
void mcu_toggle_output(uint8_t pin)
{
	uint8_t offset = mcu_get_pin_offset(pin);
	if (offset > 31)
	{
		return;
	}

	if (pin >= DOUT0)
	{
		virtualmap.outputs ^= (1UL << offset);
	}
	else
	{
		virtualmap.special_outputs ^= (1UL << offset);
	}
}

uint8_t mcu_get_analog(uint8_t channel)
{
	channel -= ANALOG0;
	return virtualmap.analog[channel];
}

// Outputs
void mcu_set_pwm(uint8_t pwm, uint8_t value)
{
	pwm -= PWM0;
	virtualmap.pwm[pwm] = value;
}

uint8_t mcu_get_pwm(uint8_t pwm)
{
	pwm -= PWM0;
	return virtualmap.pwm[pwm];
}

void mcu_set_servo(uint8_t servo, uint8_t value)
{
	servo -= SERVO0;
	virtualmap.servos[value] = value;
}

uint8_t mcu_get_servo(uint8_t servo)
{
	servo -= SERVO0;
	return virtualmap.servos[servo];
}

// Communication functions
// sends a packet
void mcu_enable_tx_isr(void)
{
#ifndef USECONSOLE
	mcu_com_tx_cb();
#endif
	uart0.io.tx.empty = true;
}

void mcu_disable_tx_isr(void)
{
	uart0.io.tx.empty = false;
}

bool mcu_tx_ready(void)
{
	return uart0.io.tx.empty;
}

static char mcu_tx_buffer[256];
void mcu_putc(char c)
{
	static int buff_index = 0;
	if (c != 0)
	{
		//		while (!uart0.io.tx.empty)
		//			;
		//		uart0.io.tx.empty = false;

		mcu_tx_buffer[buff_index++] = c;
		if (c == '\n')
		{
			mcu_tx_buffer[buff_index++] = 0;
			com_send(mcu_tx_buffer, strlen(mcu_tx_buffer));
			buff_index = 0;
		}
		putchar(c);
	}
	uart0.io.tx.empty = true;
	uart0.io.tx.empty = true;
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

// RealTime
void mcu_freq_to_clocks(float frequency, uint16_t *ticks, uint16_t *tick_reps)
{
	if (frequency < F_STEP_MIN)
		frequency = F_STEP_MIN;
	if (frequency > F_STEP_MAX)
		frequency = F_STEP_MAX;

	*ticks = (uint16_t)floorf((F_CPU / frequency));
	*tick_reps = 1;
}

// enables all interrupts on the mcu. Must be called to enable all IRS functions
void mcu_enable_global_isr(void)
{
	global_isr_enabled = true;
}
// disables all ISR functions
void mcu_disable_global_isr(void)
{
	global_isr_enabled = false;
}

bool mcu_get_global_isr(void)
{
	return global_isr_enabled;
}

// starts a constant rate pulse at a given frequency. This triggers to ISR handles with an offset of MIN_PULSE_WIDTH useconds
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
// stops the pulse
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

void virtual_delay_us(uint16_t delay)
{
	unsigned long start = getTickCounter();
	double elapsed = 0;
	do
	{
		elapsed = ((double)(getTickCounter()) - (double)(start)) / (double)(getCPUFreq());
		elapsed *= 1000000;
	} while (elapsed < delay);
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
		}

		fclose(fp);
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

void mcu_eeprom_flush()
{
}

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
}

void mcu_config_input_isr(int pin)
{
}

int main(void)
{
	// initializes all systems
	cnc_init();

	for (;;)
	{
		cnc_run();
	}
}

#endif
