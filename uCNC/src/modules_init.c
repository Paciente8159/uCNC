/*
	Name: modules_init.c
	Description: User modules startup code.
				 This is the file to edit and add all user modules

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 18/01/2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "cnc.h"

// this is the place to declare all parser extension registration calls
#ifdef ENABLE_PARSER_EXTENSIONS
extern void m42_register(void);
#endif

void modules_init(void)
{

	// initializes PID module
#if PID_CONTROLLERS > 0
	pid_init(); // pid
#endif

// initializes parser extension modules
#ifdef ENABLE_PARSER_EXTENSIONS
	m42_register();
#endif
}