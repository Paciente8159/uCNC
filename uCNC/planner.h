/*
	Name: 
	Copyright: 
	Author: 
	Date: 23/09/19 23:14
	Description: 
*/

#ifndef PLANNER_H
#define PLANNER_H

#include <stdint.h>
#include "config.h"
#include "machinedefs.h"

typedef struct
{
	uint8_t type;
	uint8_t dirs;
	uint32_t steps[STEPPER_COUNT];
	uint32_t totalsteps;
	uint32_t accel_until;
	uint32_t deaccel_from;
	uint16_t accel_rate;
	uint16_t speed_rate;
} PLANNER_MOTION;

void planner_init();
void planner_add_line(float* axis, float feed);
void planner_add_analog_output(uint8_t output, uint8_t value);
void planner_add_digital_output(uint8_t output, uint8_t value);
void planner_add_arc(float* axis, float* center, uint8_t clockwise, uint8_t plane, float feed);

#endif
