/*
	Name: encoder.c
	Description: Encoder module for µCNC.

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

#include "../cnc.h"

#if ENCODERS > 0

static int32_t encoders_pos[ENCODERS];

#ifdef ENABLE_ENCODER_RPM

#ifndef ENCODER_RPM_MIN
#define ENCODER_RPM_MIN 4
#endif
#ifndef RPM_PPR
#define RPM_PPR 4
#endif

#define MAX_RPM_PULSE_INTERVAL (1000000UL / (ENCODER_RPM_MIN * RPM_PPR))
#define RPM_CONV_CONSTANT (60000000.f / (float)RPM_PPR)

static volatile uint32_t prev_time;
static volatile uint32_t current_time;
bool encoder_rpm_updated;
CREATE_HOOK(encoder_index);

uint16_t encoder_get_rpm(void)
{
	uint32_t elapsed, prev;

	ATOMIC_CODEBLOCK
	{
		elapsed = current_time;
		prev = prev_time;
		encoder_rpm_updated = false;
	}

	if (ABS(mcu_micros() - elapsed) > MAX_RPM_PULSE_INTERVAL)
	{
		return 0;
	}

	elapsed -= prev;
	float spindle = RPM_CONV_CONSTANT / (float)ABS(elapsed);
	return (uint16_t)lroundf(spindle);
}

#endif

static FORCEINLINE uint8_t encoder_read_dirs(void)
{
	uint8_t value = 0;
#if ENCODERS > 0
	value |= ((io_get_input(ENC0_DIR)) ? ENC0_MASK : 0);
#endif
#if ENCODERS > 1
	value |= ((io_get_input(ENC1_DIR)) ? ENC1_MASK : 0);
#endif
#if ENCODERS > 2
	value |= ((io_get_input(ENC2_DIR)) ? ENC2_MASK : 0);
#endif
#if ENCODERS > 3
	value |= ((io_get_input(ENC3_DIR)) ? ENC3_MASK : 0);
#endif
#if ENCODERS > 4
	value |= ((io_get_input(ENC4_DIR)) ? ENC4_MASK : 0);
#endif
#if ENCODERS > 5
	value |= ((io_get_input(ENC5_DIR)) ? ENC5_MASK : 0);
#endif
#if ENCODERS > 6
	value |= ((io_get_input(ENC6_DIR)) ? ENC6_MASK : 0);
#endif
#if ENCODERS > 7
	value |= ((io_get_input(ENC7_DIR)) ? ENC7_MASK : 0);
#endif
	return value ^ g_settings.encoders_dir_invert_mask;
}

void encoders_update(uint8_t pulse, uint8_t diff)
{
	uint8_t dir = encoder_read_dirs();

	// leave only those active
	diff &= pulse;

// checks if pulse pin changed state and is logical 1
#if ENCODERS > 0
	if ((diff & ENC0_MASK))
	{
		encoders_pos[0] += (dir & ENC0_MASK) ? 1 : -1;
	}
#endif
#if ENCODERS > 1
	if ((diff & ENC1_MASK))
	{
		encoders_pos[1] += (dir & ENC1_MASK) ? 1 : -1;
	}
#endif
#if ENCODERS > 2
	if ((diff & ENC2_MASK))
	{
		encoders_pos[2] += (dir & ENC2_MASK) ? 1 : -1;
	}
#endif
#if ENCODERS > 3
	if ((diff & ENC3_MASK))
	{
		encoders_pos[3] += (dir & ENC3_MASK) ? 1 : -1;
	}
#endif
#if ENCODERS > 4
	if ((diff & ENC4_MASK))
	{
		encoders_pos[4] += (dir & ENC4_MASK) ? 1 : -1;
	}
#endif
#if ENCODERS > 5
	if ((diff & ENC5_MASK))
	{
		encoders_pos[5] += (dir & ENC5_MASK) ? 1 : -1;
	}
#endif
#if ENCODERS > 6
	if ((diff & ENC6_MASK))
	{
		encoders_pos[6] += (dir & ENC6_MASK) ? 1 : -1;
	}
#endif
#if ENCODERS > 7
	if ((diff & ENC7_MASK))
	{
		encoders_pos[7] += (dir & ENC7_MASK) ? 1 : -1;
	}
#endif

#ifdef ENABLE_ENCODER_RPM
	if ((diff & RPM_ENCODER_MASK))
	{
		encoder_rpm_updated = true;
		uint32_t time = mcu_micros();
		prev_time = current_time;
		current_time = time;
#ifdef RPM_INDEX_INPUT
		if (io_get_input(RPM_INDEX_INPUT))
#else
		if (encoders_pos[RPM_ENCODER] >= RPM_PPR)
#endif
		{
			encoders_pos[RPM_ENCODER] = 0;
			HOOK_INVOKE(encoder_index);
		}
	}
#endif
}

int32_t encoder_get_position(uint8_t i)
{
	ATOMIC_CODEBLOCK
	{
		return encoders_pos[i];
	}

	return 0;
}

void encoder_print_values(void)
{
	proto_printf("[EC:%"STRGIFY(ENCODERS)"lld"MSG_FEEDBACK_END, encoders_pos);
}

void encoder_reset_position(uint8_t i, int32_t position)
{
	ATOMIC_CODEBLOCK
	{
		encoders_pos[i] = position;
	}
}

void encoders_reset_position(void)
{
	ATOMIC_CODEBLOCK
	{
		for (uint8_t i = 0; i < ENCODERS; i++)
		{
			if ((~STEPPERS_ENCODERS_MASK) & (1 << i))
			{
				encoders_pos[i] = 0;
			}
		}
	}
}

void encoders_itp_reset_rt_position(float *origin)
{
#if STEPPER_COUNT > 0
#ifdef STEP0_ENCODER
	encoder_reset_position(STEP0_ENCODER, origin[0]);
#endif
#endif
#if STEPPER_COUNT > 1
#ifdef STEP1_ENCODER
	encoder_reset_position(STEP1_ENCODER, origin[1]);
#endif
#endif
#if STEPPER_COUNT > 2
#ifdef STEP2_ENCODER
	encoder_reset_position(STEP2_ENCODER, origin[2]);
#endif
#endif
#if STEPPER_COUNT > 3
#ifdef STEP3_ENCODER
	encoder_reset_position(STEP3_ENCODER, origin[3]);
#endif
#endif
#if STEPPER_COUNT > 4
#ifdef STEP4_ENCODER
	encoder_reset_position(STEP4_ENCODER, origin[4]);
#endif
#endif
#if STEPPER_COUNT > 5
#ifdef STEP5_ENCODER
	encoder_reset_position(STEP5_ENCODER, origin[5]);
#endif
#endif
}

DECL_MODULE(encoder)
{
	encoders_reset_position();
}

#endif
