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

#include "../cnc.h"
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <float.h>

typedef struct
{
    uint8_t feed_override;
    uint8_t rapid_feed_override;
#if TOOL_COUNT > 0
    uint8_t spindle_override;
    uint8_t coolant_override;
#endif
} planner_overrides_t;

#if TOOL_COUNT > 0
static int16_t planner_spindle;
static uint8_t planner_coolant;
#endif
static planner_block_t planner_data[PLANNER_BUFFER_SIZE];
static uint8_t planner_data_write;
static uint8_t planner_data_read;
static uint8_t planner_data_blocks;
static planner_overrides_t planner_overrides;
static uint8_t planner_ovr_counter;

FORCEINLINE static void planner_add_block(void);
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
void planner_add_line(motion_data_t *block_data)
{
#ifdef ENABLE_LINACT_PLANNER
    static float last_dir_vect[STEPPER_COUNT];
#else
    static float last_dir_vect[AXIS_COUNT];
#endif
    // clear the planner block
    memset(&planner_data[planner_data_write], 0, sizeof(planner_block_t));
    planner_data[planner_data_write].dirbits = block_data->dirbits;
    planner_data[planner_data_write].main_stepper = block_data->main_stepper;
    planner_data[planner_data_write].flags_u.flags_t.feed_override = block_data->feed_override;

#if TOOL_COUNT > 0
    planner_spindle = planner_data[planner_data_write].spindle = block_data->spindle;
    planner_coolant = planner_data[planner_data_write].flags_u.flags_t.coolant = block_data->coolant;
#endif
#ifdef GCODE_PROCESS_LINE_NUMBERS
    planner_data[planner_data_write].line = block_data->line;
#endif

#ifdef ENABLE_BACKLASH_COMPENSATION
    if (CHECKFLAG(block_data->motion_mode, MOTIONCONTROL_MODE_BACKLASH_COMPENSATION))
    {
        planner_data[planner_data_write].backlash_comp = true;
    }
#endif

    memcpy(planner_data[planner_data_write].steps, block_data->steps, sizeof(planner_data[planner_data_write].steps));
    planner_data[planner_data_write].total_steps = block_data->total_steps;

    // calculates the normalized vector with the amount of motion in any linear actuator
    // also calculates the maximum feedrate and acceleration for each linear actuator
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
                if (last_dir_vect[i] == 0) // tests if actuator is starting from a full stop
                {
                    coldstart = true;
                }
#endif
            }

            last_dir_vect[i] = dir_vect[i];
#endif
            // calculate (per linear actuator) the minimum inverted time of travel (1/min) an acceleration (1/s^2)
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

    // converts to steps per second (st/s)
    float feed = block_data->feed * MIN_SEC_MULT;
    rapid_feed *= MIN_SEC_MULT;
    rapid_feed *= (float)block_data->total_steps;
    // converts to steps per second^2 (st/s^2)
    planner_data[planner_data_write].acceleration *= (float)block_data->total_steps;

    if (feed > rapid_feed)
    {
        feed = rapid_feed;
    }

    planner_data[planner_data_write].feed_sqr = fast_flt_pow2(feed);
    planner_data[planner_data_write].rapid_feed_sqr = fast_flt_pow2(rapid_feed);

    // consider initial angle factor of 1 (90 degree angle corner or more)
    float angle_factor = 1.0f;
    uint8_t prev = 0;

    if (!planner_buffer_is_empty())
    {
        prev = planner_buffer_prev(planner_data_write); // BUFFER_PTR(planner_buffer, prev_index);
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
    
    cos_theta = CLAMP(0, cos_theta, 1.0f);

    // if more than one move stored cals juntion speeds and recalculates speed profiles
    if (cos_theta != 0 && !CHECKFLAG(block_data->motion_mode, PLANNER_MOTION_EXACT_STOP | MOTIONCONTROL_MODE_BACKLASH_COMPENSATION))
    {
        // calculates the junction angle with previous
        if (cos_theta > 0)
        {
            // uses the half angle identity conversion to convert from cos(theta) to tan(theta/2) where:
            //	tan(theta/2) = sqrt((1-cos(theta)/(1+cos(theta))
            // to simplify the calculations it multiplies by sqrt((1+cos(theta)/(1+cos(theta))
            // transforming the equation to sqrt((1^2-cos(theta)^2))/(1+cos(theta))
            // this way the output will be between 0<tan(theta/2)<inf
            // but if theta is 0<theta<90 the tan(theta/2) will be 0<tan(theta/2)<1
            // all angles greater than 1 that can be excluded
            angle_factor = 1.0f / (1.0f + cos_theta);
            cos_theta = (1.0f - fast_flt_pow2(cos_theta));
            angle_factor *= fast_flt_sqrt(cos_theta);
        }

        // sets the maximum allowed speed at junction (if angle doesn't force a full stop)
        float factor = ((!CHECKFLAG(block_data->motion_mode, PLANNER_MOTION_CONTINUOUS)) ? 0 : g_settings.g64_angle_factor);
        angle_factor = CLAMP(0, angle_factor - factor, 1);

        if (angle_factor < 1.0f)
        {
            float junc_feed_sqr = (1 - angle_factor);
            junc_feed_sqr = fast_flt_pow2(junc_feed_sqr);
            junc_feed_sqr *= planner_data[prev].feed_sqr;
            // the maximum feed is the minimal feed between the previous feed given the angle and the current feed
            planner_data[planner_data_write].entry_max_feed_sqr = MIN(planner_data[planner_data_write].feed_sqr, junc_feed_sqr);
        }

        // forces reaclculation with the new block
        planner_recalculate();
    }

    // advances the buffer
    planner_add_block();
}

/*
    Planner buffer functions
*/

static void planner_add_block(void)
{
    if (++planner_data_write == PLANNER_BUFFER_SIZE)
    {
        planner_data_write = 0;
    }

    planner_data_blocks++;
}

void planner_discard_block(void)
{
    if (!planner_data_blocks)
    {
        return;
    }

    if (++planner_data_read == PLANNER_BUFFER_SIZE)
    {
        planner_data_read = 0;
    }

    planner_data_blocks--;
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
    return (!planner_data_blocks);
}

bool planner_buffer_is_full(void)
{
    return (planner_data_blocks == PLANNER_BUFFER_SIZE);
}

static void planner_buffer_clear(void)
{
    planner_data_write = 0;
    planner_data_read = 0;
    planner_data_blocks = 0;
#ifdef FORCE_GLOBALS_TO_0
    memset(planner_data, 0, sizeof(planner_data));
#endif
}

void planner_init(void)
{
#ifdef FORCE_GLOBALS_TO_0
#if TOOL_COUNT > 0
    planner_spindle = 0;
    planner_coolant = 0;
#endif
#endif
    planner_buffer_clear();
    planner_feed_ovr_reset();
    planner_rapid_feed_ovr_reset();
#if TOOL_COUNT > 0
    planner_spindle_ovr_reset();
    planner_coolant_ovr_reset();
#endif
}

void planner_clear(void)
{
    // clears all motions stored in the buffer
    planner_buffer_clear();
#if TOOL_COUNT > 0
    planner_spindle = 0;
    planner_coolant = 0;
#endif
}

planner_block_t *planner_get_block(void)
{
    return &planner_data[planner_data_read];
}

float planner_get_block_exit_speed_sqr(void)
{
    // only one block in the buffer (exit speed is 0)
    if (planner_data_blocks < 2)
        return 0;

    // exit speed = next block entry speed
    uint8_t next = planner_buffer_next(planner_data_read);
    float exit_speed_sqr = planner_data[next].entry_feed_sqr;
    float rapid_feed_sqr = planner_data[next].rapid_feed_sqr;

    if (planner_data[next].flags_u.flags_t.feed_override)
    {
        if (planner_overrides.feed_override != 100)
        {
            exit_speed_sqr *= fast_flt_pow2((float)planner_overrides.feed_override);
            exit_speed_sqr *= 0.0001f;
        }

        // if rapid overrides are active the feed must not exceed the rapid motion feed
        if (planner_overrides.rapid_feed_override != 100)
        {
            rapid_feed_sqr *= fast_flt_pow2((float)planner_overrides.rapid_feed_override);
            rapid_feed_sqr *= 0.0001f;
        }
    }

    return MIN(exit_speed_sqr, rapid_feed_sqr);
}

float planner_get_block_top_speed(float exit_speed_sqr)
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
    // calculates the difference between the entry speed and the exit speed
    float speed_delta = exit_speed_sqr - planner_data[planner_data_read].entry_feed_sqr;
    // caclculates the speed increase/decrease for the given distance
    float junction_speed_sqr = planner_data[planner_data_read].acceleration * (float)(planner_data[planner_data_read].total_steps);
    junction_speed_sqr = fast_flt_mul2(junction_speed_sqr);
    // if there is enough space to accelerate computes the junction speed
    if (junction_speed_sqr >= speed_delta)
    {
        junction_speed_sqr += exit_speed_sqr + planner_data[planner_data_read].entry_feed_sqr;
        junction_speed_sqr = fast_flt_div2(junction_speed_sqr);
    }
    else if (exit_speed_sqr > planner_data[planner_data_read].entry_feed_sqr)
    {
        // will never reach the desired exit speed even accelerating all the way
        junction_speed_sqr += planner_data[planner_data_read].entry_feed_sqr;
    }
    else
    {
        // will overshoot the desired exit speed even deaccelerating all the way
        junction_speed_sqr = planner_data[planner_data_read].entry_feed_sqr;
    }

    float rapid_feed_sqr = planner_data[planner_data_read].rapid_feed_sqr;
    float target_speed_sqr = planner_data[planner_data_read].feed_sqr;
    if (planner_data[planner_data_read].flags_u.flags_t.feed_override)
    {
        if (planner_overrides.feed_override != 100)
        {
            target_speed_sqr *= fast_flt_pow2((float)planner_overrides.feed_override);
            target_speed_sqr *= 0.0001f;
        }

        // if rapid overrides are active the feed must not exceed the rapid motion feed
        if (planner_overrides.rapid_feed_override != 100)
        {
            rapid_feed_sqr *= fast_flt_pow2((float)planner_overrides.rapid_feed_override);
            rapid_feed_sqr *= 0.0001f;
        }
    }

    // can't ever exceed rapid move speed
    target_speed_sqr = MIN(target_speed_sqr, rapid_feed_sqr);
    return MIN(junction_speed_sqr, target_speed_sqr);
}

#if TOOL_COUNT > 0
int16_t planner_get_spindle_speed(float scale)
{
    float spindle = (!planner_data_blocks) ? planner_spindle : planner_data[planner_data_read].spindle;
    int16_t pwm = 0;

    if (spindle != 0)
    {
        bool neg = (spindle < 0);
        spindle = ABS(spindle);

        if (g_settings.laser_mode && neg) // scales laser power only if invert is active (M4)
        {
            spindle *= scale; // scale calculated in laser mode (otherwise scale is always 1)
        }

        if (planner_data[planner_data_read].flags_u.flags_t.feed_override && planner_overrides.spindle_override != 100)
        {
            spindle = 0.01f * (float)planner_overrides.spindle_override * spindle;
        }
        spindle = MIN(spindle, g_settings.spindle_max_rpm);
        spindle = MAX(spindle, g_settings.spindle_min_rpm);
        pwm = (uint8_t)truncf(255 * (spindle / g_settings.spindle_max_rpm));
        pwm = MAX(pwm, PWM_MIN_OUTPUT);

        return (!neg) ? pwm : -pwm;
    }

    return 0;
}

float planner_get_previous_spindle_speed(void)
{
    return (float)planner_spindle;
}

uint8_t planner_get_coolant(void)
{
    uint8_t coolant = (!planner_data_blocks) ? planner_coolant : planner_data[planner_data_read].flags_u.flags_t.coolant;

    coolant ^= planner_overrides.coolant_override;

    return coolant;
}

uint8_t planner_get_previous_coolant(void)
{
    return planner_coolant;
}
#endif

static void planner_recalculate(void)
{
    uint8_t last = planner_data_write;
    uint8_t first = planner_data_read;
    uint8_t block = last;

    // starts in the last added block
    // calculates the maximum entry speed of the block so that it can do a full stop in the end
    if (planner_data_blocks < 2)
    {
        planner_data[block].entry_feed_sqr = 0;
        return;
    }
    // optimizes entry speeds given the current exit speed (backward pass)
    uint8_t next = planner_buffer_next(block);

    while (!planner_data[block].flags_u.flags_t.optimal && block != first)
    {
        float speedchange = ((float)(planner_data[block].total_steps << 1)) * planner_data[block].acceleration;
        if (planner_data[block].entry_feed_sqr != planner_data[block].entry_max_feed_sqr)
        {
            speedchange = MIN(planner_data[block].entry_max_feed_sqr, speedchange);
            speedchange += (block != last) ? planner_data[next].entry_max_feed_sqr : 0;
            planner_data[block].entry_feed_sqr = MIN(planner_data[block].entry_max_feed_sqr, speedchange);
        }
        else
        {
            // found optimal
            break;
        }

        next = block;
        block = planner_buffer_prev(block);
    }

    next = planner_buffer_next(block);
    // optimizes exit speeds (forward pass)
    while (block != last)
    {
        // next block is moving at a faster speed
        if (planner_data[block].entry_feed_sqr < planner_data[next].entry_feed_sqr)
        {
            float speedchange = ((float)(planner_data[block].total_steps << 1)) * planner_data[block].acceleration;
            // check if the next block entry speed can be achieved
            speedchange += planner_data[block].entry_feed_sqr;
            if (speedchange < planner_data[next].entry_feed_sqr)
            {
                // lowers next entry speed (aka exit speed) to the maximum reachable speed from current block
                // optimization achieved for this movement
                planner_data[next].entry_feed_sqr = MIN(planner_data[next].entry_max_feed_sqr, speedchange);
                planner_data[next].flags_u.flags_t.optimal = true;
            }
        }

        // if the executing block was updated then update the interpolator limits
        if (block == first)
        {
            itp_update();
        }

        block = next;
        next = planner_buffer_next(block);
    }
}

void planner_sync_tools(motion_data_t *block_data)
{
#if TOOL_COUNT > 0
    planner_spindle = block_data->spindle;
    planner_coolant = block_data->coolant;
#endif
}

// overrides
void planner_feed_ovr_inc(uint8_t value)
{
    uint8_t ovr_val = planner_overrides.feed_override;
    ovr_val += value;
    ovr_val = MAX(ovr_val, FEED_OVR_MIN);
    ovr_val = MIN(ovr_val, FEED_OVR_MAX);

    if (ovr_val != planner_overrides.feed_override)
    {
        planner_overrides.feed_override = ovr_val;
        planner_ovr_counter = 0;
        itp_update();
    }
}

void planner_rapid_feed_ovr(uint8_t value)
{
    if (planner_overrides.rapid_feed_override != value)
    {
        planner_overrides.rapid_feed_override = value;
        planner_ovr_counter = 0;
        itp_update();
    }
}

void planner_feed_ovr_reset(void)
{
    if (planner_overrides.feed_override != 100)
    {
        planner_overrides.feed_override = 100;
        planner_ovr_counter = 0;
        itp_update();
    }
}

void planner_rapid_feed_ovr_reset(void)
{
    if (planner_overrides.rapid_feed_override != 100)
    {
        planner_overrides.rapid_feed_override = 100;
        planner_ovr_counter = 0;
        itp_update();
    }
}

#if TOOL_COUNT > 0
void planner_spindle_ovr_inc(uint8_t value)
{
    uint8_t ovr_val = planner_overrides.spindle_override;
    ovr_val += value;
    ovr_val = MAX(ovr_val, SPINDLE_OVR_MIN);
    ovr_val = MIN(ovr_val, SPINDLE_OVR_MAX);

    if (ovr_val != planner_overrides.spindle_override)
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
#if TOOL_COUNT > 0
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
