/*
	Name: io_control.h
	Description: The input control unit for µCNC.
        This is responsible to check all limit switches (both hardware and software), control switches,
        and probe.

		TODO:
			-implement generic inputs
			-implement outputs

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

#ifndef DIGITAL_IO_CONTROL_H
#define DIGITAL_IO_CONTROL_H

#include <stdbool.h>

#define ESTOP_MASK 1
#define SAFETY_DOOR_MASK 2
#define FHOLD_MASK 4
#define CS_RES_MASK 8

#define CONTROLS_MASK (ESTOP_MASK | FHOLD_MASK | CS_RES_MASK | SAFETY_DOOR_MASK)

#define LIMIT_X_MASK 1
#define LIMIT_Y_MASK 2
#define LIMIT_Z_MASK 4
#define LIMIT_A_MASK 8
#define LIMIT_B_MASK 16
#define LIMIT_C_MASK 32

#define LIMITS_MASK (LIMIT_X_MASK | LIMIT_Y_MASK | LIMIT_Z_MASK | LIMIT_A_MASK | LIMIT_B_MASK | LIMIT_C_MASK)

#define STEP0_MASK 1
#define STEP1_MASK 2
#define STEP2_MASK 4
#define STEP3_MASK 8
#define STEP4_MASK 16
#define STEP5_MASK 32

#define DIR0_MASK 1
#define DIR1_MASK 2
#define DIR2_MASK 4
#define DIR3_MASK 8
#define DIR4_MASK 16
#define DIR5_MASK 32

//ISR
void io_limits_isr();
void io_controls_isr();
void io_probe_isr();

//inputs
bool io_check_boundaries(float* axis);
uint8_t io_get_limits();
uint8_t io_get_controls();
void io_enable_probe();
void io_disable_probe();
bool io_get_probe();

//outputs
void io_set_steps(uint8_t mask);
void io_toggle_steps(uint8_t mask);
void io_set_dirs(uint8_t mask);

void io_enable_steps();

#endif

