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
#define FILE_EOF 0x03
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

	void grbl_stream_start_broadcast(void);
	void grbl_stream_putc(char c);
	void grbl_stream_printf(const char* fmt, ...);

	char grbl_stream_getc(void);
	char grbl_stream_peek(void);
	uint8_t grbl_stream_available(void);
	void grbl_stream_clear(void);
	uint8_t grbl_stream_write_available(void);
	uint8_t grbl_stream_busy(void);

#ifdef ENABLE_DEBUG_STREAM
	// to customize the debug stream you can reference it to an existing stream
	// for example to set it to the USB stream you can define DEBUG_STREAM like this
	// #define DEBUG_STREAM (&usb_serial_stream)

#ifndef DEBUG_STREAM
	extern grbl_stream_t *default_stream;
#define DEBUG_STREAM default_stream
#endif

#ifndef DEBUG_PRELUDE
#define DEBUG_PRELUDE "[DBG:"
#endif

// not to be used directly
void debug_printf(const char *fmt, ...);
#define DBGMSG(fmt, ...) debug_printf(__romstr__(DEBUG_PRELUDE fmt MSG_FEEDBACK_END), ##__VA_ARGS__)
#else
#define DBGMSG(fmt, ...)
#endif

#ifdef __cplusplus
}
#endif

#endif