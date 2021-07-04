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

#include "cnc.h"

#if ENCODERS > 0
#ifndef ENC0_PULSE
#error "The ENC0 pulse pin is not defined"
#endif
#ifndef ENC0_DIR
#error "The ENC0 dir pin is not defined"
#endif
#define ENC0_MASK (1 << 0)
#endif
#if ENCODERS > 1
#ifndef ENC1_PULSE
#error "The ENC1 pulse pin is not defined"
#endif
#ifndef ENC1_DIR
#error "The ENC1 dir pin is not defined"
#endif
#define ENC1_MASK (1 << 1)
#endif
#if ENCODERS > 2
#ifndef ENC2_PULSE
#error "The ENC2 pulse pin is not defined"
#endif
#ifndef ENC2_DIR
#error "The ENC2 dir pin is not defined"
#endif
#define ENC2_MASK (1 << 2)
#endif
#if ENCODERS > 3
#ifndef ENC3_PULSE
#error "The ENC3 pulse pin is not defined"
#endif
#ifndef ENC3_DIR
#error "The ENC3 dir pin is not defined"
#endif
#define ENC3_MASK (1 << 3)
#endif
#if ENCODERS > 4
#ifndef ENC4_PULSE
#error "The ENC4 pulse pin is not defined"
#endif
#ifndef ENC4_DIR
#error "The ENC4 dir pin is not defined"
#endif
#define ENC4_MASK (1 << 4)
#endif
#if ENCODERS > 5
#ifndef ENC5_PULSE
#error "The ENC5 pulse pin is not defined"
#endif
#ifndef ENC5_DIR
#error "The ENC5 dir pin is not defined"
#endif
#define ENC5_MASK (1 << 5)
#endif
#if ENCODERS > 6
#ifndef ENC6_PULSE
#error "The ENC6 pulse pin is not defined"
#endif
#ifndef ENC6_DIR
#error "The ENC6 dir pin is not defined"
#endif
#define ENC6_MASK (1 << 6)
#endif
#if ENCODERS > 7
#ifndef ENC7_PULSE
#error "The ENC7 pulse pin is not defined"
#endif
#ifndef ENC7_DIR
#error "The ENC7 dir pin is not defined"
#endif
#define ENC7_MASK (1 << 7)
#endif

	extern int32_t g_encoders_pos[ENCODERS];

	void encoders_isr(void);
	void get_encoder(uint8_t id);

#ifdef __cplusplus
}
#endif

#endif