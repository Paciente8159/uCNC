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
#define F_STEP_MAX 40000
#define F_STEP_MIN 1
#define __rom__
#define __romstr__
#define __romarr__ const char
#define rom_strptr *
#define rom_strcpy strcpy
#define rom_strncpy strncpy
#define rom_memcpy memcpy
#define rom_read_byte *

#define ENABLE_SYNC_TX

//uncomment to use sockets
//#define USESOCKETS
#ifdef USESOCKETS
#define DEFAULT_BUFLEN 127
#define DEFAULT_PORT "34000"
#endif

//uncomment to use serial port
#define USESERIAL
#ifdef USESERIAL
#ifndef COMPORT
#define COMPORT "\\\\.\\COM11"
#endif
#endif

//#define USECONSOLE

//joints step/dir pins
#define STEP0 0
#define STEP1 1
#define STEP2 2
#define STEP3 3
#define STEP4 4
#define STEP5 5
#define STEP6 6
#define STEP7 7
#define DIR0 8
#define DIR1 9
#define DIR2 10
#define DIR3 11
#define DIR4 12
#define DIR5 13
#define STEP0_EN 14
#define STEP1_EN 15
#define STEP2_EN 16
#define STEP3_EN 17
#define STEP4_EN 18
#define STEP5_EN 19
#define PWM0 20
#define PWM1 21
#define PWM2 22
#define PWM3 23
#define PWM4 24
#define PWM5 25
#define PWM6 26
#define PWM7 27
#define PWM8 28
#define PWM9 29
#define PWM10 30
#define PWM11 31
#define PWM12 32
#define PWM13 33
#define PWM14 34
#define PWM15 35
#define DOUT0 36
#define DOUT1 37
#define DOUT2 38
#define DOUT3 39
#define DOUT4 40
#define DOUT5 41
#define DOUT6 42
#define DOUT7 43
#define DOUT8 44
#define DOUT9 45
#define DOUT10 46
#define DOUT11 47
#define DOUT12 48
#define DOUT13 49
#define DOUT14 50
#define DOUT15 51
#define LIMIT_X 52
#define LIMIT_Y 53
#define LIMIT_Z 54
#define LIMIT_X2 55
#define LIMIT_Y2 56
#define LIMIT_Z2 57
#define LIMIT_A 58
#define LIMIT_B 59
#define LIMIT_C 60
#define PROBE 61
#define ESTOP 62
#define SAFETY_DOOR 63
#define FHOLD 64
#define CS_RES 65
#define ANALOG0 66
#define ANALOG1 67
#define ANALOG2 68
#define ANALOG3 69
#define ANALOG4 70
#define ANALOG5 71
#define ANALOG6 72
#define ANALOG7 73
#define ANALOG8 74
#define ANALOG9 75
#define ANALOG10 76
#define ANALOG11 77
#define ANALOG12 78
#define ANALOG13 79
#define ANALOG14 80
#define ANALOG15 81
#define DIN0 82
#define DIN1 83
#define DIN2 84
#define DIN3 85
#define DIN4 86
#define DIN5 87
#define DIN6 88
#define DIN7 89
#define DIN8 90
#define DIN9 91
#define DIN10 92
#define DIN11 93
#define DIN12 94
#define DIN13 95
#define DIN14 96
#define DIN15 97

//just to compile
#define mcu_rx_ready() true

#endif
