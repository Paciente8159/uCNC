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
#ifndef SERVO0
#define SERVO0 -1
#ifdef DIO36
#undef DIO36
#endif
#define DIO36 -1
#endif
#ifndef SERVO1
#define SERVO1 -1
#ifdef DIO37
#undef DIO37
#endif
#define DIO37 -1
#endif
#ifndef SERVO2
#define SERVO2 -1
#ifdef DIO38
#undef DIO38
#endif
#define DIO38 -1
#endif
#ifndef SERVO3
#define SERVO3 -1
#ifdef DIO39
#undef DIO39
#endif
#define DIO39 -1
#endif
#ifndef SERVO4
#define SERVO4 -1
#ifdef DIO40
#undef DIO40
#endif
#define DIO40 -1
#endif
#ifndef SERVO5
#define SERVO5 -1
#ifdef DIO41
#undef DIO41
#endif
#define DIO41 -1
#endif
#ifndef DOUT0
#define DOUT0 -1
#ifdef DIO42
#undef DIO42
#endif
#define DIO42 -1
#endif
#ifndef DOUT1
#define DOUT1 -1
#ifdef DIO43
#undef DIO43
#endif
#define DIO43 -1
#endif
#ifndef DOUT2
#define DOUT2 -1
#ifdef DIO44
#undef DIO44
#endif
#define DIO44 -1
#endif
#ifndef DOUT3
#define DOUT3 -1
#ifdef DIO45
#undef DIO45
#endif
#define DIO45 -1
#endif
#ifndef DOUT4
#define DOUT4 -1
#ifdef DIO46
#undef DIO46
#endif
#define DIO46 -1
#endif
#ifndef DOUT5
#define DOUT5 -1
#ifdef DIO47
#undef DIO47
#endif
#define DIO47 -1
#endif
#ifndef DOUT6
#define DOUT6 -1
#ifdef DIO48
#undef DIO48
#endif
#define DIO48 -1
#endif
#ifndef DOUT7
#define DOUT7 -1
#ifdef DIO49
#undef DIO49
#endif
#define DIO49 -1
#endif
#ifndef DOUT8
#define DOUT8 -1
#ifdef DIO50
#undef DIO50
#endif
#define DIO50 -1
#endif
#ifndef DOUT9
#define DOUT9 -1
#ifdef DIO51
#undef DIO51
#endif
#define DIO51 -1
#endif
#ifndef DOUT10
#define DOUT10 -1
#ifdef DIO52
#undef DIO52
#endif
#define DIO52 -1
#endif
#ifndef DOUT11
#define DOUT11 -1
#ifdef DIO53
#undef DIO53
#endif
#define DIO53 -1
#endif
#ifndef DOUT12
#define DOUT12 -1
#ifdef DIO54
#undef DIO54
#endif
#define DIO54 -1
#endif
#ifndef DOUT13
#define DOUT13 -1
#ifdef DIO55
#undef DIO55
#endif
#define DIO55 -1
#endif
#ifndef DOUT14
#define DOUT14 -1
#ifdef DIO56
#undef DIO56
#endif
#define DIO56 -1
#endif
#ifndef DOUT15
#define DOUT15 -1
#ifdef DIO57
#undef DIO57
#endif
#define DIO57 -1
#endif
#ifndef DOUT16
#define DOUT16 -1
#ifdef DIO58
#undef DIO58
#endif
#define DIO58 -1
#endif
#ifndef DOUT17
#define DOUT17 -1
#ifdef DIO59
#undef DIO59
#endif
#define DIO59 -1
#endif
#ifndef DOUT18
#define DOUT18 -1
#ifdef DIO60
#undef DIO60
#endif
#define DIO60 -1
#endif
#ifndef DOUT19
#define DOUT19 -1
#ifdef DIO61
#undef DIO61
#endif
#define DIO61 -1
#endif
#ifndef DOUT20
#define DOUT20 -1
#ifdef DIO62
#undef DIO62
#endif
#define DIO62 -1
#endif
#ifndef DOUT21
#define DOUT21 -1
#ifdef DIO63
#undef DIO63
#endif
#define DIO63 -1
#endif
#ifndef DOUT22
#define DOUT22 -1
#ifdef DIO64
#undef DIO64
#endif
#define DIO64 -1
#endif
#ifndef DOUT23
#define DOUT23 -1
#ifdef DIO65
#undef DIO65
#endif
#define DIO65 -1
#endif
#ifndef DOUT24
#define DOUT24 -1
#ifdef DIO66
#undef DIO66
#endif
#define DIO66 -1
#endif
#ifndef DOUT25
#define DOUT25 -1
#ifdef DIO67
#undef DIO67
#endif
#define DIO67 -1
#endif
#ifndef DOUT26
#define DOUT26 -1
#ifdef DIO68
#undef DIO68
#endif
#define DIO68 -1
#endif
#ifndef DOUT27
#define DOUT27 -1
#ifdef DIO69
#undef DIO69
#endif
#define DIO69 -1
#endif
#ifndef DOUT28
#define DOUT28 -1
#ifdef DIO70
#undef DIO70
#endif
#define DIO70 -1
#endif
#ifndef DOUT29
#define DOUT29 -1
#ifdef DIO71
#undef DIO71
#endif
#define DIO71 -1
#endif
#ifndef DOUT30
#define DOUT30 -1
#ifdef DIO72
#undef DIO72
#endif
#define DIO72 -1
#endif
#ifndef DOUT31
#define DOUT31 -1
#ifdef DIO73
#undef DIO73
#endif
#define DIO73 -1
#endif
#ifndef LIMIT_X
#define LIMIT_X -1
#ifdef DIO100
#undef DIO100
#endif
#define DIO100 -1
#endif
#ifndef LIMIT_Y
#define LIMIT_Y -1
#ifdef DIO101
#undef DIO101
#endif
#define DIO101 -1
#endif
#ifndef LIMIT_Z
#define LIMIT_Z -1
#ifdef DIO102
#undef DIO102
#endif
#define DIO102 -1
#endif
#ifndef LIMIT_X2
#define LIMIT_X2 -1
#ifdef DIO103
#undef DIO103
#endif
#define DIO103 -1
#endif
#ifndef LIMIT_Y2
#define LIMIT_Y2 -1
#ifdef DIO104
#undef DIO104
#endif
#define DIO104 -1
#endif
#ifndef LIMIT_Z2
#define LIMIT_Z2 -1
#ifdef DIO105
#undef DIO105
#endif
#define DIO105 -1
#endif
#ifndef LIMIT_A
#define LIMIT_A -1
#ifdef DIO106
#undef DIO106
#endif
#define DIO106 -1
#endif
#ifndef LIMIT_B
#define LIMIT_B -1
#ifdef DIO107
#undef DIO107
#endif
#define DIO107 -1
#endif
#ifndef LIMIT_C
#define LIMIT_C -1
#ifdef DIO108
#undef DIO108
#endif
#define DIO108 -1
#endif
#ifndef PROBE
#define PROBE -1
#ifdef DIO109
#undef DIO109
#endif
#define DIO109 -1
#endif
#ifndef ESTOP
#define ESTOP -1
#ifdef DIO110
#undef DIO110
#endif
#define DIO110 -1
#endif
#ifndef SAFETY_DOOR
#define SAFETY_DOOR -1
#ifdef DIO111
#undef DIO111
#endif
#define DIO111 -1
#endif
#ifndef FHOLD
#define FHOLD -1
#ifdef DIO112
#undef DIO112
#endif
#define DIO112 -1
#endif
#ifndef CS_RES
#define CS_RES -1
#ifdef DIO113
#undef DIO113
#endif
#define DIO113 -1
#endif
#ifndef ANALOG0
#define ANALOG0 -1
#ifdef DIO114
#undef DIO114
#endif
#define DIO114 -1
#endif
#ifndef ANALOG1
#define ANALOG1 -1
#ifdef DIO115
#undef DIO115
#endif
#define DIO115 -1
#endif
#ifndef ANALOG2
#define ANALOG2 -1
#ifdef DIO116
#undef DIO116
#endif
#define DIO116 -1
#endif
#ifndef ANALOG3
#define ANALOG3 -1
#ifdef DIO117
#undef DIO117
#endif
#define DIO117 -1
#endif
#ifndef ANALOG4
#define ANALOG4 -1
#ifdef DIO118
#undef DIO118
#endif
#define DIO118 -1
#endif
#ifndef ANALOG5
#define ANALOG5 -1
#ifdef DIO119
#undef DIO119
#endif
#define DIO119 -1
#endif
#ifndef ANALOG6
#define ANALOG6 -1
#ifdef DIO120
#undef DIO120
#endif
#define DIO120 -1
#endif
#ifndef ANALOG7
#define ANALOG7 -1
#ifdef DIO121
#undef DIO121
#endif
#define DIO121 -1
#endif
#ifndef ANALOG8
#define ANALOG8 -1
#ifdef DIO122
#undef DIO122
#endif
#define DIO122 -1
#endif
#ifndef ANALOG9
#define ANALOG9 -1
#ifdef DIO123
#undef DIO123
#endif
#define DIO123 -1
#endif
#ifndef ANALOG10
#define ANALOG10 -1
#ifdef DIO124
#undef DIO124
#endif
#define DIO124 -1
#endif
#ifndef ANALOG11
#define ANALOG11 -1
#ifdef DIO125
#undef DIO125
#endif
#define DIO125 -1
#endif
#ifndef ANALOG12
#define ANALOG12 -1
#ifdef DIO126
#undef DIO126
#endif
#define DIO126 -1
#endif
#ifndef ANALOG13
#define ANALOG13 -1
#ifdef DIO127
#undef DIO127
#endif
#define DIO127 -1
#endif
#ifndef ANALOG14
#define ANALOG14 -1
#ifdef DIO128
#undef DIO128
#endif
#define DIO128 -1
#endif
#ifndef ANALOG15
#define ANALOG15 -1
#ifdef DIO129
#undef DIO129
#endif
#define DIO129 -1
#endif
#ifndef DIN0
#define DIN0 -1
#ifdef DIO130
#undef DIO130
#endif
#define DIO130 -1
#endif
#ifndef DIN1
#define DIN1 -1
#ifdef DIO131
#undef DIO131
#endif
#define DIO131 -1
#endif
#ifndef DIN2
#define DIN2 -1
#ifdef DIO132
#undef DIO132
#endif
#define DIO132 -1
#endif
#ifndef DIN3
#define DIN3 -1
#ifdef DIO133
#undef DIO133
#endif
#define DIO133 -1
#endif
#ifndef DIN4
#define DIN4 -1
#ifdef DIO134
#undef DIO134
#endif
#define DIO134 -1
#endif
#ifndef DIN5
#define DIN5 -1
#ifdef DIO135
#undef DIO135
#endif
#define DIO135 -1
#endif
#ifndef DIN6
#define DIN6 -1
#ifdef DIO136
#undef DIO136
#endif
#define DIO136 -1
#endif
#ifndef DIN7
#define DIN7 -1
#ifdef DIO137
#undef DIO137
#endif
#define DIO137 -1
#endif
#ifndef DIN8
#define DIN8 -1
#ifdef DIO138
#undef DIO138
#endif
#define DIO138 -1
#endif
#ifndef DIN9
#define DIN9 -1
#ifdef DIO139
#undef DIO139
#endif
#define DIO139 -1
#endif
#ifndef DIN10
#define DIN10 -1
#ifdef DIO140
#undef DIO140
#endif
#define DIO140 -1
#endif
#ifndef DIN11
#define DIN11 -1
#ifdef DIO141
#undef DIO141
#endif
#define DIO141 -1
#endif
#ifndef DIN12
#define DIN12 -1
#ifdef DIO142
#undef DIO142
#endif
#define DIO142 -1
#endif
#ifndef DIN13
#define DIN13 -1
#ifdef DIO143
#undef DIO143
#endif
#define DIO143 -1
#endif
#ifndef DIN14
#define DIN14 -1
#ifdef DIO144
#undef DIO144
#endif
#define DIO144 -1
#endif
#ifndef DIN15
#define DIN15 -1
#ifdef DIO145
#undef DIO145
#endif
#define DIO145 -1
#endif
#ifndef DIN16
#define DIN16 -1
#ifdef DIO146
#undef DIO146
#endif
#define DIO146 -1
#endif
#ifndef DIN17
#define DIN17 -1
#ifdef DIO147
#undef DIO147
#endif
#define DIO147 -1
#endif
#ifndef DIN18
#define DIN18 -1
#ifdef DIO148
#undef DIO148
#endif
#define DIO148 -1
#endif
#ifndef DIN19
#define DIN19 -1
#ifdef DIO149
#undef DIO149
#endif
#define DIO149 -1
#endif
#ifndef DIN20
#define DIN20 -1
#ifdef DIO150
#undef DIO150
#endif
#define DIO150 -1
#endif
#ifndef DIN21
#define DIN21 -1
#ifdef DIO151
#undef DIO151
#endif
#define DIO151 -1
#endif
#ifndef DIN22
#define DIN22 -1
#ifdef DIO152
#undef DIO152
#endif
#define DIO152 -1
#endif
#ifndef DIN23
#define DIN23 -1
#ifdef DIO153
#undef DIO153
#endif
#define DIO153 -1
#endif
#ifndef DIN24
#define DIN24 -1
#ifdef DIO154
#undef DIO154
#endif
#define DIO154 -1
#endif
#ifndef DIN25
#define DIN25 -1
#ifdef DIO155
#undef DIO155
#endif
#define DIO155 -1
#endif
#ifndef DIN26
#define DIN26 -1
#ifdef DIO156
#undef DIO156
#endif
#define DIO156 -1
#endif
#ifndef DIN27
#define DIN27 -1
#ifdef DIO157
#undef DIO157
#endif
#define DIO157 -1
#endif
#ifndef DIN28
#define DIN28 -1
#ifdef DIO158
#undef DIO158
#endif
#define DIO158 -1
#endif
#ifndef DIN29
#define DIN29 -1
#ifdef DIO159
#undef DIO159
#endif
#define DIO159 -1
#endif
#ifndef DIN30
#define DIN30 -1
#ifdef DIO160
#undef DIO160
#endif
#define DIO160 -1
#endif
#ifndef DIN31
#define DIN31 -1
#ifdef DIO161
#undef DIO161
#endif
#define DIO161 -1
#endif
#ifndef TX
#define TX -1
#ifdef DIO200
#undef DIO200
#endif
#define DIO200 -1
#endif
#ifndef RX
#define RX -1
#ifdef DIO201
#undef DIO201
#endif
#define DIO201 -1
#endif
#ifndef USB_DM
#define USB_DM -1
#ifdef DIO202
#undef DIO202
#endif
#define DIO202 -1
#endif
#ifndef USB_DP
#define USB_DP -1
#ifdef DIO203
#undef DIO203
#endif
#define DIO203 -1
#endif
#ifndef SPI_CLK
#define SPI_CLK -1
#ifdef DIO204
#undef DIO204
#endif
#define DIO204 -1
#endif
#ifndef SPI_SDI
#define SPI_SDI -1
#ifdef DIO205
#undef DIO205
#endif
#define DIO205 -1
#endif
#ifndef SPI_SDO
#define SPI_SDO -1
#ifdef DIO206
#undef DIO206
#endif
#define DIO206 -1
#endif

// if the pins are undefined turn on option
#if (ESTOP < 0 && SAFETY_DOOR < 0 && FHOLD < 0 && CS_RES < 0 && !defined(DISABLE_ALL_CONTROLS))
#define DISABLE_ALL_CONTROLS
#endif

#if (LIMIT_X < 0 && LIMIT_X2 < 0 && LIMIT_Y < 0 && LIMIT_Y2 < 0 && LIMIT_Z < 0 && LIMIT_Z2 < 0 && LIMIT_A < 0 && LIMIT_B < 0 && LIMIT_C < 0 && !defined(DISABLE_ALL_LIMITS))
#define DISABLE_ALL_LIMITS
#endif

#if (PROBE < 0 && !defined(DISABLE_PROBE))
#define DISABLE_PROBE
#endif

#if SERVO0 >= 0
#define SERVO0_MASK (1U << 0)
#define SERVO0_FRAME 0
#else
#define SERVO0_MASK 0
#endif
#if SERVO1 >= 0
#define SERVO1_MASK (1U << 1)
#define SERVO1_FRAME 3
#else
#define SERVO1_MASK 0
#endif
#if SERVO2 >= 0
#define SERVO2_MASK (1U << 2)
#define SERVO2_FRAME 6
#else
#define SERVO2_MASK 0
#endif
#if SERVO3 >= 0
#define SERVO3_MASK (1U << 3)
#define SERVO3_FRAME 9
#else
#define SERVO3_MASK 0
#endif
#if SERVO4 >= 0
#define SERVO4_MASK (1U << 4)
#define SERVO4_FRAME 12
#else
#define SERVO4_MASK 0
#endif
#if SERVO5 >= 0
#define SERVO5_MASK (1U << 5)
#define SERVO5_FRAME 15
#else
#define SERVO5_MASK 0
#endif

#define SERVOS_MASK (SERVO0_MASK | SERVO1_MASK | SERVO2_MASK | SERVO3_MASK | SERVO4_MASK | SERVO5_MASK)

#if (INTERFACE < 0 || INTERFACE > 1)
#error "undefined COM interface"
#endif

#ifdef BRESENHAM_16BIT
#if (DSS_MAX_OVERSAMPLING < 0 || DSS_MAX_OVERSAMPLING > 3)
#error DSS_MAX_OVERSAMPLING invalid value! Should be set between 0 and 3
#endif
#else
#if (DSS_MAX_OVERSAMPLING < 0 || DSS_MAX_OVERSAMPLING > 5)
#error DSS_MAX_OVERSAMPLING invalid value! Should be set between 0 and 5
#endif
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
#define MAX_STEPS_PER_LINE (1UL << MAX_STEPS_PER_LINE_BITS)

#if DSS_CUTOFF_FREQ > (F_STEP_MAX >> 3)
#error "DSS_CUTOFF_FREQ should not be set above 1/8th of the max step rate"
#endif

#ifdef ENABLE_S_CURVE_ACCELERATION
#ifdef USE_LEGACY_STEP_INTERPOLATOR
#undef USE_LEGACY_STEP_INTERPOLATOR
#endif
#endif

#if (KINEMATIC == KINEMATIC_DELTA)
#if (MCU == MCU_AVR && BOARD != BOARD_UNO && BOARD != BOARD_MKS_DLC)
#define PLANNER_BUFFER_SIZE 40
#define INTERPOLATOR_FREQ 1
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
