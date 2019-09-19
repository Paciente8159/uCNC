#ifndef MOTION_H
#define MOTION_H

#include <stdlib.h>
#include <stdint.h>

void motion_home();
void motion_rt_linear(uint16_t *steps, uint16_t *totalsteps, uint16_t initial_speed, int16_t accel);
void motion_rt_buffer_dequeue();

void motion_rt_buffer_enqueue();

uint8_t motion_rt_buffer_empty();


uint8_t motion_rt_buffer_full();

#endif
