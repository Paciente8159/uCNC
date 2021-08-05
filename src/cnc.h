/*
	Name: cnc.h
	Description: µCNC main unit.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 17/09/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef CNC_H
#define CNC_H

#ifdef __cplusplus
extern "C"
{
#endif

//rt_cmd
#define RT_CMD_CLEAR 0

#define RT_CMD_REPORT 1
#define RT_CMD_CYCLE_START 2
#define RT_CMD_RESET 4

//feed_ovr_cmd
#define RT_CMD_FEED_100 1
#define RT_CMD_FEED_INC_COARSE 2
#define RT_CMD_FEED_DEC_COARSE 4
#define RT_CMD_FEED_INC_FINE 8
#define RT_CMD_FEED_DEC_FINE 16
#define RT_CMD_RAPIDFEED_100 32
#define RT_CMD_RAPIDFEED_OVR1 64
#define RT_CMD_RAPIDFEED_OVR2 128
//tool_ovr_cmd
#define RT_CMD_SPINDLE_100 1
#define RT_CMD_SPINDLE_INC_COARSE 2
#define RT_CMD_SPINDLE_DEC_COARSE 4
#define RT_CMD_SPINDLE_INC_FINE 8
#define RT_CMD_SPINDLE_DEC_FINE 16
#define RT_CMD_SPINDLE_TOGGLE 32
#define RT_CMD_COOL_FLD_TOGGLE 64
#define RT_CMD_COOL_MST_TOGGLE 128

//current cnc states (multiple can be active/overlapped at the same time)
#define EXEC_IDLE 0												// All flags cleared
#define EXEC_RUN 1												// Motions are being executed
#define EXEC_RESUMING 2											// Motions are being resumed from a hold
#define EXEC_HOLD 4												// Feed hold is active
#define EXEC_JOG 8												// Jogging in execution
#define EXEC_HOMING 16											// Homing in execution
#define EXEC_HALT 32											// Limit switch is active or position lost due to abrupt stop
#define EXEC_DOOR 64											// Safety door open
#define EXEC_KILL 128											// Emergency stop
#define EXEC_ALARM (EXEC_HALT | EXEC_DOOR | EXEC_KILL)			// System alarms
#define EXEC_GCODE_LOCKED (EXEC_ALARM | EXEC_HOMING | EXEC_JOG) // Gcode is locked by an alarm or any special motion state
#define EXEC_ALLACTIVE 255										// All states

//creates a set of helper masks used to configure the controller
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

#define DIR0_MASK 1
#define DIR1_MASK 2
#define DIR2_MASK 4
#define DIR3_MASK 8
#define DIR4_MASK 16
#define DIR5_MASK 32

#include "cnc_build.h"
//make the needed includes (do not change the order)
//user configurations
#include "cnc_config.h"
//board and mcu configurations
#include "hal/boards/boarddefs.h" //configures the board IO and service interrupts
//machine kinematics configurations
#include "hal/kinematics/kinematicdefs.h" //configures the kinematics for the cnc machine
//general user options configurations
#include "cnc_config_helper.h"
//final HAL configurations
#include "hal/cnc_hal_config.h" //inicializes the HAL hardcoded connections
//initializes core utilities (like fast math functions)
#include "core/utils.h"

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

#if (STEP0_MASK == STEP_DUAL0)
#define STEP0_ITP_MASK (STEP0_MASK | 64)
#elif (STEP0_MASK == STEP_DUAL1)
#define STEP0_ITP_MASK (STEP0_MASK | 128)
#else
#define STEP0_ITP_MASK STEP0_MASK
#endif
#if (STEP1_MASK == STEP_DUAL0)
#define STEP1_ITP_MASK (STEP1_MASK | 64)
#elif (STEP1_MASK == STEP_DUAL1)
#define STEP1_ITP_MASK (STEP1_MASK | 128)
#else
#define STEP1_ITP_MASK STEP1_MASK
#endif
#if (STEP2_MASK == STEP_DUAL0)
#define STEP2_ITP_MASK (STEP2_MASK | 64)
#elif (STEP2_MASK == STEP_DUAL1)
#define STEP2_ITP_MASK (STEP2_MASK | 128)
#else
#define STEP2_ITP_MASK STEP2_MASK
#endif
#if (STEP3_MASK == STEP_DUAL0)
#define STEP3_ITP_MASK (STEP3_MASK | 64)
#elif (STEP3_MASK == STEP_DUAL1)
#define STEP3_ITP_MASK (STEP3_MASK | 128)
#else
#define STEP3_ITP_MASK STEP3_MASK
#endif
#if (STEP4_MASK == STEP_DUAL0)
#define STEP4_ITP_MASK (STEP4_MASK | 64)
#elif (STEP4_MASK == STEP_DUAL1)
#define STEP4_ITP_MASK (STEP4_MASK | 128)
#else
#define STEP4_ITP_MASK STEP4_MASK
#endif
#if (STEP5_MASK == STEP_DUAL0)
#define STEP5_ITP_MASK (STEP5_MASK | 64)
#elif (STEP5_MASK == STEP_DUAL1)
#define STEP5_ITP_MASK (STEP5_MASK | 128)
#else
#define STEP5_ITP_MASK STEP5_MASK
#endif

	/**
 * 
 * From this point on the CNC controller HAL is defined
 * 
 **/

#include <stdbool.h>
#include <stdint.h>

	void cnc_init(void);
	void cnc_run(void);
	//do events returns true if all OK and false if an ABORT alarm is reached
	bool cnc_dotasks(void);
	void cnc_home(void);
	void cnc_alarm(uint8_t code);
	void cnc_stop(void);
	bool cnc_unlock(bool force);
	void cnc_delay_ms(uint32_t miliseconds);

	uint8_t cnc_get_exec_state(uint8_t statemask);
	void cnc_set_exec_state(uint8_t statemask);
	void cnc_clear_exec_state(uint8_t statemask);
	void cnc_call_rt_command(uint8_t command);

#ifdef __cplusplus
}
#endif

#endif
