/*
	Name: mcumap_stm32f10x.h
	Description: Contains all MCU and PIN definitions for STM32F10x to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 04-02-2020

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef MCUMAP_LPC176X_H
#define MCUMAP_LPC176X_H

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
#include "LPC17xx.h"
#include <stdbool.h>
#include "core_cm3.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "lpc_types.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_systick.h"
#include "lpc17xx_pwm.h"

// defines the frequency of the mcu
#ifndef F_CPU
#define F_CPU 100000000UL
#endif
// defines the maximum and minimum step rates
#ifndef F_STEP_MAX
#define F_STEP_MAX 200000
#endif
#ifndef F_STEP_MIN
#define F_STEP_MIN 4
#endif
// defines special mcu to access flash strings and arrays
#define __rom__
#define __romstr__
#define __romarr__ const char
#define rom_strptr *
#define rom_strcpy strcpy
#define rom_strncpy strncpy
#define rom_memcpy memcpy
#define rom_read_byte *

#if (INTERFACE == INTERFACE_USB)
// if USB VCP is used force RX sync also
#ifndef ENABLE_SYNC_TX
#define ENABLE_SYNC_TX
#endif
#ifndef ENABLE_SYNC_RX
#define ENABLE_SYNC_RX
#endif
#endif

// Helper macros
#define __helper_ex__(left, mid, right) left##mid##right
#define __helper__(left, mid, right) __helper_ex__(left, mid, right)

#define __mbedpin_ex__(port, pin) P##port##_##pin
#define __mbedpin__(port, pin) __mbedpin_ex__(port, pin)

#define __gpioreg__(X) __helper__(LPC_GPIO, X, )
#define __pinmodereg__(X) __helper__(PINMODE, X, )

#define PINCON_0_L 0
#define PINCON_0_H 1
#define PINCON_1_L 2
#define PINCON_1_H 3
#define PINCON_2_L 4
#define PINCON_2_H 5
#define PINCON_3_L 6
#define PINCON_3_H 7
#define PINCON_4_L 8
#define PINCON_4_H 9
#define __pincon(X, Y) PINCON_##X##_##Y
#define __pincon__(X, Y) __pincon(X, Y)

#if (defined(STEP0_PORT) && defined(STEP0_BIT))
#define STEP0 0
#define STEP0_MBED_PIN __mbedpin__(STEP0_PORT, STEP0_BIT)
#define STEP0_GPIOREG __gpioreg__(STEP0_PORT)
#if (STEP0_BIT < 16)
#define STEP0_PINHALF L
#else
#define STEP0_PINHALF H
#endif
#define STEP0_PINCON __pincon__(STEP0_PORT, STEP0_PINHALF)
#define DIO0 0
#define DIO0_MBED_PIN STEP0_MBED_PIN
#define DIO0_PORT STEP0_PORT
#define DIO0_BIT STEP0_BIT
#define DIO0_GPIOREG STEP0_GPIOREG
#define DIO0_PINHALF STEP0_PINHALF
#define DIO0_PINCON STEP0_PINCON
#endif
#if (defined(STEP1_PORT) && defined(STEP1_BIT))
#define STEP1 1
#define STEP1_MBED_PIN __mbedpin__(STEP1_PORT, STEP1_BIT)
#define STEP1_GPIOREG __gpioreg__(STEP1_PORT)
#if (STEP1_BIT < 16)
#define STEP1_PINHALF L
#else
#define STEP1_PINHALF H
#endif
#define STEP1_PINCON __pincon__(STEP1_PORT, STEP1_PINHALF)
#define DIO1 1
#define DIO1_MBED_PIN STEP1_MBED_PIN
#define DIO1_PORT STEP1_PORT
#define DIO1_BIT STEP1_BIT
#define DIO1_GPIOREG STEP1_GPIOREG
#define DIO1_PINHALF STEP1_PINHALF
#define DIO1_PINCON STEP1_PINCON
#endif
#if (defined(STEP2_PORT) && defined(STEP2_BIT))
#define STEP2 2
#define STEP2_MBED_PIN __mbedpin__(STEP2_PORT, STEP2_BIT)
#define STEP2_GPIOREG __gpioreg__(STEP2_PORT)
#if (STEP2_BIT < 16)
#define STEP2_PINHALF L
#else
#define STEP2_PINHALF H
#endif
#define STEP2_PINCON __pincon__(STEP2_PORT, STEP2_PINHALF)
#define DIO2 2
#define DIO2_MBED_PIN STEP2_MBED_PIN
#define DIO2_PORT STEP2_PORT
#define DIO2_BIT STEP2_BIT
#define DIO2_GPIOREG STEP2_GPIOREG
#define DIO2_PINHALF STEP2_PINHALF
#define DIO2_PINCON STEP2_PINCON
#endif
#if (defined(STEP3_PORT) && defined(STEP3_BIT))
#define STEP3 3
#define STEP3_MBED_PIN __mbedpin__(STEP3_PORT, STEP3_BIT)
#define STEP3_GPIOREG __gpioreg__(STEP3_PORT)
#if (STEP3_BIT < 16)
#define STEP3_PINHALF L
#else
#define STEP3_PINHALF H
#endif
#define STEP3_PINCON __pincon__(STEP3_PORT, STEP3_PINHALF)
#define DIO3 3
#define DIO3_MBED_PIN STEP3_MBED_PIN
#define DIO3_PORT STEP3_PORT
#define DIO3_BIT STEP3_BIT
#define DIO3_GPIOREG STEP3_GPIOREG
#define DIO3_PINHALF STEP3_PINHALF
#define DIO3_PINCON STEP3_PINCON
#endif
#if (defined(STEP4_PORT) && defined(STEP4_BIT))
#define STEP4 4
#define STEP4_MBED_PIN __mbedpin__(STEP4_PORT, STEP4_BIT)
#define STEP4_GPIOREG __gpioreg__(STEP4_PORT)
#if (STEP4_BIT < 16)
#define STEP4_PINHALF L
#else
#define STEP4_PINHALF H
#endif
#define STEP4_PINCON __pincon__(STEP4_PORT, STEP4_PINHALF)
#define DIO4 4
#define DIO4_MBED_PIN STEP4_MBED_PIN
#define DIO4_PORT STEP4_PORT
#define DIO4_BIT STEP4_BIT
#define DIO4_GPIOREG STEP4_GPIOREG
#define DIO4_PINHALF STEP4_PINHALF
#define DIO4_PINCON STEP4_PINCON
#endif
#if (defined(STEP5_PORT) && defined(STEP5_BIT))
#define STEP5 5
#define STEP5_MBED_PIN __mbedpin__(STEP5_PORT, STEP5_BIT)
#define STEP5_GPIOREG __gpioreg__(STEP5_PORT)
#if (STEP5_BIT < 16)
#define STEP5_PINHALF L
#else
#define STEP5_PINHALF H
#endif
#define STEP5_PINCON __pincon__(STEP5_PORT, STEP5_PINHALF)
#define DIO5 5
#define DIO5_MBED_PIN STEP5_MBED_PIN
#define DIO5_PORT STEP5_PORT
#define DIO5_BIT STEP5_BIT
#define DIO5_GPIOREG STEP5_GPIOREG
#define DIO5_PINHALF STEP5_PINHALF
#define DIO5_PINCON STEP5_PINCON
#endif
#if (defined(STEP6_PORT) && defined(STEP6_BIT))
#define STEP6 6
#define STEP6_MBED_PIN __mbedpin__(STEP6_PORT, STEP6_BIT)
#define STEP6_GPIOREG __gpioreg__(STEP6_PORT)
#if (STEP6_BIT < 16)
#define STEP6_PINHALF L
#else
#define STEP6_PINHALF H
#endif
#define STEP6_PINCON __pincon__(STEP6_PORT, STEP6_PINHALF)
#define DIO6 6
#define DIO6_MBED_PIN STEP6_MBED_PIN
#define DIO6_PORT STEP6_PORT
#define DIO6_BIT STEP6_BIT
#define DIO6_GPIOREG STEP6_GPIOREG
#define DIO6_PINHALF STEP6_PINHALF
#define DIO6_PINCON STEP6_PINCON
#endif
#if (defined(STEP7_PORT) && defined(STEP7_BIT))
#define STEP7 7
#define STEP7_MBED_PIN __mbedpin__(STEP7_PORT, STEP7_BIT)
#define STEP7_GPIOREG __gpioreg__(STEP7_PORT)
#if (STEP7_BIT < 16)
#define STEP7_PINHALF L
#else
#define STEP7_PINHALF H
#endif
#define STEP7_PINCON __pincon__(STEP7_PORT, STEP7_PINHALF)
#define DIO7 7
#define DIO7_MBED_PIN STEP7_MBED_PIN
#define DIO7_PORT STEP7_PORT
#define DIO7_BIT STEP7_BIT
#define DIO7_GPIOREG STEP7_GPIOREG
#define DIO7_PINHALF STEP7_PINHALF
#define DIO7_PINCON STEP7_PINCON
#endif
#if (defined(DIR0_PORT) && defined(DIR0_BIT))
#define DIR0 8
#define DIR0_MBED_PIN __mbedpin__(DIR0_PORT, DIR0_BIT)
#define DIR0_GPIOREG __gpioreg__(DIR0_PORT)
#if (DIR0_BIT < 16)
#define DIR0_PINHALF L
#else
#define DIR0_PINHALF H
#endif
#define DIR0_PINCON __pincon__(DIR0_PORT, DIR0_PINHALF)
#define DIO8 8
#define DIO8_MBED_PIN DIR0_MBED_PIN
#define DIO8_PORT DIR0_PORT
#define DIO8_BIT DIR0_BIT
#define DIO8_GPIOREG DIR0_GPIOREG
#define DIO8_PINHALF DIR0_PINHALF
#define DIO8_PINCON DIR0_PINCON
#endif
#if (defined(DIR1_PORT) && defined(DIR1_BIT))
#define DIR1 9
#define DIR1_MBED_PIN __mbedpin__(DIR1_PORT, DIR1_BIT)
#define DIR1_GPIOREG __gpioreg__(DIR1_PORT)
#if (DIR1_BIT < 16)
#define DIR1_PINHALF L
#else
#define DIR1_PINHALF H
#endif
#define DIR1_PINCON __pincon__(DIR1_PORT, DIR1_PINHALF)
#define DIO9 9
#define DIO9_MBED_PIN DIR1_MBED_PIN
#define DIO9_PORT DIR1_PORT
#define DIO9_BIT DIR1_BIT
#define DIO9_GPIOREG DIR1_GPIOREG
#define DIO9_PINHALF DIR1_PINHALF
#define DIO9_PINCON DIR1_PINCON
#endif
#if (defined(DIR2_PORT) && defined(DIR2_BIT))
#define DIR2 10
#define DIR2_MBED_PIN __mbedpin__(DIR2_PORT, DIR2_BIT)
#define DIR2_GPIOREG __gpioreg__(DIR2_PORT)
#if (DIR2_BIT < 16)
#define DIR2_PINHALF L
#else
#define DIR2_PINHALF H
#endif
#define DIR2_PINCON __pincon__(DIR2_PORT, DIR2_PINHALF)
#define DIO10 10
#define DIO10_MBED_PIN DIR2_MBED_PIN
#define DIO10_PORT DIR2_PORT
#define DIO10_BIT DIR2_BIT
#define DIO10_GPIOREG DIR2_GPIOREG
#define DIO10_PINHALF DIR2_PINHALF
#define DIO10_PINCON DIR2_PINCON
#endif
#if (defined(DIR3_PORT) && defined(DIR3_BIT))
#define DIR3 11
#define DIR3_MBED_PIN __mbedpin__(DIR3_PORT, DIR3_BIT)
#define DIR3_GPIOREG __gpioreg__(DIR3_PORT)
#if (DIR3_BIT < 16)
#define DIR3_PINHALF L
#else
#define DIR3_PINHALF H
#endif
#define DIR3_PINCON __pincon__(DIR3_PORT, DIR3_PINHALF)
#define DIO11 11
#define DIO11_MBED_PIN DIR3_MBED_PIN
#define DIO11_PORT DIR3_PORT
#define DIO11_BIT DIR3_BIT
#define DIO11_GPIOREG DIR3_GPIOREG
#define DIO11_PINHALF DIR3_PINHALF
#define DIO11_PINCON DIR3_PINCON
#endif
#if (defined(DIR4_PORT) && defined(DIR4_BIT))
#define DIR4 12
#define DIR4_MBED_PIN __mbedpin__(DIR4_PORT, DIR4_BIT)
#define DIR4_GPIOREG __gpioreg__(DIR4_PORT)
#if (DIR4_BIT < 16)
#define DIR4_PINHALF L
#else
#define DIR4_PINHALF H
#endif
#define DIR4_PINCON __pincon__(DIR4_PORT, DIR4_PINHALF)
#define DIO12 12
#define DIO12_MBED_PIN DIR4_MBED_PIN
#define DIO12_PORT DIR4_PORT
#define DIO12_BIT DIR4_BIT
#define DIO12_GPIOREG DIR4_GPIOREG
#define DIO12_PINHALF DIR4_PINHALF
#define DIO12_PINCON DIR4_PINCON
#endif
#if (defined(DIR5_PORT) && defined(DIR5_BIT))
#define DIR5 13
#define DIR5_MBED_PIN __mbedpin__(DIR5_PORT, DIR5_BIT)
#define DIR5_GPIOREG __gpioreg__(DIR5_PORT)
#if (DIR5_BIT < 16)
#define DIR5_PINHALF L
#else
#define DIR5_PINHALF H
#endif
#define DIR5_PINCON __pincon__(DIR5_PORT, DIR5_PINHALF)
#define DIO13 13
#define DIO13_MBED_PIN DIR5_MBED_PIN
#define DIO13_PORT DIR5_PORT
#define DIO13_BIT DIR5_BIT
#define DIO13_GPIOREG DIR5_GPIOREG
#define DIO13_PINHALF DIR5_PINHALF
#define DIO13_PINCON DIR5_PINCON
#endif
#if (defined(DIR6_PORT) && defined(DIR6_BIT))
#define DIR6 14
#define DIR6_MBED_PIN __mbedpin__(DIR6_PORT, DIR6_BIT)
#define DIR6_GPIOREG __gpioreg__(DIR6_PORT)
#if (DIR6_BIT < 16)
#define DIR6_PINHALF L
#else
#define DIR6_PINHALF H
#endif
#define DIR6_PINCON __pincon__(DIR6_PORT, DIR6_PINHALF)
#define DIO14 14
#define DIO14_MBED_PIN DIR6_MBED_PIN
#define DIO14_PORT DIR6_PORT
#define DIO14_BIT DIR6_BIT
#define DIO14_GPIOREG DIR6_GPIOREG
#define DIO14_PINHALF DIR6_PINHALF
#define DIO14_PINCON DIR6_PINCON
#endif
#if (defined(DIR7_PORT) && defined(DIR7_BIT))
#define DIR7 15
#define DIR7_MBED_PIN __mbedpin__(DIR7_PORT, DIR7_BIT)
#define DIR7_GPIOREG __gpioreg__(DIR7_PORT)
#if (DIR7_BIT < 16)
#define DIR7_PINHALF L
#else
#define DIR7_PINHALF H
#endif
#define DIR7_PINCON __pincon__(DIR7_PORT, DIR7_PINHALF)
#define DIO15 15
#define DIO15_MBED_PIN DIR7_MBED_PIN
#define DIO15_PORT DIR7_PORT
#define DIO15_BIT DIR7_BIT
#define DIO15_GPIOREG DIR7_GPIOREG
#define DIO15_PINHALF DIR7_PINHALF
#define DIO15_PINCON DIR7_PINCON
#endif
#if (defined(STEP0_EN_PORT) && defined(STEP0_EN_BIT))
#define STEP0_EN 16
#define STEP0_EN_MBED_PIN __mbedpin__(STEP0_EN_PORT, STEP0_EN_BIT)
#define STEP0_EN_GPIOREG __gpioreg__(STEP0_EN_PORT)
#if (STEP0_EN_BIT < 16)
#define STEP0_EN_PINHALF L
#else
#define STEP0_EN_PINHALF H
#endif
#define STEP0_EN_PINCON __pincon__(STEP0_EN_PORT, STEP0_EN_PINHALF)
#define DIO16 16
#define DIO16_MBED_PIN STEP0_EN_MBED_PIN
#define DIO16_PORT STEP0_EN_PORT
#define DIO16_BIT STEP0_EN_BIT
#define DIO16_GPIOREG STEP0_EN_GPIOREG
#define DIO16_PINHALF STEP0_EN_PINHALF
#define DIO16_PINCON STEP0_EN_PINCON
#endif
#if (defined(STEP1_EN_PORT) && defined(STEP1_EN_BIT))
#define STEP1_EN 17
#define STEP1_EN_MBED_PIN __mbedpin__(STEP1_EN_PORT, STEP1_EN_BIT)
#define STEP1_EN_GPIOREG __gpioreg__(STEP1_EN_PORT)
#if (STEP1_EN_BIT < 16)
#define STEP1_EN_PINHALF L
#else
#define STEP1_EN_PINHALF H
#endif
#define STEP1_EN_PINCON __pincon__(STEP1_EN_PORT, STEP1_EN_PINHALF)
#define DIO17 17
#define DIO17_MBED_PIN STEP1_EN_MBED_PIN
#define DIO17_PORT STEP1_EN_PORT
#define DIO17_BIT STEP1_EN_BIT
#define DIO17_GPIOREG STEP1_EN_GPIOREG
#define DIO17_PINHALF STEP1_EN_PINHALF
#define DIO17_PINCON STEP1_EN_PINCON
#endif
#if (defined(STEP2_EN_PORT) && defined(STEP2_EN_BIT))
#define STEP2_EN 18
#define STEP2_EN_MBED_PIN __mbedpin__(STEP2_EN_PORT, STEP2_EN_BIT)
#define STEP2_EN_GPIOREG __gpioreg__(STEP2_EN_PORT)
#if (STEP2_EN_BIT < 16)
#define STEP2_EN_PINHALF L
#else
#define STEP2_EN_PINHALF H
#endif
#define STEP2_EN_PINCON __pincon__(STEP2_EN_PORT, STEP2_EN_PINHALF)
#define DIO18 18
#define DIO18_MBED_PIN STEP2_EN_MBED_PIN
#define DIO18_PORT STEP2_EN_PORT
#define DIO18_BIT STEP2_EN_BIT
#define DIO18_GPIOREG STEP2_EN_GPIOREG
#define DIO18_PINHALF STEP2_EN_PINHALF
#define DIO18_PINCON STEP2_EN_PINCON
#endif
#if (defined(STEP3_EN_PORT) && defined(STEP3_EN_BIT))
#define STEP3_EN 19
#define STEP3_EN_MBED_PIN __mbedpin__(STEP3_EN_PORT, STEP3_EN_BIT)
#define STEP3_EN_GPIOREG __gpioreg__(STEP3_EN_PORT)
#if (STEP3_EN_BIT < 16)
#define STEP3_EN_PINHALF L
#else
#define STEP3_EN_PINHALF H
#endif
#define STEP3_EN_PINCON __pincon__(STEP3_EN_PORT, STEP3_EN_PINHALF)
#define DIO19 19
#define DIO19_MBED_PIN STEP3_EN_MBED_PIN
#define DIO19_PORT STEP3_EN_PORT
#define DIO19_BIT STEP3_EN_BIT
#define DIO19_GPIOREG STEP3_EN_GPIOREG
#define DIO19_PINHALF STEP3_EN_PINHALF
#define DIO19_PINCON STEP3_EN_PINCON
#endif
#if (defined(STEP4_EN_PORT) && defined(STEP4_EN_BIT))
#define STEP4_EN 20
#define STEP4_EN_MBED_PIN __mbedpin__(STEP4_EN_PORT, STEP4_EN_BIT)
#define STEP4_EN_GPIOREG __gpioreg__(STEP4_EN_PORT)
#if (STEP4_EN_BIT < 16)
#define STEP4_EN_PINHALF L
#else
#define STEP4_EN_PINHALF H
#endif
#define STEP4_EN_PINCON __pincon__(STEP4_EN_PORT, STEP4_EN_PINHALF)
#define DIO20 20
#define DIO20_MBED_PIN STEP4_EN_MBED_PIN
#define DIO20_PORT STEP4_EN_PORT
#define DIO20_BIT STEP4_EN_BIT
#define DIO20_GPIOREG STEP4_EN_GPIOREG
#define DIO20_PINHALF STEP4_EN_PINHALF
#define DIO20_PINCON STEP4_EN_PINCON
#endif
#if (defined(STEP5_EN_PORT) && defined(STEP5_EN_BIT))
#define STEP5_EN 21
#define STEP5_EN_MBED_PIN __mbedpin__(STEP5_EN_PORT, STEP5_EN_BIT)
#define STEP5_EN_GPIOREG __gpioreg__(STEP5_EN_PORT)
#if (STEP5_EN_BIT < 16)
#define STEP5_EN_PINHALF L
#else
#define STEP5_EN_PINHALF H
#endif
#define STEP5_EN_PINCON __pincon__(STEP5_EN_PORT, STEP5_EN_PINHALF)
#define DIO21 21
#define DIO21_MBED_PIN STEP5_EN_MBED_PIN
#define DIO21_PORT STEP5_EN_PORT
#define DIO21_BIT STEP5_EN_BIT
#define DIO21_GPIOREG STEP5_EN_GPIOREG
#define DIO21_PINHALF STEP5_EN_PINHALF
#define DIO21_PINCON STEP5_EN_PINCON
#endif
#if (defined(STEP6_EN_PORT) && defined(STEP6_EN_BIT))
#define STEP6_EN 22
#define STEP6_EN_MBED_PIN __mbedpin__(STEP6_EN_PORT, STEP6_EN_BIT)
#define STEP6_EN_GPIOREG __gpioreg__(STEP6_EN_PORT)
#if (STEP6_EN_BIT < 16)
#define STEP6_EN_PINHALF L
#else
#define STEP6_EN_PINHALF H
#endif
#define STEP6_EN_PINCON __pincon__(STEP6_EN_PORT, STEP6_EN_PINHALF)
#define DIO22 22
#define DIO22_MBED_PIN STEP6_EN_MBED_PIN
#define DIO22_PORT STEP6_EN_PORT
#define DIO22_BIT STEP6_EN_BIT
#define DIO22_GPIOREG STEP6_EN_GPIOREG
#define DIO22_PINHALF STEP6_EN_PINHALF
#define DIO22_PINCON STEP6_EN_PINCON
#endif
#if (defined(STEP7_EN_PORT) && defined(STEP7_EN_BIT))
#define STEP7_EN 23
#define STEP7_EN_MBED_PIN __mbedpin__(STEP7_EN_PORT, STEP7_EN_BIT)
#define STEP7_EN_GPIOREG __gpioreg__(STEP7_EN_PORT)
#if (STEP7_EN_BIT < 16)
#define STEP7_EN_PINHALF L
#else
#define STEP7_EN_PINHALF H
#endif
#define STEP7_EN_PINCON __pincon__(STEP7_EN_PORT, STEP7_EN_PINHALF)
#define DIO23 23
#define DIO23_MBED_PIN STEP7_EN_MBED_PIN
#define DIO23_PORT STEP7_EN_PORT
#define DIO23_BIT STEP7_EN_BIT
#define DIO23_GPIOREG STEP7_EN_GPIOREG
#define DIO23_PINHALF STEP7_EN_PINHALF
#define DIO23_PINCON STEP7_EN_PINCON
#endif
#if (defined(PWM0_PORT) && defined(PWM0_BIT))
#define PWM0 24
#define PWM0_MBED_PIN __mbedpin__(PWM0_PORT, PWM0_BIT)
#define PWM0_GPIOREG __gpioreg__(PWM0_PORT)
#if (PWM0_BIT < 16)
#define PWM0_PINHALF L
#else
#define PWM0_PINHALF H
#endif
#define PWM0_PINCON __pincon__(PWM0_PORT, PWM0_PINHALF)
#define DIO24 24
#define DIO24_MBED_PIN PWM0_MBED_PIN
#define DIO24_PORT PWM0_PORT
#define DIO24_BIT PWM0_BIT
#define DIO24_GPIOREG PWM0_GPIOREG
#define DIO24_PINHALF PWM0_PINHALF
#define DIO24_PINCON PWM0_PINCON
#endif
#if (defined(PWM1_PORT) && defined(PWM1_BIT))
#define PWM1 25
#define PWM1_MBED_PIN __mbedpin__(PWM1_PORT, PWM1_BIT)
#define PWM1_GPIOREG __gpioreg__(PWM1_PORT)
#if (PWM1_BIT < 16)
#define PWM1_PINHALF L
#else
#define PWM1_PINHALF H
#endif
#define PWM1_PINCON __pincon__(PWM1_PORT, PWM1_PINHALF)
#define DIO25 25
#define DIO25_MBED_PIN PWM1_MBED_PIN
#define DIO25_PORT PWM1_PORT
#define DIO25_BIT PWM1_BIT
#define DIO25_GPIOREG PWM1_GPIOREG
#define DIO25_PINHALF PWM1_PINHALF
#define DIO25_PINCON PWM1_PINCON
#endif
#if (defined(PWM2_PORT) && defined(PWM2_BIT))
#define PWM2 26
#define PWM2_MBED_PIN __mbedpin__(PWM2_PORT, PWM2_BIT)
#define PWM2_GPIOREG __gpioreg__(PWM2_PORT)
#if (PWM2_BIT < 16)
#define PWM2_PINHALF L
#else
#define PWM2_PINHALF H
#endif
#define PWM2_PINCON __pincon__(PWM2_PORT, PWM2_PINHALF)
#define DIO26 26
#define DIO26_MBED_PIN PWM2_MBED_PIN
#define DIO26_PORT PWM2_PORT
#define DIO26_BIT PWM2_BIT
#define DIO26_GPIOREG PWM2_GPIOREG
#define DIO26_PINHALF PWM2_PINHALF
#define DIO26_PINCON PWM2_PINCON
#endif
#if (defined(PWM3_PORT) && defined(PWM3_BIT))
#define PWM3 27
#define PWM3_MBED_PIN __mbedpin__(PWM3_PORT, PWM3_BIT)
#define PWM3_GPIOREG __gpioreg__(PWM3_PORT)
#if (PWM3_BIT < 16)
#define PWM3_PINHALF L
#else
#define PWM3_PINHALF H
#endif
#define PWM3_PINCON __pincon__(PWM3_PORT, PWM3_PINHALF)
#define DIO27 27
#define DIO27_MBED_PIN PWM3_MBED_PIN
#define DIO27_PORT PWM3_PORT
#define DIO27_BIT PWM3_BIT
#define DIO27_GPIOREG PWM3_GPIOREG
#define DIO27_PINHALF PWM3_PINHALF
#define DIO27_PINCON PWM3_PINCON
#endif
#if (defined(PWM4_PORT) && defined(PWM4_BIT))
#define PWM4 28
#define PWM4_MBED_PIN __mbedpin__(PWM4_PORT, PWM4_BIT)
#define PWM4_GPIOREG __gpioreg__(PWM4_PORT)
#if (PWM4_BIT < 16)
#define PWM4_PINHALF L
#else
#define PWM4_PINHALF H
#endif
#define PWM4_PINCON __pincon__(PWM4_PORT, PWM4_PINHALF)
#define DIO28 28
#define DIO28_MBED_PIN PWM4_MBED_PIN
#define DIO28_PORT PWM4_PORT
#define DIO28_BIT PWM4_BIT
#define DIO28_GPIOREG PWM4_GPIOREG
#define DIO28_PINHALF PWM4_PINHALF
#define DIO28_PINCON PWM4_PINCON
#endif
#if (defined(PWM5_PORT) && defined(PWM5_BIT))
#define PWM5 29
#define PWM5_MBED_PIN __mbedpin__(PWM5_PORT, PWM5_BIT)
#define PWM5_GPIOREG __gpioreg__(PWM5_PORT)
#if (PWM5_BIT < 16)
#define PWM5_PINHALF L
#else
#define PWM5_PINHALF H
#endif
#define PWM5_PINCON __pincon__(PWM5_PORT, PWM5_PINHALF)
#define DIO29 29
#define DIO29_MBED_PIN PWM5_MBED_PIN
#define DIO29_PORT PWM5_PORT
#define DIO29_BIT PWM5_BIT
#define DIO29_GPIOREG PWM5_GPIOREG
#define DIO29_PINHALF PWM5_PINHALF
#define DIO29_PINCON PWM5_PINCON
#endif
#if (defined(PWM6_PORT) && defined(PWM6_BIT))
#define PWM6 30
#define PWM6_MBED_PIN __mbedpin__(PWM6_PORT, PWM6_BIT)
#define PWM6_GPIOREG __gpioreg__(PWM6_PORT)
#if (PWM6_BIT < 16)
#define PWM6_PINHALF L
#else
#define PWM6_PINHALF H
#endif
#define PWM6_PINCON __pincon__(PWM6_PORT, PWM6_PINHALF)
#define DIO30 30
#define DIO30_MBED_PIN PWM6_MBED_PIN
#define DIO30_PORT PWM6_PORT
#define DIO30_BIT PWM6_BIT
#define DIO30_GPIOREG PWM6_GPIOREG
#define DIO30_PINHALF PWM6_PINHALF
#define DIO30_PINCON PWM6_PINCON
#endif
#if (defined(PWM7_PORT) && defined(PWM7_BIT))
#define PWM7 31
#define PWM7_MBED_PIN __mbedpin__(PWM7_PORT, PWM7_BIT)
#define PWM7_GPIOREG __gpioreg__(PWM7_PORT)
#if (PWM7_BIT < 16)
#define PWM7_PINHALF L
#else
#define PWM7_PINHALF H
#endif
#define PWM7_PINCON __pincon__(PWM7_PORT, PWM7_PINHALF)
#define DIO31 31
#define DIO31_MBED_PIN PWM7_MBED_PIN
#define DIO31_PORT PWM7_PORT
#define DIO31_BIT PWM7_BIT
#define DIO31_GPIOREG PWM7_GPIOREG
#define DIO31_PINHALF PWM7_PINHALF
#define DIO31_PINCON PWM7_PINCON
#endif
#if (defined(PWM8_PORT) && defined(PWM8_BIT))
#define PWM8 32
#define PWM8_MBED_PIN __mbedpin__(PWM8_PORT, PWM8_BIT)
#define PWM8_GPIOREG __gpioreg__(PWM8_PORT)
#if (PWM8_BIT < 16)
#define PWM8_PINHALF L
#else
#define PWM8_PINHALF H
#endif
#define PWM8_PINCON __pincon__(PWM8_PORT, PWM8_PINHALF)
#define DIO32 32
#define DIO32_MBED_PIN PWM8_MBED_PIN
#define DIO32_PORT PWM8_PORT
#define DIO32_BIT PWM8_BIT
#define DIO32_GPIOREG PWM8_GPIOREG
#define DIO32_PINHALF PWM8_PINHALF
#define DIO32_PINCON PWM8_PINCON
#endif
#if (defined(PWM9_PORT) && defined(PWM9_BIT))
#define PWM9 33
#define PWM9_MBED_PIN __mbedpin__(PWM9_PORT, PWM9_BIT)
#define PWM9_GPIOREG __gpioreg__(PWM9_PORT)
#if (PWM9_BIT < 16)
#define PWM9_PINHALF L
#else
#define PWM9_PINHALF H
#endif
#define PWM9_PINCON __pincon__(PWM9_PORT, PWM9_PINHALF)
#define DIO33 33
#define DIO33_MBED_PIN PWM9_MBED_PIN
#define DIO33_PORT PWM9_PORT
#define DIO33_BIT PWM9_BIT
#define DIO33_GPIOREG PWM9_GPIOREG
#define DIO33_PINHALF PWM9_PINHALF
#define DIO33_PINCON PWM9_PINCON
#endif
#if (defined(PWM10_PORT) && defined(PWM10_BIT))
#define PWM10 34
#define PWM10_MBED_PIN __mbedpin__(PWM10_PORT, PWM10_BIT)
#define PWM10_GPIOREG __gpioreg__(PWM10_PORT)
#if (PWM10_BIT < 16)
#define PWM10_PINHALF L
#else
#define PWM10_PINHALF H
#endif
#define PWM10_PINCON __pincon__(PWM10_PORT, PWM10_PINHALF)
#define DIO34 34
#define DIO34_MBED_PIN PWM10_MBED_PIN
#define DIO34_PORT PWM10_PORT
#define DIO34_BIT PWM10_BIT
#define DIO34_GPIOREG PWM10_GPIOREG
#define DIO34_PINHALF PWM10_PINHALF
#define DIO34_PINCON PWM10_PINCON
#endif
#if (defined(PWM11_PORT) && defined(PWM11_BIT))
#define PWM11 35
#define PWM11_MBED_PIN __mbedpin__(PWM11_PORT, PWM11_BIT)
#define PWM11_GPIOREG __gpioreg__(PWM11_PORT)
#if (PWM11_BIT < 16)
#define PWM11_PINHALF L
#else
#define PWM11_PINHALF H
#endif
#define PWM11_PINCON __pincon__(PWM11_PORT, PWM11_PINHALF)
#define DIO35 35
#define DIO35_MBED_PIN PWM11_MBED_PIN
#define DIO35_PORT PWM11_PORT
#define DIO35_BIT PWM11_BIT
#define DIO35_GPIOREG PWM11_GPIOREG
#define DIO35_PINHALF PWM11_PINHALF
#define DIO35_PINCON PWM11_PINCON
#endif
#if (defined(PWM12_PORT) && defined(PWM12_BIT))
#define PWM12 36
#define PWM12_MBED_PIN __mbedpin__(PWM12_PORT, PWM12_BIT)
#define PWM12_GPIOREG __gpioreg__(PWM12_PORT)
#if (PWM12_BIT < 16)
#define PWM12_PINHALF L
#else
#define PWM12_PINHALF H
#endif
#define PWM12_PINCON __pincon__(PWM12_PORT, PWM12_PINHALF)
#define DIO36 36
#define DIO36_MBED_PIN PWM12_MBED_PIN
#define DIO36_PORT PWM12_PORT
#define DIO36_BIT PWM12_BIT
#define DIO36_GPIOREG PWM12_GPIOREG
#define DIO36_PINHALF PWM12_PINHALF
#define DIO36_PINCON PWM12_PINCON
#endif
#if (defined(PWM13_PORT) && defined(PWM13_BIT))
#define PWM13 37
#define PWM13_MBED_PIN __mbedpin__(PWM13_PORT, PWM13_BIT)
#define PWM13_GPIOREG __gpioreg__(PWM13_PORT)
#if (PWM13_BIT < 16)
#define PWM13_PINHALF L
#else
#define PWM13_PINHALF H
#endif
#define PWM13_PINCON __pincon__(PWM13_PORT, PWM13_PINHALF)
#define DIO37 37
#define DIO37_MBED_PIN PWM13_MBED_PIN
#define DIO37_PORT PWM13_PORT
#define DIO37_BIT PWM13_BIT
#define DIO37_GPIOREG PWM13_GPIOREG
#define DIO37_PINHALF PWM13_PINHALF
#define DIO37_PINCON PWM13_PINCON
#endif
#if (defined(PWM14_PORT) && defined(PWM14_BIT))
#define PWM14 38
#define PWM14_MBED_PIN __mbedpin__(PWM14_PORT, PWM14_BIT)
#define PWM14_GPIOREG __gpioreg__(PWM14_PORT)
#if (PWM14_BIT < 16)
#define PWM14_PINHALF L
#else
#define PWM14_PINHALF H
#endif
#define PWM14_PINCON __pincon__(PWM14_PORT, PWM14_PINHALF)
#define DIO38 38
#define DIO38_MBED_PIN PWM14_MBED_PIN
#define DIO38_PORT PWM14_PORT
#define DIO38_BIT PWM14_BIT
#define DIO38_GPIOREG PWM14_GPIOREG
#define DIO38_PINHALF PWM14_PINHALF
#define DIO38_PINCON PWM14_PINCON
#endif
#if (defined(PWM15_PORT) && defined(PWM15_BIT))
#define PWM15 39
#define PWM15_MBED_PIN __mbedpin__(PWM15_PORT, PWM15_BIT)
#define PWM15_GPIOREG __gpioreg__(PWM15_PORT)
#if (PWM15_BIT < 16)
#define PWM15_PINHALF L
#else
#define PWM15_PINHALF H
#endif
#define PWM15_PINCON __pincon__(PWM15_PORT, PWM15_PINHALF)
#define DIO39 39
#define DIO39_MBED_PIN PWM15_MBED_PIN
#define DIO39_PORT PWM15_PORT
#define DIO39_BIT PWM15_BIT
#define DIO39_GPIOREG PWM15_GPIOREG
#define DIO39_PINHALF PWM15_PINHALF
#define DIO39_PINCON PWM15_PINCON
#endif
#if (defined(SERVO0_PORT) && defined(SERVO0_BIT))
#define SERVO0 40
#define SERVO0_MBED_PIN __mbedpin__(SERVO0_PORT, SERVO0_BIT)
#define SERVO0_GPIOREG __gpioreg__(SERVO0_PORT)
#if (SERVO0_BIT < 16)
#define SERVO0_PINHALF L
#else
#define SERVO0_PINHALF H
#endif
#define SERVO0_PINCON __pincon__(SERVO0_PORT, SERVO0_PINHALF)
#define DIO40 40
#define DIO40_MBED_PIN SERVO0_MBED_PIN
#define DIO40_PORT SERVO0_PORT
#define DIO40_BIT SERVO0_BIT
#define DIO40_GPIOREG SERVO0_GPIOREG
#define DIO40_PINHALF SERVO0_PINHALF
#define DIO40_PINCON SERVO0_PINCON
#endif
#if (defined(SERVO1_PORT) && defined(SERVO1_BIT))
#define SERVO1 41
#define SERVO1_MBED_PIN __mbedpin__(SERVO1_PORT, SERVO1_BIT)
#define SERVO1_GPIOREG __gpioreg__(SERVO1_PORT)
#if (SERVO1_BIT < 16)
#define SERVO1_PINHALF L
#else
#define SERVO1_PINHALF H
#endif
#define SERVO1_PINCON __pincon__(SERVO1_PORT, SERVO1_PINHALF)
#define DIO41 41
#define DIO41_MBED_PIN SERVO1_MBED_PIN
#define DIO41_PORT SERVO1_PORT
#define DIO41_BIT SERVO1_BIT
#define DIO41_GPIOREG SERVO1_GPIOREG
#define DIO41_PINHALF SERVO1_PINHALF
#define DIO41_PINCON SERVO1_PINCON
#endif
#if (defined(SERVO2_PORT) && defined(SERVO2_BIT))
#define SERVO2 42
#define SERVO2_MBED_PIN __mbedpin__(SERVO2_PORT, SERVO2_BIT)
#define SERVO2_GPIOREG __gpioreg__(SERVO2_PORT)
#if (SERVO2_BIT < 16)
#define SERVO2_PINHALF L
#else
#define SERVO2_PINHALF H
#endif
#define SERVO2_PINCON __pincon__(SERVO2_PORT, SERVO2_PINHALF)
#define DIO42 42
#define DIO42_MBED_PIN SERVO2_MBED_PIN
#define DIO42_PORT SERVO2_PORT
#define DIO42_BIT SERVO2_BIT
#define DIO42_GPIOREG SERVO2_GPIOREG
#define DIO42_PINHALF SERVO2_PINHALF
#define DIO42_PINCON SERVO2_PINCON
#endif
#if (defined(SERVO3_PORT) && defined(SERVO3_BIT))
#define SERVO3 43
#define SERVO3_MBED_PIN __mbedpin__(SERVO3_PORT, SERVO3_BIT)
#define SERVO3_GPIOREG __gpioreg__(SERVO3_PORT)
#if (SERVO3_BIT < 16)
#define SERVO3_PINHALF L
#else
#define SERVO3_PINHALF H
#endif
#define SERVO3_PINCON __pincon__(SERVO3_PORT, SERVO3_PINHALF)
#define DIO43 43
#define DIO43_MBED_PIN SERVO3_MBED_PIN
#define DIO43_PORT SERVO3_PORT
#define DIO43_BIT SERVO3_BIT
#define DIO43_GPIOREG SERVO3_GPIOREG
#define DIO43_PINHALF SERVO3_PINHALF
#define DIO43_PINCON SERVO3_PINCON
#endif
#if (defined(SERVO4_PORT) && defined(SERVO4_BIT))
#define SERVO4 44
#define SERVO4_MBED_PIN __mbedpin__(SERVO4_PORT, SERVO4_BIT)
#define SERVO4_GPIOREG __gpioreg__(SERVO4_PORT)
#if (SERVO4_BIT < 16)
#define SERVO4_PINHALF L
#else
#define SERVO4_PINHALF H
#endif
#define SERVO4_PINCON __pincon__(SERVO4_PORT, SERVO4_PINHALF)
#define DIO44 44
#define DIO44_MBED_PIN SERVO4_MBED_PIN
#define DIO44_PORT SERVO4_PORT
#define DIO44_BIT SERVO4_BIT
#define DIO44_GPIOREG SERVO4_GPIOREG
#define DIO44_PINHALF SERVO4_PINHALF
#define DIO44_PINCON SERVO4_PINCON
#endif
#if (defined(SERVO5_PORT) && defined(SERVO5_BIT))
#define SERVO5 45
#define SERVO5_MBED_PIN __mbedpin__(SERVO5_PORT, SERVO5_BIT)
#define SERVO5_GPIOREG __gpioreg__(SERVO5_PORT)
#if (SERVO5_BIT < 16)
#define SERVO5_PINHALF L
#else
#define SERVO5_PINHALF H
#endif
#define SERVO5_PINCON __pincon__(SERVO5_PORT, SERVO5_PINHALF)
#define DIO45 45
#define DIO45_MBED_PIN SERVO5_MBED_PIN
#define DIO45_PORT SERVO5_PORT
#define DIO45_BIT SERVO5_BIT
#define DIO45_GPIOREG SERVO5_GPIOREG
#define DIO45_PINHALF SERVO5_PINHALF
#define DIO45_PINCON SERVO5_PINCON
#endif
#if (defined(DOUT0_PORT) && defined(DOUT0_BIT))
#define DOUT0 46
#define DOUT0_MBED_PIN __mbedpin__(DOUT0_PORT, DOUT0_BIT)
#define DOUT0_GPIOREG __gpioreg__(DOUT0_PORT)
#if (DOUT0_BIT < 16)
#define DOUT0_PINHALF L
#else
#define DOUT0_PINHALF H
#endif
#define DOUT0_PINCON __pincon__(DOUT0_PORT, DOUT0_PINHALF)
#define DIO46 46
#define DIO46_MBED_PIN DOUT0_MBED_PIN
#define DIO46_PORT DOUT0_PORT
#define DIO46_BIT DOUT0_BIT
#define DIO46_GPIOREG DOUT0_GPIOREG
#define DIO46_PINHALF DOUT0_PINHALF
#define DIO46_PINCON DOUT0_PINCON
#endif
#if (defined(DOUT1_PORT) && defined(DOUT1_BIT))
#define DOUT1 47
#define DOUT1_MBED_PIN __mbedpin__(DOUT1_PORT, DOUT1_BIT)
#define DOUT1_GPIOREG __gpioreg__(DOUT1_PORT)
#if (DOUT1_BIT < 16)
#define DOUT1_PINHALF L
#else
#define DOUT1_PINHALF H
#endif
#define DOUT1_PINCON __pincon__(DOUT1_PORT, DOUT1_PINHALF)
#define DIO47 47
#define DIO47_MBED_PIN DOUT1_MBED_PIN
#define DIO47_PORT DOUT1_PORT
#define DIO47_BIT DOUT1_BIT
#define DIO47_GPIOREG DOUT1_GPIOREG
#define DIO47_PINHALF DOUT1_PINHALF
#define DIO47_PINCON DOUT1_PINCON
#endif
#if (defined(DOUT2_PORT) && defined(DOUT2_BIT))
#define DOUT2 48
#define DOUT2_MBED_PIN __mbedpin__(DOUT2_PORT, DOUT2_BIT)
#define DOUT2_GPIOREG __gpioreg__(DOUT2_PORT)
#if (DOUT2_BIT < 16)
#define DOUT2_PINHALF L
#else
#define DOUT2_PINHALF H
#endif
#define DOUT2_PINCON __pincon__(DOUT2_PORT, DOUT2_PINHALF)
#define DIO48 48
#define DIO48_MBED_PIN DOUT2_MBED_PIN
#define DIO48_PORT DOUT2_PORT
#define DIO48_BIT DOUT2_BIT
#define DIO48_GPIOREG DOUT2_GPIOREG
#define DIO48_PINHALF DOUT2_PINHALF
#define DIO48_PINCON DOUT2_PINCON
#endif
#if (defined(DOUT3_PORT) && defined(DOUT3_BIT))
#define DOUT3 49
#define DOUT3_MBED_PIN __mbedpin__(DOUT3_PORT, DOUT3_BIT)
#define DOUT3_GPIOREG __gpioreg__(DOUT3_PORT)
#if (DOUT3_BIT < 16)
#define DOUT3_PINHALF L
#else
#define DOUT3_PINHALF H
#endif
#define DOUT3_PINCON __pincon__(DOUT3_PORT, DOUT3_PINHALF)
#define DIO49 49
#define DIO49_MBED_PIN DOUT3_MBED_PIN
#define DIO49_PORT DOUT3_PORT
#define DIO49_BIT DOUT3_BIT
#define DIO49_GPIOREG DOUT3_GPIOREG
#define DIO49_PINHALF DOUT3_PINHALF
#define DIO49_PINCON DOUT3_PINCON
#endif
#if (defined(DOUT4_PORT) && defined(DOUT4_BIT))
#define DOUT4 50
#define DOUT4_MBED_PIN __mbedpin__(DOUT4_PORT, DOUT4_BIT)
#define DOUT4_GPIOREG __gpioreg__(DOUT4_PORT)
#if (DOUT4_BIT < 16)
#define DOUT4_PINHALF L
#else
#define DOUT4_PINHALF H
#endif
#define DOUT4_PINCON __pincon__(DOUT4_PORT, DOUT4_PINHALF)
#define DIO50 50
#define DIO50_MBED_PIN DOUT4_MBED_PIN
#define DIO50_PORT DOUT4_PORT
#define DIO50_BIT DOUT4_BIT
#define DIO50_GPIOREG DOUT4_GPIOREG
#define DIO50_PINHALF DOUT4_PINHALF
#define DIO50_PINCON DOUT4_PINCON
#endif
#if (defined(DOUT5_PORT) && defined(DOUT5_BIT))
#define DOUT5 51
#define DOUT5_MBED_PIN __mbedpin__(DOUT5_PORT, DOUT5_BIT)
#define DOUT5_GPIOREG __gpioreg__(DOUT5_PORT)
#if (DOUT5_BIT < 16)
#define DOUT5_PINHALF L
#else
#define DOUT5_PINHALF H
#endif
#define DOUT5_PINCON __pincon__(DOUT5_PORT, DOUT5_PINHALF)
#define DIO51 51
#define DIO51_MBED_PIN DOUT5_MBED_PIN
#define DIO51_PORT DOUT5_PORT
#define DIO51_BIT DOUT5_BIT
#define DIO51_GPIOREG DOUT5_GPIOREG
#define DIO51_PINHALF DOUT5_PINHALF
#define DIO51_PINCON DOUT5_PINCON
#endif
#if (defined(DOUT6_PORT) && defined(DOUT6_BIT))
#define DOUT6 52
#define DOUT6_MBED_PIN __mbedpin__(DOUT6_PORT, DOUT6_BIT)
#define DOUT6_GPIOREG __gpioreg__(DOUT6_PORT)
#if (DOUT6_BIT < 16)
#define DOUT6_PINHALF L
#else
#define DOUT6_PINHALF H
#endif
#define DOUT6_PINCON __pincon__(DOUT6_PORT, DOUT6_PINHALF)
#define DIO52 52
#define DIO52_MBED_PIN DOUT6_MBED_PIN
#define DIO52_PORT DOUT6_PORT
#define DIO52_BIT DOUT6_BIT
#define DIO52_GPIOREG DOUT6_GPIOREG
#define DIO52_PINHALF DOUT6_PINHALF
#define DIO52_PINCON DOUT6_PINCON
#endif
#if (defined(DOUT7_PORT) && defined(DOUT7_BIT))
#define DOUT7 53
#define DOUT7_MBED_PIN __mbedpin__(DOUT7_PORT, DOUT7_BIT)
#define DOUT7_GPIOREG __gpioreg__(DOUT7_PORT)
#if (DOUT7_BIT < 16)
#define DOUT7_PINHALF L
#else
#define DOUT7_PINHALF H
#endif
#define DOUT7_PINCON __pincon__(DOUT7_PORT, DOUT7_PINHALF)
#define DIO53 53
#define DIO53_MBED_PIN DOUT7_MBED_PIN
#define DIO53_PORT DOUT7_PORT
#define DIO53_BIT DOUT7_BIT
#define DIO53_GPIOREG DOUT7_GPIOREG
#define DIO53_PINHALF DOUT7_PINHALF
#define DIO53_PINCON DOUT7_PINCON
#endif
#if (defined(DOUT8_PORT) && defined(DOUT8_BIT))
#define DOUT8 54
#define DOUT8_MBED_PIN __mbedpin__(DOUT8_PORT, DOUT8_BIT)
#define DOUT8_GPIOREG __gpioreg__(DOUT8_PORT)
#if (DOUT8_BIT < 16)
#define DOUT8_PINHALF L
#else
#define DOUT8_PINHALF H
#endif
#define DOUT8_PINCON __pincon__(DOUT8_PORT, DOUT8_PINHALF)
#define DIO54 54
#define DIO54_MBED_PIN DOUT8_MBED_PIN
#define DIO54_PORT DOUT8_PORT
#define DIO54_BIT DOUT8_BIT
#define DIO54_GPIOREG DOUT8_GPIOREG
#define DIO54_PINHALF DOUT8_PINHALF
#define DIO54_PINCON DOUT8_PINCON
#endif
#if (defined(DOUT9_PORT) && defined(DOUT9_BIT))
#define DOUT9 55
#define DOUT9_MBED_PIN __mbedpin__(DOUT9_PORT, DOUT9_BIT)
#define DOUT9_GPIOREG __gpioreg__(DOUT9_PORT)
#if (DOUT9_BIT < 16)
#define DOUT9_PINHALF L
#else
#define DOUT9_PINHALF H
#endif
#define DOUT9_PINCON __pincon__(DOUT9_PORT, DOUT9_PINHALF)
#define DIO55 55
#define DIO55_MBED_PIN DOUT9_MBED_PIN
#define DIO55_PORT DOUT9_PORT
#define DIO55_BIT DOUT9_BIT
#define DIO55_GPIOREG DOUT9_GPIOREG
#define DIO55_PINHALF DOUT9_PINHALF
#define DIO55_PINCON DOUT9_PINCON
#endif
#if (defined(DOUT10_PORT) && defined(DOUT10_BIT))
#define DOUT10 56
#define DOUT10_MBED_PIN __mbedpin__(DOUT10_PORT, DOUT10_BIT)
#define DOUT10_GPIOREG __gpioreg__(DOUT10_PORT)
#if (DOUT10_BIT < 16)
#define DOUT10_PINHALF L
#else
#define DOUT10_PINHALF H
#endif
#define DOUT10_PINCON __pincon__(DOUT10_PORT, DOUT10_PINHALF)
#define DIO56 56
#define DIO56_MBED_PIN DOUT10_MBED_PIN
#define DIO56_PORT DOUT10_PORT
#define DIO56_BIT DOUT10_BIT
#define DIO56_GPIOREG DOUT10_GPIOREG
#define DIO56_PINHALF DOUT10_PINHALF
#define DIO56_PINCON DOUT10_PINCON
#endif
#if (defined(DOUT11_PORT) && defined(DOUT11_BIT))
#define DOUT11 57
#define DOUT11_MBED_PIN __mbedpin__(DOUT11_PORT, DOUT11_BIT)
#define DOUT11_GPIOREG __gpioreg__(DOUT11_PORT)
#if (DOUT11_BIT < 16)
#define DOUT11_PINHALF L
#else
#define DOUT11_PINHALF H
#endif
#define DOUT11_PINCON __pincon__(DOUT11_PORT, DOUT11_PINHALF)
#define DIO57 57
#define DIO57_MBED_PIN DOUT11_MBED_PIN
#define DIO57_PORT DOUT11_PORT
#define DIO57_BIT DOUT11_BIT
#define DIO57_GPIOREG DOUT11_GPIOREG
#define DIO57_PINHALF DOUT11_PINHALF
#define DIO57_PINCON DOUT11_PINCON
#endif
#if (defined(DOUT12_PORT) && defined(DOUT12_BIT))
#define DOUT12 58
#define DOUT12_MBED_PIN __mbedpin__(DOUT12_PORT, DOUT12_BIT)
#define DOUT12_GPIOREG __gpioreg__(DOUT12_PORT)
#if (DOUT12_BIT < 16)
#define DOUT12_PINHALF L
#else
#define DOUT12_PINHALF H
#endif
#define DOUT12_PINCON __pincon__(DOUT12_PORT, DOUT12_PINHALF)
#define DIO58 58
#define DIO58_MBED_PIN DOUT12_MBED_PIN
#define DIO58_PORT DOUT12_PORT
#define DIO58_BIT DOUT12_BIT
#define DIO58_GPIOREG DOUT12_GPIOREG
#define DIO58_PINHALF DOUT12_PINHALF
#define DIO58_PINCON DOUT12_PINCON
#endif
#if (defined(DOUT13_PORT) && defined(DOUT13_BIT))
#define DOUT13 59
#define DOUT13_MBED_PIN __mbedpin__(DOUT13_PORT, DOUT13_BIT)
#define DOUT13_GPIOREG __gpioreg__(DOUT13_PORT)
#if (DOUT13_BIT < 16)
#define DOUT13_PINHALF L
#else
#define DOUT13_PINHALF H
#endif
#define DOUT13_PINCON __pincon__(DOUT13_PORT, DOUT13_PINHALF)
#define DIO59 59
#define DIO59_MBED_PIN DOUT13_MBED_PIN
#define DIO59_PORT DOUT13_PORT
#define DIO59_BIT DOUT13_BIT
#define DIO59_GPIOREG DOUT13_GPIOREG
#define DIO59_PINHALF DOUT13_PINHALF
#define DIO59_PINCON DOUT13_PINCON
#endif
#if (defined(DOUT14_PORT) && defined(DOUT14_BIT))
#define DOUT14 60
#define DOUT14_MBED_PIN __mbedpin__(DOUT14_PORT, DOUT14_BIT)
#define DOUT14_GPIOREG __gpioreg__(DOUT14_PORT)
#if (DOUT14_BIT < 16)
#define DOUT14_PINHALF L
#else
#define DOUT14_PINHALF H
#endif
#define DOUT14_PINCON __pincon__(DOUT14_PORT, DOUT14_PINHALF)
#define DIO60 60
#define DIO60_MBED_PIN DOUT14_MBED_PIN
#define DIO60_PORT DOUT14_PORT
#define DIO60_BIT DOUT14_BIT
#define DIO60_GPIOREG DOUT14_GPIOREG
#define DIO60_PINHALF DOUT14_PINHALF
#define DIO60_PINCON DOUT14_PINCON
#endif
#if (defined(DOUT15_PORT) && defined(DOUT15_BIT))
#define DOUT15 61
#define DOUT15_MBED_PIN __mbedpin__(DOUT15_PORT, DOUT15_BIT)
#define DOUT15_GPIOREG __gpioreg__(DOUT15_PORT)
#if (DOUT15_BIT < 16)
#define DOUT15_PINHALF L
#else
#define DOUT15_PINHALF H
#endif
#define DOUT15_PINCON __pincon__(DOUT15_PORT, DOUT15_PINHALF)
#define DIO61 61
#define DIO61_MBED_PIN DOUT15_MBED_PIN
#define DIO61_PORT DOUT15_PORT
#define DIO61_BIT DOUT15_BIT
#define DIO61_GPIOREG DOUT15_GPIOREG
#define DIO61_PINHALF DOUT15_PINHALF
#define DIO61_PINCON DOUT15_PINCON
#endif
#if (defined(DOUT16_PORT) && defined(DOUT16_BIT))
#define DOUT16 62
#define DOUT16_MBED_PIN __mbedpin__(DOUT16_PORT, DOUT16_BIT)
#define DOUT16_GPIOREG __gpioreg__(DOUT16_PORT)
#if (DOUT16_BIT < 16)
#define DOUT16_PINHALF L
#else
#define DOUT16_PINHALF H
#endif
#define DOUT16_PINCON __pincon__(DOUT16_PORT, DOUT16_PINHALF)
#define DIO62 62
#define DIO62_MBED_PIN DOUT16_MBED_PIN
#define DIO62_PORT DOUT16_PORT
#define DIO62_BIT DOUT16_BIT
#define DIO62_GPIOREG DOUT16_GPIOREG
#define DIO62_PINHALF DOUT16_PINHALF
#define DIO62_PINCON DOUT16_PINCON
#endif
#if (defined(DOUT17_PORT) && defined(DOUT17_BIT))
#define DOUT17 63
#define DOUT17_MBED_PIN __mbedpin__(DOUT17_PORT, DOUT17_BIT)
#define DOUT17_GPIOREG __gpioreg__(DOUT17_PORT)
#if (DOUT17_BIT < 16)
#define DOUT17_PINHALF L
#else
#define DOUT17_PINHALF H
#endif
#define DOUT17_PINCON __pincon__(DOUT17_PORT, DOUT17_PINHALF)
#define DIO63 63
#define DIO63_MBED_PIN DOUT17_MBED_PIN
#define DIO63_PORT DOUT17_PORT
#define DIO63_BIT DOUT17_BIT
#define DIO63_GPIOREG DOUT17_GPIOREG
#define DIO63_PINHALF DOUT17_PINHALF
#define DIO63_PINCON DOUT17_PINCON
#endif
#if (defined(DOUT18_PORT) && defined(DOUT18_BIT))
#define DOUT18 64
#define DOUT18_MBED_PIN __mbedpin__(DOUT18_PORT, DOUT18_BIT)
#define DOUT18_GPIOREG __gpioreg__(DOUT18_PORT)
#if (DOUT18_BIT < 16)
#define DOUT18_PINHALF L
#else
#define DOUT18_PINHALF H
#endif
#define DOUT18_PINCON __pincon__(DOUT18_PORT, DOUT18_PINHALF)
#define DIO64 64
#define DIO64_MBED_PIN DOUT18_MBED_PIN
#define DIO64_PORT DOUT18_PORT
#define DIO64_BIT DOUT18_BIT
#define DIO64_GPIOREG DOUT18_GPIOREG
#define DIO64_PINHALF DOUT18_PINHALF
#define DIO64_PINCON DOUT18_PINCON
#endif
#if (defined(DOUT19_PORT) && defined(DOUT19_BIT))
#define DOUT19 65
#define DOUT19_MBED_PIN __mbedpin__(DOUT19_PORT, DOUT19_BIT)
#define DOUT19_GPIOREG __gpioreg__(DOUT19_PORT)
#if (DOUT19_BIT < 16)
#define DOUT19_PINHALF L
#else
#define DOUT19_PINHALF H
#endif
#define DOUT19_PINCON __pincon__(DOUT19_PORT, DOUT19_PINHALF)
#define DIO65 65
#define DIO65_MBED_PIN DOUT19_MBED_PIN
#define DIO65_PORT DOUT19_PORT
#define DIO65_BIT DOUT19_BIT
#define DIO65_GPIOREG DOUT19_GPIOREG
#define DIO65_PINHALF DOUT19_PINHALF
#define DIO65_PINCON DOUT19_PINCON
#endif
#if (defined(DOUT20_PORT) && defined(DOUT20_BIT))
#define DOUT20 66
#define DOUT20_MBED_PIN __mbedpin__(DOUT20_PORT, DOUT20_BIT)
#define DOUT20_GPIOREG __gpioreg__(DOUT20_PORT)
#if (DOUT20_BIT < 16)
#define DOUT20_PINHALF L
#else
#define DOUT20_PINHALF H
#endif
#define DOUT20_PINCON __pincon__(DOUT20_PORT, DOUT20_PINHALF)
#define DIO66 66
#define DIO66_MBED_PIN DOUT20_MBED_PIN
#define DIO66_PORT DOUT20_PORT
#define DIO66_BIT DOUT20_BIT
#define DIO66_GPIOREG DOUT20_GPIOREG
#define DIO66_PINHALF DOUT20_PINHALF
#define DIO66_PINCON DOUT20_PINCON
#endif
#if (defined(DOUT21_PORT) && defined(DOUT21_BIT))
#define DOUT21 67
#define DOUT21_MBED_PIN __mbedpin__(DOUT21_PORT, DOUT21_BIT)
#define DOUT21_GPIOREG __gpioreg__(DOUT21_PORT)
#if (DOUT21_BIT < 16)
#define DOUT21_PINHALF L
#else
#define DOUT21_PINHALF H
#endif
#define DOUT21_PINCON __pincon__(DOUT21_PORT, DOUT21_PINHALF)
#define DIO67 67
#define DIO67_MBED_PIN DOUT21_MBED_PIN
#define DIO67_PORT DOUT21_PORT
#define DIO67_BIT DOUT21_BIT
#define DIO67_GPIOREG DOUT21_GPIOREG
#define DIO67_PINHALF DOUT21_PINHALF
#define DIO67_PINCON DOUT21_PINCON
#endif
#if (defined(DOUT22_PORT) && defined(DOUT22_BIT))
#define DOUT22 68
#define DOUT22_MBED_PIN __mbedpin__(DOUT22_PORT, DOUT22_BIT)
#define DOUT22_GPIOREG __gpioreg__(DOUT22_PORT)
#if (DOUT22_BIT < 16)
#define DOUT22_PINHALF L
#else
#define DOUT22_PINHALF H
#endif
#define DOUT22_PINCON __pincon__(DOUT22_PORT, DOUT22_PINHALF)
#define DIO68 68
#define DIO68_MBED_PIN DOUT22_MBED_PIN
#define DIO68_PORT DOUT22_PORT
#define DIO68_BIT DOUT22_BIT
#define DIO68_GPIOREG DOUT22_GPIOREG
#define DIO68_PINHALF DOUT22_PINHALF
#define DIO68_PINCON DOUT22_PINCON
#endif
#if (defined(DOUT23_PORT) && defined(DOUT23_BIT))
#define DOUT23 69
#define DOUT23_MBED_PIN __mbedpin__(DOUT23_PORT, DOUT23_BIT)
#define DOUT23_GPIOREG __gpioreg__(DOUT23_PORT)
#if (DOUT23_BIT < 16)
#define DOUT23_PINHALF L
#else
#define DOUT23_PINHALF H
#endif
#define DOUT23_PINCON __pincon__(DOUT23_PORT, DOUT23_PINHALF)
#define DIO69 69
#define DIO69_MBED_PIN DOUT23_MBED_PIN
#define DIO69_PORT DOUT23_PORT
#define DIO69_BIT DOUT23_BIT
#define DIO69_GPIOREG DOUT23_GPIOREG
#define DIO69_PINHALF DOUT23_PINHALF
#define DIO69_PINCON DOUT23_PINCON
#endif
#if (defined(DOUT24_PORT) && defined(DOUT24_BIT))
#define DOUT24 70
#define DOUT24_MBED_PIN __mbedpin__(DOUT24_PORT, DOUT24_BIT)
#define DOUT24_GPIOREG __gpioreg__(DOUT24_PORT)
#if (DOUT24_BIT < 16)
#define DOUT24_PINHALF L
#else
#define DOUT24_PINHALF H
#endif
#define DOUT24_PINCON __pincon__(DOUT24_PORT, DOUT24_PINHALF)
#define DIO70 70
#define DIO70_MBED_PIN DOUT24_MBED_PIN
#define DIO70_PORT DOUT24_PORT
#define DIO70_BIT DOUT24_BIT
#define DIO70_GPIOREG DOUT24_GPIOREG
#define DIO70_PINHALF DOUT24_PINHALF
#define DIO70_PINCON DOUT24_PINCON
#endif
#if (defined(DOUT25_PORT) && defined(DOUT25_BIT))
#define DOUT25 71
#define DOUT25_MBED_PIN __mbedpin__(DOUT25_PORT, DOUT25_BIT)
#define DOUT25_GPIOREG __gpioreg__(DOUT25_PORT)
#if (DOUT25_BIT < 16)
#define DOUT25_PINHALF L
#else
#define DOUT25_PINHALF H
#endif
#define DOUT25_PINCON __pincon__(DOUT25_PORT, DOUT25_PINHALF)
#define DIO71 71
#define DIO71_MBED_PIN DOUT25_MBED_PIN
#define DIO71_PORT DOUT25_PORT
#define DIO71_BIT DOUT25_BIT
#define DIO71_GPIOREG DOUT25_GPIOREG
#define DIO71_PINHALF DOUT25_PINHALF
#define DIO71_PINCON DOUT25_PINCON
#endif
#if (defined(DOUT26_PORT) && defined(DOUT26_BIT))
#define DOUT26 72
#define DOUT26_MBED_PIN __mbedpin__(DOUT26_PORT, DOUT26_BIT)
#define DOUT26_GPIOREG __gpioreg__(DOUT26_PORT)
#if (DOUT26_BIT < 16)
#define DOUT26_PINHALF L
#else
#define DOUT26_PINHALF H
#endif
#define DOUT26_PINCON __pincon__(DOUT26_PORT, DOUT26_PINHALF)
#define DIO72 72
#define DIO72_MBED_PIN DOUT26_MBED_PIN
#define DIO72_PORT DOUT26_PORT
#define DIO72_BIT DOUT26_BIT
#define DIO72_GPIOREG DOUT26_GPIOREG
#define DIO72_PINHALF DOUT26_PINHALF
#define DIO72_PINCON DOUT26_PINCON
#endif
#if (defined(DOUT27_PORT) && defined(DOUT27_BIT))
#define DOUT27 73
#define DOUT27_MBED_PIN __mbedpin__(DOUT27_PORT, DOUT27_BIT)
#define DOUT27_GPIOREG __gpioreg__(DOUT27_PORT)
#if (DOUT27_BIT < 16)
#define DOUT27_PINHALF L
#else
#define DOUT27_PINHALF H
#endif
#define DOUT27_PINCON __pincon__(DOUT27_PORT, DOUT27_PINHALF)
#define DIO73 73
#define DIO73_MBED_PIN DOUT27_MBED_PIN
#define DIO73_PORT DOUT27_PORT
#define DIO73_BIT DOUT27_BIT
#define DIO73_GPIOREG DOUT27_GPIOREG
#define DIO73_PINHALF DOUT27_PINHALF
#define DIO73_PINCON DOUT27_PINCON
#endif
#if (defined(DOUT28_PORT) && defined(DOUT28_BIT))
#define DOUT28 74
#define DOUT28_MBED_PIN __mbedpin__(DOUT28_PORT, DOUT28_BIT)
#define DOUT28_GPIOREG __gpioreg__(DOUT28_PORT)
#if (DOUT28_BIT < 16)
#define DOUT28_PINHALF L
#else
#define DOUT28_PINHALF H
#endif
#define DOUT28_PINCON __pincon__(DOUT28_PORT, DOUT28_PINHALF)
#define DIO74 74
#define DIO74_MBED_PIN DOUT28_MBED_PIN
#define DIO74_PORT DOUT28_PORT
#define DIO74_BIT DOUT28_BIT
#define DIO74_GPIOREG DOUT28_GPIOREG
#define DIO74_PINHALF DOUT28_PINHALF
#define DIO74_PINCON DOUT28_PINCON
#endif
#if (defined(DOUT29_PORT) && defined(DOUT29_BIT))
#define DOUT29 75
#define DOUT29_MBED_PIN __mbedpin__(DOUT29_PORT, DOUT29_BIT)
#define DOUT29_GPIOREG __gpioreg__(DOUT29_PORT)
#if (DOUT29_BIT < 16)
#define DOUT29_PINHALF L
#else
#define DOUT29_PINHALF H
#endif
#define DOUT29_PINCON __pincon__(DOUT29_PORT, DOUT29_PINHALF)
#define DIO75 75
#define DIO75_MBED_PIN DOUT29_MBED_PIN
#define DIO75_PORT DOUT29_PORT
#define DIO75_BIT DOUT29_BIT
#define DIO75_GPIOREG DOUT29_GPIOREG
#define DIO75_PINHALF DOUT29_PINHALF
#define DIO75_PINCON DOUT29_PINCON
#endif
#if (defined(DOUT30_PORT) && defined(DOUT30_BIT))
#define DOUT30 76
#define DOUT30_MBED_PIN __mbedpin__(DOUT30_PORT, DOUT30_BIT)
#define DOUT30_GPIOREG __gpioreg__(DOUT30_PORT)
#if (DOUT30_BIT < 16)
#define DOUT30_PINHALF L
#else
#define DOUT30_PINHALF H
#endif
#define DOUT30_PINCON __pincon__(DOUT30_PORT, DOUT30_PINHALF)
#define DIO76 76
#define DIO76_MBED_PIN DOUT30_MBED_PIN
#define DIO76_PORT DOUT30_PORT
#define DIO76_BIT DOUT30_BIT
#define DIO76_GPIOREG DOUT30_GPIOREG
#define DIO76_PINHALF DOUT30_PINHALF
#define DIO76_PINCON DOUT30_PINCON
#endif
#if (defined(DOUT31_PORT) && defined(DOUT31_BIT))
#define DOUT31 77
#define DOUT31_MBED_PIN __mbedpin__(DOUT31_PORT, DOUT31_BIT)
#define DOUT31_GPIOREG __gpioreg__(DOUT31_PORT)
#if (DOUT31_BIT < 16)
#define DOUT31_PINHALF L
#else
#define DOUT31_PINHALF H
#endif
#define DOUT31_PINCON __pincon__(DOUT31_PORT, DOUT31_PINHALF)
#define DIO77 77
#define DIO77_MBED_PIN DOUT31_MBED_PIN
#define DIO77_PORT DOUT31_PORT
#define DIO77_BIT DOUT31_BIT
#define DIO77_GPIOREG DOUT31_GPIOREG
#define DIO77_PINHALF DOUT31_PINHALF
#define DIO77_PINCON DOUT31_PINCON
#endif
#if (defined(LIMIT_X_PORT) && defined(LIMIT_X_BIT))
#define LIMIT_X 100
#define LIMIT_X_MBED_PIN __mbedpin__(LIMIT_X_PORT, LIMIT_X_BIT)
#define LIMIT_X_GPIOREG __gpioreg__(LIMIT_X_PORT)
#if (LIMIT_X_BIT < 16)
#define LIMIT_X_PINHALF L
#else
#define LIMIT_X_PINHALF H
#endif
#define LIMIT_X_PINCON __pincon__(LIMIT_X_PORT, LIMIT_X_PINHALF)
#define DIO100 100
#define DIO100_MBED_PIN LIMIT_X_MBED_PIN
#define DIO100_PORT LIMIT_X_PORT
#define DIO100_BIT LIMIT_X_BIT
#define DIO100_GPIOREG LIMIT_X_GPIOREG
#define DIO100_PINHALF LIMIT_X_PINHALF
#define DIO100_PINCON LIMIT_X_PINCON
#endif
#if (defined(LIMIT_Y_PORT) && defined(LIMIT_Y_BIT))
#define LIMIT_Y 101
#define LIMIT_Y_MBED_PIN __mbedpin__(LIMIT_Y_PORT, LIMIT_Y_BIT)
#define LIMIT_Y_GPIOREG __gpioreg__(LIMIT_Y_PORT)
#if (LIMIT_Y_BIT < 16)
#define LIMIT_Y_PINHALF L
#else
#define LIMIT_Y_PINHALF H
#endif
#define LIMIT_Y_PINCON __pincon__(LIMIT_Y_PORT, LIMIT_Y_PINHALF)
#define DIO101 101
#define DIO101_MBED_PIN LIMIT_Y_MBED_PIN
#define DIO101_PORT LIMIT_Y_PORT
#define DIO101_BIT LIMIT_Y_BIT
#define DIO101_GPIOREG LIMIT_Y_GPIOREG
#define DIO101_PINHALF LIMIT_Y_PINHALF
#define DIO101_PINCON LIMIT_Y_PINCON
#endif
#if (defined(LIMIT_Z_PORT) && defined(LIMIT_Z_BIT))
#define LIMIT_Z 102
#define LIMIT_Z_MBED_PIN __mbedpin__(LIMIT_Z_PORT, LIMIT_Z_BIT)
#define LIMIT_Z_GPIOREG __gpioreg__(LIMIT_Z_PORT)
#if (LIMIT_Z_BIT < 16)
#define LIMIT_Z_PINHALF L
#else
#define LIMIT_Z_PINHALF H
#endif
#define LIMIT_Z_PINCON __pincon__(LIMIT_Z_PORT, LIMIT_Z_PINHALF)
#define DIO102 102
#define DIO102_MBED_PIN LIMIT_Z_MBED_PIN
#define DIO102_PORT LIMIT_Z_PORT
#define DIO102_BIT LIMIT_Z_BIT
#define DIO102_GPIOREG LIMIT_Z_GPIOREG
#define DIO102_PINHALF LIMIT_Z_PINHALF
#define DIO102_PINCON LIMIT_Z_PINCON
#endif
#if (defined(LIMIT_X2_PORT) && defined(LIMIT_X2_BIT))
#define LIMIT_X2 103
#define LIMIT_X2_MBED_PIN __mbedpin__(LIMIT_X2_PORT, LIMIT_X2_BIT)
#define LIMIT_X2_GPIOREG __gpioreg__(LIMIT_X2_PORT)
#if (LIMIT_X2_BIT < 16)
#define LIMIT_X2_PINHALF L
#else
#define LIMIT_X2_PINHALF H
#endif
#define LIMIT_X2_PINCON __pincon__(LIMIT_X2_PORT, LIMIT_X2_PINHALF)
#define DIO103 103
#define DIO103_MBED_PIN LIMIT_X2_MBED_PIN
#define DIO103_PORT LIMIT_X2_PORT
#define DIO103_BIT LIMIT_X2_BIT
#define DIO103_GPIOREG LIMIT_X2_GPIOREG
#define DIO103_PINHALF LIMIT_X2_PINHALF
#define DIO103_PINCON LIMIT_X2_PINCON
#endif
#if (defined(LIMIT_Y2_PORT) && defined(LIMIT_Y2_BIT))
#define LIMIT_Y2 104
#define LIMIT_Y2_MBED_PIN __mbedpin__(LIMIT_Y2_PORT, LIMIT_Y2_BIT)
#define LIMIT_Y2_GPIOREG __gpioreg__(LIMIT_Y2_PORT)
#if (LIMIT_Y2_BIT < 16)
#define LIMIT_Y2_PINHALF L
#else
#define LIMIT_Y2_PINHALF H
#endif
#define LIMIT_Y2_PINCON __pincon__(LIMIT_Y2_PORT, LIMIT_Y2_PINHALF)
#define DIO104 104
#define DIO104_MBED_PIN LIMIT_Y2_MBED_PIN
#define DIO104_PORT LIMIT_Y2_PORT
#define DIO104_BIT LIMIT_Y2_BIT
#define DIO104_GPIOREG LIMIT_Y2_GPIOREG
#define DIO104_PINHALF LIMIT_Y2_PINHALF
#define DIO104_PINCON LIMIT_Y2_PINCON
#endif
#if (defined(LIMIT_Z2_PORT) && defined(LIMIT_Z2_BIT))
#define LIMIT_Z2 105
#define LIMIT_Z2_MBED_PIN __mbedpin__(LIMIT_Z2_PORT, LIMIT_Z2_BIT)
#define LIMIT_Z2_GPIOREG __gpioreg__(LIMIT_Z2_PORT)
#if (LIMIT_Z2_BIT < 16)
#define LIMIT_Z2_PINHALF L
#else
#define LIMIT_Z2_PINHALF H
#endif
#define LIMIT_Z2_PINCON __pincon__(LIMIT_Z2_PORT, LIMIT_Z2_PINHALF)
#define DIO105 105
#define DIO105_MBED_PIN LIMIT_Z2_MBED_PIN
#define DIO105_PORT LIMIT_Z2_PORT
#define DIO105_BIT LIMIT_Z2_BIT
#define DIO105_GPIOREG LIMIT_Z2_GPIOREG
#define DIO105_PINHALF LIMIT_Z2_PINHALF
#define DIO105_PINCON LIMIT_Z2_PINCON
#endif
#if (defined(LIMIT_A_PORT) && defined(LIMIT_A_BIT))
#define LIMIT_A 106
#define LIMIT_A_MBED_PIN __mbedpin__(LIMIT_A_PORT, LIMIT_A_BIT)
#define LIMIT_A_GPIOREG __gpioreg__(LIMIT_A_PORT)
#if (LIMIT_A_BIT < 16)
#define LIMIT_A_PINHALF L
#else
#define LIMIT_A_PINHALF H
#endif
#define LIMIT_A_PINCON __pincon__(LIMIT_A_PORT, LIMIT_A_PINHALF)
#define DIO106 106
#define DIO106_MBED_PIN LIMIT_A_MBED_PIN
#define DIO106_PORT LIMIT_A_PORT
#define DIO106_BIT LIMIT_A_BIT
#define DIO106_GPIOREG LIMIT_A_GPIOREG
#define DIO106_PINHALF LIMIT_A_PINHALF
#define DIO106_PINCON LIMIT_A_PINCON
#endif
#if (defined(LIMIT_B_PORT) && defined(LIMIT_B_BIT))
#define LIMIT_B 107
#define LIMIT_B_MBED_PIN __mbedpin__(LIMIT_B_PORT, LIMIT_B_BIT)
#define LIMIT_B_GPIOREG __gpioreg__(LIMIT_B_PORT)
#if (LIMIT_B_BIT < 16)
#define LIMIT_B_PINHALF L
#else
#define LIMIT_B_PINHALF H
#endif
#define LIMIT_B_PINCON __pincon__(LIMIT_B_PORT, LIMIT_B_PINHALF)
#define DIO107 107
#define DIO107_MBED_PIN LIMIT_B_MBED_PIN
#define DIO107_PORT LIMIT_B_PORT
#define DIO107_BIT LIMIT_B_BIT
#define DIO107_GPIOREG LIMIT_B_GPIOREG
#define DIO107_PINHALF LIMIT_B_PINHALF
#define DIO107_PINCON LIMIT_B_PINCON
#endif
#if (defined(LIMIT_C_PORT) && defined(LIMIT_C_BIT))
#define LIMIT_C 108
#define LIMIT_C_MBED_PIN __mbedpin__(LIMIT_C_PORT, LIMIT_C_BIT)
#define LIMIT_C_GPIOREG __gpioreg__(LIMIT_C_PORT)
#if (LIMIT_C_BIT < 16)
#define LIMIT_C_PINHALF L
#else
#define LIMIT_C_PINHALF H
#endif
#define LIMIT_C_PINCON __pincon__(LIMIT_C_PORT, LIMIT_C_PINHALF)
#define DIO108 108
#define DIO108_MBED_PIN LIMIT_C_MBED_PIN
#define DIO108_PORT LIMIT_C_PORT
#define DIO108_BIT LIMIT_C_BIT
#define DIO108_GPIOREG LIMIT_C_GPIOREG
#define DIO108_PINHALF LIMIT_C_PINHALF
#define DIO108_PINCON LIMIT_C_PINCON
#endif
#if (defined(PROBE_PORT) && defined(PROBE_BIT))
#define PROBE 109
#define PROBE_MBED_PIN __mbedpin__(PROBE_PORT, PROBE_BIT)
#define PROBE_GPIOREG __gpioreg__(PROBE_PORT)
#if (PROBE_BIT < 16)
#define PROBE_PINHALF L
#else
#define PROBE_PINHALF H
#endif
#define PROBE_PINCON __pincon__(PROBE_PORT, PROBE_PINHALF)
#define DIO109 109
#define DIO109_MBED_PIN PROBE_MBED_PIN
#define DIO109_PORT PROBE_PORT
#define DIO109_BIT PROBE_BIT
#define DIO109_GPIOREG PROBE_GPIOREG
#define DIO109_PINHALF PROBE_PINHALF
#define DIO109_PINCON PROBE_PINCON
#endif
#if (defined(ESTOP_PORT) && defined(ESTOP_BIT))
#define ESTOP 110
#define ESTOP_MBED_PIN __mbedpin__(ESTOP_PORT, ESTOP_BIT)
#define ESTOP_GPIOREG __gpioreg__(ESTOP_PORT)
#if (ESTOP_BIT < 16)
#define ESTOP_PINHALF L
#else
#define ESTOP_PINHALF H
#endif
#define ESTOP_PINCON __pincon__(ESTOP_PORT, ESTOP_PINHALF)
#define DIO110 110
#define DIO110_MBED_PIN ESTOP_MBED_PIN
#define DIO110_PORT ESTOP_PORT
#define DIO110_BIT ESTOP_BIT
#define DIO110_GPIOREG ESTOP_GPIOREG
#define DIO110_PINHALF ESTOP_PINHALF
#define DIO110_PINCON ESTOP_PINCON
#endif
#if (defined(SAFETY_DOOR_PORT) && defined(SAFETY_DOOR_BIT))
#define SAFETY_DOOR 111
#define SAFETY_DOOR_MBED_PIN __mbedpin__(SAFETY_DOOR_PORT, SAFETY_DOOR_BIT)
#define SAFETY_DOOR_GPIOREG __gpioreg__(SAFETY_DOOR_PORT)
#if (SAFETY_DOOR_BIT < 16)
#define SAFETY_DOOR_PINHALF L
#else
#define SAFETY_DOOR_PINHALF H
#endif
#define SAFETY_DOOR_PINCON __pincon__(SAFETY_DOOR_PORT, SAFETY_DOOR_PINHALF)
#define DIO111 111
#define DIO111_MBED_PIN SAFETY_DOOR_MBED_PIN
#define DIO111_PORT SAFETY_DOOR_PORT
#define DIO111_BIT SAFETY_DOOR_BIT
#define DIO111_GPIOREG SAFETY_DOOR_GPIOREG
#define DIO111_PINHALF SAFETY_DOOR_PINHALF
#define DIO111_PINCON SAFETY_DOOR_PINCON
#endif
#if (defined(FHOLD_PORT) && defined(FHOLD_BIT))
#define FHOLD 112
#define FHOLD_MBED_PIN __mbedpin__(FHOLD_PORT, FHOLD_BIT)
#define FHOLD_GPIOREG __gpioreg__(FHOLD_PORT)
#if (FHOLD_BIT < 16)
#define FHOLD_PINHALF L
#else
#define FHOLD_PINHALF H
#endif
#define FHOLD_PINCON __pincon__(FHOLD_PORT, FHOLD_PINHALF)
#define DIO112 112
#define DIO112_MBED_PIN FHOLD_MBED_PIN
#define DIO112_PORT FHOLD_PORT
#define DIO112_BIT FHOLD_BIT
#define DIO112_GPIOREG FHOLD_GPIOREG
#define DIO112_PINHALF FHOLD_PINHALF
#define DIO112_PINCON FHOLD_PINCON
#endif
#if (defined(CS_RES_PORT) && defined(CS_RES_BIT))
#define CS_RES 113
#define CS_RES_MBED_PIN __mbedpin__(CS_RES_PORT, CS_RES_BIT)
#define CS_RES_GPIOREG __gpioreg__(CS_RES_PORT)
#if (CS_RES_BIT < 16)
#define CS_RES_PINHALF L
#else
#define CS_RES_PINHALF H
#endif
#define CS_RES_PINCON __pincon__(CS_RES_PORT, CS_RES_PINHALF)
#define DIO113 113
#define DIO113_MBED_PIN CS_RES_MBED_PIN
#define DIO113_PORT CS_RES_PORT
#define DIO113_BIT CS_RES_BIT
#define DIO113_GPIOREG CS_RES_GPIOREG
#define DIO113_PINHALF CS_RES_PINHALF
#define DIO113_PINCON CS_RES_PINCON
#endif
#if (defined(ANALOG0_PORT) && defined(ANALOG0_BIT))
#define ANALOG0 114
#define ANALOG0_MBED_PIN __mbedpin__(ANALOG0_PORT, ANALOG0_BIT)
#define ANALOG0_GPIOREG __gpioreg__(ANALOG0_PORT)
#if (ANALOG0_BIT < 16)
#define ANALOG0_PINHALF L
#else
#define ANALOG0_PINHALF H
#endif
#define ANALOG0_PINCON __pincon__(ANALOG0_PORT, ANALOG0_PINHALF)
#define DIO114 114
#define DIO114_MBED_PIN ANALOG0_MBED_PIN
#define DIO114_PORT ANALOG0_PORT
#define DIO114_BIT ANALOG0_BIT
#define DIO114_GPIOREG ANALOG0_GPIOREG
#define DIO114_PINHALF ANALOG0_PINHALF
#define DIO114_PINCON ANALOG0_PINCON
#endif
#if (defined(ANALOG1_PORT) && defined(ANALOG1_BIT))
#define ANALOG1 115
#define ANALOG1_MBED_PIN __mbedpin__(ANALOG1_PORT, ANALOG1_BIT)
#define ANALOG1_GPIOREG __gpioreg__(ANALOG1_PORT)
#if (ANALOG1_BIT < 16)
#define ANALOG1_PINHALF L
#else
#define ANALOG1_PINHALF H
#endif
#define ANALOG1_PINCON __pincon__(ANALOG1_PORT, ANALOG1_PINHALF)
#define DIO115 115
#define DIO115_MBED_PIN ANALOG1_MBED_PIN
#define DIO115_PORT ANALOG1_PORT
#define DIO115_BIT ANALOG1_BIT
#define DIO115_GPIOREG ANALOG1_GPIOREG
#define DIO115_PINHALF ANALOG1_PINHALF
#define DIO115_PINCON ANALOG1_PINCON
#endif
#if (defined(ANALOG2_PORT) && defined(ANALOG2_BIT))
#define ANALOG2 116
#define ANALOG2_MBED_PIN __mbedpin__(ANALOG2_PORT, ANALOG2_BIT)
#define ANALOG2_GPIOREG __gpioreg__(ANALOG2_PORT)
#if (ANALOG2_BIT < 16)
#define ANALOG2_PINHALF L
#else
#define ANALOG2_PINHALF H
#endif
#define ANALOG2_PINCON __pincon__(ANALOG2_PORT, ANALOG2_PINHALF)
#define DIO116 116
#define DIO116_MBED_PIN ANALOG2_MBED_PIN
#define DIO116_PORT ANALOG2_PORT
#define DIO116_BIT ANALOG2_BIT
#define DIO116_GPIOREG ANALOG2_GPIOREG
#define DIO116_PINHALF ANALOG2_PINHALF
#define DIO116_PINCON ANALOG2_PINCON
#endif
#if (defined(ANALOG3_PORT) && defined(ANALOG3_BIT))
#define ANALOG3 117
#define ANALOG3_MBED_PIN __mbedpin__(ANALOG3_PORT, ANALOG3_BIT)
#define ANALOG3_GPIOREG __gpioreg__(ANALOG3_PORT)
#if (ANALOG3_BIT < 16)
#define ANALOG3_PINHALF L
#else
#define ANALOG3_PINHALF H
#endif
#define ANALOG3_PINCON __pincon__(ANALOG3_PORT, ANALOG3_PINHALF)
#define DIO117 117
#define DIO117_MBED_PIN ANALOG3_MBED_PIN
#define DIO117_PORT ANALOG3_PORT
#define DIO117_BIT ANALOG3_BIT
#define DIO117_GPIOREG ANALOG3_GPIOREG
#define DIO117_PINHALF ANALOG3_PINHALF
#define DIO117_PINCON ANALOG3_PINCON
#endif
#if (defined(ANALOG4_PORT) && defined(ANALOG4_BIT))
#define ANALOG4 118
#define ANALOG4_MBED_PIN __mbedpin__(ANALOG4_PORT, ANALOG4_BIT)
#define ANALOG4_GPIOREG __gpioreg__(ANALOG4_PORT)
#if (ANALOG4_BIT < 16)
#define ANALOG4_PINHALF L
#else
#define ANALOG4_PINHALF H
#endif
#define ANALOG4_PINCON __pincon__(ANALOG4_PORT, ANALOG4_PINHALF)
#define DIO118 118
#define DIO118_MBED_PIN ANALOG4_MBED_PIN
#define DIO118_PORT ANALOG4_PORT
#define DIO118_BIT ANALOG4_BIT
#define DIO118_GPIOREG ANALOG4_GPIOREG
#define DIO118_PINHALF ANALOG4_PINHALF
#define DIO118_PINCON ANALOG4_PINCON
#endif
#if (defined(ANALOG5_PORT) && defined(ANALOG5_BIT))
#define ANALOG5 119
#define ANALOG5_MBED_PIN __mbedpin__(ANALOG5_PORT, ANALOG5_BIT)
#define ANALOG5_GPIOREG __gpioreg__(ANALOG5_PORT)
#if (ANALOG5_BIT < 16)
#define ANALOG5_PINHALF L
#else
#define ANALOG5_PINHALF H
#endif
#define ANALOG5_PINCON __pincon__(ANALOG5_PORT, ANALOG5_PINHALF)
#define DIO119 119
#define DIO119_MBED_PIN ANALOG5_MBED_PIN
#define DIO119_PORT ANALOG5_PORT
#define DIO119_BIT ANALOG5_BIT
#define DIO119_GPIOREG ANALOG5_GPIOREG
#define DIO119_PINHALF ANALOG5_PINHALF
#define DIO119_PINCON ANALOG5_PINCON
#endif
#if (defined(ANALOG6_PORT) && defined(ANALOG6_BIT))
#define ANALOG6 120
#define ANALOG6_MBED_PIN __mbedpin__(ANALOG6_PORT, ANALOG6_BIT)
#define ANALOG6_GPIOREG __gpioreg__(ANALOG6_PORT)
#if (ANALOG6_BIT < 16)
#define ANALOG6_PINHALF L
#else
#define ANALOG6_PINHALF H
#endif
#define ANALOG6_PINCON __pincon__(ANALOG6_PORT, ANALOG6_PINHALF)
#define DIO120 120
#define DIO120_MBED_PIN ANALOG6_MBED_PIN
#define DIO120_PORT ANALOG6_PORT
#define DIO120_BIT ANALOG6_BIT
#define DIO120_GPIOREG ANALOG6_GPIOREG
#define DIO120_PINHALF ANALOG6_PINHALF
#define DIO120_PINCON ANALOG6_PINCON
#endif
#if (defined(ANALOG7_PORT) && defined(ANALOG7_BIT))
#define ANALOG7 121
#define ANALOG7_MBED_PIN __mbedpin__(ANALOG7_PORT, ANALOG7_BIT)
#define ANALOG7_GPIOREG __gpioreg__(ANALOG7_PORT)
#if (ANALOG7_BIT < 16)
#define ANALOG7_PINHALF L
#else
#define ANALOG7_PINHALF H
#endif
#define ANALOG7_PINCON __pincon__(ANALOG7_PORT, ANALOG7_PINHALF)
#define DIO121 121
#define DIO121_MBED_PIN ANALOG7_MBED_PIN
#define DIO121_PORT ANALOG7_PORT
#define DIO121_BIT ANALOG7_BIT
#define DIO121_GPIOREG ANALOG7_GPIOREG
#define DIO121_PINHALF ANALOG7_PINHALF
#define DIO121_PINCON ANALOG7_PINCON
#endif
#if (defined(ANALOG8_PORT) && defined(ANALOG8_BIT))
#define ANALOG8 122
#define ANALOG8_MBED_PIN __mbedpin__(ANALOG8_PORT, ANALOG8_BIT)
#define ANALOG8_GPIOREG __gpioreg__(ANALOG8_PORT)
#if (ANALOG8_BIT < 16)
#define ANALOG8_PINHALF L
#else
#define ANALOG8_PINHALF H
#endif
#define ANALOG8_PINCON __pincon__(ANALOG8_PORT, ANALOG8_PINHALF)
#define DIO122 122
#define DIO122_MBED_PIN ANALOG8_MBED_PIN
#define DIO122_PORT ANALOG8_PORT
#define DIO122_BIT ANALOG8_BIT
#define DIO122_GPIOREG ANALOG8_GPIOREG
#define DIO122_PINHALF ANALOG8_PINHALF
#define DIO122_PINCON ANALOG8_PINCON
#endif
#if (defined(ANALOG9_PORT) && defined(ANALOG9_BIT))
#define ANALOG9 123
#define ANALOG9_MBED_PIN __mbedpin__(ANALOG9_PORT, ANALOG9_BIT)
#define ANALOG9_GPIOREG __gpioreg__(ANALOG9_PORT)
#if (ANALOG9_BIT < 16)
#define ANALOG9_PINHALF L
#else
#define ANALOG9_PINHALF H
#endif
#define ANALOG9_PINCON __pincon__(ANALOG9_PORT, ANALOG9_PINHALF)
#define DIO123 123
#define DIO123_MBED_PIN ANALOG9_MBED_PIN
#define DIO123_PORT ANALOG9_PORT
#define DIO123_BIT ANALOG9_BIT
#define DIO123_GPIOREG ANALOG9_GPIOREG
#define DIO123_PINHALF ANALOG9_PINHALF
#define DIO123_PINCON ANALOG9_PINCON
#endif
#if (defined(ANALOG10_PORT) && defined(ANALOG10_BIT))
#define ANALOG10 124
#define ANALOG10_MBED_PIN __mbedpin__(ANALOG10_PORT, ANALOG10_BIT)
#define ANALOG10_GPIOREG __gpioreg__(ANALOG10_PORT)
#if (ANALOG10_BIT < 16)
#define ANALOG10_PINHALF L
#else
#define ANALOG10_PINHALF H
#endif
#define ANALOG10_PINCON __pincon__(ANALOG10_PORT, ANALOG10_PINHALF)
#define DIO124 124
#define DIO124_MBED_PIN ANALOG10_MBED_PIN
#define DIO124_PORT ANALOG10_PORT
#define DIO124_BIT ANALOG10_BIT
#define DIO124_GPIOREG ANALOG10_GPIOREG
#define DIO124_PINHALF ANALOG10_PINHALF
#define DIO124_PINCON ANALOG10_PINCON
#endif
#if (defined(ANALOG11_PORT) && defined(ANALOG11_BIT))
#define ANALOG11 125
#define ANALOG11_MBED_PIN __mbedpin__(ANALOG11_PORT, ANALOG11_BIT)
#define ANALOG11_GPIOREG __gpioreg__(ANALOG11_PORT)
#if (ANALOG11_BIT < 16)
#define ANALOG11_PINHALF L
#else
#define ANALOG11_PINHALF H
#endif
#define ANALOG11_PINCON __pincon__(ANALOG11_PORT, ANALOG11_PINHALF)
#define DIO125 125
#define DIO125_MBED_PIN ANALOG11_MBED_PIN
#define DIO125_PORT ANALOG11_PORT
#define DIO125_BIT ANALOG11_BIT
#define DIO125_GPIOREG ANALOG11_GPIOREG
#define DIO125_PINHALF ANALOG11_PINHALF
#define DIO125_PINCON ANALOG11_PINCON
#endif
#if (defined(ANALOG12_PORT) && defined(ANALOG12_BIT))
#define ANALOG12 126
#define ANALOG12_MBED_PIN __mbedpin__(ANALOG12_PORT, ANALOG12_BIT)
#define ANALOG12_GPIOREG __gpioreg__(ANALOG12_PORT)
#if (ANALOG12_BIT < 16)
#define ANALOG12_PINHALF L
#else
#define ANALOG12_PINHALF H
#endif
#define ANALOG12_PINCON __pincon__(ANALOG12_PORT, ANALOG12_PINHALF)
#define DIO126 126
#define DIO126_MBED_PIN ANALOG12_MBED_PIN
#define DIO126_PORT ANALOG12_PORT
#define DIO126_BIT ANALOG12_BIT
#define DIO126_GPIOREG ANALOG12_GPIOREG
#define DIO126_PINHALF ANALOG12_PINHALF
#define DIO126_PINCON ANALOG12_PINCON
#endif
#if (defined(ANALOG13_PORT) && defined(ANALOG13_BIT))
#define ANALOG13 127
#define ANALOG13_MBED_PIN __mbedpin__(ANALOG13_PORT, ANALOG13_BIT)
#define ANALOG13_GPIOREG __gpioreg__(ANALOG13_PORT)
#if (ANALOG13_BIT < 16)
#define ANALOG13_PINHALF L
#else
#define ANALOG13_PINHALF H
#endif
#define ANALOG13_PINCON __pincon__(ANALOG13_PORT, ANALOG13_PINHALF)
#define DIO127 127
#define DIO127_MBED_PIN ANALOG13_MBED_PIN
#define DIO127_PORT ANALOG13_PORT
#define DIO127_BIT ANALOG13_BIT
#define DIO127_GPIOREG ANALOG13_GPIOREG
#define DIO127_PINHALF ANALOG13_PINHALF
#define DIO127_PINCON ANALOG13_PINCON
#endif
#if (defined(ANALOG14_PORT) && defined(ANALOG14_BIT))
#define ANALOG14 128
#define ANALOG14_MBED_PIN __mbedpin__(ANALOG14_PORT, ANALOG14_BIT)
#define ANALOG14_GPIOREG __gpioreg__(ANALOG14_PORT)
#if (ANALOG14_BIT < 16)
#define ANALOG14_PINHALF L
#else
#define ANALOG14_PINHALF H
#endif
#define ANALOG14_PINCON __pincon__(ANALOG14_PORT, ANALOG14_PINHALF)
#define DIO128 128
#define DIO128_MBED_PIN ANALOG14_MBED_PIN
#define DIO128_PORT ANALOG14_PORT
#define DIO128_BIT ANALOG14_BIT
#define DIO128_GPIOREG ANALOG14_GPIOREG
#define DIO128_PINHALF ANALOG14_PINHALF
#define DIO128_PINCON ANALOG14_PINCON
#endif
#if (defined(ANALOG15_PORT) && defined(ANALOG15_BIT))
#define ANALOG15 129
#define ANALOG15_MBED_PIN __mbedpin__(ANALOG15_PORT, ANALOG15_BIT)
#define ANALOG15_GPIOREG __gpioreg__(ANALOG15_PORT)
#if (ANALOG15_BIT < 16)
#define ANALOG15_PINHALF L
#else
#define ANALOG15_PINHALF H
#endif
#define ANALOG15_PINCON __pincon__(ANALOG15_PORT, ANALOG15_PINHALF)
#define DIO129 129
#define DIO129_MBED_PIN ANALOG15_MBED_PIN
#define DIO129_PORT ANALOG15_PORT
#define DIO129_BIT ANALOG15_BIT
#define DIO129_GPIOREG ANALOG15_GPIOREG
#define DIO129_PINHALF ANALOG15_PINHALF
#define DIO129_PINCON ANALOG15_PINCON
#endif
#if (defined(DIN0_PORT) && defined(DIN0_BIT))
#define DIN0 130
#define DIN0_MBED_PIN __mbedpin__(DIN0_PORT, DIN0_BIT)
#define DIN0_GPIOREG __gpioreg__(DIN0_PORT)
#if (DIN0_BIT < 16)
#define DIN0_PINHALF L
#else
#define DIN0_PINHALF H
#endif
#define DIN0_PINCON __pincon__(DIN0_PORT, DIN0_PINHALF)
#define DIO130 130
#define DIO130_MBED_PIN DIN0_MBED_PIN
#define DIO130_PORT DIN0_PORT
#define DIO130_BIT DIN0_BIT
#define DIO130_GPIOREG DIN0_GPIOREG
#define DIO130_PINHALF DIN0_PINHALF
#define DIO130_PINCON DIN0_PINCON
#endif
#if (defined(DIN1_PORT) && defined(DIN1_BIT))
#define DIN1 131
#define DIN1_MBED_PIN __mbedpin__(DIN1_PORT, DIN1_BIT)
#define DIN1_GPIOREG __gpioreg__(DIN1_PORT)
#if (DIN1_BIT < 16)
#define DIN1_PINHALF L
#else
#define DIN1_PINHALF H
#endif
#define DIN1_PINCON __pincon__(DIN1_PORT, DIN1_PINHALF)
#define DIO131 131
#define DIO131_MBED_PIN DIN1_MBED_PIN
#define DIO131_PORT DIN1_PORT
#define DIO131_BIT DIN1_BIT
#define DIO131_GPIOREG DIN1_GPIOREG
#define DIO131_PINHALF DIN1_PINHALF
#define DIO131_PINCON DIN1_PINCON
#endif
#if (defined(DIN2_PORT) && defined(DIN2_BIT))
#define DIN2 132
#define DIN2_MBED_PIN __mbedpin__(DIN2_PORT, DIN2_BIT)
#define DIN2_GPIOREG __gpioreg__(DIN2_PORT)
#if (DIN2_BIT < 16)
#define DIN2_PINHALF L
#else
#define DIN2_PINHALF H
#endif
#define DIN2_PINCON __pincon__(DIN2_PORT, DIN2_PINHALF)
#define DIO132 132
#define DIO132_MBED_PIN DIN2_MBED_PIN
#define DIO132_PORT DIN2_PORT
#define DIO132_BIT DIN2_BIT
#define DIO132_GPIOREG DIN2_GPIOREG
#define DIO132_PINHALF DIN2_PINHALF
#define DIO132_PINCON DIN2_PINCON
#endif
#if (defined(DIN3_PORT) && defined(DIN3_BIT))
#define DIN3 133
#define DIN3_MBED_PIN __mbedpin__(DIN3_PORT, DIN3_BIT)
#define DIN3_GPIOREG __gpioreg__(DIN3_PORT)
#if (DIN3_BIT < 16)
#define DIN3_PINHALF L
#else
#define DIN3_PINHALF H
#endif
#define DIN3_PINCON __pincon__(DIN3_PORT, DIN3_PINHALF)
#define DIO133 133
#define DIO133_MBED_PIN DIN3_MBED_PIN
#define DIO133_PORT DIN3_PORT
#define DIO133_BIT DIN3_BIT
#define DIO133_GPIOREG DIN3_GPIOREG
#define DIO133_PINHALF DIN3_PINHALF
#define DIO133_PINCON DIN3_PINCON
#endif
#if (defined(DIN4_PORT) && defined(DIN4_BIT))
#define DIN4 134
#define DIN4_MBED_PIN __mbedpin__(DIN4_PORT, DIN4_BIT)
#define DIN4_GPIOREG __gpioreg__(DIN4_PORT)
#if (DIN4_BIT < 16)
#define DIN4_PINHALF L
#else
#define DIN4_PINHALF H
#endif
#define DIN4_PINCON __pincon__(DIN4_PORT, DIN4_PINHALF)
#define DIO134 134
#define DIO134_MBED_PIN DIN4_MBED_PIN
#define DIO134_PORT DIN4_PORT
#define DIO134_BIT DIN4_BIT
#define DIO134_GPIOREG DIN4_GPIOREG
#define DIO134_PINHALF DIN4_PINHALF
#define DIO134_PINCON DIN4_PINCON
#endif
#if (defined(DIN5_PORT) && defined(DIN5_BIT))
#define DIN5 135
#define DIN5_MBED_PIN __mbedpin__(DIN5_PORT, DIN5_BIT)
#define DIN5_GPIOREG __gpioreg__(DIN5_PORT)
#if (DIN5_BIT < 16)
#define DIN5_PINHALF L
#else
#define DIN5_PINHALF H
#endif
#define DIN5_PINCON __pincon__(DIN5_PORT, DIN5_PINHALF)
#define DIO135 135
#define DIO135_MBED_PIN DIN5_MBED_PIN
#define DIO135_PORT DIN5_PORT
#define DIO135_BIT DIN5_BIT
#define DIO135_GPIOREG DIN5_GPIOREG
#define DIO135_PINHALF DIN5_PINHALF
#define DIO135_PINCON DIN5_PINCON
#endif
#if (defined(DIN6_PORT) && defined(DIN6_BIT))
#define DIN6 136
#define DIN6_MBED_PIN __mbedpin__(DIN6_PORT, DIN6_BIT)
#define DIN6_GPIOREG __gpioreg__(DIN6_PORT)
#if (DIN6_BIT < 16)
#define DIN6_PINHALF L
#else
#define DIN6_PINHALF H
#endif
#define DIN6_PINCON __pincon__(DIN6_PORT, DIN6_PINHALF)
#define DIO136 136
#define DIO136_MBED_PIN DIN6_MBED_PIN
#define DIO136_PORT DIN6_PORT
#define DIO136_BIT DIN6_BIT
#define DIO136_GPIOREG DIN6_GPIOREG
#define DIO136_PINHALF DIN6_PINHALF
#define DIO136_PINCON DIN6_PINCON
#endif
#if (defined(DIN7_PORT) && defined(DIN7_BIT))
#define DIN7 137
#define DIN7_MBED_PIN __mbedpin__(DIN7_PORT, DIN7_BIT)
#define DIN7_GPIOREG __gpioreg__(DIN7_PORT)
#if (DIN7_BIT < 16)
#define DIN7_PINHALF L
#else
#define DIN7_PINHALF H
#endif
#define DIN7_PINCON __pincon__(DIN7_PORT, DIN7_PINHALF)
#define DIO137 137
#define DIO137_MBED_PIN DIN7_MBED_PIN
#define DIO137_PORT DIN7_PORT
#define DIO137_BIT DIN7_BIT
#define DIO137_GPIOREG DIN7_GPIOREG
#define DIO137_PINHALF DIN7_PINHALF
#define DIO137_PINCON DIN7_PINCON
#endif
#if (defined(DIN8_PORT) && defined(DIN8_BIT))
#define DIN8 138
#define DIN8_MBED_PIN __mbedpin__(DIN8_PORT, DIN8_BIT)
#define DIN8_GPIOREG __gpioreg__(DIN8_PORT)
#if (DIN8_BIT < 16)
#define DIN8_PINHALF L
#else
#define DIN8_PINHALF H
#endif
#define DIN8_PINCON __pincon__(DIN8_PORT, DIN8_PINHALF)
#define DIO138 138
#define DIO138_MBED_PIN DIN8_MBED_PIN
#define DIO138_PORT DIN8_PORT
#define DIO138_BIT DIN8_BIT
#define DIO138_GPIOREG DIN8_GPIOREG
#define DIO138_PINHALF DIN8_PINHALF
#define DIO138_PINCON DIN8_PINCON
#endif
#if (defined(DIN9_PORT) && defined(DIN9_BIT))
#define DIN9 139
#define DIN9_MBED_PIN __mbedpin__(DIN9_PORT, DIN9_BIT)
#define DIN9_GPIOREG __gpioreg__(DIN9_PORT)
#if (DIN9_BIT < 16)
#define DIN9_PINHALF L
#else
#define DIN9_PINHALF H
#endif
#define DIN9_PINCON __pincon__(DIN9_PORT, DIN9_PINHALF)
#define DIO139 139
#define DIO139_MBED_PIN DIN9_MBED_PIN
#define DIO139_PORT DIN9_PORT
#define DIO139_BIT DIN9_BIT
#define DIO139_GPIOREG DIN9_GPIOREG
#define DIO139_PINHALF DIN9_PINHALF
#define DIO139_PINCON DIN9_PINCON
#endif
#if (defined(DIN10_PORT) && defined(DIN10_BIT))
#define DIN10 140
#define DIN10_MBED_PIN __mbedpin__(DIN10_PORT, DIN10_BIT)
#define DIN10_GPIOREG __gpioreg__(DIN10_PORT)
#if (DIN10_BIT < 16)
#define DIN10_PINHALF L
#else
#define DIN10_PINHALF H
#endif
#define DIN10_PINCON __pincon__(DIN10_PORT, DIN10_PINHALF)
#define DIO140 140
#define DIO140_MBED_PIN DIN10_MBED_PIN
#define DIO140_PORT DIN10_PORT
#define DIO140_BIT DIN10_BIT
#define DIO140_GPIOREG DIN10_GPIOREG
#define DIO140_PINHALF DIN10_PINHALF
#define DIO140_PINCON DIN10_PINCON
#endif
#if (defined(DIN11_PORT) && defined(DIN11_BIT))
#define DIN11 141
#define DIN11_MBED_PIN __mbedpin__(DIN11_PORT, DIN11_BIT)
#define DIN11_GPIOREG __gpioreg__(DIN11_PORT)
#if (DIN11_BIT < 16)
#define DIN11_PINHALF L
#else
#define DIN11_PINHALF H
#endif
#define DIN11_PINCON __pincon__(DIN11_PORT, DIN11_PINHALF)
#define DIO141 141
#define DIO141_MBED_PIN DIN11_MBED_PIN
#define DIO141_PORT DIN11_PORT
#define DIO141_BIT DIN11_BIT
#define DIO141_GPIOREG DIN11_GPIOREG
#define DIO141_PINHALF DIN11_PINHALF
#define DIO141_PINCON DIN11_PINCON
#endif
#if (defined(DIN12_PORT) && defined(DIN12_BIT))
#define DIN12 142
#define DIN12_MBED_PIN __mbedpin__(DIN12_PORT, DIN12_BIT)
#define DIN12_GPIOREG __gpioreg__(DIN12_PORT)
#if (DIN12_BIT < 16)
#define DIN12_PINHALF L
#else
#define DIN12_PINHALF H
#endif
#define DIN12_PINCON __pincon__(DIN12_PORT, DIN12_PINHALF)
#define DIO142 142
#define DIO142_MBED_PIN DIN12_MBED_PIN
#define DIO142_PORT DIN12_PORT
#define DIO142_BIT DIN12_BIT
#define DIO142_GPIOREG DIN12_GPIOREG
#define DIO142_PINHALF DIN12_PINHALF
#define DIO142_PINCON DIN12_PINCON
#endif
#if (defined(DIN13_PORT) && defined(DIN13_BIT))
#define DIN13 143
#define DIN13_MBED_PIN __mbedpin__(DIN13_PORT, DIN13_BIT)
#define DIN13_GPIOREG __gpioreg__(DIN13_PORT)
#if (DIN13_BIT < 16)
#define DIN13_PINHALF L
#else
#define DIN13_PINHALF H
#endif
#define DIN13_PINCON __pincon__(DIN13_PORT, DIN13_PINHALF)
#define DIO143 143
#define DIO143_MBED_PIN DIN13_MBED_PIN
#define DIO143_PORT DIN13_PORT
#define DIO143_BIT DIN13_BIT
#define DIO143_GPIOREG DIN13_GPIOREG
#define DIO143_PINHALF DIN13_PINHALF
#define DIO143_PINCON DIN13_PINCON
#endif
#if (defined(DIN14_PORT) && defined(DIN14_BIT))
#define DIN14 144
#define DIN14_MBED_PIN __mbedpin__(DIN14_PORT, DIN14_BIT)
#define DIN14_GPIOREG __gpioreg__(DIN14_PORT)
#if (DIN14_BIT < 16)
#define DIN14_PINHALF L
#else
#define DIN14_PINHALF H
#endif
#define DIN14_PINCON __pincon__(DIN14_PORT, DIN14_PINHALF)
#define DIO144 144
#define DIO144_MBED_PIN DIN14_MBED_PIN
#define DIO144_PORT DIN14_PORT
#define DIO144_BIT DIN14_BIT
#define DIO144_GPIOREG DIN14_GPIOREG
#define DIO144_PINHALF DIN14_PINHALF
#define DIO144_PINCON DIN14_PINCON
#endif
#if (defined(DIN15_PORT) && defined(DIN15_BIT))
#define DIN15 145
#define DIN15_MBED_PIN __mbedpin__(DIN15_PORT, DIN15_BIT)
#define DIN15_GPIOREG __gpioreg__(DIN15_PORT)
#if (DIN15_BIT < 16)
#define DIN15_PINHALF L
#else
#define DIN15_PINHALF H
#endif
#define DIN15_PINCON __pincon__(DIN15_PORT, DIN15_PINHALF)
#define DIO145 145
#define DIO145_MBED_PIN DIN15_MBED_PIN
#define DIO145_PORT DIN15_PORT
#define DIO145_BIT DIN15_BIT
#define DIO145_GPIOREG DIN15_GPIOREG
#define DIO145_PINHALF DIN15_PINHALF
#define DIO145_PINCON DIN15_PINCON
#endif
#if (defined(DIN16_PORT) && defined(DIN16_BIT))
#define DIN16 146
#define DIN16_MBED_PIN __mbedpin__(DIN16_PORT, DIN16_BIT)
#define DIN16_GPIOREG __gpioreg__(DIN16_PORT)
#if (DIN16_BIT < 16)
#define DIN16_PINHALF L
#else
#define DIN16_PINHALF H
#endif
#define DIN16_PINCON __pincon__(DIN16_PORT, DIN16_PINHALF)
#define DIO146 146
#define DIO146_MBED_PIN DIN16_MBED_PIN
#define DIO146_PORT DIN16_PORT
#define DIO146_BIT DIN16_BIT
#define DIO146_GPIOREG DIN16_GPIOREG
#define DIO146_PINHALF DIN16_PINHALF
#define DIO146_PINCON DIN16_PINCON
#endif
#if (defined(DIN17_PORT) && defined(DIN17_BIT))
#define DIN17 147
#define DIN17_MBED_PIN __mbedpin__(DIN17_PORT, DIN17_BIT)
#define DIN17_GPIOREG __gpioreg__(DIN17_PORT)
#if (DIN17_BIT < 16)
#define DIN17_PINHALF L
#else
#define DIN17_PINHALF H
#endif
#define DIN17_PINCON __pincon__(DIN17_PORT, DIN17_PINHALF)
#define DIO147 147
#define DIO147_MBED_PIN DIN17_MBED_PIN
#define DIO147_PORT DIN17_PORT
#define DIO147_BIT DIN17_BIT
#define DIO147_GPIOREG DIN17_GPIOREG
#define DIO147_PINHALF DIN17_PINHALF
#define DIO147_PINCON DIN17_PINCON
#endif
#if (defined(DIN18_PORT) && defined(DIN18_BIT))
#define DIN18 148
#define DIN18_MBED_PIN __mbedpin__(DIN18_PORT, DIN18_BIT)
#define DIN18_GPIOREG __gpioreg__(DIN18_PORT)
#if (DIN18_BIT < 16)
#define DIN18_PINHALF L
#else
#define DIN18_PINHALF H
#endif
#define DIN18_PINCON __pincon__(DIN18_PORT, DIN18_PINHALF)
#define DIO148 148
#define DIO148_MBED_PIN DIN18_MBED_PIN
#define DIO148_PORT DIN18_PORT
#define DIO148_BIT DIN18_BIT
#define DIO148_GPIOREG DIN18_GPIOREG
#define DIO148_PINHALF DIN18_PINHALF
#define DIO148_PINCON DIN18_PINCON
#endif
#if (defined(DIN19_PORT) && defined(DIN19_BIT))
#define DIN19 149
#define DIN19_MBED_PIN __mbedpin__(DIN19_PORT, DIN19_BIT)
#define DIN19_GPIOREG __gpioreg__(DIN19_PORT)
#if (DIN19_BIT < 16)
#define DIN19_PINHALF L
#else
#define DIN19_PINHALF H
#endif
#define DIN19_PINCON __pincon__(DIN19_PORT, DIN19_PINHALF)
#define DIO149 149
#define DIO149_MBED_PIN DIN19_MBED_PIN
#define DIO149_PORT DIN19_PORT
#define DIO149_BIT DIN19_BIT
#define DIO149_GPIOREG DIN19_GPIOREG
#define DIO149_PINHALF DIN19_PINHALF
#define DIO149_PINCON DIN19_PINCON
#endif
#if (defined(DIN20_PORT) && defined(DIN20_BIT))
#define DIN20 150
#define DIN20_MBED_PIN __mbedpin__(DIN20_PORT, DIN20_BIT)
#define DIN20_GPIOREG __gpioreg__(DIN20_PORT)
#if (DIN20_BIT < 16)
#define DIN20_PINHALF L
#else
#define DIN20_PINHALF H
#endif
#define DIN20_PINCON __pincon__(DIN20_PORT, DIN20_PINHALF)
#define DIO150 150
#define DIO150_MBED_PIN DIN20_MBED_PIN
#define DIO150_PORT DIN20_PORT
#define DIO150_BIT DIN20_BIT
#define DIO150_GPIOREG DIN20_GPIOREG
#define DIO150_PINHALF DIN20_PINHALF
#define DIO150_PINCON DIN20_PINCON
#endif
#if (defined(DIN21_PORT) && defined(DIN21_BIT))
#define DIN21 151
#define DIN21_MBED_PIN __mbedpin__(DIN21_PORT, DIN21_BIT)
#define DIN21_GPIOREG __gpioreg__(DIN21_PORT)
#if (DIN21_BIT < 16)
#define DIN21_PINHALF L
#else
#define DIN21_PINHALF H
#endif
#define DIN21_PINCON __pincon__(DIN21_PORT, DIN21_PINHALF)
#define DIO151 151
#define DIO151_MBED_PIN DIN21_MBED_PIN
#define DIO151_PORT DIN21_PORT
#define DIO151_BIT DIN21_BIT
#define DIO151_GPIOREG DIN21_GPIOREG
#define DIO151_PINHALF DIN21_PINHALF
#define DIO151_PINCON DIN21_PINCON
#endif
#if (defined(DIN22_PORT) && defined(DIN22_BIT))
#define DIN22 152
#define DIN22_MBED_PIN __mbedpin__(DIN22_PORT, DIN22_BIT)
#define DIN22_GPIOREG __gpioreg__(DIN22_PORT)
#if (DIN22_BIT < 16)
#define DIN22_PINHALF L
#else
#define DIN22_PINHALF H
#endif
#define DIN22_PINCON __pincon__(DIN22_PORT, DIN22_PINHALF)
#define DIO152 152
#define DIO152_MBED_PIN DIN22_MBED_PIN
#define DIO152_PORT DIN22_PORT
#define DIO152_BIT DIN22_BIT
#define DIO152_GPIOREG DIN22_GPIOREG
#define DIO152_PINHALF DIN22_PINHALF
#define DIO152_PINCON DIN22_PINCON
#endif
#if (defined(DIN23_PORT) && defined(DIN23_BIT))
#define DIN23 153
#define DIN23_MBED_PIN __mbedpin__(DIN23_PORT, DIN23_BIT)
#define DIN23_GPIOREG __gpioreg__(DIN23_PORT)
#if (DIN23_BIT < 16)
#define DIN23_PINHALF L
#else
#define DIN23_PINHALF H
#endif
#define DIN23_PINCON __pincon__(DIN23_PORT, DIN23_PINHALF)
#define DIO153 153
#define DIO153_MBED_PIN DIN23_MBED_PIN
#define DIO153_PORT DIN23_PORT
#define DIO153_BIT DIN23_BIT
#define DIO153_GPIOREG DIN23_GPIOREG
#define DIO153_PINHALF DIN23_PINHALF
#define DIO153_PINCON DIN23_PINCON
#endif
#if (defined(DIN24_PORT) && defined(DIN24_BIT))
#define DIN24 154
#define DIN24_MBED_PIN __mbedpin__(DIN24_PORT, DIN24_BIT)
#define DIN24_GPIOREG __gpioreg__(DIN24_PORT)
#if (DIN24_BIT < 16)
#define DIN24_PINHALF L
#else
#define DIN24_PINHALF H
#endif
#define DIN24_PINCON __pincon__(DIN24_PORT, DIN24_PINHALF)
#define DIO154 154
#define DIO154_MBED_PIN DIN24_MBED_PIN
#define DIO154_PORT DIN24_PORT
#define DIO154_BIT DIN24_BIT
#define DIO154_GPIOREG DIN24_GPIOREG
#define DIO154_PINHALF DIN24_PINHALF
#define DIO154_PINCON DIN24_PINCON
#endif
#if (defined(DIN25_PORT) && defined(DIN25_BIT))
#define DIN25 155
#define DIN25_MBED_PIN __mbedpin__(DIN25_PORT, DIN25_BIT)
#define DIN25_GPIOREG __gpioreg__(DIN25_PORT)
#if (DIN25_BIT < 16)
#define DIN25_PINHALF L
#else
#define DIN25_PINHALF H
#endif
#define DIN25_PINCON __pincon__(DIN25_PORT, DIN25_PINHALF)
#define DIO155 155
#define DIO155_MBED_PIN DIN25_MBED_PIN
#define DIO155_PORT DIN25_PORT
#define DIO155_BIT DIN25_BIT
#define DIO155_GPIOREG DIN25_GPIOREG
#define DIO155_PINHALF DIN25_PINHALF
#define DIO155_PINCON DIN25_PINCON
#endif
#if (defined(DIN26_PORT) && defined(DIN26_BIT))
#define DIN26 156
#define DIN26_MBED_PIN __mbedpin__(DIN26_PORT, DIN26_BIT)
#define DIN26_GPIOREG __gpioreg__(DIN26_PORT)
#if (DIN26_BIT < 16)
#define DIN26_PINHALF L
#else
#define DIN26_PINHALF H
#endif
#define DIN26_PINCON __pincon__(DIN26_PORT, DIN26_PINHALF)
#define DIO156 156
#define DIO156_MBED_PIN DIN26_MBED_PIN
#define DIO156_PORT DIN26_PORT
#define DIO156_BIT DIN26_BIT
#define DIO156_GPIOREG DIN26_GPIOREG
#define DIO156_PINHALF DIN26_PINHALF
#define DIO156_PINCON DIN26_PINCON
#endif
#if (defined(DIN27_PORT) && defined(DIN27_BIT))
#define DIN27 157
#define DIN27_MBED_PIN __mbedpin__(DIN27_PORT, DIN27_BIT)
#define DIN27_GPIOREG __gpioreg__(DIN27_PORT)
#if (DIN27_BIT < 16)
#define DIN27_PINHALF L
#else
#define DIN27_PINHALF H
#endif
#define DIN27_PINCON __pincon__(DIN27_PORT, DIN27_PINHALF)
#define DIO157 157
#define DIO157_MBED_PIN DIN27_MBED_PIN
#define DIO157_PORT DIN27_PORT
#define DIO157_BIT DIN27_BIT
#define DIO157_GPIOREG DIN27_GPIOREG
#define DIO157_PINHALF DIN27_PINHALF
#define DIO157_PINCON DIN27_PINCON
#endif
#if (defined(DIN28_PORT) && defined(DIN28_BIT))
#define DIN28 158
#define DIN28_MBED_PIN __mbedpin__(DIN28_PORT, DIN28_BIT)
#define DIN28_GPIOREG __gpioreg__(DIN28_PORT)
#if (DIN28_BIT < 16)
#define DIN28_PINHALF L
#else
#define DIN28_PINHALF H
#endif
#define DIN28_PINCON __pincon__(DIN28_PORT, DIN28_PINHALF)
#define DIO158 158
#define DIO158_MBED_PIN DIN28_MBED_PIN
#define DIO158_PORT DIN28_PORT
#define DIO158_BIT DIN28_BIT
#define DIO158_GPIOREG DIN28_GPIOREG
#define DIO158_PINHALF DIN28_PINHALF
#define DIO158_PINCON DIN28_PINCON
#endif
#if (defined(DIN29_PORT) && defined(DIN29_BIT))
#define DIN29 159
#define DIN29_MBED_PIN __mbedpin__(DIN29_PORT, DIN29_BIT)
#define DIN29_GPIOREG __gpioreg__(DIN29_PORT)
#if (DIN29_BIT < 16)
#define DIN29_PINHALF L
#else
#define DIN29_PINHALF H
#endif
#define DIN29_PINCON __pincon__(DIN29_PORT, DIN29_PINHALF)
#define DIO159 159
#define DIO159_MBED_PIN DIN29_MBED_PIN
#define DIO159_PORT DIN29_PORT
#define DIO159_BIT DIN29_BIT
#define DIO159_GPIOREG DIN29_GPIOREG
#define DIO159_PINHALF DIN29_PINHALF
#define DIO159_PINCON DIN29_PINCON
#endif
#if (defined(DIN30_PORT) && defined(DIN30_BIT))
#define DIN30 160
#define DIN30_MBED_PIN __mbedpin__(DIN30_PORT, DIN30_BIT)
#define DIN30_GPIOREG __gpioreg__(DIN30_PORT)
#if (DIN30_BIT < 16)
#define DIN30_PINHALF L
#else
#define DIN30_PINHALF H
#endif
#define DIN30_PINCON __pincon__(DIN30_PORT, DIN30_PINHALF)
#define DIO160 160
#define DIO160_MBED_PIN DIN30_MBED_PIN
#define DIO160_PORT DIN30_PORT
#define DIO160_BIT DIN30_BIT
#define DIO160_GPIOREG DIN30_GPIOREG
#define DIO160_PINHALF DIN30_PINHALF
#define DIO160_PINCON DIN30_PINCON
#endif
#if (defined(DIN31_PORT) && defined(DIN31_BIT))
#define DIN31 161
#define DIN31_MBED_PIN __mbedpin__(DIN31_PORT, DIN31_BIT)
#define DIN31_GPIOREG __gpioreg__(DIN31_PORT)
#if (DIN31_BIT < 16)
#define DIN31_PINHALF L
#else
#define DIN31_PINHALF H
#endif
#define DIN31_PINCON __pincon__(DIN31_PORT, DIN31_PINHALF)
#define DIO161 161
#define DIO161_MBED_PIN DIN31_MBED_PIN
#define DIO161_PORT DIN31_PORT
#define DIO161_BIT DIN31_BIT
#define DIO161_GPIOREG DIN31_GPIOREG
#define DIO161_PINHALF DIN31_PINHALF
#define DIO161_PINCON DIN31_PINCON
#endif
#if (defined(TX_PORT) && defined(TX_BIT))
#define TX 200
#define TX_MBED_PIN __mbedpin__(TX_PORT, TX_BIT)
#define TX_GPIOREG __gpioreg__(TX_PORT)
#if (TX_BIT < 16)
#define TX_PINHALF L
#else
#define TX_PINHALF H
#endif
#define TX_PINCON __pincon__(TX_PORT, TX_PINHALF)
#define DIO200 200
#define DIO200_MBED_PIN TX_MBED_PIN
#define DIO200_PORT TX_PORT
#define DIO200_BIT TX_BIT
#define DIO200_GPIOREG TX_GPIOREG
#define DIO200_PINHALF TX_PINHALF
#define DIO200_PINCON TX_PINCON
#endif
#if (defined(RX_PORT) && defined(RX_BIT))
#define RX 201
#define RX_MBED_PIN __mbedpin__(RX_PORT, RX_BIT)
#define RX_GPIOREG __gpioreg__(RX_PORT)
#if (RX_BIT < 16)
#define RX_PINHALF L
#else
#define RX_PINHALF H
#endif
#define RX_PINCON __pincon__(RX_PORT, RX_PINHALF)
#define DIO201 201
#define DIO201_MBED_PIN RX_MBED_PIN
#define DIO201_PORT RX_PORT
#define DIO201_BIT RX_BIT
#define DIO201_GPIOREG RX_GPIOREG
#define DIO201_PINHALF RX_PINHALF
#define DIO201_PINCON RX_PINCON
#endif
#if (defined(USB_DM_PORT) && defined(USB_DM_BIT))
#define USB_DM 202
#define USB_DM_MBED_PIN __mbedpin__(USB_DM_PORT, USB_DM_BIT)
#define USB_DM_GPIOREG __gpioreg__(USB_DM_PORT)
#if (USB_DM_BIT < 16)
#define USB_DM_PINHALF L
#else
#define USB_DM_PINHALF H
#endif
#define USB_DM_PINCON __pincon__(USB_DM_PORT, USB_DM_PINHALF)
#define DIO202 202
#define DIO202_MBED_PIN USB_DM_MBED_PIN
#define DIO202_PORT USB_DM_PORT
#define DIO202_BIT USB_DM_BIT
#define DIO202_GPIOREG USB_DM_GPIOREG
#define DIO202_PINHALF USB_DM_PINHALF
#define DIO202_PINCON USB_DM_PINCON
#endif
#if (defined(USB_DP_PORT) && defined(USB_DP_BIT))
#define USB_DP 203
#define USB_DP_MBED_PIN __mbedpin__(USB_DP_PORT, USB_DP_BIT)
#define USB_DP_GPIOREG __gpioreg__(USB_DP_PORT)
#if (USB_DP_BIT < 16)
#define USB_DP_PINHALF L
#else
#define USB_DP_PINHALF H
#endif
#define USB_DP_PINCON __pincon__(USB_DP_PORT, USB_DP_PINHALF)
#define DIO203 203
#define DIO203_MBED_PIN USB_DP_MBED_PIN
#define DIO203_PORT USB_DP_PORT
#define DIO203_BIT USB_DP_BIT
#define DIO203_GPIOREG USB_DP_GPIOREG
#define DIO203_PINHALF USB_DP_PINHALF
#define DIO203_PINCON USB_DP_PINCON
#endif
#if (defined(SPI_CLK_PORT) && defined(SPI_CLK_BIT))
#define SPI_CLK 204
#define SPI_CLK_MBED_PIN __mbedpin__(SPI_CLK_PORT, SPI_CLK_BIT)
#define SPI_CLK_GPIOREG __gpioreg__(SPI_CLK_PORT)
#if (SPI_CLK_BIT < 16)
#define SPI_CLK_PINHALF L
#else
#define SPI_CLK_PINHALF H
#endif
#define SPI_CLK_PINCON __pincon__(SPI_CLK_PORT, SPI_CLK_PINHALF)
#define DIO204 204
#define DIO204_MBED_PIN SPI_CLK_MBED_PIN
#define DIO204_PORT SPI_CLK_PORT
#define DIO204_BIT SPI_CLK_BIT
#define DIO204_GPIOREG SPI_CLK_GPIOREG
#define DIO204_PINHALF SPI_CLK_PINHALF
#define DIO204_PINCON SPI_CLK_PINCON
#endif
#if (defined(SPI_SDI_PORT) && defined(SPI_SDI_BIT))
#define SPI_SDI 205
#define SPI_SDI_MBED_PIN __mbedpin__(SPI_SDI_PORT, SPI_SDI_BIT)
#define SPI_SDI_GPIOREG __gpioreg__(SPI_SDI_PORT)
#if (SPI_SDI_BIT < 16)
#define SPI_SDI_PINHALF L
#else
#define SPI_SDI_PINHALF H
#endif
#define SPI_SDI_PINCON __pincon__(SPI_SDI_PORT, SPI_SDI_PINHALF)
#define DIO205 205
#define DIO205_MBED_PIN SPI_SDI_MBED_PIN
#define DIO205_PORT SPI_SDI_PORT
#define DIO205_BIT SPI_SDI_BIT
#define DIO205_GPIOREG SPI_SDI_GPIOREG
#define DIO205_PINHALF SPI_SDI_PINHALF
#define DIO205_PINCON SPI_SDI_PINCON
#endif
#if (defined(SPI_SDO_PORT) && defined(SPI_SDO_BIT))
#define SPI_SDO 206
#define SPI_SDO_MBED_PIN __mbedpin__(SPI_SDO_PORT, SPI_SDO_BIT)
#define SPI_SDO_GPIOREG __gpioreg__(SPI_SDO_PORT)
#if (SPI_SDO_BIT < 16)
#define SPI_SDO_PINHALF L
#else
#define SPI_SDO_PINHALF H
#endif
#define SPI_SDO_PINCON __pincon__(SPI_SDO_PORT, SPI_SDO_PINHALF)
#define DIO206 206
#define DIO206_MBED_PIN SPI_SDO_MBED_PIN
#define DIO206_PORT SPI_SDO_PORT
#define DIO206_BIT SPI_SDO_BIT
#define DIO206_GPIOREG SPI_SDO_GPIOREG
#define DIO206_PINHALF SPI_SDO_PINHALF
#define DIO206_PINCON SPI_SDO_PINCON
#endif

/**********************************************
 *	ISR on change inputs
 **********************************************/
// ISR on change inputs
#define __risereg__(X) (__helper__(IO, X, IntEnR))
#if (defined(LIMIT_X_ISR) && defined(LIMIT_X))
#define LIMIT_X_RISEREG (__risereg__(LIMIT_X_PORT))
#define LIMIT_X_FALLREG (__fallreg__(LIMIT_X_PORT))
#define DIO100_ISR (LIMIT_X_ISR)
#define DIO100_RISEREG LIMIT_X_RISEREG
#define DIO100_FALLREG LIMIT_X_FALLREG
#endif
#if (defined(LIMIT_Y_ISR) && defined(LIMIT_Y))
#define LIMIT_Y_RISEREG (__risereg__(LIMIT_Y_PORT))
#define LIMIT_Y_FALLREG (__fallreg__(LIMIT_Y_PORT))
#define DIO101_ISR (LIMIT_Y_ISR)
#define DIO101_RISEREG LIMIT_Y_RISEREG
#define DIO101_FALLREG LIMIT_Y_FALLREG
#endif
#if (defined(LIMIT_Z_ISR) && defined(LIMIT_Z))
#define LIMIT_Z_RISEREG (__risereg__(LIMIT_Z_PORT))
#define LIMIT_Z_FALLREG (__fallreg__(LIMIT_Z_PORT))
#define DIO102_ISR (LIMIT_Z_ISR)
#define DIO102_RISEREG LIMIT_Z_RISEREG
#define DIO102_FALLREG LIMIT_Z_FALLREG
#endif
#if (defined(LIMIT_X2_ISR) && defined(LIMIT_X2))
#define LIMIT_X2_RISEREG (__risereg__(LIMIT_X2_PORT))
#define LIMIT_X2_FALLREG (__fallreg__(LIMIT_X2_PORT))
#define DIO103_ISR (LIMIT_X2_ISR)
#define DIO103_RISEREG LIMIT_X2_RISEREG
#define DIO103_FALLREG LIMIT_X2_FALLREG
#endif
#if (defined(LIMIT_Y2_ISR) && defined(LIMIT_Y2))
#define LIMIT_Y2_RISEREG (__risereg__(LIMIT_Y2_PORT))
#define LIMIT_Y2_FALLREG (__fallreg__(LIMIT_Y2_PORT))
#define DIO104_ISR (LIMIT_Y2_ISR)
#define DIO104_RISEREG LIMIT_Y2_RISEREG
#define DIO104_FALLREG LIMIT_Y2_FALLREG
#endif
#if (defined(LIMIT_Z2_ISR) && defined(LIMIT_Z2))
#define LIMIT_Z2_RISEREG (__risereg__(LIMIT_Z2_PORT))
#define LIMIT_Z2_FALLREG (__fallreg__(LIMIT_Z2_PORT))
#define DIO105_ISR (LIMIT_Z2_ISR)
#define DIO105_RISEREG LIMIT_Z2_RISEREG
#define DIO105_FALLREG LIMIT_Z2_FALLREG
#endif
#if (defined(LIMIT_A_ISR) && defined(LIMIT_A))
#define LIMIT_A_RISEREG (__risereg__(LIMIT_A_PORT))
#define LIMIT_A_FALLREG (__fallreg__(LIMIT_A_PORT))
#define DIO106_ISR (LIMIT_A_ISR)
#define DIO106_RISEREG LIMIT_A_RISEREG
#define DIO106_FALLREG LIMIT_A_FALLREG
#endif
#if (defined(LIMIT_B_ISR) && defined(LIMIT_B))
#define LIMIT_B_RISEREG (__risereg__(LIMIT_B_PORT))
#define LIMIT_B_FALLREG (__fallreg__(LIMIT_B_PORT))
#define DIO107_ISR (LIMIT_B_ISR)
#define DIO107_RISEREG LIMIT_B_RISEREG
#define DIO107_FALLREG LIMIT_B_FALLREG
#endif
#if (defined(LIMIT_C_ISR) && defined(LIMIT_C))
#define LIMIT_C_RISEREG (__risereg__(LIMIT_C_PORT))
#define LIMIT_C_FALLREG (__fallreg__(LIMIT_C_PORT))
#define DIO108_ISR (LIMIT_C_ISR)
#define DIO108_RISEREG LIMIT_C_RISEREG
#define DIO108_FALLREG LIMIT_C_FALLREG
#endif
#if (defined(PROBE_ISR) && defined(PROBE))
#define PROBE_RISEREG (__risereg__(PROBE_PORT))
#define PROBE_FALLREG (__fallreg__(PROBE_PORT))
#define DIO109_ISR (PROBE_ISR)
#define DIO109_RISEREG PROBE_RISEREG
#define DIO109_FALLREG PROBE_FALLREG
#endif
#if (defined(ESTOP_ISR) && defined(ESTOP))
#define ESTOP_RISEREG (__risereg__(ESTOP_PORT))
#define ESTOP_FALLREG (__fallreg__(ESTOP_PORT))
#define DIO110_ISR (ESTOP_ISR)
#define DIO110_RISEREG ESTOP_RISEREG
#define DIO110_FALLREG ESTOP_FALLREG
#endif
#if (defined(SAFETY_DOOR_ISR) && defined(SAFETY_DOOR))
#define SAFETY_DOOR_RISEREG (__risereg__(SAFETY_DOOR_PORT))
#define SAFETY_DOOR_FALLREG (__fallreg__(SAFETY_DOOR_PORT))
#define DIO111_ISR (SAFETY_DOOR_ISR)
#define DIO111_RISEREG SAFETY_DOOR_RISEREG
#define DIO111_FALLREG SAFETY_DOOR_FALLREG
#endif
#if (defined(FHOLD_ISR) && defined(FHOLD))
#define FHOLD_RISEREG (__risereg__(FHOLD_PORT))
#define FHOLD_FALLREG (__fallreg__(FHOLD_PORT))
#define DIO112_ISR (FHOLD_ISR)
#define DIO112_RISEREG FHOLD_RISEREG
#define DIO112_FALLREG FHOLD_FALLREG
#endif
#if (defined(CS_RES_ISR) && defined(CS_RES))
#define CS_RES_RISEREG (__risereg__(CS_RES_PORT))
#define CS_RES_FALLREG (__fallreg__(CS_RES_PORT))
#define DIO113_ISR (CS_RES_ISR)
#define DIO113_RISEREG CS_RES_RISEREG
#define DIO113_FALLREG CS_RES_FALLREG
#endif
#if (defined(DIN0_ISR) && defined(DIN0))
#define DIN0_RISEREG (__risereg__(DIN0_PORT))
#define DIN0_FALLREG (__fallreg__(DIN0_PORT))
#define DIO130_ISR (DIN0_ISR)
#define DIO130_RISEREG DIN0_RISEREG
#define DIO130_FALLREG DIN0_FALLREG
#endif
#if (defined(DIN1_ISR) && defined(DIN1))
#define DIN1_RISEREG (__risereg__(DIN1_PORT))
#define DIN1_FALLREG (__fallreg__(DIN1_PORT))
#define DIO131_ISR (DIN1_ISR)
#define DIO131_RISEREG DIN1_RISEREG
#define DIO131_FALLREG DIN1_FALLREG
#endif
#if (defined(DIN2_ISR) && defined(DIN2))
#define DIN2_RISEREG (__risereg__(DIN2_PORT))
#define DIN2_FALLREG (__fallreg__(DIN2_PORT))
#define DIO132_ISR (DIN2_ISR)
#define DIO132_RISEREG DIN2_RISEREG
#define DIO132_FALLREG DIN2_FALLREG
#endif
#if (defined(DIN3_ISR) && defined(DIN3))
#define DIN3_RISEREG (__risereg__(DIN3_PORT))
#define DIN3_FALLREG (__fallreg__(DIN3_PORT))
#define DIO133_ISR (DIN3_ISR)
#define DIO133_RISEREG DIN3_RISEREG
#define DIO133_FALLREG DIN3_FALLREG
#endif
#if (defined(DIN4_ISR) && defined(DIN4))
#define DIN4_RISEREG (__risereg__(DIN4_PORT))
#define DIN4_FALLREG (__fallreg__(DIN4_PORT))
#define DIO134_ISR (DIN4_ISR)
#define DIO134_RISEREG DIN4_RISEREG
#define DIO134_FALLREG DIN4_FALLREG
#endif
#if (defined(DIN5_ISR) && defined(DIN5))
#define DIN5_RISEREG (__risereg__(DIN5_PORT))
#define DIN5_FALLREG (__fallreg__(DIN5_PORT))
#define DIO135_ISR (DIN5_ISR)
#define DIO135_RISEREG DIN5_RISEREG
#define DIO135_FALLREG DIN5_FALLREG
#endif
#if (defined(DIN6_ISR) && defined(DIN6))
#define DIN6_RISEREG (__risereg__(DIN6_PORT))
#define DIN6_FALLREG (__fallreg__(DIN6_PORT))
#define DIO136_ISR (DIN6_ISR)
#define DIO136_RISEREG DIN6_RISEREG
#define DIO136_FALLREG DIN6_FALLREG
#endif
#if (defined(DIN7_ISR) && defined(DIN7))
#define DIN7_RISEREG (__risereg__(DIN7_PORT))
#define DIN7_FALLREG (__fallreg__(DIN7_PORT))
#define DIO137_ISR (DIN7_ISR)
#define DIO137_RISEREG DIN7_RISEREG
#define DIO137_FALLREG DIN7_FALLREG
#endif

	/**
	 *
	 * PWM
	 *
	 * */
#define __mrreg__(X) __helper__(MR, X, )
#ifdef PWM0
#define PWM0_MR __mrreg__(PWM0_CHANNEL)
#if (PWM0_PORT == 1)
#define PWM0_PINSEL 2
#define PWM0_FUNC 2
#elif (PWM0_PORT == 2)
#define PWM0_PINSEL 1
#define PWM0_FUNC 1
#else
#define PWM0_PINSEL 0
#define PWM0_FUNC 0
#endif
#define DIO24_PINSEL PWM0_PINSEL
#define DIO24_FUNC PWM0_FUNC
#define DIO24_CHANNEL PWM0_CHANNEL
#define DIO24_MR PWM0_MR
#endif
#ifdef PWM1
#define PWM1_MR __mrreg__(PWM1_CHANNEL)
#if (PWM1_PORT == 1)
#define PWM1_PINSEL 2
#define PWM1_FUNC 2
#elif (PWM1_PORT == 2)
#define PWM1_PINSEL 1
#define PWM1_FUNC 1
#else
#define PWM1_PINSEL 0
#define PWM1_FUNC 0
#endif
#define DIO25_PINSEL PWM1_PINSEL
#define DIO25_FUNC PWM1_FUNC
#define DIO25_CHANNEL PWM1_CHANNEL
#define DIO25_MR PWM1_MR
#endif
#ifdef PWM2
#define PWM2_MR __mrreg__(PWM2_CHANNEL)
#if (PWM2_PORT == 1)
#define PWM2_PINSEL 2
#define PWM2_FUNC 2
#elif (PWM2_PORT == 2)
#define PWM2_PINSEL 1
#define PWM2_FUNC 1
#else
#define PWM2_PINSEL 0
#define PWM2_FUNC 0
#endif
#define DIO26_PINSEL PWM2_PINSEL
#define DIO26_FUNC PWM2_FUNC
#define DIO26_CHANNEL PWM2_CHANNEL
#define DIO26_MR PWM2_MR
#endif
#ifdef PWM3
#define PWM3_MR __mrreg__(PWM3_CHANNEL)
#if (PWM3_PORT == 1)
#define PWM3_PINSEL 2
#define PWM3_FUNC 2
#elif (PWM3_PORT == 2)
#define PWM3_PINSEL 1
#define PWM3_FUNC 1
#else
#define PWM3_PINSEL 0
#define PWM3_FUNC 0
#endif
#define DIO27_PINSEL PWM3_PINSEL
#define DIO27_FUNC PWM3_FUNC
#define DIO27_CHANNEL PWM3_CHANNEL
#define DIO27_MR PWM3_MR
#endif
#ifdef PWM4
#define PWM4_MR __mrreg__(PWM4_CHANNEL)
#if (PWM4_PORT == 1)
#define PWM4_PINSEL 2
#define PWM4_FUNC 2
#elif (PWM4_PORT == 2)
#define PWM4_PINSEL 1
#define PWM4_FUNC 1
#else
#define PWM4_PINSEL 0
#define PWM4_FUNC 0
#endif
#define DIO28_PINSEL PWM4_PINSEL
#define DIO28_FUNC PWM4_FUNC
#define DIO28_CHANNEL PWM4_CHANNEL
#define DIO28_MR PWM4_MR
#endif
#ifdef PWM5
#define PWM5_MR __mrreg__(PWM5_CHANNEL)
#if (PWM5_PORT == 1)
#define PWM5_PINSEL 2
#define PWM5_FUNC 2
#elif (PWM5_PORT == 2)
#define PWM5_PINSEL 1
#define PWM5_FUNC 1
#else
#define PWM5_PINSEL 0
#define PWM5_FUNC 0
#endif
#define DIO29_PINSEL PWM5_PINSEL
#define DIO29_FUNC PWM5_FUNC
#define DIO29_CHANNEL PWM5_CHANNEL
#define DIO29_MR PWM5_MR
#endif
#ifdef PWM6
#define PWM6_MR __mrreg__(PWM6_CHANNEL)
#if (PWM6_PORT == 1)
#define PWM6_PINSEL 2
#define PWM6_FUNC 2
#elif (PWM6_PORT == 2)
#define PWM6_PINSEL 1
#define PWM6_FUNC 1
#else
#define PWM6_PINSEL 0
#define PWM6_FUNC 0
#endif
#define DIO30_PINSEL PWM6_PINSEL
#define DIO30_FUNC PWM6_FUNC
#define DIO30_CHANNEL PWM6_CHANNEL
#define DIO30_MR PWM6_MR
#endif
#ifdef PWM7
#define PWM7_MR __mrreg__(PWM7_CHANNEL)
#if (PWM7_PORT == 1)
#define PWM7_PINSEL 2
#define PWM7_FUNC 2
#elif (PWM7_PORT == 2)
#define PWM7_PINSEL 1
#define PWM7_FUNC 1
#else
#define PWM7_PINSEL 0
#define PWM7_FUNC 0
#endif
#define DIO31_PINSEL PWM7_PINSEL
#define DIO31_FUNC PWM7_FUNC
#define DIO31_CHANNEL PWM7_CHANNEL
#define DIO31_MR PWM7_MR
#endif
#ifdef PWM8
#define PWM8_MR __mrreg__(PWM8_CHANNEL)
#if (PWM8_PORT == 1)
#define PWM8_PINSEL 2
#define PWM8_FUNC 2
#elif (PWM8_PORT == 2)
#define PWM8_PINSEL 1
#define PWM8_FUNC 1
#else
#define PWM8_PINSEL 0
#define PWM8_FUNC 0
#endif
#define DIO32_PINSEL PWM8_PINSEL
#define DIO32_FUNC PWM8_FUNC
#define DIO32_CHANNEL PWM8_CHANNEL
#define DIO32_MR PWM8_MR
#endif
#ifdef PWM9
#define PWM9_MR __mrreg__(PWM9_CHANNEL)
#if (PWM9_PORT == 1)
#define PWM9_PINSEL 2
#define PWM9_FUNC 2
#elif (PWM9_PORT == 2)
#define PWM9_PINSEL 1
#define PWM9_FUNC 1
#else
#define PWM9_PINSEL 0
#define PWM9_FUNC 0
#endif
#define DIO33_PINSEL PWM9_PINSEL
#define DIO33_FUNC PWM9_FUNC
#define DIO33_CHANNEL PWM9_CHANNEL
#define DIO33_MR PWM9_MR
#endif
#ifdef PWM10
#define PWM10_MR __mrreg__(PWM10_CHANNEL)
#if (PWM10_PORT == 1)
#define PWM10_PINSEL 2
#define PWM10_FUNC 2
#elif (PWM10_PORT == 2)
#define PWM10_PINSEL 1
#define PWM10_FUNC 1
#else
#define PWM10_PINSEL 0
#define PWM10_FUNC 0
#endif
#define DIO34_PINSEL PWM10_PINSEL
#define DIO34_FUNC PWM10_FUNC
#define DIO34_CHANNEL PWM10_CHANNEL
#define DIO34_MR PWM10_MR
#endif
#ifdef PWM11
#define PWM11_MR __mrreg__(PWM11_CHANNEL)
#if (PWM11_PORT == 1)
#define PWM11_PINSEL 2
#define PWM11_FUNC 2
#elif (PWM11_PORT == 2)
#define PWM11_PINSEL 1
#define PWM11_FUNC 1
#else
#define PWM11_PINSEL 0
#define PWM11_FUNC 0
#endif
#define DIO35_PINSEL PWM11_PINSEL
#define DIO35_FUNC PWM11_FUNC
#define DIO35_CHANNEL PWM11_CHANNEL
#define DIO35_MR PWM11_MR
#endif
#ifdef PWM12
#define PWM12_MR __mrreg__(PWM12_CHANNEL)
#if (PWM12_PORT == 1)
#define PWM12_PINSEL 2
#define PWM12_FUNC 2
#elif (PWM12_PORT == 2)
#define PWM12_PINSEL 1
#define PWM12_FUNC 1
#else
#define PWM12_PINSEL 0
#define PWM12_FUNC 0
#endif
#define DIO36_PINSEL PWM12_PINSEL
#define DIO36_FUNC PWM12_FUNC
#define DIO36_CHANNEL PWM12_CHANNEL
#define DIO36_MR PWM12_MR
#endif
#ifdef PWM13
#define PWM13_MR __mrreg__(PWM13_CHANNEL)
#if (PWM13_PORT == 1)
#define PWM13_PINSEL 2
#define PWM13_FUNC 2
#elif (PWM13_PORT == 2)
#define PWM13_PINSEL 1
#define PWM13_FUNC 1
#else
#define PWM13_PINSEL 0
#define PWM13_FUNC 0
#endif
#define DIO37_PINSEL PWM13_PINSEL
#define DIO37_FUNC PWM13_FUNC
#define DIO37_CHANNEL PWM13_CHANNEL
#define DIO37_MR PWM13_MR
#endif
#ifdef PWM14
#define PWM14_MR __mrreg__(PWM14_CHANNEL)
#if (PWM14_PORT == 1)
#define PWM14_PINSEL 2
#define PWM14_FUNC 2
#elif (PWM14_PORT == 2)
#define PWM14_PINSEL 1
#define PWM14_FUNC 1
#else
#define PWM14_PINSEL 0
#define PWM14_FUNC 0
#endif
#define DIO38_PINSEL PWM14_PINSEL
#define DIO38_FUNC PWM14_FUNC
#define DIO38_CHANNEL PWM14_CHANNEL
#define DIO38_MR PWM14_MR
#endif
#ifdef PWM15
#define PWM15_MR __mrreg__(PWM15_CHANNEL)
#if (PWM15_PORT == 1)
#define PWM15_PINSEL 2
#define PWM15_FUNC 2
#elif (PWM15_PORT == 2)
#define PWM15_PINSEL 1
#define PWM15_FUNC 1
#else
#define PWM15_PINSEL 0
#define PWM15_FUNC 0
#endif
#define DIO39_PINSEL PWM15_PINSEL
#define DIO39_FUNC PWM15_FUNC
#define DIO39_CHANNEL PWM15_CHANNEL
#define DIO39_MR PWM15_MR
#endif

#define PWM_LER ((1 << PWM0_CHANNEL) | (1 << PWM1_CHANNEL) | (1 << PWM2_CHANNEL) | (1 << PWM3_CHANNEL) | (1 << PWM4_CHANNEL) | (1 << PWM5_CHANNEL) | (1 << PWM6_CHANNEL) | (1 << PWM7_CHANNEL) | (1 << PWM8_CHANNEL) | (1 << PWM9_CHANNEL) | (1 << PWM10_CHANNEL) | (1 << PWM11_CHANNEL) | (1 << PWM12_CHANNEL) | (1 << PWM13_CHANNEL) | (1 << PWM14_CHANNEL) | (1 << PWM15_CHANNEL))
#define PWM_ENA ((1 << (9 + PWM0_CHANNEL)) | (1 << (9 + PWM1_CHANNEL)) | (1 << (9 + PWM2_CHANNEL)) | (1 << (9 + PWM3_CHANNEL)) | (1 << (9 + PWM4_CHANNEL)) | (1 << (9 + PWM5_CHANNEL)) | (1 << (9 + PWM6_CHANNEL)) | (1 << (9 + PWM7_CHANNEL)) | (1 << (9 + PWM8_CHANNEL)) | (1 << (9 + PWM9_CHANNEL)) | (1 << (9 + PWM10_CHANNEL)) | (1 << (9 + PWM11_CHANNEL)) | (1 << (9 + PWM12_CHANNEL)) | (1 << (9 + PWM13_CHANNEL)) | (1 << (9 + PWM14_CHANNEL)) | (1 << (9 + PWM15_CHANNEL)))

// COM registers
#if (INTERFACE == INTERFACE_UART)
#ifndef COM_NUMBER
#define COM_NUMBER 0
#endif
#define COM_PCONP0 3
#define COM_PCONP1 4
#define COM_PCONP2 24
#define COM_PCONP3 25
#define __com_pconp__(X) COM_PCONP##X
#define __compconp__(X) __com_pconp__(X)

// this MCU does not work well with both TX and RX interrupt
// this forces the sync TX method to fix communication
#define COM_USART __helper__(LPC_UART, COM_NUMBER, )
#define COM_IRQ __helper__(UART, COM_NUMBER, _IRQn)
#define COM_PCLK __helper__(CLKPWR_PCLKSEL_UART, COM_NUMBER, )
#if (!defined(ENABLE_SYNC_TX) || !defined(ENABLE_SYNC_RX))
#define MCU_COM_ISR __helper__(UART, COM_NUMBER, _IRQHandler)
#endif

#define COM_OUTREG (COM_USART)->THR
#define COM_INREG (COM_USART)->RBR

#if (COM_NUMBER == 1 || COM_NUMBER == 2)
#if (TX_PORT == 2)
#define TX_ALT_FUNC 2
#endif
#if (RX_PORT == 2)
#define RX_ALT_FUNC 2
#endif
#endif
#if (COM_NUMBER == 3)
#if (TX_PORT == 0)
#define TX_ALT_FUNC 2
#else
#define TX_ALT_FUNC 3
#endif
#if (RX_PORT == 0)
#define RX_ALT_FUNC 2
#else
#define RX_ALT_FUNC 3
#endif
#endif

#ifndef TX_ALT_FUNC
#define TX_ALT_FUNC 1
#endif
#ifndef RX_ALT_FUNC
#define RX_ALT_FUNC 1
#endif

#endif

//SPI
#if (defined(SPI_CLK) && defined(SPI_SDI) && defined(SPI_SDO))
#define MCU_HAS_SPI
#ifndef SPI_MODE
#define SPI_MODE 0
#endif
#ifndef SPI_FREQ
#define SPI_FREQ 1000000UL
#endif

#include "lpc17xx_spi.h"

#endif

#ifndef ITP_TIMER
#define ITP_TIMER 0
#endif
#define ITP_TIMER_REG __helper__(LPC_TIM, ITP_TIMER, )
#define MCU_ITP_ISR __helper__(TIMER, ITP_TIMER, _IRQHandler)
#define ITP_INT_FLAG __helper__(TIM_MR, ITP_TIMER, _INT)
#define ITP_TIMER_IRQ __helper__(TIMER, ITP_TIMER, _IRQn)

#define MCU_RTC_ISR SysTick_Handler

#ifndef SERVO_TIMER
#define SERVO_TIMER 1
#endif
#define SERVO_TIMER_REG __helper__(LPC_TIM, SERVO_TIMER, )
#define MCU_SERVO_ISR __helper__(TIMER, SERVO_TIMER, _IRQHandler)
#define SERVO_INT_FLAG __helper__(TIM_MR, SERVO_TIMER, _INT)
#define SERVO_TIMER_IRQ __helper__(TIMER, SERVO_TIMER, _IRQn)

// Indirect macro access
#define __indirect__ex__(X, Y) DIO##X##_##Y
#define __indirect__(X, Y) __indirect__ex__(X, Y)

#define mcu_config_output(diopin) SETBIT(__indirect__(diopin, GPIOREG)->FIODIR, __indirect__(diopin, BIT))
#define mcu_config_input(diopin) CLEARBIT(__indirect__(diopin, GPIOREG)->FIODIR, __indirect__(diopin, BIT))
#define mcu_config_analog(X) mcu_config_input(X)
#define mcu_config_input_isr(diopin)                                                   \
	{                                                                                  \
		SETBIT(LPC_GPIOINT->__indirect__(diopin, RISEREG), __indirect__(diopin, BIT)); \
		SETBIT(LPC_GPIOINT->__indirect__(diopin, FALLREG), __indirect__(diopin, BIT)); \
		NVIC_SetPriority(EINT3_IRQn, 5);                                               \
		NVIC_ClearPendingIRQ(EINT3_IRQn);                                              \
		NVIC_EnableIRQ(EINT3_IRQn);                                                    \
	}

#define mcu_config_pullup(diopin)                                                                                             \
	{                                                                                                                         \
		LPC_PINCON->__helper__(PINMODE, __indirect__(diopin, PINCON), ) &= ~(3 << (0x1F & (__indirect__(diopin, BIT) << 1))); \
	}

#define mcu_config_pwm(diopin)                                                                                                                                   \
	{                                                                                                                                                            \
		mcu_config_output(diopin);                                                                                                                               \
		CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCPWM1, ENABLE);                                                                                                          \
		CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_PWM1, CLKPWR_PCLKSEL_CCLK_DIV_4);                                                                                       \
		LPC_PWM1->IR = 0xFF & PWM_IR_BITMASK;                                                                                                                    \
		LPC_PWM1->TCR = 0;                                                                                                                                       \
		LPC_PWM1->CTCR = 0;                                                                                                                                      \
		LPC_PWM1->MCR = 0;                                                                                                                                       \
		LPC_PWM1->CCR = 0;                                                                                                                                       \
		LPC_PWM1->PCR &= 0xFF00;                                                                                                                                 \
		LPC_PWM1->LER |= (1UL << 0) | (1UL << __indirect__(diopin, CHANNEL));                                                                                    \
		LPC_PWM1->PCR |= (1UL << (8 + __indirect__(diopin, CHANNEL)));                                                                                           \
		LPC_PWM1->PR = (CLKPWR_GetPCLK(CLKPWR_PCLKSEL_PWM1) / (255 * 1000)) - 1;                                                                                 \
		LPC_PWM1->MCR = (1UL << 1);                                                                                                                              \
		LPC_PWM1->MR0 = 255;                                                                                                                                     \
		LPC_PWM1->TCR = (1UL << 3) | (1UL << 0);                                                                                                                 \
		mcu_config_output(diopin);                                                                                                                               \
		PINSEL_CFG_Type pwm = {__indirect__(diopin, PORT), __indirect__(diopin, BIT), __indirect__(diopin, FUNC), PINSEL_PINMODE_PULLUP, PINSEL_PINMODE_NORMAL}; \
		PINSEL_ConfigPin(&pwm);                                                                                                                                  \
		mcu_set_pwm(diopin, 0);                                                                                                                                  \
	}

#define mcu_get_input(diopin) CHECKBIT(__indirect__(diopin, GPIOREG)->FIOPIN, __indirect__(diopin, BIT))
#define mcu_get_output(diopin) CHECKBIT(__indirect__(diopin, GPIOREG)->FIOPIN, __indirect__(diopin, BIT))
#define mcu_set_output(diopin) (__indirect__(diopin, GPIOREG)->FIOSET = (1 << __indirect__(diopin, BIT)))
#define mcu_clear_output(diopin) (__indirect__(diopin, GPIOREG)->FIOCLR = (1 << __indirect__(diopin, BIT)))
#define mcu_toggle_output(diopin) TOGGLEBIT(__indirect__(diopin, GPIOREG)->FIOPIN, __indirect__(diopin, BIT))
#define mcu_set_pwm(diopin, pwmvalue)                                                                                      \
	{                                                                                                                      \
		LPC_PWM1->__indirect__(diopin, MR) = pwmvalue;                                                                     \
		LPC_PWM1->LER |= ((uint32_t)(((__indirect__(diopin, CHANNEL)) < 7) ? (1 << (__indirect__(diopin, CHANNEL))) : 0)); \
		LPC_PWM1->TCR |= ((uint32_t)(1 << 1));                                                                             \
		LPC_PWM1->TCR &= (~((uint32_t)(1 << 1))) & ((uint32_t)(0x0000000B));                                               \
	}
#define mcu_get_pwm(diopin) LPC_PWM1->__indirect__(diopin, MR)

	extern volatile bool lpc_global_isr_enabled;
#define mcu_enable_global_isr()        \
	{                                  \
		__enable_irq();                \
		lpc_global_isr_enabled = true; \
	}
#define mcu_disable_global_isr()        \
	{                                   \
		lpc_global_isr_enabled = false; \
		__disable_irq();                \
	}
#define mcu_get_global_isr() lpc_global_isr_enabled

#if (INTERFACE == INTERFACE_UART)
#define mcu_rx_ready() (CHECKBIT(COM_USART->LSR, 0))
#define mcu_tx_ready() (CHECKBIT(COM_USART->LSR, 5))
#elif (INTERFACE == INTERFACE_USB)
extern uint32_t tud_cdc_n_write_available(uint8_t itf);
extern uint32_t tud_cdc_n_available(uint8_t itf);
#define mcu_rx_ready() tud_cdc_n_available(0)
#define mcu_tx_ready() tud_cdc_n_write_available(0)
#endif

#define mcu_spi_xmit(X)               \
	{                                 \
		LPC_SPI->SPDR = X;                     \
		while (!(LPC_SPI->SPSR & SPI_SPSR_SPIF)); \
		LPC_SPI->SPDR;                         \
	}

#ifdef __cplusplus
}
#endif

#endif