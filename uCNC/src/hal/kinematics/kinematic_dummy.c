/*
		Name: kinematic_dummy.c
		Description: Custom kinematics definitions for a dummy kinematics
	
		Copyright: Copyright (c) João Martins
		Author: João Martins
		Date: 01-03-2025

		µCNC is free software: you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation, either version 3 of the License, or
		(at your option) any later version. Please see <http://www.gnu.org/licenses/>

		µCNC is distributed WITHOUT ANY WARRANTY;
		Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
		See the	GNU General Public License for more details.
*/

#include "../../cnc.h"

#if (KINEMATIC == KINEMATIC_DUMMY)

void kinematics_init(void)
{
}

void kinematics_apply_inverse(float *axis, int32_t *steps)
{
}

void kinematics_apply_forward(int32_t *steps, float *axis)
{
}

uint8_t kinematics_home(void)
{
	return STATUS_OK;
}

bool kinematics_check_boundaries(float *axis)
{
	return true;
}

#endif
