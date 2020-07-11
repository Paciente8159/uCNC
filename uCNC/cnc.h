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
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef CNC_H
#define CNC_H

#include <stdbool.h>
#include <stdint.h>
#include "config.h"

//rt_cmd
#define RT_CMD_CLEAR 0

#define RT_CMD_STARTUP_BLOCK0 1
#define RT_CMD_STARTUP_BLOCK1 2
#define RT_CMD_REPORT 4
#define RT_CMD_ABORT 128

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
#define EXEC_IDLE 0   // All flags cleared
#define EXEC_RUN 1	// Motions are being executed
#define EXEC_HOLD 2  // Feed hold is active
#define EXEC_JOG 4	// Jogging in execution
#define EXEC_HOMING 8 // Homing in execution
#define EXEC_NOHOME 16 //Homing enable but home position is unkowned
#define EXEC_LIMITS 32 //Limit switch is active
#define EXEC_DOOR 64 //Safety door open
#define EXEC_ABORT 128 //Emergency stop
#define EXEC_ALARM (EXEC_NOHOME | EXEC_LIMITS | EXEC_DOOR | EXEC_ABORT) //System alarms
#define EXEC_ALARM_ABORT (EXEC_LIMITS | EXEC_DOOR | EXEC_ABORT) //System alarms checked after abort
#define EXEC_LOCKED	(EXEC_ALARM | EXEC_HOLD | EXEC_HOMING | EXEC_JOG) //Gcode is locked by an active state
#define EXEC_GCODE_LOCKED (EXEC_ALARM | EXEC_HOMING | EXEC_JOG) //Gcode is locked by an alarm or any special motion state
#define EXEC_ALLACTIVE 255 //All states


void cnc_init(void);
void cnc_run(void);
//do events returns true if all OK and false if an ABORT alarm is reached
bool cnc_doevents(void);
void cnc_home(void);
void cnc_alarm(uint8_t code);
void cnc_stop(void);
void cnc_unlock(void);

uint8_t cnc_get_exec_state(uint8_t statemask);
void cnc_set_exec_state(uint8_t statemask);
void cnc_clear_exec_state(uint8_t statemask);
void cnc_call_rt_command(uint8_t command);

#endif
