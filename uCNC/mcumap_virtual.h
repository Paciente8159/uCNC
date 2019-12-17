#ifndef MCUMAP_VIRTUAL_H
#define MCUMAP_VIRTUAL_H

#include <stdint.h>
#include "util\timer.h"
#define F_CPU 1000000
#define F_PULSE_MAX 30000
#define F_PULSE_MIN 4
#define F_INTEGRATOR 50

typedef struct virtual_map_t virtual_map_t;

typedef virtual_map_t* virtports_t;
extern virtports_t virtualports;

//joints step/dir pins
#define STEP0 0
#define STEP1 1
#define STEP2 2
#define STEPS_OUTREG virtualports->steps

#define DIR0 0
#define DIR1 1
#define DIR2 2
#define DIRS_OUTREG virtualports->dirs

//critical inputs
#define ESTOP 0
#define FHOLD 1
#define CS_RES 2
#define CONTROLS_INREG virtualports->controls

#define LIMIT_X 0
#define LIMIT_Y 1
#define LIMIT_Z 2
#define LIMITS_INREG virtualports->limits

#define PROBE 0
#define PROBE_INREG virtualports->probe

#endif
