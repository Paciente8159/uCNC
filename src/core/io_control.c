/*
	Name: io_control.c
	Description: The input control unit for µCNC.
        This is responsible to check all limit switches (both hardware and software), control switches,
        and probe.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 07/12/2019

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
#include "core/io_control.h"
#include "core/parser.h"
#include "core/interpolator.h"

    static volatile uint8_t io_limits_homing_filter;
#if PID_CONTROLLERS > 0
    static volatile uint8_t io_spindle_speed;
#endif

    void io_limits_isr(void)
    {
        uint8_t limits = io_get_limits();

        if (g_settings.hard_limits_enabled)
        {
            if (limits)
            {
#ifdef ENABLE_DUAL_DRIVE_AXIS
                if (cnc_get_exec_state(EXEC_RUN) & cnc_get_exec_state(EXEC_HOMING))
                {
//if homing and dual drive axis are enabled
#ifdef DUAL_DRIVE_AXIS0
                    if ((limits & (LIMIT_DUAL0 | LIMITS_DUAL_MASK) & io_limits_homing_filter)) //the limit triggered matches the first dual drive axis
                    {
                        itp_lock_stepper((limits & LIMITS_LIMIT1_MASK) ? STEP6_MASK : STEP_DUAL0);

                        if ((limits & LIMITS_DUAL_MASK) != LIMITS_DUAL_MASK) //but not both
                        {
                            return; //exits and doesn't trip the alarm
                        }
                    }
#endif
#ifdef DUAL_DRIVE_AXIS1
                    if (limits & LIMIT_DUAL1 & io_limits_homing_filter) //the limit triggered matches the second dual drive axis
                    {
                        if ((limits & LIMITS_DUAL_MASK) != LIMITS_DUAL_MASK) //but not both
                        {
                            itp_lock_stepper((limits & LIMITS_LIMIT1_MASK) ? STEP7_MASK : STEP_DUAL1);
                        }
                    }
#endif
                }
#endif
#ifdef ENABLE_DUAL_DRIVE_AXIS
                itp_lock_stepper(0); //unlocks axis
#endif
                itp_stop();
                cnc_set_exec_state(EXEC_HALT);
            }
        }
    }

    void io_controls_isr(void)
    {
        uint8_t controls = io_get_controls();

#ifdef ESTOP
        if (CHECKFLAG(controls, ESTOP_MASK))
        {
            cnc_set_exec_state(EXEC_KILL);
            return; //forces exit
        }
#endif
#ifdef SAFETY_DOOR
        if (CHECKFLAG(controls, SAFETY_DOOR_MASK))
        {
            //safety door activates hold simultaneously to start the controlled stop
            cnc_set_exec_state(EXEC_DOOR | EXEC_HOLD);
        }
#endif
#ifdef FHOLD
        if (CHECKFLAG(controls, FHOLD_MASK))
        {
            cnc_set_exec_state(EXEC_HOLD);
        }
#endif
#ifdef CS_RES
        if (CHECKFLAG(controls, CS_RES_MASK))
        {
            cnc_call_rt_command(CMD_CODE_CYCLE_START);
        }
#endif
    }

    void io_probe_isr(void)
    {
        //on hit enables hold (directly)
        cnc_set_exec_state(EXEC_HOLD);
        //stores rt position
        parser_sync_probe();
    }

    bool io_check_boundaries(float *axis)
    {
        if (!g_settings.soft_limits_enabled)
        {
            return true;
        }

        for (uint8_t i = AXIS_COUNT; i != 0;)
        {
            i--;
            float value = (axis[i] < 0) ? -axis[i] : axis[i];
            if (value > g_settings.max_distance[i])
            {
                return false;
            }
        }

        return true;
    }

    uint8_t io_get_limits(void)
    {
        uint8_t value = 0;
#ifdef LIMIT_X
#ifdef LIMIT_X2
        value |= ((mcu_get_input(LIMIT_X)) ? (LIMIT_X_MASK | LIMITS_LIMIT0_MASK) : 0);
#else
        value |= ((mcu_get_input(LIMIT_X)) ? LIMIT_X_MASK : 0);
#endif
#endif
#ifdef LIMIT_Y
#ifdef LIMIT_Y2
        value |= ((mcu_get_input(LIMIT_Y)) ? (LIMIT_Y_MASK | LIMITS_LIMIT0_MASK) : 0);
#else
        value |= ((mcu_get_input(LIMIT_Y)) ? LIMIT_Y_MASK : 0);
#endif
#endif
#ifdef LIMIT_Z
#ifdef LIMIT_Z2
        value |= ((mcu_get_input(LIMIT_Z)) ? (LIMIT_Z_MASK | LIMITS_LIMIT0_MASK) : 0);
#else
        value |= ((mcu_get_input(LIMIT_Z)) ? LIMIT_Z_MASK : 0);
#endif
#endif
#ifdef LIMIT_X2
        value |= ((mcu_get_input(LIMIT_X2)) ? (LIMIT_X_MASK | LIMITS_LIMIT1_MASK) : 0);
#endif
#ifdef LIMIT_Y2
        value |= ((mcu_get_input(LIMIT_Y2)) ? (LIMIT_Y_MASK | LIMITS_LIMIT1_MASK) : 0);
#endif
#ifdef LIMIT_Z2
        value |= ((mcu_get_input(LIMIT_Z2)) ? (LIMIT_Z_MASK | LIMITS_LIMIT1_MASK) : 0);
#endif
#ifdef LIMIT_A
        value |= ((mcu_get_input(LIMIT_A)) ? LIMIT_A_MASK : 0);
#endif
#ifdef LIMIT_B
        value |= ((mcu_get_input(LIMIT_B)) ? LIMIT_B_MASK : 0);
#endif
#ifdef LIMIT_C
        value |= ((mcu_get_input(LIMIT_C)) ? LIMIT_C_MASK : 0);
#endif

        return (value ^ g_settings.limits_invert_mask);
    }

    uint8_t io_get_controls(void)
    {
        uint8_t value = 0;
#ifdef ESTOP
        value |= ((mcu_get_input(ESTOP)) ? ESTOP_MASK : 0);
#endif
#ifdef SAFETY_DOOR
        value |= ((mcu_get_input(SAFETY_DOOR)) ? SAFETY_DOOR_MASK : 0);
#endif
#ifdef FHOLD
        value |= ((mcu_get_input(FHOLD)) ? FHOLD_MASK : 0);
#endif
#ifdef CS_RES
        value |= ((mcu_get_input(CS_RES)) ? CS_RES_MASK : 0);
#endif

        return (value ^ g_settings.control_invert_mask);
    }

    void io_enable_probe(void)
    {
#ifndef FORCE_SOFT_POLLING
#ifdef PROBE
        mcu_enable_probe_isr();
#endif
#endif
    }

    void io_disable_probe(void)
    {
#ifndef FORCE_SOFT_POLLING
#ifdef PROBE
        mcu_disable_probe_isr();
#endif
#endif
    }

    bool io_get_probe(void)
    {
#ifdef PROBE
        bool probe = (mcu_get_input(PROBE) != 0);
        return (!g_settings.probe_invert_mask) ? probe : !probe;
#else
    return false;
#endif
    }

    void io_set_homing_limits_filter(uint8_t filter_mask)
    {
        io_limits_homing_filter = filter_mask;
    }

    //outputs
    void io_set_steps(uint8_t mask)
    {
#if (STEPPER_COUNT > 0 && defined(STEP0))
#ifndef STEP0_NOTSTEPPER
        if (mask & STEP0_MASK)
        {
            mcu_set_output(STEP0);
        }
        else
        {
            mcu_clear_output(STEP0);
        }
#endif
#endif
#if (STEPPER_COUNT > 1 && defined(STEP1))
#ifndef STEP1_NOTSTEPPER
        if (mask & STEP1_MASK)
        {
            mcu_set_output(STEP1);
        }
        else
        {
            mcu_clear_output(STEP1);
        }
#endif
#endif
#if (STEPPER_COUNT > 2 && defined(STEP2))
#ifndef STEP2_NOTSTEPPER
        if (mask & STEP2_MASK)
        {
            mcu_set_output(STEP2);
        }
        else
        {
            mcu_clear_output(STEP2);
        }
#endif
#endif
#if (STEPPER_COUNT > 3 && defined(STEP3))
#ifndef STEP3_NOTSTEPPER
        if (mask & STEP3_MASK)
        {
            mcu_set_output(STEP3);
        }
        else
        {
            mcu_clear_output(STEP3);
        }
#endif
#endif
#if (STEPPER_COUNT > 4 && defined(STEP4))
#ifndef STEP4_NOTSTEPPER
        if (mask & STEP4_MASK)
        {
            mcu_set_output(STEP4);
        }
        else
        {
            mcu_clear_output(STEP4);
        }
#endif
#endif
#if (STEPPER_COUNT > 5 && defined(STEP5))
#ifndef STEP5_NOTSTEPPER
        if (mask & STEP5_MASK)
        {
            mcu_set_output(STEP5);
        }
        else
        {
            mcu_clear_output(STEP5);
        }
#endif
#endif
#ifdef STEP6
#ifndef STEP6_NOTSTEPPER
        if (mask & STEP6_MASK)
        {
            mcu_set_output(STEP6);
        }
        else
        {
            mcu_clear_output(STEP6);
        }
#endif
#endif
#ifdef STEP7
#ifndef STEP7_NOTSTEPPER
        if (mask & STEP7_MASK)
        {
            mcu_set_output(STEP7);
        }
        else
        {
            mcu_clear_output(STEP7);
        }
#endif
#endif
    }

    void io_toggle_steps(uint8_t mask)
    {
#if (STEPPER_COUNT > 0 && defined(STEP0))
#ifdef STEP0_ISSTEPPER
        if (mask & STEP0_MASK)
        {
            mcu_toggle_output(STEP0);
        }
#endif
#endif
#if (STEPPER_COUNT > 1 && defined(STEP1))
#ifdef STEP1_ISSTEPPER
        if (mask & STEP1_MASK)
        {
            mcu_toggle_output(STEP1);
        }
#endif
#endif
#if (STEPPER_COUNT > 2 && defined(STEP2))
#ifdef STEP2_ISSTEPPER
        if (mask & STEP2_MASK)
        {
            mcu_toggle_output(STEP2);
        }
#endif
#endif
#if (STEPPER_COUNT > 3 && defined(STEP3))
#ifdef STEP3_ISSTEPPER
        if (mask & STEP3_MASK)
        {
            mcu_toggle_output(STEP3);
        }
#endif
#endif
#if (STEPPER_COUNT > 4 && defined(STEP4))
#ifdef STEP4_ISSTEPPER
        if (mask & STEP4_MASK)
        {
            mcu_toggle_output(STEP4);
        }
#endif
#endif
#if (STEPPER_COUNT > 5 && defined(STEP5))
#ifdef STEP5_ISSTEPPER
        if (mask & STEP5_MASK)
        {
            mcu_toggle_output(STEP5);
        }
#endif
#endif
#ifdef STEP6
#ifdef STEP6_ISSTEPPER
        if (mask & STEP6_MASK)
        {
            mcu_toggle_output(STEP6);
        }
#endif
#endif
#ifdef STEP7
#ifdef STEP7_ISSTEPPER
        if (mask & STEP7_MASK)
        {
            mcu_toggle_output(STEP7);
        }
#endif
#endif
    }

    void io_set_dirs(uint8_t mask)
    {
#if (STEPPER_COUNT > 0 && defined(DIR0))
#ifdef STEP0_ISSTEPPER
        if (mask & DIR0_MASK)
        {
            mcu_set_output(DIR0);
        }
        else
        {
            mcu_clear_output(DIR0);
        }
#endif
#endif
#if (STEPPER_COUNT > 1 && defined(DIR1))
#ifdef STEP1_ISSTEPPER
        if (mask & DIR1_MASK)
        {
            mcu_set_output(DIR1);
        }
        else
        {
            mcu_clear_output(DIR1);
        }
#endif
#endif
#if (STEPPER_COUNT > 2 && defined(DIR2))
#ifdef STEP2_ISSTEPPER
        if (mask & DIR2_MASK)
        {
            mcu_set_output(DIR2);
        }
        else
        {
            mcu_clear_output(DIR2);
        }
#endif
#endif
#if (STEPPER_COUNT > 3 && defined(DIR3))
#ifdef STEP3_ISSTEPPER
        if (mask & DIR3_MASK)
        {
            mcu_set_output(DIR3);
        }
        else
        {
            mcu_clear_output(DIR3);
        }
#endif
#endif
#if (STEPPER_COUNT > 4 && defined(DIR4))
#ifdef STEP4_ISSTEPPER
        if (mask & DIR4_MASK)
        {
            mcu_set_output(DIR4);
        }
        else
        {
            mcu_clear_output(DIR4);
        }
#endif
#endif
#if (STEPPER_COUNT > 5 && defined(DIR5))
#ifdef STEP5_ISSTEPPER
        if (mask & DIR5_MASK)
        {
            mcu_set_output(DIR5);
        }
        else
        {
            mcu_clear_output(DIR5);
        }
#endif
#endif
    }

    void io_enable_steps(void)
    {
#ifdef STEPPER_ENABLE
        mcu_set_output(STEPPER_ENABLE);
#endif
#ifdef STEPPER1_ENABLE
        mcu_set_output(STEPPER1_ENABLE);
#endif
#ifdef STEPPER2_ENABLE
        mcu_set_output(STEPPER2_ENABLE);
#endif
#ifdef STEPPER3_ENABLE
        mcu_set_output(STEPPER3_ENABLE);
#endif
#ifdef STEPPER4_ENABLE
        mcu_set_output(STEPPER4_ENABLE);
#endif
#ifdef STEPPER5_ENABLE
        mcu_set_output(STEPPER5_ENABLE);
#endif
    }

#ifdef USE_SPINDLE
    void io_set_spindle(uint8_t value, bool invert)
    {
#if PID_CONTROLLERS > 0
        io_spindle_speed = value;
#endif
#ifdef LASER_MODE
        if (!g_settings.laser_mode)
        {
#endif
            if (!invert)
            {
                mcu_clear_output(SPINDLE_DIR);
            }
            else
            {
                mcu_set_output(SPINDLE_DIR);
            }
#ifdef LASER_MODE
        }
#endif
        mcu_set_pwm(SPINDLE_PWM, value);
    }

    uint8_t io_get_spindle(void)
    {
#if PID_CONTROLLERS > 0
        return io_spindle_speed;
#else
        return mcu_get_pwm(SPINDLE_PWM);
#endif
    }
#endif

#ifdef USE_COOLANT
    void io_set_coolant(uint8_t value)
    {
#ifdef COOLANT_FLOOD
        if (value & COOLANT_MASK)
        {
            mcu_set_output(COOLANT_FLOOD);
        }
        else
        {
            mcu_clear_output(COOLANT_FLOOD);
        }
#endif
#ifdef COOLANT_MIST
#if (COOLANT_FLOOD != COOLANT_MIST)
        if (value & MIST_MASK)
        {
            mcu_set_output(COOLANT_MIST);
        }
        else
        {
            mcu_clear_output(COOLANT_MIST);
        }
#endif
#endif
    }
#endif

#ifdef __cplusplus
}
#endif
