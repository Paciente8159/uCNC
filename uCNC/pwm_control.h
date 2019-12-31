/*
	Name: 
	Copyright: 
	Author: 
	Date: 20/12/19 15:21
	Description: 
*/

#ifndef PWM_CONTROL_H
#define PWM_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

void pwm_run(uint8_t pwm, uint8_t value);
void pwm_wait_run(uint8_t pwm, uint8_t value);
void pwm_stop(uint8_t pwm);

#endif
