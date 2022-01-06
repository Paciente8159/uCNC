/*
	Name: encoder.h
	Description: Encoder module for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 03-07-2021

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef ENCODER_H
#define ENCODER_H

#ifdef __cplusplus
extern "C"
{
#endif

	void encoders_update(void);
	int32_t encoder_get_position(uint8_t i);
	void encoder_reset_position(uint8_t i, int32_t position);
	void encoders_reset_position(void);

#ifdef __cplusplus
}
#endif

#endif
