/*
	Name: kinematic_cartesian.h
	Description: Custom kinematics definitions for cartesian machine

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

#ifndef KINEMATIC_CARTESIAN_H
#define KINEMATIC_CARTESIAN_H

//this should match the number of linear actuators on the machines (do not change unless you know what you are doing)
#define STEPPER_COUNT AXIS_COUNT

/*
	Uncomment this feature to enable tool length compensation
*/
#define AXIS_TOOL AXIS_Z

/*
	Uncomment this feature to enable up to 2 dual drive axis
*/
//#define ENABLE_DUAL_DRIVE_AXIS
#ifdef ENABLE_DUAL_DRIVE_AXIS
//defines the first dual drive capable step output
//#define DUAL_DRIVE_AXIS0 X
//#define DUAL_DRIVE_AXIS1 Y
#endif

/*
	Enable Skew compensation
*/
//#define ENABLE_SKEW_COMPENSATION

#endif
