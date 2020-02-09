/*
	Name: config_helper.h
	Description: Automated compile time configurations for µCNC.
		Generates remaining settings based on mcu, board and machine.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 19/09/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef CONFIG_HELPER_H
#define CONFIG_HELPER_H

#define ESTOP_MASK 1
#define SAFETY_DOOR_MASK 2
#define FHOLD_MASK 4
#define CS_RES_MASK 8

#define CONTROLS_MASK (ESTOP_MASK | FHOLD_MASK | CS_RES_MASK | SAFETY_DOOR_MASK)

#define LIMIT_X_MASK 1
#define LIMIT_Y_MASK 2
#define LIMIT_Z_MASK 4
#define LIMIT_A_MASK 8
#define LIMIT_B_MASK 16
#define LIMIT_C_MASK 32

#define LIMITS_MASK (LIMIT_X_MASK | LIMIT_Y_MASK | LIMIT_Z_MASK | LIMIT_A_MASK | LIMIT_B_MASK | LIMIT_C_MASK)
#define LIMITS_LIMIT0_MASK 64
#define LIMITS_LIMIT1_MASK 128
#define LIMITS_DUAL_MASK (LIMITS_LIMIT0_MASK | LIMITS_LIMIT1_MASK)

#define STEP0_MASK 1
#define STEP1_MASK 2
#define STEP2_MASK 4
#define STEP3_MASK 8
#define STEP4_MASK 16
#define STEP5_MASK 32
#define STEP6_MASK 64
#define STEP7_MASK 128

#ifdef ENABLE_DUAL_DRIVE_AXIS

#define __stepname_helper__(x) STEP##x##_MASK
#define __stepname__(x) __stepname_helper__(x)

#define __axisname_helper__(x) AXIS_##x
#define __axisname__(x) __axisname_helper__(x)

#define __limitname_helper__(x) LIMIT_##x##_MASK
#define __limitname__(x) __limitname_helper__(x)

#ifdef DUAL_DRIVE_AXIS0
#define AXIS_DUAL0 __axisname__(DUAL_DRIVE_AXIS0)
#define STEP_DUAL0 (1 << AXIS_DUAL0)
#define LIMIT_DUAL0 __limitname__(DUAL_DRIVE_AXIS0)
#endif

#ifdef DUAL_DRIVE_AXIS1
#define AXIS_DUAL1 __axisname__(DUAL_DRIVE_AXIS1)
#define STEP_DUAL1 (1 << AXIS_DUAL1)
#define LIMIT_DUAL1 __limitname__(DUAL_DRIVE_AXIS1)
#endif

#if(STEP0_MASK == STEP_DUAL0)
#define STEP0_ITP_MASK (STEP0_MASK | 64)
#elif(STEP0_MASK == STEP_DUAL1)
#define STEP0_ITP_MASK (STEP0_MASK | 128)
#else
#define STEP0_ITP_MASK STEP0_MASK
#endif
#if(STEP1_MASK == STEP_DUAL0)
#define STEP1_ITP_MASK (STEP1_MASK | 64)
#elif(STEP1_MASK == STEP_DUAL1)
#define STEP1_ITP_MASK (STEP1_MASK | 128)
#else
#define STEP1_ITP_MASK STEP1_MASK
#endif
#if(STEP2_MASK == STEP_DUAL0)
#define STEP2_ITP_MASK (STEP2_MASK | 64)
#elif(STEP2_MASK == STEP_DUAL1)
#define STEP2_ITP_MASK (STEP2_MASK | 128)
#else
#define STEP2_ITP_MASK STEP2_MASK
#endif
#if(STEP3_MASK == STEP_DUAL0)
#define STEP3_ITP_MASK (STEP3_MASK | 64)
#elif(STEP3_MASK == STEP_DUAL1)
#define STEP3_ITP_MASK (STEP3_MASK | 128)
#else
#define STEP3_ITP_MASK STEP3_MASK
#endif
#if(STEP4_MASK == STEP_DUAL0)
#define STEP4_ITP_MASK (STEP4_MASK | 64)
#elif(STEP4_MASK == STEP_DUAL1)
#define STEP4_ITP_MASK (STEP4_MASK | 128)
#else
#define STEP4_ITP_MASK STEP4_MASK
#endif
#if(STEP5_MASK == STEP_DUAL0)
#define STEP5_ITP_MASK (STEP5_MASK | 64)
#elif(STEP5_MASK == STEP_DUAL1)
#define STEP5_ITP_MASK (STEP5_MASK | 128)
#else
#define STEP5_ITP_MASK STEP5_MASK
#endif

#endif

#define DIR0_MASK 1
#define DIR1_MASK 2
#define DIR2_MASK 4
#define DIR3_MASK 8
#define DIR4_MASK 16
#define DIR5_MASK 32

#endif
