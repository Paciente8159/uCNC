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

#include "../cnc.h"
#include <stdint.h>

#if (PID_CONTROLLERS > 0)
static int32_t cumulative_delta[PID_CONTROLLERS];
static int32_t last_error[PID_CONTROLLERS];
static int32_t kp[PID_CONTROLLERS];
static int32_t ki[PID_CONTROLLERS];
static int32_t kd[PID_CONTROLLERS];
static uint8_t pid_freqdiv[PID_CONTROLLERS];
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

//PID ISR should run once every millisecond
//this is equal to a 125Hz sampling rate (for 8 PID controllers)
//this precomputes the PID factors to save computation cycles
void pid_init(void)
{
#if (PID_CONTROLLERS > 0)
    for (uint8_t i = 0; i < PID_CONTROLLERS; i++)
    {
        kp[i] = (int32_t)(g_settings.pid_gain[i][0] * (float)(1 << PID_BITSHIFT_FACTOR));
        ki[i] = (int32_t)((g_settings.pid_gain[i][1] * (float)(1 << PID_BITSHIFT_FACTOR)) / (125.0f / (float)pid_get_freqdiv(i)));
        kd[i] = (int32_t)((g_settings.pid_gain[i][2] * (float)(1 << PID_BITSHIFT_FACTOR)) * (125.0f / (float)pid_get_freqdiv(i)));
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
            int32_t error = pid_get_error(current_pid);
            int64_t output = 0;
            if (kp[current_pid])
            {
                output = kp[current_pid] * error;
            }

            if (ki[current_pid])
            {
                int64_t sum = cumulative_delta[current_pid] + error;
                sum = MIN(sum, 0x7FFFFFFF);
                cumulative_delta[current_pid] = (int32_t)MAX(sum, -0x7FFFFFFF);

                output += ki[current_pid] * cumulative_delta[current_pid];
            }

            if (kd[current_pid])
            {
                int32_t rateerror = (error - last_error[current_pid]) * kd[current_pid];
                output += rateerror;
            }

            last_error[current_pid] = error;
            output >>= PID_BITSHIFT_FACTOR;
            output = MIN(output, PID_OUTPUT_MAX);
            output = MAX(output, PID_OUTPUT_MIN);
            uint8_t pid_result = (uint8_t)output;
            pid_set_output(current_pid, pid_result);
        }

        if (++pid_freqdiv[current_pid] >= pid_get_freqdiv(current_pid))
        {
            pid_freqdiv[current_pid] = 0;
        }
    }

    if (++current_pid >= 8)
    {
        current_pid = 0;
    }

#endif
}
