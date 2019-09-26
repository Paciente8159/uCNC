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



void planner_init();
void planner_add_line(float* axis, float feed);
void planner_add_analog_output(uint8_t output, uint8_t value);
void planner_add_digital_output(uint8_t output, uint8_t value);
void planner_add_curve(float* axis, float* center, float feed);

#endif

