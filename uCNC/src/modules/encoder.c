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
#include <string.h>

#if ENCODERS > 0

#ifndef MIN_ENC_RPM
#define MIN_ENC_RPM 1
#endif

#define MIN_ENC_RPM_FACTOR (60000000UL / MIN_ENC_RPM)

static int32_t encoders_pos[ENCODERS];
static uint32_t encoders_tstamp[ENCODERS][2];
static uint16_t encoders_rpm[ENCODERS];

#ifndef ENCODER_RPM_SAMPLE_US
#define ENCODER_RPM_SAMPLE_US 100000UL
#endif

#ifndef ENCODER_INDEX_DEBUG_LINE_LEN
#define ENCODER_INDEX_DEBUG_LINE_LEN 128
#endif

#ifndef ENCODER_VIRTUAL_INDEX_FIRE_HOOK
#define ENCODER_VIRTUAL_INDEX_FIRE_HOOK 1
#endif

#ifndef ENC0_VIRTUAL_INDEX
#define ENC0_VIRTUAL_INDEX 0
#endif
#ifndef ENC1_VIRTUAL_INDEX
#define ENC1_VIRTUAL_INDEX 0
#endif
#ifndef ENC2_VIRTUAL_INDEX
#define ENC2_VIRTUAL_INDEX 0
#endif
#ifndef ENC3_VIRTUAL_INDEX
#define ENC3_VIRTUAL_INDEX 0
#endif
#ifndef ENC4_VIRTUAL_INDEX
#define ENC4_VIRTUAL_INDEX 0
#endif
#ifndef ENC5_VIRTUAL_INDEX
#define ENC5_VIRTUAL_INDEX 0
#endif
#ifndef ENC6_VIRTUAL_INDEX
#define ENC6_VIRTUAL_INDEX 0
#endif
#ifndef ENC7_VIRTUAL_INDEX
#define ENC7_VIRTUAL_INDEX 0
#endif

#ifndef ENC0_VIRTUAL_INDEX_CPR
#define ENC0_VIRTUAL_INDEX_CPR 0
#endif
#ifndef ENC1_VIRTUAL_INDEX_CPR
#define ENC1_VIRTUAL_INDEX_CPR 0
#endif
#ifndef ENC2_VIRTUAL_INDEX_CPR
#define ENC2_VIRTUAL_INDEX_CPR 0
#endif
#ifndef ENC3_VIRTUAL_INDEX_CPR
#define ENC3_VIRTUAL_INDEX_CPR 0
#endif
#ifndef ENC4_VIRTUAL_INDEX_CPR
#define ENC4_VIRTUAL_INDEX_CPR 0
#endif
#ifndef ENC5_VIRTUAL_INDEX_CPR
#define ENC5_VIRTUAL_INDEX_CPR 0
#endif
#ifndef ENC6_VIRTUAL_INDEX_CPR
#define ENC6_VIRTUAL_INDEX_CPR 0
#endif
#ifndef ENC7_VIRTUAL_INDEX_CPR
#define ENC7_VIRTUAL_INDEX_CPR 0
#endif

#ifndef ENC0_VIRTUAL_INDEX_OFFSET
#define ENC0_VIRTUAL_INDEX_OFFSET 0
#endif
#ifndef ENC1_VIRTUAL_INDEX_OFFSET
#define ENC1_VIRTUAL_INDEX_OFFSET 0
#endif
#ifndef ENC2_VIRTUAL_INDEX_OFFSET
#define ENC2_VIRTUAL_INDEX_OFFSET 0
#endif
#ifndef ENC3_VIRTUAL_INDEX_OFFSET
#define ENC3_VIRTUAL_INDEX_OFFSET 0
#endif
#ifndef ENC4_VIRTUAL_INDEX_OFFSET
#define ENC4_VIRTUAL_INDEX_OFFSET 0
#endif
#ifndef ENC5_VIRTUAL_INDEX_OFFSET
#define ENC5_VIRTUAL_INDEX_OFFSET 0
#endif
#ifndef ENC6_VIRTUAL_INDEX_OFFSET
#define ENC6_VIRTUAL_INDEX_OFFSET 0
#endif
#ifndef ENC7_VIRTUAL_INDEX_OFFSET
#define ENC7_VIRTUAL_INDEX_OFFSET 0
#endif

#ifndef ENC0_VIRTUAL_INDEX_HYSTERESIS
#define ENC0_VIRTUAL_INDEX_HYSTERESIS 1
#endif
#ifndef ENC1_VIRTUAL_INDEX_HYSTERESIS
#define ENC1_VIRTUAL_INDEX_HYSTERESIS 1
#endif
#ifndef ENC2_VIRTUAL_INDEX_HYSTERESIS
#define ENC2_VIRTUAL_INDEX_HYSTERESIS 1
#endif
#ifndef ENC3_VIRTUAL_INDEX_HYSTERESIS
#define ENC3_VIRTUAL_INDEX_HYSTERESIS 1
#endif
#ifndef ENC4_VIRTUAL_INDEX_HYSTERESIS
#define ENC4_VIRTUAL_INDEX_HYSTERESIS 1
#endif
#ifndef ENC5_VIRTUAL_INDEX_HYSTERESIS
#define ENC5_VIRTUAL_INDEX_HYSTERESIS 1
#endif
#ifndef ENC6_VIRTUAL_INDEX_HYSTERESIS
#define ENC6_VIRTUAL_INDEX_HYSTERESIS 1
#endif
#ifndef ENC7_VIRTUAL_INDEX_HYSTERESIS
#define ENC7_VIRTUAL_INDEX_HYSTERESIS 1
#endif

#ifndef ENC0_VIRTUAL_INDEX_ONLY
#define ENC0_VIRTUAL_INDEX_ONLY 0
#endif
#ifndef ENC1_VIRTUAL_INDEX_ONLY
#define ENC1_VIRTUAL_INDEX_ONLY 0
#endif
#ifndef ENC2_VIRTUAL_INDEX_ONLY
#define ENC2_VIRTUAL_INDEX_ONLY 0
#endif
#ifndef ENC3_VIRTUAL_INDEX_ONLY
#define ENC3_VIRTUAL_INDEX_ONLY 0
#endif
#ifndef ENC4_VIRTUAL_INDEX_ONLY
#define ENC4_VIRTUAL_INDEX_ONLY 0
#endif
#ifndef ENC5_VIRTUAL_INDEX_ONLY
#define ENC5_VIRTUAL_INDEX_ONLY 0
#endif
#ifndef ENC6_VIRTUAL_INDEX_ONLY
#define ENC6_VIRTUAL_INDEX_ONLY 0
#endif
#ifndef ENC7_VIRTUAL_INDEX_ONLY
#define ENC7_VIRTUAL_INDEX_ONLY 0
#endif

#if ((ENCODERS > 0 && ENC0_VIRTUAL_INDEX) || (ENCODERS > 1 && ENC1_VIRTUAL_INDEX) || (ENCODERS > 2 && ENC2_VIRTUAL_INDEX) || (ENCODERS > 3 && ENC3_VIRTUAL_INDEX) || (ENCODERS > 4 && ENC4_VIRTUAL_INDEX) || (ENCODERS > 5 && ENC5_VIRTUAL_INDEX) || (ENCODERS > 6 && ENC6_VIRTUAL_INDEX) || (ENCODERS > 7 && ENC7_VIRTUAL_INDEX))
#define ENCODER_HAS_VIRTUAL_INDEX 1
#else
#define ENCODER_HAS_VIRTUAL_INDEX 0
#endif

#if ENCODERS > 0
#ifndef ENC0_TYPE
#define ENC0_TYPE ENC_TYPE_PULSE
#endif
#if ENC0_TYPE == ENC_TYPE_PULSE
#if (!ASSERT_PIN(ENC0_PULSE))
#error "The ENC0 pulse pin is not defined"
#endif
#if (!ASSERT_PIN(ENC0_DIR))
#error "The ENC0 dir pin is not defined"
#endif
#define ENC0_IO_MASK (1 << (ENC0_PULSE - DIN_PINS_OFFSET))
#elif ENC0_TYPE == ENC_TYPE_I2C
#define ENC0_IO_MASK 0
#ifndef ENC0_FREQ
#define ENC0_FREQ 400000
#endif
#ifndef ENC0_READ
#define ENC0_READ read_encoder_mt6701_i2c(&enc0)
#endif
#elif ENC0_TYPE == ENC_TYPE_SSI
#define ENC0_IO_MASK 0
#ifndef ENC0_FREQ
#define ENC0_FREQ 15000000
#endif
#ifndef ENC0_READ
#define ENC0_READ read_encoder_mt6701_ssi(&enc0)
#endif
#elif ENC0_TYPE == ENC_TYPE_CUSTOM
#define ENC0_IO_MASK 0
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
#if ENC1_TYPE == ENC_TYPE_PULSE
#define ENC1_IO_MASK (1 << (ENC1_PULSE - DIN_PINS_OFFSET))
#elif ENC1_TYPE == ENC_TYPE_I2C
#ifndef ENC1_FREQ
#define ENC1_FREQ 400000
#endif
#ifndef ENC1_READ
#define ENC1_READ read_encoder_mt6701_i2c(&enc1)
#endif
#elif ENC1_TYPE == ENC_TYPE_SSI
#ifndef ENC1_FREQ
#define ENC1_FREQ 15000000
#endif
#ifndef ENC1_READ
#define ENC1_READ read_encoder_mt6701_ssi(&enc1)
#endif
#elif ENC1_TYPE == ENC_TYPE_CUSTOM
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
#if ENC2_TYPE == ENC_TYPE_PULSE
#define ENC2_IO_MASK (1 << (ENC2_PULSE - DIN_PINS_OFFSET))
#elif ENC2_TYPE == ENC_TYPE_I2C
#ifndef ENC2_FREQ
#define ENC2_FREQ 400000
#endif
#ifndef ENC2_READ
#define ENC2_READ read_encoder_mt6701_i2c(&enc2)
#endif
#elif ENC2_TYPE == ENC_TYPE_SSI
#ifndef ENC2_FREQ
#define ENC2_FREQ 15000000
#endif
#ifndef ENC2_READ
#define ENC2_READ read_encoder_mt6701_ssi(&enc2)
#endif
#elif ENC2_TYPE == ENC_TYPE_CUSTOM
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
#if ENC3_TYPE == ENC_TYPE_PULSE
#define ENC3_IO_MASK (1 << (ENC3_PULSE - DIN_PINS_OFFSET))
#elif ENC3_TYPE == ENC_TYPE_I2C
#ifndef ENC3_FREQ
#define ENC3_FREQ 400000
#endif
#ifndef ENC3_READ
#define ENC3_READ read_encoder_mt6701_i2c(&enc3)
#endif
#elif ENC3_TYPE == ENC_TYPE_SSI
#ifndef ENC3_FREQ
#define ENC3_FREQ 15000000
#endif
#ifndef ENC3_READ
#define ENC3_READ read_encoder_mt6701_ssi(&enc3)
#endif
#elif ENC3_TYPE == ENC_TYPE_CUSTOM
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
#if ENC4_TYPE == ENC_TYPE_PULSE
#define ENC4_IO_MASK (1 << (ENC4_PULSE - DIN_PINS_OFFSET))
#elif ENC4_TYPE == ENC_TYPE_I2C
#ifndef ENC4_FREQ
#define ENC4_FREQ 400000
#endif
#ifndef ENC4_READ
#define ENC4_READ read_encoder_mt6701_i2c(&enc4)
#endif
#elif ENC4_TYPE == ENC_TYPE_SSI
#ifndef ENC4_FREQ
#define ENC4_FREQ 15000000
#endif
#ifndef ENC4_READ
#define ENC4_READ read_encoder_mt6701_ssi(&enc4)
#endif
#elif ENC4_TYPE == ENC_TYPE_CUSTOM
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
#if ENC5_TYPE == ENC_TYPE_PULSE
#define ENC5_IO_MASK (1 << (ENC5_PULSE - DIN_PINS_OFFSET))
#elif ENC5_TYPE == ENC_TYPE_I2C
#ifndef ENC5_FREQ
#define ENC5_FREQ 400000
#endif
#ifndef ENC5_READ
#define ENC5_READ read_encoder_mt6701_i2c(&enc5)
#endif
#elif ENC5_TYPE == ENC_TYPE_SSI
#ifndef ENC5_FREQ
#define ENC5_FREQ 15000000
#endif
#ifndef ENC5_READ
#define ENC5_READ read_encoder_mt6701_ssi(&enc5)
#endif
#elif ENC5_TYPE == ENC_TYPE_CUSTOM
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
#if ENC6_TYPE == ENC_TYPE_PULSE
#define ENC6_IO_MASK (1 << (ENC6_PULSE - DIN_PINS_OFFSET))
#elif ENC6_TYPE == ENC_TYPE_I2C
#ifndef ENC6_FREQ
#define ENC6_FREQ 400000
#endif
#ifndef ENC6_READ
#define ENC6_READ read_encoder_mt6701_i2c(&enc6)
#endif
#elif ENC6_TYPE == ENC_TYPE_SSI
#ifndef ENC6_FREQ
#define ENC6_FREQ 15000000
#endif
#ifndef ENC6_READ
#define ENC6_READ read_encoder_mt6701_ssi(&enc6)
#endif
#elif ENC6_TYPE == ENC_TYPE_CUSTOM
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
#if ENC7_TYPE == ENC_TYPE_PULSE
#define ENC7_IO_MASK (1 << (ENC7_PULSE - DIN_PINS_OFFSET))
#elif ENC7_TYPE == ENC_TYPE_I2C
#ifndef ENC7_FREQ
#define ENC7_FREQ 400000
#endif
#ifndef ENC7_READ
#define ENC7_READ read_encoder_mt6701_i2c(&enc7)
#endif
#elif ENC7_TYPE == ENC_TYPE_SSI
#ifndef ENC7_FREQ
#define ENC7_FREQ 15000000
#endif
#ifndef ENC7_READ
#define ENC7_READ read_encoder_mt6701_ssi(&enc7)
#endif
#elif ENC7_TYPE == ENC_TYPE_CUSTOM
#ifndef ENC7_READ
#define ENC7_READ enc_custom_read(ENC7)
#endif
#endif
#endif

#ifndef ENC0_IO_MASK
#define ENC0_IO_MASK 0
#endif
#ifndef ENC1_IO_MASK
#define ENC1_IO_MASK 0
#endif
#ifndef ENC2_IO_MASK
#define ENC2_IO_MASK 0
#endif
#ifndef ENC3_IO_MASK
#define ENC3_IO_MASK 0
#endif
#ifndef ENC4_IO_MASK
#define ENC4_IO_MASK 0
#endif
#ifndef ENC5_IO_MASK
#define ENC5_IO_MASK 0
#endif
#ifndef ENC6_IO_MASK
#define ENC6_IO_MASK 0
#endif
#ifndef ENC7_IO_MASK
#define ENC7_IO_MASK 0
#endif

#define ENCODERS_IO_MASK (ENC0_IO_MASK | ENC1_IO_MASK | ENC2_IO_MASK | ENC3_IO_MASK | ENC4_IO_MASK | ENC5_IO_MASK | ENC6_IO_MASK | ENC7_IO_MASK)

#if ((ENC0_INDEX == DIN0 && defined(DIN0_ISR)) || (ENC0_INDEX == DIN1 && defined(DIN1_ISR)) || (ENC0_INDEX == DIN2 && defined(DIN2_ISR)) || (ENC0_INDEX == DIN3 && defined(DIN3_ISR)) || (ENC0_INDEX == DIN4 && defined(DIN4_ISR)) || (ENC0_INDEX == DIN5 && defined(DIN5_ISR)) || (ENC0_INDEX == DIN6 && defined(DIN6_ISR)) || (ENC0_INDEX == DIN7 && defined(DIN7_ISR)))
#define ENC0_INDEX_IO_MASK (1 << (ENC0_INDEX - DIN_PINS_OFFSET))
#endif
#if ((ENC1_INDEX == DIN0 && defined(DIN0_ISR)) || (ENC1_INDEX == DIN1 && defined(DIN1_ISR)) || (ENC1_INDEX == DIN2 && defined(DIN2_ISR)) || (ENC1_INDEX == DIN3 && defined(DIN3_ISR)) || (ENC1_INDEX == DIN4 && defined(DIN4_ISR)) || (ENC1_INDEX == DIN5 && defined(DIN5_ISR)) || (ENC1_INDEX == DIN6 && defined(DIN6_ISR)) || (ENC1_INDEX == DIN7 && defined(DIN7_ISR)))
#define ENC1_INDEX_IO_MASK (1 << (ENC1_INDEX - DIN_PINS_OFFSET))
#endif
#if ((ENC2_INDEX == DIN0 && defined(DIN0_ISR)) || (ENC2_INDEX == DIN1 && defined(DIN1_ISR)) || (ENC2_INDEX == DIN2 && defined(DIN2_ISR)) || (ENC2_INDEX == DIN3 && defined(DIN3_ISR)) || (ENC2_INDEX == DIN4 && defined(DIN4_ISR)) || (ENC2_INDEX == DIN5 && defined(DIN5_ISR)) || (ENC2_INDEX == DIN6 && defined(DIN6_ISR)) || (ENC2_INDEX == DIN7 && defined(DIN7_ISR)))
#define ENC2_INDEX_IO_MASK (1 << (ENC2_INDEX - DIN_PINS_OFFSET))
#endif
#if ((ENC3_INDEX == DIN0 && defined(DIN0_ISR)) || (ENC3_INDEX == DIN1 && defined(DIN1_ISR)) || (ENC3_INDEX == DIN2 && defined(DIN2_ISR)) || (ENC3_INDEX == DIN3 && defined(DIN3_ISR)) || (ENC3_INDEX == DIN4 && defined(DIN4_ISR)) || (ENC3_INDEX == DIN5 && defined(DIN5_ISR)) || (ENC3_INDEX == DIN6 && defined(DIN6_ISR)) || (ENC3_INDEX == DIN7 && defined(DIN7_ISR)))
#define ENC3_INDEX_IO_MASK (1 << (ENC3_INDEX - DIN_PINS_OFFSET))
#endif
#if ((ENC4_INDEX == DIN0 && defined(DIN0_ISR)) || (ENC4_INDEX == DIN1 && defined(DIN1_ISR)) || (ENC4_INDEX == DIN2 && defined(DIN2_ISR)) || (ENC4_INDEX == DIN3 && defined(DIN3_ISR)) || (ENC4_INDEX == DIN4 && defined(DIN4_ISR)) || (ENC4_INDEX == DIN5 && defined(DIN5_ISR)) || (ENC4_INDEX == DIN6 && defined(DIN6_ISR)) || (ENC4_INDEX == DIN7 && defined(DIN7_ISR)))
#define ENC4_INDEX_IO_MASK (1 << (ENC4_INDEX - DIN_PINS_OFFSET))
#endif
#if ((ENC5_INDEX == DIN0 && defined(DIN0_ISR)) || (ENC5_INDEX == DIN1 && defined(DIN1_ISR)) || (ENC5_INDEX == DIN2 && defined(DIN2_ISR)) || (ENC5_INDEX == DIN3 && defined(DIN3_ISR)) || (ENC5_INDEX == DIN4 && defined(DIN4_ISR)) || (ENC5_INDEX == DIN5 && defined(DIN5_ISR)) || (ENC5_INDEX == DIN6 && defined(DIN6_ISR)) || (ENC5_INDEX == DIN7 && defined(DIN7_ISR)))
#define ENC5_INDEX_IO_MASK (1 << (ENC5_INDEX - DIN_PINS_OFFSET))
#endif
#if ((ENC6_INDEX == DIN0 && defined(DIN0_ISR)) || (ENC6_INDEX == DIN1 && defined(DIN1_ISR)) || (ENC6_INDEX == DIN2 && defined(DIN2_ISR)) || (ENC6_INDEX == DIN3 && defined(DIN3_ISR)) || (ENC6_INDEX == DIN4 && defined(DIN4_ISR)) || (ENC6_INDEX == DIN5 && defined(DIN5_ISR)) || (ENC6_INDEX == DIN6 && defined(DIN6_ISR)) || (ENC6_INDEX == DIN7 && defined(DIN7_ISR)))
#define ENC6_INDEX_IO_MASK (1 << (ENC6_INDEX - DIN_PINS_OFFSET))
#endif
#if ((ENC7_INDEX == DIN0 && defined(DIN0_ISR)) || (ENC7_INDEX == DIN1 && defined(DIN1_ISR)) || (ENC7_INDEX == DIN2 && defined(DIN2_ISR)) || (ENC7_INDEX == DIN3 && defined(DIN3_ISR)) || (ENC7_INDEX == DIN4 && defined(DIN4_ISR)) || (ENC7_INDEX == DIN5 && defined(DIN5_ISR)) || (ENC7_INDEX == DIN6 && defined(DIN6_ISR)) || (ENC7_INDEX == DIN7 && defined(DIN7_ISR)))
#define ENC7_INDEX_IO_MASK (1 << (ENC7_INDEX - DIN_PINS_OFFSET))
#endif

#ifndef ENC0_INDEX_IO_MASK
#define ENC0_INDEX_IO_MASK 0
#endif
#ifndef ENC1_INDEX_IO_MASK
#define ENC1_INDEX_IO_MASK 0
#endif
#ifndef ENC2_INDEX_IO_MASK
#define ENC2_INDEX_IO_MASK 0
#endif
#ifndef ENC3_INDEX_IO_MASK
#define ENC3_INDEX_IO_MASK 0
#endif
#ifndef ENC4_INDEX_IO_MASK
#define ENC4_INDEX_IO_MASK 0
#endif
#ifndef ENC5_INDEX_IO_MASK
#define ENC5_INDEX_IO_MASK 0
#endif
#ifndef ENC6_INDEX_IO_MASK
#define ENC6_INDEX_IO_MASK 0
#endif
#ifndef ENC7_INDEX_IO_MASK
#define ENC7_INDEX_IO_MASK 0
#endif

#define ENCODERS_INDEX_IO_MASK (ENC0_INDEX_IO_MASK | ENC1_INDEX_IO_MASK | ENC2_INDEX_IO_MASK | ENC3_INDEX_IO_MASK | ENC4_INDEX_IO_MASK | ENC5_INDEX_IO_MASK | ENC6_INDEX_IO_MASK | ENC7_INDEX_IO_MASK)

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

#if ENCODERS > 0 && (defined(ENC0_INDEX) || ENC0_VIRTUAL_INDEX)
CREATE_HOOK(enc0_index);
#define ENC0_INDEX_EVENT() HOOK_INVOKE(enc0_index)
#endif
#if ENCODERS > 1 && (defined(ENC1_INDEX) || ENC1_VIRTUAL_INDEX)
CREATE_HOOK(enc1_index);
#define ENC1_INDEX_EVENT() HOOK_INVOKE(enc1_index)
#endif
#if ENCODERS > 2 && (defined(ENC2_INDEX) || ENC2_VIRTUAL_INDEX)
CREATE_HOOK(enc2_index);
#define ENC2_INDEX_EVENT() HOOK_INVOKE(enc2_index)
#endif
#if ENCODERS > 3 && (defined(ENC3_INDEX) || ENC3_VIRTUAL_INDEX)
CREATE_HOOK(enc3_index);
#define ENC3_INDEX_EVENT() HOOK_INVOKE(enc3_index)
#endif
#if ENCODERS > 4 && (defined(ENC4_INDEX) || ENC4_VIRTUAL_INDEX)
CREATE_HOOK(enc4_index);
#define ENC4_INDEX_EVENT() HOOK_INVOKE(enc4_index)
#endif
#if ENCODERS > 5 && (defined(ENC5_INDEX) || ENC5_VIRTUAL_INDEX)
CREATE_HOOK(enc5_index);
#define ENC5_INDEX_EVENT() HOOK_INVOKE(enc5_index)
#endif
#if ENCODERS > 6 && (defined(ENC6_INDEX) || ENC6_VIRTUAL_INDEX)
CREATE_HOOK(enc6_index);
#define ENC6_INDEX_EVENT() HOOK_INVOKE(enc6_index)
#endif
#if ENCODERS > 7 && (defined(ENC7_INDEX) || ENC7_VIRTUAL_INDEX)
CREATE_HOOK(enc7_index);
#define ENC7_INDEX_EVENT() HOOK_INVOKE(enc7_index)
#endif

void __attribute__((weak)) enc0_pulse(void) {}
void __attribute__((weak)) enc1_pulse(void) {}
void __attribute__((weak)) enc2_pulse(void) {}
void __attribute__((weak)) enc3_pulse(void) {}
void __attribute__((weak)) enc4_pulse(void) {}
void __attribute__((weak)) enc5_pulse(void) {}
void __attribute__((weak)) enc6_pulse(void) {}
void __attribute__((weak)) enc7_pulse(void) {}

#ifndef ENC0_PULSE_EVENT
#define ENC0_PULSE_EVENT() enc0_pulse()
#endif
#ifndef ENC1_PULSE_EVENT
#define ENC1_PULSE_EVENT() enc1_pulse()
#endif
#ifndef ENC2_PULSE_EVENT
#define ENC2_PULSE_EVENT() enc2_pulse()
#endif
#ifndef ENC3_PULSE_EVENT
#define ENC3_PULSE_EVENT() enc3_pulse()
#endif
#ifndef ENC4_PULSE_EVENT
#define ENC4_PULSE_EVENT() enc4_pulse()
#endif
#ifndef ENC5_PULSE_EVENT
#define ENC5_PULSE_EVENT() enc5_pulse()
#endif
#ifndef ENC6_PULSE_EVENT
#define ENC6_PULSE_EVENT() enc6_pulse()
#endif
#ifndef ENC7_PULSE_EVENT
#define ENC7_PULSE_EVENT() enc7_pulse()
#endif

#define ENC_PULSE_EVENT(X) X##_PULSE_EVENT()
#define ENC_INDEX_EVENT(X) X##_INDEX_EVENT()

/**
 * Additional read functions for other types of encoders can be added later
 * For now support for the MT6701 is added
 */
static int32_t encoder_last_read[ENCODERS] __attribute__((unused));
// static uint16_t encoder_last_read[ENCODERS] __attribute__((unused));
static int32_t encoder_rpm_accum[ENCODERS] __attribute__((unused));
static uint32_t encoder_rpm_tstamp[ENCODERS] __attribute__((unused));
static uint8_t encoder_index_reset_done[ENCODERS] __attribute__((unused));
uint16_t read_encoder_mt6701_i2c(softi2c_port_t *port);
uint16_t read_encoder_mt6701_ssi(softspi_port_t *port);

static void encoder_set_position_from_current_read(uint8_t i, int32_t position)
{
#if defined(ENC0_READ) || defined(ENC1_READ) || defined(ENC2_READ) || defined(ENC3_READ) || defined(ENC4_READ) || defined(ENC5_READ) || defined(ENC6_READ) || defined(ENC7_READ)
	int32_t encoder_read = 0;
	switch (i)
	{
#ifdef ENC0_READ
	case ENC0:
		encoder_read = ENC0_READ;
		break;
#endif
#ifdef ENC1_READ
	case ENC1:
		encoder_read = ENC1_READ;
		break;
#endif
#ifdef ENC2_READ
	case ENC2:
		encoder_read = ENC2_READ;
		break;
#endif
#ifdef ENC3_READ
	case ENC3:
		encoder_read = ENC3_READ;
		break;
#endif
#ifdef ENC4_READ
	case ENC4:
		encoder_read = ENC4_READ;
		break;
#endif
#ifdef ENC5_READ
	case ENC5:
		encoder_read = ENC5_READ;
		break;
#endif
#ifdef ENC6_READ
	case ENC6:
		encoder_read = ENC6_READ;
		break;
#endif
#ifdef ENC7_READ
	case ENC7:
		encoder_read = ENC7_READ;
		break;
#endif
	default:
		break;
	}
	encoder_last_read[i] = encoder_read;
#endif
	encoders_pos[i] = position;
}

static void encoder_set_position_from_current_read_once(uint8_t i, int32_t position)
{
	if (!encoder_index_reset_done[i])
	{
		encoder_set_position_from_current_read(i, position);
		encoder_index_reset_done[i] = true;
	}
}

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

uint32_t encoder_get_delta(uint8_t i)
{
	uint32_t t0 = 0, t1 = 0;
	ATOMIC_CODEBLOCK
	{
		t0 = encoders_tstamp[i][0]; // last timestamp
		t1 = encoders_tstamp[i][1]; // previous timestamp
	}

	return (t0 - t1);
}

uint16_t encoder_get_rpm(uint8_t i)
{
#ifdef ENC0_READ
	if (i == ENC0)
	{
		return encoders_rpm[i];
	}
#endif
#ifdef ENC1_READ
	if (i == ENC1)
	{
		return encoders_rpm[i];
	}
#endif
#ifdef ENC2_READ
	if (i == ENC2)
	{
		return encoders_rpm[i];
	}
#endif
#ifdef ENC3_READ
	if (i == ENC3)
	{
		return encoders_rpm[i];
	}
#endif
#ifdef ENC4_READ
	if (i == ENC4)
	{
		return encoders_rpm[i];
	}
#endif
#ifdef ENC5_READ
	if (i == ENC5)
	{
		return encoders_rpm[i];
	}
#endif
#ifdef ENC6_READ
	if (i == ENC6)
	{
		return encoders_rpm[i];
	}
#endif
#ifdef ENC7_READ
	if (i == ENC7)
	{
		return encoders_rpm[i];
	}
#endif

	uint32_t t0 = 0, t1 = 0, t_now = mcu_micros();
	ATOMIC_CODEBLOCK
	{
		t0 = encoders_tstamp[i][0]; // last timestamp
		t1 = encoders_tstamp[i][1]; // previous timestamp
	}

	uint32_t delta = (t0 - t1);
	uint32_t max_elapsed = MIN_ENC_RPM_FACTOR / g_settings.encoders_resolution[i];
	uint32_t last_elapsed = (t_now - t0);

	if (!delta || (last_elapsed > max_elapsed))
	{
		ATOMIC_CODEBLOCK
		{
			encoders_tstamp[i][0] = 0;
			encoders_tstamp[i][1] = 0;
		}
		return 0;
	}

	if (last_elapsed > delta)
	{
		delta = last_elapsed;
	}

	uint32_t rpm = 60000000UL / delta / g_settings.encoders_resolution[i];

	return (uint16_t)rpm;
}

#if ENCODER_HAS_VIRTUAL_INDEX

typedef struct
{
	bool have_origin;
	bool have_slot;
	bool have_stats;
	int32_t origin;
	int32_t last_slot;
	int32_t last_position;
	int32_t last_delta;
	int32_t min_delta;
	int32_t max_delta;
	uint32_t count;
	uint32_t ignored_count;
	uint32_t abs_delta_sum;
	uint32_t seq;
	char debug_line[ENCODER_INDEX_DEBUG_LINE_LEN];
} encoder_index_state_t;

static encoder_index_state_t encoder_index_state[ENCODERS];

static FORCEINLINE int32_t encoder_floor_div_i32(int32_t value, int32_t divisor)
{
	int32_t q = value / divisor;
	int32_t r = value % divisor;
	if (r && ((r < 0) != (divisor < 0)))
	{
		q--;
	}
	return q;
}

static uint32_t encoder_virtual_index_cpr(uint8_t i)
{
	switch (i)
	{
#if ENCODERS > 0 && ENC0_VIRTUAL_INDEX
	case ENC0:
		return (uint32_t)ENC0_VIRTUAL_INDEX_CPR;
#endif
#if ENCODERS > 1 && ENC1_VIRTUAL_INDEX
	case ENC1:
		return (uint32_t)ENC1_VIRTUAL_INDEX_CPR;
#endif
#if ENCODERS > 2 && ENC2_VIRTUAL_INDEX
	case ENC2:
		return (uint32_t)ENC2_VIRTUAL_INDEX_CPR;
#endif
#if ENCODERS > 3 && ENC3_VIRTUAL_INDEX
	case ENC3:
		return (uint32_t)ENC3_VIRTUAL_INDEX_CPR;
#endif
#if ENCODERS > 4 && ENC4_VIRTUAL_INDEX
	case ENC4:
		return (uint32_t)ENC4_VIRTUAL_INDEX_CPR;
#endif
#if ENCODERS > 5 && ENC5_VIRTUAL_INDEX
	case ENC5:
		return (uint32_t)ENC5_VIRTUAL_INDEX_CPR;
#endif
#if ENCODERS > 6 && ENC6_VIRTUAL_INDEX
	case ENC6:
		return (uint32_t)ENC6_VIRTUAL_INDEX_CPR;
#endif
#if ENCODERS > 7 && ENC7_VIRTUAL_INDEX
	case ENC7:
		return (uint32_t)ENC7_VIRTUAL_INDEX_CPR;
#endif
	default:
		break;
	}
	return 0;
}

static int32_t encoder_virtual_index_offset(uint8_t i)
{
	switch (i)
	{
#if ENCODERS > 0 && ENC0_VIRTUAL_INDEX
	case ENC0:
		return (int32_t)ENC0_VIRTUAL_INDEX_OFFSET;
#endif
#if ENCODERS > 1 && ENC1_VIRTUAL_INDEX
	case ENC1:
		return (int32_t)ENC1_VIRTUAL_INDEX_OFFSET;
#endif
#if ENCODERS > 2 && ENC2_VIRTUAL_INDEX
	case ENC2:
		return (int32_t)ENC2_VIRTUAL_INDEX_OFFSET;
#endif
#if ENCODERS > 3 && ENC3_VIRTUAL_INDEX
	case ENC3:
		return (int32_t)ENC3_VIRTUAL_INDEX_OFFSET;
#endif
#if ENCODERS > 4 && ENC4_VIRTUAL_INDEX
	case ENC4:
		return (int32_t)ENC4_VIRTUAL_INDEX_OFFSET;
#endif
#if ENCODERS > 5 && ENC5_VIRTUAL_INDEX
	case ENC5:
		return (int32_t)ENC5_VIRTUAL_INDEX_OFFSET;
#endif
#if ENCODERS > 6 && ENC6_VIRTUAL_INDEX
	case ENC6:
		return (int32_t)ENC6_VIRTUAL_INDEX_OFFSET;
#endif
#if ENCODERS > 7 && ENC7_VIRTUAL_INDEX
	case ENC7:
		return (int32_t)ENC7_VIRTUAL_INDEX_OFFSET;
#endif
	default:
		break;
	}
	return 0;
}

static uint32_t encoder_virtual_index_hysteresis(uint8_t i)
{
	switch (i)
	{
#if ENCODERS > 0 && ENC0_VIRTUAL_INDEX
	case ENC0:
		return (uint32_t)ENC0_VIRTUAL_INDEX_HYSTERESIS;
#endif
#if ENCODERS > 1 && ENC1_VIRTUAL_INDEX
	case ENC1:
		return (uint32_t)ENC1_VIRTUAL_INDEX_HYSTERESIS;
#endif
#if ENCODERS > 2 && ENC2_VIRTUAL_INDEX
	case ENC2:
		return (uint32_t)ENC2_VIRTUAL_INDEX_HYSTERESIS;
#endif
#if ENCODERS > 3 && ENC3_VIRTUAL_INDEX
	case ENC3:
		return (uint32_t)ENC3_VIRTUAL_INDEX_HYSTERESIS;
#endif
#if ENCODERS > 4 && ENC4_VIRTUAL_INDEX
	case ENC4:
		return (uint32_t)ENC4_VIRTUAL_INDEX_HYSTERESIS;
#endif
#if ENCODERS > 5 && ENC5_VIRTUAL_INDEX
	case ENC5:
		return (uint32_t)ENC5_VIRTUAL_INDEX_HYSTERESIS;
#endif
#if ENCODERS > 6 && ENC6_VIRTUAL_INDEX
	case ENC6:
		return (uint32_t)ENC6_VIRTUAL_INDEX_HYSTERESIS;
#endif
#if ENCODERS > 7 && ENC7_VIRTUAL_INDEX
	case ENC7:
		return (uint32_t)ENC7_VIRTUAL_INDEX_HYSTERESIS;
#endif
	default:
		break;
	}
	return 1;
}

static bool encoder_virtual_index_enabled(uint8_t i)
{
	switch (i)
	{
#if ENCODERS > 0 && ENC0_VIRTUAL_INDEX
	case ENC0:
		return true;
#endif
#if ENCODERS > 1 && ENC1_VIRTUAL_INDEX
	case ENC1:
		return true;
#endif
#if ENCODERS > 2 && ENC2_VIRTUAL_INDEX
	case ENC2:
		return true;
#endif
#if ENCODERS > 3 && ENC3_VIRTUAL_INDEX
	case ENC3:
		return true;
#endif
#if ENCODERS > 4 && ENC4_VIRTUAL_INDEX
	case ENC4:
		return true;
#endif
#if ENCODERS > 5 && ENC5_VIRTUAL_INDEX
	case ENC5:
		return true;
#endif
#if ENCODERS > 6 && ENC6_VIRTUAL_INDEX
	case ENC6:
		return true;
#endif
#if ENCODERS > 7 && ENC7_VIRTUAL_INDEX
	case ENC7:
		return true;
#endif
	default:
		break;
	}
	return false;
}

void encoder_invoke_index(uint8_t i)
{
	switch (i)
	{
#if ENCODERS > 0 && (defined(ENC0_INDEX) || ENC0_VIRTUAL_INDEX)
	case ENC0:
		HOOK_INVOKE(enc0_index);
		break;
#endif
#if ENCODERS > 1 && (defined(ENC1_INDEX) || ENC1_VIRTUAL_INDEX)
	case ENC1:
		HOOK_INVOKE(enc1_index);
		break;
#endif
#if ENCODERS > 2 && (defined(ENC2_INDEX) || ENC2_VIRTUAL_INDEX)
	case ENC2:
		HOOK_INVOKE(enc2_index);
		break;
#endif
#if ENCODERS > 3 && (defined(ENC3_INDEX) || ENC3_VIRTUAL_INDEX)
	case ENC3:
		HOOK_INVOKE(enc3_index);
		break;
#endif
#if ENCODERS > 4 && (defined(ENC4_INDEX) || ENC4_VIRTUAL_INDEX)
	case ENC4:
		HOOK_INVOKE(enc4_index);
		break;
#endif
#if ENCODERS > 5 && (defined(ENC5_INDEX) || ENC5_VIRTUAL_INDEX)
	case ENC5:
		HOOK_INVOKE(enc5_index);
		break;
#endif
#if ENCODERS > 6 && (defined(ENC6_INDEX) || ENC6_VIRTUAL_INDEX)
	case ENC6:
		HOOK_INVOKE(enc6_index);
		break;
#endif
#if ENCODERS > 7 && (defined(ENC7_INDEX) || ENC7_VIRTUAL_INDEX)
	case ENC7:
		HOOK_INVOKE(enc7_index);
		break;
#endif
	default:
		break;
	}
}

void encoder_record_index_reference(uint8_t i, int32_t position)
{
	encoder_index_state_t *s;
	if (i >= ENCODERS)
	{
		return;
	}
	s = &encoder_index_state[i];
	s->origin = position - encoder_virtual_index_offset(i);
	s->last_position = position;
	s->have_origin = true;
	s->have_slot = false;
	encoder_invoke_index(i);
}

static void encoder_accept_virtual_index(uint8_t i, int32_t boundary)
{
	encoder_index_state_t *s = &encoder_index_state[i];
	int32_t delta = boundary - s->last_position;
	uint32_t avg10;

	s->last_position = boundary;
	s->last_delta = delta;
	s->count++;
	s->abs_delta_sum += (uint32_t)ABS(delta);

	if (!s->have_stats)
	{
		s->min_delta = delta;
		s->max_delta = delta;
		s->have_stats = true;
	}
	else
	{
		if (delta < s->min_delta)
		{
			s->min_delta = delta;
		}
		if (delta > s->max_delta)
		{
			s->max_delta = delta;
		}
	}

	avg10 = (s->count) ? (uint32_t)(((uint64_t)s->abs_delta_sum * 10ULL + (s->count / 2U)) / s->count) : 0;
	str_snprintf(s->debug_line, sizeof(s->debug_line), "ENCIDX EC:%ld ECB:%ld LAST:%ld AVG:%lu.%lu MIN:%ld MAX:%ld N:%lu IGN:%lu",
				 (long)encoder_get_position(i),
				 (long)(encoder_get_position(i) - s->last_position),
				 (long)s->last_delta,
				 (unsigned long)(avg10 / 10U),
				 (unsigned long)(avg10 % 10U),
				 (long)s->min_delta,
				 (long)s->max_delta,
				 (unsigned long)s->count,
				 (unsigned long)s->ignored_count);
	s->seq++;

#if ENCODER_VIRTUAL_INDEX_FIRE_HOOK
	encoder_invoke_index(i);
#endif
}

void encoder_virtual_index_clear(uint8_t i)
{
	if (i < ENCODERS)
	{
		memset(&encoder_index_state[i], 0, sizeof(encoder_index_state[i]));
	}
}

void encoder_virtual_index_update(uint8_t i)
{
	encoder_index_state_t *s;
	uint32_t cpr;
	uint32_t hysteresis;
	int32_t now;
	int32_t slot;
	int32_t slot_delta;

	if (i >= ENCODERS || !encoder_virtual_index_enabled(i))
	{
		return;
	}

	cpr = encoder_virtual_index_cpr(i);
	if (!cpr)
	{
		cpr = (uint32_t)g_settings.encoders_resolution[i];
	}
	if (!cpr)
	{
		return;
	}

	s = &encoder_index_state[i];
	now = encoder_get_position(i);
	hysteresis = encoder_virtual_index_hysteresis(i);

	if (!s->have_origin)
	{
		s->origin = now - encoder_virtual_index_offset(i);
		s->last_position = now;
		s->have_origin = true;
		s->have_slot = false;
	}

	slot = encoder_floor_div_i32(now - s->origin, (int32_t)cpr);
	if (!s->have_slot)
	{
		s->last_slot = slot;
		s->have_slot = true;
		return;
	}

	slot_delta = slot - s->last_slot;
	if (!slot_delta || (ABS(now - (s->origin + slot * (int32_t)cpr)) < hysteresis))
	{
		return;
	}

	if (slot_delta > 1)
	{
		s->ignored_count += (uint32_t)(slot_delta - 1);
	}
	else if (slot_delta < -1)
	{
		s->ignored_count += (uint32_t)((-slot_delta) - 1);
	}

	s->last_slot = slot;
	encoder_accept_virtual_index(i, s->origin + slot * (int32_t)cpr);
}

bool encoder_get_index_stats(uint8_t i, int32_t *last, int32_t *min, int32_t *max, uint32_t *count)
{
	encoder_index_state_t *s;
	if (i >= ENCODERS || !encoder_index_state[i].have_stats)
	{
		return false;
	}
	s = &encoder_index_state[i];
	if (last)
	{
		*last = s->last_delta;
	}
	if (min)
	{
		*min = s->min_delta;
	}
	if (max)
	{
		*max = s->max_delta;
	}
	if (count)
	{
		*count = s->count;
	}
	return true;
}

bool encoder_get_index_live_delta(uint8_t i, int32_t *delta)
{
	if (i >= ENCODERS || !encoder_index_state[i].have_origin || !delta)
	{
		return false;
	}
	*delta = encoder_get_position(i) - encoder_index_state[i].last_position;
	return true;
}

bool encoder_get_index_debug_line(uint8_t i, char *line, uint32_t line_len, uint32_t *seq)
{
	if (i >= ENCODERS || !line || !line_len || !encoder_index_state[i].seq)
	{
		return false;
	}
	strncpy(line, encoder_index_state[i].debug_line, line_len - 1);
	line[line_len - 1] = '\0';
	if (seq)
	{
		*seq = encoder_index_state[i].seq;
	}
	return true;
}

#if ENC0_VIRTUAL_INDEX
#undef ENC0_INDEX_EVENT
#ifdef ENC0_INDEX_RESET_POSITION
#define ENC0_INDEX_EVENT() encoder_set_position_from_current_read_once(ENC0, ENC0_INDEX_RESET_POSITION);encoder_record_index_reference(ENC0, encoder_get_position(ENC0))
#else
#define ENC0_INDEX_EVENT() encoder_record_index_reference(ENC0, encoder_get_position(ENC0))
#endif
#endif
#if ENC1_VIRTUAL_INDEX
#undef ENC1_INDEX_EVENT
#ifdef ENC1_INDEX_RESET_POSITION
#define ENC1_INDEX_EVENT() encoder_set_position_from_current_read_once(ENC1, ENC1_INDEX_RESET_POSITION);encoder_record_index_reference(ENC1, encoder_get_position(ENC1))
#else
#define ENC1_INDEX_EVENT() encoder_record_index_reference(ENC1, encoder_get_position(ENC1))
#endif
#endif
#if ENC2_VIRTUAL_INDEX
#undef ENC2_INDEX_EVENT
#ifdef ENC2_INDEX_RESET_POSITION
#define ENC2_INDEX_EVENT() encoder_set_position_from_current_read_once(ENC2, ENC2_INDEX_RESET_POSITION);encoder_record_index_reference(ENC2, encoder_get_position(ENC2))
#else
#define ENC2_INDEX_EVENT() encoder_record_index_reference(ENC2, encoder_get_position(ENC2))
#endif
#endif
#if ENC3_VIRTUAL_INDEX
#undef ENC3_INDEX_EVENT
#ifdef ENC3_INDEX_RESET_POSITION
#define ENC3_INDEX_EVENT() encoder_set_position_from_current_read_once(ENC3, ENC3_INDEX_RESET_POSITION);encoder_record_index_reference(ENC3, encoder_get_position(ENC3))
#else
#define ENC3_INDEX_EVENT() encoder_record_index_reference(ENC3, encoder_get_position(ENC3))
#endif
#endif
#if ENC4_VIRTUAL_INDEX
#undef ENC4_INDEX_EVENT
#ifdef ENC4_INDEX_RESET_POSITION
#define ENC4_INDEX_EVENT() encoder_set_position_from_current_read_once(ENC4, ENC4_INDEX_RESET_POSITION);encoder_record_index_reference(ENC4, encoder_get_position(ENC4))
#else
#define ENC4_INDEX_EVENT() encoder_record_index_reference(ENC4, encoder_get_position(ENC4))
#endif
#endif
#if ENC5_VIRTUAL_INDEX
#undef ENC5_INDEX_EVENT
#ifdef ENC5_INDEX_RESET_POSITION
#define ENC5_INDEX_EVENT() encoder_set_position_from_current_read_once(ENC5, ENC5_INDEX_RESET_POSITION);encoder_record_index_reference(ENC5, encoder_get_position(ENC5))
#else
#define ENC5_INDEX_EVENT() encoder_record_index_reference(ENC5, encoder_get_position(ENC5))
#endif
#endif
#if ENC6_VIRTUAL_INDEX
#undef ENC6_INDEX_EVENT
#ifdef ENC6_INDEX_RESET_POSITION
#define ENC6_INDEX_EVENT() encoder_set_position_from_current_read_once(ENC6, ENC6_INDEX_RESET_POSITION);encoder_record_index_reference(ENC6, encoder_get_position(ENC6))
#else
#define ENC6_INDEX_EVENT() encoder_record_index_reference(ENC6, encoder_get_position(ENC6))
#endif
#endif
#if ENC7_VIRTUAL_INDEX
#undef ENC7_INDEX_EVENT
#ifdef ENC7_INDEX_RESET_POSITION
#define ENC7_INDEX_EVENT() encoder_set_position_from_current_read_once(ENC7, ENC7_INDEX_RESET_POSITION);encoder_record_index_reference(ENC7, encoder_get_position(ENC7))
#else
#define ENC7_INDEX_EVENT() encoder_record_index_reference(ENC7, encoder_get_position(ENC7))
#endif
#endif


#else

void encoder_invoke_index(uint8_t i)
{
	switch (i)
	{
#if ENCODERS > 0 && (defined(ENC0_INDEX) || ENC0_VIRTUAL_INDEX)
	case ENC0:
		HOOK_INVOKE(enc0_index);
		break;
#endif
#if ENCODERS > 1 && (defined(ENC1_INDEX) || ENC1_VIRTUAL_INDEX)
	case ENC1:
		HOOK_INVOKE(enc1_index);
		break;
#endif
#if ENCODERS > 2 && (defined(ENC2_INDEX) || ENC2_VIRTUAL_INDEX)
	case ENC2:
		HOOK_INVOKE(enc2_index);
		break;
#endif
#if ENCODERS > 3 && (defined(ENC3_INDEX) || ENC3_VIRTUAL_INDEX)
	case ENC3:
		HOOK_INVOKE(enc3_index);
		break;
#endif
#if ENCODERS > 4 && (defined(ENC4_INDEX) || ENC4_VIRTUAL_INDEX)
	case ENC4:
		HOOK_INVOKE(enc4_index);
		break;
#endif
#if ENCODERS > 5 && (defined(ENC5_INDEX) || ENC5_VIRTUAL_INDEX)
	case ENC5:
		HOOK_INVOKE(enc5_index);
		break;
#endif
#if ENCODERS > 6 && (defined(ENC6_INDEX) || ENC6_VIRTUAL_INDEX)
	case ENC6:
		HOOK_INVOKE(enc6_index);
		break;
#endif
#if ENCODERS > 7 && (defined(ENC7_INDEX) || ENC7_VIRTUAL_INDEX)
	case ENC7:
		HOOK_INVOKE(enc7_index);
		break;
#endif
	default:
		break;
	}
}

void encoder_record_index_reference(uint8_t i, int32_t position)
{
	(void)position;
	encoder_invoke_index(i);
}

void encoder_virtual_index_clear(uint8_t i)
{
	(void)i;
}

void encoder_virtual_index_update(uint8_t i)
{
	(void)i;
}

bool encoder_get_index_stats(uint8_t i, int32_t *last, int32_t *min, int32_t *max, uint32_t *count)
{
	(void)i;
	(void)last;
	(void)min;
	(void)max;
	(void)count;
	return false;
}

bool encoder_get_index_live_delta(uint8_t i, int32_t *delta)
{
	(void)i;
	(void)delta;
	return false;
}

bool encoder_get_index_debug_line(uint8_t i, char *line, uint32_t line_len, uint32_t *seq)
{
	(void)i;
	(void)line;
	(void)line_len;
	(void)seq;
	return false;
}

#endif

/**
 * Updates pulse encoder types
 */
#if ((ENCODERS_IO_MASK | ENCODERS_INDEX_IO_MASK) != 0)

static FORCEINLINE uint8_t encoder_read_dirs(void)
{
	uint8_t value = 0;
#if ENCODERS > 0 && (ENC0_PULSE != ENC0_DIR)
	value |= ((io_get_input(ENC0_DIR)) ? ENC0_IO_MASK : 0);
#endif
#if ENCODERS > 1 && (ENC1_PULSE != ENC1_DIR)
	value |= ((io_get_input(ENC1_DIR)) ? ENC1_IO_MASK : 0);
#endif
#if ENCODERS > 2 && (ENC2_PULSE != ENC2_DIR)
	value |= ((io_get_input(ENC2_DIR)) ? ENC2_IO_MASK : 0);
#endif
#if ENCODERS > 3 && (ENC3_PULSE != ENC3_DIR)
	value |= ((io_get_input(ENC3_DIR)) ? ENC3_IO_MASK : 0);
#endif
#if ENCODERS > 4 && (ENC4_PULSE != ENC4_DIR)
	value |= ((io_get_input(ENC4_DIR)) ? ENC4_IO_MASK : 0);
#endif
#if ENCODERS > 5 && (ENC5_PULSE != ENC5_DIR)
	value |= ((io_get_input(ENC5_DIR)) ? ENC5_IO_MASK : 0);
#endif
#if ENCODERS > 6 && (ENC6_PULSE != ENC6_DIR)
	value |= ((io_get_input(ENC6_DIR)) ? ENC6_IO_MASK : 0);
#endif
#if ENCODERS > 7 && (ENC7_PULSE != ENC7_DIR)
	value |= ((io_get_input(ENC7_DIR)) ? ENC7_IO_MASK : 0);
#endif
	return value ^ g_settings.encoders_dir_invert_mask;
}

void encoders_update(uint8_t pulse, uint8_t diff)
{
#if ((ENC0_PULSE != ENC0_DIR) || (ENC1_PULSE != ENC1_DIR) || (ENC2_PULSE != ENC2_DIR) || (ENC3_PULSE != ENC3_DIR) || (ENC4_PULSE != ENC4_DIR) || (ENC5_PULSE != ENC5_DIR) || (ENC6_PULSE != ENC6_DIR) || (ENC7_PULSE != ENC7_DIR))
	uint8_t dir = encoder_read_dirs();
#endif

	uint32_t micros = mcu_micros();
	// leave only those that transitioned from off to on
	diff &= pulse;

// checks if pulse pin changed state and is logical 1
#if ENCODERS > 0
	if ((diff & ENC0_IO_MASK))
	{
#if (ENC0_PULSE != ENC0_DIR)
		encoders_pos[ENC0] += (dir & ENC0_IO_MASK) ? 1 : -1;
#else
		encoders_pos[ENC0]++;
		encoders_tstamp[ENC0][1] = encoders_tstamp[ENC0][0];
		encoders_tstamp[ENC0][0] = micros;
#endif
		enc0_pulse();
#if (defined(ENC0_INDEX) && !ENC0_VIRTUAL_INDEX_ONLY && (ENC0_INDEX_IO_MASK == 0))
		if (io_get_input(ENC0_INDEX))
		{
			ENC0_INDEX_EVENT();
		}
#endif
	}
#if (defined(ENC0_INDEX) && !ENC0_VIRTUAL_INDEX_ONLY && (ENC0_INDEX_IO_MASK != 0))
	if ((diff & ENC0_INDEX_IO_MASK))
	{
		ENC0_INDEX_EVENT();
	}
#endif
#endif
#if ENCODERS > 1
	if ((diff & ENC1_IO_MASK))
	{
#if (ENC1_PULSE != ENC1_DIR)
		encoders_pos[ENC1] += (dir & ENC1_IO_MASK) ? 1 : -1;
#else
		encoders_pos[ENC1]++;
		encoders_tstamp[ENC1][1] = encoders_tstamp[ENC1][0];
		encoders_tstamp[ENC1][0] = micros;
#endif
		enc1_pulse();
#if (defined(ENC1_INDEX) && !ENC1_VIRTUAL_INDEX_ONLY && (ENC1_INDEX_IO_MASK == 0))
		if (io_get_input(ENC1_INDEX))
		{
			ENC1_INDEX_EVENT();
		}
#endif
	}
#if (defined(ENC1_INDEX) && !ENC1_VIRTUAL_INDEX_ONLY && (ENC1_INDEX_IO_MASK != 0))
	if ((diff & ENC1_INDEX_IO_MASK))
	{
		ENC1_INDEX_EVENT();
	}
#endif
#endif
#if ENCODERS > 2
	if ((diff & ENC2_IO_MASK))
	{
#if (ENC2_PULSE != ENC2_DIR)
		encoders_pos[ENC2] += (dir & ENC2_IO_MASK) ? 1 : -1;
#else
		encoders_pos[ENC2]++;
		encoders_tstamp[ENC2][1] = encoders_tstamp[ENC2][0];
		encoders_tstamp[ENC2][0] = micros;
#endif
		enc2_pulse();
#if (defined(ENC2_INDEX) && !ENC2_VIRTUAL_INDEX_ONLY && (ENC2_INDEX_IO_MASK == 0))
		if (io_get_input(ENC2_INDEX))
		{
			ENC2_INDEX_EVENT();
		}
#endif
	}
#if (defined(ENC2_INDEX) && !ENC2_VIRTUAL_INDEX_ONLY && (ENC2_INDEX_IO_MASK != 0))
	if ((diff & ENC2_INDEX_IO_MASK))
	{
		ENC2_INDEX_EVENT();
	}
#endif
#endif
#if ENCODERS > 3
	if ((diff & ENC3_IO_MASK))
	{
#if (ENC3_PULSE != ENC3_DIR)
		encoders_pos[ENC3] += (dir & ENC3_IO_MASK) ? 1 : -1;
#else
		encoders_pos[ENC3]++;
		encoders_tstamp[ENC3][1] = encoders_tstamp[ENC3][0];
		encoders_tstamp[ENC3][0] = micros;
#endif
		enc3_pulse();
#if (defined(ENC3_INDEX) && !ENC3_VIRTUAL_INDEX_ONLY && (ENC3_INDEX_IO_MASK == 0))
		if (io_get_input(ENC3_INDEX))
		{
			ENC3_INDEX_EVENT();
		}
#endif
	}
#if (defined(ENC3_INDEX) && !ENC3_VIRTUAL_INDEX_ONLY && (ENC3_INDEX_IO_MASK != 0))
	if ((diff & ENC3_INDEX_IO_MASK))
	{
		ENC3_INDEX_EVENT();
	}
#endif
#endif
#if ENCODERS > 4
	if ((diff & ENC4_IO_MASK))
	{
#if (ENC4_PULSE != ENC4_DIR)
		encoders_pos[ENC4] += (dir & ENC4_IO_MASK) ? 1 : -1;
#else
		encoders_pos[ENC4]++;
		encoders_tstamp[ENC4][1] = encoders_tstamp[ENC4][0];
		encoders_tstamp[ENC4][0] = micros;
#endif
		enc4_pulse();
#if (defined(ENC4_INDEX) && !ENC4_VIRTUAL_INDEX_ONLY && (ENC4_INDEX_IO_MASK == 0))
		if (io_get_input(ENC4_INDEX))
		{
			ENC4_INDEX_EVENT();
		}
#endif
	}
#if (defined(ENC4_INDEX) && !ENC4_VIRTUAL_INDEX_ONLY && (ENC4_INDEX_IO_MASK != 0))
	if ((diff & ENC4_INDEX_IO_MASK))
	{
		ENC4_INDEX_EVENT();
	}
#endif
#endif
#if ENCODERS > 5
	if ((diff & ENC5_IO_MASK))
	{
#if (ENC5_PULSE != ENC5_DIR)
		encoders_pos[ENC5] += (dir & ENC5_IO_MASK) ? 1 : -1;
#else
		encoders_pos[ENC5]++;
		encoders_tstamp[ENC5][1] = encoders_tstamp[ENC5][0];
		encoders_tstamp[ENC5][0] = micros;
#endif
		enc5_pulse();
#if (defined(ENC5_INDEX) && !ENC5_VIRTUAL_INDEX_ONLY && (ENC5_INDEX_IO_MASK == 0))
		if (io_get_input(ENC5_INDEX))
		{
			ENC5_INDEX_EVENT();
		}
#endif
	}
#if (defined(ENC5_INDEX) && !ENC5_VIRTUAL_INDEX_ONLY && (ENC5_INDEX_IO_MASK != 0))
	if ((diff & ENC5_INDEX_IO_MASK))
	{
		ENC5_INDEX_EVENT();
	}
#endif
#endif
#if ENCODERS > 6
	if ((diff & ENC6_IO_MASK))
	{
#if (ENC6_PULSE != ENC6_DIR)
		encoders_pos[ENC6] += (dir & ENC6_IO_MASK) ? 1 : -1;
#else
		encoders_pos[ENC6]++;
		encoders_tstamp[ENC6][1] = encoders_tstamp[ENC6][0];
		encoders_tstamp[ENC6][0] = micros;
#endif
		enc6_pulse();
#if (defined(ENC6_INDEX) && !ENC6_VIRTUAL_INDEX_ONLY && (ENC6_INDEX_IO_MASK == 0))
		if (io_get_input(ENC6_INDEX))
		{
			ENC6_INDEX_EVENT();
		}
#endif
	}
#if (defined(ENC6_INDEX) && !ENC6_VIRTUAL_INDEX_ONLY && (ENC6_INDEX_IO_MASK != 0))
	if ((diff & ENC6_INDEX_IO_MASK))
	{
		ENC6_INDEX_EVENT();
	}
#endif
#endif
#if ENCODERS > 7
	if ((diff & ENC7_IO_MASK))
	{
#if (ENC7_PULSE != ENC7_DIR)
		encoders_pos[ENC7] += (dir & ENC7_IO_MASK) ? 1 : -1;
#else
		encoders_pos[ENC7]++;
		encoders_tstamp[ENC7][1] = encoders_tstamp[ENC7][0];
		encoders_tstamp[ENC7][0] = micros;
#endif
		enc7_pulse();
#if (defined(ENC7_INDEX) && !ENC7_VIRTUAL_INDEX_ONLY && (ENC7_INDEX_IO_MASK == 0))
		if (io_get_input(ENC7_INDEX))
		{
			ENC7_INDEX_EVENT();
		}
#endif
	}
#if (defined(ENC7_INDEX) && !ENC7_VIRTUAL_INDEX_ONLY && (ENC7_INDEX_IO_MASK != 0))
	if ((diff & ENC7_INDEX_IO_MASK))
	{
		ENC7_INDEX_EVENT();
	}
#endif
#endif
}
#else
void encoders_update(uint8_t pulse, uint8_t diff) {}
#endif

#if defined(ENC0_READ) || defined(ENC1_READ) || defined(ENC2_READ) || defined(ENC3_READ) || defined(ENC4_READ) || defined(ENC5_READ) || defined(ENC6_READ) || defined(ENC7_READ)
// static uint16_t encoder_last_read[ENCODERS];
static bool encoder_apply_configured_wrap(uint8_t i, int32_t *diff)
{
	int32_t wrap = 0;
	bool no_wrap_correction = false;

	switch (i)
	{
#if defined(ENC0_READ_WRAP) || defined(ENC0_NO_WRAP_CORRECTION)
	case ENC0:
#ifdef ENC0_READ_WRAP
		wrap = (int32_t)ENC0_READ_WRAP;
#endif
#ifdef ENC0_NO_WRAP_CORRECTION
		no_wrap_correction = true;
#endif
		break;
#endif
#if defined(ENC1_READ_WRAP) || defined(ENC1_NO_WRAP_CORRECTION)
	case ENC1:
#ifdef ENC1_READ_WRAP
		wrap = (int32_t)ENC1_READ_WRAP;
#endif
#ifdef ENC1_NO_WRAP_CORRECTION
		no_wrap_correction = true;
#endif
		break;
#endif
#if defined(ENC2_READ_WRAP) || defined(ENC2_NO_WRAP_CORRECTION)
	case ENC2:
#ifdef ENC2_READ_WRAP
		wrap = (int32_t)ENC2_READ_WRAP;
#endif
#ifdef ENC2_NO_WRAP_CORRECTION
		no_wrap_correction = true;
#endif
		break;
#endif
#if defined(ENC3_READ_WRAP) || defined(ENC3_NO_WRAP_CORRECTION)
	case ENC3:
#ifdef ENC3_READ_WRAP
		wrap = (int32_t)ENC3_READ_WRAP;
#endif
#ifdef ENC3_NO_WRAP_CORRECTION
		no_wrap_correction = true;
#endif
		break;
#endif
#if defined(ENC4_READ_WRAP) || defined(ENC4_NO_WRAP_CORRECTION)
	case ENC4:
#ifdef ENC4_READ_WRAP
		wrap = (int32_t)ENC4_READ_WRAP;
#endif
#ifdef ENC4_NO_WRAP_CORRECTION
		no_wrap_correction = true;
#endif
		break;
#endif
#if defined(ENC5_READ_WRAP) || defined(ENC5_NO_WRAP_CORRECTION)
	case ENC5:
#ifdef ENC5_READ_WRAP
		wrap = (int32_t)ENC5_READ_WRAP;
#endif
#ifdef ENC5_NO_WRAP_CORRECTION
		no_wrap_correction = true;
#endif
		break;
#endif
#if defined(ENC6_READ_WRAP) || defined(ENC6_NO_WRAP_CORRECTION)
	case ENC6:
#ifdef ENC6_READ_WRAP
		wrap = (int32_t)ENC6_READ_WRAP;
#endif
#ifdef ENC6_NO_WRAP_CORRECTION
		no_wrap_correction = true;
#endif
		break;
#endif
#if defined(ENC7_READ_WRAP) || defined(ENC7_NO_WRAP_CORRECTION)
	case ENC7:
#ifdef ENC7_READ_WRAP
		wrap = (int32_t)ENC7_READ_WRAP;
#endif
#ifdef ENC7_NO_WRAP_CORRECTION
		no_wrap_correction = true;
#endif
		break;
#endif
	default:
		break;
	}

	if (wrap > 0)
	{
		int32_t half_wrap = wrap / 2;
		if (*diff < -half_wrap)
		{
			*diff += wrap;
		}
		else if (*diff > half_wrap)
		{
			*diff -= wrap;
		}
		return false;
	}

	return !no_wrap_correction;
}

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

		bool do_wrap_correction = encoder_apply_configured_wrap(i, &diff);

		if (do_wrap_correction && g_settings.encoders_resolution[i] > 0)
		{
			float half_res = g_settings.encoders_resolution[i] * 0.5f;

			if ((float)diff < -half_res)
				diff += (int32_t)g_settings.encoders_resolution[i];

			if ((float)diff > half_res)
				diff -= (int32_t)g_settings.encoders_resolution[i];
		}

		encoders_pos[i] += diff;

		uint32_t now = mcu_micros();
		uint32_t elapsed = now - encoder_rpm_tstamp[i];
		if (!encoder_rpm_tstamp[i])
		{
			encoder_rpm_tstamp[i] = now;
		}
		else
		{
			encoder_rpm_accum[i] += ABS(diff);
			if (elapsed >= ENCODER_RPM_SAMPLE_US)
			{
				if (encoder_rpm_accum[i] && g_settings.encoders_resolution[i])
				{
					uint64_t rpm = ((uint64_t)encoder_rpm_accum[i] * 60000000ULL);
					rpm /= elapsed;
					rpm /= g_settings.encoders_resolution[i];
					encoders_rpm[i] = (uint16_t)MIN(rpm, 65535);
				}
				else
				{
					encoders_rpm[i] = 0;
				}
				encoder_rpm_accum[i] = 0;
				encoder_rpm_tstamp[i] = now;
			}
		}
	}
	else
	{
		encoders_pos[i] = encoder_read;
	}
}

#ifdef ENABLE_MAIN_LOOP_MODULES
bool encoders_dotasks(void *args)
{
#ifdef ENC0_READ
	encoder_update(ENC0);
#endif
#ifdef ENC1_READ
	encoder_update(ENC1);
#endif
#ifdef ENC2_READ
	encoder_update(ENC2);
#endif
#ifdef ENC3_READ
	encoder_update(ENC3);
#endif
#ifdef ENC4_READ
	encoder_update(ENC4);
#endif
#ifdef ENC5_READ
	encoder_update(ENC5);
#endif
#ifdef ENC6_READ
	encoder_update(ENC6);
#endif
#ifdef ENC7_READ
	encoder_update(ENC7);
#endif
#ifdef ENCODER_DEBUG_PRINT_100MS
	static uint32_t enc_dbg_ms = 0;
	uint32_t now_ms = mcu_millis();
	if ((now_ms - enc_dbg_ms) >= 100)
	{
		enc_dbg_ms = now_ms;
		encoder_print_values();
	}
#endif

	return EVENT_CONTINUE;
}
CREATE_EVENT_LISTENER(cnc_io_dotasks, encoders_dotasks);
#endif
#endif

#if ENCODER_HAS_VIRTUAL_INDEX
static bool encoders_virtual_index_dotasks(void *args)
{
	(void)args;
#if ENCODERS > 0 && ENC0_VIRTUAL_INDEX
	encoder_virtual_index_update(ENC0);
#endif
#if ENCODERS > 1 && ENC1_VIRTUAL_INDEX
	encoder_virtual_index_update(ENC1);
#endif
#if ENCODERS > 2 && ENC2_VIRTUAL_INDEX
	encoder_virtual_index_update(ENC2);
#endif
#if ENCODERS > 3 && ENC3_VIRTUAL_INDEX
	encoder_virtual_index_update(ENC3);
#endif
#if ENCODERS > 4 && ENC4_VIRTUAL_INDEX
	encoder_virtual_index_update(ENC4);
#endif
#if ENCODERS > 5 && ENC5_VIRTUAL_INDEX
	encoder_virtual_index_update(ENC5);
#endif
#if ENCODERS > 6 && ENC6_VIRTUAL_INDEX
	encoder_virtual_index_update(ENC6);
#endif
#if ENCODERS > 7 && ENC7_VIRTUAL_INDEX
	encoder_virtual_index_update(ENC7);
#endif
	return EVENT_CONTINUE;
}
CREATE_EVENT_LISTENER(cnc_io_dotasks, encoders_virtual_index_dotasks);
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
	proto_printf("[EC:%ld RPM:%u]" MSG_FEEDBACK_END, (long)encoder_get_position(ENC0), encoder_get_rpm(ENC0));
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

#if ENCODER_HAS_VIRTUAL_INDEX
#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_io_dotasks, encoders_virtual_index_dotasks);
#else
#warning "ENCx_VIRTUAL_INDEX requires ENABLE_MAIN_LOOP_MODULES"
#endif
#endif
}

// allow custom encoder implementations
int32_t __attribute__((weak)) enc_custom_read(uint8_t i) { return 0; }

#endif
