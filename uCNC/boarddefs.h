#ifndef BOARDDEFS_H
#define BOARDDEFS_H

#include "config.h"
#include "board.h"

#ifndef BOARD
#error Undefined board
#endif

//check if valid board was selected
#if (BOARD == BOARD_UNO)
#define F_CPU 16000000UL
#define F_PULSE 30000
#define F_INTEGRATOR 600
#elif (BOARD == BOARD_VIRTUAL)
#define F_PULSE 30000
#define F_INTEGRATOR 600
#else
#error Board not implemented
#endif

#endif
