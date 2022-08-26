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
#define F_STEP_MAX 100000UL
#endif
#ifndef F_STEP_MIN
#define F_STEP_MIN 1
#endif

// defines special mcu to access flash strings and arrays
#define __rom__
#define __romstr__
#define rom_strptr *
#define rom_strcpy strcpy
#define rom_strncpy strncpy
#define rom_memcpy memcpy
#define rom_read_byte *

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

#define __gpioreg__(X) (__helper__(PERIPHS_IO_MUX_GPIO, X, ))

// IO pins
#if(defined(STEP0_BIT))
#define STEP0 0
#define DIO0 0
#define DIO0_BIT STEP0_BIT
#endif
#if(defined(STEP1_BIT))
#define STEP1 1
#define DIO1 1
#define DIO1_BIT STEP1_BIT
#endif
#if(defined(STEP2_BIT))
#define STEP2 2
#define DIO2 2
#define DIO2_BIT STEP2_BIT
#endif
#if(defined(STEP3_BIT))
#define STEP3 3
#define DIO3 3
#define DIO3_BIT STEP3_BIT
#endif
#if(defined(STEP4_BIT))
#define STEP4 4
#define DIO4 4
#define DIO4_BIT STEP4_BIT
#endif
#if(defined(STEP5_BIT))
#define STEP5 5
#define DIO5 5
#define DIO5_BIT STEP5_BIT
#endif
#if(defined(STEP6_BIT))
#define STEP6 6
#define DIO6 6
#define DIO6_BIT STEP6_BIT
#endif
#if(defined(STEP7_BIT))
#define STEP7 7
#define DIO7 7
#define DIO7_BIT STEP7_BIT
#endif
#if(defined(DIR0_BIT))
#define DIR0 8
#define DIO8 8
#define DIO8_BIT DIR0_BIT
#endif
#if(defined(DIR1_BIT))
#define DIR1 9
#define DIO9 9
#define DIO9_BIT DIR1_BIT
#endif
#if(defined(DIR2_BIT))
#define DIR2 10
#define DIO10 10
#define DIO10_BIT DIR2_BIT
#endif
#if(defined(DIR3_BIT))
#define DIR3 11
#define DIO11 11
#define DIO11_BIT DIR3_BIT
#endif
#if(defined(DIR4_BIT))
#define DIR4 12
#define DIO12 12
#define DIO12_BIT DIR4_BIT
#endif
#if(defined(DIR5_BIT))
#define DIR5 13
#define DIO13 13
#define DIO13_BIT DIR5_BIT
#endif
#if(defined(DIR6_BIT))
#define DIR6 14
#define DIO14 14
#define DIO14_BIT DIR6_BIT
#endif
#if(defined(DIR7_BIT))
#define DIR7 15
#define DIO15 15
#define DIO15_BIT DIR7_BIT
#endif
#if(defined(STEP0_EN_BIT))
#define STEP0_EN 16
#define DIO16 16
#define DIO16_BIT STEP0_EN_BIT
#endif
#if(defined(STEP1_EN_BIT))
#define STEP1_EN 17
#define DIO17 17
#define DIO17_BIT STEP1_EN_BIT
#endif
#if(defined(STEP2_EN_BIT))
#define STEP2_EN 18
#define DIO18 18
#define DIO18_BIT STEP2_EN_BIT
#endif
#if(defined(STEP3_EN_BIT))
#define STEP3_EN 19
#define DIO19 19
#define DIO19_BIT STEP3_EN_BIT
#endif
#if(defined(STEP4_EN_BIT))
#define STEP4_EN 20
#define DIO20 20
#define DIO20_BIT STEP4_EN_BIT
#endif
#if(defined(STEP5_EN_BIT))
#define STEP5_EN 21
#define DIO21 21
#define DIO21_BIT STEP5_EN_BIT
#endif
#if(defined(STEP6_EN_BIT))
#define STEP6_EN 22
#define DIO22 22
#define DIO22_BIT STEP6_EN_BIT
#endif
#if(defined(STEP7_EN_BIT))
#define STEP7_EN 23
#define DIO23 23
#define DIO23_BIT STEP7_EN_BIT
#endif
#if(defined(PWM0_BIT))
#define PWM0 24
#define DIO24 24
#define DIO24_BIT PWM0_BIT
#endif
#if(defined(PWM1_BIT))
#define PWM1 25
#define DIO25 25
#define DIO25_BIT PWM1_BIT
#endif
#if(defined(PWM2_BIT))
#define PWM2 26
#define DIO26 26
#define DIO26_BIT PWM2_BIT
#endif
#if(defined(PWM3_BIT))
#define PWM3 27
#define DIO27 27
#define DIO27_BIT PWM3_BIT
#endif
#if(defined(PWM4_BIT))
#define PWM4 28
#define DIO28 28
#define DIO28_BIT PWM4_BIT
#endif
#if(defined(PWM5_BIT))
#define PWM5 29
#define DIO29 29
#define DIO29_BIT PWM5_BIT
#endif
#if(defined(PWM6_BIT))
#define PWM6 30
#define DIO30 30
#define DIO30_BIT PWM6_BIT
#endif
#if(defined(PWM7_BIT))
#define PWM7 31
#define DIO31 31
#define DIO31_BIT PWM7_BIT
#endif
#if(defined(PWM8_BIT))
#define PWM8 32
#define DIO32 32
#define DIO32_BIT PWM8_BIT
#endif
#if(defined(PWM9_BIT))
#define PWM9 33
#define DIO33 33
#define DIO33_BIT PWM9_BIT
#endif
#if(defined(PWM10_BIT))
#define PWM10 34
#define DIO34 34
#define DIO34_BIT PWM10_BIT
#endif
#if(defined(PWM11_BIT))
#define PWM11 35
#define DIO35 35
#define DIO35_BIT PWM11_BIT
#endif
#if(defined(PWM12_BIT))
#define PWM12 36
#define DIO36 36
#define DIO36_BIT PWM12_BIT
#endif
#if(defined(PWM13_BIT))
#define PWM13 37
#define DIO37 37
#define DIO37_BIT PWM13_BIT
#endif
#if(defined(PWM14_BIT))
#define PWM14 38
#define DIO38 38
#define DIO38_BIT PWM14_BIT
#endif
#if(defined(PWM15_BIT))
#define PWM15 39
#define DIO39 39
#define DIO39_BIT PWM15_BIT
#endif
#if(defined(SERVO0_BIT))
#define SERVO0 40
#define DIO40 40
#define DIO40_BIT SERVO0_BIT
#endif
#if(defined(SERVO1_BIT))
#define SERVO1 41
#define DIO41 41
#define DIO41_BIT SERVO1_BIT
#endif
#if(defined(SERVO2_BIT))
#define SERVO2 42
#define DIO42 42
#define DIO42_BIT SERVO2_BIT
#endif
#if(defined(SERVO3_BIT))
#define SERVO3 43
#define DIO43 43
#define DIO43_BIT SERVO3_BIT
#endif
#if(defined(SERVO4_BIT))
#define SERVO4 44
#define DIO44 44
#define DIO44_BIT SERVO4_BIT
#endif
#if(defined(SERVO5_BIT))
#define SERVO5 45
#define DIO45 45
#define DIO45_BIT SERVO5_BIT
#endif
#if(defined(DOUT0_BIT))
#define DOUT0 46
#define DIO46 46
#define DIO46_BIT DOUT0_BIT
#endif
#if(defined(DOUT1_BIT))
#define DOUT1 47
#define DIO47 47
#define DIO47_BIT DOUT1_BIT
#endif
#if(defined(DOUT2_BIT))
#define DOUT2 48
#define DIO48 48
#define DIO48_BIT DOUT2_BIT
#endif
#if(defined(DOUT3_BIT))
#define DOUT3 49
#define DIO49 49
#define DIO49_BIT DOUT3_BIT
#endif
#if(defined(DOUT4_BIT))
#define DOUT4 50
#define DIO50 50
#define DIO50_BIT DOUT4_BIT
#endif
#if(defined(DOUT5_BIT))
#define DOUT5 51
#define DIO51 51
#define DIO51_BIT DOUT5_BIT
#endif
#if(defined(DOUT6_BIT))
#define DOUT6 52
#define DIO52 52
#define DIO52_BIT DOUT6_BIT
#endif
#if(defined(DOUT7_BIT))
#define DOUT7 53
#define DIO53 53
#define DIO53_BIT DOUT7_BIT
#endif
#if(defined(DOUT8_BIT))
#define DOUT8 54
#define DIO54 54
#define DIO54_BIT DOUT8_BIT
#endif
#if(defined(DOUT9_BIT))
#define DOUT9 55
#define DIO55 55
#define DIO55_BIT DOUT9_BIT
#endif
#if(defined(DOUT10_BIT))
#define DOUT10 56
#define DIO56 56
#define DIO56_BIT DOUT10_BIT
#endif
#if(defined(DOUT11_BIT))
#define DOUT11 57
#define DIO57 57
#define DIO57_BIT DOUT11_BIT
#endif
#if(defined(DOUT12_BIT))
#define DOUT12 58
#define DIO58 58
#define DIO58_BIT DOUT12_BIT
#endif
#if(defined(DOUT13_BIT))
#define DOUT13 59
#define DIO59 59
#define DIO59_BIT DOUT13_BIT
#endif
#if(defined(DOUT14_BIT))
#define DOUT14 60
#define DIO60 60
#define DIO60_BIT DOUT14_BIT
#endif
#if(defined(DOUT15_BIT))
#define DOUT15 61
#define DIO61 61
#define DIO61_BIT DOUT15_BIT
#endif
#if(defined(DOUT16_BIT))
#define DOUT16 62
#define DIO62 62
#define DIO62_BIT DOUT16_BIT
#endif
#if(defined(DOUT17_BIT))
#define DOUT17 63
#define DIO63 63
#define DIO63_BIT DOUT17_BIT
#endif
#if(defined(DOUT18_BIT))
#define DOUT18 64
#define DIO64 64
#define DIO64_BIT DOUT18_BIT
#endif
#if(defined(DOUT19_BIT))
#define DOUT19 65
#define DIO65 65
#define DIO65_BIT DOUT19_BIT
#endif
#if(defined(DOUT20_BIT))
#define DOUT20 66
#define DIO66 66
#define DIO66_BIT DOUT20_BIT
#endif
#if(defined(DOUT21_BIT))
#define DOUT21 67
#define DIO67 67
#define DIO67_BIT DOUT21_BIT
#endif
#if(defined(DOUT22_BIT))
#define DOUT22 68
#define DIO68 68
#define DIO68_BIT DOUT22_BIT
#endif
#if(defined(DOUT23_BIT))
#define DOUT23 69
#define DIO69 69
#define DIO69_BIT DOUT23_BIT
#endif
#if(defined(DOUT24_BIT))
#define DOUT24 70
#define DIO70 70
#define DIO70_BIT DOUT24_BIT
#endif
#if(defined(DOUT25_BIT))
#define DOUT25 71
#define DIO71 71
#define DIO71_BIT DOUT25_BIT
#endif
#if(defined(DOUT26_BIT))
#define DOUT26 72
#define DIO72 72
#define DIO72_BIT DOUT26_BIT
#endif
#if(defined(DOUT27_BIT))
#define DOUT27 73
#define DIO73 73
#define DIO73_BIT DOUT27_BIT
#endif
#if(defined(DOUT28_BIT))
#define DOUT28 74
#define DIO74 74
#define DIO74_BIT DOUT28_BIT
#endif
#if(defined(DOUT29_BIT))
#define DOUT29 75
#define DIO75 75
#define DIO75_BIT DOUT29_BIT
#endif
#if(defined(DOUT30_BIT))
#define DOUT30 76
#define DIO76 76
#define DIO76_BIT DOUT30_BIT
#endif
#if(defined(DOUT31_BIT))
#define DOUT31 77
#define DIO77 77
#define DIO77_BIT DOUT31_BIT
#endif
#if(defined(LIMIT_X_BIT))
#define LIMIT_X 100
#define DIO100 100
#define DIO100_BIT LIMIT_X_BIT
#endif
#if(defined(LIMIT_Y_BIT))
#define LIMIT_Y 101
#define DIO101 101
#define DIO101_BIT LIMIT_Y_BIT
#endif
#if(defined(LIMIT_Z_BIT))
#define LIMIT_Z 102
#define DIO102 102
#define DIO102_BIT LIMIT_Z_BIT
#endif
#if(defined(LIMIT_X2_BIT))
#define LIMIT_X2 103
#define DIO103 103
#define DIO103_BIT LIMIT_X2_BIT
#endif
#if(defined(LIMIT_Y2_BIT))
#define LIMIT_Y2 104
#define DIO104 104
#define DIO104_BIT LIMIT_Y2_BIT
#endif
#if(defined(LIMIT_Z2_BIT))
#define LIMIT_Z2 105
#define DIO105 105
#define DIO105_BIT LIMIT_Z2_BIT
#endif
#if(defined(LIMIT_A_BIT))
#define LIMIT_A 106
#define DIO106 106
#define DIO106_BIT LIMIT_A_BIT
#endif
#if(defined(LIMIT_B_BIT))
#define LIMIT_B 107
#define DIO107 107
#define DIO107_BIT LIMIT_B_BIT
#endif
#if(defined(LIMIT_C_BIT))
#define LIMIT_C 108
#define DIO108 108
#define DIO108_BIT LIMIT_C_BIT
#endif
#if(defined(PROBE_BIT))
#define PROBE 109
#define DIO109 109
#define DIO109_BIT PROBE_BIT
#endif
#if(defined(ESTOP_BIT))
#define ESTOP 110
#define DIO110 110
#define DIO110_BIT ESTOP_BIT
#endif
#if(defined(SAFETY_DOOR_BIT))
#define SAFETY_DOOR 111
#define DIO111 111
#define DIO111_BIT SAFETY_DOOR_BIT
#endif
#if(defined(FHOLD_BIT))
#define FHOLD 112
#define DIO112 112
#define DIO112_BIT FHOLD_BIT
#endif
#if(defined(CS_RES_BIT))
#define CS_RES 113
#define DIO113 113
#define DIO113_BIT CS_RES_BIT
#endif
#if(defined(ANALOG0_BIT))
#define ANALOG0 114
#define DIO114 114
#define DIO114_BIT ANALOG0_BIT
#endif
#if(defined(ANALOG1_BIT))
#define ANALOG1 115
#define DIO115 115
#define DIO115_BIT ANALOG1_BIT
#endif
#if(defined(ANALOG2_BIT))
#define ANALOG2 116
#define DIO116 116
#define DIO116_BIT ANALOG2_BIT
#endif
#if(defined(ANALOG3_BIT))
#define ANALOG3 117
#define DIO117 117
#define DIO117_BIT ANALOG3_BIT
#endif
#if(defined(ANALOG4_BIT))
#define ANALOG4 118
#define DIO118 118
#define DIO118_BIT ANALOG4_BIT
#endif
#if(defined(ANALOG5_BIT))
#define ANALOG5 119
#define DIO119 119
#define DIO119_BIT ANALOG5_BIT
#endif
#if(defined(ANALOG6_BIT))
#define ANALOG6 120
#define DIO120 120
#define DIO120_BIT ANALOG6_BIT
#endif
#if(defined(ANALOG7_BIT))
#define ANALOG7 121
#define DIO121 121
#define DIO121_BIT ANALOG7_BIT
#endif
#if(defined(ANALOG8_BIT))
#define ANALOG8 122
#define DIO122 122
#define DIO122_BIT ANALOG8_BIT
#endif
#if(defined(ANALOG9_BIT))
#define ANALOG9 123
#define DIO123 123
#define DIO123_BIT ANALOG9_BIT
#endif
#if(defined(ANALOG10_BIT))
#define ANALOG10 124
#define DIO124 124
#define DIO124_BIT ANALOG10_BIT
#endif
#if(defined(ANALOG11_BIT))
#define ANALOG11 125
#define DIO125 125
#define DIO125_BIT ANALOG11_BIT
#endif
#if(defined(ANALOG12_BIT))
#define ANALOG12 126
#define DIO126 126
#define DIO126_BIT ANALOG12_BIT
#endif
#if(defined(ANALOG13_BIT))
#define ANALOG13 127
#define DIO127 127
#define DIO127_BIT ANALOG13_BIT
#endif
#if(defined(ANALOG14_BIT))
#define ANALOG14 128
#define DIO128 128
#define DIO128_BIT ANALOG14_BIT
#endif
#if(defined(ANALOG15_BIT))
#define ANALOG15 129
#define DIO129 129
#define DIO129_BIT ANALOG15_BIT
#endif
#if(defined(DIN0_BIT))
#define DIN0 130
#define DIO130 130
#define DIO130_BIT DIN0_BIT
#endif
#if(defined(DIN1_BIT))
#define DIN1 131
#define DIO131 131
#define DIO131_BIT DIN1_BIT
#endif
#if(defined(DIN2_BIT))
#define DIN2 132
#define DIO132 132
#define DIO132_BIT DIN2_BIT
#endif
#if(defined(DIN3_BIT))
#define DIN3 133
#define DIO133 133
#define DIO133_BIT DIN3_BIT
#endif
#if(defined(DIN4_BIT))
#define DIN4 134
#define DIO134 134
#define DIO134_BIT DIN4_BIT
#endif
#if(defined(DIN5_BIT))
#define DIN5 135
#define DIO135 135
#define DIO135_BIT DIN5_BIT
#endif
#if(defined(DIN6_BIT))
#define DIN6 136
#define DIO136 136
#define DIO136_BIT DIN6_BIT
#endif
#if(defined(DIN7_BIT))
#define DIN7 137
#define DIO137 137
#define DIO137_BIT DIN7_BIT
#endif
#if(defined(DIN8_BIT))
#define DIN8 138
#define DIO138 138
#define DIO138_BIT DIN8_BIT
#endif
#if(defined(DIN9_BIT))
#define DIN9 139
#define DIO139 139
#define DIO139_BIT DIN9_BIT
#endif
#if(defined(DIN10_BIT))
#define DIN10 140
#define DIO140 140
#define DIO140_BIT DIN10_BIT
#endif
#if(defined(DIN11_BIT))
#define DIN11 141
#define DIO141 141
#define DIO141_BIT DIN11_BIT
#endif
#if(defined(DIN12_BIT))
#define DIN12 142
#define DIO142 142
#define DIO142_BIT DIN12_BIT
#endif
#if(defined(DIN13_BIT))
#define DIN13 143
#define DIO143 143
#define DIO143_BIT DIN13_BIT
#endif
#if(defined(DIN14_BIT))
#define DIN14 144
#define DIO144 144
#define DIO144_BIT DIN14_BIT
#endif
#if(defined(DIN15_BIT))
#define DIN15 145
#define DIO145 145
#define DIO145_BIT DIN15_BIT
#endif
#if(defined(DIN16_BIT))
#define DIN16 146
#define DIO146 146
#define DIO146_BIT DIN16_BIT
#endif
#if(defined(DIN17_BIT))
#define DIN17 147
#define DIO147 147
#define DIO147_BIT DIN17_BIT
#endif
#if(defined(DIN18_BIT))
#define DIN18 148
#define DIO148 148
#define DIO148_BIT DIN18_BIT
#endif
#if(defined(DIN19_BIT))
#define DIN19 149
#define DIO149 149
#define DIO149_BIT DIN19_BIT
#endif
#if(defined(DIN20_BIT))
#define DIN20 150
#define DIO150 150
#define DIO150_BIT DIN20_BIT
#endif
#if(defined(DIN21_BIT))
#define DIN21 151
#define DIO151 151
#define DIO151_BIT DIN21_BIT
#endif
#if(defined(DIN22_BIT))
#define DIN22 152
#define DIO152 152
#define DIO152_BIT DIN22_BIT
#endif
#if(defined(DIN23_BIT))
#define DIN23 153
#define DIO153 153
#define DIO153_BIT DIN23_BIT
#endif
#if(defined(DIN24_BIT))
#define DIN24 154
#define DIO154 154
#define DIO154_BIT DIN24_BIT
#endif
#if(defined(DIN25_BIT))
#define DIN25 155
#define DIO155 155
#define DIO155_BIT DIN25_BIT
#endif
#if(defined(DIN26_BIT))
#define DIN26 156
#define DIO156 156
#define DIO156_BIT DIN26_BIT
#endif
#if(defined(DIN27_BIT))
#define DIN27 157
#define DIO157 157
#define DIO157_BIT DIN27_BIT
#endif
#if(defined(DIN28_BIT))
#define DIN28 158
#define DIO158 158
#define DIO158_BIT DIN28_BIT
#endif
#if(defined(DIN29_BIT))
#define DIN29 159
#define DIO159 159
#define DIO159_BIT DIN29_BIT
#endif
#if(defined(DIN30_BIT))
#define DIN30 160
#define DIO160 160
#define DIO160_BIT DIN30_BIT
#endif
#if(defined(DIN31_BIT))
#define DIN31 161
#define DIO161 161
#define DIO161_BIT DIN31_BIT
#endif
#if(defined(TX_BIT))
#define TX 200
#define DIO200 200
#define DIO200_BIT TX_BIT
#endif
#if(defined(RX_BIT))
#define RX 201
#define DIO201 201
#define DIO201_BIT RX_BIT
#endif
#if(defined(USB_DM_BIT))
#define USB_DM 202
#define DIO202 202
#define DIO202_BIT USB_DM_BIT
#endif
#if(defined(USB_DP_BIT))
#define USB_DP 203
#define DIO203 203
#define DIO203_BIT USB_DP_BIT
#endif
#if(defined(SPI_CLK_BIT))
#define SPI_CLK 204
#define DIO204 204
#define DIO204_BIT SPI_CLK_BIT
#endif
#if(defined(SPI_SDI_BIT))
#define SPI_SDI 205
#define DIO205 205
#define DIO205_BIT SPI_SDI_BIT
#endif
#if(defined(SPI_SDO_BIT))
#define SPI_SDO 206
#define DIO206 206
#define DIO206_BIT SPI_SDO_BIT
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
#define DIO89_ISRCALLBACK __indirect__(X, ISRCALLBACK)
#endif

#if (INTERFACE == INTERFACE_UART)
#ifndef COM_PORT
#define COM_PORT 0
#endif
#endif

#define ENABLE_SYNC_RX
#define ENABLE_SYNC_TX

#ifndef RTC_TIMER
#define RTC_TIMER 0
#endif
#define RTC_TIMER_TG (RTC_TIMER & 0x01)
#define RTC_TIMER_IDX ((RTC_TIMER >> 1) & 0x01)

#ifndef ITP_TIMER
#define ITP_TIMER 1
#endif
#define ITP_TIMER_TG (ITP_TIMER & 0x01)
#define ITP_TIMER_IDX ((ITP_TIMER >> 1) & 0x01)

// SPI
#if (defined(SPI_CLK) && defined(SPI_SDI) && defined(SPI_SDO))
#define MCU_HAS_SPI
#ifndef SPI_MODE
#define SPI_MODE 0
#endif
#ifndef SPI_FREQ
#define SPI_FREQ 1000000UL
#endif
#endif

// I2C
#if (defined(I2C_SCL) && defined(I2C_SDA))
#define MCU_HAS_I2C
#ifndef I2C_PORT
#define I2C_PORT 0
#endif
#ifndef I2C_FREQ
#define I2C_FREQ 1000000UL
#endif
#endif

// Helper macros
#define __helper_ex__(left, mid, right) (left##mid##right)
#define __helper__(left, mid, right) (__helper_ex__(left, mid, right))
#define __indirect__ex__(X, Y) DIO##X##_##Y
#define __indirect__(X, Y) __indirect__ex__(X, Y)

#define mcu_config_output(X) pinMode(__indirect__(X, BIT), OUTPUT)
#define mcu_config_pwm(X) pinMode(__indirect__(X, BIT), OUTPUT)
#define mcu_config_input(X) pinMode(__indirect__(X, BIT), INPUT)
#define mcu_config_pullup(X) pinMode(__indirect__(X, BIT), INPUT_PULLUP)
#define mcu_config_input_isr(X) attachInterrupt(digitalPinToInterrupt(__indirect__(X, BIT)), __indirect__(X, ISRCALLBACK), CHANGE)

#define mcu_get_input(X) digitalRead(__indirect__(X, BIT))
#define mcu_get_output(X) digitalRead(__indirect__(X, BIT))
#define mcu_set_output(X) digitalWrite(__indirect__(X, BIT), 1)
#define mcu_clear_output(X) digitalWrite(__indirect__(X, BIT), 0)
#define mcu_toggle_output(X) digitalWrite(__indirect__(X, BIT), !digitalRead(__indirect__(X, BIT)))

	extern uint8_t esp32_pwm[16];
#define mcu_set_pwm(X, Y) (esp32_pwm[X - PWM_PINS_OFFSET] = (0x7F & (Y >> 1)))
#define mcu_get_pwm(X) (esp32_pwm[X - PWM_PINS_OFFSET] << 1)
#define mcu_get_analog(X) (analogRead(__indirect__(X, BIT)) >> 2)

#define mcu_spi_xmit(X) esp32_spi_xmit(X)

#ifdef __cplusplus
}
#endif

#endif
