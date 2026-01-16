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
#include "../../module.h"
#include <stdint.h>

	typedef void (*tool_func)(void);
	typedef int16_t (*tool_range_speed_func)(int16_t, uint8_t);
	typedef uint16_t (*tool_get_speed_func)(void);
	typedef void (*tool_set_speed_func)(int16_t);
	typedef void (*tool_coolant_func)(uint8_t);

	typedef struct
	{
		tool_func startup_code;			   /*runs any custom code after the tool is loaded*/
		tool_func shutdown_code;		   /*runs any custom code before the tool is unloaded*/
		tool_func pid_update;			   /*runs de PID update code needed to keep the tool at the desired speed/power*/
		tool_range_speed_func range_speed; /*converts core speed to tool speed*/
		tool_get_speed_func get_speed;	   /*gets the tool speed/power (converts from tool speed to core speed)*/
		tool_set_speed_func set_speed;	   /*sets the speed/power of the tool*/
		tool_coolant_func set_coolant;	   /*enables/disables the coolant*/
	} tool_t;

	void tool_init(void);								   // initializes tool inside µCNC
	uint8_t tool_change(uint8_t tool, uint8_t status);						   // executes a tool change µCNC. This runs the shutdown code of the current tool and then runs the startup code of the next tool
	void tool_pid_update(void);							   // if tool PID option is enabled this function is called in the main loop to update the tool or make some adjustment
	int16_t tool_get_setpoint(void);					   // return the current tool setpoint. That is the value set with S parameter in gcode
	int16_t tool_range_speed(int16_t value, uint8_t conv); // this function converts from GCode S to tool speed and back. For example if using PWM will convert from tool speed S to a PWM value 0-255 if conv=0 or vice-versa if conv=1
	uint16_t tool_get_speed(void);						   // this function returns the current tool speed. Always returns the setpoint value, unless the tool has a custom get_speed function (for example to return the true speed of a feedback sensor)
	void tool_set_speed(int16_t value);					   // this sets the tool speed. The value passed to this function is the actual IO value needed (for example a PWM value). On M5 or tool shutdown, this value is always 0.
	void tool_set_coolant(uint8_t value);				   // this gets a maks with the coolant outputs to enable(1) or disable(0)
	void tool_stop(void);								   // this stops the tool and coolant
	DECL_HOOK(tool_atc_unmount, uint8_t, uint8_t*);
	DECL_HOOK(tool_atc_mount, uint8_t, uint8_t*);

#ifdef __cplusplus
}
#endif

#endif
