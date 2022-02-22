/*
	Name: cnc_hal_config_helper.h
	Description: Compile time configurations for µCNC. This file takes care of some final configuration definitions based on the user options

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 2022-01-04

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef CNC_HAL_CONFIG_HELPER_H
#define CNC_HAL_CONFIG_HELPER_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ENCODERS
#define ENCODERS 0
#endif

#ifndef PID_CONTROLLERS
#define PID_CONTROLLERS 0
#endif

#if PID_CONTROLLERS > 0
/*PID controllers*/
#if PID_CONTROLLERS == 1
#define PID_DIVISIONS 0
#elif PID_CONTROLLERS == 2
#define PID_DIVISIONS 1
#elif PID_CONTROLLERS <= 4
#define PID_DIVISIONS 2
#else
#define PID_DIVISIONS 3
#endif

#define PID_SAMP_FREQ (1 << (10 - PID_DIVISIONS))
#endif

#if PID_CONTROLLERS > 0
#ifdef PID0_DELTA
#error "The PID0 is reserved for the tool PID"
#else
#define PID0_DELTA() tool_pid_error()
#endif
#ifdef PID0_OUTPUT
#error "The PID0 is reserved for the tool PID"
#else
#define PID0_OUTPUT(X) tool_pid_update(X)
#endif
#ifdef PID0_STOP
#error "The PID0 is reserved for the tool PID"
#else
#define PID0_STOP() tool_stop()
#endif
#ifndef PID0_FREQ_DIV
#define PID0_FREQ_DIV 1
#elif (PID0_FREQ_DIV < 1 || PID0_FREQ_DIV > PID_SAMP_FREQ)
#error "The PID0 sampling frequency devider value must be between 1 and MAX SAMPLE RATE = 1000/log2(Total PID"s)"
#endif
#endif
#if PID_CONTROLLERS > 1
#ifndef PID1_DELTA
#error "The PID1 error is not defined"
#endif
#ifndef PID1_OUTPUT
#error "The PID1 output is not defined"
#endif
#ifndef PID1_STOP
#error "The PID1 stop is not defined"
#endif
#ifndef PID1_FREQ_DIV
#define PID1_FREQ_DIV 1
#elif (PID1_FREQ_DIV < 1 || PID1_FREQ_DIV > PID_SAMP_FREQ)
#error "The PID1 sampling frequency devider value must be between 1 and MAX SAMPLE RATE = 1000/log2(Total PID"s)"
#endif
#endif
#if PID_CONTROLLERS > 2
#ifndef PID2_DELTA
#error "The PID2 error is not defined"
#endif
#ifndef PID2_OUTPUT
#error "The PID2 output is not defined"
#endif
#ifndef PID2_STOP
#error "The PID2 stop is not defined"
#endif
#ifndef PID2_FREQ_DIV
#define PID2_FREQ_DIV 1
#elif (PID2_FREQ_DIV < 1 || PID2_FREQ_DIV > PID_SAMP_FREQ)
#error "The PID2 sampling frequency devider value must be between 1 and MAX SAMPLE RATE = 1000/log2(Total PID"s)"
#endif
#endif
#if PID_CONTROLLERS > 3
#ifndef PID3_DELTA
#error "The PID3 error is not defined"
#endif
#ifndef PID3_OUTPUT
#error "The PID3 output is not defined"
#endif
#ifndef PID3_STOP
#error "The PID3 stop is not defined"
#endif
#ifndef PID3_FREQ_DIV
#define PID3_FREQ_DIV 1
#elif (PID3_FREQ_DIV < 1 || PID3_FREQ_DIV > PID_SAMP_FREQ)
#error "The PID3 sampling frequency devider value must be between 1 and MAX SAMPLE RATE = 1000/log2(Total PID"s)"
#endif
#endif
#if PID_CONTROLLERS > 4
#ifndef PID4_DELTA
#error "The PID4 error is not defined"
#endif
#ifndef PID4_OUTPUT
#error "The PID4 output is not defined"
#endif
#ifndef PID4_STOP
#error "The PID4 stop is not defined"
#endif
#ifndef PID4_FREQ_DIV
#define PID4_FREQ_DIV 1
#elif (PID4_FREQ_DIV < 1 || PID4_FREQ_DIV > PID_SAMP_FREQ)
#error "The PID4 sampling frequency devider value must be between 1 and MAX SAMPLE RATE = 1000/log2(Total PID"s)"
#endif
#endif
#if PID_CONTROLLERS > 5
#ifndef PID5_DELTA
#error "The PID5 error is not defined"
#endif
#ifndef PID5_OUTPUT
#error "The PID5 output is not defined"
#endif
#ifndef PID5_STOP
#error "The PID5 stop is not defined"
#endif
#ifndef PID5_FREQ_DIV
#define PID5_FREQ_DIV 1
#elif (PID5_FREQ_DIV < 1 || PID5_FREQ_DIV > PID_SAMP_FREQ)
#error "The PID5 sampling frequency devider value must be between 1 and MAX SAMPLE RATE = 1000/log2(Total PID"s)"
#endif
#endif
#if PID_CONTROLLERS > 6
#ifndef PID6_DELTA
#error "The PID6 error is not defined"
#endif
#ifndef PID6_OUTPUT
#error "The PID6 output is not defined"
#endif
#ifndef PID6_STOP
#error "The PID6 stop is not defined"
#endif
#ifndef PID6_FREQ_DIV
#define PID6_FREQ_DIV 1
#elif (PID6_FREQ_DIV < 1 || PID6_FREQ_DIV > PID_SAMP_FREQ)
#error "The PID6 sampling frequency devider value must be between 1 and MAX SAMPLE RATE = 1000/log2(Total PID"s)"
#endif
#endif
#if PID_CONTROLLERS > 7
#ifndef PID7_DELTA
#error "The PID7 error is not defined"
#endif
#ifndef PID7_OUTPUT
#error "The PID7 output is not defined"
#endif
#ifndef PID7_STOP
#error "The PID7 stop is not defined"
#endif
#ifndef PID7_FREQ_DIV
#define PID7_FREQ_DIV 1
#elif (PID7_FREQ_DIV < 1 || PID7_FREQ_DIV > PID_SAMP_FREQ)
#error "The PID7 sampling frequency devider value must be between 1 and MAX SAMPLE RATE = 1000/log2(Total PID"s)"
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
