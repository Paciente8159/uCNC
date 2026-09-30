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
#include <sys/time.h>

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

#ifdef PIO_UNIT_TESTING

#ifndef PIO_UNIT_TESTING_TX_BUFFER_SIZE
#define PIO_UNIT_TESTING_TX_BUFFER_SIZE 65536
#endif

	DECL_BUFFER(uint8_t, unit_test_rx, RX_BUFFER_SIZE);

	/*
	 * The normal MCU ring buffer uses small embedded-target index types and is
	 * therefore not suitable for a 4096-byte host-side transcript. Keep the
	 * test transcript linear and protect it because Unity and cnc_run execute
	 * on different threads.
	 */
	static pthread_mutex_t unit_test_tx_mutex = PTHREAD_MUTEX_INITIALIZER;
	static pthread_cond_t unit_test_tx_changed = PTHREAD_COND_INITIALIZER;
	static char unit_test_tx_buffer[PIO_UNIT_TESTING_TX_BUFFER_SIZE];
	static char unit_test_tx_snapshot[PIO_UNIT_TESTING_TX_BUFFER_SIZE];
	static size_t unit_test_tx_length;
	static uint64_t unit_test_tx_base;
	static uint64_t unit_test_tx_total;
	static bool unit_test_tx_overflow;

	uint8_t mcu_unit_test_getc(void)
	{
		uint8_t c = 0;
		BUFFER_DEQUEUE(unit_test_rx, &c);
		return c;
	}
	uint8_t mcu_unit_test_available(void) { return BUFFER_READ_AVAILABLE(unit_test_rx); }
	void mcu_unit_test_clear(void) { BUFFER_CLEAR(unit_test_rx); }

	void mcu_unit_test_putc(uint8_t c)
	{
		pthread_mutex_lock(&unit_test_tx_mutex);
		if (unit_test_tx_length + 1U < sizeof(unit_test_tx_buffer))
		{
			unit_test_tx_buffer[unit_test_tx_length++] = (char)c;
			unit_test_tx_buffer[unit_test_tx_length] = '\0';
			unit_test_tx_total++;
		}
		else
		{
			unit_test_tx_overflow = true;
		}
		pthread_cond_broadcast(&unit_test_tx_changed);
		pthread_mutex_unlock(&unit_test_tx_mutex);
	}

	uint64_t mcu_unit_test_output_cursor(void)
	{
		pthread_mutex_lock(&unit_test_tx_mutex);
		uint64_t cursor = unit_test_tx_total;
		pthread_mutex_unlock(&unit_test_tx_mutex);
		return cursor;
	}

	bool mcu_unit_test_wait_for_output(uint64_t cursor, uint32_t timeout_ms)
	{
		struct timeval now;
		gettimeofday(&now, NULL);
		struct timespec deadline = {
			.tv_sec = now.tv_sec + (time_t)(timeout_ms / 1000U),
			.tv_nsec = (long)now.tv_usec * 1000L + (long)(timeout_ms % 1000U) * 1000000L};
		if (deadline.tv_nsec >= 1000000000L)
		{
			deadline.tv_sec++;
			deadline.tv_nsec -= 1000000000L;
		}

		pthread_mutex_lock(&unit_test_tx_mutex);
		while (unit_test_tx_total <= cursor)
		{
			if (pthread_cond_timedwait(&unit_test_tx_changed, &unit_test_tx_mutex, &deadline))
			{
				break;
			}
		}
		bool changed = unit_test_tx_total > cursor;
		pthread_mutex_unlock(&unit_test_tx_mutex);
		return changed;
	}

	size_t mcu_unit_test_buffer_read_since(uint64_t cursor, char *destination, size_t capacity)
	{
		if (!destination || !capacity)
		{
			return 0;
		}

		pthread_mutex_lock(&unit_test_tx_mutex);
		size_t offset = 0;
		if (cursor > unit_test_tx_base)
		{
			uint64_t relative = cursor - unit_test_tx_base;
			offset = relative < unit_test_tx_length ? (size_t)relative : unit_test_tx_length;
		}
		size_t copied = unit_test_tx_length - offset;
		if (copied >= capacity)
		{
			copied = capacity - 1U;
		}
		memcpy(destination, unit_test_tx_buffer + offset, copied);
		destination[copied] = '\0';
		pthread_mutex_unlock(&unit_test_tx_mutex);
		return copied;
	}

	void mcu_unit_test_flush(void) {}

	bool mcu_unit_test_inject(const char *cmd)
	{
		if (!cmd)
		{
			return false;
		}

		while (*cmd)
		{
			uint8_t c = (uint8_t)*cmd++;

			/*
			 * Use the same realtime-character path as normal UART input.
			 */
			if (mcu_com_rx_cb(c))
			{
				if (!BUFFER_TRY_ENQUEUE(unit_test_rx, &c))
				{
					grbl_stream_overflow(c);
					return false;
				}
			}
		}

		return true;
	}

	const char *mcu_unit_test_buffer(void)
	{
		pthread_mutex_lock(&unit_test_tx_mutex);
		memcpy(unit_test_tx_snapshot, unit_test_tx_buffer, unit_test_tx_length + 1U);
		pthread_mutex_unlock(&unit_test_tx_mutex);
		return unit_test_tx_snapshot;
	}

	size_t mcu_unit_test_buffer_read(char *destination, size_t capacity)
	{
		if (!destination || !capacity)
		{
			return 0;
		}

		pthread_mutex_lock(&unit_test_tx_mutex);
		size_t copied = unit_test_tx_length;
		if (copied >= capacity)
		{
			copied = capacity - 1U;
		}
		memcpy(destination, unit_test_tx_buffer, copied);
		destination[copied] = '\0';
		pthread_mutex_unlock(&unit_test_tx_mutex);
		return copied;
	}

	bool mcu_unit_test_buffer_overflowed(void)
	{
		pthread_mutex_lock(&unit_test_tx_mutex);
		bool overflowed = unit_test_tx_overflow;
		pthread_mutex_unlock(&unit_test_tx_mutex);
		return overflowed;
	}

	void mcu_unit_test_buffer_clear(void)
	{
		pthread_mutex_lock(&unit_test_tx_mutex);
		unit_test_tx_length = 0;
		unit_test_tx_buffer[0] = '\0';
		unit_test_tx_overflow = false;
		unit_test_tx_base = unit_test_tx_total;
		pthread_mutex_unlock(&unit_test_tx_mutex);
	}

	DECL_GRBL_STREAM(unit_test_grbl_stream, mcu_unit_test_getc, mcu_unit_test_available, mcu_unit_test_clear, mcu_unit_test_putc, mcu_unit_test_flush);

	typedef bool (*test_io_callback_t)(void);

	typedef struct
	{
		bool static_value;

		bool timed_enabled;
		bool timed_initial;
		bool timed_final;
		uint32_t timed_start_ms;
		uint32_t timed_delay_ms;

		test_io_callback_t callback;
	} test_io_signal_t;

	static test_io_signal_t g_test_io[TEST_IO_COUNT];

	bool test_io_condition(test_io_id_t input)
	{
		if (input >= TEST_IO_COUNT)
		{
			return false;
		}

		test_io_signal_t *sig = &g_test_io[input];

		if (sig->callback)
		{
			return sig->callback();
		}

		if (sig->timed_enabled)
		{
			bool elapsed =
				(uint32_t)(mcu_millis() - sig->timed_start_ms) >=
				sig->timed_delay_ms;

			return elapsed ? sig->timed_final : sig->timed_initial;
		}

		return sig->static_value;
	}

	void test_io_reset(void)
	{
		memset(g_test_io, 0, sizeof(g_test_io));
	}

	void test_io_set(test_io_id_t input, bool value)
	{
		if (input < TEST_IO_COUNT)
		{
			g_test_io[input].static_value = value;
			g_test_io[input].timed_enabled = false;
			g_test_io[input].callback = NULL;
		}
	}

	void test_io_set_after(test_io_id_t input,
						   uint32_t delay_ms,
						   bool initial_value,
						   bool final_value)
	{
		if (input < TEST_IO_COUNT)
		{
			g_test_io[input].timed_enabled = true;
			g_test_io[input].timed_initial = initial_value;
			g_test_io[input].timed_final = final_value;
			g_test_io[input].timed_start_ms = mcu_millis();
			g_test_io[input].timed_delay_ms = delay_ms;
			g_test_io[input].callback = NULL;
		}
	}

	void test_io_set_callback(test_io_id_t input,
							  test_io_callback_t cb)
	{
		if (input < TEST_IO_COUNT)
		{
			g_test_io[input].callback = cb;
			g_test_io[input].timed_enabled = false;
		}
	}

#endif /* PIO_UNIT_TEST */

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

#ifdef PIO_UNIT_TESTING
	static uint8_t unit_test_eeprom[UINT16_MAX + 1U];
#endif

	uint8_t mcu_eeprom_getc(uint16_t address)
	{
#ifdef PIO_UNIT_TESTING
		return unit_test_eeprom[address];
#else
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
#endif
	}
	void mcu_eeprom_putc(uint16_t address, uint8_t value)
	{
#ifdef PIO_UNIT_TESTING
		unit_test_eeprom[address] = value;
#else
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
#endif
	}
	void mcu_eeprom_flush(void) {}

	/* ----- IO simulation (per-pin model) ----------------------------------- */

	volatile io_pin_t io_pins[IO_PIN_COUNT];

	const io_pin_info_t io_pin_info[IO_PIN_COUNT] = {
		/* step/dir/enable */
		[1] = {"STEP0", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[2] = {"STEP1", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[3] = {"STEP2", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[4] = {"STEP3", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[5] = {"STEP4", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[6] = {"STEP5", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[7] = {"STEP6", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[8] = {"STEP7", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[9] = {"DIR0", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[10] = {"DIR1", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[11] = {"DIR2", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[12] = {"DIR3", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[13] = {"DIR4", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[14] = {"DIR5", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[15] = {"DIR6", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[16] = {"DIR7", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[17] = {"STEP0_EN", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[18] = {"STEP1_EN", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[19] = {"STEP2_EN", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[20] = {"STEP3_EN", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[21] = {"STEP4_EN", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[22] = {"STEP5_EN", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[23] = {"STEP6_EN", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		[24] = {"STEP7_EN", IO_GROUP_STEPDIR, IO_PIN_OUTPUT},
		/* pwm */
		[25] = {"PWM0", IO_GROUP_PWM_SERVO, IO_PIN_PWM},
		[26] = {"PWM1", IO_GROUP_PWM_SERVO, IO_PIN_PWM},
		[27] = {"PWM2", IO_GROUP_PWM_SERVO, IO_PIN_PWM},
		[28] = {"PWM3", IO_GROUP_PWM_SERVO, IO_PIN_PWM},
		[29] = {"PWM4", IO_GROUP_PWM_SERVO, IO_PIN_PWM},
		[30] = {"PWM5", IO_GROUP_PWM_SERVO, IO_PIN_PWM},
		[31] = {"PWM6", IO_GROUP_PWM_SERVO, IO_PIN_PWM},
		[32] = {"PWM7", IO_GROUP_PWM_SERVO, IO_PIN_PWM},
		[33] = {"PWM8", IO_GROUP_PWM_SERVO, IO_PIN_PWM},
		[34] = {"PWM9", IO_GROUP_PWM_SERVO, IO_PIN_PWM},
		[35] = {"PWM10", IO_GROUP_PWM_SERVO, IO_PIN_PWM},
		[36] = {"PWM11", IO_GROUP_PWM_SERVO, IO_PIN_PWM},
		[37] = {"PWM12", IO_GROUP_PWM_SERVO, IO_PIN_PWM},
		[38] = {"PWM13", IO_GROUP_PWM_SERVO, IO_PIN_PWM},
		[39] = {"PWM14", IO_GROUP_PWM_SERVO, IO_PIN_PWM},
		[40] = {"PWM15", IO_GROUP_PWM_SERVO, IO_PIN_PWM},
		/* servo */
		[41] = {"SERVO0", IO_GROUP_PWM_SERVO, IO_PIN_SERVO},
		[42] = {"SERVO1", IO_GROUP_PWM_SERVO, IO_PIN_SERVO},
		[43] = {"SERVO2", IO_GROUP_PWM_SERVO, IO_PIN_SERVO},
		[44] = {"SERVO3", IO_GROUP_PWM_SERVO, IO_PIN_SERVO},
		[45] = {"SERVO4", IO_GROUP_PWM_SERVO, IO_PIN_SERVO},
		[46] = {"SERVO5", IO_GROUP_PWM_SERVO, IO_PIN_SERVO},
		/* generic outputs */
		[47] = {"DOUT0", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[48] = {"DOUT1", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[49] = {"DOUT2", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[50] = {"DOUT3", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[51] = {"DOUT4", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[52] = {"DOUT5", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[53] = {"DOUT6", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[54] = {"DOUT7", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[55] = {"DOUT8", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[56] = {"DOUT9", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[57] = {"DOUT10", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[58] = {"DOUT11", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[59] = {"DOUT12", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[60] = {"DOUT13", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[61] = {"DOUT14", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[62] = {"DOUT15", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[63] = {"DOUT16", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[64] = {"DOUT17", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[65] = {"DOUT18", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[66] = {"DOUT19", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[67] = {"DOUT20", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[68] = {"DOUT21", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[69] = {"DOUT22", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[70] = {"DOUT23", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[71] = {"DOUT24", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[72] = {"DOUT25", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[73] = {"DOUT26", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[74] = {"DOUT27", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[75] = {"DOUT28", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[76] = {"DOUT29", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[77] = {"DOUT30", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[78] = {"DOUT31", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[79] = {"DOUT32", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[80] = {"DOUT33", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[81] = {"DOUT34", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[82] = {"DOUT35", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[83] = {"DOUT36", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[84] = {"DOUT37", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[85] = {"DOUT38", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[86] = {"DOUT39", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[87] = {"DOUT40", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[88] = {"DOUT41", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[89] = {"DOUT42", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[90] = {"DOUT43", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[91] = {"DOUT44", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[92] = {"DOUT45", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[93] = {"DOUT46", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[94] = {"DOUT47", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[95] = {"DOUT48", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		[96] = {"DOUT49", IO_GROUP_OUTPUT, IO_PIN_OUTPUT},
		/* control inputs */
		[100] = {"LIMIT_X", IO_GROUP_CONTROL, IO_PIN_INPUT},
		[101] = {"LIMIT_Y", IO_GROUP_CONTROL, IO_PIN_INPUT},
		[102] = {"LIMIT_Z", IO_GROUP_CONTROL, IO_PIN_INPUT},
		[103] = {"LIMIT_X2", IO_GROUP_CONTROL, IO_PIN_INPUT},
		[104] = {"LIMIT_Y2", IO_GROUP_CONTROL, IO_PIN_INPUT},
		[105] = {"LIMIT_Z2", IO_GROUP_CONTROL, IO_PIN_INPUT},
		[106] = {"LIMIT_A", IO_GROUP_CONTROL, IO_PIN_INPUT},
		[107] = {"LIMIT_B", IO_GROUP_CONTROL, IO_PIN_INPUT},
		[108] = {"LIMIT_C", IO_GROUP_CONTROL, IO_PIN_INPUT},
		[109] = {"PROBE", IO_GROUP_CONTROL, IO_PIN_INPUT},
		[110] = {"ESTOP", IO_GROUP_CONTROL, IO_PIN_INPUT},
		[111] = {"SAFETY_DOOR", IO_GROUP_CONTROL, IO_PIN_INPUT},
		[112] = {"FHOLD", IO_GROUP_CONTROL, IO_PIN_INPUT},
		[113] = {"CS_RES", IO_GROUP_CONTROL, IO_PIN_INPUT},
		/* analog inputs */
		[114] = {"ANALOG0", IO_GROUP_ANALOG, IO_PIN_ANALOG},
		[115] = {"ANALOG1", IO_GROUP_ANALOG, IO_PIN_ANALOG},
		[116] = {"ANALOG2", IO_GROUP_ANALOG, IO_PIN_ANALOG},
		[117] = {"ANALOG3", IO_GROUP_ANALOG, IO_PIN_ANALOG},
		[118] = {"ANALOG4", IO_GROUP_ANALOG, IO_PIN_ANALOG},
		[119] = {"ANALOG5", IO_GROUP_ANALOG, IO_PIN_ANALOG},
		[120] = {"ANALOG6", IO_GROUP_ANALOG, IO_PIN_ANALOG},
		[121] = {"ANALOG7", IO_GROUP_ANALOG, IO_PIN_ANALOG},
		[122] = {"ANALOG8", IO_GROUP_ANALOG, IO_PIN_ANALOG},
		[123] = {"ANALOG9", IO_GROUP_ANALOG, IO_PIN_ANALOG},
		[124] = {"ANALOG10", IO_GROUP_ANALOG, IO_PIN_ANALOG},
		[125] = {"ANALOG11", IO_GROUP_ANALOG, IO_PIN_ANALOG},
		[126] = {"ANALOG12", IO_GROUP_ANALOG, IO_PIN_ANALOG},
		[127] = {"ANALOG13", IO_GROUP_ANALOG, IO_PIN_ANALOG},
		[128] = {"ANALOG14", IO_GROUP_ANALOG, IO_PIN_ANALOG},
		[129] = {"ANALOG15", IO_GROUP_ANALOG, IO_PIN_ANALOG},
		/* generic inputs */
		[130] = {"DIN0", IO_GROUP_INPUT, IO_PIN_INPUT},
		[131] = {"DIN1", IO_GROUP_INPUT, IO_PIN_INPUT},
		[132] = {"DIN2", IO_GROUP_INPUT, IO_PIN_INPUT},
		[133] = {"DIN3", IO_GROUP_INPUT, IO_PIN_INPUT},
		[134] = {"DIN4", IO_GROUP_INPUT, IO_PIN_INPUT},
		[135] = {"DIN5", IO_GROUP_INPUT, IO_PIN_INPUT},
		[136] = {"DIN6", IO_GROUP_INPUT, IO_PIN_INPUT},
		[137] = {"DIN7", IO_GROUP_INPUT, IO_PIN_INPUT},
		[138] = {"DIN8", IO_GROUP_INPUT, IO_PIN_INPUT},
		[139] = {"DIN9", IO_GROUP_INPUT, IO_PIN_INPUT},
		[140] = {"DIN10", IO_GROUP_INPUT, IO_PIN_INPUT},
		[141] = {"DIN11", IO_GROUP_INPUT, IO_PIN_INPUT},
		[142] = {"DIN12", IO_GROUP_INPUT, IO_PIN_INPUT},
		[143] = {"DIN13", IO_GROUP_INPUT, IO_PIN_INPUT},
		[144] = {"DIN14", IO_GROUP_INPUT, IO_PIN_INPUT},
		[145] = {"DIN15", IO_GROUP_INPUT, IO_PIN_INPUT},
		[146] = {"DIN16", IO_GROUP_INPUT, IO_PIN_INPUT},
		[147] = {"DIN17", IO_GROUP_INPUT, IO_PIN_INPUT},
		[148] = {"DIN18", IO_GROUP_INPUT, IO_PIN_INPUT},
		[149] = {"DIN19", IO_GROUP_INPUT, IO_PIN_INPUT},
		[150] = {"DIN20", IO_GROUP_INPUT, IO_PIN_INPUT},
		[151] = {"DIN21", IO_GROUP_INPUT, IO_PIN_INPUT},
		[152] = {"DIN22", IO_GROUP_INPUT, IO_PIN_INPUT},
		[153] = {"DIN23", IO_GROUP_INPUT, IO_PIN_INPUT},
		[154] = {"DIN24", IO_GROUP_INPUT, IO_PIN_INPUT},
		[155] = {"DIN25", IO_GROUP_INPUT, IO_PIN_INPUT},
		[156] = {"DIN26", IO_GROUP_INPUT, IO_PIN_INPUT},
		[157] = {"DIN27", IO_GROUP_INPUT, IO_PIN_INPUT},
		[158] = {"DIN28", IO_GROUP_INPUT, IO_PIN_INPUT},
		[159] = {"DIN29", IO_GROUP_INPUT, IO_PIN_INPUT},
		[160] = {"DIN30", IO_GROUP_INPUT, IO_PIN_INPUT},
		[161] = {"DIN31", IO_GROUP_INPUT, IO_PIN_INPUT},
		[162] = {"DIN32", IO_GROUP_INPUT, IO_PIN_INPUT},
		[163] = {"DIN33", IO_GROUP_INPUT, IO_PIN_INPUT},
		[164] = {"DIN34", IO_GROUP_INPUT, IO_PIN_INPUT},
		[165] = {"DIN35", IO_GROUP_INPUT, IO_PIN_INPUT},
		[166] = {"DIN36", IO_GROUP_INPUT, IO_PIN_INPUT},
		[167] = {"DIN37", IO_GROUP_INPUT, IO_PIN_INPUT},
		[168] = {"DIN38", IO_GROUP_INPUT, IO_PIN_INPUT},
		[169] = {"DIN39", IO_GROUP_INPUT, IO_PIN_INPUT},
		[170] = {"DIN40", IO_GROUP_INPUT, IO_PIN_INPUT},
		[171] = {"DIN41", IO_GROUP_INPUT, IO_PIN_INPUT},
		[172] = {"DIN42", IO_GROUP_INPUT, IO_PIN_INPUT},
		[173] = {"DIN43", IO_GROUP_INPUT, IO_PIN_INPUT},
		[174] = {"DIN44", IO_GROUP_INPUT, IO_PIN_INPUT},
		[175] = {"DIN45", IO_GROUP_INPUT, IO_PIN_INPUT},
		[176] = {"DIN46", IO_GROUP_INPUT, IO_PIN_INPUT},
		[177] = {"DIN47", IO_GROUP_INPUT, IO_PIN_INPUT},
		[178] = {"DIN48", IO_GROUP_INPUT, IO_PIN_INPUT},
		[179] = {"DIN49", IO_GROUP_INPUT, IO_PIN_INPUT},
		/* comms (initialized but not shown in the dashboard) */
		[200] = {"TX", IO_GROUP_HIDDEN, IO_PIN_OUTPUT},
		[201] = {"RX", IO_GROUP_HIDDEN, IO_PIN_INPUT},
		[202] = {"USB_DM", IO_GROUP_HIDDEN, IO_PIN_INPUT},
		[203] = {"USB_DP", IO_GROUP_HIDDEN, IO_PIN_INPUT},
		[204] = {"SPI_CLK", IO_GROUP_HIDDEN, IO_PIN_OUTPUT},
		[205] = {"SPI_SDI", IO_GROUP_HIDDEN, IO_PIN_INPUT},
		[206] = {"SPI_SDO", IO_GROUP_HIDDEN, IO_PIN_OUTPUT},
		[207] = {"SPI_CS", IO_GROUP_HIDDEN, IO_PIN_OUTPUT},
		[208] = {"I2C_CLK", IO_GROUP_HIDDEN, IO_PIN_OUTPUT},
		[209] = {"I2C_DATA", IO_GROUP_HIDDEN, IO_PIN_OUTPUT},
		[210] = {"TX2", IO_GROUP_HIDDEN, IO_PIN_OUTPUT},
		[211] = {"RX2", IO_GROUP_HIDDEN, IO_PIN_INPUT},
	};

	static void virtual_io_pins_init(void)
	{
		for (uint16_t i = 0; i < IO_PIN_COUNT; i++)
		{
			io_pins[i].type = io_pin_info[i].default_type;
			io_pins[i].value = 0;
			io_pins[i].vcd_char = (char)(33 + i);
		}
	}

	#ifndef PIO_UNIT_TESTING
	static uint32_t virtual_outputs_signature(void)
	{
		uint32_t sig = 0;
		for (uint8_t b = 0; b < 24; b++)
		{
			if (io_pins[b + 1].value)
				sig |= (1UL << b);
		}
		return sig;
	}
#endif

	void mcu_config_input(uint8_t pin) { io_pins[pin].type = IO_PIN_INPUT; }
	void mcu_config_output(uint8_t pin)
	{
		io_pins[pin].type = (pin >= SERVO0 && pin <= SERVO5) ? IO_PIN_SERVO : IO_PIN_OUTPUT;
	}
	void mcu_config_pwm(uint8_t pin, uint16_t freq)
	{
		(void)freq;
		io_pins[pin].type = IO_PIN_PWM;
	}
	void mcu_config_analog(uint8_t pin) { io_pins[pin].type = IO_PIN_ANALOG; }

	uint8_t mcu_get_input(uint8_t pin) { return io_pins[pin].value ? 1 : 0; }
	uint8_t mcu_get_output(uint8_t pin) { return io_pins[pin].value ? 1 : 0; }

	void mcu_set_output(uint8_t pin) { io_pins[pin].value = 1; }
	void mcu_clear_output(uint8_t pin) { io_pins[pin].value = 0; }
	void mcu_toggle_output(uint8_t pin) { io_pins[pin].value = io_pins[pin].value ? 0 : 1; }

	uint16_t mcu_get_analog(uint8_t channel) { return io_pins[channel].value; }
	void mcu_set_pwm(uint8_t pwm, uint8_t value)
	{
		io_pins[pwm].type = IO_PIN_PWM;
		io_pins[pwm].value = value;
	}
	uint8_t mcu_get_pwm(uint8_t pwm) { return (uint8_t)io_pins[pwm].value; }
	void mcu_set_servo(uint8_t servo, uint8_t v)
	{
		io_pins[servo].type = IO_PIN_SERVO;
		io_pins[servo].value = v;
	}
	uint8_t mcu_get_servo(uint8_t servo) { return (uint8_t)io_pins[servo].value; }

	void mcu_enable_probe_isr(void) {}
	void mcu_disable_probe_isr(void) {}

	/* ----- Interpolator timer & timekeeping -------------------------------- */

#ifndef ITP_SAMPLE_RATE
#define ITP_SAMPLE_RATE (F_STEP_MAX * 2)
#endif

#if defined(MCU_HAS_ONESHOT_TIMER)
	extern MCU_CALLBACK mcu_timeout_delgate mcu_timeout_cb;
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
	fprintf(stimuli, "$var wire 1 %c " #X " $end\n", io_pins[X].vcd_char)
#define printpin(X) \
	if (stimuli)    \
	fprintf(stimuli, "%d%c\n", io_pins[X].value, io_pins[X].vcd_char)

	volatile unsigned long g_cpu_freq = 0;

	FILE *stimuli;
	uint64_t tickcount;

#define def_printpin(X) \
	if (stimuli)        \
	fprintf(stimuli, "$var wire 1 %c " #X " $end\n", io_pins[X].vcd_char)
#define printpin(X) \
	if (stimuli)    \
	fprintf(stimuli, "%d%c\n", io_pins[X].value, io_pins[X].vcd_char)

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

	typedef struct timed_event_
	{
		uint64_t stamp;
		void (*callback)(void *args);
		void *args;
		struct timed_event_ *next;
	} timed_event_t;

#ifdef PIO_UNIT_TESTING
	timed_event_t *current_event;

	void mcu_add_event(uint32_t delay_us, void (*callback)(void *args), void *args)
	{
		timed_event_t *new = calloc(1, sizeof(timed_event_t));
		new->stamp = tickcount + delay_us;
		new->callback = callback;
		new->args = args;

		// Insert at head
		if (!current_event || new->stamp < current_event->stamp)
		{
			new->next = current_event;
			current_event = new;
			return;
		}

		// Insert somewhere after head
		timed_event_t *prev = current_event;
		timed_event_t *next = current_event->next;

		while (next && next->stamp < new->stamp)
		{
			prev = next;
			next = next->next;
		}

		prev->next = new;
		new->next = next;
	}

	void mcu_run_events()
	{
		if (!current_event)
		{
			return;
		}

		while (current_event && current_event->stamp <= tickcount)
		{
			current_event->callback(current_event->args);
			timed_event_t *prev = current_event;
			current_event = current_event->next;
			free(prev);
		}
	}

	void mcu_test_clear_events(void)
	{
		while (current_event)
		{
			timed_event_t *prev = current_event;
			current_event = current_event->next;
			free(prev);
		}
	}
#endif

#ifdef PIO_UNIT_TESTING
	static uint64_t unit_test_next_rtc = 1000U;
	static float unit_test_partial_us;

	static void mcu_unit_test_simulate_sample(uint32_t elapsed_us)
	{
		tickcount += elapsed_us;
		mcu_run_events();
		mcu_gen_step();
#if defined(MCU_HAS_ONESHOT_TIMER)
		mcu_gen_oneshot();
#endif
		while (tickcount >= unit_test_next_rtc)
		{
			mcu_rtc_cb(mcu_millis());
			unit_test_next_rtc += 1000U;
		}
	}

	void mcu_unit_test_clock_reset(void)
	{
		mcu_test_clear_events();
		tickcount = 0;
		unit_test_next_rtc = 1000U;
		unit_test_partial_us = 0.0f;
		oneshot_alarm = 0;
#if defined(MCU_HAS_ONESHOT_TIMER)
		virtual_oneshot_counter = 0;
#endif
	}

	void mcu_unit_test_runtime_reset(void)
	{
		mcu_test_clear_events();
		unit_test_next_rtc = tickcount + 1000U;
		unit_test_partial_us = 0.0f;
		oneshot_alarm = 0;
#if defined(MCU_HAS_ONESHOT_TIMER)
		virtual_oneshot_counter = 0;
#endif
		BUFFER_CLEAR(unit_test_rx);
		mcu_unit_test_buffer_clear();
		virtual_io_pins_init();
		test_io_reset();
	}

	void mcu_unit_test_advance_time(uint32_t microseconds)
	{
		const float sample_us = 1000000.0f / (float)ITP_SAMPLE_RATE;
		uint64_t target = tickcount + microseconds;
		while (tickcount < target)
		{
			unit_test_partial_us += sample_us;
			uint32_t elapsed = (uint32_t)unit_test_partial_us;
			if (!elapsed)
			{
				continue;
			}
			unit_test_partial_us -= (float)elapsed;
			uint64_t remaining = target - tickcount;
			if ((uint64_t)elapsed > remaining)
			{
				elapsed = (uint32_t)remaining;
			}
			mcu_unit_test_simulate_sample(elapsed);
		}
	}

	void ticksimul(void)
	{
		mcu_unit_test_advance_time(EMULATION_MS_TICK * 1000U);
	}
#else
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
		if (prev ^ virtual_outputs_signature())
		{
			prev = virtual_outputs_signature();
			if (stimuli)
				fprintf(stimuli, "#%lu\n", tickcount);
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
#endif

/**
 * OTA emulation
 */
// void ota_server_start(void);
#include "../../../modules/flash_update.h"
	FILE *otafile;

	size_t virtual_get_flash_size() { return 1000000; /*just to allow the upload*/ }

	bool virtual_flash_begin(size_t filesize)
	{
		otafile = fopen("firmware_file", "wb+");
		return (otafile != NULL);
	}

	size_t virtual_flash_write(uint8_t *data, size_t len)
	{
		return fwrite(data, len, 1, otafile);
	}

	bool virtual_flash_end(bool flush)
	{
		if (!otafile)
			return false;
		if (flush)
			fflush(otafile);
		fclose(otafile);
		otafile = NULL;
		return true;
	}

	void virtual_restart() {}

	static flash_udpate_t virtual_flashupdate = {.get_flash_size = virtual_get_flash_size, .flash_begin = virtual_flash_begin, .flash_write = virtual_flash_write, .flash_end = virtual_flash_end, .device_restart = virtual_restart};

	/* ----- MCU init and main ------------------------------------------------ */

void mcu_usb_init() {}
	void mcu_uart_init() {}
	void mcu_uart2_init() {}
	// emulate flash update

	void mcu_network_init()
	{
#if defined(ENABLE_SOCKETS)
		extern int socket_init(void);
		socket_init();
		extern socket_device_t wifi_socket;
		socket_register_device(&wifi_socket);
		// ota_server_start();
		flash_update_register(&virtual_flashupdate);
#endif
	}

	#ifndef PIO_UNIT_TESTING
	/* ----- HTTP IO dashboard (localhost:VIRTUAL_HTTP_PORT) ------------------ */

	extern int virtual_http_open(uint16_t port);
	extern int virtual_http_accept(int fd);
	extern int virtual_http_recv(int fd, char *buf, int len);
	extern int virtual_http_send(int fd, const char *buf, int len);
	extern void virtual_http_close(int fd);

	static pthread_t thread_http;

	static const char http_html[] =
		"<!doctype html><html><head><meta charset='utf-8'><title>uCNC IO</title>\n"
		"<style>\n"
		"body{font-family:sans-serif;background:#111;color:#eee;margin:1em}\n"
		"h1{font-size:1.2em}h2{font-size:1em;border-bottom:1px solid #444;margin:0 0 .4em}\n"
		".grid{display:flex;flex-wrap:wrap;gap:1.25rem 2rem;align-items:flex-start;margin-bottom:1.25rem}\n"
		".group{flex:1 1 14rem;min-width:14rem;max-width:24rem}\n"
		".group.cols{display:grid;grid-template-columns:1fr 1fr;column-gap:1.5rem;flex-basis:26rem;min-width:26rem;max-width:48rem}\n"
		".group.cols h2{grid-column:1 / -1}\n"
		".pin{display:flex;align-items:center;gap:.6em;margin:.15em 0}\n"
		".lab{width:7em;font-family:monospace}.led{width:.9em;height:.9em;border-radius:50%;display:inline-block;background:#333}\n"
		".on{background:#0c0}.off{background:#500}input[type=range]{width:12em}\n"
		"#status{display:flex;flex-wrap:wrap;gap:.25em 1.5em;font-family:monospace;font-size:.85em;background:#1a1a1a;border:1px solid #333;border-radius:4px;padding:.5em .8em;margin-bottom:1em}\n"
		".stat{white-space:nowrap}\n"
		"</style></head><body><h1>uCNC IO pins</h1><div id='status'></div><div id='outputs' class='grid'></div><div id='inputs' class='grid'></div><script>\n"
		"const T=['','Step/Dir','PWM/Servo','Generic outputs','Control inputs','Generic inputs','Analog inputs'];\n"
		"const setPin=async(pin,value)=>{await fetch('/api/input',{method:'POST',body:`{\"pin\":${pin},\"value\":${value}}`});};\n"
		"const byGroup=pins=>{const g=[[],[],[],[],[],[],[]];for(const p of pins)if(p.group>0&&p.group<7)g[p.group].push(p);return g;};\n"
		"const outLine=p=>`<div class='pin'><span class='led ${p.value?'on':'off'}'></span><span class='lab'>${p.label}</span><span>${p.value}</span></div>`;\n"
		"const inLine=(p,a)=>a?`<div class='pin'><span class='lab'>${p.label}</span><input type='range' min='0' max='1023' value='0' data-pin='${p.pin}'><span class='val'>0</span></div>`:`<label class='pin'><input type='checkbox' data-pin='${p.pin}'><span class='lab'>${p.label}</span></label>`;\n"
		"async function buildInputs(){const pins=await (await fetch('/api/state')).json();const g=byGroup(pins);let html='';for(let i=4;i<=6;i++){if(!g[i].length)continue;html+=`<section class='group${i===5?' cols':''}'><h2>${T[i]}</h2>`;for(const p of g[i])html+=inLine(p,i===6);html+='</section>';}document.getElementById('inputs').innerHTML=html;document.querySelectorAll('#inputs input[type=checkbox]').forEach(e=>e.onchange=()=>setPin(e.dataset.pin,e.checked?1:0));document.querySelectorAll('#inputs input[type=range]').forEach(e=>{e.oninput=()=>e.nextElementSibling.textContent=e.value;e.onchange=()=>setPin(e.dataset.pin,e.value);});}\n"
		"async function refreshOutputs(){const pins=await (await fetch('/api/state')).json();const g=byGroup(pins);let html='';for(let i=1;i<=3;i++){if(!g[i].length)continue;html+=`<section class='group${i===3?' cols':''}'><h2>${T[i]}</h2>`;for(const p of g[i])html+=outLine(p);html+='</section>';}document.getElementById('outputs').innerHTML=html;}\n"
		"async function refreshInfo(){const j=await (await fetch('/api/info')).json();const ax=j.axes.map(a=>a.name+':'+a.pos).join(' ');const st=j.steps.map(s=>s.name+':'+s.pos).join(' ');document.getElementById('status').innerHTML=`<span class='stat'>time ${j.time} us</span><span class='stat'>axes ${ax}</span><span class='stat'>steps ${st}</span>`;}\n"
		"buildInputs();refreshOutputs();refreshInfo();setInterval(refreshOutputs,100);setInterval(refreshInfo,200);\n"
		"</script></body></html>\n";

	static void http_respond(int fd, const char *status, const char *ctype, const char *body)
	{
		char hdr[512];
		int n = snprintf(hdr, sizeof(hdr), "HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", status, ctype, (int)strlen(body));
		virtual_http_send(fd, hdr, n);
		virtual_http_send(fd, body, (int)strlen(body));
	}

	static void http_state_json(char *buf, size_t cap)
	{
		size_t n = 0;
		bool first = true;
		n += snprintf(buf + n, cap - n, "[");
		for (uint16_t i = 1; i < IO_PIN_COUNT; i++)
		{
			if (io_pin_info[i].group == IO_GROUP_HIDDEN)
				continue;
			n += snprintf(buf + n, cap - n, "%s{\"pin\":%u,\"label\":\"%s\",\"group\":%u,\"type\":%u,\"value\":%u}",
						  first ? "" : ",", (unsigned)i, io_pin_info[i].label,
						  (unsigned)io_pin_info[i].group, (unsigned)io_pins[i].type, (unsigned)io_pins[i].value);
			first = false;
		}
		snprintf(buf + n, cap - n, "]");
	}

	static void http_info_json(char *buf, size_t cap)
	{
		int32_t steps[STEPPER_COUNT];
		float axes[AXIS_COUNT];
		itp_get_rt_position(steps);
		kinematics_steps_to_coordinates(steps, axes);

		size_t n = 0;
		n += snprintf(buf + n, cap - n, "{\"time\":%lu,\"axes\":[", (unsigned long)mcu_micros());
		for (uint8_t i = 0; i < AXIS_COUNT; i++)
		{
			char name = (i < 3) ? (char)('X' + i) : (char)('A' + (i - 3));
			n += snprintf(buf + n, cap - n, "%s{\"name\":\"%c\",\"pos\":%.3f}", i ? "," : "", name, (double)axes[i]);
		}
		n += snprintf(buf + n, cap - n, "],\"steps\":[");
		for (uint8_t i = 0; i < STEPPER_COUNT; i++)
		{
			n += snprintf(buf + n, cap - n, "%s{\"name\":\"STEP%d\",\"pos\":%ld}", i ? "," : "", (int)i, (long)steps[i]);
		}
		snprintf(buf + n, cap - n, "]}");
	}

	static void http_handle_input(int fd, const char *body)
	{
		int pin = -1;
		int value = -1;
		if (sscanf(body, "{\"pin\":%d,\"value\":%d}", &pin, &value) != 2 || pin < 1 || pin >= IO_PIN_COUNT)
		{
			http_respond(fd, "400 Bad Request", "application/json", "{\"error\":\"bad request\"}");
			return;
		}
		io_pin_group_t grp = io_pin_info[pin].group;
		if (grp == IO_GROUP_CONTROL || grp == IO_GROUP_INPUT)
		{
			io_pins[pin].type = IO_PIN_INPUT;
			io_pins[pin].value = value ? 1 : 0;
			if (pin == PROBE)
				mcu_probe_changed_cb();
			else if (pin >= LIMIT_X && pin <= LIMIT_C)
				mcu_limits_changed_cb();
			else if (pin >= ESTOP && pin <= CS_RES)
				mcu_controls_changed_cb();
			else
				mcu_inputs_changed_cb();
			http_respond(fd, "200 OK", "application/json", "{\"ok\":true}");
		}
		else if (grp == IO_GROUP_ANALOG)
		{
			if (value < 0)
				value = 0;
			else if (value > 1023)
				value = 1023;
			io_pins[pin].type = IO_PIN_ANALOG;
			io_pins[pin].value = (uint16_t)value;
			http_respond(fd, "200 OK", "application/json", "{\"ok\":true}");
		}
		else
		{
			http_respond(fd, "400 Bad Request", "application/json", "{\"error\":\"not an input\"}");
		}
	}

	static char *http_find_content_length(char *req)
	{
		const char needle[] = "content-length:";
		size_t n = sizeof(needle) - 1;
		for (char *p = req; *p; p++)
		{
			size_t i = 0;
			while (i < n && p[i] && (p[i] | 0x20) == needle[i])
				i++;
			if (i == n)
				return p + n;
		}
		return NULL;
	}

	static int http_read_request(int client, char *req, int cap)
	{
		int total = 0;
		int need = -1; /* -1 = waiting for header terminator */

		while (total < cap - 1)
		{
			int r = virtual_http_recv(client, req + total, cap - 1 - total);
			if (r <= 0)
				break;
			total += r;
			req[total] = 0;

			char *hend = strstr(req, "\r\n\r\n");
			if (!hend)
				continue;

			if (need < 0)
			{
				char *cl = http_find_content_length(req);
				need = cl ? atoi(cl) : 0;
			}
			if (total - (int)(hend + 4 - req) >= need)
				break;
		}
		return total;
	}

	static void *httpd(void *args)
	{
		(void)args;
		int server = virtual_http_open(VIRTUAL_HTTP_PORT);
		if (server < 0)
		{
			printf("HTTP dashboard: failed to bind 127.0.0.1:%d\n", VIRTUAL_HTTP_PORT);
			return NULL;
		}
		printf("HTTP dashboard: http://127.0.0.1:%d\n", VIRTUAL_HTTP_PORT);
		for (;;)
		{
			int client = virtual_http_accept(server);
			if (client < 0)
				continue;
			char req[4096];
			int got = http_read_request(client, req, (int)sizeof(req));
			if (got > 0)
			{
				if (strncmp(req, "GET /api/state", 14) == 0)
				{
					char json[16384];
					http_state_json(json, sizeof(json));
					http_respond(client, "200 OK", "application/json", json);
				}
				else if (strncmp(req, "POST /api/input", 15) == 0)
				{
					char *body = strstr(req, "\r\n\r\n");
					http_handle_input(client, body ? body + 4 : "");
				}
				else if (strncmp(req, "GET /api/info", 13) == 0)
				{
					char json[2048];
					http_info_json(json, sizeof(json));
					http_respond(client, "200 OK", "application/json", json);
				}
				else if (strncmp(req, "GET ", 4) == 0)
				{
					http_respond(client, "200 OK", "text/html; charset=utf-8", http_html);
				}
				else
				{
					http_respond(client, "404 Not Found", "text/plain", "not found");
				}
			}
			virtual_http_close(client);
		}
	}
#endif

	extern void get_current_dir(char *cwd, size_t len);
	extern int start_timer(int mSec, void (*timer_func_handler)(void));
	extern void flash_fs_init(void);

	void mcu_init(void)
	{
		virtual_io_pins_init();

#ifndef PIO_UNIT_TESTING
		char cwd[1024];
		get_current_dir(cwd, 1024);
		printf("%s\n", cwd);

		printf("Creating simuli file\n\r");
		stimuli = fopen("stimuli.vcd", "w+");
		if (stimuli)
			fprintf(stimuli, "$timescale 1us $end\n$scope module logic $end\n");
		def_printpin(STEP0);
		def_printpin(DIR0);
		def_printpin(STEP1);
		def_printpin(DIR1);
		def_printpin(STEP2);
		def_printpin(DIR2);
		def_printpin(STEP3);
		def_printpin(DIR3);
		if (stimuli)
			fprintf(stimuli, "$upscope $end\n$enddefinitions $end\n\n");
#else
	stimuli = NULL;
#endif

#ifndef PIO_UNIT_TESTING
		start_timer(EMULATION_MS_TICK, &ticksimul);
		pthread_create(&thread_http, NULL, &httpd, NULL);
#else
	mcu_unit_test_clock_reset();
#endif

#if defined(MCU_HAS_UART) && !defined(PIO_UNIT_TESTING)
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

#ifdef PIO_UNIT_TESTING
		BUFFER_INIT(uint8_t, unit_test_rx, RX_BUFFER_SIZE);
		mcu_unit_test_buffer_clear();
		grbl_stream_register(&unit_test_grbl_stream);
#endif

		mcu_enable_global_isr();
#ifndef PIO_UNIT_TESTING
		flash_fs_init();
		// ota_server_start();
		flash_update_register(&virtual_flashupdate);
#endif
	}

#ifndef PIO_UNIT_TESTING
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
#endif

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
	// #ifndef OTA_URI
	// #define OTA_URI "/update"
	// #endif

	// #include "../../../modules/net/http.h"
	// 	// HTML form for firmware upload (simplified from ESP8266HTTPUpdateServer)
	// 	// Request handler for GET /update
	// 	static void ota_page_cb(int client_idx)
	// 	{
	// 		const char fmt[] = "text/html";
	// 		const char updateForm[] =
	// 			"<!DOCTYPE html><html><body>"
	// 			"<form method='POST' action='" OTA_URI "' enctype='multipart/form-data'>"
	// 			"Firmware:<br><input type='file' name='firmware'>"
	// 			"<input type='submit' value='Update'>"
	// 			"</form></body></html>";
	// 		http_send_header(client_idx, "Cache-Control", "no-cache", false);
	// 		http_send_header(client_idx, "Cache-Control", "max-age=300", false);
	// 		http_send_str(client_idx, 200, (char *)fmt, (char *)updateForm);
	// 		http_send(client_idx, 200, (char *)fmt, NULL, 0);
	// 	}

	// 	FILE *otafile;

	// 	// File upload handler for POST /update
	// 	static void ota_upload_cb(int client_idx)
	// 	{
	// 		http_upload_t up = http_file_upload_status(client_idx);
	// 		static uint32_t received_bytes = 0;

	// 		if (up.status == HTTP_UPLOAD_START)
	// 		{
	// 			otafile = fopen(up.filename, "wb+");
	// 			// Called once at start of upload
	// 			printf("Update start: %s\n", up.filename);
	// 			received_bytes = 0;
	// 		}
	// 		else if (up.status == HTTP_UPLOAD_PART)
	// 		{
	// 			// Called for each chunk
	// 			fwrite(up.data, up.datalen, 1, otafile);
	// 			received_bytes += up.datalen;
	// 			printf("Writing data: %u/%u/%u bytes\r\n", up.datalen, received_bytes, up.filelen);
	// 		}
	// 		else if (up.status == HTTP_UPLOAD_END)
	// 		{
	// 			fflush(otafile);
	// 			fclose(otafile);
	// 			otafile = NULL;
	// 			const char fmt[] = "text/plain";
	// 			printf("Update Success: %u/%u bytes\r\n", received_bytes, up.filelen);
	// 			const char suc[] = "Update Success! Rebooting...";
	// 			http_send_str(client_idx, 200, (char *)fmt, (char *)suc);
	// 			http_send(client_idx, 200, (char *)fmt, NULL, 0);
	// 		}
	// 		else if (up.status == HTTP_UPLOAD_ABORT)
	// 		{
	// 			fflush(otafile);
	// 			fclose(otafile);
	// 			otafile = NULL;
	// 			printf("Update aborted\r\n");
	// 		}
	// 	}

	// 	void ota_server_start(void)
	// 	{
	// 		LOAD_MODULE(http_server);
	// 		http_add(OTA_URI, HTTP_REQ_ANY, ota_page_cb, ota_upload_cb);
	// 	}

#ifdef __cplusplus
}
#endif
#endif /* MCU == MCU_VIRTUAL_WIN */
