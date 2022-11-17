/*
	Name: kinematics.h
	Description: Defines the available machine types.

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

#ifndef KINEMATICS_H
#define KINEMATICS_H

#ifdef __cplusplus
extern "C"
{
#endif

#define KINEMATIC_CARTESIAN 1
#define KINEMATIC_COREXY 2
#define KINEMATIC_LINEAR_DELTA 3
#define KINEMATIC_DELTA 4

#ifdef __cplusplus
}
#endif

#endif
