/*
	Name: settings.h
	Description: µCNC runtime settings. These functions store settings and other parameters
		in non-volatile memory.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 26/09/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef SETTINGS_H
#define SETTINGS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../cnc.h"
#include <stdint.h>
#include <stdbool.h>

        typedef struct
        {
                char version[3];
                float max_step_rate;
                //step delay not used
                uint8_t step_invert_mask;
                uint8_t dir_invert_mask;

                uint8_t limits_invert_mask;

                uint8_t status_report_mask;
                uint8_t control_invert_mask;
                //value must be set between -1.0 and 1.0 If set to 0.0 is the same as exact path mode (G61) and -1.0 is the same as exact stop mode (G61.1)
                float g64_angle_factor;
                //juntion deviation is automatic and always on
                float arc_tolerance;
                uint16_t debounce_ms;
                uint8_t homing_dir_invert_mask;
                float homing_fast_feed_rate;
                float homing_slow_feed_rate;
                //debouncing not used
                float homing_offset;
                float spindle_max_rpm;
                float spindle_min_rpm;

                float step_per_mm[STEPPER_COUNT];
                float max_feed_rate[STEPPER_COUNT];
                float acceleration[STEPPER_COUNT];
                float max_distance[AXIS_COUNT];
                uint8_t tool_count;
                union
                {
                        struct
                        {
                                uint8_t step_enable_invert : 1;
                                uint8_t probe_invert_mask : 1;
                                uint8_t report_inches : 1;
                                uint8_t soft_limits_enabled : 1;
                                uint8_t hard_limits_enabled : 1;
                                uint8_t homing_enabled : 1;
                                uint8_t laser_mode : 1;
                                uint8_t : 1; //unused
                        };
                        uint8_t flags;
                };
#ifdef ENABLE_BACKLASH_COMPENSATION
                uint16_t backlash_steps[STEPPER_COUNT];
#endif
#ifdef ENABLE_SKEW_COMPENSATION
                float skew_xy_factor;
#ifndef SKEW_COMPENSATION_XY_ONLY
                float skew_xz_factor;
                float skew_yz_factor;
#endif
#endif
                
#if PID_CONTROLLERS > 0
                float pid_gain[PID_CONTROLLERS][3];
#endif
        } settings_t;

#ifndef SETTINGS_ADDRESS_OFFSET
#define SETTINGS_ADDRESS_OFFSET 0
#endif
#ifndef SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET
#define SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET (SETTINGS_ADDRESS_OFFSET + sizeof(settings_t) + 1)
#endif
#ifndef STARTUP_BLOCK0_ADDRESS_OFFSET
#define STARTUP_BLOCK0_ADDRESS_OFFSET (SETTINGS_PARSER_PARAMETERS_ADDRESS_OFFSET + (((AXIS_COUNT * sizeof(float)) + 1) * (COORD_SYS_COUNT + 3)))
#endif
#ifndef STARTUP_BLOCK1_ADDRESS_OFFSET
#define STARTUP_BLOCK1_ADDRESS_OFFSET (STARTUP_BLOCK0_ADDRESS_OFFSET + RX_BUFFER_SIZE)
#endif

        extern settings_t g_settings;

        void settings_init(void);
        //Assumes that no structure being saved is bigger than 255 bytes
        uint8_t settings_load(uint16_t address, uint8_t *__ptr, uint8_t size);
        void settings_save(uint16_t address, const uint8_t *__ptr, uint8_t size);
        void settings_reset(void);
        uint8_t settings_change(uint8_t setting, float value);
        void settings_erase(uint16_t address, uint8_t size);
        bool settings_check_startup_gcode(uint16_t address);
        void settings_save_startup_gcode(uint16_t address);

#ifdef __cplusplus
}
#endif

#endif
