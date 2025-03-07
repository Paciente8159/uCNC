/*
	Name: mcu.c
	Description: Contains common MCU (agnostic) functions.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 26/08/2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"

#ifdef MCU_HAS_ONESHOT_TIMER
MCU_CALLBACK mcu_timeout_delgate mcu_timeout_cb;
#endif

// most MCU can perform some sort of loop within 4 to 6 CPU cycles + a small function call overhead
// the amount of cycles per loop and overhead can be tuned with a scope or by inspecting the produced asm
// and adjusted in each MCU
// by adding the noinline and the empty asm instruction prevents optimization from removing the code executing the loop
// if used inside atomic operations can execute delays with good precision in any MCU
__attribute__((noinline, optimize("O3"))) void mcu_delay_loop(uint16_t loops)
{
	do
	{
		asm volatile("");
	} while (--loops);
}

void __attribute__((weak)) mcu_io_init(void)
{
#if ASSERT_PIN_IO(STEP0)
	mcu_config_output(STEP0);
#endif
#if ASSERT_PIN_IO(STEP1)
	mcu_config_output(STEP1);
#endif
#if ASSERT_PIN_IO(STEP2)
	mcu_config_output(STEP2);
#endif
#if ASSERT_PIN_IO(STEP3)
	mcu_config_output(STEP3);
#endif
#if ASSERT_PIN_IO(STEP4)
	mcu_config_output(STEP4);
#endif
#if ASSERT_PIN_IO(STEP5)
	mcu_config_output(STEP5);
#endif
#if ASSERT_PIN_IO(STEP6)
	mcu_config_output(STEP6);
#endif
#if ASSERT_PIN_IO(STEP7)
	mcu_config_output(STEP7);
#endif
#if ASSERT_PIN_IO(DIR0)
	mcu_config_output(DIR0);
#endif
#if ASSERT_PIN_IO(DIR1)
	mcu_config_output(DIR1);
#endif
#if ASSERT_PIN_IO(DIR2)
	mcu_config_output(DIR2);
#endif
#if ASSERT_PIN_IO(DIR3)
	mcu_config_output(DIR3);
#endif
#if ASSERT_PIN_IO(DIR4)
	mcu_config_output(DIR4);
#endif
#if ASSERT_PIN_IO(DIR5)
	mcu_config_output(DIR5);
#endif
#if ASSERT_PIN_IO(DIR6)
	mcu_config_output(DIR6);
#endif
#if ASSERT_PIN_IO(DIR7)
	mcu_config_output(DIR7);
#endif
#if ASSERT_PIN_IO(STEP0_EN)
	mcu_config_output(STEP0_EN);
#endif
#if ASSERT_PIN_IO(STEP1_EN)
	mcu_config_output(STEP1_EN);
#endif
#if ASSERT_PIN_IO(STEP2_EN)
	mcu_config_output(STEP2_EN);
#endif
#if ASSERT_PIN_IO(STEP3_EN)
	mcu_config_output(STEP3_EN);
#endif
#if ASSERT_PIN_IO(STEP4_EN)
	mcu_config_output(STEP4_EN);
#endif
#if ASSERT_PIN_IO(STEP5_EN)
	mcu_config_output(STEP5_EN);
#endif
#if ASSERT_PIN_IO(STEP6_EN)
	mcu_config_output(STEP6_EN);
#endif
#if ASSERT_PIN_IO(STEP7_EN)
	mcu_config_output(STEP7_EN);
#endif
#if ASSERT_PIN_IO(PWM0)
	mcu_config_pwm(PWM0, 1000);
#endif
#if ASSERT_PIN_IO(PWM1)
	mcu_config_pwm(PWM1, 1000);
#endif
#if ASSERT_PIN_IO(PWM2)
	mcu_config_pwm(PWM2, 1000);
#endif
#if ASSERT_PIN_IO(PWM3)
	mcu_config_pwm(PWM3, 1000);
#endif
#if ASSERT_PIN_IO(PWM4)
	mcu_config_pwm(PWM4, 1000);
#endif
#if ASSERT_PIN_IO(PWM5)
	mcu_config_pwm(PWM5, 1000);
#endif
#if ASSERT_PIN_IO(PWM6)
	mcu_config_pwm(PWM6, 1000);
#endif
#if ASSERT_PIN_IO(PWM7)
	mcu_config_pwm(PWM7, 1000);
#endif
#if ASSERT_PIN_IO(PWM8)
	mcu_config_pwm(PWM8, 1000);
#endif
#if ASSERT_PIN_IO(PWM9)
	mcu_config_pwm(PWM9, 1000);
#endif
#if ASSERT_PIN_IO(PWM10)
	mcu_config_pwm(PWM10, 1000);
#endif
#if ASSERT_PIN_IO(PWM11)
	mcu_config_pwm(PWM11, 1000);
#endif
#if ASSERT_PIN_IO(PWM12)
	mcu_config_pwm(PWM12, 1000);
#endif
#if ASSERT_PIN_IO(PWM13)
	mcu_config_pwm(PWM13, 1000);
#endif
#if ASSERT_PIN_IO(PWM14)
	mcu_config_pwm(PWM14, 1000);
#endif
#if ASSERT_PIN_IO(PWM15)
	mcu_config_pwm(PWM15, 1000);
#endif
#if ASSERT_PIN_IO(SERVO0)
	mcu_config_output(SERVO0);
#endif
#if ASSERT_PIN_IO(SERVO1)
	mcu_config_output(SERVO1);
#endif
#if ASSERT_PIN_IO(SERVO2)
	mcu_config_output(SERVO2);
#endif
#if ASSERT_PIN_IO(SERVO3)
	mcu_config_output(SERVO3);
#endif
#if ASSERT_PIN_IO(SERVO4)
	mcu_config_output(SERVO4);
#endif
#if ASSERT_PIN_IO(SERVO5)
	mcu_config_output(SERVO5);
#endif
#if ASSERT_PIN_IO(DOUT0)
	mcu_config_output(DOUT0);
#endif
#if ASSERT_PIN_IO(DOUT1)
	mcu_config_output(DOUT1);
#endif
#if ASSERT_PIN_IO(DOUT2)
	mcu_config_output(DOUT2);
#endif
#if ASSERT_PIN_IO(DOUT3)
	mcu_config_output(DOUT3);
#endif
#if ASSERT_PIN_IO(DOUT4)
	mcu_config_output(DOUT4);
#endif
#if ASSERT_PIN_IO(DOUT5)
	mcu_config_output(DOUT5);
#endif
#if ASSERT_PIN_IO(DOUT6)
	mcu_config_output(DOUT6);
#endif
#if ASSERT_PIN_IO(DOUT7)
	mcu_config_output(DOUT7);
#endif
#if ASSERT_PIN_IO(DOUT8)
	mcu_config_output(DOUT8);
#endif
#if ASSERT_PIN_IO(DOUT9)
	mcu_config_output(DOUT9);
#endif
#if ASSERT_PIN_IO(DOUT10)
	mcu_config_output(DOUT10);
#endif
#if ASSERT_PIN_IO(DOUT11)
	mcu_config_output(DOUT11);
#endif
#if ASSERT_PIN_IO(DOUT12)
	mcu_config_output(DOUT12);
#endif
#if ASSERT_PIN_IO(DOUT13)
	mcu_config_output(DOUT13);
#endif
#if ASSERT_PIN_IO(DOUT14)
	mcu_config_output(DOUT14);
#endif
#if ASSERT_PIN_IO(DOUT15)
	mcu_config_output(DOUT15);
#endif
#if ASSERT_PIN_IO(DOUT16)
	mcu_config_output(DOUT16);
#endif
#if ASSERT_PIN_IO(DOUT17)
	mcu_config_output(DOUT17);
#endif
#if ASSERT_PIN_IO(DOUT18)
	mcu_config_output(DOUT18);
#endif
#if ASSERT_PIN_IO(DOUT19)
	mcu_config_output(DOUT19);
#endif
#if ASSERT_PIN_IO(DOUT20)
	mcu_config_output(DOUT20);
#endif
#if ASSERT_PIN_IO(DOUT21)
	mcu_config_output(DOUT21);
#endif
#if ASSERT_PIN_IO(DOUT22)
	mcu_config_output(DOUT22);
#endif
#if ASSERT_PIN_IO(DOUT23)
	mcu_config_output(DOUT23);
#endif
#if ASSERT_PIN_IO(DOUT24)
	mcu_config_output(DOUT24);
#endif
#if ASSERT_PIN_IO(DOUT25)
	mcu_config_output(DOUT25);
#endif
#if ASSERT_PIN_IO(DOUT26)
	mcu_config_output(DOUT26);
#endif
#if ASSERT_PIN_IO(DOUT27)
	mcu_config_output(DOUT27);
#endif
#if ASSERT_PIN_IO(DOUT28)
	mcu_config_output(DOUT28);
#endif
#if ASSERT_PIN_IO(DOUT29)
	mcu_config_output(DOUT29);
#endif
#if ASSERT_PIN_IO(DOUT30)
	mcu_config_output(DOUT30);
#endif
#if ASSERT_PIN_IO(DOUT31)
	mcu_config_output(DOUT31);
#endif
#if ASSERT_PIN_IO(DOUT32)
	mcu_config_output(DOUT32);
#endif
#if ASSERT_PIN_IO(DOUT33)
	mcu_config_output(DOUT33);
#endif
#if ASSERT_PIN_IO(DOUT34)
	mcu_config_output(DOUT34);
#endif
#if ASSERT_PIN_IO(DOUT35)
	mcu_config_output(DOUT35);
#endif
#if ASSERT_PIN_IO(DOUT36)
	mcu_config_output(DOUT36);
#endif
#if ASSERT_PIN_IO(DOUT37)
	mcu_config_output(DOUT37);
#endif
#if ASSERT_PIN_IO(DOUT38)
	mcu_config_output(DOUT38);
#endif
#if ASSERT_PIN_IO(DOUT39)
	mcu_config_output(DOUT39);
#endif
#if ASSERT_PIN_IO(DOUT40)
	mcu_config_output(DOUT40);
#endif
#if ASSERT_PIN_IO(DOUT41)
	mcu_config_output(DOUT41);
#endif
#if ASSERT_PIN_IO(DOUT42)
	mcu_config_output(DOUT42);
#endif
#if ASSERT_PIN_IO(DOUT43)
	mcu_config_output(DOUT43);
#endif
#if ASSERT_PIN_IO(DOUT44)
	mcu_config_output(DOUT44);
#endif
#if ASSERT_PIN_IO(DOUT45)
	mcu_config_output(DOUT45);
#endif
#if ASSERT_PIN_IO(DOUT46)
	mcu_config_output(DOUT46);
#endif
#if ASSERT_PIN_IO(DOUT47)
	mcu_config_output(DOUT47);
#endif
#if ASSERT_PIN_IO(DOUT48)
	mcu_config_output(DOUT48);
#endif
#if ASSERT_PIN_IO(DOUT49)
	mcu_config_output(DOUT49);
#endif
#if ASSERT_PIN_IO(LIMIT_X)
	mcu_config_input(LIMIT_X);
#ifdef LIMIT_X_PULLUP
	mcu_config_pullup(LIMIT_X);
#endif
#ifdef LIMIT_X_ISR
	mcu_config_input_isr(LIMIT_X);
#endif
#endif
#if ASSERT_PIN_IO(LIMIT_Y)
	mcu_config_input(LIMIT_Y);
#ifdef LIMIT_Y_PULLUP
	mcu_config_pullup(LIMIT_Y);
#endif
#ifdef LIMIT_Y_ISR
	mcu_config_input_isr(LIMIT_Y);
#endif
#endif
#if ASSERT_PIN_IO(LIMIT_Z)
	mcu_config_input(LIMIT_Z);
#ifdef LIMIT_Z_PULLUP
	mcu_config_pullup(LIMIT_Z);
#endif
#ifdef LIMIT_Z_ISR
	mcu_config_input_isr(LIMIT_Z);
#endif
#endif
#if ASSERT_PIN_IO(LIMIT_X2)
	mcu_config_input(LIMIT_X2);
#ifdef LIMIT_X2_PULLUP
	mcu_config_pullup(LIMIT_X2);
#endif
#ifdef LIMIT_X2_ISR
	mcu_config_input_isr(LIMIT_X2);
#endif
#endif
#if ASSERT_PIN_IO(LIMIT_Y2)
	mcu_config_input(LIMIT_Y2);
#ifdef LIMIT_Y2_PULLUP
	mcu_config_pullup(LIMIT_Y2);
#endif
#ifdef LIMIT_Y2_ISR
	mcu_config_input_isr(LIMIT_Y2);
#endif
#endif
#if ASSERT_PIN_IO(LIMIT_Z2)
	mcu_config_input(LIMIT_Z2);
#ifdef LIMIT_Z2_PULLUP
	mcu_config_pullup(LIMIT_Z2);
#endif
#ifdef LIMIT_Z2_ISR
	mcu_config_input_isr(LIMIT_Z2);
#endif
#endif
#if ASSERT_PIN_IO(LIMIT_A)
	mcu_config_input(LIMIT_A);
#ifdef LIMIT_A_PULLUP
	mcu_config_pullup(LIMIT_A);
#endif
#ifdef LIMIT_A_ISR
	mcu_config_input_isr(LIMIT_A);
#endif
#endif
#if ASSERT_PIN_IO(LIMIT_B)
	mcu_config_input(LIMIT_B);
#ifdef LIMIT_B_PULLUP
	mcu_config_pullup(LIMIT_B);
#endif
#ifdef LIMIT_B_ISR
	mcu_config_input_isr(LIMIT_B);
#endif
#endif
#if ASSERT_PIN_IO(LIMIT_C)
	mcu_config_input(LIMIT_C);
#ifdef LIMIT_C_PULLUP
	mcu_config_pullup(LIMIT_C);
#endif
#ifdef LIMIT_C_ISR
	mcu_config_input_isr(LIMIT_C);
#endif
#endif
#if ASSERT_PIN_IO(PROBE)
	mcu_config_input(PROBE);
#ifdef PROBE_PULLUP
	mcu_config_pullup(PROBE);
#endif
#ifdef PROBE_ISR
	mcu_config_input_isr(PROBE);
#endif
#endif
#if ASSERT_PIN_IO(ESTOP)
	mcu_config_input(ESTOP);
#ifdef ESTOP_PULLUP
	mcu_config_pullup(ESTOP);
#endif
#ifdef ESTOP_ISR
	mcu_config_input_isr(ESTOP);
#endif
#endif
#if ASSERT_PIN_IO(SAFETY_DOOR)
	mcu_config_input(SAFETY_DOOR);
#ifdef SAFETY_DOOR_PULLUP
	mcu_config_pullup(SAFETY_DOOR);
#endif
#ifdef SAFETY_DOOR_ISR
	mcu_config_input_isr(SAFETY_DOOR);
#endif
#endif
#if ASSERT_PIN_IO(FHOLD)
	mcu_config_input(FHOLD);
#ifdef FHOLD_PULLUP
	mcu_config_pullup(FHOLD);
#endif
#ifdef FHOLD_ISR
	mcu_config_input_isr(FHOLD);
#endif
#endif
#if ASSERT_PIN_IO(CS_RES)
	mcu_config_input(CS_RES);
#ifdef CS_RES_PULLUP
	mcu_config_pullup(CS_RES);
#endif
#ifdef CS_RES_ISR
	mcu_config_input_isr(CS_RES);
#endif
#endif
#if ASSERT_PIN_IO(ANALOG0)
	mcu_config_analog(ANALOG0);
#endif
#if ASSERT_PIN_IO(ANALOG1)
	mcu_config_analog(ANALOG1);
#endif
#if ASSERT_PIN_IO(ANALOG2)
	mcu_config_analog(ANALOG2);
#endif
#if ASSERT_PIN_IO(ANALOG3)
	mcu_config_analog(ANALOG3);
#endif
#if ASSERT_PIN_IO(ANALOG4)
	mcu_config_analog(ANALOG4);
#endif
#if ASSERT_PIN_IO(ANALOG5)
	mcu_config_analog(ANALOG5);
#endif
#if ASSERT_PIN_IO(ANALOG6)
	mcu_config_analog(ANALOG6);
#endif
#if ASSERT_PIN_IO(ANALOG7)
	mcu_config_analog(ANALOG7);
#endif
#if ASSERT_PIN_IO(ANALOG8)
	mcu_config_analog(ANALOG8);
#endif
#if ASSERT_PIN_IO(ANALOG9)
	mcu_config_analog(ANALOG9);
#endif
#if ASSERT_PIN_IO(ANALOG10)
	mcu_config_analog(ANALOG10);
#endif
#if ASSERT_PIN_IO(ANALOG11)
	mcu_config_analog(ANALOG11);
#endif
#if ASSERT_PIN_IO(ANALOG12)
	mcu_config_analog(ANALOG12);
#endif
#if ASSERT_PIN_IO(ANALOG13)
	mcu_config_analog(ANALOG13);
#endif
#if ASSERT_PIN_IO(ANALOG14)
	mcu_config_analog(ANALOG14);
#endif
#if ASSERT_PIN_IO(ANALOG15)
	mcu_config_analog(ANALOG15);
#endif
#if ASSERT_PIN_IO(DIN0)
	mcu_config_input(DIN0);
#ifdef DIN0_PULLUP
	mcu_config_pullup(DIN0);
#endif
#ifdef DIN0_ISR
	mcu_config_input_isr(DIN0);
#endif
#endif
#if ASSERT_PIN_IO(DIN1)
	mcu_config_input(DIN1);
#ifdef DIN1_PULLUP
	mcu_config_pullup(DIN1);
#endif
#ifdef DIN1_ISR
	mcu_config_input_isr(DIN1);
#endif
#endif
#if ASSERT_PIN_IO(DIN2)
	mcu_config_input(DIN2);
#ifdef DIN2_PULLUP
	mcu_config_pullup(DIN2);
#endif
#ifdef DIN2_ISR
	mcu_config_input_isr(DIN2);
#endif
#endif
#if ASSERT_PIN_IO(DIN3)
	mcu_config_input(DIN3);
#ifdef DIN3_PULLUP
	mcu_config_pullup(DIN3);
#endif
#ifdef DIN3_ISR
	mcu_config_input_isr(DIN3);
#endif
#endif
#if ASSERT_PIN_IO(DIN4)
	mcu_config_input(DIN4);
#ifdef DIN4_PULLUP
	mcu_config_pullup(DIN4);
#endif
#ifdef DIN4_ISR
	mcu_config_input_isr(DIN4);
#endif
#endif
#if ASSERT_PIN_IO(DIN5)
	mcu_config_input(DIN5);
#ifdef DIN5_PULLUP
	mcu_config_pullup(DIN5);
#endif
#ifdef DIN5_ISR
	mcu_config_input_isr(DIN5);
#endif
#endif
#if ASSERT_PIN_IO(DIN6)
	mcu_config_input(DIN6);
#ifdef DIN6_PULLUP
	mcu_config_pullup(DIN6);
#endif
#ifdef DIN6_ISR
	mcu_config_input_isr(DIN6);
#endif
#endif
#if ASSERT_PIN_IO(DIN7)
	mcu_config_input(DIN7);
#ifdef DIN7_PULLUP
	mcu_config_pullup(DIN7);
#endif
#ifdef DIN7_ISR
	mcu_config_input_isr(DIN7);
#endif
#endif
#if ASSERT_PIN_IO(DIN8)
	mcu_config_input(DIN8);
#ifdef DIN8_PULLUP
	mcu_config_pullup(DIN8);
#endif
#endif
#if ASSERT_PIN_IO(DIN9)
	mcu_config_input(DIN9);
#ifdef DIN9_PULLUP
	mcu_config_pullup(DIN9);
#endif
#endif
#if ASSERT_PIN_IO(DIN10)
	mcu_config_input(DIN10);
#ifdef DIN10_PULLUP
	mcu_config_pullup(DIN10);
#endif
#endif
#if ASSERT_PIN_IO(DIN11)
	mcu_config_input(DIN11);
#ifdef DIN11_PULLUP
	mcu_config_pullup(DIN11);
#endif
#endif
#if ASSERT_PIN_IO(DIN12)
	mcu_config_input(DIN12);
#ifdef DIN12_PULLUP
	mcu_config_pullup(DIN12);
#endif
#endif
#if ASSERT_PIN_IO(DIN13)
	mcu_config_input(DIN13);
#ifdef DIN13_PULLUP
	mcu_config_pullup(DIN13);
#endif
#endif
#if ASSERT_PIN_IO(DIN14)
	mcu_config_input(DIN14);
#ifdef DIN14_PULLUP
	mcu_config_pullup(DIN14);
#endif
#endif
#if ASSERT_PIN_IO(DIN15)
	mcu_config_input(DIN15);
#ifdef DIN15_PULLUP
	mcu_config_pullup(DIN15);
#endif
#endif
#if ASSERT_PIN_IO(DIN16)
	mcu_config_input(DIN16);
#ifdef DIN16_PULLUP
	mcu_config_pullup(DIN16);
#endif
#endif
#if ASSERT_PIN_IO(DIN17)
	mcu_config_input(DIN17);
#ifdef DIN17_PULLUP
	mcu_config_pullup(DIN17);
#endif
#endif
#if ASSERT_PIN_IO(DIN18)
	mcu_config_input(DIN18);
#ifdef DIN18_PULLUP
	mcu_config_pullup(DIN18);
#endif
#endif
#if ASSERT_PIN_IO(DIN19)
	mcu_config_input(DIN19);
#ifdef DIN19_PULLUP
	mcu_config_pullup(DIN19);
#endif
#endif
#if ASSERT_PIN_IO(DIN20)
	mcu_config_input(DIN20);
#ifdef DIN20_PULLUP
	mcu_config_pullup(DIN20);
#endif
#endif
#if ASSERT_PIN_IO(DIN21)
	mcu_config_input(DIN21);
#ifdef DIN21_PULLUP
	mcu_config_pullup(DIN21);
#endif
#endif
#if ASSERT_PIN_IO(DIN22)
	mcu_config_input(DIN22);
#ifdef DIN22_PULLUP
	mcu_config_pullup(DIN22);
#endif
#endif
#if ASSERT_PIN_IO(DIN23)
	mcu_config_input(DIN23);
#ifdef DIN23_PULLUP
	mcu_config_pullup(DIN23);
#endif
#endif
#if ASSERT_PIN_IO(DIN24)
	mcu_config_input(DIN24);
#ifdef DIN24_PULLUP
	mcu_config_pullup(DIN24);
#endif
#endif
#if ASSERT_PIN_IO(DIN25)
	mcu_config_input(DIN25);
#ifdef DIN25_PULLUP
	mcu_config_pullup(DIN25);
#endif
#endif
#if ASSERT_PIN_IO(DIN26)
	mcu_config_input(DIN26);
#ifdef DIN26_PULLUP
	mcu_config_pullup(DIN26);
#endif
#endif
#if ASSERT_PIN_IO(DIN27)
	mcu_config_input(DIN27);
#ifdef DIN27_PULLUP
	mcu_config_pullup(DIN27);
#endif
#endif
#if ASSERT_PIN_IO(DIN28)
	mcu_config_input(DIN28);
#ifdef DIN28_PULLUP
	mcu_config_pullup(DIN28);
#endif
#endif
#if ASSERT_PIN_IO(DIN29)
	mcu_config_input(DIN29);
#ifdef DIN29_PULLUP
	mcu_config_pullup(DIN29);
#endif
#endif
#if ASSERT_PIN_IO(DIN30)
	mcu_config_input(DIN30);
#ifdef DIN30_PULLUP
	mcu_config_pullup(DIN30);
#endif
#endif
#if ASSERT_PIN_IO(DIN31)
	mcu_config_input(DIN31);
#ifdef DIN31_PULLUP
	mcu_config_pullup(DIN31);
#endif
#endif
#if ASSERT_PIN_IO(DIN32)
	mcu_config_input(DIN32);
#ifdef DIN32_PULLUP
	mcu_config_pullup(DIN32);
#endif
#endif
#if ASSERT_PIN_IO(DIN33)
	mcu_config_input(DIN33);
#ifdef DIN33_PULLUP
	mcu_config_pullup(DIN33);
#endif
#endif
#if ASSERT_PIN_IO(DIN34)
	mcu_config_input(DIN34);
#ifdef DIN34_PULLUP
	mcu_config_pullup(DIN34);
#endif
#endif
#if ASSERT_PIN_IO(DIN35)
	mcu_config_input(DIN35);
#ifdef DIN35_PULLUP
	mcu_config_pullup(DIN35);
#endif
#endif
#if ASSERT_PIN_IO(DIN36)
	mcu_config_input(DIN36);
#ifdef DIN36_PULLUP
	mcu_config_pullup(DIN36);
#endif
#endif
#if ASSERT_PIN_IO(DIN37)
	mcu_config_input(DIN37);
#ifdef DIN37_PULLUP
	mcu_config_pullup(DIN37);
#endif
#endif
#if ASSERT_PIN_IO(DIN38)
	mcu_config_input(DIN38);
#ifdef DIN38_PULLUP
	mcu_config_pullup(DIN38);
#endif
#endif
#if ASSERT_PIN_IO(DIN39)
	mcu_config_input(DIN39);
#ifdef DIN39_PULLUP
	mcu_config_pullup(DIN39);
#endif
#endif
#if ASSERT_PIN_IO(DIN40)
	mcu_config_input(DIN40);
#ifdef DIN40_PULLUP
	mcu_config_pullup(DIN40);
#endif
#endif
#if ASSERT_PIN_IO(DIN41)
	mcu_config_input(DIN41);
#ifdef DIN41_PULLUP
	mcu_config_pullup(DIN41);
#endif
#endif
#if ASSERT_PIN_IO(DIN42)
	mcu_config_input(DIN42);
#ifdef DIN42_PULLUP
	mcu_config_pullup(DIN42);
#endif
#endif
#if ASSERT_PIN_IO(DIN43)
	mcu_config_input(DIN43);
#ifdef DIN43_PULLUP
	mcu_config_pullup(DIN43);
#endif
#endif
#if ASSERT_PIN_IO(DIN44)
	mcu_config_input(DIN44);
#ifdef DIN44_PULLUP
	mcu_config_pullup(DIN44);
#endif
#endif
#if ASSERT_PIN_IO(DIN45)
	mcu_config_input(DIN45);
#ifdef DIN45_PULLUP
	mcu_config_pullup(DIN45);
#endif
#endif
#if ASSERT_PIN_IO(DIN46)
	mcu_config_input(DIN46);
#ifdef DIN46_PULLUP
	mcu_config_pullup(DIN46);
#endif
#endif
#if ASSERT_PIN_IO(DIN47)
	mcu_config_input(DIN47);
#ifdef DIN47_PULLUP
	mcu_config_pullup(DIN47);
#endif
#endif
#if ASSERT_PIN_IO(DIN48)
	mcu_config_input(DIN48);
#ifdef DIN48_PULLUP
	mcu_config_pullup(DIN48);
#endif
#endif
#if ASSERT_PIN_IO(DIN49)
	mcu_config_input(DIN49);
#ifdef DIN49_PULLUP
	mcu_config_pullup(DIN49);
#endif
#endif
#if ASSERT_PIN_IO(TX)
	mcu_config_output(TX);
#endif
#if ASSERT_PIN_IO(RX)
	mcu_config_input(RX);
#ifdef RX_PULLUP
	mcu_config_pullup(RX);
#endif
#endif
#if ASSERT_PIN_IO(USB_DM)
	mcu_config_input(USB_DM);
#ifdef USB_DM_PULLUP
	mcu_config_pullup(USB_DM);
#endif
#endif
#if ASSERT_PIN_IO(USB_DP)
	mcu_config_input(USB_DP);
#ifdef USB_DP_PULLUP
	mcu_config_pullup(USB_DP);
#endif
#endif
#if ASSERT_PIN_IO(SPI_CLK)
	mcu_config_output(SPI_CLK);
#endif
#if ASSERT_PIN_IO(SPI_SDI)
	mcu_config_input(SPI_SDI);
#ifdef SPI_SDI_PULLUP
	mcu_config_pullup(SPI_SDI);
#endif
#endif
#if ASSERT_PIN_IO(SPI_SDO)
	mcu_config_output(SPI_SDO);
#endif
#if ASSERT_PIN(SPI_CS)
	mcu_config_output(SPI_CS);
#endif
#if ASSERT_PIN_IO(I2C_CLK)
	mcu_config_input(I2C_CLK);
	mcu_config_pullup(I2C_CLK);
#endif
#if ASSERT_PIN_IO(I2C_DATA)
	mcu_config_input(I2C_DATA);
	mcu_config_pullup(I2C_DATA);
#endif
#if ASSERT_PIN_IO(TX2)
	mcu_config_output(TX2);
#endif
#if ASSERT_PIN_IO(RX2)
	mcu_config_input(RX2);
#ifdef RX2_PULLUP
	mcu_config_pullup(RX2);
#endif
#endif
#if ASSERT_PIN_IO(SPI2_CLK)
	mcu_config_output(SPI2_CLK);
#endif
#if ASSERT_PIN_IO(SPI2_SDI)
	mcu_config_output(SPI2_SDI);
#endif
#if ASSERT_PIN_IO(SPI2_SDO)
	mcu_config_output(SPI2_SDO);
#endif
#if ASSERT_PIN(SPI2_CS)
	mcu_config_output(SPI2_CS);
#endif

#ifdef MCU_HAS_UART
#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE 64
#endif
	BUFFER_INIT(uint8_t, uart_tx, UART_TX_BUFFER_SIZE);
	BUFFER_INIT(uint8_t, uart_rx, RX_BUFFER_SIZE);
#endif
#ifdef MCU_HAS_UART2
#ifndef UART2_TX_BUFFER_SIZE
#define UART2_TX_BUFFER_SIZE 64
#endif
	BUFFER_INIT(uint8_t, uart2_tx, UART2_TX_BUFFER_SIZE);
	BUFFER_INIT(uint8_t, uart2_rx, RX_BUFFER_SIZE);
#endif
#ifdef MCU_HAS_USB
#ifndef USB_TX_BUFFER_SIZE
#define USB_TX_BUFFER_SIZE 64
#endif
	BUFFER_INIT(uint8_t, usb_tx, USB_TX_BUFFER_SIZE);
	BUFFER_INIT(uint8_t, usb_rx, RX_BUFFER_SIZE);
#endif
#ifdef MCU_HAS_WIFI
#ifndef WIFI_TX_BUFFER_SIZE
#define WIFI_TX_BUFFER_SIZE 64
#endif
	BUFFER_INIT(uint8_t, wifi_tx, WIFI_TX_BUFFER_SIZE);
	BUFFER_INIT(uint8_t, wifi_rx, RX_BUFFER_SIZE);
#endif
#ifdef MCU_HAS_BLUETOOTH
#ifndef BLUETOOTH_TX_BUFFER_SIZE
#define BLUETOOTH_TX_BUFFER_SIZE 64
#endif
	BUFFER_INIT(uint8_t, bt_tx, BLUETOOTH_TX_BUFFER_SIZE);
	BUFFER_INIT(uint8_t, bt_rx, RX_BUFFER_SIZE);
#endif
}

#ifndef mcu_io_reset
void __attribute__((weak)) mcu_io_reset(void)
{
}
#endif

// Non volatile memory
/**
 * gets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
 * */
uint8_t __attribute__((weak)) mcu_eeprom_getc(uint16_t address)
{
	return 0;
}

/**
 * sets a byte at the given EEPROM (or other non volatile memory) address of the MCU.
 * */
void __attribute__((weak)) mcu_eeprom_putc(uint16_t address, uint8_t value)
{
}

/**
 * flushes all recorded registers into the eeprom.
 * */
void __attribute__((weak)) mcu_eeprom_flush(void)
{
}

// ISR
// New uint8_t handle strategy
// All ascii will be sent to buffer and processed later (including comments)
MCU_RX_CALLBACK bool mcu_com_rx_cb(uint8_t c)
{
	static bool is_grbl_cmd = false;
	if (c < ((uint8_t)0x7F)) // ascii (all bellow DEL)
	{
		switch (c)
		{
		case CMD_CODE_REPORT:
#if STATUS_AUTOMATIC_REPORT_INTERVAL >= 100
			return false;
#endif
		case CMD_CODE_RESET:
		case CMD_CODE_FEED_HOLD:
			cnc_call_rt_command((uint8_t)c);
			return false;
		case '\n':
		case '\r':
		case 0:
			// EOL marker
			is_grbl_cmd = false;
			break;
		case '$':
			is_grbl_cmd = true;
			break;
		case CMD_CODE_CYCLE_START:
			if (!is_grbl_cmd)
			{
				cnc_call_rt_command(CMD_CODE_CYCLE_START);
				return false;
			}
			break;
		}
	}
	else // extended ascii (plus CMD_CODE_CYCLE_START and DEL)
	{
		cnc_call_rt_command((uint8_t)c);
		return false;
	}

	return true;
}

#ifdef MCU_HAS_UART
#ifdef DETACH_UART_FROM_MAIN_PROTOCOL
MCU_RX_CALLBACK void __attribute__((weak)) mcu_uart_rx_cb(uint8_t c) {}
#endif
#endif

#ifdef MCU_HAS_UART2
#ifdef DETACH_UART2_FROM_MAIN_PROTOCOL
MCU_RX_CALLBACK void __attribute__((weak)) mcu_uart2_rx_cb(uint8_t c) {}
#endif
#endif

#ifdef MCU_HAS_USB
#ifdef DETACH_USB_FROM_MAIN_PROTOCOL
MCU_RX_CALLBACK void __attribute__((weak)) mcu_usb_rx_cb(uint8_t c) {}
#endif
#endif

#ifdef MCU_HAS_WIFI
#ifdef DETACH_WIFI_FROM_MAIN_PROTOCOL
MCU_RX_CALLBACK void __attribute__((weak)) mcu_wifi_rx_cb(uint8_t c) {}
#endif
#endif

#ifdef MCU_HAS_BLUETOOTH
#ifdef DETACH_BLUETOOTH_FROM_MAIN_PROTOCOL
MCU_RX_CALLBACK void __attribute__((weak)) mcu_bt_rx_cb(uint8_t c) {}
#endif
#endif

#if (defined(MCU_HAS_I2C))
#if defined(MCU_SUPPORTS_I2C_SLAVE) && (I2C_ADDRESS != 0)
void __attribute__((weak)) mcu_i2c_slave_cb(uint8_t *data, uint8_t *datalen)
{
}
#endif
#endif

#if (defined(MCU_HAS_SPI))
void __attribute__((weak)) mcu_spi_start(spi_config_t config, uint32_t frequency)
{
	// reapply port settings if port is shared
	mcu_spi_config(config, frequency);
}

// the maximum amount of time in milliseconds it will transmit data without running the main loop
#ifndef BULK_SPI_TIMEOUT
#define BULK_SPI_TIMEOUT (1000 / INTERPOLATOR_FREQ)
#endif

bool __attribute__((weak)) mcu_spi_bulk_transfer(const uint8_t *out, uint8_t *in, uint16_t len)
{
	uint32_t timeout = BULK_SPI_TIMEOUT + mcu_millis();
	while (len--)
	{
		uint8_t c = mcu_spi_xmit(*out++);
		if (in)
		{
			*in++ = c;
		}

		if (timeout < mcu_millis())
		{
			timeout = BULK_SPI_TIMEOUT + mcu_millis();
			cnc_dotasks();
		}
	}

	return false;
}

void __attribute__((weak)) mcu_spi_stop(void)
{
}

spi_port_t __attribute__((used)) mcu_spi_port = {.isbusy = false, .start = mcu_spi_start, .xmit = mcu_spi_xmit, .bulk_xmit = mcu_spi_bulk_transfer, .stop = mcu_spi_stop};
#endif

#if (defined(MCU_HAS_SPI2))
void __attribute__((weak)) mcu_spi2_start(spi_config_t config, uint32_t frequency)
{
	// reapply port settings if port is shared
	mcu_spi2_config(config, frequency);
}

// the maximum amount of time in milliseconds it will transmit data without running the main loop
#ifndef BULK_SPI2_TIMEOUT
#define BULK_SPI2_TIMEOUT (1000 / INTERPOLATOR_FREQ)
#endif

bool __attribute__((weak)) mcu_spi2_bulk_transfer(const uint8_t *out, uint8_t *in, uint16_t len)
{
	uint32_t timeout = BULK_SPI2_TIMEOUT + mcu_millis();
	while (len--)
	{
		uint8_t c = mcu_spi2_xmit(*out++);
		if (in)
		{
			*in++ = c;
		}

		if (timeout < mcu_millis())
		{
			timeout = BULK_SPI2_TIMEOUT + mcu_millis();
			cnc_dotasks();
		}
	}

	return false;
}

void __attribute__((weak)) mcu_spi2_stop(void)
{
}

spi_port_t __attribute__((used)) mcu_spi2_port = {.isbusy = false, .start = mcu_spi2_start, .xmit = mcu_spi2_xmit, .bulk_xmit = mcu_spi2_bulk_transfer, .stop = mcu_spi2_stop};
#endif

uint8_t __attribute__((weak)) mcu_softpwm_freq_config(uint16_t freq)
{
	// keeps 8 bit resolution up to 500Hz
	// reduces bit resolution for higher frequencies

	// determines the bit resolution (7 - esp32_pwm_res);
	uint8_t res = (uint8_t)MAX((int8_t)ceilf(LOG2(freq * 0.002f)), 0);
	return res;
}
