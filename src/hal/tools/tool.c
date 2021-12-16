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

#ifdef __cplusplus
extern "C"
{
#endif

#include "cnc.h"
#include <stdlib.h>

	static tool_t *tool_current;

	void tool_init(void)
	{
#ifdef FORCE_GLOBALS_TO_0
		tool_current = NULL;
#endif
	}

	void tool_change(uint8_t tool)
	{
		tool_stop();
		if (tool_current->shutdown_code)
		{
			(tool_current->shutdown_code)();
		}

		switch (tool)
		{
#if TOOL_COUNT > 0
		case 0:
			tool_current = (tool_t *)&tool0;
			break;
#endif
#if TOOL_COUNT > 1
		case 1:
			tool_current = (tool_t *)&tool1;
			break;
#endif
#if TOOL_COUNT > 2
		case 2:
			tool_current = (tool_t *)&tool2;
			break;
#endif
#if TOOL_COUNT > 3
		case 3:
			tool_current = (tool_t *)&tool3;
			break;
#endif
#if TOOL_COUNT > 4
		case 4:
			tool_current = (tool_t *)&tool4;
			break;
#endif
#if TOOL_COUNT > 5
		case 5:
			tool_current = (tool_t *)&tool5;
			break;
#endif
#if TOOL_COUNT > 6
		case 6:
			tool_current = (tool_t *)&tool6;
			break;
#endif
#if TOOL_COUNT > 7
		case 7:
			tool_current = (tool_t *)&tool7;
			break;
#endif
#if TOOL_COUNT > 8
		case 8:
			tool_current = (tool_t *)&tool8;
			break;
#endif
#if TOOL_COUNT > 9
		case 9:
			tool_current = (tool_t *)&tool9;
			break;
#endif
#if TOOL_COUNT > 10
		case 10:
			tool_current = (tool_t *)&tool10;
			break;
#endif
#if TOOL_COUNT > 11
		case 11:
			tool_current = (tool_t *)&tool11;
			break;
#endif
#if TOOL_COUNT > 12
		case 12:
			tool_current = (tool_t *)&tool12;
			break;
#endif
#if TOOL_COUNT > 13
		case 13:
			tool_current = (tool_t *)&tool13;
			break;
#endif
#if TOOL_COUNT > 14
		case 14:
			tool_current = (tool_t *)&tool14;
			break;
#endif
#if TOOL_COUNT > 15
		case 15:
			tool_current = (tool_t *)&tool15;
			break;
#endif
		default:
			tool_current = NULL;
			break;
		}

		if (tool_current->startup_code)
		{
			(tool_current->startup_code)();
		}
	}

	void tool_update_spindle(uint8_t value, bool invert)
	{
		if (tool_current->update_spindle)
		{
			(tool_current->update_spindle)(value, invert);
		}
	}

	int tool_get_spindle()
	{
		if (tool_current->get_spindle)
		{
			return (tool_current->get_spindle)();
		}

		return -1;
	}

	void tool_update_coolant(uint8_t value)
	{
		if ((tool_current->update_coolant))
		{
			return (tool_current->update_coolant)(value);
		}
	}

	void tool_stop()
	{
		(tool_current->update_spindle)(0, false);
		(tool_current->update_coolant)(0);
	}

#ifdef __cplusplus
}
#endif
