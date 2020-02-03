/*
	Name: mcumap.h
	Description: Constains all basic definitions about the I/O names and masks.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 01/11/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef MCUMAP_H
#define MCUMAP_H

#include "config.h"
#include "mcudefs.h"

#define BITMASK0 1
#define BITMASK1 2
#define BITMASK2 4
#define BITMASK3 8
#define BITMASK4 16
#define BITMASK5 32
#define BITMASK6 64
#define BITMASK7 128
#define _BITMASK(X) BITMASK##X
#define BITMASK(X) _BITMASK(X)

#ifdef STEP0
#define STEP0_MASK BITMASK(STEP0)
#else
#define STEP0_MASK 0
#endif
#ifdef DIR0
#define DIR0_MASK BITMASK(DIR0)
#else
#define DIR0_MASK 0
#endif
#ifdef STEP1
#define STEP1_MASK BITMASK(STEP1)
#else
#define STEP1_MASK 0
#endif
#ifdef DIR1
#define DIR1_MASK BITMASK(DIR1)
#else
#define DIR1_MASK 0
#endif
#ifdef STEP2
#define STEP2_MASK BITMASK(STEP2)
#else
#define STEP2_MASK 0
#endif
#ifdef DIR2
#define DIR2_MASK BITMASK(DIR2)
#else
#define DIR2_MASK 0
#endif
#ifdef STEP3
#define STEP3_MASK BITMASK(STEP3)
#else
#define STEP3_MASK 0
#endif
#ifdef DIR3
#define DIR3_MASK BITMASK(DIR3)
#else
#define DIR3_MASK 0
#endif
#ifdef STEP4
#define STEP4_MASK BITMASK(STEP4)
#else
#define STEP4_MASK 0
#endif
#ifdef DIR4
#define DIR4_MASK BITMASK(DIR4)
#else
#define DIR4_MASK 0
#endif
#ifdef STEP5
#define STEP5_MASK BITMASK(STEP5)
#else
#define STEP5_MASK 0
#endif
#ifdef DIR5
#define DIR5_MASK BITMASK(DIR5)
#else
#define DIR5_MASK 0
#endif

#define STEPS_MASK (STEP0_MASK | STEP1_MASK | STEP2_MASK | STEP3_MASK | STEP4_MASK | STEP5_MASK)
#define DIRS_MASK (DIR0_MASK | DIR1_MASK | DIR2_MASK | DIR3_MASK | DIR4_MASK | DIR5_MASK)

#ifndef STEPS_OUTREG
#error Undefined step output register
#endif
#ifndef DIRS_OUTREG
#error Undefined dir output register
#endif

#ifdef DOUT0
#define DOUT0_MASK (BITMASK(DOUT0))
#else
#define DOUT0_MASK 0
#endif
#ifdef DOUT1
#define DOUT1_MASK (BITMASK(DOUT1))
#else
#define DOUT1_MASK 0
#endif
#ifdef DOUT2
#define DOUT2_MASK (BITMASK(DOUT2))
#else
#define DOUT2_MASK 0
#endif
#ifdef DOUT3
#define DOUT3_MASK (BITMASK(DOUT3))
#else
#define DOUT3_MASK 0
#endif
#ifdef DOUT4
#define DOUT4_MASK (BITMASK(DOUT4))
#else
#define DOUT4_MASK 0
#endif
#ifdef DOUT5
#define DOUT5_MASK (BITMASK(DOUT5))
#else
#define DOUT5_MASK 0
#endif
#ifdef DOUT6
#define DOUT6_MASK (BITMASK(DOUT6))
#else
#define DOUT6_MASK 0
#endif
#ifdef DOUT7
#define DOUT7_MASK (BITMASK(DOUT7))
#else
#define DOUT7_MASK 0
#endif
#ifdef DOUT8
#define DOUT8_MASK (BITMASK(DOUT8)<<8)
#else
#define DOUT8_MASK 0
#endif
#ifdef DOUT9
#define DOUT9_MASK (BITMASK(DOUT9)<<8)
#else
#define DOUT9_MASK 0
#endif
#ifdef DOUT10
#define DOUT10_MASK (BITMASK(DOUT10)<<8)
#else
#define DOUT10_MASK 0
#endif
#ifdef DOUT11
#define DOUT11_MASK (BITMASK(DOUT11)<<8)
#else
#define DOUT11_MASK 0
#endif
#ifdef DOUT12
#define DOUT12_MASK (BITMASK(DOUT12)<<8)
#else
#define DOUT12_MASK 0
#endif
#ifdef DOUT13
#define DOUT13_MASK (BITMASK(DOUT13)<<8)
#else
#define DOUT13_MASK 0
#endif
#ifdef DOUT14
#define DOUT14_MASK (BITMASK(DOUT14)<<8)
#else
#define DOUT14_MASK 0
#endif
#ifdef DOUT15
#define DOUT15_MASK (BITMASK(DOUT15)<<8)
#else
#define DOUT15_MASK 0
#endif
#ifdef DOUT16
#define DOUT16_MASK (BITMASK(DOUT16)<<16)
#else
#define DOUT16_MASK 0
#endif
#ifdef DOUT17
#define DOUT17_MASK (BITMASK(DOUT17)<<16)
#else
#define DOUT17_MASK 0
#endif
#ifdef DOUT18
#define DOUT18_MASK (BITMASK(DOUT18)<<16)
#else
#define DOUT18_MASK 0
#endif
#ifdef DOUT19
#define DOUT19_MASK (BITMASK(DOUT19)<<16)
#else
#define DOUT19_MASK 0
#endif
#ifdef DOUT20
#define DOUT20_MASK (BITMASK(DOUT20)<<16)
#else
#define DOUT20_MASK 0
#endif
#ifdef DOUT21
#define DOUT21_MASK (BITMASK(DOUT21)<<16)
#else
#define DOUT21_MASK 0
#endif
#ifdef DOUT22
#define DOUT22_MASK (BITMASK(DOUT22)<<16)
#else
#define DOUT22_MASK 0
#endif
#ifdef DOUT23
#define DOUT23_MASK (BITMASK(DOUT23)<<16)
#else
#define DOUT23_MASK 0
#endif
#ifdef DOUT24
#define DOUT24_MASK (BITMASK(DOUT24)<<24)
#else
#define DOUT24_MASK 0
#endif
#ifdef DOUT25
#define DOUT25_MASK (BITMASK(DOUT25)<<24)
#else
#define DOUT25_MASK 0
#endif
#ifdef DOUT26
#define DOUT26_MASK (BITMASK(DOUT26)<<24)
#else
#define DOUT26_MASK 0
#endif
#ifdef DOUT27
#define DOUT27_MASK (BITMASK(DOUT27)<<24)
#else
#define DOUT27_MASK 0
#endif
#ifdef DOUT28
#define DOUT28_MASK (BITMASK(DOUT28)<<24)
#else
#define DOUT28_MASK 0
#endif
#ifdef DOUT29
#define DOUT29_MASK (BITMASK(DOUT29)<<24)
#else
#define DOUT29_MASK 0
#endif
#ifdef DOUT30
#define DOUT30_MASK (BITMASK(DOUT30)<<24)
#else
#define DOUT30_MASK 0
#endif
#ifdef DOUT31
#define DOUT31_MASK (BITMASK(DOUT31)<<24)
#else
#define DOUT31_MASK 0
#endif

#define DOUTS_R0 (DOUT0_MASK | DOUT1_MASK | DOUT2_MASK | DOUT3_MASK | DOUT4_MASK | DOUT5_MASK | DOUT6_MASK | DOUT7_MASK)
#define DOUTS_R1 (DOUT8_MASK | DOUT9_MASK | DOUT10_MASK | DOUT11_MASK | DOUT12_MASK | DOUT13_MASK | DOUT14_MASK | DOUT15_MASK)
#define DOUTS_R2 (DOUT16_MASK | DOUT17_MASK | DOUT18_MASK | DOUT19_MASK | DOUT20_MASK | DOUT21_MASK | DOUT22_MASK | DOUT23_MASK)
#define DOUTS_R3 (DOUT24_MASK | DOUT25_MASK | DOUT26_MASK | DOUT27_MASK | DOUT28_MASK | DOUT29_MASK | DOUT30_MASK | DOUT31_MASK)
#define DOUTS_R0_MASK DOUTS_R0
#define DOUTS_R1_MASK (DOUTS_R1>>8)
#define DOUTS_R2_MASK (DOUTS_R2>>16)
#define DOUTS_R3_MASK (DOUTS_R3>>24)
#define DOUTS_MASK (DOUTS_R0 | DOUTS_R1 | DOUTS_R2 | DOUTS_R3)

#if(DOUTS_R0_MASK != 0)
#ifndef DOUTS_R0_OUTREG
#error Undefined digital output register 0
#endif
#endif

#if(DOUTS_R1_MASK != 0)
#ifndef DOUTS_R1_OUTREG
#error Undefined digital output register 1
#endif
#endif

#if(DOUTS_R2_MASK != 0)
#ifndef DOUTS_R2_OUTREG
#error Undefined digital output register 2
#endif
#endif

#if(DOUTS_R3_MASK != 0)
#ifndef DOUTS_R3_OUTREG
#error Undefined digital output register 3
#endif
#endif

#ifdef ESTOP
#define ESTOP_MASK BITMASK(ESTOP)
#else
#define ESTOP_MASK 0
#endif
#ifdef FHOLD
#define FHOLD_MASK BITMASK(FHOLD)
#else
#define FHOLD_MASK 0
#endif
#ifdef CS_RES
#define CS_RES_MASK BITMASK(CS_RES)
#else
#define CS_RES_MASK 0
#endif
#ifdef SAFETY_DOOR
#define SAFETY_DOOR_MASK BITMASK(SAFETY_DOOR)
#else
#define SAFETY_DOOR_MASK 0
#endif

#define CONTROLS_MASK (ESTOP_MASK | FHOLD_MASK | CS_RES_MASK | SAFETY_DOOR_MASK)

#if(CONTROLS_MASK != 0)
#ifndef CONTROLS_INREG
#error Undefined controls input register
#endif
#endif

#ifdef PWM0
#define PWM0_MASK BITMASK(PWM0)
#else
#define PWM0_MASK 0
#endif
#ifdef PWM1
#define PWM1_MASK BITMASK(PWM1)
#else
#define PWM1_MASK 0
#endif
#ifdef PWM2
#define PWM2_MASK BITMASK(PWM2)
#else
#define PWM2_MASK 0
#endif
#ifdef PWM3
#define PWM3_MASK BITMASK(PWM3)
#else
#define PWM3_MASK 0
#endif

#define PWMS_MASK (PWM0_MASK | PWM1_MASK | PWM1_MASK | PWM2_MASK | PWM3_MASK)

#ifdef ANALOG0
#define ANALOG0_MASK BITMASK(ANALOG0)
#else
#define ANALOG0_MASK 0
#endif
#ifdef ANALOG1
#define ANALOG1_MASK BITMASK(ANALOG1)
#else
#define ANALOG1_MASK 0
#endif
#ifdef ANALOG2
#define ANALOG2_MASK BITMASK(ANALOG2)
#else
#define ANALOG2_MASK 0
#endif
#ifdef ANALOG3
#define ANALOG3_MASK BITMASK(ANALOG3)
#else
#define ANALOG3_MASK 0
#endif

#define ANALOGS_MASK (ANALOG0_MASK | ANALOG1_MASK | ANALOG1_MASK | ANALOG2_MASK | ANALOG3_MASK)

#ifdef PROBE
#define PROBE_MASK BITMASK(PROBE)
#ifndef PROBE_INREG
#error Undefined probe input register
#endif
#else
#define PROBE_MASK 0
#endif

#ifdef LIMIT_X
#define LIMIT_X_MASK BITMASK(LIMIT_X)
#else
#define LIMIT_X_MASK 0
#endif
#ifdef LIMIT_Y
#define LIMIT_Y_MASK BITMASK(LIMIT_Y)
#else
#define LIMIT_Y_MASK 0
#endif
#ifdef LIMIT_Z
#define LIMIT_Z_MASK BITMASK(LIMIT_Z)
#else
#define LIMIT_Z_MASK 0
#endif
#ifdef LIMIT_A
#define LIMIT_A_MASK BITMASK(LIMIT_A)
#else
#define LIMIT_A_MASK 0
#endif
#ifdef LIMIT_B
#define LIMIT_B_MASK BITMASK(LIMIT_B)
#else
#define LIMIT_B_MASK 0
#endif
#ifdef LIMIT_C
#define LIMIT_C_MASK BITMASK(LIMIT_C)
#else
#define LIMIT_C_MASK 0
#endif

#define LIMITS_MASK (LIMIT_X_MASK | LIMIT_Y_MASK | LIMIT_Z_MASK | LIMIT_A_MASK | LIMIT_B_MASK | LIMIT_C_MASK)

#if(LIMITS_MASK != 0)
#ifndef LIMITS_INREG
#error Undefined limits input register
#endif
#endif

#ifdef RX
#define RX_MASK BITMASK(RX)
#else
#define RX_MASK 0
#endif
#ifdef TX
#define TX_MASK BITMASK(TX)
#else
#define TX_MASK 0
#endif

#define COM_MASK (RX_MASK | TX_MASK)
#ifndef COM_INREG
#error Undefined COM input register
#endif
#ifndef COM_OUTREG
#error Undefined COM output register
#endif

#ifdef DIN0
#define DIN0_MASK BITMASK(DIN0)
#else
#define DIN0_MASK 0
#endif
#ifdef DIN1
#define DIN1_MASK BITMASK(DIN1)
#else
#define DIN1_MASK 0
#endif
#ifdef DIN2
#define DIN2_MASK BITMASK(DIN2)
#else
#define DIN2_MASK 0
#endif
#ifdef DIN3
#define DIN3_MASK BITMASK(DIN3)
#else
#define DIN3_MASK 0
#endif
#ifdef DIN4
#define DIN4_MASK BITMASK(DIN4)
#else
#define DIN4_MASK 0
#endif
#ifdef DIN5
#define DIN5_MASK BITMASK(DIN5)
#else
#define DIN5_MASK 0
#endif
#ifdef DIN6
#define DIN6_MASK BITMASK(DIN6)
#else
#define DIN6_MASK 0
#endif
#ifdef DIN7
#define DIN7_MASK BITMASK(DIN7)
#else
#define DIN7_MASK 0
#endif
#ifdef DIN8
#define DIN8_MASK (BITMASK(DIN8)<<8)
#else
#define DIN8_MASK 0
#endif
#ifdef DIN9
#define DIN9_MASK (BITMASK(DIN9)<<8)
#else
#define DIN9_MASK 0
#endif
#ifdef DIN10
#define DIN10_MASK (BITMASK(DIN10)<<8)
#else
#define DIN10_MASK 0
#endif
#ifdef DIN11
#define DIN11_MASK (BITMASK(DIN11)<<8)
#else
#define DIN11_MASK 0
#endif
#ifdef DIN12
#define DIN12_MASK (BITMASK(DIN12)<<8)
#else
#define DIN12_MASK 0
#endif
#ifdef DIN13
#define DIN13_MASK (BITMASK(DIN13)<<8)
#else
#define DIN13_MASK 0
#endif
#ifdef DIN14
#define DIN14_MASK (BITMASK(DIN14)<<8)
#else
#define DIN14_MASK 0
#endif
#ifdef DIN15
#define DIN15_MASK (BITMASK(DIN15)<<8)
#else
#define DIN15_MASK 0
#endif
#ifdef DIN16
#define DIN16_MASK (BITMASK(DIN16)<<16)
#else
#define DIN16_MASK 0
#endif
#ifdef DIN17
#define DIN17_MASK (BITMASK(DIN17)<<16)
#else
#define DIN17_MASK 0
#endif
#ifdef DIN18
#define DIN18_MASK (BITMASK(DIN18)<<16)
#else
#define DIN18_MASK 0
#endif
#ifdef DIN19
#define DIN19_MASK (BITMASK(DIN19)<<16)
#else
#define DIN19_MASK 0
#endif
#ifdef DIN20
#define DIN20_MASK (BITMASK(DIN20)<<16)
#else
#define DIN20_MASK 0
#endif
#ifdef DIN21
#define DIN21_MASK (BITMASK(DIN21)<<16)
#else
#define DIN21_MASK 0
#endif
#ifdef DIN22
#define DIN22_MASK (BITMASK(DIN22)<<16)
#else
#define DIN22_MASK 0
#endif
#ifdef DIN23
#define DIN23_MASK (BITMASK(DIN23)<<16)
#else
#define DIN23_MASK 0
#endif
#ifdef DIN24
#define DIN24_MASK (BITMASK(DIN24)<<24)
#else
#define DIN24_MASK 0
#endif
#ifdef DIN25
#define DIN25_MASK (BITMASK(DIN25)<<24)
#else
#define DIN25_MASK 0
#endif
#ifdef DIN26
#define DIN26_MASK (BITMASK(DIN26)<<24)
#else
#define DIN26_MASK 0
#endif
#ifdef DIN27
#define DIN27_MASK (BITMASK(DIN27)<<24)
#else
#define DIN27_MASK 0
#endif
#ifdef DIN28
#define DIN28_MASK (BITMASK(DIN28)<<24)
#else
#define DIN28_MASK 0
#endif
#ifdef DIN29
#define DIN29_MASK (BITMASK(DIN29)<<24)
#else
#define DIN29_MASK 0
#endif
#ifdef DIN30
#define DIN30_MASK (BITMASK(DIN30)<<24)
#else
#define DIN30_MASK 0
#endif
#ifdef DIN31
#define DIN31_MASK (BITMASK(DIN31)<<24)
#else
#define DIN31_MASK 0
#endif

#define DINS_R0 (DIN0_MASK | DIN1_MASK | DIN2_MASK | DIN3_MASK | DIN4_MASK | DIN5_MASK | DIN6_MASK | DIN7_MASK)
#define DINS_R1 (DIN8_MASK | DIN9_MASK | DIN10_MASK | DIN11_MASK | DIN12_MASK | DIN13_MASK | DIN14_MASK | DIN15_MASK)
#define DINS_R2 (DIN16_MASK | DIN17_MASK | DIN18_MASK | DIN19_MASK | DIN20_MASK | DIN21_MASK | DIN22_MASK | DIN23_MASK)
#define DINS_R3 (DIN24_MASK | DIN25_MASK | DIN26_MASK | DIN27_MASK | DIN28_MASK | DIN29_MASK | DIN30_MASK | DIN31_MASK)
#define DINS_R0_MASK DINS_R0
#define DINS_R1_MASK (DINS_R1>>8)
#define DINS_R2_MASK (DINS_R2>>16)
#define DINS_R3_MASK (DINS_R3>>24)
#define DINS_MASK (DINS_R0 | DINS_R1 | DINS_R2 | DINS_R3)

#if(DINS_R0_MASK != 0)
#ifndef DINS_R0_INREG
#error Undefined digital input register 0
#endif
#endif

#if(DINS_R1_MASK != 0)
#ifndef DINS_R1_INREG
#error Undefined digital input register 1
#endif
#endif

#if(DINS_R2_MASK != 0)
#ifndef DINS_R2_INREG
#error Undefined digital input register 2
#endif
#endif

#if(DINS_R3_MASK != 0)
#ifndef DINS_R3_INREG
#error Undefined digital input register 3
#endif
#endif

//Generates mask DOUT/DIN names based on the IO number
#define _OUTPIN_MASK(X) DOUT##X##_MASK
#define OUTPIN_MASK(X) _OUTPIN_MASK(X)
#define _INPIN_MASK(X) DIN##X##_MASK
#define INPIN_MASK(X) _INPIN_MASK(X)

//Defines masks for all generic purpose pins (using the DOUTS or DIN)
#ifdef STEP0_EN_OUTPIN
#define STEP0_EN_MASK (OUTPIN_MASK(STEP0_EN_OUTPIN))
#else
#define STEP0_EN_MASK 0
#endif
#ifdef STEP1_EN_OUTPIN
#define STEP1_EN_MASK (OUTPIN_MASK(STEP1_EN_OUTPIN))
#else
#define STEP1_EN_MASK 0
#endif
#ifdef STEP2_EN_OUTPIN
#define STEP2_EN_MASK (OUTPIN_MASK(STEP2_EN_OUTPIN))
#else
#define STEP2_EN_MASK 0
#endif
#ifdef STEP3_EN_OUTPIN
#define STEP3_EN_MASK (OUTPIN_MASK(STEP3_EN_OUTPIN))
#else
#define STEP3_EN_MASK 0
#endif
#ifdef STEP4_EN_OUTPIN
#define STEP4_EN_MASK (OUTPIN_MASK(STEP4_EN_OUTPIN))
#else
#define STEP4_EN_MASK 0
#endif
#ifdef STEP5_EN_OUTPIN
#define STEP5_EN_MASK (OUTPIN_MASK(STEP5_EN_OUTPIN))
#else
#define STEP5_EN_MASK 0
#endif

#define STEPS_EN_MASK (STEP0_EN_MASK | STEP1_EN_MASK | STEP2_EN_MASK | STEP3_EN_MASK | STEP4_EN_MASK | STEP5_EN_MASK)

#ifdef SPINDLE_DIR_OUTPIN
#define SPINDLE_DIR_MASK (OUTPIN_MASK(SPINDLE_DIR_OUTPIN))
#else
#define SPINDLE_DIR_MASK 0
#endif
#ifdef COOLANT_FLOOD_OUTPIN
#define COOLANT_FLOOD_MASK (OUTPIN_MASK(COOLANT_FLOOD_OUTPIN))
#else
#define COOLANT_FLOOD_MASK 0
#endif
#ifdef COOLANT_MIST_OUTPIN
#define COOLANT_MIST_MASK (OUTPIN_MASK(COOLANT_MIST_OUTPIN))
#else
#define COOLANT_MIST_MASK 0
#endif

#endif
