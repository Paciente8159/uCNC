#ifndef MCUSDEFS_H
#define MCUSDEFS_H

#include "config.h"
#include "mcus.h"

#if(MCU == MCU_ATMEGA328P)
#include <avr/pgmspace.h>
#include "mcumap_atmega328p.h"
#define F_PULSE_MAX 30000
#define F_PULSE_MIN 1
#define F_INTEGRATOR 100 //integrator calculates 10ms slices
#endif

#if(MCU == MCU_VIRTUAL)
#include "util\timer.h"
#include "mcumap_virtual.h"
#define F_CPU 1000000
#define F_PULSE_MAX 30000
#define F_PULSE_MIN 4
#define F_INTEGRATOR 50
#endif

#ifndef MCU
#error Undefined mcu
#endif

#endif
