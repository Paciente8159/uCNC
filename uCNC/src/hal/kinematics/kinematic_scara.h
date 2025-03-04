/*
	Name: kinematic_scara.h
	Description: Custom kinematics definitions for scara machine

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 28/072023

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef KINEMATIC_SCARA_H
#define KINEMATIC_SCARA_H

#ifdef __cplusplus
extern "C"
{
#endif

#define KINEMATIC_TYPE_STR "SC"

// kinematic motion is done by segments to cope with non linear kinematics motion
#define KINEMATICS_MOTION_BY_SEGMENTS
// kinematics homing
#define IS_SCARA_KINEMATICS

// the maximum size of the computed segments that are sent to the planner
// this forces linear motions in the scara to treated has an arc motion to
// cope with the non linear kinematic motion of the towers
#ifndef KINEMATICS_MOTION_SEGMENT_SIZE
#define KINEMATICS_MOTION_SEGMENT_SIZE 1.0f
#endif

	/**
	 * Enable Skew compensation
	 */

	// #define ENABLE_SKEW_COMPENSATION

#define KINEMATICS_VARS_DECL    \
	float scara_arm_length;       \
	float scara_forearm_length;   \
	float scara_arm_homing_angle; \
	float scara_forearm_homing_angle;

#define KINEMATICS_VARS_DEFAULTS_INIT .scara_arm_length = DEFAULT_SCARA_ARM_LENGTH,             \
																			.scara_forearm_length = DEFAULT_SCARA_FOREARM_LENGTH,     \
																			.scara_arm_homing_angle = DEFAULT_SCARA_ARM_HOMING_ANGLE, \
																			.scara_forearm_homing_angle = DEFAULT_SCARA_FOREARM_HOMING_ANGLE,

#define KINEMATICS_VARS_SETTINGS_INIT {.id = 28, .memptr = &g_settings.scara_arm_homing_angle, .type = SETTING_TYPE_FLOAT},     \
																			{.id = 29, .memptr = &g_settings.scara_forearm_homing_angle, .type = SETTING_TYPE_FLOAT}, \
																			{.id = 106, .memptr = &g_settings.scara_arm_length, .type = SETTING_TYPE_FLOAT},          \
																			{.id = 107, .memptr = &g_settings.scara_forearm_length, .type = SETTING_TYPE_FLOAT},

#define KINEMATICS_VARS_SYSTEM_MENU_INIT                                                                     \
	DECL_MENU_VAR(SYSTEM_MENU_ID_HOMING, s28, STR_OFFSET, &g_settings.scara_arm_homing_angle, VAR_TYPE_FLOAT); \
	DECL_MENU_VAR(SYSTEM_MENU_ID_HOMING, s29, STR_OFFSET, &g_settings.scara_forearm_homing_angle, VAR_TYPE_FLOAT);

#ifdef __cplusplus
}
#endif
#endif
