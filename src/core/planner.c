/*
	Name: planner.c
	Description: Chain planner for linear motions and acceleration/deacceleration profiles.
        It uses a similar algorithm to Grbl.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 24/09/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifdef __cplusplus
extern "C"
{
#endif

#include "cnc.h"
#include "interface/grbl_interface.h"
#include "interface/settings.h"
#include "core/planner.h"
#include "core/motion_control.h"
#include "core/interpolator.h"
#include "core/io_control.h"
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <float.h>

    typedef struct
    {
        uint8_t feed_override;
        uint8_t rapid_feed_override;
#ifdef USE_SPINDLE
        uint8_t spindle_override;
#endif
#ifdef USE_COOLANT
        uint8_t coolant_override;
#endif
        bool overrides_enabled;

    } planner_overrides_t;

    static int32_t planner_step_pos[STEPPER_COUNT];
#ifdef USE_SPINDLE
    static int16_t planner_spindle;
#endif
#ifdef USE_COOLANT
    static uint8_t planner_coolant;
#endif
    static planner_block_t planner_data[PLANNER_BUFFER_SIZE];
    static uint8_t planner_data_write;
    static uint8_t planner_data_read;
    static uint8_t planner_data_slots;
    static planner_overrides_t planner_overrides;
    static uint8_t planner_ovr_counter;

    static void planner_buffer_write(void);
    static void planner_buffer_read(void);
    FORCEINLINE static uint8_t planner_buffer_next(uint8_t index);
    FORCEINLINE static uint8_t planner_buffer_prev(uint8_t index);
    FORCEINLINE static void planner_recalculate(void);
    FORCEINLINE static void planner_buffer_clear(void);

    /*
	Adds a new line to the trajectory planner
	The planner is responsible for calculating the entry and exit speeds of the transitions
	The trajectory planner does the following actions:
		1. Calculates the direction change of the new movement
		2. Adjusts maximum entry feed according to the angle of the junction point
		3. Recalculates all chained segments

	For profiling the motion 4 feeds are calculated
		1. The target feed
		2. The rapid motion feed given the direction (maximum allowed feed with overrides)
		3. The entry feed (initialy set to 0)
		4. The maximum entry feed given the juntion angle between planner blocks
*/
    void planner_add_line(int32_t *target, motion_data_t *block_data)
    {
#ifdef ENABLE_LINACT_PLANNER
        static float last_dir_vect[STEPPER_COUNT];
#else
    static float last_dir_vect[AXIS_COUNT];
#endif

        planner_data[planner_data_write].dirbits = block_data->dirbits;
        planner_data[planner_data_write].total_steps = block_data->total_steps;
        //planner_data[planner_data_write].step_indexer = block_data->step_indexer;
        planner_data[planner_data_write].optimal = false;
        planner_data[planner_data_write].acceleration = 0;
        planner_data[planner_data_write].rapid_feed_sqr = 0;
        planner_data[planner_data_write].feed_sqr = 0;
        //sets entry and max entry feeds as if it would start and finish from a stoped state
        planner_data[planner_data_write].entry_feed_sqr = 0;
        planner_data[planner_data_write].entry_max_feed_sqr = 0;
#ifdef USE_SPINDLE
        planner_spindle = planner_data[planner_data_write].spindle = block_data->spindle;
#endif
#ifdef USE_COOLANT
        planner_coolant = planner_data[planner_data_write].coolant = block_data->coolant;
#endif
#ifdef GCODE_PROCESS_LINE_NUMBERS
        planner_data[planner_data_write].line = block_data->line;
#endif
        planner_data[planner_data_write].dwell = block_data->dwell;

#ifdef ENABLE_BACKLASH_COMPENSATION
        if (CHECKFLAG(block_data->motion_mode, MOTIONCONTROL_MODE_BACKLASH_COMPENSATION))
        {
            planner_data[planner_data_write].backlash_comp = true;
        }
#endif

        if (CHECKFLAG(block_data->motion_mode, MOTIONCONTROL_MODE_NOMOTION))
        {
            memset(planner_data[planner_data_write].steps, 0, sizeof(planner_data[planner_data_write].steps));
            planner_data[planner_data_write].total_steps = 0;
            planner_buffer_write();
            return;
        }
        else
        {
            memcpy(planner_data[planner_data_write].steps, block_data->steps, sizeof(planner_data[planner_data_write].steps));
            planner_data[planner_data_write].total_steps = block_data->total_steps;
        }

        //calculates the normalized vector with the amount of motion in any linear actuator
        //also calculates the maximum feedrate and acceleration for each linear actuator
#ifdef ENABLE_LINACT_PLANNER
        float inv_total_steps = 1.0f / (float)(block_data->full_steps);
#endif
#ifdef ENABLE_LINACT_COLD_START
        bool coldstart = false;
#endif
        float cos_theta = 0;
        float rapid_feed = FLT_MAX;
        planner_data[planner_data_write].acceleration = FLT_MAX;

#ifdef ENABLE_LINACT_PLANNER
        float dir_vect[STEPPER_COUNT];
        memset(dir_vect, 0, sizeof(dir_vect));
#else
    for (uint8_t i = AXIS_COUNT; i != 0;)
    {
        i--;
        cos_theta += block_data->dir_vect[i] * last_dir_vect[i];
        last_dir_vect[i] = block_data->dir_vect[i];
    }
#endif

        for (uint8_t i = STEPPER_COUNT; i != 0;)
        {
            i--;
            if (planner_data[planner_data_write].steps[i] != 0)
            {
#ifdef ENABLE_LINACT_PLANNER
                dir_vect[i] = inv_total_steps * (float)planner_data[planner_data_write].steps[i];

                if (!planner_buffer_is_empty())
                {
                    cos_theta += last_dir_vect[i] * dir_vect[i];
#ifdef ENABLE_LINACT_COLD_START
                    if (last_dir_vect[i] == 0) //tests if actuator is starting from a full stop
                    {
                        coldstart = true;
                    }
#endif
                }

                last_dir_vect[i] = dir_vect[i];
#endif
                //calculate (per linear actuator) the minimum inverted time of travel (1/min) an acceleration (1/s^2)
                float step_ratio = g_settings.step_per_mm[i] / (float)planner_data[planner_data_write].steps[i];
                float stepper_feed = g_settings.max_feed_rate[i] * step_ratio;
                rapid_feed = MIN(rapid_feed, stepper_feed);
                float stepper_accel = g_settings.acceleration[i] * step_ratio;
                planner_data[planner_data_write].acceleration = MIN(planner_data[planner_data_write].acceleration, stepper_accel);
            }
            else
            {
                last_dir_vect[i] = 0;
            }
        }

        //converts to steps per second (st/s)
        block_data->feed *= MIN_SEC_MULT;
        rapid_feed *= MIN_SEC_MULT;
        rapid_feed *= (float)block_data->total_steps;
        //converts to steps per second^2 (st/s^2)
        planner_data[planner_data_write].acceleration *= (float)block_data->total_steps;

        if (block_data->feed > rapid_feed)
        {
            block_data->feed = rapid_feed;
        }

        planner_data[planner_data_write].feed_sqr = fast_flt_pow2(block_data->feed);
        planner_data[planner_data_write].rapid_feed_sqr = fast_flt_pow2(rapid_feed);

        //consider initial angle factor of 1 (90 degree angle corner or more)
        float angle_factor = 1.0f;
        uint8_t prev = 0;

        if (!planner_buffer_is_empty())
        {
            prev = planner_buffer_prev(planner_data_write); //BUFFER_PTR(planner_buffer, prev_index);
#ifdef ENABLE_LINACT_COLD_START
            if ((planner_data[prev].dirbits ^ planner_data[planner_data_write].dirbits))
            {
                cos_theta = 0;
            }
#endif
        }
        else
        {
            cos_theta = 0;
        }

        //if more than one move stored cals juntion speeds and recalculates speed profiles
        if (cos_theta != 0 && !CHECKFLAG(block_data->motion_mode, PLANNER_MOTION_EXACT_STOP | MOTIONCONTROL_MODE_BACKLASH_COMPENSATION))
        {
            //calculates the junction angle with previous
            if (cos_theta > 0)
            {
                //uses the half angle identity conversion to convert from cos(theta) to tan(theta/2) where:
                //	tan(theta/2) = sqrt((1-cos(theta)/(1+cos(theta))
                //to simplify the calculations it multiplies by sqrt((1+cos(theta)/(1+cos(theta))
                //transforming the equation to sqrt((1^2-cos(theta)^2))/(1+cos(theta))
                //this way the output will be between 0<tan(theta/2)<inf
                //but if theta is 0<theta<90 the tan(theta/2) will be 0<tan(theta/2)<1
                //all angles greater than 1 that can be excluded
                angle_factor = 1.0f / (1.0f + cos_theta);
                cos_theta = (1.0f - fast_flt_pow2(cos_theta));
                angle_factor *= fast_flt_sqrt(cos_theta);
            }

            //sets the maximum allowed speed at junction (if angle doesn't force a full stop)
            float factor = ((!CHECKFLAG(block_data->motion_mode, PLANNER_MOTION_CONTINUOUS)) ? 0 : G64_MAX_ANGLE_FACTOR);
            angle_factor = MAX(angle_factor - factor, 0);

            if (angle_factor < 1.0f)
            {
                float junc_feed_sqr = (1 - angle_factor);
                junc_feed_sqr = fast_flt_pow2(junc_feed_sqr);
                junc_feed_sqr *= planner_data[prev].feed_sqr;
                //the maximum feed is the minimal feed between the previous feed given the angle and the current feed
                planner_data[planner_data_write].entry_max_feed_sqr = MIN(planner_data[planner_data_write].feed_sqr, junc_feed_sqr);
            }

            //forces reaclculation with the new block
            planner_recalculate();
        }

        //advances the buffer
        planner_buffer_write();
        //updates the current planner coordinates
        if (target != NULL)
        {
            memcpy(planner_step_pos, target, sizeof(planner_step_pos));
        }
    }

    /*
	Planner buffer functions
*/
    static void planner_buffer_read(void)
    {
        planner_data_slots++;
        if (++planner_data_read == PLANNER_BUFFER_SIZE)
        {
            planner_data_read = 0;
        }
    }

    static void planner_buffer_write(void)
    {
        planner_data_slots--;
        if (++planner_data_write == PLANNER_BUFFER_SIZE)
        {
            planner_data_write = 0;
        }
    }

    static uint8_t planner_buffer_next(uint8_t index)
    {
        if (++index == PLANNER_BUFFER_SIZE)
        {
            index = 0;
        }

        return index;
    }

    static uint8_t planner_buffer_prev(uint8_t index)
    {
        if (index == 0)
        {
            index = PLANNER_BUFFER_SIZE;
        }

        return --index;
    }

    bool planner_buffer_is_empty(void)
    {
        return (planner_data_slots == PLANNER_BUFFER_SIZE);
    }

    bool planner_buffer_is_full(void)
    {
        return (planner_data_slots == 0);
    }

    static void planner_buffer_clear(void)
    {
        planner_data_write = 0;
        planner_data_read = 0;
        planner_data_slots = PLANNER_BUFFER_SIZE;
#ifdef FORCE_GLOBALS_TO_0
        memset(planner_data, 0, sizeof(planner_data));
#endif
    }

    void planner_init(void)
    {
#ifdef FORCE_GLOBALS_TO_0
        memset(planner_step_pos, 0, sizeof(planner_step_pos));
        //resets buffer
        memset(planner_data, 0, sizeof(planner_data));
#endif
        planner_buffer_clear();
        planner_overrides.overrides_enabled = true;
        planner_feed_ovr_reset();
        planner_rapid_feed_ovr_reset();
#ifdef USE_SPINDLE
        planner_spindle_ovr_reset();
#endif
#ifdef USE_COOLANT
        planner_coolant_ovr_reset();
#endif
    }

    void planner_clear(void)
    {
        //clears all motions stored in the buffer
        planner_buffer_clear();
#ifdef USE_SPINDLE
        planner_spindle = 0;
#endif
#ifdef USE_COOLANT
        planner_coolant = 0;
#endif
        //resyncs position with interpolator
        planner_resync_position();
        //forces motion control to resync postition after clearing the planner buffer
        mc_resync_position();
    }

    planner_block_t *planner_get_block(void)
    {
        return &planner_data[planner_data_read];
    }

    float planner_get_block_exit_speed_sqr(void)
    {
        //only one block in the buffer (exit speed is 0)
        if (PLANNER_BUFFER_SIZE - planner_data_slots < 2)
            return 0;

        //exit speed = next block entry speed
        uint8_t next = planner_buffer_next(planner_data_read);
        float exit_speed_sqr = planner_data[next].entry_feed_sqr;
        float rapid_feed_sqr = planner_data[next].rapid_feed_sqr;

        if (planner_overrides.overrides_enabled)
        {
            if (planner_overrides.feed_override != 100)
            {
                exit_speed_sqr *= fast_flt_pow2((float)planner_overrides.feed_override);
                exit_speed_sqr *= 0.0001f;
            }

            //if rapid overrides are active the feed must not exceed the rapid motion feed
            if (planner_overrides.rapid_feed_override != 100)
            {
                rapid_feed_sqr *= fast_flt_pow2((float)planner_overrides.rapid_feed_override);
                rapid_feed_sqr *= 0.0001f;
            }
        }

        return MIN(exit_speed_sqr, rapid_feed_sqr);
    }

    float planner_get_block_top_speed(void)
    {
        /*
    Computed the junction speed

    At full acceleration and deacceleration we have the following equations
    	v_max_entry^2 = v_entry^2 + 2 * d_start * acceleration
    	v_max_exit^2 = v_exit^2 + 2 * d_deaccel * acceleration

    In this case v_max_entry^2 = v_max_exit^2 at the point where

    d_deaccel = d_total - d_start;

    this translates to the equation

    v_max^2 = (v_exit^2 + 2 * acceleration * distance + v_entry)/2
    */
        float exit_speed_sqr = planner_get_block_exit_speed_sqr();
        float speed_delta = exit_speed_sqr + planner_data[planner_data_read].entry_feed_sqr;
        float speed_change = planner_data[planner_data_read].acceleration * (float)(planner_data[planner_data_read].total_steps);
        speed_change = fast_flt_mul2(speed_change);
        speed_change += speed_delta;
        float junction_speed_sqr = fast_flt_div2(speed_change);
        float rapid_feed_sqr = planner_data[planner_data_read].rapid_feed_sqr;
        float target_speed_sqr = planner_data[planner_data_read].feed_sqr;
        if (planner_overrides.overrides_enabled)
        {
            if (planner_overrides.feed_override != 100)
            {
                target_speed_sqr *= fast_flt_pow2((float)planner_overrides.feed_override);
                target_speed_sqr *= 0.0001f;
            }

            //if rapid overrides are active the feed must not exceed the rapid motion feed
            if (planner_overrides.rapid_feed_override != 100)
            {
                rapid_feed_sqr *= fast_flt_pow2((float)planner_overrides.rapid_feed_override);
                rapid_feed_sqr *= 0.0001f;
            }
        }

        //can't ever exceed rapid move speed
        target_speed_sqr = MIN(target_speed_sqr, rapid_feed_sqr);
        return MIN(junction_speed_sqr, target_speed_sqr);
    }

#ifdef USE_SPINDLE
    void planner_get_spindle_speed(float scale, uint8_t *pwm, bool *invert)
    {
        float spindle = (planner_data_slots == PLANNER_BUFFER_SIZE) ? planner_spindle : planner_data[planner_data_read].spindle;
        *pwm = 0;
        *invert = (spindle < 0);

        if (spindle != 0)
        {
            spindle = ABS(spindle);
#ifdef LASER_MODE
            if (g_settings.laser_mode && *invert) //scales laser power only if invert is active (M4)
            {
                spindle *= scale; //scale calculated in laser mode (otherwise scale is always 1)
            }
#endif
            if (planner_overrides.overrides_enabled && planner_overrides.spindle_override != 100)
            {
                spindle = 0.01f * (float)planner_overrides.spindle_override * spindle;
            }
            spindle = MIN(spindle, g_settings.spindle_max_rpm);
            spindle = MAX(spindle, g_settings.spindle_min_rpm);
            *pwm = (uint8_t)truncf(255 * (spindle / g_settings.spindle_max_rpm));
            *pwm = MAX(*pwm, PWM_MIN_OUTPUT);
        }
    }

    float planner_get_previous_spindle_speed(void)
    {
        return planner_spindle;
    }
#endif

#ifdef USE_COOLANT
    uint8_t planner_get_coolant(void)
    {
        uint8_t coolant = (planner_data_slots == PLANNER_BUFFER_SIZE) ? planner_coolant : planner_data[planner_data_read].coolant;

        if (planner_overrides.overrides_enabled)
        {
            coolant ^= planner_overrides.coolant_override;
        }

        return coolant;
    }

    uint8_t planner_get_previous_coolant(void)
    {
        return planner_coolant;
    }
#endif

    void planner_discard_block(void)
    {
        planner_buffer_read();
    }

    static void planner_recalculate(void)
    {
        uint8_t last = planner_data_write;
        uint8_t first = planner_data_read;
        uint8_t block = planner_data_write;
        //starts in the last added block
        //calculates the maximum entry speed of the block so that it can do a full stop in the end
        float doubledistaccel = ((float)(planner_data[block].total_steps << 1)) * planner_data[block].acceleration;
        float entry_feed_sqr = (planner_data[block].dwell == 0) ? (doubledistaccel) : 0;
        planner_data[block].entry_feed_sqr = MIN(planner_data[block].entry_max_feed_sqr, entry_feed_sqr);
        //optimizes entry speeds given the current exit speed (backward pass)
        uint8_t next = block;
        block = planner_buffer_prev(block);

        while (!planner_data[block].optimal && block != first)
        {
            if (planner_data[block].dwell != 0)
            {
                planner_data[block].entry_feed_sqr = 0;
            }
            else if (planner_data[block].entry_feed_sqr != planner_data[block].entry_max_feed_sqr)
            {
                entry_feed_sqr = planner_data[next].entry_feed_sqr + doubledistaccel;
                planner_data[block].entry_feed_sqr = MIN(planner_data[block].entry_max_feed_sqr, entry_feed_sqr);
            }

            next = block;
            block = planner_buffer_prev(block);
        }

        //optimizes exit speeds (forward pass)
        while (block != last)
        {
            //next block is moving at a faster speed
            if (planner_data[block].entry_feed_sqr < planner_data[next].entry_feed_sqr)
            {
                //check if the next block entry speed can be achieved
                float exit_speed_sqr = planner_data[block].entry_feed_sqr + (doubledistaccel);
                if (exit_speed_sqr < planner_data[next].entry_feed_sqr)
                {
                    //lowers next entry speed (aka exit speed) to the maximum reachable speed from current block
                    //optimization achieved for this movement
                    planner_data[next].entry_feed_sqr = exit_speed_sqr;
                    planner_data[next].optimal = true;
                }
            }

            //if the executing block was updated then update the interpolator limits
            if (block == first)
            {
                itp_update();
            }

            block = next;
            next = planner_buffer_next(block);
        }
    }

    void planner_get_position(int32_t *steps)
    {
        memcpy(steps, planner_step_pos, sizeof(planner_step_pos));
    }

    void planner_resync_position(void)
    {
        //resyncs the position with the interpolator
        itp_get_rt_position(planner_step_pos);
    }

    //overrides
    void planner_toggle_overrides(void)
    {
        planner_overrides.overrides_enabled = !planner_overrides.overrides_enabled;
        itp_update();
        planner_ovr_counter = 0;
    }

    bool planner_get_overrides(void)
    {
        return planner_overrides.overrides_enabled;
    }

    void planner_feed_ovr_inc(uint8_t value)
    {
        uint8_t ovr_val = planner_overrides.feed_override;
        ovr_val += value;
        ovr_val = MAX(ovr_val, FEED_OVR_MIN);
        ovr_val = MIN(ovr_val, FEED_OVR_MAX);

        if (planner_overrides.overrides_enabled && ovr_val != planner_overrides.feed_override)
        {
            planner_overrides.feed_override = ovr_val;
            planner_ovr_counter = 0;
            itp_update();
        }
    }

    void planner_rapid_feed_ovr(uint8_t value)
    {
        if (planner_overrides.overrides_enabled && planner_overrides.rapid_feed_override != value)
        {
            planner_overrides.rapid_feed_override = value;
            planner_ovr_counter = 0;
            itp_update();
        }
    }

    void planner_feed_ovr_reset(void)
    {
        if (planner_overrides.overrides_enabled && planner_overrides.feed_override != 100)
        {
            itp_update();
        }
        planner_overrides.feed_override = 100;
        planner_ovr_counter = 0;
    }

    void planner_rapid_feed_ovr_reset(void)
    {
        if (planner_overrides.overrides_enabled && planner_overrides.rapid_feed_override != 100)
        {
            itp_update();
        }
        planner_overrides.rapid_feed_override = 100;
        planner_ovr_counter = 0;
    }
#ifdef USE_SPINDLE
    void planner_spindle_ovr_inc(uint8_t value)
    {
        uint8_t ovr_val = planner_overrides.spindle_override;
        ovr_val += value;
        ovr_val = MAX(ovr_val, FEED_OVR_MIN);
        ovr_val = MIN(ovr_val, FEED_OVR_MAX);

        if (planner_overrides.overrides_enabled && ovr_val != planner_overrides.spindle_override)
        {
            planner_overrides.spindle_override = ovr_val;
            planner_ovr_counter = 0;
        }
    }

    void planner_spindle_ovr_reset(void)
    {
        planner_overrides.spindle_override = 100;
        planner_ovr_counter = 0;
    }
#endif

#ifdef USE_COOLANT
    uint8_t planner_coolant_ovr_toggle(uint8_t value)
    {
        planner_overrides.coolant_override ^= value;
        return planner_overrides.coolant_override;
    }

    void planner_coolant_ovr_reset(void)
    {
        planner_coolant = 0;
        planner_overrides.coolant_override = 0;
    }
#endif

    bool planner_get_overflows(uint8_t *overflows)
    {
        if (!planner_ovr_counter)
        {
            overflows[0] = planner_overrides.feed_override;
            overflows[1] = planner_overrides.rapid_feed_override;
#ifdef USE_SPINDLE
            overflows[2] = planner_overrides.spindle_override;
#else
        overflows[2] = 0;
#endif
            planner_ovr_counter = STATUS_WCO_REPORT_MIN_FREQUENCY;
            return true;
        }

        planner_ovr_counter--;
        return false;
    }

#ifdef __cplusplus
}
#endif