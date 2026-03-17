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
#include "../../../cnc.h"
#if (MCU == MCU_VIRTUAL_WIN) || (MCU == MCU_VIRTUAL_LINUX)

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
#include <pthread.h>

	/* ----- Global ISR enable/disable --------------------------------------- */

	volatile bool global_isr_enabled = false;

	void mcu_enable_global_isr(void) { global_isr_enabled = true; }
	void mcu_disable_global_isr(void) { global_isr_enabled = false; }
	bool mcu_get_global_isr(void) { return global_isr_enabled; }

	extern void *ioserver(void *args);

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

	extern void serial_init(void);
	extern int serial_read(char *buffer, unsigned int nbChar);
	extern bool serial_write(const uint8_t *buffer, unsigned int nbChar);
	extern bool uart_connected(void);

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
		while (!BUFFER_TRY_ENQUEUE(uart_tx, &c) && uart_connected())
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
			if (!uart_connected())
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
	extern int console_kbhit(void);
	extern int console_getch(void);
	static void mcu_uart2_process(void)
	{
		if (console_kbhit())
		{
			int kc = console_getch();
			char c = (char)kc;
			putchar(c);
			if (c == '\r')
				putchar('\n');
			if (mcu_com_rx_cb((uint8_t)c))
			{
				if (!BUFFER_TRY_ENQUEUE(uart2_rx, &c))
				{
					STREAM_OVF(c);
				}
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

	volatile VIRTUAL_MAP virtualmap;

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
	FILE *stimuli;
	uint64_t tickcount;

#define def_printpin(X) \
	if (stimuli)        \
	fprintf(stimuli, "$var wire 1 %c " #X " $end\n", 33 + X)
#define printpin(X) \
	if (stimuli)    \
	fprintf(stimuli, "%d%c\n", ((virtualmap.special_outputs & (1 << (X - 1))) ? 1 : 0), 33 + X)

	volatile unsigned long g_cpu_freq = 0;

	FILE *stimuli;
	uint64_t tickcount;

#define def_printpin(X) \
	if (stimuli)        \
	fprintf(stimuli, "$var wire 1 %c " #X " $end\n", 33 + X)
#define printpin(X) \
	if (stimuli)    \
	fprintf(stimuli, "%d%c\n", ((virtualmap.special_outputs & (1 << (X - 1))) ? 1 : 0), 33 + X)

	void virtual_delay_us(uint16_t delay)
	{
		uint64_t start = tickcount;
		double elapsed = 0;
		do
		{
			elapsed = (tickcount - start);
		} while (elapsed < delay);
	}

	uint32_t mcu_micros(void)
	{
		return (uint32_t)tickcount;
	}

	uint32_t mcu_millis(void)
	{
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
	void ticksimul(void)
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
		float timestep = ceil((float)EMULATION_MS_TICK * ITP_SAMPLE_RATE * 0.001f);
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

	/**
	 * OTA emulation
	 */
	void ota_server_start(void);

	/* ----- MCU init and main ------------------------------------------------ */

	static pthread_t thread_io;
	void mcu_usb_init() {}
	void mcu_uart_init() {}
	void mcu_uart2_init() {}
	void mcu_network_init()
	{
#if defined(ENABLE_SOCKETS)
		extern int socket_init(void);
		socket_init();
		extern socket_device_t wifi_socket;
		socket_register_device(&wifi_socket);

		extern socket_if_t *telnet_sock;
		extern const telnet_protocol_t telnet_proto;
		telnet_sock = telnet_start_listen(&telnet_proto, 23);
		ota_server_start();
#endif
	}

	extern void get_current_dir(char *cwd, size_t len);
	extern int start_timer(int mSec, void (*timer_func_handler)(void));
	extern void flash_fs_init(void);

	void mcu_init(void)
	{
		char cwd[1024];
		get_current_dir(cwd, 1024);
		printf("%s\n", cwd);

		printf("Creating simuli file\n\r");
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

		virtualmap.special_outputs = 0;
		virtualmap.special_inputs = 0;
		virtualmap.inputs = 0;
		virtualmap.outputs = 0;

		start_timer(EMULATION_MS_TICK, &ticksimul);
		pthread_create(&thread_io, NULL, &ioserver, NULL);

#ifdef MCU_HAS_UART
		serial_init();
#endif

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
#ifndef TELNET_TX_BUFFER_SIZE
#define TELNET_TX_BUFFER_SIZE 64
#endif
		BUFFER_INIT(uint8_t, telnet_tx, TELNET_TX_BUFFER_SIZE);
		BUFFER_INIT(uint8_t, telnet_rx, RX_BUFFER_SIZE);
		mcu_network_init();
#endif
#ifdef MCU_HAS_BLUETOOTH
#ifndef BLUETOOTH_TX_BUFFER_SIZE
#define BLUETOOTH_TX_BUFFER_SIZE 64
#endif
		BUFFER_INIT(uint8_t, bt_tx, BLUETOOTH_TX_BUFFER_SIZE);
		BUFFER_INIT(uint8_t, bt_rx, RX_BUFFER_SIZE);
#endif

		mcu_enable_global_isr();
		flash_fs_init();
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

/**
 * Emulate OTA page
 */
#ifndef OTA_URI
#define OTA_URI "/update"
#endif

#include "../../../modules/net/http.h"
	// HTML form for firmware upload (simplified from ESP8266HTTPUpdateServer)
	// Request handler for GET /update
	static void ota_page_cb(int client_idx)
	{
		const char fmt[] = "text/html";
		const char updateForm[] =
			"<!DOCTYPE html><html><body>"
			"<form method='POST' action='" OTA_URI "' enctype='multipart/form-data'>"
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
		static uint32_t received_bytes = 0;

		if (up.status == HTTP_UPLOAD_START)
		{
			// Called once at start of upload
			printf("Update start: %s\n", up.filename);
			received_bytes = 0;
		}
		else if (up.status == HTTP_UPLOAD_PART)
		{
			// Called for each chunk
			received_bytes += up.datalen;
			printf("Writing data: %lu/%lu bytes\r\n", up.datalen, received_bytes);
		}
		else if (up.status == HTTP_UPLOAD_END)
		{
			const char fmt[] = "text/plain";
			printf("Update Success: %u bytes\r\n", up.datalen);
			const char suc[] = "Update Success! Rebooting...";
			http_send_str(client_idx, 200, (char *)fmt, (char *)suc);
			http_send(client_idx, 200, (char *)fmt, NULL, 0);
		}
		else if (up.status == HTTP_UPLOAD_ABORT)
		{
			printf("Update aborted\r\n");
		}
	}

	void ota_server_start(void)
	{
		LOAD_MODULE(http_server);
		http_add(OTA_URI, HTTP_REQ_ANY, ota_page_cb, ota_upload_cb);
	}

#ifdef __cplusplus
}
#endif
#endif /* MCU == MCU_VIRTUAL_WIN */
