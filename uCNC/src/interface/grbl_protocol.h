/*
	Name: grbl_protocol.h
	Description: µCNC implementation of a Grbl compatible send-response protocol
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

#ifndef GRBL_PROTOCOL_H
#define GRBL_PROTOCOL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../module.h"
#include "grbl_print.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

// protocol->stream callback
// this is the base function call to output via stream
#define proto_putc grbl_stream_putc
#define proto_printf(fmt, ...) grbl_stream_printf(__romstr__(fmt), ##__VA_ARGS__)
	void proto_puts(const char *str);
#define proto_print(s) proto_puts(__romstr__(s))
	void proto_error(uint8_t error);
	void proto_alarm(int8_t alarm);
	void proto_status(void);
	DECL_EVENT_HANDLER(proto_status);
	void proto_feedback_fmt(const char *fmt, ...);
#define proto_feedback(__s) proto_print(MSG_FEEDBACK_START __s MSG_FEEDBACK_END)
#define proto_info(__s, ...) proto_feedback_fmt(__romstr__(__s), ##__VA_ARGS__)
#define proto_itoa(value) prt_int((void*)proto_putc, PRINT_CALLBACK, (uint32_t)(value), 0)
#define proto_ftoa(value) prt_flt((void*)proto_putc, PRINT_CALLBACK, (float)(value), ((!g_settings.report_inches) ? 3 : 5))
	void proto_probe_result(uint8_t val);
	void proto_gcode_coordsys(void);
	void proto_gcode_modes(void);
	void proto_cnc_settings(void);
	void proto_start_blocks(void);
	void proto_gcode_setting_line_int(setting_offset_t setting, uint16_t value);
	void proto_gcode_setting_line_flt(setting_offset_t setting, float value);

#ifdef ENABLE_PIN_DEBUG_EXTRA_CMD
	void proto_pins_states(void);
#endif
#ifdef ENABLE_SYSTEM_INFO
	void proto_cnc_info(bool extended);
	DECL_EVENT_HANDLER(proto_cnc_info);
#endif

#ifdef ENABLE_IO_MODULES
	// event_proto_pins_states_handler
	DECL_EVENT_HANDLER(proto_pins_states);
#endif

#ifdef ENABLE_SETTINGS_MODULES
	// event_proto_cnc_settings_handler
	DECL_EVENT_HANDLER(proto_cnc_settings);
#endif

#ifdef ENABLE_PARSER_MODULES
	// event_proto_gcode_modes_handler
	DECL_EVENT_HANDLER(proto_gcode_modes);
#endif

// this ensures portability to version 1.10 and older
#include "serial_compatibility.h"

#ifdef __cplusplus
}
#endif

#endif
