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
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef DEFAULTS_H
#define DEFAULTS_H

#ifdef __cplusplus
extern "C"
{
#endif

// default step per mm
#define DEFAULT_STEP_PER_MM 200
// default feed in mm/m
#define DEFAULT_MAX_FEED 500
// default acceleration in mm/s^2
#define DEFAULT_ACCEL 10
// default max distance traveled by each axis in mm
#define DEFAULT_MAX_DIST 200

#define DEFAULT_HOMING_DIR_INV_MASK 0
#define DEFAULT_HOMING_SLOW 10
#define DEFAULT_HOMING_FAST 50
#define DEFAULT_HOMING_OFFSET 2

#define DEFAULT_STEP_INV_MASK 0
#define DEFAULT_STEP_ENA_INV 0
#define DEFAULT_DIR_INV_MASK 0

#define DEFAULT_STATUS_MASK 1
#define DEFAULT_CONTROL_INV_MASK 0
#define DEFAULT_LIMIT_INV_MASK 0
#define DEFAULT_PROBE_INV_MASK 0

#define DEFAULT_INPUT_MASK0 0
#define DEFAULT_INPUT_MASK1 0

#define DEFAULT_G64_FACTOR 0.2
#define DEFAULT_ARC_TOLERANCE 0.002

#define DEFAULT_STARTUP_TOOL 1

#define DEFAULT_SPINDLE_MAX_RPM 1000
#define DEFAULT_SPINDLE_MIN_RPM 0

#define DEFAULT_REPORT_INCHES 0
#define DEFAULT_HOMING_ENABLED 0
#define DEFAULT_HARD_LIMITS_ENABLED 0
#define DEFAULT_SOFT_LIMITS_ENABLED 0

#define DEFAULT_DEBOUNCE_MS 250

#define DEFAULT_DELTA_ARM_LENGTH 230
#define DEFAULT_DELTA_BASE_RADIUS 115

#define DEFAULT_PID ({0, 0, 0})

#define DEFAULT_ARRAY_0(y) \
	{                      \
	}
#define DEFAULT_ARRAY_1(y) \
	{                      \
		y                  \
	}
#define DEFAULT_ARRAY_2(y) \
	{                      \
		y, y               \
	}
#define DEFAULT_ARRAY_3(y) \
	{                      \
		y, y, y            \
	}
#define DEFAULT_ARRAY_4(y) \
	{                      \
		y, y, y, y         \
	}
#define DEFAULT_ARRAY_5(y) \
	{                      \
		y, y, y, y, y      \
	}
#define DEFAULT_ARRAY_6(y) \
	{                      \
		y, y, y, y, y, y   \
	}
#define DEFAULT_ARRAY_7(y)  \
	{                       \
		y, y, y, y, y, y, y \
	}
#define DEFAULT_ARRAY_8(y)     \
	{                          \
		y, y, y, y, y, y, y, y \
	}
#define DEFAULT_ARRAY_9(y)        \
	{                             \
		y, y, y, y, y, y, y, y, y \
	}
#define DEFAULT_ARRAY_10(y)          \
	{                                \
		y, y, y, y, y, y, y, y, y, y \
	}
#define DEFAULT_ARRAY_11(y)             \
	{                                   \
		y, y, y, y, y, y, y, y, y, y, y \
	}
#define DEFAULT_ARRAY_12(y)                \
	{                                      \
		y, y, y, y, y, y, y, y, y, y, y, y \
	}
#define DEFAULT_ARRAY_13(y)                   \
	{                                         \
		y, y, y, y, y, y, y, y, y, y, y, y, y \
	}
#define DEFAULT_ARRAY_14(y)                      \
	{                                            \
		y, y, y, y, y, y, y, y, y, y, y, y, y, y \
	}
#define DEFAULT_ARRAY_15(y)                         \
	{                                               \
		y, y, y, y, y, y, y, y, y, y, y, y, y, y, y \
	}
#define _DEFAULT_ARRAY(x, y) DEFAULT_ARRAY_##x(y)
#define DEFAULT_ARRAY(x, y) _DEFAULT_ARRAY(x, y)

#ifdef __cplusplus
}
#endif

#endif
