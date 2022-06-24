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

#define TOOLDEF __attribute__((used))

#include "tool_helper.h"
#include <stdbool.h>
#include <stdint.h>

	typedef void (*tool_func)(void);
	typedef uint16_t (*tool_func_getspeed)(void);
	typedef void (*tool_spindle_func)(uint8_t, bool);
	typedef void (*tool_coolant_func)(uint8_t);
	typedef int16_t (*tool_pid_err_func)(void);
	typedef void (*tool_pid_upd_func)(int16_t);

	typedef struct
	{
		tool_func startup_code;		   /*runs any custom code after the tool is loaded*/
		tool_func shutdown_code;	   /*runs any custom code before the tool is unloaded*/
		tool_spindle_func set_speed;   /*sets the speed/power of the tool*/
		tool_coolant_func set_coolant; /*enables/disables the coolant*/
		tool_func_getspeed get_speed;  /*gets the tool speed/power*/
		tool_pid_upd_func pid_update;  /*runs de PID update code needed to keep the tool at the desired speed/power*/
		tool_pid_err_func pid_error;   /*runs de PID update code needed to keep the tool at the desired speed/power*/
	} tool_t;

	void tool_init(void);
	void tool_change(uint8_t tool);
	void tool_set_speed(int16_t value);
	void tool_set_coolant(uint8_t value);
	uint16_t tool_get_speed(void);
	void tool_stop(void);
	void tool_pid_update(int16_t value);
	int16_t tool_pid_error(void);

#ifdef __cplusplus
}
#endif

#endif
