/*
	Name: mcumap_virtual.h
	Description: Contains all MCU and PIN definitions for a PC to run µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 01/11/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef MCUMAP_VIRTUAL_H
#define MCUMAP_VIRTUAL_H

#include <stdint.h>
#define F_CPU 1000000
#ifndef F_STEP_MAX
#define F_STEP_MAX 40000
#endif
#ifndef F_STEP_MIN
#define F_STEP_MIN 1
#endif
#define __rom__
#define __romstr__
#define __romarr__ const uint8_t
#define rom_strptr *
#define rom_strcpy strcpy
#define rom_strncpy strncpy
#define rom_memcpy memcpy
#define rom_read_byte *

#define MCU_WEAK __attribute__((weak,weakref))

/* 7.18.2.1  Limits of exact-width integer types */
#define INT8_MIN (-128)
#define INT16_MIN (-32768)
#define INT32_MIN (-2147483647 - 1)
#define INT64_MIN (-9223372036854775807LL - 1)

#define INT8_MAX 127
#define INT16_MAX 32767
#define INT32_MAX 2147483647
#define INT64_MAX 9223372036854775807LL

#define UINT8_MAX 255
#define UINT16_MAX 65535
#define UINT32_MAX 0xffffffffU					 /* 4294967295U */
#define UINT64_MAX 0xffffffffffffffffULL /* 18446744073709551615ULL */

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

//#define MCU_HAS_UART
#ifndef UART_PORT_NAME
#define UART_PORT_NAME "\\\\.\\COM14"
#endif

#define MCU_HAS_UART2

// #define EMULATE_74HC595

// joints step/dir pins
#ifndef EMULATE_74HC595
#define STEP0 1
#define DIO1 1
#define STEP1 2
#define DIO2 2
#define STEP2 3
#define DIO3 3
#define STEP3 4
#define DIO4 4
#define STEP4 5
#define DIO5 5
#define STEP5 6
#define DIO6 6
#define STEP6 7
#define DIO7 7
#define STEP7 8
#define DIO8 8
#define DIR0 9
#define DIO9 9
#define DIR1 10
#define DIO10 10
#define DIR2 11
#define DIO11 11
#define DIR3 12
#define DIO12 12
#define DIR4 13
#define DIO13 13
#define DIR5 14
#define DIO14 14
#define DIR6 15
#define DIO15 15
#define DIR7 16
#define DIO16 16
#define STEP0_EN 17
#define DIO17 17
#define STEP1_EN 18
#define DIO18 18
#define STEP2_EN 19
#define DIO19 19
#define STEP3_EN 20
#define DIO20 20
#define STEP4_EN 21
#define DIO21 21
#define STEP5_EN 22
#define DIO22 22
#define STEP6_EN 23
#define DIO23 23
#define STEP7_EN 24
#define DIO24 24
#else
#define IC74HC595_COUNT 4
#define STEP0_IO_OFFSET 0
#define STEP1_IO_OFFSET 1
#define STEP2_IO_OFFSET 2
#define STEP3_IO_OFFSET 3
#define STEP4_IO_OFFSET 4
#define STEP5_IO_OFFSET 5
#define STEP6_IO_OFFSET 6
#define STEP7_IO_OFFSET 7
#define DIR0_IO_OFFSET 8
#define DIR1_IO_OFFSET 9
#define DIR2_IO_OFFSET 10
#define DIR3_IO_OFFSET 11
#define DIR4_IO_OFFSET 12
#define DIR5_IO_OFFSET 13
#define DIR6_IO_OFFSET 14
#define DIR7_IO_OFFSET 15
#define STEP0_EN_IO_OFFSET 16
#define STEP1_EN_IO_OFFSET 17
#define STEP2_EN_IO_OFFSET 18
#define STEP3_EN_IO_OFFSET 19
#define STEP4_EN_IO_OFFSET 20
#define STEP5_EN_IO_OFFSET 21
#define STEP6_EN_IO_OFFSET 22
#define STEP7_EN_IO_OFFSET 23
// #define PWM0_IO_OFFSET 24
#endif
#define PWM0 25
#define DIO25 25
#define PWM1 26
#define DIO26 26
#define PWM2 27
#define DIO27 27
#define PWM3 28
#define DIO28 28
#define PWM4 29
#define DIO29 29
#define PWM5 30
#define DIO30 30
#define PWM6 31
#define DIO31 31
#define PWM7 32
#define DIO32 32
#define PWM8 33
#define DIO33 33
#define PWM9 34
#define DIO34 34
#define PWM10 35
#define DIO35 35
#define PWM11 36
#define DIO36 36
#define PWM12 37
#define DIO37 37
#define PWM13 38
#define DIO38 38
#define PWM14 39
#define DIO39 39
#define PWM15 40
#define DIO40 40
#define SERVO0 41
#define DIO41 41
#define SERVO1 42
#define DIO42 42
#define SERVO2 43
#define DIO43 43
#define SERVO3 44
#define DIO44 44
#define SERVO4 45
#define DIO45 45
#define SERVO5 46
#define DIO46 46
#define DOUT0 47
#define DIO47 47
#define DOUT1 48
#define DIO48 48
#define DOUT2 49
#define DIO49 49
#define DOUT3 50
#define DIO50 50
#define DOUT4 51
#define DIO51 51
#define DOUT5 52
#define DIO52 52
#define DOUT6 53
#define DIO53 53
#define DOUT7 54
#define DIO54 54
#define DOUT8 55
#define DIO55 55
#define DOUT9 56
#define DIO56 56
#define DOUT10 57
#define DIO57 57
#define DOUT11 58
#define DIO58 58
#define DOUT12 59
#define DIO59 59
#define DOUT13 60
#define DIO60 60
#define DOUT14 61
#define DIO61 61
#define DOUT15 62
#define DIO62 62
#define DOUT16 63
#define DIO63 63
#define DOUT17 64
#define DIO64 64
#define DOUT18 65
#define DIO65 65
#define DOUT19 66
#define DIO66 66
#define DOUT20 67
#define DIO67 67
#define DOUT21 68
#define DIO68 68
#define DOUT22 69
#define DIO69 69
#define DOUT23 70
#define DIO70 70
#define DOUT24 71
#define DIO71 71
#define DOUT25 72
#define DIO72 72
#define DOUT26 73
#define DIO73 73
#define DOUT27 74
#define DIO74 74
#define DOUT28 75
#define DIO75 75
#define DOUT29 76
#define DIO76 76
#define DOUT30 77
#define DIO77 77
#define DOUT31 UNDEF_PIN
#define DIO78 UNDEF_PIN

#define ACTIVITY_LED UNDEF_PIN

#ifndef EMULATE_74HC165
#define LIMIT_X 100
#define DIO100 100
#define LIMIT_Y 101
#define DIO101 101
#define LIMIT_Z 102
#define DIO102 102
#define LIMIT_X2 103
#define DIO103 103
#define LIMIT_Y2 104
#define DIO104 104
#define LIMIT_Z2 105
#define DIO105 105
#define LIMIT_A 106
#define DIO106 106
#define LIMIT_B 107
#define DIO107 107
#define LIMIT_C 108
#define DIO108 108
#define PROBE 109
#define DIO109 109
#define ESTOP 110
#define DIO110 110
#define SAFETY_DOOR 111
#define DIO111 111
#define FHOLD 112
#define DIO112 112
#define CS_RES 113
#define DIO113 113
#define DIN0 130
#define DIN0_ISR
#define DIO130 130
#define DIN1 131
#define DIO131 131
#define DIN2 132
#define DIO132 132
#define DIN3 133
#define DIO133 133
#define DIN4 134
#define DIO134 134
#define DIN5 135
#define DIO135 135
#define DIN6 136
#define DIO136 136
#define DIN7 137
#define DIO137 137
#else
#define IC74HC165_COUNT 4
#define LIMIT_X_IO_OFFSET 0
#define LIMIT_Y_IO_OFFSET 1
#define LIMIT_Z_IO_OFFSET 2
#define LIMIT_A_IO_OFFSET 3
#define LIMIT_B_IO_OFFSET 4
#define LIMIT_C_IO_OFFSET 5
#define PROBE_IO_OFFSET 6
#define ESTOP_IO_OFFSET 7
#define SAFETY_DOOR_IO_OFFSET 8
#define FHOLD_IO_OFFSET 9
#define CS_RES_IO_OFFSET 10
#define DIN0_IO_OFFSET 11
#define DIN1_IO_OFFSET 12
#define DIN2_IO_OFFSET 13
#define DIN3_IO_OFFSET 14
#define DIN4_IO_OFFSET 15
#define DIN5_IO_OFFSET 16
#define DIN6_IO_OFFSET 17
#define DIN7_IO_OFFSET 18
// #define PWM0_IO_OFFSET 24
#endif

#define DIN8 138
#define DIO138 138
#define DIN9 139
#define DIO139 139
#define DIN10 140
#define DIO140 140
#define DIN11 141
#define DIO141 141
#define DIN12 142
#define DIO142 142
#define DIN13 143
#define DIO143 143
#define DIN14 144
#define DIO144 144
#define DIN15 145
#define DIO145 145
#define DIN16 146
#define DIO146 146
#define DIN17 147
#define DIO147 147
#define DIN18 148
#define DIO148 148
#define DIN19 149
#define DIO149 149
#define DIN20 150
#define DIO150 150
#define DIN21 151
#define DIO151 151
#define DIN22 152
#define DIO152 152
#define DIN23 153
#define DIO153 153
#define DIN24 154
#define DIO154 154
#define DIN25 155
#define DIO155 155
#define DIN26 156
#define DIO156 156
#define DIN27 157
#define DIO157 157
#define DIN28 158
#define DIO158 158
#define DIN29 159
#define DIO159 159
#define DIN30 160
#define DIO160 160
#define DIN31 161
#define DIO161 161
#define ANALOG0 114
#define DIO114 114
#define ANALOG1 115
#define DIO115 115
#define ANALOG2 116
#define DIO116 116
#define ANALOG3 117
#define DIO117 117
#define ANALOG4 118
#define DIO118 118
#define ANALOG5 119
#define DIO119 119
#define ANALOG6 120
#define DIO120 120
#define ANALOG7 121
#define DIO121 121
#define ANALOG8 122
#define DIO122 122
#define ANALOG9 123
#define DIO123 123
#define ANALOG10 124
#define DIO124 124
#define ANALOG11 125
#define DIO125 125
#define ANALOG12 126
#define DIO126 126
#define ANALOG13 127
#define DIO127 127
#define ANALOG14 128
#define DIO128 128
#define ANALOG15 129
#define DIO129 129
#define TX 200
#define DIO200 200
#define RX 201
#define DIO201 201
#define USB_DM 202
#define DIO202 202
#define USB_DP 203
#define DIO203 203
#define SPI_CLK 204
#define DIO204 204
#define SPI_SDI 205
#define DIO205 205
#define SPI_SDO 206
#define DIO206 206
#define SPI_CS 207
#define DIO207 207
#define I2C_CLK 208
#define DIO208 208
#define I2C_DATA 209
#define DIO209 209
#define TX2 210
#define DIO210 210
#define RX2 211
#define DIO211 211

#define MCU_HAS_ONESHOT_TIMER

#ifndef BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
#define BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
#endif
#ifndef ENABLE_PARSER_MODULES
#define ENABLE_PARSER_MODULES
#endif
#define DISABLE_SYSTEM_MENU
#define freefile_ptr(x)

// just to compile
#define mcu_nop()
#define mcu_config_pullup(diopin)
#define mcu_config_analog(diopin)
#define asm __asm__
#define mcu_config_input_isr(x)

#define mcu_delay_cycles(X)
extern void virtual_delay_us(uint16_t delay);
#define mcu_delay_us(X) virtual_delay_us(X)

#include "../../tools/tool.h"
extern const tool_t embroidery_stepper;
extern const tool_t laser_ppi;
extern const tool_t laser_pwm;
extern const tool_t pen_servo;
extern const tool_t plasma_thc;
extern const tool_t spindle_besc;
extern const tool_t spindle_pwm;
extern const tool_t spindle_relay;
extern const tool_t vfd_modbus;
extern const tool_t vfd_pwm;

#define EMULATION_MS_TICK 100

#define ATOMIC_LOAD_N(src, mode) __atomic_load_n((src), mode)
#define ATOMIC_STORE_N(dst, val, mode) __atomic_store_n((dst), (val), mode)
#define ATOMIC_COMPARE_EXCHANGE_N(dst, cmp, des, sucmode, failmode) __atomic_compare_exchange_n((dst), (void*)(cmp), (des), false, sucmode, failmode)
#define ATOMIC_FETCH_OR(dst, val, mode) __atomic_fetch_or((dst), (val), mode)
#define ATOMIC_FETCH_AND(dst, val, mode) __atomic_fetch_and((dst), (val), mode)
#define ATOMIC_FETCH_ADD(dst, val, mode) __atomic_fetch_add((dst), (val), mode)
#define ATOMIC_FETCH_SUB(dst, val, mode) __atomic_fetch_sub((dst), (val), mode)
#define ATOMIC_FETCH_XOR(dst, val, mode) __atomic_fetch_xor((dst), (val), mode)
#define ATOMIC_SPIN()

#define asm __asm__

#endif
