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

// default step per mm
#if (defined(DEFAULT_STEP_PER_MM) && defined(DEFAULT_STEP_PER_MM_PER_AXIS))
#error "Use either DEFAULT_STEP_PER_MM or DEFAULT_STEP_PER_MM_PER_AXIS, not both."
#endif

#if (!defined(DEFAULT_STEP_PER_MM))
#define DEFAULT_STEP_PER_MM 200
#endif

#if (!defined(DEFAULT_STEP_PER_MM_PER_AXIS))
#define DEFAULT_STEP_PER_MM_PER_AXIS DEFAULT_ARRAY(AXIS_COUNT, DEFAULT_STEP_PER_MM)
#endif

// default feed in mm/m
#if (defined(DEFAULT_MAX_FEED) && defined(DEFAULT_MAX_FEED_PER_AXIS))
#error "Use either DEFAULT_MAX or DEFAULT_MAX_PER_AXIS, not both."
#endif

#if (!defined(DEFAULT_MAX_FEED))
#define DEFAULT_MAX_FEED 500
#endif

#if (!defined(DEFAULT_MAX_FEED_PER_AXIS))
#define DEFAULT_MAX_FEED_PER_AXIS DEFAULT_ARRAY(AXIS_COUNT, DEFAULT_MAX_FEED)
#endif

// default acceleration in mm/s^2
#if (defined(DEFAULT_ACCEL) && defined(DEFAULT_ACCEL_PER_AXIS))
#error "Use either DEFAULT_ACCEL or DEFAULT_ACCEL_AXIS, not both."
#endif

#if (!defined(DEFAULT_ACCEL))
#define DEFAULT_ACCEL 10
#endif

#if (!defined(DEFAULT_ACCEL_PER_AXIS))
#define DEFAULT_ACCEL_PER_AXIS DEFAULT_ARRAY(AXIS_COUNT, DEFAULT_ACCEL)
#endif

// default max distance traveled by each axis in mm
#if (defined(DEFAULT_MAX_DIST) && defined(DEFAULT_MAX_DIST_PER_AXIS))
#error "Use either DEFAULT_MAX_DIST or DEFAULT_MAX_DIST_PER_AXIS, not both."
#endif

#if (!defined(DEFAULT_MAX_DIST))
#define DEFAULT_MAX_DIST 200
#endif

#if (!defined(DEFAULT_MAX_DIST_PER_AXIS))
#define DEFAULT_MAX_DIST_PER_AXIS DEFAULT_ARRAY(AXIS_COUNT, DEFAULT_MAX_DIST)
#endif

// limits
#if (!defined(DEFAULT_LIMIT_INV_MASK))
#define DEFAULT_LIMIT_INV_MASK 0
#endif

#if (!defined(DEFAULT_SOFT_LIMITS_ENABLED))
#define DEFAULT_SOFT_LIMITS_ENABLED 0
#endif

#if (!defined(DEFAULT_HARD_LIMITS_ENABLED))
#define DEFAULT_HARD_LIMITS_ENABLED 0
#endif

// misc
#if (!defined(DEFAULT_PROBE_INV_MASK))
#define DEFAULT_PROBE_INV_MASK 0
#endif

#if (!defined(DEFAULT_STATUS_MASK))
#define DEFAULT_STATUS_MASK 1
#endif

#if (!defined(DEFAULT_CONTROL_INV_MASK))
#define DEFAULT_CONTROL_INV_MASK 0
#endif

#if (!defined(DEFAULT_STARTUP_TOOL))
#define DEFAULT_STARTUP_TOOL 1
#endif

#if (!defined(DEFAULT_G64_FACTOR))
#define DEFAULT_G64_FACTOR 0.2
#endif

#if (!defined(DEFAULT_ARC_TOLERANCE))
#define DEFAULT_ARC_TOLERANCE 0.002
#endif

#if (!defined(DEFAULT_REPORT_INCHES))
#define DEFAULT_REPORT_INCHES 0
#endif

#if (!defined(DEFAULT_DEBOUNCE_MS))
#define DEFAULT_DEBOUNCE_MS 250
#endif

#define DEFAULT_INPUT_MASK0 0
#define DEFAULT_INPUT_MASK1 0

// spindle
#if (!defined(DEFAULT_SPINDLE_MAX_RPM))
#define DEFAULT_SPINDLE_MAX_RPM 1000
#endif

#if (!defined(DEFAULT_SPINDLE_MIN_RPM))
#define DEFAULT_SPINDLE_MIN_RPM 0
#endif

// default homing configuration
#if (!defined(DEFAULT_HOMING_DIR_INV_MASK))
#define DEFAULT_HOMING_DIR_INV_MASK 0
#endif

#if (!defined(DEFAULT_HOMING_ENABLED))
#define DEFAULT_HOMING_ENABLED 0
#endif

#if (!defined(DEFAULT_HOMING_FAST))
#define DEFAULT_HOMING_FAST 50
#endif

#if (!defined(DEFAULT_HOMING_SLOW))
#define DEFAULT_HOMING_SLOW 10
#endif

#if (!defined(DEFAULT_HOMING_OFFSET))
#define DEFAULT_HOMING_OFFSET 2
#endif

// step/dir/motor_enabled
#if (!defined(DEFAULT_STEP_INV_MASK))
#define DEFAULT_STEP_INV_MASK 0
#endif

#if (!defined(DEFAULT_STEP_ENA_INV))
#define DEFAULT_STEP_ENA_INV 0
#endif

#if (!defined(DEFAULT_DIR_INV_MASK))
#define DEFAULT_DIR_INV_MASK 0
#endif

// linear delta
#if (!defined(DEFAULT_LIN_DELTA_ARM_LENGTH))
#define DEFAULT_LIN_DELTA_ARM_LENGTH 230
#endif

#if (!defined(DEFAULT_LIN_DELTA_BASE_RADIUS))
#define DEFAULT_LIN_DELTA_BASE_RADIUS 115
#endif

// delta kinematics
#if (!defined(DEFAULT_DELTA_BICEP_LENGTH))
#define DEFAULT_DELTA_BICEP_LENGTH 100
#endif

#if (!defined(DEFAULT_DELTA_FOREARM_LENGTH))
#define DEFAULT_DELTA_FOREARM_LENGTH 300
#endif

#if (!defined(DEFAULT_DELTA_EFFECTOR_RADIUS))
#define DEFAULT_DELTA_EFFECTOR_RADIUS 24
#endif

#if (!defined(DEFAULT_DELTA_BASE_RADIUS))
#define DEFAULT_DELTA_BASE_RADIUS 75
#endif

#if (!defined(DEFAULT_DELTA_BICEP_HOMING_ANGLE))
#define DEFAULT_DELTA_BICEP_HOMING_ANGLE 0
#endif

#if (!defined(DEFAULT_SCARA_ARM_LENGTH))
#define DEFAULT_SCARA_ARM_LENGTH 100
#endif

#if (!defined(DEFAULT_SCARA_FOREARM_LENGTH))
#define DEFAULT_SCARA_FOREARM_LENGTH 80
#endif

#if (!defined(DEFAULT_SCARA_ARM_HOMING_ANGLE))
#define DEFAULT_SCARA_ARM_HOMING_ANGLE 0
#endif

#if (!defined(DEFAULT_SCARA_FOREARM_HOMING_ANGLE))
#define DEFAULT_SCARA_FOREARM_HOMING_ANGLE 0
#endif

// laser mode
#if (!defined(DEFAULT_LASER_PPI))
#define DEFAULT_LASER_PPI 254
#endif

#if (!defined(DEFAULT_LASER_PPI_USWIDTH))
#define DEFAULT_LASER_PPI_USWIDTH 1500
#endif

#define DEFAULT_PID ({0, 0, 0})

#ifdef __cplusplus
}
#endif

#endif
