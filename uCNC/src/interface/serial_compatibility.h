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

#define serial_putc grbl_protocol_putc
#define serial_print_str(__s) grbl_protocol_printf("%s", __s)
#define serial_print_int(num) print_int(grbl_protocol_putc, PRINT_CALLBACK, (uint32_t)(num), 0)
#define serial_print_flt(num) print_flt(grbl_protocol_putc, PRINT_CALLBACK, (float)(num), 9)
#define serial_stream_change grbl_stream_change
#define serial_stream_register grbl_stream_register
#define protocol_send_gcode_setting_line_int grbl_protocol_gcode_setting_line_int
#define protocol_send_gcode_setting_line_flt grbl_protocol_gcode_setting_line_flt
#define protocol_send_string grbl_protocol_puts
#define protocol_send_feedback grbl_protocol_print(MSG_START);grbl_protocol_puts(__s);grbl_protocol_print(MSG_END)

// Debugging

#define DEBUG_PUTC(c) DBGMSG("%c", c)
#define DEBUG_STR(__s) DBGMSG(__s)

#ifdef __cplusplus
}
#endif

#endif