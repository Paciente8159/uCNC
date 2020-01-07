/*
	Name: pwm_control.c
	Description: Controller for PWM pins.
			
	Copyright: Copyright (c) João Martins 
	Author: João Martins
	Date: 31/12/2019

	uCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	uCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "pwm_control.h"
#include "config.h"
#include "mcumap.h"
#include "cnc.h"

void pwm_run(uint8_t pwm, uint8_t value)
{
	mcu_set_pwm(pwm, value);
}

void pwm_wait_run(uint8_t pwm, uint8_t value)
{
	while(cnc_get_exec_state(EXEC_RUN));
	mcu_set_pwm(pwm, value);
}

void pwm_stop(uint8_t pwm)
{
	mcu_set_pwm(pwm, 0);
}

