/*
	Name: kinematics_cartesian_xyz.c
	Copyright: 
	Author: Jo�o Martins
	Date: 24/09/19 11:27
	Description: Implements the kinematics interface module for a XYZ cartesian machine
*/

#include "config.h"

#if(MACHINE_KINEMATICS==MACHINE_COREXY)
#include <stdio.h>
#include "mcu.h"
#include "settings.h"
#include "kinematics.h"
#include "machinedefs.h"

float step_mm_inv[3];

void kinematics_init()
{
	step_mm_inv[0] = 1.0f / (float)g_settings.step_mm[0];
	step_mm_inv[1] = 1.0f / (float)g_settings.step_mm[1];
	step_mm_inv[2] = 1.0f / (float)g_settings.step_mm[2];
}

void kinematics_apply_inverse(float* axis, uint32_t* steps)
{
	steps[0] = g_settings.step_mm[0] * (axis[AXIS_X] + axis[AXIS_Y]);
	steps[1] = g_settings.step_mm[1] * (axis[AXIS_X] - axis[AXIS_Y]);
	steps[2] = g_settings.step_mm[2] * axis[AXIS_Z];
}

void kinematics_apply_forward(uint32_t* steps, float* axis)
{
	axis[AXIS_X] = step_mm_inv[0] * 0.5f * (steps[0] + steps[1]);
	axis[AXIS_Y] = step_mm_inv[1] * 0.5f * (steps[0] - steps[1]);
	axis[AXIS_Z] = step_mm_inv[2] * steps[2];
}

#endif
