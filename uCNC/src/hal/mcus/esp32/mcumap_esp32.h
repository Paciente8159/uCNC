/*
	Name: mcumap_esp32.h
	Description: Contains all MCU and PIN definitions for Arduino ESP32 to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 05-02-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef MCUMAP_ESP32_H
#define MCUMAP_ESP32_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <Arduino.h>
#include "driver/timer.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/ledc.h"

/*
	Generates all the interface definitions.
	This creates a middle HAL layer between the board IO pins and the AVR funtionalities
*/
/*
	MCU specific definitions and replacements
*/

/*
	ESP32 Defaults
*/
// defines the frequency of the mcu
#ifndef F_CPU
#define F_CPU 240000000L
#endif
// defines the maximum and minimum step rates
#ifndef F_STEP_MAX
#define F_STEP_MAX 125000UL
#endif
#ifndef F_STEP_MIN
#define F_STEP_MIN 1
#endif

#ifndef USE_CUSTOM_EEPROM_LIBRARY
#ifndef USE_ARDUINO_EEPROM_LIBRARY
#define USE_ARDUINO_EEPROM_LIBRARY
#endif
#endif

// defines special mcu to access flash strings and arrays
#define __rom__
#define __romstr__
#define rom_strptr *
#define rom_strcpy strcpy
#define rom_strncpy strncpy
#define rom_memcpy memcpy
#define rom_read_byte *

#define __ATOMIC__
#define __ATOMIC_FORCEON__

// needed by software delays
#ifndef MCU_CLOCKS_PER_CYCLE
#define MCU_CLOCKS_PER_CYCLE 1
#endif
#ifndef MCU_CYCLES_PER_LOOP
#define MCU_CYCLES_PER_LOOP 1
#endif
#ifndef MCU_CYCLES_PER_LOOP_OVERHEAD
#define MCU_CYCLES_PER_LOOP_OVERHEAD 0
#endif

#ifndef MCU_CALLBACK
#define MCU_CALLBACK IRAM_ATTR
#endif

#ifdef ENABLE_RX_SYNC
#define MCU_RX_CALLBACK ICACHE_FLASH_ATTR
#endif

#ifdef ENABLE_TX_SYNC
#define MCU_TX_CALLBACK ICACHE_FLASH_ATTR
#endif

#define MCU_IO_CALLBACK ICACHE_FLASH_ATTR

#ifdef RX_BUFFER_CAPACITY
#define RX_BUFFER_CAPACITY 255
#endif

#define __SIZEOF_FLOAT__ 4

// used by the parser
// this method is faster then normal multiplication (for 32 bit for 16 and 8 bits is slightly lower)
// overrides utils.h definition to implement this method with or without fast math option enabled
#define fast_int_mul10(x) ((((x) << 2) + (x)) << 1)

// PINNAMES for ESP32
#define PERIPHS_IO_MUX_GPIO0 (PERIPHS_IO_MUX + 0x34)
#define PERIPHS_IO_MUX_GPIO1 (PERIPHS_IO_MUX + 0x18)
#define PERIPHS_IO_MUX_GPIO2 (PERIPHS_IO_MUX + 0x38)
#define PERIPHS_IO_MUX_GPIO3 (PERIPHS_IO_MUX + 0x14)
#define PERIPHS_IO_MUX_GPIO4 (PERIPHS_IO_MUX + 0x3C)
#define PERIPHS_IO_MUX_GPIO5 (PERIPHS_IO_MUX + 0x40)
#define PERIPHS_IO_MUX_GPIO6 (PERIPHS_IO_MUX + 0x1c)
#define PERIPHS_IO_MUX_GPIO7 (PERIPHS_IO_MUX + 0x20)
#define PERIPHS_IO_MUX_GPIO8 (PERIPHS_IO_MUX + 0x24)
#define PERIPHS_IO_MUX_GPIO9 (PERIPHS_IO_MUX + 0x28)
#define PERIPHS_IO_MUX_GPIO10 (PERIPHS_IO_MUX + 0x2c)
#define PERIPHS_IO_MUX_GPIO11 (PERIPHS_IO_MUX + 0x30)
#define PERIPHS_IO_MUX_GPIO12 (PERIPHS_IO_MUX + 0x04)
#define PERIPHS_IO_MUX_GPIO13 (PERIPHS_IO_MUX + 0x08)
#define PERIPHS_IO_MUX_GPIO14 (PERIPHS_IO_MUX + 0x0C)
#define PERIPHS_IO_MUX_GPIO15 (PERIPHS_IO_MUX + 0x10)

	typedef struct
	{
		volatile uint32_t OUT;
		volatile uint32_t OUTSET;
		volatile uint32_t OUTCLR;
	} IO_OUT_TypeDef;

	typedef struct
	{
		volatile uint32_t IN;
	} IO_IN_TypeDef;

#define OUT0 ((IO_OUT_TypeDef *)GPIO_OUT_REG)
#ifdef GPIO_OUT1_REG
#define OUT1 ((IO_OUT_TypeDef *)GPIO_OUT1_REG)
#endif

#define IN0 ((IO_IN_TypeDef *)GPIO_IN_REG)
#ifdef GPIO_IN1_REG
#define IN1 ((IO_IN_TypeDef *)GPIO_IN1_REG)
#endif

// IO pins
#if (defined(STEP0_BIT))
#define STEP0 1
#if (STEP0_BIT < 32)
#define STEP0_OUTREG OUT0
#define STEP0_INREG IN0
#else
#define STEP0_OUTREG OUT1
#define STEP0_INREG IN1
#endif
#define DIO1 1
#define DIO1_BIT STEP0_BIT
#define DIO1_OUTREG STEP0_OUTREG
#define DIO1_INREG STEP0_INREG
#endif
#if (defined(STEP1_BIT))
#define STEP1 2
#if (STEP1_BIT < 32)
#define STEP1_OUTREG OUT0
#define STEP1_INREG IN0
#else
#define STEP1_OUTREG OUT1
#define STEP1_INREG IN1
#endif
#define DIO2 2
#define DIO2_BIT STEP1_BIT
#define DIO2_OUTREG STEP1_OUTREG
#define DIO2_INREG STEP1_INREG
#endif
#if (defined(STEP2_BIT))
#define STEP2 3
#if (STEP2_BIT < 32)
#define STEP2_OUTREG OUT0
#define STEP2_INREG IN0
#else
#define STEP2_OUTREG OUT1
#define STEP2_INREG IN1
#endif
#define DIO3 3
#define DIO3_BIT STEP2_BIT
#define DIO3_OUTREG STEP2_OUTREG
#define DIO3_INREG STEP2_INREG
#endif
#if (defined(STEP3_BIT))
#define STEP3 4
#if (STEP3_BIT < 32)
#define STEP3_OUTREG OUT0
#define STEP3_INREG IN0
#else
#define STEP3_OUTREG OUT1
#define STEP3_INREG IN1
#endif
#define DIO4 4
#define DIO4_BIT STEP3_BIT
#define DIO4_OUTREG STEP3_OUTREG
#define DIO4_INREG STEP3_INREG
#endif
#if (defined(STEP4_BIT))
#define STEP4 5
#if (STEP4_BIT < 32)
#define STEP4_OUTREG OUT0
#define STEP4_INREG IN0
#else
#define STEP4_OUTREG OUT1
#define STEP4_INREG IN1
#endif
#define DIO5 5
#define DIO5_BIT STEP4_BIT
#define DIO5_OUTREG STEP4_OUTREG
#define DIO5_INREG STEP4_INREG
#endif
#if (defined(STEP5_BIT))
#define STEP5 6
#if (STEP5_BIT < 32)
#define STEP5_OUTREG OUT0
#define STEP5_INREG IN0
#else
#define STEP5_OUTREG OUT1
#define STEP5_INREG IN1
#endif
#define DIO6 6
#define DIO6_BIT STEP5_BIT
#define DIO6_OUTREG STEP5_OUTREG
#define DIO6_INREG STEP5_INREG
#endif
#if (defined(STEP6_BIT))
#define STEP6 7
#if (STEP6_BIT < 32)
#define STEP6_OUTREG OUT0
#define STEP6_INREG IN0
#else
#define STEP6_OUTREG OUT1
#define STEP6_INREG IN1
#endif
#define DIO7 7
#define DIO7_BIT STEP6_BIT
#define DIO7_OUTREG STEP6_OUTREG
#define DIO7_INREG STEP6_INREG
#endif
#if (defined(STEP7_BIT))
#define STEP7 8
#if (STEP7_BIT < 32)
#define STEP7_OUTREG OUT0
#define STEP7_INREG IN0
#else
#define STEP7_OUTREG OUT1
#define STEP7_INREG IN1
#endif
#define DIO8 8
#define DIO8_BIT STEP7_BIT
#define DIO8_OUTREG STEP7_OUTREG
#define DIO8_INREG STEP7_INREG
#endif
#if (defined(DIR0_BIT))
#define DIR0 9
#if (DIR0_BIT < 32)
#define DIR0_OUTREG OUT0
#define DIR0_INREG IN0
#else
#define DIR0_OUTREG OUT1
#define DIR0_INREG IN1
#endif
#define DIO9 9
#define DIO9_BIT DIR0_BIT
#define DIO9_OUTREG DIR0_OUTREG
#define DIO9_INREG DIR0_INREG
#endif
#if (defined(DIR1_BIT))
#define DIR1 10
#if (DIR1_BIT < 32)
#define DIR1_OUTREG OUT0
#define DIR1_INREG IN0
#else
#define DIR1_OUTREG OUT1
#define DIR1_INREG IN1
#endif
#define DIO10 10
#define DIO10_BIT DIR1_BIT
#define DIO10_OUTREG DIR1_OUTREG
#define DIO10_INREG DIR1_INREG
#endif
#if (defined(DIR2_BIT))
#define DIR2 11
#if (DIR2_BIT < 32)
#define DIR2_OUTREG OUT0
#define DIR2_INREG IN0
#else
#define DIR2_OUTREG OUT1
#define DIR2_INREG IN1
#endif
#define DIO11 11
#define DIO11_BIT DIR2_BIT
#define DIO11_OUTREG DIR2_OUTREG
#define DIO11_INREG DIR2_INREG
#endif
#if (defined(DIR3_BIT))
#define DIR3 12
#if (DIR3_BIT < 32)
#define DIR3_OUTREG OUT0
#define DIR3_INREG IN0
#else
#define DIR3_OUTREG OUT1
#define DIR3_INREG IN1
#endif
#define DIO12 12
#define DIO12_BIT DIR3_BIT
#define DIO12_OUTREG DIR3_OUTREG
#define DIO12_INREG DIR3_INREG
#endif
#if (defined(DIR4_BIT))
#define DIR4 13
#if (DIR4_BIT < 32)
#define DIR4_OUTREG OUT0
#define DIR4_INREG IN0
#else
#define DIR4_OUTREG OUT1
#define DIR4_INREG IN1
#endif
#define DIO13 13
#define DIO13_BIT DIR4_BIT
#define DIO13_OUTREG DIR4_OUTREG
#define DIO13_INREG DIR4_INREG
#endif
#if (defined(DIR5_BIT))
#define DIR5 14
#if (DIR5_BIT < 32)
#define DIR5_OUTREG OUT0
#define DIR5_INREG IN0
#else
#define DIR5_OUTREG OUT1
#define DIR5_INREG IN1
#endif
#define DIO14 14
#define DIO14_BIT DIR5_BIT
#define DIO14_OUTREG DIR5_OUTREG
#define DIO14_INREG DIR5_INREG
#endif
#if (defined(DIR6_BIT))
#define DIR6 15
#if (DIR6_BIT < 32)
#define DIR6_OUTREG OUT0
#define DIR6_INREG IN0
#else
#define DIR6_OUTREG OUT1
#define DIR6_INREG IN1
#endif
#define DIO15 15
#define DIO15_BIT DIR6_BIT
#define DIO15_OUTREG DIR6_OUTREG
#define DIO15_INREG DIR6_INREG
#endif
#if (defined(DIR7_BIT))
#define DIR7 16
#if (DIR7_BIT < 32)
#define DIR7_OUTREG OUT0
#define DIR7_INREG IN0
#else
#define DIR7_OUTREG OUT1
#define DIR7_INREG IN1
#endif
#define DIO16 16
#define DIO16_BIT DIR7_BIT
#define DIO16_OUTREG DIR7_OUTREG
#define DIO16_INREG DIR7_INREG
#endif
#if (defined(STEP0_EN_BIT))
#define STEP0_EN 17
#if (STEP0_EN_BIT < 32)
#define STEP0_EN_OUTREG OUT0
#define STEP0_EN_INREG IN0
#else
#define STEP0_EN_OUTREG OUT1
#define STEP0_EN_INREG IN1
#endif
#define DIO17 17
#define DIO17_BIT STEP0_EN_BIT
#define DIO17_OUTREG STEP0_EN_OUTREG
#define DIO17_INREG STEP0_EN_INREG
#endif
#if (defined(STEP1_EN_BIT))
#define STEP1_EN 18
#if (STEP1_EN_BIT < 32)
#define STEP1_EN_OUTREG OUT0
#define STEP1_EN_INREG IN0
#else
#define STEP1_EN_OUTREG OUT1
#define STEP1_EN_INREG IN1
#endif
#define DIO18 18
#define DIO18_BIT STEP1_EN_BIT
#define DIO18_OUTREG STEP1_EN_OUTREG
#define DIO18_INREG STEP1_EN_INREG
#endif
#if (defined(STEP2_EN_BIT))
#define STEP2_EN 19
#if (STEP2_EN_BIT < 32)
#define STEP2_EN_OUTREG OUT0
#define STEP2_EN_INREG IN0
#else
#define STEP2_EN_OUTREG OUT1
#define STEP2_EN_INREG IN1
#endif
#define DIO19 19
#define DIO19_BIT STEP2_EN_BIT
#define DIO19_OUTREG STEP2_EN_OUTREG
#define DIO19_INREG STEP2_EN_INREG
#endif
#if (defined(STEP3_EN_BIT))
#define STEP3_EN 20
#if (STEP3_EN_BIT < 32)
#define STEP3_EN_OUTREG OUT0
#define STEP3_EN_INREG IN0
#else
#define STEP3_EN_OUTREG OUT1
#define STEP3_EN_INREG IN1
#endif
#define DIO20 20
#define DIO20_BIT STEP3_EN_BIT
#define DIO20_OUTREG STEP3_EN_OUTREG
#define DIO20_INREG STEP3_EN_INREG
#endif
#if (defined(STEP4_EN_BIT))
#define STEP4_EN 21
#if (STEP4_EN_BIT < 32)
#define STEP4_EN_OUTREG OUT0
#define STEP4_EN_INREG IN0
#else
#define STEP4_EN_OUTREG OUT1
#define STEP4_EN_INREG IN1
#endif
#define DIO21 21
#define DIO21_BIT STEP4_EN_BIT
#define DIO21_OUTREG STEP4_EN_OUTREG
#define DIO21_INREG STEP4_EN_INREG
#endif
#if (defined(STEP5_EN_BIT))
#define STEP5_EN 22
#if (STEP5_EN_BIT < 32)
#define STEP5_EN_OUTREG OUT0
#define STEP5_EN_INREG IN0
#else
#define STEP5_EN_OUTREG OUT1
#define STEP5_EN_INREG IN1
#endif
#define DIO22 22
#define DIO22_BIT STEP5_EN_BIT
#define DIO22_OUTREG STEP5_EN_OUTREG
#define DIO22_INREG STEP5_EN_INREG
#endif
#if (defined(STEP6_EN_BIT))
#define STEP6_EN 23
#if (STEP6_EN_BIT < 32)
#define STEP6_EN_OUTREG OUT0
#define STEP6_EN_INREG IN0
#else
#define STEP6_EN_OUTREG OUT1
#define STEP6_EN_INREG IN1
#endif
#define DIO23 23
#define DIO23_BIT STEP6_EN_BIT
#define DIO23_OUTREG STEP6_EN_OUTREG
#define DIO23_INREG STEP6_EN_INREG
#endif
#if (defined(STEP7_EN_BIT))
#define STEP7_EN 24
#if (STEP7_EN_BIT < 32)
#define STEP7_EN_OUTREG OUT0
#define STEP7_EN_INREG IN0
#else
#define STEP7_EN_OUTREG OUT1
#define STEP7_EN_INREG IN1
#endif
#define DIO24 24
#define DIO24_BIT STEP7_EN_BIT
#define DIO24_OUTREG STEP7_EN_OUTREG
#define DIO24_INREG STEP7_EN_INREG
#endif
#if (defined(PWM0_BIT))
#define PWM0 25
#if (PWM0_BIT < 32)
#define PWM0_OUTREG OUT0
#define PWM0_INREG IN0
#else
#define PWM0_OUTREG OUT1
#define PWM0_INREG IN1
#endif
#define DIO25 25
#define DIO25_BIT PWM0_BIT
#define DIO25_OUTREG PWM0_OUTREG
#define DIO25_INREG PWM0_INREG
#endif
#if (defined(PWM1_BIT))
#define PWM1 26
#if (PWM1_BIT < 32)
#define PWM1_OUTREG OUT0
#define PWM1_INREG IN0
#else
#define PWM1_OUTREG OUT1
#define PWM1_INREG IN1
#endif
#define DIO26 26
#define DIO26_BIT PWM1_BIT
#define DIO26_OUTREG PWM1_OUTREG
#define DIO26_INREG PWM1_INREG
#endif
#if (defined(PWM2_BIT))
#define PWM2 27
#if (PWM2_BIT < 32)
#define PWM2_OUTREG OUT0
#define PWM2_INREG IN0
#else
#define PWM2_OUTREG OUT1
#define PWM2_INREG IN1
#endif
#define DIO27 27
#define DIO27_BIT PWM2_BIT
#define DIO27_OUTREG PWM2_OUTREG
#define DIO27_INREG PWM2_INREG
#endif
#if (defined(PWM3_BIT))
#define PWM3 28
#if (PWM3_BIT < 32)
#define PWM3_OUTREG OUT0
#define PWM3_INREG IN0
#else
#define PWM3_OUTREG OUT1
#define PWM3_INREG IN1
#endif
#define DIO28 28
#define DIO28_BIT PWM3_BIT
#define DIO28_OUTREG PWM3_OUTREG
#define DIO28_INREG PWM3_INREG
#endif
#if (defined(PWM4_BIT))
#define PWM4 29
#if (PWM4_BIT < 32)
#define PWM4_OUTREG OUT0
#define PWM4_INREG IN0
#else
#define PWM4_OUTREG OUT1
#define PWM4_INREG IN1
#endif
#define DIO29 29
#define DIO29_BIT PWM4_BIT
#define DIO29_OUTREG PWM4_OUTREG
#define DIO29_INREG PWM4_INREG
#endif
#if (defined(PWM5_BIT))
#define PWM5 30
#if (PWM5_BIT < 32)
#define PWM5_OUTREG OUT0
#define PWM5_INREG IN0
#else
#define PWM5_OUTREG OUT1
#define PWM5_INREG IN1
#endif
#define DIO30 30
#define DIO30_BIT PWM5_BIT
#define DIO30_OUTREG PWM5_OUTREG
#define DIO30_INREG PWM5_INREG
#endif
#if (defined(PWM6_BIT))
#define PWM6 31
#if (PWM6_BIT < 32)
#define PWM6_OUTREG OUT0
#define PWM6_INREG IN0
#else
#define PWM6_OUTREG OUT1
#define PWM6_INREG IN1
#endif
#define DIO31 31
#define DIO31_BIT PWM6_BIT
#define DIO31_OUTREG PWM6_OUTREG
#define DIO31_INREG PWM6_INREG
#endif
#if (defined(PWM7_BIT))
#define PWM7 32
#if (PWM7_BIT < 32)
#define PWM7_OUTREG OUT0
#define PWM7_INREG IN0
#else
#define PWM7_OUTREG OUT1
#define PWM7_INREG IN1
#endif
#define DIO32 32
#define DIO32_BIT PWM7_BIT
#define DIO32_OUTREG PWM7_OUTREG
#define DIO32_INREG PWM7_INREG
#endif
#if (defined(PWM8_BIT))
#define PWM8 33
#if (PWM8_BIT < 32)
#define PWM8_OUTREG OUT0
#define PWM8_INREG IN0
#else
#define PWM8_OUTREG OUT1
#define PWM8_INREG IN1
#endif
#define DIO33 33
#define DIO33_BIT PWM8_BIT
#define DIO33_OUTREG PWM8_OUTREG
#define DIO33_INREG PWM8_INREG
#endif
#if (defined(PWM9_BIT))
#define PWM9 34
#if (PWM9_BIT < 32)
#define PWM9_OUTREG OUT0
#define PWM9_INREG IN0
#else
#define PWM9_OUTREG OUT1
#define PWM9_INREG IN1
#endif
#define DIO34 34
#define DIO34_BIT PWM9_BIT
#define DIO34_OUTREG PWM9_OUTREG
#define DIO34_INREG PWM9_INREG
#endif
#if (defined(PWM10_BIT))
#define PWM10 35
#if (PWM10_BIT < 32)
#define PWM10_OUTREG OUT0
#define PWM10_INREG IN0
#else
#define PWM10_OUTREG OUT1
#define PWM10_INREG IN1
#endif
#define DIO35 35
#define DIO35_BIT PWM10_BIT
#define DIO35_OUTREG PWM10_OUTREG
#define DIO35_INREG PWM10_INREG
#endif
#if (defined(PWM11_BIT))
#define PWM11 36
#if (PWM11_BIT < 32)
#define PWM11_OUTREG OUT0
#define PWM11_INREG IN0
#else
#define PWM11_OUTREG OUT1
#define PWM11_INREG IN1
#endif
#define DIO36 36
#define DIO36_BIT PWM11_BIT
#define DIO36_OUTREG PWM11_OUTREG
#define DIO36_INREG PWM11_INREG
#endif
#if (defined(PWM12_BIT))
#define PWM12 37
#if (PWM12_BIT < 32)
#define PWM12_OUTREG OUT0
#define PWM12_INREG IN0
#else
#define PWM12_OUTREG OUT1
#define PWM12_INREG IN1
#endif
#define DIO37 37
#define DIO37_BIT PWM12_BIT
#define DIO37_OUTREG PWM12_OUTREG
#define DIO37_INREG PWM12_INREG
#endif
#if (defined(PWM13_BIT))
#define PWM13 38
#if (PWM13_BIT < 32)
#define PWM13_OUTREG OUT0
#define PWM13_INREG IN0
#else
#define PWM13_OUTREG OUT1
#define PWM13_INREG IN1
#endif
#define DIO38 38
#define DIO38_BIT PWM13_BIT
#define DIO38_OUTREG PWM13_OUTREG
#define DIO38_INREG PWM13_INREG
#endif
#if (defined(PWM14_BIT))
#define PWM14 39
#if (PWM14_BIT < 32)
#define PWM14_OUTREG OUT0
#define PWM14_INREG IN0
#else
#define PWM14_OUTREG OUT1
#define PWM14_INREG IN1
#endif
#define DIO39 39
#define DIO39_BIT PWM14_BIT
#define DIO39_OUTREG PWM14_OUTREG
#define DIO39_INREG PWM14_INREG
#endif
#if (defined(PWM15_BIT))
#define PWM15 40
#if (PWM15_BIT < 32)
#define PWM15_OUTREG OUT0
#define PWM15_INREG IN0
#else
#define PWM15_OUTREG OUT1
#define PWM15_INREG IN1
#endif
#define DIO40 40
#define DIO40_BIT PWM15_BIT
#define DIO40_OUTREG PWM15_OUTREG
#define DIO40_INREG PWM15_INREG
#endif
#if (defined(SERVO0_BIT))
#define SERVO0 41
#if (SERVO0_BIT < 32)
#define SERVO0_OUTREG OUT0
#define SERVO0_INREG IN0
#else
#define SERVO0_OUTREG OUT1
#define SERVO0_INREG IN1
#endif
#define DIO41 41
#define DIO41_BIT SERVO0_BIT
#define DIO41_OUTREG SERVO0_OUTREG
#define DIO41_INREG SERVO0_INREG
#endif
#if (defined(SERVO1_BIT))
#define SERVO1 42
#if (SERVO1_BIT < 32)
#define SERVO1_OUTREG OUT0
#define SERVO1_INREG IN0
#else
#define SERVO1_OUTREG OUT1
#define SERVO1_INREG IN1
#endif
#define DIO42 42
#define DIO42_BIT SERVO1_BIT
#define DIO42_OUTREG SERVO1_OUTREG
#define DIO42_INREG SERVO1_INREG
#endif
#if (defined(SERVO2_BIT))
#define SERVO2 43
#if (SERVO2_BIT < 32)
#define SERVO2_OUTREG OUT0
#define SERVO2_INREG IN0
#else
#define SERVO2_OUTREG OUT1
#define SERVO2_INREG IN1
#endif
#define DIO43 43
#define DIO43_BIT SERVO2_BIT
#define DIO43_OUTREG SERVO2_OUTREG
#define DIO43_INREG SERVO2_INREG
#endif
#if (defined(SERVO3_BIT))
#define SERVO3 44
#if (SERVO3_BIT < 32)
#define SERVO3_OUTREG OUT0
#define SERVO3_INREG IN0
#else
#define SERVO3_OUTREG OUT1
#define SERVO3_INREG IN1
#endif
#define DIO44 44
#define DIO44_BIT SERVO3_BIT
#define DIO44_OUTREG SERVO3_OUTREG
#define DIO44_INREG SERVO3_INREG
#endif
#if (defined(SERVO4_BIT))
#define SERVO4 45
#if (SERVO4_BIT < 32)
#define SERVO4_OUTREG OUT0
#define SERVO4_INREG IN0
#else
#define SERVO4_OUTREG OUT1
#define SERVO4_INREG IN1
#endif
#define DIO45 45
#define DIO45_BIT SERVO4_BIT
#define DIO45_OUTREG SERVO4_OUTREG
#define DIO45_INREG SERVO4_INREG
#endif
#if (defined(SERVO5_BIT))
#define SERVO5 46
#if (SERVO5_BIT < 32)
#define SERVO5_OUTREG OUT0
#define SERVO5_INREG IN0
#else
#define SERVO5_OUTREG OUT1
#define SERVO5_INREG IN1
#endif
#define DIO46 46
#define DIO46_BIT SERVO5_BIT
#define DIO46_OUTREG SERVO5_OUTREG
#define DIO46_INREG SERVO5_INREG
#endif
#if (defined(DOUT0_BIT))
#define DOUT0 47
#if (DOUT0_BIT < 32)
#define DOUT0_OUTREG OUT0
#define DOUT0_INREG IN0
#else
#define DOUT0_OUTREG OUT1
#define DOUT0_INREG IN1
#endif
#define DIO47 47
#define DIO47_BIT DOUT0_BIT
#define DIO47_OUTREG DOUT0_OUTREG
#define DIO47_INREG DOUT0_INREG
#endif
#if (defined(DOUT1_BIT))
#define DOUT1 48
#if (DOUT1_BIT < 32)
#define DOUT1_OUTREG OUT0
#define DOUT1_INREG IN0
#else
#define DOUT1_OUTREG OUT1
#define DOUT1_INREG IN1
#endif
#define DIO48 48
#define DIO48_BIT DOUT1_BIT
#define DIO48_OUTREG DOUT1_OUTREG
#define DIO48_INREG DOUT1_INREG
#endif
#if (defined(DOUT2_BIT))
#define DOUT2 49
#if (DOUT2_BIT < 32)
#define DOUT2_OUTREG OUT0
#define DOUT2_INREG IN0
#else
#define DOUT2_OUTREG OUT1
#define DOUT2_INREG IN1
#endif
#define DIO49 49
#define DIO49_BIT DOUT2_BIT
#define DIO49_OUTREG DOUT2_OUTREG
#define DIO49_INREG DOUT2_INREG
#endif
#if (defined(DOUT3_BIT))
#define DOUT3 50
#if (DOUT3_BIT < 32)
#define DOUT3_OUTREG OUT0
#define DOUT3_INREG IN0
#else
#define DOUT3_OUTREG OUT1
#define DOUT3_INREG IN1
#endif
#define DIO50 50
#define DIO50_BIT DOUT3_BIT
#define DIO50_OUTREG DOUT3_OUTREG
#define DIO50_INREG DOUT3_INREG
#endif
#if (defined(DOUT4_BIT))
#define DOUT4 51
#if (DOUT4_BIT < 32)
#define DOUT4_OUTREG OUT0
#define DOUT4_INREG IN0
#else
#define DOUT4_OUTREG OUT1
#define DOUT4_INREG IN1
#endif
#define DIO51 51
#define DIO51_BIT DOUT4_BIT
#define DIO51_OUTREG DOUT4_OUTREG
#define DIO51_INREG DOUT4_INREG
#endif
#if (defined(DOUT5_BIT))
#define DOUT5 52
#if (DOUT5_BIT < 32)
#define DOUT5_OUTREG OUT0
#define DOUT5_INREG IN0
#else
#define DOUT5_OUTREG OUT1
#define DOUT5_INREG IN1
#endif
#define DIO52 52
#define DIO52_BIT DOUT5_BIT
#define DIO52_OUTREG DOUT5_OUTREG
#define DIO52_INREG DOUT5_INREG
#endif
#if (defined(DOUT6_BIT))
#define DOUT6 53
#if (DOUT6_BIT < 32)
#define DOUT6_OUTREG OUT0
#define DOUT6_INREG IN0
#else
#define DOUT6_OUTREG OUT1
#define DOUT6_INREG IN1
#endif
#define DIO53 53
#define DIO53_BIT DOUT6_BIT
#define DIO53_OUTREG DOUT6_OUTREG
#define DIO53_INREG DOUT6_INREG
#endif
#if (defined(DOUT7_BIT))
#define DOUT7 54
#if (DOUT7_BIT < 32)
#define DOUT7_OUTREG OUT0
#define DOUT7_INREG IN0
#else
#define DOUT7_OUTREG OUT1
#define DOUT7_INREG IN1
#endif
#define DIO54 54
#define DIO54_BIT DOUT7_BIT
#define DIO54_OUTREG DOUT7_OUTREG
#define DIO54_INREG DOUT7_INREG
#endif
#if (defined(DOUT8_BIT))
#define DOUT8 55
#if (DOUT8_BIT < 32)
#define DOUT8_OUTREG OUT0
#define DOUT8_INREG IN0
#else
#define DOUT8_OUTREG OUT1
#define DOUT8_INREG IN1
#endif
#define DIO55 55
#define DIO55_BIT DOUT8_BIT
#define DIO55_OUTREG DOUT8_OUTREG
#define DIO55_INREG DOUT8_INREG
#endif
#if (defined(DOUT9_BIT))
#define DOUT9 56
#if (DOUT9_BIT < 32)
#define DOUT9_OUTREG OUT0
#define DOUT9_INREG IN0
#else
#define DOUT9_OUTREG OUT1
#define DOUT9_INREG IN1
#endif
#define DIO56 56
#define DIO56_BIT DOUT9_BIT
#define DIO56_OUTREG DOUT9_OUTREG
#define DIO56_INREG DOUT9_INREG
#endif
#if (defined(DOUT10_BIT))
#define DOUT10 57
#if (DOUT10_BIT < 32)
#define DOUT10_OUTREG OUT0
#define DOUT10_INREG IN0
#else
#define DOUT10_OUTREG OUT1
#define DOUT10_INREG IN1
#endif
#define DIO57 57
#define DIO57_BIT DOUT10_BIT
#define DIO57_OUTREG DOUT10_OUTREG
#define DIO57_INREG DOUT10_INREG
#endif
#if (defined(DOUT11_BIT))
#define DOUT11 58
#if (DOUT11_BIT < 32)
#define DOUT11_OUTREG OUT0
#define DOUT11_INREG IN0
#else
#define DOUT11_OUTREG OUT1
#define DOUT11_INREG IN1
#endif
#define DIO58 58
#define DIO58_BIT DOUT11_BIT
#define DIO58_OUTREG DOUT11_OUTREG
#define DIO58_INREG DOUT11_INREG
#endif
#if (defined(DOUT12_BIT))
#define DOUT12 59
#if (DOUT12_BIT < 32)
#define DOUT12_OUTREG OUT0
#define DOUT12_INREG IN0
#else
#define DOUT12_OUTREG OUT1
#define DOUT12_INREG IN1
#endif
#define DIO59 59
#define DIO59_BIT DOUT12_BIT
#define DIO59_OUTREG DOUT12_OUTREG
#define DIO59_INREG DOUT12_INREG
#endif
#if (defined(DOUT13_BIT))
#define DOUT13 60
#if (DOUT13_BIT < 32)
#define DOUT13_OUTREG OUT0
#define DOUT13_INREG IN0
#else
#define DOUT13_OUTREG OUT1
#define DOUT13_INREG IN1
#endif
#define DIO60 60
#define DIO60_BIT DOUT13_BIT
#define DIO60_OUTREG DOUT13_OUTREG
#define DIO60_INREG DOUT13_INREG
#endif
#if (defined(DOUT14_BIT))
#define DOUT14 61
#if (DOUT14_BIT < 32)
#define DOUT14_OUTREG OUT0
#define DOUT14_INREG IN0
#else
#define DOUT14_OUTREG OUT1
#define DOUT14_INREG IN1
#endif
#define DIO61 61
#define DIO61_BIT DOUT14_BIT
#define DIO61_OUTREG DOUT14_OUTREG
#define DIO61_INREG DOUT14_INREG
#endif
#if (defined(DOUT15_BIT))
#define DOUT15 62
#if (DOUT15_BIT < 32)
#define DOUT15_OUTREG OUT0
#define DOUT15_INREG IN0
#else
#define DOUT15_OUTREG OUT1
#define DOUT15_INREG IN1
#endif
#define DIO62 62
#define DIO62_BIT DOUT15_BIT
#define DIO62_OUTREG DOUT15_OUTREG
#define DIO62_INREG DOUT15_INREG
#endif
#if (defined(DOUT16_BIT))
#define DOUT16 63
#if (DOUT16_BIT < 32)
#define DOUT16_OUTREG OUT0
#define DOUT16_INREG IN0
#else
#define DOUT16_OUTREG OUT1
#define DOUT16_INREG IN1
#endif
#define DIO63 63
#define DIO63_BIT DOUT16_BIT
#define DIO63_OUTREG DOUT16_OUTREG
#define DIO63_INREG DOUT16_INREG
#endif
#if (defined(DOUT17_BIT))
#define DOUT17 64
#if (DOUT17_BIT < 32)
#define DOUT17_OUTREG OUT0
#define DOUT17_INREG IN0
#else
#define DOUT17_OUTREG OUT1
#define DOUT17_INREG IN1
#endif
#define DIO64 64
#define DIO64_BIT DOUT17_BIT
#define DIO64_OUTREG DOUT17_OUTREG
#define DIO64_INREG DOUT17_INREG
#endif
#if (defined(DOUT18_BIT))
#define DOUT18 65
#if (DOUT18_BIT < 32)
#define DOUT18_OUTREG OUT0
#define DOUT18_INREG IN0
#else
#define DOUT18_OUTREG OUT1
#define DOUT18_INREG IN1
#endif
#define DIO65 65
#define DIO65_BIT DOUT18_BIT
#define DIO65_OUTREG DOUT18_OUTREG
#define DIO65_INREG DOUT18_INREG
#endif
#if (defined(DOUT19_BIT))
#define DOUT19 66
#if (DOUT19_BIT < 32)
#define DOUT19_OUTREG OUT0
#define DOUT19_INREG IN0
#else
#define DOUT19_OUTREG OUT1
#define DOUT19_INREG IN1
#endif
#define DIO66 66
#define DIO66_BIT DOUT19_BIT
#define DIO66_OUTREG DOUT19_OUTREG
#define DIO66_INREG DOUT19_INREG
#endif
#if (defined(DOUT20_BIT))
#define DOUT20 67
#if (DOUT20_BIT < 32)
#define DOUT20_OUTREG OUT0
#define DOUT20_INREG IN0
#else
#define DOUT20_OUTREG OUT1
#define DOUT20_INREG IN1
#endif
#define DIO67 67
#define DIO67_BIT DOUT20_BIT
#define DIO67_OUTREG DOUT20_OUTREG
#define DIO67_INREG DOUT20_INREG
#endif
#if (defined(DOUT21_BIT))
#define DOUT21 68
#if (DOUT21_BIT < 32)
#define DOUT21_OUTREG OUT0
#define DOUT21_INREG IN0
#else
#define DOUT21_OUTREG OUT1
#define DOUT21_INREG IN1
#endif
#define DIO68 68
#define DIO68_BIT DOUT21_BIT
#define DIO68_OUTREG DOUT21_OUTREG
#define DIO68_INREG DOUT21_INREG
#endif
#if (defined(DOUT22_BIT))
#define DOUT22 69
#if (DOUT22_BIT < 32)
#define DOUT22_OUTREG OUT0
#define DOUT22_INREG IN0
#else
#define DOUT22_OUTREG OUT1
#define DOUT22_INREG IN1
#endif
#define DIO69 69
#define DIO69_BIT DOUT22_BIT
#define DIO69_OUTREG DOUT22_OUTREG
#define DIO69_INREG DOUT22_INREG
#endif
#if (defined(DOUT23_BIT))
#define DOUT23 70
#if (DOUT23_BIT < 32)
#define DOUT23_OUTREG OUT0
#define DOUT23_INREG IN0
#else
#define DOUT23_OUTREG OUT1
#define DOUT23_INREG IN1
#endif
#define DIO70 70
#define DIO70_BIT DOUT23_BIT
#define DIO70_OUTREG DOUT23_OUTREG
#define DIO70_INREG DOUT23_INREG
#endif
#if (defined(DOUT24_BIT))
#define DOUT24 71
#if (DOUT24_BIT < 32)
#define DOUT24_OUTREG OUT0
#define DOUT24_INREG IN0
#else
#define DOUT24_OUTREG OUT1
#define DOUT24_INREG IN1
#endif
#define DIO71 71
#define DIO71_BIT DOUT24_BIT
#define DIO71_OUTREG DOUT24_OUTREG
#define DIO71_INREG DOUT24_INREG
#endif
#if (defined(DOUT25_BIT))
#define DOUT25 72
#if (DOUT25_BIT < 32)
#define DOUT25_OUTREG OUT0
#define DOUT25_INREG IN0
#else
#define DOUT25_OUTREG OUT1
#define DOUT25_INREG IN1
#endif
#define DIO72 72
#define DIO72_BIT DOUT25_BIT
#define DIO72_OUTREG DOUT25_OUTREG
#define DIO72_INREG DOUT25_INREG
#endif
#if (defined(DOUT26_BIT))
#define DOUT26 73
#if (DOUT26_BIT < 32)
#define DOUT26_OUTREG OUT0
#define DOUT26_INREG IN0
#else
#define DOUT26_OUTREG OUT1
#define DOUT26_INREG IN1
#endif
#define DIO73 73
#define DIO73_BIT DOUT26_BIT
#define DIO73_OUTREG DOUT26_OUTREG
#define DIO73_INREG DOUT26_INREG
#endif
#if (defined(DOUT27_BIT))
#define DOUT27 74
#if (DOUT27_BIT < 32)
#define DOUT27_OUTREG OUT0
#define DOUT27_INREG IN0
#else
#define DOUT27_OUTREG OUT1
#define DOUT27_INREG IN1
#endif
#define DIO74 74
#define DIO74_BIT DOUT27_BIT
#define DIO74_OUTREG DOUT27_OUTREG
#define DIO74_INREG DOUT27_INREG
#endif
#if (defined(DOUT28_BIT))
#define DOUT28 75
#if (DOUT28_BIT < 32)
#define DOUT28_OUTREG OUT0
#define DOUT28_INREG IN0
#else
#define DOUT28_OUTREG OUT1
#define DOUT28_INREG IN1
#endif
#define DIO75 75
#define DIO75_BIT DOUT28_BIT
#define DIO75_OUTREG DOUT28_OUTREG
#define DIO75_INREG DOUT28_INREG
#endif
#if (defined(DOUT29_BIT))
#define DOUT29 76
#if (DOUT29_BIT < 32)
#define DOUT29_OUTREG OUT0
#define DOUT29_INREG IN0
#else
#define DOUT29_OUTREG OUT1
#define DOUT29_INREG IN1
#endif
#define DIO76 76
#define DIO76_BIT DOUT29_BIT
#define DIO76_OUTREG DOUT29_OUTREG
#define DIO76_INREG DOUT29_INREG
#endif
#if (defined(DOUT30_BIT))
#define DOUT30 77
#if (DOUT30_BIT < 32)
#define DOUT30_OUTREG OUT0
#define DOUT30_INREG IN0
#else
#define DOUT30_OUTREG OUT1
#define DOUT30_INREG IN1
#endif
#define DIO77 77
#define DIO77_BIT DOUT30_BIT
#define DIO77_OUTREG DOUT30_OUTREG
#define DIO77_INREG DOUT30_INREG
#endif
#if (defined(DOUT31_BIT))
#define DOUT31 78
#if (DOUT31_BIT < 32)
#define DOUT31_OUTREG OUT0
#define DOUT31_INREG IN0
#else
#define DOUT31_OUTREG OUT1
#define DOUT31_INREG IN1
#endif
#if (defined(DOUT32_BIT))
#define DOUT32 79
#if (DOUT32_BIT < 32)
#define DOUT32_OUTREG OUT0
#define DOUT32_INREG IN0
#else
#define DOUT32_OUTREG OUT1
#define DOUT32_INREG IN1
#endif
#define DIO79 79
#define DIO79_BIT DOUT32_BIT
#define DIO79_OUTREG DOUT32_OUTREG
#define DIO79_INREG DOUT32_INREG
#endif
#if (defined(DOUT33_BIT))
#define DOUT33 80
#if (DOUT33_BIT < 32)
#define DOUT33_OUTREG OUT0
#define DOUT33_INREG IN0
#else
#define DOUT33_OUTREG OUT1
#define DOUT33_INREG IN1
#endif
#define DIO80 80
#define DIO80_BIT DOUT33_BIT
#define DIO80_OUTREG DOUT33_OUTREG
#define DIO80_INREG DOUT33_INREG
#endif
#if (defined(DOUT34_BIT))
#define DOUT34 81
#if (DOUT34_BIT < 32)
#define DOUT34_OUTREG OUT0
#define DOUT34_INREG IN0
#else
#define DOUT34_OUTREG OUT1
#define DOUT34_INREG IN1
#endif
#define DIO81 81
#define DIO81_BIT DOUT34_BIT
#define DIO81_OUTREG DOUT34_OUTREG
#define DIO81_INREG DOUT34_INREG
#endif
#if (defined(DOUT35_BIT))
#define DOUT35 82
#if (DOUT35_BIT < 32)
#define DOUT35_OUTREG OUT0
#define DOUT35_INREG IN0
#else
#define DOUT35_OUTREG OUT1
#define DOUT35_INREG IN1
#endif
#define DIO82 82
#define DIO82_BIT DOUT35_BIT
#define DIO82_OUTREG DOUT35_OUTREG
#define DIO82_INREG DOUT35_INREG
#endif
#if (defined(DOUT36_BIT))
#define DOUT36 83
#if (DOUT36_BIT < 32)
#define DOUT36_OUTREG OUT0
#define DOUT36_INREG IN0
#else
#define DOUT36_OUTREG OUT1
#define DOUT36_INREG IN1
#endif
#define DIO83 83
#define DIO83_BIT DOUT36_BIT
#define DIO83_OUTREG DOUT36_OUTREG
#define DIO83_INREG DOUT36_INREG
#endif
#if (defined(DOUT37_BIT))
#define DOUT37 84
#if (DOUT37_BIT < 32)
#define DOUT37_OUTREG OUT0
#define DOUT37_INREG IN0
#else
#define DOUT37_OUTREG OUT1
#define DOUT37_INREG IN1
#endif
#define DIO84 84
#define DIO84_BIT DOUT37_BIT
#define DIO84_OUTREG DOUT37_OUTREG
#define DIO84_INREG DOUT37_INREG
#endif
#if (defined(DOUT38_BIT))
#define DOUT38 85
#if (DOUT38_BIT < 32)
#define DOUT38_OUTREG OUT0
#define DOUT38_INREG IN0
#else
#define DOUT38_OUTREG OUT1
#define DOUT38_INREG IN1
#endif
#define DIO85 85
#define DIO85_BIT DOUT38_BIT
#define DIO85_OUTREG DOUT38_OUTREG
#define DIO85_INREG DOUT38_INREG
#endif
#if (defined(DOUT39_BIT))
#define DOUT39 86
#if (DOUT39_BIT < 32)
#define DOUT39_OUTREG OUT0
#define DOUT39_INREG IN0
#else
#define DOUT39_OUTREG OUT1
#define DOUT39_INREG IN1
#endif
#define DIO86 86
#define DIO86_BIT DOUT39_BIT
#define DIO86_OUTREG DOUT39_OUTREG
#define DIO86_INREG DOUT39_INREG
#endif
#if (defined(DOUT40_BIT))
#define DOUT40 87
#if (DOUT40_BIT < 32)
#define DOUT40_OUTREG OUT0
#define DOUT40_INREG IN0
#else
#define DOUT40_OUTREG OUT1
#define DOUT40_INREG IN1
#endif
#define DIO87 87
#define DIO87_BIT DOUT40_BIT
#define DIO87_OUTREG DOUT40_OUTREG
#define DIO87_INREG DOUT40_INREG
#endif
#if (defined(DOUT41_BIT))
#define DOUT41 88
#if (DOUT41_BIT < 32)
#define DOUT41_OUTREG OUT0
#define DOUT41_INREG IN0
#else
#define DOUT41_OUTREG OUT1
#define DOUT41_INREG IN1
#endif
#define DIO88 88
#define DIO88_BIT DOUT41_BIT
#define DIO88_OUTREG DOUT41_OUTREG
#define DIO88_INREG DOUT41_INREG
#endif
#if (defined(DOUT42_BIT))
#define DOUT42 89
#if (DOUT42_BIT < 32)
#define DOUT42_OUTREG OUT0
#define DOUT42_INREG IN0
#else
#define DOUT42_OUTREG OUT1
#define DOUT42_INREG IN1
#endif
#define DIO89 89
#define DIO89_BIT DOUT42_BIT
#define DIO89_OUTREG DOUT42_OUTREG
#define DIO89_INREG DOUT42_INREG
#endif
#if (defined(DOUT43_BIT))
#define DOUT43 90
#if (DOUT43_BIT < 32)
#define DOUT43_OUTREG OUT0
#define DOUT43_INREG IN0
#else
#define DOUT43_OUTREG OUT1
#define DOUT43_INREG IN1
#endif
#define DIO90 90
#define DIO90_BIT DOUT43_BIT
#define DIO90_OUTREG DOUT43_OUTREG
#define DIO90_INREG DOUT43_INREG
#endif
#if (defined(DOUT44_BIT))
#define DOUT44 91
#if (DOUT44_BIT < 32)
#define DOUT44_OUTREG OUT0
#define DOUT44_INREG IN0
#else
#define DOUT44_OUTREG OUT1
#define DOUT44_INREG IN1
#endif
#define DIO91 91
#define DIO91_BIT DOUT44_BIT
#define DIO91_OUTREG DOUT44_OUTREG
#define DIO91_INREG DOUT44_INREG
#endif
#if (defined(DOUT45_BIT))
#define DOUT45 92
#if (DOUT45_BIT < 32)
#define DOUT45_OUTREG OUT0
#define DOUT45_INREG IN0
#else
#define DOUT45_OUTREG OUT1
#define DOUT45_INREG IN1
#endif
#define DIO92 92
#define DIO92_BIT DOUT45_BIT
#define DIO92_OUTREG DOUT45_OUTREG
#define DIO92_INREG DOUT45_INREG
#endif
#if (defined(DOUT46_BIT))
#define DOUT46 93
#if (DOUT46_BIT < 32)
#define DOUT46_OUTREG OUT0
#define DOUT46_INREG IN0
#else
#define DOUT46_OUTREG OUT1
#define DOUT46_INREG IN1
#endif
#define DIO93 93
#define DIO93_BIT DOUT46_BIT
#define DIO93_OUTREG DOUT46_OUTREG
#define DIO93_INREG DOUT46_INREG
#endif
#if (defined(DOUT47_BIT))
#define DOUT47 94
#if (DOUT47_BIT < 32)
#define DOUT47_OUTREG OUT0
#define DOUT47_INREG IN0
#else
#define DOUT47_OUTREG OUT1
#define DOUT47_INREG IN1
#endif
#define DIO94 94
#define DIO94_BIT DOUT47_BIT
#define DIO94_OUTREG DOUT47_OUTREG
#define DIO94_INREG DOUT47_INREG
#endif
#if (defined(DOUT48_BIT))
#define DOUT48 95
#if (DOUT48_BIT < 32)
#define DOUT48_OUTREG OUT0
#define DOUT48_INREG IN0
#else
#define DOUT48_OUTREG OUT1
#define DOUT48_INREG IN1
#endif
#define DIO95 95
#define DIO95_BIT DOUT48_BIT
#define DIO95_OUTREG DOUT48_OUTREG
#define DIO95_INREG DOUT48_INREG
#endif
#if (defined(DOUT49_BIT))
#define DOUT49 96
#if (DOUT49_BIT < 32)
#define DOUT49_OUTREG OUT0
#define DOUT49_INREG IN0
#else
#define DOUT49_OUTREG OUT1
#define DOUT49_INREG IN1
#endif
#define DIO96 96
#define DIO96_BIT DOUT49_BIT
#define DIO96_OUTREG DOUT49_OUTREG
#define DIO96_INREG DOUT49_INREG
#endif
#define DIO78 78
#define DIO78_BIT DOUT31_BIT
#define DIO78_OUTREG DOUT31_OUTREG
#define DIO78_INREG DOUT31_INREG
#endif
#if (defined(LIMIT_X_BIT))
#define LIMIT_X 100
#if (LIMIT_X_BIT < 32)
#define LIMIT_X_OUTREG OUT0
#define LIMIT_X_INREG IN0
#else
#define LIMIT_X_OUTREG OUT1
#define LIMIT_X_INREG IN1
#endif
#define DIO100 100
#define DIO100_BIT LIMIT_X_BIT
#define DIO100_OUTREG LIMIT_X_OUTREG
#define DIO100_INREG LIMIT_X_INREG
#endif
#if (defined(LIMIT_Y_BIT))
#define LIMIT_Y 101
#if (LIMIT_Y_BIT < 32)
#define LIMIT_Y_OUTREG OUT0
#define LIMIT_Y_INREG IN0
#else
#define LIMIT_Y_OUTREG OUT1
#define LIMIT_Y_INREG IN1
#endif
#define DIO101 101
#define DIO101_BIT LIMIT_Y_BIT
#define DIO101_OUTREG LIMIT_Y_OUTREG
#define DIO101_INREG LIMIT_Y_INREG
#endif
#if (defined(LIMIT_Z_BIT))
#define LIMIT_Z 102
#if (LIMIT_Z_BIT < 32)
#define LIMIT_Z_OUTREG OUT0
#define LIMIT_Z_INREG IN0
#else
#define LIMIT_Z_OUTREG OUT1
#define LIMIT_Z_INREG IN1
#endif
#define DIO102 102
#define DIO102_BIT LIMIT_Z_BIT
#define DIO102_OUTREG LIMIT_Z_OUTREG
#define DIO102_INREG LIMIT_Z_INREG
#endif
#if (defined(LIMIT_X2_BIT))
#define LIMIT_X2 103
#if (LIMIT_X2_BIT < 32)
#define LIMIT_X2_OUTREG OUT0
#define LIMIT_X2_INREG IN0
#else
#define LIMIT_X2_OUTREG OUT1
#define LIMIT_X2_INREG IN1
#endif
#define DIO103 103
#define DIO103_BIT LIMIT_X2_BIT
#define DIO103_OUTREG LIMIT_X2_OUTREG
#define DIO103_INREG LIMIT_X2_INREG
#endif
#if (defined(LIMIT_Y2_BIT))
#define LIMIT_Y2 104
#if (LIMIT_Y2_BIT < 32)
#define LIMIT_Y2_OUTREG OUT0
#define LIMIT_Y2_INREG IN0
#else
#define LIMIT_Y2_OUTREG OUT1
#define LIMIT_Y2_INREG IN1
#endif
#define DIO104 104
#define DIO104_BIT LIMIT_Y2_BIT
#define DIO104_OUTREG LIMIT_Y2_OUTREG
#define DIO104_INREG LIMIT_Y2_INREG
#endif
#if (defined(LIMIT_Z2_BIT))
#define LIMIT_Z2 105
#if (LIMIT_Z2_BIT < 32)
#define LIMIT_Z2_OUTREG OUT0
#define LIMIT_Z2_INREG IN0
#else
#define LIMIT_Z2_OUTREG OUT1
#define LIMIT_Z2_INREG IN1
#endif
#define DIO105 105
#define DIO105_BIT LIMIT_Z2_BIT
#define DIO105_OUTREG LIMIT_Z2_OUTREG
#define DIO105_INREG LIMIT_Z2_INREG
#endif
#if (defined(LIMIT_A_BIT))
#define LIMIT_A 106
#if (LIMIT_A_BIT < 32)
#define LIMIT_A_OUTREG OUT0
#define LIMIT_A_INREG IN0
#else
#define LIMIT_A_OUTREG OUT1
#define LIMIT_A_INREG IN1
#endif
#define DIO106 106
#define DIO106_BIT LIMIT_A_BIT
#define DIO106_OUTREG LIMIT_A_OUTREG
#define DIO106_INREG LIMIT_A_INREG
#endif
#if (defined(LIMIT_B_BIT))
#define LIMIT_B 107
#if (LIMIT_B_BIT < 32)
#define LIMIT_B_OUTREG OUT0
#define LIMIT_B_INREG IN0
#else
#define LIMIT_B_OUTREG OUT1
#define LIMIT_B_INREG IN1
#endif
#define DIO107 107
#define DIO107_BIT LIMIT_B_BIT
#define DIO107_OUTREG LIMIT_B_OUTREG
#define DIO107_INREG LIMIT_B_INREG
#endif
#if (defined(LIMIT_C_BIT))
#define LIMIT_C 108
#if (LIMIT_C_BIT < 32)
#define LIMIT_C_OUTREG OUT0
#define LIMIT_C_INREG IN0
#else
#define LIMIT_C_OUTREG OUT1
#define LIMIT_C_INREG IN1
#endif
#define DIO108 108
#define DIO108_BIT LIMIT_C_BIT
#define DIO108_OUTREG LIMIT_C_OUTREG
#define DIO108_INREG LIMIT_C_INREG
#endif
#if (defined(PROBE_BIT))
#define PROBE 109
#if (PROBE_BIT < 32)
#define PROBE_OUTREG OUT0
#define PROBE_INREG IN0
#else
#define PROBE_OUTREG OUT1
#define PROBE_INREG IN1
#endif
#define DIO109 109
#define DIO109_BIT PROBE_BIT
#define DIO109_OUTREG PROBE_OUTREG
#define DIO109_INREG PROBE_INREG
#endif
#if (defined(ESTOP_BIT))
#define ESTOP 110
#if (ESTOP_BIT < 32)
#define ESTOP_OUTREG OUT0
#define ESTOP_INREG IN0
#else
#define ESTOP_OUTREG OUT1
#define ESTOP_INREG IN1
#endif
#define DIO110 110
#define DIO110_BIT ESTOP_BIT
#define DIO110_OUTREG ESTOP_OUTREG
#define DIO110_INREG ESTOP_INREG
#endif
#if (defined(SAFETY_DOOR_BIT))
#define SAFETY_DOOR 111
#if (SAFETY_DOOR_BIT < 32)
#define SAFETY_DOOR_OUTREG OUT0
#define SAFETY_DOOR_INREG IN0
#else
#define SAFETY_DOOR_OUTREG OUT1
#define SAFETY_DOOR_INREG IN1
#endif
#define DIO111 111
#define DIO111_BIT SAFETY_DOOR_BIT
#define DIO111_OUTREG SAFETY_DOOR_OUTREG
#define DIO111_INREG SAFETY_DOOR_INREG
#endif
#if (defined(FHOLD_BIT))
#define FHOLD 112
#if (FHOLD_BIT < 32)
#define FHOLD_OUTREG OUT0
#define FHOLD_INREG IN0
#else
#define FHOLD_OUTREG OUT1
#define FHOLD_INREG IN1
#endif
#define DIO112 112
#define DIO112_BIT FHOLD_BIT
#define DIO112_OUTREG FHOLD_OUTREG
#define DIO112_INREG FHOLD_INREG
#endif
#if (defined(CS_RES_BIT))
#define CS_RES 113
#if (CS_RES_BIT < 32)
#define CS_RES_OUTREG OUT0
#define CS_RES_INREG IN0
#else
#define CS_RES_OUTREG OUT1
#define CS_RES_INREG IN1
#endif
#define DIO113 113
#define DIO113_BIT CS_RES_BIT
#define DIO113_OUTREG CS_RES_OUTREG
#define DIO113_INREG CS_RES_INREG
#endif
#if (defined(ANALOG0_BIT))
#define ANALOG0 114
#if (ANALOG0_BIT < 32)
#define ANALOG0_OUTREG OUT0
#define ANALOG0_INREG IN0
#else
#define ANALOG0_OUTREG OUT1
#define ANALOG0_INREG IN1
#endif
#define DIO114 114
#define DIO114_BIT ANALOG0_BIT
#define DIO114_OUTREG ANALOG0_OUTREG
#define DIO114_INREG ANALOG0_INREG
#endif
#if (defined(ANALOG1_BIT))
#define ANALOG1 115
#if (ANALOG1_BIT < 32)
#define ANALOG1_OUTREG OUT0
#define ANALOG1_INREG IN0
#else
#define ANALOG1_OUTREG OUT1
#define ANALOG1_INREG IN1
#endif
#define DIO115 115
#define DIO115_BIT ANALOG1_BIT
#define DIO115_OUTREG ANALOG1_OUTREG
#define DIO115_INREG ANALOG1_INREG
#endif
#if (defined(ANALOG2_BIT))
#define ANALOG2 116
#if (ANALOG2_BIT < 32)
#define ANALOG2_OUTREG OUT0
#define ANALOG2_INREG IN0
#else
#define ANALOG2_OUTREG OUT1
#define ANALOG2_INREG IN1
#endif
#define DIO116 116
#define DIO116_BIT ANALOG2_BIT
#define DIO116_OUTREG ANALOG2_OUTREG
#define DIO116_INREG ANALOG2_INREG
#endif
#if (defined(ANALOG3_BIT))
#define ANALOG3 117
#if (ANALOG3_BIT < 32)
#define ANALOG3_OUTREG OUT0
#define ANALOG3_INREG IN0
#else
#define ANALOG3_OUTREG OUT1
#define ANALOG3_INREG IN1
#endif
#define DIO117 117
#define DIO117_BIT ANALOG3_BIT
#define DIO117_OUTREG ANALOG3_OUTREG
#define DIO117_INREG ANALOG3_INREG
#endif
#if (defined(ANALOG4_BIT))
#define ANALOG4 118
#if (ANALOG4_BIT < 32)
#define ANALOG4_OUTREG OUT0
#define ANALOG4_INREG IN0
#else
#define ANALOG4_OUTREG OUT1
#define ANALOG4_INREG IN1
#endif
#define DIO118 118
#define DIO118_BIT ANALOG4_BIT
#define DIO118_OUTREG ANALOG4_OUTREG
#define DIO118_INREG ANALOG4_INREG
#endif
#if (defined(ANALOG5_BIT))
#define ANALOG5 119
#if (ANALOG5_BIT < 32)
#define ANALOG5_OUTREG OUT0
#define ANALOG5_INREG IN0
#else
#define ANALOG5_OUTREG OUT1
#define ANALOG5_INREG IN1
#endif
#define DIO119 119
#define DIO119_BIT ANALOG5_BIT
#define DIO119_OUTREG ANALOG5_OUTREG
#define DIO119_INREG ANALOG5_INREG
#endif
#if (defined(ANALOG6_BIT))
#define ANALOG6 120
#if (ANALOG6_BIT < 32)
#define ANALOG6_OUTREG OUT0
#define ANALOG6_INREG IN0
#else
#define ANALOG6_OUTREG OUT1
#define ANALOG6_INREG IN1
#endif
#define DIO120 120
#define DIO120_BIT ANALOG6_BIT
#define DIO120_OUTREG ANALOG6_OUTREG
#define DIO120_INREG ANALOG6_INREG
#endif
#if (defined(ANALOG7_BIT))
#define ANALOG7 121
#if (ANALOG7_BIT < 32)
#define ANALOG7_OUTREG OUT0
#define ANALOG7_INREG IN0
#else
#define ANALOG7_OUTREG OUT1
#define ANALOG7_INREG IN1
#endif
#define DIO121 121
#define DIO121_BIT ANALOG7_BIT
#define DIO121_OUTREG ANALOG7_OUTREG
#define DIO121_INREG ANALOG7_INREG
#endif
#if (defined(ANALOG8_BIT))
#define ANALOG8 122
#if (ANALOG8_BIT < 32)
#define ANALOG8_OUTREG OUT0
#define ANALOG8_INREG IN0
#else
#define ANALOG8_OUTREG OUT1
#define ANALOG8_INREG IN1
#endif
#define DIO122 122
#define DIO122_BIT ANALOG8_BIT
#define DIO122_OUTREG ANALOG8_OUTREG
#define DIO122_INREG ANALOG8_INREG
#endif
#if (defined(ANALOG9_BIT))
#define ANALOG9 123
#if (ANALOG9_BIT < 32)
#define ANALOG9_OUTREG OUT0
#define ANALOG9_INREG IN0
#else
#define ANALOG9_OUTREG OUT1
#define ANALOG9_INREG IN1
#endif
#define DIO123 123
#define DIO123_BIT ANALOG9_BIT
#define DIO123_OUTREG ANALOG9_OUTREG
#define DIO123_INREG ANALOG9_INREG
#endif
#if (defined(ANALOG10_BIT))
#define ANALOG10 124
#if (ANALOG10_BIT < 32)
#define ANALOG10_OUTREG OUT0
#define ANALOG10_INREG IN0
#else
#define ANALOG10_OUTREG OUT1
#define ANALOG10_INREG IN1
#endif
#define DIO124 124
#define DIO124_BIT ANALOG10_BIT
#define DIO124_OUTREG ANALOG10_OUTREG
#define DIO124_INREG ANALOG10_INREG
#endif
#if (defined(ANALOG11_BIT))
#define ANALOG11 125
#if (ANALOG11_BIT < 32)
#define ANALOG11_OUTREG OUT0
#define ANALOG11_INREG IN0
#else
#define ANALOG11_OUTREG OUT1
#define ANALOG11_INREG IN1
#endif
#define DIO125 125
#define DIO125_BIT ANALOG11_BIT
#define DIO125_OUTREG ANALOG11_OUTREG
#define DIO125_INREG ANALOG11_INREG
#endif
#if (defined(ANALOG12_BIT))
#define ANALOG12 126
#if (ANALOG12_BIT < 32)
#define ANALOG12_OUTREG OUT0
#define ANALOG12_INREG IN0
#else
#define ANALOG12_OUTREG OUT1
#define ANALOG12_INREG IN1
#endif
#define DIO126 126
#define DIO126_BIT ANALOG12_BIT
#define DIO126_OUTREG ANALOG12_OUTREG
#define DIO126_INREG ANALOG12_INREG
#endif
#if (defined(ANALOG13_BIT))
#define ANALOG13 127
#if (ANALOG13_BIT < 32)
#define ANALOG13_OUTREG OUT0
#define ANALOG13_INREG IN0
#else
#define ANALOG13_OUTREG OUT1
#define ANALOG13_INREG IN1
#endif
#define DIO127 127
#define DIO127_BIT ANALOG13_BIT
#define DIO127_OUTREG ANALOG13_OUTREG
#define DIO127_INREG ANALOG13_INREG
#endif
#if (defined(ANALOG14_BIT))
#define ANALOG14 128
#if (ANALOG14_BIT < 32)
#define ANALOG14_OUTREG OUT0
#define ANALOG14_INREG IN0
#else
#define ANALOG14_OUTREG OUT1
#define ANALOG14_INREG IN1
#endif
#define DIO128 128
#define DIO128_BIT ANALOG14_BIT
#define DIO128_OUTREG ANALOG14_OUTREG
#define DIO128_INREG ANALOG14_INREG
#endif
#if (defined(ANALOG15_BIT))
#define ANALOG15 129
#if (ANALOG15_BIT < 32)
#define ANALOG15_OUTREG OUT0
#define ANALOG15_INREG IN0
#else
#define ANALOG15_OUTREG OUT1
#define ANALOG15_INREG IN1
#endif
#define DIO129 129
#define DIO129_BIT ANALOG15_BIT
#define DIO129_OUTREG ANALOG15_OUTREG
#define DIO129_INREG ANALOG15_INREG
#endif
#if (defined(DIN0_BIT))
#define DIN0 130
#if (DIN0_BIT < 32)
#define DIN0_OUTREG OUT0
#define DIN0_INREG IN0
#else
#define DIN0_OUTREG OUT1
#define DIN0_INREG IN1
#endif
#define DIO130 130
#define DIO130_BIT DIN0_BIT
#define DIO130_OUTREG DIN0_OUTREG
#define DIO130_INREG DIN0_INREG
#endif
#if (defined(DIN1_BIT))
#define DIN1 131
#if (DIN1_BIT < 32)
#define DIN1_OUTREG OUT0
#define DIN1_INREG IN0
#else
#define DIN1_OUTREG OUT1
#define DIN1_INREG IN1
#endif
#define DIO131 131
#define DIO131_BIT DIN1_BIT
#define DIO131_OUTREG DIN1_OUTREG
#define DIO131_INREG DIN1_INREG
#endif
#if (defined(DIN2_BIT))
#define DIN2 132
#if (DIN2_BIT < 32)
#define DIN2_OUTREG OUT0
#define DIN2_INREG IN0
#else
#define DIN2_OUTREG OUT1
#define DIN2_INREG IN1
#endif
#define DIO132 132
#define DIO132_BIT DIN2_BIT
#define DIO132_OUTREG DIN2_OUTREG
#define DIO132_INREG DIN2_INREG
#endif
#if (defined(DIN3_BIT))
#define DIN3 133
#if (DIN3_BIT < 32)
#define DIN3_OUTREG OUT0
#define DIN3_INREG IN0
#else
#define DIN3_OUTREG OUT1
#define DIN3_INREG IN1
#endif
#define DIO133 133
#define DIO133_BIT DIN3_BIT
#define DIO133_OUTREG DIN3_OUTREG
#define DIO133_INREG DIN3_INREG
#endif
#if (defined(DIN4_BIT))
#define DIN4 134
#if (DIN4_BIT < 32)
#define DIN4_OUTREG OUT0
#define DIN4_INREG IN0
#else
#define DIN4_OUTREG OUT1
#define DIN4_INREG IN1
#endif
#define DIO134 134
#define DIO134_BIT DIN4_BIT
#define DIO134_OUTREG DIN4_OUTREG
#define DIO134_INREG DIN4_INREG
#endif
#if (defined(DIN5_BIT))
#define DIN5 135
#if (DIN5_BIT < 32)
#define DIN5_OUTREG OUT0
#define DIN5_INREG IN0
#else
#define DIN5_OUTREG OUT1
#define DIN5_INREG IN1
#endif
#define DIO135 135
#define DIO135_BIT DIN5_BIT
#define DIO135_OUTREG DIN5_OUTREG
#define DIO135_INREG DIN5_INREG
#endif
#if (defined(DIN6_BIT))
#define DIN6 136
#if (DIN6_BIT < 32)
#define DIN6_OUTREG OUT0
#define DIN6_INREG IN0
#else
#define DIN6_OUTREG OUT1
#define DIN6_INREG IN1
#endif
#define DIO136 136
#define DIO136_BIT DIN6_BIT
#define DIO136_OUTREG DIN6_OUTREG
#define DIO136_INREG DIN6_INREG
#endif
#if (defined(DIN7_BIT))
#define DIN7 137
#if (DIN7_BIT < 32)
#define DIN7_OUTREG OUT0
#define DIN7_INREG IN0
#else
#define DIN7_OUTREG OUT1
#define DIN7_INREG IN1
#endif
#define DIO137 137
#define DIO137_BIT DIN7_BIT
#define DIO137_OUTREG DIN7_OUTREG
#define DIO137_INREG DIN7_INREG
#endif
#if (defined(DIN8_BIT))
#define DIN8 138
#if (DIN8_BIT < 32)
#define DIN8_OUTREG OUT0
#define DIN8_INREG IN0
#else
#define DIN8_OUTREG OUT1
#define DIN8_INREG IN1
#endif
#define DIO138 138
#define DIO138_BIT DIN8_BIT
#define DIO138_OUTREG DIN8_OUTREG
#define DIO138_INREG DIN8_INREG
#endif
#if (defined(DIN9_BIT))
#define DIN9 139
#if (DIN9_BIT < 32)
#define DIN9_OUTREG OUT0
#define DIN9_INREG IN0
#else
#define DIN9_OUTREG OUT1
#define DIN9_INREG IN1
#endif
#define DIO139 139
#define DIO139_BIT DIN9_BIT
#define DIO139_OUTREG DIN9_OUTREG
#define DIO139_INREG DIN9_INREG
#endif
#if (defined(DIN10_BIT))
#define DIN10 140
#if (DIN10_BIT < 32)
#define DIN10_OUTREG OUT0
#define DIN10_INREG IN0
#else
#define DIN10_OUTREG OUT1
#define DIN10_INREG IN1
#endif
#define DIO140 140
#define DIO140_BIT DIN10_BIT
#define DIO140_OUTREG DIN10_OUTREG
#define DIO140_INREG DIN10_INREG
#endif
#if (defined(DIN11_BIT))
#define DIN11 141
#if (DIN11_BIT < 32)
#define DIN11_OUTREG OUT0
#define DIN11_INREG IN0
#else
#define DIN11_OUTREG OUT1
#define DIN11_INREG IN1
#endif
#define DIO141 141
#define DIO141_BIT DIN11_BIT
#define DIO141_OUTREG DIN11_OUTREG
#define DIO141_INREG DIN11_INREG
#endif
#if (defined(DIN12_BIT))
#define DIN12 142
#if (DIN12_BIT < 32)
#define DIN12_OUTREG OUT0
#define DIN12_INREG IN0
#else
#define DIN12_OUTREG OUT1
#define DIN12_INREG IN1
#endif
#define DIO142 142
#define DIO142_BIT DIN12_BIT
#define DIO142_OUTREG DIN12_OUTREG
#define DIO142_INREG DIN12_INREG
#endif
#if (defined(DIN13_BIT))
#define DIN13 143
#if (DIN13_BIT < 32)
#define DIN13_OUTREG OUT0
#define DIN13_INREG IN0
#else
#define DIN13_OUTREG OUT1
#define DIN13_INREG IN1
#endif
#define DIO143 143
#define DIO143_BIT DIN13_BIT
#define DIO143_OUTREG DIN13_OUTREG
#define DIO143_INREG DIN13_INREG
#endif
#if (defined(DIN14_BIT))
#define DIN14 144
#if (DIN14_BIT < 32)
#define DIN14_OUTREG OUT0
#define DIN14_INREG IN0
#else
#define DIN14_OUTREG OUT1
#define DIN14_INREG IN1
#endif
#define DIO144 144
#define DIO144_BIT DIN14_BIT
#define DIO144_OUTREG DIN14_OUTREG
#define DIO144_INREG DIN14_INREG
#endif
#if (defined(DIN15_BIT))
#define DIN15 145
#if (DIN15_BIT < 32)
#define DIN15_OUTREG OUT0
#define DIN15_INREG IN0
#else
#define DIN15_OUTREG OUT1
#define DIN15_INREG IN1
#endif
#define DIO145 145
#define DIO145_BIT DIN15_BIT
#define DIO145_OUTREG DIN15_OUTREG
#define DIO145_INREG DIN15_INREG
#endif
#if (defined(DIN16_BIT))
#define DIN16 146
#if (DIN16_BIT < 32)
#define DIN16_OUTREG OUT0
#define DIN16_INREG IN0
#else
#define DIN16_OUTREG OUT1
#define DIN16_INREG IN1
#endif
#define DIO146 146
#define DIO146_BIT DIN16_BIT
#define DIO146_OUTREG DIN16_OUTREG
#define DIO146_INREG DIN16_INREG
#endif
#if (defined(DIN17_BIT))
#define DIN17 147
#if (DIN17_BIT < 32)
#define DIN17_OUTREG OUT0
#define DIN17_INREG IN0
#else
#define DIN17_OUTREG OUT1
#define DIN17_INREG IN1
#endif
#define DIO147 147
#define DIO147_BIT DIN17_BIT
#define DIO147_OUTREG DIN17_OUTREG
#define DIO147_INREG DIN17_INREG
#endif
#if (defined(DIN18_BIT))
#define DIN18 148
#if (DIN18_BIT < 32)
#define DIN18_OUTREG OUT0
#define DIN18_INREG IN0
#else
#define DIN18_OUTREG OUT1
#define DIN18_INREG IN1
#endif
#define DIO148 148
#define DIO148_BIT DIN18_BIT
#define DIO148_OUTREG DIN18_OUTREG
#define DIO148_INREG DIN18_INREG
#endif
#if (defined(DIN19_BIT))
#define DIN19 149
#if (DIN19_BIT < 32)
#define DIN19_OUTREG OUT0
#define DIN19_INREG IN0
#else
#define DIN19_OUTREG OUT1
#define DIN19_INREG IN1
#endif
#define DIO149 149
#define DIO149_BIT DIN19_BIT
#define DIO149_OUTREG DIN19_OUTREG
#define DIO149_INREG DIN19_INREG
#endif
#if (defined(DIN20_BIT))
#define DIN20 150
#if (DIN20_BIT < 32)
#define DIN20_OUTREG OUT0
#define DIN20_INREG IN0
#else
#define DIN20_OUTREG OUT1
#define DIN20_INREG IN1
#endif
#define DIO150 150
#define DIO150_BIT DIN20_BIT
#define DIO150_OUTREG DIN20_OUTREG
#define DIO150_INREG DIN20_INREG
#endif
#if (defined(DIN21_BIT))
#define DIN21 151
#if (DIN21_BIT < 32)
#define DIN21_OUTREG OUT0
#define DIN21_INREG IN0
#else
#define DIN21_OUTREG OUT1
#define DIN21_INREG IN1
#endif
#define DIO151 151
#define DIO151_BIT DIN21_BIT
#define DIO151_OUTREG DIN21_OUTREG
#define DIO151_INREG DIN21_INREG
#endif
#if (defined(DIN22_BIT))
#define DIN22 152
#if (DIN22_BIT < 32)
#define DIN22_OUTREG OUT0
#define DIN22_INREG IN0
#else
#define DIN22_OUTREG OUT1
#define DIN22_INREG IN1
#endif
#define DIO152 152
#define DIO152_BIT DIN22_BIT
#define DIO152_OUTREG DIN22_OUTREG
#define DIO152_INREG DIN22_INREG
#endif
#if (defined(DIN23_BIT))
#define DIN23 153
#if (DIN23_BIT < 32)
#define DIN23_OUTREG OUT0
#define DIN23_INREG IN0
#else
#define DIN23_OUTREG OUT1
#define DIN23_INREG IN1
#endif
#define DIO153 153
#define DIO153_BIT DIN23_BIT
#define DIO153_OUTREG DIN23_OUTREG
#define DIO153_INREG DIN23_INREG
#endif
#if (defined(DIN24_BIT))
#define DIN24 154
#if (DIN24_BIT < 32)
#define DIN24_OUTREG OUT0
#define DIN24_INREG IN0
#else
#define DIN24_OUTREG OUT1
#define DIN24_INREG IN1
#endif
#define DIO154 154
#define DIO154_BIT DIN24_BIT
#define DIO154_OUTREG DIN24_OUTREG
#define DIO154_INREG DIN24_INREG
#endif
#if (defined(DIN25_BIT))
#define DIN25 155
#if (DIN25_BIT < 32)
#define DIN25_OUTREG OUT0
#define DIN25_INREG IN0
#else
#define DIN25_OUTREG OUT1
#define DIN25_INREG IN1
#endif
#define DIO155 155
#define DIO155_BIT DIN25_BIT
#define DIO155_OUTREG DIN25_OUTREG
#define DIO155_INREG DIN25_INREG
#endif
#if (defined(DIN26_BIT))
#define DIN26 156
#if (DIN26_BIT < 32)
#define DIN26_OUTREG OUT0
#define DIN26_INREG IN0
#else
#define DIN26_OUTREG OUT1
#define DIN26_INREG IN1
#endif
#define DIO156 156
#define DIO156_BIT DIN26_BIT
#define DIO156_OUTREG DIN26_OUTREG
#define DIO156_INREG DIN26_INREG
#endif
#if (defined(DIN27_BIT))
#define DIN27 157
#if (DIN27_BIT < 32)
#define DIN27_OUTREG OUT0
#define DIN27_INREG IN0
#else
#define DIN27_OUTREG OUT1
#define DIN27_INREG IN1
#endif
#define DIO157 157
#define DIO157_BIT DIN27_BIT
#define DIO157_OUTREG DIN27_OUTREG
#define DIO157_INREG DIN27_INREG
#endif
#if (defined(DIN28_BIT))
#define DIN28 158
#if (DIN28_BIT < 32)
#define DIN28_OUTREG OUT0
#define DIN28_INREG IN0
#else
#define DIN28_OUTREG OUT1
#define DIN28_INREG IN1
#endif
#define DIO158 158
#define DIO158_BIT DIN28_BIT
#define DIO158_OUTREG DIN28_OUTREG
#define DIO158_INREG DIN28_INREG
#endif
#if (defined(DIN29_BIT))
#define DIN29 159
#if (DIN29_BIT < 32)
#define DIN29_OUTREG OUT0
#define DIN29_INREG IN0
#else
#define DIN29_OUTREG OUT1
#define DIN29_INREG IN1
#endif
#define DIO159 159
#define DIO159_BIT DIN29_BIT
#define DIO159_OUTREG DIN29_OUTREG
#define DIO159_INREG DIN29_INREG
#endif
#if (defined(DIN30_BIT))
#define DIN30 160
#if (DIN30_BIT < 32)
#define DIN30_OUTREG OUT0
#define DIN30_INREG IN0
#else
#define DIN30_OUTREG OUT1
#define DIN30_INREG IN1
#endif
#define DIO160 160
#define DIO160_BIT DIN30_BIT
#define DIO160_OUTREG DIN30_OUTREG
#define DIO160_INREG DIN30_INREG
#endif
#if (defined(DIN31_BIT))
#define DIN31 161
#if (DIN31_BIT < 32)
#define DIN31_OUTREG OUT0
#define DIN31_INREG IN0
#else
#define DIN31_OUTREG OUT1
#define DIN31_INREG IN1
#endif
#define DIO161 161
#define DIO161_BIT DIN31_BIT
#define DIO161_OUTREG DIN31_OUTREG
#define DIO161_INREG DIN31_INREG
#endif
#if (defined(DIN32_BIT))
#define DIN32 162
#if (DIN32_BIT < 32)
#define DIN32_OUTREG OUT0
#define DIN32_INREG IN0
#else
#define DIN32_OUTREG OUT1
#define DIN32_INREG IN1
#endif
#define DIO162 162
#define DIO162_BIT DIN32_BIT
#define DIO162_OUTREG DIN32_OUTREG
#define DIO162_INREG DIN32_INREG
#endif
#if (defined(DIN33_BIT))
#define DIN33 163
#if (DIN33_BIT < 32)
#define DIN33_OUTREG OUT0
#define DIN33_INREG IN0
#else
#define DIN33_OUTREG OUT1
#define DIN33_INREG IN1
#endif
#define DIO163 163
#define DIO163_BIT DIN33_BIT
#define DIO163_OUTREG DIN33_OUTREG
#define DIO163_INREG DIN33_INREG
#endif
#if (defined(DIN34_BIT))
#define DIN34 164
#if (DIN34_BIT < 32)
#define DIN34_OUTREG OUT0
#define DIN34_INREG IN0
#else
#define DIN34_OUTREG OUT1
#define DIN34_INREG IN1
#endif
#define DIO164 164
#define DIO164_BIT DIN34_BIT
#define DIO164_OUTREG DIN34_OUTREG
#define DIO164_INREG DIN34_INREG
#endif
#if (defined(DIN35_BIT))
#define DIN35 165
#if (DIN35_BIT < 32)
#define DIN35_OUTREG OUT0
#define DIN35_INREG IN0
#else
#define DIN35_OUTREG OUT1
#define DIN35_INREG IN1
#endif
#define DIO165 165
#define DIO165_BIT DIN35_BIT
#define DIO165_OUTREG DIN35_OUTREG
#define DIO165_INREG DIN35_INREG
#endif
#if (defined(DIN36_BIT))
#define DIN36 166
#if (DIN36_BIT < 32)
#define DIN36_OUTREG OUT0
#define DIN36_INREG IN0
#else
#define DIN36_OUTREG OUT1
#define DIN36_INREG IN1
#endif
#define DIO166 166
#define DIO166_BIT DIN36_BIT
#define DIO166_OUTREG DIN36_OUTREG
#define DIO166_INREG DIN36_INREG
#endif
#if (defined(DIN37_BIT))
#define DIN37 167
#if (DIN37_BIT < 32)
#define DIN37_OUTREG OUT0
#define DIN37_INREG IN0
#else
#define DIN37_OUTREG OUT1
#define DIN37_INREG IN1
#endif
#define DIO167 167
#define DIO167_BIT DIN37_BIT
#define DIO167_OUTREG DIN37_OUTREG
#define DIO167_INREG DIN37_INREG
#endif
#if (defined(DIN38_BIT))
#define DIN38 168
#if (DIN38_BIT < 32)
#define DIN38_OUTREG OUT0
#define DIN38_INREG IN0
#else
#define DIN38_OUTREG OUT1
#define DIN38_INREG IN1
#endif
#define DIO168 168
#define DIO168_BIT DIN38_BIT
#define DIO168_OUTREG DIN38_OUTREG
#define DIO168_INREG DIN38_INREG
#endif
#if (defined(DIN39_BIT))
#define DIN39 169
#if (DIN39_BIT < 32)
#define DIN39_OUTREG OUT0
#define DIN39_INREG IN0
#else
#define DIN39_OUTREG OUT1
#define DIN39_INREG IN1
#endif
#define DIO169 169
#define DIO169_BIT DIN39_BIT
#define DIO169_OUTREG DIN39_OUTREG
#define DIO169_INREG DIN39_INREG
#endif
#if (defined(DIN40_BIT))
#define DIN40 170
#if (DIN40_BIT < 32)
#define DIN40_OUTREG OUT0
#define DIN40_INREG IN0
#else
#define DIN40_OUTREG OUT1
#define DIN40_INREG IN1
#endif
#define DIO170 170
#define DIO170_BIT DIN40_BIT
#define DIO170_OUTREG DIN40_OUTREG
#define DIO170_INREG DIN40_INREG
#endif
#if (defined(DIN41_BIT))
#define DIN41 171
#if (DIN41_BIT < 32)
#define DIN41_OUTREG OUT0
#define DIN41_INREG IN0
#else
#define DIN41_OUTREG OUT1
#define DIN41_INREG IN1
#endif
#define DIO171 171
#define DIO171_BIT DIN41_BIT
#define DIO171_OUTREG DIN41_OUTREG
#define DIO171_INREG DIN41_INREG
#endif
#if (defined(DIN42_BIT))
#define DIN42 172
#if (DIN42_BIT < 32)
#define DIN42_OUTREG OUT0
#define DIN42_INREG IN0
#else
#define DIN42_OUTREG OUT1
#define DIN42_INREG IN1
#endif
#define DIO172 172
#define DIO172_BIT DIN42_BIT
#define DIO172_OUTREG DIN42_OUTREG
#define DIO172_INREG DIN42_INREG
#endif
#if (defined(DIN43_BIT))
#define DIN43 173
#if (DIN43_BIT < 32)
#define DIN43_OUTREG OUT0
#define DIN43_INREG IN0
#else
#define DIN43_OUTREG OUT1
#define DIN43_INREG IN1
#endif
#define DIO173 173
#define DIO173_BIT DIN43_BIT
#define DIO173_OUTREG DIN43_OUTREG
#define DIO173_INREG DIN43_INREG
#endif
#if (defined(DIN44_BIT))
#define DIN44 174
#if (DIN44_BIT < 32)
#define DIN44_OUTREG OUT0
#define DIN44_INREG IN0
#else
#define DIN44_OUTREG OUT1
#define DIN44_INREG IN1
#endif
#define DIO174 174
#define DIO174_BIT DIN44_BIT
#define DIO174_OUTREG DIN44_OUTREG
#define DIO174_INREG DIN44_INREG
#endif
#if (defined(DIN45_BIT))
#define DIN45 175
#if (DIN45_BIT < 32)
#define DIN45_OUTREG OUT0
#define DIN45_INREG IN0
#else
#define DIN45_OUTREG OUT1
#define DIN45_INREG IN1
#endif
#define DIO175 175
#define DIO175_BIT DIN45_BIT
#define DIO175_OUTREG DIN45_OUTREG
#define DIO175_INREG DIN45_INREG
#endif
#if (defined(DIN46_BIT))
#define DIN46 176
#if (DIN46_BIT < 32)
#define DIN46_OUTREG OUT0
#define DIN46_INREG IN0
#else
#define DIN46_OUTREG OUT1
#define DIN46_INREG IN1
#endif
#define DIO176 176
#define DIO176_BIT DIN46_BIT
#define DIO176_OUTREG DIN46_OUTREG
#define DIO176_INREG DIN46_INREG
#endif
#if (defined(DIN47_BIT))
#define DIN47 177
#if (DIN47_BIT < 32)
#define DIN47_OUTREG OUT0
#define DIN47_INREG IN0
#else
#define DIN47_OUTREG OUT1
#define DIN47_INREG IN1
#endif
#define DIO177 177
#define DIO177_BIT DIN47_BIT
#define DIO177_OUTREG DIN47_OUTREG
#define DIO177_INREG DIN47_INREG
#endif
#if (defined(DIN48_BIT))
#define DIN48 178
#if (DIN48_BIT < 32)
#define DIN48_OUTREG OUT0
#define DIN48_INREG IN0
#else
#define DIN48_OUTREG OUT1
#define DIN48_INREG IN1
#endif
#define DIO178 178
#define DIO178_BIT DIN48_BIT
#define DIO178_OUTREG DIN48_OUTREG
#define DIO178_INREG DIN48_INREG
#endif
#if (defined(DIN49_BIT))
#define DIN49 179
#if (DIN49_BIT < 32)
#define DIN49_OUTREG OUT0
#define DIN49_INREG IN0
#else
#define DIN49_OUTREG OUT1
#define DIN49_INREG IN1
#endif
#define DIO179 179
#define DIO179_BIT DIN49_BIT
#define DIO179_OUTREG DIN49_OUTREG
#define DIO179_INREG DIN49_INREG
#endif
#if (defined(TX_BIT))
#define TX 200
#if (TX_BIT < 32)
#define TX_OUTREG OUT0
#define TX_INREG IN0
#else
#define TX_OUTREG OUT1
#define TX_INREG IN1
#endif
#define DIO200 200
#define DIO200_BIT TX_BIT
#define DIO200_OUTREG TX_OUTREG
#define DIO200_INREG TX_INREG
#endif
#if (defined(RX_BIT))
#define RX 201
#if (RX_BIT < 32)
#define RX_OUTREG OUT0
#define RX_INREG IN0
#else
#define RX_OUTREG OUT1
#define RX_INREG IN1
#endif
#define DIO201 201
#define DIO201_BIT RX_BIT
#define DIO201_OUTREG RX_OUTREG
#define DIO201_INREG RX_INREG
#endif
#if (defined(USB_DM_BIT))
#define USB_DM 202
#if (USB_DM_BIT < 32)
#define USB_DM_OUTREG OUT0
#define USB_DM_INREG IN0
#else
#define USB_DM_OUTREG OUT1
#define USB_DM_INREG IN1
#endif
#define DIO202 202
#define DIO202_BIT USB_DM_BIT
#define DIO202_OUTREG USB_DM_OUTREG
#define DIO202_INREG USB_DM_INREG
#endif
#if (defined(USB_DP_BIT))
#define USB_DP 203
#if (USB_DP_BIT < 32)
#define USB_DP_OUTREG OUT0
#define USB_DP_INREG IN0
#else
#define USB_DP_OUTREG OUT1
#define USB_DP_INREG IN1
#endif
#define DIO203 203
#define DIO203_BIT USB_DP_BIT
#define DIO203_OUTREG USB_DP_OUTREG
#define DIO203_INREG USB_DP_INREG
#endif
#if (defined(SPI_CLK_BIT))
#define SPI_CLK 204
#if (SPI_CLK_BIT < 32)
#define SPI_CLK_OUTREG OUT0
#define SPI_CLK_INREG IN0
#else
#define SPI_CLK_OUTREG OUT1
#define SPI_CLK_INREG IN1
#endif
#define DIO204 204
#define DIO204_BIT SPI_CLK_BIT
#define DIO204_OUTREG SPI_CLK_OUTREG
#define DIO204_INREG SPI_CLK_INREG
#endif
#if (defined(SPI_SDI_BIT))
#define SPI_SDI 205
#if (SPI_SDI_BIT < 32)
#define SPI_SDI_OUTREG OUT0
#define SPI_SDI_INREG IN0
#else
#define SPI_SDI_OUTREG OUT1
#define SPI_SDI_INREG IN1
#endif
#define DIO205 205
#define DIO205_BIT SPI_SDI_BIT
#define DIO205_OUTREG SPI_SDI_OUTREG
#define DIO205_INREG SPI_SDI_INREG
#endif
#if (defined(SPI_SDO_BIT))
#define SPI_SDO 206
#if (SPI_SDO_BIT < 32)
#define SPI_SDO_OUTREG OUT0
#define SPI_SDO_INREG IN0
#else
#define SPI_SDO_OUTREG OUT1
#define SPI_SDO_INREG IN1
#endif
#define DIO206 206
#define DIO206_BIT SPI_SDO_BIT
#define DIO206_OUTREG SPI_SDO_OUTREG
#define DIO206_INREG SPI_SDO_INREG
#endif
#if (defined(SPI_CS_BIT))
#define SPI_CS 207
#if (SPI_CS_BIT < 32)
#define SPI_CS_OUTREG OUT0
#define SPI_CS_INREG IN0
#else
#define SPI_CS_OUTREG OUT1
#define SPI_CS_INREG IN1
#endif
#define DIO207 207
#define DIO207_BIT SPI_CS_BIT
#define DIO207_OUTREG SPI_CS_OUTREG
#define DIO207_INREG SPI_CS_INREG
#endif
#if (defined(I2C_CLK_BIT))
#define I2C_CLK 208
#if (I2C_CLK_BIT < 32)
#define I2C_CLK_OUTREG OUT0
#define I2C_CLK_INREG IN0
#else
#define I2C_CLK_OUTREG OUT1
#define I2C_CLK_INREG IN1
#endif
#define DIO208 208
#define DIO208_BIT I2C_CLK_BIT
#define DIO208_OUTREG I2C_CLK_OUTREG
#define DIO208_INREG I2C_CLK_INREG
#endif
#if (defined(I2C_DATA_BIT))
#define I2C_DATA 209
#if (I2C_DATA_BIT < 32)
#define I2C_DATA_OUTREG OUT0
#define I2C_DATA_INREG IN0
#else
#define I2C_DATA_OUTREG OUT1
#define I2C_DATA_INREG IN1
#endif
#define DIO209 209
#define DIO209_BIT I2C_DATA_BIT
#define DIO209_OUTREG I2C_DATA_OUTREG
#define DIO209_INREG I2C_DATA_INREG
#endif
#if (defined(TX2_BIT))
#define TX2 210
#if (TX2_BIT < 32)
#define TX2_OUTREG OUT0
#define TX2_INREG IN0
#else
#define TX2_OUTREG OUT1
#define TX2_INREG IN1
#endif
#define DIO210 210
#define DIO210_BIT TX2_BIT
#define DIO210_OUTREG TX2_OUTREG
#define DIO210_INREG TX2_INREG
#endif
#if (defined(RX2_BIT))
#define RX2 211
#if (RX2_BIT < 32)
#define RX2_OUTREG OUT0
#define RX2_INREG IN0
#else
#define RX2_OUTREG OUT1
#define RX2_INREG IN1
#endif
#define DIO211 211
#define DIO211_BIT RX2_BIT
#define DIO211_OUTREG RX2_OUTREG
#define DIO211_INREG RX2_INREG
#endif

// ISR on change inputs
#if (defined(LIMIT_X_ISR) && defined(LIMIT_X))
#define DIO100_ISR (LIMIT_X_ISR)
#define LIMIT_X_ISRVAR 1
#define DIO100_ISRVAR 1
#endif
#if (defined(LIMIT_Y_ISR) && defined(LIMIT_Y))
#define DIO101_ISR (LIMIT_Y_ISR)
#define LIMIT_Y_ISRVAR 1
#define DIO101_ISRVAR 1
#endif
#if (defined(LIMIT_Z_ISR) && defined(LIMIT_Z))
#define DIO102_ISR (LIMIT_Z_ISR)
#define LIMIT_Z_ISRVAR 1
#define DIO102_ISRVAR 1
#endif
#if (defined(LIMIT_X2_ISR) && defined(LIMIT_X2))
#define DIO103_ISR (LIMIT_X2_ISR)
#define LIMIT_X2_ISRVAR 1
#define DIO103_ISRVAR 1
#endif
#if (defined(LIMIT_Y2_ISR) && defined(LIMIT_Y2))
#define DIO104_ISR (LIMIT_Y2_ISR)
#define LIMIT_Y2_ISRVAR 1
#define DIO104_ISRVAR 1
#endif
#if (defined(LIMIT_Z2_ISR) && defined(LIMIT_Z2))
#define DIO105_ISR (LIMIT_Z2_ISR)
#define LIMIT_Z2_ISRVAR 1
#define DIO105_ISRVAR 1
#endif
#if (defined(LIMIT_A_ISR) && defined(LIMIT_A))
#define DIO106_ISR (LIMIT_A_ISR)
#define LIMIT_A_ISRVAR 1
#define DIO106_ISRVAR 1
#endif
#if (defined(LIMIT_B_ISR) && defined(LIMIT_B))
#define DIO107_ISR (LIMIT_B_ISR)
#define LIMIT_B_ISRVAR 1
#define DIO107_ISRVAR 1
#endif
#if (defined(LIMIT_C_ISR) && defined(LIMIT_C))
#define DIO108_ISR (LIMIT_C_ISR)
#define LIMIT_C_ISRVAR 1
#define DIO108_ISRVAR 1
#endif
#if (defined(PROBE_ISR) && defined(PROBE))
#define DIO109_ISR (PROBE_ISR)
#define PROBE_ISRVAR 2
#define DIO109_ISRVAR 2
#endif
#if (defined(ESTOP_ISR) && defined(ESTOP))
#define DIO110_ISR (ESTOP_ISR)
#define ESTOP_ISRVAR 0
#define DIO110_ISRVAR 0
#endif
#if (defined(SAFETY_DOOR_ISR) && defined(SAFETY_DOOR))
#define DIO111_ISR (SAFETY_DOOR_ISR)
#define SAFETY_DOOR_ISRVAR 0
#define DIO111_ISRVAR 0
#endif
#if (defined(FHOLD_ISR) && defined(FHOLD))
#define DIO112_ISR (FHOLD_ISR)
#define FHOLD_ISRVAR 0
#define DIO112_ISRVAR 0
#endif
#if (defined(CS_RES_ISR) && defined(CS_RES))
#define DIO113_ISR (CS_RES_ISR)
#define CS_RES_ISRVAR 0
#define DIO113_ISRVAR 0
#endif
#if (defined(DIN0_ISR) && defined(DIN0))
#define DIO130_ISR (DIN0_ISR)
#define DIN0_ISRVAR 3
#define DIO130_ISRVAR 3
#endif
#if (defined(DIN1_ISR) && defined(DIN1))
#define DIO131_ISR (DIN1_ISR)
#define DIN1_ISRVAR 3
#define DIO131_ISRVAR 3
#endif
#if (defined(DIN2_ISR) && defined(DIN2))
#define DIO132_ISR (DIN2_ISR)
#define DIN2_ISRVAR 3
#define DIO132_ISRVAR 3
#endif
#if (defined(DIN3_ISR) && defined(DIN3))
#define DIO133_ISR (DIN3_ISR)
#define DIN3_ISRVAR 3
#define DIO133_ISRVAR 3
#endif
#if (defined(DIN4_ISR) && defined(DIN4))
#define DIO134_ISR (DIN4_ISR)
#define DIN4_ISRVAR 3
#define DIO134_ISRVAR 3
#endif
#if (defined(DIN5_ISR) && defined(DIN5))
#define DIO135_ISR (DIN5_ISR)
#define DIN5_ISRVAR 3
#define DIO135_ISRVAR 3
#endif
#if (defined(DIN6_ISR) && defined(DIN6))
#define DIO136_ISR (DIN6_ISR)
#define DIN6_ISRVAR 3
#define DIO136_ISRVAR 3
#endif
#if (defined(DIN7_ISR) && defined(DIN7))
#define DIO137_ISR (DIN7_ISR)
#define DIN7_ISRVAR 3
#define DIO137_ISRVAR 3
#endif

#define __adc_channel_helper__(X, Y) ADC##X##_CHANNEL_##Y
#define _adc_channel_helper_(X, Y) __adc_channel_helper__(X, Y)

#ifdef ANALOG0
#define ANALOG0_ADC_CHANNEL _adc_channel_helper_(ANALOG0_CHANNEL, ANALOG0_ADC)
#define DIO114_CHANNEL ANALOG0_CHANNEL
#define DIO114_ADC ANALOG0_ADC
#define DIO114_ADC_CHANNEL ANALOG0_ADC_CHANNEL
#endif
#ifdef ANALOG1
#define ANALOG1_ADC_CHANNEL _adc_channel_helper_(ANALOG1_CHANNEL, ANALOG1_ADC)
#define DIO115_CHANNEL ANALOG1_CHANNEL
#define DIO115_ADC ANALOG1_ADC
#define DIO115_ADC_CHANNEL ANALOG1_ADC_CHANNEL
#endif
#ifdef ANALOG2
#define ANALOG2_ADC_CHANNEL _adc_channel_helper_(ANALOG2_CHANNEL, ANALOG2_ADC)
#define DIO116_CHANNEL ANALOG2_CHANNEL
#define DIO116_ADC ANALOG2_ADC
#define DIO116_ADC_CHANNEL ANALOG2_ADC_CHANNEL
#endif
#ifdef ANALOG3
#define ANALOG3_ADC_CHANNEL _adc_channel_helper_(ANALOG3_CHANNEL, ANALOG3_ADC)
#define DIO117_CHANNEL ANALOG3_CHANNEL
#define DIO117_ADC ANALOG3_ADC
#define DIO117_ADC_CHANNEL ANALOG3_ADC_CHANNEL
#endif
#ifdef ANALOG4
#define ANALOG4_ADC_CHANNEL _adc_channel_helper_(ANALOG4_CHANNEL, ANALOG4_ADC)
#define DIO118_CHANNEL ANALOG4_CHANNEL
#define DIO118_ADC ANALOG4_ADC
#define DIO118_ADC_CHANNEL ANALOG4_ADC_CHANNEL
#endif
#ifdef ANALOG5
#define ANALOG5_ADC_CHANNEL _adc_channel_helper_(ANALOG5_CHANNEL, ANALOG5_ADC)
#define DIO119_CHANNEL ANALOG5_CHANNEL
#define DIO119_ADC ANALOG5_ADC
#define DIO119_ADC_CHANNEL ANALOG5_ADC_CHANNEL
#endif
#ifdef ANALOG6
#define ANALOG6_ADC_CHANNEL _adc_channel_helper_(ANALOG6_CHANNEL, ANALOG6_ADC)
#define DIO120_CHANNEL ANALOG6_CHANNEL
#define DIO120_ADC ANALOG6_ADC
#define DIO120_ADC_CHANNEL ANALOG6_ADC_CHANNEL
#endif
#ifdef ANALOG7
#define ANALOG7_ADC_CHANNEL _adc_channel_helper_(ANALOG7_CHANNEL, ANALOG7_ADC)
#define DIO121_CHANNEL ANALOG7_CHANNEL
#define DIO121_ADC ANALOG7_ADC
#define DIO121_ADC_CHANNEL ANALOG7_ADC_CHANNEL
#endif
#ifdef ANALOG8
#define ANALOG8_ADC_CHANNEL _adc_channel_helper_(ANALOG8_CHANNEL, ANALOG8_ADC)
#define DIO122_CHANNEL ANALOG8_CHANNEL
#define DIO122_ADC ANALOG8_ADC
#define DIO122_ADC_CHANNEL ANALOG8_ADC_CHANNEL
#endif
#ifdef ANALOG9
#define ANALOG9_ADC_CHANNEL _adc_channel_helper_(ANALOG9_CHANNEL, ANALOG9_ADC)
#define DIO123_CHANNEL ANALOG9_CHANNEL
#define DIO123_ADC ANALOG9_ADC
#define DIO123_ADC_CHANNEL ANALOG9_ADC_CHANNEL
#endif
#ifdef ANALOG10
#define ANALOG10_ADC_CHANNEL _adc_channel_helper_(ANALOG10_CHANNEL, ANALOG10_ADC)
#define DIO124_CHANNEL ANALOG10_CHANNEL
#define DIO124_ADC ANALOG10_ADC
#define DIO124_ADC_CHANNEL ANALOG10_ADC_CHANNEL
#endif
#ifdef ANALOG11
#define ANALOG11_ADC_CHANNEL _adc_channel_helper_(ANALOG11_CHANNEL, ANALOG11_ADC)
#define DIO125_CHANNEL ANALOG11_CHANNEL
#define DIO125_ADC ANALOG11_ADC
#define DIO125_ADC_CHANNEL ANALOG11_ADC_CHANNEL
#endif
#ifdef ANALOG12
#define ANALOG12_ADC_CHANNEL _adc_channel_helper_(ANALOG12_CHANNEL, ANALOG12_ADC)
#define DIO126_CHANNEL ANALOG12_CHANNEL
#define DIO126_ADC ANALOG12_ADC
#define DIO126_ADC_CHANNEL ANALOG12_ADC_CHANNEL
#endif
#ifdef ANALOG13
#define ANALOG13_ADC_CHANNEL _adc_channel_helper_(ANALOG13_CHANNEL, ANALOG13_ADC)
#define DIO127_CHANNEL ANALOG13_CHANNEL
#define DIO127_ADC ANALOG13_ADC
#define DIO127_ADC_CHANNEL ANALOG13_ADC_CHANNEL
#endif
#ifdef ANALOG14
#define ANALOG14_ADC_CHANNEL _adc_channel_helper_(ANALOG14_CHANNEL, ANALOG14_ADC)
#define DIO128_CHANNEL ANALOG14_CHANNEL
#define DIO128_ADC ANALOG14_ADC
#define DIO128_ADC_CHANNEL ANALOG14_ADC_CHANNEL
#endif
#ifdef ANALOG15
#define ANALOG15_ADC_CHANNEL _adc_channel_helper_(ANALOG15_CHANNEL, ANALOG15_ADC)
#define DIO129_CHANNEL ANALOG15_CHANNEL
#define DIO129_ADC ANALOG15_ADC
#define DIO129_ADC_CHANNEL ANALOG15_ADC_CHANNEL
#endif

	/*PWM's*/

#ifdef PWM0_CHANNEL
#define PWM0_LEDCCHANNEL (PWM0_CHANNEL & 0x07)
#if (PWM0_CHANNEL < 8) && SOC_LEDC_SUPPORT_HS_MODE
#define PWM0_SPEEDMODE LEDC_HIGH_SPEED_MODE
#else
#define PWM0_SPEEDMODE LEDC_LOW_SPEED_MODE
#endif
#define DIO25_CHANNEL PWM0_CHANNEL
#define DIO25_TIMER PWM0_TIMER
#define DIO25_LEDCCHANNEL PWM0_LEDCCHANNEL
#define DIO25_SPEEDMODE PWM0_SPEEDMODE
#endif
#ifdef PWM1_CHANNEL
#define PWM1_LEDCCHANNEL (PWM1_CHANNEL & 0x07)
#if (PWM1_CHANNEL < 8) && SOC_LEDC_SUPPORT_HS_MODE
#define PWM1_SPEEDMODE LEDC_HIGH_SPEED_MODE
#else
#define PWM1_SPEEDMODE LEDC_LOW_SPEED_MODE
#endif
#define DIO26_CHANNEL PWM1_CHANNEL
#define DIO26_TIMER PWM1_TIMER
#define DIO26_LEDCCHANNEL PWM1_LEDCCHANNEL
#define DIO26_SPEEDMODE PWM1_SPEEDMODE
#endif
#ifdef PWM2_CHANNEL
#define PWM2_LEDCCHANNEL (PWM2_CHANNEL & 0x07)
#if (PWM2_CHANNEL < 8) && SOC_LEDC_SUPPORT_HS_MODE
#define PWM2_SPEEDMODE LEDC_HIGH_SPEED_MODE
#else
#define PWM2_SPEEDMODE LEDC_LOW_SPEED_MODE
#endif
#define DIO27_CHANNEL PWM2_CHANNEL
#define DIO27_TIMER PWM2_TIMER
#define DIO27_LEDCCHANNEL PWM2_LEDCCHANNEL
#define DIO27_SPEEDMODE PWM2_SPEEDMODE
#endif
#ifdef PWM3_CHANNEL
#define PWM3_LEDCCHANNEL (PWM3_CHANNEL & 0x07)
#if (PWM3_CHANNEL < 8) && SOC_LEDC_SUPPORT_HS_MODE
#define PWM3_SPEEDMODE LEDC_HIGH_SPEED_MODE
#else
#define PWM3_SPEEDMODE LEDC_LOW_SPEED_MODE
#endif
#define DIO28_CHANNEL PWM3_CHANNEL
#define DIO28_TIMER PWM3_TIMER
#define DIO28_LEDCCHANNEL PWM3_LEDCCHANNEL
#define DIO28_SPEEDMODE PWM3_SPEEDMODE
#endif
#ifdef PWM4_CHANNEL
#define PWM4_LEDCCHANNEL (PWM4_CHANNEL & 0x07)
#if (PWM4_CHANNEL < 8) && SOC_LEDC_SUPPORT_HS_MODE
#define PWM4_SPEEDMODE LEDC_HIGH_SPEED_MODE
#else
#define PWM4_SPEEDMODE LEDC_LOW_SPEED_MODE
#endif
#define DIO29_CHANNEL PWM4_CHANNEL
#define DIO29_TIMER PWM4_TIMER
#define DIO29_LEDCCHANNEL PWM4_LEDCCHANNEL
#define DIO29_SPEEDMODE PWM4_SPEEDMODE
#endif
#ifdef PWM5_CHANNEL
#define PWM5_LEDCCHANNEL (PWM5_CHANNEL & 0x07)
#if (PWM5_CHANNEL < 8) && SOC_LEDC_SUPPORT_HS_MODE
#define PWM5_SPEEDMODE LEDC_HIGH_SPEED_MODE
#else
#define PWM5_SPEEDMODE LEDC_LOW_SPEED_MODE
#endif
#define DIO30_CHANNEL PWM5_CHANNEL
#define DIO30_TIMER PWM5_TIMER
#define DIO30_LEDCCHANNEL PWM5_LEDCCHANNEL
#define DIO30_SPEEDMODE PWM5_SPEEDMODE
#endif
#ifdef PWM6_CHANNEL
#define PWM6_LEDCCHANNEL (PWM6_CHANNEL & 0x07)
#if (PWM6_CHANNEL < 8) && SOC_LEDC_SUPPORT_HS_MODE
#define PWM6_SPEEDMODE LEDC_HIGH_SPEED_MODE
#else
#define PWM6_SPEEDMODE LEDC_LOW_SPEED_MODE
#endif
#define DIO31_CHANNEL PWM6_CHANNEL
#define DIO31_TIMER PWM6_TIMER
#define DIO31_LEDCCHANNEL PWM6_LEDCCHANNEL
#define DIO31_SPEEDMODE PWM6_SPEEDMODE
#endif
#ifdef PWM7_CHANNEL
#define PWM7_LEDCCHANNEL (PWM7_CHANNEL & 0x07)
#if (PWM7_CHANNEL < 8) && SOC_LEDC_SUPPORT_HS_MODE
#define PWM7_SPEEDMODE LEDC_HIGH_SPEED_MODE
#else
#define PWM7_SPEEDMODE LEDC_LOW_SPEED_MODE
#endif
#define DIO32_CHANNEL PWM7_CHANNEL
#define DIO32_TIMER PWM7_TIMER
#define DIO32_LEDCCHANNEL PWM7_LEDCCHANNEL
#define DIO32_SPEEDMODE PWM7_SPEEDMODE
#endif
#ifdef PWM8_CHANNEL
#define PWM8_LEDCCHANNEL (PWM8_CHANNEL & 0x07)
#if (PWM8_CHANNEL < 8) && SOC_LEDC_SUPPORT_HS_MODE
#define PWM8_SPEEDMODE LEDC_HIGH_SPEED_MODE
#else
#define PWM8_SPEEDMODE LEDC_LOW_SPEED_MODE
#endif
#define DIO33_CHANNEL PWM8_CHANNEL
#define DIO33_TIMER PWM8_TIMER
#define DIO33_LEDCCHANNEL PWM8_LEDCCHANNEL
#define DIO33_SPEEDMODE PWM8_SPEEDMODE
#endif
#ifdef PWM9_CHANNEL
#define PWM9_LEDCCHANNEL (PWM9_CHANNEL & 0x07)
#if (PWM9_CHANNEL < 8) && SOC_LEDC_SUPPORT_HS_MODE
#define PWM9_SPEEDMODE LEDC_HIGH_SPEED_MODE
#else
#define PWM9_SPEEDMODE LEDC_LOW_SPEED_MODE
#endif
#define DIO34_CHANNEL PWM9_CHANNEL
#define DIO34_TIMER PWM9_TIMER
#define DIO34_LEDCCHANNEL PWM9_LEDCCHANNEL
#define DIO34_SPEEDMODE PWM9_SPEEDMODE
#endif
#ifdef PWM10_CHANNEL
#define PWM10_LEDCCHANNEL (PWM10_CHANNEL & 0x07)
#if (PWM10_CHANNEL < 8) && SOC_LEDC_SUPPORT_HS_MODE
#define PWM10_SPEEDMODE LEDC_HIGH_SPEED_MODE
#else
#define PWM10_SPEEDMODE LEDC_LOW_SPEED_MODE
#endif
#define DIO35_CHANNEL PWM10_CHANNEL
#define DIO35_TIMER PWM10_TIMER
#define DIO35_LEDCCHANNEL PWM10_LEDCCHANNEL
#define DIO35_SPEEDMODE PWM10_SPEEDMODE
#endif
#ifdef PWM11_CHANNEL
#define PWM11_LEDCCHANNEL (PWM11_CHANNEL & 0x07)
#if (PWM11_CHANNEL < 8) && SOC_LEDC_SUPPORT_HS_MODE
#define PWM11_SPEEDMODE LEDC_HIGH_SPEED_MODE
#else
#define PWM11_SPEEDMODE LEDC_LOW_SPEED_MODE
#endif
#define DIO36_CHANNEL PWM11_CHANNEL
#define DIO36_TIMER PWM11_TIMER
#define DIO36_LEDCCHANNEL PWM11_LEDCCHANNEL
#define DIO36_SPEEDMODE PWM11_SPEEDMODE
#endif
#ifdef PWM12_CHANNEL
#define PWM12_LEDCCHANNEL (PWM12_CHANNEL & 0x07)
#if (PWM12_CHANNEL < 8) && SOC_LEDC_SUPPORT_HS_MODE
#define PWM12_SPEEDMODE LEDC_HIGH_SPEED_MODE
#else
#define PWM12_SPEEDMODE LEDC_LOW_SPEED_MODE
#endif
#define DIO37_CHANNEL PWM12_CHANNEL
#define DIO37_TIMER PWM12_TIMER
#define DIO37_LEDCCHANNEL PWM12_LEDCCHANNEL
#define DIO37_SPEEDMODE PWM12_SPEEDMODE
#endif
#ifdef PWM13_CHANNEL
#define PWM13_LEDCCHANNEL (PWM13_CHANNEL & 0x07)
#if (PWM13_CHANNEL < 8) && SOC_LEDC_SUPPORT_HS_MODE
#define PWM13_SPEEDMODE LEDC_HIGH_SPEED_MODE
#else
#define PWM13_SPEEDMODE LEDC_LOW_SPEED_MODE
#endif
#define DIO38_CHANNEL PWM13_CHANNEL
#define DIO38_TIMER PWM13_TIMER
#define DIO38_LEDCCHANNEL PWM13_LEDCCHANNEL
#define DIO38_SPEEDMODE PWM13_SPEEDMODE
#endif
#ifdef PWM14_CHANNEL
#define PWM14_LEDCCHANNEL (PWM14_CHANNEL & 0x07)
#if (PWM14_CHANNEL < 8) && SOC_LEDC_SUPPORT_HS_MODE
#define PWM14_SPEEDMODE LEDC_HIGH_SPEED_MODE
#else
#define PWM14_SPEEDMODE LEDC_LOW_SPEED_MODE
#endif
#define DIO39_CHANNEL PWM14_CHANNEL
#define DIO39_TIMER PWM14_TIMER
#define DIO39_LEDCCHANNEL PWM14_LEDCCHANNEL
#define DIO39_SPEEDMODE PWM14_SPEEDMODE
#endif
#ifdef PWM15_CHANNEL
#define PWM15_LEDCCHANNEL (PWM15_CHANNEL & 0x07)
#if (PWM15_CHANNEL < 8) && SOC_LEDC_SUPPORT_HS_MODE
#define PWM15_SPEEDMODE LEDC_HIGH_SPEED_MODE
#else
#define PWM15_SPEEDMODE LEDC_LOW_SPEED_MODE
#endif
#define DIO40_CHANNEL PWM15_CHANNEL
#define DIO40_TIMER PWM15_TIMER
#define DIO40_LEDCCHANNEL PWM15_LEDCCHANNEL
#define DIO40_SPEEDMODE PWM15_SPEEDMODE
#endif

#if (defined(TX) && defined(RX))
#define MCU_HAS_UART
#ifndef UART_PORT
#define UART_PORT 0
#endif
#endif
#if (defined(TX2) && defined(RX2))
#define MCU_HAS_UART2
#ifndef UART2_PORT
#define UART2_PORT 0
#endif
#ifndef BAUDRATE2
#define BAUDRATE2 BAUDRATE
#endif
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

// servo timer is also used by emulated PWM if used
#ifndef SERVO_TIMER
#define SERVO_TIMER 1
#endif
#define SERVO_TIMER_TG (SERVO_TIMER & 0x01)
#define SERVO_TIMER_IDX ((SERVO_TIMER >> 1) & 0x01)

#ifndef ITP_TIMER
#define ITP_TIMER 3
#endif
#define ITP_TIMER_TG (ITP_TIMER & 0x01)
#define ITP_TIMER_IDX ((ITP_TIMER >> 1) & 0x01)

#ifdef ONESHOT_TIMER
#define MCU_HAS_ONESHOT_TIMER
#define ONESHOT_TIMER_TG (ONESHOT_TIMER & 0x01)
#define ONESHOT_TIMER_IDX ((ONESHOT_TIMER >> 1) & 0x01)
#endif

// SPI
#if (defined(SPI_CLK) && defined(SPI_SDI) && defined(SPI_SDO))
#define MCU_HAS_SPI
#ifndef SPI_PORT
#define SPI_PORT 2
#endif
#ifndef SPI_MODE
#define SPI_MODE 0
#endif
#ifndef SPI_FREQ
#define SPI_FREQ 1000000UL
#endif
#endif

// Helper macros
#define __helper_ex__(left, mid, right) (left##mid##right)
#define __helper__(left, mid, right) (__helper_ex__(left, mid, right))
#ifndef __indirect__
#define __indirect__ex__(X, Y) DIO##X##_##Y
#define __indirect__(X, Y) __indirect__ex__(X, Y)
#endif

// I2C
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

#if (I2C_PORT == 0)
#define I2C_REG Wire
#else
#define I2C_REG __helper__(Wire, I2C_PORT, )
#endif
#endif

#ifndef IC74HC595_I2S_PORT
#define IC74HC595_I2S_PORT 0
#endif
#ifdef IC74HC595_CUSTOM_SHIFT_IO
#ifdef IC74HC595_COUNT
#undef IC74HC595_COUNT
#endif
#define IC74HC595_COUNT 4
#define I2SREG __helper__(I2S, IC74HC595_I2S_PORT, )

	// custom pin operations for 74HS595
	extern volatile uint32_t ic74hc595_i2s_pins;
#define ic74hc595_pin_offset(pin) (__indirect__(pin, IO_OFFSET))
#define ic74hc595_pin_mask(pin) (uint32_t)(1UL << ic74hc595_pin_offset(pin))
#define ic74hc595_set_pin(pin) __atomic_fetch_or((uint32_t *)&ic74hc595_i2s_pins, ic74hc595_pin_mask(pin), __ATOMIC_RELAXED)
#define ic74hc595_clear_pin(pin) __atomic_fetch_and((uint32_t *)&ic74hc595_i2s_pins, ~(ic74hc595_pin_mask(pin)), __ATOMIC_RELAXED)
#define ic74hc595_toggle_pin(pin) __atomic_fetch_xor((uint32_t *)&ic74hc595_i2s_pins, ic74hc595_pin_mask(pin), __ATOMIC_RELAXED)
#define ic74hc595_get_pin(pin) (__atomic_load_n((uint32_t *)&ic74hc595_i2s_pins, __ATOMIC_RELAXED) & ic74hc595_pin_mask(pin))
#endif

#define mcu_config_output(X)                                                      \
	{                                                                               \
		gpio_pad_select_gpio(__indirect__(X, BIT));                                   \
		gpio_set_direction((gpio_num_t)__indirect__(X, BIT), GPIO_MODE_INPUT_OUTPUT); \
	}
#define mcu_config_input(X)                                                \
	{                                                                        \
		gpio_pad_select_gpio(__indirect__(X, BIT));                            \
		gpio_set_direction((gpio_num_t)__indirect__(X, BIT), GPIO_MODE_INPUT); \
		gpio_pulldown_dis((gpio_num_t)__indirect__(X, BIT));                   \
		gpio_pullup_dis((gpio_num_t)__indirect__(X, BIT));                     \
	}
#define mcu_config_analog(X)                                                      \
	{                                                                               \
		mcu_config_input(X);                                                          \
		adc1_config_width(ADC_WIDTH_MAX - 1);                                         \
		adc1_config_channel_atten(__indirect__(X, ADC_CHANNEL), (ADC_ATTEN_MAX - 1)); \
	}
#define mcu_config_pullup(X)                                               \
	{                                                                        \
		gpio_pad_select_gpio(__indirect__(X, BIT));                            \
		gpio_set_direction((gpio_num_t)__indirect__(X, BIT), GPIO_MODE_INPUT); \
		gpio_pulldown_dis((gpio_num_t)__indirect__(X, BIT));                   \
		gpio_pullup_en((gpio_num_t)__indirect__(X, BIT));                      \
	}
	extern void mcu_gpio_isr(void *);
#define mcu_config_input_isr(X)                                                                              \
	{                                                                                                          \
		gpio_set_intr_type((gpio_num_t)(__indirect__(X, BIT)), GPIO_INTR_ANYEDGE);                               \
		gpio_isr_handler_add((gpio_num_t)(__indirect__(X, BIT)), mcu_gpio_isr, (void *)__indirect__(X, ISRVAR)); \
	}

#define mcu_get_input(X) gpio_get_level((gpio_num_t)__indirect__(X, BIT))
// #define mcu_get_input(X) ((__indirect__(X,INREG)->IN) & (1UL<<(0x1F & __indirect__(X, BIT))))
#define mcu_get_output(X) ((__indirect__(X, OUTREG)->OUT) & (1UL << (0x1F & __indirect__(X, BIT))))
#define mcu_set_output(X)                                                     \
	{                                                                           \
		__indirect__(X, OUTREG)->OUTSET = (1UL << (0x1F & __indirect__(X, BIT))); \
	}
#define mcu_clear_output(X)                                                   \
	{                                                                           \
		__indirect__(X, OUTREG)->OUTCLR = (1UL << (0x1F & __indirect__(X, BIT))); \
	}
#define mcu_toggle_output(X)                                                \
	{                                                                         \
		__indirect__(X, OUTREG)->OUT ^= (1UL << (0x1F & __indirect__(X, BIT))); \
	}

#define mcu_config_pwm(X, Y)                          \
	{                                                   \
		ledc_timer_config_t pwmtimer = {0};               \
		pwmtimer.speed_mode = __indirect__(X, SPEEDMODE); \
		pwmtimer.timer_num = __indirect__(X, TIMER);      \
		pwmtimer.duty_resolution = LEDC_TIMER_8_BIT;      \
		pwmtimer.freq_hz = Y;                             \
		pwmtimer.clk_cfg = LEDC_AUTO_CLK;                 \
		ledc_timer_config(&pwmtimer);                     \
		ledc_channel_config_t pwm = {0};                  \
		pwm.channel = __indirect__(X, CHANNEL);           \
		pwm.duty = 0;                                     \
		pwm.gpio_num = __indirect__(X, BIT);              \
		pwm.hpoint = 0;                                   \
		pwm.speed_mode = __indirect__(X, SPEEDMODE);      \
		pwm.timer_sel = __indirect__(X, TIMER);           \
		ledc_channel_config(&pwm);                        \
	}

#define mcu_set_pwm(X, Y)                                                       \
	{                                                                             \
		ledc_set_duty(__indirect__(X, SPEEDMODE), __indirect__(X, LEDCCHANNEL), Y); \
		ledc_update_duty(__indirect__(X, SPEEDMODE), __indirect__(X, LEDCCHANNEL)); \
	}
#define mcu_get_pwm(X) ledc_get_duty(__indirect__(X, SPEEDMODE), __indirect__(X, LEDCCHANNEL))
#define mcu_get_analog(X) (adc1_get_raw(__indirect__(X, ADC_CHANNEL)) >> (ADC_WIDTH_MAX - 2))

	extern void esp32_delay_us(uint16_t delay);
#define mcu_delay_us(X) esp32_delay_us(X)

#include "xtensa/core-macros.h"
#define mcu_delay_cycles(X)                          \
	{                                                  \
		uint32_t x = XTHAL_GET_CCOUNT();                 \
		while (X > (((uint32_t)XTHAL_GET_CCOUNT()) - x)) \
			;                                              \
	}

#ifdef __cplusplus
}
#endif

#endif
