/*
	Name: mcu_stm32f10x.h
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

#include "../../../cnc.h"

#if (MCU == MCU_STM32F1X)
#include "core_cm3.h"
#include "mcumap_stm32f1x.h"

#if (INTERFACE == INTERFACE_USB)
#include "../../../tinyusb/tusb_config.h"
#include "../../../tinyusb/src/tusb.h"
#endif

#ifndef FLASH_SIZE
#error "Device FLASH size undefined"
#endif
// set the FLASH EEPROM SIZE
#define FLASH_EEPROM_SIZE 0x400

#if (FLASH_BANK1_END <= 0x0801FFFFUL)
#define FLASH_EEPROM_PAGES (((FLASH_EEPROM_SIZE - 1) >> 10) + 1)
#define FLASH_EEPROM (FLASH_BASE + (FLASH_SIZE - 1) - ((FLASH_EEPROM_PAGES << 10) - 1))
#define FLASH_PAGE_MASK (0xFFFF - (1 << 10) + 1)
#define FLASH_PAGE_OFFSET_MASK (0xFFFF & ~FLASH_PAGE_MASK)
#else
#define FLASH_EEPROM_PAGES (((FLASH_EEPROM_SIZE - 1) >> 11) + 1)
#define FLASH_EEPROM (FLASH_BASE + (FLASH_SIZE - 1) - ((FLASH_EEPROM_PAGES << 11) - 1))
#define FLASH_PAGE_MASK (0xFFFF - (1 << 11) + 1)
#define FLASH_PAGE_OFFSET_MASK (0xFFFF & ~FLASH_PAGE_MASK)
#endif

static uint8_t stm32_flash_page[FLASH_EEPROM_SIZE];
static uint16_t stm32_flash_current_page;
static bool stm32_flash_modified;

/**
 * The internal clock counter
 * Increments every millisecond
 * Can count up to almost 50 days
 **/
static volatile uint32_t mcu_runtime_ms;
volatile bool stm32_global_isr_enabled;

#define mcu_config_output_af(diopin, mode)                                                                            \
	{                                                                                                                 \
		RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;                                                                           \
		RCC->APB2ENR |= __indirect__(diopin, APB2EN);                                                                 \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) &= ~(GPIO_RESET << (__indirect__(diopin, CROFF) << 2U)); \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) |= (mode << (__indirect__(diopin, CROFF) << 2U));        \
	}

#define mcu_config_input_af(diopin)                                                                                     \
	{                                                                                                                   \
		RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;                                                                             \
		RCC->APB2ENR |= __indirect__(diopin, APB2EN);                                                                   \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) &= ~(GPIO_RESET << (__indirect__(diopin, CROFF) << 2U));   \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) |= (GPIO_IN_FLOAT << (__indirect__(diopin, CROFF) << 2U)); \
	}

#define mcu_config_pullup(diopin)                                                                                     \
	{                                                                                                                 \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) &= ~(GPIO_RESET << (__indirect__(diopin, CROFF) << 2U)); \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) |= (GPIO_IN_PUP << (__indirect__(diopin, CROFF) << 2U)); \
		__indirect__(diopin, GPIO)->BSRR = (1U << __indirect__(diopin, BIT));                                         \
	}

#define mcu_config_pwm(diopin)                                                                                                   \
	{                                                                                                                            \
		RCC->APB2ENR |= 0x1U;                                                                                                    \
		__indirect__(diopin, ENREG) |= __indirect__(diopin, APBEN);                                                              \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) &= ~(GPIO_RESET << ((__indirect__(diopin, CROFF)) << 2U));          \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) |= (GPIO_OUTALT_PP_50MHZ << ((__indirect__(diopin, CROFF)) << 2U)); \
		__indirect__(diopin, TIMREG)->CR1 = 0;                                                                                   \
		__indirect__(diopin, TIMREG)->PSC = (uint16_t)(F_CPU / 1000000UL) - 1;                                                   \
		__indirect__(diopin, TIMREG)->ARR = (uint16_t)(1000000UL / __indirect__(diopin, FREQ)) - 1;                              \
		__indirect__(diopin, TIMREG)->__indirect__(diopin, CCR) = 0;                                                             \
		__indirect__(diopin, TIMREG)->__indirect__(diopin, CCMREG) = __indirect__(diopin, MODE);                                 \
		__indirect__(diopin, TIMREG)->CCER |= (1U << ((__indirect__(diopin, CHANNEL) - 1) << 2));                                \
		__indirect__(diopin, TIMREG)->BDTR |= (1 << 15);                                                                         \
		__indirect__(diopin, TIMREG)->CR1 |= 0x01U;                                                                              \
		__indirect__(diopin, ENOUTPUT);                                                                                          \
	}

#define mcu_config_input_isr(diopin)                                                                            \
	{                                                                                                           \
		RCC->APB2ENR |= 0x1U;                                                                                   \
		AFIO->EXTICR[(__indirect__(diopin, EXTIREG))] &= ~(0xF << (((__indirect__(diopin, BIT)) & 0x03) << 2)); \
		AFIO->EXTICR[(__indirect__(diopin, EXTIREG))] |= (__indirect__(diopin, EXTIVAL));                       \
		SETBIT(EXTI->RTSR, __indirect__(diopin, BIT));                                                          \
		SETBIT(EXTI->FTSR, __indirect__(diopin, BIT));                                                          \
		SETBIT(EXTI->IMR, __indirect__(diopin, BIT));                                                           \
		NVIC_SetPriority(__indirect__(diopin, IRQ), 5);                                                         \
		NVIC_ClearPendingIRQ(__indirect__(diopin, IRQ));                                                        \
		NVIC_EnableIRQ(__indirect__(diopin, IRQ));                                                              \
	}

#define mcu_config_analog(diopin)                                                                                       \
	{                                                                                                                   \
		RCC->CFGR &= ~(RCC_CFGR_ADCPRE);                                                                                \
		RCC->CFGR |= RCC_CFGR_ADCPRE_DIV8;                                                                              \
		RCC->APB2ENR |= (RCC_APB2ENR_ADC1EN | __indirect__(diopin, APB2EN) | 0x1U);                                     \
		ADC1->SQR1 = 1; /*one conversion*/                                                                              \
		ADC1->SMPR1 = 0x00ffffff & 0x36DB6DB6;                                                                          \
		ADC1->SMPR2 = 0x36DB6DB6;                                                                                       \
		ADC1->CR2 &= ~ADC_CR2_CONT; /*single conversion mode*/                                                          \
		__indirect__(diopin, GPIO)->__indirect__(diopin, CR) &= ~(GPIO_RESET << ((__indirect__(diopin, CROFF)) << 2U)); \
		ADC1->CR2 |= ADC_CR2_ADON;	 /*enable adc*/                                                                     \
		ADC1->CR2 |= ADC_CR2_RSTCAL; /*reset calibration*/                                                              \
		while (ADC1->CR2 & ADC_CR2_RSTCAL)                                                                              \
			;                                                                                                           \
		ADC1->CR2 |= ADC_CR2_CAL; /*start calibration*/                                                                 \
		while (ADC1->CR2 & ADC_CR2_CAL)                                                                                 \
			;                                                                                                           \
		ADC1->CR2 |= (ADC_CR2_EXTSEL | ADC_CR2_EXTTRIG); /*external start trigger software*/                            \
	}

/**
 * The isr functions
 * The respective IRQHandler will execute these functions
 **/
#if (INTERFACE == INTERFACE_USART)
void MCU_SERIAL_ISR(void)
{
	mcu_disable_global_isr();
#ifndef ENABLE_SYNC_RX
	if (COM_USART->SR & USART_SR_RXNE)
	{
		unsigned char c = COM_INREG;
		mcu_com_rx_cb(c);
	}
#endif

#ifndef ENABLE_SYNC_TX
	if ((COM_USART->SR & USART_SR_TXE) && (COM_USART->CR1 & USART_CR1_TXEIE))
	{
		COM_USART->CR1 &= ~(USART_CR1_TXEIE);
		mcu_com_tx_cb();
	}
#endif
	mcu_enable_global_isr();
}
#elif (INTERFACE == INTERFACE_USB)
void USB_HP_CAN1_TX_IRQHandler(void)
{
	mcu_disable_global_isr();
	tud_int_handler(0);
	mcu_enable_global_isr();
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
	mcu_disable_global_isr();
	tud_int_handler(0);
	mcu_enable_global_isr();
}

void USBWakeUp_IRQHandler(void)
{
	mcu_disable_global_isr();
	tud_int_handler(0);
	mcu_enable_global_isr();
}

#endif

// define the mcu internal servo variables
#if SERVOS_MASK > 0

static uint8_t mcu_servos[6];

static FORCEINLINE void mcu_clear_servos()
{
#if SERVO0 >= 0
	mcu_clear_output(SERVO0);
#endif
#if SERVO1 >= 0
	mcu_clear_output(SERVO1);
#endif
#if SERVO2 >= 0
	mcu_clear_output(SERVO2);
#endif
#if SERVO3 >= 0
	mcu_clear_output(SERVO3);
#endif
#if SERVO4 >= 0
	mcu_clear_output(SERVO4);
#endif
#if SERVO5 >= 0
	mcu_clear_output(SERVO5);
#endif
}

// starts a constant rate pulse at a given frequency.
void servo_timer_init(void)
{
	RCC->SERVO_TIMER_ENREG |= SERVO_TIMER_APB;
	SERVO_TIMER_REG->CR1 = 0;
	SERVO_TIMER_REG->DIER = 0;
	SERVO_TIMER_REG->PSC = (F_CPU / 255000) - 1;
	SERVO_TIMER_REG->ARR = 255;
	SERVO_TIMER_REG->EGR |= 0x01;
	SERVO_TIMER_REG->SR &= ~0x01;
}

void servo_start_timeout(uint8_t val)
{
	SERVO_TIMER_REG->ARR = (val << 1) + 125;
	NVIC_SetPriority(SERVO_TIMER_IRQ, 10);
	NVIC_ClearPendingIRQ(SERVO_TIMER_IRQ);
	NVIC_EnableIRQ(SERVO_TIMER_IRQ);
	SERVO_TIMER_REG->DIER |= 1;
	SERVO_TIMER_REG->CR1 |= 1; // enable timer upcounter no preload
}

void MCU_SERVO_ISR(void)
{
	mcu_enable_global_isr();
	if ((SERVO_TIMER_REG->SR & 1))
	{
		mcu_clear_servos();
		SERVO_TIMER_REG->DIER = 0;
		SERVO_TIMER_REG->SR = 0;
		SERVO_TIMER_REG->CR1 = 0;
	}
}

#endif

void MCU_ITP_ISR(void)
{
	mcu_disable_global_isr();

	static bool resetstep = false;
	if ((ITP_TIMER_REG->SR & 1))
	{
		if (!resetstep)
			mcu_step_cb();
		else
			mcu_step_reset_cb();
		resetstep = !resetstep;
	}
	ITP_TIMER_REG->SR = 0;

	mcu_enable_global_isr();
}

#define LIMITS_EXTIBITMASK (LIMIT_X_EXTIBITMASK | LIMIT_Y_EXTIBITMASK | LIMIT_Z_EXTIBITMASK | LIMIT_X2_EXTIBITMASK | LIMIT_Y2_EXTIBITMASK | LIMIT_Z2_EXTIBITMASK | LIMIT_A_EXTIBITMASK | LIMIT_B_EXTIBITMASK | LIMIT_C_EXTIBITMASK)
#define CONTROLS_EXTIBITMASK (ESTOP_EXTIBITMASK | SAFETY_DOOR_EXTIBITMASK | FHOLD_EXTIBITMASK | CS_RES_EXTIBITMASK)
#define DIN_IO_EXTIBITMASK (DIN0_EXTIBITMASK | DIN1_EXTIBITMASK | DIN2_EXTIBITMASK | DIN3_EXTIBITMASK | DIN4_EXTIBITMASK | DIN5_EXTIBITMASK | DIN6_EXTIBITMASK | DIN7_EXTIBITMASK)
#define ALL_EXTIBITMASK (LIMITS_EXTIBITMASK | CONTROLS_EXTIBITMASK | PROBE_EXTIBITMASK | DIN_IO_EXTIBITMASK)

#if (ALL_EXTIBITMASK != 0)
static void mcu_input_isr(void)
{
	mcu_disable_global_isr();
#if (LIMITS_EXTIBITMASK != 0)
	if (EXTI->PR & LIMITS_EXTIBITMASK)
	{
		mcu_limits_changed_cb();
	}
#endif
#if (CONTROLS_EXTIBITMASK != 0)
	if (EXTI->PR & CONTROLS_EXTIBITMASK)
	{
		mcu_controls_changed_cb();
	}
#endif
#if (PROBE_EXTIBITMASK & 0x01)
	if (EXTI->PR & PROBE_EXTIBITMASK)
	{
		mcu_probe_changed_cb();
	}
#endif
#if (DIN_IO_EXTIBITMASK != 0)
	if (EXTI->PR & DIN_IO_EXTIBITMASK)
	{
		mcu_inputs_changed_cb();
	}
#endif

	EXTI->PR = ALL_EXTIBITMASK;
	mcu_enable_global_isr();
}

#if (ALL_EXTIBITMASK & 0x0001)
void EXTI0_IRQHandler(void)
{
	mcu_input_isr();
}
#endif
#if (ALL_EXTIBITMASK & 0x0002)
void EXTI1_IRQHandler(void)
{
	mcu_input_isr();
}
#endif
#if (ALL_EXTIBITMASK & 0x0004)
void EXTI2_IRQHandler(void)
{
	mcu_input_isr();
}
#endif
#if (ALL_EXTIBITMASK & 0x0008)
void EXTI3_IRQHandler(void)
{
	mcu_input_isr();
}
#endif
#if (ALL_EXTIBITMASK & 0x0010)
void EXTI4_IRQHandler(void)
{
	mcu_input_isr();
}
#endif
#if (ALL_EXTIBITMASK & 0x03E0)
void EXTI9_5_IRQHandler(void)
{
	mcu_input_isr();
}
#endif
#if (ALL_EXTIBITMASK & 0xFC00)
void EXTI15_10_IRQHandler(void)
{
	mcu_input_isr();
}
#endif
#endif

#ifndef ARDUINO_ARCH_STM32
void SysTick_Handler(void)
#else
void osSystickHandler(void)
#endif
{
	mcu_disable_global_isr();
#if SERVOS_MASK > 0
	static uint8_t ms_servo_counter = 0;
	uint8_t servo_counter = ms_servo_counter;

	switch (servo_counter)
	{
#if SERVO0 >= 0
	case SERVO0_FRAME:
		servo_start_timeout(mcu_servos[0]);
		mcu_set_output(SERVO0);
		break;
#endif
#if SERVO1 >= 0
	case SERVO1_FRAME:
		mcu_set_output(SERVO1);
		servo_start_timeout(mcu_servos[1]);
		break;
#endif
#if SERVO2 >= 0
	case SERVO2_FRAME:
		mcu_set_output(SERVO2);
		servo_start_timeout(mcu_servos[2]);
		break;
#endif
#if SERVO3 >= 0
	case SERVO3_FRAME:
		mcu_set_output(SERVO3);
		servo_start_timeout(mcu_servos[3]);
		break;
#endif
#if SERVO4 >= 0
	case SERVO4_FRAME:
		mcu_set_output(SERVO4);
		servo_start_timeout(mcu_servos[4]);
		break;
#endif
#if SERVO5 >= 0
	case SERVO5_FRAME:
		mcu_set_output(SERVO5);
		servo_start_timeout(mcu_servos[5]);
		break;
#endif
	}

	servo_counter++;
	ms_servo_counter = (servo_counter != 20) ? servo_counter : 0;

#endif
	uint32_t millis = mcu_runtime_ms;
	millis++;
	mcu_runtime_ms = millis;
	mcu_rtc_cb(millis);
	mcu_enable_global_isr();
}

/**
 *
 * Initializes the mcu:
 *   1. Configures all IO
 *   2. Configures UART/USB
 *   3. Starts internal clock (RTC)
 **/
static void mcu_rtc_init(void);
static void mcu_usart_init(void);

#if (INTERFACE == INTERFACE_USART)
#define APB2_PRESCALER RCC_CFGR_PPRE2_DIV2
#else
#define APB2_PRESCALER RCC_CFGR_PPRE2_DIV1
#endif

void mcu_clocks_init()
{
	/* Reset the RCC clock configuration to the default reset state */
	/* Set HSION bit */
	RCC->CR |= (uint32_t)0x00000001;
	/* Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits */
	RCC->CFGR &= (uint32_t)0xF8FF0000;
	/* Reset HSEON, CSSON and PLLON bits */
	RCC->CR &= (uint32_t)0xFEF6FFFF;
	/* Reset HSEBYP bit */
	RCC->CR &= (uint32_t)0xFFFBFFFF;
	/* Disable all interrupts and clear pending bits */
	RCC->CIR = 0x009F0000;
	/* Enable HSE */
	RCC->CR |= ((uint32_t)RCC_CR_HSEON);
	/* Wait till HSE is ready */
	while (!(RCC->CR & RCC_CR_HSERDY))
		;
	/* Configure the Flash Latency cycles and enable prefetch buffer */
	FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_2;
	/* Configure the System clock frequency, HCLK, PCLK2 and PCLK1 prescalers */
	/* HCLK = SYSCLK, PCLK2 = HCLK, PCLK1 = HCLK / 2
	 * If crystal is 16MHz, add in PLLXTPRE flag to prescale by 2
	 */
	RCC->CFGR = (uint32_t)(RCC_CFGR_HPRE_DIV1 |
						   APB2_PRESCALER |
						   RCC_CFGR_PPRE1_DIV2 |
						   RCC_CFGR_PLLSRC |
						   RCC_CFGR_PLLMULL9);
	/* Enable PLL */
	RCC->CR |= RCC_CR_PLLON;
	/* Wait till PLL is ready */
	while (!(RCC->CR & RCC_CR_PLLRDY))
		;
	/* Select PLL as system clock source */
	RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;
	/* Wait till PLL is used as system clock source */
	while (!(RCC->CFGR & (uint32_t)RCC_CFGR_SWS))
		;
}

void mcu_usart_init(void)
{
#if (INTERFACE == INTERFACE_USART)
	/*enables RCC clocks and GPIO*/
	mcu_config_output_af(TX, GPIO_OUTALT_OD_50MHZ);
	mcu_config_input_af(RX);
#ifdef COM_REMAP
	__indirect__(diopin, GPIO)->__indirect__(diopin, MAPR) |= COM_REMAP;
#endif
	RCC->COM_APB |= (COM_APBEN);
	/*setup UART*/
	COM_USART->CR1 = 0; // 8 bits No parity M=0 PCE=0
	COM_USART->CR2 = 0; // 1 stop bit STOP=00
	COM_USART->CR3 = 0;
	COM_USART->SR = 0;
	// //115200 baudrate
	float baudrate = ((float)(F_CPU >> 5) / ((float)BAUDRATE));
	uint16_t brr = (uint16_t)baudrate;
	baudrate -= brr;
	brr <<= 4;
	brr += (uint16_t)roundf(16.0f * baudrate);
	COM_USART->BRR = brr;
#ifndef ENABLE_SYNC_RX
	COM_USART->CR1 |= USART_CR1_RXNEIE; // enable RXNEIE
#endif
#if (!defined(ENABLE_SYNC_TX) || !defined(ENABLE_SYNC_RX))
	NVIC_SetPriority(COM_IRQ, 3);
	NVIC_ClearPendingIRQ(COM_IRQ);
	NVIC_EnableIRQ(COM_IRQ);
#endif
	COM_USART->CR1 |= (USART_CR1_RE | USART_CR1_TE | USART_CR1_UE); // enable TE, RE and UART
#elif (INTERFACE == INTERFACE_USB)
	// configure USB as Virtual COM port
	RCC->APB1ENR &= ~RCC_APB1ENR_USBEN;
	mcu_config_input(USB_DM);
	mcu_config_input(USB_DP);
	NVIC_SetPriority(USB_HP_CAN1_TX_IRQn, 10);
	NVIC_ClearPendingIRQ(USB_HP_CAN1_TX_IRQn);
	NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
	NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 10);
	NVIC_ClearPendingIRQ(USB_LP_CAN1_RX0_IRQn);
	NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
	NVIC_SetPriority(USBWakeUp_IRQn, 10);
	NVIC_ClearPendingIRQ(USBWakeUp_IRQn);
	NVIC_EnableIRQ(USBWakeUp_IRQn);

	// Enable USB interrupts and enable usb
	USB->CNTR |= (USB_CNTR_WKUPM | USB_CNTR_SOFM | USB_CNTR_ESOFM | USB_CNTR_CTRM);
	RCC->APB1ENR |= RCC_APB1ENR_USBEN;
	tusb_init();
#endif

	// initialize debugger clock (used by us delay)
	if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
	{
		CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
		DWT->CYCCNT = 0;
		DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	}
}

void mcu_putc(char c)
{
#if !(LED < 0)
	mcu_toggle_output(LED);
#endif
#if (INTERFACE == INTERFACE_USART)
#ifdef ENABLE_SYNC_TX
	while (!(COM_USART->SR & USART_SR_TC))
		;
#endif
	COM_OUTREG = c;
#ifndef ENABLE_SYNC_TX
	COM_USART->CR1 |= (USART_CR1_TXEIE);
#endif
#elif (INTERFACE == INTERFACE_USB)
	if (c != 0)
	{
		tud_cdc_write_char(c);
	}
	if (c == '\r' || c == 0)
	{
		tud_cdc_write_flush();
	}
#endif
}

void mcu_init(void)
{
	// make sure both APB1 and APB2 are running at the same clock (36MHz)
	mcu_clocks_init();
	stm32_flash_current_page = -1;
	stm32_global_isr_enabled = false;
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

	mcu_usart_init();
	mcu_rtc_init();
#if SERVOS_MASK > 0
	servo_timer_init();
#endif
	mcu_disable_probe_isr();
	mcu_enable_global_isr();
}

/*IO functions*/
// IO functions
void mcu_set_servo(uint8_t servo, uint8_t value)
{
#if SERVOS_MASK > 0
	mcu_servos[servo - SERVO0_UCNC_INTERNAL_PIN] = value;
#endif
}

/**
 * gets the pwm for a servo (50Hz with tON between 1~2ms)
 * can be defined either as a function or a macro call
 * */
uint8_t mcu_get_servo(uint8_t servo)
{
#if SERVOS_MASK > 0
	uint8_t offset = servo - SERVO0_UCNC_INTERNAL_PIN;

	if ((1U << offset) & SERVOS_MASK)
	{
		return mcu_servos[offset];
	}
#endif
	return 0;
}

static uint8_t mcu_tx_buffer[TX_BUFFER_SIZE];

char mcu_getc(void)
{
#if !(LED < 0)
	mcu_toggle_output(LED);
#endif
#if (INTERFACE == INTERFACE_USART)
#ifdef ENABLE_SYNC_RX
	while (!(COM_USART->SR & USART_SR_RXNE))
		;
#endif
	return COM_INREG;
#elif (INTERFACE == INTERFACE_USB)
	while (!tud_cdc_available())
	{
		tud_task();
	}

	return (unsigned char)tud_cdc_read_char();
#endif
}

// ISR
// enables all interrupts on the mcu. Must be called to enable all IRS functions
#ifndef mcu_enable_global_isr
#error "mcu_enable_global_isr undefined"
#endif
// disables all ISR functions
#ifndef mcu_disable_global_isr
#error "mcu_disable_global_isr undefined"
#endif

// Timers
// convert step rate to clock cycles
void mcu_freq_to_clocks(float frequency, uint16_t *ticks, uint16_t *prescaller)
{
	// up and down counter (generates half the step rate at each event)
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

// starts a constant rate pulse at a given frequency.
void mcu_start_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	RCC->ITP_TIMER_ENREG |= ITP_TIMER_APB;
	ITP_TIMER_REG->CR1 = 0;
	ITP_TIMER_REG->DIER = 0;
	ITP_TIMER_REG->PSC = prescaller;
	ITP_TIMER_REG->ARR = ticks;
	ITP_TIMER_REG->EGR |= 0x01;
	ITP_TIMER_REG->SR &= ~0x01;

	NVIC_SetPriority(ITP_TIMER_IRQ, 1);
	NVIC_ClearPendingIRQ(ITP_TIMER_IRQ);
	NVIC_EnableIRQ(ITP_TIMER_IRQ);

	ITP_TIMER_REG->DIER |= 1;
	ITP_TIMER_REG->CR1 |= 1; // enable timer upcounter no preload
}

// modifies the pulse frequency
void mcu_change_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	ITP_TIMER_REG->ARR = ticks;
	ITP_TIMER_REG->PSC = prescaller;
	ITP_TIMER_REG->EGR |= 0x01;
}

// stops the pulse
void mcu_stop_itp_isr(void)
{
	ITP_TIMER_REG->CR1 &= ~0x1;
	ITP_TIMER_REG->DIER &= ~0x1;
	ITP_TIMER_REG->SR &= ~0x01;
	NVIC_DisableIRQ(ITP_TIMER_IRQ);
}

// Custom delay function
// gets the mcu running time in ms
uint32_t mcu_millis()
{
	uint32_t val = mcu_runtime_ms;
	return val;
}

void mcu_rtc_init()
{
	SysTick->CTRL = 0;
	SysTick->LOAD = (((F_CPU >> 3) / 1000) - 1);
	SysTick->VAL = 0;
	NVIC_SetPriority(SysTick_IRQn, 10);
	SysTick->CTRL = 3; // Start SysTick (ABH clock/8)
}

void mcu_delay_us(uint8_t delay)
{
	uint32_t startTick = DWT->CYCCNT,
			 delayTicks = startTick + delay * (F_CPU / 1000000);

	while (DWT->CYCCNT < delayTicks)
		;
}

void mcu_dotasks()
{
#if (INTERFACE == INTERFACE_USB)
	tud_cdc_write_flush();
	tud_task(); // tinyusb device task
#endif
#ifdef ENABLE_SYNC_RX
	while (mcu_rx_ready())
	{
		unsigned char c = mcu_getc();
		mcu_com_rx_cb(c);
	}
#endif
}

// checks if the current page is loaded to ram
// if not loads it
static uint16_t mcu_access_flash_page(uint16_t address)
{
	uint16_t address_page = address & FLASH_PAGE_MASK;
	uint16_t address_offset = address & FLASH_PAGE_OFFSET_MASK;
	if (stm32_flash_current_page != address_page)
	{
		stm32_flash_modified = false;
		stm32_flash_current_page = address_page;
		uint16_t counter = (uint16_t)(FLASH_EEPROM_SIZE >> 2);
		uint32_t *ptr = ((uint32_t *)&stm32_flash_page[0]);
		volatile uint32_t *eeprom = ((volatile uint32_t *)(FLASH_EEPROM + address_page));
		while (counter--)
		{
			*ptr = *eeprom;
			eeprom++;
			ptr++;
		}
	}

	return address_offset;
}

// Non volatile memory
uint8_t mcu_eeprom_getc(uint16_t address)
{
	uint16_t offset = mcu_access_flash_page(address);
	return stm32_flash_page[offset];
}

static void mcu_eeprom_erase(uint16_t address)
{
	while (FLASH->SR & FLASH_SR_BSY)
		; // wait while busy
	// unlock flash if locked
	if (FLASH->CR & FLASH_CR_LOCK)
	{
		FLASH->KEYR = 0x45670123;
		FLASH->KEYR = 0xCDEF89AB;
	}
	FLASH->CR = 0;			   // Ensure PG bit is low
	FLASH->CR |= FLASH_CR_PER; // set the PER bit
	FLASH->AR = (FLASH_EEPROM + address);
	FLASH->CR |= FLASH_CR_STRT; // set the start bit
	while (FLASH->SR & FLASH_SR_BSY)
		; // wait while busy
	FLASH->CR = 0;
}

void mcu_eeprom_putc(uint16_t address, uint8_t value)
{
	uint16_t offset = mcu_access_flash_page(address);

	if (stm32_flash_page[offset] != value)
	{
		stm32_flash_modified = true;
	}

	stm32_flash_page[offset] = value;
}

void mcu_eeprom_flush()
{
	if (stm32_flash_modified)
	{
		mcu_eeprom_erase(stm32_flash_current_page);
		volatile uint16_t *eeprom = ((volatile uint16_t *)(FLASH_EEPROM + stm32_flash_current_page));
		uint16_t *ptr = ((uint16_t *)&stm32_flash_page[0]);
		uint16_t counter = (uint16_t)(FLASH_EEPROM_SIZE >> 1);
		while (counter--)
		{
			while (FLASH->SR & FLASH_SR_BSY)
				; // wait while busy
			mcu_disable_global_isr();
			// unlock flash if locked
			if (FLASH->CR & FLASH_CR_LOCK)
			{
				FLASH->KEYR = 0x45670123;
				FLASH->KEYR = 0xCDEF89AB;
			}
			FLASH->CR = 0;
			FLASH->CR |= FLASH_CR_PG; // Ensure PG bit is high
			*eeprom = *ptr;
			while (FLASH->SR & FLASH_SR_BSY)
				; // wait while busy
			mcu_enable_global_isr();
			if (FLASH->SR & FLASH_SR_PGERR)
				protocol_send_error(42); // STATUS_SETTING_WRITE_FAIL
			if (FLASH->SR & FLASH_SR_WRPRTERR)
				protocol_send_error(43); // STATUS_SETTING_PROTECTED_FAIL
			FLASH->CR = 0;				 // Ensure PG bit is low
			FLASH->SR = 0;
			eeprom++;
			ptr++;
		}
		stm32_flash_modified = false;
		// Restore interrupt flag state.*/
	}
}

#endif
