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
#include "softi2c.h"
#include "softspi.h"

#if ENCODERS > 0

static int32_t encoders_pos[ENCODERS];

#if ENCODERS > 0
#if ENC0_TYPE == ENC_TYPE_I2C
SOFTI2C(enc0, ENC0_FREQ, ENC0_PULSE, ENC0_DIR);
#elif ENC0_TYPE == ENC_TYPE_SSI
SOFTSPI(enc0, ENC0_FREQ, 0, UNDEF_PIN, ENC0_DIR, ENC0_PULSE);
#endif
#endif
#if ENCODERS > 1
#if ENC1_TYPE == ENC_TYPE_I2C
SOFTI2C(enc1, ENC1_FREQ, ENC1_PULSE, ENC1_DIR);
#elif ENC1_TYPE == ENC_TYPE_SSI
SOFTSPI(enc1, ENC1_FREQ, 0, UNDEF_PIN, ENC1_DIR, ENC1_PULSE);
#endif
#endif
#if ENCODERS > 2
#if ENC2_TYPE == ENC_TYPE_I2C
SOFTI2C(enc2, ENC2_FREQ, ENC2_PULSE, ENC2_DIR);
#elif ENC2_TYPE == ENC_TYPE_SSI
SOFTSPI(enc2, ENC2_FREQ, 0, UNDEF_PIN, ENC2_DIR, ENC2_PULSE);
#endif
#endif
#if ENCODERS > 3
#if ENC3_TYPE == ENC_TYPE_I2C
SOFTI2C(enc3, ENC3_FREQ, ENC3_PULSE, ENC3_DIR);
#elif ENC3_TYPE == ENC_TYPE_SSI
SOFTSPI(enc3, ENC3_FREQ, 0, UNDEF_PIN, ENC3_DIR, ENC3_PULSE);
#endif
#endif
#if ENCODERS > 4
#if ENC4_TYPE == ENC_TYPE_I2C
SOFTI2C(enc4, ENC4_FREQ, ENC4_PULSE, ENC4_DIR);
#elif ENC4_TYPE == ENC_TYPE_SSI
SOFTSPI(enc4, ENC4_FREQ, 0, UNDEF_PIN, ENC4_DIR, ENC4_PULSE);
#endif
#endif
#if ENCODERS > 5
#if ENC5_TYPE == ENC_TYPE_I2C
SOFTI2C(enc5, ENC5_FREQ, ENC5_PULSE, ENC5_DIR);
#elif ENC5_TYPE == ENC_TYPE_SSI
SOFTSPI(enc5, ENC5_FREQ, 0, UNDEF_PIN, ENC5_DIR, ENC5_PULSE);
#endif
#endif
#if ENCODERS > 6
#if ENC6_TYPE == ENC_TYPE_I2C
SOFTI2C(enc6, ENC6_FREQ, ENC6_PULSE, ENC6_DIR);
#elif ENC6_TYPE == ENC_TYPE_SSI
SOFTSPI(enc6, ENC6_FREQ, 0, UNDEF_PIN, ENC6_DIR, ENC6_PULSE);
#endif
#endif
#if ENCODERS > 7
#if ENC7_TYPE == ENC_TYPE_I2C
SOFTI2C(enc7, ENC7_FREQ, ENC7_PULSE, ENC7_DIR);
#elif ENC7_TYPE == ENC_TYPE_SSI
SOFTSPI(enc7, ENC7_FREQ, 0, UNDEF_PIN, ENC7_DIR, ENC7_PULSE);
#endif
#endif

/**
 * Additional read functions for other types of encoders can be added later
 * For now support for the MT6701 is added
 */
static uint16_t encoder_last_read[ENCODERS];
uint16_t read_encoder_mt6701_i2c(softi2c_port_t *port)
{
	uint8_t reg = 0x03;
	uint8_t data[2] = {0};
	uint16_t res = 0;
	if (softi2c_send(port, 0x06, &reg, 1, false, 10) == I2C_OK)
	{
		softi2c_receive(port, 0x06, data, 2, 10);
		res = ((uint16_t)data[0]) << 6;
		res |= (data[1] >> 2);
	}

	return res;
}

uint16_t read_encoder_mt6701_ssi(softspi_port_t *port)
{
	uint32_t data = 0;
	softspi_start(port);
	softspi_bulk_xmit(port, (uint8_t *)&data, (uint8_t *)&data, 3);
	softspi_stop(port);
	return (uint16_t)((data >> 10) & 0x3fff);
}

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

	__ATOMIC__
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

/**
 * Updates pulse encoder types
 */
#if ENCODERS_MASK

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

#endif

#if defined(ENC0_READ) || defined(ENC1_READ) || defined(ENC2_READ) || defined(ENC3_READ) || defined(ENC4_READ) || defined(ENC5_READ) || defined(ENC6_READ) || defined(ENC7_READ)
// static uint16_t encoder_last_read[ENCODERS];
static int32_t encoder_get_diff_read(uint8_t i)
{
	int32_t encoder_read = 0;
	int32_t diff = 0;
	switch (i)
	{
#ifdef ENC0_READ // enc0 uses communication
	case ENC0:
		encoder_read = ENC0_READ;
		diff = (!(g_settings.encoders_dir_invert_mask & (1 << ENC0))) ? (encoder_read - encoder_last_read[ENC0]) : (encoder_last_read[ENC0] - encoder_read);
		encoder_last_read[ENC0] = encoder_read;
		if (diff < -(ENC0_PPR >> 1))
		{
			return (diff + ENC0_PPR);
		}
		if (diff > (ENC0_PPR >> 1))
		{
			return (diff - ENC0_PPR);
		}
		return diff;
#endif
#ifdef ENC1_READ // enc1 uses communication
	case ENC1:
		encoder_read = ENC1_READ;
		diff = (!(g_settings.encoders_dir_invert_mask & (1 << ENC1))) ? (encoder_read - encoder_last_read[ENC1]) : (encoder_last_read[ENC1] - encoder_read);
		encoder_last_read[ENC1] = encoder_read;
		if (diff < -(ENC1_PPR >> 1))
		{
			return (diff + ENC1_PPR);
		}
		if (diff > (ENC1_PPR >> 1))
		{
			return (diff + ENC1_PPR);
		}
		return diff;
#endif
#ifdef ENC2_READ // enc2 uses communication
	case ENC2:
		encoder_read = ENC2_READ;
		diff = (!(g_settings.encoders_dir_invert_mask & (1 << ENC2))) ? (encoder_read - encoder_last_read[ENC2]) : (encoder_last_read[ENC2] - encoder_read);
		encoder_last_read[ENC2] = encoder_read;
		if (diff < -(ENC2_PPR >> 1))
		{
			return (diff + ENC2_PPR);
		}
		if (diff > (ENC2_PPR >> 1))
		{
			return (diff + ENC2_PPR);
		}
		return diff;
#endif
#ifdef ENC3_READ // enc3 uses communication
	case ENC3:
		encoder_read = ENC3_READ;
		diff = (!(g_settings.encoders_dir_invert_mask & (1 << ENC3))) ? (encoder_read - encoder_last_read[ENC3]) : (encoder_last_read[ENC3] - encoder_read);
		encoder_last_read[ENC3] = encoder_read;
		if (diff < -(ENC3_PPR >> 1))
		{
			return (diff + ENC3_PPR);
		}
		if (diff > (ENC3_PPR >> 1))
		{
			return (diff + ENC3_PPR);
		}
		return diff;
#endif
#ifdef ENC4_READ // enc4 uses communication
	case ENC4:
		encoder_read = ENC4_READ;
		diff = (!(g_settings.encoders_dir_invert_mask & (1 << ENC4))) ? (encoder_read - encoder_last_read[ENC4]) : (encoder_last_read[ENC4] - encoder_read);
		encoder_last_read[ENC4] = encoder_read;
		if (diff < -(ENC4_PPR >> 1))
		{
			return (diff + ENC4_PPR);
		}
		if (diff > (ENC4_PPR >> 1))
		{
			return (diff + ENC4_PPR);
		}
		return diff;
#endif
#ifdef ENC5_READ // enc5 uses communication
	case ENC5:
		encoder_read = ENC5_READ;
		diff = (!(g_settings.encoders_dir_invert_mask & (1 << ENC5))) ? (encoder_read - encoder_last_read[ENC5]) : (encoder_last_read[ENC5] - encoder_read);
		encoder_last_read[ENC5] = encoder_read;
		if (diff < -(ENC5_PPR >> 1))
		{
			return (diff + ENC5_PPR);
		}
		if (diff > (ENC5_PPR >> 1))
		{
			return (diff + ENC5_PPR);
		}
		return diff;
#endif
#ifdef ENC6_READ // enc6 uses communication
	case ENC6:
		encoder_read = ENC6_READ;
		diff = (!(g_settings.encoders_dir_invert_mask & (1 << ENC6))) ? (encoder_read - encoder_last_read[ENC6]) : (encoder_last_read[ENC6] - encoder_read);
		encoder_last_read[ENC6] = encoder_read;
		if (diff < -(ENC6_PPR >> 1))
		{
			return (diff + ENC6_PPR);
		}
		if (diff > (ENC6_PPR >> 1))
		{
			return (diff + ENC6_PPR);
		}
		return diff;
#endif
#ifdef ENC7_READ // enc7 uses communication
	case ENC7:
		encoder_read = ENC7_READ;
		diff = (!(g_settings.encoders_dir_invert_mask & (1 << ENC7))) ? (encoder_read - encoder_last_read[ENC7]) : (encoder_last_read[ENC7] - encoder_read);
		encoder_last_read[ENC7] = encoder_read;
		if (diff < -(ENC7_PPR >> 1))
		{
			return (diff + ENC7_PPR);
		}
		if (diff > (ENC7_PPR >> 1))
		{
			return (diff + ENC7_PPR);
		}
		return diff;
#endif
	}

	return 0;
}
#endif

void encoders_dotasks(void)
{
#ifdef ENC0_READ
	encoders_pos[0] += encoder_get_diff_read(0);
#endif
#ifdef ENC1_READ
	encoders_pos[1] += encoder_get_diff_read(1);
#endif
#ifdef ENC2_READ
	encoders_pos[2] += encoder_get_diff_read(2);
#endif
#ifdef ENC3_READ
	encoders_pos[3] += encoder_get_diff_read(3);
#endif
#ifdef ENC4_READ
	encoders_pos[4] += encoder_get_diff_read(4);
#endif
#ifdef ENC5_READ
	encoders_pos[5] += encoder_get_diff_read(5);
#endif
#ifdef ENC6_READ
	encoders_pos[6] += encoder_get_diff_read(6);
#endif
#ifdef ENC7_READ
	encoders_pos[7] += encoder_get_diff_read(7);
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
	proto_printf("[EC:%" STRGIFY(ENCODERS) "lld" MSG_FEEDBACK_END, encoders_pos);
}

void encoder_reset_position(uint8_t i, int32_t position)
{
	__ATOMIC__
	{
// reads the position to throw away last diff read
#if defined(ENC0_READ) || defined(ENC1_READ) || defined(ENC2_READ) || defined(ENC3_READ) || defined(ENC4_READ) || defined(ENC5_READ) || defined(ENC6_READ) || defined(ENC7_READ)
		encoder_get_diff_read(i);
#endif
		encoders_pos[i] = position;
	}
}

void encoders_reset_position(void)
{
	for (uint8_t i = 0; i < ENCODERS; i++)
	{
		if (!((1UL << i) && STEPPERS_ENCODERS_MASK))
		{
			encoder_reset_position(i, 0);
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
#if ENC0_TYPE == ENC_TYPE_I2C
	softi2c_config(&enc0, ENC0_FREQ);
#endif
#if ENC1_TYPE == ENC_TYPE_I2C
	softi2c_config(&enc1, ENC1_FREQ);
#endif
#if ENC2_TYPE == ENC_TYPE_I2C
	softi2c_config(&enc2, ENC2_FREQ);
#endif
#if ENC3_TYPE == ENC_TYPE_I2C
	softi2c_config(&enc3, ENC3_FREQ);
#endif
#if ENC4_TYPE == ENC_TYPE_I2C
	softi2c_config(&enc4, ENC4_FREQ);
#endif
#if ENC5_TYPE == ENC_TYPE_I2C
	softi2c_config(&enc5, ENC5_FREQ);
#endif
#if ENC6_TYPE == ENC_TYPE_I2C
	softi2c_config(&enc6, ENC6_FREQ);
#endif
#if ENC7_TYPE == ENC_TYPE_I2C
	softi2c_config(&enc7, ENC7_FREQ);
#endif
	encoders_reset_position();
}

#endif
