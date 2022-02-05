/*
    Name: mcumap_esp8266.h
    Description: Contains all MCU and PIN definitions for Arduino ESP8266 to run µCNC.

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 04-02-2020

    µCNC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version. Please see <http://www.gnu.org/licenses/>

    µCNC is distributed WITHOUT ANY WARRANTY;
    Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the	GNU General Public License for more details.
*/

#ifndef MCUMAP_ESP8266_H
#define MCUMAP_ESP8266_H

#ifdef __cplusplus
extern "C"
{
#endif

/*
    Generates all the interface definitions.
    This creates a middle HAL layer between the board IO pins and the AVR funtionalities
*/
/*
    MCU specific definitions and replacements
*/

/*
    AVR Defaults
*/
// defines the frequency of the mcu
#ifndef F_CPU
#define F_CPU 80000000UL
#endif
// defines the maximum and minimum step rates
#ifndef F_STEP_MAX
#define F_STEP_MAX 100000
#endif
#ifndef F_STEP_MIN
#define F_STEP_MIN 4
#endif

// defines special mcu to access flash strings and arrays
#define __rom__ PROGMEM
#define __romstr__ PSTR
#define rom_strptr pgm_read_byte
#define rom_strcpy strcpy_P
#define rom_strncpy strncpy_P
#define rom_memcpy memcpy_P
#define rom_read_byte pgm_read_byte

#define __SIZEOF_FLOAT__ 4

// used by the parser
// this method is faster then normal multiplication (for 32 bit for 16 and 8 bits is slightly lower)
// overrides utils.h definition to implement this method with or without fast math option enabled
#define fast_int_mul10(x) ((((x) << 2) + (x)) << 1)

#if (defined(STEP0_BIT))
#define DIO0 0
#define STEP0 0
#define DIO0_BIT (STEP0_BIT)
#endif
#if (defined(STEP1_BIT))
#define DIO1 1
#define STEP1 1
#define DIO1_BIT (STEP1_BIT)
#endif
#if (defined(STEP2_BIT))
#define DIO2 2
#define STEP2 2
#define DIO2_BIT (STEP2_BIT)
#endif
#if (defined(STEP3_BIT))
#define DIO3 3
#define STEP3 3
#define DIO3_BIT (STEP3_BIT)
#endif
#if (defined(STEP4_BIT))
#define DIO4 4
#define STEP4 4
#define DIO4_BIT (STEP4_BIT)
#endif
#if (defined(STEP5_BIT))
#define DIO5 5
#define STEP5 5
#define DIO5_BIT (STEP5_BIT)
#endif
#if (defined(STEP6_BIT))
#define DIO6 6
#define STEP6 6
#define DIO6_BIT (STEP6_BIT)
#endif
#if (defined(STEP7_BIT))
#define DIO7 7
#define STEP7 7
#define DIO7_BIT (STEP7_BIT)
#endif
#if (defined(DIR0_BIT))
#define DIO8 8
#define DIR0 8
#define DIO8_BIT (DIR0_BIT)
#endif
#if (defined(DIR1_BIT))
#define DIO9 9
#define DIR1 9
#define DIO9_BIT (DIR1_BIT)
#endif
#if (defined(DIR2_BIT))
#define DIO10 10
#define DIR2 10
#define DIO10_BIT (DIR2_BIT)
#endif
#if (defined(DIR3_BIT))
#define DIO11 11
#define DIR3 11
#define DIO11_BIT (DIR3_BIT)
#endif
#if (defined(DIR4_BIT))
#define DIO12 12
#define DIR4 12
#define DIO12_BIT (DIR4_BIT)
#endif
#if (defined(DIR5_BIT))
#define DIO13 13
#define DIR5 13
#define DIO13_BIT (DIR5_BIT)
#endif
#if (defined(STEP0_EN_BIT))
#define DIO14 14
#define STEP0_EN 14
#define DIO14_BIT (STEP0_EN_BIT)
#endif
#if (defined(STEP1_EN_BIT))
#define DIO15 15
#define STEP1_EN 15
#define DIO15_BIT (STEP1_EN_BIT)
#endif
#if (defined(STEP2_EN_BIT))
#define DIO16 16
#define STEP2_EN 16
#define DIO16_BIT (STEP2_EN_BIT)
#endif
#if (defined(STEP3_EN_BIT))
#define DIO17 17
#define STEP3_EN 17
#define DIO17_BIT (STEP3_EN_BIT)
#endif
#if (defined(STEP4_EN_BIT))
#define DIO18 18
#define STEP4_EN 18
#define DIO18_BIT (STEP4_EN_BIT)
#endif
#if (defined(STEP5_EN_BIT))
#define DIO19 19
#define STEP5_EN 19
#define DIO19_BIT (STEP5_EN_BIT)
#endif
#if (defined(PWM0_BIT))
#define DIO20 20
#define PWM0 20
#define DIO20_BIT (PWM0_BIT)
#endif
#if (defined(PWM1_BIT))
#define DIO21 21
#define PWM1 21
#define DIO21_BIT (PWM1_BIT)
#endif
#if (defined(PWM2_BIT))
#define DIO22 22
#define PWM2 22
#define DIO22_BIT (PWM2_BIT)
#endif
#if (defined(PWM3_BIT))
#define DIO23 23
#define PWM3 23
#define DIO23_BIT (PWM3_BIT)
#endif
#if (defined(PWM4_BIT))
#define DIO24 24
#define PWM4 24
#define DIO24_BIT (PWM4_BIT)
#endif
#if (defined(PWM5_BIT))
#define DIO25 25
#define PWM5 25
#define DIO25_BIT (PWM5_BIT)
#endif
#if (defined(PWM6_BIT))
#define DIO26 26
#define PWM6 26
#define DIO26_BIT (PWM6_BIT)
#endif
#if (defined(PWM7_BIT))
#define DIO27 27
#define PWM7 27
#define DIO27_BIT (PWM7_BIT)
#endif
#if (defined(PWM8_BIT))
#define DIO28 28
#define PWM8 28
#define DIO28_BIT (PWM8_BIT)
#endif
#if (defined(PWM9_BIT))
#define DIO29 29
#define PWM9 29
#define DIO29_BIT (PWM9_BIT)
#endif
#if (defined(PWM10_BIT))
#define DIO30 30
#define PWM10 30
#define DIO30_BIT (PWM10_BIT)
#endif
#if (defined(PWM11_BIT))
#define DIO31 31
#define PWM11 31
#define DIO31_BIT (PWM11_BIT)
#endif
#if (defined(PWM12_BIT))
#define DIO32 32
#define PWM12 32
#define DIO32_BIT (PWM12_BIT)
#endif
#if (defined(PWM13_BIT))
#define DIO33 33
#define PWM13 33
#define DIO33_BIT (PWM13_BIT)
#endif
#if (defined(PWM14_BIT))
#define DIO34 34
#define PWM14 34
#define DIO34_BIT (PWM14_BIT)
#endif
#if (defined(PWM15_BIT))
#define DIO35 35
#define PWM15 35
#define DIO35_BIT (PWM15_BIT)
#endif
#if (defined(DOUT0_BIT))
#define DIO36 36
#define DOUT0 36
#define DIO36_BIT (DOUT0_BIT)
#endif
#if (defined(DOUT1_BIT))
#define DIO37 37
#define DOUT1 37
#define DIO37_BIT (DOUT1_BIT)
#endif
#if (defined(DOUT2_BIT))
#define DIO38 38
#define DOUT2 38
#define DIO38_BIT (DOUT2_BIT)
#endif
#if (defined(DOUT3_BIT))
#define DIO39 39
#define DOUT3 39
#define DIO39_BIT (DOUT3_BIT)
#endif
#if (defined(DOUT4_BIT))
#define DIO40 40
#define DOUT4 40
#define DIO40_BIT (DOUT4_BIT)
#endif
#if (defined(DOUT5_BIT))
#define DIO41 41
#define DOUT5 41
#define DIO41_BIT (DOUT5_BIT)
#endif
#if (defined(DOUT6_BIT))
#define DIO42 42
#define DOUT6 42
#define DIO42_BIT (DOUT6_BIT)
#endif
#if (defined(DOUT7_BIT))
#define DIO43 43
#define DOUT7 43
#define DIO43_BIT (DOUT7_BIT)
#endif
#if (defined(DOUT8_BIT))
#define DIO44 44
#define DOUT8 44
#define DIO44_BIT (DOUT8_BIT)
#endif
#if (defined(DOUT9_BIT))
#define DIO45 45
#define DOUT9 45
#define DIO45_BIT (DOUT9_BIT)
#endif
#if (defined(DOUT10_BIT))
#define DIO46 46
#define DOUT10 46
#define DIO46_BIT (DOUT10_BIT)
#endif
#if (defined(DOUT11_BIT))
#define DIO47 47
#define DOUT11 47
#define DIO47_BIT (DOUT11_BIT)
#endif
#if (defined(DOUT12_BIT))
#define DIO48 48
#define DOUT12 48
#define DIO48_BIT (DOUT12_BIT)
#endif
#if (defined(DOUT13_BIT))
#define DIO49 49
#define DOUT13 49
#define DIO49_BIT (DOUT13_BIT)
#endif
#if (defined(DOUT14_BIT))
#define DIO50 50
#define DOUT14 50
#define DIO50_BIT (DOUT14_BIT)
#endif
#if (defined(DOUT15_BIT))
#define DIO51 51
#define DOUT15 51
#define DIO51_BIT (DOUT15_BIT)
#endif
#if (defined(LIMIT_X_BIT))
#define DIO52 52
#define LIMIT_X 52
#define DIO52_BIT (LIMIT_X_BIT)
#endif
#if (defined(LIMIT_Y_BIT))
#define DIO53 53
#define LIMIT_Y 53
#define DIO53_BIT (LIMIT_Y_BIT)
#endif
#if (defined(LIMIT_Z_BIT))
#define DIO54 54
#define LIMIT_Z 54
#define DIO54_BIT (LIMIT_Z_BIT)
#endif
#if (defined(LIMIT_X2_BIT))
#define DIO55 55
#define LIMIT_X2 55
#define DIO55_BIT (LIMIT_X2_BIT)
#endif
#if (defined(LIMIT_Y2_BIT))
#define DIO56 56
#define LIMIT_Y2 56
#define DIO56_BIT (LIMIT_Y2_BIT)
#endif
#if (defined(LIMIT_Z2_BIT))
#define DIO57 57
#define LIMIT_Z2 57
#define DIO57_BIT (LIMIT_Z2_BIT)
#endif
#if (defined(LIMIT_A_BIT))
#define DIO58 58
#define LIMIT_A 58
#define DIO58_BIT (LIMIT_A_BIT)
#endif
#if (defined(LIMIT_B_BIT))
#define DIO59 59
#define LIMIT_B 59
#define DIO59_BIT (LIMIT_B_BIT)
#endif
#if (defined(LIMIT_C_BIT))
#define DIO60 60
#define LIMIT_C 60
#define DIO60_BIT (LIMIT_C_BIT)
#endif
#if (defined(PROBE_BIT))
#define DIO61 61
#define PROBE 61
#define DIO61_BIT (PROBE_BIT)
#endif
#if (defined(ESTOP_BIT))
#define DIO62 62
#define ESTOP 62
#define DIO62_BIT (ESTOP_BIT)
#endif
#if (defined(SAFETY_DOOR_BIT))
#define DIO63 63
#define SAFETY_DOOR 63
#define DIO63_BIT (SAFETY_DOOR_BIT)
#endif
#if (defined(FHOLD_BIT))
#define DIO64 64
#define FHOLD 64
#define DIO64_BIT (FHOLD_BIT)
#endif
#if (defined(CS_RES_BIT))
#define DIO65 65
#define CS_RES 65
#define DIO65_BIT (CS_RES_BIT)
#endif
#if (defined(ANALOG0_BIT))
#define DIO66 66
#define ANALOG0 66
#define DIO66_BIT (ANALOG0_BIT)
#endif
#if (defined(ANALOG1_BIT))
#define DIO67 67
#define ANALOG1 67
#define DIO67_BIT (ANALOG1_BIT)
#endif
#if (defined(ANALOG2_BIT))
#define DIO68 68
#define ANALOG2 68
#define DIO68_BIT (ANALOG2_BIT)
#endif
#if (defined(ANALOG3_BIT))
#define DIO69 69
#define ANALOG3 69
#define DIO69_BIT (ANALOG3_BIT)
#endif
#if (defined(ANALOG4_BIT))
#define DIO70 70
#define ANALOG4 70
#define DIO70_BIT (ANALOG4_BIT)
#endif
#if (defined(ANALOG5_BIT))
#define DIO71 71
#define ANALOG5 71
#define DIO71_BIT (ANALOG5_BIT)
#endif
#if (defined(ANALOG6_BIT))
#define DIO72 72
#define ANALOG6 72
#define DIO72_BIT (ANALOG6_BIT)
#endif
#if (defined(ANALOG7_BIT))
#define DIO73 73
#define ANALOG7 73
#define DIO73_BIT (ANALOG7_BIT)
#endif
#if (defined(ANALOG8_BIT))
#define DIO74 74
#define ANALOG8 74
#define DIO74_BIT (ANALOG8_BIT)
#endif
#if (defined(ANALOG9_BIT))
#define DIO75 75
#define ANALOG9 75
#define DIO75_BIT (ANALOG9_BIT)
#endif
#if (defined(ANALOG10_BIT))
#define DIO76 76
#define ANALOG10 76
#define DIO76_BIT (ANALOG10_BIT)
#endif
#if (defined(ANALOG11_BIT))
#define DIO77 77
#define ANALOG11 77
#define DIO77_BIT (ANALOG11_BIT)
#endif
#if (defined(ANALOG12_BIT))
#define DIO78 78
#define ANALOG12 78
#define DIO78_BIT (ANALOG12_BIT)
#endif
#if (defined(ANALOG13_BIT))
#define DIO79 79
#define ANALOG13 79
#define DIO79_BIT (ANALOG13_BIT)
#endif
#if (defined(ANALOG14_BIT))
#define DIO80 80
#define ANALOG14 80
#define DIO80_BIT (ANALOG14_BIT)
#endif
#if (defined(ANALOG15_BIT))
#define DIO81 81
#define ANALOG15 81
#define DIO81_BIT (ANALOG15_BIT)
#endif
#if (defined(DIN0_BIT))
#define DIO82 82
#define DIN0 82
#define DIO82_BIT (DIN0_BIT)
#endif
#if (defined(DIN1_BIT))
#define DIO83 83
#define DIN1 83
#define DIO83_BIT (DIN1_BIT)
#endif
#if (defined(DIN2_BIT))
#define DIO84 84
#define DIN2 84
#define DIO84_BIT (DIN2_BIT)
#endif
#if (defined(DIN3_BIT))
#define DIO85 85
#define DIN3 85
#define DIO85_BIT (DIN3_BIT)
#endif
#if (defined(DIN4_BIT))
#define DIO86 86
#define DIN4 86
#define DIO86_BIT (DIN4_BIT)
#endif
#if (defined(DIN5_BIT))
#define DIO87 87
#define DIN5 87
#define DIO87_BIT (DIN5_BIT)
#endif
#if (defined(DIN6_BIT))
#define DIO88 88
#define DIN6 88
#define DIO88_BIT (DIN6_BIT)
#endif
#if (defined(DIN7_BIT))
#define DIO89 89
#define DIN7 89
#define DIO89_BIT (DIN7_BIT)
#endif
#if (defined(DIN8_BIT))
#define DIO90 90
#define DIN8 90
#define DIO90_BIT (DIN8_BIT)
#endif
#if (defined(DIN9_BIT))
#define DIO91 91
#define DIN9 91
#define DIO91_BIT (DIN9_BIT)
#endif
#if (defined(DIN10_BIT))
#define DIO92 92
#define DIN10 92
#define DIO92_BIT (DIN10_BIT)
#endif
#if (defined(DIN11_BIT))
#define DIO93 93
#define DIN11 93
#define DIO93_BIT (DIN11_BIT)
#endif
#if (defined(DIN12_BIT))
#define DIO94 94
#define DIN12 94
#define DIO94_BIT (DIN12_BIT)
#endif
#if (defined(DIN13_BIT))
#define DIO95 95
#define DIN13 95
#define DIO95_BIT (DIN13_BIT)
#endif
#if (defined(DIN14_BIT))
#define DIO96 96
#define DIN14 96
#define DIO96_BIT (DIN14_BIT)
#endif
#if (defined(DIN15_BIT))
#define DIO97 97
#define DIN15 97
#define DIO97_BIT (DIN15_BIT)
#endif
#if (defined(TX_BIT))
#define DIO98 98
#define TX 98
#define DIO98_BIT (TX_BIT)
#endif
#if (defined(RX_BIT))
#define DIO99 99
#define RX 99
#define DIO99_BIT (RX_BIT)
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

#define mcu_config_output(X) pinMode(__helper__(X, _BIT, ), OUTPUT)
#define mcu_config_pwm(X) analogWriteMode(__helper__(X, _BIT, ), 0, OUTPUT_OPEN_DRAIN)
#define mcu_config_pwm(X) analogWriteMode(__helper__(X, _BIT, ), 0, OUTPUT_OPEN_DRAIN)
#define mcu_config_input(X) pinMode(__helper__(X, _BIT, ), INPUT)
#define mcu_config_pullup(X) pinMode(__helper__(X, _BIT, ), INPUT_PULLUP)
#define mcu_config_input_isr(X) attachInterrupt(digitalPinToInterrupt(__helper__(X, _BIT, )), __helper__(X, _ISRCALLBACK, ), CHANGE)

#define mcu_get_input(X) digitalRead(__helper__(X, _BIT, ))
#define mcu_get_output(X) digitalRead(__helper__(X, _BIT, ))
#define mcu_set_output(X) digitalWrite(__helper__(X, _BIT, ), 1)
#define mcu_clear_output(X) digitalWrite(__helper__(X, _BIT, ), 0)
#define mcu_toggle_output(X) digitalWrite(__helper__(X, _BIT, ), !digitalRead(__helper__(X, _BIT, )))

#ifdef __cplusplus
}
#endif

#endif
