#ifndef CNC_H
#define CNC_H

#include <stdbool.h>
#include <stdint.h>
#include "config.h"
#include "machinedefs.h"

#define RT_CMD_REPORT			1
#define RT_CMD_CYCLE_START		2
#define RT_CMD_CYCLE_STOP		4
#define RT_CMD_FEED_HOLD		8
#define RT_CMD_RESET			16
#define RT_CMD_SAFETY_DOOR		32
#define RT_CMD_MOTION_CANCEL	64
#define RT_CMD_SLEEP			128

#define EXEC_IDLE			0 // All flags cleared
#define EXEC_CYCLE			1 // Cycle is running or motions are being executed
#define EXEC_JOG			2 // Jogging motion
#define EXEC_HOMING			4 // Homing motion
#define EXEC_HOLD			8 // Feed hold is active
#define EXEC_ALARM			16 //Alarm mode

#define KILL_FLAG_MOTION		1
#define KILL_FLAG_FEED			2
#define KILL_FLAG_COOLANT		4
#define KILL_FLAG_ALL			(KILL_FLAG_MOTION | KILL_FLAG_FEED | KILL_FLAG_COOLANT)

typedef struct
{
    bool halt;
    bool unlocked;
    bool is_homed;
    uint8_t rt_cmd;
    uint8_t exec_state;
    uint8_t hard_limits;
    uint8_t soft_limits;
    uint8_t controls;
    uint32_t rt_position[STEPPER_COUNT];
} cnc_state_t;

extern cnc_state_t g_cnc_state;

void cnc_init();
void cnc_reset();
void cnc_run();
void cnc_doevents();
void cnc_unhome();
void cnc_kill();

#endif
