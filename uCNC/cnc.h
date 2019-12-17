#ifndef CNC_H
#define CNC_H

#include <stdbool.h>
#include <stdint.h>
#include "config.h"
#include "machinedefs.h"

//realtime commands oredered by priority (high to low)
#define RT_CMD_RESET			32
#define RT_CMD_SAFETY_DOOR		16
#define RT_CMD_FEED_HOLD		8
#define RT_CMD_JOG_CANCEL		4
#define RT_CMD_CYCLE_START		2
#define RT_CMD_REPORT			1

#define RT_CLEAR_RESET			0
#define RT_CLEAR_SAFETY_DOOR	~(RT_CMD_RESET - 1)
#define RT_CLEAR_FEED_HOLD		~(RT_CMD_SAFETY_DOOR - 1)
#define RT_CLEAR_JOG_CANCEL		~(RT_CMD_FEED_HOLD - 1)
#define RT_CLEAR_CYCLE_START	~(RT_CMD_JOG_CANCEL - 1)
#define RT_CLEAR_REPORT			~(RT_CMD_REPORT)

//current cnc states (multiple can be active at the same time)
#define EXEC_IDLE			0 // All flags cleared
#define EXEC_RUN			1 // Motions are being executed
#define EXEC_JOG			2 // Jogging motion
#define EXEC_JOGCANCEL		4
#define EXEC_HOMING			8 // Homing motion
#define EXEC_HOLD			16 // Feed hold is active
#define EXEC_DOOR			32
#define EXEC_ALARM			64 //Alarm mode
#define EXEC_SLEEP			128 //Sleep mode
/*
#define KILL_FLAG_MOTION		1
#define KILL_FLAG_FEED			2
#define KILL_FLAG_COOLANT		4
#define KILL_FLAG_ALL			(KILL_FLAG_MOTION | KILL_FLAG_FEED | KILL_FLAG_COOLANT)*/

typedef struct
{
    bool halt;
    bool dry_run;
    bool unlocked;
    bool is_homed;
    uint8_t rt_cmd;
    uint8_t exec_state;
    uint8_t alarm;
    uint8_t limits;
    uint8_t controls;
    uint32_t rt_position[STEPPER_COUNT];
} cnc_state_t;

extern cnc_state_t g_cnc_state;

void cnc_init();
void cnc_reset();
void cnc_run();
void cnc_doevents();
void cnc_home();
void cnc_alarm(uint8_t code);
void cnc_kill();
void cnc_unlock();

#endif
