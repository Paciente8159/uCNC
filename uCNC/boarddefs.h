#ifndef BOARDDEFS_H
#define BOARDDEFS_H

#include "config.h"
#include "board.h"

#ifndef BOARD
#error Undefined board
#endif

//check if valid board was selected
#if (BOARD == BOARD_UNO)

#elif (BOARD == BOARD_VIRTUAL)

#else
#error Board not implemented
#endif

#endif
