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
#ifdef DIO0
#undef DIO0
#endif
#define DIO0 -1
#endif
#ifndef STEP1
#define STEP1 -1
#ifdef DIO1
#undef DIO1
#endif
#define DIO1 -1
#endif
#ifndef STEP2
#define STEP2 -1
#ifdef DIO2
#undef DIO2
#endif
#define DIO2 -1
#endif
#ifndef STEP3
#define STEP3 -1
#ifdef DIO3
#undef DIO3
#endif
#define DIO3 -1
#endif
#ifndef STEP4
#define STEP4 -1
#ifdef DIO4
#undef DIO4
#endif
#define DIO4 -1
#endif
#ifndef STEP5
#define STEP5 -1
#ifdef DIO5
#undef DIO5
#endif
#define DIO5 -1
#endif
#ifndef STEP6
#define STEP6 -1
#ifdef DIO6
#undef DIO6
#endif
#define DIO6 -1
#endif
#ifndef STEP7
#define STEP7 -1
#ifdef DIO7
#undef DIO7
#endif
#define DIO7 -1
#endif
#ifndef DIR0
#define DIR0 -1
#ifdef DIO8
#undef DIO8
#endif
#define DIO8 -1
#endif
#ifndef DIR1
#define DIR1 -1
#ifdef DIO9
#undef DIO9
#endif
#define DIO9 -1
#endif
#ifndef DIR2
#define DIR2 -1
#ifdef DIO10
#undef DIO10
#endif
#define DIO10 -1
#endif
#ifndef DIR3
#define DIR3 -1
#ifdef DIO11
#undef DIO11
#endif
#define DIO11 -1
#endif
#ifndef DIR4
#define DIR4 -1
#ifdef DIO12
#undef DIO12
#endif
#define DIO12 -1
#endif
#ifndef DIR5
#define DIR5 -1
#ifdef DIO13
#undef DIO13
#endif
#define DIO13 -1
#endif
#ifndef STEP0_EN
#define STEP0_EN -1
#ifdef DIO14
#undef DIO14
#endif
#define DIO14 -1
#endif
#ifndef STEP1_EN
#define STEP1_EN -1
#ifdef DIO15
#undef DIO15
#endif
#define DIO15 -1
#endif
#ifndef STEP2_EN
#define STEP2_EN -1
#ifdef DIO16
#undef DIO16
#endif
#define DIO16 -1
#endif
#ifndef STEP3_EN
#define STEP3_EN -1
#ifdef DIO17
#undef DIO17
#endif
#define DIO17 -1
#endif
#ifndef STEP4_EN
#define STEP4_EN -1
#ifdef DIO18
#undef DIO18
#endif
#define DIO18 -1
#endif
#ifndef STEP5_EN
#define STEP5_EN -1
#ifdef DIO19
#undef DIO19
#endif
#define DIO19 -1
#endif
#ifndef PWM0
#define PWM0 -1
#ifdef DIO20
#undef DIO20
#endif
#define DIO20 -1
#endif
#ifndef PWM1
#define PWM1 -1
#ifdef DIO21
#undef DIO21
#endif
#define DIO21 -1
#endif
#ifndef PWM2
#define PWM2 -1
#ifdef DIO22
#undef DIO22
#endif
#define DIO22 -1
#endif
#ifndef PWM3
#define PWM3 -1
#ifdef DIO23
#undef DIO23
#endif
#define DIO23 -1
#endif
#ifndef PWM4
#define PWM4 -1
#ifdef DIO24
#undef DIO24
#endif
#define DIO24 -1
#endif
#ifndef PWM5
#define PWM5 -1
#ifdef DIO25
#undef DIO25
#endif
#define DIO25 -1
#endif
#ifndef PWM6
#define PWM6 -1
#ifdef DIO26
#undef DIO26
#endif
#define DIO26 -1
#endif
#ifndef PWM7
#define PWM7 -1
#ifdef DIO27
#undef DIO27
#endif
#define DIO27 -1
#endif
#ifndef PWM8
#define PWM8 -1
#ifdef DIO28
#undef DIO28
#endif
#define DIO28 -1
#endif
#ifndef PWM9
#define PWM9 -1
#ifdef DIO29
#undef DIO29
#endif
#define DIO29 -1
#endif
#ifndef PWM10
#define PWM10 -1
#ifdef DIO30
#undef DIO30
#endif
#define DIO30 -1
#endif
#ifndef PWM11
#define PWM11 -1
#ifdef DIO31
#undef DIO31
#endif
#define DIO31 -1
#endif
#ifndef PWM12
#define PWM12 -1
#ifdef DIO32
#undef DIO32
#endif
#define DIO32 -1
#endif
#ifndef PWM13
#define PWM13 -1
#ifdef DIO33
#undef DIO33
#endif
#define DIO33 -1
#endif
#ifndef PWM14
#define PWM14 -1
#ifdef DIO34
#undef DIO34
#endif
#define DIO34 -1
#endif
#ifndef PWM15
#define PWM15 -1
#ifdef DIO35
#undef DIO35
#endif
#define DIO35 -1
#endif
#ifndef DOUT0
#define DOUT0 -1
#ifdef DIO36
#undef DIO36
#endif
#define DIO36 -1
#endif
#ifndef DOUT1
#define DOUT1 -1
#ifdef DIO37
#undef DIO37
#endif
#define DIO37 -1
#endif
#ifndef DOUT2
#define DOUT2 -1
#ifdef DIO38
#undef DIO38
#endif
#define DIO38 -1
#endif
#ifndef DOUT3
#define DOUT3 -1
#ifdef DIO39
#undef DIO39
#endif
#define DIO39 -1
#endif
#ifndef DOUT4
#define DOUT4 -1
#ifdef DIO40
#undef DIO40
#endif
#define DIO40 -1
#endif
#ifndef DOUT5
#define DOUT5 -1
#ifdef DIO41
#undef DIO41
#endif
#define DIO41 -1
#endif
#ifndef DOUT6
#define DOUT6 -1
#ifdef DIO42
#undef DIO42
#endif
#define DIO42 -1
#endif
#ifndef DOUT7
#define DOUT7 -1
#ifdef DIO43
#undef DIO43
#endif
#define DIO43 -1
#endif
#ifndef DOUT8
#define DOUT8 -1
#ifdef DIO44
#undef DIO44
#endif
#define DIO44 -1
#endif
#ifndef DOUT9
#define DOUT9 -1
#ifdef DIO45
#undef DIO45
#endif
#define DIO45 -1
#endif
#ifndef DOUT10
#define DOUT10 -1
#ifdef DIO46
#undef DIO46
#endif
#define DIO46 -1
#endif
#ifndef DOUT11
#define DOUT11 -1
#ifdef DIO47
#undef DIO47
#endif
#define DIO47 -1
#endif
#ifndef DOUT12
#define DOUT12 -1
#ifdef DIO48
#undef DIO48
#endif
#define DIO48 -1
#endif
#ifndef DOUT13
#define DOUT13 -1
#ifdef DIO49
#undef DIO49
#endif
#define DIO49 -1
#endif
#ifndef DOUT14
#define DOUT14 -1
#ifdef DIO50
#undef DIO50
#endif
#define DIO50 -1
#endif
#ifndef DOUT15
#define DOUT15 -1
#ifdef DIO51
#undef DIO51
#endif
#define DIO51 -1
#endif
#ifndef LIMIT_X
#define LIMIT_X -1
#ifdef DIO52
#undef DIO52
#endif
#define DIO52 -1
#endif
#ifndef LIMIT_Y
#define LIMIT_Y -1
#ifdef DIO53
#undef DIO53
#endif
#define DIO53 -1
#endif
#ifndef LIMIT_Z
#define LIMIT_Z -1
#ifdef DIO54
#undef DIO54
#endif
#define DIO54 -1
#endif
#ifndef LIMIT_X2
#define LIMIT_X2 -1
#ifdef DIO55
#undef DIO55
#endif
#define DIO55 -1
#endif
#ifndef LIMIT_Y2
#define LIMIT_Y2 -1
#ifdef DIO56
#undef DIO56
#endif
#define DIO56 -1
#endif
#ifndef LIMIT_Z2
#define LIMIT_Z2 -1
#ifdef DIO57
#undef DIO57
#endif
#define DIO57 -1
#endif
#ifndef LIMIT_A
#define LIMIT_A -1
#ifdef DIO58
#undef DIO58
#endif
#define DIO58 -1
#endif
#ifndef LIMIT_B
#define LIMIT_B -1
#ifdef DIO59
#undef DIO59
#endif
#define DIO59 -1
#endif
#ifndef LIMIT_C
#define LIMIT_C -1
#ifdef DIO60
#undef DIO60
#endif
#define DIO60 -1
#endif
#ifndef PROBE
#define PROBE -1
#ifdef DIO61
#undef DIO61
#endif
#define DIO61 -1
#endif
#ifndef ESTOP
#define ESTOP -1
#ifdef DIO62
#undef DIO62
#endif
#define DIO62 -1
#endif
#ifndef SAFETY_DOOR
#define SAFETY_DOOR -1
#ifdef DIO63
#undef DIO63
#endif
#define DIO63 -1
#endif
#ifndef FHOLD
#define FHOLD -1
#ifdef DIO64
#undef DIO64
#endif
#define DIO64 -1
#endif
#ifndef CS_RES
#define CS_RES -1
#ifdef DIO65
#undef DIO65
#endif
#define DIO65 -1
#endif
#ifndef ANALOG0
#define ANALOG0 -1
#ifdef DIO66
#undef DIO66
#endif
#define DIO66 -1
#endif
#ifndef ANALOG1
#define ANALOG1 -1
#ifdef DIO67
#undef DIO67
#endif
#define DIO67 -1
#endif
#ifndef ANALOG2
#define ANALOG2 -1
#ifdef DIO68
#undef DIO68
#endif
#define DIO68 -1
#endif
#ifndef ANALOG3
#define ANALOG3 -1
#ifdef DIO69
#undef DIO69
#endif
#define DIO69 -1
#endif
#ifndef ANALOG4
#define ANALOG4 -1
#ifdef DIO70
#undef DIO70
#endif
#define DIO70 -1
#endif
#ifndef ANALOG5
#define ANALOG5 -1
#ifdef DIO71
#undef DIO71
#endif
#define DIO71 -1
#endif
#ifndef ANALOG6
#define ANALOG6 -1
#ifdef DIO72
#undef DIO72
#endif
#define DIO72 -1
#endif
#ifndef ANALOG7
#define ANALOG7 -1
#ifdef DIO73
#undef DIO73
#endif
#define DIO73 -1
#endif
#ifndef ANALOG8
#define ANALOG8 -1
#ifdef DIO74
#undef DIO74
#endif
#define DIO74 -1
#endif
#ifndef ANALOG9
#define ANALOG9 -1
#ifdef DIO75
#undef DIO75
#endif
#define DIO75 -1
#endif
#ifndef ANALOG10
#define ANALOG10 -1
#ifdef DIO76
#undef DIO76
#endif
#define DIO76 -1
#endif
#ifndef ANALOG11
#define ANALOG11 -1
#ifdef DIO77
#undef DIO77
#endif
#define DIO77 -1
#endif
#ifndef ANALOG12
#define ANALOG12 -1
#ifdef DIO78
#undef DIO78
#endif
#define DIO78 -1
#endif
#ifndef ANALOG13
#define ANALOG13 -1
#ifdef DIO79
#undef DIO79
#endif
#define DIO79 -1
#endif
#ifndef ANALOG14
#define ANALOG14 -1
#ifdef DIO80
#undef DIO80
#endif
#define DIO80 -1
#endif
#ifndef ANALOG15
#define ANALOG15 -1
#ifdef DIO81
#undef DIO81
#endif
#define DIO81 -1
#endif
#ifndef DIN0
#define DIN0 -1
#ifdef DIO82
#undef DIO82
#endif
#define DIO82 -1
#endif
#ifndef DIN1
#define DIN1 -1
#ifdef DIO83
#undef DIO83
#endif
#define DIO83 -1
#endif
#ifndef DIN2
#define DIN2 -1
#ifdef DIO84
#undef DIO84
#endif
#define DIO84 -1
#endif
#ifndef DIN3
#define DIN3 -1
#ifdef DIO85
#undef DIO85
#endif
#define DIO85 -1
#endif
#ifndef DIN4
#define DIN4 -1
#ifdef DIO86
#undef DIO86
#endif
#define DIO86 -1
#endif
#ifndef DIN5
#define DIN5 -1
#ifdef DIO87
#undef DIO87
#endif
#define DIO87 -1
#endif
#ifndef DIN6
#define DIN6 -1
#ifdef DIO88
#undef DIO88
#endif
#define DIO88 -1
#endif
#ifndef DIN7
#define DIN7 -1
#ifdef DIO89
#undef DIO89
#endif
#define DIO89 -1
#endif
#ifndef DIN8
#define DIN8 -1
#ifdef DIO90
#undef DIO90
#endif
#define DIO90 -1
#endif
#ifndef DIN9
#define DIN9 -1
#ifdef DIO91
#undef DIO91
#endif
#define DIO91 -1
#endif
#ifndef DIN10
#define DIN10 -1
#ifdef DIO92
#undef DIO92
#endif
#define DIO92 -1
#endif
#ifndef DIN11
#define DIN11 -1
#ifdef DIO93
#undef DIO93
#endif
#define DIO93 -1
#endif
#ifndef DIN12
#define DIN12 -1
#ifdef DIO94
#undef DIO94
#endif
#define DIO94 -1
#endif
#ifndef DIN13
#define DIN13 -1
#ifdef DIO95
#undef DIO95
#endif
#define DIO95 -1
#endif
#ifndef DIN14
#define DIN14 -1
#ifdef DIO96
#undef DIO96
#endif
#define DIO96 -1
#endif
#ifndef DIN15
#define DIN15 -1
#ifdef DIO97
#undef DIO97
#endif
#define DIO97 -1
#endif
#ifndef TX
#define TX -1
#ifdef DIO98
#undef DIO98
#endif
#define DIO98 -1
#endif
#ifndef RX
#define RX -1
#ifdef DIO99
#undef DIO99
#endif
#define DIO99 -1
#endif
#ifndef USB_DM
#define USB_DM -1
#ifdef DIO100
#undef DIO100
#endif
#define DIO100 -1
#endif
#ifndef USB_DP
#define USB_DP -1
#ifdef DIO101
#undef DIO101
#endif
#define DIO101 -1
#endif

#if (INTERFACE < 0 || INTERFACE > 1)
#error "undefined COM interface"
#endif

#if (DSS_MAX_OVERSAMPLING < 0 || DSS_MAX_OVERSAMPLING > 3)
#error DSS_MAX_OVERSAMPLING invalid value! Should be set between 0 and 3
#endif

#ifndef CTRL_SCHED_CHECK
#define CTRL_SCHED_CHECK -1
#else
#if CTRL_SCHED_CHECK > 7
#error CTRL_SCHED_CHECK invalid value! Max is 7
#endif
#define CTRL_SCHED_CHECK_MASK ((1 << (CTRL_SCHED_CHECK + 1)) - 1)
#define CTRL_SCHED_CHECK_VAL (1 << (CTRL_SCHED_CHECK))
#endif

#ifndef BRESENHAM_16BIT
	typedef uint32_t step_t;
#define MAX_STEPS_PER_LINE_BITS (32 - (2 + DSS_MAX_OVERSAMPLING))
#else
typedef uint16_t step_t;
#define MAX_STEPS_PER_LINE_BITS (16 - (2 + DSS_MAX_OVERSAMPLING))
#endif
#define MAX_STEPS_PER_LINE (1 << MAX_STEPS_PER_LINE_BITS)

#if DSS_CUTOFF_FREQ > (F_STEP_MAX >> 3)
#error "DSS_CUTOFF_FREQ should not be set above 1/8th of the max step rate"
#endif

#ifdef __cplusplus
}
#endif

#endif
