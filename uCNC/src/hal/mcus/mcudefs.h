/*
	Name: mcudefs.h
	Description: Defines the available machine types.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 01/11/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef MCUSDEFS_H
#define MCUSDEFS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "mcus.h"

/*
	MCU port map
*/
#if (MCU == MCU_AVR)
#include "avr/mcumap_avr.h"
#endif

#if (MCU == MCU_STM32F1X)
#include "stm32f1x/mcumap_stm32f1x.h"
#endif

#if (MCU == MCU_STM32F4X)
#include "stm32f4x/mcumap_stm32f4x.h"
#endif

#if (MCU == MCU_SAMD21)
#include "samd21/mcumap_samd21.h"
#endif

#if (MCU == MCU_LPC176X)
#include "lpc176x/mcumap_lpc176x.h"
#endif

#if (MCU == MCU_ESP8266)
#include "esp8266/mcumap_esp8266.h"
#endif

#if (MCU == MCU_ESP32)
#include "esp32/mcumap_esp32.h"
#endif

#if (MCU == MCU_VIRTUAL_WIN)
#include "virtual/mcumap_virtual.h"
#endif

#ifndef MCU
#error Undefined mcu
#endif

#include "mcu.h" //exposes the MCU HAL interface

#ifdef __cplusplus
}
#endif

#endif
