/*
	Name: machinedefs.h
	Description: For every existing machine defines the number of AXIS and STEPPERS and the AXIS index.

	Copyright: Copyright (c) Jo�o Martins
	Author: Jo�o Martins
	Date: 11/11/2019

	�CNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	�CNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef MACHINEDEFS_H
#define MACHINEDEFS_H

#include "config.h"

#ifndef MACHINE_KINEMATICS
#error Undefined kinematics
#endif

//define kynematics
#if (MACHINE_KINEMATICS == MACHINE_CARTESIAN)
#include "kinematics_cartesian.h"
#elif (MACHINE_KINEMATICS == MACHINE_COREXY)
#include "kinematics_corexy.h"
#else
#error Kinematics not implemented
#endif

#endif
