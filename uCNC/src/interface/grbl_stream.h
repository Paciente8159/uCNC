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

#ifndef GRBL_STREAM_H
#define GRBL_STREAM_H

#ifdef __cplusplus
extern "C"
{
#endif


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

	typedef uint8_t (*grbl_stream_getc_cb)(void);
	typedef uint8_t (*grbl_stream_available_cb)(void);
	typedef void (*grbl_stream_clear_cb)(void);

	typedef struct grbl_stream_
	{
		grbl_stream_getc_cb stream_getc;
		grbl_stream_available_cb stream_available;
		grbl_stream_clear_cb stream_clear;
		void (*stream_putc)(uint8_t);
		void (*stream_flush)(void);
		struct grbl_stream_ *next;
	} grbl_stream_t;

#define DECL_GRBL_STREAM(name, getc_cb, available_cb, clear_cb, putc_cb, flush_cb) grbl_stream_t name = {getc_cb, available_cb, clear_cb, putc_cb, flush_cb, NULL}

	void grbl_stream_init();

	void grbl_stream_register(grbl_stream_t *stream);
	bool grbl_stream_change(grbl_stream_t *stream);
	bool grbl_stream_readonly(grbl_stream_getc_cb getc_cb, grbl_stream_available_cb available_cb, grbl_stream_clear_cb clear_cb);
	void grbl_stream_eeprom(uint16_t address);

	void grbl_stream_broadcast(bool enable);
	void grbl_stream_putc(char c);
	void grbl_stream_flush(void);

	char grbl_stream_getc(void);
	char grbl_stream_peek(void);
	uint8_t grbl_stream_available(void);
	void grbl_stream_clear(void);
	uint8_t grbl_stream_write_available(void);
	uint8_t grbl_stream_busy(void);

// // printing utils
// #include "print.h"
// 	void grbl_stream_print_cb(void *arg, char c);
// 	// base defines
// #define grbl_stream_printf_va(fmt, vargs) print_fmtva(grbl_stream_print_cb, NULL, fmt, false, vargs)
// #define grbl_stream_romprintf_va(fmt, vargs) print_fmtva(grbl_stream_print_cb, NULL, fmt, true, vargs)
// // derivatives
// #define grbl_stream_printf(fmt, ...) print_fmt(grbl_stream_print_cb, NULL, fmt, false, __VA_ARGS__)
// #define grbl_stream_print(fmt) print_fmt(grbl_stream_print_cb, NULL, fmt, false)
// #define grbl_stream_romprintf(fmt, ...) print_fmt(grbl_stream_print_cb, NULL, fmt, true, __VA_ARGS__)
// #define grbl_stream_romprint(fmt) print_fmt(grbl_stream_print_cb, NULL, fmt, true)

#ifdef ENABLE_DEBUG_STREAM
	// to customize the debug stream you can reference it to an existing stream
	// for example to set it to the USB stream you can define DEBUG_STREAM like this
	// #define DEBUG_STREAM (&usb_serial_stream)

#ifndef DEBUG_STREAM
	extern grbl_stream_t *default_stream;
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