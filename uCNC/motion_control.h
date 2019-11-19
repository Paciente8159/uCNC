/*
	Name: 
	Copyright: 
	Author: 
	Date: 23/09/19 23:14
	Description: 
*/

#ifndef MOTION_CONTROL_H
#define MOTION_CONTROL_H

#include <stdint.h>
//#include <stdbool.h>

void mc_init();
void mc_line(float* target, float feed);
void mc_arc(float* target, float* center_offset, float radius, uint8_t plane, bool isclockwise, float feed);

#endif
