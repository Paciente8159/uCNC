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
#define __romarr__ const char
#define rom_strptr *
#define rom_strcpy strcpy
#define rom_strncpy strncpy
#define rom_memcpy memcpy
#define rom_read_byte *

#define ENABLE_SYNC_TX

// uncomment to use sockets
//#define USESOCKETS
#ifdef USESOCKETS
#define DEFAULT_BUFLEN 127
#define DEFAULT_PORT "34000"
#endif

// uncomment to use serial port
#define USESERIAL
#ifdef USESERIAL
#ifndef COMPORT
#define COMPORT "\\\\.\\COM11"
#endif
#endif

//#define USECONSOLE

// joints step/dir pins
#define STEP0 0
#define STEP1 1
#define STEP2 2
#define STEP3 3
#define STEP4 4
//#define STEP5 5
//#define STEP6 6
//#define STEP7 7
#define DIR0 8
#define DIR1 9
#define DIR2 10
#define DIR3 11
#define DIR4 12
//#define DIR5 13
//#define DIR6 14
//#define DIR7 15
#define STEP0_EN 16
#define STEP1_EN 17
#define STEP2_EN 18
#define STEP3_EN 19
#define STEP4_EN 20
//#define STEP5_EN 21
//#define STEP6_EN 22
//#define STEP7_EN 23
#define PWM0 24
//#define PWM1 25
//#define PWM2 26
//#define PWM3 27
//#define PWM4 28
//#define PWM5 29
//#define PWM6 30
//#define PWM7 31
//#define PWM8 32
//#define PWM9 33
//#define PWM10 34
//#define PWM11 35
//#define PWM12 36
//#define PWM13 37
//#define PWM14 38
//#define PWM15 39
//#define SERVO0 40
//#define SERVO1 41
//#define SERVO2 42
//#define SERVO3 43
//#define SERVO4 44
//#define SERVO5 45
#define DOUT0 46
#define DOUT1 47
#define DOUT2 48
#define DOUT3 49
#define DOUT4 50
#define DOUT5 51
#define DOUT6 52
#define DOUT7 53
//#define DOUT8 54
//#define DOUT9 55
//#define DOUT10 56
//#define DOUT11 57
//#define DOUT12 58
//#define DOUT13 59
//#define DOUT14 60
//#define DOUT15 61
//#define DOUT16 62
//#define DOUT17 63
//#define DOUT18 64
//#define DOUT19 65
//#define DOUT20 66
//#define DOUT21 67
//#define DOUT22 68
//#define DOUT23 69
//#define DOUT24 70
//#define DOUT25 71
//#define DOUT26 72
//#define DOUT27 73
//#define DOUT28 74
//#define DOUT29 75
//#define DOUT30 76
//#define DOUT31 77
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
//#define ANALOG1 115
//#define ANALOG2 116
//#define ANALOG3 117
//#define ANALOG4 118
//#define ANALOG5 119
//#define ANALOG6 120
//#define ANALOG7 121
//#define ANALOG8 122
//#define ANALOG9 123
//#define ANALOG10 124
//#define ANALOG11 125
//#define ANALOG12 126
//#define ANALOG13 127
//#define ANALOG14 128
//#define ANALOG15 129
#define DIN0 130
#define DIN1 131
#define DIN2 132
#define DIN3 133
#define DIN4 134
#define DIN5 135
#define DIN6 136
#define DIN7 137
//#define DIN8 138
//#define DIN9 139
//#define DIN10 140
//#define DIN11 141
//#define DIN12 142
//#define DIN13 143
//#define DIN14 144
//#define DIN15 145
//#define DIN16 146
//#define DIN17 147
//#define DIN18 148
//#define DIN19 149
//#define DIN20 150
//#define DIN21 151
//#define DIN22 152
//#define DIN23 153
//#define DIN24 154
//#define DIN25 155
//#define DIN26 156
//#define DIN27 157
//#define DIN28 158
//#define DIN29 159
//#define DIN30 160
//#define DIN31 161
#define TX 200
#define RX 201
#define USB_DM 202
#define USB_DP 203
#define SPI_CLK 204
#define SPI_SDI 205
#define SPI_SDO 206

// just to compile
#define mcu_rx_ready() true

#endif
