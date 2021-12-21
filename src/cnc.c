/*
	Name: cnc.c
	Description: µCNC main unit.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 17/09/2019

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "cnc.h"
#include "interface/settings.h"
#include "interface/grbl_interface.h"
#include "interface/serial.h"
#include "interface/protocol.h"
#include "core/parser.h"
#include "core/motion_control.h"
#include "core/planner.h"
#include "core/interpolator.h"
#include "core/io_control.h"
#include "modules/encoder.h"
#include "modules/pid_controller.h"

#define LOOP_STARTUP_RESET 0
#define LOOP_RUNNING_FIRST_RUN 1
#define LOOP_RUNNING 2
#define LOOP_ERROR_RESET 3

#define UNLOCK_ERROR 0
#define UNLOCK_LOCKED 1
#define UNLOCK_OK 2

    typedef struct
    {
        //uint8_t system_state;		//signals if CNC is system_state and gcode can run
        volatile uint8_t exec_state;
        uint8_t loop_state;
        volatile uint8_t rt_cmd;
        volatile uint8_t feed_ovr_cmd;
        volatile uint8_t tool_ovr_cmd;
        volatile int8_t alarm;
    } cnc_state_t;

    static cnc_state_t cnc_state;

    static void cnc_check_fault_systems(void);
    static bool cnc_check_interlocking(void);
    static void cnc_exec_rt_commands(void);
    static bool cnc_reset(void);
    static bool cnc_exec_cmd(void);

    void cnc_init(void)
    {
        //initializes cnc state
#ifdef FORCE_GLOBALS_TO_0
        memset(&cnc_state, 0, sizeof(cnc_state_t));
#endif
        cnc_state.loop_state = LOOP_STARTUP_RESET;
        //initializes all systems
        mcu_init();            //mcu
        io_disable_steppers(); //disables steppers at start
        io_disable_probe();    //forces probe isr disabling
        serial_init();         //serial
        settings_init();       //settings
        itp_init();            //interpolator
        planner_init();        //motion planner
        mc_init();             //motion control
        parser_init();         //parser
#if TOOL_COUNT > 0
        tool_init();
#endif
#if PID_CONTROLLERS > 0
        pid_init(); //pid
#endif
    }

    void cnc_run(void)
    {
        // tries to reset. If fails jumps to error
        if (cnc_reset())
        {
            cnc_state.loop_state = LOOP_RUNNING_FIRST_RUN;
            serial_select(SERIAL_UART);

            do
            {
            } while (cnc_exec_cmd());
        }

        cnc_state.loop_state = LOOP_ERROR_RESET;
        serial_flush();
        if (cnc_state.alarm < EXEC_ALARM_PROBE_FAIL_INITIAL)
        {
            io_disable_steppers();
            if (cnc_state.alarm > 0)
            {
                protocol_send_alarm(cnc_state.alarm);
            }

            cnc_check_fault_systems();
            cnc_state.alarm = 0;
            do
            {
                cnc_clear_exec_state(EXEC_ALARM);
                cnc_dotasks();
            } while (cnc_state.alarm);
        }
    }

    bool cnc_exec_cmd(void)
    {
        //process gcode commands
        if (!serial_rx_is_empty())
        {
            uint8_t error = 0;
            //protocol_echo();
            uint8_t c = serial_peek();
            switch (c)
            {
            case EOL: //not necessary but faster to catch empty lines and windows newline (CR+LF)
                serial_getc();
                break;
            default:
                error = parser_read_command();
                break;
            }
            if (!error)
            {
                protocol_send_ok();
            }
            else
            {
                protocol_send_error(error);
            }

            cnc_state.loop_state = LOOP_RUNNING;
        }

        return cnc_dotasks();
    }

    bool cnc_dotasks(void)
    {
        static bool lock_itp = false;
        //run all mcu_internal tasks
        mcu_dotasks();

        //let µCNC finnish startup/reset code
        if (cnc_state.loop_state == LOOP_STARTUP_RESET)
        {
            return true;
        }

#if ((LIMITEN_MASK ^ LIMITISR_MASK) || defined(FORCE_SOFT_POLLING))
        io_limits_isr();
#endif
#if ((CONTROLEN_MASK ^ CONTROLISR_MASK) || defined(FORCE_SOFT_POLLING))
        io_controls_isr();
#endif

        cnc_exec_rt_commands(); //executes all pending realtime commands

        //µCNC already in error loop. No point in sending the alarms
        if (cnc_state.loop_state == LOOP_ERROR_RESET)
        {
            return !cnc_get_exec_state(EXEC_KILL);
        }

        //check security interlocking for any problem
        if (!cnc_check_interlocking())
        {
            return !cnc_get_exec_state(EXEC_KILL);
        }

        if (!lock_itp)
        {
            lock_itp = true;
            itp_run();
            lock_itp = false;
        }

        return !cnc_get_exec_state(EXEC_KILL);
    }

    void cnc_home(void)
    {
        cnc_set_exec_state(EXEC_HOMING);
        uint8_t error = kinematics_home();
        if (error)
        {
            //disables homing and reenables alarm messages
            cnc_clear_exec_state(EXEC_HOMING);
            //cnc_alarm(error);
            return;
        }

        //unlocks the machine to go to offset
        cnc_unlock(true);

        float target[AXIS_COUNT];
        motion_data_t block_data = {0};
        mc_get_position(target);

        for (uint8_t i = AXIS_COUNT; i != 0;)
        {
            i--;
            target[i] += ((g_settings.homing_dir_invert_mask & (1 << i)) ? -g_settings.homing_offset : g_settings.homing_offset);
        }

        block_data.feed = g_settings.homing_fast_feed_rate;
        block_data.spindle = 0;
        block_data.dwell = 0;
        //starts offset and waits to finnish
        mc_line(target, &block_data);
        itp_sync();

        //reset position
        itp_reset_rt_position();
        planner_sync_position();
    }

    void cnc_alarm(int8_t code)
    {
        cnc_set_exec_state(EXEC_KILL);
        cnc_stop();
        cnc_state.alarm = code;
    }

    void cnc_stop(void)
    {
        itp_stop();
        //stop tools
        itp_stop_tools();
    }

    uint8_t cnc_unlock(bool force)
    {
        //tries to clear alarms or any active hold state
        cnc_clear_exec_state(EXEC_ALARM | EXEC_HOLD);
        //checks all interlocking again
        cnc_check_interlocking();

        //forces to clear HALT error to allow motion after limit switch trigger
        if (force)
        {
            CLEARFLAG(cnc_state.exec_state, EXEC_HALT);
        }

        //if any alarm state is still active checks system faults
        if (cnc_get_exec_state(EXEC_ALARM))
        {
            if (!cnc_get_exec_state(EXEC_KILL))
            {
                protocol_send_feedback(MSG_FEEDBACK_2);
                return UNLOCK_LOCKED;
            }
            else
            {
                return UNLOCK_ERROR;
            }
        }
        else
        {
            //on unlock any alarm caused by not having homing reference or hitting a limit switch is reset at user request
            //this must be done directly beacuse cnc_clear_exec_state will check the limit switch state
            //all other alarm flags remain active if any input is still active
            CLEARFLAG(cnc_state.exec_state, EXEC_HALT);
            //clears all other locking flags
            cnc_clear_exec_state(EXEC_GCODE_LOCKED | EXEC_HOLD);
            //signals stepper enable pins

            io_set_steps(g_settings.step_invert_mask);
            io_set_dirs(g_settings.dir_invert_mask);
            io_enable_steppers();

            if (cnc_state.loop_state < LOOP_RUNNING)
            {
                serial_select(SERIAL_N0);
                if (!cnc_exec_cmd())
                {
                    return UNLOCK_ERROR;
                }
                serial_select(SERIAL_N1);
                if (!cnc_exec_cmd())
                {
                    return UNLOCK_ERROR;
                }
                serial_select(SERIAL_UART);
            }
        }

        return UNLOCK_OK;
    }

    uint8_t cnc_get_exec_state(uint8_t statemask)
    {
        return CHECKFLAG(cnc_state.exec_state, statemask);
    }

    void cnc_set_exec_state(uint8_t statemask)
    {
        SETFLAG(cnc_state.exec_state, statemask);
    }

    void cnc_clear_exec_state(uint8_t statemask)
    {
        uint8_t controls = io_get_controls();

#if (ESTOP >= 0)
        if (CHECKFLAG(controls, ESTOP_MASK)) //can't clear the alarm flag if ESTOP is active
        {
            CLEARFLAG(statemask, EXEC_KILL);
        }
#endif
#if (SAFETY_DOOR >= 0)
        if (CHECKFLAG(controls, SAFETY_DOOR_MASK)) //can't clear the door flag if SAFETY_DOOR is active
        {
            CLEARFLAG(statemask, EXEC_DOOR | EXEC_HOLD);
        }
#endif
#if (FHOLD >= 0)
        if (CHECKFLAG(controls, FHOLD_MASK)) //can't clear the hold flag if FHOLD is active
        {
            CLEARFLAG(statemask, EXEC_HOLD);
        }
#endif

        uint8_t limits = 0;
#if (LIMITS_MASK != 0)
        limits = io_get_limits(); //can't clear the EXEC_HALT is any limit is triggered
#endif
        if (g_settings.hard_limits_enabled) //if hardlimits are enabled and limits are triggered
        {
            if (limits || g_settings.homing_enabled)
            {
                CLEARFLAG(statemask, EXEC_HALT);
            }
        }

        //if releasing from a HOLD state with and active delay in exec
        if (CHECKFLAG(statemask, EXEC_HOLD) && CHECKFLAG(cnc_state.exec_state, EXEC_HOLD))
        {
            SETFLAG(cnc_state.exec_state, EXEC_RESUMING);
            CLEARFLAG(cnc_state.exec_state, EXEC_HOLD);
#ifdef USE_SPINDLE
            itp_sync_spindle();
#if (DELAY_ON_RESUME_SPINDLE > 0)
#ifdef LASER_MODE
            if (!g_settings.laser_mode)
            {
#endif
                if (!planner_buffer_is_empty())
                {
                    cnc_delay_ms(DELAY_ON_RESUME_SPINDLE * 1000);
                }
#ifdef LASER_MODE
            }
#endif
#endif
#endif
#ifdef USE_COOLANT
            //updated the coolant pins
            tool_set_coolant(planner_get_coolant());
#if (DELAY_ON_RESUME_COOLANT > 0)
#ifdef LASER_MODE
            if (!g_settings.laser_mode)
            {
#endif
                if (!planner_buffer_is_empty())
                {

                    cnc_delay_ms(DELAY_ON_RESUME_COOLANT * 1000);
                }
#ifdef LASER_MODE
            }
#endif
#endif
#endif
            CLEARFLAG(cnc_state.exec_state, EXEC_RESUMING);
        }

        CLEARFLAG(cnc_state.exec_state, statemask);
    }

    void cnc_delay_ms(uint32_t miliseconds)
    {
        uint32_t t_start = mcu_millis();
        uint32_t t_end = mcu_millis();
        while (t_end - t_start < miliseconds && cnc_dotasks())
        {
            t_end = mcu_millis();
        }
    }

    bool cnc_reset(void)
    {
        cnc_state.loop_state = LOOP_STARTUP_RESET;
        //resets all realtime command flags
        cnc_state.rt_cmd = RT_CMD_CLEAR;
        cnc_state.feed_ovr_cmd = RT_CMD_CLEAR;
        cnc_state.tool_ovr_cmd = RT_CMD_CLEAR;
        cnc_state.exec_state = EXEC_ALARM | EXEC_HOLD; //Activates all alarms and hold

        //clear all systems
        serial_rx_clear();
        itp_clear();
        planner_clear();
        parser_init();
        protocol_send_string(MSG_STARTUP);

        uint8_t ok = cnc_unlock(false);

        if (ok)
        {
            io_enable_steppers();
        }

        return (ok != 0);
    }

    void cnc_call_rt_command(uint8_t command)
    {
        //executes the realtime commands
        //only reset command is executed right away
        //control commands affect the exec_state directly (Abort, hold, safety door, cycle_start)
        //the effects are then propagate in the cnc_dotasks
        //uses macro to be faster
        switch (command)
        {
        case CMD_CODE_RESET:
            SETFLAG(cnc_state.rt_cmd, RT_CMD_RESET);
            SETFLAG(cnc_state.exec_state, EXEC_KILL);
            break;
        case CMD_CODE_FEED_HOLD:
            SETFLAG(cnc_state.exec_state, EXEC_HOLD);
            break;
        case CMD_CODE_REPORT:
            SETFLAG(cnc_state.rt_cmd, RT_CMD_REPORT);
            break;
        case CMD_CODE_CYCLE_START:
            //prevents loop if cycle start is always pressed or unconnected (during cnc_dotasks)
            if (!CHECKFLAG(cnc_state.exec_state, EXEC_RESUMING))
            {
                SETFLAG(cnc_state.rt_cmd, RT_CMD_CYCLE_START); //tries to clear hold if possible
            }
            break;
        case CMD_CODE_SAFETY_DOOR:
            SETFLAG(cnc_state.exec_state, (EXEC_HOLD | EXEC_DOOR));
            break;
        case CMD_CODE_JOG_CANCEL:
            if (CHECKFLAG(cnc_state.exec_state, EXEC_JOG))
            {
                SETFLAG(cnc_state.exec_state, EXEC_HOLD);
            }
            break;
        case CMD_CODE_FEED_100:
            SETFLAG(cnc_state.feed_ovr_cmd, RT_CMD_FEED_100);
            break;
        case CMD_CODE_FEED_INC_COARSE:
            SETFLAG(cnc_state.feed_ovr_cmd, RT_CMD_FEED_INC_COARSE);
            break;
        case CMD_CODE_FEED_DEC_COARSE:
            SETFLAG(cnc_state.feed_ovr_cmd, RT_CMD_FEED_DEC_COARSE);
            break;
        case CMD_CODE_FEED_INC_FINE:
            SETFLAG(cnc_state.feed_ovr_cmd, RT_CMD_FEED_INC_FINE);
            break;
        case CMD_CODE_FEED_DEC_FINE:
            SETFLAG(cnc_state.feed_ovr_cmd, RT_CMD_FEED_DEC_FINE);
            break;
        case CMD_CODE_RAPIDFEED_100:
            SETFLAG(cnc_state.feed_ovr_cmd, RT_CMD_RAPIDFEED_100);
            break;
        case CMD_CODE_RAPIDFEED_OVR1:
            SETFLAG(cnc_state.feed_ovr_cmd, RT_CMD_RAPIDFEED_OVR1);
            break;
        case CMD_CODE_RAPIDFEED_OVR2:
            SETFLAG(cnc_state.feed_ovr_cmd, RT_CMD_RAPIDFEED_OVR2);
            break;
        case CMD_CODE_SPINDLE_100:
            SETFLAG(cnc_state.tool_ovr_cmd, RT_CMD_SPINDLE_100);
            break;
        case CMD_CODE_SPINDLE_INC_COARSE:
            SETFLAG(cnc_state.tool_ovr_cmd, RT_CMD_SPINDLE_INC_COARSE);
            break;
        case CMD_CODE_SPINDLE_DEC_COARSE:
            SETFLAG(cnc_state.tool_ovr_cmd, RT_CMD_SPINDLE_DEC_COARSE);
            break;
        case CMD_CODE_SPINDLE_INC_FINE:
            SETFLAG(cnc_state.tool_ovr_cmd, RT_CMD_SPINDLE_INC_FINE);
            break;
        case CMD_CODE_SPINDLE_DEC_FINE:
            SETFLAG(cnc_state.tool_ovr_cmd, RT_CMD_SPINDLE_DEC_FINE);
            break;
        case CMD_CODE_SPINDLE_TOGGLE:
            SETFLAG(cnc_state.tool_ovr_cmd, RT_CMD_SPINDLE_TOGGLE);
            break;
        case CMD_CODE_COOL_FLD_TOGGLE:
            SETFLAG(cnc_state.tool_ovr_cmd, RT_CMD_COOL_FLD_TOGGLE);
            break;
        case CMD_CODE_COOL_MST_TOGGLE:
            SETFLAG(cnc_state.tool_ovr_cmd, RT_CMD_COOL_MST_TOGGLE);
            break;
        }
    }

    //Executes pending realtime commands
    //Realtime commands are split in to 3 groups
    //  -operation commands
    //  -feed override commands
    //  -tools override commands
    //All active flags will be executed by MSB order (higher first)
    //If two flags have different effects on the same attribute the one with the LSB will run last and overwrite the other
    void cnc_exec_rt_commands(void)
    {
        bool update_tools = false;

        //executes feeds override rt commands
        uint8_t cmd_mask = 0x04;
        uint8_t command = cnc_state.rt_cmd;            //copies realtime flags states
        CLEARFLAG(cnc_state.rt_cmd, ~(RT_CMD_REPORT)); //clears all command flags except report request
        while (command)
        {
            switch (command & cmd_mask)
            {
            case RT_CMD_RESET:
                cnc_alarm(EXEC_ALARM_SOFTRESET);
                return;
            case RT_CMD_REPORT:
                if (!protocol_is_busy() && cnc_state.loop_state)
                {
                    protocol_send_status();
                    CLEARFLAG(cnc_state.rt_cmd, RT_CMD_REPORT); //if a report request is sent, clear the respective flag
                }
                break;
            case RT_CMD_CYCLE_START:
                cnc_clear_exec_state(EXEC_HOLD);
                break;
            }

            CLEARFLAG(command, cmd_mask);
            cmd_mask >>= 1;
        }

        //executes feeds override rt commands
        cmd_mask = 0x80;
        command = cnc_state.feed_ovr_cmd;      //copies realtime flags states
        cnc_state.feed_ovr_cmd = RT_CMD_CLEAR; //clears command flags
        while (command)
        {
            switch (command & cmd_mask)
            {
            case RT_CMD_FEED_100:
                planner_feed_ovr_reset();
                break;
            case RT_CMD_FEED_INC_COARSE:
                planner_feed_ovr_inc(FEED_OVR_COARSE);
                break;
            case RT_CMD_FEED_DEC_COARSE:
                planner_feed_ovr_inc(-FEED_OVR_COARSE);
                break;
            case RT_CMD_FEED_INC_FINE:
                planner_feed_ovr_inc(FEED_OVR_FINE);
                break;
            case RT_CMD_FEED_DEC_FINE:
                planner_feed_ovr_inc(-FEED_OVR_FINE);
                break;
            case RT_CMD_RAPIDFEED_100:
                planner_rapid_feed_ovr_reset();
                break;
            case RT_CMD_RAPIDFEED_OVR1:
                planner_rapid_feed_ovr(RAPID_FEED_OVR1);
                break;
            case RT_CMD_RAPIDFEED_OVR2:
                planner_rapid_feed_ovr(RAPID_FEED_OVR2);
                break;
            }

            CLEARFLAG(command, cmd_mask);
            cmd_mask >>= 1;
        }

        //executes tools override rt commands
        cmd_mask = 0x80;
        command = cnc_state.tool_ovr_cmd;      //copies realtime flags states
        cnc_state.tool_ovr_cmd = RT_CMD_CLEAR; //clears command flags
        while (command)
        {
            update_tools = true;
            switch (command & cmd_mask)
            {
#ifdef USE_SPINDLE
            case RT_CMD_SPINDLE_100:
                planner_spindle_ovr_reset();
                break;
            case RT_CMD_SPINDLE_INC_COARSE:
                planner_spindle_ovr_inc(SPINDLE_OVR_COARSE);
                break;
            case RT_CMD_SPINDLE_DEC_COARSE:
                planner_spindle_ovr_inc(-SPINDLE_OVR_COARSE);
                break;
            case RT_CMD_SPINDLE_INC_FINE:
                planner_spindle_ovr_inc(SPINDLE_OVR_FINE);
                break;
            case RT_CMD_SPINDLE_DEC_FINE:
                planner_spindle_ovr_inc(-SPINDLE_OVR_FINE);
                break;
            case RT_CMD_SPINDLE_TOGGLE:
                if (cnc_get_exec_state(EXEC_HOLD | EXEC_DOOR | EXEC_RUN) == EXEC_HOLD) //only available if a TRUE hold is active
                {
//toogle state
#ifdef USE_SPINDLE
                    if (tool_get_speed())
                    {
                        update_tools = false;
                        tool_set_speed(0, false);
                    }
#endif
                }
                break;
#endif
#ifdef USE_COOLANT
            case RT_CMD_COOL_FLD_TOGGLE:
#ifdef COOLANT_MIST
            case RT_CMD_COOL_MST_TOGGLE:
#endif
                if (!cnc_get_exec_state(EXEC_ALARM)) //if no alarm is active
                {
                    if (cmd_mask == RT_CMD_COOL_FLD_TOGGLE)
                    {
                        planner_coolant_ovr_toggle(COOLANT_MASK);
                    }
#ifdef COOLANT_MIST
                    if (cmd_mask == RT_CMD_COOL_MST_TOGGLE)
                    {
                        planner_coolant_ovr_toggle(MIST_MASK);
                    }
#endif
                }
                break;
#endif
            }

            CLEARFLAG(command, cmd_mask);
            cmd_mask >>= 1;
        }

        if (update_tools)
        {
            itp_update();
            if (planner_buffer_is_empty())
            {
                motion_data_t block = {0};
#ifdef USE_SPINDLE
#ifdef USE_COOLANT
                block.coolant = planner_get_previous_coolant();
#endif
                block.spindle = planner_get_previous_spindle_speed();
#endif
                mc_update_tools(&block);
            }
        }
    }

    void cnc_check_fault_systems(void)
    {
        uint8_t inputs;
#ifdef CONTROLS_MASK
        inputs = io_get_controls();
#endif
#if (ESTOP >= 0)
        if (CHECKFLAG(inputs, ESTOP_MASK)) //fault on emergency stop
        {
            protocol_send_feedback(MSG_FEEDBACK_12);
        }
#endif
#if (SAFETY_DOOR >= 0)
        if (CHECKFLAG(inputs, SAFETY_DOOR_MASK)) //fault on safety door
        {
            protocol_send_feedback(MSG_FEEDBACK_6);
        }
#endif
#if (LIMITS_MASK != 0)
        if (g_settings.hard_limits_enabled) //fault on limits
        {
            inputs = io_get_limits();
            if (CHECKFLAG(inputs, LIMITS_MASK))
            {
                protocol_send_feedback(MSG_FEEDBACK_7);
            }
        }
#endif

        if (cnc_get_exec_state(EXEC_KILL))
        {
            switch (cnc_state.alarm)
            {
            case EXEC_ALARM_SOFTRESET:
            case EXEC_ALARM_NOALARM:
                break;
            default:
                protocol_send_feedback(MSG_FEEDBACK_1);
                break;
            }
        }
    }

    bool cnc_check_interlocking(void)
    {
        //check all flags
        //if kill leave
        if (CHECKFLAG(cnc_state.exec_state, EXEC_KILL))
        {
#if (ESTOP >= 0)
            //the emergency stop is pressed.
            if (io_get_controls() & ESTOP_MASK)
            {
                cnc_alarm(EXEC_ALARM_EMERGENCY_STOP);
                return false;
            }
#endif
            if (CHECKFLAG(cnc_state.exec_state, EXEC_HOMING)) //reset or emergency stop during a homing cycle
            {
                cnc_alarm(EXEC_ALARM_HOMING_FAIL_RESET);
            }
            else if (CHECKFLAG(cnc_state.exec_state, EXEC_RUN)) //reset or emergency stop during a running cycle
            {
                cnc_alarm(EXEC_ALARM_ABORT_CYCLE);
            }
            return false;
        }

        if (CHECKFLAG(cnc_state.exec_state, EXEC_DOOR) && CHECKFLAG(cnc_state.exec_state, EXEC_HOMING)) //door opened during a homing cycle
        {
            cnc_alarm(EXEC_ALARM_HOMING_FAIL_DOOR);
            return false;
        }

        if (CHECKFLAG(cnc_state.exec_state, EXEC_HALT) && CHECKFLAG(cnc_state.exec_state, EXEC_RUN))
        {
            if (!CHECKFLAG(cnc_state.exec_state, EXEC_HOMING) && io_get_limits()) //if a motion is being performed allow trigger the limit switch alarm
            {
                cnc_alarm(EXEC_ALARM_HARD_LIMIT);
            }
            else
            {
                CLEARFLAG(cnc_state.exec_state, EXEC_RUN);
            }

            return false;
        }

        //opened door or hold with the machine still moving
        if (CHECKFLAG(cnc_state.exec_state, EXEC_DOOR | EXEC_HOLD) && !CHECKFLAG(cnc_state.exec_state, EXEC_RUN))
        {
            if (CHECKFLAG(cnc_state.exec_state, EXEC_DOOR))
            {
                cnc_stop(); //stop all tools not only motion
            }
            else
            {
                itp_stop(); //stop motion
            }

            if (CHECKFLAG(cnc_state.exec_state, EXEC_HOMING | EXEC_JOG)) //flushes the buffers if motions was homing or jog
            {
                itp_clear();
                planner_clear();
                CLEARFLAG(cnc_state.exec_state, EXEC_HOMING | EXEC_JOG | EXEC_HOLD);
            }

            return false;
        }

        return true;
    }

#ifdef __cplusplus
}
#endif
