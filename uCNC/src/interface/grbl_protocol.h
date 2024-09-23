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
#include "print.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

// protocol->stream callback
// this is the base function call to output via stream
#define grbl_protocol_putc grbl_stream_putc
#define grbl_protocol_printf(fmt, ...) grbl_stream_printf(__romstr__(fmt), ## __VA_ARGS__)
#define grbl_protocol_print(fmt) grbl_protocol_printf(fmt)
	void grbl_protocol_error(uint8_t error);
	void grbl_protocol_alarm(int8_t alarm);
	void grbl_protocol_status(void);
	DECL_EVENT_HANDLER(grbl_protocol_status);
	void grbl_protocol_feedback_base(void *__s, uint8_t type);
#define grbl_protocol_feedback(__s) grbl_protocol_feedback_base((void*)__romstr__(__s), 0)
#define grbl_protocol_sprintf(__s) grbl_protocol_feedback_base((void*)__s, 1)
	void grbl_protocol_probe_result(uint8_t val);
	void grbl_protocol_gcode_coordsys(void);
	void grbl_protocol_gcode_modes(void);
	void grbl_protocol_cnc_settings(void);
	void grbl_protocol_start_blocks(void);
	void grbl_protocol_gcode_setting_line_int(setting_offset_t setting, uint16_t value);
	void grbl_protocol_gcode_setting_line_flt(setting_offset_t setting, float value);

#ifdef ENABLE_EXTRA_SYSTEM_CMDS
	void grbl_protocol_pins_states(void);
#endif
#ifdef ENABLE_SYSTEM_INFO
	void grbl_protocol_cnc_info(bool extended);
	DECL_EVENT_HANDLER(grbl_protocol_cnc_info);
#endif

#ifdef ENABLE_IO_MODULES
	// event_grbl_protocol_pins_states_handler
	DECL_EVENT_HANDLER(grbl_protocol_pins_states);
#endif

#ifdef ENABLE_SETTINGS_MODULES
	// event_grbl_protocol_cnc_settings_handler
	DECL_EVENT_HANDLER(grbl_protocol_cnc_settings);
#endif

#ifdef ENABLE_PARSER_MODULES
	// event_grbl_protocol_gcode_modes_handler
	DECL_EVENT_HANDLER(grbl_protocol_gcode_modes);
#endif

#ifdef __cplusplus
}
#endif

#endif
