/*
        Name: motion_control.h
        Description: Contains the building blocks for performing motions/actions in µCNC.

        Copyright: Copyright (c) João Martins
        Author: João Martins
        Date: 19/11/2019

        µCNC is free software: you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version. Please see <http://www.gnu.org/licenses/>

        µCNC is distributed WITHOUT ANY WARRANTY;
        Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
        See the	GNU General Public License for more details.
*/

#ifndef MOTION_CONTROL_H
#define MOTION_CONTROL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

#define MOTIONCONTROL_MODE_FEED 0
#define MOTIONCONTROL_MODE_INVERSEFEED 1
#define MOTIONCONTROL_MODE_BACKLASH_COMPENSATION 2
#define MOTIONCONTROL_MODE_PAUSEPROGRAM 4
#define MOTIONCONTROL_MODE_PAUSEPROGRAM_CONDITIONAL 8

#define MOTIONCONTROL_PROBE_INVERT 1
#define MOTIONCONTROL_PROBE_NOALARM_ONFAIL 2

        typedef struct
        {
#ifdef GCODE_PROCESS_LINE_NUMBERS
                uint32_t line;
#endif
                step_t steps[STEPPER_COUNT];
                float dir_vect[AXIS_COUNT];
                uint8_t dirbits;
                uint32_t full_steps; // number of steps of all linear actuators
                step_t total_steps;  // the number of pulses needed to generate all steps (maximum of all linear actuators)
                float feed;
                uint8_t main_stepper;
                int16_t spindle;
                uint16_t dwell;
                uint8_t motion_mode;
#if TOOL_COUNT > 0
                uint8_t coolant;
#endif
                bool update_tools;
                bool feed_override;
                bool is_subsegment;
        } motion_data_t;

        void mc_init(void);
        bool mc_get_checkmode(void);
        bool mc_toogle_checkmode(void);

        // async motions
        uint8_t mc_line(float *target, motion_data_t *block_data);
        uint8_t mc_arc(float *target, float center_offset_a, float center_offset_b, float radius, uint8_t axis_0, uint8_t axis_1, bool isclockwise, motion_data_t *block_data);

        // sync motions
        uint8_t mc_dwell(motion_data_t *block_data);
        uint8_t mc_pause(void);
        uint8_t mc_update_tools(motion_data_t *block_data);

        // mixed/special motions
        uint8_t mc_home_axis(uint8_t axis, uint8_t axis_limit);
        uint8_t mc_probe(float *target, uint8_t flags, motion_data_t *block_data);

        void mc_get_position(float *target);
        void mc_sync_position(void);

#ifdef __cplusplus
}
#endif

#endif
