/*
	Name: motion_control.c
	Description: Contains the building blocks for performing motions/actions in µCNC
	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 19/11/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include <math.h>
#include <string.h>
#include "config.h"
#include "mcu.h"
#include "grbl_interface.h"
#include "settings.h"
#include "utils.h"
#include "io_control.h"
#include "parser.h"
#include "kinematics.h"
#include "planner.h"
#include "interpolator.h"
#include "cnc.h"
#include "motion_control.h"

static bool mc_checkmode;

void mc_init(void)
{
#ifdef FORCE_GLOBALS_TO_0
    mc_checkmode = false;
#endif
}

bool mc_get_checkmode(void)
{
    return mc_checkmode;
}

bool mc_toogle_checkmode(void)
{
    mc_checkmode = !mc_checkmode;
    return mc_checkmode;
}

uint8_t mc_line(float *target, planner_block_data_t block_data)
{
    kinematics_apply_transform(target);

    //check travel limits
    if (!io_check_boundaries(target))
    {
        if (cnc_get_exec_state(EXEC_JOG))
        {
            return STATUS_TRAVEL_EXCEEDED;
        }
        cnc_alarm(EXEC_ALARM_SOFT_LIMIT);
        return STATUS_OK;
    }

    if (mc_checkmode) // check mode (gcode simulation) doesn't send code to planner
    {
	planner_set_position(target);
        return STATUS_OK;
    }

    while (planner_buffer_is_full())
    {
        if(!cnc_doevents())
        {
            return STATUS_CRITICAL_FAIL;
        }
    }

    if(!CHECKFLAG(block_data.motion_mode,PLANNER_MOTION_MODE_NOMOTION))
    {
        float mc_position[AXIS_COUNT];

        //copy planner last position
        planner_get_position(mc_position);
        for (uint8_t i = AXIS_COUNT; i != 0;)
        {
            i--;
            block_data.dir_vect[i] = target[i] - mc_position[i];
            if (block_data.dir_vect[i] != 0)
            {
                block_data.distance += (block_data.dir_vect[i] * block_data.dir_vect[i]);
            }
        }

        block_data.distance = sqrtf(block_data.distance);
        if(CHECKFLAG(block_data.motion_mode,PLANNER_MOTION_MODE_INVERSEFEED))
        {
            //calculates feed rate in reverse feed rate mode
            block_data.feed = block_data.distance / block_data.feed;
        }
    }

    planner_add_line(target, block_data);
    return STATUS_OK;
}

//applies an algorithm similar to grbl with slight changes
uint8_t mc_arc(float *target, float center_offset_a, float center_offset_b, float radius, uint8_t axis_0, uint8_t axis_1, bool isclockwise, planner_block_data_t block_data)
{
    float mc_position[AXIS_COUNT];

    //copy planner last position
    planner_get_position(mc_position);
    //reverses any transformation aplied before
    kinematics_apply_reverse_transform(mc_position);

    float ptcenter_a = mc_position[axis_0] + center_offset_a;
    float ptcenter_b = mc_position[axis_1] + center_offset_b;

    float pt0_a = -center_offset_a; // Radius vector from center to current location
    float pt0_b = -center_offset_b;
    float pt1_a = target[axis_0] - ptcenter_a; // Radius vector from center to current location
    float pt1_b = target[axis_1] - ptcenter_b;

    //dot product between vect_a and vect_b
    float dotprod = pt0_a * pt1_a + pt0_b * pt1_b;
    //determinant
    float det = pt0_a * pt1_b - pt0_b * pt1_a;
    float arc_angle = atan2(det, dotprod);

    if (isclockwise)
    {
        if (arc_angle >= 0)
        {
            arc_angle -= 2 * M_PI;
        }
    }
    else
    {
        if (arc_angle <= 0)
        {
            arc_angle += 2 * M_PI;
        }
    }

    //uses as temporary vars
    float radiusangle = radius * arc_angle;
    radiusangle = fast_flt_div2(radiusangle);
    float diameter = fast_flt_mul2(radius);
    uint16_t segment_count = floor(fabs(radiusangle) / sqrt(g_settings.arc_tolerance * (diameter - g_settings.arc_tolerance)));
    float arc_per_sgm = (segment_count != 0) ? arc_angle/segment_count : arc_angle;
    float dist_sgm = 0;

    //for all other axis finds the linear motion distance
    float increment[AXIS_COUNT];

    for (uint8_t i = AXIS_COUNT; i != 0;)
    {
        i--;
        increment[i] = (target[i] - mc_position[i])/segment_count;
    }

    increment[axis_0] = 0;
    increment[axis_1] = 0;

    if(CHECKFLAG(block_data.motion_mode,PLANNER_MOTION_MODE_INVERSEFEED))
    {
        //split the required time to complete the motion with the number of segments
        block_data.feed /= segment_count;
    }

    //calculates an aproximation to sine and cosine of the angle segment
    //improves the error for the cosine by calculating an extra term of the taylor series at the expence of an extra multiplication and addition
    //applies arc correction has grbl does
    float arc_per_sgm_sqr = arc_per_sgm * arc_per_sgm;
    float cos_per_sgm = 1 - 0.1666666667f * arc_per_sgm_sqr;
    float sin_per_sgm = arc_per_sgm * cos_per_sgm;
    cos_per_sgm = arc_per_sgm_sqr * (cos_per_sgm + 1);
    cos_per_sgm = 1 - fast_flt_div4(cos_per_sgm);

    uint8_t count = 0;

    for (uint16_t i = 1; i < segment_count; i++)
    {
        if (count < N_ARC_CORRECTION)
        {
            // Apply incremental vector rotation matrix.
            float new_pt = pt0_a * sin_per_sgm + pt0_b * cos_per_sgm;
            pt0_a = pt0_a * cos_per_sgm - pt0_b * sin_per_sgm;
            pt0_b = new_pt;
            count++;
        }
        else
        {
            // Arc correction to radius vector. Computed only every N_ARC_CORRECTION increments.
            // Compute exact location by applying transformation matrix from initial radius vector(=-offset).
            float angle = i * arc_per_sgm;
            float precise_cos = cos(angle);
            //calculates sine using sine and cosine relation equation
            //	sin(x)^2 + cos(x)^2 = 1
            //
            //this is executes in about 50% the time of a sin function
            //https://www.nongnu.org/avr-libc/user-manual/benchmarks.html
            float precise_sin = sqrt(1 - precise_cos * precise_cos);
            if(angle >= 0)
            {
                precise_sin = (ABS(angle)<=M_PI) ? precise_sin : -precise_sin;
            }
            else
            {
                precise_sin = (ABS(angle)<=M_PI) ? -precise_sin : precise_sin;
            }

            pt0_a = -center_offset_a * precise_cos + center_offset_b * precise_sin;
            pt0_b = -center_offset_a * precise_sin - center_offset_b * precise_cos;
            count = 0;
        }

        // Update arc_target location
        mc_position[axis_0] = ptcenter_a + pt0_a;
        mc_position[axis_1] = ptcenter_b + pt0_b;
        for (uint8_t i = AXIS_COUNT; i != 0;)
        {
            i--;
            if (i != axis_0 && i != axis_1)
            {
                mc_position[i] += increment[i];
            }
        }

        uint8_t error = mc_line(mc_position, block_data);
        if (error)
        {
            return error;
        }
    }
    // Ensure last segment arrives at target location.
    return mc_line(target, block_data);
}

uint8_t mc_dwell(planner_block_data_t block_data)
{
    if (mc_checkmode) // check mode (gcode simulation) doesn't send code to planner
    {
        return STATUS_OK;
    }

    while (planner_buffer_is_full())
    {
        if(!cnc_doevents())
        {
            return STATUS_CRITICAL_FAIL;
        }
    }

    //send dwell (planner linear motion with distance == 0)
    block_data.motion_mode = PLANNER_MOTION_MODE_NOMOTION;
    planner_add_line(NULL, block_data);
    return STATUS_OK;
}

uint8_t mc_home_axis(uint8_t axis, uint8_t axis_limit)
{
    float target[AXIS_COUNT];
    uint8_t axis_mask = (1 << axis);
    planner_block_data_t block_data;
    uint8_t limits_flags;

    #ifdef ENABLE_DUAL_DRIVE_AXIS
    #ifdef DUAL_DRIVE_AXIS0
    axis_limit |= (axis != AXIS_DUAL0) ? 0 : (64|128); //if dual limit pins
    #endif
    #ifdef DUAL_DRIVE_AXIS1
    axis_limit |= (axis != AXIS_DUAL1) ? 0 : (64|128); //if dual limit pins
    #endif
    #endif

    planner_get_position(target);

    cnc_unlock();

    //if HOLD or ALARM are still active or any limit switch is not cleared fails to home
    if (cnc_get_exec_state(EXEC_HOLD | EXEC_ALARM) || CHECKFLAG(io_get_limits(), LIMITS_MASK))
    {
        return EXEC_ALARM_HOMING_FAIL_LIMIT_ACTIVE;
    }
    
    io_set_homing_limits_filter(axis_limit);

    float max_home_dist;
    max_home_dist = -g_settings.max_distance[axis] * 1.5f;
    
    //checks homing dir
    if (g_settings.homing_dir_invert_mask & axis_mask)
    {
        max_home_dist = -max_home_dist;
    }
    planner_resync_position();
    planner_get_position(target);
    target[axis] += max_home_dist;
    //initializes planner block data
    block_data.distance = ABS(max_home_dist);
    memset(&block_data.dir_vect,0, sizeof(block_data.dir_vect));
    block_data.dir_vect[axis] = max_home_dist;
    block_data.feed = g_settings.homing_fast_feed_rate * MIN_SEC_MULT;
    block_data.spindle = 0;
    block_data.dwell = 0;
    block_data.motion_mode = PLANNER_MOTION_MODE_FEED;
    cnc_unlock();
    planner_add_line((float *)&target, block_data);
    //flags homing clear by the unlock
    cnc_set_exec_state(EXEC_HOMING);
    do
    {
        if(!cnc_doevents())
        {
            return STATUS_CRITICAL_FAIL;
        }
    }
    while (cnc_get_exec_state(EXEC_RUN));

    //flushes buffers
    itp_stop();
    itp_clear();
    planner_clear();

    if(cnc_get_exec_state(EXEC_ABORT))
    {
        return EXEC_ALARM_HOMING_FAIL_RESET;
    }

    limits_flags = io_get_limits();

    //the wrong switch was activated bails
    if (!CHECKFLAG(limits_flags, axis_limit))
    {
        return EXEC_ALARM_HOMING_FAIL_APPROACH;
    }

    //back off from switch at lower speed
    max_home_dist = g_settings.homing_offset * 5.0f;

    //sync's the planner
    planner_resync_position();
    planner_get_position(target);
    if (g_settings.homing_dir_invert_mask & axis_mask)
    {
        max_home_dist = -max_home_dist;
    }

    target[axis] += max_home_dist;
    block_data.feed = g_settings.homing_slow_feed_rate * MIN_SEC_MULT;
    block_data.distance = ABS(max_home_dist);
    block_data.dir_vect[axis] = max_home_dist;
    //unlocks the machine for next motion (this will clear the EXEC_LIMITS flag
    //temporary inverts the limit mask to trigger ISR on switch release
    g_settings.limits_invert_mask ^= axis_limit;
    //io_set_homing_limits_filter(LIMITS_DUAL_MASK);//if axis pin goes off triggers
    cnc_unlock();
    planner_add_line((float *)&target, block_data);
    //flags homing clear by the unlock
    cnc_set_exec_state(EXEC_HOMING);
    do
    {
        if(!cnc_doevents())
        {
            return STATUS_CRITICAL_FAIL;
        }
    }
    while (cnc_get_exec_state(EXEC_RUN));

    //resets limit mask
    g_settings.limits_invert_mask ^= axis_limit;
    //stops, flushes buffers and clears the hold if active
    cnc_stop();
    //clearing the interpolator unlockes any locked stepper
    itp_clear();
    planner_clear();

    if(cnc_get_exec_state(EXEC_ABORT))
    {
        return EXEC_ALARM_HOMING_FAIL_RESET;
    }

    limits_flags = io_get_limits();

    if (CHECKFLAG(limits_flags, axis_limit))
    {
        return EXEC_ALARM_HOMING_FAIL_APPROACH;
    }

    return STATUS_OK;
}

uint8_t mc_spindle_coolant(planner_block_data_t block_data)
{
    if (mc_checkmode) // check mode (gcode simulation) doesn't send code to planner
    {
        return STATUS_OK;
    }

    while (planner_buffer_is_full())
    {
        if(!cnc_doevents())
        {
            return STATUS_CRITICAL_FAIL;
        }
    }

    block_data.motion_mode = PLANNER_MOTION_MODE_NOMOTION;
    planner_add_line(NULL, block_data);
    return STATUS_OK;
}

uint8_t mc_probe(float *target, bool invert_probe, planner_block_data_t block_data)
{
    #ifdef PROBE
    uint8_t prev_state = cnc_get_exec_state(EXEC_HOLD);
    io_enable_probe();

    mc_line(target, block_data);

    do
    {
        if(!cnc_doevents())
        {
            return STATUS_CRITICAL_FAIL;
        }
        #if(defined(FORCE_SOFT_POLLING) || (PROBEEN_MASK!=PROBEISR_MASK))
        if(io_get_probe())
        {
            io_probe_isr();
            break;
        }
        #endif
    } while (cnc_get_exec_state(EXEC_RUN));

    io_disable_probe();
    itp_stop();
    itp_clear();
    planner_clear();
    cnc_clear_exec_state(~prev_state & EXEC_HOLD); //restores HOLD previous state
    bool probe_notok = (!invert_probe) ? io_get_probe() : !io_get_probe();
    if(probe_notok)
    {
        return EXEC_ALARM_PROBE_FAIL_CONTACT;
    }

    #endif

    return STATUS_OK;
}


