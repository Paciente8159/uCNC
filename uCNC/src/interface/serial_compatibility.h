/*
	Name: serial_compatibility.h
	Description: Compatibility for previous version 1.10 and lower for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 01-10-2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef SERIAL_COMPATIBILITY_H
#define SERIAL_COMPATIBILITY_H

#ifdef __cplusplus
extern "C"
{
#endif

#define DECL_SERIAL_STREAM DECL_GRBL_STREAM
#define serial_getc grbl_stream_getc
#define serial_putc proto_putc
#define serial_print_str(__s) proto_printf("%s", __s)
#define print_int(cb, num) prt_int(cb, PRINT_CALLBACK, (uint32_t)(num), 0)
#define print_flt(cb, num) prt_flt(cb, PRINT_CALLBACK, (float)(num), 9)
#define serial_print_int(num) print_int(proto_putc, num)
#define serial_print_flt(num) print_flt(proto_putc, num)
#define serial_stream_change grbl_stream_change
#define serial_stream_register grbl_stream_register
#define serial_stream_readonly grbl_stream_readonly
#define protocol_send_gcode_setting_line_int proto_gcode_setting_line_int
#define protocol_send_gcode_setting_line_flt proto_gcode_setting_line_flt
#define protocol_send_string proto_puts
#define protocol_send_feedback(__s) proto_print(MSG_START);proto_puts(__s);proto_print(MSG_END)

// Debugging

#define DEBUG_PUTC(c) DBGMSG("%c", c)
#define DEBUG_STR(__s) DBGMSG(__s)

// Events

#define protocol_send_status_delegate_event_ proto_status_delegate_event_
#define protocol_send_status_delegate_event_t proto_status_delegate_event_t
#define protocol_send_status_delegate proto_status_delegate
#define protocol_send_status_event proto_status_event
#define protocol_send_status proto_status

#define protocol_send_cnc_settings_delegate_event_ proto_cnc_settings_delegate_event_
#define protocol_send_cnc_settings_delegate_event_t proto_cnc_settings_delegate_event_t
#define protocol_send_cnc_settings_delegate proto_cnc_settings_delegate
#define protocol_send_cnc_settings_event proto_cnc_settings_event
#define protocol_send_cnc_settings proto_cnc_settings

#define protocol_send_cnc_info_delegate_event_ proto_cnc_info_delegate_event_
#define protocol_send_cnc_info_delegate_event_t proto_cnc_info_delegate_event_t
#define protocol_send_cnc_info_delegate proto_cnc_info_delegate
#define protocol_send_cnc_info_event proto_cnc_info_event
#define protocol_send_cnc_info proto_cnc_info

#define protocol_send_pins_states_delegate_event_ proto_cnc_pins_states_delegate_event_
#define protocol_send_pins_states_delegate_event_t proto_cnc_pins_states_delegate_event_t
#define protocol_send_pins_states_delegate proto_cnc_pins_states_delegate
#define protocol_send_pins_states_event proto_cnc_pins_states_event
#define protocol_send_pins_states proto_cnc_pins_states

#define protocol_send_gcode_modes_delegate_event_ proto_gcode_modes_delegate_event_
#define protocol_send_gcode_modes_delegate_event_t proto_gcode_modes_delegate_event_t
#define protocol_send_gcode_modes_delegate proto_gcode_modes_delegate
#define protocol_send_gcode_modes_event proto_gcode_modes_event
#define protocol_send_gcode_modes proto_gcode_modes

#ifdef __cplusplus
}
#endif

#endif