#ifndef BOARDDEFS_H
#define BOARDDEFS_H

#include "config.h"
#include "board.h"
#ifndef BOARD
#error Undefined board
#endif

//define board
#if (BOARD == BOARD_UNO)
#include "board_uno.h"
#elif (BOARD == BOARD_VIRTUAL)
#include "board_virtual.h"
#else
#error Board not implemented
#endif

#endif
