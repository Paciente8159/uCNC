#ifndef GCODE_H
#define GCODE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "machinedefs.h"

extern float g_parser_current_pos[AXIS_COUNT];

bool parser_init();
void parser_reset();
bool parser_is_ready();
uint8_t parser_gcode_command();
uint8_t parser_grbl_command();
void parser_print_states();
void parser_print_coordsys();
bool parser_get_float(float *value, bool *isinteger);
void parser_get_wco(float* axis);
void parser_parameters_load();
void parser_parameters_reset();
void parser_parameters_save();

#endif
