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
#define STEP0_IO_BYTEOFFSET (STEP0_IO_OFFSET >> 3)
#define STEP0_IO_BITMASK (1 << (STEP0_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_STEPS
#define IC74HC595_HAS_STEPS
#endif
#endif
#ifndef STEP1_IO_OFFSET
#define STEP1_IO_OFFSET -1
#else
#define STEP1_IO_BYTEOFFSET (STEP1_IO_OFFSET >> 3)
#define STEP1_IO_BITMASK (1 << (STEP1_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_STEPS
#define IC74HC595_HAS_STEPS
#endif
#endif
#ifndef STEP2_IO_OFFSET
#define STEP2_IO_OFFSET -1
#else
#define STEP2_IO_BYTEOFFSET (STEP2_IO_OFFSET >> 3)
#define STEP2_IO_BITMASK (1 << (STEP2_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_STEPS
#define IC74HC595_HAS_STEPS
#endif
#endif
#ifndef STEP3_IO_OFFSET
#define STEP3_IO_OFFSET -1
#else
#define STEP3_IO_BYTEOFFSET (STEP3_IO_OFFSET >> 3)
#define STEP3_IO_BITMASK (1 << (STEP3_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_STEPS
#define IC74HC595_HAS_STEPS
#endif
#endif
#ifndef STEP4_IO_OFFSET
#define STEP4_IO_OFFSET -1
#else
#define STEP4_IO_BYTEOFFSET (STEP4_IO_OFFSET >> 3)
#define STEP4_IO_BITMASK (1 << (STEP4_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_STEPS
#define IC74HC595_HAS_STEPS
#endif
#endif
#ifndef STEP5_IO_OFFSET
#define STEP5_IO_OFFSET -1
#else
#define STEP5_IO_BYTEOFFSET (STEP5_IO_OFFSET >> 3)
#define STEP5_IO_BITMASK (1 << (STEP5_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_STEPS
#define IC74HC595_HAS_STEPS
#endif
#endif
#ifndef STEP6_IO_OFFSET
#define STEP6_IO_OFFSET -1
#else
#define STEP6_IO_BYTEOFFSET (STEP6_IO_OFFSET >> 3)
#define STEP6_IO_BITMASK (1 << (STEP6_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_STEPS
#define IC74HC595_HAS_STEPS
#endif
#endif
#ifndef STEP7_IO_OFFSET
#define STEP7_IO_OFFSET -1
#else
#define STEP7_IO_BYTEOFFSET (STEP7_IO_OFFSET >> 3)
#define STEP7_IO_BITMASK (1 << (STEP7_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_STEPS
#define IC74HC595_HAS_STEPS
#endif
#endif
#ifndef DIR0_IO_OFFSET
#define DIR0_IO_OFFSET -1
#else
#define DIR0_IO_BYTEOFFSET (DIR0_IO_OFFSET >> 3)
#define DIR0_IO_BITMASK (1 << (DIR0_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DIRS
#define IC74HC595_HAS_DIRS
#endif
#endif
#ifndef DIR1_IO_OFFSET
#define DIR1_IO_OFFSET -1
#else
#define DIR1_IO_BYTEOFFSET (DIR1_IO_OFFSET >> 3)
#define DIR1_IO_BITMASK (1 << (DIR1_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DIRS
#define IC74HC595_HAS_DIRS
#endif
#endif
#ifndef DIR2_IO_OFFSET
#define DIR2_IO_OFFSET -1
#else
#define DIR2_IO_BYTEOFFSET (DIR2_IO_OFFSET >> 3)
#define DIR2_IO_BITMASK (1 << (DIR2_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DIRS
#define IC74HC595_HAS_DIRS
#endif
#endif
#ifndef DIR3_IO_OFFSET
#define DIR3_IO_OFFSET -1
#else
#define DIR3_IO_BYTEOFFSET (DIR3_IO_OFFSET >> 3)
#define DIR3_IO_BITMASK (1 << (DIR3_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DIRS
#define IC74HC595_HAS_DIRS
#endif
#endif
#ifndef DIR4_IO_OFFSET
#define DIR4_IO_OFFSET -1
#else
#define DIR4_IO_BYTEOFFSET (DIR4_IO_OFFSET >> 3)
#define DIR4_IO_BITMASK (1 << (DIR4_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DIRS
#define IC74HC595_HAS_DIRS
#endif
#endif
#ifndef DIR5_IO_OFFSET
#define DIR5_IO_OFFSET -1
#else
#define DIR5_IO_BYTEOFFSET (DIR5_IO_OFFSET >> 3)
#define DIR5_IO_BITMASK (1 << (DIR5_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DIRS
#define IC74HC595_HAS_DIRS
#endif
#endif
#ifndef DIR6_IO_OFFSET
#define DIR6_IO_OFFSET -1
#else
#define DIR6_IO_BYTEOFFSET (DIR6_IO_OFFSET >> 3)
#define DIR6_IO_BITMASK (1 << (DIR6_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DIRS
#define IC74HC595_HAS_DIRS
#endif
#endif
#ifndef DIR7_IO_OFFSET
#define DIR7_IO_OFFSET -1
#else
#define DIR7_IO_BYTEOFFSET (DIR7_IO_OFFSET >> 3)
#define DIR7_IO_BITMASK (1 << (DIR7_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DIRS
#define IC74HC595_HAS_DIRS
#endif
#endif
#ifndef STEP0_EN_IO_OFFSET
#define STEP0_EN_IO_OFFSET -1
#else
#define STEP0_EN_IO_BYTEOFFSET (STEP0_EN_IO_OFFSET >> 3)
#define STEP0_EN_IO_BITMASK (1 << (STEP0_EN_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_STEPS_EN
#define IC74HC595_HAS_STEPS_EN
#endif
#endif
#ifndef STEP1_EN_IO_OFFSET
#define STEP1_EN_IO_OFFSET -1
#else
#define STEP1_EN_IO_BYTEOFFSET (STEP1_EN_IO_OFFSET >> 3)
#define STEP1_EN_IO_BITMASK (1 << (STEP1_EN_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_STEPS_EN
#define IC74HC595_HAS_STEPS_EN
#endif
#endif
#ifndef STEP2_EN_IO_OFFSET
#define STEP2_EN_IO_OFFSET -1
#else
#define STEP2_EN_IO_BYTEOFFSET (STEP2_EN_IO_OFFSET >> 3)
#define STEP2_EN_IO_BITMASK (1 << (STEP2_EN_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_STEPS_EN
#define IC74HC595_HAS_STEPS_EN
#endif
#endif
#ifndef STEP3_EN_IO_OFFSET
#define STEP3_EN_IO_OFFSET -1
#else
#define STEP3_EN_IO_BYTEOFFSET (STEP3_EN_IO_OFFSET >> 3)
#define STEP3_EN_IO_BITMASK (1 << (STEP3_EN_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_STEPS_EN
#define IC74HC595_HAS_STEPS_EN
#endif
#endif
#ifndef STEP4_EN_IO_OFFSET
#define STEP4_EN_IO_OFFSET -1
#else
#define STEP4_EN_IO_BYTEOFFSET (STEP4_EN_IO_OFFSET >> 3)
#define STEP4_EN_IO_BITMASK (1 << (STEP4_EN_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_STEPS_EN
#define IC74HC595_HAS_STEPS_EN
#endif
#endif
#ifndef STEP5_EN_IO_OFFSET
#define STEP5_EN_IO_OFFSET -1
#else
#define STEP5_EN_IO_BYTEOFFSET (STEP5_EN_IO_OFFSET >> 3)
#define STEP5_EN_IO_BITMASK (1 << (STEP5_EN_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_STEPS_EN
#define IC74HC595_HAS_STEPS_EN
#endif
#endif
#ifndef STEP6_EN_IO_OFFSET
#define STEP6_EN_IO_OFFSET -1
#else
#define STEP6_EN_IO_BYTEOFFSET (STEP6_EN_IO_OFFSET >> 3)
#define STEP6_EN_IO_BITMASK (1 << (STEP6_EN_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_STEPS_EN
#define IC74HC595_HAS_STEPS_EN
#endif
#endif
#ifndef STEP7_EN_IO_OFFSET
#define STEP7_EN_IO_OFFSET -1
#else
#define STEP7_EN_IO_BYTEOFFSET (STEP7_EN_IO_OFFSET >> 3)
#define STEP7_EN_IO_BITMASK (1 << (STEP7_EN_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_STEPS_EN
#define IC74HC595_HAS_STEPS_EN
#endif
#endif
#ifndef PWM0_IO_OFFSET
#define PWM0_IO_OFFSET -1
#else
#define PWM0_IO_BYTEOFFSET (PWM0_IO_OFFSET >> 3)
#define PWM0_IO_BITMASK (1 << (PWM0_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM1_IO_OFFSET
#define PWM1_IO_OFFSET -1
#else
#define PWM1_IO_BYTEOFFSET (PWM1_IO_OFFSET >> 3)
#define PWM1_IO_BITMASK (1 << (PWM1_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM2_IO_OFFSET
#define PWM2_IO_OFFSET -1
#else
#define PWM2_IO_BYTEOFFSET (PWM2_IO_OFFSET >> 3)
#define PWM2_IO_BITMASK (1 << (PWM2_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM3_IO_OFFSET
#define PWM3_IO_OFFSET -1
#else
#define PWM3_IO_BYTEOFFSET (PWM3_IO_OFFSET >> 3)
#define PWM3_IO_BITMASK (1 << (PWM3_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM4_IO_OFFSET
#define PWM4_IO_OFFSET -1
#else
#define PWM4_IO_BYTEOFFSET (PWM4_IO_OFFSET >> 3)
#define PWM4_IO_BITMASK (1 << (PWM4_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM5_IO_OFFSET
#define PWM5_IO_OFFSET -1
#else
#define PWM5_IO_BYTEOFFSET (PWM5_IO_OFFSET >> 3)
#define PWM5_IO_BITMASK (1 << (PWM5_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM6_IO_OFFSET
#define PWM6_IO_OFFSET -1
#else
#define PWM6_IO_BYTEOFFSET (PWM6_IO_OFFSET >> 3)
#define PWM6_IO_BITMASK (1 << (PWM6_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM7_IO_OFFSET
#define PWM7_IO_OFFSET -1
#else
#define PWM7_IO_BYTEOFFSET (PWM7_IO_OFFSET >> 3)
#define PWM7_IO_BITMASK (1 << (PWM7_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM8_IO_OFFSET
#define PWM8_IO_OFFSET -1
#else
#define PWM8_IO_BYTEOFFSET (PWM8_IO_OFFSET >> 3)
#define PWM8_IO_BITMASK (1 << (PWM8_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM9_IO_OFFSET
#define PWM9_IO_OFFSET -1
#else
#define PWM9_IO_BYTEOFFSET (PWM9_IO_OFFSET >> 3)
#define PWM9_IO_BITMASK (1 << (PWM9_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM10_IO_OFFSET
#define PWM10_IO_OFFSET -1
#else
#define PWM10_IO_BYTEOFFSET (PWM10_IO_OFFSET >> 3)
#define PWM10_IO_BITMASK (1 << (PWM10_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM11_IO_OFFSET
#define PWM11_IO_OFFSET -1
#else
#define PWM11_IO_BYTEOFFSET (PWM11_IO_OFFSET >> 3)
#define PWM11_IO_BITMASK (1 << (PWM11_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM12_IO_OFFSET
#define PWM12_IO_OFFSET -1
#else
#define PWM12_IO_BYTEOFFSET (PWM12_IO_OFFSET >> 3)
#define PWM12_IO_BITMASK (1 << (PWM12_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM13_IO_OFFSET
#define PWM13_IO_OFFSET -1
#else
#define PWM13_IO_BYTEOFFSET (PWM13_IO_OFFSET >> 3)
#define PWM13_IO_BITMASK (1 << (PWM13_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM14_IO_OFFSET
#define PWM14_IO_OFFSET -1
#else
#define PWM14_IO_BYTEOFFSET (PWM14_IO_OFFSET >> 3)
#define PWM14_IO_BITMASK (1 << (PWM14_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef PWM15_IO_OFFSET
#define PWM15_IO_OFFSET -1
#else
#define PWM15_IO_BYTEOFFSET (PWM15_IO_OFFSET >> 3)
#define PWM15_IO_BITMASK (1 << (PWM15_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_PWMS
#define IC74HC595_HAS_PWMS
#endif
#endif
#ifndef SERVO0_IO_OFFSET
#define SERVO0_IO_OFFSET -1
#else
#define SERVO0_IO_BYTEOFFSET (SERVO0_IO_OFFSET >> 3)
#define SERVO0_IO_BITMASK (1 << (SERVO0_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_SERVOS
#define IC74HC595_HAS_SERVOS
#endif
#endif
#ifndef SERVO1_IO_OFFSET
#define SERVO1_IO_OFFSET -1
#else
#define SERVO1_IO_BYTEOFFSET (SERVO1_IO_OFFSET >> 3)
#define SERVO1_IO_BITMASK (1 << (SERVO1_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_SERVOS
#define IC74HC595_HAS_SERVOS
#endif
#endif
#ifndef SERVO2_IO_OFFSET
#define SERVO2_IO_OFFSET -1
#else
#define SERVO2_IO_BYTEOFFSET (SERVO2_IO_OFFSET >> 3)
#define SERVO2_IO_BITMASK (1 << (SERVO2_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_SERVOS
#define IC74HC595_HAS_SERVOS
#endif
#endif
#ifndef SERVO3_IO_OFFSET
#define SERVO3_IO_OFFSET -1
#else
#define SERVO3_IO_BYTEOFFSET (SERVO3_IO_OFFSET >> 3)
#define SERVO3_IO_BITMASK (1 << (SERVO3_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_SERVOS
#define IC74HC595_HAS_SERVOS
#endif
#endif
#ifndef SERVO4_IO_OFFSET
#define SERVO4_IO_OFFSET -1
#else
#define SERVO4_IO_BYTEOFFSET (SERVO4_IO_OFFSET >> 3)
#define SERVO4_IO_BITMASK (1 << (SERVO4_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_SERVOS
#define IC74HC595_HAS_SERVOS
#endif
#endif
#ifndef SERVO5_IO_OFFSET
#define SERVO5_IO_OFFSET -1
#else
#define SERVO5_IO_BYTEOFFSET (SERVO5_IO_OFFSET >> 3)
#define SERVO5_IO_BITMASK (1 << (SERVO5_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_SERVOS
#define IC74HC595_HAS_SERVOS
#endif
#endif
#ifndef DOUT0_IO_OFFSET
#define DOUT0_IO_OFFSET -1
#else
#define DOUT0_IO_BYTEOFFSET (DOUT0_IO_OFFSET >> 3)
#define DOUT0_IO_BITMASK (1 << (DOUT0_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT1_IO_OFFSET
#define DOUT1_IO_OFFSET -1
#else
#define DOUT1_IO_BYTEOFFSET (DOUT1_IO_OFFSET >> 3)
#define DOUT1_IO_BITMASK (1 << (DOUT1_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT2_IO_OFFSET
#define DOUT2_IO_OFFSET -1
#else
#define DOUT2_IO_BYTEOFFSET (DOUT2_IO_OFFSET >> 3)
#define DOUT2_IO_BITMASK (1 << (DOUT2_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT3_IO_OFFSET
#define DOUT3_IO_OFFSET -1
#else
#define DOUT3_IO_BYTEOFFSET (DOUT3_IO_OFFSET >> 3)
#define DOUT3_IO_BITMASK (1 << (DOUT3_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT4_IO_OFFSET
#define DOUT4_IO_OFFSET -1
#else
#define DOUT4_IO_BYTEOFFSET (DOUT4_IO_OFFSET >> 3)
#define DOUT4_IO_BITMASK (1 << (DOUT4_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT5_IO_OFFSET
#define DOUT5_IO_OFFSET -1
#else
#define DOUT5_IO_BYTEOFFSET (DOUT5_IO_OFFSET >> 3)
#define DOUT5_IO_BITMASK (1 << (DOUT5_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT6_IO_OFFSET
#define DOUT6_IO_OFFSET -1
#else
#define DOUT6_IO_BYTEOFFSET (DOUT6_IO_OFFSET >> 3)
#define DOUT6_IO_BITMASK (1 << (DOUT6_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT7_IO_OFFSET
#define DOUT7_IO_OFFSET -1
#else
#define DOUT7_IO_BYTEOFFSET (DOUT7_IO_OFFSET >> 3)
#define DOUT7_IO_BITMASK (1 << (DOUT7_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT8_IO_OFFSET
#define DOUT8_IO_OFFSET -1
#else
#define DOUT8_IO_BYTEOFFSET (DOUT8_IO_OFFSET >> 3)
#define DOUT8_IO_BITMASK (1 << (DOUT8_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT9_IO_OFFSET
#define DOUT9_IO_OFFSET -1
#else
#define DOUT9_IO_BYTEOFFSET (DOUT9_IO_OFFSET >> 3)
#define DOUT9_IO_BITMASK (1 << (DOUT9_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT10_IO_OFFSET
#define DOUT10_IO_OFFSET -1
#else
#define DOUT10_IO_BYTEOFFSET (DOUT10_IO_OFFSET >> 3)
#define DOUT10_IO_BITMASK (1 << (DOUT10_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT11_IO_OFFSET
#define DOUT11_IO_OFFSET -1
#else
#define DOUT11_IO_BYTEOFFSET (DOUT11_IO_OFFSET >> 3)
#define DOUT11_IO_BITMASK (1 << (DOUT11_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT12_IO_OFFSET
#define DOUT12_IO_OFFSET -1
#else
#define DOUT12_IO_BYTEOFFSET (DOUT12_IO_OFFSET >> 3)
#define DOUT12_IO_BITMASK (1 << (DOUT12_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT13_IO_OFFSET
#define DOUT13_IO_OFFSET -1
#else
#define DOUT13_IO_BYTEOFFSET (DOUT13_IO_OFFSET >> 3)
#define DOUT13_IO_BITMASK (1 << (DOUT13_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT14_IO_OFFSET
#define DOUT14_IO_OFFSET -1
#else
#define DOUT14_IO_BYTEOFFSET (DOUT14_IO_OFFSET >> 3)
#define DOUT14_IO_BITMASK (1 << (DOUT14_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT15_IO_OFFSET
#define DOUT15_IO_OFFSET -1
#else
#define DOUT15_IO_BYTEOFFSET (DOUT15_IO_OFFSET >> 3)
#define DOUT15_IO_BITMASK (1 << (DOUT15_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT16_IO_OFFSET
#define DOUT16_IO_OFFSET -1
#else
#define DOUT16_IO_BYTEOFFSET (DOUT16_IO_OFFSET >> 3)
#define DOUT16_IO_BITMASK (1 << (DOUT16_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT17_IO_OFFSET
#define DOUT17_IO_OFFSET -1
#else
#define DOUT17_IO_BYTEOFFSET (DOUT17_IO_OFFSET >> 3)
#define DOUT17_IO_BITMASK (1 << (DOUT17_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT18_IO_OFFSET
#define DOUT18_IO_OFFSET -1
#else
#define DOUT18_IO_BYTEOFFSET (DOUT18_IO_OFFSET >> 3)
#define DOUT18_IO_BITMASK (1 << (DOUT18_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT19_IO_OFFSET
#define DOUT19_IO_OFFSET -1
#else
#define DOUT19_IO_BYTEOFFSET (DOUT19_IO_OFFSET >> 3)
#define DOUT19_IO_BITMASK (1 << (DOUT19_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT20_IO_OFFSET
#define DOUT20_IO_OFFSET -1
#else
#define DOUT20_IO_BYTEOFFSET (DOUT20_IO_OFFSET >> 3)
#define DOUT20_IO_BITMASK (1 << (DOUT20_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT21_IO_OFFSET
#define DOUT21_IO_OFFSET -1
#else
#define DOUT21_IO_BYTEOFFSET (DOUT21_IO_OFFSET >> 3)
#define DOUT21_IO_BITMASK (1 << (DOUT21_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT22_IO_OFFSET
#define DOUT22_IO_OFFSET -1
#else
#define DOUT22_IO_BYTEOFFSET (DOUT22_IO_OFFSET >> 3)
#define DOUT22_IO_BITMASK (1 << (DOUT22_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT23_IO_OFFSET
#define DOUT23_IO_OFFSET -1
#else
#define DOUT23_IO_BYTEOFFSET (DOUT23_IO_OFFSET >> 3)
#define DOUT23_IO_BITMASK (1 << (DOUT23_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT24_IO_OFFSET
#define DOUT24_IO_OFFSET -1
#else
#define DOUT24_IO_BYTEOFFSET (DOUT24_IO_OFFSET >> 3)
#define DOUT24_IO_BITMASK (1 << (DOUT24_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT25_IO_OFFSET
#define DOUT25_IO_OFFSET -1
#else
#define DOUT25_IO_BYTEOFFSET (DOUT25_IO_OFFSET >> 3)
#define DOUT25_IO_BITMASK (1 << (DOUT25_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT26_IO_OFFSET
#define DOUT26_IO_OFFSET -1
#else
#define DOUT26_IO_BYTEOFFSET (DOUT26_IO_OFFSET >> 3)
#define DOUT26_IO_BITMASK (1 << (DOUT26_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT27_IO_OFFSET
#define DOUT27_IO_OFFSET -1
#else
#define DOUT27_IO_BYTEOFFSET (DOUT27_IO_OFFSET >> 3)
#define DOUT27_IO_BITMASK (1 << (DOUT27_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT28_IO_OFFSET
#define DOUT28_IO_OFFSET -1
#else
#define DOUT28_IO_BYTEOFFSET (DOUT28_IO_OFFSET >> 3)
#define DOUT28_IO_BITMASK (1 << (DOUT28_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT29_IO_OFFSET
#define DOUT29_IO_OFFSET -1
#else
#define DOUT29_IO_BYTEOFFSET (DOUT29_IO_OFFSET >> 3)
#define DOUT29_IO_BITMASK (1 << (DOUT29_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT30_IO_OFFSET
#define DOUT30_IO_OFFSET -1
#else
#define DOUT30_IO_BYTEOFFSET (DOUT30_IO_OFFSET >> 3)
#define DOUT30_IO_BITMASK (1 << (DOUT30_IO_OFFSET & 0x7))
#ifndef IC74HC595_HAS_DOUTS
#define IC74HC595_HAS_DOUTS
#endif
#endif
#ifndef DOUT31_IO_OFFSET
#define DOUT31_IO_OFFSET -1
#else
#define DOUT31_IO_BYTEOFFSET (DOUT31_IO_OFFSET >> 3)
#define DOUT31_IO_BITMASK (1 << (DOUT31_IO_OFFSET & 0x7))
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

	// #define _IC74HC595_DELAY_CYCLES_0
	// #define _IC74HC595_DELAY_CYCLES_1 mcu_nop()
	// #define _IC74HC595_DELAY_CYCLES_2 _IC74HC595_DELAY_CYCLES_1();mcu_nop()
	// #define _IC74HC595_DELAY_CYCLES_3 _IC74HC595_DELAY_CYCLES_2();mcu_nop()
	// #define _IC74HC595_DELAY_CYCLES_4 _IC74HC595_DELAY_CYCLES_3();mcu_nop()
	// #define _IC74HC595_DELAY_CYCLES_5 _IC74HC595_DELAY_CYCLES_4();mcu_nop()
	// #define _IC74HC595_DELAY_CYCLES_6 _IC74HC595_DELAY_CYCLES_5();mcu_nop()
	// #define _IC74HC595_DELAY_CYCLES_7 _IC74HC595_DELAY_CYCLES_6();mcu_nop()
	// #define _IC74HC595_DELAY_CYCLES_8 _IC74HC595_DELAY_CYCLES_7();mcu_nop()
	// #define _IC74HC595_DELAY_CYCLES_9 _IC74HC595_DELAY_CYCLES_8();mcu_nop()
	// #define _IC74HC595_DELAY_CYCLES_10 _IC74HC595_DELAY_CYCLES_9();mcu_nop()

#define _IC74HC595_DELAY(X) \
	{                       \
		if (X)              \
		{                   \
			uint8_t t = X;  \
			do              \
			{               \
			} while (--t);  \
		}                   \
	}
#define IC74HC595_DELAY(X) _IC74HC595_DELAY(X)

	void ic74hc595_set_steps(uint8_t mask);
	void ic74hc595_toggle_steps(uint8_t mask);
	void ic74hc595_set_dirs(uint8_t mask);
	void ic74hc595_enable_steppers(uint8_t mask);
	void ic74hc595_set_pwms(uint16_t mask);
	void ic74hc595_set_servos(uint8_t mask);
	void ic74hc595_set_output(uint8_t pin, bool state);

#ifdef __cplusplus
}
#endif

#endif
