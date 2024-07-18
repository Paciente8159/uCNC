/*
	Name: mcumap_stm32f0x.h
	Description: Contains all MCU and PIN definitions for STM32F0x to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 15-02-2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef MCUMAP_STM32F0x_H
#define MCUMAP_STM32F0x_H

#ifdef __cplusplus
extern "C"
{
#endif

	/*
		Generates all the interface definitions.
		This creates a middle HAL layer between the board IO pins and the AVR funtionalities
	*/
	/*
		MCU specific definitions and replacements
	*/

#include <stm32f0xx.h>
#include <stdbool.h>
#include <stm32f0xx_hal_rcc.h>

// defines the frequency of the mcu
#ifndef F_CPU
#define F_CPU SystemCoreClock
#endif

// defines the maximum and minimum step rates
#ifndef F_STEP_MAX
#define F_STEP_MAX 100000
#endif
#ifndef F_STEP_MIN
#define F_STEP_MIN 4
#endif
// defines special mcu to access flash strings and arrays
#define __rom__
#define __romstr__
#define __romarr__ const uint8_t
#define rom_strptr *
#define rom_strcpy strcpy
#define rom_strncpy strncpy
#define rom_memcpy memcpy
#define rom_read_byte *

	// needed by software delays

#ifndef MCU_CLOCKS_PER_CYCLE
#define MCU_CLOCKS_PER_CYCLE 1
#endif

// Helper macros
#define __helper_ex__(left, mid, right) left##mid##right
#define __helper__(left, mid, right) __helper_ex__(left, mid, right)
#define __iopin_ex__(port, bit) STM32IO_##port##bit
#define __iopin__(port, bit) __iopin_ex__(port, bit)
#define STM32IO_A0 1
#define STM32IO_A1 2
#define STM32IO_A2 3
#define STM32IO_A3 4
#define STM32IO_A4 5
#define STM32IO_A5 6
#define STM32IO_A6 7
#define STM32IO_A7 8
#define STM32IO_A8 9
#define STM32IO_A9 10
#define STM32IO_A10 11
#define STM32IO_A11 12
#define STM32IO_A12 13
#define STM32IO_A13 14
#define STM32IO_A14 15
#define STM32IO_A15 16
#define STM32IO_B0 17
#define STM32IO_B1 18
#define STM32IO_B2 19
#define STM32IO_B3 20
#define STM32IO_B4 21
#define STM32IO_B5 22
#define STM32IO_B6 23
#define STM32IO_B7 24
#define STM32IO_B8 25
#define STM32IO_B9 26
#define STM32IO_B10 27
#define STM32IO_B11 28
#define STM32IO_B12 29
#define STM32IO_B13 30
#define STM32IO_B14 31
#define STM32IO_B15 32
#define STM32IO_C0 33
#define STM32IO_C1 34
#define STM32IO_C2 35
#define STM32IO_C3 36
#define STM32IO_C4 37
#define STM32IO_C5 38
#define STM32IO_C6 39
#define STM32IO_C7 40
#define STM32IO_C8 41
#define STM32IO_C9 42
#define STM32IO_C10 43
#define STM32IO_C11 44
#define STM32IO_C12 45
#define STM32IO_C13 46
#define STM32IO_C14 47
#define STM32IO_C15 48
#define STM32IO_D0 49
#define STM32IO_D1 50
#define STM32IO_D2 51
#define STM32IO_D3 52
#define STM32IO_D4 53
#define STM32IO_D5 54
#define STM32IO_D6 55
#define STM32IO_D7 56
#define STM32IO_D8 57
#define STM32IO_D9 58
#define STM32IO_D10 59
#define STM32IO_D11 60
#define STM32IO_D12 61
#define STM32IO_D13 62
#define STM32IO_D14 63
#define STM32IO_D15 64
#define STM32IO_E0 65
#define STM32IO_E1 66
#define STM32IO_E2 67
#define STM32IO_E3 68
#define STM32IO_E4 69
#define STM32IO_E5 70
#define STM32IO_E6 71
#define STM32IO_E7 72
#define STM32IO_E8 73
#define STM32IO_E9 74
#define STM32IO_E10 75
#define STM32IO_E11 76
#define STM32IO_E12 77
#define STM32IO_E13 78
#define STM32IO_E14 79
#define STM32IO_E15 80
#define STM32IO_F0 81
#define STM32IO_F1 82
#define STM32IO_F2 83
#define STM32IO_F3 84
#define STM32IO_F4 85
#define STM32IO_F5 86
#define STM32IO_F6 87
#define STM32IO_F7 88
#define STM32IO_F8 89
#define STM32IO_F9 90
#define STM32IO_F10 91
#define STM32IO_F11 92
#define STM32IO_F12 93
#define STM32IO_F13 94
#define STM32IO_F14 95
#define STM32IO_F15 96
#define STM32IO_G0 97
#define STM32IO_G1 98
#define STM32IO_G2 99
#define STM32IO_G3 100
#define STM32IO_G4 101
#define STM32IO_G5 102
#define STM32IO_G6 103
#define STM32IO_G7 104
#define STM32IO_G8 105
#define STM32IO_G9 106
#define STM32IO_G10 107
#define STM32IO_G11 108
#define STM32IO_G12 109
#define STM32IO_G13 110
#define STM32IO_G14 111
#define STM32IO_G15 112
#define STM32IO_H0 113
#define STM32IO_H1 114
#define STM32IO_H2 115
#define STM32IO_H3 116
#define STM32IO_H4 117
#define STM32IO_H5 118
#define STM32IO_H6 119
#define STM32IO_H7 120
#define STM32IO_H8 121
#define STM32IO_H9 122
#define STM32IO_H10 123
#define STM32IO_H11 124
#define STM32IO_H12 125
#define STM32IO_H13 126
#define STM32IO_H14 127
#define STM32IO_H15 128

// STM32 internal register names
#define __gpio__(X) (__helper__(GPIO, X, ))
#define __rccgpioen__(X) (__helper__(RCC_AHBENR_GPIO, X, EN))
#define __afioexti__(X) __helper__(EXTI, X, )
#define __extirq__(X) __helper__(EXTI, X, _IRQn)
#define __tim__(X) (__helper__(TIM, X, ))
#define __irq__(X) (__helper__(X, _IRQn, ))
#define __rccapb1timen__(X) (__helper__(RCC_APB1ENR_TIM, X, EN))
#define __rccapb2timen__(X) (__helper__(RCC_APB2ENR_TIM, X, EN))
#define __ccr__(X) __helper__(CCR, X, )
#define __usart__(X) __helper__(USART, X, )
#define __uart__(X) __helper__(UART, X, )

/**********************************************
 *	Autogenerated macros
 **********************************************/

/**********************************************
 *	IO Pins
 **********************************************/
#if (defined(STEP0_PORT) && defined(STEP0_BIT))
#define STEP0 1
#define STEP0_AHBEN (__rccgpioen__(STEP0_PORT))
#define STEP0_GPIO (__gpio__(STEP0_PORT))
#define DIO1 1
#define DIO1_PORT STEP0_PORT
#define DIO1_BIT STEP0_BIT
#define DIO1_AHBEN STEP0_AHBEN
#define DIO1_GPIO STEP0_GPIO
#endif
#if (defined(STEP1_PORT) && defined(STEP1_BIT))
#define STEP1 2
#define STEP1_AHBEN (__rccgpioen__(STEP1_PORT))
#define STEP1_GPIO (__gpio__(STEP1_PORT))
#define DIO2 2
#define DIO2_PORT STEP1_PORT
#define DIO2_BIT STEP1_BIT
#define DIO2_AHBEN STEP1_AHBEN
#define DIO2_GPIO STEP1_GPIO
#endif
#if (defined(STEP2_PORT) && defined(STEP2_BIT))
#define STEP2 3
#define STEP2_AHBEN (__rccgpioen__(STEP2_PORT))
#define STEP2_GPIO (__gpio__(STEP2_PORT))
#define DIO3 3
#define DIO3_PORT STEP2_PORT
#define DIO3_BIT STEP2_BIT
#define DIO3_AHBEN STEP2_AHBEN
#define DIO3_GPIO STEP2_GPIO
#endif
#if (defined(STEP3_PORT) && defined(STEP3_BIT))
#define STEP3 4
#define STEP3_AHBEN (__rccgpioen__(STEP3_PORT))
#define STEP3_GPIO (__gpio__(STEP3_PORT))
#define DIO4 4
#define DIO4_PORT STEP3_PORT
#define DIO4_BIT STEP3_BIT
#define DIO4_AHBEN STEP3_AHBEN
#define DIO4_GPIO STEP3_GPIO
#endif
#if (defined(STEP4_PORT) && defined(STEP4_BIT))
#define STEP4 5
#define STEP4_AHBEN (__rccgpioen__(STEP4_PORT))
#define STEP4_GPIO (__gpio__(STEP4_PORT))
#define DIO5 5
#define DIO5_PORT STEP4_PORT
#define DIO5_BIT STEP4_BIT
#define DIO5_AHBEN STEP4_AHBEN
#define DIO5_GPIO STEP4_GPIO
#endif
#if (defined(STEP5_PORT) && defined(STEP5_BIT))
#define STEP5 6
#define STEP5_AHBEN (__rccgpioen__(STEP5_PORT))
#define STEP5_GPIO (__gpio__(STEP5_PORT))
#define DIO6 6
#define DIO6_PORT STEP5_PORT
#define DIO6_BIT STEP5_BIT
#define DIO6_AHBEN STEP5_AHBEN
#define DIO6_GPIO STEP5_GPIO
#endif
#if (defined(STEP6_PORT) && defined(STEP6_BIT))
#define STEP6 7
#define STEP6_AHBEN (__rccgpioen__(STEP6_PORT))
#define STEP6_GPIO (__gpio__(STEP6_PORT))
#define DIO7 7
#define DIO7_PORT STEP6_PORT
#define DIO7_BIT STEP6_BIT
#define DIO7_AHBEN STEP6_AHBEN
#define DIO7_GPIO STEP6_GPIO
#endif
#if (defined(STEP7_PORT) && defined(STEP7_BIT))
#define STEP7 8
#define STEP7_AHBEN (__rccgpioen__(STEP7_PORT))
#define STEP7_GPIO (__gpio__(STEP7_PORT))
#define DIO8 8
#define DIO8_PORT STEP7_PORT
#define DIO8_BIT STEP7_BIT
#define DIO8_AHBEN STEP7_AHBEN
#define DIO8_GPIO STEP7_GPIO
#endif
#if (defined(DIR0_PORT) && defined(DIR0_BIT))
#define DIR0 9
#define DIR0_AHBEN (__rccgpioen__(DIR0_PORT))
#define DIR0_GPIO (__gpio__(DIR0_PORT))
#define DIO9 9
#define DIO9_PORT DIR0_PORT
#define DIO9_BIT DIR0_BIT
#define DIO9_AHBEN DIR0_AHBEN
#define DIO9_GPIO DIR0_GPIO
#endif
#if (defined(DIR1_PORT) && defined(DIR1_BIT))
#define DIR1 10
#define DIR1_AHBEN (__rccgpioen__(DIR1_PORT))
#define DIR1_GPIO (__gpio__(DIR1_PORT))
#define DIO10 10
#define DIO10_PORT DIR1_PORT
#define DIO10_BIT DIR1_BIT
#define DIO10_AHBEN DIR1_AHBEN
#define DIO10_GPIO DIR1_GPIO
#endif
#if (defined(DIR2_PORT) && defined(DIR2_BIT))
#define DIR2 11
#define DIR2_AHBEN (__rccgpioen__(DIR2_PORT))
#define DIR2_GPIO (__gpio__(DIR2_PORT))
#define DIO11 11
#define DIO11_PORT DIR2_PORT
#define DIO11_BIT DIR2_BIT
#define DIO11_AHBEN DIR2_AHBEN
#define DIO11_GPIO DIR2_GPIO
#endif
#if (defined(DIR3_PORT) && defined(DIR3_BIT))
#define DIR3 12
#define DIR3_AHBEN (__rccgpioen__(DIR3_PORT))
#define DIR3_GPIO (__gpio__(DIR3_PORT))
#define DIO12 12
#define DIO12_PORT DIR3_PORT
#define DIO12_BIT DIR3_BIT
#define DIO12_AHBEN DIR3_AHBEN
#define DIO12_GPIO DIR3_GPIO
#endif
#if (defined(DIR4_PORT) && defined(DIR4_BIT))
#define DIR4 13
#define DIR4_AHBEN (__rccgpioen__(DIR4_PORT))
#define DIR4_GPIO (__gpio__(DIR4_PORT))
#define DIO13 13
#define DIO13_PORT DIR4_PORT
#define DIO13_BIT DIR4_BIT
#define DIO13_AHBEN DIR4_AHBEN
#define DIO13_GPIO DIR4_GPIO
#endif
#if (defined(DIR5_PORT) && defined(DIR5_BIT))
#define DIR5 14
#define DIR5_AHBEN (__rccgpioen__(DIR5_PORT))
#define DIR5_GPIO (__gpio__(DIR5_PORT))
#define DIO14 14
#define DIO14_PORT DIR5_PORT
#define DIO14_BIT DIR5_BIT
#define DIO14_AHBEN DIR5_AHBEN
#define DIO14_GPIO DIR5_GPIO
#endif
#if (defined(DIR6_PORT) && defined(DIR6_BIT))
#define DIR6 15
#define DIR6_AHBEN (__rccgpioen__(DIR6_PORT))
#define DIR6_GPIO (__gpio__(DIR6_PORT))
#define DIO15 15
#define DIO15_PORT DIR6_PORT
#define DIO15_BIT DIR6_BIT
#define DIO15_AHBEN DIR6_AHBEN
#define DIO15_GPIO DIR6_GPIO
#endif
#if (defined(DIR7_PORT) && defined(DIR7_BIT))
#define DIR7 16
#define DIR7_AHBEN (__rccgpioen__(DIR7_PORT))
#define DIR7_GPIO (__gpio__(DIR7_PORT))
#define DIO16 16
#define DIO16_PORT DIR7_PORT
#define DIO16_BIT DIR7_BIT
#define DIO16_AHBEN DIR7_AHBEN
#define DIO16_GPIO DIR7_GPIO
#endif
#if (defined(STEP0_EN_PORT) && defined(STEP0_EN_BIT))
#define STEP0_EN 17
#define STEP0_EN_AHBEN (__rccgpioen__(STEP0_EN_PORT))
#define STEP0_EN_GPIO (__gpio__(STEP0_EN_PORT))
#define DIO17 17
#define DIO17_PORT STEP0_EN_PORT
#define DIO17_BIT STEP0_EN_BIT
#define DIO17_AHBEN STEP0_EN_AHBEN
#define DIO17_GPIO STEP0_EN_GPIO
#endif
#if (defined(STEP1_EN_PORT) && defined(STEP1_EN_BIT))
#define STEP1_EN 18
#define STEP1_EN_AHBEN (__rccgpioen__(STEP1_EN_PORT))
#define STEP1_EN_GPIO (__gpio__(STEP1_EN_PORT))
#define DIO18 18
#define DIO18_PORT STEP1_EN_PORT
#define DIO18_BIT STEP1_EN_BIT
#define DIO18_AHBEN STEP1_EN_AHBEN
#define DIO18_GPIO STEP1_EN_GPIO
#endif
#if (defined(STEP2_EN_PORT) && defined(STEP2_EN_BIT))
#define STEP2_EN 19
#define STEP2_EN_AHBEN (__rccgpioen__(STEP2_EN_PORT))
#define STEP2_EN_GPIO (__gpio__(STEP2_EN_PORT))
#define DIO19 19
#define DIO19_PORT STEP2_EN_PORT
#define DIO19_BIT STEP2_EN_BIT
#define DIO19_AHBEN STEP2_EN_AHBEN
#define DIO19_GPIO STEP2_EN_GPIO
#endif
#if (defined(STEP3_EN_PORT) && defined(STEP3_EN_BIT))
#define STEP3_EN 20
#define STEP3_EN_AHBEN (__rccgpioen__(STEP3_EN_PORT))
#define STEP3_EN_GPIO (__gpio__(STEP3_EN_PORT))
#define DIO20 20
#define DIO20_PORT STEP3_EN_PORT
#define DIO20_BIT STEP3_EN_BIT
#define DIO20_AHBEN STEP3_EN_AHBEN
#define DIO20_GPIO STEP3_EN_GPIO
#endif
#if (defined(STEP4_EN_PORT) && defined(STEP4_EN_BIT))
#define STEP4_EN 21
#define STEP4_EN_AHBEN (__rccgpioen__(STEP4_EN_PORT))
#define STEP4_EN_GPIO (__gpio__(STEP4_EN_PORT))
#define DIO21 21
#define DIO21_PORT STEP4_EN_PORT
#define DIO21_BIT STEP4_EN_BIT
#define DIO21_AHBEN STEP4_EN_AHBEN
#define DIO21_GPIO STEP4_EN_GPIO
#endif
#if (defined(STEP5_EN_PORT) && defined(STEP5_EN_BIT))
#define STEP5_EN 22
#define STEP5_EN_AHBEN (__rccgpioen__(STEP5_EN_PORT))
#define STEP5_EN_GPIO (__gpio__(STEP5_EN_PORT))
#define DIO22 22
#define DIO22_PORT STEP5_EN_PORT
#define DIO22_BIT STEP5_EN_BIT
#define DIO22_AHBEN STEP5_EN_AHBEN
#define DIO22_GPIO STEP5_EN_GPIO
#endif
#if (defined(STEP6_EN_PORT) && defined(STEP6_EN_BIT))
#define STEP6_EN 23
#define STEP6_EN_AHBEN (__rccgpioen__(STEP6_EN_PORT))
#define STEP6_EN_GPIO (__gpio__(STEP6_EN_PORT))
#define DIO23 23
#define DIO23_PORT STEP6_EN_PORT
#define DIO23_BIT STEP6_EN_BIT
#define DIO23_AHBEN STEP6_EN_AHBEN
#define DIO23_GPIO STEP6_EN_GPIO
#endif
#if (defined(STEP7_EN_PORT) && defined(STEP7_EN_BIT))
#define STEP7_EN 24
#define STEP7_EN_AHBEN (__rccgpioen__(STEP7_EN_PORT))
#define STEP7_EN_GPIO (__gpio__(STEP7_EN_PORT))
#define DIO24 24
#define DIO24_PORT STEP7_EN_PORT
#define DIO24_BIT STEP7_EN_BIT
#define DIO24_AHBEN STEP7_EN_AHBEN
#define DIO24_GPIO STEP7_EN_GPIO
#endif
#if (defined(PWM0_PORT) && defined(PWM0_BIT))
#define PWM0 25
#define PWM0_AHBEN (__rccgpioen__(PWM0_PORT))
#define PWM0_GPIO (__gpio__(PWM0_PORT))
#define DIO25 25
#define DIO25_PORT PWM0_PORT
#define DIO25_BIT PWM0_BIT
#define DIO25_AHBEN PWM0_AHBEN
#define DIO25_GPIO PWM0_GPIO
#endif
#if (defined(PWM1_PORT) && defined(PWM1_BIT))
#define PWM1 26
#define PWM1_AHBEN (__rccgpioen__(PWM1_PORT))
#define PWM1_GPIO (__gpio__(PWM1_PORT))
#define DIO26 26
#define DIO26_PORT PWM1_PORT
#define DIO26_BIT PWM1_BIT
#define DIO26_AHBEN PWM1_AHBEN
#define DIO26_GPIO PWM1_GPIO
#endif
#if (defined(PWM2_PORT) && defined(PWM2_BIT))
#define PWM2 27
#define PWM2_AHBEN (__rccgpioen__(PWM2_PORT))
#define PWM2_GPIO (__gpio__(PWM2_PORT))
#define DIO27 27
#define DIO27_PORT PWM2_PORT
#define DIO27_BIT PWM2_BIT
#define DIO27_AHBEN PWM2_AHBEN
#define DIO27_GPIO PWM2_GPIO
#endif
#if (defined(PWM3_PORT) && defined(PWM3_BIT))
#define PWM3 28
#define PWM3_AHBEN (__rccgpioen__(PWM3_PORT))
#define PWM3_GPIO (__gpio__(PWM3_PORT))
#define DIO28 28
#define DIO28_PORT PWM3_PORT
#define DIO28_BIT PWM3_BIT
#define DIO28_AHBEN PWM3_AHBEN
#define DIO28_GPIO PWM3_GPIO
#endif
#if (defined(PWM4_PORT) && defined(PWM4_BIT))
#define PWM4 29
#define PWM4_AHBEN (__rccgpioen__(PWM4_PORT))
#define PWM4_GPIO (__gpio__(PWM4_PORT))
#define DIO29 29
#define DIO29_PORT PWM4_PORT
#define DIO29_BIT PWM4_BIT
#define DIO29_AHBEN PWM4_AHBEN
#define DIO29_GPIO PWM4_GPIO
#endif
#if (defined(PWM5_PORT) && defined(PWM5_BIT))
#define PWM5 30
#define PWM5_AHBEN (__rccgpioen__(PWM5_PORT))
#define PWM5_GPIO (__gpio__(PWM5_PORT))
#define DIO30 30
#define DIO30_PORT PWM5_PORT
#define DIO30_BIT PWM5_BIT
#define DIO30_AHBEN PWM5_AHBEN
#define DIO30_GPIO PWM5_GPIO
#endif
#if (defined(PWM6_PORT) && defined(PWM6_BIT))
#define PWM6 31
#define PWM6_AHBEN (__rccgpioen__(PWM6_PORT))
#define PWM6_GPIO (__gpio__(PWM6_PORT))
#define DIO31 31
#define DIO31_PORT PWM6_PORT
#define DIO31_BIT PWM6_BIT
#define DIO31_AHBEN PWM6_AHBEN
#define DIO31_GPIO PWM6_GPIO
#endif
#if (defined(PWM7_PORT) && defined(PWM7_BIT))
#define PWM7 32
#define PWM7_AHBEN (__rccgpioen__(PWM7_PORT))
#define PWM7_GPIO (__gpio__(PWM7_PORT))
#define DIO32 32
#define DIO32_PORT PWM7_PORT
#define DIO32_BIT PWM7_BIT
#define DIO32_AHBEN PWM7_AHBEN
#define DIO32_GPIO PWM7_GPIO
#endif
#if (defined(PWM8_PORT) && defined(PWM8_BIT))
#define PWM8 33
#define PWM8_AHBEN (__rccgpioen__(PWM8_PORT))
#define PWM8_GPIO (__gpio__(PWM8_PORT))
#define DIO33 33
#define DIO33_PORT PWM8_PORT
#define DIO33_BIT PWM8_BIT
#define DIO33_AHBEN PWM8_AHBEN
#define DIO33_GPIO PWM8_GPIO
#endif
#if (defined(PWM9_PORT) && defined(PWM9_BIT))
#define PWM9 34
#define PWM9_AHBEN (__rccgpioen__(PWM9_PORT))
#define PWM9_GPIO (__gpio__(PWM9_PORT))
#define DIO34 34
#define DIO34_PORT PWM9_PORT
#define DIO34_BIT PWM9_BIT
#define DIO34_AHBEN PWM9_AHBEN
#define DIO34_GPIO PWM9_GPIO
#endif
#if (defined(PWM10_PORT) && defined(PWM10_BIT))
#define PWM10 35
#define PWM10_AHBEN (__rccgpioen__(PWM10_PORT))
#define PWM10_GPIO (__gpio__(PWM10_PORT))
#define DIO35 35
#define DIO35_PORT PWM10_PORT
#define DIO35_BIT PWM10_BIT
#define DIO35_AHBEN PWM10_AHBEN
#define DIO35_GPIO PWM10_GPIO
#endif
#if (defined(PWM11_PORT) && defined(PWM11_BIT))
#define PWM11 36
#define PWM11_AHBEN (__rccgpioen__(PWM11_PORT))
#define PWM11_GPIO (__gpio__(PWM11_PORT))
#define DIO36 36
#define DIO36_PORT PWM11_PORT
#define DIO36_BIT PWM11_BIT
#define DIO36_AHBEN PWM11_AHBEN
#define DIO36_GPIO PWM11_GPIO
#endif
#if (defined(PWM12_PORT) && defined(PWM12_BIT))
#define PWM12 37
#define PWM12_AHBEN (__rccgpioen__(PWM12_PORT))
#define PWM12_GPIO (__gpio__(PWM12_PORT))
#define DIO37 37
#define DIO37_PORT PWM12_PORT
#define DIO37_BIT PWM12_BIT
#define DIO37_AHBEN PWM12_AHBEN
#define DIO37_GPIO PWM12_GPIO
#endif
#if (defined(PWM13_PORT) && defined(PWM13_BIT))
#define PWM13 38
#define PWM13_AHBEN (__rccgpioen__(PWM13_PORT))
#define PWM13_GPIO (__gpio__(PWM13_PORT))
#define DIO38 38
#define DIO38_PORT PWM13_PORT
#define DIO38_BIT PWM13_BIT
#define DIO38_AHBEN PWM13_AHBEN
#define DIO38_GPIO PWM13_GPIO
#endif
#if (defined(PWM14_PORT) && defined(PWM14_BIT))
#define PWM14 39
#define PWM14_AHBEN (__rccgpioen__(PWM14_PORT))
#define PWM14_GPIO (__gpio__(PWM14_PORT))
#define DIO39 39
#define DIO39_PORT PWM14_PORT
#define DIO39_BIT PWM14_BIT
#define DIO39_AHBEN PWM14_AHBEN
#define DIO39_GPIO PWM14_GPIO
#endif
#if (defined(PWM15_PORT) && defined(PWM15_BIT))
#define PWM15 40
#define PWM15_AHBEN (__rccgpioen__(PWM15_PORT))
#define PWM15_GPIO (__gpio__(PWM15_PORT))
#define DIO40 40
#define DIO40_PORT PWM15_PORT
#define DIO40_BIT PWM15_BIT
#define DIO40_AHBEN PWM15_AHBEN
#define DIO40_GPIO PWM15_GPIO
#endif
#if (defined(SERVO0_PORT) && defined(SERVO0_BIT))
#define SERVO0 41
#define SERVO0_AHBEN (__rccgpioen__(SERVO0_PORT))
#define SERVO0_GPIO (__gpio__(SERVO0_PORT))
#define DIO41 41
#define DIO41_PORT SERVO0_PORT
#define DIO41_BIT SERVO0_BIT
#define DIO41_AHBEN SERVO0_AHBEN
#define DIO41_GPIO SERVO0_GPIO
#endif
#if (defined(SERVO1_PORT) && defined(SERVO1_BIT))
#define SERVO1 42
#define SERVO1_AHBEN (__rccgpioen__(SERVO1_PORT))
#define SERVO1_GPIO (__gpio__(SERVO1_PORT))
#define DIO42 42
#define DIO42_PORT SERVO1_PORT
#define DIO42_BIT SERVO1_BIT
#define DIO42_AHBEN SERVO1_AHBEN
#define DIO42_GPIO SERVO1_GPIO
#endif
#if (defined(SERVO2_PORT) && defined(SERVO2_BIT))
#define SERVO2 43
#define SERVO2_AHBEN (__rccgpioen__(SERVO2_PORT))
#define SERVO2_GPIO (__gpio__(SERVO2_PORT))
#define DIO43 43
#define DIO43_PORT SERVO2_PORT
#define DIO43_BIT SERVO2_BIT
#define DIO43_AHBEN SERVO2_AHBEN
#define DIO43_GPIO SERVO2_GPIO
#endif
#if (defined(SERVO3_PORT) && defined(SERVO3_BIT))
#define SERVO3 44
#define SERVO3_AHBEN (__rccgpioen__(SERVO3_PORT))
#define SERVO3_GPIO (__gpio__(SERVO3_PORT))
#define DIO44 44
#define DIO44_PORT SERVO3_PORT
#define DIO44_BIT SERVO3_BIT
#define DIO44_AHBEN SERVO3_AHBEN
#define DIO44_GPIO SERVO3_GPIO
#endif
#if (defined(SERVO4_PORT) && defined(SERVO4_BIT))
#define SERVO4 45
#define SERVO4_AHBEN (__rccgpioen__(SERVO4_PORT))
#define SERVO4_GPIO (__gpio__(SERVO4_PORT))
#define DIO45 45
#define DIO45_PORT SERVO4_PORT
#define DIO45_BIT SERVO4_BIT
#define DIO45_AHBEN SERVO4_AHBEN
#define DIO45_GPIO SERVO4_GPIO
#endif
#if (defined(SERVO5_PORT) && defined(SERVO5_BIT))
#define SERVO5 46
#define SERVO5_AHBEN (__rccgpioen__(SERVO5_PORT))
#define SERVO5_GPIO (__gpio__(SERVO5_PORT))
#define DIO46 46
#define DIO46_PORT SERVO5_PORT
#define DIO46_BIT SERVO5_BIT
#define DIO46_AHBEN SERVO5_AHBEN
#define DIO46_GPIO SERVO5_GPIO
#endif
#if (defined(DOUT0_PORT) && defined(DOUT0_BIT))
#define DOUT0 47
#define DOUT0_AHBEN (__rccgpioen__(DOUT0_PORT))
#define DOUT0_GPIO (__gpio__(DOUT0_PORT))
#define DIO47 47
#define DIO47_PORT DOUT0_PORT
#define DIO47_BIT DOUT0_BIT
#define DIO47_AHBEN DOUT0_AHBEN
#define DIO47_GPIO DOUT0_GPIO
#endif
#if (defined(DOUT1_PORT) && defined(DOUT1_BIT))
#define DOUT1 48
#define DOUT1_AHBEN (__rccgpioen__(DOUT1_PORT))
#define DOUT1_GPIO (__gpio__(DOUT1_PORT))
#define DIO48 48
#define DIO48_PORT DOUT1_PORT
#define DIO48_BIT DOUT1_BIT
#define DIO48_AHBEN DOUT1_AHBEN
#define DIO48_GPIO DOUT1_GPIO
#endif
#if (defined(DOUT2_PORT) && defined(DOUT2_BIT))
#define DOUT2 49
#define DOUT2_AHBEN (__rccgpioen__(DOUT2_PORT))
#define DOUT2_GPIO (__gpio__(DOUT2_PORT))
#define DIO49 49
#define DIO49_PORT DOUT2_PORT
#define DIO49_BIT DOUT2_BIT
#define DIO49_AHBEN DOUT2_AHBEN
#define DIO49_GPIO DOUT2_GPIO
#endif
#if (defined(DOUT3_PORT) && defined(DOUT3_BIT))
#define DOUT3 50
#define DOUT3_AHBEN (__rccgpioen__(DOUT3_PORT))
#define DOUT3_GPIO (__gpio__(DOUT3_PORT))
#define DIO50 50
#define DIO50_PORT DOUT3_PORT
#define DIO50_BIT DOUT3_BIT
#define DIO50_AHBEN DOUT3_AHBEN
#define DIO50_GPIO DOUT3_GPIO
#endif
#if (defined(DOUT4_PORT) && defined(DOUT4_BIT))
#define DOUT4 51
#define DOUT4_AHBEN (__rccgpioen__(DOUT4_PORT))
#define DOUT4_GPIO (__gpio__(DOUT4_PORT))
#define DIO51 51
#define DIO51_PORT DOUT4_PORT
#define DIO51_BIT DOUT4_BIT
#define DIO51_AHBEN DOUT4_AHBEN
#define DIO51_GPIO DOUT4_GPIO
#endif
#if (defined(DOUT5_PORT) && defined(DOUT5_BIT))
#define DOUT5 52
#define DOUT5_AHBEN (__rccgpioen__(DOUT5_PORT))
#define DOUT5_GPIO (__gpio__(DOUT5_PORT))
#define DIO52 52
#define DIO52_PORT DOUT5_PORT
#define DIO52_BIT DOUT5_BIT
#define DIO52_AHBEN DOUT5_AHBEN
#define DIO52_GPIO DOUT5_GPIO
#endif
#if (defined(DOUT6_PORT) && defined(DOUT6_BIT))
#define DOUT6 53
#define DOUT6_AHBEN (__rccgpioen__(DOUT6_PORT))
#define DOUT6_GPIO (__gpio__(DOUT6_PORT))
#define DIO53 53
#define DIO53_PORT DOUT6_PORT
#define DIO53_BIT DOUT6_BIT
#define DIO53_AHBEN DOUT6_AHBEN
#define DIO53_GPIO DOUT6_GPIO
#endif
#if (defined(DOUT7_PORT) && defined(DOUT7_BIT))
#define DOUT7 54
#define DOUT7_AHBEN (__rccgpioen__(DOUT7_PORT))
#define DOUT7_GPIO (__gpio__(DOUT7_PORT))
#define DIO54 54
#define DIO54_PORT DOUT7_PORT
#define DIO54_BIT DOUT7_BIT
#define DIO54_AHBEN DOUT7_AHBEN
#define DIO54_GPIO DOUT7_GPIO
#endif
#if (defined(DOUT8_PORT) && defined(DOUT8_BIT))
#define DOUT8 55
#define DOUT8_AHBEN (__rccgpioen__(DOUT8_PORT))
#define DOUT8_GPIO (__gpio__(DOUT8_PORT))
#define DIO55 55
#define DIO55_PORT DOUT8_PORT
#define DIO55_BIT DOUT8_BIT
#define DIO55_AHBEN DOUT8_AHBEN
#define DIO55_GPIO DOUT8_GPIO
#endif
#if (defined(DOUT9_PORT) && defined(DOUT9_BIT))
#define DOUT9 56
#define DOUT9_AHBEN (__rccgpioen__(DOUT9_PORT))
#define DOUT9_GPIO (__gpio__(DOUT9_PORT))
#define DIO56 56
#define DIO56_PORT DOUT9_PORT
#define DIO56_BIT DOUT9_BIT
#define DIO56_AHBEN DOUT9_AHBEN
#define DIO56_GPIO DOUT9_GPIO
#endif
#if (defined(DOUT10_PORT) && defined(DOUT10_BIT))
#define DOUT10 57
#define DOUT10_AHBEN (__rccgpioen__(DOUT10_PORT))
#define DOUT10_GPIO (__gpio__(DOUT10_PORT))
#define DIO57 57
#define DIO57_PORT DOUT10_PORT
#define DIO57_BIT DOUT10_BIT
#define DIO57_AHBEN DOUT10_AHBEN
#define DIO57_GPIO DOUT10_GPIO
#endif
#if (defined(DOUT11_PORT) && defined(DOUT11_BIT))
#define DOUT11 58
#define DOUT11_AHBEN (__rccgpioen__(DOUT11_PORT))
#define DOUT11_GPIO (__gpio__(DOUT11_PORT))
#define DIO58 58
#define DIO58_PORT DOUT11_PORT
#define DIO58_BIT DOUT11_BIT
#define DIO58_AHBEN DOUT11_AHBEN
#define DIO58_GPIO DOUT11_GPIO
#endif
#if (defined(DOUT12_PORT) && defined(DOUT12_BIT))
#define DOUT12 59
#define DOUT12_AHBEN (__rccgpioen__(DOUT12_PORT))
#define DOUT12_GPIO (__gpio__(DOUT12_PORT))
#define DIO59 59
#define DIO59_PORT DOUT12_PORT
#define DIO59_BIT DOUT12_BIT
#define DIO59_AHBEN DOUT12_AHBEN
#define DIO59_GPIO DOUT12_GPIO
#endif
#if (defined(DOUT13_PORT) && defined(DOUT13_BIT))
#define DOUT13 60
#define DOUT13_AHBEN (__rccgpioen__(DOUT13_PORT))
#define DOUT13_GPIO (__gpio__(DOUT13_PORT))
#define DIO60 60
#define DIO60_PORT DOUT13_PORT
#define DIO60_BIT DOUT13_BIT
#define DIO60_AHBEN DOUT13_AHBEN
#define DIO60_GPIO DOUT13_GPIO
#endif
#if (defined(DOUT14_PORT) && defined(DOUT14_BIT))
#define DOUT14 61
#define DOUT14_AHBEN (__rccgpioen__(DOUT14_PORT))
#define DOUT14_GPIO (__gpio__(DOUT14_PORT))
#define DIO61 61
#define DIO61_PORT DOUT14_PORT
#define DIO61_BIT DOUT14_BIT
#define DIO61_AHBEN DOUT14_AHBEN
#define DIO61_GPIO DOUT14_GPIO
#endif
#if (defined(DOUT15_PORT) && defined(DOUT15_BIT))
#define DOUT15 62
#define DOUT15_AHBEN (__rccgpioen__(DOUT15_PORT))
#define DOUT15_GPIO (__gpio__(DOUT15_PORT))
#define DIO62 62
#define DIO62_PORT DOUT15_PORT
#define DIO62_BIT DOUT15_BIT
#define DIO62_AHBEN DOUT15_AHBEN
#define DIO62_GPIO DOUT15_GPIO
#endif
#if (defined(DOUT16_PORT) && defined(DOUT16_BIT))
#define DOUT16 63
#define DOUT16_AHBEN (__rccgpioen__(DOUT16_PORT))
#define DOUT16_GPIO (__gpio__(DOUT16_PORT))
#define DIO63 63
#define DIO63_PORT DOUT16_PORT
#define DIO63_BIT DOUT16_BIT
#define DIO63_AHBEN DOUT16_AHBEN
#define DIO63_GPIO DOUT16_GPIO
#endif
#if (defined(DOUT17_PORT) && defined(DOUT17_BIT))
#define DOUT17 64
#define DOUT17_AHBEN (__rccgpioen__(DOUT17_PORT))
#define DOUT17_GPIO (__gpio__(DOUT17_PORT))
#define DIO64 64
#define DIO64_PORT DOUT17_PORT
#define DIO64_BIT DOUT17_BIT
#define DIO64_AHBEN DOUT17_AHBEN
#define DIO64_GPIO DOUT17_GPIO
#endif
#if (defined(DOUT18_PORT) && defined(DOUT18_BIT))
#define DOUT18 65
#define DOUT18_AHBEN (__rccgpioen__(DOUT18_PORT))
#define DOUT18_GPIO (__gpio__(DOUT18_PORT))
#define DIO65 65
#define DIO65_PORT DOUT18_PORT
#define DIO65_BIT DOUT18_BIT
#define DIO65_AHBEN DOUT18_AHBEN
#define DIO65_GPIO DOUT18_GPIO
#endif
#if (defined(DOUT19_PORT) && defined(DOUT19_BIT))
#define DOUT19 66
#define DOUT19_AHBEN (__rccgpioen__(DOUT19_PORT))
#define DOUT19_GPIO (__gpio__(DOUT19_PORT))
#define DIO66 66
#define DIO66_PORT DOUT19_PORT
#define DIO66_BIT DOUT19_BIT
#define DIO66_AHBEN DOUT19_AHBEN
#define DIO66_GPIO DOUT19_GPIO
#endif
#if (defined(DOUT20_PORT) && defined(DOUT20_BIT))
#define DOUT20 67
#define DOUT20_AHBEN (__rccgpioen__(DOUT20_PORT))
#define DOUT20_GPIO (__gpio__(DOUT20_PORT))
#define DIO67 67
#define DIO67_PORT DOUT20_PORT
#define DIO67_BIT DOUT20_BIT
#define DIO67_AHBEN DOUT20_AHBEN
#define DIO67_GPIO DOUT20_GPIO
#endif
#if (defined(DOUT21_PORT) && defined(DOUT21_BIT))
#define DOUT21 68
#define DOUT21_AHBEN (__rccgpioen__(DOUT21_PORT))
#define DOUT21_GPIO (__gpio__(DOUT21_PORT))
#define DIO68 68
#define DIO68_PORT DOUT21_PORT
#define DIO68_BIT DOUT21_BIT
#define DIO68_AHBEN DOUT21_AHBEN
#define DIO68_GPIO DOUT21_GPIO
#endif
#if (defined(DOUT22_PORT) && defined(DOUT22_BIT))
#define DOUT22 69
#define DOUT22_AHBEN (__rccgpioen__(DOUT22_PORT))
#define DOUT22_GPIO (__gpio__(DOUT22_PORT))
#define DIO69 69
#define DIO69_PORT DOUT22_PORT
#define DIO69_BIT DOUT22_BIT
#define DIO69_AHBEN DOUT22_AHBEN
#define DIO69_GPIO DOUT22_GPIO
#endif
#if (defined(DOUT23_PORT) && defined(DOUT23_BIT))
#define DOUT23 70
#define DOUT23_AHBEN (__rccgpioen__(DOUT23_PORT))
#define DOUT23_GPIO (__gpio__(DOUT23_PORT))
#define DIO70 70
#define DIO70_PORT DOUT23_PORT
#define DIO70_BIT DOUT23_BIT
#define DIO70_AHBEN DOUT23_AHBEN
#define DIO70_GPIO DOUT23_GPIO
#endif
#if (defined(DOUT24_PORT) && defined(DOUT24_BIT))
#define DOUT24 71
#define DOUT24_AHBEN (__rccgpioen__(DOUT24_PORT))
#define DOUT24_GPIO (__gpio__(DOUT24_PORT))
#define DIO71 71
#define DIO71_PORT DOUT24_PORT
#define DIO71_BIT DOUT24_BIT
#define DIO71_AHBEN DOUT24_AHBEN
#define DIO71_GPIO DOUT24_GPIO
#endif
#if (defined(DOUT25_PORT) && defined(DOUT25_BIT))
#define DOUT25 72
#define DOUT25_AHBEN (__rccgpioen__(DOUT25_PORT))
#define DOUT25_GPIO (__gpio__(DOUT25_PORT))
#define DIO72 72
#define DIO72_PORT DOUT25_PORT
#define DIO72_BIT DOUT25_BIT
#define DIO72_AHBEN DOUT25_AHBEN
#define DIO72_GPIO DOUT25_GPIO
#endif
#if (defined(DOUT26_PORT) && defined(DOUT26_BIT))
#define DOUT26 73
#define DOUT26_AHBEN (__rccgpioen__(DOUT26_PORT))
#define DOUT26_GPIO (__gpio__(DOUT26_PORT))
#define DIO73 73
#define DIO73_PORT DOUT26_PORT
#define DIO73_BIT DOUT26_BIT
#define DIO73_AHBEN DOUT26_AHBEN
#define DIO73_GPIO DOUT26_GPIO
#endif
#if (defined(DOUT27_PORT) && defined(DOUT27_BIT))
#define DOUT27 74
#define DOUT27_AHBEN (__rccgpioen__(DOUT27_PORT))
#define DOUT27_GPIO (__gpio__(DOUT27_PORT))
#define DIO74 74
#define DIO74_PORT DOUT27_PORT
#define DIO74_BIT DOUT27_BIT
#define DIO74_AHBEN DOUT27_AHBEN
#define DIO74_GPIO DOUT27_GPIO
#endif
#if (defined(DOUT28_PORT) && defined(DOUT28_BIT))
#define DOUT28 75
#define DOUT28_AHBEN (__rccgpioen__(DOUT28_PORT))
#define DOUT28_GPIO (__gpio__(DOUT28_PORT))
#define DIO75 75
#define DIO75_PORT DOUT28_PORT
#define DIO75_BIT DOUT28_BIT
#define DIO75_AHBEN DOUT28_AHBEN
#define DIO75_GPIO DOUT28_GPIO
#endif
#if (defined(DOUT29_PORT) && defined(DOUT29_BIT))
#define DOUT29 76
#define DOUT29_AHBEN (__rccgpioen__(DOUT29_PORT))
#define DOUT29_GPIO (__gpio__(DOUT29_PORT))
#define DIO76 76
#define DIO76_PORT DOUT29_PORT
#define DIO76_BIT DOUT29_BIT
#define DIO76_AHBEN DOUT29_AHBEN
#define DIO76_GPIO DOUT29_GPIO
#endif
#if (defined(DOUT30_PORT) && defined(DOUT30_BIT))
#define DOUT30 77
#define DOUT30_AHBEN (__rccgpioen__(DOUT30_PORT))
#define DOUT30_GPIO (__gpio__(DOUT30_PORT))
#define DIO77 77
#define DIO77_PORT DOUT30_PORT
#define DIO77_BIT DOUT30_BIT
#define DIO77_AHBEN DOUT30_AHBEN
#define DIO77_GPIO DOUT30_GPIO
#endif
#if (defined(DOUT31_PORT) && defined(DOUT31_BIT))
#define DOUT31 78
#define DOUT31_AHBEN (__rccgpioen__(DOUT31_PORT))
#define DOUT31_GPIO (__gpio__(DOUT31_PORT))
#define DIO78 78
#define DIO78_PORT DOUT31_PORT
#define DIO78_BIT DOUT31_BIT
#define DIO78_AHBEN DOUT31_AHBEN
#define DIO78_GPIO DOUT31_GPIO
#endif
#if (defined(LIMIT_X_PORT) && defined(LIMIT_X_BIT))
#define LIMIT_X 100
#define LIMIT_X_AHBEN (__rccgpioen__(LIMIT_X_PORT))
#define LIMIT_X_GPIO (__gpio__(LIMIT_X_PORT))
#define DIO100 100
#define DIO100_PORT LIMIT_X_PORT
#define DIO100_BIT LIMIT_X_BIT
#define DIO100_AHBEN LIMIT_X_AHBEN
#define DIO100_GPIO LIMIT_X_GPIO
#endif
#if (defined(LIMIT_Y_PORT) && defined(LIMIT_Y_BIT))
#define LIMIT_Y 101
#define LIMIT_Y_AHBEN (__rccgpioen__(LIMIT_Y_PORT))
#define LIMIT_Y_GPIO (__gpio__(LIMIT_Y_PORT))
#define DIO101 101
#define DIO101_PORT LIMIT_Y_PORT
#define DIO101_BIT LIMIT_Y_BIT
#define DIO101_AHBEN LIMIT_Y_AHBEN
#define DIO101_GPIO LIMIT_Y_GPIO
#endif
#if (defined(LIMIT_Z_PORT) && defined(LIMIT_Z_BIT))
#define LIMIT_Z 102
#define LIMIT_Z_AHBEN (__rccgpioen__(LIMIT_Z_PORT))
#define LIMIT_Z_GPIO (__gpio__(LIMIT_Z_PORT))
#define DIO102 102
#define DIO102_PORT LIMIT_Z_PORT
#define DIO102_BIT LIMIT_Z_BIT
#define DIO102_AHBEN LIMIT_Z_AHBEN
#define DIO102_GPIO LIMIT_Z_GPIO
#endif
#if (defined(LIMIT_X2_PORT) && defined(LIMIT_X2_BIT))
#define LIMIT_X2 103
#define LIMIT_X2_AHBEN (__rccgpioen__(LIMIT_X2_PORT))
#define LIMIT_X2_GPIO (__gpio__(LIMIT_X2_PORT))
#define DIO103 103
#define DIO103_PORT LIMIT_X2_PORT
#define DIO103_BIT LIMIT_X2_BIT
#define DIO103_AHBEN LIMIT_X2_AHBEN
#define DIO103_GPIO LIMIT_X2_GPIO
#endif
#if (defined(LIMIT_Y2_PORT) && defined(LIMIT_Y2_BIT))
#define LIMIT_Y2 104
#define LIMIT_Y2_AHBEN (__rccgpioen__(LIMIT_Y2_PORT))
#define LIMIT_Y2_GPIO (__gpio__(LIMIT_Y2_PORT))
#define DIO104 104
#define DIO104_PORT LIMIT_Y2_PORT
#define DIO104_BIT LIMIT_Y2_BIT
#define DIO104_AHBEN LIMIT_Y2_AHBEN
#define DIO104_GPIO LIMIT_Y2_GPIO
#endif
#if (defined(LIMIT_Z2_PORT) && defined(LIMIT_Z2_BIT))
#define LIMIT_Z2 105
#define LIMIT_Z2_AHBEN (__rccgpioen__(LIMIT_Z2_PORT))
#define LIMIT_Z2_GPIO (__gpio__(LIMIT_Z2_PORT))
#define DIO105 105
#define DIO105_PORT LIMIT_Z2_PORT
#define DIO105_BIT LIMIT_Z2_BIT
#define DIO105_AHBEN LIMIT_Z2_AHBEN
#define DIO105_GPIO LIMIT_Z2_GPIO
#endif
#if (defined(LIMIT_A_PORT) && defined(LIMIT_A_BIT))
#define LIMIT_A 106
#define LIMIT_A_AHBEN (__rccgpioen__(LIMIT_A_PORT))
#define LIMIT_A_GPIO (__gpio__(LIMIT_A_PORT))
#define DIO106 106
#define DIO106_PORT LIMIT_A_PORT
#define DIO106_BIT LIMIT_A_BIT
#define DIO106_AHBEN LIMIT_A_AHBEN
#define DIO106_GPIO LIMIT_A_GPIO
#endif
#if (defined(LIMIT_B_PORT) && defined(LIMIT_B_BIT))
#define LIMIT_B 107
#define LIMIT_B_AHBEN (__rccgpioen__(LIMIT_B_PORT))
#define LIMIT_B_GPIO (__gpio__(LIMIT_B_PORT))
#define DIO107 107
#define DIO107_PORT LIMIT_B_PORT
#define DIO107_BIT LIMIT_B_BIT
#define DIO107_AHBEN LIMIT_B_AHBEN
#define DIO107_GPIO LIMIT_B_GPIO
#endif
#if (defined(LIMIT_C_PORT) && defined(LIMIT_C_BIT))
#define LIMIT_C 108
#define LIMIT_C_AHBEN (__rccgpioen__(LIMIT_C_PORT))
#define LIMIT_C_GPIO (__gpio__(LIMIT_C_PORT))
#define DIO108 108
#define DIO108_PORT LIMIT_C_PORT
#define DIO108_BIT LIMIT_C_BIT
#define DIO108_AHBEN LIMIT_C_AHBEN
#define DIO108_GPIO LIMIT_C_GPIO
#endif
#if (defined(PROBE_PORT) && defined(PROBE_BIT))
#define PROBE 109
#define PROBE_AHBEN (__rccgpioen__(PROBE_PORT))
#define PROBE_GPIO (__gpio__(PROBE_PORT))
#define DIO109 109
#define DIO109_PORT PROBE_PORT
#define DIO109_BIT PROBE_BIT
#define DIO109_AHBEN PROBE_AHBEN
#define DIO109_GPIO PROBE_GPIO
#endif
#if (defined(ESTOP_PORT) && defined(ESTOP_BIT))
#define ESTOP 110
#define ESTOP_AHBEN (__rccgpioen__(ESTOP_PORT))
#define ESTOP_GPIO (__gpio__(ESTOP_PORT))
#define DIO110 110
#define DIO110_PORT ESTOP_PORT
#define DIO110_BIT ESTOP_BIT
#define DIO110_AHBEN ESTOP_AHBEN
#define DIO110_GPIO ESTOP_GPIO
#endif
#if (defined(SAFETY_DOOR_PORT) && defined(SAFETY_DOOR_BIT))
#define SAFETY_DOOR 111
#define SAFETY_DOOR_AHBEN (__rccgpioen__(SAFETY_DOOR_PORT))
#define SAFETY_DOOR_GPIO (__gpio__(SAFETY_DOOR_PORT))
#define DIO111 111
#define DIO111_PORT SAFETY_DOOR_PORT
#define DIO111_BIT SAFETY_DOOR_BIT
#define DIO111_AHBEN SAFETY_DOOR_AHBEN
#define DIO111_GPIO SAFETY_DOOR_GPIO
#endif
#if (defined(FHOLD_PORT) && defined(FHOLD_BIT))
#define FHOLD 112
#define FHOLD_AHBEN (__rccgpioen__(FHOLD_PORT))
#define FHOLD_GPIO (__gpio__(FHOLD_PORT))
#define DIO112 112
#define DIO112_PORT FHOLD_PORT
#define DIO112_BIT FHOLD_BIT
#define DIO112_AHBEN FHOLD_AHBEN
#define DIO112_GPIO FHOLD_GPIO
#endif
#if (defined(CS_RES_PORT) && defined(CS_RES_BIT))
#define CS_RES 113
#define CS_RES_AHBEN (__rccgpioen__(CS_RES_PORT))
#define CS_RES_GPIO (__gpio__(CS_RES_PORT))
#define DIO113 113
#define DIO113_PORT CS_RES_PORT
#define DIO113_BIT CS_RES_BIT
#define DIO113_AHBEN CS_RES_AHBEN
#define DIO113_GPIO CS_RES_GPIO
#endif
#if (defined(ANALOG0_PORT) && defined(ANALOG0_BIT))
#define ANALOG0 114
#define ANALOG0_AHBEN (__rccgpioen__(ANALOG0_PORT))
#define ANALOG0_GPIO (__gpio__(ANALOG0_PORT))
#define DIO114 114
#define DIO114_PORT ANALOG0_PORT
#define DIO114_BIT ANALOG0_BIT
#define DIO114_AHBEN ANALOG0_AHBEN
#define DIO114_GPIO ANALOG0_GPIO
#endif
#if (defined(ANALOG1_PORT) && defined(ANALOG1_BIT))
#define ANALOG1 115
#define ANALOG1_AHBEN (__rccgpioen__(ANALOG1_PORT))
#define ANALOG1_GPIO (__gpio__(ANALOG1_PORT))
#define DIO115 115
#define DIO115_PORT ANALOG1_PORT
#define DIO115_BIT ANALOG1_BIT
#define DIO115_AHBEN ANALOG1_AHBEN
#define DIO115_GPIO ANALOG1_GPIO
#endif
#if (defined(ANALOG2_PORT) && defined(ANALOG2_BIT))
#define ANALOG2 116
#define ANALOG2_AHBEN (__rccgpioen__(ANALOG2_PORT))
#define ANALOG2_GPIO (__gpio__(ANALOG2_PORT))
#define DIO116 116
#define DIO116_PORT ANALOG2_PORT
#define DIO116_BIT ANALOG2_BIT
#define DIO116_AHBEN ANALOG2_AHBEN
#define DIO116_GPIO ANALOG2_GPIO
#endif
#if (defined(ANALOG3_PORT) && defined(ANALOG3_BIT))
#define ANALOG3 117
#define ANALOG3_AHBEN (__rccgpioen__(ANALOG3_PORT))
#define ANALOG3_GPIO (__gpio__(ANALOG3_PORT))
#define DIO117 117
#define DIO117_PORT ANALOG3_PORT
#define DIO117_BIT ANALOG3_BIT
#define DIO117_AHBEN ANALOG3_AHBEN
#define DIO117_GPIO ANALOG3_GPIO
#endif
#if (defined(ANALOG4_PORT) && defined(ANALOG4_BIT))
#define ANALOG4 118
#define ANALOG4_AHBEN (__rccgpioen__(ANALOG4_PORT))
#define ANALOG4_GPIO (__gpio__(ANALOG4_PORT))
#define DIO118 118
#define DIO118_PORT ANALOG4_PORT
#define DIO118_BIT ANALOG4_BIT
#define DIO118_AHBEN ANALOG4_AHBEN
#define DIO118_GPIO ANALOG4_GPIO
#endif
#if (defined(ANALOG5_PORT) && defined(ANALOG5_BIT))
#define ANALOG5 119
#define ANALOG5_AHBEN (__rccgpioen__(ANALOG5_PORT))
#define ANALOG5_GPIO (__gpio__(ANALOG5_PORT))
#define DIO119 119
#define DIO119_PORT ANALOG5_PORT
#define DIO119_BIT ANALOG5_BIT
#define DIO119_AHBEN ANALOG5_AHBEN
#define DIO119_GPIO ANALOG5_GPIO
#endif
#if (defined(ANALOG6_PORT) && defined(ANALOG6_BIT))
#define ANALOG6 120
#define ANALOG6_AHBEN (__rccgpioen__(ANALOG6_PORT))
#define ANALOG6_GPIO (__gpio__(ANALOG6_PORT))
#define DIO120 120
#define DIO120_PORT ANALOG6_PORT
#define DIO120_BIT ANALOG6_BIT
#define DIO120_AHBEN ANALOG6_AHBEN
#define DIO120_GPIO ANALOG6_GPIO
#endif
#if (defined(ANALOG7_PORT) && defined(ANALOG7_BIT))
#define ANALOG7 121
#define ANALOG7_AHBEN (__rccgpioen__(ANALOG7_PORT))
#define ANALOG7_GPIO (__gpio__(ANALOG7_PORT))
#define DIO121 121
#define DIO121_PORT ANALOG7_PORT
#define DIO121_BIT ANALOG7_BIT
#define DIO121_AHBEN ANALOG7_AHBEN
#define DIO121_GPIO ANALOG7_GPIO
#endif
#if (defined(ANALOG8_PORT) && defined(ANALOG8_BIT))
#define ANALOG8 122
#define ANALOG8_AHBEN (__rccgpioen__(ANALOG8_PORT))
#define ANALOG8_GPIO (__gpio__(ANALOG8_PORT))
#define DIO122 122
#define DIO122_PORT ANALOG8_PORT
#define DIO122_BIT ANALOG8_BIT
#define DIO122_AHBEN ANALOG8_AHBEN
#define DIO122_GPIO ANALOG8_GPIO
#endif
#if (defined(ANALOG9_PORT) && defined(ANALOG9_BIT))
#define ANALOG9 123
#define ANALOG9_AHBEN (__rccgpioen__(ANALOG9_PORT))
#define ANALOG9_GPIO (__gpio__(ANALOG9_PORT))
#define DIO123 123
#define DIO123_PORT ANALOG9_PORT
#define DIO123_BIT ANALOG9_BIT
#define DIO123_AHBEN ANALOG9_AHBEN
#define DIO123_GPIO ANALOG9_GPIO
#endif
#if (defined(ANALOG10_PORT) && defined(ANALOG10_BIT))
#define ANALOG10 124
#define ANALOG10_AHBEN (__rccgpioen__(ANALOG10_PORT))
#define ANALOG10_GPIO (__gpio__(ANALOG10_PORT))
#define DIO124 124
#define DIO124_PORT ANALOG10_PORT
#define DIO124_BIT ANALOG10_BIT
#define DIO124_AHBEN ANALOG10_AHBEN
#define DIO124_GPIO ANALOG10_GPIO
#endif
#if (defined(ANALOG11_PORT) && defined(ANALOG11_BIT))
#define ANALOG11 125
#define ANALOG11_AHBEN (__rccgpioen__(ANALOG11_PORT))
#define ANALOG11_GPIO (__gpio__(ANALOG11_PORT))
#define DIO125 125
#define DIO125_PORT ANALOG11_PORT
#define DIO125_BIT ANALOG11_BIT
#define DIO125_AHBEN ANALOG11_AHBEN
#define DIO125_GPIO ANALOG11_GPIO
#endif
#if (defined(ANALOG12_PORT) && defined(ANALOG12_BIT))
#define ANALOG12 126
#define ANALOG12_AHBEN (__rccgpioen__(ANALOG12_PORT))
#define ANALOG12_GPIO (__gpio__(ANALOG12_PORT))
#define DIO126 126
#define DIO126_PORT ANALOG12_PORT
#define DIO126_BIT ANALOG12_BIT
#define DIO126_AHBEN ANALOG12_AHBEN
#define DIO126_GPIO ANALOG12_GPIO
#endif
#if (defined(ANALOG13_PORT) && defined(ANALOG13_BIT))
#define ANALOG13 127
#define ANALOG13_AHBEN (__rccgpioen__(ANALOG13_PORT))
#define ANALOG13_GPIO (__gpio__(ANALOG13_PORT))
#define DIO127 127
#define DIO127_PORT ANALOG13_PORT
#define DIO127_BIT ANALOG13_BIT
#define DIO127_AHBEN ANALOG13_AHBEN
#define DIO127_GPIO ANALOG13_GPIO
#endif
#if (defined(ANALOG14_PORT) && defined(ANALOG14_BIT))
#define ANALOG14 128
#define ANALOG14_AHBEN (__rccgpioen__(ANALOG14_PORT))
#define ANALOG14_GPIO (__gpio__(ANALOG14_PORT))
#define DIO128 128
#define DIO128_PORT ANALOG14_PORT
#define DIO128_BIT ANALOG14_BIT
#define DIO128_AHBEN ANALOG14_AHBEN
#define DIO128_GPIO ANALOG14_GPIO
#endif
#if (defined(ANALOG15_PORT) && defined(ANALOG15_BIT))
#define ANALOG15 129
#define ANALOG15_AHBEN (__rccgpioen__(ANALOG15_PORT))
#define ANALOG15_GPIO (__gpio__(ANALOG15_PORT))
#define DIO129 129
#define DIO129_PORT ANALOG15_PORT
#define DIO129_BIT ANALOG15_BIT
#define DIO129_AHBEN ANALOG15_AHBEN
#define DIO129_GPIO ANALOG15_GPIO
#endif
#if (defined(DIN0_PORT) && defined(DIN0_BIT))
#define DIN0 130
#define DIN0_AHBEN (__rccgpioen__(DIN0_PORT))
#define DIN0_GPIO (__gpio__(DIN0_PORT))
#define DIO130 130
#define DIO130_PORT DIN0_PORT
#define DIO130_BIT DIN0_BIT
#define DIO130_AHBEN DIN0_AHBEN
#define DIO130_GPIO DIN0_GPIO
#endif
#if (defined(DIN1_PORT) && defined(DIN1_BIT))
#define DIN1 131
#define DIN1_AHBEN (__rccgpioen__(DIN1_PORT))
#define DIN1_GPIO (__gpio__(DIN1_PORT))
#define DIO131 131
#define DIO131_PORT DIN1_PORT
#define DIO131_BIT DIN1_BIT
#define DIO131_AHBEN DIN1_AHBEN
#define DIO131_GPIO DIN1_GPIO
#endif
#if (defined(DIN2_PORT) && defined(DIN2_BIT))
#define DIN2 132
#define DIN2_AHBEN (__rccgpioen__(DIN2_PORT))
#define DIN2_GPIO (__gpio__(DIN2_PORT))
#define DIO132 132
#define DIO132_PORT DIN2_PORT
#define DIO132_BIT DIN2_BIT
#define DIO132_AHBEN DIN2_AHBEN
#define DIO132_GPIO DIN2_GPIO
#endif
#if (defined(DIN3_PORT) && defined(DIN3_BIT))
#define DIN3 133
#define DIN3_AHBEN (__rccgpioen__(DIN3_PORT))
#define DIN3_GPIO (__gpio__(DIN3_PORT))
#define DIO133 133
#define DIO133_PORT DIN3_PORT
#define DIO133_BIT DIN3_BIT
#define DIO133_AHBEN DIN3_AHBEN
#define DIO133_GPIO DIN3_GPIO
#endif
#if (defined(DIN4_PORT) && defined(DIN4_BIT))
#define DIN4 134
#define DIN4_AHBEN (__rccgpioen__(DIN4_PORT))
#define DIN4_GPIO (__gpio__(DIN4_PORT))
#define DIO134 134
#define DIO134_PORT DIN4_PORT
#define DIO134_BIT DIN4_BIT
#define DIO134_AHBEN DIN4_AHBEN
#define DIO134_GPIO DIN4_GPIO
#endif
#if (defined(DIN5_PORT) && defined(DIN5_BIT))
#define DIN5 135
#define DIN5_AHBEN (__rccgpioen__(DIN5_PORT))
#define DIN5_GPIO (__gpio__(DIN5_PORT))
#define DIO135 135
#define DIO135_PORT DIN5_PORT
#define DIO135_BIT DIN5_BIT
#define DIO135_AHBEN DIN5_AHBEN
#define DIO135_GPIO DIN5_GPIO
#endif
#if (defined(DIN6_PORT) && defined(DIN6_BIT))
#define DIN6 136
#define DIN6_AHBEN (__rccgpioen__(DIN6_PORT))
#define DIN6_GPIO (__gpio__(DIN6_PORT))
#define DIO136 136
#define DIO136_PORT DIN6_PORT
#define DIO136_BIT DIN6_BIT
#define DIO136_AHBEN DIN6_AHBEN
#define DIO136_GPIO DIN6_GPIO
#endif
#if (defined(DIN7_PORT) && defined(DIN7_BIT))
#define DIN7 137
#define DIN7_AHBEN (__rccgpioen__(DIN7_PORT))
#define DIN7_GPIO (__gpio__(DIN7_PORT))
#define DIO137 137
#define DIO137_PORT DIN7_PORT
#define DIO137_BIT DIN7_BIT
#define DIO137_AHBEN DIN7_AHBEN
#define DIO137_GPIO DIN7_GPIO
#endif
#if (defined(DIN8_PORT) && defined(DIN8_BIT))
#define DIN8 138
#define DIN8_AHBEN (__rccgpioen__(DIN8_PORT))
#define DIN8_GPIO (__gpio__(DIN8_PORT))
#define DIO138 138
#define DIO138_PORT DIN8_PORT
#define DIO138_BIT DIN8_BIT
#define DIO138_AHBEN DIN8_AHBEN
#define DIO138_GPIO DIN8_GPIO
#endif
#if (defined(DIN9_PORT) && defined(DIN9_BIT))
#define DIN9 139
#define DIN9_AHBEN (__rccgpioen__(DIN9_PORT))
#define DIN9_GPIO (__gpio__(DIN9_PORT))
#define DIO139 139
#define DIO139_PORT DIN9_PORT
#define DIO139_BIT DIN9_BIT
#define DIO139_AHBEN DIN9_AHBEN
#define DIO139_GPIO DIN9_GPIO
#endif
#if (defined(DIN10_PORT) && defined(DIN10_BIT))
#define DIN10 140
#define DIN10_AHBEN (__rccgpioen__(DIN10_PORT))
#define DIN10_GPIO (__gpio__(DIN10_PORT))
#define DIO140 140
#define DIO140_PORT DIN10_PORT
#define DIO140_BIT DIN10_BIT
#define DIO140_AHBEN DIN10_AHBEN
#define DIO140_GPIO DIN10_GPIO
#endif
#if (defined(DIN11_PORT) && defined(DIN11_BIT))
#define DIN11 141
#define DIN11_AHBEN (__rccgpioen__(DIN11_PORT))
#define DIN11_GPIO (__gpio__(DIN11_PORT))
#define DIO141 141
#define DIO141_PORT DIN11_PORT
#define DIO141_BIT DIN11_BIT
#define DIO141_AHBEN DIN11_AHBEN
#define DIO141_GPIO DIN11_GPIO
#endif
#if (defined(DIN12_PORT) && defined(DIN12_BIT))
#define DIN12 142
#define DIN12_AHBEN (__rccgpioen__(DIN12_PORT))
#define DIN12_GPIO (__gpio__(DIN12_PORT))
#define DIO142 142
#define DIO142_PORT DIN12_PORT
#define DIO142_BIT DIN12_BIT
#define DIO142_AHBEN DIN12_AHBEN
#define DIO142_GPIO DIN12_GPIO
#endif
#if (defined(DIN13_PORT) && defined(DIN13_BIT))
#define DIN13 143
#define DIN13_AHBEN (__rccgpioen__(DIN13_PORT))
#define DIN13_GPIO (__gpio__(DIN13_PORT))
#define DIO143 143
#define DIO143_PORT DIN13_PORT
#define DIO143_BIT DIN13_BIT
#define DIO143_AHBEN DIN13_AHBEN
#define DIO143_GPIO DIN13_GPIO
#endif
#if (defined(DIN14_PORT) && defined(DIN14_BIT))
#define DIN14 144
#define DIN14_AHBEN (__rccgpioen__(DIN14_PORT))
#define DIN14_GPIO (__gpio__(DIN14_PORT))
#define DIO144 144
#define DIO144_PORT DIN14_PORT
#define DIO144_BIT DIN14_BIT
#define DIO144_AHBEN DIN14_AHBEN
#define DIO144_GPIO DIN14_GPIO
#endif
#if (defined(DIN15_PORT) && defined(DIN15_BIT))
#define DIN15 145
#define DIN15_AHBEN (__rccgpioen__(DIN15_PORT))
#define DIN15_GPIO (__gpio__(DIN15_PORT))
#define DIO145 145
#define DIO145_PORT DIN15_PORT
#define DIO145_BIT DIN15_BIT
#define DIO145_AHBEN DIN15_AHBEN
#define DIO145_GPIO DIN15_GPIO
#endif
#if (defined(DIN16_PORT) && defined(DIN16_BIT))
#define DIN16 146
#define DIN16_AHBEN (__rccgpioen__(DIN16_PORT))
#define DIN16_GPIO (__gpio__(DIN16_PORT))
#define DIO146 146
#define DIO146_PORT DIN16_PORT
#define DIO146_BIT DIN16_BIT
#define DIO146_AHBEN DIN16_AHBEN
#define DIO146_GPIO DIN16_GPIO
#endif
#if (defined(DIN17_PORT) && defined(DIN17_BIT))
#define DIN17 147
#define DIN17_AHBEN (__rccgpioen__(DIN17_PORT))
#define DIN17_GPIO (__gpio__(DIN17_PORT))
#define DIO147 147
#define DIO147_PORT DIN17_PORT
#define DIO147_BIT DIN17_BIT
#define DIO147_AHBEN DIN17_AHBEN
#define DIO147_GPIO DIN17_GPIO
#endif
#if (defined(DIN18_PORT) && defined(DIN18_BIT))
#define DIN18 148
#define DIN18_AHBEN (__rccgpioen__(DIN18_PORT))
#define DIN18_GPIO (__gpio__(DIN18_PORT))
#define DIO148 148
#define DIO148_PORT DIN18_PORT
#define DIO148_BIT DIN18_BIT
#define DIO148_AHBEN DIN18_AHBEN
#define DIO148_GPIO DIN18_GPIO
#endif
#if (defined(DIN19_PORT) && defined(DIN19_BIT))
#define DIN19 149
#define DIN19_AHBEN (__rccgpioen__(DIN19_PORT))
#define DIN19_GPIO (__gpio__(DIN19_PORT))
#define DIO149 149
#define DIO149_PORT DIN19_PORT
#define DIO149_BIT DIN19_BIT
#define DIO149_AHBEN DIN19_AHBEN
#define DIO149_GPIO DIN19_GPIO
#endif
#if (defined(DIN20_PORT) && defined(DIN20_BIT))
#define DIN20 150
#define DIN20_AHBEN (__rccgpioen__(DIN20_PORT))
#define DIN20_GPIO (__gpio__(DIN20_PORT))
#define DIO150 150
#define DIO150_PORT DIN20_PORT
#define DIO150_BIT DIN20_BIT
#define DIO150_AHBEN DIN20_AHBEN
#define DIO150_GPIO DIN20_GPIO
#endif
#if (defined(DIN21_PORT) && defined(DIN21_BIT))
#define DIN21 151
#define DIN21_AHBEN (__rccgpioen__(DIN21_PORT))
#define DIN21_GPIO (__gpio__(DIN21_PORT))
#define DIO151 151
#define DIO151_PORT DIN21_PORT
#define DIO151_BIT DIN21_BIT
#define DIO151_AHBEN DIN21_AHBEN
#define DIO151_GPIO DIN21_GPIO
#endif
#if (defined(DIN22_PORT) && defined(DIN22_BIT))
#define DIN22 152
#define DIN22_AHBEN (__rccgpioen__(DIN22_PORT))
#define DIN22_GPIO (__gpio__(DIN22_PORT))
#define DIO152 152
#define DIO152_PORT DIN22_PORT
#define DIO152_BIT DIN22_BIT
#define DIO152_AHBEN DIN22_AHBEN
#define DIO152_GPIO DIN22_GPIO
#endif
#if (defined(DIN23_PORT) && defined(DIN23_BIT))
#define DIN23 153
#define DIN23_AHBEN (__rccgpioen__(DIN23_PORT))
#define DIN23_GPIO (__gpio__(DIN23_PORT))
#define DIO153 153
#define DIO153_PORT DIN23_PORT
#define DIO153_BIT DIN23_BIT
#define DIO153_AHBEN DIN23_AHBEN
#define DIO153_GPIO DIN23_GPIO
#endif
#if (defined(DIN24_PORT) && defined(DIN24_BIT))
#define DIN24 154
#define DIN24_AHBEN (__rccgpioen__(DIN24_PORT))
#define DIN24_GPIO (__gpio__(DIN24_PORT))
#define DIO154 154
#define DIO154_PORT DIN24_PORT
#define DIO154_BIT DIN24_BIT
#define DIO154_AHBEN DIN24_AHBEN
#define DIO154_GPIO DIN24_GPIO
#endif
#if (defined(DIN25_PORT) && defined(DIN25_BIT))
#define DIN25 155
#define DIN25_AHBEN (__rccgpioen__(DIN25_PORT))
#define DIN25_GPIO (__gpio__(DIN25_PORT))
#define DIO155 155
#define DIO155_PORT DIN25_PORT
#define DIO155_BIT DIN25_BIT
#define DIO155_AHBEN DIN25_AHBEN
#define DIO155_GPIO DIN25_GPIO
#endif
#if (defined(DIN26_PORT) && defined(DIN26_BIT))
#define DIN26 156
#define DIN26_AHBEN (__rccgpioen__(DIN26_PORT))
#define DIN26_GPIO (__gpio__(DIN26_PORT))
#define DIO156 156
#define DIO156_PORT DIN26_PORT
#define DIO156_BIT DIN26_BIT
#define DIO156_AHBEN DIN26_AHBEN
#define DIO156_GPIO DIN26_GPIO
#endif
#if (defined(DIN27_PORT) && defined(DIN27_BIT))
#define DIN27 157
#define DIN27_AHBEN (__rccgpioen__(DIN27_PORT))
#define DIN27_GPIO (__gpio__(DIN27_PORT))
#define DIO157 157
#define DIO157_PORT DIN27_PORT
#define DIO157_BIT DIN27_BIT
#define DIO157_AHBEN DIN27_AHBEN
#define DIO157_GPIO DIN27_GPIO
#endif
#if (defined(DIN28_PORT) && defined(DIN28_BIT))
#define DIN28 158
#define DIN28_AHBEN (__rccgpioen__(DIN28_PORT))
#define DIN28_GPIO (__gpio__(DIN28_PORT))
#define DIO158 158
#define DIO158_PORT DIN28_PORT
#define DIO158_BIT DIN28_BIT
#define DIO158_AHBEN DIN28_AHBEN
#define DIO158_GPIO DIN28_GPIO
#endif
#if (defined(DIN29_PORT) && defined(DIN29_BIT))
#define DIN29 159
#define DIN29_AHBEN (__rccgpioen__(DIN29_PORT))
#define DIN29_GPIO (__gpio__(DIN29_PORT))
#define DIO159 159
#define DIO159_PORT DIN29_PORT
#define DIO159_BIT DIN29_BIT
#define DIO159_AHBEN DIN29_AHBEN
#define DIO159_GPIO DIN29_GPIO
#endif
#if (defined(DIN30_PORT) && defined(DIN30_BIT))
#define DIN30 160
#define DIN30_AHBEN (__rccgpioen__(DIN30_PORT))
#define DIN30_GPIO (__gpio__(DIN30_PORT))
#define DIO160 160
#define DIO160_PORT DIN30_PORT
#define DIO160_BIT DIN30_BIT
#define DIO160_AHBEN DIN30_AHBEN
#define DIO160_GPIO DIN30_GPIO
#endif
#if (defined(DIN31_PORT) && defined(DIN31_BIT))
#define DIN31 161
#define DIN31_AHBEN (__rccgpioen__(DIN31_PORT))
#define DIN31_GPIO (__gpio__(DIN31_PORT))
#define DIO161 161
#define DIO161_PORT DIN31_PORT
#define DIO161_BIT DIN31_BIT
#define DIO161_AHBEN DIN31_AHBEN
#define DIO161_GPIO DIN31_GPIO
#endif
#if (defined(TX_PORT) && defined(TX_BIT))
#define TX 200
#define TX_AHBEN (__rccgpioen__(TX_PORT))
#define TX_GPIO (__gpio__(TX_PORT))
#define DIO200 200
#define DIO200_PORT TX_PORT
#define DIO200_BIT TX_BIT
#define DIO200_AHBEN TX_AHBEN
#define DIO200_GPIO TX_GPIO
#endif
#if (defined(RX_PORT) && defined(RX_BIT))
#define RX 201
#define RX_AHBEN (__rccgpioen__(RX_PORT))
#define RX_GPIO (__gpio__(RX_PORT))
#define DIO201 201
#define DIO201_PORT RX_PORT
#define DIO201_BIT RX_BIT
#define DIO201_AHBEN RX_AHBEN
#define DIO201_GPIO RX_GPIO
#endif
#if (defined(USB_DM_PORT) && defined(USB_DM_BIT))
#define USB_DM 202
#define USB_DM_AHBEN (__rccgpioen__(USB_DM_PORT))
#define USB_DM_GPIO (__gpio__(USB_DM_PORT))
#define DIO202 202
#define DIO202_PORT USB_DM_PORT
#define DIO202_BIT USB_DM_BIT
#define DIO202_AHBEN USB_DM_AHBEN
#define DIO202_GPIO USB_DM_GPIO
#endif
#if (defined(USB_DP_PORT) && defined(USB_DP_BIT))
#define USB_DP 203
#define USB_DP_AHBEN (__rccgpioen__(USB_DP_PORT))
#define USB_DP_GPIO (__gpio__(USB_DP_PORT))
#define DIO203 203
#define DIO203_PORT USB_DP_PORT
#define DIO203_BIT USB_DP_BIT
#define DIO203_AHBEN USB_DP_AHBEN
#define DIO203_GPIO USB_DP_GPIO
#endif
#if (defined(SPI_CLK_PORT) && defined(SPI_CLK_BIT))
#define SPI_CLK 204
#define SPI_CLK_AHBEN (__rccgpioen__(SPI_CLK_PORT))
#define SPI_CLK_GPIO (__gpio__(SPI_CLK_PORT))
#define DIO204 204
#define DIO204_PORT SPI_CLK_PORT
#define DIO204_BIT SPI_CLK_BIT
#define DIO204_AHBEN SPI_CLK_AHBEN
#define DIO204_GPIO SPI_CLK_GPIO
#endif
#if (defined(SPI_SDI_PORT) && defined(SPI_SDI_BIT))
#define SPI_SDI 205
#define SPI_SDI_AHBEN (__rccgpioen__(SPI_SDI_PORT))
#define SPI_SDI_GPIO (__gpio__(SPI_SDI_PORT))
#define DIO205 205
#define DIO205_PORT SPI_SDI_PORT
#define DIO205_BIT SPI_SDI_BIT
#define DIO205_AHBEN SPI_SDI_AHBEN
#define DIO205_GPIO SPI_SDI_GPIO
#endif
#if (defined(SPI_SDO_PORT) && defined(SPI_SDO_BIT))
#define SPI_SDO 206
#define SPI_SDO_AHBEN (__rccgpioen__(SPI_SDO_PORT))
#define SPI_SDO_GPIO (__gpio__(SPI_SDO_PORT))
#define DIO206 206
#define DIO206_PORT SPI_SDO_PORT
#define DIO206_BIT SPI_SDO_BIT
#define DIO206_AHBEN SPI_SDO_AHBEN
#define DIO206_GPIO SPI_SDO_GPIO
#endif
#if (defined(SPI_CS_PORT) && defined(SPI_CS_BIT))
#define SPI_CS 207
#define SPI_CS_AHBEN (__rccgpioen__(SPI_CS_PORT))
#define SPI_CS_GPIO (__gpio__(SPI_CS_PORT))
#define DIO207 207
#define DIO207_PORT SPI_CS_PORT
#define DIO207_BIT SPI_CS_BIT
#define DIO207_AHBEN SPI_CS_AHBEN
#define DIO207_GPIO SPI_CS_GPIO
#endif
#if (defined(I2C_SCL_PORT) && defined(I2C_SCL_BIT))
#define I2C_SCL 208
#define I2C_SCL_AHBEN (__rccgpioen__(I2C_SCL_PORT))
#define I2C_SCL_GPIO (__gpio__(I2C_SCL_PORT))
#define DIO208 208
#define DIO208_PORT I2C_SCL_PORT
#define DIO208_BIT I2C_SCL_BIT
#define DIO208_AHBEN I2C_SCL_AHBEN
#define DIO208_GPIO I2C_SCL_GPIO
#endif
#if (defined(I2C_SDA_PORT) && defined(I2C_SDA_BIT))
#define I2C_SDA 209
#define I2C_SDA_AHBEN (__rccgpioen__(I2C_SDA_PORT))
#define I2C_SDA_GPIO (__gpio__(I2C_SDA_PORT))
#define DIO209 209
#define DIO209_PORT I2C_SDA_PORT
#define DIO209_BIT I2C_SDA_BIT
#define DIO209_AHBEN I2C_SDA_AHBEN
#define DIO209_GPIO I2C_SDA_GPIO
#endif
#if (defined(TX2_PORT) && defined(TX2_BIT))
#define TX2 210
#define TX2_AHBEN (__rccgpioen__(TX2_PORT))
#define TX2_GPIO (__gpio__(TX2_PORT))
#define DIO210 210
#define DIO210_PORT TX2_PORT
#define DIO210_BIT TX2_BIT
#define DIO210_AHBEN TX2_AHBEN
#define DIO210_GPIO TX2_GPIO
#endif
#if (defined(RX2_PORT) && defined(RX2_BIT))
#define RX2 211
#define RX2_AHBEN (__rccgpioen__(RX2_PORT))
#define RX2_GPIO (__gpio__(RX2_PORT))
#define DIO211 211
#define DIO211_PORT RX2_PORT
#define DIO211_BIT RX2_BIT
#define DIO211_AHBEN RX2_AHBEN
#define DIO211_GPIO RX2_GPIO
#endif


#if (defined(TX) && defined(RX))
#define MCU_HAS_UART
#endif
#if (defined(TX2) && defined(RX2))
#define MCU_HAS_UART2
#endif
#if (defined(USB_DP) && defined(USB_DM))
#define GPIO_OTG_FS 0x0A
#define MCU_HAS_USB
	extern uint32_t tud_cdc_n_write_available(uint8_t itf);
	extern uint32_t tud_cdc_n_available(uint8_t itf);
	extern bool tud_cdc_n_connected(uint8_t itf);
#define usb_tx_available() (tud_cdc_n_write_available(0) || !tud_cdc_n_connected(0))
#define usb_rx_available() tud_cdc_n_available(0)
#endif

/**********************************************
 *	ISR on change inputs
 **********************************************/
#define EXTINT_A 0
#define EXTINT_B 1
#define EXTINT_C 2
#define EXTINT_D 3
#define EXTINT_E 4
#define EXTINT_F 5
#define EXTINT_G 6
#define EXTINT_H 7
#define EXTINT_I 8
#define EXTINT_J 9
#define EXTINT_K 10

#define _EXTINT(X) EXTINT_##X
#define EXTINT(X) _EXTINT(X)

#define EXTIRQ_0 EXTI0_1_IRQn
#define EXTIRQ_1 EXTI0_1_IRQn
#define EXTIRQ_2 EXTI2_3_IRQn
#define EXTIRQ_3 EXTI2_3_IRQn
#define EXTIRQ_4 EXTI4_15_IRQn
#define EXTIRQ_5 EXTI4_15_IRQn
#define EXTIRQ_6 EXTI4_15_IRQn
#define EXTIRQ_7 EXTI4_15_IRQn
#define EXTIRQ_8 EXTI4_15_IRQn
#define EXTIRQ_9 EXTI4_15_IRQn
#define EXTIRQ_10 EXTI4_15_IRQn
#define EXTIRQ_11 EXTI4_15_IRQn
#define EXTIRQ_12 EXTI4_15_IRQn
#define EXTIRQ_13 EXTI4_15_IRQn
#define EXTIRQ_14 EXTI4_15_IRQn
#define EXTIRQ_15 EXTI4_15_IRQn
#define _EXTIRQ(X) EXTIRQ_##X
#define EXTIRQ(X) _EXTIRQ(X)

#if (defined(LIMIT_X_ISR) && defined(LIMIT_X))
#define LIMIT_X_EXTIREG (LIMIT_X_BIT >> 2)
#define LIMIT_X_EXTIBITMASK (1 << LIMIT_X_BIT)
#define LIMIT_X_IRQ EXTIRQ(LIMIT_X_BIT)
#define LIMIT_X_EXTIVAL ((EXTINT(LIMIT_X_PORT)) << ((LIMIT_X_BIT & 0x03) << 2))
#define DIO100_EXTIREG LIMIT_X_EXTIREG
#define DIO100_EXTIVAL LIMIT_X_EXTIVAL
#define DIO100_IRQ LIMIT_X_IRQ
#define DIO100_EXTIBITMASK LIMIT_X_EXTIBITMASK
#else
#define LIMIT_X_EXTIMASK 0
#define LIMIT_X_EXTIBITMASK 0
#endif
#if (defined(LIMIT_Y_ISR) && defined(LIMIT_Y))
#define LIMIT_Y_EXTIREG (LIMIT_Y_BIT >> 2)
#define LIMIT_Y_EXTIBITMASK (1 << LIMIT_Y_BIT)
#define LIMIT_Y_IRQ EXTIRQ(LIMIT_Y_BIT)
#define LIMIT_Y_EXTIVAL ((EXTINT(LIMIT_Y_PORT)) << ((LIMIT_Y_BIT & 0x03) << 2))
#define DIO101_EXTIREG LIMIT_Y_EXTIREG
#define DIO101_EXTIVAL LIMIT_Y_EXTIVAL
#define DIO101_IRQ LIMIT_Y_IRQ
#define DIO101_EXTIBITMASK LIMIT_Y_EXTIBITMASK
#else
#define LIMIT_Y_EXTIMASK 0
#define LIMIT_Y_EXTIBITMASK 0
#endif
#if (defined(LIMIT_Z_ISR) && defined(LIMIT_Z))
#define LIMIT_Z_EXTIREG (LIMIT_Z_BIT >> 2)
#define LIMIT_Z_EXTIBITMASK (1 << LIMIT_Z_BIT)
#define LIMIT_Z_IRQ EXTIRQ(LIMIT_Z_BIT)
#define LIMIT_Z_EXTIVAL ((EXTINT(LIMIT_Z_PORT)) << ((LIMIT_Z_BIT & 0x03) << 2))
#define DIO102_EXTIREG LIMIT_Z_EXTIREG
#define DIO102_EXTIVAL LIMIT_Z_EXTIVAL
#define DIO102_IRQ LIMIT_Z_IRQ
#define DIO102_EXTIBITMASK LIMIT_Z_EXTIBITMASK
#else
#define LIMIT_Z_EXTIMASK 0
#define LIMIT_Z_EXTIBITMASK 0
#endif
#if (defined(LIMIT_X2_ISR) && defined(LIMIT_X2))
#define LIMIT_X2_EXTIREG (LIMIT_X2_BIT >> 2)
#define LIMIT_X2_EXTIBITMASK (1 << LIMIT_X2_BIT)
#define LIMIT_X2_IRQ EXTIRQ(LIMIT_X2_BIT)
#define LIMIT_X2_EXTIVAL ((EXTINT(LIMIT_X2_PORT)) << ((LIMIT_X2_BIT & 0x03) << 2))
#define DIO103_EXTIREG LIMIT_X2_EXTIREG
#define DIO103_EXTIVAL LIMIT_X2_EXTIVAL
#define DIO103_IRQ LIMIT_X2_IRQ
#define DIO103_EXTIBITMASK LIMIT_X2_EXTIBITMASK
#else
#define LIMIT_X2_EXTIMASK 0
#define LIMIT_X2_EXTIBITMASK 0
#endif
#if (defined(LIMIT_Y2_ISR) && defined(LIMIT_Y2))
#define LIMIT_Y2_EXTIREG (LIMIT_Y2_BIT >> 2)
#define LIMIT_Y2_EXTIBITMASK (1 << LIMIT_Y2_BIT)
#define LIMIT_Y2_IRQ EXTIRQ(LIMIT_Y2_BIT)
#define LIMIT_Y2_EXTIVAL ((EXTINT(LIMIT_Y2_PORT)) << ((LIMIT_Y2_BIT & 0x03) << 2))
#define DIO104_EXTIREG LIMIT_Y2_EXTIREG
#define DIO104_EXTIVAL LIMIT_Y2_EXTIVAL
#define DIO104_IRQ LIMIT_Y2_IRQ
#define DIO104_EXTIBITMASK LIMIT_Y2_EXTIBITMASK
#else
#define LIMIT_Y2_EXTIMASK 0
#define LIMIT_Y2_EXTIBITMASK 0
#endif
#if (defined(LIMIT_Z2_ISR) && defined(LIMIT_Z2))
#define LIMIT_Z2_EXTIREG (LIMIT_Z2_BIT >> 2)
#define LIMIT_Z2_EXTIBITMASK (1 << LIMIT_Z2_BIT)
#define LIMIT_Z2_IRQ EXTIRQ(LIMIT_Z2_BIT)
#define LIMIT_Z2_EXTIVAL ((EXTINT(LIMIT_Z2_PORT)) << ((LIMIT_Z2_BIT & 0x03) << 2))
#define DIO105_EXTIREG LIMIT_Z2_EXTIREG
#define DIO105_EXTIVAL LIMIT_Z2_EXTIVAL
#define DIO105_IRQ LIMIT_Z2_IRQ
#define DIO105_EXTIBITMASK LIMIT_Z2_EXTIBITMASK
#else
#define LIMIT_Z2_EXTIMASK 0
#define LIMIT_Z2_EXTIBITMASK 0
#endif
#if (defined(LIMIT_A_ISR) && defined(LIMIT_A))
#define LIMIT_A_EXTIREG (LIMIT_A_BIT >> 2)
#define LIMIT_A_EXTIBITMASK (1 << LIMIT_A_BIT)
#define LIMIT_A_IRQ EXTIRQ(LIMIT_A_BIT)
#define LIMIT_A_EXTIVAL ((EXTINT(LIMIT_A_PORT)) << ((LIMIT_A_BIT & 0x03) << 2))
#define DIO106_EXTIREG LIMIT_A_EXTIREG
#define DIO106_EXTIVAL LIMIT_A_EXTIVAL
#define DIO106_IRQ LIMIT_A_IRQ
#define DIO106_EXTIBITMASK LIMIT_A_EXTIBITMASK
#else
#define LIMIT_A_EXTIMASK 0
#define LIMIT_A_EXTIBITMASK 0
#endif
#if (defined(LIMIT_B_ISR) && defined(LIMIT_B))
#define LIMIT_B_EXTIREG (LIMIT_B_BIT >> 2)
#define LIMIT_B_EXTIBITMASK (1 << LIMIT_B_BIT)
#define LIMIT_B_IRQ EXTIRQ(LIMIT_B_BIT)
#define LIMIT_B_EXTIVAL ((EXTINT(LIMIT_B_PORT)) << ((LIMIT_B_BIT & 0x03) << 2))
#define DIO107_EXTIREG LIMIT_B_EXTIREG
#define DIO107_EXTIVAL LIMIT_B_EXTIVAL
#define DIO107_IRQ LIMIT_B_IRQ
#define DIO107_EXTIBITMASK LIMIT_B_EXTIBITMASK
#else
#define LIMIT_B_EXTIMASK 0
#define LIMIT_B_EXTIBITMASK 0
#endif
#if (defined(LIMIT_C_ISR) && defined(LIMIT_C))
#define LIMIT_C_EXTIREG (LIMIT_C_BIT >> 2)
#define LIMIT_C_EXTIBITMASK (1 << LIMIT_C_BIT)
#define LIMIT_C_IRQ EXTIRQ(LIMIT_C_BIT)
#define LIMIT_C_EXTIVAL ((EXTINT(LIMIT_C_PORT)) << ((LIMIT_C_BIT & 0x03) << 2))
#define DIO108_EXTIREG LIMIT_C_EXTIREG
#define DIO108_EXTIVAL LIMIT_C_EXTIVAL
#define DIO108_IRQ LIMIT_C_IRQ
#define DIO108_EXTIBITMASK LIMIT_C_EXTIBITMASK
#else
#define LIMIT_C_EXTIMASK 0
#define LIMIT_C_EXTIBITMASK 0
#endif
#if (defined(PROBE_ISR) && defined(PROBE))
#define PROBE_EXTIREG (PROBE_BIT >> 2)
#define PROBE_EXTIBITMASK (1 << PROBE_BIT)
#define PROBE_IRQ EXTIRQ(PROBE_BIT)
#define PROBE_EXTIVAL ((EXTINT(PROBE_PORT)) << ((PROBE_BIT & 0x03) << 2))
#define DIO109_EXTIREG PROBE_EXTIREG
#define DIO109_EXTIVAL PROBE_EXTIVAL
#define DIO109_IRQ PROBE_IRQ
#define DIO109_EXTIBITMASK PROBE_EXTIBITMASK
#else
#define PROBE_EXTIMASK 0
#define PROBE_EXTIBITMASK 0
#endif
#if (defined(ESTOP_ISR) && defined(ESTOP))
#define ESTOP_EXTIREG (ESTOP_BIT >> 2)
#define ESTOP_EXTIBITMASK (1 << ESTOP_BIT)
#define ESTOP_IRQ EXTIRQ(ESTOP_BIT)
#define ESTOP_EXTIVAL ((EXTINT(ESTOP_PORT)) << ((ESTOP_BIT & 0x03) << 2))
#define DIO110_EXTIREG ESTOP_EXTIREG
#define DIO110_EXTIVAL ESTOP_EXTIVAL
#define DIO110_IRQ ESTOP_IRQ
#define DIO110_EXTIBITMASK ESTOP_EXTIBITMASK
#else
#define ESTOP_EXTIMASK 0
#define ESTOP_EXTIBITMASK 0
#endif
#if (defined(SAFETY_DOOR_ISR) && defined(SAFETY_DOOR))
#define SAFETY_DOOR_EXTIREG (SAFETY_DOOR_BIT >> 2)
#define SAFETY_DOOR_EXTIBITMASK (1 << SAFETY_DOOR_BIT)
#define SAFETY_DOOR_IRQ EXTIRQ(SAFETY_DOOR_BIT)
#define SAFETY_DOOR_EXTIVAL ((EXTINT(SAFETY_DOOR_PORT)) << ((SAFETY_DOOR_BIT & 0x03) << 2))
#define DIO111_EXTIREG SAFETY_DOOR_EXTIREG
#define DIO111_EXTIVAL SAFETY_DOOR_EXTIVAL
#define DIO111_IRQ SAFETY_DOOR_IRQ
#define DIO111_EXTIBITMASK SAFETY_DOOR_EXTIBITMASK
#else
#define SAFETY_DOOR_EXTIMASK 0
#define SAFETY_DOOR_EXTIBITMASK 0
#endif
#if (defined(FHOLD_ISR) && defined(FHOLD))
#define FHOLD_EXTIREG (FHOLD_BIT >> 2)
#define FHOLD_EXTIBITMASK (1 << FHOLD_BIT)
#define FHOLD_IRQ EXTIRQ(FHOLD_BIT)
#define FHOLD_EXTIVAL ((EXTINT(FHOLD_PORT)) << ((FHOLD_BIT & 0x03) << 2))
#define DIO112_EXTIREG FHOLD_EXTIREG
#define DIO112_EXTIVAL FHOLD_EXTIVAL
#define DIO112_IRQ FHOLD_IRQ
#define DIO112_EXTIBITMASK FHOLD_EXTIBITMASK
#else
#define FHOLD_EXTIMASK 0
#define FHOLD_EXTIBITMASK 0
#endif
#if (defined(CS_RES_ISR) && defined(CS_RES))
#define CS_RES_EXTIREG (CS_RES_BIT >> 2)
#define CS_RES_EXTIBITMASK (1 << CS_RES_BIT)
#define CS_RES_IRQ EXTIRQ(CS_RES_BIT)
#define CS_RES_EXTIVAL ((EXTINT(CS_RES_PORT)) << ((CS_RES_BIT & 0x03) << 2))
#define DIO113_EXTIREG CS_RES_EXTIREG
#define DIO113_EXTIVAL CS_RES_EXTIVAL
#define DIO113_IRQ CS_RES_IRQ
#define DIO113_EXTIBITMASK CS_RES_EXTIBITMASK
#else
#define CS_RES_EXTIMASK 0
#define CS_RES_EXTIBITMASK 0
#endif
#if (defined(DIN0_ISR) && defined(DIN0))
#define DIN0_EXTIREG (DIN0_BIT >> 2)
#define DIN0_EXTIBITMASK (1 << DIN0_BIT)
#define DIN0_IRQ EXTIRQ(DIN0_BIT)
#define DIN0_EXTIVAL ((EXTINT(DIN0_PORT)) << ((DIN0_BIT & 0x03) << 2))
#define DIO130_EXTIREG DIN0_EXTIREG
#define DIO130_EXTIVAL DIN0_EXTIVAL
#define DIO130_IRQ DIN0_IRQ
#define DIO130_EXTIBITMASK DIN0_EXTIBITMASK
#else
#define DIN0_EXTIMASK 0
#define DIN0_EXTIBITMASK 0
#endif
#if (defined(DIN1_ISR) && defined(DIN1))
#define DIN1_EXTIREG (DIN1_BIT >> 2)
#define DIN1_EXTIBITMASK (1 << DIN1_BIT)
#define DIN1_IRQ EXTIRQ(DIN1_BIT)
#define DIN1_EXTIVAL ((EXTINT(DIN1_PORT)) << ((DIN1_BIT & 0x03) << 2))
#define DIO131_EXTIREG DIN1_EXTIREG
#define DIO131_EXTIVAL DIN1_EXTIVAL
#define DIO131_IRQ DIN1_IRQ
#define DIO131_EXTIBITMASK DIN1_EXTIBITMASK
#else
#define DIN1_EXTIMASK 0
#define DIN1_EXTIBITMASK 0
#endif
#if (defined(DIN2_ISR) && defined(DIN2))
#define DIN2_EXTIREG (DIN2_BIT >> 2)
#define DIN2_EXTIBITMASK (1 << DIN2_BIT)
#define DIN2_IRQ EXTIRQ(DIN2_BIT)
#define DIN2_EXTIVAL ((EXTINT(DIN2_PORT)) << ((DIN2_BIT & 0x03) << 2))
#define DIO132_EXTIREG DIN2_EXTIREG
#define DIO132_EXTIVAL DIN2_EXTIVAL
#define DIO132_IRQ DIN2_IRQ
#define DIO132_EXTIBITMASK DIN2_EXTIBITMASK
#else
#define DIN2_EXTIMASK 0
#define DIN2_EXTIBITMASK 0
#endif
#if (defined(DIN3_ISR) && defined(DIN3))
#define DIN3_EXTIREG (DIN3_BIT >> 2)
#define DIN3_EXTIBITMASK (1 << DIN3_BIT)
#define DIN3_IRQ EXTIRQ(DIN3_BIT)
#define DIN3_EXTIVAL ((EXTINT(DIN3_PORT)) << ((DIN3_BIT & 0x03) << 2))
#define DIO133_EXTIREG DIN3_EXTIREG
#define DIO133_EXTIVAL DIN3_EXTIVAL
#define DIO133_IRQ DIN3_IRQ
#define DIO133_EXTIBITMASK DIN3_EXTIBITMASK
#else
#define DIN3_EXTIMASK 0
#define DIN3_EXTIBITMASK 0
#endif
#if (defined(DIN4_ISR) && defined(DIN4))
#define DIN4_EXTIREG (DIN4_BIT >> 2)
#define DIN4_EXTIBITMASK (1 << DIN4_BIT)
#define DIN4_IRQ EXTIRQ(DIN4_BIT)
#define DIN4_EXTIVAL ((EXTINT(DIN4_PORT)) << ((DIN4_BIT & 0x03) << 2))
#define DIO134_EXTIREG DIN4_EXTIREG
#define DIO134_EXTIVAL DIN4_EXTIVAL
#define DIO134_IRQ DIN4_IRQ
#define DIO134_EXTIBITMASK DIN4_EXTIBITMASK
#else
#define DIN4_EXTIMASK 0
#define DIN4_EXTIBITMASK 0
#endif
#if (defined(DIN5_ISR) && defined(DIN5))
#define DIN5_EXTIREG (DIN5_BIT >> 2)
#define DIN5_EXTIBITMASK (1 << DIN5_BIT)
#define DIN5_IRQ EXTIRQ(DIN5_BIT)
#define DIN5_EXTIVAL ((EXTINT(DIN5_PORT)) << ((DIN5_BIT & 0x03) << 2))
#define DIO135_EXTIREG DIN5_EXTIREG
#define DIO135_EXTIVAL DIN5_EXTIVAL
#define DIO135_IRQ DIN5_IRQ
#define DIO135_EXTIBITMASK DIN5_EXTIBITMASK
#else
#define DIN5_EXTIMASK 0
#define DIN5_EXTIBITMASK 0
#endif
#if (defined(DIN6_ISR) && defined(DIN6))
#define DIN6_EXTIREG (DIN6_BIT >> 2)
#define DIN6_EXTIBITMASK (1 << DIN6_BIT)
#define DIN6_IRQ EXTIRQ(DIN6_BIT)
#define DIN6_EXTIVAL ((EXTINT(DIN6_PORT)) << ((DIN6_BIT & 0x03) << 2))
#define DIO136_EXTIREG DIN6_EXTIREG
#define DIO136_EXTIVAL DIN6_EXTIVAL
#define DIO136_IRQ DIN6_IRQ
#define DIO136_EXTIBITMASK DIN6_EXTIBITMASK
#else
#define DIN6_EXTIMASK 0
#define DIN6_EXTIBITMASK 0
#endif
#if (defined(DIN7_ISR) && defined(DIN7))
#define DIN7_EXTIREG (DIN7_BIT >> 2)
#define DIN7_EXTIBITMASK (1 << DIN7_BIT)
#define DIN7_IRQ EXTIRQ(DIN7_BIT)
#define DIN7_EXTIVAL ((EXTINT(DIN7_PORT)) << ((DIN7_BIT & 0x03) << 2))
#define DIO137_EXTIREG DIN7_EXTIREG
#define DIO137_EXTIVAL DIN7_EXTIVAL
#define DIO137_IRQ DIN7_IRQ
#define DIO137_EXTIBITMASK DIN7_EXTIBITMASK
#else
#define DIN7_EXTIMASK 0
#define DIN7_EXTIBITMASK 0
#endif

/**********************************************
 *	PWM pins
 **********************************************/
#if (defined(PWM0_CHANNEL) && defined(PWM0_TIMER) && defined(PWM0))
#if (PWM0_TIMER==1 || (PWM0_TIMER>=15 & PWM0_TIMER<=17))
#define PWM0_ENREG RCC->APB2ENR
#define PWM0_APBEN __helper__(RCC_APB2ENR_TIM,PWM0_TIMER, EN)
#define PWM0_CLOCK HAL_RCC_GetPCLK1Freq()
#else
#define PWM0_ENREG RCC->APB1ENR
#define PWM0_APBEN __helper__(RCC_APB1ENR_TIM,PWM0_TIMER, EN)
#define PWM0_CLOCK HAL_RCC_GetPCLK1Freq()
#endif
#define PWM0_TIMREG (__tim__(PWM0_TIMER))
#ifndef PWM0_FREQ
#define PWM0_FREQ 1000
#endif
#if (PWM0_CHANNEL & 0x01)
#define PWM0_MODE 0x0064
#else
#define PWM0_MODE 0x6400
#endif
#if (PWM0_CHANNEL > 2)
#define PWM0_CCMREG CCMR2
#else
#define PWM0_CCMREG CCMR1
#endif
#if (PWM0_TIMER==1) || (PWM0_TIMER>=15 & PWM0_TIMER<=17)
#define PWM0_ENOUTPUT {PWM0_TIMREG->BDTR |= (1 << 15);}
#else
#define PWM0_ENOUTPUT {}
#endif
#if (PWM0_TIMER==1)
#define PWM0_AF 0x02
#elif (PWM0_PORT==A)
#if (PWM0_TIMER==3)
#define PWM0_AF 0x01
#elif (PWM0_TIMER==14)
#define PWM0_AF 0x04
#elif (PWM0_TIMER==15)
#define PWM0_AF 0x00
#elif (PWM0_TIMER==16) || (PWM0_TIMER==17)
#define PWM0_AF 0x05
#endif
#elif (PWM0_PORT==B)
#if (PWM0_TIMER==14)
#define PWM0_AF 0x00
#elif (PWM0_TIMER==15)
#define PWM0_AF 0x01
#elif (PWM0_TIMER==16) || (PWM0_TIMER==17)
#define PWM0_AF 0x02
#endif
#elif (PWM0_PORT==C)
#if (PWM0_TIMER==3)
#define PWM0_AF 0x00
#endif
#endif
#define PWM0_CCR __ccr__(PWM0_CHANNEL)
#define DIO25_TIMER PWM0_TIMER
#define DIO25_CHANNEL PWM0_CHANNEL
#define DIO25_ENREG PWM0_ENREG
#define DIO25_APBEN PWM0_APBEN
#define DIO25_TIMREG PWM0_TIMREG
#define DIO25_APBEN PWM0_APBEN
#define DIO25_FREQ PWM0_FREQ
#define DIO25_MODE PWM0_MODE
#define DIO25_CCMREG PWM0_CCMREG
#define DIO25_CCR PWM0_CCR
#define DIO25_ENOUTPUT PWM0_ENOUTPUT
#define DIO25_AF PWM0_AF
#define DIO25_CLOCK PWM0_CLOCK
#endif
#if (defined(PWM1_CHANNEL) && defined(PWM1_TIMER) && defined(PWM1))
#if (PWM1_TIMER==1 || (PWM1_TIMER>=15 & PWM1_TIMER<=17))
#define PWM1_ENREG RCC->APB2ENR
#define PWM1_APBEN __helper__(RCC_APB2ENR_TIM,PWM1_TIMER, EN)
#define PWM1_CLOCK HAL_RCC_GetPCLK1Freq()
#else
#define PWM1_ENREG RCC->APB1ENR
#define PWM1_APBEN __helper__(RCC_APB1ENR_TIM,PWM1_TIMER, EN)
#define PWM1_CLOCK HAL_RCC_GetPCLK1Freq()
#endif
#define PWM1_TIMREG (__tim__(PWM1_TIMER))
#ifndef PWM1_FREQ
#define PWM1_FREQ 1000
#endif
#if (PWM1_CHANNEL & 0x01)
#define PWM1_MODE 0x0064
#else
#define PWM1_MODE 0x6400
#endif
#if (PWM1_CHANNEL > 2)
#define PWM1_CCMREG CCMR2
#else
#define PWM1_CCMREG CCMR1
#endif
#if (PWM1_TIMER==1) || (PWM1_TIMER>=15 & PWM1_TIMER<=17)
#define PWM1_ENOUTPUT {PWM1_TIMREG->BDTR |= (1 << 15);}
#else
#define PWM1_ENOUTPUT {}
#endif
#if (PWM1_TIMER==1)
#define PWM1_AF 0x02
#elif (PWM1_PORT==A)
#if (PWM1_TIMER==3)
#define PWM1_AF 0x01
#elif (PWM1_TIMER==14)
#define PWM1_AF 0x04
#elif (PWM1_TIMER==15)
#define PWM1_AF 0x00
#elif (PWM1_TIMER==16) || (PWM1_TIMER==17)
#define PWM1_AF 0x05
#endif
#elif (PWM1_PORT==B)
#if (PWM1_TIMER==14)
#define PWM1_AF 0x00
#elif (PWM1_TIMER==15)
#define PWM1_AF 0x01
#elif (PWM1_TIMER==16) || (PWM1_TIMER==17)
#define PWM1_AF 0x02
#endif
#elif (PWM1_PORT==C)
#if (PWM1_TIMER==3)
#define PWM1_AF 0x00
#endif
#endif
#define PWM1_CCR __ccr__(PWM1_CHANNEL)
#define DIO26_TIMER PWM1_TIMER
#define DIO26_CHANNEL PWM1_CHANNEL
#define DIO26_ENREG PWM1_ENREG
#define DIO26_APBEN PWM1_APBEN
#define DIO26_TIMREG PWM1_TIMREG
#define DIO26_APBEN PWM1_APBEN
#define DIO26_FREQ PWM1_FREQ
#define DIO26_MODE PWM1_MODE
#define DIO26_CCMREG PWM1_CCMREG
#define DIO26_CCR PWM1_CCR
#define DIO26_ENOUTPUT PWM1_ENOUTPUT
#define DIO26_AF PWM1_AF
#define DIO26_CLOCK PWM1_CLOCK
#endif
#if (defined(PWM2_CHANNEL) && defined(PWM2_TIMER) && defined(PWM2))
#if (PWM2_TIMER==1 || (PWM2_TIMER>=15 & PWM2_TIMER<=17))
#define PWM2_ENREG RCC->APB2ENR
#define PWM2_APBEN __helper__(RCC_APB2ENR_TIM,PWM2_TIMER, EN)
#define PWM2_CLOCK HAL_RCC_GetPCLK1Freq()
#else
#define PWM2_ENREG RCC->APB1ENR
#define PWM2_APBEN __helper__(RCC_APB1ENR_TIM,PWM2_TIMER, EN)
#define PWM2_CLOCK HAL_RCC_GetPCLK1Freq()
#endif
#define PWM2_TIMREG (__tim__(PWM2_TIMER))
#ifndef PWM2_FREQ
#define PWM2_FREQ 1000
#endif
#if (PWM2_CHANNEL & 0x01)
#define PWM2_MODE 0x0064
#else
#define PWM2_MODE 0x6400
#endif
#if (PWM2_CHANNEL > 2)
#define PWM2_CCMREG CCMR2
#else
#define PWM2_CCMREG CCMR1
#endif
#if (PWM2_TIMER==1) || (PWM2_TIMER>=15 & PWM2_TIMER<=17)
#define PWM2_ENOUTPUT {PWM2_TIMREG->BDTR |= (1 << 15);}
#else
#define PWM2_ENOUTPUT {}
#endif
#if (PWM2_TIMER==1)
#define PWM2_AF 0x02
#elif (PWM2_PORT==A)
#if (PWM2_TIMER==3)
#define PWM2_AF 0x01
#elif (PWM2_TIMER==14)
#define PWM2_AF 0x04
#elif (PWM2_TIMER==15)
#define PWM2_AF 0x00
#elif (PWM2_TIMER==16) || (PWM2_TIMER==17)
#define PWM2_AF 0x05
#endif
#elif (PWM2_PORT==B)
#if (PWM2_TIMER==14)
#define PWM2_AF 0x00
#elif (PWM2_TIMER==15)
#define PWM2_AF 0x01
#elif (PWM2_TIMER==16) || (PWM2_TIMER==17)
#define PWM2_AF 0x02
#endif
#elif (PWM2_PORT==C)
#if (PWM2_TIMER==3)
#define PWM2_AF 0x00
#endif
#endif
#define PWM2_CCR __ccr__(PWM2_CHANNEL)
#define DIO27_TIMER PWM2_TIMER
#define DIO27_CHANNEL PWM2_CHANNEL
#define DIO27_ENREG PWM2_ENREG
#define DIO27_APBEN PWM2_APBEN
#define DIO27_TIMREG PWM2_TIMREG
#define DIO27_APBEN PWM2_APBEN
#define DIO27_FREQ PWM2_FREQ
#define DIO27_MODE PWM2_MODE
#define DIO27_CCMREG PWM2_CCMREG
#define DIO27_CCR PWM2_CCR
#define DIO27_ENOUTPUT PWM2_ENOUTPUT
#define DIO27_AF PWM2_AF
#define DIO27_CLOCK PWM2_CLOCK
#endif
#if (defined(PWM3_CHANNEL) && defined(PWM3_TIMER) && defined(PWM3))
#if (PWM3_TIMER==1 || (PWM3_TIMER>=15 & PWM3_TIMER<=17))
#define PWM3_ENREG RCC->APB2ENR
#define PWM3_APBEN __helper__(RCC_APB2ENR_TIM,PWM3_TIMER, EN)
#define PWM3_CLOCK HAL_RCC_GetPCLK1Freq()
#else
#define PWM3_ENREG RCC->APB1ENR
#define PWM3_APBEN __helper__(RCC_APB1ENR_TIM,PWM3_TIMER, EN)
#define PWM3_CLOCK HAL_RCC_GetPCLK1Freq()
#endif
#define PWM3_TIMREG (__tim__(PWM3_TIMER))
#ifndef PWM3_FREQ
#define PWM3_FREQ 1000
#endif
#if (PWM3_CHANNEL & 0x01)
#define PWM3_MODE 0x0064
#else
#define PWM3_MODE 0x6400
#endif
#if (PWM3_CHANNEL > 2)
#define PWM3_CCMREG CCMR2
#else
#define PWM3_CCMREG CCMR1
#endif
#if (PWM3_TIMER==1) || (PWM3_TIMER>=15 & PWM3_TIMER<=17)
#define PWM3_ENOUTPUT {PWM3_TIMREG->BDTR |= (1 << 15);}
#else
#define PWM3_ENOUTPUT {}
#endif
#if (PWM3_TIMER==1)
#define PWM3_AF 0x02
#elif (PWM3_PORT==A)
#if (PWM3_TIMER==3)
#define PWM3_AF 0x01
#elif (PWM3_TIMER==14)
#define PWM3_AF 0x04
#elif (PWM3_TIMER==15)
#define PWM3_AF 0x00
#elif (PWM3_TIMER==16) || (PWM3_TIMER==17)
#define PWM3_AF 0x05
#endif
#elif (PWM3_PORT==B)
#if (PWM3_TIMER==14)
#define PWM3_AF 0x00
#elif (PWM3_TIMER==15)
#define PWM3_AF 0x01
#elif (PWM3_TIMER==16) || (PWM3_TIMER==17)
#define PWM3_AF 0x02
#endif
#elif (PWM3_PORT==C)
#if (PWM3_TIMER==3)
#define PWM3_AF 0x00
#endif
#endif
#define PWM3_CCR __ccr__(PWM3_CHANNEL)
#define DIO28_TIMER PWM3_TIMER
#define DIO28_CHANNEL PWM3_CHANNEL
#define DIO28_ENREG PWM3_ENREG
#define DIO28_APBEN PWM3_APBEN
#define DIO28_TIMREG PWM3_TIMREG
#define DIO28_APBEN PWM3_APBEN
#define DIO28_FREQ PWM3_FREQ
#define DIO28_MODE PWM3_MODE
#define DIO28_CCMREG PWM3_CCMREG
#define DIO28_CCR PWM3_CCR
#define DIO28_ENOUTPUT PWM3_ENOUTPUT
#define DIO28_AF PWM3_AF
#define DIO28_CLOCK PWM3_CLOCK
#endif
#if (defined(PWM4_CHANNEL) && defined(PWM4_TIMER) && defined(PWM4))
#if (PWM4_TIMER==1 || (PWM4_TIMER>=15 & PWM4_TIMER<=17))
#define PWM4_ENREG RCC->APB2ENR
#define PWM4_APBEN __helper__(RCC_APB2ENR_TIM,PWM4_TIMER, EN)
#define PWM4_CLOCK HAL_RCC_GetPCLK1Freq()
#else
#define PWM4_ENREG RCC->APB1ENR
#define PWM4_APBEN __helper__(RCC_APB1ENR_TIM,PWM4_TIMER, EN)
#define PWM4_CLOCK HAL_RCC_GetPCLK1Freq()
#endif
#define PWM4_TIMREG (__tim__(PWM4_TIMER))
#ifndef PWM4_FREQ
#define PWM4_FREQ 1000
#endif
#if (PWM4_CHANNEL & 0x01)
#define PWM4_MODE 0x0064
#else
#define PWM4_MODE 0x6400
#endif
#if (PWM4_CHANNEL > 2)
#define PWM4_CCMREG CCMR2
#else
#define PWM4_CCMREG CCMR1
#endif
#if (PWM4_TIMER==1) || (PWM4_TIMER>=15 & PWM4_TIMER<=17)
#define PWM4_ENOUTPUT {PWM4_TIMREG->BDTR |= (1 << 15);}
#else
#define PWM4_ENOUTPUT {}
#endif
#if (PWM4_TIMER==1)
#define PWM4_AF 0x02
#elif (PWM4_PORT==A)
#if (PWM4_TIMER==3)
#define PWM4_AF 0x01
#elif (PWM4_TIMER==14)
#define PWM4_AF 0x04
#elif (PWM4_TIMER==15)
#define PWM4_AF 0x00
#elif (PWM4_TIMER==16) || (PWM4_TIMER==17)
#define PWM4_AF 0x05
#endif
#elif (PWM4_PORT==B)
#if (PWM4_TIMER==14)
#define PWM4_AF 0x00
#elif (PWM4_TIMER==15)
#define PWM4_AF 0x01
#elif (PWM4_TIMER==16) || (PWM4_TIMER==17)
#define PWM4_AF 0x02
#endif
#elif (PWM4_PORT==C)
#if (PWM4_TIMER==3)
#define PWM4_AF 0x00
#endif
#endif
#define PWM4_CCR __ccr__(PWM4_CHANNEL)
#define DIO29_TIMER PWM4_TIMER
#define DIO29_CHANNEL PWM4_CHANNEL
#define DIO29_ENREG PWM4_ENREG
#define DIO29_APBEN PWM4_APBEN
#define DIO29_TIMREG PWM4_TIMREG
#define DIO29_APBEN PWM4_APBEN
#define DIO29_FREQ PWM4_FREQ
#define DIO29_MODE PWM4_MODE
#define DIO29_CCMREG PWM4_CCMREG
#define DIO29_CCR PWM4_CCR
#define DIO29_ENOUTPUT PWM4_ENOUTPUT
#define DIO29_AF PWM4_AF
#define DIO29_CLOCK PWM4_CLOCK
#endif
#if (defined(PWM5_CHANNEL) && defined(PWM5_TIMER) && defined(PWM5))
#if (PWM5_TIMER==1 || (PWM5_TIMER>=15 & PWM5_TIMER<=17))
#define PWM5_ENREG RCC->APB2ENR
#define PWM5_APBEN __helper__(RCC_APB2ENR_TIM,PWM5_TIMER, EN)
#define PWM5_CLOCK HAL_RCC_GetPCLK1Freq()
#else
#define PWM5_ENREG RCC->APB1ENR
#define PWM5_APBEN __helper__(RCC_APB1ENR_TIM,PWM5_TIMER, EN)
#define PWM5_CLOCK HAL_RCC_GetPCLK1Freq()
#endif
#define PWM5_TIMREG (__tim__(PWM5_TIMER))
#ifndef PWM5_FREQ
#define PWM5_FREQ 1000
#endif
#if (PWM5_CHANNEL & 0x01)
#define PWM5_MODE 0x0064
#else
#define PWM5_MODE 0x6400
#endif
#if (PWM5_CHANNEL > 2)
#define PWM5_CCMREG CCMR2
#else
#define PWM5_CCMREG CCMR1
#endif
#if (PWM5_TIMER==1) || (PWM5_TIMER>=15 & PWM5_TIMER<=17)
#define PWM5_ENOUTPUT {PWM5_TIMREG->BDTR |= (1 << 15);}
#else
#define PWM5_ENOUTPUT {}
#endif
#if (PWM5_TIMER==1)
#define PWM5_AF 0x02
#elif (PWM5_PORT==A)
#if (PWM5_TIMER==3)
#define PWM5_AF 0x01
#elif (PWM5_TIMER==14)
#define PWM5_AF 0x04
#elif (PWM5_TIMER==15)
#define PWM5_AF 0x00
#elif (PWM5_TIMER==16) || (PWM5_TIMER==17)
#define PWM5_AF 0x05
#endif
#elif (PWM5_PORT==B)
#if (PWM5_TIMER==14)
#define PWM5_AF 0x00
#elif (PWM5_TIMER==15)
#define PWM5_AF 0x01
#elif (PWM5_TIMER==16) || (PWM5_TIMER==17)
#define PWM5_AF 0x02
#endif
#elif (PWM5_PORT==C)
#if (PWM5_TIMER==3)
#define PWM5_AF 0x00
#endif
#endif
#define PWM5_CCR __ccr__(PWM5_CHANNEL)
#define DIO30_TIMER PWM5_TIMER
#define DIO30_CHANNEL PWM5_CHANNEL
#define DIO30_ENREG PWM5_ENREG
#define DIO30_APBEN PWM5_APBEN
#define DIO30_TIMREG PWM5_TIMREG
#define DIO30_APBEN PWM5_APBEN
#define DIO30_FREQ PWM5_FREQ
#define DIO30_MODE PWM5_MODE
#define DIO30_CCMREG PWM5_CCMREG
#define DIO30_CCR PWM5_CCR
#define DIO30_ENOUTPUT PWM5_ENOUTPUT
#define DIO30_AF PWM5_AF
#define DIO30_CLOCK PWM5_CLOCK
#endif
#if (defined(PWM6_CHANNEL) && defined(PWM6_TIMER) && defined(PWM6))
#if (PWM6_TIMER==1 || (PWM6_TIMER>=15 & PWM6_TIMER<=17))
#define PWM6_ENREG RCC->APB2ENR
#define PWM6_APBEN __helper__(RCC_APB2ENR_TIM,PWM6_TIMER, EN)
#define PWM6_CLOCK HAL_RCC_GetPCLK1Freq()
#else
#define PWM6_ENREG RCC->APB1ENR
#define PWM6_APBEN __helper__(RCC_APB1ENR_TIM,PWM6_TIMER, EN)
#define PWM6_CLOCK HAL_RCC_GetPCLK1Freq()
#endif
#define PWM6_TIMREG (__tim__(PWM6_TIMER))
#ifndef PWM6_FREQ
#define PWM6_FREQ 1000
#endif
#if (PWM6_CHANNEL & 0x01)
#define PWM6_MODE 0x0064
#else
#define PWM6_MODE 0x6400
#endif
#if (PWM6_CHANNEL > 2)
#define PWM6_CCMREG CCMR2
#else
#define PWM6_CCMREG CCMR1
#endif
#if (PWM6_TIMER==1) || (PWM6_TIMER>=15 & PWM6_TIMER<=17)
#define PWM6_ENOUTPUT {PWM6_TIMREG->BDTR |= (1 << 15);}
#else
#define PWM6_ENOUTPUT {}
#endif
#if (PWM6_TIMER==1)
#define PWM6_AF 0x02
#elif (PWM6_PORT==A)
#if (PWM6_TIMER==3)
#define PWM6_AF 0x01
#elif (PWM6_TIMER==14)
#define PWM6_AF 0x04
#elif (PWM6_TIMER==15)
#define PWM6_AF 0x00
#elif (PWM6_TIMER==16) || (PWM6_TIMER==17)
#define PWM6_AF 0x05
#endif
#elif (PWM6_PORT==B)
#if (PWM6_TIMER==14)
#define PWM6_AF 0x00
#elif (PWM6_TIMER==15)
#define PWM6_AF 0x01
#elif (PWM6_TIMER==16) || (PWM6_TIMER==17)
#define PWM6_AF 0x02
#endif
#elif (PWM6_PORT==C)
#if (PWM6_TIMER==3)
#define PWM6_AF 0x00
#endif
#endif
#define PWM6_CCR __ccr__(PWM6_CHANNEL)
#define DIO31_TIMER PWM6_TIMER
#define DIO31_CHANNEL PWM6_CHANNEL
#define DIO31_ENREG PWM6_ENREG
#define DIO31_APBEN PWM6_APBEN
#define DIO31_TIMREG PWM6_TIMREG
#define DIO31_APBEN PWM6_APBEN
#define DIO31_FREQ PWM6_FREQ
#define DIO31_MODE PWM6_MODE
#define DIO31_CCMREG PWM6_CCMREG
#define DIO31_CCR PWM6_CCR
#define DIO31_ENOUTPUT PWM6_ENOUTPUT
#define DIO31_AF PWM6_AF
#define DIO31_CLOCK PWM6_CLOCK
#endif
#if (defined(PWM7_CHANNEL) && defined(PWM7_TIMER) && defined(PWM7))
#if (PWM7_TIMER==1 || (PWM7_TIMER>=15 & PWM7_TIMER<=17))
#define PWM7_ENREG RCC->APB2ENR
#define PWM7_APBEN __helper__(RCC_APB2ENR_TIM,PWM7_TIMER, EN)
#define PWM7_CLOCK HAL_RCC_GetPCLK1Freq()
#else
#define PWM7_ENREG RCC->APB1ENR
#define PWM7_APBEN __helper__(RCC_APB1ENR_TIM,PWM7_TIMER, EN)
#define PWM7_CLOCK HAL_RCC_GetPCLK1Freq()
#endif
#define PWM7_TIMREG (__tim__(PWM7_TIMER))
#ifndef PWM7_FREQ
#define PWM7_FREQ 1000
#endif
#if (PWM7_CHANNEL & 0x01)
#define PWM7_MODE 0x0064
#else
#define PWM7_MODE 0x6400
#endif
#if (PWM7_CHANNEL > 2)
#define PWM7_CCMREG CCMR2
#else
#define PWM7_CCMREG CCMR1
#endif
#if (PWM7_TIMER==1) || (PWM7_TIMER>=15 & PWM7_TIMER<=17)
#define PWM7_ENOUTPUT {PWM7_TIMREG->BDTR |= (1 << 15);}
#else
#define PWM7_ENOUTPUT {}
#endif
#if (PWM7_TIMER==1)
#define PWM7_AF 0x02
#elif (PWM7_PORT==A)
#if (PWM7_TIMER==3)
#define PWM7_AF 0x01
#elif (PWM7_TIMER==14)
#define PWM7_AF 0x04
#elif (PWM7_TIMER==15)
#define PWM7_AF 0x00
#elif (PWM7_TIMER==16) || (PWM7_TIMER==17)
#define PWM7_AF 0x05
#endif
#elif (PWM7_PORT==B)
#if (PWM7_TIMER==14)
#define PWM7_AF 0x00
#elif (PWM7_TIMER==15)
#define PWM7_AF 0x01
#elif (PWM7_TIMER==16) || (PWM7_TIMER==17)
#define PWM7_AF 0x02
#endif
#elif (PWM7_PORT==C)
#if (PWM7_TIMER==3)
#define PWM7_AF 0x00
#endif
#endif
#define PWM7_CCR __ccr__(PWM7_CHANNEL)
#define DIO32_TIMER PWM7_TIMER
#define DIO32_CHANNEL PWM7_CHANNEL
#define DIO32_ENREG PWM7_ENREG
#define DIO32_APBEN PWM7_APBEN
#define DIO32_TIMREG PWM7_TIMREG
#define DIO32_APBEN PWM7_APBEN
#define DIO32_FREQ PWM7_FREQ
#define DIO32_MODE PWM7_MODE
#define DIO32_CCMREG PWM7_CCMREG
#define DIO32_CCR PWM7_CCR
#define DIO32_ENOUTPUT PWM7_ENOUTPUT
#define DIO32_AF PWM7_AF
#define DIO32_CLOCK PWM7_CLOCK
#endif
#if (defined(PWM8_CHANNEL) && defined(PWM8_TIMER) && defined(PWM8))
#if (PWM8_TIMER==1 || (PWM8_TIMER>=15 & PWM8_TIMER<=17))
#define PWM8_ENREG RCC->APB2ENR
#define PWM8_APBEN __helper__(RCC_APB2ENR_TIM,PWM8_TIMER, EN)
#define PWM8_CLOCK HAL_RCC_GetPCLK1Freq()
#else
#define PWM8_ENREG RCC->APB1ENR
#define PWM8_APBEN __helper__(RCC_APB1ENR_TIM,PWM8_TIMER, EN)
#define PWM8_CLOCK HAL_RCC_GetPCLK1Freq()
#endif
#define PWM8_TIMREG (__tim__(PWM8_TIMER))
#ifndef PWM8_FREQ
#define PWM8_FREQ 1000
#endif
#if (PWM8_CHANNEL & 0x01)
#define PWM8_MODE 0x0064
#else
#define PWM8_MODE 0x6400
#endif
#if (PWM8_CHANNEL > 2)
#define PWM8_CCMREG CCMR2
#else
#define PWM8_CCMREG CCMR1
#endif
#if (PWM8_TIMER==1) || (PWM8_TIMER>=15 & PWM8_TIMER<=17)
#define PWM8_ENOUTPUT {PWM8_TIMREG->BDTR |= (1 << 15);}
#else
#define PWM8_ENOUTPUT {}
#endif
#if (PWM8_TIMER==1)
#define PWM8_AF 0x02
#elif (PWM8_PORT==A)
#if (PWM8_TIMER==3)
#define PWM8_AF 0x01
#elif (PWM8_TIMER==14)
#define PWM8_AF 0x04
#elif (PWM8_TIMER==15)
#define PWM8_AF 0x00
#elif (PWM8_TIMER==16) || (PWM8_TIMER==17)
#define PWM8_AF 0x05
#endif
#elif (PWM8_PORT==B)
#if (PWM8_TIMER==14)
#define PWM8_AF 0x00
#elif (PWM8_TIMER==15)
#define PWM8_AF 0x01
#elif (PWM8_TIMER==16) || (PWM8_TIMER==17)
#define PWM8_AF 0x02
#endif
#elif (PWM8_PORT==C)
#if (PWM8_TIMER==3)
#define PWM8_AF 0x00
#endif
#endif
#define PWM8_CCR __ccr__(PWM8_CHANNEL)
#define DIO33_TIMER PWM8_TIMER
#define DIO33_CHANNEL PWM8_CHANNEL
#define DIO33_ENREG PWM8_ENREG
#define DIO33_APBEN PWM8_APBEN
#define DIO33_TIMREG PWM8_TIMREG
#define DIO33_APBEN PWM8_APBEN
#define DIO33_FREQ PWM8_FREQ
#define DIO33_MODE PWM8_MODE
#define DIO33_CCMREG PWM8_CCMREG
#define DIO33_CCR PWM8_CCR
#define DIO33_ENOUTPUT PWM8_ENOUTPUT
#define DIO33_AF PWM8_AF
#define DIO33_CLOCK PWM8_CLOCK
#endif
#if (defined(PWM9_CHANNEL) && defined(PWM9_TIMER) && defined(PWM9))
#if (PWM9_TIMER==1 || (PWM9_TIMER>=15 & PWM9_TIMER<=17))
#define PWM9_ENREG RCC->APB2ENR
#define PWM9_APBEN __helper__(RCC_APB2ENR_TIM,PWM9_TIMER, EN)
#define PWM9_CLOCK HAL_RCC_GetPCLK1Freq()
#else
#define PWM9_ENREG RCC->APB1ENR
#define PWM9_APBEN __helper__(RCC_APB1ENR_TIM,PWM9_TIMER, EN)
#define PWM9_CLOCK HAL_RCC_GetPCLK1Freq()
#endif
#define PWM9_TIMREG (__tim__(PWM9_TIMER))
#ifndef PWM9_FREQ
#define PWM9_FREQ 1000
#endif
#if (PWM9_CHANNEL & 0x01)
#define PWM9_MODE 0x0064
#else
#define PWM9_MODE 0x6400
#endif
#if (PWM9_CHANNEL > 2)
#define PWM9_CCMREG CCMR2
#else
#define PWM9_CCMREG CCMR1
#endif
#if (PWM9_TIMER==1) || (PWM9_TIMER>=15 & PWM9_TIMER<=17)
#define PWM9_ENOUTPUT {PWM9_TIMREG->BDTR |= (1 << 15);}
#else
#define PWM9_ENOUTPUT {}
#endif
#if (PWM9_TIMER==1)
#define PWM9_AF 0x02
#elif (PWM9_PORT==A)
#if (PWM9_TIMER==3)
#define PWM9_AF 0x01
#elif (PWM9_TIMER==14)
#define PWM9_AF 0x04
#elif (PWM9_TIMER==15)
#define PWM9_AF 0x00
#elif (PWM9_TIMER==16) || (PWM9_TIMER==17)
#define PWM9_AF 0x05
#endif
#elif (PWM9_PORT==B)
#if (PWM9_TIMER==14)
#define PWM9_AF 0x00
#elif (PWM9_TIMER==15)
#define PWM9_AF 0x01
#elif (PWM9_TIMER==16) || (PWM9_TIMER==17)
#define PWM9_AF 0x02
#endif
#elif (PWM9_PORT==C)
#if (PWM9_TIMER==3)
#define PWM9_AF 0x00
#endif
#endif
#define PWM9_CCR __ccr__(PWM9_CHANNEL)
#define DIO34_TIMER PWM9_TIMER
#define DIO34_CHANNEL PWM9_CHANNEL
#define DIO34_ENREG PWM9_ENREG
#define DIO34_APBEN PWM9_APBEN
#define DIO34_TIMREG PWM9_TIMREG
#define DIO34_APBEN PWM9_APBEN
#define DIO34_FREQ PWM9_FREQ
#define DIO34_MODE PWM9_MODE
#define DIO34_CCMREG PWM9_CCMREG
#define DIO34_CCR PWM9_CCR
#define DIO34_ENOUTPUT PWM9_ENOUTPUT
#define DIO34_AF PWM9_AF
#define DIO34_CLOCK PWM9_CLOCK
#endif
#if (defined(PWM10_CHANNEL) && defined(PWM10_TIMER) && defined(PWM10))
#if (PWM10_TIMER==1 || (PWM10_TIMER>=15 & PWM10_TIMER<=17))
#define PWM10_ENREG RCC->APB2ENR
#define PWM10_APBEN __helper__(RCC_APB2ENR_TIM,PWM10_TIMER, EN)
#define PWM10_CLOCK HAL_RCC_GetPCLK1Freq()
#else
#define PWM10_ENREG RCC->APB1ENR
#define PWM10_APBEN __helper__(RCC_APB1ENR_TIM,PWM10_TIMER, EN)
#define PWM10_CLOCK HAL_RCC_GetPCLK1Freq()
#endif
#define PWM10_TIMREG (__tim__(PWM10_TIMER))
#ifndef PWM10_FREQ
#define PWM10_FREQ 1000
#endif
#if (PWM10_CHANNEL & 0x01)
#define PWM10_MODE 0x0064
#else
#define PWM10_MODE 0x6400
#endif
#if (PWM10_CHANNEL > 2)
#define PWM10_CCMREG CCMR2
#else
#define PWM10_CCMREG CCMR1
#endif
#if (PWM10_TIMER==1) || (PWM10_TIMER>=15 & PWM10_TIMER<=17)
#define PWM10_ENOUTPUT {PWM10_TIMREG->BDTR |= (1 << 15);}
#else
#define PWM10_ENOUTPUT {}
#endif
#if (PWM10_TIMER==1)
#define PWM10_AF 0x02
#elif (PWM10_PORT==A)
#if (PWM10_TIMER==3)
#define PWM10_AF 0x01
#elif (PWM10_TIMER==14)
#define PWM10_AF 0x04
#elif (PWM10_TIMER==15)
#define PWM10_AF 0x00
#elif (PWM10_TIMER==16) || (PWM10_TIMER==17)
#define PWM10_AF 0x05
#endif
#elif (PWM10_PORT==B)
#if (PWM10_TIMER==14)
#define PWM10_AF 0x00
#elif (PWM10_TIMER==15)
#define PWM10_AF 0x01
#elif (PWM10_TIMER==16) || (PWM10_TIMER==17)
#define PWM10_AF 0x02
#endif
#elif (PWM10_PORT==C)
#if (PWM10_TIMER==3)
#define PWM10_AF 0x00
#endif
#endif
#define PWM10_CCR __ccr__(PWM10_CHANNEL)
#define DIO35_TIMER PWM10_TIMER
#define DIO35_CHANNEL PWM10_CHANNEL
#define DIO35_ENREG PWM10_ENREG
#define DIO35_APBEN PWM10_APBEN
#define DIO35_TIMREG PWM10_TIMREG
#define DIO35_APBEN PWM10_APBEN
#define DIO35_FREQ PWM10_FREQ
#define DIO35_MODE PWM10_MODE
#define DIO35_CCMREG PWM10_CCMREG
#define DIO35_CCR PWM10_CCR
#define DIO35_ENOUTPUT PWM10_ENOUTPUT
#define DIO35_AF PWM10_AF
#define DIO35_CLOCK PWM10_CLOCK
#endif
#if (defined(PWM11_CHANNEL) && defined(PWM11_TIMER) && defined(PWM11))
#if (PWM11_TIMER==1 || (PWM11_TIMER>=15 & PWM11_TIMER<=17))
#define PWM11_ENREG RCC->APB2ENR
#define PWM11_APBEN __helper__(RCC_APB2ENR_TIM,PWM11_TIMER, EN)
#define PWM11_CLOCK HAL_RCC_GetPCLK1Freq()
#else
#define PWM11_ENREG RCC->APB1ENR
#define PWM11_APBEN __helper__(RCC_APB1ENR_TIM,PWM11_TIMER, EN)
#define PWM11_CLOCK HAL_RCC_GetPCLK1Freq()
#endif
#define PWM11_TIMREG (__tim__(PWM11_TIMER))
#ifndef PWM11_FREQ
#define PWM11_FREQ 1000
#endif
#if (PWM11_CHANNEL & 0x01)
#define PWM11_MODE 0x0064
#else
#define PWM11_MODE 0x6400
#endif
#if (PWM11_CHANNEL > 2)
#define PWM11_CCMREG CCMR2
#else
#define PWM11_CCMREG CCMR1
#endif
#if (PWM11_TIMER==1) || (PWM11_TIMER>=15 & PWM11_TIMER<=17)
#define PWM11_ENOUTPUT {PWM11_TIMREG->BDTR |= (1 << 15);}
#else
#define PWM11_ENOUTPUT {}
#endif
#if (PWM11_TIMER==1)
#define PWM11_AF 0x02
#elif (PWM11_PORT==A)
#if (PWM11_TIMER==3)
#define PWM11_AF 0x01
#elif (PWM11_TIMER==14)
#define PWM11_AF 0x04
#elif (PWM11_TIMER==15)
#define PWM11_AF 0x00
#elif (PWM11_TIMER==16) || (PWM11_TIMER==17)
#define PWM11_AF 0x05
#endif
#elif (PWM11_PORT==B)
#if (PWM11_TIMER==14)
#define PWM11_AF 0x00
#elif (PWM11_TIMER==15)
#define PWM11_AF 0x01
#elif (PWM11_TIMER==16) || (PWM11_TIMER==17)
#define PWM11_AF 0x02
#endif
#elif (PWM11_PORT==C)
#if (PWM11_TIMER==3)
#define PWM11_AF 0x00
#endif
#endif
#define PWM11_CCR __ccr__(PWM11_CHANNEL)
#define DIO36_TIMER PWM11_TIMER
#define DIO36_CHANNEL PWM11_CHANNEL
#define DIO36_ENREG PWM11_ENREG
#define DIO36_APBEN PWM11_APBEN
#define DIO36_TIMREG PWM11_TIMREG
#define DIO36_APBEN PWM11_APBEN
#define DIO36_FREQ PWM11_FREQ
#define DIO36_MODE PWM11_MODE
#define DIO36_CCMREG PWM11_CCMREG
#define DIO36_CCR PWM11_CCR
#define DIO36_ENOUTPUT PWM11_ENOUTPUT
#define DIO36_AF PWM11_AF
#define DIO36_CLOCK PWM11_CLOCK
#endif
#if (defined(PWM12_CHANNEL) && defined(PWM12_TIMER) && defined(PWM12))
#if (PWM12_TIMER==1 || (PWM12_TIMER>=15 & PWM12_TIMER<=17))
#define PWM12_ENREG RCC->APB2ENR
#define PWM12_APBEN __helper__(RCC_APB2ENR_TIM,PWM12_TIMER, EN)
#define PWM12_CLOCK HAL_RCC_GetPCLK1Freq()
#else
#define PWM12_ENREG RCC->APB1ENR
#define PWM12_APBEN __helper__(RCC_APB1ENR_TIM,PWM12_TIMER, EN)
#define PWM12_CLOCK HAL_RCC_GetPCLK1Freq()
#endif
#define PWM12_TIMREG (__tim__(PWM12_TIMER))
#ifndef PWM12_FREQ
#define PWM12_FREQ 1000
#endif
#if (PWM12_CHANNEL & 0x01)
#define PWM12_MODE 0x0064
#else
#define PWM12_MODE 0x6400
#endif
#if (PWM12_CHANNEL > 2)
#define PWM12_CCMREG CCMR2
#else
#define PWM12_CCMREG CCMR1
#endif
#if (PWM12_TIMER==1) || (PWM12_TIMER>=15 & PWM12_TIMER<=17)
#define PWM12_ENOUTPUT {PWM12_TIMREG->BDTR |= (1 << 15);}
#else
#define PWM12_ENOUTPUT {}
#endif
#if (PWM12_TIMER==1)
#define PWM12_AF 0x02
#elif (PWM12_PORT==A)
#if (PWM12_TIMER==3)
#define PWM12_AF 0x01
#elif (PWM12_TIMER==14)
#define PWM12_AF 0x04
#elif (PWM12_TIMER==15)
#define PWM12_AF 0x00
#elif (PWM12_TIMER==16) || (PWM12_TIMER==17)
#define PWM12_AF 0x05
#endif
#elif (PWM12_PORT==B)
#if (PWM12_TIMER==14)
#define PWM12_AF 0x00
#elif (PWM12_TIMER==15)
#define PWM12_AF 0x01
#elif (PWM12_TIMER==16) || (PWM12_TIMER==17)
#define PWM12_AF 0x02
#endif
#elif (PWM12_PORT==C)
#if (PWM12_TIMER==3)
#define PWM12_AF 0x00
#endif
#endif
#define PWM12_CCR __ccr__(PWM12_CHANNEL)
#define DIO37_TIMER PWM12_TIMER
#define DIO37_CHANNEL PWM12_CHANNEL
#define DIO37_ENREG PWM12_ENREG
#define DIO37_APBEN PWM12_APBEN
#define DIO37_TIMREG PWM12_TIMREG
#define DIO37_APBEN PWM12_APBEN
#define DIO37_FREQ PWM12_FREQ
#define DIO37_MODE PWM12_MODE
#define DIO37_CCMREG PWM12_CCMREG
#define DIO37_CCR PWM12_CCR
#define DIO37_ENOUTPUT PWM12_ENOUTPUT
#define DIO37_AF PWM12_AF
#define DIO37_CLOCK PWM12_CLOCK
#endif
#if (defined(PWM13_CHANNEL) && defined(PWM13_TIMER) && defined(PWM13))
#if (PWM13_TIMER==1 || (PWM13_TIMER>=15 & PWM13_TIMER<=17))
#define PWM13_ENREG RCC->APB2ENR
#define PWM13_APBEN __helper__(RCC_APB2ENR_TIM,PWM13_TIMER, EN)
#define PWM13_CLOCK HAL_RCC_GetPCLK1Freq()
#else
#define PWM13_ENREG RCC->APB1ENR
#define PWM13_APBEN __helper__(RCC_APB1ENR_TIM,PWM13_TIMER, EN)
#define PWM13_CLOCK HAL_RCC_GetPCLK1Freq()
#endif
#define PWM13_TIMREG (__tim__(PWM13_TIMER))
#ifndef PWM13_FREQ
#define PWM13_FREQ 1000
#endif
#if (PWM13_CHANNEL & 0x01)
#define PWM13_MODE 0x0064
#else
#define PWM13_MODE 0x6400
#endif
#if (PWM13_CHANNEL > 2)
#define PWM13_CCMREG CCMR2
#else
#define PWM13_CCMREG CCMR1
#endif
#if (PWM13_TIMER==1) || (PWM13_TIMER>=15 & PWM13_TIMER<=17)
#define PWM13_ENOUTPUT {PWM13_TIMREG->BDTR |= (1 << 15);}
#else
#define PWM13_ENOUTPUT {}
#endif
#if (PWM13_TIMER==1)
#define PWM13_AF 0x02
#elif (PWM13_PORT==A)
#if (PWM13_TIMER==3)
#define PWM13_AF 0x01
#elif (PWM13_TIMER==14)
#define PWM13_AF 0x04
#elif (PWM13_TIMER==15)
#define PWM13_AF 0x00
#elif (PWM13_TIMER==16) || (PWM13_TIMER==17)
#define PWM13_AF 0x05
#endif
#elif (PWM13_PORT==B)
#if (PWM13_TIMER==14)
#define PWM13_AF 0x00
#elif (PWM13_TIMER==15)
#define PWM13_AF 0x01
#elif (PWM13_TIMER==16) || (PWM13_TIMER==17)
#define PWM13_AF 0x02
#endif
#elif (PWM13_PORT==C)
#if (PWM13_TIMER==3)
#define PWM13_AF 0x00
#endif
#endif
#define PWM13_CCR __ccr__(PWM13_CHANNEL)
#define DIO38_TIMER PWM13_TIMER
#define DIO38_CHANNEL PWM13_CHANNEL
#define DIO38_ENREG PWM13_ENREG
#define DIO38_APBEN PWM13_APBEN
#define DIO38_TIMREG PWM13_TIMREG
#define DIO38_APBEN PWM13_APBEN
#define DIO38_FREQ PWM13_FREQ
#define DIO38_MODE PWM13_MODE
#define DIO38_CCMREG PWM13_CCMREG
#define DIO38_CCR PWM13_CCR
#define DIO38_ENOUTPUT PWM13_ENOUTPUT
#define DIO38_AF PWM13_AF
#define DIO38_CLOCK PWM13_CLOCK
#endif
#if (defined(PWM14_CHANNEL) && defined(PWM14_TIMER) && defined(PWM14))
#if (PWM14_TIMER==1 || (PWM14_TIMER>=15 & PWM14_TIMER<=17))
#define PWM14_ENREG RCC->APB2ENR
#define PWM14_APBEN __helper__(RCC_APB2ENR_TIM,PWM14_TIMER, EN)
#define PWM14_CLOCK HAL_RCC_GetPCLK1Freq()
#else
#define PWM14_ENREG RCC->APB1ENR
#define PWM14_APBEN __helper__(RCC_APB1ENR_TIM,PWM14_TIMER, EN)
#define PWM14_CLOCK HAL_RCC_GetPCLK1Freq()
#endif
#define PWM14_TIMREG (__tim__(PWM14_TIMER))
#ifndef PWM14_FREQ
#define PWM14_FREQ 1000
#endif
#if (PWM14_CHANNEL & 0x01)
#define PWM14_MODE 0x0064
#else
#define PWM14_MODE 0x6400
#endif
#if (PWM14_CHANNEL > 2)
#define PWM14_CCMREG CCMR2
#else
#define PWM14_CCMREG CCMR1
#endif
#if (PWM14_TIMER==1) || (PWM14_TIMER>=15 & PWM14_TIMER<=17)
#define PWM14_ENOUTPUT {PWM14_TIMREG->BDTR |= (1 << 15);}
#else
#define PWM14_ENOUTPUT {}
#endif
#if (PWM14_TIMER==1)
#define PWM14_AF 0x02
#elif (PWM14_PORT==A)
#if (PWM14_TIMER==3)
#define PWM14_AF 0x01
#elif (PWM14_TIMER==14)
#define PWM14_AF 0x04
#elif (PWM14_TIMER==15)
#define PWM14_AF 0x00
#elif (PWM14_TIMER==16) || (PWM14_TIMER==17)
#define PWM14_AF 0x05
#endif
#elif (PWM14_PORT==B)
#if (PWM14_TIMER==14)
#define PWM14_AF 0x00
#elif (PWM14_TIMER==15)
#define PWM14_AF 0x01
#elif (PWM14_TIMER==16) || (PWM14_TIMER==17)
#define PWM14_AF 0x02
#endif
#elif (PWM14_PORT==C)
#if (PWM14_TIMER==3)
#define PWM14_AF 0x00
#endif
#endif
#define PWM14_CCR __ccr__(PWM14_CHANNEL)
#define DIO39_TIMER PWM14_TIMER
#define DIO39_CHANNEL PWM14_CHANNEL
#define DIO39_ENREG PWM14_ENREG
#define DIO39_APBEN PWM14_APBEN
#define DIO39_TIMREG PWM14_TIMREG
#define DIO39_APBEN PWM14_APBEN
#define DIO39_FREQ PWM14_FREQ
#define DIO39_MODE PWM14_MODE
#define DIO39_CCMREG PWM14_CCMREG
#define DIO39_CCR PWM14_CCR
#define DIO39_ENOUTPUT PWM14_ENOUTPUT
#define DIO39_AF PWM14_AF
#define DIO39_CLOCK PWM14_CLOCK
#endif
#if (defined(PWM15_CHANNEL) && defined(PWM15_TIMER) && defined(PWM15))
#if (PWM15_TIMER==1 || (PWM15_TIMER>=15 & PWM15_TIMER<=17))
#define PWM15_ENREG RCC->APB2ENR
#define PWM15_APBEN __helper__(RCC_APB2ENR_TIM,PWM15_TIMER, EN)
#define PWM15_CLOCK HAL_RCC_GetPCLK1Freq()
#else
#define PWM15_ENREG RCC->APB1ENR
#define PWM15_APBEN __helper__(RCC_APB1ENR_TIM,PWM15_TIMER, EN)
#define PWM15_CLOCK HAL_RCC_GetPCLK1Freq()
#endif
#define PWM15_TIMREG (__tim__(PWM15_TIMER))
#ifndef PWM15_FREQ
#define PWM15_FREQ 1000
#endif
#if (PWM15_CHANNEL & 0x01)
#define PWM15_MODE 0x0064
#else
#define PWM15_MODE 0x6400
#endif
#if (PWM15_CHANNEL > 2)
#define PWM15_CCMREG CCMR2
#else
#define PWM15_CCMREG CCMR1
#endif
#if (PWM15_TIMER==1) || (PWM15_TIMER>=15 & PWM15_TIMER<=17)
#define PWM15_ENOUTPUT {PWM15_TIMREG->BDTR |= (1 << 15);}
#else
#define PWM15_ENOUTPUT {}
#endif
#if (PWM15_TIMER==1)
#define PWM15_AF 0x02
#elif (PWM15_PORT==A)
#if (PWM15_TIMER==3)
#define PWM15_AF 0x01
#elif (PWM15_TIMER==14)
#define PWM15_AF 0x04
#elif (PWM15_TIMER==15)
#define PWM15_AF 0x00
#elif (PWM15_TIMER==16) || (PWM15_TIMER==17)
#define PWM15_AF 0x05
#endif
#elif (PWM15_PORT==B)
#if (PWM15_TIMER==14)
#define PWM15_AF 0x00
#elif (PWM15_TIMER==15)
#define PWM15_AF 0x01
#elif (PWM15_TIMER==16) || (PWM15_TIMER==17)
#define PWM15_AF 0x02
#endif
#elif (PWM15_PORT==C)
#if (PWM15_TIMER==3)
#define PWM15_AF 0x00
#endif
#endif
#define PWM15_CCR __ccr__(PWM15_CHANNEL)
#define DIO40_TIMER PWM15_TIMER
#define DIO40_CHANNEL PWM15_CHANNEL
#define DIO40_ENREG PWM15_ENREG
#define DIO40_APBEN PWM15_APBEN
#define DIO40_TIMREG PWM15_TIMREG
#define DIO40_APBEN PWM15_APBEN
#define DIO40_FREQ PWM15_FREQ
#define DIO40_MODE PWM15_MODE
#define DIO40_CCMREG PWM15_CCMREG
#define DIO40_CCR PWM15_CCR
#define DIO40_ENOUTPUT PWM15_ENOUTPUT
#define DIO40_AF PWM15_AF
#define DIO40_CLOCK PWM15_CLOCK
#endif


/**********************************************
 *	Analog pins
 **********************************************/
#ifdef ANALOG0
#ifndef ANALOG0_CHANNEL
#define ANALOG0_CHANNEL -1
#endif
#define DIO114_CHANNEL ANALOG0_CHANNEL
#endif
#ifdef ANALOG1
#ifndef ANALOG1_CHANNEL
#define ANALOG1_CHANNEL -1
#endif
#define DIO115_CHANNEL ANALOG1_CHANNEL
#endif
#ifdef ANALOG2
#ifndef ANALOG2_CHANNEL
#define ANALOG2_CHANNEL -1
#endif
#define DIO116_CHANNEL ANALOG2_CHANNEL
#endif
#ifdef ANALOG3
#ifndef ANALOG3_CHANNEL
#define ANALOG3_CHANNEL -1
#endif
#define DIO117_CHANNEL ANALOG3_CHANNEL
#endif
#ifdef ANALOG4
#ifndef ANALOG4_CHANNEL
#define ANALOG4_CHANNEL -1
#endif
#define DIO118_CHANNEL ANALOG4_CHANNEL
#endif
#ifdef ANALOG5
#ifndef ANALOG5_CHANNEL
#define ANALOG5_CHANNEL -1
#endif
#define DIO119_CHANNEL ANALOG5_CHANNEL
#endif
#ifdef ANALOG6
#ifndef ANALOG6_CHANNEL
#define ANALOG6_CHANNEL -1
#endif
#define DIO120_CHANNEL ANALOG6_CHANNEL
#endif
#ifdef ANALOG7
#ifndef ANALOG7_CHANNEL
#define ANALOG7_CHANNEL -1
#endif
#define DIO121_CHANNEL ANALOG7_CHANNEL
#endif
#ifdef ANALOG8
#ifndef ANALOG8_CHANNEL
#define ANALOG8_CHANNEL -1
#endif
#define DIO122_CHANNEL ANALOG8_CHANNEL
#endif
#ifdef ANALOG9
#ifndef ANALOG9_CHANNEL
#define ANALOG9_CHANNEL -1
#endif
#define DIO123_CHANNEL ANALOG9_CHANNEL
#endif
#ifdef ANALOG10
#ifndef ANALOG10_CHANNEL
#define ANALOG10_CHANNEL -1
#endif
#define DIO124_CHANNEL ANALOG10_CHANNEL
#endif
#ifdef ANALOG11
#ifndef ANALOG11_CHANNEL
#define ANALOG11_CHANNEL -1
#endif
#define DIO125_CHANNEL ANALOG11_CHANNEL
#endif
#ifdef ANALOG12
#ifndef ANALOG12_CHANNEL
#define ANALOG12_CHANNEL -1
#endif
#define DIO126_CHANNEL ANALOG12_CHANNEL
#endif
#ifdef ANALOG13
#ifndef ANALOG13_CHANNEL
#define ANALOG13_CHANNEL -1
#endif
#define DIO127_CHANNEL ANALOG13_CHANNEL
#endif
#ifdef ANALOG14
#ifndef ANALOG14_CHANNEL
#define ANALOG14_CHANNEL -1
#endif
#define DIO128_CHANNEL ANALOG14_CHANNEL
#endif
#ifdef ANALOG15
#ifndef ANALOG15_CHANNEL
#define ANALOG15_CHANNEL -1
#endif
#define DIO129_CHANNEL ANALOG15_CHANNEL
#endif

// COM registers
#ifdef MCU_HAS_UART
// this MCU does not work well with both TX and RX interrupt
// this forces the sync TX method to fix communication
#define COM_UART __usart__(UART_PORT)

#if (UART_PORT < 3)
#define COM_IRQ __helper__(USART, UART_PORT, _IRQn)
#define MCU_SERIAL_ISR __helper__(USART, UART_PORT, _IRQHandler)
#else
#define COM_IRQ USART3_8_IRQn
#define MCU_SERIAL_ISR USART3_8_IRQHandler
#endif

#define COM_OUTREG (COM_UART)->TDR
#define COM_INREG (COM_UART)->RDR

#if (UART_PORT == 1 || UART_PORT >= 6)
#define COM_APB APB2ENR
#define COM_APBEN __helper__(RCC_APB2ENR_USART, UART_PORT, EN)
#else
#define COM_APB APB1ENR
#define COM_APBEN __helper__(RCC_APB1ENR_USART, UART_PORT, EN)
#endif

#define UART_CLOCK HAL_RCC_GetPCLK1Freq()

#define TX_PIN __iopin__(TX_PORT, TX_BIT)
#define RX_PIN __iopin__(RX_PORT,RX_BIT)

#if (UART_PORT == 2) && (TX_PIN == STM32IO_A2)
#define UART_TX_AFIO 1
#endif
#if (UART_PORT == 2) && (RX_PIN == STM32IO_A3)
#define UART_RX_AFIO 1
#endif
#if (UART_PORT == 1) && (TX_PIN == STM32IO_A9)
#define UART_TX_AFIO 1
#endif
#if (UART_PORT == 1) && (RX_PIN == STM32IO_A10)
#define UART_RX_AFIO 1
#endif
#if (UART_PORT == 2) && (TX_PIN == STM32IO_A14)
#define UART_TX_AFIO 1
#endif
#if (UART_PORT == 2) && (RX_PIN == STM32IO_A15)
#define UART_RX_AFIO 1
#endif
#if (UART_PORT == 4) && (TX_PIN == STM32IO_A0)
#define UART_TX_AFIO 4
#endif
#if (UART_PORT == 4) && (RX_PIN == STM32IO_A1)
#define UART_RX_AFIO 4
#endif
#if (UART_PORT == 5) && (TX_PIN == STM32IO_A4)
#define UART_TX_AFIO 5
#endif
#if (UART_PORT == 5) && (RX_PIN == STM32IO_A5)
#define UART_RX_AFIO 5
#endif
#if (UART_PORT == 1) && (TX_PIN == STM32IO_B6)
#define UART_TX_AFIO 0
#endif
#if (UART_PORT == 1) && (RX_PIN == STM32IO_B7)
#define UART_RX_AFIO 0
#endif
#if (UART_PORT == 5) && (TX_PIN == STM32IO_B3)
#define UART_TX_AFIO 4
#endif
#if (UART_PORT == 5) && (RX_PIN == STM32IO_B4)
#define UART_RX_AFIO 4
#endif
#if (UART_PORT == 3) && (TX_PIN == STM32IO_B10)
#define UART_TX_AFIO 4
#endif
#if (UART_PORT == 3) && (RX_PIN == STM32IO_B11)
#define UART_RX_AFIO 4
#endif
#if (UART_PORT == 4) && (TX_PIN == STM32IO_C10)
#define UART_TX_AFIO 0
#endif
#if (UART_PORT == 4) && (RX_PIN == STM32IO_C11)
#define UART_RX_AFIO 0
#endif
#if (UART_PORT == 7) && (TX_PIN == STM32IO_C0)
#define UART_TX_AFIO 1
#endif
#if (UART_PORT == 7) && (RX_PIN == STM32IO_C1)
#define UART_RX_AFIO 1
#endif
#if (UART_PORT == 7) && (TX_PIN == STM32IO_C6)
#define UART_TX_AFIO 1
#endif
#if (UART_PORT == 7) && (RX_PIN == STM32IO_C7)
#define UART_RX_AFIO 1
#endif
#if (UART_PORT == 3) && (TX_PIN == STM32IO_C4)
#define UART_TX_AFIO 1
#endif
#if (UART_PORT == 3) && (RX_PIN == STM32IO_C5)
#define UART_RX_AFIO 1
#endif
#if (UART_PORT == 3) && (TX_PIN == STM32IO_C10)
#define UART_TX_AFIO 1
#endif
#if (UART_PORT == 3) && (RX_PIN == STM32IO_C11)
#define UART_RX_AFIO 1
#endif
#if (UART_PORT == 8) && (TX_PIN == STM32IO_C8)
#define UART_TX_AFIO 1
#endif
#if (UART_PORT == 8) && (RX_PIN == STM32IO_C9)
#define UART_RX_AFIO 1
#endif
#if (UART_PORT == 6) && (TX_PIN == STM32IO_C0)
#define UART_TX_AFIO 2
#endif
#if (UART_PORT == 6) && (RX_PIN == STM32IO_C1)
#define UART_RX_AFIO 2
#endif
#if (UART_PORT == 8) && (TX_PIN == STM32IO_C2)
#define UART_TX_AFIO 2
#endif
#if (UART_PORT == 8) && (RX_PIN == STM32IO_C3)
#define UART_RX_AFIO 2
#endif
#if (UART_PORT == 5) && (TX_PIN == STM32IO_C12)
#define UART_TX_AFIO 2
#endif
#if (UART_PORT == 5) && (RX_PIN == STM32IO_D2)
#define UART_RX_AFIO 2
#endif
#if (UART_PORT == 2) && (TX_PIN == STM32IO_D5)
#define UART_TX_AFIO 0
#endif
#if (UART_PORT == 2) && (TX_PIN == STM32IO_D6)
#define UART_TX_AFIO 0
#endif
#if (UART_PORT == 3) && (TX_PIN == STM32IO_D8)
#define UART_TX_AFIO 0
#endif
#if (UART_PORT == 3) && (RX_PIN == STM32IO_D9)
#define UART_RX_AFIO 0
#endif
#if (UART_PORT == 8) && (TX_PIN == STM32IO_D13)
#define UART_TX_AFIO 0
#endif
#if (UART_PORT == 8) && (RX_PIN == STM32IO_D14)
#define UART_RX_AFIO 0
#endif
#if (UART_PORT == 4) && (TX_PIN == STM32IO_E8)
#define UART_TX_AFIO 2
#endif
#if (UART_PORT == 4) && (RX_PIN == STM32IO_E9)
#define UART_RX_AFIO 2
#endif
#if (UART_PORT == 5) && (TX_PIN == STM32IO_E10)
#define UART_TX_AFIO 2
#endif
#if (UART_PORT == 5) && (RX_PIN == STM32IO_E11)
#define UART_RX_AFIO 2
#endif
#if (UART_PORT == 7) && (TX_PIN == STM32IO_F2)
#define UART_TX_AFIO 1
#endif
#if (UART_PORT == 7) && (RX_PIN == STM32IO_F3)
#define UART_RX_AFIO 1
#endif

#endif


#ifdef MCU_HAS_UART2
// this MCU does not work well with both TX and RX interrupt
// this forces the sync TX method to fix communication
#define COM2_UART2 __usart__(UART2_PORT)

#if (UART2_PORT < 3)
#define COM2_IRQ __helper__(USART, UART2_PORT, _IRQn)
#define MCU_SERIAL_ISR __helper__(USART, UART2_PORT, _IRQHandler)
#else
#define COM2_IRQ USART3_8_IRQn
#define MCU_SERIAL_ISR USART3_8_IRQHandler
#endif

#define COM2_OUTREG (COM2_UART2)->TDR
#define COM2_INREG (COM2_UART2)->RDR

#if (UART2_PORT == 1 || UART2_PORT >= 6)
#define COM2_APB APB2ENR
#define COM2_APBEN __helper__(RCC_APB2ENR_USART, UART2_PORT, EN)
#else
#define COM2_APB APB1ENR
#define COM2_APBEN __helper__(RCC_APB1ENR_USART, UART2_PORT, EN)
#endif

#define UART2_CLOCK HAL_RCC_GetPCLK1Freq()

#define TX2_PIN __iopin__(TX2_PORT, TX2_BIT)
#define RX2_PIN __iopin__(RX_PORT,RX_BIT)

#if (UART2_PORT == 2) && (TX2_PIN == STM32IO_A2)
#define UART2_TX_AFIO 1
#endif
#if (UART2_PORT == 2) && (RX2_PIN == STM32IO_A3)
#define UART2_RX_AFIO 1
#endif
#if (UART2_PORT == 1) && (TX2_PIN == STM32IO_A9)
#define UART2_TX_AFIO 1
#endif
#if (UART2_PORT == 1) && (RX2_PIN == STM32IO_A10)
#define UART2_RX_AFIO 1
#endif
#if (UART2_PORT == 2) && (TX2_PIN == STM32IO_A14)
#define UART2_TX_AFIO 1
#endif
#if (UART2_PORT == 2) && (RX2_PIN == STM32IO_A15)
#define UART2_RX_AFIO 1
#endif
#if (UART2_PORT == 4) && (TX2_PIN == STM32IO_A0)
#define UART2_TX_AFIO 4
#endif
#if (UART2_PORT == 4) && (RX2_PIN == STM32IO_A1)
#define UART2_RX_AFIO 4
#endif
#if (UART2_PORT == 5) && (TX2_PIN == STM32IO_A4)
#define UART2_TX_AFIO 5
#endif
#if (UART2_PORT == 5) && (RX2_PIN == STM32IO_A5)
#define UART2_RX_AFIO 5
#endif
#if (UART2_PORT == 1) && (TX2_PIN == STM32IO_B6)
#define UART2_TX_AFIO 0
#endif
#if (UART2_PORT == 1) && (RX2_PIN == STM32IO_B7)
#define UART2_RX_AFIO 0
#endif
#if (UART2_PORT == 5) && (TX2_PIN == STM32IO_B3)
#define UART2_TX_AFIO 4
#endif
#if (UART2_PORT == 5) && (RX2_PIN == STM32IO_B4)
#define UART2_RX_AFIO 4
#endif
#if (UART2_PORT == 3) && (TX2_PIN == STM32IO_B10)
#define UART2_TX_AFIO 4
#endif
#if (UART2_PORT == 3) && (RX2_PIN == STM32IO_B11)
#define UART2_RX_AFIO 4
#endif
#if (UART2_PORT == 4) && (TX2_PIN == STM32IO_C10)
#define UART2_TX_AFIO 0
#endif
#if (UART2_PORT == 4) && (RX2_PIN == STM32IO_C11)
#define UART2_RX_AFIO 0
#endif
#if (UART2_PORT == 7) && (TX2_PIN == STM32IO_C0)
#define UART2_TX_AFIO 1
#endif
#if (UART2_PORT == 7) && (RX2_PIN == STM32IO_C1)
#define UART2_RX_AFIO 1
#endif
#if (UART2_PORT == 7) && (TX2_PIN == STM32IO_C6)
#define UART2_TX_AFIO 1
#endif
#if (UART2_PORT == 7) && (RX2_PIN == STM32IO_C7)
#define UART2_RX_AFIO 1
#endif
#if (UART2_PORT == 3) && (TX2_PIN == STM32IO_C4)
#define UART2_TX_AFIO 1
#endif
#if (UART2_PORT == 3) && (RX2_PIN == STM32IO_C5)
#define UART2_RX_AFIO 1
#endif
#if (UART2_PORT == 3) && (TX2_PIN == STM32IO_C10)
#define UART2_TX_AFIO 1
#endif
#if (UART2_PORT == 3) && (RX2_PIN == STM32IO_C11)
#define UART2_RX_AFIO 1
#endif
#if (UART2_PORT == 8) && (TX2_PIN == STM32IO_C8)
#define UART2_TX_AFIO 1
#endif
#if (UART2_PORT == 8) && (RX2_PIN == STM32IO_C9)
#define UART2_RX_AFIO 1
#endif
#if (UART2_PORT == 6) && (TX2_PIN == STM32IO_C0)
#define UART2_TX_AFIO 2
#endif
#if (UART2_PORT == 6) && (RX2_PIN == STM32IO_C1)
#define UART2_RX_AFIO 2
#endif
#if (UART2_PORT == 8) && (TX2_PIN == STM32IO_C2)
#define UART2_TX_AFIO 2
#endif
#if (UART2_PORT == 8) && (RX2_PIN == STM32IO_C3)
#define UART2_RX_AFIO 2
#endif
#if (UART2_PORT == 5) && (TX2_PIN == STM32IO_C12)
#define UART2_TX_AFIO 2
#endif
#if (UART2_PORT == 5) && (RX2_PIN == STM32IO_D2)
#define UART2_RX_AFIO 2
#endif
#if (UART2_PORT == 2) && (TX2_PIN == STM32IO_D5)
#define UART2_TX_AFIO 0
#endif
#if (UART2_PORT == 2) && (TX2_PIN == STM32IO_D6)
#define UART2_TX_AFIO 0
#endif
#if (UART2_PORT == 3) && (TX2_PIN == STM32IO_D8)
#define UART2_TX_AFIO 0
#endif
#if (UART2_PORT == 3) && (RX2_PIN == STM32IO_D9)
#define UART2_RX_AFIO 0
#endif
#if (UART2_PORT == 8) && (TX2_PIN == STM32IO_D13)
#define UART2_TX_AFIO 0
#endif
#if (UART2_PORT == 8) && (RX2_PIN == STM32IO_D14)
#define UART2_RX_AFIO 0
#endif
#if (UART2_PORT == 4) && (TX2_PIN == STM32IO_E8)
#define UART2_TX_AFIO 2
#endif
#if (UART2_PORT == 4) && (RX2_PIN == STM32IO_E9)
#define UART2_RX_AFIO 2
#endif
#if (UART2_PORT == 5) && (TX2_PIN == STM32IO_E10)
#define UART2_TX_AFIO 2
#endif
#if (UART2_PORT == 5) && (RX2_PIN == STM32IO_E11)
#define UART2_RX_AFIO 2
#endif
#if (UART2_PORT == 7) && (TX2_PIN == STM32IO_F2)
#define UART2_TX_AFIO 1
#endif
#if (UART2_PORT == 7) && (RX2_PIN == STM32IO_F3)
#define UART2_RX_AFIO 1
#endif

#endif

#if (defined(SPI_CLK) && defined(SPI_SDO) && defined(SPI_SDI))
#define SPI_CLK_PIN __iopin__(SPI_CLK_PORT, SPI_CLK_BIT)
#define SPI_SDO_PIN __iopin__(SPI_SDO_PORT, SPI_SDO_BIT)
#define SPI_SDI_PIN __iopin__(SPI_SDI_PORT, SPI_SDI_BIT)
#ifdef SPI_CS
#define SPI_CS_PIN __iopin__(SPI_CS_PORT, SPI_CS_BIT)
#endif
#define MCU_HAS_SPI
#ifndef SPI_PORT
#define SPI_PORT 1
#endif
#ifndef SPI_MODE
#define SPI_MODE 0
#endif
#ifndef SPI_FREQ
#define SPI_FREQ 1000000UL
#endif
// remmaping and pin checking
#if (SPI_PORT == 4) && (SPI_SDO_PIN == STM32IO_A1)
#define SPI_SDO_AFIO 5
#endif
#if (SPI_PORT == 3) && (SPI_CLK_PIN == STM32IO_B12)
#define SPI_CLK_AFIO 7
#endif
#if (SPI_PORT == 1) && (SPI_CS_PIN == STM32IO_A4)
#define SPI_CS_AFIO 5
#endif
#if (SPI_PORT == 1) && (SPI_CLK_PIN == STM32IO_A5)
#define SPI_CLK_AFIO 5
#endif
#if (SPI_PORT == 1) && (SPI_SDI_PIN == STM32IO_A6)
#define SPI_SDI_AFIO 5
#endif
#if (SPI_PORT == 1) && (SPI_SDO_PIN == STM32IO_A7)
#define SPI_SDO_AFIO 5
#endif
#if (SPI_PORT == 1) && (SPI_CS_PIN == STM32IO_A15)
#define SPI_CS_AFIO 5
#endif
#if (SPI_PORT == 1) && (SPI_CLK_PIN == STM32IO_B3)
#define SPI_CLK_AFIO 5
#endif
#if (SPI_PORT == 1) && (SPI_SDI_PIN == STM32IO_B4)
#define SPI_SDI_AFIO 5
#endif
#if (SPI_PORT == 1) && (SPI_SDO_PIN == STM32IO_B5)
#define SPI_SDO_AFIO 5
#endif
#if (SPI_PORT == 2) && (SPI_CS_PIN == STM32IO_B9)
#define SPI_CS_AFIO 5
#endif
#if (SPI_PORT == 2) && (SPI_CLK_PIN == STM32IO_B10)
#define SPI_CLK_AFIO 5
#endif
#if (SPI_PORT == 2) && (SPI_CS_PIN == STM32IO_B12)
#define SPI_CS_AFIO 5
#endif
#if (SPI_PORT == 2) && (SPI_CLK_PIN == STM32IO_B13)
#define SPI_CLK_AFIO 5
#endif
#if (SPI_PORT == 2) && (SPI_SDI_PIN == STM32IO_B14)
#define SPI_SDI_AFIO 5
#endif
#if (SPI_PORT == 2) && (SPI_SDO_PIN == STM32IO_B15)
#define SPI_SDO_AFIO 5
#endif
#if (SPI_PORT == 2) && (SPI_SDI_PIN == STM32IO_C2)
#define SPI_SDI_AFIO 5
#endif
#if (SPI_PORT == 2) && (SPI_SDO_PIN == STM32IO_C3)
#define SPI_SDO_AFIO 5
#endif
#if (SPI_PORT == 2) && (SPI_CLK_PIN == STM32IO_C7)
#define SPI_CLK_AFIO 5
#endif
#if (SPI_PORT == 2) && (SPI_CLK_PIN == STM32IO_D3)
#define SPI_CLK_AFIO 5
#endif
#if (SPI_PORT == 3) && (SPI_SDO_PIN == STM32IO_D6)
#define SPI_SDO_AFIO 5
#endif
#if (SPI_PORT == 4) && (SPI_CLK_PIN == STM32IO_E3)
#define SPI_CLK_AFIO 5
#endif
#if (SPI_PORT == 4) && (SPI_CS_PIN == STM32IO_E5)
#define SPI_CS_AFIO 5
#endif
#if (SPI_PORT == 4) && (SPI_SDI_PIN == STM32IO_E6)
#define SPI_SDI_AFIO 5
#endif
#if (SPI_PORT == 4) && (SPI_SDO_PIN == STM32IO_E7)
#define SPI_SDO_AFIO 5
#endif
#if (SPI_PORT == 4) && (SPI_CS_PIN == STM32IO_E11)
#define SPI_CS_AFIO 5
#endif
#if (SPI_PORT == 4) && (SPI_CLK_PIN == STM32IO_E12)
#define SPI_CLK_AFIO 5
#endif
#if (SPI_PORT == 4) && (SPI_SDI_PIN == STM32IO_E13)
#define SPI_SDI_AFIO 5
#endif
#if (SPI_PORT == 4) && (SPI_SDO_PIN == STM32IO_E14)
#define SPI_SDO_AFIO 5
#endif
#if (SPI_PORT == 3) && (SPI_CS_PIN == STM32IO_A4)
#define SPI_CS_AFIO 6
#endif
#if (SPI_PORT == 5) && (SPI_SDO_PIN == STM32IO_A10)
#define SPI_SDO_AFIO 6
#endif
#if (SPI_PORT == 4) && (SPI_SDI_PIN == STM32IO_A11)
#define SPI_SDI_AFIO 6
#endif
#if (SPI_PORT == 5) && (SPI_SDI_PIN == STM32IO_A12)
#define SPI_SDI_AFIO 6
#endif
#if (SPI_PORT == 3) && (SPI_CS_PIN == STM32IO_A15)
#define SPI_CS_AFIO 6
#endif
#if (SPI_PORT == 5) && (SPI_CLK_PIN == STM32IO_B0)
#define SPI_CLK_AFIO 6
#endif
#if (SPI_PORT == 5) && (SPI_CD_PIN == STM32IO_B1)
#define SPI_CD_AFIO 6
#endif
#if (SPI_PORT == 3) && (SPI_CLK_PIN == STM32IO_B3)
#define SPI_CLK_AFIO 6
#endif
#if (SPI_PORT == 3) && (SPI_SDI_PIN == STM32IO_B4)
#define SPI_SDI_AFIO 6
#endif
#if (SPI_PORT == 3) && (SPI_SDO_PIN == STM32IO_B5)
#define SPI_SDO_AFIO 6
#endif
#if (SPI_PORT == 5) && (SPI_SDO_PIN == STM32IO_B8)
#define SPI_SDO_AFIO 6
#endif
#if (SPI_PORT == 4) && (SPI_CS_PIN == STM32IO_B12)
#define SPI_CS_AFIO 6
#endif
#if (SPI_PORT == 4) && (SPI_CLK_PIN == STM32IO_B13)
#define SPI_CLK_AFIO 6
#endif
#if (SPI_PORT == 3) && (SPI_CLK_PIN == STM32IO_C10)
#define SPI_CLK_AFIO 6
#endif
#if (SPI_PORT == 3) && (SPI_SDI_PIN == STM32IO_C11)
#define SPI_SDI_AFIO 6
#endif
#if (SPI_PORT == 3) && (SPI_SDO_PIN == STM32IO_C12)
#define SPI_SDO_AFIO 6
#endif
#if (SPI_PORT == 5) && (SPI_CLK_PIN == STM32IO_E3)
#define SPI_CLK_AFIO 6
#endif
#if (SPI_PORT == 5) && (SPI_CS_PIN == STM32IO_E5)
#define SPI_CS_AFIO 6
#endif
#if (SPI_PORT == 5) && (SPI_SDI_PIN == STM32IO_E6)
#define SPI_SDI_AFIO 6
#endif
#if (SPI_PORT == 5) && (SPI_SDO_PIN == STM32IO_E7)
#define SPI_SDO_AFIO 6
#endif
#if (SPI_PORT == 5) && (SPI_CS_PIN == STM32IO_E11)
#define SPI_CS_AFIO 6
#endif
#if (SPI_PORT == 5) && (SPI_CLK_PIN == STM32IO_E12)
#define SPI_CLK_AFIO 6
#endif
#if (SPI_PORT == 5) && (SPI_SDI_PIN == STM32IO_E13)
#define SPI_SDI_AFIO 6
#endif
#if (SPI_PORT == 5) && (SPI_SDO_PIN == STM32IO_E14)
#define SPI_SDO_AFIO 6
#endif

#ifndef SPI_CLK_AFIO
#error "SPI pin configuration not supported"
#endif
#ifndef SPI_SDO_AFIO
#error "SPI pin configuration not supported"
#endif
#ifndef SPI_SDI_AFIO
#error "SPI pin configuration not supported"
#endif
#ifdef SPI_CS
#ifndef SPI_CS_AFIO
#error "SPI pin configuration not supported"
#endif
#endif

#define SPI_REG __helper__(SPI, SPI_PORT, )
#if (SPI_PORT == 2 || SPI_PORT == 3)
#define SPI_ENREG RCC->APB1ENR
#define SPI_ENVAL __helper__(RCC_APB1ENR_SPI, SPI_PORT, EN)
#define SPI_CLOCK HAL_RCC_GetPCLK1Freq()
#else
#define SPI_ENREG RCC->APB2ENR
#define SPI_ENVAL __helper__(RCC_APB2ENR_SPI, SPI_PORT, EN)
#define SPI_CLOCK HAL_RCC_GetPCLK1Freq()
#endif

#if (SPI_PORT == 1)
#define SPI_DMA_TX_CHANNEL_NUM 3
#define SPI_DMA_RX_CHANNEL_NUM 2
#define SPI_DMA_TX_IFR_POS 8
#define SPI_DMA_RX_IFR_POS 4
#elif (SPI_PORT == 2)
#define SPI_DMA_TX_CHANNEL_NUM 5
#define SPI_DMA_RX_CHANNEL_NUM 4
#define SPI_DMA_TX_IFR_POS 16
#define SPI_DMA_RX_IFR_POS 12
#endif

#define SPI_DMA_TX_CHANNEL __helper__(DMA1_Channel, SPI_DMA_TX_CHANNEL_NUM, )
#define SPI_DMA_RX_CHANNEL __helper__(DMA1_Channel, SPI_DMA_RX_CHANNEL_NUM, )

#define SPI_DMA_TX_IFCR_MASK (0b1111 << SPI_DMA_TX_IFR_POS)
#define SPI_DMA_RX_IFCR_MASK (0b1111 << SPI_DMA_RX_IFR_POS)

#endif

// I2C
#if (defined(I2C_CLK) && defined(I2C_DATA))

#define I2C_CLK_PIN __iopin__(I2C_CLK_PORT, I2C_CLK_BIT)
#define I2C_DATA_PIN __iopin__(I2C_DATA_PORT, I2C_DATA_BIT)

#define MCU_HAS_I2C
#define MCU_SUPPORTS_I2C_SLAVE
#ifndef I2C_ADDRESS
#define I2C_ADDRESS 0
#endif

#ifndef I2C_PORT
#define I2C_PORT 1
#endif

#define I2C_APBEN __helper__(RCC_APB1ENR_I2C, I2C_PORT, EN)
#define I2C_REG __helper__(I2C, I2C_PORT, )
#define I2C_SPEEDRANGE (HAL_RCC_GetPCLK1Freq() / 1000000UL)

#if (I2C_PORT == 1) && (I2C_CLK_PIN == STM32IO_B6)
#define I2C_CLK_AFIO 1
#endif
#if (I2C_PORT == 1) && (I2C_DATA_PIN == STM32IO_B7)
#define I2C_DATA_AFIO 1
#endif
#if (I2C_PORT == 1) && (I2C_CLK_PIN == STM32IO_B8)
#define I2C_CLK_AFIO 1
#endif
#if (I2C_PORT == 1) && (I2C_DATA_PIN == STM32IO_B9)
#define I2C_DATA_AFIO 1
#endif
#if (I2C_PORT == 2) && (I2C_CLK_PIN == STM32IO_B10)
#define I2C_CLK_AFIO 1
#endif
#if (I2C_PORT == 2) && (I2C_DATA_PIN == STM32IO_B11)
#define I2C_DATA_AFIO 1
#endif
#if (I2C_PORT == 2) && (I2C_CLK_PIN == STM32IO_B13)
#define I2C_CLK_AFIO 5
#endif
#if (I2C_PORT == 2) && (I2C_DATA_PIN == STM32IO_B14)
#define I2C_DATA_AFIO 5
#endif


#define I2C_IRQ __helper__(I2C, I2C_PORT, _EV_IRQn)
#define I2C_ISR __helper__(I2C, I2C_PORT, _EV_IRQHandler)

#ifndef I2C_FREQ
#define I2C_FREQ 400000UL
#endif
#endif

// Timer registers
#ifndef ITP_TIMER
#define ITP_TIMER 15
#endif
#if (ITP_TIMER == 1)
#define MCU_ITP_ISR TIM1_BRK_UP_TRG_COM_IRQHandler
#define MCU_ITP_IRQ TIM1_BRK_UP_TRG_COM_IRQn
#elif (ITP_TIMER == 6)
#define MCU_ITP_ISR TIM6_DAC_IRQHandler
#define MCU_ITP_IRQ TIM6_DAC_IRQn
#else
#define MCU_ITP_ISR __helper__(TIM, ITP_TIMER, _IRQHandler)
#define MCU_ITP_IRQ __helper__(TIM, ITP_TIMER, _IRQn)
#endif
#define TIMER_REG __helper__(TIM, ITP_TIMER, )
#if (ITP_TIMER == 1 || (ITP_TIMER >=15  & ITP_TIMER <= 17))
#define TIMER_ENREG APB2ENR
#define TIMER_RESETREG APB2RSTR
#define TIMER_APB __helper__(RCC_APB2ENR_TIM, ITP_TIMER, EN)
#define TIMER_CLOCK HAL_RCC_GetPCLK1Freq()
#else
#define TIMER_ENREG APB1ENR
#define TIMER_RESETREG APB1RSTR
#define TIMER_APB __helper__(RCC_APB1ENR_TIM, ITP_TIMER, EN)
#define TIMER_CLOCK HAL_RCC_GetPCLK1Freq()
#endif

#ifndef SERVO_TIMER
#define SERVO_TIMER 16
#endif
#if (SERVO_TIMER == 1)
#define MCU_SERVO_ISR TIM1_BRK_UP_TRG_COM_IRQHandler
#define MCU_SERVO_IRQ TIM1_BRK_UP_TRG_COM_IRQn
#elif (SERVO_TIMER == 6)
#define MCU_SERVO_ISR TIM6_DAC_IRQHandler
#define MCU_SERVO_IRQ TIM6_DAC_IRQn
#else
#define MCU_SERVO_ISR __helper__(TIM, SERVO_TIMER, _IRQHandler)
#define MCU_SERVO_IRQ __helper__(TIM, SERVO_TIMER, _IRQn)
#endif
#define SERVO_TIMER_REG __helper__(TIM, SERVO_TIMER, )
#if (SERVO_TIMER == 1 || (SERVO_TIMER >= 15 & SERVO_TIMER <= 17))
#define SERVO_TIMER_ENREG APB2ENR
#define SERVO_TIMER_RESETREG APB1RSTR
#define SERVO_TIMER_APB __helper__(RCC_APB2ENR_TIM, SERVO_TIMER, EN)
#define SERVO_CLOCK HAL_RCC_GetPCLK1Freq()
#else
#define SERVO_TIMER_ENREG APB1ENR
#define SERVO_TIMER_RESETREG APB1RSTR
#define SERVO_TIMER_APB __helper__(RCC_APB1ENR_TIM, SERVO_TIMER, EN)
#define SERVO_CLOCK HAL_RCC_GetPCLK1Freq()
#endif

#ifdef ONESHOT_TIMER
#define MCU_HAS_ONESHOT_TIMER
#if (ONESHOT_TIMER == 1)
#define MCU_ONESHOT_ISR TIM1_BRK_UP_TRG_COM_IRQHandler
#define MCU_ONESHOT_IRQ TIM1_BRK_UP_TRG_COM_IRQn
#elif (ONESHOT_TIMER == 6)
#define MCU_ONESHOT_ISR TIM6_DAC_IRQHandler
#define MCU_ONESHOT_IRQ TIM6_DAC_IRQn
#else
#define MCU_ONESHOT_ISR __helper__(TIM, ONESHOT_TIMER, _IRQHandler)
#define MCU_ONESHOT_IRQ __helper__(TIM, ONESHOT_TIMER, _IRQn)
#endif
#define ONESHOT_TIMER_REG __helper__(TIM, ONESHOT_TIMER, )
#if (ONESHOT_TIMER == 1 || (ONESHOT_TIMER >= 15 & ONESHOT_TIMER <= 17))
#define ONESHOT_TIMER_ENREG APB2ENR
#define ONESHOT_TIMER_RESETREG APB1RSTR
#define ONESHOT_TIMER_APB __helper__(RCC_APB2ENR_TIM, ONESHOT_TIMER, EN)
#define ONESHOT_TIMER_CLOCK HAL_RCC_GetPCLK1Freq()
#else
#define ONESHOT_TIMER_ENREG APB1ENR
#define ONESHOT_TIMER_RESETREG APB1RSTR
#define ONESHOT_TIMER_APB __helper__(RCC_APB1ENR_TIM, ONESHOT_TIMER, EN)
#define ONESHOT_TIMER_CLOCK HAL_RCC_GetPCLK1Freq()
#endif
#endif

#ifndef __indirect__
#define __indirect__ex__(X, Y) DIO##X##_##Y
#define __indirect__(X, Y) __indirect__ex__(X, Y)
#endif

#ifndef BYTE_OPS
#define BYTE_OPS
// Set bit y in byte x
#define SETBIT(x, y) ((x) |= (1 << (y)))
// Clear bit y in byte x
#define CLEARBIT(x, y) ((x) &= ~(1 << (y)))
// Check bit y in byte x
#define CHECKBIT(x, y) ((x) & (1 << (y)))
// Toggle bit y in byte x
#define TOGGLEBIT(x, y) ((x) ^= (1 << (y)))
// Set byte y in byte x
#define SETFLAG(x, y) ((x) |= (y))
// Clear byte y in byte x
#define CLEARFLAG(x, y) ((x) &= ~(y))
// Check byte y in byte x
#define CHECKFLAG(x, y) ((x) & (y))
// Toggle byte y in byte x
#define TOGGLEFLAG(x, y) ((x) ^= (y))
#endif

#define mcu_config_input(diopin)                                                                                                    \
	{                                                                                                                               \
		RCC->AHBENR |= __indirect__(diopin, AHBEN);                                                                               \
		__indirect__(diopin, GPIO)->MODER &= ~(GPIO_RESET << ((__indirect__(diopin, BIT)) << 1)); /*reset dir (defaults to input)*/ \
	}

#define mcu_config_output(diopin)                                                                                 \
	{                                                                                                             \
		RCC->AHBENR |= __indirect__(diopin, AHBEN);                                                             \
		__indirect__(diopin, GPIO)->MODER &= ~(GPIO_RESET << ((__indirect__(diopin, BIT)) << 1)); /*reset dir*/   \
		__indirect__(diopin, GPIO)->MODER |= (GPIO_OUTPUT << ((__indirect__(diopin, BIT)) << 1)); /*output mode*/ \
		__indirect__(diopin, GPIO)->OSPEEDR |= (0x03 << ((__indirect__(diopin, BIT)) << 1));	  /*fast speed mode*/ \
	}

#define mcu_config_af(diopin, afrval)                                                                                                               \
	{                                                                                                                                               \
		RCC->AHBENR |= __indirect__(diopin, AHBEN);                                                                                               \
		__indirect__(diopin, GPIO)->MODER &= ~(GPIO_RESET << ((__indirect__(diopin, BIT)) << 1)); /*reset dir*/                                     \
		__indirect__(diopin, GPIO)->MODER |= (GPIO_AF << ((__indirect__(diopin, BIT)) << 1));	  /*af mode*/                                       \
		__indirect__(diopin, GPIO)->AFR[(__indirect__(diopin, BIT) >> 3)] &= ~(0xf << ((__indirect__(diopin, BIT) & 0x07) << 2));                   \
		__indirect__(diopin, GPIO)->AFR[(__indirect__(diopin, BIT) >> 3)] |= (afrval << ((__indirect__(diopin, BIT) & 0x07) << 2)); /*af mode*/     \
		__indirect__(diopin, GPIO)->OSPEEDR |= (0x03 << ((__indirect__(diopin, BIT)) << 1));										/*fast speed mode*/ \
	}

#define mcu_config_pullup(diopin)                                                                    \
	{                                                                                                \
		__indirect__(diopin, GPIO)->PUPDR &= ~(GPIO_RESET << ((__indirect__(diopin, BIT)) << 1));    \
		__indirect__(diopin, GPIO)->PUPDR |= (GPIO_IN_PULLUP << ((__indirect__(diopin, BIT)) << 1)); \
	}

#define mcu_config_opendrain(diopin)                                                \
	{                                                                               \
		__indirect__(diopin, GPIO)->OTYPER |= (1 << ((__indirect__(diopin, BIT)))); \
	}

#define mcu_config_pwm(diopin, freq)                                                                                                                                \
	{                                                                                                                                                               \
		RCC->AHBENR |= __indirect__(diopin, AHBEN);                                                                                                               \
		__indirect__(diopin, ENREG) |= __indirect__(diopin, APBEN);                                                                                                 \
		__indirect__(diopin, GPIO)->MODER &= ~(GPIO_RESET << ((__indirect__(diopin, BIT)) << 1)); /*reset dir*/                                                     \
		__indirect__(diopin, GPIO)->MODER |= (GPIO_AF << ((__indirect__(diopin, BIT)) << 1));	  /*af mode*/                                                       \
		__indirect__(diopin, GPIO)->AFR[(__indirect__(diopin, BIT) >> 3)] &= ~(0xf << ((__indirect__(diopin, BIT) & 0x07) << 2));                                   \
		__indirect__(diopin, GPIO)->AFR[(__indirect__(diopin, BIT) >> 3)] |= ((__indirect__(diopin, AF) << ((__indirect__(diopin, BIT) & 0x07) << 2))); /*af mode*/ \
		__indirect__(diopin, TIMREG)->CR1 = 0;                                                                                                                      \
		__indirect__(diopin, TIMREG)->PSC = (uint16_t)(__indirect__(diopin, CLOCK) / 1000000UL) - 1;                                                                \
		__indirect__(diopin, TIMREG)->ARR = (uint16_t)(1000000UL / freq);                                                                                           \
		__indirect__(diopin, TIMREG)->__indirect__(diopin, CCR) = 0;                                                                                                \
		__indirect__(diopin, TIMREG)->__indirect__(diopin, CCMREG) = __indirect__(diopin, MODE);                                                                    \
		__indirect__(diopin, TIMREG)->CCER |= (1U << ((__indirect__(diopin, CHANNEL) - 1) << 2));                                                                   \
		__indirect__(diopin, TIMREG)->BDTR |= (1 << 15);                                                                                                            \
		__indirect__(diopin, TIMREG)->CR1 |= 0x01U;                                                                                                                 \
		__indirect__(diopin, ENOUTPUT);                                                                                                                             \
	}

#define mcu_config_input_isr(diopin)                                                                              \
	{                                                                                                             \
		RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;                                                                     \
		SYSCFG->EXTICR[(__indirect__(diopin, EXTIREG))] &= ~(0xF << (((__indirect__(diopin, BIT)) & 0x03) << 2)); \
		SYSCFG->EXTICR[(__indirect__(diopin, EXTIREG))] |= (__indirect__(diopin, EXTIVAL));                       \
		SETBIT(EXTI->RTSR, __indirect__(diopin, BIT));                                                            \
		SETBIT(EXTI->FTSR, __indirect__(diopin, BIT));                                                            \
		SETBIT(EXTI->IMR, __indirect__(diopin, BIT));                                                             \
		NVIC_SetPriority(__indirect__(diopin, IRQ), 5);                                                           \
		NVIC_ClearPendingIRQ(__indirect__(diopin, IRQ));                                                          \
		NVIC_EnableIRQ(__indirect__(diopin, IRQ));                                                                \
	}

#if defined(ADC1_COMMON)
#define ADC_COMMON ADC1_COMMON
#elif defined(ADC12_COMMON)
#define ADC_COMMON ADC12_COMMON
#elif defined(ADC123_COMMON)
#define ADC_COMMON ADC123_COMMON
#endif

#define mcu_config_analog(diopin)                                                                                                     \
	{                                                                                                                                 \
		ADC_COMMON->CCR &= ~(ADC_CCR_ADCPRE);                                                                                         \
		ADC_COMMON->CCR |= (ADC_CCR_ADCPRE_0 | ADC_CCR_ADCPRE_1);                                                                     \
		RCC->APB2ENR |= (RCC_APB2ENR_ADC1EN);                                                                                         \
		RCC->AHBENR |= (__indirect__(diopin, AHBEN));                                                                               \
		ADC1->SQR1 = 1; /*one conversion*/                                                                                            \
		ADC1->SMPR1 = 0x00ffffff & 0x36DB6DB6;                                                                                        \
		ADC1->SMPR2 = 0x36DB6DB6;                                                                                                     \
		ADC1->CR2 &= ~ADC_CR2_CONT;																  /*single conversion mode*/          \
		__indirect__(diopin, GPIO)->MODER &= ~(GPIO_RESET << ((__indirect__(diopin, BIT)) << 1)); /*reset dir*/                       \
		__indirect__(diopin, GPIO)->MODER |= (GPIO_ANALOG << ((__indirect__(diopin, BIT)) << 1)); /*analog mode*/                     \
		ADC1->CR2 |= ADC_CR2_ADON;																  /*enable adc*/                      \
		ADC1->CR2 |= (ADC_CR2_EXTEN_0 | ADC_CR2_EXTEN_1);										  /*external start trigger software*/ \
	}

#define mcu_get_input(diopin) (CHECKBIT(__indirect__(diopin, GPIO)->IDR, __indirect__(diopin, BIT)))
#define mcu_get_output(diopin) (CHECKBIT(__indirect__(diopin, GPIO)->ODR, __indirect__(diopin, BIT)))
#define mcu_set_output(diopin) (__indirect__(diopin, GPIO)->BSRR = (1UL << __indirect__(diopin, BIT)))
#define mcu_clear_output(diopin) (__indirect__(diopin, GPIO)->BSRR = ((1UL << 16) << __indirect__(diopin, BIT)))
#define mcu_toggle_output(diopin) (TOGGLEBIT(__indirect__(diopin, GPIO)->ODR, __indirect__(diopin, BIT)))
#define mcu_set_pwm(diopin, pwmvalue)                                                                                               \
	{                                                                                                                               \
		__indirect__(diopin, TIMREG)->__indirect__(diopin, CCR) = (uint16_t)((__indirect__(diopin, TIMREG)->ARR * pwmvalue) / 255); \
	}

#define mcu_get_pwm(diopin) ((uint8_t)((((uint32_t)__indirect__(diopin, TIMREG)->__indirect__(diopin, CCR)) * 255) / ((uint32_t)__indirect__(diopin, TIMREG)->ARR)))

#define mcu_get_analog(diopin)                      \
	({                                              \
		ADC1->SQR3 = __indirect__(diopin, CHANNEL); \
		ADC1->CR2 |= ADC_CR2_SWSTART;               \
		ADC1->CR2 &= ~ADC_CR2_SWSTART;              \
		while (!(ADC1->SR & ADC_SR_EOC))            \
			;                                       \
		ADC1->SR &= ~ADC_SR_EOC;                    \
		(0x3FF & (ADC1->DR >> 2));                  \
	})


#if defined(PROBE) && defined(PROBE_ISR)
#define mcu_enable_probe_isr() SETBIT(EXTI->IMR, PROBE_BIT)
#define mcu_disable_probe_isr() CLEARBIT(EXTI->IMR, PROBE_BIT)
#else
#define mcu_enable_probe_isr()
#define mcu_disable_probe_isr()
#endif

	extern volatile bool stm32_global_isr_enabled;
#define mcu_enable_global_isr()          \
	{                                    \
		__enable_irq();                  \
		stm32_global_isr_enabled = true; \
	}
#define mcu_disable_global_isr()          \
	{                                     \
		stm32_global_isr_enabled = false; \
		__disable_irq();                  \
	}
#define mcu_get_global_isr() stm32_global_isr_enabled
#define mcu_free_micros() ({ (1000UL - (SysTick->VAL * 1000UL / SysTick->LOAD)); })
#define mcu_delay_cycles(X)     \
	{                           \
		uint32_t t = X;         \
		__TIMEOUT_US__(t);                   \
	}

#define GPIO_RESET 0x3U
#define GPIO_INPUT 0x0U
#define GPIO_OUTPUT 0x1U
#define GPIO_AF 0x2U
#define GPIO_ANALOG 0x3U

#define GPIO_IN_FLOAT 0x4U
#define GPIO_IN_PUP 0x8U
#define GPIO_IN_ANALOG 0 // not needed after reseting bits

#define GPIO_IN_PULLUP 0x1U

#ifdef MCU_HAS_ONESHOT_TIMER

/**
 * starts the timeout. Once hit the the respective callback is called
 * */
#define mcu_start_timeout()           \
	({                                \
		ONESHOT_TIMER_REG->SR = 0;    \
		ONESHOT_TIMER_REG->CNT = 0;   \
		ONESHOT_TIMER_REG->DIER |= 1; \
		ONESHOT_TIMER_REG->CR1 |= 1;  \
	})
#endif

#ifdef __cplusplus
}
#endif

#endif
