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
#include "../../interface/settings.h"
#include <stdlib.h>

static tool_t tool_current;

//this variable is not used but forces the compiler to compile the selected tools compilation units
TOOLDEF const tool_t *__rom__ const tools[] = {
#ifdef TOOL1
	&TOOL1,
#endif
#ifdef TOOL2
	&TOOL2,
#endif
#ifdef TOOL3
	&TOOL3,
#endif
#ifdef TOOL4
	&TOOL4,
#endif
#ifdef TOOL5
	&TOOL5,
#endif
#ifdef TOOL6
	&TOOL6,
#endif
#ifdef TOOL7
	&TOOL7,
#endif
#ifdef TOOL8
	&TOOL8,
#endif
#ifdef TOOL9
	&TOOL9,
#endif
#ifdef TOOL10
	&TOOL10,
#endif
#ifdef TOOL11
	&TOOL11,
#endif
#ifdef TOOL12
	&TOOL12,
#endif
#ifdef TOOL13
	&TOOL13,
#endif
#ifdef TOOL14
	&TOOL14,
#endif
#ifdef TOOL15
	&TOOL15,
#endif
#ifdef TOOL16
	&TOOL16,
#endif
};

void tool_init(void)
{
#ifdef FORCE_GLOBALS_TO_0
	tool_current = {};
#endif

	tool_change(g_settings.default_tool);
}

void tool_change(uint8_t tool)
{
	tool_stop();
	if (tool_current.shutdown_code)
	{
		tool_current.shutdown_code();
	}

	switch (tool)
	{
#ifdef TOOL1
	case 1:
		rom_memcpy(&tool_current, &TOOL1, sizeof(tool_t));
		break;
#endif
#ifdef TOOL2
	case 2:
		rom_memcpy(&tool_current, &TOOL2, sizeof(tool_t));
		break;
#endif
#ifdef TOOL3
	case 3:
		rom_memcpy(&tool_current, &TOOL3, sizeof(tool_t));
		break;
#endif
#ifdef TOOL4
	case 4:
		rom_memcpy(&tool_current, &TOOL4, sizeof(tool_t));
		break;
#endif
#ifdef TOOL5
	case 5:
		rom_memcpy(&tool_current, &TOOL5, sizeof(tool_t));
		break;
#endif
#ifdef TOOL6
	case 6:
		rom_memcpy(&tool_current, &TOOL6, sizeof(tool_t));
		break;
#endif
#ifdef TOOL7
	case 7:
		rom_memcpy(&tool_current, &TOOL7, sizeof(tool_t));
		break;
#endif
#ifdef TOOL8
	case 8:
		rom_memcpy(&tool_current, &TOOL8, sizeof(tool_t));
		break;
#endif
#ifdef TOOL9
	case 9:
		rom_memcpy(&tool_current, &TOOL9, sizeof(tool_t));
		break;
#endif
#ifdef TOOL10
	case 10:
		rom_memcpy(&tool_current, &TOOL10, sizeof(tool_t));
		break;
#endif
#ifdef TOOL11
	case 11:
		rom_memcpy(&tool_current, &TOOL11, sizeof(tool_t));
		break;
#endif
#ifdef TOOL12
	case 12:
		rom_memcpy(&tool_current, &TOOL12, sizeof(tool_t));
		break;
#endif
#ifdef TOOL13
	case 13:
		rom_memcpy(&tool_current, &TOOL13, sizeof(tool_t));
		break;
#endif
#ifdef TOOL14
	case 14:
		rom_memcpy(&tool_current, &TOOL14, sizeof(tool_t));
		break;
#endif
#ifdef TOOL15
	case 15:
		rom_memcpy(&tool_current, &TOOL15, sizeof(tool_t));
		break;
#endif
#ifdef TOOL16
	case 16:
		rom_memcpy(&tool_current, &TOOL16, sizeof(tool_t));
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
}

void tool_set_speed(int16_t value)
{
#ifdef USE_SPINDLE
	if (tool_current.set_speed)
	{
		tool_current.set_speed((uint8_t)(MIN(ABS(value), 0xff)), (value < 0));
	}
#endif
}

uint8_t tool_get_speed()
{
#ifdef USE_SPINDLE
	if (tool_current.get_speed)
	{
		return tool_current.get_speed();
	}
#endif
	return 0;
}

void tool_set_coolant(uint8_t value)
{
#ifdef USE_COOLANT
	if (tool_current.set_coolant)
	{
		tool_current.set_coolant(value);
	}
#endif
}

void tool_stop()
{
	tool_set_speed(0);
	tool_set_coolant(0);
}

uint8_t tool_pid_update(void)
{
#ifdef USE_SPINDLE
	if (tool_current.pid_controller)
	{
		return tool_current.pid_controller();
	}
#endif
	return 0;
}
