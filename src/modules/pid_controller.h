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

#if PID_CONTROLLERS > 0
#ifndef PID0_DELTA
#error "The PID0 error is not defined"
#endif
#ifndef PID0_OUT
#error "The PID0 output is not defined"
#endif
#endif
#if PID_CONTROLLERS > 1
#ifndef PID1_DELTA
#error "The PID1 error is not defined"
#endif
#ifndef PID1_OUT
#error "The PID1 output is not defined"
#endif
#endif
#if PID_CONTROLLERS > 2
#ifndef PID2_DELTA
#error "The PID2 error is not defined"
#endif
#ifndef PID2_OUT
#error "The PID2 output is not defined"
#endif
#endif
#if PID_CONTROLLERS > 3
#ifndef PID3_DELTA
#error "The PID3 error is not defined"
#endif
#ifndef PID3_OUT
#error "The PID3 output is not defined"
#endif
#endif
#if PID_CONTROLLERS > 4
#ifndef PID4_DELTA
#error "The PID4 error is not defined"
#endif
#ifndef PID4_OUT
#error "The PID4 output is not defined"
#endif
#endif
#if PID_CONTROLLERS > 5
#ifndef PID5_DELTA
#error "The PID5 error is not defined"
#endif
#ifndef PID5_OUT
#error "The PID5 output is not defined"
#endif
#endif
#if PID_CONTROLLERS > 6
#ifndef PID6_DELTA
#error "The PID6 error is not defined"
#endif
#ifndef PID6_OUT
#error "The PID6 output is not defined"
#endif
#endif
#if PID_CONTROLLERS > 7
#ifndef PID7_DELTA
#error "The PID7 error is not defined"
#endif
#ifndef PID7_OUT
#error "The PID7 output is not defined"
#endif
#endif

#define PID_BITSHIFT_FACTOR 8

	void pid_update_isr(void);

#ifdef __cplusplus
}
#endif

#endif