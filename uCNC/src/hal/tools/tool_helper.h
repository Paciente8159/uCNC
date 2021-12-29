/*
	Name: tool_helper.h
	Description: The tool_helper set of macros for µCNC.
        This is responsible to define and manage tools.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 17/12/2021

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef TOOL_HELPER_H
#define TOOL_HELPER_H

#ifdef __cplusplus
extern "C"
{
#endif

#define COOLANT_MASK 0x02
#define MIST_MASK 0x01

    /**
 * X - PWM pin
 * Y - DIR pin
 * W - pwm value
 * Z - dir value
 * */

#define SET_SPINDLE(X, Y, W, Z) ( \
    {                              \
        io_set_output(Y, Z);       \
        io_set_pwm(X, W);          \
    })

/**
 * X - PWM pin
 * Y - pwm value
 * */
#define SET_LASER(X, Y) (      \
    {                          \
        io_set_pwm(X, Y); \
    })

/**
 * X - flood pin
 * Y - mist pin
 * z - mask value
 * */
#define SET_COOLANT(X, Y, Z) (      \
    {                                \
    io_set_output(X, (Z && COOLANT_MASK)); \
    io_set_output(Y, (Z && MIST_MASK)); \
    })

#ifdef __cplusplus
}
#endif

#endif