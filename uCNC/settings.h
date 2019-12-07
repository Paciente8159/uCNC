/*
	Name: setting.h - uCNC settings functionalities
	Copyright: 2019 João Martins
	Author: João Martins
	Date: Nov/2019
	Description: uCNC is a free cnc controller software designed to be flexible and
	portable to several	microcontrollers/architectures.
	uCNC is a FREE SOFTWARE under the terms of the GPLv3 (see <http://www.gnu.org/licenses/>).
*/

#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "machinedefs.h"

typedef struct
{
	char version[3];
	float step_per_mm[AXIS_COUNT];
	float max_feed_rate[AXIS_COUNT];
	float acceleration[AXIS_COUNT];
	float max_distance[AXIS_COUNT];
	bool step_enable_mask;
	uint8_t step_invert_mask;
    uint8_t dir_invert_mask;
    uint8_t homing_dir_invert_mask;
    float homing_feed_rate;
  	float homing_seek_rate;
  	float homing_offset;
	float arc_tolerance;
	uint8_t tool_count;
	uint8_t crc;
} settings_t;

extern settings_t g_settings;

void settings_load();
void settings_reset();
void settings_save();

#endif
