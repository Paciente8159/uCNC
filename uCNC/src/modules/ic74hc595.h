/*
	Name: ic74hc595.h
	Description: This module adds the ability to control the IC74HC595 shift register controller (used for example in the MKS-DLC32 board) to µCNC.
				 Up to 56 output pins can be assigned.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 01/09/2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef IC74HC595_H
#define IC74HC595_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

#ifndef STEP0_IO_OFFSET
#define STEP0_IO_OFFSET -1
#else
#ifdef STEP0
#undef STEP0
#endif
#ifdef DIO1
#undef DIO1
#endif
#define STEP0 1
#define DIO1 -1
#define STEP0_IO_BYTEOFFSET (STEP0_IO_OFFSET >> 3)
#define STEP0_IO_BITMASK (1 << (STEP0_IO_OFFSET & 0x7))
#define DIO1_IO_BYTEOFFSET STEP0_IO_BYTEOFFSET
#define DIO1_IO_BITMASK STEP0_IO_BITMASK
#ifndef IC74HC595_HAS_STEPS
#define IC74HC595_HAS_STEPS
#endif
#endif
#ifndef STEP1_IO_OFFSET
#define STEP1_IO_OFFSET -1
#else
#ifdef STEP1
#undef STEP1
#endif
#ifdef DIO2
#undef DIO2
#endif
#define STEP1 2
#define DIO2 -2
#define STEP1_IO_BYTEOFFSET (STEP1_IO_OFFSET >> 3)
#define STEP1_IO_BITMASK (1 << (STEP1_IO_OFFSET & 0x7))
#define DIO2_IO_BYTEOFFSET STEP1_IO_BYTEOFFSET
#define DIO2_IO_BITMASK STEP1_IO_BITMASK
#ifndef IC74HC595_HAS_STEPS
#define IC74HC595_HAS_STEPS
#endif
#endif
#ifndef STEP2_IO_OFFSET
#define STEP2_IO_OFFSET -1
#else
#ifdef STEP2
#undef STEP2
#endif
#ifdef DIO3
#undef DIO3
#endif
#define STEP2 3
#define DIO3 -3
#define STEP2_IO_BYTEOFFSET (STEP2_IO_OFFSET >> 3)
#define STEP2_IO_BITMASK (1 << (STEP2_IO_OFFSET & 0x7))
#define DIO3_IO_BYTEOFFSET STEP2_IO_BYTEOFFSET
#define DIO3_IO_BITMASK STEP2_IO_BITMASK
#ifndef IC74HC595_HAS_STEPS
#define IC74HC595_HAS_STEPS
#endif
#endif
#ifndef STEP3_IO_OFFSET
#define STEP3_IO_OFFSET -1
#else
#ifdef STEP3
#undef STEP3
#endif
#ifdef DIO4
#undef DIO4
#endif
#define STEP3 4
#define DIO4 -4
#define STEP3_IO_BYTEOFFSET (STEP3_IO_OFFSET >> 3)
#define STEP3_IO_BITMASK (1 << (STEP3_IO_OFFSET & 0x7))
#define DIO4_IO_BYTEOFFSET STEP3_IO_BYTEOFFSET
#define DIO4_IO_BITMASK STEP3_IO_BITMASK
#ifndef IC74HC595_HAS_STEPS
#define IC74HC595_HAS_STEPS
#endif
#endif
#ifndef STEP4_IO_OFFSET
#define STEP4_IO_OFFSET -1
#else
#ifdef STEP4
#undef STEP4
#endif
#ifdef DIO5
#undef DIO5
#endif
#define STEP4 5
#define DIO5 -5
#define STEP4_IO_BYTEOFFSET (STEP4_IO_OFFSET >> 3)
#define STEP4_IO_BITMASK (1 << (STEP4_IO_OFFSET & 0x7))
#define DIO5_IO_BYTEOFFSET STEP4_IO_BYTEOFFSET
#define DIO5_IO_BITMASK STEP4_IO_BITMASK
#ifndef IC74HC595_HAS_STEPS
#define IC74HC595_HAS_STEPS
#endif
#endif
#ifndef STEP5_IO_OFFSET
#define STEP5_IO_OFFSET -1
#else
#ifdef STEP5
#undef STEP5
#endif
#ifdef DIO6
#undef DIO6
#endif
#define STEP5 6
#define DIO6 -6
#define STEP5_IO_BYTEOFFSET (STEP5_IO_OFFSET >> 3)
#define STEP5_IO_BITMASK (1 << (STEP5_IO_OFFSET & 0x7))
#define DIO6_IO_BYTEOFFSET STEP5_IO_BYTEOFFSET
#define DIO6_IO_BITMASK STEP5_IO_BITMASK
#ifndef IC74HC595_HAS_STEPS
#define IC74HC595_HAS_STEPS
#endif
#endif
#ifndef STEP6_IO_OFFSET
#define STEP6_IO_OFFSET -1
#else
#ifdef STEP6
#undef STEP6
#endif
#ifdef DIO7
#undef DIO7
#endif
#define STEP6 7
#define DIO7 -7
#define STEP6_IO_BYTEOFFSET (STEP6_IO_OFFSET >> 3)
#define STEP6_IO_BITMASK (1 << (STEP6_IO_OFFSET & 0x7))
#define DIO7_IO_BYTEOFFSET STEP6_IO_BYTEOFFSET
#define DIO7_IO_BITMASK STEP6_IO_BITMASK
#ifndef IC74HC595_HAS_STEPS
#define IC74HC595_HAS_STEPS
#endif
#endif
#ifndef STEP7_IO_OFFSET
#define STEP7_IO_OFFSET -1
#else
#ifdef STEP7
#undef STEP7
#endif
#ifdef DIO8
#undef DIO8
#endif
#define STEP7 8
#define DIO8 -8
#define STEP7_IO_BYTEOFFSET (STEP7_IO_OFFSET >> 3)
#define STEP7_IO_BITMASK (1 << (STEP7_IO_OFFSET & 0x7))
#define DIO8_IO_BYTEOFFSET STEP7_IO_BYTEOFFSET
#define DIO8_IO_BITMASK STEP7_IO_BITMASK
#ifndef IC74HC595_HAS_STEPS
#define IC74HC595_HAS_STEPS
#endif
#endif
#ifndef DIR0_IO_OFFSET
#define DIR0_IO_OFFSET -1
#else
#ifdef DIR0
#undef DIR0
#endif
#ifdef DIO9
#undef DIO9
#endif
#define DIR0 9
#define DIO9 -9
#define DIR0_IO_BYTEOFFSET (DIR0_IO_OFFSET >> 3)
#define DIR0_IO_BITMASK (1 << (DIR0_IO_OFFSET & 0x7))
#define DIO9_IO_BYTEOFFSET DIR0_IO_BYTEOFFSET
#define DIO9_IO_BITMASK DIR0_IO_BITMASK
#ifndef IC74HC595_HAS_DIRS
#define IC74HC595_HAS_DIRS
#endif
#endif
#ifndef DIR1_IO_OFFSET
#define DIR1_IO_OFFSET -1
#else
#ifdef DIR1
#undef DIR1
#endif
#ifdef DIO10
#undef DIO10
#endif
#define DIR1 10
#define DIO10 -10
#define DIR1_IO_BYTEOFFSET (DIR1_IO_OFFSET >> 3)
#define DIR1_IO_BITMASK (1 << (DIR1_IO_OFFSET & 0x7))
#define DIO10_IO_BYTEOFFSET DIR1_IO_BYTEOFFSET
#define DIO10_IO_BITMASK DIR1_IO_BITMASK
#ifndef IC74HC595_HAS_DIRS
#define IC74HC595_HAS_DIRS
#endif
#endif
#ifndef DIR2_IO_OFFSET
#define DIR2_IO_OFFSET -1
#else
#ifdef DIR2
#undef DIR2
#endif
#ifdef DIO11
#undef DIO11
#endif
#define DIR2 11
#define DIO11 -11
#define DIR2_IO_BYTEOFFSET (DIR2_IO_OFFSET >> 3)
#define DIR2_IO_BITMASK (1 << (DIR2_IO_OFFSET & 0x7))
#define DIO11_IO_BYTEOFFSET DIR2_IO_BYTEOFFSET
#define DIO11_IO_BITMASK DIR2_IO_BITMASK
#ifndef IC74HC595_HAS_DIRS
#define IC74HC595_HAS_DIRS
#endif
#endif
#ifndef DIR3_IO_OFFSET
#define DIR3_IO_OFFSET -1
#else
#ifdef DIR3
#undef DIR3
#endif
#ifdef DIO12
#undef DIO12
#endif
#define DIR3 12
#define DIO12 -12
#define DIR3_IO_BYTEOFFSET (DIR3_IO_OFFSET >> 3)
#define DIR3_IO_BITMASK (1 << (DIR3_IO_OFFSET & 0x7))
#define DIO12_IO_BYTEOFFSET DIR3_IO_BYTEOFFSET
#define DIO12_IO_BITMASK DIR3_IO_BITMASK
#ifndef IC74HC595_HAS_DIRS
#define IC74HC595_HAS_DIRS
#endif
#endif
#ifndef DIR4_IO_OFFSET
#define DIR4_IO_OFFSET -1
#else
#ifdef DIR4
#undef DIR4
#endif
#ifdef DIO13
#undef DIO13
#endif
#define DIR4 13
#define DIO13 -13
#define DIR4_IO_BYTEOFFSET (DIR4_IO_OFFSET >> 3)
#define DIR4_IO_BITMASK (1 << (DIR4_IO_OFFSET & 0x7))
#define DIO13_IO_BYTEOFFSET DIR4_IO_BYTEOFFSET
#define DIO13_IO_BITMASK DIR4_IO_BITMASK
#ifndef IC74HC595_HAS_DIRS
#define IC74HC595_HAS_DIRS
#endif
#endif
#ifndef DIR5_IO_OFFSET
#define DIR5_IO_OFFSET -1
#else
#ifdef DIR5
#undef DIR5
#endif
#ifdef DIO14
#undef DIO14
#endif
#define DIR5 14
#define DIO14 -14
#define DIR5_IO_BYTEOFFSET (DIR5_IO_OFFSET >> 3)
#define DIR5_IO_BITMASK (1 << (DIR5_IO_OFFSET & 0x7))
#define DIO14_IO_BYTEOFFSET DIR5_IO_BYTEOFFSET
#define DIO14_IO_BITMASK DIR5_IO_BITMASK
#ifndef IC74HC595_HAS_DIRS
#define IC74HC595_HAS_DIRS
#endif
#endif
#ifndef DIR6_IO_OFFSET
#define DIR6_IO_OFFSET -1
#else
#ifdef DIR6
#undef DIR6
#endif
#ifdef DIO15
#undef DIO15
#endif
#define DIR6 15
#define DIO15 -15
#define DIR6_IO_BYTEOFFSET (DIR6_IO_OFFSET >> 3)
#define DIR6_IO_BITMASK (1 << (DIR6_IO_OFFSET & 0x7))
#define DIO15_IO_BYTEOFFSET DIR6_IO_BYTEOFFSET
#define DIO15_IO_BITMASK DIR6_IO_BITMASK
#ifndef IC74HC595_HAS_DIRS
#define IC74HC595_HAS_DIRS
#endif
#endif
#ifndef DIR7_IO_OFFSET
#define DIR7_IO_OFFSET -1
#else
#ifdef DIR7
#undef DIR7
#endif
#ifdef DIO16
#undef DIO16
#endif
#define DIR7 16
#define DIO16 -16
#define DIR7_IO_BYTEOFFSET (DIR7_IO_OFFSET >> 3)
#define DIR7_IO_BITMASK (1 << (DIR7_IO_OFFSET & 0x7))
#define DIO16_IO_BYTEOFFSET DIR7_IO_BYTEOFFSET
#define DIO16_IO_BITMASK DIR7_IO_BITMASK
#ifndef IC74HC595_HAS_DIRS
#define IC74HC595_HAS_DIRS
#endif
#endif
#ifndef STEP0_EN_IO_OFFSET
#define STEP0_EN_IO_OFFSET -1
#else
#ifdef STEP0_EN
#undef STEP0_EN
#endif
#ifdef DIO17
#undef DIO17
#endif
#define STEP0_EN 17
#define DIO17 -17
#define STEP0_EN_IO_BYTEOFFSET (STEP0_EN_IO_OFFSET >> 3)
#define STEP0_EN_IO_BITMASK (1 << (STEP0_EN_IO_OFFSET & 0x7))
#define DIO17_IO_BYTEOFFSET STEP0_EN_IO_BYTEOFFSET
#define DIO17_IO_BITMASK STEP0_EN_IO_BITMASK
#ifndef IC74HC595_HAS_STEPS_EN
#define IC74HC595_HAS_STEPS_EN
#endif
#endif
#ifndef STEP1_EN_IO_OFFSET
#define STEP1_EN_IO_OFFSET -1
#else
#ifdef STEP1_EN
#undef STEP1_EN
#endif
#ifdef DIO18
#undef DIO18
#endif
#define STEP1_EN 18
#define DIO18 -18
#define STEP1_EN_IO_BYTEOFFSET (STEP1_EN_IO_OFFSET >> 3)
#define STEP1_EN_IO_BITMASK (1 << (STEP1_EN_IO_OFFSET & 0x7))
#define DIO18_IO_BYTEOFFSET STEP1_EN_IO_BYTEOFFSET
#define DIO18_IO_BITMASK STEP1_EN_IO_BITMASK
#ifndef IC74HC595_HAS_STEPS_EN
#define IC74HC595_HAS_STEPS_EN
#endif
#endif
#ifndef STEP2_EN_IO_OFFSET
#define STEP2_EN_IO_OFFSET -1
#else
#ifdef STEP2_EN
#undef STEP2_EN
#endif
#ifdef DIO19
#undef DIO19
#endif
#define STEP2_EN 19
#define DIO19 -19
#define STEP2_EN_IO_BYTEOFFSET (STEP2_EN_IO_OFFSET >> 3)
#define STEP2_EN_IO_BITMASK (1 << (STEP2_EN_IO_OFFSET & 0x7))
#define DIO19_IO_BYTEOFFSET STEP2_EN_IO_BYTEOFFSET
#define DIO19_IO_BITMASK STEP2_EN_IO_BITMASK
#ifndef IC74HC595_HAS_STEPS_EN
#define IC74HC595_HAS_STEPS_EN
#endif
#endif
#ifndef STEP3_EN_IO_OFFSET
#define STEP3_EN_IO_OFFSET -1
#else
#ifdef STEP3_EN
#undef STEP3_EN
#endif
#ifdef DIO20
#undef DIO20
#endif
#define STEP3_EN 20
#define DIO20 -20
#define STEP3_EN_IO_BYTEOFFSET (STEP3_EN_IO_OFFSET >> 3)
#define STEP3_EN_IO_BITMASK (1 << (STEP3_EN_IO_OFFSET & 0x7))
#define DIO20_IO_BYTEOFFSET STEP3_EN_IO_BYTEOFFSET
#define DIO20_IO_BITMASK STEP3_EN_IO_BITMASK
#ifndef IC74HC595_HAS_STEPS_EN
#define IC74HC595_HAS_STEPS_EN
#endif
#endif
#ifndef STEP4_EN_IO_OFFSET
#define STEP4_EN_IO_OFFSET -1
#else
#ifdef STEP4_EN
#undef STEP4_EN
#endif
#ifdef DIO21
#undef DIO21
#endif
#define STEP4_EN 21
#define DIO21 -21
#define STEP4_EN_IO_BYTEOFFSET (STEP4_EN_IO_OFFSET >> 3)
#define STEP4_EN_IO_BITMASK (1 << (STEP4_EN_IO_OFFSET & 0x7))
#define DIO21_IO_BYTEOFFSET STEP4_EN_IO_BYTEOFFSET
#define DIO21_IO_BITMASK STEP4_EN_IO_BITMASK
#ifndef IC74HC595_HAS_STEPS_EN
#define IC74HC595_HAS_STEPS_EN
#endif
#endif
#ifndef STEP5_EN_IO_OFFSET
#define STEP5_EN_IO_OFFSET -1
#else
#ifdef STEP5_EN
#undef STEP5_EN
#endif
#ifdef DIO22
#undef DIO22
#endif
#define STEP5_EN 22
#define DIO22 -22
#define STEP5_EN_IO_BYTEOFFSET (STEP5_EN_IO_OFFSET >> 3)
#define STEP5_EN_IO_BITMASK (1 << (STEP5_EN_IO_OFFSET & 0x7))
#define DIO22_IO_BYTEOFFSET STEP5_EN_IO_BYTEOFFSET
#define DIO22_IO_BITMASK STEP5_EN_IO_BITMASK
#ifndef IC74HC595_HAS_STEPS_EN
#define IC74HC595_HAS_STEPS_EN
#endif
#endif
#ifndef STEP6_EN_IO_OFFSET
#define STEP6_EN_IO_OFFSET -1
#else
#ifdef STEP6_EN
#undef STEP6_EN
#endif
#ifdef DIO23
#undef DIO23
#endif
#define STEP6_EN 23
#define DIO23 -23
#define STEP6_EN_IO_BYTEOFFSET (STEP6_EN_IO_OFFSET >> 3)
#define STEP6_EN_IO_BITMASK (1 << (STEP6_EN_IO_OFFSET & 0x7))
#define DIO23_IO_BYTEOFFSET STEP6_EN_IO_BYTEOFFSET
#define DIO23_IO_BITMASK STEP6_EN_IO_BITMASK
#ifndef IC74HC595_HAS_STEPS_EN
#define IC74HC595_HAS_STEPS_EN
#endif
#endif
#ifndef STEP7_EN_IO_OFFSET
#define STEP7_EN_IO_OFFSET -1
#else
#ifdef STEP7_EN
#undef STEP7_EN
#endif
#ifdef DIO24
#undef DIO24
#endif
#define STEP7_EN 24
#define DIO24 -24
#define STEP7_EN_IO_BYTEOFFSET (STEP7_EN_IO_OFFSET >> 3)
#define STEP7_EN_IO_BITMASK (1 << (STEP7_EN_IO_OFFSET & 0x7))
#define DIO24_IO_BYTEOFFSET STEP7_EN_IO_BYTEOFFSET
#define DIO24_IO_BITMASK STEP7_EN_IO_BITMASK
#ifndef IC74HC595_HAS_STEPS_EN
#define IC74HC595_HAS_STEPS_EN
#endif
#endif
#ifndef PWM0_IO_OFFSET
#define PWM0_IO_OFFSET -1
#else
#ifdef PWM0
#undef PWM0
#endif
#ifdef DIO25
#undef DIO25
#endif
#define PWM0 25
#define DIO25 -25
#define PWM0_IO_BYTEOFFSET (PWM0_IO_OFFSET >> 3)
#define PWM0_IO_BITMASK (1 << (PWM0_IO_OFFSET & 0x7))
#define DIO25_IO_BYTEOFFSET PWM0_IO_BYTEOFFSET
#define DIO25_IO_BITMASK PWM0_IO_BITMASK
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM1_IO_OFFSET
#define PWM1_IO_OFFSET -1
#else
#ifdef PWM1
#undef PWM1
#endif
#ifdef DIO26
#undef DIO26
#endif
#define PWM1 26
#define DIO26 -26
#define PWM1_IO_BYTEOFFSET (PWM1_IO_OFFSET >> 3)
#define PWM1_IO_BITMASK (1 << (PWM1_IO_OFFSET & 0x7))
#define DIO26_IO_BYTEOFFSET PWM1_IO_BYTEOFFSET
#define DIO26_IO_BITMASK PWM1_IO_BITMASK
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM2_IO_OFFSET
#define PWM2_IO_OFFSET -1
#else
#ifdef PWM2
#undef PWM2
#endif
#ifdef DIO27
#undef DIO27
#endif
#define PWM2 27
#define DIO27 -27
#define PWM2_IO_BYTEOFFSET (PWM2_IO_OFFSET >> 3)
#define PWM2_IO_BITMASK (1 << (PWM2_IO_OFFSET & 0x7))
#define DIO27_IO_BYTEOFFSET PWM2_IO_BYTEOFFSET
#define DIO27_IO_BITMASK PWM2_IO_BITMASK
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM3_IO_OFFSET
#define PWM3_IO_OFFSET -1
#else
#ifdef PWM3
#undef PWM3
#endif
#ifdef DIO28
#undef DIO28
#endif
#define PWM3 28
#define DIO28 -28
#define PWM3_IO_BYTEOFFSET (PWM3_IO_OFFSET >> 3)
#define PWM3_IO_BITMASK (1 << (PWM3_IO_OFFSET & 0x7))
#define DIO28_IO_BYTEOFFSET PWM3_IO_BYTEOFFSET
#define DIO28_IO_BITMASK PWM3_IO_BITMASK
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM4_IO_OFFSET
#define PWM4_IO_OFFSET -1
#else
#ifdef PWM4
#undef PWM4
#endif
#ifdef DIO29
#undef DIO29
#endif
#define PWM4 29
#define DIO29 -29
#define PWM4_IO_BYTEOFFSET (PWM4_IO_OFFSET >> 3)
#define PWM4_IO_BITMASK (1 << (PWM4_IO_OFFSET & 0x7))
#define DIO29_IO_BYTEOFFSET PWM4_IO_BYTEOFFSET
#define DIO29_IO_BITMASK PWM4_IO_BITMASK
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM5_IO_OFFSET
#define PWM5_IO_OFFSET -1
#else
#ifdef PWM5
#undef PWM5
#endif
#ifdef DIO30
#undef DIO30
#endif
#define PWM5 30
#define DIO30 -30
#define PWM5_IO_BYTEOFFSET (PWM5_IO_OFFSET >> 3)
#define PWM5_IO_BITMASK (1 << (PWM5_IO_OFFSET & 0x7))
#define DIO30_IO_BYTEOFFSET PWM5_IO_BYTEOFFSET
#define DIO30_IO_BITMASK PWM5_IO_BITMASK
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM6_IO_OFFSET
#define PWM6_IO_OFFSET -1
#else
#ifdef PWM6
#undef PWM6
#endif
#ifdef DIO31
#undef DIO31
#endif
#define PWM6 31
#define DIO31 -31
#define PWM6_IO_BYTEOFFSET (PWM6_IO_OFFSET >> 3)
#define PWM6_IO_BITMASK (1 << (PWM6_IO_OFFSET & 0x7))
#define DIO31_IO_BYTEOFFSET PWM6_IO_BYTEOFFSET
#define DIO31_IO_BITMASK PWM6_IO_BITMASK
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM7_IO_OFFSET
#define PWM7_IO_OFFSET -1
#else
#ifdef PWM7
#undef PWM7
#endif
#ifdef DIO32
#undef DIO32
#endif
#define PWM7 32
#define DIO32 -32
#define PWM7_IO_BYTEOFFSET (PWM7_IO_OFFSET >> 3)
#define PWM7_IO_BITMASK (1 << (PWM7_IO_OFFSET & 0x7))
#define DIO32_IO_BYTEOFFSET PWM7_IO_BYTEOFFSET
#define DIO32_IO_BITMASK PWM7_IO_BITMASK
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM8_IO_OFFSET
#define PWM8_IO_OFFSET -1
#else
#ifdef PWM8
#undef PWM8
#endif
#ifdef DIO33
#undef DIO33
#endif
#define PWM8 33
#define DIO33 -33
#define PWM8_IO_BYTEOFFSET (PWM8_IO_OFFSET >> 3)
#define PWM8_IO_BITMASK (1 << (PWM8_IO_OFFSET & 0x7))
#define DIO33_IO_BYTEOFFSET PWM8_IO_BYTEOFFSET
#define DIO33_IO_BITMASK PWM8_IO_BITMASK
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM9_IO_OFFSET
#define PWM9_IO_OFFSET -1
#else
#ifdef PWM9
#undef PWM9
#endif
#ifdef DIO34
#undef DIO34
#endif
#define PWM9 34
#define DIO34 -34
#define PWM9_IO_BYTEOFFSET (PWM9_IO_OFFSET >> 3)
#define PWM9_IO_BITMASK (1 << (PWM9_IO_OFFSET & 0x7))
#define DIO34_IO_BYTEOFFSET PWM9_IO_BYTEOFFSET
#define DIO34_IO_BITMASK PWM9_IO_BITMASK
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM10_IO_OFFSET
#define PWM10_IO_OFFSET -1
#else
#ifdef PWM10
#undef PWM10
#endif
#ifdef DIO35
#undef DIO35
#endif
#define PWM10 35
#define DIO35 -35
#define PWM10_IO_BYTEOFFSET (PWM10_IO_OFFSET >> 3)
#define PWM10_IO_BITMASK (1 << (PWM10_IO_OFFSET & 0x7))
#define DIO35_IO_BYTEOFFSET PWM10_IO_BYTEOFFSET
#define DIO35_IO_BITMASK PWM10_IO_BITMASK
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM11_IO_OFFSET
#define PWM11_IO_OFFSET -1
#else
#ifdef PWM11
#undef PWM11
#endif
#ifdef DIO36
#undef DIO36
#endif
#define PWM11 36
#define DIO36 -36
#define PWM11_IO_BYTEOFFSET (PWM11_IO_OFFSET >> 3)
#define PWM11_IO_BITMASK (1 << (PWM11_IO_OFFSET & 0x7))
#define DIO36_IO_BYTEOFFSET PWM11_IO_BYTEOFFSET
#define DIO36_IO_BITMASK PWM11_IO_BITMASK
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM12_IO_OFFSET
#define PWM12_IO_OFFSET -1
#else
#ifdef PWM12
#undef PWM12
#endif
#ifdef DIO37
#undef DIO37
#endif
#define PWM12 37
#define DIO37 -37
#define PWM12_IO_BYTEOFFSET (PWM12_IO_OFFSET >> 3)
#define PWM12_IO_BITMASK (1 << (PWM12_IO_OFFSET & 0x7))
#define DIO37_IO_BYTEOFFSET PWM12_IO_BYTEOFFSET
#define DIO37_IO_BITMASK PWM12_IO_BITMASK
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM13_IO_OFFSET
#define PWM13_IO_OFFSET -1
#else
#ifdef PWM13
#undef PWM13
#endif
#ifdef DIO38
#undef DIO38
#endif
#define PWM13 38
#define DIO38 -38
#define PWM13_IO_BYTEOFFSET (PWM13_IO_OFFSET >> 3)
#define PWM13_IO_BITMASK (1 << (PWM13_IO_OFFSET & 0x7))
#define DIO38_IO_BYTEOFFSET PWM13_IO_BYTEOFFSET
#define DIO38_IO_BITMASK PWM13_IO_BITMASK
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM14_IO_OFFSET
#define PWM14_IO_OFFSET -1
#else
#ifdef PWM14
#undef PWM14
#endif
#ifdef DIO39
#undef DIO39
#endif
#define PWM14 39
#define DIO39 -39
#define PWM14_IO_BYTEOFFSET (PWM14_IO_OFFSET >> 3)
#define PWM14_IO_BITMASK (1 << (PWM14_IO_OFFSET & 0x7))
#define DIO39_IO_BYTEOFFSET PWM14_IO_BYTEOFFSET
#define DIO39_IO_BITMASK PWM14_IO_BITMASK
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM15_IO_OFFSET
#define PWM15_IO_OFFSET -1
#else
#ifdef PWM15
#undef PWM15
#endif
#ifdef DIO40
#undef DIO40
#endif
#define PWM15 40
#define DIO40 -40
#define PWM15_IO_BYTEOFFSET (PWM15_IO_OFFSET >> 3)
#define PWM15_IO_BITMASK (1 << (PWM15_IO_OFFSET & 0x7))
#define DIO40_IO_BYTEOFFSET PWM15_IO_BYTEOFFSET
#define DIO40_IO_BITMASK PWM15_IO_BITMASK
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef SERVO0_IO_OFFSET
#define SERVO0_IO_OFFSET -1
#else
#ifdef SERVO0
#undef SERVO0
#endif
#ifdef DIO41
#undef DIO41
#endif
#define SERVO0 41
#define DIO41 -41
#define SERVO0_IO_BYTEOFFSET (SERVO0_IO_OFFSET >> 3)
#define SERVO0_IO_BITMASK (1 << (SERVO0_IO_OFFSET & 0x7))
#define DIO41_IO_BYTEOFFSET SERVO0_IO_BYTEOFFSET
#define DIO41_IO_BITMASK SERVO0_IO_BITMASK
#ifndef IC74HC595_HAS_SERVOS
#define IC74HC595_HAS_SERVOS
#endif
#endif
#ifndef SERVO1_IO_OFFSET
#define SERVO1_IO_OFFSET -1
#else
#ifdef SERVO1
#undef SERVO1
#endif
#ifdef DIO42
#undef DIO42
#endif
#define SERVO1 42
#define DIO42 -42
#define SERVO1_IO_BYTEOFFSET (SERVO1_IO_OFFSET >> 3)
#define SERVO1_IO_BITMASK (1 << (SERVO1_IO_OFFSET & 0x7))
#define DIO42_IO_BYTEOFFSET SERVO1_IO_BYTEOFFSET
#define DIO42_IO_BITMASK SERVO1_IO_BITMASK
#ifndef IC74HC595_HAS_SERVOS
#define IC74HC595_HAS_SERVOS
#endif
#endif
#ifndef SERVO2_IO_OFFSET
#define SERVO2_IO_OFFSET -1
#else
#ifdef SERVO2
#undef SERVO2
#endif
#ifdef DIO43
#undef DIO43
#endif
#define SERVO2 43
#define DIO43 -43
#define SERVO2_IO_BYTEOFFSET (SERVO2_IO_OFFSET >> 3)
#define SERVO2_IO_BITMASK (1 << (SERVO2_IO_OFFSET & 0x7))
#define DIO43_IO_BYTEOFFSET SERVO2_IO_BYTEOFFSET
#define DIO43_IO_BITMASK SERVO2_IO_BITMASK
#ifndef IC74HC595_HAS_SERVOS
#define IC74HC595_HAS_SERVOS
#endif
#endif
#ifndef SERVO3_IO_OFFSET
#define SERVO3_IO_OFFSET -1
#else
#ifdef SERVO3
#undef SERVO3
#endif
#ifdef DIO44
#undef DIO44
#endif
#define SERVO3 44
#define DIO44 -44
#define SERVO3_IO_BYTEOFFSET (SERVO3_IO_OFFSET >> 3)
#define SERVO3_IO_BITMASK (1 << (SERVO3_IO_OFFSET & 0x7))
#define DIO44_IO_BYTEOFFSET SERVO3_IO_BYTEOFFSET
#define DIO44_IO_BITMASK SERVO3_IO_BITMASK
#ifndef IC74HC595_HAS_SERVOS
#define IC74HC595_HAS_SERVOS
#endif
#endif
#ifndef SERVO4_IO_OFFSET
#define SERVO4_IO_OFFSET -1
#else
#ifdef SERVO4
#undef SERVO4
#endif
#ifdef DIO45
#undef DIO45
#endif
#define SERVO4 45
#define DIO45 -45
#define SERVO4_IO_BYTEOFFSET (SERVO4_IO_OFFSET >> 3)
#define SERVO4_IO_BITMASK (1 << (SERVO4_IO_OFFSET & 0x7))
#define DIO45_IO_BYTEOFFSET SERVO4_IO_BYTEOFFSET
#define DIO45_IO_BITMASK SERVO4_IO_BITMASK
#ifndef IC74HC595_HAS_SERVOS
#define IC74HC595_HAS_SERVOS
#endif
#endif
#ifndef SERVO5_IO_OFFSET
#define SERVO5_IO_OFFSET -1
#else
#ifdef SERVO5
#undef SERVO5
#endif
#ifdef DIO46
#undef DIO46
#endif
#define SERVO5 46
#define DIO46 -46
#define SERVO5_IO_BYTEOFFSET (SERVO5_IO_OFFSET >> 3)
#define SERVO5_IO_BITMASK (1 << (SERVO5_IO_OFFSET & 0x7))
#define DIO46_IO_BYTEOFFSET SERVO5_IO_BYTEOFFSET
#define DIO46_IO_BITMASK SERVO5_IO_BITMASK
#ifndef IC74HC595_HAS_SERVOS
#define IC74HC595_HAS_SERVOS
#endif
#endif
#ifndef DOUT0_IO_OFFSET
#define DOUT0_IO_OFFSET -1
#else
#ifdef DOUT0
#undef DOUT0
#endif
#ifdef DIO47
#undef DIO47
#endif
#define DOUT0 47
#define DIO47 -47
#define DOUT0_IO_BYTEOFFSET (DOUT0_IO_OFFSET >> 3)
#define DOUT0_IO_BITMASK (1 << (DOUT0_IO_OFFSET & 0x7))
#define DIO47_IO_BYTEOFFSET DOUT0_IO_BYTEOFFSET
#define DIO47_IO_BITMASK DOUT0_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT1_IO_OFFSET
#define DOUT1_IO_OFFSET -1
#else
#ifdef DOUT1
#undef DOUT1
#endif
#ifdef DIO48
#undef DIO48
#endif
#define DOUT1 48
#define DIO48 -48
#define DOUT1_IO_BYTEOFFSET (DOUT1_IO_OFFSET >> 3)
#define DOUT1_IO_BITMASK (1 << (DOUT1_IO_OFFSET & 0x7))
#define DIO48_IO_BYTEOFFSET DOUT1_IO_BYTEOFFSET
#define DIO48_IO_BITMASK DOUT1_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT2_IO_OFFSET
#define DOUT2_IO_OFFSET -1
#else
#ifdef DOUT2
#undef DOUT2
#endif
#ifdef DIO49
#undef DIO49
#endif
#define DOUT2 49
#define DIO49 -49
#define DOUT2_IO_BYTEOFFSET (DOUT2_IO_OFFSET >> 3)
#define DOUT2_IO_BITMASK (1 << (DOUT2_IO_OFFSET & 0x7))
#define DIO49_IO_BYTEOFFSET DOUT2_IO_BYTEOFFSET
#define DIO49_IO_BITMASK DOUT2_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT3_IO_OFFSET
#define DOUT3_IO_OFFSET -1
#else
#ifdef DOUT3
#undef DOUT3
#endif
#ifdef DIO50
#undef DIO50
#endif
#define DOUT3 50
#define DIO50 -50
#define DOUT3_IO_BYTEOFFSET (DOUT3_IO_OFFSET >> 3)
#define DOUT3_IO_BITMASK (1 << (DOUT3_IO_OFFSET & 0x7))
#define DIO50_IO_BYTEOFFSET DOUT3_IO_BYTEOFFSET
#define DIO50_IO_BITMASK DOUT3_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT4_IO_OFFSET
#define DOUT4_IO_OFFSET -1
#else
#ifdef DOUT4
#undef DOUT4
#endif
#ifdef DIO51
#undef DIO51
#endif
#define DOUT4 51
#define DIO51 -51
#define DOUT4_IO_BYTEOFFSET (DOUT4_IO_OFFSET >> 3)
#define DOUT4_IO_BITMASK (1 << (DOUT4_IO_OFFSET & 0x7))
#define DIO51_IO_BYTEOFFSET DOUT4_IO_BYTEOFFSET
#define DIO51_IO_BITMASK DOUT4_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT5_IO_OFFSET
#define DOUT5_IO_OFFSET -1
#else
#ifdef DOUT5
#undef DOUT5
#endif
#ifdef DIO52
#undef DIO52
#endif
#define DOUT5 52
#define DIO52 -52
#define DOUT5_IO_BYTEOFFSET (DOUT5_IO_OFFSET >> 3)
#define DOUT5_IO_BITMASK (1 << (DOUT5_IO_OFFSET & 0x7))
#define DIO52_IO_BYTEOFFSET DOUT5_IO_BYTEOFFSET
#define DIO52_IO_BITMASK DOUT5_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT6_IO_OFFSET
#define DOUT6_IO_OFFSET -1
#else
#ifdef DOUT6
#undef DOUT6
#endif
#ifdef DIO53
#undef DIO53
#endif
#define DOUT6 53
#define DIO53 -53
#define DOUT6_IO_BYTEOFFSET (DOUT6_IO_OFFSET >> 3)
#define DOUT6_IO_BITMASK (1 << (DOUT6_IO_OFFSET & 0x7))
#define DIO53_IO_BYTEOFFSET DOUT6_IO_BYTEOFFSET
#define DIO53_IO_BITMASK DOUT6_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT7_IO_OFFSET
#define DOUT7_IO_OFFSET -1
#else
#ifdef DOUT7
#undef DOUT7
#endif
#ifdef DIO54
#undef DIO54
#endif
#define DOUT7 54
#define DIO54 -54
#define DOUT7_IO_BYTEOFFSET (DOUT7_IO_OFFSET >> 3)
#define DOUT7_IO_BITMASK (1 << (DOUT7_IO_OFFSET & 0x7))
#define DIO54_IO_BYTEOFFSET DOUT7_IO_BYTEOFFSET
#define DIO54_IO_BITMASK DOUT7_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT8_IO_OFFSET
#define DOUT8_IO_OFFSET -1
#else
#ifdef DOUT8
#undef DOUT8
#endif
#ifdef DIO55
#undef DIO55
#endif
#define DOUT8 55
#define DIO55 -55
#define DOUT8_IO_BYTEOFFSET (DOUT8_IO_OFFSET >> 3)
#define DOUT8_IO_BITMASK (1 << (DOUT8_IO_OFFSET & 0x7))
#define DIO55_IO_BYTEOFFSET DOUT8_IO_BYTEOFFSET
#define DIO55_IO_BITMASK DOUT8_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT9_IO_OFFSET
#define DOUT9_IO_OFFSET -1
#else
#ifdef DOUT9
#undef DOUT9
#endif
#ifdef DIO56
#undef DIO56
#endif
#define DOUT9 56
#define DIO56 -56
#define DOUT9_IO_BYTEOFFSET (DOUT9_IO_OFFSET >> 3)
#define DOUT9_IO_BITMASK (1 << (DOUT9_IO_OFFSET & 0x7))
#define DIO56_IO_BYTEOFFSET DOUT9_IO_BYTEOFFSET
#define DIO56_IO_BITMASK DOUT9_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT10_IO_OFFSET
#define DOUT10_IO_OFFSET -1
#else
#ifdef DOUT10
#undef DOUT10
#endif
#ifdef DIO57
#undef DIO57
#endif
#define DOUT10 57
#define DIO57 -57
#define DOUT10_IO_BYTEOFFSET (DOUT10_IO_OFFSET >> 3)
#define DOUT10_IO_BITMASK (1 << (DOUT10_IO_OFFSET & 0x7))
#define DIO57_IO_BYTEOFFSET DOUT10_IO_BYTEOFFSET
#define DIO57_IO_BITMASK DOUT10_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT11_IO_OFFSET
#define DOUT11_IO_OFFSET -1
#else
#ifdef DOUT11
#undef DOUT11
#endif
#ifdef DIO58
#undef DIO58
#endif
#define DOUT11 58
#define DIO58 -58
#define DOUT11_IO_BYTEOFFSET (DOUT11_IO_OFFSET >> 3)
#define DOUT11_IO_BITMASK (1 << (DOUT11_IO_OFFSET & 0x7))
#define DIO58_IO_BYTEOFFSET DOUT11_IO_BYTEOFFSET
#define DIO58_IO_BITMASK DOUT11_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT12_IO_OFFSET
#define DOUT12_IO_OFFSET -1
#else
#ifdef DOUT12
#undef DOUT12
#endif
#ifdef DIO59
#undef DIO59
#endif
#define DOUT12 59
#define DIO59 -59
#define DOUT12_IO_BYTEOFFSET (DOUT12_IO_OFFSET >> 3)
#define DOUT12_IO_BITMASK (1 << (DOUT12_IO_OFFSET & 0x7))
#define DIO59_IO_BYTEOFFSET DOUT12_IO_BYTEOFFSET
#define DIO59_IO_BITMASK DOUT12_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT13_IO_OFFSET
#define DOUT13_IO_OFFSET -1
#else
#ifdef DOUT13
#undef DOUT13
#endif
#ifdef DIO60
#undef DIO60
#endif
#define DOUT13 60
#define DIO60 -60
#define DOUT13_IO_BYTEOFFSET (DOUT13_IO_OFFSET >> 3)
#define DOUT13_IO_BITMASK (1 << (DOUT13_IO_OFFSET & 0x7))
#define DIO60_IO_BYTEOFFSET DOUT13_IO_BYTEOFFSET
#define DIO60_IO_BITMASK DOUT13_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT14_IO_OFFSET
#define DOUT14_IO_OFFSET -1
#else
#ifdef DOUT14
#undef DOUT14
#endif
#ifdef DIO61
#undef DIO61
#endif
#define DOUT14 61
#define DIO61 -61
#define DOUT14_IO_BYTEOFFSET (DOUT14_IO_OFFSET >> 3)
#define DOUT14_IO_BITMASK (1 << (DOUT14_IO_OFFSET & 0x7))
#define DIO61_IO_BYTEOFFSET DOUT14_IO_BYTEOFFSET
#define DIO61_IO_BITMASK DOUT14_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT15_IO_OFFSET
#define DOUT15_IO_OFFSET -1
#else
#ifdef DOUT15
#undef DOUT15
#endif
#ifdef DIO62
#undef DIO62
#endif
#define DOUT15 62
#define DIO62 -62
#define DOUT15_IO_BYTEOFFSET (DOUT15_IO_OFFSET >> 3)
#define DOUT15_IO_BITMASK (1 << (DOUT15_IO_OFFSET & 0x7))
#define DIO62_IO_BYTEOFFSET DOUT15_IO_BYTEOFFSET
#define DIO62_IO_BITMASK DOUT15_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT16_IO_OFFSET
#define DOUT16_IO_OFFSET -1
#else
#ifdef DOUT16
#undef DOUT16
#endif
#ifdef DIO63
#undef DIO63
#endif
#define DOUT16 63
#define DIO63 -63
#define DOUT16_IO_BYTEOFFSET (DOUT16_IO_OFFSET >> 3)
#define DOUT16_IO_BITMASK (1 << (DOUT16_IO_OFFSET & 0x7))
#define DIO63_IO_BYTEOFFSET DOUT16_IO_BYTEOFFSET
#define DIO63_IO_BITMASK DOUT16_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT17_IO_OFFSET
#define DOUT17_IO_OFFSET -1
#else
#ifdef DOUT17
#undef DOUT17
#endif
#ifdef DIO64
#undef DIO64
#endif
#define DOUT17 64
#define DIO64 -64
#define DOUT17_IO_BYTEOFFSET (DOUT17_IO_OFFSET >> 3)
#define DOUT17_IO_BITMASK (1 << (DOUT17_IO_OFFSET & 0x7))
#define DIO64_IO_BYTEOFFSET DOUT17_IO_BYTEOFFSET
#define DIO64_IO_BITMASK DOUT17_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT18_IO_OFFSET
#define DOUT18_IO_OFFSET -1
#else
#ifdef DOUT18
#undef DOUT18
#endif
#ifdef DIO65
#undef DIO65
#endif
#define DOUT18 65
#define DIO65 -65
#define DOUT18_IO_BYTEOFFSET (DOUT18_IO_OFFSET >> 3)
#define DOUT18_IO_BITMASK (1 << (DOUT18_IO_OFFSET & 0x7))
#define DIO65_IO_BYTEOFFSET DOUT18_IO_BYTEOFFSET
#define DIO65_IO_BITMASK DOUT18_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT19_IO_OFFSET
#define DOUT19_IO_OFFSET -1
#else
#ifdef DOUT19
#undef DOUT19
#endif
#ifdef DIO66
#undef DIO66
#endif
#define DOUT19 66
#define DIO66 -66
#define DOUT19_IO_BYTEOFFSET (DOUT19_IO_OFFSET >> 3)
#define DOUT19_IO_BITMASK (1 << (DOUT19_IO_OFFSET & 0x7))
#define DIO66_IO_BYTEOFFSET DOUT19_IO_BYTEOFFSET
#define DIO66_IO_BITMASK DOUT19_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT20_IO_OFFSET
#define DOUT20_IO_OFFSET -1
#else
#ifdef DOUT20
#undef DOUT20
#endif
#ifdef DIO67
#undef DIO67
#endif
#define DOUT20 67
#define DIO67 -67
#define DOUT20_IO_BYTEOFFSET (DOUT20_IO_OFFSET >> 3)
#define DOUT20_IO_BITMASK (1 << (DOUT20_IO_OFFSET & 0x7))
#define DIO67_IO_BYTEOFFSET DOUT20_IO_BYTEOFFSET
#define DIO67_IO_BITMASK DOUT20_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT21_IO_OFFSET
#define DOUT21_IO_OFFSET -1
#else
#ifdef DOUT21
#undef DOUT21
#endif
#ifdef DIO68
#undef DIO68
#endif
#define DOUT21 68
#define DIO68 -68
#define DOUT21_IO_BYTEOFFSET (DOUT21_IO_OFFSET >> 3)
#define DOUT21_IO_BITMASK (1 << (DOUT21_IO_OFFSET & 0x7))
#define DIO68_IO_BYTEOFFSET DOUT21_IO_BYTEOFFSET
#define DIO68_IO_BITMASK DOUT21_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT22_IO_OFFSET
#define DOUT22_IO_OFFSET -1
#else
#ifdef DOUT22
#undef DOUT22
#endif
#ifdef DIO69
#undef DIO69
#endif
#define DOUT22 69
#define DIO69 -69
#define DOUT22_IO_BYTEOFFSET (DOUT22_IO_OFFSET >> 3)
#define DOUT22_IO_BITMASK (1 << (DOUT22_IO_OFFSET & 0x7))
#define DIO69_IO_BYTEOFFSET DOUT22_IO_BYTEOFFSET
#define DIO69_IO_BITMASK DOUT22_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT23_IO_OFFSET
#define DOUT23_IO_OFFSET -1
#else
#ifdef DOUT23
#undef DOUT23
#endif
#ifdef DIO70
#undef DIO70
#endif
#define DOUT23 70
#define DIO70 -70
#define DOUT23_IO_BYTEOFFSET (DOUT23_IO_OFFSET >> 3)
#define DOUT23_IO_BITMASK (1 << (DOUT23_IO_OFFSET & 0x7))
#define DIO70_IO_BYTEOFFSET DOUT23_IO_BYTEOFFSET
#define DIO70_IO_BITMASK DOUT23_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT24_IO_OFFSET
#define DOUT24_IO_OFFSET -1
#else
#ifdef DOUT24
#undef DOUT24
#endif
#ifdef DIO71
#undef DIO71
#endif
#define DOUT24 71
#define DIO71 -71
#define DOUT24_IO_BYTEOFFSET (DOUT24_IO_OFFSET >> 3)
#define DOUT24_IO_BITMASK (1 << (DOUT24_IO_OFFSET & 0x7))
#define DIO71_IO_BYTEOFFSET DOUT24_IO_BYTEOFFSET
#define DIO71_IO_BITMASK DOUT24_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT25_IO_OFFSET
#define DOUT25_IO_OFFSET -1
#else
#ifdef DOUT25
#undef DOUT25
#endif
#ifdef DIO72
#undef DIO72
#endif
#define DOUT25 72
#define DIO72 -72
#define DOUT25_IO_BYTEOFFSET (DOUT25_IO_OFFSET >> 3)
#define DOUT25_IO_BITMASK (1 << (DOUT25_IO_OFFSET & 0x7))
#define DIO72_IO_BYTEOFFSET DOUT25_IO_BYTEOFFSET
#define DIO72_IO_BITMASK DOUT25_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT26_IO_OFFSET
#define DOUT26_IO_OFFSET -1
#else
#ifdef DOUT26
#undef DOUT26
#endif
#ifdef DIO73
#undef DIO73
#endif
#define DOUT26 73
#define DIO73 -73
#define DOUT26_IO_BYTEOFFSET (DOUT26_IO_OFFSET >> 3)
#define DOUT26_IO_BITMASK (1 << (DOUT26_IO_OFFSET & 0x7))
#define DIO73_IO_BYTEOFFSET DOUT26_IO_BYTEOFFSET
#define DIO73_IO_BITMASK DOUT26_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT27_IO_OFFSET
#define DOUT27_IO_OFFSET -1
#else
#ifdef DOUT27
#undef DOUT27
#endif
#ifdef DIO74
#undef DIO74
#endif
#define DOUT27 74
#define DIO74 -74
#define DOUT27_IO_BYTEOFFSET (DOUT27_IO_OFFSET >> 3)
#define DOUT27_IO_BITMASK (1 << (DOUT27_IO_OFFSET & 0x7))
#define DIO74_IO_BYTEOFFSET DOUT27_IO_BYTEOFFSET
#define DIO74_IO_BITMASK DOUT27_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT28_IO_OFFSET
#define DOUT28_IO_OFFSET -1
#else
#ifdef DOUT28
#undef DOUT28
#endif
#ifdef DIO75
#undef DIO75
#endif
#define DOUT28 75
#define DIO75 -75
#define DOUT28_IO_BYTEOFFSET (DOUT28_IO_OFFSET >> 3)
#define DOUT28_IO_BITMASK (1 << (DOUT28_IO_OFFSET & 0x7))
#define DIO75_IO_BYTEOFFSET DOUT28_IO_BYTEOFFSET
#define DIO75_IO_BITMASK DOUT28_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT29_IO_OFFSET
#define DOUT29_IO_OFFSET -1
#else
#ifdef DOUT29
#undef DOUT29
#endif
#ifdef DIO76
#undef DIO76
#endif
#define DOUT29 76
#define DIO76 -76
#define DOUT29_IO_BYTEOFFSET (DOUT29_IO_OFFSET >> 3)
#define DOUT29_IO_BITMASK (1 << (DOUT29_IO_OFFSET & 0x7))
#define DIO76_IO_BYTEOFFSET DOUT29_IO_BYTEOFFSET
#define DIO76_IO_BITMASK DOUT29_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT30_IO_OFFSET
#define DOUT30_IO_OFFSET -1
#else
#ifdef DOUT30
#undef DOUT30
#endif
#ifdef DIO77
#undef DIO77
#endif
#define DOUT30 77
#define DIO77 -77
#define DOUT30_IO_BYTEOFFSET (DOUT30_IO_OFFSET >> 3)
#define DOUT30_IO_BITMASK (1 << (DOUT30_IO_OFFSET & 0x7))
#define DIO77_IO_BYTEOFFSET DOUT30_IO_BYTEOFFSET
#define DIO77_IO_BITMASK DOUT30_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT31_IO_OFFSET
#define DOUT31_IO_OFFSET -1
#else
#ifdef DOUT31
#undef DOUT31
#endif
#ifdef DIO78
#undef DIO78
#endif
#define DOUT31 78
#define DIO78 -78
#define DOUT31_IO_BYTEOFFSET (DOUT31_IO_OFFSET >> 3)
#define DOUT31_IO_BITMASK (1 << (DOUT31_IO_OFFSET & 0x7))
#define DIO78_IO_BYTEOFFSET DOUT31_IO_BYTEOFFSET
#define DIO78_IO_BITMASK DOUT31_IO_BITMASK
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif

#if (IC74HC595_COUNT < 0)
#undef IC74HC595_COUNT
#define IC74HC595_COUNT 0
#elif (IC74HC595_COUNT > 7)
#error "The maximum number of chained IC74HC595 is 7"
#endif

#if (IC74HC595_COUNT > 0)
#ifndef __indirect__
#define __indirect__ex__(X, Y) DIO##X##_##Y
#define __indirect__(X, Y) __indirect__ex__(X, Y)
#endif
	extern uint8_t ic74hc595_io_pins[IC74HC595_COUNT];
#define ic74hc595_set_pin(pin) ic74hc595_io_pins[(__indirect__(pin, IO_BYTEOFFSET))] |= (__indirect__(pin, IO_BITMASK))
#define ic74hc595_clear_pin(pin) ic74hc595_io_pins[__indirect__(pin, IO_BYTEOFFSET)] &= ~(__indirect__(pin, IO_BITMASK))
#define ic74hc595_toggle_pin(pin) ic74hc595_io_pins[(__indirect__(pin, IO_BYTEOFFSET))] ^= (__indirect__(pin, IO_BITMASK))
#define ic74hc595_get_pin(pin) (ic74hc595_io_pins[(__indirect__(pin, IO_BYTEOFFSET))] & (__indirect__(pin, IO_BITMASK)))
#else
#define ic74hc595_set_pin(pin)
#define ic74hc595_clear_pin(pin)
#define ic74hc595_toggle_pin(pin)
#define ic74hc595_get_pin(pin) 0
#endif

	void ic74hc595_set_steps(uint8_t mask);
	void ic74hc595_toggle_steps(uint8_t mask);
	void ic74hc595_set_dirs(uint8_t mask);
	void ic74hc595_enable_steppers(uint8_t mask);
	void ic74hc595_set_pwms(uint16_t mask);
	void ic74hc595_set_servos(uint8_t mask);
	void ic74hc595_set_output(uint8_t pin, bool state);
	void ic74hc595_shift_io_pins(void);

#ifdef __cplusplus
}
#endif

#endif
