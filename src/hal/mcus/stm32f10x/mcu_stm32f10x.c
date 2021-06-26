/*
	Name: mcu.h
	Description: Contains all the function declarations necessary to interact with the MCU.
        This provides a opac intenterface between the µCNC and the MCU unit used to power the µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 01/11/2019

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifdef __cplusplus
extern "C"
{
#endif

#include "cnc.h"

#if (MCU == MCU_STM32F10X)
#include "core_cm3.h"
#include "mcumap_stm32f10x.h"
#include "interface/serial.h"
#include "core/interpolator.h"
#include "core/io_control.h"
#ifdef USB_VCP
#include "tusb_config.h"
#include "tusb.h"

#define USB_BASE (APB1PERIPH_BASE + 0x00005C00UL)
	typedef struct
	{
		__IO uint16_t EP0R;			 /*!< USB Endpoint 0 register,                   Address offset: 0x00 */
		__IO uint16_t RESERVED0;	 /*!< Reserved */
		__IO uint16_t EP1R;			 /*!< USB Endpoint 1 register,                   Address offset: 0x04 */
		__IO uint16_t RESERVED1;	 /*!< Reserved */
		__IO uint16_t EP2R;			 /*!< USB Endpoint 2 register,                   Address offset: 0x08 */
		__IO uint16_t RESERVED2;	 /*!< Reserved */
		__IO uint16_t EP3R;			 /*!< USB Endpoint 3 register,                   Address offset: 0x0C */
		__IO uint16_t RESERVED3;	 /*!< Reserved */
		__IO uint16_t EP4R;			 /*!< USB Endpoint 4 register,                   Address offset: 0x10 */
		__IO uint16_t RESERVED4;	 /*!< Reserved */
		__IO uint16_t EP5R;			 /*!< USB Endpoint 5 register,                   Address offset: 0x14 */
		__IO uint16_t RESERVED5;	 /*!< Reserved */
		__IO uint16_t EP6R;			 /*!< USB Endpoint 6 register,                   Address offset: 0x18 */
		__IO uint16_t RESERVED6;	 /*!< Reserved */
		__IO uint16_t EP7R;			 /*!< USB Endpoint 7 register,                   Address offset: 0x1C */
		__IO uint16_t RESERVED7[17]; /*!< Reserved */
		__IO uint16_t CNTR;			 /*!< Control register,                          Address offset: 0x40 */
		__IO uint16_t RESERVED8;	 /*!< Reserved */
		__IO uint16_t ISTR;			 /*!< Interrupt status register,                 Address offset: 0x44 */
		__IO uint16_t RESERVED9;	 /*!< Reserved */
		__IO uint16_t FNR;			 /*!< Frame number register,                     Address offset: 0x48 */
		__IO uint16_t RESERVEDA;	 /*!< Reserved */
		__IO uint16_t DADDR;		 /*!< Device address register,                   Address offset: 0x4C */
		__IO uint16_t RESERVEDB;	 /*!< Reserved */
		__IO uint16_t BTABLE;		 /*!< Buffer Table address register,             Address offset: 0x50 */
		__IO uint16_t RESERVEDC;	 /*!< Reserved */
	} USB_TypeDef;

#define USB ((USB_TypeDef *)USB_BASE)

/*!< Common registers */
/*******************  Bit definition for USB_CNTR register  *******************/
#define USB_CNTR_FRES_Pos (0U)
#define USB_CNTR_FRES_Msk (0x1UL << USB_CNTR_FRES_Pos) /*!< 0x00000001 */
#define USB_CNTR_FRES USB_CNTR_FRES_Msk				   /*!< Force USB Reset */
#define USB_CNTR_PDWN_Pos (1U)
#define USB_CNTR_PDWN_Msk (0x1UL << USB_CNTR_PDWN_Pos) /*!< 0x00000002 */
#define USB_CNTR_PDWN USB_CNTR_PDWN_Msk				   /*!< Power down */
#define USB_CNTR_LP_MODE_Pos (2U)
#define USB_CNTR_LP_MODE_Msk (0x1UL << USB_CNTR_LP_MODE_Pos) /*!< 0x00000004 */
#define USB_CNTR_LP_MODE USB_CNTR_LP_MODE_Msk				 /*!< Low-power mode */
#define USB_CNTR_FSUSP_Pos (3U)
#define USB_CNTR_FSUSP_Msk (0x1UL << USB_CNTR_FSUSP_Pos) /*!< 0x00000008 */
#define USB_CNTR_FSUSP USB_CNTR_FSUSP_Msk				 /*!< Force suspend */
#define USB_CNTR_RESUME_Pos (4U)
#define USB_CNTR_RESUME_Msk (0x1UL << USB_CNTR_RESUME_Pos) /*!< 0x00000010 */
#define USB_CNTR_RESUME USB_CNTR_RESUME_Msk				   /*!< Resume request */
#define USB_CNTR_ESOFM_Pos (8U)
#define USB_CNTR_ESOFM_Msk (0x1UL << USB_CNTR_ESOFM_Pos) /*!< 0x00000100 */
#define USB_CNTR_ESOFM USB_CNTR_ESOFM_Msk				 /*!< Expected Start Of Frame Interrupt Mask */
#define USB_CNTR_SOFM_Pos (9U)
#define USB_CNTR_SOFM_Msk (0x1UL << USB_CNTR_SOFM_Pos) /*!< 0x00000200 */
#define USB_CNTR_SOFM USB_CNTR_SOFM_Msk				   /*!< Start Of Frame Interrupt Mask */
#define USB_CNTR_RESETM_Pos (10U)
#define USB_CNTR_RESETM_Msk (0x1UL << USB_CNTR_RESETM_Pos) /*!< 0x00000400 */
#define USB_CNTR_RESETM USB_CNTR_RESETM_Msk				   /*!< RESET Interrupt Mask */
#define USB_CNTR_SUSPM_Pos (11U)
#define USB_CNTR_SUSPM_Msk (0x1UL << USB_CNTR_SUSPM_Pos) /*!< 0x00000800 */
#define USB_CNTR_SUSPM USB_CNTR_SUSPM_Msk				 /*!< Suspend mode Interrupt Mask */
#define USB_CNTR_WKUPM_Pos (12U)
#define USB_CNTR_WKUPM_Msk (0x1UL << USB_CNTR_WKUPM_Pos) /*!< 0x00001000 */
#define USB_CNTR_WKUPM USB_CNTR_WKUPM_Msk				 /*!< Wakeup Interrupt Mask */
#define USB_CNTR_ERRM_Pos (13U)
#define USB_CNTR_ERRM_Msk (0x1UL << USB_CNTR_ERRM_Pos) /*!< 0x00002000 */
#define USB_CNTR_ERRM USB_CNTR_ERRM_Msk				   /*!< Error Interrupt Mask */
#define USB_CNTR_PMAOVRM_Pos (14U)
#define USB_CNTR_PMAOVRM_Msk (0x1UL << USB_CNTR_PMAOVRM_Pos) /*!< 0x00004000 */
#define USB_CNTR_PMAOVRM USB_CNTR_PMAOVRM_Msk				 /*!< Packet Memory Area Over / Underrun Interrupt Mask */
#define USB_CNTR_CTRM_Pos (15U)
#define USB_CNTR_CTRM_Msk (0x1UL << USB_CNTR_CTRM_Pos) /*!< 0x00008000 */
#define USB_CNTR_CTRM USB_CNTR_CTRM_Msk				   /*!< Correct Transfer Interrupt Mask */
#endif

	/**
	 * The internal clock counter
	 * Increments every millisecond
	 * Can count up to almost 50 days
	 **/
	static volatile uint32_t mcu_runtime_ms;

/**
 * The isr functions
 * The respective IRQHandler will execute these functions 
 **/
#ifdef COM_PORT
	void mcu_serial_isr(void)
	{
#ifndef ENABLE_SYNC_RX
		if (COM_USART->SR & USART_SR_RXNE)
		{
			unsigned char c = COM_INREG;
			serial_rx_isr(c);
			COM_USART->SR &= ~USART_SR_RXNE;
		}
#endif

#ifndef ENABLE_SYNC_TX
		if (COM_USART->SR & (USART_SR_TXE | USART_SR_TC))
		{
			serial_tx_isr();
			COM_USART->SR &= ~(USART_SR_TXE | USART_SR_TC);
		}
#endif
		NVIC_ClearPendingIRQ(COM_IRQ);
	}
#elif defined(USB_VCP)
	void USB_HP_CAN1_TX_IRQHandler(void)
	{
		tud_int_handler(0);
		NVIC_ClearPendingIRQ(USB_HP_CAN1_TX_IRQn);
	}

	void USB_LP_CAN1_RX0_IRQHandler(void)
	{
		tud_int_handler(0);
		NVIC_ClearPendingIRQ(USB_LP_CAN1_RX0_IRQn);
	}

	void USBWakeUp_IRQHandler(void)
	{
		tud_int_handler(0);
		NVIC_ClearPendingIRQ(USBWakeUp_IRQn);
	}
#endif

	void mcu_timer_isr(void)
	{
		static bool resetstep = false;
		if ((TIMER_REG->SR & 1))
		{
			if (!resetstep)
				itp_step_isr();
			else
				itp_step_reset_isr();
			resetstep = !resetstep;
		}
		TIMER_REG->SR = 0;
		NVIC_ClearPendingIRQ(TIMER_IRQ);
		mcu_enable_global_isr();
	}

#define LIMITS_EXTIBITMASK (LIMIT_X_EXTIBITMASK | LIMIT_Y_EXTIBITMASK | LIMIT_Z_EXTIBITMASK | LIMIT_X2_EXTIBITMASK | LIMIT_Y2_EXTIBITMASK | LIMIT_Z2_EXTIBITMASK | LIMIT_A_EXTIBITMASK | LIMIT_B_EXTIBITMASK | LIMIT_C_EXTIBITMASK)
#define CONTROLS_EXTIBITMASK (ESTOP_EXTIBITMASK | SAFETY_DOOR_EXTIBITMASK | FHOLD_EXTIBITMASK | CS_RES_EXTIBITMASK)
#define ALL_EXTIBITMASK (LIMITS_EXTIBITMASK | CONTROLS_EXTIBITMASK | PROBE_EXTIBITMASK)

#if (ALL_EXTIBITMASK != 0)
	void mcu_input_isr(void)
	{
#if (LIMITS_EXTIBITMASK != 0)
		if (EXTI->PR & LIMITS_EXTIBITMASK)
		{
			io_limits_isr();
			EXTI->PR = LIMITS_EXTIBITMASK;
		}
#endif
#if (CONTROLS_EXTIBITMASK != 0)
		if (EXTI->PR & CONTROLS_EXTIBITMASK)
		{
			io_controls_isr();
			EXTI->PR = CONTROLS_EXTIBITMASK;
		}
#endif
#if (PROBE_EXTIBITMASK & 0x01)
		if (EXTI->PR & PROBE_EXTIBITMASK)
		{
			io_probe_isr();
			EXTI->PR = PROBE_EXTIBITMASK;
		}
#endif
	}

#if (ALL_EXTIBITMASK == 0x0001)
	void EXTI0_IRQHandler(void)
	{
		mcu_input_isr();
		NVIC_ClearPendingIRQ(EXTI0_IRQn);
	}
#endif
#if (ALL_EXTIBITMASK == 0x0002)
	void EXTI1_IRQHandler(void)
	{
		mcu_input_isr();
		NVIC_ClearPendingIRQ(EXTI1_IRQn);
	}
#endif
#if (ALL_EXTIBITMASK == 0x0004)
	void EXTI2_IRQHandler(void)
	{
		mcu_input_isr();
		NVIC_ClearPendingIRQ(EXTI2_IRQn);
	}
#endif
#if (ALL_EXTIBITMASK == 0x0008)
	void EXTI3_IRQHandler(void)
	{
		mcu_input_isr();
		NVIC_ClearPendingIRQ(EXTI3_IRQn);
	}
#endif
#if (ALL_EXTIBITMASK == 0x0010)
	void EXTI4_IRQHandler(void)
	{
		mcu_input_isr();
		NVIC_ClearPendingIRQ(EXTI4_IRQn);
	}
#endif
#if (ALL_EXTIBITMASK & 0x03E0)
	void EXTI9_5_IRQHandler(void)
	{
		mcu_input_isr();
		NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
	}
#endif
#if (ALL_EXTIBITMASK == 0xFC00)
	void EXTI15_10_IRQHandler(void)
	{
		mcu_input_isr();
		NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
	}
#endif
#endif

	void SysTick_Handler(void)
	{
		mcu_runtime_ms++;
		static uint32_t last_ms = 0;
		if (mcu_runtime_ms - last_ms > 1000)
		{
			last_ms = mcu_runtime_ms;
#ifdef LED
			mcu_toggle_output(LED);
#endif
		}
	}

	/**
	 * 
 * Initializes the mcu:
 *   1. Configures all IO
 *   2. Configures UART/USB
 *   3. Starts internal clock (RTC)
 **/
	static void mcu_tick_init(void);
	static void mcu_usart_init(void);

	void mcu_init(void)
	{
#ifdef STEP0
		mcu_config_output(STEP0);
#endif
#ifdef STEP1
		mcu_config_output(STEP1);
#endif
#ifdef STEP2
		mcu_config_output(STEP2);
#endif
#ifdef STEP3
		mcu_config_output(STEP3);
#endif
#ifdef STEP4
		mcu_config_output(STEP4);
#endif
#ifdef STEP5
		mcu_config_output(STEP5);
#endif
#ifdef STEP6
		mcu_config_output(STEP6);
#endif
#ifdef STEP7
		mcu_config_output(STEP7);
#endif
#ifdef STEP0_EN
		mcu_config_output(STEP0_EN);
#endif
#ifdef STEP1_EN
		mcu_config_output(STEP1_EN);
#endif
#ifdef STEP2_EN
		mcu_config_output(STEP2_EN);
#endif
#ifdef STEP3_EN
		mcu_config_output(STEP3_EN);
#endif
#ifdef STEP4_EN
		mcu_config_output(STEP4_EN);
#endif
#ifdef STEP5_EN
		mcu_config_output(STEP5_EN);
#endif
#ifdef DIR0
		mcu_config_output(DIR0);
#endif
#ifdef DIR1
		mcu_config_output(DIR1);
#endif
#ifdef DIR2
		mcu_config_output(DIR2);
#endif
#ifdef DIR3
		mcu_config_output(DIR3);
#endif
#ifdef DIR4
		mcu_config_output(DIR4);
#endif
#ifdef DIR5
		mcu_config_output(DIR5);
#endif
#ifdef PWM0
		mcu_config_pwm(PWM0);
#endif
#ifdef PWM1
		mcu_config_pwm(PWM1);
#endif
#ifdef PWM2
		mcu_config_pwm(PWM2);
#endif
#ifdef PWM3
		mcu_config_pwm(PWM3);
#endif
#ifdef PWM4
		mcu_config_pwm(PWM4);
#endif
#ifdef PWM5
		mcu_config_pwm(PWM5);
#endif
#ifdef PWM6
		mcu_config_pwm(PWM6);
#endif
#ifdef PWM7
		mcu_config_pwm(PWM7);
#endif
#ifdef PWM8
		mcu_config_pwm(PWM8);
#endif
#ifdef PWM9
		mcu_config_pwm(PWM9);
#endif
#ifdef PWM10
		mcu_config_pwm(PWM10);
#endif
#ifdef PWM11
		mcu_config_pwm(PWM11);
#endif
#ifdef PWM12
		mcu_config_pwm(PWM12);
#endif
#ifdef PWM13
		mcu_config_pwm(PWM13);
#endif
#ifdef PWM14
		mcu_config_pwm(PWM14);
#endif
#ifdef PWM15
		mcu_config_pwm(PWM15);
#endif

#ifdef DOUT0
		mcu_config_output(DOUT0);
#endif
#ifdef DOUT1
		mcu_config_output(DOUT1);
#endif
#ifdef DOUT2
		mcu_config_output(DOUT2);
#endif
#ifdef DOUT3
		mcu_config_output(DOUT3);
#endif
#ifdef DOUT4
		mcu_config_output(DOUT4);
#endif
#ifdef DOUT5
		mcu_config_output(DOUT5);
#endif
#ifdef DOUT6
		mcu_config_output(DOUT6);
#endif
#ifdef DOUT7
		mcu_config_output(DOUT7);
#endif
#ifdef DOUT8
		mcu_config_output(DOUT8);
#endif
#ifdef DOUT9
		mcu_config_output(DOUT9);
#endif
#ifdef DOUT10
		mcu_config_output(DOUT10);
#endif
#ifdef DOUT11
		mcu_config_output(DOUT11);
#endif
#ifdef DOUT12
		mcu_config_output(DOUT12);
#endif
#ifdef DOUT13
		mcu_config_output(DOUT13);
#endif
#ifdef DOUT14
		mcu_config_output(DOUT14);
#endif
#ifdef DOUT15
		mcu_config_output(DOUT15);
#endif
#ifdef LIMIT_X
		mcu_config_input(LIMIT_X);
#ifdef LIMIT_X_PULLUP
		mcu_config_pullup(LIMIT_X);
#endif
#ifdef LIMIT_X_ISR
		mcu_config_input_isr(LIMIT_X);
#endif
#endif
#ifdef LIMIT_Y
		mcu_config_input(LIMIT_Y);
#ifdef LIMIT_Y_PULLUP
		mcu_config_pullup(LIMIT_Y);
#endif
#ifdef LIMIT_Y_ISR
		mcu_config_input_isr(LIMIT_Y);
#endif
#endif
#ifdef LIMIT_Z
		mcu_config_input(LIMIT_Z);
#ifdef LIMIT_Z_PULLUP
		mcu_config_pullup(LIMIT_Z);
#endif
#ifdef LIMIT_Z_ISR
		mcu_config_input_isr(LIMIT_Z);
#endif
#endif
#ifdef LIMIT_X2
		mcu_config_input(LIMIT_X2);
#ifdef LIMIT_X2_PULLUP
		mcu_config_pullup(LIMIT_X2);
#endif
#ifdef LIMIT_X2_ISR
		mcu_config_input_isr(LIMIT_X2);
#endif
#endif
#ifdef LIMIT_Y2
		mcu_config_input(LIMIT_Y2);
#ifdef LIMIT_Y2_PULLUP
		mcu_config_pullup(LIMIT_Y2);
#endif
#ifdef LIMIT_Y2_ISR
		mcu_config_input_isr(LIMIT_Y2);
#endif
#endif
#ifdef LIMIT_Z2
		mcu_config_input(LIMIT_Z2);
#ifdef LIMIT_Z2_PULLUP
		mcu_config_pullup(LIMIT_Z2);
#endif
#ifdef LIMIT_Z2_ISR
		mcu_config_input_isr(LIMIT_Z2);
#endif
#endif
#ifdef LIMIT_A
		mcu_config_input(LIMIT_A);
#ifdef LIMIT_A_PULLUP
		mcu_config_pullup(LIMIT_A);
#endif
#ifdef LIMIT_A_ISR
		mcu_config_input_isr(LIMIT_A);
#endif
#endif
#ifdef LIMIT_B
		mcu_config_input(LIMIT_B);
#ifdef LIMIT_B_PULLUP
		mcu_config_pullup(LIMIT_B);
#endif
#ifdef LIMIT_B_ISR
		mcu_config_input_isr(LIMIT_B);
#endif
#endif
#ifdef LIMIT_C
		mcu_config_input(LIMIT_C);
#ifdef LIMIT_C_PULLUP
		mcu_config_pullup(LIMIT_C);
#endif
#ifdef LIMIT_C_ISR
		mcu_config_input_isr(LIMIT_C);
#endif
#endif
#ifdef PROBE
		mcu_config_input(PROBE);
#ifdef PROBE_PULLUP
		mcu_config_pullup(PROBE);
#endif
#ifdef PROBE_ISR
		mcu_config_input_isr(PROBE);
#endif
#endif
#ifdef ESTOP
		mcu_config_input(ESTOP);
#ifdef ESTOP_PULLUP
		mcu_config_pullup(ESTOP);
#endif
#ifdef ESTOP_ISR
		mcu_config_input_isr(ESTOP);
#endif
#endif
#ifdef SAFETY_DOOR
		mcu_config_input(SAFETY_DOOR);
#ifdef SAFETY_DOOR_PULLUP
		mcu_config_pullup(SAFETY_DOOR);
#endif
#ifdef SAFETY_DOOR_ISR
		mcu_config_input_isr(SAFETY_DOOR);
#endif
#endif
#ifdef FHOLD
		mcu_config_input(FHOLD);
#ifdef FHOLD_PULLUP
		mcu_config_pullup(FHOLD);
#endif
#ifdef FHOLD_ISR
		mcu_config_input_isr(FHOLD);
#endif
#endif
#ifdef CS_RES
		mcu_config_input(CS_RES);
#ifdef CS_RES_PULLUP
		mcu_config_pullup(CS_RES);
#endif
#ifdef CS_RES_ISR
		mcu_config_input_isr(CS_RES);
#endif
#endif
#ifdef ANALOG0
		mcu_config_analog(ANALOG0);
#endif
#ifdef ANALOG1
		mcu_config_analog(ANALOG1);
#endif
#ifdef ANALOG2
		mcu_config_analog(ANALOG2);
#endif
#ifdef ANALOG3
		mcu_config_analog(ANALOG3);
#endif
#ifdef ANALOG4
		mcu_config_analog(ANALOG4);
#endif
#ifdef ANALOG5
		mcu_config_analog(ANALOG5);
#endif
#ifdef ANALOG6
		mcu_config_analog(ANALOG6);
#endif
#ifdef ANALOG7
		mcu_config_analog(ANALOG7);
#endif
#ifdef ANALOG8
		mcu_config_analog(ANALOG8);
#endif
#ifdef ANALOG9
		mcu_config_analog(ANALOG9);
#endif
#ifdef ANALOG10
		mcu_config_analog(ANALOG10);
#endif
#ifdef ANALOG11
		mcu_config_analog(ANALOG11);
#endif
#ifdef ANALOG12
		mcu_config_analog(ANALOG12);
#endif
#ifdef ANALOG13
		mcu_config_analog(ANALOG13);
#endif
#ifdef ANALOG14
		mcu_config_analog(ANALOG14);
#endif
#ifdef ANALOG15
		mcu_config_analog(ANALOG15);
#endif

#ifdef DIN0
		mcu_config_input(DIN0);
#ifdef DIN0_PULLUP
		mcu_config_pullup(DIN0);
#endif
#endif
#ifdef DIN1
		mcu_config_input(DIN1);
#ifdef DIN1_PULLUP
		mcu_config_pullup(DIN1);
#endif
#endif
#ifdef DIN2
		mcu_config_input(DIN2);
#ifdef DIN2_PULLUP
		mcu_config_pullup(DIN2);
#endif
#endif
#ifdef DIN3
		mcu_config_input(DIN3);
#ifdef DIN3_PULLUP
		mcu_config_pullup(DIN3);
#endif
#endif
#ifdef DIN4
		mcu_config_input(DIN4);
#ifdef DIN4_PULLUP
		mcu_config_pullup(DIN4);
#endif
#endif
#ifdef DIN5
		mcu_config_input(DIN5);
#ifdef DIN5_PULLUP
		mcu_config_pullup(DIN5);
#endif
#endif
#ifdef DIN6
		mcu_config_input(DIN6);
#ifdef DIN6_PULLUP
		mcu_config_pullup(DIN6);
#endif
#endif
#ifdef DIN7
		mcu_config_input(DIN7);
#ifdef DIN7_PULLUP
		mcu_config_pullup(DIN7);
#endif
#endif
#ifdef DIN8
		mcu_config_input(DIN8);
#ifdef DIN8_PULLUP
		mcu_config_pullup(DIN8);
#endif
#endif
#ifdef DIN9
		mcu_config_input(DIN9);
#ifdef DIN9_PULLUP
		mcu_config_pullup(DIN9);
#endif
#endif
#ifdef DIN10
		mcu_config_input(DIN10);
#ifdef DIN10_PULLUP
		mcu_config_pullup(DIN10);
#endif
#endif
#ifdef DIN11
		mcu_config_input(DIN11);
#ifdef DIN11_PULLUP
		mcu_config_pullup(DIN11);
#endif
#endif
#ifdef DIN12
		mcu_config_input(DIN12);
#ifdef DIN12_PULLUP
		mcu_config_pullup(DIN12);
#endif
#endif
#ifdef DIN13
		mcu_config_input(DIN13);
#ifdef DIN13_PULLUP
		mcu_config_pullup(DIN13);
#endif
#endif
#ifdef DIN14
		mcu_config_input(DIN14);
#ifdef DIN14_PULLUP
		mcu_config_pullup(DIN14);
#endif
#endif
#ifdef DIN15
		mcu_config_input(DIN15);
#ifdef DIN15_PULLUP
		mcu_config_pullup(DIN15);
#endif
#endif

#ifdef LED
		mcu_config_output(LED);
#endif
		mcu_usart_init();
		mcu_tick_init();
		mcu_disable_probe_isr();
		mcu_enable_global_isr();
	}

	/*IO functions*/
#ifndef mcu_get_input
	uint8_t mcu_get_input(uint8_t pin)
	{
	}
#endif

#ifndef mcu_get_output
	uint8_t mcu_get_output(uint8_t pin)
	{
	}
#endif

#ifndef mcu_set_output
	void mcu_set_output(uint8_t pin)
	{
	}
#endif

#ifndef mcu_clear_output
	void mcu_clear_output(uint8_t pin)
	{
	}
#endif

#ifndef mcu_toggle_output
	void mcu_toggle_output(uint8_t pin)
	{
	}
#endif

#ifndef mcu_enable_probe_isr
	void mcu_enable_probe_isr(void)
	{
	}
#endif
#ifndef mcu_disable_probe_isr
	void mcu_disable_probe_isr(void)
	{
	}
#endif

//Analog input
#ifndef mcu_get_analog
	uint8_t mcu_get_analog(uint8_t channel)
	{
	}
#endif

//PWM
#ifndef mcu_set_pwm
	void mcu_set_pwm(uint8_t pwm, uint8_t value)
	{
	}
#endif

#ifndef mcu_get_pwm
	uint8_t mcu_get_pwm(uint8_t pwm)
	{
	}
#endif

	static uint8_t mcu_tx_buffer[TX_BUFFER_SIZE];

	void mcu_usart_init(void)
	{
#ifdef COM_PORT
		/*enables RCC clocks and GPIO*/
		RCC->APB2ENR |= (RCC_APB2ENR_AFIOEN);
		RCC->COM_APB |= (COM_APBEN);
		RCC->APB2ENR |= __indirect__(TX, APB2EN);
		__indirect__(TX, GPIO)->__indirect__(TX, CR) &= ~(GPIO_RESET << ((__indirect__(TX, CROFF)) << 2));
		__indirect__(TX, GPIO)->__indirect__(TX, CR) |= (GPIO_OUTALT_PP_50MHZ << ((__indirect__(TX, CROFF)) << 2));
		RCC->APB2ENR |= __indirect__(RX, APB2EN);
		__indirect__(RX, GPIO)->__indirect__(RX, CR) &= ~(GPIO_RESET << ((__indirect__(RX, CROFF)) << 2));
		__indirect__(RX, GPIO)->__indirect__(RX, CR) |= (GPIO_IN_FLOAT << ((__indirect__(RX, CROFF)) << 2));
		/*setup UART*/
		COM_USART->CR1 = 0; //8 bits No parity M=0 PCE=0
		COM_USART->CR2 = 0; //1 stop bit STOP=00
		COM_USART->CR3 = 0;
		COM_USART->SR = 0;
		// //115200 baudrate
		float baudrate = ((float)(F_CPU >> 4) / ((float)BAUD));
		uint16_t brr = (uint16_t)baudrate;
		baudrate -= brr;
		brr <<= 4;
		brr += (uint16_t)roundf(16.0f * baudrate);
		COM_USART->BRR = brr;
#if (defined(ENABLE_SYNC_TX) || defined(ENABLE_SYNC_RX))
		NVIC_EnableIRQ(COM_IRQ);
		NVIC_SetPriority(COM_IRQ, 3);
		NVIC_ClearPendingIRQ(COM_IRQ);
#endif
		COM_USART->CR1 |= (USART_CR1_RE | USART_CR1_TE); // enable TE, RE, Oversampling 8-bit
// #ifndef ENABLE_SYNC_TX
// 		COM_USART->CR1 |= (USART_CR1_TXEIE); // enable TXEIE
// #endif
#ifndef ENABLE_SYNC_RX
		COM_USART->CR1 |= USART_CR1_RXNEIE; // enable RXNEIE
#endif
		COM_USART->CR1 |= USART_CR1_UE; //Enable UART
#else
		//configure USB as Virtual COM port
		RCC->APB1ENR &= ~RCC_APB1ENR_USBEN;
		mcu_config_input(USB_DM);
		mcu_config_input(USB_DP);
		NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
		NVIC_SetPriority(USB_HP_CAN1_TX_IRQn, 10);
		NVIC_ClearPendingIRQ(USB_HP_CAN1_TX_IRQn);
		NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
		NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 10);
		NVIC_ClearPendingIRQ(USB_LP_CAN1_RX0_IRQn);
		NVIC_EnableIRQ(USBWakeUp_IRQn);
		NVIC_SetPriority(USBWakeUp_IRQn, 10);
		NVIC_ClearPendingIRQ(USBWakeUp_IRQn);
		//Enable USB interrupts and enable usb
		USB->CNTR |= (USB_CNTR_WKUPM | USB_CNTR_SOFM | USB_CNTR_ESOFM | USB_CNTR_CTRM);
		RCC->APB1ENR |= RCC_APB1ENR_USBEN;
		tusb_init();
#endif
	}

	void mcu_putc(char c)
	{
#ifdef LED
		mcu_toggle_output(LED);
#endif
#ifdef COM_PORT
#ifdef ENABLE_SYNC_TX
		while (!(COM_USART->SR & USART_SR_TXE))
			;
#endif
		COM_OUTREG = c;
#endif
#ifdef USB_VCP
		while (!tud_cdc_write_available())
		{
			tud_task();
		}
		tud_cdc_write_char(c);
#endif
	}

	char mcu_getc(void)
	{
#ifdef LED
		mcu_toggle_output(LED);
#endif
#ifdef COM_PORT
#ifdef ENABLE_SYNC_RX
		while (!(COM_USART->SR & USART_SR_RXNE))
			;
#endif
		return COM_INREG;
#endif
#ifdef USB_VCP
		while (!tud_cdc_available())
		{
			tud_task();
		}

		return (unsigned char)tud_cdc_read_char();
#endif
	}

//ISR
//enables all interrupts on the mcu. Must be called to enable all IRS functions
#ifndef mcu_enable_global_isr
	void mcu_enable_global_isr(void)
	{
	}
#endif
//disables all ISR functions
#ifndef mcu_disable_global_isr
	void mcu_disable_global_isr(void)
	{
	}
#endif

	//Timers
	//convert step rate to clock cycles
	void mcu_freq_to_clocks(float frequency, uint16_t *ticks, uint16_t *prescaller)
	{
		//up and down counter (generates half the step rate at each event)
		uint32_t totalticks = (uint32_t)((float)(F_CPU >> 2) / frequency);
		*prescaller = 1;
		while (totalticks > 0xFFFF)
		{
			*prescaller <<= 1;
			totalticks >>= 1;
		}

		*prescaller--;
		*ticks = (uint16_t)totalticks;
	}

	//starts a constant rate pulse at a given frequency.
	void mcu_start_itp_isr(uint16_t ticks, uint16_t prescaller)
	{
		RCC->TIMER_ENREG |= TIMER_APB;
		TIMER_REG->CR1 = 0;
		TIMER_REG->DIER = 0;
		TIMER_REG->PSC = prescaller;
		TIMER_REG->ARR = ticks;
		TIMER_REG->EGR |= 0x01;
		TIMER_REG->SR &= ~0x01;

		NVIC_EnableIRQ(TIMER_IRQ);
		NVIC_SetPriority(TIMER_IRQ, 1);
		NVIC_ClearPendingIRQ(TIMER_IRQ);
		TIMER_REG->DIER |= 1;
		TIMER_REG->CR1 |= 1; //enable timer upcounter no preload
	}

	//modifies the pulse frequency
	void mcu_change_itp_isr(uint16_t ticks, uint16_t prescaller)
	{
		TIMER_REG->ARR = ticks;
		TIMER_REG->PSC = prescaller;
		TIMER_REG->EGR |= 0x01;
	}

	//stops the pulse
	void mcu_stop_itp_isr(void)
	{
		TIMER_REG->CR1 &= ~0x1;
		TIMER_REG->DIER &= ~0x1;
		TIMER_REG->SR &= ~0x01;
		NVIC_DisableIRQ(TIMER_IRQ);
	}

	//Custom delay function
	//gets the mcu running time in ms
	uint32_t mcu_millis()
	{
		uint32_t val = mcu_runtime_ms;
		return val;
	}

	void mcu_delay_ms(uint32_t miliseconds)
	{
		uint32_t start = mcu_runtime_ms;
		uint32_t end = mcu_runtime_ms;
		while (end - start < miliseconds)
		{
			mcu_dotasks();
			end = mcu_runtime_ms;
		}
	}

	void mcu_tick_init()
	{
		SysTick->CTRL = 0;
		SysTick->LOAD = (((F_CPU >> 3) / 1000) - 1);
		SysTick->VAL = 0;
		NVIC_SetPriority(SysTick_IRQn, 10);
		SysTick->CTRL = 3; //Start SysTick (ABH clock/8)
	}

#ifdef COM_PORT
#define mcu_read_available() CHECKBIT(COM_USART->SR, 5)
#define mcu_write_available() CHECKBIT(COM_USART->SR, 7)
#else
#ifdef USB_VCP
#define mcu_read_available() tud_cdc_available()
#define mcu_write_available() tud_cdc_write_available()
#endif
#endif

	void mcu_dotasks()
	{
#ifdef USB_VCP
		tud_task(); // tinyusb device task
#endif
#ifdef ENABLE_SYNC_RX
		while (mcu_read_available())
		{
			unsigned char c = mcu_getc();
			serial_rx_isr(c);
		}
#endif
#ifdef ENABLE_SYNC_TX
		if (!serial_tx_is_empty())
		{
			serial_tx_isr();
		}
#endif
	}

	//Non volatile memory
	uint8_t mcu_eeprom_getc(uint16_t address)
	{
		return 0;
	}

	void mcu_eeprom_putc(uint16_t address, uint8_t value)
	{
	}

#endif

#ifdef __cplusplus
}
#endif
