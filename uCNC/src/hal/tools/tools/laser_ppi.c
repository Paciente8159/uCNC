/*
	Name: laser_ppi.c
	Description: Defines a laser PPI tool using PWM0 pin for µCNC.
				 Defines a coolant output using DOUT2 and DOUT3 (can be used for air assist).

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 05/10/2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"

#include <stdbool.h>
#include <float.h>
#include <math.h>

// #define ENABLE_COOLANT
#ifdef ENABLE_COOLANT
#ifndef COOLANT_FLOOD
#define COOLANT_FLOOD DOUT2
#endif
#ifndef COOLANT_MIST
#define COOLANT_MIST DOUT3
#endif
#endif

#ifdef ENABLE_LASER_PPI

static void laser_ppi_startup_code(void)
{
// force laser mode
#if !(LASER_PPI < 0)
	mcu_config_output(LASER_PPI);
#ifndef INVERT_LASER_PPI_LOGIC
	mcu_clear_output(LASER_PPI);
#else
	mcu_set_output(LASER_PPI);
#endif
#endif
	g_settings.laser_mode |= LASER_PPI_MODE;
	parser_config_ppi();
}

static void laser_ppi_shutdown_code(void)
{
#if !(LASER_PPI < 0)
#ifndef INVERT_LASER_PPI_LOGIC
	mcu_clear_output(LASER_PPI);
#else
	mcu_set_output(LASER_PPI);
#endif
#endif
	// restore laser mode
	g_settings.laser_mode &= ~(LASER_PPI_MODE | LASER_PPI_VARPOWER_MODE);
	parser_config_ppi();
}

static void laser_ppi_set_coolant(uint8_t value)
{
// easy macro
#ifdef ENABLE_COOLANT
	SET_COOLANT(COOLANT_FLOOD, COOLANT_MIST, value);
#endif
}

static uint16_t laser_ppi_get_speed(void)
{
	return g_settings.step_per_mm[STEPPER_COUNT - 1];
}

const tool_t __rom__ laser_ppi = {
	.startup_code = &laser_ppi_startup_code,
	.shutdown_code = &laser_ppi_shutdown_code,
#if PID_CONTROLLERS > 0
	.pid_update = NULL,
	.pid_error = NULL,
#endif
	.range_speed = NULL,
	.get_speed = &laser_ppi_get_speed,
	.set_speed = NULL,
	.set_coolant = &laser_ppi_set_coolant};

#endif
