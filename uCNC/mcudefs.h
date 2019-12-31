#ifndef MCUSDEFS_H
#define MCUSDEFS_H

#include "config.h"
#include "mcus.h"

#if(MCU == MCU_ATMEGA328P)
/*
	MCU port map
*/
#include "mcumap_atmega328p.h"
/*
	MCU specific definitions and replacements
*/
#include <avr/pgmspace.h>
#define F_PULSE_MAX 30000
#define F_PULSE_MIN 1
#define __rom__ PROGMEM
#define __romstr__ PSTR
#define rom_strcpy strcpy_P
#define rom_strncpy strncpy_P
#define rom_memcpy memcpy_P
#define rom_read_byte pgm_read_byte

#endif

#if(MCU == MCU_VIRTUAL)
#include "mcumap_virtual.h"
#endif

#ifndef MCU
#error Undefined mcu
#endif

#endif
