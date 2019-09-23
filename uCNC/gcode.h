#ifndef GCODE_H
#define GCODE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define GCODE_GROUP_MOTION      1
#define GCODE_GROUP_PLANE       2
#define GCODE_GROUP_DISTANCE    4
#define GCODE_GROUP_FEEDRATE    8
#define GCODE_GROUP_UNITS       16
#define GCODE_GROUP_CUTTERRAD   32
#define GCODE_GROUP_TOOLLENGTH  64
#define GCODE_GROUP_RETURNMODE  128
#define GCODE_GROUP_COORDSYS    1
#define GCODE_GROUP_PATH        2
#define GCODE_GROUP_STOPPING    4
#define GCODE_GROUP_TOOLCHANGE  8
#define GCODE_GROUP_SPINDLE     16
#define GCODE_GROUP_COOLANT     32
#define GCODE_GROUP_ENABLEOVER  64
#define GCODE_GROUP_NONMODAL    128

#define GCODE_WORD_X    1
#define GCODE_WORD_Y    2
#define GCODE_WORD_Z    4
#define GCODE_WORD_A    8
#define GCODE_WORD_B    16
#define GCODE_WORD_C    32
#define GCODE_WORD_D    64
#define GCODE_WORD_F    128
#define GCODE_WORD_H    1
#define GCODE_WORD_I    2
#define GCODE_WORD_J    4
#define GCODE_WORD_K    8
#define GCODE_WORD_L    16
#define GCODE_WORD_P    32
#define GCODE_WORD_Q    64
#define GCODE_WORD_R    128
#define GCODE_WORD_S    1
#define GCODE_WORD_T    2
#define GCODE_WORD_N    4

#define GCODE_PARSER_BUFFER_SIZE 255

typedef struct
{
    //group1
    uint8_t motion : 4;
    uint8_t plane : 2;
    uint8_t distance_mode : 1;
    uint8_t feedrate_mode : 1;

    uint8_t units : 1;
    int8_t cutter_radius_compensation : 2;
    uint8_t tool_length_offset : 1;
    uint8_t return_mode : 1;
    uint8_t coord_system : 3;

    uint8_t path_mode : 2;
    uint8_t stopping : 3;
    uint8_t tool_change : 1;
    uint8_t spindle_turning : 2;

    uint8_t coolant : 2;
    uint8_t feed_speed_override : 1;
    uint8_t nonmodal : 4; //reset to 0 in every line (non permanent)
    uint8_t : 1; //unused
} GCODE_GROUPS;

typedef struct
{
    float xyzabc[6];
    float d;
    float f;
    float h;
    float ijk[3];
    float l;
    float p;
    float q;
    float r;
    float s;
    float t;
} GCODE_WORDS;

typedef struct
{
    uint32_t linenum;
    GCODE_GROUPS groups;
    GCODE_WORDS words;
} GCODE_PARSER_STATE;

void gcode_init();
void gcode_parse_nextline();

#endif
