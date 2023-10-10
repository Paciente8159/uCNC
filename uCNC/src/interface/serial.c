/*
	Name: serial.c
	Description: Serial communication basic read/write functions µCNC.
	The serial has been completelly redesigned to allow multiple stream sources of code.
	Streams are prority based.
	Priority is as follows
	UART
	UART2
	USB
	WIFI
	BT
	Others



	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 07-10-2023

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

static uint8_t (*stream_getc)(void);
uint8_t (*stream_available)(void);
void (*stream_clear)(void);

#ifndef DISABLE_MULTISTREAM_SERIAL
static serial_stream_t *serial_stream;
static serial_stream_t *current_stream;

#if defined(MCU_HAS_UART) && !defined(DETACH_UART_FROM_MAIN_PROTOCOL)
DECL_SERIAL_STREAM(uart_serial_stream, mcu_uart_getc, mcu_uart_available, mcu_uart_clear, mcu_uart_putc, mcu_uart_flush);
#endif
#if defined(MCU_HAS_UART2) && !defined(DETACH_UART2_FROM_MAIN_PROTOCOL)
DECL_SERIAL_STREAM(uart2_serial_stream, mcu_uart2_getc, mcu_uart2_available, mcu_uart2_clear, mcu_uart2_putc, mcu_uart2_flush);
#endif
#if defined(MCU_HAS_USB) && !defined(DETACH_USB_FROM_MAIN_PROTOCOL)
DECL_SERIAL_STREAM(usb_serial_stream, mcu_usb_getc, mcu_usb_available, mcu_usb_clear, mcu_usb_putc, mcu_usb_flush);
#endif
#if defined(MCU_HAS_WIFI) && !defined(DETACH_WIFI_FROM_MAIN_PROTOCOL)
DECL_SERIAL_STREAM(wifi_serial_stream, mcu_wifi_getc, mcu_wifi_available, mcu_wifi_clear, mcu_wifi_putc, mcu_wifi_flush);
#endif
#if defined(MCU_HAS_BLUETOOTH) && !defined(DETACH_BLUETOOTH_FROM_MAIN_PROTOCOL)
DECL_SERIAL_STREAM(bt_serial_stream, mcu_bt_getc, mcu_bt_available, mcu_bt_clear, mcu_bt_putc, mcu_bt_flush);
#endif
#else
// if not multistreaming
void (*stream_putc)(uint8_t);
void (*stream_flush)(void);

#endif

static uint8_t serial_peek_buffer;

void serial_init(void)
{
#ifdef FORCE_GLOBALS_TO_0
	serial_stream = NULL;
#endif
#ifndef DISABLE_MULTISTREAM_SERIAL
#if defined(MCU_HAS_UART) && !defined(DETACH_UART_FROM_MAIN_PROTOCOL)
	serial_stream_register(&uart_serial_stream);
#endif
#if defined(MCU_HAS_UART2) && !defined(DETACH_UART2_FROM_MAIN_PROTOCOL)
	serial_stream_register(&uart2_serial_stream);
#endif
#if defined(MCU_HAS_USB) && !defined(DETACH_USB_FROM_MAIN_PROTOCOL)
	serial_stream_register(&usb_serial_stream);
#endif
#if defined(MCU_HAS_WIFI) && !defined(DETACH_WIFI_FROM_MAIN_PROTOCOL)
	serial_stream_register(&wifi_serial_stream);
#endif
#if defined(MCU_HAS_BLUETOOTH) && !defined(DETACH_BLUETOOTH_FROM_MAIN_PROTOCOL)
	serial_stream_register(&bt_serial_stream);
#endif
current_stream = serial_stream;
#else
serial_stream_change(NULL);
#endif
}

#ifndef DISABLE_MULTISTREAM_SERIAL
void serial_stream_register(serial_stream_t *stream)
{
	if (serial_stream == NULL)
	{
		serial_stream = stream;
		serial_stream->next = NULL;
	}
	else
	{
		serial_stream_t *p = serial_stream;
		while (p->next != NULL)
		{
			p = p->next;
		}
		p->next = stream;
		p->next->next = NULL;
	}
}

// on cleanup sets the correct stdin streams
void stream_stdin(uint8_t *p)
{
	stream_getc = current_stream->stream_getc;
	stream_available = current_stream->stream_available;
	stream_clear = current_stream->stream_clear;
}
#endif

void serial_stream_change(serial_stream_t *stream)
{
#ifndef DISABLE_MULTISTREAM_SERIAL
	uint8_t cleanup __attribute__((__cleanup__(stream_stdin))) = 0;
	serial_peek_buffer = 0;
	if (stream != NULL)
	{
		current_stream = stream;
		return;
	}

	current_stream = current_stream->next;

	if (!current_stream)
	{
		current_stream = serial_stream;
	}
#else
	stream_getc = mcu_getc;
	stream_available = mcu_available;
	stream_clear = mcu_clear;
#endif
}

static uint16_t stream_eeprom_address;
static uint8_t stream_eeprom_getc(void)
{
	return mcu_eeprom_getc(stream_eeprom_address++);
}

void serial_stream_eeprom(uint16_t address)
{
	stream_eeprom_address = address;
	stream_getc = &stream_eeprom_getc;
	stream_available = NULL;
}

uint8_t serial_getc(void)
{
	uint8_t peek = serial_peek();
	serial_peek_buffer = 0;
	return peek;
}

static FORCEINLINE uint8_t _serial_peek(void)
{
	uint8_t peek = serial_peek_buffer;
	if (peek)
	{
		return peek;
	}

	while (!serial_available())
		;
	peek = stream_getc();
	// prevents null char reading from eeprom
	if(!peek) {
		peek = '\n';
	}
	serial_peek_buffer = peek;
	return peek;
}

uint8_t serial_peek(void)
{
	uint8_t peek = _serial_peek();
	switch (peek)
	{
	case '\n':
	case '\r':
	case 0:
		return EOL;
	case '\t':
		return ' ';
	}
	return peek;
}

uint8_t serial_available(void)
{
	if (!stream_available)
	{
		// if undef allow to continue
		return 1;
	}
	return stream_available();
}

uint8_t serial_freebytes(void)
{
	return (RX_BUFFER_SIZE - serial_available());
}

void serial_clear(void)
{
#ifndef DISABLE_MULTISTREAM_SERIAL
	current_stream->stream_clear();
#else
	mcu_clear();
#endif
}

#ifndef DISABLE_MULTISTREAM_SERIAL
static bool serial_broadcast_enabled;
#endif
void serial_broadcast(bool enable)
{
#ifndef DISABLE_MULTISTREAM_SERIAL
	serial_broadcast_enabled = enable;
#endif
}

static uint8_t serial_tx_count;
void serial_putc(uint8_t c)
{
	serial_tx_count++;
#ifndef DISABLE_MULTISTREAM_SERIAL
	if (!serial_broadcast_enabled)
	{
		current_stream->stream_putc(c);
	}
	else
	{
		serial_stream_t *p = serial_stream;
		while (p)
		{
			p->stream_putc(c);
			p = p->next;
		}
	}
#else
	mcu_putc(c);
#endif

	if (c == '\n')
	{
		serial_tx_count = 0;
		serial_flush();
	}
#if ASSERT_PIN(ACTIVITY_LED)
	io_toggle_output(ACTIVITY_LED);
#endif
}

void serial_flush(void)
{
#ifndef DISABLE_MULTISTREAM_SERIAL
	if (!serial_broadcast_enabled)
	{
		current_stream->stream_flush();
	}
	else
	{
		serial_stream_t *p = serial_stream;
		while (p)
		{
			p->stream_flush();
			p = p->next;
		}
	}
#else
	mcu_flush();
#endif
}

uint8_t serial_tx_busy(void)
{
	return serial_tx_count;
}

void print_str(print_cb cb, const uint8_t *__s)
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
		uint8_t c = (up > 9) ? ('a' + up - 10) : ('0' + up);
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

	uint8_t buffer[11];
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
