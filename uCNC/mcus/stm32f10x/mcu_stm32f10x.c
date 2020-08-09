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

#include "config.h"
#include "boarddefs.h"
#include "mcumap_stm32f10x.h"
#include "mcu.h"

extern void serial_rx_isr(unsigned char c);
extern void serial_tx_isr(void);
extern void itp_step_isr(void);
extern void itp_step_reset_isr(void);
extern void io_limits_isr(void);
extern void io_controls_isr(void);
extern void io_probe_isr(void);

#define GPIO_RESET 0xfU
#define GPIO_OUT_PP_10MHZ 0x1U
#define GPIO_OUTALT_PP_2MHZ 0xaU
#define GPIO_OUTALT_OD_2MHZ 0xeU
#define GPIO_IN_FLOAT 0x4U
#define GPIO_IN_PP 0x8U
#define GPIO_IN_ANALOG 0 //not needed after reseting bits

#define mcu_config_output(diopin)                                                                                           \
	{                                                                                                                       \
		RCC->APB2ENR |= __indirect__(diopin, APB2EN);                                                                       \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) &= ~(GPIO_RESET << (__indirect__(diopin, CROFF) << 2U));       \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) |= (GPIO_OUT_PP_10MHZ << (__indirect__(diopin, CROFF) << 2U)); \
	}
#define mcu_config_input(diopin)                                                                                        \
	{                                                                                                                   \
		RCC->APB2ENR |= __indirect__(diopin, APB2EN);                                                                   \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) &= ~(GPIO_RESET << (__indirect__(diopin, CROFF) << 2U));   \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) |= (GPIO_IN_FLOAT << (__indirect__(diopin, CROFF) << 2U)); \
	}
#define mcu_config_pullup(diopin)                                                                                        \
	{                                                                                                                    \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) &= ~(GPIO_RESET << (__indirect__(diopin, CROFF) << 2U)); \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) |= (GPIO_IN_PP << (__indirect__(diopin, CROFF) << 2U));     \
		__indirect__(diopin, GPIO)->BSRR = (1U << __indirect__(diopin, BIT));                                            \
	}
#define mcu_config_pwm(diopin)                                                                                                  \
	{                                                                                                                           \
		RCC->APB2ENR |= 0x1U;                                                                                                   \
		__indirect__(diopin, ENREG) |= __indirect__(diopin, APBEN);                                                             \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) &= ~(GPIO_RESET << ((__indirect__(diopin, CROFF)) << 2U));         \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) |= (GPIO_OUTALT_PP_2MHZ << ((__indirect__(diopin, CROFF)) << 2U)); \
		__indirect__(diopin, TIMREG)->CR1 = 0;                                                                                  \
		__indirect__(diopin, TIMREG)->PSC = (uint16_t)(F_CPU / 1000000UL) - 1;                                                  \
		__indirect__(diopin, TIMREG)->ARR = (uint16_t)(1000000UL / __indirect__(diopin, FREQ)) - 1;                             \
		__indirect__(diopin, TIMREG)->__indirect__(diopin, CCR) = 0;                                                            \
		__indirect__(diopin, TIMREG)->__indirect__(diopin, CCMREG) = __indirect__(diopin, MODE);                                \
		__indirect__(diopin, TIMREG)->CCER |= (1U << ((__indirect__(diopin, CHANNEL) - 1) << 2));                               \
		__indirect__(diopin, TIMREG)->BDTR |= (1 << 15);                                                                        \
		__indirect__(diopin, TIMREG)->CR1 |= 0x01U;                                                                             \
		__indirect__(diopin, ENOUTPUT);                                                                                         \
	}

#define mcu_config_input_isr(diopin)                                                                                      \
	{                                                                                                                     \
		RCC->APB2ENR |= 0x1U;                                                                                             \
		AFIO->EXTICR[__indirect__(diopin, EXTIREG)] &= ~(0xF << ((__indirect__(diopin, BIT) & 0x03) << 2));               \
		AFIO->EXTICR[__indirect__(diopin, EXTIREG)] |= (__indirect__(diopin, EXTIVAL));                                   \
		SETBIT(EXTI->RTSR, __indirect__(diopin, BIT));                                                                    \
		SETBIT(EXTI->FTSR, __indirect__(diopin, BIT));                                                                    \
		SETBIT(EXTI->IMR, __indirect__(diopin, BIT));                                                                     \
		NVIC->ISER[((uint32_t)(__indirect__(diopin, IRQ)) >> 5)] = (1 << ((uint32_t)(__indirect__(diopin, IRQ)) & 0x1F)); \
		NVIC->IP[(uint32_t)(__indirect__(diopin, IRQ))] = ((1 << (8 - __NVIC_PRIO_BITS)) & 0xff);                         \
		NVIC->ICPR[((uint32_t)(__indirect__(diopin, IRQ)) >> 5)] = (1 << ((uint32_t)(__indirect__(diopin, IRQ)) & 0x1F)); \
	}

#define mcu_config_analog(diopin)                                                                                       \
	{                                                                                                                   \
		RCC->CFGR &= ~(0x11U << 14U);                                                                                   \
		RCC->CFGR |= (0x10U << 14U);                                                                                    \
		RCC->APB2ENR |= ((0x1U << (8U + __indirect__(diopin, ADCEN))) | __indirect__(diopin, APB2EN) | 0x1U);           \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) &= ~(GPIO_RESET << ((__indirect__(diopin, CROFF)) << 2U)); \
		__indirect__(diopin, ADC)->CR2 |= 0x1;                                                                          \
	}

#ifdef COM_PORT
void mcu_serial_isr(void)
{
	if (COM_USART->SR & (1U << 5U))
	{
		unsigned char c = COM_INREG;
		serial_rx_isr(c);
	}
	if (COM_USART->SR & (1U << 7U))
	{
		serial_tx_isr();
	}
	COM_USART->SR = 0;
	NVIC->ICPR[((uint32_t)(COM_IRQ) >> 5)] = (1 << ((uint32_t)(COM_IRQ)&0x1F));
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
	NVIC->ICPR[((uint32_t)(TIMER_IRQ) >> 5)] = (1 << ((uint32_t)(TIMER_IRQ)&0x1F));
	mcu_enable_interrupts();
}

#define LIMITS_EXTIBITMASK (LIMIT_X_EXTIBITMASK | LIMIT_Y_EXTIBITMASK | LIMIT_Z_EXTIBITMASK | LIMIT_X2_EXTIBITMASK | LIMIT_Y2_EXTIBITMASK | LIMIT_Z2_EXTIBITMASK | LIMIT_A_EXTIBITMASK | LIMIT_B_EXTIBITMASK | LIMIT_C_EXTIBITMASK)
#define CONTROLS_EXTIBITMASK (ESTOP_EXTIBITMASK | SAFETY_DOOR_EXTIBITMASK | FHOLD_EXTIBITMASK | CS_RES_EXTIBITMASK)
#define ALL_EXTIBITMASK (LIMITS_EXTIBITMASK | CONTROLS_EXTIBITMASK | PROBE_EXTIBITMASK)

#if (ALL_EXTIBITMASK!=0)
void mcu_input_isr(void)
{
#if (LIMITS_EXTIBITMASK!=0)
	if (EXTI->PR & LIMITS_EXTIBITMASK)
	{
		io_limits_isr();
		EXTI->PR = LIMITS_EXTIBITMASK;
	}
#endif
#if (CONTROLS_EXTIBITMASK!=0)
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
#if(ALL_EXTIBITMASK==0x0001)
NVIC->ICPR[((uint32_t)(EXTI0_IRQn) >> 5)] = (1 << ((uint32_t)(EXTI0_IRQn)&0x1F));
#endif
#if(ALL_EXTIBITMASK==0x0002)
NVIC->ICPR[((uint32_t)(EXTI1_IRQn) >> 5)] = (1 << ((uint32_t)(EXTI1_IRQn)&0x1F));
#endif
#if(ALL_EXTIBITMASK==0x0004)
NVIC->ICPR[((uint32_t)(EXTI2_IRQn) >> 5)] = (1 << ((uint32_t)(EXTI2_IRQn)&0x1F));
#endif
#if(ALL_EXTIBITMASK==0x0008)
NVIC->ICPR[((uint32_t)(EXTI3_IRQn) >> 5)] = (1 << ((uint32_t)(EXTI3_IRQn)&0x1F));
#endif
#if(ALL_EXTIBITMASK==0x0010)
NVIC->ICPR[((uint32_t)(EXTI4_IRQn) >> 5)] = (1 << ((uint32_t)(EXTI4_IRQn)&0x1F));
#endif
#if(ALL_EXTIBITMASK&0x03E0)
NVIC->ICPR[((uint32_t)(EXTI9_5_IRQn) >> 5)] = (1 << ((uint32_t)(EXTI9_5_IRQn)&0x1F));
#endif
#if(ALL_EXTIBITMASK==0xFC00)
NVIC->ICPR[((uint32_t)(EXTI15_10_IRQn) >> 5)] = (1 << ((uint32_t)(EXTI15_10_IRQn)&0x1F));
#endif
}
#endif


static void mcu_usart_init(void)
{
#ifdef COM_PORT
	RCC->APB2ENR |= (RCC_APB2ENR_AFIOEN);
	RCC->COM_APB |= (COM_APBEN);
	RCC->APB2ENR |= __indirect__(TX, APB2EN);
	__indirect__(TX, GPIO)->__indirect__(TX, CR) &= ~(GPIO_RESET << ((__indirect__(TX, CROFF)) << 2));
	__indirect__(TX, GPIO)->__indirect__(TX, CR) |= (GPIO_OUTALT_PP_10MHZ << ((__indirect__(TX, CROFF)) << 2));
	RCC->APB2ENR |= __indirect__(RX, APB2EN);
	__indirect__(RX, GPIO)->__indirect__(RX, CR) &= ~(GPIO_RESET << ((__indirect__(RX, CROFF)) << 2));
	__indirect__(RX, GPIO)->__indirect__(RX, CR) |= (GPIO_IN_FLOAT << ((__indirect__(RX, CROFF)) << 2));
	COM_USART->CR1 = 0; //8 bits No parity M=0 PCE=0
	COM_USART->CR2 = 0; //1 stop bit STOP=00
	COM_USART->CR3 = 0;
	COM_USART->SR = 0;
	// //115200 baudrate
	float baudrate = ((float)F_CPU / (16.0f * (float)BAUD));
	uint16_t brr = (uint16_t)baudrate;
	baudrate -= brr;
	brr <<= 4;
	brr += (uint16_t)roundf(16.0f * baudrate);
	COM_USART->BRR = brr;
	NVIC->ISER[((uint32_t)(COM_IRQ) >> 5)] = (1 << ((uint32_t)(COM_IRQ)&0x1F));
	NVIC->IP[(uint32_t)(COM_IRQ)] = ((1 << (8 - __NVIC_PRIO_BITS)) & 0xff);
	NVIC->ICPR[((uint32_t)(COM_IRQ) >> 5)] = (1 << ((uint32_t)(COM_IRQ)&0x1F));
	COM_USART->CR1 |= 0x202C; // enable TE, RE, UE, TXEIE
#endif
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

#ifdef COM_PORT
	mcu_usart_init();
#endif
                 
	mcu_enable_interrupts();
#ifdef LED
	mcu_clear_output(LED);
#endif
}

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

//Communication functions
void mcu_start_send(void)
{
//not used with USB VCP
#ifdef COM_PORT
	COM_USART->CR1 |= (1 << 7);
#endif
}
void mcu_stop_send(void)
{
//not used with USB VCP
#ifdef COM_PORT
	COM_USART->CR1 &= ~(1 << 7);
#endif
}

#ifdef TX_BUFFER_SIZE
#undef TX_BUFFER_SIZE
#endif
#define TX_BUFFER_SIZE 128
static uint8_t mcu_tx_buffer[TX_BUFFER_SIZE];

extern uint8_t CDC_Transmit_FS(uint8_t *Buf, uint16_t Len);

void mcu_putc(char c)
{
#ifdef LED
	mcu_toggle_output(LED);
#endif
#ifdef COM_PORT
	while (!(COM_USART->SR & (1 << 7)))
		;
	COM_OUTREG = c;
#else
	static uint16_t i = 0;
	mcu_tx_buffer[i] = (uint8_t)c;
	i++;
	if (c == '\n' || i == (TX_BUFFER_SIZE - 1))
	{
		mcu_tx_buffer[i] = 0;
		while (CDC_Transmit_FS(mcu_tx_buffer, i))
			;
		i = 0;
	}
#endif
}

char mcu_getc(void)
{
#ifdef COM_PORT
	while (!(COM_USART->SR & (1 << 5)))
		;
	return COM_INREG;
#endif
}

//ISR
//enables all interrupts on the mcu. Must be called to enable all IRS functions
#ifndef mcu_enable_interrupts
void mcu_enable_interrupts(void)
{
}
#endif
//disables all ISR functions
#ifndef mcu_disable_interrupts
void mcu_disable_interrupts(void)
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
void mcu_start_step_ISR(uint16_t ticks, uint16_t prescaller)
{
	RCC->TIMER_ENREG |= TIMER_APB;
	TIMER_REG->CR1 = 0;
	TIMER_REG->DIER = 0;
	TIMER_REG->PSC = prescaller;
	TIMER_REG->ARR = ticks;
	TIMER_REG->EGR |= 0x01;
	TIMER_REG->SR &= ~0x01;

	NVIC->ISER[((uint32_t)(TIMER_IRQ) >> 5)] = (1 << ((uint32_t)(TIMER_IRQ)&0x1F));
	NVIC->IP[(uint32_t)(TIMER_IRQ)] = ((1 << (8 - __NVIC_PRIO_BITS)) & 0xff);
	NVIC->ICPR[((uint32_t)(TIMER_IRQ) >> 5)] = (1 << ((uint32_t)(TIMER_IRQ)&0x1F));
	TIMER_REG->DIER |= 1;
	TIMER_REG->CR1 |= 1; //enable timer upcounter no preload
}

//modifies the pulse frequency
void mcu_change_step_ISR(uint16_t ticks, uint16_t prescaller)
{
	TIMER_REG->ARR = ticks;
	TIMER_REG->PSC = prescaller;
	TIMER_REG->EGR |= 0x01;
}

//stops the pulse
void mcu_step_stop_ISR(void)
{
	TIMER_REG->CR1 &= ~0x1;
	TIMER_REG->DIER &= ~0x1;
	TIMER_REG->SR &= ~0x01;
	NVIC->ICER[((uint32_t)(TIMER_IRQ) >> 5)] = (1 << ((uint32_t)(TIMER_IRQ)&0x1F));
}

//Custom delay function
//void mcu_delay_ms(uint16_t miliseconds);

//Non volatile memory
uint8_t mcu_eeprom_getc(uint16_t address)
{
	return 0;
}

void mcu_eeprom_putc(uint16_t address, uint8_t value)
{
}

#ifdef __PERFSTATS__
uint16_t mcu_get_step_clocks(void);
uint16_t mcu_get_step_reset_clocks(void);
#endif
