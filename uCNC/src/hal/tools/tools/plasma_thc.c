/*
    Name: plasma_thc.c
    Description: Defines a plasma tool with THC for µCNC.

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 29/06/2023

    µCNC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version. Please see <http://www.gnu.org/licenses/>

    µCNC is distributed WITHOUT ANY WARRANTY;
    Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the	GNU General Public License for more details.
*/

#include <math.h>
#include <float.h>
#include <stdint.h>
#include <stdio.h>

#include "../../../cnc.h"

#ifndef PLASMA_UP_INPUT
#define PLASMA_UP_INPUT DIN15
#endif

#ifndef PLASMA_DOWN_INPUT
#define PLASMA_DOWN_INPUT DIN14
#endif

#ifndef PLASMA_ARC_OK_INPUT
#define PLASMA_ARC_OK_INPUT DIN13
#endif

#ifndef PLASMA_ON_OUTPUT
#define PLASMA_ON_OUTPUT DOUT0
#endif

#ifndef PLASMA_STEPPERS_MASK
#define PLASMA_STEPPERS_MASK (1 << 2)
#endif

// overridable
// user can implement the plasma thc up condition based on analog voltage reading using analog the controller analog inputs and PID controllers
uint8_t __attribute__((weak)) plasma_thc_up(void)
{
#if ASSERT_PIN(PLASMA_UP_INPUT)
    return mcu_get_input(PLASMA_UP_INPUT);
#else
    return 0;
#endif
}

// overridable
// user can implement the plasma thc down condition based on analog voltage reading using analog the controller analog inputs and PID controllers
uint8_t __attribute__((weak)) plasma_thc_down(void)
{
#if ASSERT_PIN(PLASMA_DOWN_INPUT)
    return mcu_get_input(PLASMA_DOWN_INPUT);
#else
    return 0;
#endif
}

// overridable
// user can implement the plasma thc arc ok condition based on analog voltage reading using analog the controller analog inputs and PID controllers
uint8_t __attribute__((weak)) plasma_thc_arc_ok(void)
{
#if ASSERT_PIN(PLASMA_ARC_OK_INPUT)
    return mcu_get_input(PLASMA_ARC_OK_INPUT);
#else
    return 0;
#endif
}

// plasma thc controller variables
static bool plasma_thc_enabled;
static volatile int8_t plasma_step_error;
typedef struct plasma_start_params_
{
    float probe_depth;    // I
    float probe_feed;     // J
    float retract_height; // R
    float cut_depth;      // K
    float cut_feed;       // F
    uint16_t dwell;       // P*1000
    uint8_t retries;      // L

} plasma_start_params_t;
static plasma_start_params_t plasma_start_params;

bool plasma_thc_probe_and_start(plasma_start_params_t start_params)
{
    static bool plasma_starting = false;
    plasma_thc_enabled = false;
    if (plasma_starting)
    {
        // prevent reentrancy
        return false;
    }

    plasma_starting = true;
    while (start_params.retries--)
    {
        // cutoff torch
        motion_data_t block = {0};
        block.motion_flags.bit.spindle_running = 0;
        mc_update_tools(&block);

        // get current position
        float pos[AXIS_COUNT];
        mc_get_position(pos);

        // modify target to probe depth
        pos[AXIS_Z] += start_params.probe_depth;
        // probe feed speed
        block.feed = start_params.probe_feed;
        // similar to G38.2
        if (mc_probe(pos, 0, &block) == STATUS_PROBE_SUCCESS)
        {
            // modify target to probe depth
            mc_get_position(pos);
            pos[AXIS_Z] -= start_params.probe_depth * 0.5;
            block.feed = start_params.probe_feed * 0.5f; // half speed
            // similar to G38.4
            if (mc_probe(pos, 1, &block) == STATUS_PROBE_SUCCESS)
            {
                // modify target to torch start height
                mc_get_position(pos);
                pos[AXIS_Z] += start_params.retract_height;
                // rapid feed
                block.feed = FLT_MAX;
                mc_line(pos, &block);
                // turn torch on and wait before confirm the arc on signal
                block.motion_flags.bit.spindle_running = 1;
                block.dwell = start_params.dwell;
                // updated tools and wait
                mc_dwell(&block);

                // confirm if arc is ok
                if (plasma_thc_arc_ok())
                {
                    mc_get_position(pos);
                    pos[AXIS_Z] -= start_params.cut_depth;
                    // rapid feed
                    block.feed = start_params.cut_feed;
                    mc_line(pos, &block);
                    cnc_set_exec_state(EXEC_HOLD);
                    plasma_thc_enabled = true;
                    // continues program
                    plasma_starting = false;
                    return true;
                }
            }
        }
    }

    plasma_starting = false;
    return false;
}

#ifdef ENABLE_RT_SYNC_MOTIONS
void itp_rt_stepbits(uint8_t *stepbits, itp_segment_t *rt_sgm)
{
    uint8_t step_error = plasma_step_error;
    if (!step_error)
    {
        return;
    }

    if (step_error > 0)
    {
        *stepbits |= PLASMA_STEPPERS_MASK;
        io_set_dirs(rt_sgm->block->dirbits & ~PLASMA_STEPPERS_MASK);
        step_error--;
    }

    if (step_error < 0)
    {
        *stepbits |= PLASMA_STEPPERS_MASK;
        io_set_dirs(rt_sgm->block->dirbits | PLASMA_STEPPERS_MASK);
        step_error--;
    }

    plasma_step_error = step_error;
}
#endif

#ifdef ENABLE_PARSER_MODULES
#define M103 EXTENDED_MCODE(103)

bool m103_parse(void *args);
bool m103_exec(void *args);

CREATE_EVENT_LISTENER(gcode_parse, m103_parse);
CREATE_EVENT_LISTENER(gcode_exec, m103_exec);

// this just parses and acceps the code
bool m103_parse(void *args)
{
    gcode_parse_args_t *ptr = (gcode_parse_args_t *)args;
    if (ptr->word == 'M' && ptr->code == 103)
    {
        if (ptr->cmd->group_extended != 0)
        {
            // there is a collision of custom gcode commands (only one per line can be processed)
            *(ptr->error) = STATUS_GCODE_MODAL_GROUP_VIOLATION;
            return EVENT_HANDLED;
        }
        // tells the gcode validation and execution functions this is custom code M42 (ID must be unique)
        ptr->cmd->group_extended = M103;
        *(ptr->error) = STATUS_OK;
        return EVENT_HANDLED;
    }

    // if this is not catched by this parser, just send back the error so other extenders can process it
    return EVENT_CONTINUE;
}

// this actually performs 2 steps in 1 (validation and execution)
bool m103_exec(void *args)
{
    gcode_exec_args_t *ptr = (gcode_exec_args_t *)args;
    if (ptr->cmd->group_extended == M103)
    {
        if (CHECKFLAG(ptr->cmd->words, (GCODE_WORD_I | GCODE_WORD_J | GCODE_WORD_K | GCODE_WORD_R | GCODE_WORD_P)) != (GCODE_WORD_I | GCODE_WORD_J | GCODE_WORD_K | GCODE_WORD_R | GCODE_WORD_P))
        {
            *(ptr->error) = STATUS_GCODE_VALUE_WORD_MISSING;
            return EVENT_HANDLED;
        }

        plasma_start_params.dwell = (uint16_t)(ptr->words->p * 1000);
        plasma_start_params.probe_depth = ptr->words->ijk[0];
        plasma_start_params.probe_feed = ptr->words->ijk[1];
        plasma_start_params.cut_depth = ptr->words->ijk[2];
        plasma_start_params.retract_height = ptr->words->r;
        if (CHECKFLAG(ptr->cmd->words, (GCODE_WORD_F)))
        {
            plasma_start_params.cut_feed = ptr->words->f;
        }
        plasma_start_params.retries = (CHECKFLAG(ptr->cmd->words, (GCODE_WORD_L))) ? ptr->words->l : 1;

        *(ptr->error) = STATUS_OK;
        return EVENT_HANDLED;
    }

    return EVENT_CONTINUE;
}

#endif

#ifdef ENABLE_MAIN_LOOP_MODULES
bool plasma_thc_update_loop(void *ptr)
{
    if (plasma_thc_enabled)
    {
        // arc lost
        // on arc lost the plasma must enter hold
        if (!(plasma_thc_arc_ok()))
        {
            // set hold and wait for motion to stop
            cnc_set_exec_state(EXEC_HOLD);
            itp_sync();
            // store planner and motion controll data away
            planner_store();
            mc_store();
            // reset planner and sync systems
            planner_clear();
            mc_sync_position();

            if (plasma_thc_probe_and_start(plasma_start_params))
            {
                // restore the motion controller, planner and parser
                mc_restore();
                planner_restore();
                parser_sync_position();

                // clear the current hold state
                cnc_clear_exec_state(EXEC_HOLD);
            }
            else
            {
                cnc_alarm(EXEC_ALARM_PLASMA_THC_ARC_START_FAILURE);
            }
        }

        if (plasma_thc_up())
        {
            // option 1 - modify the planner block
            // this assumes Z is not moving in this motion
            // planner_block_t *p = planner_get_block();
            // p->steps[2] = p->steps[p->main_stepper];
            // p->dirbits &= 0xFB;

            // option 2 - mask the step bits directly
            plasma_step_error += 1;
        }
        else if (plasma_thc_down())
        {
            // option 1 - modify the planner block
            // this assumes Z is not moving in this motion
            // planner_block_t *p = planner_get_block();
            // p->steps[2] = p->steps[p->main_stepper];
            // p->dirbits |= 4;

            // option 2 - mask the step bits directly
            plasma_step_error -= 1;
        }
        else
        {
            // option 1 - modify the planner block
            // this assumes Z is not moving in this motion
            // planner_block_t *p = planner_get_block();
            // p->steps[2] = 0;

            // option 2 - mask the step bits directly
            plasma_step_error = 0;
        }
    }
    return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(cnc_dotasks, plasma_thc_update_loop);
#endif

DECL_MODULE(plasma_thc)
{
#ifdef ENABLE_MAIN_LOOP_MODULES
    ADD_EVENT_LISTENER(cnc_dotasks, plasma_thc_update_loop);
#else
#error "Main loop extensions are not enabled. TMC configurations will not work."
#endif
#ifdef ENABLE_PARSER_MODULES
    ADD_EVENT_LISTENER(gcode_parse, m103_parse);
    ADD_EVENT_LISTENER(gcode_exec, m103_exec);
#else
#error "Parser extensions are not enabled. M103 code extension will not work."
#endif
}

static void startup_code(void)
{
// force plasma off
#if ASSERT_PIN(PLASMA_ON_OUTPUT)
    mcu_clear_output(PLASMA_ON_OUTPUT);
#endif
}

static void shutdown_code(void)
{
// force plasma off
#if ASSERT_PIN(PLASMA_ON_OUTPUT)
    mcu_clear_output(PLASMA_ON_OUTPUT);
#endif
}

static void set_speed(int16_t value)
{
    // turn plasma on
    if (value)
    {
        if (!plasma_thc_arc_ok())
        {
            if (plasma_thc_probe_and_start(plasma_start_params))
            {
                cnc_clear_exec_state(EXEC_HOLD);
#if ASSERT_PIN(PLASMA_ON_OUTPUT)
                mcu_set_output(PLASMA_ON_OUTPUT);
#endif
            }
        }
    }
    else
    {
#if ASSERT_PIN(PLASMA_ON_OUTPUT)
        mcu_clear_output(PLASMA_ON_OUTPUT);
#endif
    }
}

static int16_t range_speed(int16_t value)
{
    // binary output
    value = value ? 1 : 0;
    return value;
}

static void set_coolant(uint8_t value)
{
// easy macro
#ifdef ENABLE_COOLANT
    SET_COOLANT(LASER_PWM_AIR_ASSIST, UNDEF_PIN, value);
#endif
}

static uint16_t get_speed(void)
{
#if ASSERT_PIN(PLASMA_ON_OUTPUT)
    return mcu_get_output(PLASMA_ON_OUTPUT);
#else
    return 0;
#endif
}

const tool_t plasma_thc = {
    .startup_code = &startup_code,
    .shutdown_code = &shutdown_code,
#if PID_CONTROLLERS > 0
    .pid_update = NULL,
    .pid_error = NULL,
#endif
    .range_speed = &range_speed,
    .get_speed = &get_speed,
    .set_speed = &set_speed,
    .set_coolant = &set_coolant};
