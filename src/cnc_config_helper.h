/*
	Name: cnc_config_helper.h
	Description: Compile time configurations for µCNC. This file takes care of some final configuration definitions based on the user options

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 16-07-2021

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef CNC_CONFIG_HELPER_H
#define CNC_CONFIG_HELPER_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef DISABLE_ALL_CONTROLS
#ifdef ESTOP
#undef ESTOP
#endif
#ifdef FHOLD
#undef FHOLD
#endif
#ifdef CS_RES
#undef CS_RES
#endif
#ifdef SAFETY_DOOR
#undef SAFETY_DOOR
#endif
#endif

#ifdef DISABLE_ALL_LIMITS
#ifdef LIMIT_X
#undef LIMIT_X
#endif
#ifdef LIMIT_X2
#undef LIMIT_X2
#endif
#ifdef LIMIT_Y
#undef LIMIT_Y
#endif
#ifdef LIMIT_Y2
#undef LIMIT_Y2
#endif
#ifdef LIMIT_Z
#undef LIMIT_Z
#endif
#ifdef LIMIT_Z2
#undef LIMIT_Z2
#endif
#ifdef LIMIT_A
#undef LIMIT_A
#endif
#ifdef LIMIT_B
#undef LIMIT_B
#endif
#ifdef LIMIT_C
#undef LIMIT_C
#endif
#endif

#ifdef DISABLE_PROBE
#ifdef PROBE
#undef PROBE
#endif
#endif

#if STEPPER_COUNT < 1
#ifdef STEP0
#undef STEP0
#endif
#ifdef DIR0
#undef DIR0
#endif
#endif
#if STEPPER_COUNT < 2
#ifdef STEP1
#undef STEP1
#endif
#ifdef DIR1
#undef DIR1
#endif
#endif
#if STEPPER_COUNT < 3
#ifdef STEP2
#undef STEP2
#endif
#ifdef DIR2
#undef DIR2
#endif
#endif
#if STEPPER_COUNT < 4
#ifdef STEP3
#undef STEP3
#endif
#ifdef DIR3
#undef DIR3
#endif
#endif
#if STEPPER_COUNT < 5
#ifdef STEP4
#undef STEP4
#endif
#ifdef DIR4
#undef DIR4
#endif
#endif
#if STEPPER_COUNT < 6
#ifdef STEP5
#undef STEP5
#endif
#ifdef DIR5
#undef DIR5
#endif
#endif

/**
 * sets all undefined pins
 **/
#ifndef STEP0
#define STEP0 -1
#define DIO0 -1
#endif
#ifndef STEP1
#define STEP1 -1
#define DIO1 -1
#endif
#ifndef STEP2
#define STEP2 -1
#define DIO2 -1
#endif
#ifndef STEP3
#define STEP3 -1
#define DIO3 -1
#endif
#ifndef STEP4
#define STEP4 -1
#define DIO4 -1
#endif
#ifndef STEP5
#define STEP5 -1
#define DIO5 -1
#endif
#ifndef STEP6
#define STEP6 -1
#define DIO6 -1
#endif
#ifndef STEP7
#define STEP7 -1
#define DIO7 -1
#endif
#ifndef DIR0
#define DIR0 -1
#define DIO8 -1
#endif
#ifndef DIR1
#define DIR1 -1
#define DIO9 -1
#endif
#ifndef DIR2
#define DIR2 -1
#define DIO10 -1
#endif
#ifndef DIR3
#define DIR3 -1
#define DIO11 -1
#endif
#ifndef DIR4
#define DIR4 -1
#define DIO12 -1
#endif
#ifndef DIR5
#define DIR5 -1
#define DIO13 -1
#endif
#ifndef STEP0_EN
#define STEP0_EN -1
#define DIO14 -1
#endif
#ifndef STEP1_EN
#define STEP1_EN -1
#define DIO15 -1
#endif
#ifndef STEP2_EN
#define STEP2_EN -1
#define DIO16 -1
#endif
#ifndef STEP3_EN
#define STEP3_EN -1
#define DIO17 -1
#endif
#ifndef STEP4_EN
#define STEP4_EN -1
#define DIO18 -1
#endif
#ifndef STEP5_EN
#define STEP5_EN -1
#define DIO19 -1
#endif
#ifndef PWM0
#define PWM0 -1
#define DIO20 -1
#endif
#ifndef PWM1
#define PWM1 -1
#define DIO21 -1
#endif
#ifndef PWM2
#define PWM2 -1
#define DIO22 -1
#endif
#ifndef PWM3
#define PWM3 -1
#define DIO23 -1
#endif
#ifndef PWM4
#define PWM4 -1
#define DIO24 -1
#endif
#ifndef PWM5
#define PWM5 -1
#define DIO25 -1
#endif
#ifndef PWM6
#define PWM6 -1
#define DIO26 -1
#endif
#ifndef PWM7
#define PWM7 -1
#define DIO27 -1
#endif
#ifndef PWM8
#define PWM8 -1
#define DIO28 -1
#endif
#ifndef PWM9
#define PWM9 -1
#define DIO29 -1
#endif
#ifndef PWM10
#define PWM10 -1
#define DIO30 -1
#endif
#ifndef PWM11
#define PWM11 -1
#define DIO31 -1
#endif
#ifndef PWM12
#define PWM12 -1
#define DIO32 -1
#endif
#ifndef PWM13
#define PWM13 -1
#define DIO33 -1
#endif
#ifndef PWM14
#define PWM14 -1
#define DIO34 -1
#endif
#ifndef PWM15
#define PWM15 -1
#define DIO35 -1
#endif
#ifndef DOUT0
#define DOUT0 -1
#define DIO36 -1
#endif
#ifndef DOUT1
#define DOUT1 -1
#define DIO37 -1
#endif
#ifndef DOUT2
#define DOUT2 -1
#define DIO38 -1
#endif
#ifndef DOUT3
#define DOUT3 -1
#define DIO39 -1
#endif
#ifndef DOUT4
#define DOUT4 -1
#define DIO40 -1
#endif
#ifndef DOUT5
#define DOUT5 -1
#define DIO41 -1
#endif
#ifndef DOUT6
#define DOUT6 -1
#define DIO42 -1
#endif
#ifndef DOUT7
#define DOUT7 -1
#define DIO43 -1
#endif
#ifndef DOUT8
#define DOUT8 -1
#define DIO44 -1
#endif
#ifndef DOUT9
#define DOUT9 -1
#define DIO45 -1
#endif
#ifndef DOUT10
#define DOUT10 -1
#define DIO46 -1
#endif
#ifndef DOUT11
#define DOUT11 -1
#define DIO47 -1
#endif
#ifndef DOUT12
#define DOUT12 -1
#define DIO48 -1
#endif
#ifndef DOUT13
#define DOUT13 -1
#define DIO49 -1
#endif
#ifndef DOUT14
#define DOUT14 -1
#define DIO50 -1
#endif
#ifndef DOUT15
#define DOUT15 -1
#define DIO51 -1
#endif
#ifndef LIMIT_X
#define LIMIT_X -1
#define DIO52 -1
#endif
#ifndef LIMIT_Y
#define LIMIT_Y -1
#define DIO53 -1
#endif
#ifndef LIMIT_Z
#define LIMIT_Z -1
#define DIO54 -1
#endif
#ifndef LIMIT_X2
#define LIMIT_X2 -1
#define DIO55 -1
#endif
#ifndef LIMIT_Y2
#define LIMIT_Y2 -1
#define DIO56 -1
#endif
#ifndef LIMIT_Z2
#define LIMIT_Z2 -1
#define DIO57 -1
#endif
#ifndef LIMIT_A
#define LIMIT_A -1
#define DIO58 -1
#endif
#ifndef LIMIT_B
#define LIMIT_B -1
#define DIO59 -1
#endif
#ifndef LIMIT_C
#define LIMIT_C -1
#define DIO60 -1
#endif
#ifndef PROBE
#define PROBE -1
#define DIO61 -1
#endif
#ifndef ESTOP
#define ESTOP -1
#define DIO62 -1
#endif
#ifndef SAFETY_DOOR
#define SAFETY_DOOR -1
#define DIO63 -1
#endif
#ifndef FHOLD
#define FHOLD -1
#define DIO64 -1
#endif
#ifndef CS_RES
#define CS_RES -1
#define DIO65 -1
#endif
#ifndef ANALOG0
#define ANALOG0 -1
#define DIO66 -1
#endif
#ifndef ANALOG1
#define ANALOG1 -1
#define DIO67 -1
#endif
#ifndef ANALOG2
#define ANALOG2 -1
#define DIO68 -1
#endif
#ifndef ANALOG3
#define ANALOG3 -1
#define DIO69 -1
#endif
#ifndef ANALOG4
#define ANALOG4 -1
#define DIO70 -1
#endif
#ifndef ANALOG5
#define ANALOG5 -1
#define DIO71 -1
#endif
#ifndef ANALOG6
#define ANALOG6 -1
#define DIO72 -1
#endif
#ifndef ANALOG7
#define ANALOG7 -1
#define DIO73 -1
#endif
#ifndef ANALOG8
#define ANALOG8 -1
#define DIO74 -1
#endif
#ifndef ANALOG9
#define ANALOG9 -1
#define DIO75 -1
#endif
#ifndef ANALOG10
#define ANALOG10 -1
#define DIO76 -1
#endif
#ifndef ANALOG11
#define ANALOG11 -1
#define DIO77 -1
#endif
#ifndef ANALOG12
#define ANALOG12 -1
#define DIO78 -1
#endif
#ifndef ANALOG13
#define ANALOG13 -1
#define DIO79 -1
#endif
#ifndef ANALOG14
#define ANALOG14 -1
#define DIO80 -1
#endif
#ifndef ANALOG15
#define ANALOG15 -1
#define DIO81 -1
#endif
#ifndef DIN0
#define DIN0 -1
#define DIO82 -1
#endif
#ifndef DIN1
#define DIN1 -1
#define DIO83 -1
#endif
#ifndef DIN2
#define DIN2 -1
#define DIO84 -1
#endif
#ifndef DIN3
#define DIN3 -1
#define DIO85 -1
#endif
#ifndef DIN4
#define DIN4 -1
#define DIO86 -1
#endif
#ifndef DIN5
#define DIN5 -1
#define DIO87 -1
#endif
#ifndef DIN6
#define DIN6 -1
#define DIO88 -1
#endif
#ifndef DIN7
#define DIN7 -1
#define DIO89 -1
#endif
#ifndef DIN8
#define DIN8 -1
#define DIO90 -1
#endif
#ifndef DIN9
#define DIN9 -1
#define DIO91 -1
#endif
#ifndef DIN10
#define DIN10 -1
#define DIO92 -1
#endif
#ifndef DIN11
#define DIN11 -1
#define DIO93 -1
#endif
#ifndef DIN12
#define DIN12 -1
#define DIO94 -1
#endif
#ifndef DIN13
#define DIN13 -1
#define DIO95 -1
#endif
#ifndef DIN14
#define DIN14 -1
#define DIO96 -1
#endif
#ifndef DIN15
#define DIN15 -1
#define DIO97 -1
#endif
#ifndef TX
#define TX -1
#define DIO98 -1
#endif
#ifndef RX
#define RX -1
#define DIO99 -1
#endif
#ifndef USB_DM
#define USB_DM -1
#define DIO100 -1
#endif
#ifndef USB_DP
#define USB_DP -1
#define DIO101 -1
#endif


#if (DSS_MAX_OVERSAMPLING < 0 || DSS_MAX_OVERSAMPLING > 3)
#error DSS_MAX_OVERSAMPLING invalid value! Should be set between 0 and 3
#endif

#ifndef BRESENHAM_16BIT
	typedef uint32_t step_t;
#define MAX_STEPS_PER_LINE_BITS (32 - (2 + DSS_MAX_OVERSAMPLING))
#else
typedef uint16_t step_t;
#define MAX_STEPS_PER_LINE_BITS (16 - (2 + DSS_MAX_OVERSAMPLING))
#endif
#define MAX_STEPS_PER_LINE (1 << MAX_STEPS_PER_LINE_BITS)

#ifdef __cplusplus
}
#endif

#endif
