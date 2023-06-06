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

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

// #pragma comment(lib, "Ws2_32.lib")
#include "win_port.h"

#define F_CPU 1000000
#ifndef F_STEP_MAX
#define F_STEP_MAX 40000
#endif
#ifndef F_STEP_MIN
#define F_STEP_MIN 1
#endif
#define __rom__
#define __romstr__
#define __romarr__ const char
#define rom_strptr *
#define rom_strcpy strcpy
#define rom_strncpy strncpy
#define rom_memcpy memcpy
#define rom_read_byte *

//needed by software delays
#ifndef MCU_CLOCKS_PER_CYCLE
#define MCU_CLOCKS_PER_CYCLE 1
#endif
#ifndef MCU_CYCLES_PER_LOOP
#define MCU_CYCLES_PER_LOOP 1
#endif
#ifndef MCU_CYCLES_PER_LOOP_OVERHEAD
#define MCU_CYCLES_PER_LOOP_OVERHEAD 0
#endif

#define ENABLE_SYNC_TX

// joints step/dir pins
#define STEP0 1
#define STEP1 2
#define STEP2 3
#define STEP3 4
#define STEP4 5
#define STEP5 6
#define STEP6 7
#define STEP7 8
#define DIR0 9
#define DIR1 10
#define DIR2 11
#define DIR3 12
#define DIR4 13
#define DIR5 14
#define DIR6 15
#define DIR7 16
#define STEP0_EN 17
#define STEP1_EN 18
#define STEP2_EN 19
#define STEP3_EN 20
#define STEP4_EN 21
#define STEP5_EN 22
#define STEP6_EN 23
#define STEP7_EN 24
#define PWM0 25
#define PWM1 26
#define PWM2 27
#define PWM3 28
#define PWM4 29
#define PWM5 30
#define PWM6 31
#define PWM7 32
#define PWM8 33
#define PWM9 34
#define PWM10 35
#define PWM11 36
#define PWM12 37
#define PWM13 38
#define PWM14 39
#define PWM15 40
#define SERVO0 41
#define SERVO1 42
#define SERVO2 43
#define SERVO3 44
#define SERVO4 45
#define SERVO5 46
#define DOUT0 47
#define DOUT1 48
#define DOUT2 49
#define DOUT3 50
#define DOUT4 51
#define DOUT5 52
#define DOUT6 53
#define DOUT7 54
#define DOUT8 55
#define DOUT9 56
#define DOUT10 57
#define DOUT11 58
#define DOUT12 59
#define DOUT13 60
#define DOUT14 61
#define DOUT15 62
#define DOUT16 63
#define DOUT17 64
#define DOUT18 65
#define DOUT19 66
#define DOUT20 67
#define DOUT21 68
#define DOUT22 69
#define DOUT23 70
#define DOUT24 71
#define DOUT25 72
#define DOUT26 73
#define DOUT27 74
#define DOUT28 75
#define DOUT29 76
#define DOUT30 77
#define DOUT31 78
#define LIMIT_X 100
#define LIMIT_Y 101
#define LIMIT_Z 102
#define LIMIT_X2 103
#define LIMIT_Y2 104
#define LIMIT_Z2 105
#define LIMIT_A 106
#define LIMIT_B 107
#define LIMIT_C 108
#define PROBE 109
#define ESTOP 110
#define SAFETY_DOOR 111
#define FHOLD 112
#define CS_RES 113
#define ANALOG0 114
#define ANALOG1 115
#define ANALOG2 116
#define ANALOG3 117
#define ANALOG4 118
#define ANALOG5 119
#define ANALOG6 120
#define ANALOG7 121
#define ANALOG8 122
#define ANALOG9 123
#define ANALOG10 124
#define ANALOG11 125
#define ANALOG12 126
#define ANALOG13 127
#define ANALOG14 128
#define ANALOG15 129
#define DIN0 130
#define DIN1 131
#define DIN2 132
#define DIN3 133
#define DIN4 134
#define DIN5 135
#define DIN6 136
#define DIN7 137
#define DIN8 138
#define DIN9 139
#define DIN10 140
#define DIN11 141
#define DIN12 142
#define DIN13 143
#define DIN14 144
#define DIN15 145
#define DIN16 146
#define DIN17 147
#define DIN18 148
#define DIN19 149
#define DIN20 150
#define DIN21 151
#define DIN22 152
#define DIN23 153
#define DIN24 154
#define DIN25 155
#define DIN26 156
#define DIN27 157
#define DIN28 158
#define DIN29 159
#define DIN30 160
#define DIN31 161
#define TX 200
#define RX 201
#define USB_DM 202
#define USB_DP 203
#define SPI_CLK 204
#define SPI_SDI 205
#define SPI_SDO 206
#define SPI_CS 207
#define I2C_CLK 208
#define I2C_DATA 209

#define MCU_HAS_ONESHOT_TIMER

// just to compile
#define mcu_rx_ready() true
#define mcu_nop()
#define mcu_config_pullup(diopin)
#define mcu_config_analog(diopin)
#define asm __asm__

#define mcu_delay_cycles(X)
extern void virtual_delay_us(uint16_t delay);
#define mcu_delay_us(X) virtual_delay_us(X)

#define asm __asm__
// #define VFD_HUANYANG_TYPE1

//allow use uart2 via socket
#define MCU_HAS_UART2
#define UART2_DETACH_MAIN_PROTOCOL


#endif
