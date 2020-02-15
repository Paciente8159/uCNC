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
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "config.h"
#include "grbl_interface.h"
#include "settings.h"
#include "mcu.h"
#include "io_control.h"
#include "parser.h"
#include "interpolator.h"
#include "cnc.h"

static volatile uint8_t io_limits_homing_filter;

void io_limits_isr()
{
    uint8_t limits = io_get_limits();

    if(g_settings.hard_limits_enabled)
    {
        if(limits)
        {
            if(cnc_get_exec_state(EXEC_RUN))
            {
                if(!cnc_get_exec_state(EXEC_HOMING)) //if not in a homing motion triggers an alarm
                {
                	if(g_settings.homing_enabled)
	                {
	                    cnc_set_exec_state(EXEC_NOHOME); //if motions was executing flags home position lost
	                }
	                
                    cnc_alarm(EXEC_ALARM_HARD_LIMIT);
                }
                #ifdef ENABLE_DUAL_DRIVE_AXIS
                else
                {
                	//if homing and dual drive axis are enabled
                	#ifdef DUAL_DRIVE_AXIS0
                	if((limits & (LIMIT_DUAL0 | LIMITS_DUAL_MASK) & io_limits_homing_filter)) //the limit triggered matches the first dual drive axis
                	{
                        itp_lock_stepper((limits & LIMITS_LIMIT1_MASK) ? STEP6_MASK : STEP_DUAL0);

                		if((limits & LIMITS_DUAL_MASK) != LIMITS_DUAL_MASK) //but not both
                		{
                			return; //exits and doesn't trip the alarm
						}
					}
                	#endif
                	#ifdef DUAL_DRIVE_AXIS1
                	if(limits & LIMIT_DUAL1 & io_limits_homing_filter) //the limit triggered matches the second dual drive axis
                	{
                		if((limits & LIMITS_DUAL_MASK) != LIMITS_DUAL_MASK) //but not both
                		{
                			itp_lock_stepper((limits & LIMITS_LIMIT1_MASK) ? STEP7_MASK : STEP_DUAL1);
                		}
					}
                	#endif
				}
				#endif
            }
            #ifdef ENABLE_DUAL_DRIVE_AXIS
            itp_lock_stepper(0); //unlocks axis
            #endif
            cnc_set_exec_state(EXEC_LIMITS);
            itp_stop();
        }
    }
}

void io_controls_isr()
{
    uint8_t controls = io_get_controls();

#ifdef ESTOP
    if(CHECKFLAG(controls,ESTOP_MASK))
    {
        cnc_call_rt_command(CMD_CODE_RESET);
        return; //forces exit
    }
#endif
#ifdef SAFETY_DOOR
    if(CHECKFLAG(controls,SAFETY_DOOR_MASK))
    {
        //safety door activates hold simultaneously to start the controlled stop
        cnc_call_rt_command(CMD_CODE_SAFETY_DOOR);
    }
#endif
#ifdef FHOLD
    if(CHECKFLAG(controls,FHOLD_MASK))
    {
        cnc_call_rt_command(CMD_CODE_FEED_HOLD);
    }
#endif
#ifdef CS_RES
    if(CHECKFLAG(controls,CS_RES_MASK))
    {
        cnc_call_rt_command(CMD_CODE_CYCLE_START);
    }
#endif
}

void io_probe_isr()
{
    //on hit enables hold (directly)
    cnc_set_exec_state(EXEC_HOLD);
    //stores rt position
    parser_sync_probe();
}

bool io_check_boundaries(float* axis)
{
    if(!g_settings.soft_limits_enabled)
    {
        return true;
    }

    for(uint8_t i = AXIS_COUNT; i!=0;)
    {
        i--;
        float value = (axis[i] < 0) ? -axis[i] : axis[i];
        if(value > g_settings.max_distance[i])
        {
            return false;
        }
    }

    return true;
}

uint8_t io_get_limits()
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

uint8_t io_get_controls()
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

void io_enable_probe()
{
    #ifndef USE_INPUTS_POOLING_ONLY
    #ifdef PROBE
    mcu_enable_probe_isr();
    #endif
    #endif
}

void io_disable_probe()
{
    #ifndef USE_INPUTS_POOLING_ONLY
    #ifdef PROBE
    mcu_disable_probe_isr();
    #endif
    #endif
}

bool io_get_probe()
{
    #ifdef PROBE
    bool probe = (mcu_get_input(PROBE)!=0);
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
#ifdef STEP0
    if(mask & STEP0_MASK)
    {
        mcu_set_output(STEP0);
    }
    else
    {
        mcu_clear_output(STEP0);
    }
#endif
#ifdef STEP1
    if(mask & STEP1_MASK)
    {
        mcu_set_output(STEP1);
    }
    else
    {
        mcu_clear_output(STEP1);
    }
#endif
#ifdef STEP2
    if(mask & STEP2_MASK)
    {
        mcu_set_output(STEP2);
    }
    else
    {
        mcu_clear_output(STEP2);
    }
#endif
#ifdef STEP3
    if(mask & STEP3_MASK)
    {
        mcu_set_output(STEP3);
    }
    else
    {
        mcu_clear_output(STEP3);
    }
#endif
#ifdef STEP4
    if(mask & STEP4_MASK)
    {
        mcu_set_output(STEP4);
    }
    else
    {
        mcu_clear_output(STEP4);
    }
#endif
#ifdef STEP5
    if(mask & STEP5_MASK)
    {
        mcu_set_output(STEP5);
    }
    else
    {
        mcu_clear_output(STEP5);
    }
#endif
#ifdef STEP6
    if(mask & STEP6_MASK)
    {
        mcu_set_output(STEP6);
    }
    else
    {
        mcu_clear_output(STEP6);
    }
#endif
#ifdef STEP7
    if(mask & STEP7_MASK)
    {
        mcu_set_output(STEP7);
    }
    else
    {
        mcu_clear_output(STEP7);
    }
#endif
}

void io_toggle_steps(uint8_t mask)
{
#ifdef STEP0
    if(mask & STEP0_MASK)
    {
        mcu_toggle_output(STEP0);
    }
#endif
#ifdef STEP1
    if(mask & STEP1_MASK)
    {
        mcu_toggle_output(STEP1);
    }
#endif
#ifdef STEP2
    if(mask & STEP2_MASK)
    {
        mcu_toggle_output(STEP2);
    }
#endif
#ifdef STEP3
    if(mask & STEP3_MASK)
    {
        mcu_toggle_output(STEP3);
    }
#endif
#ifdef STEP4
    if(mask & STEP4_MASK)
    {
        mcu_toggle_output(STEP4);
    }
#endif
#ifdef STEP5
    if(mask & STEP5_MASK)
    {
        mcu_toggle_output(STEP5);
    }
#endif
#ifdef STEP6
    if(mask & STEP6_MASK)
    {
        mcu_toggle_output(STEP6);
    }
#endif
#ifdef STEP7
    if(mask & STEP7_MASK)
    {
        mcu_toggle_output(STEP7);
    }
#endif

}

void io_set_dirs(uint8_t mask)
{
#ifdef DIR0
    if(mask & DIR0_MASK)
    {
        mcu_set_output(DIR0);
    }
    else
    {
        mcu_clear_output(DIR0);
    }
#endif
#ifdef DIR1
    if(mask & DIR1_MASK)
    {
        mcu_set_output(DIR1);
    }
    else
    {
        mcu_clear_output(DIR1);
    }
#endif
#ifdef DIR2
    if(mask & DIR2_MASK)
    {
        mcu_set_output(DIR2);
    }
    else
    {
        mcu_clear_output(DIR2);
    }
#endif
#ifdef DIR3
    if(mask & DIR3_MASK)
    {
        mcu_set_output(DIR3);
    }
    else
    {
        mcu_clear_output(DIR3);
    }
#endif
#ifdef DIR4
    if(mask & DIR4_MASK)
    {
        mcu_set_output(DIR4);
    }
    else
    {
        mcu_clear_output(DIR4);
    }
#endif
#ifdef DIR5
    if(mask & DIR5_MASK)
    {
        mcu_set_output(DIR5);
    }
    else
    {
        mcu_clear_output(DIR5);
    }
#endif

}

void io_enable_steps()
{
    #ifdef STEP0_EN
    mcu_set_output(STEP0_EN);
    #endif
    #ifdef STEP1_EN
    mcu_set_output(STEP1_EN);
    #endif
    #ifdef STEP2_EN
    mcu_set_output(STEP2_EN);
    #endif
    #ifdef STEP3_EN
    mcu_set_output(STEP3_EN);
    #endif
    #ifdef STEP4_EN
    mcu_set_output(STEP4_EN);
    #endif
    #ifdef STEP5_EN
    mcu_set_output(STEP5_EN);
    #endif
}
