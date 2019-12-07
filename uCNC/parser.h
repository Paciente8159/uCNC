#ifndef GCODE_H
#define GCODE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "machinedefs.h"

extern float g_parser_current_pos[AXIS_COUNT];

void parser_init();
bool parser_is_ready();
uint8_t parser_parse_command();
void parser_print_states();

#endif
