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

//System states
#define SYS_OK 0
#define SYS_RESET 1
#define SYS_LOCKED 2

//realtime commands             Equivalent extended-ASCII input code
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

//current cnc states (multiple can be active at the same time)
#define EXEC_IDLE 0   // All flags cleared
#define EXEC_RUN 1	// Motions are being executed
#define EXEC_CYCLE 2  // Is executing a parking (NOT USED-MAY BE ERASED)
#define EXEC_JOG 4	// Jogging motion
#define EXEC_HOMING 8 // Homing motion
#define EXEC_HOLD 16  // Feed hold is active
#define EXEC_ALARM 32 //Alarm mode
#define EXEC_DOOR 64
#define EXEC_SLEEP 128 //Sleep mode

//Homing flags. Not implemented yet
#define HOME_UNKONOW 0
#define HOME_OK 1
#define HOME_NEEDS_REHOME 2

void cnc_init();
void cnc_reset();
void cnc_run();
void cnc_doevents();
void cnc_home();
void cnc_alarm(uint8_t code);
void cnc_kill();
void cnc_stop();
void cnc_unlock();

uint8_t cnc_get_exec_state(uint8_t statemask);
void cnc_set_exec_state(uint8_t statemask);
void cnc_clear_exec_state(uint8_t statemask);
uint8_t cnc_is_homed();
void cnc_call_rt_command(uint8_t command);
void cnc_exec_rt_command(uint8_t command);
/*bool cnc_overrides_enabled();
void cnc_set_overrides(bool enable);*/

#endif
