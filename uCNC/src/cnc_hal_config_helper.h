/*
	Name: cnc_hal_config_helper.h
	Description: Compile time configurations for µCNC. This file takes care of some final configuration definitions based on the user options

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 2022-01-04

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef CNC_HAL_CONFIG_HELPER_H
#define CNC_HAL_CONFIG_HELPER_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef LIMIT_X_DISABLE
#ifdef LIMIT_X
#undef LIMIT_X
#define LIMIT_X -1
#endif
#endif
#ifdef LIMIT_X2_DISABLE
#ifdef LIMIT_X2
#undef LIMIT_X2
#define LIMIT_X2 -1
#endif
#endif
#ifdef LIMIT_Y_DISABLE
#ifdef LIMIT_Y
#undef LIMIT_Y
#define LIMIT_Y -1
#endif
#endif
#ifdef LIMIT_Y2_DISABLE
#ifdef LIMIT_Y2
#undef LIMIT_Y2
#define LIMIT_Y2 -1
#endif
#endif
#ifdef LIMIT_Z_DISABLE
#ifdef LIMIT_Z
#undef LIMIT_Z
#define LIMIT_Z -1
#endif
#endif
#ifdef LIMIT_Z2_DISABLE
#ifdef LIMIT_Z2
#undef LIMIT_Z2
#define LIMIT_Z2 -1
#endif
#endif
#ifdef LIMIT_A_DISABLE
#ifdef LIMIT_A
#undef LIMIT_A
#define LIMIT_A -1
#endif
#endif
#ifdef LIMIT_B_DISABLE
#ifdef LIMIT_B
#undef LIMIT_B
#define LIMIT_B -1
#endif
#endif
#ifdef LIMIT_C_DISABLE
#ifdef LIMIT_C
#undef LIMIT_C
#define LIMIT_C -1
#endif
#endif

#ifndef ENCODERS
#define ENCODERS 0
#endif

#ifndef PID_CONTROLLERS
#define PID_CONTROLLERS 0
#endif

#if PID_CONTROLLERS > 0
/*PID controllers*/
#if PID_CONTROLLERS == 1
#define PID_DIVISIONS 0
#elif PID_CONTROLLERS == 2
#define PID_DIVISIONS 1
#elif PID_CONTROLLERS <= 4
#define PID_DIVISIONS 2
#else
#define PID_DIVISIONS 3
#endif

#define PID_SAMP_FREQ (1 << (10 - PID_DIVISIONS))
#endif

#if PID_CONTROLLERS > 0
#ifdef PID0_DELTA
#error "The PID0 is reserved for the tool PID"
#else
#define PID0_DELTA() tool_pid_error()
#endif
#ifdef PID0_OUTPUT
#error "The PID0 is reserved for the tool PID"
#else
#define PID0_OUTPUT(X) tool_pid_update(X)
#endif
#ifdef PID0_STOP
#error "The PID0 is reserved for the tool PID"
#else
#define PID0_STOP() tool_stop()
#endif
#ifndef PID0_FREQ_DIV
#define PID0_FREQ_DIV 1
#elif (PID0_FREQ_DIV < 1 || PID0_FREQ_DIV > PID_SAMP_FREQ)
#error "The PID0 sampling frequency devider value must be between 1 and MAX SAMPLE RATE = 1000/log2(Total PID's)"
#endif
#endif
#if PID_CONTROLLERS > 1
#ifndef PID1_DELTA
#error "The PID1 error is not defined"
#endif
#ifndef PID1_OUTPUT
#error "The PID1 output is not defined"
#endif
#ifndef PID1_STOP
#error "The PID1 stop is not defined"
#endif
#ifndef PID1_FREQ_DIV
#define PID1_FREQ_DIV 1
#elif (PID1_FREQ_DIV < 1 || PID1_FREQ_DIV > PID_SAMP_FREQ)
#error "The PID1 sampling frequency devider value must be between 1 and MAX SAMPLE RATE = 1000/log2(Total PID's)"
#endif
#endif
#if PID_CONTROLLERS > 2
#ifndef PID2_DELTA
#error "The PID2 error is not defined"
#endif
#ifndef PID2_OUTPUT
#error "The PID2 output is not defined"
#endif
#ifndef PID2_STOP
#error "The PID2 stop is not defined"
#endif
#ifndef PID2_FREQ_DIV
#define PID2_FREQ_DIV 1
#elif (PID2_FREQ_DIV < 1 || PID2_FREQ_DIV > PID_SAMP_FREQ)
#error "The PID2 sampling frequency devider value must be between 1 and MAX SAMPLE RATE = 1000/log2(Total PID's)"
#endif
#endif
#if PID_CONTROLLERS > 3
#ifndef PID3_DELTA
#error "The PID3 error is not defined"
#endif
#ifndef PID3_OUTPUT
#error "The PID3 output is not defined"
#endif
#ifndef PID3_STOP
#error "The PID3 stop is not defined"
#endif
#ifndef PID3_FREQ_DIV
#define PID3_FREQ_DIV 1
#elif (PID3_FREQ_DIV < 1 || PID3_FREQ_DIV > PID_SAMP_FREQ)
#error "The PID3 sampling frequency devider value must be between 1 and MAX SAMPLE RATE = 1000/log2(Total PID's)"
#endif
#endif
#if PID_CONTROLLERS > 4
#ifndef PID4_DELTA
#error "The PID4 error is not defined"
#endif
#ifndef PID4_OUTPUT
#error "The PID4 output is not defined"
#endif
#ifndef PID4_STOP
#error "The PID4 stop is not defined"
#endif
#ifndef PID4_FREQ_DIV
#define PID4_FREQ_DIV 1
#elif (PID4_FREQ_DIV < 1 || PID4_FREQ_DIV > PID_SAMP_FREQ)
#error "The PID4 sampling frequency devider value must be between 1 and MAX SAMPLE RATE = 1000/log2(Total PID's)"
#endif
#endif
#if PID_CONTROLLERS > 5
#ifndef PID5_DELTA
#error "The PID5 error is not defined"
#endif
#ifndef PID5_OUTPUT
#error "The PID5 output is not defined"
#endif
#ifndef PID5_STOP
#error "The PID5 stop is not defined"
#endif
#ifndef PID5_FREQ_DIV
#define PID5_FREQ_DIV 1
#elif (PID5_FREQ_DIV < 1 || PID5_FREQ_DIV > PID_SAMP_FREQ)
#error "The PID5 sampling frequency devider value must be between 1 and MAX SAMPLE RATE = 1000/log2(Total PID's)"
#endif
#endif
#if PID_CONTROLLERS > 6
#ifndef PID6_DELTA
#error "The PID6 error is not defined"
#endif
#ifndef PID6_OUTPUT
#error "The PID6 output is not defined"
#endif
#ifndef PID6_STOP
#error "The PID6 stop is not defined"
#endif
#ifndef PID6_FREQ_DIV
#define PID6_FREQ_DIV 1
#elif (PID6_FREQ_DIV < 1 || PID6_FREQ_DIV > PID_SAMP_FREQ)
#error "The PID6 sampling frequency devider value must be between 1 and MAX SAMPLE RATE = 1000/log2(Total PID's)"
#endif
#endif
#if PID_CONTROLLERS > 7
#ifndef PID7_DELTA
#error "The PID7 error is not defined"
#endif
#ifndef PID7_OUTPUT
#error "The PID7 output is not defined"
#endif
#ifndef PID7_STOP
#error "The PID7 stop is not defined"
#endif
#ifndef PID7_FREQ_DIV
#define PID7_FREQ_DIV 1
#elif (PID7_FREQ_DIV < 1 || PID7_FREQ_DIV > PID_SAMP_FREQ)
#error "The PID7 sampling frequency devider value must be between 1 and MAX SAMPLE RATE = 1000/log2(Total PID's)"
#endif
#endif

#ifdef STEPPER0_HAS_TMC
#if (STEPPER0_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#if (STEPPER0_UART_TX < 0 || STEPPER0_UART_RX < 0)
#undef STEPPER0_HAS_TMC
#error "Stepper 0 undefined UART pins"
#endif
#elif (STEPPER0_TMC_INTERFACE == TMC_SPI)
#if (STEPPER0_UART_DO < 0 || STEPPER0_UART_DI < 0 || STEPPER0_UART_CLK < 0 || STEPPER0_UART_CS < 0)
#undef STEPPER0_HAS_TMC
#error "Stepper 0 undefined SPI pins"
#endif
#endif
#endif
#ifdef STEPPER1_HAS_TMC
#if (STEPPER1_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#if (STEPPER1_UART_TX < 0 || STEPPER1_UART_RX < 0)
#undef STEPPER1_HAS_TMC
#error "Stepper 1 undefined UART pins"
#endif
#elif (STEPPER1_TMC_INTERFACE == TMC_SPI)
#if (STEPPER1_UART_DO < 0 || STEPPER1_UART_DI < 0 || STEPPER1_UART_CLK < 0 || STEPPER1_UART_CS < 0)
#undef STEPPER1_HAS_TMC
#error "Stepper 1 undefined SPI pins"
#endif
#endif
#endif
#ifdef STEPPER2_HAS_TMC
#if (STEPPER2_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#if (STEPPER2_UART_TX < 0 || STEPPER2_UART_RX < 0)
#undef STEPPER2_HAS_TMC
#error "Stepper 2 undefined UART pins"
#endif
#elif (STEPPER2_TMC_INTERFACE == TMC_SPI)
#if (STEPPER2_UART_DO < 0 || STEPPER2_UART_DI < 0 || STEPPER2_UART_CLK < 0 || STEPPER2_UART_CS < 0)
#undef STEPPER2_HAS_TMC
#error "Stepper 2 undefined SPI pins"
#endif
#endif
#endif
#ifdef STEPPER3_HAS_TMC
#if (STEPPER3_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#if (STEPPER3_UART_TX < 0 || STEPPER3_UART_RX < 0)
#undef STEPPER3_HAS_TMC
#error "Stepper 3 undefined UART pins"
#endif
#elif (STEPPER3_TMC_INTERFACE == TMC_SPI)
#if (STEPPER3_UART_DO < 0 || STEPPER3_UART_DI < 0 || STEPPER3_UART_CLK < 0 || STEPPER3_UART_CS < 0)
#undef STEPPER3_HAS_TMC
#error "Stepper 3 undefined SPI pins"
#endif
#endif
#endif
#ifdef STEPPER4_HAS_TMC
#if (STEPPER4_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#if (STEPPER4_UART_TX < 0 || STEPPER4_UART_RX < 0)
#undef STEPPER4_HAS_TMC
#error "Stepper 4 undefined UART pins"
#endif
#elif (STEPPER4_TMC_INTERFACE == TMC_SPI)
#if (STEPPER4_UART_DO < 0 || STEPPER4_UART_DI < 0 || STEPPER4_UART_CLK < 0 || STEPPER4_UART_CS < 0)
#undef STEPPER4_HAS_TMC
#error "Stepper 4 undefined SPI pins"
#endif
#endif
#endif
#ifdef STEPPER5_HAS_TMC
#if (STEPPER5_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#if (STEPPER5_UART_TX < 0 || STEPPER5_UART_RX < 0)
#undef STEPPER5_HAS_TMC
#error "Stepper 5 undefined UART pins"
#endif
#elif (STEPPER5_TMC_INTERFACE == TMC_SPI)
#if (STEPPER5_UART_DO < 0 || STEPPER5_UART_DI < 0 || STEPPER5_UART_CLK < 0 || STEPPER5_UART_CS < 0)
#undef STEPPER5_HAS_TMC
#error "Stepper 5 undefined SPI pins"
#endif
#endif
#endif
#ifdef STEPPER6_HAS_TMC
#if (STEPPER6_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#if (STEPPER6_UART_TX < 0 || STEPPER6_UART_RX < 0)
#undef STEPPER6_HAS_TMC
#error "Stepper 6 undefined UART pins"
#endif
#elif (STEPPER6_TMC_INTERFACE == TMC_SPI)
#if (STEPPER6_UART_DO < 0 || STEPPER6_UART_DI < 0 || STEPPER6_UART_CLK < 0 || STEPPER6_UART_CS < 0)
#undef STEPPER6_HAS_TMC
#error "Stepper 6 undefined SPI pins"
#endif
#endif
#endif
#ifdef STEPPER7_HAS_TMC
#if (STEPPER7_TMC_INTERFACE == TMC_UART)
// if driver uses uart set pins
#if (STEPPER7_UART_TX < 0 || STEPPER7_UART_RX < 0)
#undef STEPPER7_HAS_TMC
#error "Stepper 7 undefined UART pins"
#endif
#elif (STEPPER7_TMC_INTERFACE == TMC_SPI)
#if (STEPPER7_UART_DO < 0 || STEPPER7_UART_DI < 0 || STEPPER7_UART_CLK < 0 || STEPPER7_UART_CS < 0)
#undef STEPPER7_HAS_TMC
#error "Stepper 7 undefined SPI pins"
#endif
#endif
#endif

#if defined(STEPPER0_HAS_TMC) || defined(STEPPER1_HAS_TMC) || defined(STEPPER2_HAS_TMC) || defined(STEPPER3_HAS_TMC) || defined(STEPPER4_HAS_TMC) || defined(STEPPER5_HAS_TMC) || defined(STEPPER6_HAS_TMC) || defined(STEPPER7_HAS_TMC)
#define ENABLE_TMC_DRIVERS
#define ENABLE_MAIN_LOOP_MODULES
#endif

#if defined(STEPPER0_HAS_MSTEP) || defined(STEPPER1_HAS_MSTEP) || defined(STEPPER2_HAS_MSTEP) || defined(STEPPER3_HAS_MSTEP) || defined(STEPPER4_HAS_MSTEP) || defined(STEPPER5_HAS_MSTEP) || defined(STEPPER6_HAS_MSTEP) || defined(STEPPER7_HAS_MSTEP)
#define ENABLE_DIGITAL_MSTEP
#endif

#define __stepname_helper__(x) STEP##x##_MASK
#define __stepname__(x) __stepname_helper__(x)

#define __axisname_helper__(x) AXIS_##x
#define __axisname__(x) __axisname_helper__(x)

#define __limitname_helper__(x) LIMIT_##x##_MASK
#define __limitname__(x) __limitname_helper__(x)

#ifdef ENABLE_DUAL_DRIVE_AXIS

#ifndef DUAL_DRIVE0_STEPPER
#define DUAL_DRIVE0_STEPPER 6
#endif
#ifndef DUAL_DRIVE1_STEPPER
#define DUAL_DRIVE1_STEPPER 7
#endif

#if (!defined(DUAL_DRIVE0_AXIS) && !defined(DUAL_DRIVE1_AXIS))
#error "Enabling dual axis drive requires to configure at least one axis with dual drive"
#endif

#if (STEPPER_COUNT > 0 && (DUAL_DRIVE0_STEPPER == 0 || DUAL_DRIVE1_STEPPER == 0))
#error "Stepper  0 cannot be used as a axis drive and a dual axis drive at the same time"
#endif
#if (STEPPER_COUNT > 1 && (DUAL_DRIVE0_STEPPER == 1 || DUAL_DRIVE1_STEPPER == 1))
#error "Stepper  1 cannot be used as a axis drive and a dual axis drive at the same time"
#endif
#if (STEPPER_COUNT > 2 && (DUAL_DRIVE0_STEPPER == 2 || DUAL_DRIVE1_STEPPER == 2))
#error "Stepper  2 cannot be used as a axis drive and a dual axis drive at the same time"
#endif
#if (STEPPER_COUNT > 3 && (DUAL_DRIVE0_STEPPER == 3 || DUAL_DRIVE1_STEPPER == 3))
#error "Stepper  3 cannot be used as a axis drive and a dual axis drive at the same time"
#endif
#if (STEPPER_COUNT > 4 && (DUAL_DRIVE0_STEPPER == 4 || DUAL_DRIVE1_STEPPER == 4))
#error "Stepper  4 cannot be used as a axis drive and a dual axis drive at the same time"
#endif
#if (STEPPER_COUNT > 5 && (DUAL_DRIVE0_STEPPER == 5 || DUAL_DRIVE1_STEPPER == 5))
#error "Stepper  5 cannot be used as a axis drive and a dual axis drive at the same time"
#endif

// dual axis0
#ifdef DUAL_DRIVE0_AXIS
#define AXIS_DUAL0 __axisname__(DUAL_DRIVE0_AXIS)
#define STEP_DUAL0 (1 << AXIS_DUAL0)
#define LIMIT_DUAL0 __limitname__(DUAL_DRIVE0_AXIS)
#define STEP_DUAL0_MASK (1 << DUAL_DRIVE0_STEPPER)
#endif

// dual axis1
#ifdef DUAL_DRIVE1_AXIS
#define AXIS_DUAL1 __axisname__(DUAL_DRIVE1_AXIS)
#define STEP_DUAL1 (1 << AXIS_DUAL1)
#define LIMIT_DUAL1 __limitname__(DUAL_DRIVE1_AXIS)
#define STEP_DUAL1_MASK (1 << DUAL_DRIVE1_STEPPER)
#endif
#endif

#ifndef LIMIT_DUAL0
#define LIMIT_DUAL0 0
#endif
#ifndef LIMIT_DUAL1
#define LIMIT_DUAL1 0
#endif

#define LIMITS_DUAL (LIMIT_DUAL0 | LIMIT_DUAL1)

#if (STEP0_MASK == STEP_DUAL0)
#define STEP0_ITP_MASK (STEP0_MASK | STEP_DUAL0_MASK)
#elif (STEP0_MASK == STEP_DUAL1)
#define STEP0_ITP_MASK (STEP0_MASK | STEP_DUAL1_MASK)
#else
#define STEP0_ITP_MASK STEP0_MASK
#endif
#if (STEP1_MASK == STEP_DUAL0)
#define STEP1_ITP_MASK (STEP1_MASK | STEP_DUAL0_MASK)
#elif (STEP1_MASK == STEP_DUAL1)
#define STEP1_ITP_MASK (STEP1_MASK | STEP_DUAL1_MASK)
#else
#define STEP1_ITP_MASK STEP0_MASK
#endif
#if (STEP2_MASK == STEP_DUAL0)
#define STEP2_ITP_MASK (STEP2_MASK | STEP_DUAL0_MASK)
#elif (STEP2_MASK == STEP_DUAL1)
#define STEP2_ITP_MASK (STEP2_MASK | STEP_DUAL1_MASK)
#else
#define STEP2_ITP_MASK STEP0_MASK
#endif
#if (STEP3_MASK == STEP_DUAL0)
#define STEP3_ITP_MASK (STEP3_MASK | STEP_DUAL0_MASK)
#elif (STEP3_MASK == STEP_DUAL1)
#define STEP3_ITP_MASK (STEP3_MASK | STEP_DUAL1_MASK)
#else
#define STEP3_ITP_MASK STEP0_MASK
#endif
#if (STEP4_MASK == STEP_DUAL0)
#define STEP4_ITP_MASK (STEP4_MASK | STEP_DUAL0_MASK)
#elif (STEP4_MASK == STEP_DUAL1)
#define STEP4_ITP_MASK (STEP4_MASK | STEP_DUAL1_MASK)
#else
#define STEP4_ITP_MASK STEP0_MASK
#endif
#if (STEP5_MASK == STEP_DUAL0)
#define STEP5_ITP_MASK (STEP5_MASK | STEP_DUAL0_MASK)
#elif (STEP5_MASK == STEP_DUAL1)
#define STEP5_ITP_MASK (STEP5_MASK | STEP_DUAL1_MASK)
#else
#define STEP5_ITP_MASK STEP0_MASK
#endif

#ifndef STEP_DUAL0
#define STEP_DUAL0 -1
#endif

#ifndef STEP_DUAL1
#define STEP_DUAL1 -1
#endif

#if (STEPPER_COUNT < 1 && DUAL_DRIVE0_STEPPER != 0 && STEPDUAL_DRIVE1_STEPPER_DUAL1 != 0)
#ifdef STEP0
#undef STEP0
#define STEP0 -1
#endif
#ifdef DIR0
#undef DIR0
#define DIR0 -1
#endif
#endif
#if (STEPPER_COUNT < 2 && DUAL_DRIVE0_STEPPER != 1 && STEPDUAL_DRIVE1_STEPPER_DUAL1 != 1)
#ifdef STEP1
#undef STEP1
#define STEP1 -1
#endif
#ifdef DIR1
#undef DIR1
#define DIR1 -1
#endif
#endif
#if (STEPPER_COUNT < 3 && DUAL_DRIVE0_STEPPER != 2 && STEPDUAL_DRIVE1_STEPPER_DUAL1 != 2)
#ifdef STEP2
#undef STEP2
#define STEP2 -1
#endif
#ifdef DIR2
#undef DIR2
#define DIR2 -1
#endif
#endif
#if (STEPPER_COUNT < 4 && DUAL_DRIVE0_STEPPER != 3 && STEPDUAL_DRIVE1_STEPPER_DUAL1 != 3)
#ifdef STEP3
#undef STEP3
#define STEP3 -1
#endif
#ifdef DIR3
#undef DIR3
#define DIR3 -1
#endif
#endif
#if (STEPPER_COUNT < 5 && DUAL_DRIVE0_STEPPER != 4 && STEPDUAL_DRIVE1_STEPPER_DUAL1 != 4)
#ifdef STEP4
#undef STEP4
#define STEP4 -1
#endif
#ifdef DIR4
#undef DIR4
#define DIR4 -1
#endif
#endif
#if (STEPPER_COUNT < 6 && DUAL_DRIVE0_STEPPER != 5 && STEPDUAL_DRIVE1_STEPPER_DUAL1 != 5)
#ifdef STEP5
#undef STEP5
#define STEP5 -1
#endif
#ifdef DIR5
#undef DIR5
#define DIR5 -1
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
