#ifndef CPUDEFS_H
#define CPUDEFS_H

#include "config.h"

#ifndef BOARD
#error Undefined board
#endif

//define board
#if (BOARD == BOARD_UNO)
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "pins_uno.h"
#elif (BOARD == BOARD_VIRTUAL)
#include "pins_virtual.h"
#else
#error Board not implemented
#endif

#endif
