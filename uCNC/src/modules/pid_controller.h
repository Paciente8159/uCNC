/*
	Name: pid_controller.h
	Description: PID controller for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 03-07-2021

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../cnc.h"

#if PID_CONTROLLERS > 0
#ifndef PID0_DELTA
#error "The PID0 error is not defined"
#endif
#ifndef PID0_OUTPUT
#error "The PID0 output is not defined"
#endif
#ifndef PID0_FREQ_DIV
#define PID0_FREQ_DIV 1
#elif (PID0_FREQ_DIV < 1 || PID0_FREQ_DIV > 250)
#error "The PID0 sampling frequency devider value must be between 1 and 250"
#endif
#endif
#if PID_CONTROLLERS > 1
#ifndef PID1_DELTA
#error "The PID1 error is not defined"
#endif
#ifndef PID1_OUTPUT
#error "The PID1 output is not defined"
#endif
#ifndef PID1_FREQ_DIV
#define PID1_FREQ_DIV 1
#elif (PID1_FREQ_DIV < 1 || PID1_FREQ_DIV > 250)
#error "The PID1 sampling frequency devider value must be between 1 and 250"
#endif
#endif
#if PID_CONTROLLERS > 2
#ifndef PID2_DELTA
#error "The PID2 error is not defined"
#endif
#ifndef PID2_OUTPUT
#error "The PID2 output is not defined"
#endif
#ifndef PID2_FREQ_DIV
#define PID2_FREQ_DIV 1
#elif (PID2_FREQ_DIV < 1 || PID2_FREQ_DIV > 250)
#error "The PID2 sampling frequency devider value must be between 1 and 250"
#endif
#endif
#if PID_CONTROLLERS > 3
#ifndef PID3_DELTA
#error "The PID3 error is not defined"
#endif
#ifndef PID3_OUTPUT
#error "The PID3 output is not defined"
#endif
#ifndef PID3_FREQ_DIV
#define PID3_FREQ_DIV 1
#elif (PID3_FREQ_DIV < 1 || PID3_FREQ_DIV > 250)
#error "The PID3 sampling frequency devider value must be between 1 and 250"
#endif
#endif
#if PID_CONTROLLERS > 4
#ifndef PID4_DELTA
#error "The PID4 error is not defined"
#endif
#ifndef PID4_OUTPUT
#error "The PID4 output is not defined"
#endif
#ifndef PID4_FREQ_DIV
#define PID4_FREQ_DIV 1
#elif (PID4_FREQ_DIV < 1 || PID4_FREQ_DIV > 250)
#error "The PID4 sampling frequency devider value must be between 1 and 250"
#endif
#endif
#if PID_CONTROLLERS > 5
#ifndef PID5_DELTA
#error "The PID5 error is not defined"
#endif
#ifndef PID5_OUTPUT
#error "The PID5 output is not defined"
#endif
#ifndef PID5_FREQ_DIV
#define PID5_FREQ_DIV 1
#elif (PID5_FREQ_DIV < 1 || PID5_FREQ_DIV > 250)
#error "The PID5 sampling frequency devider value must be between 1 and 250"
#endif
#endif
#if PID_CONTROLLERS > 6
#ifndef PID6_DELTA
#error "The PID6 error is not defined"
#endif
#ifndef PID6_OUTPUT
#error "The PID6 output is not defined"
#endif
#ifndef PID6_FREQ_DIV
#define PID6_FREQ_DIV 1
#elif (PID6_FREQ_DIV < 1 || PID6_FREQ_DIV > 250)
#error "The PID6 sampling frequency devider value must be between 1 and 250"
#endif
#endif
#if PID_CONTROLLERS > 7
#ifndef PID7_DELTA
#error "The PID7 error is not defined"
#endif
#ifndef PID7_OUTPUT
#error "The PID7 output is not defined"
#endif
#ifndef PID7_FREQ_DIV
#define PID7_FREQ_DIV 1
#elif (PID7_FREQ_DIV < 1 || PID7_FREQ_DIV > 250)
#error "The PID7 sampling frequency devider value must be between 1 and 250"
#endif
#endif

//These parameters adjust the PID to use integer math only and output limiting (usually to be used with PWM)
#define PID_BITSHIFT_FACTOR 8
#define PID_OUTPUT_MAX 127
#define PID_OUTPUT_MIN -127

	void pid_init(void);
	void pid_update(void);

#ifdef __cplusplus
}
#endif

#endif