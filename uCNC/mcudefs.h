#ifndef MCUSDEFS_H
#define MCUSDEFS_H

#include "config.h"
#include "mcus.h"

#if(MCU == MCU_ATMEGA328P)
#include "mcumap_atmega328p.h"
#endif

#if(MCU == MCU_VIRTUAL)
#include "mcumap_virtual.h"
#endif

#ifndef MCU
#error Undefined mcu
#endif

#endif
