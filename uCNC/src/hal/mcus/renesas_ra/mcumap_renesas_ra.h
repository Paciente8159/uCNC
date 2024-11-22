/*
	Name: mcumap_rp2040.h
	Description: Contains all MCU and PIN definitions for RP2040 to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 16-01-2023

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef MCUMAP_RENESAS_RA_H
#define MCUMAP_RENESAS_RA_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>
#include <Arduino.h>

/*
	Generates all the interface definitions.
	This creates a middle HAL layer between the board IO pins and the AVR funtionalities
*/
/*
	MCU specific definitions and replacements
*/

/*
	RP2040 Defaults
*/
// defines the frequency of the mcu
#ifndef F_CPU
#define F_CPU 133000000L
#endif
// defines the maximum and minimum step rates
#ifndef F_STEP_MAX
#define F_STEP_MAX 64000
#endif
#ifndef F_STEP_MIN
#define F_STEP_MIN 1
#endif

#ifndef RP2040
#define RP2040
#endif

// defines special mcu to access flash strings and arrays
#define __rom__
#define __romstr__
#define rom_strptr *
#define rom_strcpy strcpy
#define rom_strncpy strncpy
#define rom_memcpy memcpy
#define rom_read_byte *

// needed by software delays
// this can be ignored since custom delay functions will be defined
#ifndef MCU_CLOCKS_PER_CYCLE
#define MCU_CLOCKS_PER_CYCLE 1
#endif
#ifndef MCU_CYCLES_PER_LOOP
#define MCU_CYCLES_PER_LOOP 1
#endif
#ifndef MCU_CYCLES_PER_LOOP_OVERHEAD
#define MCU_CYCLES_PER_LOOP_OVERHEAD 0
#endif

	// this next set of rules defines the internal delay macros
	// #define F_CPU_MHZ (F_CPU / 1000000UL)
	// #define US_TO_CYCLES(X) (X * F_CPU_MHZ)

	// 	extern unsigned long ulMainGetRunTimeCounterValue();
	/*
	#define mcu_delay_cycles(X)                                 \
	{                                                           \
		uint32_t target = ulMainGetRunTimeCounterValue() + (X); 	\
		while (target > ulMainGetRunTimeCounterValue())         	\
			;                                                   		\
	}
	*/
	// #define mcu_delay_100ns() mcu_delay_cycles(F_CPU_MHZ / 10UL)
	// #define mcu_delay_us(X) (mcu_delay_cycles(US_TO_CYCLES(X)))

#ifdef RX_BUFFER_CAPACITY
#define RX_BUFFER_CAPACITY 255
#endif

#define __SIZEOF_FLOAT__ 4

// used by the parser
// this method is faster then normal multiplication (for 32 bit for 16 and 8 bits is slightly lower)
// overrides utils.h definition to implement this method with or without fast math option enabled
#define fast_int_mul10(x) ((((x) << 2) + (x)) << 1)

// IO pins
#if (defined(STEP0_PORT) && defined(STEP0_BIT))
#define STEP0 1
#define STEP0_GPIO ((STEP0_PORT << 8) | STEP0_BIT)
#define DIO1 1
#define DIO1_PORT STEP0_PORT
#define DIO1_BIT STEP0_BIT
#define DIO1_GPIO STEP0_GPIO
#endif
#if (defined(STEP1_PORT) && defined(STEP1_BIT))
#define STEP1 2
#define STEP1_GPIO ((STEP1_PORT << 8) | STEP1_BIT)
#define DIO2 2
#define DIO2_PORT STEP1_PORT
#define DIO2_BIT STEP1_BIT
#define DIO2_GPIO STEP1_GPIO
#endif
#if (defined(STEP2_PORT) && defined(STEP2_BIT))
#define STEP2 3
#define STEP2_GPIO ((STEP2_PORT << 8) | STEP2_BIT)
#define DIO3 3
#define DIO3_PORT STEP2_PORT
#define DIO3_BIT STEP2_BIT
#define DIO3_GPIO STEP2_GPIO
#endif
#if (defined(STEP3_PORT) && defined(STEP3_BIT))
#define STEP3 4
#define STEP3_GPIO ((STEP3_PORT << 8) | STEP3_BIT)
#define DIO4 4
#define DIO4_PORT STEP3_PORT
#define DIO4_BIT STEP3_BIT
#define DIO4_GPIO STEP3_GPIO
#endif
#if (defined(STEP4_PORT) && defined(STEP4_BIT))
#define STEP4 5
#define STEP4_GPIO ((STEP4_PORT << 8) | STEP4_BIT)
#define DIO5 5
#define DIO5_PORT STEP4_PORT
#define DIO5_BIT STEP4_BIT
#define DIO5_GPIO STEP4_GPIO
#endif
#if (defined(STEP5_PORT) && defined(STEP5_BIT))
#define STEP5 6
#define STEP5_GPIO ((STEP5_PORT << 8) | STEP5_BIT)
#define DIO6 6
#define DIO6_PORT STEP5_PORT
#define DIO6_BIT STEP5_BIT
#define DIO6_GPIO STEP5_GPIO
#endif
#if (defined(STEP6_PORT) && defined(STEP6_BIT))
#define STEP6 7
#define STEP6_GPIO ((STEP6_PORT << 8) | STEP6_BIT)
#define DIO7 7
#define DIO7_PORT STEP6_PORT
#define DIO7_BIT STEP6_BIT
#define DIO7_GPIO STEP6_GPIO
#endif
#if (defined(STEP7_PORT) && defined(STEP7_BIT))
#define STEP7 8
#define STEP7_GPIO ((STEP7_PORT << 8) | STEP7_BIT)
#define DIO8 8
#define DIO8_PORT STEP7_PORT
#define DIO8_BIT STEP7_BIT
#define DIO8_GPIO STEP7_GPIO
#endif
#if (defined(DIR0_PORT) && defined(DIR0_BIT))
#define DIR0 9
#define DIR0_GPIO ((DIR0_PORT << 8) | DIR0_BIT)
#define DIO9 9
#define DIO9_PORT DIR0_PORT
#define DIO9_BIT DIR0_BIT
#define DIO9_GPIO DIR0_GPIO
#endif
#if (defined(DIR1_PORT) && defined(DIR1_BIT))
#define DIR1 10
#define DIR1_GPIO ((DIR1_PORT << 8) | DIR1_BIT)
#define DIO10 10
#define DIO10_PORT DIR1_PORT
#define DIO10_BIT DIR1_BIT
#define DIO10_GPIO DIR1_GPIO
#endif
#if (defined(DIR2_PORT) && defined(DIR2_BIT))
#define DIR2 11
#define DIR2_GPIO ((DIR2_PORT << 8) | DIR2_BIT)
#define DIO11 11
#define DIO11_PORT DIR2_PORT
#define DIO11_BIT DIR2_BIT
#define DIO11_GPIO DIR2_GPIO
#endif
#if (defined(DIR3_PORT) && defined(DIR3_BIT))
#define DIR3 12
#define DIR3_GPIO ((DIR3_PORT << 8) | DIR3_BIT)
#define DIO12 12
#define DIO12_PORT DIR3_PORT
#define DIO12_BIT DIR3_BIT
#define DIO12_GPIO DIR3_GPIO
#endif
#if (defined(DIR4_PORT) && defined(DIR4_BIT))
#define DIR4 13
#define DIR4_GPIO ((DIR4_PORT << 8) | DIR4_BIT)
#define DIO13 13
#define DIO13_PORT DIR4_PORT
#define DIO13_BIT DIR4_BIT
#define DIO13_GPIO DIR4_GPIO
#endif
#if (defined(DIR5_PORT) && defined(DIR5_BIT))
#define DIR5 14
#define DIR5_GPIO ((DIR5_PORT << 8) | DIR5_BIT)
#define DIO14 14
#define DIO14_PORT DIR5_PORT
#define DIO14_BIT DIR5_BIT
#define DIO14_GPIO DIR5_GPIO
#endif
#if (defined(DIR6_PORT) && defined(DIR6_BIT))
#define DIR6 15
#define DIR6_GPIO ((DIR6_PORT << 8) | DIR6_BIT)
#define DIO15 15
#define DIO15_PORT DIR6_PORT
#define DIO15_BIT DIR6_BIT
#define DIO15_GPIO DIR6_GPIO
#endif
#if (defined(DIR7_PORT) && defined(DIR7_BIT))
#define DIR7 16
#define DIR7_GPIO ((DIR7_PORT << 8) | DIR7_BIT)
#define DIO16 16
#define DIO16_PORT DIR7_PORT
#define DIO16_BIT DIR7_BIT
#define DIO16_GPIO DIR7_GPIO
#endif
#if (defined(STEP0_EN_PORT) && defined(STEP0_EN_BIT))
#define STEP0_EN 17
#define STEP0_EN_GPIO ((STEP0_EN_PORT << 8) | STEP0_EN_BIT)
#define DIO17 17
#define DIO17_PORT STEP0_EN_PORT
#define DIO17_BIT STEP0_EN_BIT
#define DIO17_GPIO STEP0_EN_GPIO
#endif
#if (defined(STEP1_EN_PORT) && defined(STEP1_EN_BIT))
#define STEP1_EN 18
#define STEP1_EN_GPIO ((STEP1_EN_PORT << 8) | STEP1_EN_BIT)
#define DIO18 18
#define DIO18_PORT STEP1_EN_PORT
#define DIO18_BIT STEP1_EN_BIT
#define DIO18_GPIO STEP1_EN_GPIO
#endif
#if (defined(STEP2_EN_PORT) && defined(STEP2_EN_BIT))
#define STEP2_EN 19
#define STEP2_EN_GPIO ((STEP2_EN_PORT << 8) | STEP2_EN_BIT)
#define DIO19 19
#define DIO19_PORT STEP2_EN_PORT
#define DIO19_BIT STEP2_EN_BIT
#define DIO19_GPIO STEP2_EN_GPIO
#endif
#if (defined(STEP3_EN_PORT) && defined(STEP3_EN_BIT))
#define STEP3_EN 20
#define STEP3_EN_GPIO ((STEP3_EN_PORT << 8) | STEP3_EN_BIT)
#define DIO20 20
#define DIO20_PORT STEP3_EN_PORT
#define DIO20_BIT STEP3_EN_BIT
#define DIO20_GPIO STEP3_EN_GPIO
#endif
#if (defined(STEP4_EN_PORT) && defined(STEP4_EN_BIT))
#define STEP4_EN 21
#define STEP4_EN_GPIO ((STEP4_EN_PORT << 8) | STEP4_EN_BIT)
#define DIO21 21
#define DIO21_PORT STEP4_EN_PORT
#define DIO21_BIT STEP4_EN_BIT
#define DIO21_GPIO STEP4_EN_GPIO
#endif
#if (defined(STEP5_EN_PORT) && defined(STEP5_EN_BIT))
#define STEP5_EN 22
#define STEP5_EN_GPIO ((STEP5_EN_PORT << 8) | STEP5_EN_BIT)
#define DIO22 22
#define DIO22_PORT STEP5_EN_PORT
#define DIO22_BIT STEP5_EN_BIT
#define DIO22_GPIO STEP5_EN_GPIO
#endif
#if (defined(STEP6_EN_PORT) && defined(STEP6_EN_BIT))
#define STEP6_EN 23
#define STEP6_EN_GPIO ((STEP6_EN_PORT << 8) | STEP6_EN_BIT)
#define DIO23 23
#define DIO23_PORT STEP6_EN_PORT
#define DIO23_BIT STEP6_EN_BIT
#define DIO23_GPIO STEP6_EN_GPIO
#endif
#if (defined(STEP7_EN_PORT) && defined(STEP7_EN_BIT))
#define STEP7_EN 24
#define STEP7_EN_GPIO ((STEP7_EN_PORT << 8) | STEP7_EN_BIT)
#define DIO24 24
#define DIO24_PORT STEP7_EN_PORT
#define DIO24_BIT STEP7_EN_BIT
#define DIO24_GPIO STEP7_EN_GPIO
#endif
#if (defined(PWM0_PORT) && defined(PWM0_BIT))
#define PWM0 25
#define PWM0_GPIO ((PWM0_PORT << 8) | PWM0_BIT)
#define DIO25 25
#define DIO25_PORT PWM0_PORT
#define DIO25_BIT PWM0_BIT
#define DIO25_GPIO PWM0_GPIO
#endif
#if (defined(PWM1_PORT) && defined(PWM1_BIT))
#define PWM1 26
#define PWM1_GPIO ((PWM1_PORT << 8) | PWM1_BIT)
#define DIO26 26
#define DIO26_PORT PWM1_PORT
#define DIO26_BIT PWM1_BIT
#define DIO26_GPIO PWM1_GPIO
#endif
#if (defined(PWM2_PORT) && defined(PWM2_BIT))
#define PWM2 27
#define PWM2_GPIO ((PWM2_PORT << 8) | PWM2_BIT)
#define DIO27 27
#define DIO27_PORT PWM2_PORT
#define DIO27_BIT PWM2_BIT
#define DIO27_GPIO PWM2_GPIO
#endif
#if (defined(PWM3_PORT) && defined(PWM3_BIT))
#define PWM3 28
#define PWM3_GPIO ((PWM3_PORT << 8) | PWM3_BIT)
#define DIO28 28
#define DIO28_PORT PWM3_PORT
#define DIO28_BIT PWM3_BIT
#define DIO28_GPIO PWM3_GPIO
#endif
#if (defined(PWM4_PORT) && defined(PWM4_BIT))
#define PWM4 29
#define PWM4_GPIO ((PWM4_PORT << 8) | PWM4_BIT)
#define DIO29 29
#define DIO29_PORT PWM4_PORT
#define DIO29_BIT PWM4_BIT
#define DIO29_GPIO PWM4_GPIO
#endif
#if (defined(PWM5_PORT) && defined(PWM5_BIT))
#define PWM5 30
#define PWM5_GPIO ((PWM5_PORT << 8) | PWM5_BIT)
#define DIO30 30
#define DIO30_PORT PWM5_PORT
#define DIO30_BIT PWM5_BIT
#define DIO30_GPIO PWM5_GPIO
#endif
#if (defined(PWM6_PORT) && defined(PWM6_BIT))
#define PWM6 31
#define PWM6_GPIO ((PWM6_PORT << 8) | PWM6_BIT)
#define DIO31 31
#define DIO31_PORT PWM6_PORT
#define DIO31_BIT PWM6_BIT
#define DIO31_GPIO PWM6_GPIO
#endif
#if (defined(PWM7_PORT) && defined(PWM7_BIT))
#define PWM7 32
#define PWM7_GPIO ((PWM7_PORT << 8) | PWM7_BIT)
#define DIO32 32
#define DIO32_PORT PWM7_PORT
#define DIO32_BIT PWM7_BIT
#define DIO32_GPIO PWM7_GPIO
#endif
#if (defined(PWM8_PORT) && defined(PWM8_BIT))
#define PWM8 33
#define PWM8_GPIO ((PWM8_PORT << 8) | PWM8_BIT)
#define DIO33 33
#define DIO33_PORT PWM8_PORT
#define DIO33_BIT PWM8_BIT
#define DIO33_GPIO PWM8_GPIO
#endif
#if (defined(PWM9_PORT) && defined(PWM9_BIT))
#define PWM9 34
#define PWM9_GPIO ((PWM9_PORT << 8) | PWM9_BIT)
#define DIO34 34
#define DIO34_PORT PWM9_PORT
#define DIO34_BIT PWM9_BIT
#define DIO34_GPIO PWM9_GPIO
#endif
#if (defined(PWM10_PORT) && defined(PWM10_BIT))
#define PWM10 35
#define PWM10_GPIO ((PWM10_PORT << 8) | PWM10_BIT)
#define DIO35 35
#define DIO35_PORT PWM10_PORT
#define DIO35_BIT PWM10_BIT
#define DIO35_GPIO PWM10_GPIO
#endif
#if (defined(PWM11_PORT) && defined(PWM11_BIT))
#define PWM11 36
#define PWM11_GPIO ((PWM11_PORT << 8) | PWM11_BIT)
#define DIO36 36
#define DIO36_PORT PWM11_PORT
#define DIO36_BIT PWM11_BIT
#define DIO36_GPIO PWM11_GPIO
#endif
#if (defined(PWM12_PORT) && defined(PWM12_BIT))
#define PWM12 37
#define PWM12_GPIO ((PWM12_PORT << 8) | PWM12_BIT)
#define DIO37 37
#define DIO37_PORT PWM12_PORT
#define DIO37_BIT PWM12_BIT
#define DIO37_GPIO PWM12_GPIO
#endif
#if (defined(PWM13_PORT) && defined(PWM13_BIT))
#define PWM13 38
#define PWM13_GPIO ((PWM13_PORT << 8) | PWM13_BIT)
#define DIO38 38
#define DIO38_PORT PWM13_PORT
#define DIO38_BIT PWM13_BIT
#define DIO38_GPIO PWM13_GPIO
#endif
#if (defined(PWM14_PORT) && defined(PWM14_BIT))
#define PWM14 39
#define PWM14_GPIO ((PWM14_PORT << 8) | PWM14_BIT)
#define DIO39 39
#define DIO39_PORT PWM14_PORT
#define DIO39_BIT PWM14_BIT
#define DIO39_GPIO PWM14_GPIO
#endif
#if (defined(PWM15_PORT) && defined(PWM15_BIT))
#define PWM15 40
#define PWM15_GPIO ((PWM15_PORT << 8) | PWM15_BIT)
#define DIO40 40
#define DIO40_PORT PWM15_PORT
#define DIO40_BIT PWM15_BIT
#define DIO40_GPIO PWM15_GPIO
#endif
#if (defined(SERVO0_PORT) && defined(SERVO0_BIT))
#define SERVO0 41
#define SERVO0_GPIO ((SERVO0_PORT << 8) | SERVO0_BIT)
#define DIO41 41
#define DIO41_PORT SERVO0_PORT
#define DIO41_BIT SERVO0_BIT
#define DIO41_GPIO SERVO0_GPIO
#endif
#if (defined(SERVO1_PORT) && defined(SERVO1_BIT))
#define SERVO1 42
#define SERVO1_GPIO ((SERVO1_PORT << 8) | SERVO1_BIT)
#define DIO42 42
#define DIO42_PORT SERVO1_PORT
#define DIO42_BIT SERVO1_BIT
#define DIO42_GPIO SERVO1_GPIO
#endif
#if (defined(SERVO2_PORT) && defined(SERVO2_BIT))
#define SERVO2 43
#define SERVO2_GPIO ((SERVO2_PORT << 8) | SERVO2_BIT)
#define DIO43 43
#define DIO43_PORT SERVO2_PORT
#define DIO43_BIT SERVO2_BIT
#define DIO43_GPIO SERVO2_GPIO
#endif
#if (defined(SERVO3_PORT) && defined(SERVO3_BIT))
#define SERVO3 44
#define SERVO3_GPIO ((SERVO3_PORT << 8) | SERVO3_BIT)
#define DIO44 44
#define DIO44_PORT SERVO3_PORT
#define DIO44_BIT SERVO3_BIT
#define DIO44_GPIO SERVO3_GPIO
#endif
#if (defined(SERVO4_PORT) && defined(SERVO4_BIT))
#define SERVO4 45
#define SERVO4_GPIO ((SERVO4_PORT << 8) | SERVO4_BIT)
#define DIO45 45
#define DIO45_PORT SERVO4_PORT
#define DIO45_BIT SERVO4_BIT
#define DIO45_GPIO SERVO4_GPIO
#endif
#if (defined(SERVO5_PORT) && defined(SERVO5_BIT))
#define SERVO5 46
#define SERVO5_GPIO ((SERVO5_PORT << 8) | SERVO5_BIT)
#define DIO46 46
#define DIO46_PORT SERVO5_PORT
#define DIO46_BIT SERVO5_BIT
#define DIO46_GPIO SERVO5_GPIO
#endif
#if (defined(DOUT0_PORT) && defined(DOUT0_BIT))
#define DOUT0 47
#define DOUT0_GPIO ((DOUT0_PORT << 8) | DOUT0_BIT)
#define DIO47 47
#define DIO47_PORT DOUT0_PORT
#define DIO47_BIT DOUT0_BIT
#define DIO47_GPIO DOUT0_GPIO
#endif
#if (defined(DOUT1_PORT) && defined(DOUT1_BIT))
#define DOUT1 48
#define DOUT1_GPIO ((DOUT1_PORT << 8) | DOUT1_BIT)
#define DIO48 48
#define DIO48_PORT DOUT1_PORT
#define DIO48_BIT DOUT1_BIT
#define DIO48_GPIO DOUT1_GPIO
#endif
#if (defined(DOUT2_PORT) && defined(DOUT2_BIT))
#define DOUT2 49
#define DOUT2_GPIO ((DOUT2_PORT << 8) | DOUT2_BIT)
#define DIO49 49
#define DIO49_PORT DOUT2_PORT
#define DIO49_BIT DOUT2_BIT
#define DIO49_GPIO DOUT2_GPIO
#endif
#if (defined(DOUT3_PORT) && defined(DOUT3_BIT))
#define DOUT3 50
#define DOUT3_GPIO ((DOUT3_PORT << 8) | DOUT3_BIT)
#define DIO50 50
#define DIO50_PORT DOUT3_PORT
#define DIO50_BIT DOUT3_BIT
#define DIO50_GPIO DOUT3_GPIO
#endif
#if (defined(DOUT4_PORT) && defined(DOUT4_BIT))
#define DOUT4 51
#define DOUT4_GPIO ((DOUT4_PORT << 8) | DOUT4_BIT)
#define DIO51 51
#define DIO51_PORT DOUT4_PORT
#define DIO51_BIT DOUT4_BIT
#define DIO51_GPIO DOUT4_GPIO
#endif
#if (defined(DOUT5_PORT) && defined(DOUT5_BIT))
#define DOUT5 52
#define DOUT5_GPIO ((DOUT5_PORT << 8) | DOUT5_BIT)
#define DIO52 52
#define DIO52_PORT DOUT5_PORT
#define DIO52_BIT DOUT5_BIT
#define DIO52_GPIO DOUT5_GPIO
#endif
#if (defined(DOUT6_PORT) && defined(DOUT6_BIT))
#define DOUT6 53
#define DOUT6_GPIO ((DOUT6_PORT << 8) | DOUT6_BIT)
#define DIO53 53
#define DIO53_PORT DOUT6_PORT
#define DIO53_BIT DOUT6_BIT
#define DIO53_GPIO DOUT6_GPIO
#endif
#if (defined(DOUT7_PORT) && defined(DOUT7_BIT))
#define DOUT7 54
#define DOUT7_GPIO ((DOUT7_PORT << 8) | DOUT7_BIT)
#define DIO54 54
#define DIO54_PORT DOUT7_PORT
#define DIO54_BIT DOUT7_BIT
#define DIO54_GPIO DOUT7_GPIO
#endif
#if (defined(DOUT8_PORT) && defined(DOUT8_BIT))
#define DOUT8 55
#define DOUT8_GPIO ((DOUT8_PORT << 8) | DOUT8_BIT)
#define DIO55 55
#define DIO55_PORT DOUT8_PORT
#define DIO55_BIT DOUT8_BIT
#define DIO55_GPIO DOUT8_GPIO
#endif
#if (defined(DOUT9_PORT) && defined(DOUT9_BIT))
#define DOUT9 56
#define DOUT9_GPIO ((DOUT9_PORT << 8) | DOUT9_BIT)
#define DIO56 56
#define DIO56_PORT DOUT9_PORT
#define DIO56_BIT DOUT9_BIT
#define DIO56_GPIO DOUT9_GPIO
#endif
#if (defined(DOUT10_PORT) && defined(DOUT10_BIT))
#define DOUT10 57
#define DOUT10_GPIO ((DOUT10_PORT << 8) | DOUT10_BIT)
#define DIO57 57
#define DIO57_PORT DOUT10_PORT
#define DIO57_BIT DOUT10_BIT
#define DIO57_GPIO DOUT10_GPIO
#endif
#if (defined(DOUT11_PORT) && defined(DOUT11_BIT))
#define DOUT11 58
#define DOUT11_GPIO ((DOUT11_PORT << 8) | DOUT11_BIT)
#define DIO58 58
#define DIO58_PORT DOUT11_PORT
#define DIO58_BIT DOUT11_BIT
#define DIO58_GPIO DOUT11_GPIO
#endif
#if (defined(DOUT12_PORT) && defined(DOUT12_BIT))
#define DOUT12 59
#define DOUT12_GPIO ((DOUT12_PORT << 8) | DOUT12_BIT)
#define DIO59 59
#define DIO59_PORT DOUT12_PORT
#define DIO59_BIT DOUT12_BIT
#define DIO59_GPIO DOUT12_GPIO
#endif
#if (defined(DOUT13_PORT) && defined(DOUT13_BIT))
#define DOUT13 60
#define DOUT13_GPIO ((DOUT13_PORT << 8) | DOUT13_BIT)
#define DIO60 60
#define DIO60_PORT DOUT13_PORT
#define DIO60_BIT DOUT13_BIT
#define DIO60_GPIO DOUT13_GPIO
#endif
#if (defined(DOUT14_PORT) && defined(DOUT14_BIT))
#define DOUT14 61
#define DOUT14_GPIO ((DOUT14_PORT << 8) | DOUT14_BIT)
#define DIO61 61
#define DIO61_PORT DOUT14_PORT
#define DIO61_BIT DOUT14_BIT
#define DIO61_GPIO DOUT14_GPIO
#endif
#if (defined(DOUT15_PORT) && defined(DOUT15_BIT))
#define DOUT15 62
#define DOUT15_GPIO ((DOUT15_PORT << 8) | DOUT15_BIT)
#define DIO62 62
#define DIO62_PORT DOUT15_PORT
#define DIO62_BIT DOUT15_BIT
#define DIO62_GPIO DOUT15_GPIO
#endif
#if (defined(DOUT16_PORT) && defined(DOUT16_BIT))
#define DOUT16 63
#define DOUT16_GPIO ((DOUT16_PORT << 8) | DOUT16_BIT)
#define DIO63 63
#define DIO63_PORT DOUT16_PORT
#define DIO63_BIT DOUT16_BIT
#define DIO63_GPIO DOUT16_GPIO
#endif
#if (defined(DOUT17_PORT) && defined(DOUT17_BIT))
#define DOUT17 64
#define DOUT17_GPIO ((DOUT17_PORT << 8) | DOUT17_BIT)
#define DIO64 64
#define DIO64_PORT DOUT17_PORT
#define DIO64_BIT DOUT17_BIT
#define DIO64_GPIO DOUT17_GPIO
#endif
#if (defined(DOUT18_PORT) && defined(DOUT18_BIT))
#define DOUT18 65
#define DOUT18_GPIO ((DOUT18_PORT << 8) | DOUT18_BIT)
#define DIO65 65
#define DIO65_PORT DOUT18_PORT
#define DIO65_BIT DOUT18_BIT
#define DIO65_GPIO DOUT18_GPIO
#endif
#if (defined(DOUT19_PORT) && defined(DOUT19_BIT))
#define DOUT19 66
#define DOUT19_GPIO ((DOUT19_PORT << 8) | DOUT19_BIT)
#define DIO66 66
#define DIO66_PORT DOUT19_PORT
#define DIO66_BIT DOUT19_BIT
#define DIO66_GPIO DOUT19_GPIO
#endif
#if (defined(DOUT20_PORT) && defined(DOUT20_BIT))
#define DOUT20 67
#define DOUT20_GPIO ((DOUT20_PORT << 8) | DOUT20_BIT)
#define DIO67 67
#define DIO67_PORT DOUT20_PORT
#define DIO67_BIT DOUT20_BIT
#define DIO67_GPIO DOUT20_GPIO
#endif
#if (defined(DOUT21_PORT) && defined(DOUT21_BIT))
#define DOUT21 68
#define DOUT21_GPIO ((DOUT21_PORT << 8) | DOUT21_BIT)
#define DIO68 68
#define DIO68_PORT DOUT21_PORT
#define DIO68_BIT DOUT21_BIT
#define DIO68_GPIO DOUT21_GPIO
#endif
#if (defined(DOUT22_PORT) && defined(DOUT22_BIT))
#define DOUT22 69
#define DOUT22_GPIO ((DOUT22_PORT << 8) | DOUT22_BIT)
#define DIO69 69
#define DIO69_PORT DOUT22_PORT
#define DIO69_BIT DOUT22_BIT
#define DIO69_GPIO DOUT22_GPIO
#endif
#if (defined(DOUT23_PORT) && defined(DOUT23_BIT))
#define DOUT23 70
#define DOUT23_GPIO ((DOUT23_PORT << 8) | DOUT23_BIT)
#define DIO70 70
#define DIO70_PORT DOUT23_PORT
#define DIO70_BIT DOUT23_BIT
#define DIO70_GPIO DOUT23_GPIO
#endif
#if (defined(DOUT24_PORT) && defined(DOUT24_BIT))
#define DOUT24 71
#define DOUT24_GPIO ((DOUT24_PORT << 8) | DOUT24_BIT)
#define DIO71 71
#define DIO71_PORT DOUT24_PORT
#define DIO71_BIT DOUT24_BIT
#define DIO71_GPIO DOUT24_GPIO
#endif
#if (defined(DOUT25_PORT) && defined(DOUT25_BIT))
#define DOUT25 72
#define DOUT25_GPIO ((DOUT25_PORT << 8) | DOUT25_BIT)
#define DIO72 72
#define DIO72_PORT DOUT25_PORT
#define DIO72_BIT DOUT25_BIT
#define DIO72_GPIO DOUT25_GPIO
#endif
#if (defined(DOUT26_PORT) && defined(DOUT26_BIT))
#define DOUT26 73
#define DOUT26_GPIO ((DOUT26_PORT << 8) | DOUT26_BIT)
#define DIO73 73
#define DIO73_PORT DOUT26_PORT
#define DIO73_BIT DOUT26_BIT
#define DIO73_GPIO DOUT26_GPIO
#endif
#if (defined(DOUT27_PORT) && defined(DOUT27_BIT))
#define DOUT27 74
#define DOUT27_GPIO ((DOUT27_PORT << 8) | DOUT27_BIT)
#define DIO74 74
#define DIO74_PORT DOUT27_PORT
#define DIO74_BIT DOUT27_BIT
#define DIO74_GPIO DOUT27_GPIO
#endif
#if (defined(DOUT28_PORT) && defined(DOUT28_BIT))
#define DOUT28 75
#define DOUT28_GPIO ((DOUT28_PORT << 8) | DOUT28_BIT)
#define DIO75 75
#define DIO75_PORT DOUT28_PORT
#define DIO75_BIT DOUT28_BIT
#define DIO75_GPIO DOUT28_GPIO
#endif
#if (defined(DOUT29_PORT) && defined(DOUT29_BIT))
#define DOUT29 76
#define DOUT29_GPIO ((DOUT29_PORT << 8) | DOUT29_BIT)
#define DIO76 76
#define DIO76_PORT DOUT29_PORT
#define DIO76_BIT DOUT29_BIT
#define DIO76_GPIO DOUT29_GPIO
#endif
#if (defined(DOUT30_PORT) && defined(DOUT30_BIT))
#define DOUT30 77
#define DOUT30_GPIO ((DOUT30_PORT << 8) | DOUT30_BIT)
#define DIO77 77
#define DIO77_PORT DOUT30_PORT
#define DIO77_BIT DOUT30_BIT
#define DIO77_GPIO DOUT30_GPIO
#endif
#if (defined(DOUT31_PORT) && defined(DOUT31_BIT))
#define DOUT31 78
#define DOUT31_GPIO ((DOUT31_PORT << 8) | DOUT31_BIT)
#define DIO78 78
#define DIO78_PORT DOUT31_PORT
#define DIO78_BIT DOUT31_BIT
#define DIO78_GPIO DOUT31_GPIO
#endif
#if (defined(DOUT32_PORT) && defined(DOUT32_BIT))
#define DOUT32 79
#define DOUT32_GPIO ((DOUT32_PORT << 8) | DOUT32_BIT)
#define DIO79 79
#define DIO79_PORT DOUT32_PORT
#define DIO79_BIT DOUT32_BIT
#define DIO79_GPIO DOUT32_GPIO
#endif
#if (defined(DOUT33_PORT) && defined(DOUT33_BIT))
#define DOUT33 80
#define DOUT33_GPIO ((DOUT33_PORT << 8) | DOUT33_BIT)
#define DIO80 80
#define DIO80_PORT DOUT33_PORT
#define DIO80_BIT DOUT33_BIT
#define DIO80_GPIO DOUT33_GPIO
#endif
#if (defined(DOUT34_PORT) && defined(DOUT34_BIT))
#define DOUT34 81
#define DOUT34_GPIO ((DOUT34_PORT << 8) | DOUT34_BIT)
#define DIO81 81
#define DIO81_PORT DOUT34_PORT
#define DIO81_BIT DOUT34_BIT
#define DIO81_GPIO DOUT34_GPIO
#endif
#if (defined(DOUT35_PORT) && defined(DOUT35_BIT))
#define DOUT35 82
#define DOUT35_GPIO ((DOUT35_PORT << 8) | DOUT35_BIT)
#define DIO82 82
#define DIO82_PORT DOUT35_PORT
#define DIO82_BIT DOUT35_BIT
#define DIO82_GPIO DOUT35_GPIO
#endif
#if (defined(DOUT36_PORT) && defined(DOUT36_BIT))
#define DOUT36 83
#define DOUT36_GPIO ((DOUT36_PORT << 8) | DOUT36_BIT)
#define DIO83 83
#define DIO83_PORT DOUT36_PORT
#define DIO83_BIT DOUT36_BIT
#define DIO83_GPIO DOUT36_GPIO
#endif
#if (defined(DOUT37_PORT) && defined(DOUT37_BIT))
#define DOUT37 84
#define DOUT37_GPIO ((DOUT37_PORT << 8) | DOUT37_BIT)
#define DIO84 84
#define DIO84_PORT DOUT37_PORT
#define DIO84_BIT DOUT37_BIT
#define DIO84_GPIO DOUT37_GPIO
#endif
#if (defined(DOUT38_PORT) && defined(DOUT38_BIT))
#define DOUT38 85
#define DOUT38_GPIO ((DOUT38_PORT << 8) | DOUT38_BIT)
#define DIO85 85
#define DIO85_PORT DOUT38_PORT
#define DIO85_BIT DOUT38_BIT
#define DIO85_GPIO DOUT38_GPIO
#endif
#if (defined(DOUT39_PORT) && defined(DOUT39_BIT))
#define DOUT39 86
#define DOUT39_GPIO ((DOUT39_PORT << 8) | DOUT39_BIT)
#define DIO86 86
#define DIO86_PORT DOUT39_PORT
#define DIO86_BIT DOUT39_BIT
#define DIO86_GPIO DOUT39_GPIO
#endif
#if (defined(DOUT40_PORT) && defined(DOUT40_BIT))
#define DOUT40 87
#define DOUT40_GPIO ((DOUT40_PORT << 8) | DOUT40_BIT)
#define DIO87 87
#define DIO87_PORT DOUT40_PORT
#define DIO87_BIT DOUT40_BIT
#define DIO87_GPIO DOUT40_GPIO
#endif
#if (defined(DOUT41_PORT) && defined(DOUT41_BIT))
#define DOUT41 88
#define DOUT41_GPIO ((DOUT41_PORT << 8) | DOUT41_BIT)
#define DIO88 88
#define DIO88_PORT DOUT41_PORT
#define DIO88_BIT DOUT41_BIT
#define DIO88_GPIO DOUT41_GPIO
#endif
#if (defined(DOUT42_PORT) && defined(DOUT42_BIT))
#define DOUT42 89
#define DOUT42_GPIO ((DOUT42_PORT << 8) | DOUT42_BIT)
#define DIO89 89
#define DIO89_PORT DOUT42_PORT
#define DIO89_BIT DOUT42_BIT
#define DIO89_GPIO DOUT42_GPIO
#endif
#if (defined(DOUT43_PORT) && defined(DOUT43_BIT))
#define DOUT43 90
#define DOUT43_GPIO ((DOUT43_PORT << 8) | DOUT43_BIT)
#define DIO90 90
#define DIO90_PORT DOUT43_PORT
#define DIO90_BIT DOUT43_BIT
#define DIO90_GPIO DOUT43_GPIO
#endif
#if (defined(DOUT44_PORT) && defined(DOUT44_BIT))
#define DOUT44 91
#define DOUT44_GPIO ((DOUT44_PORT << 8) | DOUT44_BIT)
#define DIO91 91
#define DIO91_PORT DOUT44_PORT
#define DIO91_BIT DOUT44_BIT
#define DIO91_GPIO DOUT44_GPIO
#endif
#if (defined(DOUT45_PORT) && defined(DOUT45_BIT))
#define DOUT45 92
#define DOUT45_GPIO ((DOUT45_PORT << 8) | DOUT45_BIT)
#define DIO92 92
#define DIO92_PORT DOUT45_PORT
#define DIO92_BIT DOUT45_BIT
#define DIO92_GPIO DOUT45_GPIO
#endif
#if (defined(DOUT46_PORT) && defined(DOUT46_BIT))
#define DOUT46 93
#define DOUT46_GPIO ((DOUT46_PORT << 8) | DOUT46_BIT)
#define DIO93 93
#define DIO93_PORT DOUT46_PORT
#define DIO93_BIT DOUT46_BIT
#define DIO93_GPIO DOUT46_GPIO
#endif
#if (defined(DOUT47_PORT) && defined(DOUT47_BIT))
#define DOUT47 94
#define DOUT47_GPIO ((DOUT47_PORT << 8) | DOUT47_BIT)
#define DIO94 94
#define DIO94_PORT DOUT47_PORT
#define DIO94_BIT DOUT47_BIT
#define DIO94_GPIO DOUT47_GPIO
#endif
#if (defined(DOUT48_PORT) && defined(DOUT48_BIT))
#define DOUT48 95
#define DOUT48_GPIO ((DOUT48_PORT << 8) | DOUT48_BIT)
#define DIO95 95
#define DIO95_PORT DOUT48_PORT
#define DIO95_BIT DOUT48_BIT
#define DIO95_GPIO DOUT48_GPIO
#endif
#if (defined(DOUT49_PORT) && defined(DOUT49_BIT))
#define DOUT49 96
#define DOUT49_GPIO ((DOUT49_PORT << 8) | DOUT49_BIT)
#define DIO96 96
#define DIO96_PORT DOUT49_PORT
#define DIO96_BIT DOUT49_BIT
#define DIO96_GPIO DOUT49_GPIO
#endif
#if (defined(LIMIT_X_PORT) && defined(LIMIT_X_BIT))
#define LIMIT_X 100
#define LIMIT_X_GPIO ((LIMIT_X_PORT << 8) | LIMIT_X_BIT)
#define DIO100 100
#define DIO100_PORT LIMIT_X_PORT
#define DIO100_BIT LIMIT_X_BIT
#define DIO100_GPIO LIMIT_X_GPIO
#endif
#if (defined(LIMIT_Y_PORT) && defined(LIMIT_Y_BIT))
#define LIMIT_Y 101
#define LIMIT_Y_GPIO ((LIMIT_Y_PORT << 8) | LIMIT_Y_BIT)
#define DIO101 101
#define DIO101_PORT LIMIT_Y_PORT
#define DIO101_BIT LIMIT_Y_BIT
#define DIO101_GPIO LIMIT_Y_GPIO
#endif
#if (defined(LIMIT_Z_PORT) && defined(LIMIT_Z_BIT))
#define LIMIT_Z 102
#define LIMIT_Z_GPIO ((LIMIT_Z_PORT << 8) | LIMIT_Z_BIT)
#define DIO102 102
#define DIO102_PORT LIMIT_Z_PORT
#define DIO102_BIT LIMIT_Z_BIT
#define DIO102_GPIO LIMIT_Z_GPIO
#endif
#if (defined(LIMIT_X2_PORT) && defined(LIMIT_X2_BIT))
#define LIMIT_X2 103
#define LIMIT_X2_GPIO ((LIMIT_X2_PORT << 8) | LIMIT_X2_BIT)
#define DIO103 103
#define DIO103_PORT LIMIT_X2_PORT
#define DIO103_BIT LIMIT_X2_BIT
#define DIO103_GPIO LIMIT_X2_GPIO
#endif
#if (defined(LIMIT_Y2_PORT) && defined(LIMIT_Y2_BIT))
#define LIMIT_Y2 104
#define LIMIT_Y2_GPIO ((LIMIT_Y2_PORT << 8) | LIMIT_Y2_BIT)
#define DIO104 104
#define DIO104_PORT LIMIT_Y2_PORT
#define DIO104_BIT LIMIT_Y2_BIT
#define DIO104_GPIO LIMIT_Y2_GPIO
#endif
#if (defined(LIMIT_Z2_PORT) && defined(LIMIT_Z2_BIT))
#define LIMIT_Z2 105
#define LIMIT_Z2_GPIO ((LIMIT_Z2_PORT << 8) | LIMIT_Z2_BIT)
#define DIO105 105
#define DIO105_PORT LIMIT_Z2_PORT
#define DIO105_BIT LIMIT_Z2_BIT
#define DIO105_GPIO LIMIT_Z2_GPIO
#endif
#if (defined(LIMIT_A_PORT) && defined(LIMIT_A_BIT))
#define LIMIT_A 106
#define LIMIT_A_GPIO ((LIMIT_A_PORT << 8) | LIMIT_A_BIT)
#define DIO106 106
#define DIO106_PORT LIMIT_A_PORT
#define DIO106_BIT LIMIT_A_BIT
#define DIO106_GPIO LIMIT_A_GPIO
#endif
#if (defined(LIMIT_B_PORT) && defined(LIMIT_B_BIT))
#define LIMIT_B 107
#define LIMIT_B_GPIO ((LIMIT_B_PORT << 8) | LIMIT_B_BIT)
#define DIO107 107
#define DIO107_PORT LIMIT_B_PORT
#define DIO107_BIT LIMIT_B_BIT
#define DIO107_GPIO LIMIT_B_GPIO
#endif
#if (defined(LIMIT_C_PORT) && defined(LIMIT_C_BIT))
#define LIMIT_C 108
#define LIMIT_C_GPIO ((LIMIT_C_PORT << 8) | LIMIT_C_BIT)
#define DIO108 108
#define DIO108_PORT LIMIT_C_PORT
#define DIO108_BIT LIMIT_C_BIT
#define DIO108_GPIO LIMIT_C_GPIO
#endif
#if (defined(PROBE_PORT) && defined(PROBE_BIT))
#define PROBE 109
#define PROBE_GPIO ((PROBE_PORT << 8) | PROBE_BIT)
#define DIO109 109
#define DIO109_PORT PROBE_PORT
#define DIO109_BIT PROBE_BIT
#define DIO109_GPIO PROBE_GPIO
#endif
#if (defined(ESTOP_PORT) && defined(ESTOP_BIT))
#define ESTOP 110
#define ESTOP_GPIO ((ESTOP_PORT << 8) | ESTOP_BIT)
#define DIO110 110
#define DIO110_PORT ESTOP_PORT
#define DIO110_BIT ESTOP_BIT
#define DIO110_GPIO ESTOP_GPIO
#endif
#if (defined(SAFETY_DOOR_PORT) && defined(SAFETY_DOOR_BIT))
#define SAFETY_DOOR 111
#define SAFETY_DOOR_GPIO ((SAFETY_DOOR_PORT << 8) | SAFETY_DOOR_BIT)
#define DIO111 111
#define DIO111_PORT SAFETY_DOOR_PORT
#define DIO111_BIT SAFETY_DOOR_BIT
#define DIO111_GPIO SAFETY_DOOR_GPIO
#endif
#if (defined(FHOLD_PORT) && defined(FHOLD_BIT))
#define FHOLD 112
#define FHOLD_GPIO ((FHOLD_PORT << 8) | FHOLD_BIT)
#define DIO112 112
#define DIO112_PORT FHOLD_PORT
#define DIO112_BIT FHOLD_BIT
#define DIO112_GPIO FHOLD_GPIO
#endif
#if (defined(CS_RES_PORT) && defined(CS_RES_BIT))
#define CS_RES 113
#define CS_RES_GPIO ((CS_RES_PORT << 8) | CS_RES_BIT)
#define DIO113 113
#define DIO113_PORT CS_RES_PORT
#define DIO113_BIT CS_RES_BIT
#define DIO113_GPIO CS_RES_GPIO
#endif
#if (defined(ANALOG0_PORT) && defined(ANALOG0_BIT))
#define ANALOG0 114
#define ANALOG0_GPIO ((ANALOG0_PORT << 8) | ANALOG0_BIT)
#define DIO114 114
#define DIO114_PORT ANALOG0_PORT
#define DIO114_BIT ANALOG0_BIT
#define DIO114_GPIO ANALOG0_GPIO
#endif
#if (defined(ANALOG1_PORT) && defined(ANALOG1_BIT))
#define ANALOG1 115
#define ANALOG1_GPIO ((ANALOG1_PORT << 8) | ANALOG1_BIT)
#define DIO115 115
#define DIO115_PORT ANALOG1_PORT
#define DIO115_BIT ANALOG1_BIT
#define DIO115_GPIO ANALOG1_GPIO
#endif
#if (defined(ANALOG2_PORT) && defined(ANALOG2_BIT))
#define ANALOG2 116
#define ANALOG2_GPIO ((ANALOG2_PORT << 8) | ANALOG2_BIT)
#define DIO116 116
#define DIO116_PORT ANALOG2_PORT
#define DIO116_BIT ANALOG2_BIT
#define DIO116_GPIO ANALOG2_GPIO
#endif
#if (defined(ANALOG3_PORT) && defined(ANALOG3_BIT))
#define ANALOG3 117
#define ANALOG3_GPIO ((ANALOG3_PORT << 8) | ANALOG3_BIT)
#define DIO117 117
#define DIO117_PORT ANALOG3_PORT
#define DIO117_BIT ANALOG3_BIT
#define DIO117_GPIO ANALOG3_GPIO
#endif
#if (defined(ANALOG4_PORT) && defined(ANALOG4_BIT))
#define ANALOG4 118
#define ANALOG4_GPIO ((ANALOG4_PORT << 8) | ANALOG4_BIT)
#define DIO118 118
#define DIO118_PORT ANALOG4_PORT
#define DIO118_BIT ANALOG4_BIT
#define DIO118_GPIO ANALOG4_GPIO
#endif
#if (defined(ANALOG5_PORT) && defined(ANALOG5_BIT))
#define ANALOG5 119
#define ANALOG5_GPIO ((ANALOG5_PORT << 8) | ANALOG5_BIT)
#define DIO119 119
#define DIO119_PORT ANALOG5_PORT
#define DIO119_BIT ANALOG5_BIT
#define DIO119_GPIO ANALOG5_GPIO
#endif
#if (defined(ANALOG6_PORT) && defined(ANALOG6_BIT))
#define ANALOG6 120
#define ANALOG6_GPIO ((ANALOG6_PORT << 8) | ANALOG6_BIT)
#define DIO120 120
#define DIO120_PORT ANALOG6_PORT
#define DIO120_BIT ANALOG6_BIT
#define DIO120_GPIO ANALOG6_GPIO
#endif
#if (defined(ANALOG7_PORT) && defined(ANALOG7_BIT))
#define ANALOG7 121
#define ANALOG7_GPIO ((ANALOG7_PORT << 8) | ANALOG7_BIT)
#define DIO121 121
#define DIO121_PORT ANALOG7_PORT
#define DIO121_BIT ANALOG7_BIT
#define DIO121_GPIO ANALOG7_GPIO
#endif
#if (defined(ANALOG8_PORT) && defined(ANALOG8_BIT))
#define ANALOG8 122
#define ANALOG8_GPIO ((ANALOG8_PORT << 8) | ANALOG8_BIT)
#define DIO122 122
#define DIO122_PORT ANALOG8_PORT
#define DIO122_BIT ANALOG8_BIT
#define DIO122_GPIO ANALOG8_GPIO
#endif
#if (defined(ANALOG9_PORT) && defined(ANALOG9_BIT))
#define ANALOG9 123
#define ANALOG9_GPIO ((ANALOG9_PORT << 8) | ANALOG9_BIT)
#define DIO123 123
#define DIO123_PORT ANALOG9_PORT
#define DIO123_BIT ANALOG9_BIT
#define DIO123_GPIO ANALOG9_GPIO
#endif
#if (defined(ANALOG10_PORT) && defined(ANALOG10_BIT))
#define ANALOG10 124
#define ANALOG10_GPIO ((ANALOG10_PORT << 8) | ANALOG10_BIT)
#define DIO124 124
#define DIO124_PORT ANALOG10_PORT
#define DIO124_BIT ANALOG10_BIT
#define DIO124_GPIO ANALOG10_GPIO
#endif
#if (defined(ANALOG11_PORT) && defined(ANALOG11_BIT))
#define ANALOG11 125
#define ANALOG11_GPIO ((ANALOG11_PORT << 8) | ANALOG11_BIT)
#define DIO125 125
#define DIO125_PORT ANALOG11_PORT
#define DIO125_BIT ANALOG11_BIT
#define DIO125_GPIO ANALOG11_GPIO
#endif
#if (defined(ANALOG12_PORT) && defined(ANALOG12_BIT))
#define ANALOG12 126
#define ANALOG12_GPIO ((ANALOG12_PORT << 8) | ANALOG12_BIT)
#define DIO126 126
#define DIO126_PORT ANALOG12_PORT
#define DIO126_BIT ANALOG12_BIT
#define DIO126_GPIO ANALOG12_GPIO
#endif
#if (defined(ANALOG13_PORT) && defined(ANALOG13_BIT))
#define ANALOG13 127
#define ANALOG13_GPIO ((ANALOG13_PORT << 8) | ANALOG13_BIT)
#define DIO127 127
#define DIO127_PORT ANALOG13_PORT
#define DIO127_BIT ANALOG13_BIT
#define DIO127_GPIO ANALOG13_GPIO
#endif
#if (defined(ANALOG14_PORT) && defined(ANALOG14_BIT))
#define ANALOG14 128
#define ANALOG14_GPIO ((ANALOG14_PORT << 8) | ANALOG14_BIT)
#define DIO128 128
#define DIO128_PORT ANALOG14_PORT
#define DIO128_BIT ANALOG14_BIT
#define DIO128_GPIO ANALOG14_GPIO
#endif
#if (defined(ANALOG15_PORT) && defined(ANALOG15_BIT))
#define ANALOG15 129
#define ANALOG15_GPIO ((ANALOG15_PORT << 8) | ANALOG15_BIT)
#define DIO129 129
#define DIO129_PORT ANALOG15_PORT
#define DIO129_BIT ANALOG15_BIT
#define DIO129_GPIO ANALOG15_GPIO
#endif
#if (defined(DIN0_PORT) && defined(DIN0_BIT))
#define DIN0 130
#define DIN0_GPIO ((DIN0_PORT << 8) | DIN0_BIT)
#define DIO130 130
#define DIO130_PORT DIN0_PORT
#define DIO130_BIT DIN0_BIT
#define DIO130_GPIO DIN0_GPIO
#endif
#if (defined(DIN1_PORT) && defined(DIN1_BIT))
#define DIN1 131
#define DIN1_GPIO ((DIN1_PORT << 8) | DIN1_BIT)
#define DIO131 131
#define DIO131_PORT DIN1_PORT
#define DIO131_BIT DIN1_BIT
#define DIO131_GPIO DIN1_GPIO
#endif
#if (defined(DIN2_PORT) && defined(DIN2_BIT))
#define DIN2 132
#define DIN2_GPIO ((DIN2_PORT << 8) | DIN2_BIT)
#define DIO132 132
#define DIO132_PORT DIN2_PORT
#define DIO132_BIT DIN2_BIT
#define DIO132_GPIO DIN2_GPIO
#endif
#if (defined(DIN3_PORT) && defined(DIN3_BIT))
#define DIN3 133
#define DIN3_GPIO ((DIN3_PORT << 8) | DIN3_BIT)
#define DIO133 133
#define DIO133_PORT DIN3_PORT
#define DIO133_BIT DIN3_BIT
#define DIO133_GPIO DIN3_GPIO
#endif
#if (defined(DIN4_PORT) && defined(DIN4_BIT))
#define DIN4 134
#define DIN4_GPIO ((DIN4_PORT << 8) | DIN4_BIT)
#define DIO134 134
#define DIO134_PORT DIN4_PORT
#define DIO134_BIT DIN4_BIT
#define DIO134_GPIO DIN4_GPIO
#endif
#if (defined(DIN5_PORT) && defined(DIN5_BIT))
#define DIN5 135
#define DIN5_GPIO ((DIN5_PORT << 8) | DIN5_BIT)
#define DIO135 135
#define DIO135_PORT DIN5_PORT
#define DIO135_BIT DIN5_BIT
#define DIO135_GPIO DIN5_GPIO
#endif
#if (defined(DIN6_PORT) && defined(DIN6_BIT))
#define DIN6 136
#define DIN6_GPIO ((DIN6_PORT << 8) | DIN6_BIT)
#define DIO136 136
#define DIO136_PORT DIN6_PORT
#define DIO136_BIT DIN6_BIT
#define DIO136_GPIO DIN6_GPIO
#endif
#if (defined(DIN7_PORT) && defined(DIN7_BIT))
#define DIN7 137
#define DIN7_GPIO ((DIN7_PORT << 8) | DIN7_BIT)
#define DIO137 137
#define DIO137_PORT DIN7_PORT
#define DIO137_BIT DIN7_BIT
#define DIO137_GPIO DIN7_GPIO
#endif
#if (defined(DIN8_PORT) && defined(DIN8_BIT))
#define DIN8 138
#define DIN8_GPIO ((DIN8_PORT << 8) | DIN8_BIT)
#define DIO138 138
#define DIO138_PORT DIN8_PORT
#define DIO138_BIT DIN8_BIT
#define DIO138_GPIO DIN8_GPIO
#endif
#if (defined(DIN9_PORT) && defined(DIN9_BIT))
#define DIN9 139
#define DIN9_GPIO ((DIN9_PORT << 8) | DIN9_BIT)
#define DIO139 139
#define DIO139_PORT DIN9_PORT
#define DIO139_BIT DIN9_BIT
#define DIO139_GPIO DIN9_GPIO
#endif
#if (defined(DIN10_PORT) && defined(DIN10_BIT))
#define DIN10 140
#define DIN10_GPIO ((DIN10_PORT << 8) | DIN10_BIT)
#define DIO140 140
#define DIO140_PORT DIN10_PORT
#define DIO140_BIT DIN10_BIT
#define DIO140_GPIO DIN10_GPIO
#endif
#if (defined(DIN11_PORT) && defined(DIN11_BIT))
#define DIN11 141
#define DIN11_GPIO ((DIN11_PORT << 8) | DIN11_BIT)
#define DIO141 141
#define DIO141_PORT DIN11_PORT
#define DIO141_BIT DIN11_BIT
#define DIO141_GPIO DIN11_GPIO
#endif
#if (defined(DIN12_PORT) && defined(DIN12_BIT))
#define DIN12 142
#define DIN12_GPIO ((DIN12_PORT << 8) | DIN12_BIT)
#define DIO142 142
#define DIO142_PORT DIN12_PORT
#define DIO142_BIT DIN12_BIT
#define DIO142_GPIO DIN12_GPIO
#endif
#if (defined(DIN13_PORT) && defined(DIN13_BIT))
#define DIN13 143
#define DIN13_GPIO ((DIN13_PORT << 8) | DIN13_BIT)
#define DIO143 143
#define DIO143_PORT DIN13_PORT
#define DIO143_BIT DIN13_BIT
#define DIO143_GPIO DIN13_GPIO
#endif
#if (defined(DIN14_PORT) && defined(DIN14_BIT))
#define DIN14 144
#define DIN14_GPIO ((DIN14_PORT << 8) | DIN14_BIT)
#define DIO144 144
#define DIO144_PORT DIN14_PORT
#define DIO144_BIT DIN14_BIT
#define DIO144_GPIO DIN14_GPIO
#endif
#if (defined(DIN15_PORT) && defined(DIN15_BIT))
#define DIN15 145
#define DIN15_GPIO ((DIN15_PORT << 8) | DIN15_BIT)
#define DIO145 145
#define DIO145_PORT DIN15_PORT
#define DIO145_BIT DIN15_BIT
#define DIO145_GPIO DIN15_GPIO
#endif
#if (defined(DIN16_PORT) && defined(DIN16_BIT))
#define DIN16 146
#define DIN16_GPIO ((DIN16_PORT << 8) | DIN16_BIT)
#define DIO146 146
#define DIO146_PORT DIN16_PORT
#define DIO146_BIT DIN16_BIT
#define DIO146_GPIO DIN16_GPIO
#endif
#if (defined(DIN17_PORT) && defined(DIN17_BIT))
#define DIN17 147
#define DIN17_GPIO ((DIN17_PORT << 8) | DIN17_BIT)
#define DIO147 147
#define DIO147_PORT DIN17_PORT
#define DIO147_BIT DIN17_BIT
#define DIO147_GPIO DIN17_GPIO
#endif
#if (defined(DIN18_PORT) && defined(DIN18_BIT))
#define DIN18 148
#define DIN18_GPIO ((DIN18_PORT << 8) | DIN18_BIT)
#define DIO148 148
#define DIO148_PORT DIN18_PORT
#define DIO148_BIT DIN18_BIT
#define DIO148_GPIO DIN18_GPIO
#endif
#if (defined(DIN19_PORT) && defined(DIN19_BIT))
#define DIN19 149
#define DIN19_GPIO ((DIN19_PORT << 8) | DIN19_BIT)
#define DIO149 149
#define DIO149_PORT DIN19_PORT
#define DIO149_BIT DIN19_BIT
#define DIO149_GPIO DIN19_GPIO
#endif
#if (defined(DIN20_PORT) && defined(DIN20_BIT))
#define DIN20 150
#define DIN20_GPIO ((DIN20_PORT << 8) | DIN20_BIT)
#define DIO150 150
#define DIO150_PORT DIN20_PORT
#define DIO150_BIT DIN20_BIT
#define DIO150_GPIO DIN20_GPIO
#endif
#if (defined(DIN21_PORT) && defined(DIN21_BIT))
#define DIN21 151
#define DIN21_GPIO ((DIN21_PORT << 8) | DIN21_BIT)
#define DIO151 151
#define DIO151_PORT DIN21_PORT
#define DIO151_BIT DIN21_BIT
#define DIO151_GPIO DIN21_GPIO
#endif
#if (defined(DIN22_PORT) && defined(DIN22_BIT))
#define DIN22 152
#define DIN22_GPIO ((DIN22_PORT << 8) | DIN22_BIT)
#define DIO152 152
#define DIO152_PORT DIN22_PORT
#define DIO152_BIT DIN22_BIT
#define DIO152_GPIO DIN22_GPIO
#endif
#if (defined(DIN23_PORT) && defined(DIN23_BIT))
#define DIN23 153
#define DIN23_GPIO ((DIN23_PORT << 8) | DIN23_BIT)
#define DIO153 153
#define DIO153_PORT DIN23_PORT
#define DIO153_BIT DIN23_BIT
#define DIO153_GPIO DIN23_GPIO
#endif
#if (defined(DIN24_PORT) && defined(DIN24_BIT))
#define DIN24 154
#define DIN24_GPIO ((DIN24_PORT << 8) | DIN24_BIT)
#define DIO154 154
#define DIO154_PORT DIN24_PORT
#define DIO154_BIT DIN24_BIT
#define DIO154_GPIO DIN24_GPIO
#endif
#if (defined(DIN25_PORT) && defined(DIN25_BIT))
#define DIN25 155
#define DIN25_GPIO ((DIN25_PORT << 8) | DIN25_BIT)
#define DIO155 155
#define DIO155_PORT DIN25_PORT
#define DIO155_BIT DIN25_BIT
#define DIO155_GPIO DIN25_GPIO
#endif
#if (defined(DIN26_PORT) && defined(DIN26_BIT))
#define DIN26 156
#define DIN26_GPIO ((DIN26_PORT << 8) | DIN26_BIT)
#define DIO156 156
#define DIO156_PORT DIN26_PORT
#define DIO156_BIT DIN26_BIT
#define DIO156_GPIO DIN26_GPIO
#endif
#if (defined(DIN27_PORT) && defined(DIN27_BIT))
#define DIN27 157
#define DIN27_GPIO ((DIN27_PORT << 8) | DIN27_BIT)
#define DIO157 157
#define DIO157_PORT DIN27_PORT
#define DIO157_BIT DIN27_BIT
#define DIO157_GPIO DIN27_GPIO
#endif
#if (defined(DIN28_PORT) && defined(DIN28_BIT))
#define DIN28 158
#define DIN28_GPIO ((DIN28_PORT << 8) | DIN28_BIT)
#define DIO158 158
#define DIO158_PORT DIN28_PORT
#define DIO158_BIT DIN28_BIT
#define DIO158_GPIO DIN28_GPIO
#endif
#if (defined(DIN29_PORT) && defined(DIN29_BIT))
#define DIN29 159
#define DIN29_GPIO ((DIN29_PORT << 8) | DIN29_BIT)
#define DIO159 159
#define DIO159_PORT DIN29_PORT
#define DIO159_BIT DIN29_BIT
#define DIO159_GPIO DIN29_GPIO
#endif
#if (defined(DIN30_PORT) && defined(DIN30_BIT))
#define DIN30 160
#define DIN30_GPIO ((DIN30_PORT << 8) | DIN30_BIT)
#define DIO160 160
#define DIO160_PORT DIN30_PORT
#define DIO160_BIT DIN30_BIT
#define DIO160_GPIO DIN30_GPIO
#endif
#if (defined(DIN31_PORT) && defined(DIN31_BIT))
#define DIN31 161
#define DIN31_GPIO ((DIN31_PORT << 8) | DIN31_BIT)
#define DIO161 161
#define DIO161_PORT DIN31_PORT
#define DIO161_BIT DIN31_BIT
#define DIO161_GPIO DIN31_GPIO
#endif
#if (defined(DIN32_PORT) && defined(DIN32_BIT))
#define DIN32 162
#define DIN32_GPIO ((DIN32_PORT << 8) | DIN32_BIT)
#define DIO162 162
#define DIO162_PORT DIN32_PORT
#define DIO162_BIT DIN32_BIT
#define DIO162_GPIO DIN32_GPIO
#endif
#if (defined(DIN33_PORT) && defined(DIN33_BIT))
#define DIN33 163
#define DIN33_GPIO ((DIN33_PORT << 8) | DIN33_BIT)
#define DIO163 163
#define DIO163_PORT DIN33_PORT
#define DIO163_BIT DIN33_BIT
#define DIO163_GPIO DIN33_GPIO
#endif
#if (defined(DIN34_PORT) && defined(DIN34_BIT))
#define DIN34 164
#define DIN34_GPIO ((DIN34_PORT << 8) | DIN34_BIT)
#define DIO164 164
#define DIO164_PORT DIN34_PORT
#define DIO164_BIT DIN34_BIT
#define DIO164_GPIO DIN34_GPIO
#endif
#if (defined(DIN35_PORT) && defined(DIN35_BIT))
#define DIN35 165
#define DIN35_GPIO ((DIN35_PORT << 8) | DIN35_BIT)
#define DIO165 165
#define DIO165_PORT DIN35_PORT
#define DIO165_BIT DIN35_BIT
#define DIO165_GPIO DIN35_GPIO
#endif
#if (defined(DIN36_PORT) && defined(DIN36_BIT))
#define DIN36 166
#define DIN36_GPIO ((DIN36_PORT << 8) | DIN36_BIT)
#define DIO166 166
#define DIO166_PORT DIN36_PORT
#define DIO166_BIT DIN36_BIT
#define DIO166_GPIO DIN36_GPIO
#endif
#if (defined(DIN37_PORT) && defined(DIN37_BIT))
#define DIN37 167
#define DIN37_GPIO ((DIN37_PORT << 8) | DIN37_BIT)
#define DIO167 167
#define DIO167_PORT DIN37_PORT
#define DIO167_BIT DIN37_BIT
#define DIO167_GPIO DIN37_GPIO
#endif
#if (defined(DIN38_PORT) && defined(DIN38_BIT))
#define DIN38 168
#define DIN38_GPIO ((DIN38_PORT << 8) | DIN38_BIT)
#define DIO168 168
#define DIO168_PORT DIN38_PORT
#define DIO168_BIT DIN38_BIT
#define DIO168_GPIO DIN38_GPIO
#endif
#if (defined(DIN39_PORT) && defined(DIN39_BIT))
#define DIN39 169
#define DIN39_GPIO ((DIN39_PORT << 8) | DIN39_BIT)
#define DIO169 169
#define DIO169_PORT DIN39_PORT
#define DIO169_BIT DIN39_BIT
#define DIO169_GPIO DIN39_GPIO
#endif
#if (defined(DIN40_PORT) && defined(DIN40_BIT))
#define DIN40 170
#define DIN40_GPIO ((DIN40_PORT << 8) | DIN40_BIT)
#define DIO170 170
#define DIO170_PORT DIN40_PORT
#define DIO170_BIT DIN40_BIT
#define DIO170_GPIO DIN40_GPIO
#endif
#if (defined(DIN41_PORT) && defined(DIN41_BIT))
#define DIN41 171
#define DIN41_GPIO ((DIN41_PORT << 8) | DIN41_BIT)
#define DIO171 171
#define DIO171_PORT DIN41_PORT
#define DIO171_BIT DIN41_BIT
#define DIO171_GPIO DIN41_GPIO
#endif
#if (defined(DIN42_PORT) && defined(DIN42_BIT))
#define DIN42 172
#define DIN42_GPIO ((DIN42_PORT << 8) | DIN42_BIT)
#define DIO172 172
#define DIO172_PORT DIN42_PORT
#define DIO172_BIT DIN42_BIT
#define DIO172_GPIO DIN42_GPIO
#endif
#if (defined(DIN43_PORT) && defined(DIN43_BIT))
#define DIN43 173
#define DIN43_GPIO ((DIN43_PORT << 8) | DIN43_BIT)
#define DIO173 173
#define DIO173_PORT DIN43_PORT
#define DIO173_BIT DIN43_BIT
#define DIO173_GPIO DIN43_GPIO
#endif
#if (defined(DIN44_PORT) && defined(DIN44_BIT))
#define DIN44 174
#define DIN44_GPIO ((DIN44_PORT << 8) | DIN44_BIT)
#define DIO174 174
#define DIO174_PORT DIN44_PORT
#define DIO174_BIT DIN44_BIT
#define DIO174_GPIO DIN44_GPIO
#endif
#if (defined(DIN45_PORT) && defined(DIN45_BIT))
#define DIN45 175
#define DIN45_GPIO ((DIN45_PORT << 8) | DIN45_BIT)
#define DIO175 175
#define DIO175_PORT DIN45_PORT
#define DIO175_BIT DIN45_BIT
#define DIO175_GPIO DIN45_GPIO
#endif
#if (defined(DIN46_PORT) && defined(DIN46_BIT))
#define DIN46 176
#define DIN46_GPIO ((DIN46_PORT << 8) | DIN46_BIT)
#define DIO176 176
#define DIO176_PORT DIN46_PORT
#define DIO176_BIT DIN46_BIT
#define DIO176_GPIO DIN46_GPIO
#endif
#if (defined(DIN47_PORT) && defined(DIN47_BIT))
#define DIN47 177
#define DIN47_GPIO ((DIN47_PORT << 8) | DIN47_BIT)
#define DIO177 177
#define DIO177_PORT DIN47_PORT
#define DIO177_BIT DIN47_BIT
#define DIO177_GPIO DIN47_GPIO
#endif
#if (defined(DIN48_PORT) && defined(DIN48_BIT))
#define DIN48 178
#define DIN48_GPIO ((DIN48_PORT << 8) | DIN48_BIT)
#define DIO178 178
#define DIO178_PORT DIN48_PORT
#define DIO178_BIT DIN48_BIT
#define DIO178_GPIO DIN48_GPIO
#endif
#if (defined(DIN49_PORT) && defined(DIN49_BIT))
#define DIN49 179
#define DIN49_GPIO ((DIN49_PORT << 8) | DIN49_BIT)
#define DIO179 179
#define DIO179_PORT DIN49_PORT
#define DIO179_BIT DIN49_BIT
#define DIO179_GPIO DIN49_GPIO
#endif
#if (defined(TX_PORT) && defined(TX_BIT))
#define TX 200
#define TX_GPIO ((TX_PORT << 8) | TX_BIT)
#define DIO200 200
#define DIO200_PORT TX_PORT
#define DIO200_BIT TX_BIT
#define DIO200_GPIO TX_GPIO
#endif
#if (defined(RX_PORT) && defined(RX_BIT))
#define RX 201
#define RX_GPIO ((RX_PORT << 8) | RX_BIT)
#define DIO201 201
#define DIO201_PORT RX_PORT
#define DIO201_BIT RX_BIT
#define DIO201_GPIO RX_GPIO
#endif
#if (defined(USB_DM_PORT) && defined(USB_DM_BIT))
#define USB_DM 202
#define USB_DM_GPIO ((USB_DM_PORT << 8) | USB_DM_BIT)
#define DIO202 202
#define DIO202_PORT USB_DM_PORT
#define DIO202_BIT USB_DM_BIT
#define DIO202_GPIO USB_DM_GPIO
#endif
#if (defined(USB_DP_PORT) && defined(USB_DP_BIT))
#define USB_DP 203
#define USB_DP_GPIO ((USB_DP_PORT << 8) | USB_DP_BIT)
#define DIO203 203
#define DIO203_PORT USB_DP_PORT
#define DIO203_BIT USB_DP_BIT
#define DIO203_GPIO USB_DP_GPIO
#endif
#if (defined(SPI_CLK_PORT) && defined(SPI_CLK_BIT))
#define SPI_CLK 204
#define SPI_CLK_GPIO ((SPI_CLK_PORT << 8) | SPI_CLK_BIT)
#define DIO204 204
#define DIO204_PORT SPI_CLK_PORT
#define DIO204_BIT SPI_CLK_BIT
#define DIO204_GPIO SPI_CLK_GPIO
#endif
#if (defined(SPI_SDI_PORT) && defined(SPI_SDI_BIT))
#define SPI_SDI 205
#define SPI_SDI_GPIO ((SPI_SDI_PORT << 8) | SPI_SDI_BIT)
#define DIO205 205
#define DIO205_PORT SPI_SDI_PORT
#define DIO205_BIT SPI_SDI_BIT
#define DIO205_GPIO SPI_SDI_GPIO
#endif
#if (defined(SPI_SDO_PORT) && defined(SPI_SDO_BIT))
#define SPI_SDO 206
#define SPI_SDO_GPIO ((SPI_SDO_PORT << 8) | SPI_SDO_BIT)
#define DIO206 206
#define DIO206_PORT SPI_SDO_PORT
#define DIO206_BIT SPI_SDO_BIT
#define DIO206_GPIO SPI_SDO_GPIO
#endif
#if (defined(SPI_CS_PORT) && defined(SPI_CS_BIT))
#define SPI_CS 207
#define SPI_CS_GPIO ((SPI_CS_PORT << 8) | SPI_CS_BIT)
#define DIO207 207
#define DIO207_PORT SPI_CS_PORT
#define DIO207_BIT SPI_CS_BIT
#define DIO207_GPIO SPI_CS_GPIO
#endif
#if (defined(I2C_SCL_PORT) && defined(I2C_SCL_BIT))
#define I2C_SCL 208
#define I2C_SCL_GPIO ((I2C_SCL_PORT << 8) | I2C_SCL_BIT)
#define DIO208 208
#define DIO208_PORT I2C_SCL_PORT
#define DIO208_BIT I2C_SCL_BIT
#define DIO208_GPIO I2C_SCL_GPIO
#endif
#if (defined(I2C_SDA_PORT) && defined(I2C_SDA_BIT))
#define I2C_SDA 209
#define I2C_SDA_GPIO ((I2C_SDA_PORT << 8) | I2C_SDA_BIT)
#define DIO209 209
#define DIO209_PORT I2C_SDA_PORT
#define DIO209_BIT I2C_SDA_BIT
#define DIO209_GPIO I2C_SDA_GPIO
#endif
#if (defined(TX2_PORT) && defined(TX2_BIT))
#define TX2 210
#define TX2_GPIO ((TX2_PORT << 8) | TX2_BIT)
#define DIO210 210
#define DIO210_PORT TX2_PORT
#define DIO210_BIT TX2_BIT
#define DIO210_GPIO TX2_GPIO
#endif
#if (defined(RX2_PORT) && defined(RX2_BIT))
#define RX2 211
#define RX2_GPIO ((RX2_PORT << 8) | RX2_BIT)
#define DIO211 211
#define DIO211_PORT RX2_PORT
#define DIO211_BIT RX2_BIT
#define DIO211_GPIO RX2_GPIO
#endif
#if (defined(SPI2_CLK_PORT) && defined(SPI2_CLK_BIT))
#define SPI2_CLK 212
#define SPI2_CLK_GPIO ((SPI2_CLK_PORT << 8) | SPI2_CLK_BIT)
#define DIO212 212
#define DIO212_PORT SPI2_CLK_PORT
#define DIO212_BIT SPI2_CLK_BIT
#define DIO212_GPIO SPI2_CLK_GPIO
#endif
#if (defined(SPI2_SDI_PORT) && defined(SPI2_SDI_BIT))
#define SPI2_SDI 213
#define SPI2_SDI_GPIO ((SPI2_SDI_PORT << 8) | SPI2_SDI_BIT)
#define DIO213 213
#define DIO213_PORT SPI2_SDI_PORT
#define DIO213_BIT SPI2_SDI_BIT
#define DIO213_GPIO SPI2_SDI_GPIO
#endif
#if (defined(SPI2_SDO_PORT) && defined(SPI2_SDO_BIT))
#define SPI2_SDO 214
#define SPI2_SDO_GPIO ((SPI2_SDO_PORT << 8) | SPI2_SDO_BIT)
#define DIO214 214
#define DIO214_PORT SPI2_SDO_PORT
#define DIO214_BIT SPI2_SDO_BIT
#define DIO214_GPIO SPI2_SDO_GPIO
#endif
#if (defined(SPI2_CS_PORT) && defined(SPI2_CS_BIT))
#define SPI2_CS 215
#define SPI2_CS_GPIO ((SPI2_CS_PORT << 8) | SPI2_CS_BIT)
#define DIO215 215
#define DIO215_PORT SPI2_CS_PORT
#define DIO215_BIT SPI2_CS_BIT
#define DIO215_GPIO SPI2_CS_GPIO
#endif

// ISR on change inputs
#if (defined(LIMIT_X_ISR) && defined(LIMIT_X))
#define DIO52_ISR (LIMIT_X_ISR)
#define LIMIT_X_ISRCALLBACK mcu_limit_isr
#define DIO52_ISRCALLBACK mcu_limit_isr
#endif
#if (defined(LIMIT_Y_ISR) && defined(LIMIT_Y))
#define DIO53_ISR (LIMIT_Y_ISR)
#define LIMIT_Y_ISRCALLBACK mcu_limit_isr
#define DIO53_ISRCALLBACK mcu_limit_isr
#endif
#if (defined(LIMIT_Z_ISR) && defined(LIMIT_Z))
#define DIO54_ISR (LIMIT_Z_ISR)
#define LIMIT_Z_ISRCALLBACK mcu_limit_isr
#define DIO54_ISRCALLBACK mcu_limit_isr
#endif
#if (defined(LIMIT_X2_ISR) && defined(LIMIT_X2))
#define DIO55_ISR (LIMIT_X2_ISR)
#define LIMIT_X2_ISRCALLBACK mcu_limit_isr
#define DIO55_ISRCALLBACK mcu_limit_isr
#endif
#if (defined(LIMIT_Y2_ISR) && defined(LIMIT_Y2))
#define DIO56_ISR (LIMIT_Y2_ISR)
#define LIMIT_Y2_ISRCALLBACK mcu_limit_isr
#define DIO56_ISRCALLBACK mcu_limit_isr
#endif
#if (defined(LIMIT_Z2_ISR) && defined(LIMIT_Z2))
#define DIO57_ISR (LIMIT_Z2_ISR)
#define LIMIT_Z2_ISRCALLBACK mcu_limit_isr
#define DIO57_ISRCALLBACK mcu_limit_isr
#endif
#if (defined(LIMIT_A_ISR) && defined(LIMIT_A))
#define DIO58_ISR (LIMIT_A_ISR)
#define LIMIT_A_ISRCALLBACK mcu_limit_isr
#define DIO58_ISRCALLBACK mcu_limit_isr
#endif
#if (defined(LIMIT_B_ISR) && defined(LIMIT_B))
#define DIO59_ISR (LIMIT_B_ISR)
#define LIMIT_B_ISRCALLBACK mcu_limit_isr
#define DIO59_ISRCALLBACK mcu_limit_isr
#endif
#if (defined(LIMIT_C_ISR) && defined(LIMIT_C))
#define DIO60_ISR (LIMIT_C_ISR)
#define LIMIT_C_ISRCALLBACK mcu_limit_isr
#define DIO60_ISRCALLBACK mcu_limit_isr
#endif
#if (defined(PROBE_ISR) && defined(PROBE))
#define DIO61_ISR (PROBE_ISR)
#define PROBE_ISRCALLBACK mcu_probe_isr
#define DIO61_ISRCALLBACK mcu_probe_isr
#endif
#if (defined(ESTOP_ISR) && defined(ESTOP))
#define DIO62_ISR (ESTOP_ISR)
#define ESTOP_ISRCALLBACK mcu_control_isr
#define DIO62_ISRCALLBACK mcu_control_isr
#endif
#if (defined(SAFETY_DOOR_ISR) && defined(SAFETY_DOOR))
#define DIO63_ISR (SAFETY_DOOR_ISR)
#define SAFETY_DOOR_ISRCALLBACK mcu_control_isr
#define DIO63_ISRCALLBACK mcu_control_isr
#endif
#if (defined(FHOLD_ISR) && defined(FHOLD))
#define DIO64_ISR (FHOLD_ISR)
#define FHOLD_ISRCALLBACK mcu_control_isr
#define DIO64_ISRCALLBACK mcu_control_isr
#endif
#if (defined(CS_RES_ISR) && defined(CS_RES))
#define DIO65_ISR (CS_RES_ISR)
#define CS_RES_ISRCALLBACK mcu_control_isr
#define DIO65_ISRCALLBACK mcu_control_isr
#endif
#if (defined(DIN0_ISR) && defined(DIN0))
#define DIO82_ISR (DIN0_ISR)
#define DIN0_ISRCALLBACK mcu_din_isr
#define DIO82_ISRCALLBACK mcu_din_isr
#endif
#if (defined(DIN1_ISR) && defined(DIN1))
#define DIO83_ISR (DIN1_ISR)
#define DIN1_ISRCALLBACK mcu_din_isr
#define DIO83_ISRCALLBACK mcu_din_isr
#endif
#if (defined(DIN2_ISR) && defined(DIN2))
#define DIO84_ISR (DIN2_ISR)
#define DIN2_ISRCALLBACK mcu_din_isr
#define DIO84_ISRCALLBACK mcu_din_isr
#endif
#if (defined(DIN3_ISR) && defined(DIN3))
#define DIO85_ISR (DIN3_ISR)
#define DIN3_ISRCALLBACK mcu_din_isr
#define DIO85_ISRCALLBACK mcu_din_isr
#endif
#if (defined(DIN4_ISR) && defined(DIN4))
#define DIO86_ISR (DIN4_ISR)
#define DIN4_ISRCALLBACK mcu_din_isr
#define DIO86_ISRCALLBACK mcu_din_isr
#endif
#if (defined(DIN5_ISR) && defined(DIN5))
#define DIO87_ISR (DIN5_ISR)
#define DIN5_ISRCALLBACK mcu_din_isr
#define DIO87_ISRCALLBACK mcu_din_isr
#endif
#if (defined(DIN6_ISR) && defined(DIN6))
#define DIO88_ISR (DIN6_ISR)
#define DIN6_ISRCALLBACK mcu_din_isr
#define DIO88_ISRCALLBACK mcu_din_isr
#endif
#if (defined(DIN7_ISR) && defined(DIN7))
#define DIO89_ISR (DIN7_ISR)
#define DIN7_ISRCALLBACK mcu_din_isr
#define DIO89_ISRCALLBACK mcu_din_isr
#endif

// Helper macros
#define __helper_ex__(left, mid, right) (left##mid##right)
#define __helper__(left, mid, right) (__helper_ex__(left, mid, right))
#ifndef __indirect__
#define __indirect__ex__(X, Y) DIO##X##_##Y
#define __indirect__(X, Y) __indirect__ex__(X, Y)
#endif

#if (defined(TX) && defined(RX))
#define MCU_HAS_UART
#endif
#if (defined(TX2) && defined(RX2))
#define MCU_HAS_UART2
#endif
#if (defined(USB_DP) && defined(USB_DM))
#define MCU_HAS_USB
#endif
#ifdef ENABLE_WIFI
#define MCU_HAS_WIFI
#ifndef DISABLE_ENDPOINTS
#define MCU_HAS_ENDPOINTS
#endif
#ifndef DISABLE_WEBSOCKETS
#define MCU_HAS_WEBSOCKETS
#endif
#ifndef BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
#define BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
#endif
#endif
#ifdef ENABLE_BLUETOOTH
#define MCU_HAS_BLUETOOTH
#ifndef BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
#define BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
#endif
#endif

#ifdef MCU_HAS_UART
#ifndef UART_PORT
#define UART_PORT 0
#endif
#if (UART_PORT == 0)
#define COM_UART Serial1
#elif (UART_PORT == 1)
#define COM_UART Serial2
#else
#error "UART COM port number must be 0 or 1"
#endif
#endif

#ifdef MCU_HAS_UART2
#ifndef BAUDRATE2
#define BAUDRATE2 BAUDRATE
#endif
#ifndef UART2_PORT
#define UART2_PORT 0
#endif
#if (UART2_PORT == 0)
#define COM2_UART Serial1
#elif (UART2_PORT == 1)
#define COM2_UART Serial2
#else
#error "UART2 COM port number must be 0 or 1"
#endif
#endif

#define MCU_HAS_ONESHOT_TIMER

// SPI
#if (defined(SPI_CLK) && defined(SPI_SDI) && defined(SPI_SDO))
#define MCU_HAS_SPI
#ifndef SPI_MODE
#define SPI_MODE 0
#endif
#ifndef SPI_FREQ
#define SPI_FREQ 1000000UL
#endif
#ifndef SPI_PORT
#define SPI_PORT 0
#endif
#endif
#if (SPI_PORT == 0)
#define COM_SPI SPI
#elif (SPI_PORT == 1)
#define COM_SPI SPI1
#else
#error "SPI port number must be 0 or 1"
#endif

// SPI2
#if (defined(SPI2_CLK) && defined(SPI2_SDI) && defined(SPI2_SDO))
#define MCU_HAS_SPI2
#ifndef SPI2_MODE
#define SPI2_MODE 0
#endif
#ifndef SPI2_FREQ
#define SPI2_FREQ 1000000UL
#endif
#ifndef SPI2_PORT
#define SPI2_PORT 0
#endif
#endif
#if (SPI2_PORT == 0)
#define COM_SPI2 SPI
#elif (SPI2_PORT == 1)
#define COM_SPI2 SPI1
#else
#error "SPI2 port number must be 0 or 1"
#endif

// for SDK SPI
#define SPI_HW __helper__(spi, SPI_PORT, )

// for SDK SPI
#define SPI2_HW __helper__(spi, SPI2_PORT, )

#if (defined(I2C_CLK) && defined(I2C_DATA))
#define MCU_HAS_I2C
#define MCU_SUPPORTS_I2C_SLAVE
#ifndef I2C_ADDRESS
#define I2C_ADDRESS 0
#endif

#ifndef I2C_PORT
#define I2C_PORT 0
#endif
#ifndef I2C_FREQ
#define I2C_FREQ 400000UL
#endif
#endif

#if (I2C_PORT == 0)
#define I2C_REG Wire
#elif (I2C_PORT == 1)
#define I2C_REG Wire1
#else
#error "I2C port number must be 0 or 1"
#endif

// since Arduino Pico does not allow access to pico-SDK functions low level timer functions are used
#ifndef ITP_TIMER
#define ITP_TIMER 1
#endif

// a single timer slot is used to do RTC, SERVO and ONESHOT
#ifndef RTC_TIMER
#define RTC_TIMER 2
#endif

#ifndef SERVO_TIMER
#undef SERVO_TIMER
#define SERVO_TIMER RTC_TIMER
#endif

#ifndef ONESHOT_TIMER
#undef ONESHOT_TIMER
#define ONESHOT_TIMER RTC_TIMER
#define MCU_HAS_ONESHOT
#endif

#define ALARM_TIMER RTC_TIMER
#define RTC_ALARM 0
#define SERVO_ALARM 1
#define ONESHOT_ALARM 2

#ifdef IC74HC595_CUSTOM_SHIFT_IO
#ifdef IC74HC595_COUNT
#undef IC74HC595_COUNT
#endif
// forces IC74HC595_COUNT to 4 to prevent errors
#define IC74HC595_COUNT 4
#endif

#define __timer_irq__(X) TIMER_IRQ_##X
#define _timer_irq_(X) __timer_irq__(X)

#define ITP_TIMER_IRQ _timer_irq_(ITP_TIMER)
#define RTC_TIMER_IRQ _timer_irq_(RTC_TIMER)
#define ALARM_TIMER_IRQ _timer_irq_(ALARM_TIMER)

#ifndef BYTE_OPS
#define BYTE_OPS
#define SETBIT(x, y) ((x) |= (1UL << (y)))		/* Set bit y in byte x*/
#define CLEARBIT(x, y) ((x) &= ~(1UL << (y))) /* Clear bit y in byte x*/
#define CHECKBIT(x, y) ((x) & (1UL << (y)))		/* Check bit y in byte x*/
#define TOGGLEBIT(x, y) ((x) ^= (1UL << (y))) /* Toggle bit y in byte x*/

#define SETFLAG(x, y) ((x) |= (y))		/* Set byte y in byte x*/
#define CLEARFLAG(x, y) ((x) &= ~(y)) /* Clear byte y in byte x*/
#define CHECKFLAG(x, y) ((x) & (y))		/* Check byte y in byte x*/
#define TOGGLEFLAG(x, y) ((x) ^= (y)) /* Toggle byte y in byte x*/
#endif

// #include <ra/fsp/src/bsp/mcu/all/bsp_io.h>

#define mcu_config_output(X) R_BSP_PinCfg(__indirect__(X, GPIO), BSP_IO_DIRECTION_OUTPUT)
#define mcu_config_pwm(X, freq)            \
	{                                        \
		pinMode(__indirect__(X, BIT), OUTPUT); \
		analogWriteRange(255);                 \
		analogWriteFreq(freq);                 \
		analogWriteResolution(8);              \
	}
#define mcu_config_input(X) R_BSP_PinCfg(__indirect__(X, GPIO), BSP_IO_DIRECTION_INPUT)
#define mcu_config_analog(X) mcu_config_input(X)
#define mcu_config_pullup(X) pinMode(__indirect__(X, BIT), INPUT_PULLUP)
#define mcu_config_input_isr(X) attachInterrupt(digitalPinToInterrupt(__indirect__(X, BIT)), mcu_din_isr, CHANGE)

#define mcu_get_input(X) CHECKBIT(sio_hw->gpio_in, __indirect__(X, BIT))
#define mcu_get_output(X) CHECKBIT(sio_hw->gpio_out, __indirect__(X, BIT))
#define mcu_set_output(X) ({ sio_hw->gpio_set = (1UL << __indirect__(X, BIT)); })
#define mcu_clear_output(X) ({ sio_hw->gpio_clr = (1UL << __indirect__(X, BIT)); })
#define mcu_toggle_output(X) ({ sio_hw->gpio_togl = (1UL << __indirect__(X, BIT)); })

	extern uint8_t rp2040_pwm[16];
#define mcu_set_pwm(X, Y)                 \
	{                                       \
		rp2040_pwm[X - PWM_PINS_OFFSET] = Y;  \
		analogWrite(__indirect__(X, BIT), Y); \
	}
#define mcu_get_pwm(X) (rp2040_pwm[X - PWM_PINS_OFFSET])
#define mcu_get_analog(X) analogRead(__indirect__(X, BIT))

#define mcu_millis() millis()
#define mcu_micros() micros()
#define mcu_free_micros() ({ (1000UL - (SysTick->VAL * 1000UL / SysTick->LOAD)); })

#if (defined(ENABLE_WIFI) || defined(ENABLE_BLUETOOTH))
#ifndef BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
#define BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
#endif
#endif

/**
 * Run code on multicore mode
 * Launches code on core 0
 * Runs communications on core 0
 * Runs CNC loop on core 1
 * **/
#ifdef RP2040_RUN_MULTICORE

#define USE_CUSTOM_BUFFER_IMPLEMENTATION
#include <pico/util/queue.h>
#define DECL_BUFFER(type, name, size) \
	static queue_t name##_bufferdata;   \
	ring_buffer_t name = {0, 0, 0, (uint8_t *)&name##_bufferdata, size, sizeof(type)}
#define BUFFER_INIT(type, name, size) \
	extern ring_buffer_t name;          \
	queue_init((queue_t *)name.data, sizeof(type), size)
#define BUFFER_WRITE_AVAILABLE(buffer) (buffer.size - queue_get_level((queue_t *)buffer.data))
#define BUFFER_READ_AVAILABLE(buffer) (queue_get_level((queue_t *)buffer.data))
#define BUFFER_EMPTY(buffer) queue_is_empty((queue_t *)buffer.data)
#define BUFFER_FULL(buffer) queue_is_full((queue_t *)buffer.data)
#define BUFFER_PEEK(buffer, ptr)                    \
	if (!queue_try_peek((queue_t *)buffer.data, ptr)) \
	{                                                 \
		memset(ptr, 0, buffer.elem_size);               \
	}
#define BUFFER_DEQUEUE(buffer, ptr)                   \
	if (!queue_try_remove((queue_t *)buffer.data, ptr)) \
	{                                                   \
		memset(ptr, 0, buffer.elem_size);                 \
	}
#define BUFFER_ENQUEUE(buffer, ptr) queue_try_add((queue_t *)buffer.data, ptr)
#define BUFFER_WRITE(buffer, ptr, len, written) ({for(uint8_t i = 0; i<len; i++){if(!queue_try_add((queue_t*)buffer.data, &ptr[i])){break;}written++;} })
#define BUFFER_READ(buffer, ptr, len, read) ({for(uint8_t i = 0; i<len; i++){if(!queue_try_remove((queue_t*)buffer.data, &ptr[i])){break;}read++;} })
#define BUFFER_CLEAR(buffer)                        \
	while (!queue_is_empty((queue_t *)buffer.data))   \
	{                                                 \
		queue_try_remove((queue_t *)buffer.data, NULL); \
	}

	/**
	 * Launch multicore
	 * **/
	// 	extern void rp2040_core1_loop();
	// #define ucnc_init() cnc_init();	multicore_launch_core1(rp2040_core1_loop)
	extern void rp2040_core0_loop();
#define ucnc_run() rp2040_core0_loop()

#endif

#ifdef __cplusplus
}
#endif

#endif
