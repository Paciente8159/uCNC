/*
	Name: kinematic_corexy.h
	Description: For every existing machine defines the number of AXIS and STEPPERS and the AXIS index.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 06/02/2020

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef KINEMATIC_COREXY_H
#define KINEMATIC_COREXY_H

#ifdef __cplusplus
extern "C"
{
#endif

// define kynematics

// this should match the number of linear actuators on the machines (do not change unless you know what you are doing)
#ifndef STEPPER_COUNT
#define STEPPER_COUNT AXIS_COUNT
#endif

#ifdef __cplusplus
}
#endif

#endif