/*
	Name: tool.c
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

#include "../../cnc.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define DECL_TOOL(tool) extern const tool_t tool

static tool_t tool_current;

#ifdef TOOL1
DECL_TOOL(TOOL1);
#endif
#ifdef TOOL2
DECL_TOOL(TOOL2);
#endif
#ifdef TOOL3
DECL_TOOL(TOOL3);
#endif
#ifdef TOOL4
DECL_TOOL(TOOL4);
#endif
#ifdef TOOL5
DECL_TOOL(TOOL5);
#endif
#ifdef TOOL6
DECL_TOOL(TOOL6);
#endif
#ifdef TOOL7
DECL_TOOL(TOOL7);
#endif
#ifdef TOOL8
DECL_TOOL(TOOL8);
#endif
#ifdef TOOL9
DECL_TOOL(TOOL9);
#endif
#ifdef TOOL10
DECL_TOOL(TOOL10);
#endif
#ifdef TOOL11
DECL_TOOL(TOOL11);
#endif
#ifdef TOOL12
DECL_TOOL(TOOL12);
#endif
#ifdef TOOL13
DECL_TOOL(TOOL13);
#endif
#ifdef TOOL14
DECL_TOOL(TOOL14);
#endif
#ifdef TOOL15
DECL_TOOL(TOOL15);
#endif
#ifdef TOOL16
DECL_TOOL(TOOL16);
#endif

void tool_init(void)
{
#if TOOL_COUNT > 0
#ifdef FORCE_GLOBALS_TO_0
	memset(&tool_current, 0, sizeof(tool_t));
#endif
	tool_change(g_settings.default_tool);
#endif
}

void tool_change(uint8_t tool)
{
#if TOOL_COUNT > 0
	tool_stop();
	if (tool_current.shutdown_code)
	{
		tool_current.shutdown_code();
	}

	switch (tool)
	{
#ifdef TOOL1
	case 1:
		memcpy(&tool_current, &TOOL1, sizeof(tool_t));
		break;
#endif
#ifdef TOOL2
	case 2:
		memcpy(&tool_current, &TOOL2, sizeof(tool_t));
		break;
#endif
#ifdef TOOL3
	case 3:
		memcpy(&tool_current, &TOOL3, sizeof(tool_t));
		break;
#endif
#ifdef TOOL4
	case 4:
		memcpy(&tool_current, &TOOL4, sizeof(tool_t));
		break;
#endif
#ifdef TOOL5
	case 5:
		memcpy(&tool_current, &TOOL5, sizeof(tool_t));
		break;
#endif
#ifdef TOOL6
	case 6:
		memcpy(&tool_current, &TOOL6, sizeof(tool_t));
		break;
#endif
#ifdef TOOL7
	case 7:
		memcpy(&tool_current, &TOOL7, sizeof(tool_t));
		break;
#endif
#ifdef TOOL8
	case 8:
		memcpy(&tool_current, &TOOL8, sizeof(tool_t));
		break;
#endif
#ifdef TOOL9
	case 9:
		memcpy(&tool_current, &TOOL9, sizeof(tool_t));
		break;
#endif
#ifdef TOOL10
	case 10:
		memcpy(&tool_current, &TOOL10, sizeof(tool_t));
		break;
#endif
#ifdef TOOL11
	case 11:
		memcpy(&tool_current, &TOOL11, sizeof(tool_t));
		break;
#endif
#ifdef TOOL12
	case 12:
		memcpy(&tool_current, &TOOL12, sizeof(tool_t));
		break;
#endif
#ifdef TOOL13
	case 13:
		memcpy(&tool_current, &TOOL13, sizeof(tool_t));
		break;
#endif
#ifdef TOOL14
	case 14:
		memcpy(&tool_current, &TOOL14, sizeof(tool_t));
		break;
#endif
#ifdef TOOL15
	case 15:
		memcpy(&tool_current, &TOOL15, sizeof(tool_t));
		break;
#endif
#ifdef TOOL16
	case 16:
		memcpy(&tool_current, &TOOL16, sizeof(tool_t));
		break;
#endif
	default:
		memset(&tool_current, 0, sizeof(tool_t));
		break;
	}

	if (tool_current.startup_code)
	{
		tool_current.startup_code();
	}
#endif
}

void tool_set_speed(int16_t value)
{
#if TOOL_COUNT > 0
	if (tool_current.set_speed)
	{
		tool_current.set_speed(value);
	}
#endif
}

uint16_t tool_get_speed()
{
#if TOOL_COUNT > 0
	if (tool_current.get_speed)
	{
		return tool_current.get_speed();
	}
#endif
	return 0;
}

int16_t tool_range_speed(int16_t value)
{
	// input value will always be positive
#if TOOL_COUNT > 0
	if (tool_current.range_speed)
	{
		value = ABS(value);
		return tool_current.range_speed(value);
	}
#endif
	return value;
}

void tool_set_coolant(uint8_t value)
{
#if TOOL_COUNT > 0
	if (tool_current.set_coolant)
	{
		tool_current.set_coolant(value);
	}
#endif
}

void tool_stop()
{
#if TOOL_COUNT > 0
	tool_set_speed(0);
	tool_set_coolant(0);
#endif
}

void tool_pid_update(int16_t value)
{
#if PID_CONTROLLERS > 0
#if TOOL_COUNT > 0
	if (tool_current.pid_update)
	{
		tool_current.pid_update(value);
	}
#endif
#endif
}

int16_t tool_pid_error(void)
{
#if PID_CONTROLLERS > 0
#if TOOL_COUNT > 0
	if (tool_current.pid_error)
	{
		return tool_current.pid_error();
	}
#endif
#endif
	return 0;
}
