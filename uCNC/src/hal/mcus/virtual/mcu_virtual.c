/*
	Name: mcu_virtual.h
	Description: Implements µCNC mcu emulation on Windows.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 22-08-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#if defined(_WIN32) || defined(_WIN64)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "../../../cnc.h"
#if (MCU == MCU_VIRTUAL_WIN)

#ifdef __cplusplus
extern "C"
{
#endif

/* C99 includes */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <math.h>

/* Platform includes */
#include <conio.h>
#include <pthread.h>
#if defined(ENABLE_SOCKETS) && defined(MCU_HAS_SOCKETS)

#include "../../../modules/endpoint.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif
#include <windows.h>
#include <dirent.h>

	/* ----- Global ISR enable/disable --------------------------------------- */

	volatile bool global_isr_enabled = false;

	void mcu_enable_global_isr(void) { global_isr_enabled = true; }
	void mcu_disable_global_isr(void) { global_isr_enabled = false; }
	bool mcu_get_global_isr(void) { return global_isr_enabled; }

	/* ----- UART (Windows COM) - non-blocking connect/service ----------------
		 UART maps to a Windows serial port (UART_PORT_NAME). We connect and service
		 it from a background thread. RX is polled into a ring buffer; TX drains a
		 ring buffer. If the port is unavailable, the thread keeps retrying without
		 blocking the main loop.
	------------------------------------------------------------------------- */

#ifdef MCU_HAS_UART

#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE 64
#endif

	/* uCNC FIFO macros from cnc.h are used to keep behavior consistent */
	DECL_BUFFER(uint8_t, uart_tx, UART_TX_BUFFER_SIZE);
	DECL_BUFFER(uint8_t, uart_rx, RX_BUFFER_SIZE);

	/* Minimal C serial “driver” */
	typedef struct
	{
		HANDLE h;
		volatile bool connected;
		DWORD errors;
		COMSTAT status;
		char port_name[128];

		pthread_t thread;
		volatile bool stop;
	} serial_port_t;

	static serial_port_t g_uart = {0};

	static bool serial_configure(HANDLE h)
	{
		DCB dcbSerialParams;
		memset(&dcbSerialParams, 0, sizeof(dcbSerialParams));
		dcbSerialParams.DCBlength = sizeof(DCB);

		if (!GetCommState(h, &dcbSerialParams))
		{
			printf("Serial: GetCommState failed\n");
			return false;
		}

		/* Mirror original WindowsSerial: 9600 8N1, DTR enabled */
		dcbSerialParams.BaudRate = CBR_9600;
		dcbSerialParams.ByteSize = 8;
		dcbSerialParams.StopBits = ONESTOPBIT;
		dcbSerialParams.Parity = NOPARITY;
		dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

		if (!SetCommState(h, &dcbSerialParams))
		{
			printf("Serial: SetCommState failed\n");
			return false;
		}

		/* Non-blocking reads: quick return if no data */
		COMMTIMEOUTS to = {0};
		to.ReadIntervalTimeout = 1;
		to.ReadTotalTimeoutMultiplier = 0;
		to.ReadTotalTimeoutConstant = 0;
		to.WriteTotalTimeoutMultiplier = 0;
		to.WriteTotalTimeoutConstant = 0;
		if (!SetCommTimeouts(h, &to))
		{
			printf("Serial: SetCommTimeouts failed\n");
			return false;
		}

		PurgeComm(h, PURGE_RXCLEAR | PURGE_TXCLEAR);
		return true;
	}

	static int serial_read(char *buffer, unsigned int nbChar)
	{
		if (!g_uart.connected)
			return 0;

		DWORD bytesRead = 0;
		unsigned int toRead = 0;

		ClearCommError(g_uart.h, &g_uart.errors, &g_uart.status);

		if (g_uart.status.cbInQue > 0)
		{
			toRead = (g_uart.status.cbInQue > nbChar) ? nbChar : g_uart.status.cbInQue;
			if (toRead > 0 && ReadFile(g_uart.h, buffer, toRead, &bytesRead, NULL))
			{
				return (int)bytesRead;
			}
		}
		return 0;
	}

	static bool serial_write(const uint8_t *buffer, unsigned int nbChar)
	{
		if (!g_uart.connected)
			return false;
		DWORD bytesSent = 0;
		if (!WriteFile(g_uart.h, (void *)buffer, nbChar, &bytesSent, NULL))
		{
			ClearCommError(g_uart.h, &g_uart.errors, &g_uart.status);
			return false;
		}
		return true;
	}

	static void *uart_thread_fn(void *arg)
	{
		(void)arg;
		/* Try to connect and keep connection alive */
		while (!g_uart.stop)
		{
			if (!g_uart.connected)
			{
				HANDLE h = CreateFileA(
					g_uart.port_name, GENERIC_READ | GENERIC_WRITE, 0, NULL,
					OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

				if (h == INVALID_HANDLE_VALUE)
				{
					/* Port not present yet; non-blocking wait/retry */
					Sleep(500);
					continue;
				}

				if (!serial_configure(h))
				{
					CloseHandle(h);
					Sleep(500);
					continue;
				}

				g_uart.h = h;
				g_uart.connected = true;
				/* No startup Sleep here; non-blocking startup */
				// printf("Serial: connected to %s\n", g_uart.port_name);
			}
			else
			{
				/* Check for hang-up/errors occasionally */
				ClearCommError(g_uart.h, &g_uart.errors, &g_uart.status);
				/* Light duty periodic sleep to avoid busy loop */
				Sleep(10);
			}
		}

		if (g_uart.connected)
		{
			g_uart.connected = false;
			CloseHandle(g_uart.h);
			g_uart.h = INVALID_HANDLE_VALUE;
		}
		return NULL;
	}

	/* HAL impl: UART */
	uint8_t mcu_uart_getc(void)
	{
		uint8_t c = 0;
		BUFFER_TRY_DEQUEUE(uart_rx, &c);
		return c;
	}
	uint8_t mcu_uart_available(void) { return BUFFER_READ_AVAILABLE(uart_rx); }
	void mcu_uart_clear(void) { BUFFER_CLEAR(uart_rx); }
	void mcu_uart_putc(uint8_t c)
	{
		while (!BUFFER_TRY_ENQUEUE(uart_tx, &c))
		{
			mcu_uart_flush();
		}
	}
	void mcu_uart_flush(void)
	{
		while (!BUFFER_EMPTY(uart_tx))
		{
			uint8_t tmp[UART_TX_BUFFER_SIZE + 1];
			memset(tmp, 0, sizeof(tmp));
			uint8_t r = 0;
			BUFFER_READ(uart_tx, tmp, UART_TX_BUFFER_SIZE, r);
			if (!g_uart.connected)
			{
				/* leave remaining bytes enqueued? We already read into tmp; if not connected,
					 we can drop or re-insert. For simplicity, send attempt will fail silently. */
				return;
			}
			serial_write(tmp, r);
		}
	}

	/* Poll into RX buffer from UART thread-managed port */
	static void mcu_uart_process(void)
	{
		char buff[RX_BUFFER_SIZE];
		int count = serial_read(buff, RX_BUFFER_SIZE);
		for (int i = 0; i < count; i++)
		{
			uint8_t c = (uint8_t)buff[i];
			if (mcu_com_rx_cb(c))
			{
				if (!BUFFER_TRY_ENQUEUE(uart_rx, &c))
				{
					STREAM_OVF(c);
				}
			}
		}
	}

#endif /* MCU_HAS_UART */

	/* ----- UART2 (console) -------------------------------------------------- */

#ifdef MCU_HAS_UART2

#ifndef UART2_TX_BUFFER_SIZE
#define UART2_TX_BUFFER_SIZE 64
#endif

	DECL_BUFFER(uint8_t, uart2_tx, UART2_TX_BUFFER_SIZE);
	DECL_BUFFER(uint8_t, uart2_rx, RX_BUFFER_SIZE);

	uint8_t mcu_uart2_getc(void)
	{
		uint8_t c = 0;
		BUFFER_DEQUEUE(uart2_rx, &c);
		return c;
	}
	uint8_t mcu_uart2_available(void) { return BUFFER_READ_AVAILABLE(uart2_rx); }
	void mcu_uart2_clear(void) { BUFFER_CLEAR(uart2_rx); }
	void mcu_uart2_putc(uint8_t c)
	{
		while (!BUFFER_TRY_ENQUEUE(uart2_tx, &c))
		{
			mcu_uart2_flush();
		}
	}
	void mcu_uart2_flush(void)
	{
		while (!BUFFER_EMPTY(uart2_tx))
		{
			uint8_t tmp[UART2_TX_BUFFER_SIZE + 1];
			memset(tmp, 0, sizeof(tmp));
			uint8_t r = 0;
			BUFFER_READ(uart2_tx, tmp, UART2_TX_BUFFER_SIZE, r);
			printf("%s", tmp);
			fflush(stdout);
		}
	}

	/* Read console keypresses non-blockingly and echo */
	static void mcu_uart2_process(void)
	{
		if (_kbhit())
		{
			int kc = _getch();
			char c = (char)kc;
			putchar(c);
			if (c == '\r')
				putchar('\n');
			if (mcu_com_rx_cb((uint8_t)c))
			{
				if (!BUFFER_TRY_ENQUEUE(uart2_rx, &c))
				{
					STREAM_OVF(c);
				};
			}
		}
	}

#endif /* MCU_HAS_UART2 */

	/* ----- Run periodic device tasks --------------------------------------- */

	void mcu_dotasks(void)
	{
#ifdef MCU_HAS_UART
		mcu_uart_process();
#endif
#ifdef MCU_HAS_UART2
		mcu_uart2_process();
#endif
	}

	/* ----- EEPROM emulation (file) ----------------------------------------- */

	uint8_t mcu_eeprom_getc(uint16_t address)
	{
		FILE *fp = fopen("virtualeeprom", "rb");
		uint8_t c = 0;
		if (fp != NULL)
		{
			if (!fseek(fp, address, SEEK_SET))
			{
				int v = getc(fp);
				if (v != EOF)
					c = (uint8_t)v;
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
			if (dest)
				fclose(dest);
			src = fopen("virtualeeprom", "rb+");
		}
		if (src)
		{
			fseek(src, address, SEEK_SET);
			putc((int)value, src);
			fflush(src);
			fclose(src);
		}
	}
	void mcu_eeprom_flush(void) {}

	/* ----- IO simulation (named pipe to external UI) ----------------------- */

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

	static void *ioserver(void *args)
	{
		(void)args;
		HANDLE hPipe;
		BOOL fSuccess = FALSE;
		DWORD cbRead, cbToWrite, cbWritten;
		LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\ucncio");

		for (;;)
		{
			BOOL fConnected = FALSE;
			hPipe = CreateNamedPipe(
				lpszPipename,
				PIPE_ACCESS_DUPLEX,
				PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
				PIPE_UNLIMITED_INSTANCES,
				sizeof(VIRTUAL_MAP),
				sizeof(VIRTUAL_MAP),
				0,
				NULL);

			if (hPipe == INVALID_HANDLE_VALUE)
			{
				printf("CreateNamedPipe failed, GLE=%lu.\n", GetLastError());
				return NULL;
			}

			fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
			if (fConnected)
			{
				cbToWrite = sizeof(VIRTUAL_MAP);
				uint8_t lpvMessage[sizeof(VIRTUAL_MAP)];
				do
				{
					memcpy(lpvMessage, (const void *)&virtualmap, sizeof(VIRTUAL_MAP));
					fSuccess = WriteFile(hPipe, lpvMessage, cbToWrite, &cbWritten, NULL);
					if (!fSuccess)
					{
						printf("WriteFile to pipe failed. GLE=%lu\n", GetLastError());
						break;
					}

					fSuccess = ReadFile(hPipe, lpvMessage, cbToWrite, &cbRead, NULL);
					if (!fSuccess && GetLastError() != ERROR_MORE_DATA)
						break;

					VIRTUAL_MAP *ptr = (VIRTUAL_MAP *)&lpvMessage[0];
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
					memcpy((void *)virtualmap.analog, ptr->analog, 16);
				} while (fSuccess);

				if (!fSuccess)
				{
					printf("ReadFile from pipe failed. GLE=%lu\n", GetLastError());
				}
			}

			CloseHandle(hPipe);
		}
		return NULL;
	}

	static uint8_t mcu_get_pin_offset(uint8_t pin)
	{
		if (pin >= 1 && pin <= 24)
		{
			return pin - 1;
		}
		else if (pin >= 47 && pin <= 78)
		{
			return pin - 47;
		}
		if (pin >= 100 && pin <= 113)
		{
			return pin - 100;
		}
		else if (pin >= 130 && pin <= 161)
		{
			return pin - 130;
		}

		return 255;
	}

	void mcu_config_input(uint8_t pin) { (void)pin; }
	void mcu_config_output(uint8_t pin) { (void)pin; }
	void mcu_config_pwm(uint8_t pin, uint16_t freq)
	{
		(void)pin;
		(void)freq;
	}

	uint8_t mcu_get_input(uint8_t pin)
	{
		uint8_t offset = mcu_get_pin_offset(pin);
		if (offset > 31)
			return 0;
		if (pin >= DIN0)
			return (virtualmap.inputs & (1UL << offset)) ? 1 : 0;
		return (virtualmap.special_inputs & (1UL << offset)) ? 1 : 0;
	}

	uint8_t mcu_get_output(uint8_t pin)
	{
		uint8_t offset = mcu_get_pin_offset(pin);
		if (offset > 31)
			return 0;
		if (pin >= DOUT0)
			return (virtualmap.outputs & (1UL << offset)) ? 1 : 0;
		return (virtualmap.special_outputs & (1UL << offset)) ? 1 : 0;
	}

	void mcu_set_output(uint8_t pin)
	{
		uint8_t offset = mcu_get_pin_offset(pin);
		if (offset > 31)
			return;
		if (pin >= DOUT0)
			virtualmap.outputs |= (1UL << offset);
		else
			virtualmap.special_outputs |= (1UL << offset);
	}
	void mcu_clear_output(uint8_t pin)
	{
		uint8_t offset = mcu_get_pin_offset(pin);
		if (offset > 31)
			return;
		if (pin >= DOUT0)
			virtualmap.outputs &= ~(1UL << offset);
		else
			virtualmap.special_outputs &= ~(1UL << offset);
	}
	void mcu_toggle_output(uint8_t pin)
	{
		uint8_t offset = mcu_get_pin_offset(pin);
		if (offset > 31)
			return;
		if (pin >= DOUT0)
			virtualmap.outputs ^= (1UL << offset);
		else
			virtualmap.special_outputs ^= (1UL << offset);
	}

	uint16_t mcu_get_analog(uint8_t channel)
	{
		channel -= ANALOG0;
		return virtualmap.analog[channel];
	}
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
	void mcu_set_servo(uint8_t servo, uint8_t v)
	{
		servo -= SERVO0;
		virtualmap.servos[servo] = v;
	}
	uint8_t mcu_get_servo(uint8_t servo)
	{
		servo -= SERVO0;
		return virtualmap.servos[servo];
	}

	void mcu_enable_probe_isr(void) {}
	void mcu_disable_probe_isr(void) {}

	/* ----- Interpolator timer & timekeeping -------------------------------- */

#ifndef ITP_SAMPLE_RATE
#define ITP_SAMPLE_RATE (F_STEP_MAX * 2)
#endif

#if defined(MCU_HAS_ONESHOT_TIMER)
	MCU_CALLBACK mcu_timeout_delgate mcu_timeout_cb = NULL;
	static uint32_t virtual_oneshot_counter;
	static uint32_t virtual_oneshot_reload;
	static FORCEINLINE void mcu_gen_oneshot(void)
	{
		if (virtual_oneshot_counter)
		{
			virtual_oneshot_counter--;
			if (!virtual_oneshot_counter)
			{
				if (mcu_timeout_cb)
					mcu_timeout_cb();
			}
		}
	}
#endif

	static volatile uint32_t mcu_itp_timer_reload;
	static volatile bool mcu_itp_timer_running;
	static FORCEINLINE void mcu_gen_step(void)
	{
		static bool step_reset = true;
		static int32_t mcu_itp_timer_counter;

		// generate steps
		if (mcu_itp_timer_running)
		{
			// stream mode tick
			int32_t t = mcu_itp_timer_counter;
			bool reset = step_reset;
			t -= (int32_t)ceilf(1000000.0f / ITP_SAMPLE_RATE);
			if (t <= 0)
			{
				if (!reset)
				{
					mcu_step_cb();
				}
				else
				{
					mcu_step_reset_cb();
				}
				step_reset = !reset;
				mcu_itp_timer_counter = mcu_itp_timer_reload + t;
			}
			else
			{
				mcu_itp_timer_counter = t;
			}
		}
	}

	/**
	 * convert step rate to clock cycles
	 * */
	void mcu_freq_to_clocks(float frequency, uint16_t *ticks, uint16_t *prescaller)
	{
		frequency = CLAMP((float)F_STEP_MIN, frequency, (float)F_STEP_MAX);
		// up and down counter (generates half the step rate at each event)
		uint32_t totalticks = (uint32_t)((500000.0f) / frequency);
		*prescaller = 1;
		while (totalticks > 0xFFFF)
		{
			(*prescaller) <<= 1;
			totalticks >>= 1;
		}

		*ticks = (uint16_t)totalticks;
	}

	float mcu_clocks_to_freq(uint16_t ticks, uint16_t prescaller)
	{
		uint32_t totalticks = (uint32_t)ticks * prescaller;
		return 500000.0f / ((float)totalticks);
	}

	/**
	 * starts the timer interrupt that generates the step pulses for the interpolator
	 * */

	void mcu_start_itp_isr(uint16_t ticks, uint16_t prescaller)
	{
		if (!mcu_itp_timer_running)
		{
			mcu_itp_timer_reload = ticks * prescaller;
			mcu_itp_timer_running = true;
		}
		else
		{
			mcu_change_itp_isr(ticks, prescaller);
		}
	}

	/**
	 * changes the step rate of the timer interrupt that generates the step pulses for the interpolator
	 * */
	void mcu_change_itp_isr(uint16_t ticks, uint16_t prescaller)
	{
		if (mcu_itp_timer_running)
		{
			mcu_itp_timer_reload = ticks * prescaller;
		}
		else
		{
			mcu_start_itp_isr(ticks, prescaller);
		}
	}

	/**
	 * stops the timer interrupt that generates the step pulses for the interpolator
	 * */
	void mcu_stop_itp_isr(void)
	{
		if (mcu_itp_timer_running)
		{
			mcu_itp_timer_running = false;
		}
	}


	/* Windows timer wheel to tick emulator */
	HANDLE win_timer;
	void (*timer_func_handler_pntr)(void);
	unsigned long perf_start;
	double cyclesPerMicrosecond;
	double cyclesPerMillisecond;

	FILE *stimuli;
	uint64_t tickcount;

#define def_printpin(X) \
	if (stimuli)        \
	fprintf(stimuli, "$var wire 1 %c " #X " $end\n", 33 + X)
#define printpin(X) \
	if (stimuli)    \
	fprintf(stimuli, "%d%c\n", ((virtualmap.special_outputs & (1 << (X - 1))) ? 1 : 0), 33 + X)

	volatile unsigned long g_cpu_freq = 0;

	VOID CALLBACK timer_sig_handler(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
	{
		if (timer_func_handler_pntr)
			timer_func_handler_pntr();
	}

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

	void stop_timer(void)
	{
		DeleteTimerQueueTimer(NULL, win_timer, NULL);
		CloseHandle(win_timer);
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

	uint32_t mcu_micros(void)
	{
		// LARGE_INTEGER perf_counter;
		// QueryPerformanceCounter(&perf_counter);
		// return (uint32_t)(perf_counter.QuadPart / cyclesPerMicrosecond);
		return (uint32_t)tickcount;
	}

	uint32_t mcu_millis(void)
	{
		// LARGE_INTEGER perf_counter;
		// QueryPerformanceCounter(&perf_counter);
		// return (uint32_t)(perf_counter.QuadPart / cyclesPerMillisecond);
		return (uint32_t)(tickcount / 1000);
	}

	/**
	 * configures a single shot timeout in us
	 * */
	static uint32_t oneshot_timeout;
	static uint32_t oneshot_alarm;
	void mcu_config_timeout(mcu_timeout_delgate fp, uint32_t timeout)
	{
		oneshot_timeout = timeout;
		mcu_timeout_cb = fp;
	}

	/**
	 * starts the timeout. Once hit the the respective callback is called
	 * */
	void mcu_start_timeout()
	{
		oneshot_alarm = mcu_micros() + oneshot_timeout;
	}

	/* Periodic tick that drives stepper and RTC callbacks */
	static void ticksimul(void)
	{
		static bool running = false;
		bool test = false;
		do
		{
			test = __atomic_load_n(&running, __ATOMIC_RELAXED);
			if (test)
			{
				return;
			}
		} while (!__atomic_compare_exchange_n(&running, &test, true, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
		static uint32_t prev, next_rtc = 1000;
		float parcial = 0;
		//		long t = stopCycleCounter();
		//		printf("Elapsed %dus\n\r", (int)((double)t / cyclesPerMicrosecond));
		float timestep = round((float)EMULATION_MS_TICK * ITP_SAMPLE_RATE * 0.001f);
		for (int i = 0; i < (int)timestep; i++)
		{
			parcial += (1000000.0f / (float)ITP_SAMPLE_RATE);
			tickcount += (int)parcial;
			parcial -= (int)parcial;

			mcu_gen_step();
#if defined(MCU_HAS_ONESHOT_TIMER)
			mcu_gen_oneshot();
#endif

			if (prev ^ virtualmap.special_outputs)
			{
				prev = virtualmap.special_outputs;
				if (stimuli)
					fprintf(stimuli, "#%llu\n", tickcount);
#if AXIS_COUNT > 0
				printpin(STEP0);
				printpin(DIR0);
#endif
#if AXIS_COUNT > 1
				printpin(STEP1);
				printpin(DIR1);
#endif
#if AXIS_COUNT > 2
				printpin(STEP2);
				printpin(DIR2);
#endif
#if AXIS_COUNT > 3
				printpin(STEP3);
				printpin(DIR3);
#endif
#if AXIS_COUNT > 4
				printpin(STEP4);
				printpin(DIR4);
#endif
#if AXIS_COUNT > 5
				printpin(STEP5);
				printpin(DIR5);
#endif
			}
			
			if (tickcount > next_rtc)
			{
				mcu_rtc_cb(mcu_millis());
				next_rtc += 1000;
			}
		}
		
		//		startCycleCounter();
		__atomic_store_n(&running, false, __ATOMIC_RELAXED);
	}

/* ----- Flash filesystem shim (host FS) --------------------------------- */
#include "src/modules/file_system.h"

	static fs_t flash_fs;

	static bool flash_fs_finfo(const char *path, fs_file_info_t *finfo)
	{
		if (!path || !finfo)
			return false;

		char fpath[256];
		if (strcmp("/", path) == 0 || strcmp(".", path) == 0)
		{
			strncpy(fpath, "./*", sizeof(fpath) - 1);
			fpath[sizeof(fpath) - 1] = '\0';
		}
		else
		{
			snprintf(fpath, sizeof(fpath), "./%s", path);
		}

		WIN32_FIND_DATAA fd = {0};
		HANDLE h = FindFirstFileA(fpath, &fd);
		if (h == INVALID_HANDLE_VALUE)
			return false;

		strncpy(finfo->full_name, path, FS_PATH_NAME_MAX_LEN - 1);
		finfo->full_name[FS_PATH_NAME_MAX_LEN - 1] = '\0';

		finfo->is_dir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false;
		finfo->size = finfo->is_dir ? 0u : (uint32_t)fd.nFileSizeLow;

		FILETIME ft = fd.ftLastWriteTime;
		ULARGE_INTEGER ull;
		ull.LowPart = ft.dwLowDateTime;
		ull.HighPart = ft.dwHighDateTime;
		uint64_t fileTime = ull.QuadPart;
		fileTime -= 116444736000000000ULL;
		finfo->timestamp = (uint32_t)(fileTime / 10000000ULL);

		FindClose(h);
		return true;
	}

	static fs_file_t *flash_fs_opendir(const char *path)
	{
		fs_file_t *fp = (fs_file_t *)calloc(1, sizeof(fs_file_t));
		if (!fp)
			return NULL;
		char dir[256] = ".";
		if (strcmp("/", path))
			strncat(dir, path, sizeof(dir) - 2);

		fs_file_info_t info = {0};
		flash_fs_finfo(path, &info);
		fp->file_ptr = opendir(dir);
		if (fp->file_ptr)
		{
			memcpy(&fp->file_info, &info, sizeof(info));
			return fp;
		}
		fs_safe_free(fp);
		return NULL;
	}

	static fs_file_t *flash_fs_open(const char *path, const char *mode)
	{
		fs_file_info_t finfo = {0};
		char file[256] = ".";
		if (strcmp("/", path))
			strncat(file, path, sizeof(file) - 2);

		FILE *tmpfile = fopen(file, mode);

		if (!flash_fs_finfo(path, &finfo))
			return NULL;

		if (!finfo.is_dir)
		{
			fs_file_t *fp = (fs_file_t *)calloc(1, sizeof(fs_file_t));
			if (!fp)
				return NULL;
			fp->file_ptr = tmpfile;
			if (fp->file_ptr)
			{
				memset(fp->file_info.full_name, 0, sizeof(fp->file_info.full_name));
				fp->file_info.full_name[0] = '/';
				fp->file_info.full_name[1] = flash_fs.drive;
				fp->file_info.full_name[2] = '/';
				strncat(fp->file_info.full_name, finfo.full_name, FS_PATH_NAME_MAX_LEN - 3);
				fp->file_info.is_dir = false;
				fp->file_info.size = finfo.size;
				fp->file_info.timestamp = finfo.timestamp;
				fp->fs_ptr = &flash_fs;
				return fp;
			}
			fs_safe_free(fp);
			return NULL;
		}
		else
		{
			return flash_fs_opendir(path);
		}
	}

	static size_t flash_fs_read(fs_file_t *fp, uint8_t *buffer, size_t len)
	{
		if (fp && fp->file_ptr)
			return fread(buffer, 1, len, (FILE *)fp->file_ptr);
		return 0;
	}
	static size_t flash_fs_write(fs_file_t *fp, const uint8_t *buffer, size_t len)
	{
		if (fp && fp->file_ptr)
			return fwrite(buffer, 1, len, (FILE *)fp->file_ptr);
		return 0;
	}
	static bool flash_fs_seek(fs_file_t *fp, uint32_t position)
	{
		if (fp && fp->file_ptr)
		{
			fseek((FILE *)fp->file_ptr, (long)position, SEEK_SET);
			return true;
		}
		return false;
	}
	static int flash_fs_available(fs_file_t *fp)
	{
		if (fp && fp->file_ptr)
			return (int)(fp->file_info.size - (uint32_t)ftell((FILE *)fp->file_ptr));
		return 0;
	}
	static void flash_fs_close(fs_file_t *fp)
	{
		if (fp && fp->file_ptr)
		{
			if (fp->file_info.is_dir)
				closedir((DIR *)fp->file_ptr);
			else
				fclose((FILE *)fp->file_ptr);
		}
	}
	static bool flash_fs_remove(const char *path)
	{
		(void)path;
		if (flash_fs.drive)
			return remove(path) == 0;
		return false;
	}
	static bool flash_fs_mkdir(const char *path)
	{
		(void)path;
		if (flash_fs.drive)
			return mkdir(path) == 0;
		return false;
	}
	static bool flash_fs_rmdir(const char *path)
	{
		(void)path;
		if (flash_fs.drive)
			return rmdir(path) == 0;
		return false;
	}
	static bool flash_fs_next_file(fs_file_t *fp, fs_file_info_t *finfo)
	{
		if (fp && fp->file_ptr)
		{
			struct dirent *entry = readdir((DIR *)fp->file_ptr);
			if (entry)
			{
				flash_fs_finfo(entry->d_name, finfo);
				return true;
			}
		}
		return false;
	}

	/**
	 * OTA emulation
	 */
	void ota_server_start(void);

	/* ----- MCU init and main ------------------------------------------------ */

	static pthread_t thread_io;

	void mcu_init(void)
	{
		char cwd[1024];
		GetCurrentDirectoryA(1024, &cwd);
		printf("%s\n", cwd);

		stimuli = fopen("stimuli.vcd", "w+");
		if (stimuli)
			fprintf(stimuli, "$timescale 1us $end\n$scope module logic $end\n", tickcount);
		def_printpin(STEP0);
		def_printpin(DIR0);
		def_printpin(STEP1);
		def_printpin(DIR1);
		def_printpin(STEP2);
		def_printpin(DIR2);
		def_printpin(STEP3);
		def_printpin(DIR3);
		if (stimuli)
			fprintf(stimuli, "$upscope $end\n$enddefinitions $end\n\n", tickcount);

		startCycleCounter();
		virtualmap.special_outputs = 0;
		virtualmap.special_inputs = 0;
		virtualmap.inputs = 0;
		virtualmap.outputs = 0;

		g_cpu_freq = getCPUFreq();
		start_timer(EMULATION_MS_TICK, &ticksimul);
		pthread_create(&thread_io, NULL, &ioserver, NULL);

#ifdef MCU_HAS_UART
		memset(&g_uart, 0, sizeof(g_uart));
		g_uart.h = INVALID_HANDLE_VALUE;
		g_uart.connected = false;
		g_uart.stop = false;
		strncpy(g_uart.port_name, UART_PORT_NAME, sizeof(g_uart.port_name) - 1);
		pthread_create(&g_uart.thread, NULL, &uart_thread_fn, NULL);
#endif

		mcu_enable_global_isr();

		flash_fs.drive = 'C';
		flash_fs.open = flash_fs_open;
		flash_fs.read = flash_fs_read;
		flash_fs.write = flash_fs_write;
		flash_fs.seek = flash_fs_seek;
		flash_fs.available = flash_fs_available;
		flash_fs.close = flash_fs_close;
		flash_fs.remove = flash_fs_remove;
		flash_fs.opendir = flash_fs_opendir;
		flash_fs.mkdir = flash_fs_mkdir;
		flash_fs.rmdir = flash_fs_rmdir;
		flash_fs.next_file = flash_fs_next_file;
		flash_fs.finfo = flash_fs_finfo;
		flash_fs.next = NULL;
		fs_mount(&flash_fs);

#ifdef MCU_HAS_UART
#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE 64
#endif
		BUFFER_INIT(uint8_t, uart_tx, UART_TX_BUFFER_SIZE);
		BUFFER_INIT(uint8_t, uart_rx, RX_BUFFER_SIZE);
#endif
#ifdef MCU_HAS_UART2
#ifndef UART2_TX_BUFFER_SIZE
#define UART2_TX_BUFFER_SIZE 64
#endif
		BUFFER_INIT(uint8_t, uart2_tx, UART2_TX_BUFFER_SIZE);
		BUFFER_INIT(uint8_t, uart2_rx, RX_BUFFER_SIZE);
#endif
#ifdef MCU_HAS_USB
#ifndef USB_TX_BUFFER_SIZE
#define USB_TX_BUFFER_SIZE 64
#endif
		BUFFER_INIT(uint8_t, usb_tx, USB_TX_BUFFER_SIZE);
		BUFFER_INIT(uint8_t, usb_rx, RX_BUFFER_SIZE);
#endif
#ifdef ENABLE_SOCKETS
#ifndef WIFI_TX_BUFFER_SIZE
#define WIFI_TX_BUFFER_SIZE 64
#endif
		BUFFER_INIT(uint8_t, telnet_tx, WIFI_TX_BUFFER_SIZE);
		BUFFER_INIT(uint8_t, telnet_rx, RX_BUFFER_SIZE);
#endif
#ifdef MCU_HAS_BLUETOOTH
#ifndef BLUETOOTH_TX_BUFFER_SIZE
#define BLUETOOTH_TX_BUFFER_SIZE 64
#endif
		BUFFER_INIT(uint8_t, bt_tx, BLUETOOTH_TX_BUFFER_SIZE);
		BUFFER_INIT(uint8_t, bt_rx, RX_BUFFER_SIZE);
#endif

#if defined(ENABLE_SOCKETS) && defined(MCU_HAS_SOCKETS)
		init_winsock();
		extern socket_if_t *telnet_sock;
		extern const telnet_protocol_t telnet_proto;
		telnet_sock = telnet_start_listen(&telnet_proto, 23);
#endif

		ota_server_start();
	}

	int main(int argc, char **argv)
	{
		(void)argc;
		(void)argv;
		cnc_init();
		for (;;)
		{
			cnc_run();
		}
		return 0;
	}

	/* HAL oddities/compat */
	uint8_t itp_set_step_mode(uint8_t mode)
	{
		(void)mode;
		return 0;
	}
	uint32_t mcu_free_micros(void) { return mcu_micros() % 1000U; /* fix recursive bug */ }

	/* NVM glue to EEPROM file */
	void mcu_io_reset(void) {}
	void nvm_start_read(uint16_t address) { (void)address; }
	void nvm_start_write(uint16_t address) { (void)address; }
	uint8_t nvm_getc(uint16_t address) { return mcu_eeprom_getc(address); }
	void nvm_putc(uint16_t address, uint8_t c) { mcu_eeprom_putc(address, c); }
	void nvm_end_read(void) {}
	void nvm_end_write(void) { mcu_eeprom_flush(); }

#if defined(ENABLE_SOCKETS) && defined(MCU_HAS_SOCKETS)
	/* Link with Ws2_32.lib when building on Windows */
	/* In MinGW-w64: add -lws2_32 */

	typedef int socklen_t;

	/* Initialise Winsock 2.2 – call once at startup before using sockets */
	int init_winsock(void)
	{
		WSADATA wsaData;
		return WSAStartup(MAKEWORD(2, 2), &wsaData);
	}

	/* BSD-style wrappers mapped to Winsock functions */

	int bsd_socket(int domain, int type, int protocol)
	{
		return (int)WSASocket(domain, type, protocol, NULL, 0, 0);
	}

	int bsd_bind(int sockfd, const struct bsd_sockaddr_in *addr, socklen_t addrlen)
	{
		return bind(sockfd, (const struct sockaddr *)addr, addrlen);
	}

	int bsd_listen(int sockfd, int backlog)
	{
		return listen(sockfd, backlog);
	}

	int bsd_accept(int sockfd, struct bsd_sockaddr_in *addr, socklen_t *addrlen)
	{
		return accept(sockfd, (struct sockaddr *)addr, addrlen);
	}

	int bsd_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
	{
		return setsockopt(sockfd, level, optname, (const char *)optval, optlen);
	}

	/* Limited fcntl emulation for non-blocking mode */
	int bsd_fcntl(int fd, int cmd, long arg)
	{
		/* F_SETFL = 0x800; O_NONBLOCK = 0x800 in many POSIX impls */
		if (cmd == F_SETFL)
		{
			u_long mode = (arg & O_NONBLOCK) ? 1UL : 0UL;
			return ioctlsocket(fd, FIONBIO, &mode);
		}
		return -1; /* Unsupported command */
	}

	int bsd_recv(int sockfd, void *buf, size_t len, int flags)
	{
		return recv(sockfd, (char *)buf, (int)len, flags);
	}

	int bsd_send(int sockfd, const void *buf, size_t len, int flags)
	{
		return send(sockfd, (const char *)buf, (int)len, flags);
	}

	int bsd_close(int fd)
	{
		return closesocket(fd);
	}

#endif

/**
 * Emulate OTA page
 */
#include "../../../modules/net/http.h"
	// HTML form for firmware upload (simplified from ESP8266HTTPUpdateServer)
	// Request handler for GET /update
	static void ota_page_cb(int client_idx)
	{
		const char fmt[] = "text/html";
		const char updateForm[] =
			"<!DOCTYPE html><html><body>"
			"<form method='POST' action='/update' enctype='multipart/form-data'>"
			"Firmware:<br><input type='file' name='firmware'>"
			"<input type='submit' value='Update'>"
			"</form></body></html>";
		http_send_header(client_idx, "Cache-Control", "no-cache", false);
		http_send_header(client_idx, "Cache-Control", "max-age=300", false);
		http_send_str(client_idx, 200, (char *)fmt, (char *)updateForm);
		http_send(client_idx, 200, (char *)fmt, NULL, 0);
	}

	// File upload handler for POST /update
	static void ota_upload_cb(int client_idx)
	{
		http_upload_t up = http_file_upload_status(client_idx);

		if (up.status == HTTP_UPLOAD_START)
		{
			// Called once at start of upload
			proto_printf("Update start: %s\n", up.filename);
		}
		else if (up.status == HTTP_UPLOAD_PART)
		{
			// Called for each chunk
			proto_printf("Writing data: %lu bytes\r\n", up.datalen);
		}
		else if (up.status == HTTP_UPLOAD_END)
		{
			const char fmt[] = "text/plain";
			proto_printf("Update Success: %u bytes\r\n", up.datalen);
			const char suc[] = "Update Success! Rebooting...";
			http_send_str(client_idx, 200, (char *)fmt, (char *)suc);
			http_send(client_idx, 200, (char *)fmt, NULL, 0);
		}
		else if (up.status == HTTP_UPLOAD_ABORT)
		{
			proto_print("Update aborted\r\n");
		}
	}

	void ota_server_start(void)
	{
		LOAD_MODULE(http_server);
		http_add("/update", HTTP_REQ_ANY, ota_page_cb, ota_upload_cb);
	}

#ifdef __cplusplus
}
#endif
#endif /* MCU == MCU_VIRTUAL_WIN */
