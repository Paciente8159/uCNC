#ifndef BOARDDEFS_H
#define BOARDDEFS_H

#ifndef BOARD
#error Undefined board
#endif

//check if valid board was selected
#if (BOARD == BOARD_UNO)
#include <avr/pgmspace.h>
#define F_CPU 16000000UL
#define F_PULSE 30000
#define F_INTEGRATOR 100
#elif (BOARD == BOARD_VIRTUAL)
#define F_PULSE 30000
#define F_INTEGRATOR 100
#else
#error Board not implemented
#endif

#include "board.h"

#endif
