#ifndef CNC_H
#define CNC_H

#include <stdbool.h>
#include <stdint.h>
#include "config.h"
#include "machinedefs.h"

//realtime commands
#define RT_CMD_RESET			32
#define RT_CMD_SAFETY_DOOR		16
#define RT_CMD_FEED_HOLD		8
#define RT_CMD_JOG_CANCEL		4
#define RT_CMD_CYCLE_START		2
#define RT_CMD_REPORT			1
/*
#define RT_CLEAR_RESET			0
#define RT_CLEAR_SAFETY_DOOR	~(RT_CMD_RESET - 1)
#define RT_CLEAR_FEED_HOLD		~(RT_CMD_SAFETY_DOOR - 1)
#define RT_CLEAR_CYCLE_START	~(RT_CMD_FEED_HOLD - 1)
#define RT_CLEAR_REPORT			~(RT_CMD_REPORT)*/

//current cnc states (multiple can be active at the same time)
#define EXEC_IDLE			0 // All flags cleared
#define EXEC_RUN			1 // Motions are being executed
#define EXEC_CYCLE			2 // Is executing a parking
#define EXEC_JOG			4 // Jogging motion
#define EXEC_HOMING			8 // Homing motion
#define EXEC_HOLD			16 // Feed hold is active
#define EXEC_DOOR			32
#define EXEC_ALARM			64 //Alarm mode
#define EXEC_SLEEP			128 //Sleep mode




#define CANCEL_CYCLE		EXEC_CYCLE
#define CANCEL_HOMING		EXEC_HOMING

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
//moved to motion control
//uint8_t cnc_home_axis(uint8_t axis, uint8_t axis_limit);
void cnc_reset_position();

#endif
