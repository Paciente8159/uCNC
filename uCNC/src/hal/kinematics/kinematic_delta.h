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

#define KINEMATIC_TYPE_STR "D"

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

#define KINEMATICS_VARS_DECL   \
	float delta_base_radius;     \
	float delta_effector_radius; \
	float delta_bicep_length;    \
	float delta_forearm_length;  \
	float delta_bicep_homing_angle;

#define KINEMATICS_VARS_DEFAULTS_INIT .delta_base_radius = DEFAULT_DELTA_BASE_RADIUS,         \
																			.delta_effector_radius = DEFAULT_DELTA_EFFECTOR_RADIUS, \
																			.delta_bicep_length = DEFAULT_DELTA_BICEP_LENGTH,       \
																			.delta_forearm_length = DEFAULT_DELTA_FOREARM_LENGTH,   \
																			.delta_bicep_homing_angle = DEFAULT_DELTA_BICEP_HOMING_ANGLE,

#define KINEMATICS_VARS_SETTINGS_INIT {.id = 28, .memptr = &g_settings.delta_bicep_homing_angle, .type = SETTING_TYPE_FLOAT}, \
																			{.id = 106, .memptr = &g_settings.delta_base_radius, .type = SETTING_TYPE_FLOAT},       \
																			{.id = 107, .memptr = &g_settings.delta_effector_radius, .type = SETTING_TYPE_FLOAT},   \
																			{.id = 108, .memptr = &g_settings.delta_bicep_length, .type = SETTING_TYPE_FLOAT},      \
																			{.id = 109, .memptr = &g_settings.delta_forearm_length, .type = SETTING_TYPE_FLOAT},

#define KINEMATICS_VARS_SYSTEM_MENU_INIT                                                                                  \
	DECL_MENU_VAR(SYSTEM_MENU_ID_HOMING, s28, STR_OFFSET, &g_settings.delta_bicep_homing_angle, VAR_TYPE_FLOAT);            \
	DECL_MENU_VAR(SYSTEM_MENU_ID_KINEMATIC_SETTINGS, s106, STR_BASE_RAD, &g_settings.delta_base_radius, VAR_TYPE_FLOAT);    \
	DECL_MENU_VAR(SYSTEM_MENU_ID_KINEMATIC_SETTINGS, s107, STR_EFF_RAD, &g_settings.delta_effector_radius, VAR_TYPE_FLOAT); \
	DECL_MENU_VAR(SYSTEM_MENU_ID_KINEMATIC_SETTINGS, s108, STR_BICEP_LEN, &g_settings.delta_bicep_length, VAR_TYPE_FLOAT);  \
	DECL_MENU_VAR(SYSTEM_MENU_ID_KINEMATIC_SETTINGS, s109, STR_FARM_LEN, &g_settings.delta_forearm_length, VAR_TYPE_FLOAT);

#ifdef __cplusplus
}
#endif
#endif
