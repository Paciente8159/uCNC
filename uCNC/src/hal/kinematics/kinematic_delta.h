/*
	Name: kinematic_delta.h
	Description: Custom kinematics definitions for delta machine

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 03/11/2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef KINEMATIC_DELTA_H
#define KINEMATIC_DELTA_H

#ifdef __cplusplus
extern "C"
{
#endif

#if AXIS_COUNT < 3
#error "Delta kinematics expects at least 3 axis"
#endif

// kinematic motion is done by segments to cope with non linear kinematics motion
#define KINEMATICS_MOTION_BY_SEGMENTS
// kinematics homing
#define IS_DELTA_KINEMATICS

// the maximum size of the computed segments that are sent to the planner
// this forces linear motions in the delta to treated has an arc motion to
// cope with the non linear kinematic motion of the towers
#ifndef KINEMATICS_MOTION_SEGMENT_SIZE
#define KINEMATICS_MOTION_SEGMENT_SIZE 1.0f
#endif


	/*
	Enable Skew compensation
*/
	//#define ENABLE_SKEW_COMPENSATION

#ifdef __cplusplus
}
#endif
#endif
