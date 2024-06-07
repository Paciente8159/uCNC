/*
	Name: mcu_virtual.cpp
	Description: Simulates and MCU that runs on a Windows PC. This is mainly used to test/simulate µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 13/11/2023

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/
#include "../../uCNC/src/cnc.h"
#if (BOARD == BOARD_VIRTUAL)

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>
// #include <winsock2.h>
// #include <ws2tcpip.h>
// #pragma comment(lib, "ws2_32.lib") // Winsock Library
// #include <windows.h>
#include "WindowsSerial.h"

	/**
	 *
	 *
	 * ISR emulation
	 *
	 * **/
	volatile bool global_isr_enabled = false;
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

/**
 *
 *
 * Communications emulation
 * UART -> PC COM
 * UART2 -> console
 *
 * **/
#ifdef MCU_HAS_UART
	WindowsSerial Serial = WindowsSerial(UART_PORT_NAME);
#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE 64
#endif
	DECL_BUFFER(uint8_t, uart_tx, UART_TX_BUFFER_SIZE);
	DECL_BUFFER(uint8_t, uart_rx, RX_BUFFER_SIZE);

	uint8_t mcu_uart_getc(void)
	{
		uint8_t c = 0;
		BUFFER_DEQUEUE(uart_rx, &c);
		return c;
	}

	uint8_t mcu_uart_available(void)
	{
		return BUFFER_READ_AVAILABLE(uart_rx);
	}

	void mcu_uart_clear(void)
	{
		BUFFER_CLEAR(uart_rx);
	}

	void mcu_uart_putc(uint8_t c)
	{
		while (BUFFER_FULL(uart_tx))
		{
			mcu_uart_flush();
		}
		BUFFER_ENQUEUE(uart_tx, &c);
	}

	void mcu_uart_flush(void)
	{
		while (!BUFFER_EMPTY(uart_tx))
		{
			uint8_t tmp[UART_TX_BUFFER_SIZE + 1];
			memset(tmp, 0, sizeof(tmp));
			uint8_t r = 0;

			BUFFER_READ(uart_tx, tmp, UART_TX_BUFFER_SIZE, r);
			Serial.WriteData(tmp, r);
		}
	}

	void mcu_uart_process()
	{
		char buff[RX_BUFFER_SIZE];

		int count = Serial.ReadData(buff, RX_BUFFER_SIZE);
		for (int i = 0; i < count; i++)
		{
			uint8_t c = buff[i];
			if (mcu_com_rx_cb(c))
			{
				if (BUFFER_FULL(uart_rx))
				{
					c = OVF;
				}
				BUFFER_ENQUEUE(uart_rx, &c);
			}
		}
	}
#endif

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

	uint8_t mcu_uart2_available(void)
	{
		return BUFFER_READ_AVAILABLE(uart2_rx);
	}

	void mcu_uart2_clear(void)
	{
		BUFFER_CLEAR(uart2_rx);
	}

	void mcu_uart2_putc(uint8_t c)
	{
		while (BUFFER_FULL(uart2_tx))
		{
			mcu_uart2_flush();
		}
		BUFFER_ENQUEUE(uart2_tx, &c);
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
		}
	}

#include <conio.h>
	void mcu_uart2_process()
	{
		while (kbhit())
		{
			char c = getch();
			putchar(c);
			if (c == '\r')
			{
				putchar('\n');
			}
			if (mcu_com_rx_cb(c))
			{
				if (BUFFER_FULL(uart2_rx))
				{
					c = OVF;
				}
				BUFFER_ENQUEUE(uart2_rx, &c);
			}
		}
	}
#endif

	void mcu_dotasks()
	{
#ifdef MCU_HAS_UART
		mcu_uart_process();
#endif
#ifdef MCU_HAS_UART2
		mcu_uart2_process();
#endif
	}

	/**
	 *
	 *
	 * EEPROM emulation
	 * Uses a local file to store values
	 *
	 *
	 * **/
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

	/**
	 *
	 *
	 *  IO simulation and handling via named pipe for external app
	 *
	 *
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

	void *ioserver(void *args)
	{
		HANDLE hPipe;
		TCHAR chBuf[sizeof(VIRTUAL_MAP)];
		BOOL fSuccess = FALSE;
		DWORD cbRead, cbToWrite, cbWritten, dwMode;
		LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\ucncio");

		// Try to open a named pipe; wait for it, if necessary.

		while (1)
		{
			BOOL fConnected = FALSE;

			hPipe = CreateNamedPipe(
					lpszPipename,								// pipe name
					PIPE_ACCESS_DUPLEX,					// read/write access
					PIPE_TYPE_MESSAGE |					// message type pipe
							PIPE_READMODE_MESSAGE | // message-read mode
							PIPE_WAIT,							// blocking mode
					PIPE_UNLIMITED_INSTANCES,		// max. instances
					sizeof(VIRTUAL_MAP),				// output buffer size
					sizeof(VIRTUAL_MAP),				// input buffer size
					0,													// client time-out
					NULL);											// no template file

			if (hPipe == INVALID_HANDLE_VALUE)
			{
				printf("CreateNamedPipe failed, GLE=%d.\n", GetLastError());
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
				uint8_t lpvMessage[sizeof(VIRTUAL_MAP)];
				do
				{
					memcpy(lpvMessage, (void *)&virtualmap, sizeof(VIRTUAL_MAP));

					fSuccess = WriteFile(
							hPipe,			// pipe handle
							lpvMessage, // message
							cbToWrite,	// message length
							&cbWritten, // bytes written
							NULL);			// not overlapped

					if (!fSuccess)
					{
						printf("WriteFile to pipe failed. GLE=%d\n", GetLastError());
						break;
					}

					// Read from the pipe.

					fSuccess = ReadFile(
							hPipe,			// pipe handle
							lpvMessage, // buffer to receive reply
							cbToWrite,	// size of buffer
							&cbRead,		// number of bytes read
							NULL);			// not overlapped

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
					memcpy((void *)virtualmap.analog, ptr->analog, 16);

				} while (fSuccess); // repeat loop if ERROR_MORE_DATA

				if (!fSuccess)
				{
					printf("ReadFile from pipe failed. GLE=%d\n", GetLastError());
				}
			}

			CloseHandle(hPipe);
		}

		return NULL;
	}

	uint8_t mcu_get_pin_offset(uint8_t pin)
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

	uint16_t mcu_get_analog(uint8_t channel)
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

	void mcu_enable_probe_isr(void)
	{
	}
	void mcu_disable_probe_isr(void)
	{
	}

	/**
	 *
	 *
	 * Interpolator timer emulation
	 *
	 *
	 * **/

#ifndef ITP_SAMPLE_RATE
#define ITP_SAMPLE_RATE (F_STEP_MAX * 2)
#endif

#if defined(MCU_HAS_ONESHOT_TIMER)
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
				{
					mcu_timeout_cb();
				}
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
			t -= (int32_t)roundf(1000000.0f / (float)ITP_SAMPLE_RATE);
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

	/**
	 *
	 *
	 * Updates all the emulator via a timer callback
	 * Tracks time via QueryPerformanceFrequency
	 *
	 *
	 * **/
	HANDLE win_timer;
	void (*timer_func_handler_pntr)(void);
	unsigned long perf_start;
	double cyclesPerMicrosecond;
	double cyclesPerMillisecond;

	volatile unsigned long g_cpu_freq = 0;

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

	void ticksimul(void)
	{
		//		long t = stopCycleCounter();
		//		printf("Elapsed %dus\n\r", (int)((double)t / cyclesPerMicrosecond));
		for (int i = 0; i < (int)ceil(20 * ITP_SAMPLE_RATE / 1000); i++)
		{
			mcu_gen_step();
		}

		mcu_rtc_cb(mcu_millis());
		//		startCycleCounter();
	}

	/**
	 * Initialize the MCU
	 * **/
	pthread_t thread_io;
	void mcu_init(void)
	{
		startCycleCounter();
		virtualmap.special_outputs = 0;
		virtualmap.special_inputs = 0;
		virtualmap.inputs = 0;
		virtualmap.outputs = 0;
		g_cpu_freq = getCPUFreq();
		start_timer(20, &ticksimul);
		pthread_create(&thread_io, NULL, &ioserver, NULL);
		mcu_enable_global_isr();
	}

	void mcu_io_reset(void)
	{
	}

	int main(int argc, char **argv)
	{
		cnc_init();
		for (;;)
		{
			cnc_run();
		}
		return 0;
	}

	uint8_t itp_set_step_mode(uint8_t mode) { return 0; }

	uint32_t mcu_free_micros(void)
	{
		return (uint32_t)(mcu_free_micros() % 1000);
	}
#ifdef __cplusplus
}
#endif
#endif
