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

    typedef struct
    {
        //uint8_t system_state;		//signals if CNC is system_state and gcode can run
        volatile uint8_t exec_state;
        uint8_t active_alarm;
        volatile uint8_t rt_cmd;
        volatile uint8_t feed_ovr_cmd;
        volatile uint8_t tool_ovr_cmd;
    } cnc_state_t;

    static cnc_state_t cnc_state;

    static void cnc_check_fault_systems();
    static bool cnc_check_interlocking();
    static void cnc_exec_rt_commands();
    static void cnc_reset();

    void cnc_init(void)
    {
        //initializes cnc state
#ifdef FORCE_GLOBALS_TO_0
        memset(&cnc_state, 0, sizeof(cnc_state_t));
#endif
        cnc_state.active_alarm = EXEC_ALARM_STARTUP;
        //initializes all systems
        mcu_init();              //mcu
        mcu_disable_probe_isr(); //forces probe isr disabling
        serial_init();           //serial
        settings_init();         //settings
        itp_init();              //interpolator
        planner_init();          //motion planner
        mc_init();               //motion control
        parser_init();           //parser
#if PID_CONTROLLERS > 0
        pid_init(); //pid
#endif
        io_enable_steps(); //enables stepper motors
    }

    void cnc_run(void)
    {
        cnc_reset();

        do
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
            }
        } while (cnc_dotasks());

        cnc_clear_exec_state(EXEC_ABORT); //clears the abort flag
        serial_flush();
        if (cnc_get_exec_state(EXEC_ALARM_ABORT)) //checks if any alarm is active (except NOHOME - ignore it)
        {
            cnc_check_fault_systems();
            protocol_send_feedback(MSG_FEEDBACK_1);
            do
            {
                mcu_dotasks();
            } while (!CHECKFLAG(cnc_state.rt_cmd, RT_CMD_ABORT));
        }
    }

    void cnc_call_rt_command(uint8_t command)
    {
        //executes the realtime commands
        //control commands affect the exec_state directly (Abort, hold, safety door, cycle_start)
        switch (command)
        {
        case CMD_CODE_RESET:
            cnc_stop();
            serial_rx_clear();           //dumps all commands
            cnc_alarm(EXEC_ALARM_RESET); //abort state is activated through cnc_alarm
            SETFLAG(cnc_state.rt_cmd, RT_CMD_ABORT);
            break;
        case CMD_CODE_FEED_HOLD:
            cnc_set_exec_state(EXEC_HOLD);
            break;
        case CMD_CODE_REPORT:
            SETFLAG(cnc_state.rt_cmd, RT_CMD_REPORT);
            break;
        case CMD_CODE_CYCLE_START:
            cnc_clear_exec_state(EXEC_HOLD); //tries to clear hold if possible
            break;
        case CMD_CODE_SAFETY_DOOR:
            cnc_clear_exec_state(EXEC_HOLD | EXEC_DOOR);
            break;
        case CMD_CODE_JOG_CANCEL:
            if (cnc_get_exec_state(EXEC_JOG))
            {
                cnc_set_exec_state(EXEC_HOLD);
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

    bool cnc_dotasks(void)
    {
        //run all mcu_internal tasks
        mcu_dotasks();

        //let µCNC finnish startup/reset code
        if (cnc_state.active_alarm == EXEC_ALARM_STARTUP)
        {
            return;
        }

#if ((LIMITEN_MASK ^ LIMITISR_MASK) || defined(FORCE_SOFT_POLLING))
        io_limits_isr();
#endif
#if ((CONTROLEN_MASK ^ CONTROLISR_MASK) || defined(FORCE_SOFT_POLLING))
        io_controls_isr();
#endif

        cnc_exec_rt_commands(); //executes all pending realtime commands

        //check security interlocking for any problem
        if (!cnc_check_interlocking())
        {
            return !cnc_get_exec_state(EXEC_ABORT);
        }

        itp_run();

        return !cnc_get_exec_state(EXEC_ABORT);
    }

    void cnc_home(void)
    {
        cnc_set_exec_state(EXEC_HOMING);
        uint8_t error = kinematics_home();
        if (error)
        {
            //disables homing and reenables alarm messages
            cnc_clear_exec_state(EXEC_HOMING);
            cnc_alarm(error);
            return;
        }

        //unlocks the machine to go to offset
        cnc_unlock();

        float target[AXIS_COUNT];
        motion_data_t block_data;
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
        do
        {
            cnc_dotasks();
        } while (cnc_get_exec_state(EXEC_RUN));

        //reset position
        itp_reset_rt_position();
        planner_resync_position();
        //invokes startup block execution
        SETFLAG(cnc_state.rt_cmd, RT_CMD_STARTUP_BLOCK0);
    }

    void cnc_alarm(uint8_t code)
    {
        cnc_set_exec_state(EXEC_ABORT);
        cnc_state.active_alarm = code;
    }

    void cnc_stop(void)
    {
        //halt is active and was running flags it lost home position
        if (cnc_get_exec_state(EXEC_RUN) && g_settings.homing_enabled)
        {
            cnc_set_exec_state(EXEC_NOHOME);
        }
        itp_stop();
        //stop tools
#ifdef USE_SPINDLE
        io_set_spindle(0, false);
#endif
#ifdef USE_COOLANT
        io_set_coolant(0);
#endif
    }

    void cnc_unlock(void)
    {
        //on unlock any alarm caused by not having homing reference or hitting a limit switch is reset at user request
        //all other alarm flags remain active if any input is still active
        CLEARFLAG(cnc_state.exec_state, EXEC_NOHOME | EXEC_LIMITS);
        //clears all other locking flags
        cnc_clear_exec_state(EXEC_LOCKED);
        //signals stepper enable pins

        io_set_steps(g_settings.step_invert_mask);
        io_set_dirs(g_settings.dir_invert_mask);
        io_enable_steps();
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
#ifdef ESTOP
        if (CHECKFLAG(controls, ESTOP_MASK)) //can't clear the alarm flag if ESTOP is active
        {
            CLEARFLAG(statemask, EXEC_ABORT);
        }
#endif
#ifdef SAFETY_DOOR
        if (CHECKFLAG(controls, SAFETY_DOOR_MASK)) //can't clear the door flag if SAFETY_DOOR is active
        {
            CLEARFLAG(statemask, EXEC_DOOR);
        }
#endif
#ifdef FHOLD
        if (CHECKFLAG(controls, FHOLD_MASK)) //can't clear the hold flag if FHOLD is active
        {
            CLEARFLAG(statemask, EXEC_HOLD);
        }
#endif
#if (LIMITS_MASK != 0)
        if (g_settings.hard_limits_enabled && io_get_limits()) //can't clear the EXEC_LIMITS is any limit is triggered
        {
            CLEARFLAG(statemask, EXEC_LIMITS);
        }
#endif
        if (g_settings.homing_enabled) //if the machine doesn't know the homing position and homing is enabled
        {
            CLEARFLAG(statemask, EXEC_NOHOME);
        }

        CLEARFLAG(cnc_state.exec_state, statemask);
    }

    void cnc_reset(void)
    {
        cnc_state.active_alarm = EXEC_ALARM_STARTUP;
        //resets all realtime command flags
        cnc_state.rt_cmd = RT_CMD_CLEAR;
        cnc_state.feed_ovr_cmd = RT_CMD_CLEAR;
        cnc_state.tool_ovr_cmd = RT_CMD_CLEAR;
        cnc_state.exec_state = EXEC_ALARM | EXEC_HOLD; //Activates all alarms and hold

        //clear all systems
        itp_clear();
        planner_clear();
        protocol_send_string(MSG_STARTUP);

        cnc_state.active_alarm = EXEC_ALARM_RESET;
        //tries to clear alarms or any active hold state
        cnc_clear_exec_state(EXEC_ALARM | EXEC_HOLD);

        //if any alarm state is still active checks system faults
        if (cnc_get_exec_state(EXEC_ALARM))
        {
            //cnc_check_fault_systems();
            if (!cnc_get_exec_state(EXEC_ABORT))
            {
                protocol_send_feedback(MSG_FEEDBACK_2);
            }
        }
        else
        {
            cnc_unlock();
            SETFLAG(cnc_state.rt_cmd, RT_CMD_STARTUP_BLOCK0);
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
        uint8_t command = cnc_state.rt_cmd & 0x07; //copies realtime flags states
        cnc_state.rt_cmd = 0;                      //clears command flags
        while (command)
        {
            switch (command & cmd_mask)
            {
            case RT_CMD_REPORT:
                if (!protocol_is_busy())
                {
                    protocol_send_status();
                }

                break;
            case RT_CMD_STARTUP_BLOCK0:
                if (settings_check_startup_gcode(STARTUP_BLOCK0_ADDRESS_OFFSET)) //loads command 0
                {
                    serial_select(SERIAL_N0);
                }

                SETFLAG(cnc_state.rt_cmd, RT_CMD_STARTUP_BLOCK1); //invokes command 1 on next pass
                break;
            case RT_CMD_STARTUP_BLOCK1:
                if (settings_check_startup_gcode(STARTUP_BLOCK1_ADDRESS_OFFSET)) //loads command 1
                {
                    serial_select(SERIAL_N1);
                }
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
                    if (mcu_get_pwm(SPINDLE_PWM))
                    {
                        update_tools = false;
                        mcu_set_pwm(SPINDLE_PWM, 0);
                    }
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
        uint8_t inputs = io_get_controls();
#ifdef ESTOP
        if (CHECKFLAG(inputs, ESTOP_MASK)) //fault on emergency stop
        {
            protocol_send_feedback(MSG_FEEDBACK_12);
        }
#endif
#ifdef SAFETY_DOOR
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
    }

    bool cnc_check_interlocking(void)
    {
        //if abort is flagged
        if (CHECKFLAG(cnc_state.exec_state, EXEC_ABORT))
        {
            if (cnc_state.active_alarm) //active alarm message
            {
                protocol_send_alarm(cnc_state.active_alarm);
                cnc_state.active_alarm = 0;
                return false;
            }
            return false;
        }

        if (CHECKFLAG(cnc_state.exec_state, EXEC_DOOR | EXEC_HOLD))
        {
            if (CHECKFLAG(cnc_state.exec_state, EXEC_RUN))
            {
                return true;
            }

            itp_stop();
            if (CHECKFLAG(cnc_state.exec_state, EXEC_DOOR))
            {
                cnc_stop(); //stop all tools not only motion
            }

            if (CHECKFLAG(cnc_state.exec_state, EXEC_HOMING) && CHECKFLAG(cnc_state.exec_state, EXEC_DOOR)) //door opened during a homing cycle
            {
                cnc_alarm(EXEC_ALARM_HOMING_FAIL_DOOR);
            }

            if (CHECKFLAG(cnc_state.exec_state, EXEC_HOMING | EXEC_JOG)) //flushes the buffers if motions was homing or jog
            {
                itp_clear();
                planner_clear();
                CLEARFLAG(cnc_state.exec_state, EXEC_HOMING | EXEC_JOG | EXEC_HOLD);
            }

            return false;
        }

        if (CHECKFLAG(cnc_state.exec_state, EXEC_LIMITS))
        {
            if (CHECKFLAG(cnc_state.exec_state, EXEC_RUN)) //if a motion is being performed allow trigger the limit switch alarm
            {
                cnc_alarm(EXEC_ALARM_HARD_LIMIT);
            }

            return false;
        }

        //clears EXEC_JOG if not step ISR is stopped and planner has no more moves
        if (CHECKFLAG(cnc_state.exec_state, EXEC_JOG) && !CHECKFLAG(cnc_state.exec_state, EXEC_RUN) && planner_buffer_is_empty())
        {
            CLEARFLAG(cnc_state.exec_state, EXEC_JOG);
        }

        return true;
    }

#ifdef __cplusplus
}
#endif
