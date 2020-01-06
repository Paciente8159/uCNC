/*
	Name: cnc.h
	Description: uCNC main unit
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
#define RT_CMD_RESET			0x18
#define RT_CMD_SAFETY_DOOR		0x84
#define RT_CMD_FEED_HOLD		0x21
#define RT_CMD_JOG_CANCEL		0x85
#define RT_CMD_CYCLE_START		0x7E
#define RT_CMD_REPORT			0x3F

//current cnc states (multiple can be active at the same time)
#define EXEC_IDLE			0 // All flags cleared
#define EXEC_RUN			1 // Motions are being executed
#define EXEC_CYCLE			2 // Is executing a parking (NOT USED-MAY BE ERASED)
#define EXEC_JOG			4 // Jogging motion
#define EXEC_HOMING			8 // Homing motion
#define EXEC_HOLD			16 // Feed hold is active
#define EXEC_ALARM			32 //Alarm mode
#define EXEC_DOOR			64
#define EXEC_SLEEP			128 //Sleep mode

#define CANCEL_CYCLE		EXEC_CYCLE
#define CANCEL_HOMING		EXEC_HOMING

//Homing flags. Not implemented yet
#define HOME_UNKONOW		0
#define HOME_OK				1
#define HOME_NEEDS_REHOME	2

void cnc_init();
void cnc_reset();
void cnc_run();
void cnc_doevents();
void cnc_home();
void cnc_alarm(uint8_t code);
void cnc_kill();
void cnc_stop();
void cnc_stop_tools();
void cnc_unlock();
void cnc_offset_home();

uint8_t cnc_get_exec_state(uint8_t statemask);
void cnc_set_exec_state(uint8_t statemask);
void cnc_clear_exec_state(uint8_t statemask);
uint8_t cnc_is_homed();
void cnc_exec_rt_command(uint8_t command);
void cnc_reset_position();

#endif
