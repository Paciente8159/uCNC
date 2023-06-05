/*
	Name: serial.c
	Description: Serial communication basic read/write functions µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 30/12/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include <math.h>
#include "../cnc.h"

static unsigned char serial_rx_buffer[RX_BUFFER_SIZE];
static uint8_t serial_rx_read;
static volatile uint8_t serial_rx_write;
static volatile uint8_t serial_rx_overflow;
#ifndef ENABLE_SYNC_TX
static unsigned char serial_tx_buffer[TX_BUFFER_SIZE];
static volatile uint8_t serial_tx_read;
static uint8_t serial_tx_write;
#endif

static uint8_t serial_read_select;
static uint16_t serial_read_index;

// static void serial_rx_clear();

void serial_init(void)
{
#ifdef FORCE_GLOBALS_TO_0
	serial_rx_write = 0;
	serial_rx_read = 0;
	memset(serial_rx_buffer, 0, sizeof(serial_rx_buffer));

#ifndef ENABLE_SYNC_TX
	serial_tx_read = 0;
	serial_tx_write = 0;
	memset(serial_tx_buffer, 0, sizeof(serial_tx_buffer));
#endif
#endif
}

bool serial_rx_is_empty(void)
{
	switch (serial_read_select)
	{
	case SERIAL_UART:
		return (serial_rx_write == serial_rx_read);
	case SERIAL_N0:
	case SERIAL_N1:
		return false;
	}

	return true;
}

unsigned char serial_getc(void)
{
	unsigned char c;
	switch (serial_read_select)
	{
	case SERIAL_UART:
		while (serial_rx_write == serial_rx_read)
		{
			cnc_dotasks();
		}

		c = serial_rx_buffer[serial_rx_read];
		if (++serial_rx_read == RX_BUFFER_SIZE)
		{
			serial_rx_read = 0;
		}

		switch (c)
		{
		case '\r':
		case '\n':
		case EOL:
			return EOL;
		case '\t':
			return ' ';
		}

		return c;
		break;
	case SERIAL_N0:
	case SERIAL_N1:
		c = mcu_eeprom_getc(serial_read_index++);
		if (c > 0 && c < 128)
		{
			serial_putc(c);
		}
		else
		{
			c = 0;
			serial_putc(':');
			serial_read_select = SERIAL_UART; // resets the serial select
		}
		return c;
	}

	return EOL;
}

void serial_ungetc(void)
{
	if (--serial_rx_read == 0xFF)
	{
		serial_rx_read = RX_BUFFER_SIZE - 1;
	}
}

void serial_select(uint8_t source)
{
	serial_read_select = source;
	switch (serial_read_select)
	{
	case SERIAL_N0:
		serial_putc('>');
		serial_read_index = STARTUP_BLOCK0_ADDRESS_OFFSET;
		break;
	case SERIAL_N1:
		serial_putc('>');
		serial_read_index = STARTUP_BLOCK1_ADDRESS_OFFSET;
		break;
	}
}

unsigned char serial_peek(void)
{
	unsigned char c;
	switch (serial_read_select)
	{
	case SERIAL_UART:
		while (serial_rx_write == serial_rx_read)
		{
			cnc_dotasks();
		}
		c = serial_rx_buffer[serial_rx_read];
		switch (c)
		{
		case '\r':
		case '\n':
		case EOL:
			return EOL;
		case '\t':
			return ' ';
		default:
			return c;
		}
		break;
	case SERIAL_N0:
	case SERIAL_N1:
		c = mcu_eeprom_getc(serial_read_index);
		return (c > 0 && c < 128) ? c : 0;
	}
	return EOL;
}

void serial_inject_cmd(const char *__s)
{
	unsigned char c;
	do
	{
		c = (unsigned char)*__s++;
		mcu_com_rx_cb(c);
	} while (c);
}

void serial_putc(unsigned char c)
{
#ifndef ENABLE_SYNC_TX
	uint8_t write = serial_tx_write;
	if (++write == TX_BUFFER_SIZE)
	{
		write = 0;
	}
	while (write == serial_tx_read)
	{
		cnc_status_report_lock = true;
		cnc_dotasks();
	} // while buffer is full

	cnc_status_report_lock = false;

	serial_tx_buffer[serial_tx_write] = c;
	serial_tx_write = write;
	if (c == '\n')
	{
		serial_flush();
	}
#else
	while (!mcu_tx_ready())
	{
		cnc_status_report_lock = true;
		cnc_dotasks();
	}
	cnc_status_report_lock = false;
	mcu_putc(c);
#if ASSERT_PIN(ACTIVITY_LED)
	mcu_toggle_output(ACTIVITY_LED);
#endif
#endif
}

void print_str(print_cb cb, const char *__s)
{
	while (*__s)
	{
		cb(*__s++);
	}
}

void print_bytes(print_cb cb, const uint8_t *data, uint8_t count)
{
	do
	{
		cb(' ');
		uint8_t up = *data >> 4;
		char c = (up > 9) ? ('a' + up - 10) : ('0' + up);
		cb(c);
		up = *data & 0x0F;
		c = (up > 9) ? ('a' + up - 10) : ('0' + up);
		cb(c);
		data++;
	} while (--count);
}

void print_int(print_cb cb, int32_t num)
{
	if (num == 0)
	{
		cb('0');
		return;
	}

	unsigned char buffer[11];
	uint8_t i = 0;

	if (num < 0)
	{
		cb('-');
		num = -num;
	}

	while (num > 0)
	{
		uint8_t digit = num % 10;
		num = (uint32_t)truncf((float)num * 0.1f);
		buffer[i++] = digit;
	}

	do
	{
		i--;
		cb('0' + buffer[i]);
	} while (i);
}

void print_flt(print_cb cb, float num)
{
	if (num < 0)
	{
		cb('-');
		num = -num;
	}

	uint32_t interger = (uint32_t)floorf(num);
	num -= interger;
	uint32_t mult = (!g_settings.report_inches) ? 1000 : 10000;
	num *= mult;
	uint32_t digits = (uint32_t)lroundf(num);
	if (digits == mult)
	{
		interger++;
		digits = 0;
	}

	print_int(cb, interger);
	cb('.');
	if (g_settings.report_inches)
	{
		if (digits < 1000)
		{
			cb('0');
		}
	}

	if (digits < 100)
	{
		cb('0');
	}

	if (digits < 10)
	{
		cb('0');
	}

	print_int(cb, digits);
}

void print_fltunits(print_cb cb, float num)
{
	num = (!g_settings.report_inches) ? num : (num * MM_INCH_MULT);
	print_flt(cb, num);
}

void print_intarr(print_cb cb, int32_t *arr, uint8_t count)
{
	do
	{
		print_int(cb, *arr++);
		count--;
		if (count)
		{
			cb(',');
		}

	} while (count);
}

void print_fltarr(print_cb cb, float *arr, uint8_t count)
{
	uint8_t i = count;
	do
	{
		print_fltunits(cb, *arr++);
		i--;
		if (i)
		{
			cb(',');
		}

	} while (i);

	if (count < 3)
	{
		i = 3 - count;
		do
		{
			cb(',');
			print_flt(cb, 0);
		} while (--i);
	}
}

void serial_flush(void)
{
#ifndef ENABLE_SYNC_TX
	if (serial_tx_write != serial_tx_read && mcu_tx_ready())
	{
		mcu_com_tx_cb();
	}
#endif
}

// ISR
// New char handle strategy
// All ascii will be sent to buffer and processed later (including comments)
#if !defined(ENABLE_MULTIBOARD) && !defined(IS_MASTER_BOARD)
MCU_RX_CALLBACK void mcu_com_rx_cb(unsigned char c)
{
	static bool is_grbl_cmd = false;
	uint8_t write;
	if (c < ((unsigned char)0x7F)) // ascii (all bellow DEL)
	{
		switch (c)
		{
		case CMD_CODE_REPORT:
#if STATUS_AUTOMATIC_REPORT_INTERVAL >= 100
			return;
#endif
		case CMD_CODE_RESET:
		case CMD_CODE_FEED_HOLD:
			cnc_call_rt_command((uint8_t)c);
			return;
		case '\n':
		case '\r':
		case 0:
			// EOL marker
			is_grbl_cmd = false;
			break;
		case '$':
			is_grbl_cmd = true;
			break;
		case CMD_CODE_CYCLE_START:
			if (!is_grbl_cmd)
			{
				cnc_call_rt_command(CMD_CODE_CYCLE_START);
				return;
			}
			break;
		}

		if (serial_rx_overflow)
		{
			c = OVF;
		}
		write = serial_rx_write;
		serial_rx_buffer[write] = c;
		if (++write == RX_BUFFER_SIZE)
		{
			write = 0;
		}
		if (write == serial_rx_read)
		{
			serial_rx_overflow++;
		}

		serial_rx_write = write;
		return;
	}
	else // extended ascii (plus CMD_CODE_CYCLE_START and DEL)
	{
		cnc_call_rt_command((uint8_t)c);
	}
}
#endif

MCU_TX_CALLBACK void mcu_com_tx_cb(void)
{
#ifndef ENABLE_SYNC_TX
	uint8_t read = serial_tx_read;
	if (read == serial_tx_write)
	{
		return;
	}

	unsigned char c = serial_tx_buffer[read];
	if (++read == TX_BUFFER_SIZE)
	{
		read = 0;
	}
	serial_tx_read = read;
	mcu_putc(c);
#if ASSERT_PIN(ACTIVITY_LED)
	mcu_toggle_output(ACTIVITY_LED);
#endif
#endif
}

void serial_rx_clear(void)
{
	serial_rx_write = 0;
	serial_rx_read = 0;
	serial_rx_overflow = 0;
	serial_rx_buffer[0] = EOL;
}

uint8_t serial_get_rx_freebytes(void)
{
	uint16_t buf = serial_rx_write;
	if (serial_rx_read > buf)
	{
		buf += RX_BUFFER_SIZE;
	}

	return (uint8_t)(RX_BUFFER_CAPACITY - (buf - serial_rx_read));
}
