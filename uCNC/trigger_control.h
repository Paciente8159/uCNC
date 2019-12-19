#ifndef TRIGGER_CONTROL_H
#define TRIGGER_CONTROL_H

#include <stdbool.h>

void tc_init();
//void tc_home();
void tc_limits_isr(uint8_t limits);
void tc_controls_isr(uint8_t controls);
bool tc_check_boundaries(float* axis);
uint8_t tc_get_limits(uint8_t limitmask);
uint8_t tc_get_controls(uint8_t controlmask);
bool tc_get_probe();

#endif

