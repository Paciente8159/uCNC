#ifndef CONFIG_H
#define CONFIG_H

#include "boards.h"

// choose board
//#define BOARD BOARD_VIRTUAL
#define BOARD BOARD_UNO
//#define DEBUGMODE
//setup IO masks
#define STEPDIR_INVERT_MASK 0x0000
#define OUTPUT_INVERT_MASK 0x0000

#define CRITICAL_INVERT_MASK 0x00
#define INPUT_INVERT_MASK 0x0000

//#define USE_PULLUPS
#ifdef USE_PULLUPS
	#define CRITICAL_PULLUP_MASK 0x00
	#define INPUT_PULLUP_MASK 0x0000
#endif

#define COM_BUFFER_SIZE 10

#define TOTAL_STEPPERS 5
#define MIN_PULSE_WIDTH_US 5

#endif
