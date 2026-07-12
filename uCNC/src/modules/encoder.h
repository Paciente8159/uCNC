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

#include "../module.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * Encoders definitions
 */
#define ENC0 0
#define ENC1 1
#define ENC2 2
#define ENC3 3
#define ENC4 4
#define ENC5 5
#define ENC6 6
#define ENC7 7
/**
 * Encoders basic types
 */
#define ENC_TYPE_PULSE 0
#define ENC_TYPE_I2C 1
#define ENC_TYPE_SSI 2
#define ENC_TYPE_CUSTOM 255

	DECL_MODULE(encoder);
	DECL_HOOK(enc0_index, void);
	DECL_HOOK(enc1_index, void);
	DECL_HOOK(enc2_index, void);
	DECL_HOOK(enc3_index, void);
	DECL_HOOK(enc4_index, void);
	DECL_HOOK(enc5_index, void);
	DECL_HOOK(enc6_index, void);
	DECL_HOOK(enc7_index, void);

	void enc0_pulse(void);
	void enc1_pulse(void);
	void enc2_pulse(void);
	void enc3_pulse(void);
	void enc4_pulse(void);
	void enc5_pulse(void);
	void enc6_pulse(void);
	void enc7_pulse(void);

	int32_t encoder_get_position(uint8_t i);
	void encoder_print_values(void);
	void encoder_reset_position(uint8_t i, int32_t position);
	void encoders_reset_position(void);
	void encoders_itp_reset_rt_position(float *origin);
	void encoders_update(uint8_t pulse, uint8_t diff);
	uint32_t encoder_get_delta(uint8_t i);
	uint16_t encoder_get_rpm(uint8_t i);
	bool encoder_get_index_stats(uint8_t i, int32_t *last, int32_t *min, int32_t *max, uint32_t *count);
	bool encoder_get_index_live_delta(uint8_t i, int32_t *delta);
	bool encoder_get_index_debug_line(uint8_t i, char *line, uint32_t line_len, uint32_t *seq);
	void encoder_virtual_index_clear(uint8_t i);
	void encoder_virtual_index_update(uint8_t i);
	void encoder_record_index_reference(uint8_t i, int32_t position);
	void encoder_invoke_index(uint8_t i);
	int32_t enc_custom_read(uint8_t i);

#ifdef __cplusplus
}
#endif

#endif
