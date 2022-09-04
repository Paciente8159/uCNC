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

void __attribute__((weak)) mcu_io_init(void)
{
#if !(STEP0 < 0)
	mcu_config_output(STEP0);
#endif
#if !(STEP1 < 0)
	mcu_config_output(STEP1);
#endif
#if !(STEP2 < 0)
	mcu_config_output(STEP2);
#endif
#if !(STEP3 < 0)
	mcu_config_output(STEP3);
#endif
#if !(STEP4 < 0)
	mcu_config_output(STEP4);
#endif
#if !(STEP5 < 0)
	mcu_config_output(STEP5);
#endif
#if !(STEP6 < 0)
	mcu_config_output(STEP6);
#endif
#if !(STEP7 < 0)
	mcu_config_output(STEP7);
#endif
#if !(DIR0 < 0)
	mcu_config_output(DIR0);
#endif
#if !(DIR1 < 0)
	mcu_config_output(DIR1);
#endif
#if !(DIR2 < 0)
	mcu_config_output(DIR2);
#endif
#if !(DIR3 < 0)
	mcu_config_output(DIR3);
#endif
#if !(DIR4 < 0)
	mcu_config_output(DIR4);
#endif
#if !(DIR5 < 0)
	mcu_config_output(DIR5);
#endif
#if !(DIR6 < 0)
	mcu_config_output(DIR6);
#endif
#if !(DIR7 < 0)
	mcu_config_output(DIR7);
#endif
#if !(STEP0_EN < 0)
	mcu_config_output(STEP0_EN);
#endif
#if !(STEP1_EN < 0)
	mcu_config_output(STEP1_EN);
#endif
#if !(STEP2_EN < 0)
	mcu_config_output(STEP2_EN);
#endif
#if !(STEP3_EN < 0)
	mcu_config_output(STEP3_EN);
#endif
#if !(STEP4_EN < 0)
	mcu_config_output(STEP4_EN);
#endif
#if !(STEP5_EN < 0)
	mcu_config_output(STEP5_EN);
#endif
#if !(STEP6_EN < 0)
	mcu_config_output(STEP6_EN);
#endif
#if !(STEP7_EN < 0)
	mcu_config_output(STEP7_EN);
#endif
#if !(PWM0 < 0)
	mcu_config_pwm(PWM0);
#endif
#if !(PWM1 < 0)
	mcu_config_pwm(PWM1);
#endif
#if !(PWM2 < 0)
	mcu_config_pwm(PWM2);
#endif
#if !(PWM3 < 0)
	mcu_config_pwm(PWM3);
#endif
#if !(PWM4 < 0)
	mcu_config_pwm(PWM4);
#endif
#if !(PWM5 < 0)
	mcu_config_pwm(PWM5);
#endif
#if !(PWM6 < 0)
	mcu_config_pwm(PWM6);
#endif
#if !(PWM7 < 0)
	mcu_config_pwm(PWM7);
#endif
#if !(PWM8 < 0)
	mcu_config_pwm(PWM8);
#endif
#if !(PWM9 < 0)
	mcu_config_pwm(PWM9);
#endif
#if !(PWM10 < 0)
	mcu_config_pwm(PWM10);
#endif
#if !(PWM11 < 0)
	mcu_config_pwm(PWM11);
#endif
#if !(PWM12 < 0)
	mcu_config_pwm(PWM12);
#endif
#if !(PWM13 < 0)
	mcu_config_pwm(PWM13);
#endif
#if !(PWM14 < 0)
	mcu_config_pwm(PWM14);
#endif
#if !(PWM15 < 0)
	mcu_config_pwm(PWM15);
#endif
#if !(SERVO0 < 0)
	mcu_config_output(SERVO0);
#endif
#if !(SERVO1 < 0)
	mcu_config_output(SERVO1);
#endif
#if !(SERVO2 < 0)
	mcu_config_output(SERVO2);
#endif
#if !(SERVO3 < 0)
	mcu_config_output(SERVO3);
#endif
#if !(SERVO4 < 0)
	mcu_config_output(SERVO4);
#endif
#if !(SERVO5 < 0)
	mcu_config_output(SERVO5);
#endif
#if !(DOUT0 < 0)
	mcu_config_output(DOUT0);
#endif
#if !(DOUT1 < 0)
	mcu_config_output(DOUT1);
#endif
#if !(DOUT2 < 0)
	mcu_config_output(DOUT2);
#endif
#if !(DOUT3 < 0)
	mcu_config_output(DOUT3);
#endif
#if !(DOUT4 < 0)
	mcu_config_output(DOUT4);
#endif
#if !(DOUT5 < 0)
	mcu_config_output(DOUT5);
#endif
#if !(DOUT6 < 0)
	mcu_config_output(DOUT6);
#endif
#if !(DOUT7 < 0)
	mcu_config_output(DOUT7);
#endif
#if !(DOUT8 < 0)
	mcu_config_output(DOUT8);
#endif
#if !(DOUT9 < 0)
	mcu_config_output(DOUT9);
#endif
#if !(DOUT10 < 0)
	mcu_config_output(DOUT10);
#endif
#if !(DOUT11 < 0)
	mcu_config_output(DOUT11);
#endif
#if !(DOUT12 < 0)
	mcu_config_output(DOUT12);
#endif
#if !(DOUT13 < 0)
	mcu_config_output(DOUT13);
#endif
#if !(DOUT14 < 0)
	mcu_config_output(DOUT14);
#endif
#if !(DOUT15 < 0)
	mcu_config_output(DOUT15);
#endif
#if !(DOUT16 < 0)
	mcu_config_output(DOUT16);
#endif
#if !(DOUT17 < 0)
	mcu_config_output(DOUT17);
#endif
#if !(DOUT18 < 0)
	mcu_config_output(DOUT18);
#endif
#if !(DOUT19 < 0)
	mcu_config_output(DOUT19);
#endif
#if !(DOUT20 < 0)
	mcu_config_output(DOUT20);
#endif
#if !(DOUT21 < 0)
	mcu_config_output(DOUT21);
#endif
#if !(DOUT22 < 0)
	mcu_config_output(DOUT22);
#endif
#if !(DOUT23 < 0)
	mcu_config_output(DOUT23);
#endif
#if !(DOUT24 < 0)
	mcu_config_output(DOUT24);
#endif
#if !(DOUT25 < 0)
	mcu_config_output(DOUT25);
#endif
#if !(DOUT26 < 0)
	mcu_config_output(DOUT26);
#endif
#if !(DOUT27 < 0)
	mcu_config_output(DOUT27);
#endif
#if !(DOUT28 < 0)
	mcu_config_output(DOUT28);
#endif
#if !(DOUT29 < 0)
	mcu_config_output(DOUT29);
#endif
#if !(DOUT30 < 0)
	mcu_config_output(DOUT30);
#endif
#if !(DOUT31 < 0)
	mcu_config_output(DOUT31);
#endif
#if !(LIMIT_X < 0)
	mcu_config_input(LIMIT_X);
#ifdef LIMIT_X_PULLUP
	mcu_config_pullup(LIMIT_X);
#endif
#ifdef LIMIT_X_ISR
	mcu_config_input_isr(LIMIT_X);
#endif
#endif
#if !(LIMIT_Y < 0)
	mcu_config_input(LIMIT_Y);
#ifdef LIMIT_Y_PULLUP
	mcu_config_pullup(LIMIT_Y);
#endif
#ifdef LIMIT_Y_ISR
	mcu_config_input_isr(LIMIT_Y);
#endif
#endif
#if !(LIMIT_Z < 0)
	mcu_config_input(LIMIT_Z);
#ifdef LIMIT_Z_PULLUP
	mcu_config_pullup(LIMIT_Z);
#endif
#ifdef LIMIT_Z_ISR
	mcu_config_input_isr(LIMIT_Z);
#endif
#endif
#if !(LIMIT_X2 < 0)
	mcu_config_input(LIMIT_X2);
#ifdef LIMIT_X2_PULLUP
	mcu_config_pullup(LIMIT_X2);
#endif
#ifdef LIMIT_X2_ISR
	mcu_config_input_isr(LIMIT_X2);
#endif
#endif
#if !(LIMIT_Y2 < 0)
	mcu_config_input(LIMIT_Y2);
#ifdef LIMIT_Y2_PULLUP
	mcu_config_pullup(LIMIT_Y2);
#endif
#ifdef LIMIT_Y2_ISR
	mcu_config_input_isr(LIMIT_Y2);
#endif
#endif
#if !(LIMIT_Z2 < 0)
	mcu_config_input(LIMIT_Z2);
#ifdef LIMIT_Z2_PULLUP
	mcu_config_pullup(LIMIT_Z2);
#endif
#ifdef LIMIT_Z2_ISR
	mcu_config_input_isr(LIMIT_Z2);
#endif
#endif
#if !(LIMIT_A < 0)
	mcu_config_input(LIMIT_A);
#ifdef LIMIT_A_PULLUP
	mcu_config_pullup(LIMIT_A);
#endif
#ifdef LIMIT_A_ISR
	mcu_config_input_isr(LIMIT_A);
#endif
#endif
#if !(LIMIT_B < 0)
	mcu_config_input(LIMIT_B);
#ifdef LIMIT_B_PULLUP
	mcu_config_pullup(LIMIT_B);
#endif
#ifdef LIMIT_B_ISR
	mcu_config_input_isr(LIMIT_B);
#endif
#endif
#if !(LIMIT_C < 0)
	mcu_config_input(LIMIT_C);
#ifdef LIMIT_C_PULLUP
	mcu_config_pullup(LIMIT_C);
#endif
#ifdef LIMIT_C_ISR
	mcu_config_input_isr(LIMIT_C);
#endif
#endif
#if !(PROBE < 0)
	mcu_config_input(PROBE);
#ifdef PROBE_PULLUP
	mcu_config_pullup(PROBE);
#endif
#ifdef PROBE_ISR
	mcu_config_input_isr(PROBE);
#endif
#endif
#if !(ESTOP < 0)
	mcu_config_input(ESTOP);
#ifdef ESTOP_PULLUP
	mcu_config_pullup(ESTOP);
#endif
#ifdef ESTOP_ISR
	mcu_config_input_isr(ESTOP);
#endif
#endif
#if !(SAFETY_DOOR < 0)
	mcu_config_input(SAFETY_DOOR);
#ifdef SAFETY_DOOR_PULLUP
	mcu_config_pullup(SAFETY_DOOR);
#endif
#ifdef SAFETY_DOOR_ISR
	mcu_config_input_isr(SAFETY_DOOR);
#endif
#endif
#if !(FHOLD < 0)
	mcu_config_input(FHOLD);
#ifdef FHOLD_PULLUP
	mcu_config_pullup(FHOLD);
#endif
#ifdef FHOLD_ISR
	mcu_config_input_isr(FHOLD);
#endif
#endif
#if !(CS_RES < 0)
	mcu_config_input(CS_RES);
#ifdef CS_RES_PULLUP
	mcu_config_pullup(CS_RES);
#endif
#ifdef CS_RES_ISR
	mcu_config_input_isr(CS_RES);
#endif
#endif
#if !(ANALOG0 < 0)
	mcu_config_analog(ANALOG0);
#endif
#if !(ANALOG1 < 0)
	mcu_config_analog(ANALOG1);
#endif
#if !(ANALOG2 < 0)
	mcu_config_analog(ANALOG2);
#endif
#if !(ANALOG3 < 0)
	mcu_config_analog(ANALOG3);
#endif
#if !(ANALOG4 < 0)
	mcu_config_analog(ANALOG4);
#endif
#if !(ANALOG5 < 0)
	mcu_config_analog(ANALOG5);
#endif
#if !(ANALOG6 < 0)
	mcu_config_analog(ANALOG6);
#endif
#if !(ANALOG7 < 0)
	mcu_config_analog(ANALOG7);
#endif
#if !(ANALOG8 < 0)
	mcu_config_analog(ANALOG8);
#endif
#if !(ANALOG9 < 0)
	mcu_config_analog(ANALOG9);
#endif
#if !(ANALOG10 < 0)
	mcu_config_analog(ANALOG10);
#endif
#if !(ANALOG11 < 0)
	mcu_config_analog(ANALOG11);
#endif
#if !(ANALOG12 < 0)
	mcu_config_analog(ANALOG12);
#endif
#if !(ANALOG13 < 0)
	mcu_config_analog(ANALOG13);
#endif
#if !(ANALOG14 < 0)
	mcu_config_analog(ANALOG14);
#endif
#if !(ANALOG15 < 0)
	mcu_config_analog(ANALOG15);
#endif
#if !(DIN0 < 0)
	mcu_config_input(DIN0);
#ifdef DIN0_PULLUP
	mcu_config_pullup(DIN0);
#endif
#ifdef DIN0_ISR
	mcu_config_input_isr(DIN0);
#endif
#endif
#if !(DIN1 < 0)
	mcu_config_input(DIN1);
#ifdef DIN1_PULLUP
	mcu_config_pullup(DIN1);
#endif
#ifdef DIN1_ISR
	mcu_config_input_isr(DIN1);
#endif
#endif
#if !(DIN2 < 0)
	mcu_config_input(DIN2);
#ifdef DIN2_PULLUP
	mcu_config_pullup(DIN2);
#endif
#ifdef DIN2_ISR
	mcu_config_input_isr(DIN2);
#endif
#endif
#if !(DIN3 < 0)
	mcu_config_input(DIN3);
#ifdef DIN3_PULLUP
	mcu_config_pullup(DIN3);
#endif
#ifdef DIN3_ISR
	mcu_config_input_isr(DIN3);
#endif
#endif
#if !(DIN4 < 0)
	mcu_config_input(DIN4);
#ifdef DIN4_PULLUP
	mcu_config_pullup(DIN4);
#endif
#ifdef DIN4_ISR
	mcu_config_input_isr(DIN4);
#endif
#endif
#if !(DIN5 < 0)
	mcu_config_input(DIN5);
#ifdef DIN5_PULLUP
	mcu_config_pullup(DIN5);
#endif
#ifdef DIN5_ISR
	mcu_config_input_isr(DIN5);
#endif
#endif
#if !(DIN6 < 0)
	mcu_config_input(DIN6);
#ifdef DIN6_PULLUP
	mcu_config_pullup(DIN6);
#endif
#ifdef DIN6_ISR
	mcu_config_input_isr(DIN6);
#endif
#endif
#if !(DIN7 < 0)
	mcu_config_input(DIN7);
#ifdef DIN7_PULLUP
	mcu_config_pullup(DIN7);
#endif
#ifdef DIN7_ISR
	mcu_config_input_isr(DIN7);
#endif
#endif
#if !(DIN8 < 0)
	mcu_config_input(DIN8);
#ifdef DIN8_PULLUP
	mcu_config_pullup(DIN8);
#endif
#endif
#if !(DIN9 < 0)
	mcu_config_input(DIN9);
#ifdef DIN9_PULLUP
	mcu_config_pullup(DIN9);
#endif
#endif
#if !(DIN10 < 0)
	mcu_config_input(DIN10);
#ifdef DIN10_PULLUP
	mcu_config_pullup(DIN10);
#endif
#endif
#if !(DIN11 < 0)
	mcu_config_input(DIN11);
#ifdef DIN11_PULLUP
	mcu_config_pullup(DIN11);
#endif
#endif
#if !(DIN12 < 0)
	mcu_config_input(DIN12);
#ifdef DIN12_PULLUP
	mcu_config_pullup(DIN12);
#endif
#endif
#if !(DIN13 < 0)
	mcu_config_input(DIN13);
#ifdef DIN13_PULLUP
	mcu_config_pullup(DIN13);
#endif
#endif
#if !(DIN14 < 0)
	mcu_config_input(DIN14);
#ifdef DIN14_PULLUP
	mcu_config_pullup(DIN14);
#endif
#endif
#if !(DIN15 < 0)
	mcu_config_input(DIN15);
#ifdef DIN15_PULLUP
	mcu_config_pullup(DIN15);
#endif
#endif
#if !(DIN16 < 0)
	mcu_config_input(DIN16);
#ifdef DIN16_PULLUP
	mcu_config_pullup(DIN16);
#endif
#endif
#if !(DIN17 < 0)
	mcu_config_input(DIN17);
#ifdef DIN17_PULLUP
	mcu_config_pullup(DIN17);
#endif
#endif
#if !(DIN18 < 0)
	mcu_config_input(DIN18);
#ifdef DIN18_PULLUP
	mcu_config_pullup(DIN18);
#endif
#endif
#if !(DIN19 < 0)
	mcu_config_input(DIN19);
#ifdef DIN19_PULLUP
	mcu_config_pullup(DIN19);
#endif
#endif
#if !(DIN20 < 0)
	mcu_config_input(DIN20);
#ifdef DIN20_PULLUP
	mcu_config_pullup(DIN20);
#endif
#endif
#if !(DIN21 < 0)
	mcu_config_input(DIN21);
#ifdef DIN21_PULLUP
	mcu_config_pullup(DIN21);
#endif
#endif
#if !(DIN22 < 0)
	mcu_config_input(DIN22);
#ifdef DIN22_PULLUP
	mcu_config_pullup(DIN22);
#endif
#endif
#if !(DIN23 < 0)
	mcu_config_input(DIN23);
#ifdef DIN23_PULLUP
	mcu_config_pullup(DIN23);
#endif
#endif
#if !(DIN24 < 0)
	mcu_config_input(DIN24);
#ifdef DIN24_PULLUP
	mcu_config_pullup(DIN24);
#endif
#endif
#if !(DIN25 < 0)
	mcu_config_input(DIN25);
#ifdef DIN25_PULLUP
	mcu_config_pullup(DIN25);
#endif
#endif
#if !(DIN26 < 0)
	mcu_config_input(DIN26);
#ifdef DIN26_PULLUP
	mcu_config_pullup(DIN26);
#endif
#endif
#if !(DIN27 < 0)
	mcu_config_input(DIN27);
#ifdef DIN27_PULLUP
	mcu_config_pullup(DIN27);
#endif
#endif
#if !(DIN28 < 0)
	mcu_config_input(DIN28);
#ifdef DIN28_PULLUP
	mcu_config_pullup(DIN28);
#endif
#endif
#if !(DIN29 < 0)
	mcu_config_input(DIN29);
#ifdef DIN29_PULLUP
	mcu_config_pullup(DIN29);
#endif
#endif
#if !(DIN30 < 0)
	mcu_config_input(DIN30);
#ifdef DIN30_PULLUP
	mcu_config_pullup(DIN30);
#endif
#endif
#if !(DIN31 < 0)
	mcu_config_input(DIN31);
#ifdef DIN31_PULLUP
	mcu_config_pullup(DIN31);
#endif
#endif
#if !(TX < 0)
	mcu_config_output(TX);
#endif
#if !(RX < 0)
	mcu_config_input(RX);
#ifdef RX_PULLUP
	mcu_config_pullup(RX);
#endif
#endif
#if !(USB_DM < 0)
	mcu_config_input(USB_DM);
#ifdef USB_DM_PULLUP
	mcu_config_pullup(USB_DM);
#endif
#endif
#if !(USB_DP < 0)
	mcu_config_input(USB_DP);
#ifdef USB_DP_PULLUP
	mcu_config_pullup(USB_DP);
#endif
#endif
#if !(SPI_CLK < 0)
	mcu_config_output(SPI_CLK);
#endif
#if !(SPI_SDI < 0)
	mcu_config_input(SPI_SDI);
#ifdef SPI_SDI_PULLUP
	mcu_config_pullup(SPI_SDI);
#endif
#endif
#if !(SPI_SDO < 0)
	mcu_config_output(SPI_SDO);
#endif
#if !(SPI_CS < 0)
	mcu_config_output(SPI_CS);
#endif
#if !(I2C_SCL < 0)
	mcu_config_input(I2C_SCL);
	mcu_config_pullup(I2C_SCL);
#endif
#if !(I2C_SDA < 0)
	mcu_config_input(I2C_SDA);
	mcu_config_pullup(I2C_SDA);
#endif
}