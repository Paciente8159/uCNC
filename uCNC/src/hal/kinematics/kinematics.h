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
#define KINEMATIC_SCARA 5
#define KINEMATIC_DUMMY 99

#define COREXY_AXIS_XY 1
#define COREXY_AXIS_XZ 2
#define COREXY_AXIS_YZ 3

/**
 * Settings for the kinematics
 * 
 * KINEMATICS_VARS_DECL - variables declarations inside the g_settings
 * KINEMATICS_VARS_DEFAULTS_INIT -  default values initialization
 * KINEMATICS_VARS_SETTINGS_INIT - inititialization for the settings $ commands
 * KINEMATICS_VARS_SYSTEM_MENU_INIT - declaration/initialization of settings in the system menu 
 * 
 */
#ifndef KINEMATICS_VARS_DECL
#define KINEMATICS_VARS_DECL
#endif

#ifndef KINEMATICS_VARS_DEFAULTS_INIT
#define KINEMATICS_VARS_DEFAULTS_INIT
#endif

#ifndef KINEMATICS_VARS_SETTINGS_INIT
#define KINEMATICS_VARS_SETTINGS_INIT
#endif

#ifndef KINEMATICS_VARS_SYSTEM_MENU_INIT
#define KINEMATICS_VARS_SYSTEM_MENU_INIT
#endif

#ifdef __cplusplus
}
#endif

#endif
