/*
	Name: serial.h
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

#ifndef SERIAL_H
#define SERIAL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#define EOL 0x00 // end of line uint8_t
#define OVF 0x15 // overflow uint8_t
#define SAFEMARGIN 2
#ifndef RX_BUFFER_CAPACITY
#define RX_BUFFER_CAPACITY 128
#endif
#define RX_BUFFER_SIZE (RX_BUFFER_CAPACITY + SAFEMARGIN) // buffer sizes

	typedef uint8_t (*stream_getc_cb)(void);
	typedef uint8_t (*stream_available_cb)(void);
	typedef void (*stream_clear_cb)(void);

	typedef struct serial_stream_
	{
		stream_getc_cb stream_getc;
		stream_available_cb stream_available;
		stream_clear_cb stream_clear;
		void (*stream_putc)(uint8_t);
		void (*stream_flush)(void);
		struct serial_stream_ *next;
	} serial_stream_t;

#define DECL_SERIAL_STREAM(name, getc_cb, available_cb, clear_cb, putc_cb, flush_cb) serial_stream_t name = {getc_cb, available_cb, clear_cb, putc_cb, flush_cb, NULL}

	void serial_init();

	void serial_stream_register(serial_stream_t *stream);
	bool serial_stream_change(serial_stream_t *stream);
	bool serial_stream_readonly(stream_getc_cb getc_cb, stream_available_cb available_cb, stream_clear_cb clear_cb);
	void serial_stream_eeprom(uint16_t address);

	void serial_broadcast(void);
	void serial_putc(char c);
	void serial_flush(void);
	uint8_t serial_tx_busy(void);

	char serial_getc(void);
	char serial_peek(void);
	uint8_t serial_available(void);
	void serial_clear(void);
	uint8_t serial_freebytes(void);

	// printing utils
	typedef void (*print_cb)(char);
	void print_str(print_cb cb, const char *__s);
	void print_bytes(print_cb cb, const uint8_t *data, uint8_t count);
	void print_int(print_cb cb, int32_t num);
	void print_flt(print_cb cb, float num);
	void print_fltunits(print_cb cb, float num);
	void print_intarr(print_cb cb, int32_t *arr, uint8_t count);
	void print_fltarr(print_cb cb, float *arr, uint8_t count);

#define serial_print_str(__s) print_str(serial_putc, __s)
#define serial_print_bytes(data, count) print_bytes(serial_putc, data, count)
#define serial_print_int(num) print_int(serial_putc, num)
#define serial_print_flt(num) print_flt(serial_putc, num)
#define serial_print_fltunits(num) print_fltunits(serial_putc, num)
#define serial_print_intarr(arr, count) print_intarr(serial_putc, arr, count)
#define serial_print_fltarr(arr, count) print_fltarr(serial_putc, arr, count)

#ifdef ENABLE_DEBUG_STREAM
// to customize the debug stream you can reference it to an existing stream
// for example to set it to the USB stream you can define DEBUG_STREAM like this
// #define DEBUG_STREAM (&usb_serial_stream)

#ifndef DEBUG_STREAM
	extern serial_stream_t *default_stream;
#define DEBUG_STREAM default_stream
#endif

	extern void debug_putc(char c);
#define DEBUG_PUTC(c) debug_putc(c)
#define DEBUG_STR(__s) print_str(debug_putc, __s)
#define DEBUG_BYTES(data, count) print_bytes(debug_putc, data, count)
#define DEBUG_INT(num) print_int(debug_putc, num)
#define DEBUG_FLT(num) print_flt(debug_putc, num)
#define DEBUG_FLTUNITS(num) print_fltunits(debug_putc, num)
#define DEBUG_INTARR(arr, count) print_intarr(debug_putc, arr, count)
#define DEBUG_FLTARR(arr, count) print_fltarr(debug_putc, arr, count)
#else
#define DEBUG_PUTC(c)
#define DEBUG_STR(__s)
#define DEBUG_BYTES(data, count)
#define DEBUG_INT(num)
#define DEBUG_FLT(num)
#define DEBUG_FLTUNITS(num)
#define DEBUG_INTARR(arr, count)
#define DEBUG_FLTARR(arr, count)
#endif

#ifdef __cplusplus
}
#endif

#endif
