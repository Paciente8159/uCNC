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
#if (!ASSERT_PIN(ENC0_PULSE))
#error "The ENC0 pulse pin is not defined"
#endif
#if (!ASSERT_PIN(ENC0_DIR))
#error "The ENC0 dir pin is not defined"
#endif
#ifndef ENC0_TYPE
#define ENC0_TYPE ENC_TYPE_PULSE
#endif
#if ENC0_TYPE==ENC_TYPE_PULSE
#define ENC0_IO_MASK (1 << (ENC0_PULSE - DIN_PINS_OFFSET))
#elif ENC0_TYPE==ENC_TYPE_I2C
#ifndef ENC0_FREQ
#define ENC0_FREQ 400000
#endif
#ifndef ENC0_READ
#define ENC0_READ read_encoder_mt6701_i2c(&enc0)
#endif
#elif ENC0_TYPE==ENC_TYPE_SSI
#ifndef ENC0_FREQ
#define ENC0_FREQ 15000000
#endif
#ifndef ENC0_READ
#define ENC0_READ read_encoder_mt6701_ssi(&enc0)
#endif
#elif ENC0_TYPE==ENC_TYPE_CUSTOM
#ifndef ENC0_READ
#define ENC0_READ enc_custom_read(ENC0)
#endif
#endif
#endif
#if ENCODERS > 1
#if (!ASSERT_PIN(ENC1_PULSE))
#error "The ENC1 pulse pin is not defined"
#endif
#if (!ASSERT_PIN(ENC1_DIR))
#error "The ENC1 dir pin is not defined"
#endif
#ifndef ENC1_TYPE
#define ENC1_TYPE ENC_TYPE_PULSE
#endif
#if ENC1_TYPE==ENC_TYPE_PULSE
#define ENC1_IO_MASK (1 << (ENC1_PULSE - DIN_PINS_OFFSET))
#elif ENC1_TYPE==ENC_TYPE_I2C
#ifndef ENC1_FREQ
#define ENC1_FREQ 400000
#endif
#ifndef ENC1_READ
#define ENC1_READ read_encoder_mt6701_i2c(&enc1)
#endif
#elif ENC1_TYPE==ENC_TYPE_SSI
#ifndef ENC1_FREQ
#define ENC1_FREQ 15000000
#endif
#ifndef ENC1_READ
#define ENC1_READ read_encoder_mt6701_ssi(&enc1)
#endif
#elif ENC1_TYPE==ENC_TYPE_CUSTOM
#ifndef ENC1_READ
#define ENC1_READ enc_custom_read(ENC1)
#endif
#endif
#endif
#if ENCODERS > 2
#if (!ASSERT_PIN(ENC2_PULSE))
#error "The ENC2 pulse pin is not defined"
#endif
#if (!ASSERT_PIN(ENC2_DIR))
#error "The ENC2 dir pin is not defined"
#endif
#ifndef ENC2_TYPE
#define ENC2_TYPE ENC_TYPE_PULSE
#endif
#if ENC2_TYPE==ENC_TYPE_PULSE
#define ENC2_IO_MASK (1 << (ENC2_PULSE - DIN_PINS_OFFSET))
#elif ENC2_TYPE==ENC_TYPE_I2C
#ifndef ENC2_FREQ
#define ENC2_FREQ 400000
#endif
#ifndef ENC2_READ
#define ENC2_READ read_encoder_mt6701_i2c(&enc2)
#endif
#elif ENC2_TYPE==ENC_TYPE_SSI
#ifndef ENC2_FREQ
#define ENC2_FREQ 15000000
#endif
#ifndef ENC2_READ
#define ENC2_READ read_encoder_mt6701_ssi(&enc2)
#endif
#elif ENC2_TYPE==ENC_TYPE_CUSTOM
#ifndef ENC2_READ
#define ENC2_READ enc_custom_read(ENC2)
#endif
#endif
#endif
#if ENCODERS > 3
#if (!ASSERT_PIN(ENC3_PULSE))
#error "The ENC3 pulse pin is not defined"
#endif
#if (!ASSERT_PIN(ENC3_DIR))
#error "The ENC3 dir pin is not defined"
#endif
#ifndef ENC3_TYPE
#define ENC3_TYPE ENC_TYPE_PULSE
#endif
#if ENC3_TYPE==ENC_TYPE_PULSE
#define ENC3_IO_MASK (1 << (ENC3_PULSE - DIN_PINS_OFFSET))
#elif ENC3_TYPE==ENC_TYPE_I2C
#ifndef ENC3_FREQ
#define ENC3_FREQ 400000
#endif
#ifndef ENC3_READ
#define ENC3_READ read_encoder_mt6701_i2c(&enc3)
#endif
#elif ENC3_TYPE==ENC_TYPE_SSI
#ifndef ENC3_FREQ
#define ENC3_FREQ 15000000
#endif
#ifndef ENC3_READ
#define ENC3_READ read_encoder_mt6701_ssi(&enc3)
#endif
#elif ENC3_TYPE==ENC_TYPE_CUSTOM
#ifndef ENC3_READ
#define ENC3_READ enc_custom_read(ENC3)
#endif
#endif
#endif
#if ENCODERS > 4
#if (!ASSERT_PIN(ENC4_PULSE))
#error "The ENC4 pulse pin is not defined"
#endif
#if (!ASSERT_PIN(ENC4_DIR))
#error "The ENC4 dir pin is not defined"
#endif
#ifndef ENC4_TYPE
#define ENC4_TYPE ENC_TYPE_PULSE
#endif
#if ENC4_TYPE==ENC_TYPE_PULSE
#define ENC4_IO_MASK (1 << (ENC4_PULSE - DIN_PINS_OFFSET))
#elif ENC4_TYPE==ENC_TYPE_I2C
#ifndef ENC4_FREQ
#define ENC4_FREQ 400000
#endif
#ifndef ENC4_READ
#define ENC4_READ read_encoder_mt6701_i2c(&enc4)
#endif
#elif ENC4_TYPE==ENC_TYPE_SSI
#ifndef ENC4_FREQ
#define ENC4_FREQ 15000000
#endif
#ifndef ENC4_READ
#define ENC4_READ read_encoder_mt6701_ssi(&enc4)
#endif
#elif ENC4_TYPE==ENC_TYPE_CUSTOM
#ifndef ENC4_READ
#define ENC4_READ enc_custom_read(ENC4)
#endif
#endif
#endif
#if ENCODERS > 5
#if (!ASSERT_PIN(ENC5_PULSE))
#error "The ENC5 pulse pin is not defined"
#endif
#if (!ASSERT_PIN(ENC5_DIR))
#error "The ENC5 dir pin is not defined"
#endif
#ifndef ENC5_TYPE
#define ENC5_TYPE ENC_TYPE_PULSE
#endif
#if ENC5_TYPE==ENC_TYPE_PULSE
#define ENC5_IO_MASK (1 << (ENC5_PULSE - DIN_PINS_OFFSET))
#elif ENC5_TYPE==ENC_TYPE_I2C
#ifndef ENC5_FREQ
#define ENC5_FREQ 400000
#endif
#ifndef ENC5_READ
#define ENC5_READ read_encoder_mt6701_i2c(&enc5)
#endif
#elif ENC5_TYPE==ENC_TYPE_SSI
#ifndef ENC5_FREQ
#define ENC5_FREQ 15000000
#endif
#ifndef ENC5_READ
#define ENC5_READ read_encoder_mt6701_ssi(&enc5)
#endif
#elif ENC5_TYPE==ENC_TYPE_CUSTOM
#ifndef ENC5_READ
#define ENC5_READ enc_custom_read(ENC5)
#endif
#endif
#endif
#if ENCODERS > 6
#if (!ASSERT_PIN(ENC6_PULSE))
#error "The ENC6 pulse pin is not defined"
#endif
#if (!ASSERT_PIN(ENC6_DIR))
#error "The ENC6 dir pin is not defined"
#endif
#ifndef ENC6_TYPE
#define ENC6_TYPE ENC_TYPE_PULSE
#endif
#if ENC6_TYPE==ENC_TYPE_PULSE
#define ENC6_IO_MASK (1 << (ENC6_PULSE - DIN_PINS_OFFSET))
#elif ENC6_TYPE==ENC_TYPE_I2C
#ifndef ENC6_FREQ
#define ENC6_FREQ 400000
#endif
#ifndef ENC6_READ
#define ENC6_READ read_encoder_mt6701_i2c(&enc6)
#endif
#elif ENC6_TYPE==ENC_TYPE_SSI
#ifndef ENC6_FREQ
#define ENC6_FREQ 15000000
#endif
#ifndef ENC6_READ
#define ENC6_READ read_encoder_mt6701_ssi(&enc6)
#endif
#elif ENC6_TYPE==ENC_TYPE_CUSTOM
#ifndef ENC6_READ
#define ENC6_READ enc_custom_read(ENC6)
#endif
#endif
#endif
#if ENCODERS > 7
#if (!ASSERT_PIN(ENC7_PULSE))
#error "The ENC7 pulse pin is not defined"
#endif
#if (!ASSERT_PIN(ENC7_DIR))
#error "The ENC7 dir pin is not defined"
#endif
#ifndef ENC7_TYPE
#define ENC7_TYPE ENC_TYPE_PULSE
#endif
#if ENC7_TYPE==ENC_TYPE_PULSE
#define ENC7_IO_MASK (1 << (ENC7_PULSE - DIN_PINS_OFFSET))
#elif ENC7_TYPE==ENC_TYPE_I2C
#ifndef ENC7_FREQ
#define ENC7_FREQ 400000
#endif
#ifndef ENC7_READ
#define ENC7_READ read_encoder_mt6701_i2c(&enc7)
#endif
#elif ENC7_TYPE==ENC_TYPE_SSI
#ifndef ENC7_FREQ
#define ENC7_FREQ 15000000
#endif
#ifndef ENC7_READ
#define ENC7_READ read_encoder_mt6701_ssi(&enc7)
#endif
#elif ENC7_TYPE==ENC_TYPE_CUSTOM
#ifndef ENC7_READ
#define ENC7_READ enc_custom_read(ENC7)
#endif
#endif
#endif

#define ENCODERS_IO_MASK (ENC0_IO_MASK | ENC1_IO_MASK | ENC2_IO_MASK | ENC3_IO_MASK | ENC4_IO_MASK | ENC5_IO_MASK | ENC6_IO_MASK | ENC7_IO_MASK)

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

#ifdef ENC0_INDEX
	CREATE_HOOK(enc0_index);
#endif
#ifdef ENC1_INDEX
	CREATE_HOOK(enc1_index);
#endif
#ifdef ENC2_INDEX
	CREATE_HOOK(enc2_index);
#endif
#ifdef ENC3_INDEX
	CREATE_HOOK(enc3_index);
#endif
#ifdef ENC4_INDEX
	CREATE_HOOK(enc4_index);
#endif
#ifdef ENC5_INDEX
	CREATE_HOOK(enc5_index);
#endif
#ifdef ENC6_INDEX
	CREATE_HOOK(enc6_index);
#endif
#ifdef ENC7_INDEX
	CREATE_HOOK(enc7_index);
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

#ifndef ENCODER_MIN_RPM
#define ENCODER_MIN_RPM 6
#endif
#define ENCODER_MIN_RPM_FACTOR ((60 / ENCODER_MIN_RPM) * 1000000)

typedef struct encoder_rpm_
{
	uint16_t last_rpm;
	int32_t last_position;
	uint32_t last_timestamp;
} encoder_rpm_t;

static encoder_rpm_t encoders_rpm[ENCODERS];

uint16_t encoder_get_rpm(uint8_t i)
{
	int32_t pos = (uint32_t)encoder_get_position(i);
	uint32_t diff = (uint32_t)ABS(pos - encoders_rpm[i].last_position);
	uint32_t timestamp = mcu_micros();
	uint32_t elapsed = (uint32_t)(timestamp - encoders_rpm[i].last_timestamp);

	if (!diff) // if no motion detected
	{
		if (elapsed > 1000000) // at least one second as passed
		{
			encoders_rpm[i].last_timestamp = timestamp;
			encoders_rpm[i].last_rpm >>= 1; // assume speed as dropped to half
		}
	}
	else
	{
		if (diff >= 10 || elapsed > ENCODER_MIN_RPM_FACTOR) // at a minimum of 5RPM start to display values or a minimum of 10 pulse counts before update or at least 10 pulses to calculate RPM
		{
			float rpm = (float)diff / (float)g_settings.encoders_resolution[i];
			rpm *= 60000000.0f / (float)elapsed;
			encoders_rpm[i].last_position = pos;
			encoders_rpm[i].last_timestamp = timestamp;
			encoders_rpm[i].last_rpm = (uint16_t)lround(rpm);
		}
	}

	// none of the above conditions were matched. Return the previous RPM value
	return encoders_rpm[i].last_rpm;
}



/**
 * Updates pulse encoder types
 */
#if (ENCODERS_IO_MASK != 0)

static FORCEINLINE uint8_t encoder_read_dirs(void)
{
	uint8_t value = 0;
#if ENCODERS > 0
	value |= ((io_get_input(ENC0_DIR)) ? ENC0_IO_MASK : 0);
#endif
#if ENCODERS > 1
	value |= ((io_get_input(ENC1_DIR)) ? ENC1_IO_MASK : 0);
#endif
#if ENCODERS > 2
	value |= ((io_get_input(ENC2_DIR)) ? ENC2_IO_MASK : 0);
#endif
#if ENCODERS > 3
	value |= ((io_get_input(ENC3_DIR)) ? ENC3_IO_MASK : 0);
#endif
#if ENCODERS > 4
	value |= ((io_get_input(ENC4_DIR)) ? ENC4_IO_MASK : 0);
#endif
#if ENCODERS > 5
	value |= ((io_get_input(ENC5_DIR)) ? ENC5_IO_MASK : 0);
#endif
#if ENCODERS > 6
	value |= ((io_get_input(ENC6_DIR)) ? ENC6_IO_MASK : 0);
#endif
#if ENCODERS > 7
	value |= ((io_get_input(ENC7_DIR)) ? ENC7_IO_MASK : 0);
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
	if ((diff & ENC0_IO_MASK))
	{
		encoders_pos[0] += (dir & ENC0_IO_MASK) ? 1 : -1;
#ifdef ENC0_INDEX
		if (io_get_input(ENC0_INDEX))
		{
			HOOK_INVOKE(enc0_index);
		}
#endif
	}
#endif
#if ENCODERS > 1
	if ((diff & ENC1_IO_MASK))
	{
		encoders_pos[1] += (dir & ENC1_IO_MASK) ? 1 : -1;
#ifdef ENC1_INDEX
		if (io_get_input(ENC1_INDEX))
		{
			HOOK_INVOKE(enc1_index);
		}
#endif
	}
#endif
#if ENCODERS > 2
	if ((diff & ENC2_IO_MASK))
	{
		encoders_pos[2] += (dir & ENC2_IO_MASK) ? 1 : -1;
#ifdef ENC2_INDEX
		if (io_get_input(ENC2_INDEX))
		{
			HOOK_INVOKE(enc2_index);
		}
#endif
	}
#endif
#if ENCODERS > 3
	if ((diff & ENC3_IO_MASK))
	{
		encoders_pos[3] += (dir & ENC3_IO_MASK) ? 1 : -1;
#ifdef ENC3_INDEX
		if (io_get_input(ENC3_INDEX))
		{
			HOOK_INVOKE(enc3_index);
		}
#endif
	}
#endif
#if ENCODERS > 4
	if ((diff & ENC4_IO_MASK))
	{
		encoders_pos[4] += (dir & ENC4_IO_MASK) ? 1 : -1;
#ifdef ENC4_INDEX
		if (io_get_input(ENC4_INDEX))
		{
			HOOK_INVOKE(enc4_index);
		}
#endif
	}
#endif
#if ENCODERS > 5
	if ((diff & ENC5_IO_MASK))
	{
		encoders_pos[5] += (dir & ENC5_IO_MASK) ? 1 : -1;
#ifdef ENC5_INDEX
		if (io_get_input(ENC5_INDEX))
		{
			HOOK_INVOKE(enc5_index);
		}
#endif
	}
#endif
#if ENCODERS > 6
	if ((diff & ENC6_IO_MASK))
	{
		encoders_pos[6] += (dir & ENC6_IO_MASK) ? 1 : -1;
#ifdef ENC6_INDEX
		if (io_get_input(ENC6_INDEX))
		{
			HOOK_INVOKE(enc6_index);
		}
#endif
	}
#endif
#if ENCODERS > 7
	if ((diff & ENC7_IO_MASK))
	{
		encoders_pos[7] += (dir & ENC7_IO_MASK) ? 1 : -1;
#ifdef ENC7_INDEX
		if (io_get_input(ENC7_INDEX))
		{
			HOOK_INVOKE(enc7_index);
		}
#endif
	}
#endif
}
#else
void encoders_update(uint8_t pulse, uint8_t diff) {}
#endif

#if defined(ENC0_READ) || defined(ENC1_READ) || defined(ENC2_READ) || defined(ENC3_READ) || defined(ENC4_READ) || defined(ENC5_READ) || defined(ENC6_READ) || defined(ENC7_READ)
// static uint16_t encoder_last_read[ENCODERS];
static void encoder_update(uint8_t i)
{
	int32_t encoder_read = 0;
	int32_t diff = 0;
	bool incremental = false;
	switch (i)
	{
#ifdef ENC0_READ // enc0 has custom read
	case ENC0:
		encoder_read = ENC0_READ;
#ifdef ENC0_IS_INCREMENTAL
		incremental = true;
#endif
		break;
#endif
#ifdef ENC1_READ // enc1 has custom read
	case ENC1:
		encoder_read = ENC1_READ;
#ifdef ENC1_IS_INCREMENTAL
		incremental = true;
#endif
		break;
#endif
#ifdef ENC2_READ // enc2 has custom read
	case ENC2:
		encoder_read = ENC2_READ;
#ifdef ENC2_IS_INCREMENTAL
		incremental = true;
#endif
		break;
#endif
#ifdef ENC3_READ // enc3 has custom read
	case ENC3:
		encoder_read = ENC3_READ;
#ifdef ENC3_IS_INCREMENTAL
		incremental = true;
#endif
		break;
#endif
#ifdef ENC4_READ // enc4 has custom read
	case ENC4:
		encoder_read = ENC4_READ;
#ifdef ENC4_IS_INCREMENTAL
		incremental = true;
#endif
		break;
#endif
#ifdef ENC5_READ // enc5 has custom read
	case ENC5:
		encoder_read = ENC5_READ;
#ifdef ENC5_IS_INCREMENTAL
		incremental = true;
#endif
		break;
#endif
#ifdef ENC6_READ // enc6 has custom read
	case ENC6:
		encoder_read = ENC6_READ;
#ifdef ENC6_IS_INCREMENTAL
		incremental = true;
#endif
		break;
#endif
#ifdef ENC7_READ // enc7 has custom read
	case ENC7:
		encoder_read = ENC7_READ;
#ifdef ENC7_IS_INCREMENTAL
		incremental = true;
#endif
		break;
#endif
	}

	if (incremental)
	{
		diff = (!(g_settings.encoders_dir_invert_mask & (1 << i))) ? (encoder_read - encoder_last_read[i]) : (encoder_last_read[i] - encoder_read);
		encoder_last_read[i] = encoder_read;
		if (g_settings.encoders_resolution[i])
		{
			if (diff < -(g_settings.encoders_resolution[i] >> 1))
			{
				return (diff + g_settings.encoders_resolution[i]);
			}
			if (diff > (g_settings.encoders_resolution[i] >> 1))
			{
				return (diff + g_settings.encoders_resolution[i]);
			}
		}
		encoder_pos[i] += diff;
	}
	else
	{
		encoder_pos[i] = ENC0_READ;
	}
}

#ifdef ENABLE_MAIN_LOOP_MODULES
bool encoders_dotasks(void *args)
{
#ifdef ENC0_READ
	encoder_update(ENC0);
#endif
#ifdef ENC0_READ
	encoder_update(ENC1);
#endif
#ifdef ENC0_READ
	encoder_update(ENC2);
#endif
#ifdef ENC0_READ
	encoder_update(ENC3);
#endif
#ifdef ENC0_READ
	encoder_update(ENC4);
#endif
#ifdef ENC0_READ
	encoder_update(ENC5);
#endif
#ifdef ENC0_READ
	encoder_update(ENC6);
#endif
#ifdef ENC0_READ
	encoder_update(ENC7);
#endif

	return EVENT_CONTINUE;
}
CREATE_EVENT_LISTENER(cnc_io_dotasks, encoders_dotasks);
#endif
#endif

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
	proto_printf("[EC:%" STRGIFY(ENCODERS) "lld" MSG_FEEDBACK_END, encoders_pos);
}

void encoder_reset_position(uint8_t i, int32_t position)
{
	ATOMIC_CODEBLOCK
	{
// reads the position to throw away last diff read
#if defined(ENC0_READ) || defined(ENC1_READ) || defined(ENC2_READ) || defined(ENC3_READ) || defined(ENC4_READ) || defined(ENC5_READ) || defined(ENC6_READ) || defined(ENC7_READ)
		encoder_update(i);
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

#if defined(ENC0_READ) || defined(ENC1_READ) || defined(ENC2_READ) || defined(ENC3_READ) || defined(ENC4_READ) || defined(ENC5_READ) || defined(ENC6_READ) || defined(ENC7_READ)
#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_io_dotasks, encoders_dotasks);
#else
#warning "ENABLE_MAIN_LOOP_MODULES is not defined. Custom encoders may not get updated."
#endif
#endif
}

// allow custom encoder implementations
int32_t __attribute__((weak)) enc_custom_read(uint8_t i) { return 0; }

#endif
