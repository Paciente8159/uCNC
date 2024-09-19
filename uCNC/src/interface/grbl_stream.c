/*
	Name: grbl_stream.h
	Description: Grbl communication stream for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 19/09/2024

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

static grbl_stream_getc_cb stream_getc;
static grbl_stream_available_cb stream_available;
static grbl_stream_clear_cb stream_clear;

#ifndef DISABLE_MULTISTREAM_SERIAL
grbl_stream_t *default_stream;
static grbl_stream_t *current_stream;

#if defined(MCU_HAS_UART) && !defined(DETACH_UART_FROM_MAIN_PROTOCOL)
DECL_GRBL_STREAM(uart_grbl_stream, mcu_uart_getc, mcu_uart_available, mcu_uart_clear, mcu_uart_putc, mcu_uart_flush);
#endif
#if defined(MCU_HAS_UART2) && !defined(DETACH_UART2_FROM_MAIN_PROTOCOL)
DECL_GRBL_STREAM(uart2_grbl_stream, mcu_uart2_getc, mcu_uart2_available, mcu_uart2_clear, mcu_uart2_putc, mcu_uart2_flush);
#endif
#if defined(MCU_HAS_USB) && !defined(DETACH_USB_FROM_MAIN_PROTOCOL)
DECL_GRBL_STREAM(usb_grbl_stream, mcu_usb_getc, mcu_usb_available, mcu_usb_clear, mcu_usb_putc, mcu_usb_flush);
#endif
#if defined(MCU_HAS_WIFI) && !defined(DETACH_WIFI_FROM_MAIN_PROTOCOL)
DECL_GRBL_STREAM(wifi_grbl_stream, mcu_wifi_getc, mcu_wifi_available, mcu_wifi_clear, mcu_wifi_putc, mcu_wifi_flush);
#endif
#if defined(MCU_HAS_BLUETOOTH) && !defined(DETACH_BLUETOOTH_FROM_MAIN_PROTOCOL)
DECL_GRBL_STREAM(bt_grbl_stream, mcu_bt_getc, mcu_bt_available, mcu_bt_clear, mcu_bt_putc, mcu_bt_flush);
#endif
#endif

static uint8_t grbl_stream_peek_buffer;

void grbl_stream_init(void)
{
#ifdef FORCE_GLOBALS_TO_0
	default_stream = NULL;
#endif
#ifndef DISABLE_MULTISTREAM_SERIAL
#if defined(MCU_HAS_UART) && !defined(DETACH_UART_FROM_MAIN_PROTOCOL)
	grbl_stream_register(&uart_grbl_stream);
#endif
#if defined(MCU_HAS_UART2) && !defined(DETACH_UART2_FROM_MAIN_PROTOCOL)
	grbl_stream_register(&uart2_grbl_stream);
#endif
#if defined(MCU_HAS_USB) && !defined(DETACH_USB_FROM_MAIN_PROTOCOL)
	grbl_stream_register(&usb_grbl_stream);
#endif
#if defined(MCU_HAS_WIFI) && !defined(DETACH_WIFI_FROM_MAIN_PROTOCOL)
	grbl_stream_register(&wifi_grbl_stream);
#endif
#if defined(MCU_HAS_BLUETOOTH) && !defined(DETACH_BLUETOOTH_FROM_MAIN_PROTOCOL)
	grbl_stream_register(&bt_grbl_stream);
#endif
#endif

	grbl_stream_change(NULL);
}

#ifndef DISABLE_MULTISTREAM_SERIAL
void grbl_stream_register(grbl_stream_t *stream)
{
	if (default_stream == NULL)
	{
		default_stream = stream;
		default_stream->next = NULL;
	}
	else
	{
		grbl_stream_t *p = default_stream;
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

#ifdef ENABLE_MULTISTREAM_GUARD
static bool grbl_stream_rx_busy;
#endif

bool grbl_stream_change(grbl_stream_t *stream)
{
#ifndef DISABLE_MULTISTREAM_SERIAL
	uint8_t cleanup __attribute__((__cleanup__(stream_stdin))) = 0;

#ifdef ENABLE_MULTISTREAM_GUARD
	if (grbl_stream_rx_busy)
	{
		return false;
	}
#endif
	grbl_stream_peek_buffer = 0;
	if (stream != NULL)
	{
		current_stream = stream;
		return true;
	}

	// starts by the prioritary and test one by one until one that as characters available is found
	current_stream = default_stream;
#else
	stream_getc = mcu_getc;
	stream_available = mcu_available;
	stream_clear = mcu_clear;
#endif
	return true;
}

bool grbl_stream_readonly(stream_getc_cb getc_cb, stream_available_cb available_cb, stream_clear_cb clear_cb)
{
#ifdef ENABLE_MULTISTREAM_GUARD
	if (grbl_stream_rx_busy)
	{
		return false;
	}
#endif

	stream_getc = getc_cb;
	stream_available = available_cb;
	stream_clear = clear_cb;
	return true;
}

static uint16_t stream_eeprom_address;
static uint8_t stream_eeprom_getc(void)
{
	uint8_t c = mcu_eeprom_getc(stream_eeprom_address++);
	grbl_stream_putc((c != EOL) ? c : ':');
	return c;
}

void grbl_stream_eeprom(uint16_t address)
{
	stream_eeprom_address = address;
	grbl_stream_readonly(&stream_eeprom_getc, NULL, NULL);
}

char grbl_stream_getc(void)
{
	uint8_t peek = grbl_stream_peek();
#ifdef ENABLE_MULTISTREAM_GUARD
	grbl_stream_rx_busy = (peek != EOL);
#endif
	grbl_stream_peek_buffer = 0;
	return peek;
}

static FORCEINLINE char _grbl_stream_peek(void)
{
	char peek = grbl_stream_peek_buffer;
	if (peek)
	{
		return peek;
	}

	while (!grbl_stream_available())
	{
		cnc_dotasks();
	}

#ifndef DISABLE_MULTISTREAM_SERIAL
	if (!stream_getc)
	{
		// if getc not defined return overflow to force multiple overflow errors
		return OVF;
	}
#endif
	peek = (char)stream_getc();
	// prevents null char reading from eeprom
	if (!peek)
	{
		peek = '\n';
	}
	grbl_stream_peek_buffer = peek;
	return peek;
}

char grbl_stream_peek(void)
{
	uint8_t peek = _grbl_stream_peek();
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

uint8_t grbl_stream_available(void)
{
	if (stream_available == NULL)
	{
		// if undef allow to continue
		return 1;
	}

#ifndef DISABLE_MULTISTREAM_SERIAL
	uint8_t count = stream_available();
	if (!count)
	{
#ifdef ENABLE_MULTISTREAM_GUARD
		if (grbl_stream_rx_busy)
		{
			return count;
		}
#endif
		grbl_stream_t *p = default_stream;
		while (p != NULL)
		{
#ifdef ENABLE_DEBUG_STREAM
			// skip the debug stream, if it differs from default_stream
			if (p != DEBUG_STREAM || p == default_stream)
			{
#endif
				count = (!(p->stream_available)) ? 0 : p->stream_available();
				if (count)
				{
					grbl_stream_change(p);
					return count;
				}
#ifdef ENABLE_DEBUG_STREAM
			}
#endif
			p = p->next;
		}
	}
	return count;
#else
	return stream_available();
#endif
}

uint8_t grbl_stream_write_available(void)
{
	return (RX_BUFFER_SIZE - grbl_stream_available());
}

void grbl_stream_clear(void)
{
#ifndef DISABLE_MULTISTREAM_SERIAL
	if (stream_clear)
	{
		stream_clear();
	}
#else
	mcu_clear();
#endif
}

#ifndef DISABLE_MULTISTREAM_SERIAL
static bool grbl_stream_broadcast_enabled;
#endif
void grbl_stream_broadcast(bool enable)
{
#ifndef DISABLE_MULTISTREAM_SERIAL
	grbl_stream_broadcast_enabled = enable;
#endif
}

static uint8_t grbl_stream_tx_count;
void grbl_stream_putc(char c)
{
	grbl_stream_tx_count++;
#ifndef DISABLE_MULTISTREAM_SERIAL
	if (!grbl_stream_broadcast_enabled)
	{
		if (current_stream->stream_putc)
		{
			current_stream->stream_putc(c);
		}
	}
	else
	{
		grbl_stream_t *p = default_stream;
		while (p)
		{
			if (p->stream_putc)
			{
				p->stream_putc((uint8_t)c);
			}
			p = p->next;
		}
	}
#else
	mcu_putc(c);
#endif

	if (c == '\n')
	{
		grbl_stream_tx_count = 0;
		grbl_stream_flush();
	}
#if ASSERT_PIN(ACTIVITY_LED)
	io_toggle_output(ACTIVITY_LED);
#endif
}

#ifdef ENABLE_DEBUG_STREAM
void debug_putc(char c)
{
	if (DEBUG_STREAM)
	{
		DEBUG_STREAM->stream_putc(c);

		if (c == '\n')
		{
			DEBUG_STREAM->stream_flush();
		}
	}
}
#endif

void grbl_stream_flush(void)
{
#ifndef DISABLE_MULTISTREAM_SERIAL
	if (!grbl_stream_broadcast_enabled)
	{
		if (current_stream->stream_flush)
		{
			current_stream->stream_flush();
		}
	}
	else
	{
		grbl_stream_t *p = default_stream;
		while (p)
		{
			if (p->stream_flush)
			{
				p->stream_flush();
			}
			p = p->next;
		}
	}
#else
	mcu_flush();
#endif
}

uint8_t grbl_stream_busy(void)
{
	return grbl_stream_tx_count;
}

void grbl_stream_print_cb(void *arg, char c)
{
	grbl_stream_putc(c);
}
