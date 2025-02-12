/*
	Name: shift_register.h
	Description: This module adds the ability to control the IC74HC595(outputs) and IC74HC165(inputs) shift register controllers (used for example in the MKS-DLC32 board) to µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 12-02-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef SHIFT_REGISTER_H
#define SHIFT_REGISTER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "ic74hc595.h"
#include "ic74hc165.h"
#include "../module.h"

void shift_register_io_pins(void);

#ifdef __cplusplus
}
#endif

#endif