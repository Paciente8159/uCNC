/*
	Name: tool.h
	Description: The tool unit for µCNC.
        This is responsible to define and manage tools.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 16/12/2021

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef TOOL_H
#define TOOL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

	typedef uint8_t (*tool_func)(void);
	typedef void (*tool_spindle_func)(uint8_t, bool);
	typedef void (*tool_coolant_func)(uint8_t);

	typedef struct
	{
		tool_func startup_code;
		tool_func shutdown_code;
		tool_spindle_func update_spindle;
		tool_coolant_func update_coolant;
		tool_func get_spindle;
		tool_func pid_controller;
	} tool_t;

#if TOOL_COUNT > 0
	extern const tool_t tool0;
#endif

	void tool_init(void);
	void tool_change(uint8_t tool);
	void tool_update_spindle(uint8_t value, bool invert);
	void tool_update_coolant(uint8_t value);
	int tool_get_spindle(void);
	void tool_stop(void);

#ifdef __cplusplus
}
#endif

#endif
