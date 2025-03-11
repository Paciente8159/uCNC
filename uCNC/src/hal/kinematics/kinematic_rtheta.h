/*
	Name: kinematic_rtheta.h
	Description: Custom kinematics definitions for rtheta machine

	Copyright: Copyright (c) Devansh Garg
	Author: Devansh Garg
	Date: 04/03/2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef KINEMATIC_RTHETA_H
#define KINEMATIC_RTHETA_H

#ifdef __cplusplus
extern "C"
{
#endif

#define KINEMATIC_TYPE_STR "RT"

// kinematic motion is done by segments to cope with non linear kinematics motion
#define KINEMATICS_MOTION_BY_SEGMENTS
// kinematics homing
#define IS_RTHETA_KINEMATICS

// the maximum size of the computed segments that are sent to the planner
// this forces linear motions in the scara to treated has an arc motion to
// cope with the non linear kinematic motion of the towers
#ifndef KINEMATICS_MOTION_SEGMENT_SIZE
#define KINEMATICS_MOTION_SEGMENT_SIZE 1.0f
#endif

	/*
	Enable Skew compensation
*/
	// #define ENABLE_SKEW_COMPENSATION

#define KINEMATICS_VARS_DECL          \
	float rtheta_theta_homing_angle;    \
	float rthera_arm_homing_distance;   \
	float rtheta_theta_reduction_ratio; \
	float rtheta_arm_length;

#ifndef RTHETA_HOME_ANGLE_DEFAULT
#define RTHETA_HOME_ANGLE_DEFAULT 0
#endif
#ifndef RTHETA_HOME_DIST_DEFAULT
#define RTHETA_HOME_DIST_DEFAULT 0
#endif
#ifndef s
#define RTHETA_RATIO_DEFAULT 1
#endif
#ifndef RTHETA_ARM_LEN_DEFAULT
#define RTHETA_ARM_LEN_DEFAULT 100
#endif

#define KINEMATICS_VARS_DEFAULTS_INIT .rtheta_theta_homing_angle = RTHETA_HOME_ANGLE_DEFAULT, \
																			.rthera_arm_homing_distance = RTHETA_HOME_DIST_DEFAULT, \
																			.rtheta_theta_reduction_ratio = RTHETA_RATIO_DEFAULT,   \
																			.rtheta_arm_length = RTHETA_ARM_LEN_DEFAULT,

#define KINEMATICS_VARS_SETTINGS_INIT {.id = 28, .memptr = &g_settings.rtheta_theta_homing_angle, .type = SETTING_TYPE_FLOAT},     \
																			{.id = 29, .memptr = &g_settings.rthera_arm_homing_distance, .type = SETTING_TYPE_FLOAT},    \
																			{.id = 106, .memptr = &g_settings.rtheta_theta_reduction_ratio, .type = SETTING_TYPE_FLOAT}, \
																			{.id = 107, .memptr = &g_settings.rtheta_arm_length, .type = SETTING_TYPE_FLOAT},
#ifndef STR_RTHETA_HOME_ANGLE
#define STR_RTHETA_HOME_ANGLE "RTh Homing angle"
#endif
#ifndef STR_RTHETA_HOME_DIST
#define STR_RTHETA_HOME_DIST "RTh Homing dist."
#endif
#ifndef STR_RTHETA_RATIO
#define STR_RTHETA_RATIO "RTh red. ratio"
#endif
#ifndef STR_RTHETA_ARM_LEN
#define STR_RTHETA_ARM_LEN "RTh arm length"
#endif

#define KINEMATICS_VARS_SYSTEM_MENU_INIT                                                                                              \
	DECL_MENU_VAR(SYSTEM_MENU_ID_HOMING, s28, STR_RTHETA_HOME_ANGLE, &g_settings.rtheta_theta_homing_angle, VAR_TYPE_FLOAT);            \
	DECL_MENU_VAR(SYSTEM_MENU_ID_HOMING, s29, STR_RTHETA_HOME_DIST, &g_settings.rthera_arm_homing_distance, VAR_TYPE_FLOAT);            \
	DECL_MENU_VAR(SYSTEM_MENU_ID_KINEMATIC_SETTINGS, s106, STR_RTHETA_RATIO, &g_settings.rtheta_theta_reduction_ratio, VAR_TYPE_FLOAT); \
	DECL_MENU_VAR(SYSTEM_MENU_ID_KINEMATIC_SETTINGS, s107, STR_RTHETA_ARM_LEN, &g_settings.rtheta_arm_length, VAR_TYPE_FLOAT);

#ifdef __cplusplus
}
#endif
#endif
