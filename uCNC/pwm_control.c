/*
	Name: 
	Copyright: 
	Author: 
	Date: 20/12/19 15:21
	Description: 
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

