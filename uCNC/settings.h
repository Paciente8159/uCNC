/*
	Name: settings.h
	Description: uCNC runtime settings. These functions store settings and other parameters
		in non-volatile memory.
		
	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 26/09/2019

	uCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	uCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "machinedefs.h"

#define SETTINGS_ADDRESS_OFFSET 0
#define SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET 512

typedef struct
{
	char version[3];
	uint16_t max_step_rate;
	//step delay not used
	uint8_t step_invert_mask;
    uint8_t dir_invert_mask;
    bool step_enable_invert;
    uint8_t limits_invert_mask;
	bool probe_invert_mask;
    uint8_t status_report_mask;
    uint8_t control_invert_mask;
	//juntion deviation is automatic and always on
	float arc_tolerance;
	bool report_inches;
	bool soft_limits_enabled;
	bool hard_limits_enabled;
	bool homing_enabled;
	uint8_t homing_dir_invert_mask;
	float homing_fast_feed_rate;
	float homing_slow_feed_rate;
	//debouncing not used
	float homing_offset;
	float spindle_max_rpm;
	float spindle_min_rpm;
	
	float step_per_mm[AXIS_COUNT];
	float max_feed_rate[AXIS_COUNT];
	float acceleration[AXIS_COUNT];
	float max_distance[AXIS_COUNT];
	
	uint8_t tool_count;
	//uint8_t crc;
} settings_t;

extern settings_t g_settings;

bool settings_init();
bool settings_load(uint16_t address, uint8_t* __ptr, uint16_t size);
void settings_save(uint16_t address, const uint8_t* __ptr, uint16_t size);
void settings_reset();
uint8_t settings_change(uint8_t setting, float value);

#endif
