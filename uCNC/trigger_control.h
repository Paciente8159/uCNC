#ifndef TRIGGER_CONTROL_H
#define TRIGGER_CONTROL_H

#include <stdbool.h>

void tc_init();
void tc_set_limit_mask(uint8_t mask);
void tc_limits_isr(uint8_t limits);
void tc_controls_isr(uint8_t controls);
bool tc_check_boundaries(float* axis);

#endif

