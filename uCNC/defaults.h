/*
	Name: defaults.h
	Description: Compile time default settings for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 07/12/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef DEFAULTS_H
#define DEFAULTS_H

#include "mcu.h"

//default step per mm
#define DEFAULT_X_STEP_PER_MM 200
#define DEFAULT_Y_STEP_PER_MM 200
#define DEFAULT_Z_STEP_PER_MM 200
#define DEFAULT_A_STEP_PER_MM 200
#define DEFAULT_B_STEP_PER_MM 200
#define DEFAULT_C_STEP_PER_MM 200

//default feed in mm/m
#define DEFAULT_X_MAX_FEED 500
#define DEFAULT_Y_MAX_FEED 500
#define DEFAULT_Z_MAX_FEED 500
#define DEFAULT_A_MAX_FEED 500
#define DEFAULT_B_MAX_FEED 500
#define DEFAULT_C_MAX_FEED 500

#define DEFAULT_HOMING_DIR_INV_MASK 0
#define DEFAULT_HOMING_SLOW 10
#define DEFAULT_HOMING_FAST 50
#define DEFAULT_HOMING_OFFSET 2

//default acceleration in mm/s^2
#define DEFAULT_X_ACCEL 10
#define DEFAULT_Y_ACCEL 10
#define DEFAULT_Z_ACCEL 10
#define DEFAULT_A_ACCEL 10
#define DEFAULT_B_ACCEL 10
#define DEFAULT_C_ACCEL 10

//default max distance traveled by each axis in mm
#define DEFAULT_X_MAX_DIST 200
#define DEFAULT_Y_MAX_DIST 200
#define DEFAULT_Z_MAX_DIST 200

#define DEFAULT_STEP_INV_MASK 0
#define DEFAULT_STEP_ENA_INV 0
#define DEFAULT_DIR_INV_MASK 0

#define DEFAULT_STATUS_MASK 1
#define DEFAULT_CONTROL_INV_MASK 0
#define DEFAULT_LIMIT_INV_MASK 0
#define DEFAULT_PROBE_INV_MASK 0

#define DEFAULT_INPUT_MASK0 0
#define DEFAULT_INPUT_MASK1 0

#define DEFAULT_ARC_TOLERANCE 0.002

#define DEFAULT_TOOL_COUNT 1

#define DEFAULT_MAX_STEP_RATE F_STEP_MAX //defined by the mcumap file of the mcu used

#define DEFAULT_SPINDLE_MAX_RPM 1000
#define DEFAULT_SPINDLE_MIN_RPM 0

#define DEFAULT_REPORT_INCHES 0
#define DEFAULT_HOMING_ENABLED 0
#define DEFAULT_HARD_LIMITS_ENABLED 0
#define DEFAULT_SOFT_LIMITS_ENABLED 0

#endif
