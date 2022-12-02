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

static uint8_t encoder_last_pulse = 0;
static int32_t encoders_pos[ENCODERS];

static FORCEINLINE uint8_t encoder_read_pulses(void)
{
	uint8_t value = 0;
#if ENCODERS > 0
	value |= ((mcu_get_input(ENC0_PULSE)) ? ENC0_MASK : 0);
#endif
#if ENCODERS > 1
	value |= ((mcu_get_input(ENC1_PULSE)) ? ENC1_MASK : 0);
#endif
#if ENCODERS > 2
	value |= ((mcu_get_input(ENC2_PULSE)) ? ENC2_MASK : 0);
#endif
#if ENCODERS > 3
	value |= ((mcu_get_input(ENC3_PULSE)) ? ENC3_MASK : 0);
#endif
#if ENCODERS > 4
	value |= ((mcu_get_input(ENC4_PULSE)) ? ENC4_MASK : 0);
#endif
#if ENCODERS > 5
	value |= ((mcu_get_input(ENC5_PULSE)) ? ENC5_MASK : 0);
#endif
#if ENCODERS > 6
	value |= ((mcu_get_input(ENC6_PULSE)) ? ENC6_MASK : 0);
#endif
#if ENCODERS > 7
	value |= ((mcu_get_input(ENC7_PULSE)) ? ENC7_MASK : 0);
#endif
	return value ^ g_settings.encoders_pulse_invert_mask;
}

static FORCEINLINE uint8_t encoder_read_dirs(void)
{
	uint8_t value = 0;
#if ENCODERS > 0
	value |= ((mcu_get_input(ENC0_DIR)) ? ENC0_MASK : 0);
#endif
#if ENCODERS > 1
	value |= ((mcu_get_input(ENC1_DIR)) ? ENC1_MASK : 0);
#endif
#if ENCODERS > 2
	value |= ((mcu_get_input(ENC2_DIR)) ? ENC2_MASK : 0);
#endif
#if ENCODERS > 3
	value |= ((mcu_get_input(ENC3_DIR)) ? ENC3_MASK : 0);
#endif
#if ENCODERS > 4
	value |= ((mcu_get_input(ENC4_DIR)) ? ENC4_MASK : 0);
#endif
#if ENCODERS > 5
	value |= ((mcu_get_input(ENC5_DIR)) ? ENC5_MASK : 0);
#endif
#if ENCODERS > 6
	value |= ((mcu_get_input(ENC6_DIR)) ? ENC6_MASK : 0);
#endif
#if ENCODERS > 7
	value |= ((mcu_get_input(ENC7_DIR)) ? ENC7_MASK : 0);
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
}

int32_t encoder_get_position(uint8_t i)
{
	__ATOMIC__
	{
		return encoders_pos[i];
	}

	return 0;
}

void encoder_print_values(void)
{
	for (uint8_t i = 0; i < ENCODERS; i++)
	{
		protocol_send_string(__romstr__("[EC:"));
		serial_print_int(encoder_get_position(i));
		protocol_send_string(MSG_END);
	}
}

void encoder_reset_position(uint8_t i, int32_t position)
{
	__ATOMIC__
	{
		encoders_pos[i] = position;
	}
}

void encoders_reset_position(void)
{
	encoder_last_pulse = encoder_read_pulses();

	__ATOMIC__
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
