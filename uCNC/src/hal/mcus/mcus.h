/*
	Name: mcus.h
	Description: Defines the available mcu types.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 11/11/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef MCUS_H
#define MCUS_H

#ifdef __cplusplus
extern "C"
{
#endif

#define MCU_NONE 0
#define MCU_AVR 1
#define MCU_STM32F1X 10
#define MCU_STM32F4X 11
#define MCU_SAMD21 20
#define MCU_LPC176X 30
#define MCU_ESP8266 40
#define MCU_ESP32 50
#define MCU_VIRTUAL_WIN 99

#ifdef __cplusplus
}
#endif

#endif
