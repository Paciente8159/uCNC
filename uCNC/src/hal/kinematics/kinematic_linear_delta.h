/*
	Name: kinematic_linear_delta.h
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

#ifndef KINEMATIC_LINEAR_DELTA_H
#define KINEMATIC_LINEAR_DELTA_H

#ifdef __cplusplus
extern "C"
{
#endif

#define KINEMATIC_TYPE_STR "LD"

#if AXIS_COUNT < 3
#error "Linear delta kinematics expects at least 3 axis"
#endif

#ifndef STEPPER0_ANGLE
#define STEPPER0_ANGLE 30
#endif
#ifndef STEPPER1_ANGLE
#define STEPPER1_ANGLE (STEPPER0_ANGLE + 120)
#endif
#ifndef STEPPER2_ANGLE
#define STEPPER2_ANGLE (STEPPER0_ANGLE + 240)
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

// minimum arm angle that is allowed for the delta (for software limits)
#ifndef DELTA_ARM_MIN_ANGLE
#define DELTA_ARM_MIN_ANGLE 20
#endif

// maximum angle (should not be bigger then 90º deg angle)
#ifndef DELTA_ARM_MAX_ANGLE
#define DELTA_ARM_MAX_ANGLE 89
#endif

#define KINEMATICS_VARS_DECL \
	float delta_arm_length;    \
	float delta_armbase_radius;

#define KINEMATICS_VARS_DEFAULTS_INIT .delta_arm_length = DEFAULT_LIN_DELTA_ARM_LENGTH, \
																			.delta_armbase_radius = DEFAULT_LIN_DELTA_BASE_RADIUS,

#define KINEMATICS_VARS_SETTINGS_INIT {.id = 106, .memptr = &g_settings.delta_arm_length, .type = SETTING_TYPE_FLOAT}, \
																			{.id = 107, .memptr = &g_settings.delta_armbase_radius, .type = SETTING_TYPE_FLOAT},

#define KINEMATICS_VARS_SYSTEM_MENU_INIT                                             \
	DECL_MENU_VAR(SYSTEM_MENU_ID_KINEMATIC_SETTINGS, s106, STR_ARM_LEN, &g_settings.delta_arm_length, VAR_TYPE_FLOAT); \
	DECL_MENU_VAR(SYSTEM_MENU_ID_KINEMATIC_SETTINGS, s107, STR_BASE_RAD, &g_settings.delta_armbase_radius, VAR_TYPE_FLOAT);

#ifdef __cplusplus
}
#endif
#endif
