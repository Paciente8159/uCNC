/*
	Name: pid_controller.c
	Description: PID controller for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 07/03/2021

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "pid_controller.h"
#include "../interface/settings.h"
#include <stdint.h>

#if (PID_CONTROLLERS > 0)
static int32_t cumulative_delta[PID_CONTROLLERS];
static int32_t last_error[PID_CONTROLLERS];
static uint16_t kp[PID_CONTROLLERS];
static uint16_t ki[PID_CONTROLLERS];
static uint32_t kd[PID_CONTROLLERS];
static uint8_t pid_freqdiv[PID_CONTROLLERS];

#define GAIN_FLT_TO_INT (1 << PID_BITSHIFT_FACTOR)
#define ERROR_MAX (GAIN_FLT_TO_INT - 1)
#define KP_MAX (GAIN_FLT_TO_INT - 1)
#define KI_MAX (((GAIN_FLT_TO_INT << 1) >> PID_DIVISIONS) - 1)
#define KD_MAX ((GAIN_FLT_TO_INT << 10) - 1)
#define ERROR_SUM_MAX (1 << (31 - PID_BITSHIFT_FACTOR - PID_DIVISIONS))

#endif

static int32_t pid_get_freqdiv(uint8_t i)
{
    switch (i)
    {
#if (PID_CONTROLLERS > 0)
    case 0:
        return PID0_FREQ_DIV;
#endif
#if (PID_CONTROLLERS > 1)
    case 1:
        return PID1_FREQ_DIV;
#endif
#if (PID_CONTROLLERS > 2)
    case 2:
        return PID2_FREQ_DIV;
#endif
#if (PID_CONTROLLERS > 3)
    case 3:
        return PID3_FREQ_DIV;
#endif
#if (PID_CONTROLLERS > 4)
    case 4:
        return PID4_FREQ_DIV;
#endif
#if (PID_CONTROLLERS > 5)
    case 5:
        return PID5_FREQ_DIV;
#endif
#if (PID_CONTROLLERS > 6)
    case 6:
        return PID6_FREQ_DIV;
#endif
#if (PID_CONTROLLERS > 7)
    case 7:
        return PID7_FREQ_DIV;
#endif
    }

    return 0;
}

static int32_t pid_get_error(uint8_t i)
{
    switch (i)
    {
#if (PID_CONTROLLERS > 0)
    case 0:
        return (int32_t)PID0_DELTA();
#endif
#if (PID_CONTROLLERS > 1)
    case 1:
        return (int32_t)PID1_DELTA();
#endif
#if (PID_CONTROLLERS > 2)
    case 2:
        return (int32_t)PID2_DELTA();
#endif
#if (PID_CONTROLLERS > 3)
    case 3:
        return (int32_t)PID3_DELTA();
#endif
#if (PID_CONTROLLERS > 4)
    case 4:
        return (int32_t)PID4_DELTA();
#endif
#if (PID_CONTROLLERS > 5)
    case 5:
        return (int32_t)PID5_DELTA();
#endif
#if (PID_CONTROLLERS > 6)
    case 6:
        return (int32_t)PID6_DELTA();
#endif
#if (PID_CONTROLLERS > 7)
    case 7:
        return (int32_t)PID7_DELTA();
#endif
    }

    return 0;
}

static int32_t pid_set_output(uint8_t i, int16_t val)
{
    switch (i)
    {
#if (PID_CONTROLLERS > 0)
    case 0:
        PID0_OUTPUT(val);
        return;
#endif
#if (PID_CONTROLLERS > 1)
    case 1:
        PID1_OUTPUT(val);
        return;
#endif
#if (PID_CONTROLLERS > 2)
    case 2:
        PID2_OUTPUT(val);
        return;
#endif
#if (PID_CONTROLLERS > 3)
    case 3:
        PID3_OUTPUT(val);
        return;
#endif
#if (PID_CONTROLLERS > 4)
    case 4:
        PID4_OUTPUT(val);
        return;
#endif
#if (PID_CONTROLLERS > 5)
    case 5:
        PID5_OUTPUT(val);
        return;
#endif
#if (PID_CONTROLLERS > 6)
    case 6:
        PID6_OUTPUT(val);
        return;
#endif
#if (PID_CONTROLLERS > 7)
    case 7:
        PID7_OUTPUT(val);
        return;
#endif
    }
}

void pid_stop()
{

#if (PID_CONTROLLERS > 0)
    PID0_STOP();
#endif
#if (PID_CONTROLLERS > 1)
    PID1_STOP();
#endif
#if (PID_CONTROLLERS > 2)
    PID2_STOP(val);
#endif
#if (PID_CONTROLLERS > 3)
    PID3_STOP(val);
#endif
#if (PID_CONTROLLERS > 4)
    PID4_STOP(val);
#endif
#if (PID_CONTROLLERS > 5)
    PID5_STOP(val);
#endif
#if (PID_CONTROLLERS > 6)
    PID6_STOP(val);
#endif
#if (PID_CONTROLLERS > 7)
    PID7_STOP(val);
#endif
}

//PID ISR should run once every millisecond
//sampling rate is 1000/(log2*(Nº of PID controllers))
//a single PID controller can run at 1000Hz
//all 8 PID will run at a max freq of 125Hz
//this precomputes the PID factors to save computation cycles
void pid_init(void)
{
#if (PID_CONTROLLERS > 0)
    for (uint8_t i = 0; i < PID_CONTROLLERS; i++)
    {
        //error gains must be between 0% and 100% (0 and 1)
        uint32_t k = (uint32_t)(g_settings.pid_gain[i][0] * (float)ERROR_MAX);
        kp[i] = (uint16_t)CLAMP(k, 0, KP_MAX);
        k = (uint32_t)((g_settings.pid_gain[i][1] / (PID_SAMP_FREQ / (float)pid_get_freqdiv(i))) * (float)ERROR_MAX);
        ki[i] = (uint16_t)CLAMP(k, 0, KI_MAX);
        k = (uint32_t)((g_settings.pid_gain[i][2] * (PID_SAMP_FREQ / (float)pid_get_freqdiv(i))) * (float)ERROR_MAX);
        kd[i] = (uint32_t)CLAMP(k, 0, KD_MAX);
    }
#endif
}

void pid_update(void)
{
#if (PID_CONTROLLERS > 0)

    static uint8_t current_pid = 0;
    if (current_pid < PID_CONTROLLERS)
    {
        if (!pid_freqdiv[current_pid])
        {
            //keeps all math in 32bit
            //9bits
            int32_t error = pid_get_error(current_pid);
            CLAMP(error, -ERROR_MAX, ERROR_MAX);
            int32_t output = 0;
            if (kp[current_pid])
            {
                //max 16bits
                output = kp[current_pid] * error;
            }

            if (ki[current_pid])
            {
                int32_t sum = cumulative_delta[current_pid] + error;
                sum = CLAMP(sum, -ERROR_SUM_MAX, ERROR_SUM_MAX);
                cumulative_delta[current_pid] = (int32_t)sum;
                //max 31bits
                output += ki[current_pid] * cumulative_delta[current_pid];
            }

            if (kd[current_pid])
            {
                //max 26bits
                int32_t rateerror = (error - last_error[current_pid]) * kd[current_pid];
                output += rateerror;
            }

            last_error[current_pid] = error;
            bool isneg = (output < 0) ? true : false;
            output = (!isneg) ? output : -output;
            output >>= PID_BITSHIFT_FACTOR;
            output = (!isneg) ? output : -output;
            //9bits
            output = CLAMP(output, PID_OUTPUT_MIN, PID_OUTPUT_MAX);
            pid_set_output(current_pid, (int16_t)output);
        }

        if (++pid_freqdiv[current_pid] >= pid_get_freqdiv(current_pid))
        {
            pid_freqdiv[current_pid] = 0;
        }
    }

    //restart
    if (++current_pid >= PID_CONTROLLERS)
    {
        current_pid = 0;
    }

#endif
}
