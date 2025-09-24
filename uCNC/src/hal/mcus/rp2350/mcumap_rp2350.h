/*
	Name: mcumap_rp2350.h
	Description: Contains all MCU and PIN definitions for RP2350 to run µCNC.

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

#ifndef MCUMAP_RP2350_H
#define MCUMAP_RP2350_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>
#include <Arduino.h>
#include <hardware/timer.h>
#include <hardware/irq.h>
#include <pico/multicore.h>

/*
	Generates all the interface definitions.
	This creates a middle HAL layer between the board IO pins and the AVR funtionalities
*/
/*
	MCU specific definitions and replacements
*/

/*
	RP2350 Defaults
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

#ifndef RP2350
#define RP2350
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
	// needed by software delays
#ifndef MCU_CYCLES_PER_LOOP
#define MCU_CYCLES_PER_LOOP 3
#endif
#ifndef MCU_CYCLES_LOOP_OVERHEAD
#define MCU_CYCLES_LOOP_OVERHEAD 1
#endif

#define mcu_delay_loop(X)                                      \
    do {                                                       \
        register uint32_t __cnt __asm__("r0") = (X);           \
        __asm__ __volatile__(                                  \
            "1: subs %0, %0, #1\n\t" /* 1 cycle */             \
            "bne 1b\n\t"        /* 2 cycles taken, 1 not */    \
            "nop\n\t"           /* pad exit to 3 cycles */     \
            : "+r"(__cnt)                                     \
            :                                                  \
            : "cc");                                           \
    } while (0)

#ifdef RX_BUFFER_CAPACITY
#define RX_BUFFER_CAPACITY 255
#endif

#define __SIZEOF_FLOAT__ 4

// used by the parser
// this method is faster then normal multiplication (for 32 bit for 16 and 8 bits is slightly lower)
// overrides utils.h definition to implement this method with or without fast math option enabled
#define fast_int_mul10(x) ((((x) << 2) + (x)) << 1)

// IO pins
#if (defined(STEP0_BIT))
#define DIO1 1
#define STEP0 1
#define DIO1_BIT (STEP0_BIT)
#endif
#if (defined(STEP1_BIT))
#define DIO2 2
#define STEP1 2
#define DIO2_BIT (STEP1_BIT)
#endif
#if (defined(STEP2_BIT))
#define DIO3 3
#define STEP2 3
#define DIO3_BIT (STEP2_BIT)
#endif
#if (defined(STEP3_BIT))
#define DIO4 4
#define STEP3 4
#define DIO4_BIT (STEP3_BIT)
#endif
#if (defined(STEP4_BIT))
#define DIO5 5
#define STEP4 5
#define DIO5_BIT (STEP4_BIT)
#endif
#if (defined(STEP5_BIT))
#define DIO6 6
#define STEP5 6
#define DIO6_BIT (STEP5_BIT)
#endif
#if (defined(STEP6_BIT))
#define DIO7 7
#define STEP6 7
#define DIO7_BIT (STEP6_BIT)
#endif
#if (defined(STEP7_BIT))
#define DIO8 8
#define STEP7 8
#define DIO8_BIT (STEP7_BIT)
#endif
#if (defined(DIR0_BIT))
#define DIO9 9
#define DIR0 9
#define DIO9_BIT (DIR0_BIT)
#endif
#if (defined(DIR1_BIT))
#define DIO10 10
#define DIR1 10
#define DIO10_BIT (DIR1_BIT)
#endif
#if (defined(DIR2_BIT))
#define DIO11 11
#define DIR2 11
#define DIO11_BIT (DIR2_BIT)
#endif
#if (defined(DIR3_BIT))
#define DIO12 12
#define DIR3 12
#define DIO12_BIT (DIR3_BIT)
#endif
#if (defined(DIR4_BIT))
#define DIO13 13
#define DIR4 13
#define DIO13_BIT (DIR4_BIT)
#endif
#if (defined(DIR5_BIT))
#define DIO14 14
#define DIR5 14
#define DIO14_BIT (DIR5_BIT)
#endif
#if (defined(DIR6_BIT))
#define DIO15 15
#define DIR6 15
#define DIO15_BIT (DIR6_BIT)
#endif
#if (defined(DIR7_BIT))
#define DIO16 16
#define DIR7 16
#define DIO16_BIT (DIR7_BIT)
#endif
#if (defined(STEP0_EN_BIT))
#define DIO17 17
#define STEP0_EN 17
#define DIO17_BIT (STEP0_EN_BIT)
#endif
#if (defined(STEP1_EN_BIT))
#define DIO18 18
#define STEP1_EN 18
#define DIO18_BIT (STEP1_EN_BIT)
#endif
#if (defined(STEP2_EN_BIT))
#define DIO19 19
#define STEP2_EN 19
#define DIO19_BIT (STEP2_EN_BIT)
#endif
#if (defined(STEP3_EN_BIT))
#define DIO20 20
#define STEP3_EN 20
#define DIO20_BIT (STEP3_EN_BIT)
#endif
#if (defined(STEP4_EN_BIT))
#define DIO21 21
#define STEP4_EN 21
#define DIO21_BIT (STEP4_EN_BIT)
#endif
#if (defined(STEP5_EN_BIT))
#define DIO22 22
#define STEP5_EN 22
#define DIO22_BIT (STEP5_EN_BIT)
#endif
#if (defined(STEP6_EN_BIT))
#define DIO23 23
#define STEP6_EN 23
#define DIO23_BIT (STEP6_EN_BIT)
#endif
#if (defined(STEP7_EN_BIT))
#define DIO24 24
#define STEP7_EN 24
#define DIO24_BIT (STEP7_EN_BIT)
#endif
#if (defined(PWM0_BIT))
#define DIO25 25
#define PWM0 25
#define DIO25_BIT (PWM0_BIT)
#endif
#if (defined(PWM1_BIT))
#define DIO26 26
#define PWM1 26
#define DIO26_BIT (PWM1_BIT)
#endif
#if (defined(PWM2_BIT))
#define DIO27 27
#define PWM2 27
#define DIO27_BIT (PWM2_BIT)
#endif
#if (defined(PWM3_BIT))
#define DIO28 28
#define PWM3 28
#define DIO28_BIT (PWM3_BIT)
#endif
#if (defined(PWM4_BIT))
#define DIO29 29
#define PWM4 29
#define DIO29_BIT (PWM4_BIT)
#endif
#if (defined(PWM5_BIT))
#define DIO30 30
#define PWM5 30
#define DIO30_BIT (PWM5_BIT)
#endif
#if (defined(PWM6_BIT))
#define DIO31 31
#define PWM6 31
#define DIO31_BIT (PWM6_BIT)
#endif
#if (defined(PWM7_BIT))
#define DIO32 32
#define PWM7 32
#define DIO32_BIT (PWM7_BIT)
#endif
#if (defined(PWM8_BIT))
#define DIO33 33
#define PWM8 33
#define DIO33_BIT (PWM8_BIT)
#endif
#if (defined(PWM9_BIT))
#define DIO34 34
#define PWM9 34
#define DIO34_BIT (PWM9_BIT)
#endif
#if (defined(PWM10_BIT))
#define DIO35 35
#define PWM10 35
#define DIO35_BIT (PWM10_BIT)
#endif
#if (defined(PWM11_BIT))
#define DIO36 36
#define PWM11 36
#define DIO36_BIT (PWM11_BIT)
#endif
#if (defined(PWM12_BIT))
#define DIO37 37
#define PWM12 37
#define DIO37_BIT (PWM12_BIT)
#endif
#if (defined(PWM13_BIT))
#define DIO38 38
#define PWM13 38
#define DIO38_BIT (PWM13_BIT)
#endif
#if (defined(PWM14_BIT))
#define DIO39 39
#define PWM14 39
#define DIO39_BIT (PWM14_BIT)
#endif
#if (defined(PWM15_BIT))
#define DIO40 40
#define PWM15 40
#define DIO40_BIT (PWM15_BIT)
#endif
#if (defined(SERVO0_BIT))
#define DIO41 41
#define SERVO0 41
#define DIO41_BIT (SERVO0_BIT)
#endif
#if (defined(SERVO1_BIT))
#define DIO42 42
#define SERVO1 42
#define DIO42_BIT (SERVO1_BIT)
#endif
#if (defined(SERVO2_BIT))
#define DIO43 43
#define SERVO2 43
#define DIO43_BIT (SERVO2_BIT)
#endif
#if (defined(SERVO3_BIT))
#define DIO44 44
#define SERVO3 44
#define DIO44_BIT (SERVO3_BIT)
#endif
#if (defined(SERVO4_BIT))
#define DIO45 45
#define SERVO4 45
#define DIO45_BIT (SERVO4_BIT)
#endif
#if (defined(SERVO5_BIT))
#define DIO46 46
#define SERVO5 46
#define DIO46_BIT (SERVO5_BIT)
#endif
#if (defined(DOUT0_BIT))
#define DIO47 47
#define DOUT0 47
#define DIO47_BIT (DOUT0_BIT)
#endif
#if (defined(DOUT1_BIT))
#define DIO48 48
#define DOUT1 48
#define DIO48_BIT (DOUT1_BIT)
#endif
#if (defined(DOUT2_BIT))
#define DIO49 49
#define DOUT2 49
#define DIO49_BIT (DOUT2_BIT)
#endif
#if (defined(DOUT3_BIT))
#define DIO50 50
#define DOUT3 50
#define DIO50_BIT (DOUT3_BIT)
#endif
#if (defined(DOUT4_BIT))
#define DIO51 51
#define DOUT4 51
#define DIO51_BIT (DOUT4_BIT)
#endif
#if (defined(DOUT5_BIT))
#define DIO52 52
#define DOUT5 52
#define DIO52_BIT (DOUT5_BIT)
#endif
#if (defined(DOUT6_BIT))
#define DIO53 53
#define DOUT6 53
#define DIO53_BIT (DOUT6_BIT)
#endif
#if (defined(DOUT7_BIT))
#define DIO54 54
#define DOUT7 54
#define DIO54_BIT (DOUT7_BIT)
#endif
#if (defined(DOUT8_BIT))
#define DIO55 55
#define DOUT8 55
#define DIO55_BIT (DOUT8_BIT)
#endif
#if (defined(DOUT9_BIT))
#define DIO56 56
#define DOUT9 56
#define DIO56_BIT (DOUT9_BIT)
#endif
#if (defined(DOUT10_BIT))
#define DIO57 57
#define DOUT10 57
#define DIO57_BIT (DOUT10_BIT)
#endif
#if (defined(DOUT11_BIT))
#define DIO58 58
#define DOUT11 58
#define DIO58_BIT (DOUT11_BIT)
#endif
#if (defined(DOUT12_BIT))
#define DIO59 59
#define DOUT12 59
#define DIO59_BIT (DOUT12_BIT)
#endif
#if (defined(DOUT13_BIT))
#define DIO60 60
#define DOUT13 60
#define DIO60_BIT (DOUT13_BIT)
#endif
#if (defined(DOUT14_BIT))
#define DIO61 61
#define DOUT14 61
#define DIO61_BIT (DOUT14_BIT)
#endif
#if (defined(DOUT15_BIT))
#define DIO62 62
#define DOUT15 62
#define DIO62_BIT (DOUT15_BIT)
#endif
#if (defined(DOUT16_BIT))
#define DIO63 63
#define DOUT16 63
#define DIO63_BIT (DOUT16_BIT)
#endif
#if (defined(DOUT17_BIT))
#define DIO64 64
#define DOUT17 64
#define DIO64_BIT (DOUT17_BIT)
#endif
#if (defined(DOUT18_BIT))
#define DIO65 65
#define DOUT18 65
#define DIO65_BIT (DOUT18_BIT)
#endif
#if (defined(DOUT19_BIT))
#define DIO66 66
#define DOUT19 66
#define DIO66_BIT (DOUT19_BIT)
#endif
#if (defined(DOUT20_BIT))
#define DIO67 67
#define DOUT20 67
#define DIO67_BIT (DOUT20_BIT)
#endif
#if (defined(DOUT21_BIT))
#define DIO68 68
#define DOUT21 68
#define DIO68_BIT (DOUT21_BIT)
#endif
#if (defined(DOUT22_BIT))
#define DIO69 69
#define DOUT22 69
#define DIO69_BIT (DOUT22_BIT)
#endif
#if (defined(DOUT23_BIT))
#define DIO70 70
#define DOUT23 70
#define DIO70_BIT (DOUT23_BIT)
#endif
#if (defined(DOUT24_BIT))
#define DIO71 71
#define DOUT24 71
#define DIO71_BIT (DOUT24_BIT)
#endif
#if (defined(DOUT25_BIT))
#define DIO72 72
#define DOUT25 72
#define DIO72_BIT (DOUT25_BIT)
#endif
#if (defined(DOUT26_BIT))
#define DIO73 73
#define DOUT26 73
#define DIO73_BIT (DOUT26_BIT)
#endif
#if (defined(DOUT27_BIT))
#define DIO74 74
#define DOUT27 74
#define DIO74_BIT (DOUT27_BIT)
#endif
#if (defined(DOUT28_BIT))
#define DIO75 75
#define DOUT28 75
#define DIO75_BIT (DOUT28_BIT)
#endif
#if (defined(DOUT29_BIT))
#define DIO76 76
#define DOUT29 76
#define DIO76_BIT (DOUT29_BIT)
#endif
#if (defined(DOUT30_BIT))
#define DIO77 77
#define DOUT30 77
#define DIO77_BIT (DOUT30_BIT)
#endif
#if (defined(DOUT31_BIT))
#define DIO78 78
#define DOUT31 78
#define DIO78_BIT (DOUT31_BIT)
#endif
#if(defined(DOUT32_BIT))
#define DIO79 79
#define DOUT32 79
#define DIO79_BIT (DOUT32_BIT)
#endif
#if(defined(DOUT33_BIT))
#define DIO80 80
#define DOUT33 80
#define DIO80_BIT (DOUT33_BIT)
#endif
#if(defined(DOUT34_BIT))
#define DIO81 81
#define DOUT34 81
#define DIO81_BIT (DOUT34_BIT)
#endif
#if(defined(DOUT35_BIT))
#define DIO82 82
#define DOUT35 82
#define DIO82_BIT (DOUT35_BIT)
#endif
#if(defined(DOUT36_BIT))
#define DIO83 83
#define DOUT36 83
#define DIO83_BIT (DOUT36_BIT)
#endif
#if(defined(DOUT37_BIT))
#define DIO84 84
#define DOUT37 84
#define DIO84_BIT (DOUT37_BIT)
#endif
#if(defined(DOUT38_BIT))
#define DIO85 85
#define DOUT38 85
#define DIO85_BIT (DOUT38_BIT)
#endif
#if(defined(DOUT39_BIT))
#define DIO86 86
#define DOUT39 86
#define DIO86_BIT (DOUT39_BIT)
#endif
#if(defined(DOUT40_BIT))
#define DIO87 87
#define DOUT40 87
#define DIO87_BIT (DOUT40_BIT)
#endif
#if(defined(DOUT41_BIT))
#define DIO88 88
#define DOUT41 88
#define DIO88_BIT (DOUT41_BIT)
#endif
#if(defined(DOUT42_BIT))
#define DIO89 89
#define DOUT42 89
#define DIO89_BIT (DOUT42_BIT)
#endif
#if(defined(DOUT43_BIT))
#define DIO90 90
#define DOUT43 90
#define DIO90_BIT (DOUT43_BIT)
#endif
#if(defined(DOUT44_BIT))
#define DIO91 91
#define DOUT44 91
#define DIO91_BIT (DOUT44_BIT)
#endif
#if(defined(DOUT45_BIT))
#define DIO92 92
#define DOUT45 92
#define DIO92_BIT (DOUT45_BIT)
#endif
#if(defined(DOUT46_BIT))
#define DIO93 93
#define DOUT46 93
#define DIO93_BIT (DOUT46_BIT)
#endif
#if(defined(DOUT47_BIT))
#define DIO94 94
#define DOUT47 94
#define DIO94_BIT (DOUT47_BIT)
#endif
#if(defined(DOUT48_BIT))
#define DIO95 95
#define DOUT48 95
#define DIO95_BIT (DOUT48_BIT)
#endif
#if(defined(DOUT49_BIT))
#define DIO96 96
#define DOUT49 96
#define DIO96_BIT (DOUT49_BIT)
#endif
#if (defined(LIMIT_X_BIT))
#define DIO100 100
#define LIMIT_X 100
#define DIO100_BIT (LIMIT_X_BIT)
#endif
#if (defined(LIMIT_Y_BIT))
#define DIO101 101
#define LIMIT_Y 101
#define DIO101_BIT (LIMIT_Y_BIT)
#endif
#if (defined(LIMIT_Z_BIT))
#define DIO102 102
#define LIMIT_Z 102
#define DIO102_BIT (LIMIT_Z_BIT)
#endif
#if (defined(LIMIT_X2_BIT))
#define DIO103 103
#define LIMIT_X2 103
#define DIO103_BIT (LIMIT_X2_BIT)
#endif
#if (defined(LIMIT_Y2_BIT))
#define DIO104 104
#define LIMIT_Y2 104
#define DIO104_BIT (LIMIT_Y2_BIT)
#endif
#if (defined(LIMIT_Z2_BIT))
#define DIO105 105
#define LIMIT_Z2 105
#define DIO105_BIT (LIMIT_Z2_BIT)
#endif
#if (defined(LIMIT_A_BIT))
#define DIO106 106
#define LIMIT_A 106
#define DIO106_BIT (LIMIT_A_BIT)
#endif
#if (defined(LIMIT_B_BIT))
#define DIO107 107
#define LIMIT_B 107
#define DIO107_BIT (LIMIT_B_BIT)
#endif
#if (defined(LIMIT_C_BIT))
#define DIO108 108
#define LIMIT_C 108
#define DIO108_BIT (LIMIT_C_BIT)
#endif
#if (defined(PROBE_BIT))
#define DIO109 109
#define PROBE 109
#define DIO109_BIT (PROBE_BIT)
#endif
#if (defined(ESTOP_BIT))
#define DIO110 110
#define ESTOP 110
#define DIO110_BIT (ESTOP_BIT)
#endif
#if (defined(SAFETY_DOOR_BIT))
#define DIO111 111
#define SAFETY_DOOR 111
#define DIO111_BIT (SAFETY_DOOR_BIT)
#endif
#if (defined(FHOLD_BIT))
#define DIO112 112
#define FHOLD 112
#define DIO112_BIT (FHOLD_BIT)
#endif
#if (defined(CS_RES_BIT))
#define DIO113 113
#define CS_RES 113
#define DIO113_BIT (CS_RES_BIT)
#endif
#if (defined(ANALOG0_BIT))
#define DIO114 114
#define ANALOG0 114
#define DIO114_BIT (ANALOG0_BIT)
#endif
#if (defined(ANALOG1_BIT))
#define DIO115 115
#define ANALOG1 115
#define DIO115_BIT (ANALOG1_BIT)
#endif
#if (defined(ANALOG2_BIT))
#define DIO116 116
#define ANALOG2 116
#define DIO116_BIT (ANALOG2_BIT)
#endif
#if (defined(ANALOG3_BIT))
#define DIO117 117
#define ANALOG3 117
#define DIO117_BIT (ANALOG3_BIT)
#endif
#if (defined(ANALOG4_BIT))
#define DIO118 118
#define ANALOG4 118
#define DIO118_BIT (ANALOG4_BIT)
#endif
#if (defined(ANALOG5_BIT))
#define DIO119 119
#define ANALOG5 119
#define DIO119_BIT (ANALOG5_BIT)
#endif
#if (defined(ANALOG6_BIT))
#define DIO120 120
#define ANALOG6 120
#define DIO120_BIT (ANALOG6_BIT)
#endif
#if (defined(ANALOG7_BIT))
#define DIO121 121
#define ANALOG7 121
#define DIO121_BIT (ANALOG7_BIT)
#endif
#if (defined(ANALOG8_BIT))
#define DIO122 122
#define ANALOG8 122
#define DIO122_BIT (ANALOG8_BIT)
#endif
#if (defined(ANALOG9_BIT))
#define DIO123 123
#define ANALOG9 123
#define DIO123_BIT (ANALOG9_BIT)
#endif
#if (defined(ANALOG10_BIT))
#define DIO124 124
#define ANALOG10 124
#define DIO124_BIT (ANALOG10_BIT)
#endif
#if (defined(ANALOG11_BIT))
#define DIO125 125
#define ANALOG11 125
#define DIO125_BIT (ANALOG11_BIT)
#endif
#if (defined(ANALOG12_BIT))
#define DIO126 126
#define ANALOG12 126
#define DIO126_BIT (ANALOG12_BIT)
#endif
#if (defined(ANALOG13_BIT))
#define DIO127 127
#define ANALOG13 127
#define DIO127_BIT (ANALOG13_BIT)
#endif
#if (defined(ANALOG14_BIT))
#define DIO128 128
#define ANALOG14 128
#define DIO128_BIT (ANALOG14_BIT)
#endif
#if (defined(ANALOG15_BIT))
#define DIO129 129
#define ANALOG15 129
#define DIO129_BIT (ANALOG15_BIT)
#endif
#if (defined(DIN0_BIT))
#define DIO130 130
#define DIN0 130
#define DIO130_BIT (DIN0_BIT)
#endif
#if (defined(DIN1_BIT))
#define DIO131 131
#define DIN1 131
#define DIO131_BIT (DIN1_BIT)
#endif
#if (defined(DIN2_BIT))
#define DIO132 132
#define DIN2 132
#define DIO132_BIT (DIN2_BIT)
#endif
#if (defined(DIN3_BIT))
#define DIO133 133
#define DIN3 133
#define DIO133_BIT (DIN3_BIT)
#endif
#if (defined(DIN4_BIT))
#define DIO134 134
#define DIN4 134
#define DIO134_BIT (DIN4_BIT)
#endif
#if (defined(DIN5_BIT))
#define DIO135 135
#define DIN5 135
#define DIO135_BIT (DIN5_BIT)
#endif
#if (defined(DIN6_BIT))
#define DIO136 136
#define DIN6 136
#define DIO136_BIT (DIN6_BIT)
#endif
#if (defined(DIN7_BIT))
#define DIO137 137
#define DIN7 137
#define DIO137_BIT (DIN7_BIT)
#endif
#if (defined(DIN8_BIT))
#define DIO138 138
#define DIN8 138
#define DIO138_BIT (DIN8_BIT)
#endif
#if (defined(DIN9_BIT))
#define DIO139 139
#define DIN9 139
#define DIO139_BIT (DIN9_BIT)
#endif
#if (defined(DIN10_BIT))
#define DIO140 140
#define DIN10 140
#define DIO140_BIT (DIN10_BIT)
#endif
#if (defined(DIN11_BIT))
#define DIO141 141
#define DIN11 141
#define DIO141_BIT (DIN11_BIT)
#endif
#if (defined(DIN12_BIT))
#define DIO142 142
#define DIN12 142
#define DIO142_BIT (DIN12_BIT)
#endif
#if (defined(DIN13_BIT))
#define DIO143 143
#define DIN13 143
#define DIO143_BIT (DIN13_BIT)
#endif
#if (defined(DIN14_BIT))
#define DIO144 144
#define DIN14 144
#define DIO144_BIT (DIN14_BIT)
#endif
#if (defined(DIN15_BIT))
#define DIO145 145
#define DIN15 145
#define DIO145_BIT (DIN15_BIT)
#endif
#if (defined(DIN16_BIT))
#define DIO146 146
#define DIN16 146
#define DIO146_BIT (DIN16_BIT)
#endif
#if (defined(DIN17_BIT))
#define DIO147 147
#define DIN17 147
#define DIO147_BIT (DIN17_BIT)
#endif
#if (defined(DIN18_BIT))
#define DIO148 148
#define DIN18 148
#define DIO148_BIT (DIN18_BIT)
#endif
#if (defined(DIN19_BIT))
#define DIO149 149
#define DIN19 149
#define DIO149_BIT (DIN19_BIT)
#endif
#if (defined(DIN20_BIT))
#define DIO150 150
#define DIN20 150
#define DIO150_BIT (DIN20_BIT)
#endif
#if (defined(DIN21_BIT))
#define DIO151 151
#define DIN21 151
#define DIO151_BIT (DIN21_BIT)
#endif
#if (defined(DIN22_BIT))
#define DIO152 152
#define DIN22 152
#define DIO152_BIT (DIN22_BIT)
#endif
#if (defined(DIN23_BIT))
#define DIO153 153
#define DIN23 153
#define DIO153_BIT (DIN23_BIT)
#endif
#if (defined(DIN24_BIT))
#define DIO154 154
#define DIN24 154
#define DIO154_BIT (DIN24_BIT)
#endif
#if (defined(DIN25_BIT))
#define DIO155 155
#define DIN25 155
#define DIO155_BIT (DIN25_BIT)
#endif
#if (defined(DIN26_BIT))
#define DIO156 156
#define DIN26 156
#define DIO156_BIT (DIN26_BIT)
#endif
#if (defined(DIN27_BIT))
#define DIO157 157
#define DIN27 157
#define DIO157_BIT (DIN27_BIT)
#endif
#if (defined(DIN28_BIT))
#define DIO158 158
#define DIN28 158
#define DIO158_BIT (DIN28_BIT)
#endif
#if (defined(DIN29_BIT))
#define DIO159 159
#define DIN29 159
#define DIO159_BIT (DIN29_BIT)
#endif
#if (defined(DIN30_BIT))
#define DIO160 160
#define DIN30 160
#define DIO160_BIT (DIN30_BIT)
#endif
#if (defined(DIN31_BIT))
#define DIO161 161
#define DIN31 161
#define DIO161_BIT (DIN31_BIT)
#endif
#if(defined(DIN32_BIT))
#define DIO162 162
#define DIN32 162
#define DIO162_BIT (DIN32_BIT)
#endif
#if(defined(DIN33_BIT))
#define DIO163 163
#define DIN33 163
#define DIO163_BIT (DIN33_BIT)
#endif
#if(defined(DIN34_BIT))
#define DIO164 164
#define DIN34 164
#define DIO164_BIT (DIN34_BIT)
#endif
#if(defined(DIN35_BIT))
#define DIO165 165
#define DIN35 165
#define DIO165_BIT (DIN35_BIT)
#endif
#if(defined(DIN36_BIT))
#define DIO166 166
#define DIN36 166
#define DIO166_BIT (DIN36_BIT)
#endif
#if(defined(DIN37_BIT))
#define DIO167 167
#define DIN37 167
#define DIO167_BIT (DIN37_BIT)
#endif
#if(defined(DIN38_BIT))
#define DIO168 168
#define DIN38 168
#define DIO168_BIT (DIN38_BIT)
#endif
#if(defined(DIN39_BIT))
#define DIO169 169
#define DIN39 169
#define DIO169_BIT (DIN39_BIT)
#endif
#if(defined(DIN40_BIT))
#define DIO170 170
#define DIN40 170
#define DIO170_BIT (DIN40_BIT)
#endif
#if(defined(DIN41_BIT))
#define DIO171 171
#define DIN41 171
#define DIO171_BIT (DIN41_BIT)
#endif
#if(defined(DIN42_BIT))
#define DIO172 172
#define DIN42 172
#define DIO172_BIT (DIN42_BIT)
#endif
#if(defined(DIN43_BIT))
#define DIO173 173
#define DIN43 173
#define DIO173_BIT (DIN43_BIT)
#endif
#if(defined(DIN44_BIT))
#define DIO174 174
#define DIN44 174
#define DIO174_BIT (DIN44_BIT)
#endif
#if(defined(DIN45_BIT))
#define DIO175 175
#define DIN45 175
#define DIO175_BIT (DIN45_BIT)
#endif
#if(defined(DIN46_BIT))
#define DIO176 176
#define DIN46 176
#define DIO176_BIT (DIN46_BIT)
#endif
#if(defined(DIN47_BIT))
#define DIO177 177
#define DIN47 177
#define DIO177_BIT (DIN47_BIT)
#endif
#if(defined(DIN48_BIT))
#define DIO178 178
#define DIN48 178
#define DIO178_BIT (DIN48_BIT)
#endif
#if(defined(DIN49_BIT))
#define DIO179 179
#define DIN49 179
#define DIO179_BIT (DIN49_BIT)
#endif
#if (defined(TX_BIT))
#define DIO200 200
#define TX 200
#define DIO200_BIT (TX_BIT)
#endif
#if (defined(RX_BIT))
#define DIO201 201
#define RX 201
#define DIO201_BIT (RX_BIT)
#endif
#if (defined(USB_DM_BIT))
#define DIO202 202
#define USB_DM 202
#define DIO202_BIT (USB_DM_BIT)
#endif
#if (defined(USB_DP_BIT))
#define DIO203 203
#define USB_DP 203
#define DIO203_BIT (USB_DP_BIT)
#endif
#if (defined(SPI_CLK_BIT))
#define DIO204 204
#define SPI_CLK 204
#define DIO204_BIT (SPI_CLK_BIT)
#endif
#if (defined(SPI_SDI_BIT))
#define DIO205 205
#define SPI_SDI 205
#define DIO205_BIT (SPI_SDI_BIT)
#endif
#if (defined(SPI_SDO_BIT))
#define DIO206 206
#define SPI_SDO 206
#define DIO206_BIT (SPI_SDO_BIT)
#endif
#if (defined(SPI_CS_BIT))
#define DIO207 207
#define SPI_CS 207
#define DIO207_BIT (SPI_CS_BIT)
#endif
#if (defined(I2C_CLK_BIT))
#define DIO208 208
#define I2C_CLK 208
#define DIO208_BIT (I2C_CLK_BIT)
#endif
#if (defined(I2C_DATA_BIT))
#define DIO209 209
#define I2C_DATA 209
#define DIO209_BIT (I2C_DATA_BIT)
#endif
#if (defined(TX2_BIT))
#define DIO210 210
#define TX2 210
#define DIO210_BIT (TX2_BIT)
#endif
#if (defined(RX2_BIT))
#define DIO211 211
#define RX2 211
#define DIO211_BIT (RX2_BIT)
#endif
#if (defined(SPI2_CLK_BIT))
#define DIO212 212
#define SPI2_CLK 212
#define DIO212_BIT (SPI2_CLK_BIT)
#endif
#if (defined(SPI2_SDI_BIT))
#define DIO213 213
#define SPI2_SDI 213
#define DIO213_BIT (SPI2_SDI_BIT)
#endif
#if (defined(SPI2_SDO_BIT))
#define DIO214 214
#define SPI2_SDO 214
#define DIO214_BIT (SPI2_SDO_BIT)
#endif
#if (defined(SPI2_CS_BIT))
#define DIO215 215
#define SPI2_CS 215
#define DIO215_BIT (SPI2_CS_BIT)
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
// #ifdef ENABLE_WIFI
// #define MCU_HAS_WIFI
// #ifndef DISABLE_ENDPOINTS
// #define MCU_HAS_ENDPOINTS
// #endif
// #ifndef DISABLE_WEBSOCKETS
// #define MCU_HAS_WEBSOCKETS
// #endif
// #ifndef BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
// #define BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
// #endif
// #endif
// #ifdef ENABLE_BLUETOOTH
// #define MCU_HAS_BLUETOOTH
// #ifndef BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
// #define BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
// #endif
// #endif

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
#define SPI_HW __helper__(spi, SPI_PORT,)

// for SDK SPI
#define SPI2_HW __helper__(spi, SPI2_PORT,)


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

#define __timer_irq__(X) (TIMER0_IRQ_0 + X)
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

#define mcu_config_output(X) pinMode(__indirect__(X, BIT), OUTPUT)
#define mcu_config_pwm(X, freq)            \
	{                                        \
		pinMode(__indirect__(X, BIT), OUTPUT); \
		analogWriteRange(255);                 \
		analogWriteFreq(freq);                 \
		analogWriteResolution(8);              \
	}
#define mcu_config_input(X) pinMode(__indirect__(X, BIT), INPUT)
#define mcu_config_analog(X) mcu_config_input(X)
#define mcu_config_pullup(X) pinMode(__indirect__(X, BIT), INPUT_PULLUP)
#define mcu_config_input_isr(X) attachInterrupt(digitalPinToInterrupt(__indirect__(X, BIT)), mcu_din_isr, CHANGE)

#define mcu_get_input(X) CHECKBIT(sio_hw->gpio_in, __indirect__(X, BIT))
#define mcu_get_output(X) CHECKBIT(sio_hw->gpio_out, __indirect__(X, BIT))
#define mcu_set_output(X) ({ sio_hw->gpio_set = (1UL << __indirect__(X, BIT)); })
#define mcu_clear_output(X) ({ sio_hw->gpio_clr = (1UL << __indirect__(X, BIT)); })
#define mcu_toggle_output(X) ({ sio_hw->gpio_togl = (1UL << __indirect__(X, BIT)); })

	extern uint8_t rp2350_pwm[16];
#define mcu_set_pwm(X, Y)                 \
	{                                       \
		rp2350_pwm[X - PWM_PINS_OFFSET] = Y;  \
		analogWrite(__indirect__(X, BIT), Y); \
	}
#define mcu_get_pwm(X) (rp2350_pwm[X - PWM_PINS_OFFSET])
#define mcu_get_analog(X) analogRead(__indirect__(X, BIT))

#define mcu_millis() millis()
#define mcu_micros() micros()
#define mcu_free_micros() micros()

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
#ifdef RP2350_RUN_MULTICORE

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
	// 	extern void rp2350_core1_loop();
	// #define ucnc_init() cnc_init();	multicore_launch_core1(rp2350_core1_loop)
	extern void rp2350_core0_loop();
#define ucnc_run() rp2350_core0_loop()

#endif

#ifdef __cplusplus
}
#endif

#endif
