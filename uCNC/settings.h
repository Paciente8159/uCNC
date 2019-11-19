/*
	Name: 
	Copyright: 
	Author: 
	Date: 24/09/19 10:16
	Description: 
*/

#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>
#include "config.h"
#include "machinedefs.h"

typedef struct
{
	uint16_t step_mm[AXIS_COUNT];
	float max_speed[AXIS_COUNT];
	float max_accel[AXIS_COUNT];
	float max_x;
	float max_y;
	float max_z;
	float arc_tolerance;
} SETTINGS;

extern SETTINGS g_settings;

void settings_load();
void settings_reset();
void settings_save();

#endif
