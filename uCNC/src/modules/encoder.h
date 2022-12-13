/*
	Name: encoder.h
	Description: An encoder module for for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 07/03/2021

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

#include <stdint.h>

#define ENC0 0
#define ENC1 1
#define ENC2 2
#define ENC3 3
#define ENC4 4
#define ENC5 5
#define ENC6 6
#define ENC7 7

	typedef void (*encoder_index_cb)(void);

	void encoder_init(void);
	int32_t encoder_get_position(uint8_t i);
	void encoder_print_values(void);
	void encoder_reset_position(uint8_t i, int32_t position);
	void encoders_reset_position(void);
	void encoders_itp_reset_rt_position(float *origin);
	void encoders_update(uint8_t pulse, uint8_t diff);
	void encoder_attach_index_cb(encoder_index_cb callback);
	void encoder_dettach_index_cb(void);
	uint16_t encoder_get_rpm(void);
	extern bool encoder_rpm_updated;

#ifdef __cplusplus
}
#endif

#endif