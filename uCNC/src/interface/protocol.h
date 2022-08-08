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
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef PROTOCOL_H
#define PROTOCOL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../module.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

	void protocol_send_ok(void);
	void protocol_send_error(uint8_t error);
	void protocol_send_alarm(int8_t alarm);
	void protocol_send_status(void);
	void protocol_send_string(const char *__s);
	void protocol_send_feedback(const char *__s);
	void protocol_send_probe_result(uint8_t val);
	void protocol_send_gcode_coordsys(void);
	void protocol_send_gcode_modes(void);
	void protocol_send_cnc_settings(void);
	void protocol_send_start_blocks(void);
#ifdef ENABLE_EXTRA_SYSTEM_CMDS
	void protocol_send_pins_states(void);
#endif
#ifdef ENABLE_SYSTEM_INFO
	void protocol_send_cnc_info(void);
#endif

#ifdef ENABLE_PROTOCOL_MODULES
	// event_send_pins_states_handler
	DECL_EVENT_HANDLER(send_pins_states);
#endif

#ifdef __cplusplus
}
#endif

#endif
