/*
	Name: kinematic_delta.h
	Description: Custom kinematics definitions for delta machine

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

#ifndef KINEMATIC_DELTA_H
#define KINEMATIC_DELTA_H

#ifdef __cplusplus
extern "C"
{
#endif

#if AXIS_COUNT != 3
#error "Delta kinematics expects 3 axis"
#endif

// this should match the number of linear actuators on the machines (do not change unless you know what you are doing)
#define STEPPER_COUNT AXIS_COUNT

#ifndef STEPPER0_ANGLE
#define STEPPER0_ANGLE 30
#endif
#ifndef STEPPER1_ANGLE
#define STEPPER1_ANGLE (STEPPER0_ANGLE + 120)
#endif
#ifndef STEPPER2_ANGLE
#define STEPPER2_ANGLE (STEPPER0_ANGLE + 240)
#endif

// the maximum size of the computed segments that are sent to the planner
// this forces linear motions in the delta to treated has an arc motion to
// cope with the non linear kinematic motion of the towers
#ifndef DELTA_MOTION_SEGMENT_SIZE
#define DELTA_MOTION_SEGMENT_SIZE 1.0f
#endif
#define DELTA_MOTION_SEGMENT_FACTOR (1.0f / DELTA_MOTION_SEGMENT_SIZE)
	/*
	Enable Skew compensation
*/
	//#define ENABLE_SKEW_COMPENSATION

#ifdef __cplusplus
}
#endif
#endif
