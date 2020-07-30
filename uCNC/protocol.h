/*
	Name: protocol.h
	Description: µCNC implementation of a Grbl compatible send-response protocol
	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 19/09/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

bool protocol_is_busy(void);
void protocol_send_ok(void);
void protocol_send_error(uint8_t error);
void protocol_send_alarm(uint8_t alarm);
void protocol_send_status(void);
void protocol_send_string(const unsigned char *__s);
void protocol_send_feedback(const unsigned char *__s);
void protocol_send_gcode_coordsys(void);
void protocol_send_gcode_modes(void);
void protocol_send_ucnc_settings(void);
void protocol_send_start_blocks(void);

#endif
