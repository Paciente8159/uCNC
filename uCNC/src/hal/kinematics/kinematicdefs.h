/*
	Name: kinematicdefs.h
	Description: For every existing machine defines the number of AXIS and STEPPERS and the AXIS index.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 11/11/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef KINEMATICDEFS_H
#define KINEMATICDEFS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "kinematics.h"

#ifndef KINEMATIC
#error Undefined kinematics
#endif

/*
	Number of axis to be configured
*/
#ifndef AXIS_COUNT
#define AXIS_COUNT 3
#endif
//defines the axis word and the internal µCNC coordinate index
#if (AXIS_COUNT > 0)
#define AXIS_X 0
#endif
#if (AXIS_COUNT > 1)
#define AXIS_Y 1
#endif
#if (AXIS_COUNT > 2)
#define AXIS_Z 2
#endif
#if (AXIS_COUNT > 3)
#define AXIS_A 3
#endif
#if (AXIS_COUNT > 4)
#define AXIS_B 4
#endif
#if (AXIS_COUNT > 5)
#define AXIS_C 5
#endif

//define kynematics
#if (KINEMATIC == KINEMATIC_CARTESIAN)
#include "kinematic_cartesian.h"
#elif (KINEMATIC == KINEMATIC_COREXY)
#include "kinematic_corexy.h"
#elif (KINEMATIC == KINEMATIC_DELTA)
#include "kinematic_delta.h"
#else
#error Kinematics not implemented
#endif

#include "kinematic.h"

#ifdef __cplusplus
}
#endif

#endif
