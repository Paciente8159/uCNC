/*
	Name: cnc.h
	Description: uCNC main unit.
	
	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 17/09/2019

	uCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	uCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef CNC_H
#define CNC_H

#include <stdbool.h>
#include <stdint.h>
#include "config.h"
#include "machinedefs.h"

//realtime commands             Equivalent extended-ASCII input code
#define RT_CMD_CLEAR 0
#define RT_CMD_RESET 0x18
#define RT_CMD_SAFETY_DOOR 0x84
#define RT_CMD_FEED_HOLD 0x21
#define RT_CMD_JOG_CANCEL 0x85
#define RT_CMD_CYCLE_START 0x7E
#define RT_CMD_REPORT 0x3F
#define RT_CMD_FEED_100 0x90
#define RT_CMD_FEED_INC_COARSE 0x91
#define RT_CMD_FEED_DEC_COARSE 0x92
#define RT_CMD_FEED_INC_FINE 0x93
#define RT_CMD_FEED_DEC_FINE 0x94
#define RT_CMD_RAPIDFEED_100 0x95
#define RT_CMD_RAPIDFEED_OVR1 0x96
#define RT_CMD_RAPIDFEED_OVR2 0x97
#define RT_CMD_SPINDLE_100 0x99
#define RT_CMD_SPINDLE_INC_COARSE 0x9A
#define RT_CMD_SPINDLE_DEC_COARSE 0x9B
#define RT_CMD_SPINDLE_INC_FINE 0x9C
#define RT_CMD_SPINDLE_DEC_FINE 0x9D
#define RT_CMD_SPINDLE_TOGGLE 0x9E
#define RT_CMD_COOL_FLD_TOGGLE 0xA0
#define RT_CMD_COOL_MST_TOGGLE 0xA1

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
//#define EXEC_ALLACTIVE 255 //All states


void cnc_init();
void cnc_run();
void cnc_doevents();
void cnc_home();
void cnc_alarm(uint8_t code);
void cnc_stop();
void cnc_unlock();

uint8_t cnc_get_exec_state(uint8_t statemask);
void cnc_set_exec_state(uint8_t statemask);
void cnc_clear_exec_state(uint8_t statemask);
void cnc_call_rt_command(uint8_t command);

#endif
