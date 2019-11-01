/*
	Name: kinematics_cartesian_xyz.c
	Copyright: 
	Author: Jo�o Martins
	Date: 24/09/19 11:27
	Description: Implements the kinematics interface module for a XYZ cartesian machine
*/

#include "config.h"

#if(MACHINE_KINEMATICS==MACHINE_CARTESIAN_XYZ)
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
	steps[0] = g_settings.step_mm[0] * axis[0];
	steps[1] = g_settings.step_mm[1] * axis[1];
	steps[2] = g_settings.step_mm[2] * axis[2];
}

void kinematics_apply_forward(uint32_t* steps, float* axis)
{
	axis[0] = step_mm_inv[0] * steps[0];
	axis[1] = step_mm_inv[1] * steps[1];
	axis[2] = step_mm_inv[2] * steps[2];
}

#endif
