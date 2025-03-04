/*
	Name: mcu_stm32f4x.c
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

#if (MCU == MCU_STM32F4X)
#include "core_cm4.h"
#include "stm32f4xx.h"
#include "mcumap_stm32f4x.h"
#include <math.h>

#ifdef MCU_HAS_USB
#include <tusb_ucnc.h>
#endif

#ifndef FLASH_SIZE
#define FLASH_SIZE (FLASH_END - FLASH_BASE + 1)
#endif

// this is needed if a custom flash size is defined
#define FLASH_LIMIT (FLASH_BASE + FLASH_SIZE - 1)

#if (FLASH_LIMIT > FLASH_END)
#error "The set FLASH_SIZE is beyond the chip capability"
#endif

#define FLASH_EEPROM_SIZE_WORD (NVM_STORAGE_SIZE >> 2)
#define FLASH_EEPROM_SIZE_WORD_ALIGNED (FLASH_EEPROM_SIZE_WORD << 2)

#define FLASH_SECTOR_SIZE 0x20000UL
#define FLASH_SECTORS (FLASH_SIZE / FLASH_SECTOR_SIZE) + 4

#define FLASH_EEPROM_START (FLASH_LIMIT - FLASH_SECTOR_SIZE + 1)
#define FLASH_EEPROM_PER_SECTION (FLASH_SECTOR_SIZE / FLASH_EEPROM_SIZE_WORD_ALIGNED)
#define FLASH_EEPROM_END (FLASH_EEPROM_START + (FLASH_EEPROM_PER_SECTION * FLASH_EEPROM_SIZE_WORD_ALIGNED) - 1)
// read and write invert
#define READ_FLASH(ram_ptr, flash_ptr) (*ram_ptr = ~(*flash_ptr))
#define WRITE_FLASH(flash_ptr, ram_ptr) (*flash_ptr = ~(*ram_ptr))
#define EEPROM_CLEAN 0
#define EEPROM_DIRTY 1
#define EEPROM_NEEDS_NEWPAGE 2

static uint8_t stm32_eeprom_buffer[FLASH_EEPROM_SIZE_WORD_ALIGNED];
static uint32_t stm32_flash_current_offset;
static uint8_t stm32_flash_modified;
static void FORCEINLINE mcu_eeprom_init(void);

/**
 * The internal clock counter
 * Increments every millisecond
 * Can count up to almost 50 days
 **/
static volatile uint32_t mcu_runtime_ms;
volatile bool stm32_global_isr_enabled;

/**
 * The isr functions
 * The respective IRQHandler will execute these functions
 **/
#ifdef MCU_HAS_UART
#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE 64
#endif
DECL_BUFFER(uint8_t, uart_tx, UART_TX_BUFFER_SIZE);
DECL_BUFFER(uint8_t, uart_rx, RX_BUFFER_SIZE);

void MCU_SERIAL_ISR(void)
{
	__ATOMIC_FORCEON__
	{
		if (COM_UART->SR & USART_SR_RXNE)
		{
			uint8_t c = COM_INREG;
#if !defined(DETACH_UART_FROM_MAIN_PROTOCOL)
			if (mcu_com_rx_cb(c))
			{
				if (BUFFER_FULL(uart_rx))
				{
					c = OVF;
				}

				BUFFER_ENQUEUE(uart_rx, &c);
			}
#else
			mcu_uart_rx_cb(c);
#endif
		}

		if ((COM_UART->SR & USART_SR_TXE) && (COM_UART->CR1 & USART_CR1_TXEIE))
		{
			mcu_enable_global_isr();
			if (BUFFER_EMPTY(uart_tx))
			{
				COM_UART->CR1 &= ~(USART_CR1_TXEIE);
				return;
			}
			uint8_t c;
			BUFFER_DEQUEUE(uart_tx, &c);
			COM_OUTREG = c;
		}
	}
}
#endif

#ifdef MCU_HAS_UART2
#ifndef UART2_TX_BUFFER_SIZE
#define UART2_TX_BUFFER_SIZE 64
#endif
DECL_BUFFER(uint8_t, uart2_tx, UART2_TX_BUFFER_SIZE);
DECL_BUFFER(uint8_t, uart2_rx, RX_BUFFER_SIZE);

void MCU_SERIAL2_ISR(void)
{
	__ATOMIC_FORCEON__
	{
		if (COM2_UART->SR & USART_SR_RXNE)
		{
			uint8_t c = COM2_INREG;
#if !defined(DETACH_UART2_FROM_MAIN_PROTOCOL)
			if (mcu_com_rx_cb(c))
			{
				if (BUFFER_FULL(uart2_rx))
				{
					c = OVF;
				}

				BUFFER_ENQUEUE(uart2_rx, &c);
			}
#else
			mcu_uart2_rx_cb(c);
#ifndef UART2_DISABLE_BUFFER
			if (BUFFER_FULL(uart2_rx))
			{
				c = OVF;
			}

			BUFFER_ENQUEUE(uart2_rx, &c);
#endif
#endif
		}

		if ((COM2_UART->SR & USART_SR_TXE) && (COM2_UART->CR1 & USART_CR1_TXEIE))
		{
			mcu_enable_global_isr();
			if (BUFFER_EMPTY(uart2_tx))
			{
				COM2_UART->CR1 &= ~(USART_CR1_TXEIE);
				return;
			}
			uint8_t c;
			BUFFER_DEQUEUE(uart2_tx, &c);
			COM2_OUTREG = c;
		}
	}
}
#endif

#ifdef MCU_HAS_USB
void OTG_FS_IRQHandler(void)
{
	mcu_disable_global_isr();
	tusb_cdc_isr_handler();
	USB_OTG_FS->GINTSTS = 0xBFFFFFFFU;
	NVIC_ClearPendingIRQ(OTG_FS_IRQn);
	mcu_enable_global_isr();
}
#endif

// define the mcu internal servo variables
#if SERVOS_MASK > 0

static uint8_t mcu_servos[6];

static FORCEINLINE void mcu_clear_servos()
{
#if ASSERT_PIN(SERVO0)
	io_clear_output(SERVO0);
#endif
#if ASSERT_PIN(SERVO1)
	io_clear_output(SERVO1);
#endif
#if ASSERT_PIN(SERVO2)
	io_clear_output(SERVO2);
#endif
#if ASSERT_PIN(SERVO3)
	io_clear_output(SERVO3);
#endif
#if ASSERT_PIN(SERVO4)
	io_clear_output(SERVO4);
#endif
#if ASSERT_PIN(SERVO5)
	io_clear_output(SERVO5);
#endif
}

// starts a constant rate pulse at a given frequency.
void servo_timer_init(void)
{
	RCC->SERVO_TIMER_ENREG |= SERVO_TIMER_APB;
	SERVO_TIMER_REG->CR1 = 0;
	SERVO_TIMER_REG->DIER = 0;
	SERVO_TIMER_REG->PSC = (SERVO_CLOCK / 255000) - 1;
	SERVO_TIMER_REG->ARR = 255;
	SERVO_TIMER_REG->EGR |= 0x01;
	SERVO_TIMER_REG->SR &= ~0x01;
}

void servo_start_timeout(uint8_t val)
{
	SERVO_TIMER_REG->ARR = (val << 1) + 125;
	NVIC_SetPriority(MCU_SERVO_IRQ, 10);
	NVIC_ClearPendingIRQ(MCU_SERVO_IRQ);
	NVIC_EnableIRQ(MCU_SERVO_IRQ);
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
	if ((TIMER_REG->SR & 1))
	{
		if (!resetstep)
			mcu_step_cb();
		else
			mcu_step_reset_cb();
		resetstep = !resetstep;
	}
	TIMER_REG->SR = 0;

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
#if (PROBE_EXTIBITMASK != 0)
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

#if !defined(ARDUINO_ARCH_STM32) || defined(CUSTOM_PRE_MAIN)
void SysTick_IRQHandler(void)
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
#if ASSERT_PIN(SERVO0)
	case SERVO0_FRAME:
		servo_start_timeout(mcu_servos[0]);
		io_set_output(SERVO0);
		break;
#endif
#if ASSERT_PIN(SERVO1)
	case SERVO1_FRAME:
		io_set_output(SERVO1);
		servo_start_timeout(mcu_servos[1]);
		break;
#endif
#if ASSERT_PIN(SERVO2)
	case SERVO2_FRAME:
		io_set_output(SERVO2);
		servo_start_timeout(mcu_servos[2]);
		break;
#endif
#if ASSERT_PIN(SERVO3)
	case SERVO3_FRAME:
		io_set_output(SERVO3);
		servo_start_timeout(mcu_servos[3]);
		break;
#endif
#if ASSERT_PIN(SERVO4)
	case SERVO4_FRAME:
		io_set_output(SERVO4);
		servo_start_timeout(mcu_servos[4]);
		break;
#endif
#if ASSERT_PIN(SERVO5)
	case SERVO5_FRAME:
		io_set_output(SERVO5);
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

#ifdef CUSTOM_CLOCKS_INIT
#ifndef PLL_N
#error You need to setup PLL_N
#endif
#ifndef PLL_Q
#error You need to setup PLL_Q
#endif
#ifndef PLL_M
#error You need to setup PLL_M
#endif
#ifndef PLL_P
#error You need to setup PLL_P
#endif
#define APB1_PRESC ((F_CPU > 90000000UL) ? RCC_CFGR_PPRE1_DIV4 : ((F_CPU > 45000000UL) ? RCC_CFGR_PPRE1_DIV2 : RCC_CFGR_PPRE1_DIV1))
#define APB2_PRESC ((F_CPU > 90000000UL) ? RCC_CFGR_PPRE2_DIV2 : RCC_CFGR_PPRE2_DIV1)
#endif
#ifndef FLASH_LATENCY
#if(F_CPU <= 90000000UL)
#define FLASH_LATENCY FLASH_ACR_LATENCY_2WS
#elif(F_CPU <= 120000000UL)
#define FLASH_LATENCY FLASH_ACR_LATENCY_3WS
#elif(F_CPU <= 150000000UL)
#define FLASH_LATENCY FLASH_ACR_LATENCY_4WS
#elif(F_CPU <= 1800000000UL)
#define FLASH_LATENCY FLASH_ACR_LATENCY_5WS
#else
#define FLASH_LATENCY FLASH_ACR_LATENCY_6WS
#endif
#endif

void mcu_clocks_init()
{
#ifdef CUSTOM_CLOCKS_INIT
	SystemInit();
	// enable power clock
	SETFLAG(RCC->APB1ENR, RCC_APB1ENR_PWREN);
	// set voltage regulator scale 2
	SETFLAG(PWR->CR, (0x02UL << PWR_CR_VOS_Pos));

	FLASH->ACR = (FLASH_ACR_DCEN | FLASH_ACR_ICEN | FLASH_LATENCY);

	/* Enable HSE */
	SETFLAG(RCC->CR, RCC_CR_HSEON);
	CLEARFLAG(RCC->CR, RCC_CR_HSEBYP);
	/* Wait till HSE is ready */
	while (!(RCC->CR & RCC_CR_HSERDY))
		;

	RCC->PLLCFGR = 0;

	// Main PLL clock can be configured by the following formula:
	// choose HSI or HSE as the source:
	// fVCO = source_clock * (N / M)
	// Main PLL = fVCO / P
	// PLL48CLK = fVCO / Q
	// to make it simple just set M = external crystal
	// then all others are a fixed value to simplify making
	// fVCO = 336
	// Main PLL = fVCO / P = 336/4 = 84MHz
	// PLL48CLK = fVCO / Q = 336/7 = 48MHz
	// to run at other speeds different configuration must be applied but the limit for fast AHB is 180Mhz, APB is 90Mhz and slow APB is 45Mhz
	SETFLAG(RCC->PLLCFGR, (RCC_PLLCFGR_PLLSRC_HSE | (PLL_M << RCC_PLLCFGR_PLLM_Pos) | (PLL_N << RCC_PLLCFGR_PLLN_Pos) | (PLL_P << RCC_PLLCFGR_PLLP_Pos) /*main clock /4*/ | (PLL_Q << RCC_PLLCFGR_PLLQ_Pos)));
	/* Enable PLL */
	SETFLAG(RCC->CR, RCC_CR_PLLON);
	/* Wait till PLL is ready */
	while (!CHECKFLAG(RCC->CR, RCC_CR_PLLRDY))
		;

	/* Configure the System clock frequency, HCLK, PCLK2 and PCLK1 prescalers */
	/* HCLK = SYSCLK, PCLK2 = HCLK, PCLK1 = HCLK / 2
	 * If crystal is 16MHz, add in PLLXTPRE flag to prescale by 2
	 */
	SETFLAG(RCC->CFGR, (uint32_t)(RCC_CFGR_HPRE_DIV1 | APB2_PRESC | APB1_PRESC));

	/* Select PLL as system clock source */
	CLEARFLAG(RCC->CFGR, RCC_CFGR_SW);
	SETFLAG(RCC->CFGR, (0x02UL << RCC_CFGR_SW_Pos));
	/* Wait till PLL is used as system clock source */
	while ((RCC->CFGR & RCC_CFGR_SW) != (0x02UL << RCC_CFGR_SW_Pos))
		;
#endif
	SystemCoreClockUpdate();
	// initialize debugger clock (used by us delay)
	if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
	{
		CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
		DWT->CYCCNT = 0;
		DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	}
}

void mcu_usart_init(void)
{
#ifdef MCU_HAS_USB
	// configure USB as Virtual COM port
	mcu_config_input(USB_DM);
	mcu_config_input(USB_DP);
	mcu_config_af(USB_DP, GPIO_OTG_FS);
	mcu_config_af(USB_DM, GPIO_OTG_FS);
	RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;
	/* Disable all interrupts. */
	USB_OTG_FS->GINTMSK = 0U;

	/* Operate as device only mode */
	USB_OTG_FS->GUSBCFG |= USB_OTG_GUSBCFG_FDMOD;

	// /* Clear any pending interrupts */
	USB_OTG_FS->GINTSTS = 0xBFFFFFFFU;
	USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_OTGINT;

	NVIC_SetPriority(OTG_FS_IRQn, 10);
	NVIC_ClearPendingIRQ(OTG_FS_IRQn);
	NVIC_EnableIRQ(OTG_FS_IRQn);

	USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_NOVBUSSENS;
	USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_VBUSBSEN;
	USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_VBUSASEN;

	tusb_cdc_init();
#endif

#ifdef MCU_HAS_UART
	/*enables RCC clocks and GPIO*/
	RCC->COM_APB |= (COM_APBEN);
	mcu_config_af(TX, GPIO_AF_USART);
	mcu_config_af(RX, GPIO_AF_USART);
	/*setup UART*/
	COM_UART->CR1 = 0; // 8 bits No parity M=0 PCE=0
	COM_UART->CR2 = 0; // 1 stop bit STOP=00
	COM_UART->CR3 = 0;
	COM_UART->SR = 0;
	// //115200 baudrate
	float baudrate = ((float)(UART_CLOCK >> 4) / ((float)(BAUDRATE)));
	uint16_t brr = (uint16_t)baudrate;
	baudrate -= brr;
	brr <<= 4;
	brr += (uint16_t)roundf(16.0f * baudrate);
	COM_UART->BRR = brr;
	COM_UART->CR1 |= USART_CR1_RXNEIE; // enable RXNEIE
	NVIC_SetPriority(COM_IRQ, 3);
	NVIC_ClearPendingIRQ(COM_IRQ);
	NVIC_EnableIRQ(COM_IRQ);
	COM_UART->CR1 |= (USART_CR1_RE | USART_CR1_TE | USART_CR1_UE); // enable TE, RE and UART
#endif

#ifdef MCU_HAS_UART2
	/*enables RCC clocks and GPIO*/
	RCC->COM2_APB |= (COM2_APBEN);
	mcu_config_af(TX2, GPIO_AF_USART2);
	mcu_config_af(RX2, GPIO_AF_USART2);
	/*setup UART*/
	COM2_UART->CR1 = 0; // 8 bits No parity M=0 PCE=0
	COM2_UART->CR2 = 0; // 1 stop bit STOP=00
	COM2_UART->CR3 = 0;
	COM2_UART->SR = 0;
	// //115200 baudrate
	float baudrate2 = ((float)(UART2_CLOCK >> 4) / ((float)(BAUDRATE2)));
	uint16_t brr2 = (uint16_t)baudrate2;
	baudrate2 -= brr2;
	brr2 <<= 4;
	brr2 += (uint16_t)roundf(16.0f * baudrate2);
	COM2_UART->BRR = brr2;
	COM2_UART->CR1 |= USART_CR1_RXNEIE; // enable RXNEIE
	NVIC_SetPriority(COM2_IRQ, 3);
	NVIC_ClearPendingIRQ(COM2_IRQ);
	NVIC_EnableIRQ(COM2_IRQ);
	COM2_UART->CR1 |= (USART_CR1_RE | USART_CR1_TE | USART_CR1_UE); // enable TE, RE and UART
#endif
}

#ifdef MCU_HAS_USB
DECL_BUFFER(uint8_t, usb_rx, RX_BUFFER_SIZE);

uint8_t mcu_usb_getc(void)
{
	uint8_t c = 0;
	BUFFER_DEQUEUE(usb_rx, &c);
	return c;
}

uint8_t mcu_usb_available(void)
{
	return BUFFER_READ_AVAILABLE(usb_rx);
}

void mcu_usb_clear(void)
{
	BUFFER_CLEAR(usb_rx);
}

void mcu_usb_putc(uint8_t c)
{
	if (!tusb_cdc_write_available())
	{
		mcu_usb_flush();
	}
	tusb_cdc_write(c);
}

void mcu_usb_flush(void)
{
	tusb_cdc_flush();
	while (!tusb_cdc_write_available())
	{
		mcu_dotasks(); // tinyusb device task
		if (!tusb_cdc_connected)
		{
			return;
		}
	}
}
#endif

#ifdef MCU_HAS_UART
uint8_t mcu_uart_getc(void)
{
	uint8_t c = 0;
	BUFFER_DEQUEUE(uart_rx, &c);
	return c;
}

uint8_t mcu_uart_available(void)
{
	return BUFFER_READ_AVAILABLE(uart_rx);
}

void mcu_uart_clear(void)
{
	BUFFER_CLEAR(uart_rx);
}

void mcu_uart_putc(uint8_t c)
{
	while (BUFFER_FULL(uart_tx))
	{
		mcu_uart_flush();
	}
	BUFFER_ENQUEUE(uart_tx, &c);
}

void mcu_uart_flush(void)
{

	if (!(COM_UART->CR1 & USART_CR1_TXEIE)) // not ready start flushing
	{
		COM_UART->CR1 |= (USART_CR1_TXEIE);
#if ASSERT_PIN(ACTIVITY_LED)
		io_toggle_output(ACTIVITY_LED);
#endif
	}
}

#endif

#ifdef MCU_HAS_UART2
uint8_t mcu_uart2_getc(void)
{
	uint8_t c = 0;
	BUFFER_DEQUEUE(uart2_rx, &c);
	return c;
}

uint8_t mcu_uart2_available(void)
{
	return BUFFER_READ_AVAILABLE(uart2_rx);
}

void mcu_uart2_clear(void)
{
	BUFFER_CLEAR(uart2_rx);
}

void mcu_uart2_putc(uint8_t c)
{
	while (BUFFER_FULL(uart2_tx))
	{
		mcu_uart2_flush();
	}
	BUFFER_ENQUEUE(uart2_tx, &c);
}

void mcu_uart2_flush(void)
{
	if (!(COM2_UART->CR1 & USART_CR1_TXEIE)) // not ready start flushing
	{
		COM2_UART->CR1 |= (USART_CR1_TXEIE);
#if ASSERT_PIN(ACTIVITY_LED)
		io_toggle_output(ACTIVITY_LED);
#endif
	}
}

#endif

void mcu_init(void)
{
	// make sure both APB1 and APB2 are running at the same clock (48MHz)
	mcu_clocks_init();
	mcu_io_init();
	mcu_usart_init();
	mcu_rtc_init();
#if SERVOS_MASK > 0
	servo_timer_init();
#endif
#ifdef MCU_HAS_SPI
	SPI_ENREG |= SPI_ENVAL;
	mcu_config_af(SPI_SDI, SPI_SDI_AFIO);
	mcu_config_af(SPI_CLK, SPI_CLK_AFIO);
	mcu_config_af(SPI_SDO, SPI_SDO_AFIO);
	// #if ASSERT_PIN_IO(SPI_CS)
	// 	mcu_config_af(SPI_CS, SPI_CS_AFIO);
	// #endif
	// initialize the SPI configuration register
	SPI_REG->CR1 = SPI_CR1_SSM		 // software slave management enabled
								 | SPI_CR1_SSI	 // internal slave select
								 | SPI_CR1_MSTR; // SPI master mode
																 //    | (SPI_SPEED << 3) | SPI_MODE;
	spi_config_t spi_conf = {0};
	spi_conf.mode = SPI_MODE;
	mcu_spi_config(spi_conf, SPI_FREQ);

	RCC->AHB1ENR |= SPI_DMA_EN;

	NVIC_SetPriority(SPI_IRQ, 2);
	NVIC_ClearPendingIRQ(SPI_IRQ);
	NVIC_EnableIRQ(SPI_IRQ);

	SPI_REG->CR1 |= SPI_CR1_SPE;
#endif
#ifdef MCU_HAS_SPI2
	SPI2_ENREG |= SPI2_ENVAL;
	mcu_config_af(SPI2_SDI, SPI2_SDI_AFIO);
	mcu_config_af(SPI2_CLK, SPI2_CLK_AFIO);
	mcu_config_af(SPI2_SDO, SPI2_SDO_AFIO);
	// #if ASSERT_PIN_IO(SPI2_CS)
	// 	mcu_config_af(SPI2_CS, SPI2_CS_AFIO);
	// #endif
	// initialize the SPI2 configuration register
	SPI2_REG->CR1 = SPI_CR1_SSM			// software slave management enabled
									| SPI_CR1_SSI		// internal slave select
									| SPI_CR1_MSTR; // SPI2 master mode
																	//    | (SPI2_SPEED << 3) | SPI2_MODE;
	spi_config_t spi2_conf = {0};
	spi2_conf.mode = SPI2_MODE;
	mcu_spi2_config(spi2_conf, SPI2_FREQ);

	RCC->AHB1ENR |= SPI2_DMA_EN;

	NVIC_SetPriority(SPI2_IRQ, 2);
	NVIC_ClearPendingIRQ(SPI2_IRQ);
	NVIC_EnableIRQ(SPI2_IRQ);

	SPI2_REG->CR1 |= SPI_CR1_SPE;
#endif
#ifdef MCU_HAS_I2C
	RCC->APB1ENR |= I2C_APBEN;
	mcu_config_af(I2C_CLK, I2C_AFIO);
	mcu_config_af(I2C_DATA, I2C_AFIO);
	mcu_config_pullup(I2C_CLK);
	mcu_config_pullup(I2C_DATA);
	// set opendrain
	mcu_config_opendrain(I2C_CLK);
	mcu_config_opendrain(I2C_DATA);
	// reset I2C
	I2C_REG->CR1 |= I2C_CR1_SWRST;
	I2C_REG->CR1 &= ~I2C_CR1_SWRST;
	// set max freq
	I2C_REG->CR2 |= I2C_SPEEDRANGE;
	I2C_REG->TRISE = (I2C_SPEEDRANGE + 1);
	I2C_REG->CCR |= (I2C_FREQ <= 100000UL) ? ((I2C_SPEEDRANGE * 5) & 0x0FFF) : (((I2C_SPEEDRANGE * 5 / 6) & 0x0FFF) | I2C_CCR_FS);
	// initialize the I2C configuration register
	I2C_REG->CR1 |= I2C_CR1_PE;
#endif

	mcu_disable_probe_isr();
	stm32_flash_current_offset = 0;
	stm32_global_isr_enabled = false;
	mcu_eeprom_init();
	mcu_enable_global_isr();
}

/*IO functions*/
// IO functions
void mcu_set_servo(uint8_t servo, uint8_t value)
{
#if SERVOS_MASK > 0
	mcu_servos[servo - SERVO_PINS_OFFSET] = value;
#endif
}

/**
 * gets the pwm for a servo (50Hz with tON between 1~2ms)
 * can be defined either as a function or a macro call
 * */
uint8_t mcu_get_servo(uint8_t servo)
{
#if SERVOS_MASK > 0
	uint8_t offset = servo - SERVO_PINS_OFFSET;

	if ((1U << offset) & SERVOS_MASK)
	{
		return mcu_servos[offset];
	}
#endif
	return 0;
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
	frequency = CLAMP((float)F_STEP_MIN, frequency, (float)F_STEP_MAX);

	// up and down counter (generates half the step rate at each event)
	uint32_t totalticks = (uint32_t)((float)(TIMER_CLOCK >> 1) / frequency);
	*prescaller = 1;
	while (totalticks > 0xFFFF)
	{
		*prescaller <<= 1;
		totalticks >>= 1;
	}

	(*prescaller) -= 1;
	*ticks = (uint16_t)totalticks;
}

float mcu_clocks_to_freq(uint16_t ticks, uint16_t prescaller)
{
	return ((float)TIMER_CLOCK / (float)(((uint32_t)ticks) << (prescaller + 1)));
}

// starts a constant rate pulse at a given frequency.
void mcu_start_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	RCC->TIMER_ENREG |= TIMER_APB;
	TIMER_REG->CR1 = 0;
	TIMER_REG->DIER = 0;
	TIMER_REG->PSC = prescaller;
	TIMER_REG->ARR = ticks;
	TIMER_REG->EGR |= 0x01;
	TIMER_REG->SR &= ~0x01;

	NVIC_SetPriority(MCU_ITP_IRQ, 1);
	NVIC_ClearPendingIRQ(MCU_ITP_IRQ);
	NVIC_EnableIRQ(MCU_ITP_IRQ);

	TIMER_REG->DIER |= 1;
	TIMER_REG->CR1 |= 1; // enable timer upcounter no preload
}

// modifies the pulse frequency
void mcu_change_itp_isr(uint16_t ticks, uint16_t prescaller)
{
	TIMER_REG->ARR = ticks;
	TIMER_REG->PSC = prescaller;
	TIMER_REG->EGR |= 0x01;
}

// stops the pulse
void mcu_stop_itp_isr(void)
{
	TIMER_REG->CR1 &= ~0x1;
	TIMER_REG->DIER &= ~0x1;
	TIMER_REG->SR &= ~0x01;
	NVIC_DisableIRQ(MCU_ITP_IRQ);
}

// Custom delay function
// gets the mcu running time in ms
uint32_t mcu_millis()
{
	return mcu_runtime_ms;
}

uint32_t mcu_micros()
{
	return ((mcu_runtime_ms * 1000) + mcu_free_micros());
}

void mcu_rtc_init()
{
	SysTick->CTRL = 0;
	SysTick->LOAD = ((F_CPU / 1000) - 1);
	SysTick->VAL = 0;
	NVIC_SetPriority(SysTick_IRQn, 10);
	SysTick->CTRL = 7; // Start SysTick (ABH clock)
}

void mcu_dotasks()
{
#ifdef MCU_HAS_USB
	tusb_cdc_task(); // tinyusb device task

	while (tusb_cdc_available())
	{
		uint8_t c = (uint8_t)tusb_cdc_read();
#if !defined(DETACH_USB_FROM_MAIN_PROTOCOL)
		if (mcu_com_rx_cb(c))
		{
			if (BUFFER_FULL(usb_rx))
			{
				c = OVF;
			}

			BUFFER_ENQUEUE(usb_rx, &c);
		}
#else
		mcu_usb_rx_cb(c);
#endif
	}
#endif
}

// gets were the first copy of the eeprom is
static void mcu_eeprom_init(void)
{
	uint32_t eeprom_offset = 0;
	for (eeprom_offset = 0; eeprom_offset < FLASH_SECTOR_SIZE; eeprom_offset += FLASH_EEPROM_SIZE_WORD_ALIGNED)
	{
		if (*((volatile uint32_t *)(FLASH_EEPROM_START + eeprom_offset)) == 0xFFFFFFFF)
		{
			break;
		}
	}

	// if not found at start then it's not initialized
	if (eeprom_offset)
	{
		// one step back
		eeprom_offset -= FLASH_EEPROM_SIZE_WORD_ALIGNED;
		stm32_flash_current_offset = eeprom_offset;
	}
	else
	{
		stm32_flash_current_offset = 0;
		stm32_flash_modified = EEPROM_CLEAN;
		memset(stm32_eeprom_buffer, 0, FLASH_EEPROM_SIZE_WORD_ALIGNED);
		return;
	}

	uint32_t counter = (uint32_t)FLASH_EEPROM_SIZE_WORD;
	uint32_t *ptr = ((uint32_t *)&stm32_eeprom_buffer[0]);
	volatile uint32_t *eeprom = ((volatile uint32_t *)(FLASH_EEPROM_START + eeprom_offset));
	while (counter--)
	{
		READ_FLASH(ptr, eeprom);
		eeprom++;
		ptr++;
	}
}

// Non volatile memory
uint8_t mcu_eeprom_getc(uint16_t address)
{
	if (NVM_STORAGE_SIZE <= address)
	{
		DBGMSG("EEPROM invalid address @ %u",address);
		return 0;
	}
	return stm32_eeprom_buffer[address];
}

static void mcu_eeprom_erase(void)
{
	while (FLASH->SR & FLASH_SR_BSY)
		; // wait while busy
	// unlock flash if locked
	if (FLASH->CR & FLASH_CR_LOCK)
	{
		FLASH->KEYR = 0x45670123;
		FLASH->KEYR = 0xCDEF89AB;
	}
	FLASH->CR = 0;																																							// Ensure PG bit is low
	FLASH->CR |= FLASH_CR_SER | (((FLASH_SECTORS - 1) << FLASH_CR_SNB_Pos) & FLASH_CR_MER_Msk); // set the SER bit
	FLASH->CR |= FLASH_CR_STRT;																																	// set the start bit
	while (FLASH->SR & FLASH_SR_BSY)
		; // wait while busy
	FLASH->CR = 0;
}

void mcu_eeprom_putc(uint16_t address, uint8_t value)
{
	if (NVM_STORAGE_SIZE <= address)
	{
		DBGMSG("EEPROM invalid address @ %u",address);
	}
	// if the value of the eeprom is modified then it will be marked as dirty
	// flash default value is 0xFF. If programming can change value from 1 to 0 but not the other way around
	// if a bit is changed from 0 back to 1 then it will need to rewrite values in a new page
	// flash read and writing is done in negated form
	if (stm32_eeprom_buffer[address] != value)
	{
		stm32_flash_modified |= EEPROM_DIRTY;
		if ((value ^ stm32_eeprom_buffer[address]) & ~value)
		{
			stm32_flash_modified |= EEPROM_NEEDS_NEWPAGE;
		}
	}

	stm32_eeprom_buffer[address] = value;
}

void mcu_eeprom_flush()
{
	if (stm32_flash_modified)
	{
		if (CHECKFLAG(stm32_flash_modified, EEPROM_NEEDS_NEWPAGE))
		{
			stm32_flash_current_offset += FLASH_EEPROM_SIZE_WORD_ALIGNED;
		}

		if (stm32_flash_current_offset >= FLASH_EEPROM_END)
		{
			mcu_eeprom_erase();
			stm32_flash_current_offset = 0;
		}

		volatile uint32_t *eeprom = ((volatile uint32_t *)(FLASH_EEPROM_START + stm32_flash_current_offset));
		uint32_t *ptr = ((uint32_t *)&stm32_eeprom_buffer[0]);
		uint32_t counter = (uint32_t)FLASH_EEPROM_SIZE_WORD;
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
			FLASH->CR |= FLASH_CR_PSIZE_1;
			FLASH->CR |= FLASH_CR_PG; // Ensure PG bit is high
			WRITE_FLASH(eeprom, ptr);
			while (FLASH->SR & FLASH_SR_BSY)
				; // wait while busy
			mcu_enable_global_isr();
			if (FLASH->SR & (FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR))
				proto_error(42); // STATUS_SETTING_WRITE_FAIL
			if (FLASH->SR & FLASH_SR_WRPERR)
				proto_error(43); // STATUS_SETTING_PROTECTED_FAIL
			FLASH->CR = 0;						 // Ensure PG bit is low
			FLASH->SR = 0;
			eeprom++;
			ptr++;
		}
		stm32_flash_modified = EEPROM_CLEAN;

		// Restore interrupt flag state.*/
	}
}

typedef enum spi_port_state_enum
{
	SPI_UNKNOWN = 0,
	SPI_IDLE = 1,
	SPI_TRANSMITTING = 2,
	SPI_TRANSMIT_COMPLETE = 3,
} spi_port_state_t;

#ifdef MCU_HAS_SPI
static volatile spi_port_state_t spi_port_state = SPI_UNKNOWN;
static bool spi_enable_dma = false;

void mcu_spi_config(spi_config_t config, uint32_t frequency)
{
	uint8_t div = (uint8_t)(SPI_CLOCK / frequency);

	uint8_t speed;
	if (div < 2)
	{
		speed = 0;
	}
	else if (div < 4)
	{
		speed = 1;
	}
	else if (div < 8)
	{
		speed = 2;
	}
	else if (div < 16)
	{
		speed = 3;
	}
	else if (div < 32)
	{
		speed = 4;
	}
	else if (div < 64)
	{
		speed = 5;
	}
	else if (div < 128)
	{
		speed = 6;
	}
	else
	{
		speed = 7;
	}

	// disable SPI
	SPI_REG->CR1 &= ~SPI_CR1_SPE;
	// clear speed and mode
	SPI_REG->CR1 &= ~0x3B;
	SPI_REG->CR1 |= (speed << 3) | config.mode;
	// enable SPI
	SPI_REG->CR1 |= SPI_CR1_SPE;

	spi_port_state = SPI_IDLE;
	spi_enable_dma = config.enable_dma;
}

uint8_t mcu_spi_xmit(uint8_t c)
{
	SPI_REG->DR = c;
	while (!(SPI_REG->SR & SPI_SR_TXE) || (SPI_REG->SR & SPI_SR_BSY))
		;
	uint8_t data = SPI_REG->DR;
	spi_port_state = SPI_IDLE;
	return data;
}

static const uint8_t *spi_transfer_tx_ptr = 0;
static uint8_t *spi_transfer_rx_ptr = 0;
static uint16_t spi_transfer_tx_len = 0;
static uint16_t spi_transfer_rx_len = 0;

void SPI_ISR()
{
	if ((SPI_REG->SR & SPI_SR_TXE) && spi_transfer_tx_len)
	{
		SPI_REG->DR = *spi_transfer_tx_ptr++;
		--spi_transfer_tx_len;
	}
	if ((SPI_REG->SR & SPI_SR_RXNE) && spi_transfer_rx_len)
	{
		*spi_transfer_rx_ptr++ = SPI_REG->DR;
		--spi_transfer_rx_len;
	}
	if (spi_transfer_tx_len == 0 && spi_transfer_rx_len == 0)
	{
		SPI_REG->CR2 &= ~(SPI_CR2_TXEIE | SPI_CR2_RXNEIE);
		spi_port_state = SPI_TRANSMIT_COMPLETE;
	}
	NVIC_ClearPendingIRQ(SPI_IRQ);
}

/*
 * Performs a bulk SPI transfer
 * Returns:
 *	true - transfer in progress, keep polling
 *	false - transfer complete
 */
bool mcu_spi_bulk_transfer(const uint8_t *tx_data, uint8_t *rx_data, uint16_t datalen)
{
	if (!spi_enable_dma)
	{
		// Bulk transfer without DMA
		if (spi_port_state == SPI_IDLE)
		{
			spi_port_state = SPI_TRANSMITTING;

			spi_transfer_tx_ptr = tx_data;
			spi_transfer_tx_len = datalen;
			SPI_REG->CR2 |= SPI_CR2_TXEIE;

			if (rx_data)
			{
				spi_transfer_rx_ptr = rx_data;
				spi_transfer_rx_len = datalen;
				SPI_REG->CR2 |= SPI_CR2_RXNEIE;
			}
			else
			{
				spi_transfer_rx_ptr = 0;
				spi_transfer_rx_len = 0;
			}
		}
		else if (spi_port_state == SPI_TRANSMIT_COMPLETE)
		{
			spi_port_state = SPI_IDLE;
			return false;
		}

		return true;
	}

	if (spi_port_state == SPI_TRANSMITTING)
	{
		// Wait for transfers to complete
		if (!(SPI_DMA_TX_ISR >> (SPI_DMA_TX_IFR_POS + 5)) ||
				(!(SPI_DMA_RX_ISR >> (SPI_DMA_RX_IFR_POS + 5)) && rx_data))
			return true;

		// SPI hardware still transmitting the last byte
		if (SPI_REG->SR & SPI_SR_BSY)
			return true;

		// Disable DMA use
		SPI_REG->CR2 &= ~SPI_CR2_TXDMAEN;
		if (rx_data)
			SPI_REG->CR2 &= ~SPI_CR2_RXDMAEN;

		// Transfer finished
		spi_port_state = SPI_IDLE;
		return false;
	}

	if (spi_port_state != SPI_IDLE)
	{
		// Wait for idle
		if (SPI_REG->SR & SPI_SR_BSY)
			return true;
		spi_port_state = SPI_IDLE;
	}

	// Wait until streams are available
	if ((SPI_DMA_TX_STREAM->CR & DMA_SxCR_EN) || ((SPI_DMA_RX_STREAM->CR & DMA_SxCR_EN) && rx_data))
		return true;

	/***     Setup Transmit DMA     ***/

	// Clear flags
	SPI_DMA_TX_IFCR |= SPI_DMA_TX_IFCR_MASK;

	SPI_DMA_TX_STREAM->CR =
			(SPI_DMA_TX_CHANNEL << DMA_SxCR_CHSEL_Pos) | // Select correct channel
			(0b01 << DMA_SxCR_PL_Pos) |									 // Set priority to medium
			DMA_SxCR_MINC |															 // Increment memory
			(0b01 << DMA_SxCR_DIR_Pos);									 // Memory to peripheral transfer direction

	SPI_DMA_TX_STREAM->PAR = (uint32_t)&SPI_REG->DR;
	SPI_DMA_TX_STREAM->M0AR = (uint32_t)tx_data;

	SPI_DMA_TX_STREAM->NDTR = datalen;

	SPI_DMA_TX_STREAM->FCR &= ~DMA_SxFCR_DMDIS; // Enable direct mode

	// Enable DMA use for transmission
	SPI_REG->CR2 |= SPI_CR2_TXDMAEN;

	if (rx_data)
	{
		/***     Setup Receive DMA     ***/

		// Clear flags
		SPI_DMA_RX_IFCR |= SPI_DMA_RX_IFCR_MASK;

		SPI_DMA_RX_STREAM->CR =
				(SPI_DMA_RX_CHANNEL << DMA_SxCR_CHSEL_Pos) | // Select correct channel
				(0b01 << DMA_SxCR_PL_Pos) |									 // Set priority to medium
				DMA_SxCR_MINC |															 // Increment memory
				(0b00 << DMA_SxCR_DIR_Pos);									 // Peripheral to memory transfer direction

		SPI_DMA_RX_STREAM->PAR = (uint32_t)&SPI_REG->DR;
		SPI_DMA_RX_STREAM->M0AR = (uint32_t)rx_data;

		SPI_DMA_RX_STREAM->NDTR = datalen;

		SPI_DMA_RX_STREAM->FCR &= ~DMA_SxFCR_DMDIS; // Enable direct mode

		// Enable DMA use for reception
		SPI_REG->CR2 |= SPI_CR2_RXDMAEN;
	}

	/***     DMA Setup Complete     ***/

	// Start streams
	SPI_DMA_TX_STREAM->CR |= DMA_SxCR_EN;
	if (rx_data)
		SPI_DMA_RX_STREAM->CR |= DMA_SxCR_EN;

	// Transmission started
	spi_port_state = SPI_TRANSMITTING;
	return true;
}

#endif

#ifdef MCU_HAS_SPI2
static volatile spi_port_state_t spi2_port_state = SPI_UNKNOWN;
static bool spi2_enable_dma = false;

void mcu_spi2_config(spi_config_t config, uint32_t frequency)
{
	uint8_t div = (uint8_t)(SPI2_CLOCK / frequency);

	uint8_t speed;
	if (div < 2)
	{
		speed = 0;
	}
	else if (div < 4)
	{
		speed = 1;
	}
	else if (div < 8)
	{
		speed = 2;
	}
	else if (div < 16)
	{
		speed = 3;
	}
	else if (div < 32)
	{
		speed = 4;
	}
	else if (div < 64)
	{
		speed = 5;
	}
	else if (div < 128)
	{
		speed = 6;
	}
	else
	{
		speed = 7;
	}

	// disable SPI2
	SPI2_REG->CR1 &= ~SPI_CR1_SPE;
	// clear speed and mode
	SPI2_REG->CR1 &= ~0x3B;
	SPI2_REG->CR1 |= (speed << 3) | config.mode;
	// enable SPI2
	SPI2_REG->CR1 |= SPI_CR1_SPE;

	spi2_port_state = SPI_IDLE;
	spi2_enable_dma = config.enable_dma;
}

uint8_t mcu_spi2_xmit(uint8_t c)
{
	SPI2_REG->DR = c;
	while (!(SPI2_REG->SR & SPI_SR_TXE) || (SPI2_REG->SR & SPI_SR_BSY))
		;
	uint8_t data = SPI2_REG->DR;
	spi2_port_state = SPI_IDLE;
	return data;
}

static const uint8_t *spi2_transfer_tx_ptr = 0;
static uint8_t *spi2_transfer_rx_ptr = 0;
static uint16_t spi2_transfer_tx_len = 0;
static uint16_t spi2_transfer_rx_len = 0;

void SPI2_ISR()
{
	if ((SPI2_REG->SR & SPI_SR_TXE) && spi2_transfer_tx_len)
	{
		SPI2_REG->DR = *spi2_transfer_tx_ptr++;
		--spi2_transfer_tx_len;
	}
	if ((SPI2_REG->SR & SPI_SR_RXNE) && spi2_transfer_rx_len)
	{
		*spi2_transfer_rx_ptr++ = SPI2_REG->DR;
		--spi2_transfer_rx_len;
	}
	if (spi2_transfer_tx_len == 0 && spi2_transfer_rx_len == 0)
	{
		SPI2_REG->CR2 &= ~(SPI_CR2_TXEIE | SPI_CR2_RXNEIE);
		spi2_port_state = SPI_TRANSMIT_COMPLETE;
	}
	NVIC_ClearPendingIRQ(SPI2_IRQ);
}

/*
 * Performs a bulk SPI2 transfer
 * Returns:
 *	true - transfer in progress, keep polling
 *	false - transfer complete
 */
bool mcu_spi2_bulk_transfer(const uint8_t *tx_data, uint8_t *rx_data, uint16_t datalen)
{
	if (!spi2_enable_dma)
	{
		// Bulk transfer without DMA
		if (spi2_port_state == SPI_IDLE)
		{
			spi2_port_state = SPI_TRANSMITTING;

			spi2_transfer_tx_ptr = tx_data;
			spi2_transfer_tx_len = datalen;
			SPI2_REG->CR2 |= SPI_CR2_TXEIE;

			if (rx_data)
			{
				spi2_transfer_rx_ptr = rx_data;
				spi2_transfer_rx_len = datalen;
				SPI2_REG->CR2 |= SPI_CR2_RXNEIE;
			}
			else
			{
				spi2_transfer_rx_ptr = 0;
				spi2_transfer_rx_len = 0;
			}
		}
		else if (spi2_port_state == SPI_TRANSMIT_COMPLETE)
		{
			spi2_port_state = SPI_IDLE;
			return false;
		}

		return true;
	}

	if (spi2_port_state == SPI_TRANSMITTING)
	{
		// Wait for transfers to complete
		if (!(SPI2_DMA_TX_ISR >> (SPI2_DMA_TX_IFR_POS + 5)) ||
				(!(SPI2_DMA_RX_ISR >> (SPI2_DMA_RX_IFR_POS + 5)) && rx_data))
			return true;

		// SPI2 hardware still transmitting the last byte
		if (SPI2_REG->SR & SPI_SR_BSY)
			return true;

		// Disable DMA use
		SPI2_REG->CR2 &= ~SPI_CR2_TXDMAEN;
		if (rx_data)
			SPI2_REG->CR2 &= ~SPI_CR2_RXDMAEN;

		// Transfer finished
		spi2_port_state = SPI_IDLE;
		return false;
	}

	if (spi2_port_state != SPI_IDLE)
	{
		// Wait for idle
		if (SPI2_REG->SR & SPI_SR_BSY)
			return true;
		spi2_port_state = SPI_IDLE;
	}

	// Wait until streams are available
	if ((SPI2_DMA_TX_STREAM->CR & DMA_SxCR_EN) || ((SPI2_DMA_RX_STREAM->CR & DMA_SxCR_EN) && rx_data))
		return true;

	/***     Setup Transmit DMA     ***/

	// Clear flags
	SPI2_DMA_TX_IFCR |= SPI2_DMA_TX_IFCR_MASK;

	SPI2_DMA_TX_STREAM->CR =
			(SPI2_DMA_TX_CHANNEL << DMA_SxCR_CHSEL_Pos) | // Select correct channel
			(0b01 << DMA_SxCR_PL_Pos) |										// Set priority to medium
			DMA_SxCR_MINC |																// Increment memory
			(0b01 << DMA_SxCR_DIR_Pos);										// Memory to peripheral transfer direction

	SPI2_DMA_TX_STREAM->PAR = (uint32_t)&SPI2_REG->DR;
	SPI2_DMA_TX_STREAM->M0AR = (uint32_t)tx_data;

	SPI2_DMA_TX_STREAM->NDTR = datalen;

	SPI2_DMA_TX_STREAM->FCR &= ~DMA_SxFCR_DMDIS; // Enable direct mode

	// Enable DMA use for transmission
	SPI2_REG->CR2 |= SPI_CR2_TXDMAEN;

	if (rx_data)
	{
		/***     Setup Receive DMA     ***/

		// Clear flags
		SPI2_DMA_RX_IFCR |= SPI2_DMA_RX_IFCR_MASK;

		SPI2_DMA_RX_STREAM->CR =
				(SPI2_DMA_RX_CHANNEL << DMA_SxCR_CHSEL_Pos) | // Select correct channel
				(0b01 << DMA_SxCR_PL_Pos) |										// Set priority to medium
				DMA_SxCR_MINC |																// Increment memory
				(0b00 << DMA_SxCR_DIR_Pos);										// Peripheral to memory transfer direction

		SPI2_DMA_RX_STREAM->PAR = (uint32_t)&SPI2_REG->DR;
		SPI2_DMA_RX_STREAM->M0AR = (uint32_t)rx_data;

		SPI2_DMA_RX_STREAM->NDTR = datalen;

		SPI2_DMA_RX_STREAM->FCR &= ~DMA_SxFCR_DMDIS; // Enable direct mode

		// Enable DMA use for reception
		SPI2_REG->CR2 |= SPI_CR2_RXDMAEN;
	}

	/***     DMA Setup Complete     ***/

	// Start streams
	SPI2_DMA_TX_STREAM->CR |= DMA_SxCR_EN;
	if (rx_data)
		SPI2_DMA_RX_STREAM->CR |= DMA_SxCR_EN;

	// Transmission started
	spi2_port_state = SPI_TRANSMITTING;
	return true;
}

#endif

#ifdef MCU_HAS_I2C
#if I2C_ADDRESS == 0
void mcu_i2c_write_stop(bool *stop)
{
	if (*stop)
	{
		int32_t ms_timeout = 25;

		I2C_REG->CR1 |= I2C_CR1_STOP;
		__TIMEOUT_MS__(ms_timeout)
		{
			if (I2C_REG->CR1 & I2C_CR1_STOP)
			{
				return;
			}
		}
	}
}

static uint8_t mcu_i2c_write(uint8_t data, bool send_start, bool send_stop, uint32_t ms_timeout)
{
	bool stop __attribute__((__cleanup__(mcu_i2c_write_stop))) = send_stop;
	int32_t timeout = ms_timeout;

	uint32_t status = send_start ? I2C_SR1_ADDR : I2C_SR1_BTF;
	I2C_REG->SR1 &= ~I2C_SR1_AF;
	if (send_start)
	{
		if ((I2C_REG->SR1 & I2C_SR1_ARLO) || ((I2C_REG->CR1 & I2C_CR1_START) && (I2C_REG->CR1 & I2C_CR1_STOP)))
		{
			// Save values
			uint32_t cr2 = I2C_REG->CR2;
			uint32_t ccr = I2C_REG->CCR;
			uint32_t trise = I2C_REG->TRISE;

			// Software reset
			I2C_REG->CR1 |= I2C_CR1_SWRST;
			I2C_REG->CR1 &= ~I2C_CR1_SWRST;

			// Restore values
			I2C_REG->CR2 = cr2;
			I2C_REG->CCR = ccr;
			I2C_REG->TRISE = trise;

			// Enable
			I2C_REG->CR1 |= I2C_CR1_PE;
		}

		// init
		I2C_REG->CR1 |= I2C_CR1_START;

		__TIMEOUT_MS__(timeout)
		{
			if ((I2C_REG->SR1 & I2C_SR1_SB) && (I2C_REG->SR2 & I2C_SR2_MSL) && (I2C_REG->SR2 & I2C_SR2_BUSY))
			{
				break;
			}
			if (I2C_REG->SR1 & I2C_SR1_ARLO)
			{
				stop = false;
				return I2C_NOTOK;
			}
		}

		__TIMEOUT_ASSERT__(timeout)
		{
			stop = true;
			return I2C_NOTOK;
		}

		if (I2C_REG->SR1 & I2C_SR1_AF)
		{
			stop = true;
			return I2C_NOTOK;
		}
	}

	I2C_REG->DR = data;
	timeout = ms_timeout;
	__TIMEOUT_MS__(timeout)
	{
		if (I2C_REG->SR1 & status)
		{
			break;
		}
		if (I2C_REG->SR1 & I2C_SR1_AF)
		{
			break;
		}
		if (I2C_REG->SR1 & I2C_SR1_ARLO)
		{
			stop = false;
			return I2C_NOTOK;
		}
	}

	__TIMEOUT_ASSERT__(timeout)
	{
		stop = true;
		return I2C_NOTOK;
	}
	// read SR2 to clear ADDR
	if (send_start)
	{
		status = I2C_REG->SR2;
	}

	if (I2C_REG->SR1 & I2C_SR1_AF)
	{
		stop = true;
		return I2C_NOTOK;
	}

	return I2C_OK;
}

static uint8_t mcu_i2c_read(uint8_t *data, bool with_ack, bool send_stop, uint32_t ms_timeout)
{
	*data = 0xFF;
	bool stop __attribute__((__cleanup__(mcu_i2c_write_stop))) = send_stop;

	if (!with_ack)
	{
		I2C_REG->CR1 &= ~I2C_CR1_ACK;
		stop = false;
		mcu_i2c_write_stop(&send_stop);
	}
	else
	{
		I2C_REG->CR1 |= I2C_CR1_ACK;
	}

	__TIMEOUT_MS__(ms_timeout)
	{
		if (I2C_REG->SR1 & I2C_SR1_RXNE)
		{
			*data = I2C_REG->DR;
			return I2C_OK;
		}
	}

	stop = true;
	return I2C_NOTOK;
}

#ifndef mcu_i2c_send
// master sends command to slave
uint8_t mcu_i2c_send(uint8_t address, uint8_t *data, uint8_t datalen, bool release, uint32_t ms_timeout)
{
	if (data && datalen)
	{
		if (mcu_i2c_write(address << 1, true, false, ms_timeout) == I2C_OK) // start, send address, write
		{
			// send data, stop
			do
			{
				datalen--;
				bool last = (datalen == 0);
				if (mcu_i2c_write(*data, false, (release & last), ms_timeout) != I2C_OK)
				{
					return I2C_NOTOK;
				}
				data++;

			} while (datalen);

			return I2C_OK;
		}
	}

	return I2C_NOTOK;
}
#endif

#ifndef mcu_i2c_receive
// master receive response from slave
uint8_t mcu_i2c_receive(uint8_t address, uint8_t *data, uint8_t datalen, uint32_t ms_timeout)
{
	if (data && datalen)
	{
		if (mcu_i2c_write((address << 1) | 0x01, true, false, ms_timeout) == I2C_OK) // start, send address, write
		{
			do
			{
				datalen--;
				bool last = (datalen == 0);
				if (mcu_i2c_read(data, !last, last, ms_timeout) != I2C_OK)
				{
					return I2C_NOTOK;
				}
				data++;
			} while (datalen);
			return I2C_OK;
		}
	}

	return I2C_NOTOK;
}
#endif
#endif

#ifndef mcu_i2c_config
void mcu_i2c_config(uint32_t frequency)
{
	RCC->APB1ENR |= I2C_APBEN;
	mcu_config_opendrain(I2C_CLK);
	mcu_config_opendrain(I2C_DATA);
	mcu_config_af(I2C_CLK, I2C_AFIO);
	mcu_config_af(I2C_DATA, I2C_AFIO);
#ifdef SPI_REMAP
	AFIO->MAPR |= I2C_REMAP;
#endif
	// reset I2C
	I2C_REG->CR1 |= I2C_CR1_SWRST;
	I2C_REG->CR1 &= ~I2C_CR1_SWRST;
#if I2C_ADDRESS == 0
	// set max freq
	I2C_REG->CR2 |= I2C_SPEEDRANGE;
	I2C_REG->TRISE = (I2C_SPEEDRANGE + 1);
	I2C_REG->CCR |= (frequency <= 100000UL) ? ((I2C_SPEEDRANGE * 5) & 0x0FFF) : (((I2C_SPEEDRANGE * 5 / 6) & 0x0FFF) | I2C_CCR_FS);
#else
	// set address
	I2C_REG->OAR1 &= ~(I2C_OAR1_ADDMODE | 0x0F);
	I2C_REG->OAR1 |= (I2C_ADDRESS << 1);
	I2C_REG->OAR2 = 0;
	I2C_REG->CR1 &= ~I2C_CR1_NOSTRETCH;
	// enable events
	I2C_REG->CR2 |= (I2C_CR2_ITEVTEN | I2C_CR2_ITERREN | I2C_CR2_ITBUFEN);
	NVIC_SetPriority(I2C_IRQ, 10);
	NVIC_ClearPendingIRQ(I2C_IRQ);
	NVIC_EnableIRQ(I2C_IRQ);
#endif
	// initialize the SPI configuration register
	I2C_REG->CR1 |= (I2C_CR1_PE | I2C_CR1_ENGC);
#if I2C_ADDRESS != 0
	// prepare ACK in slave mode
	I2C_REG->CR1 |= I2C_CR1_ACK;
#endif
}
#endif

#if I2C_ADDRESS != 0
uint8_t mcu_i2c_buffer[I2C_SLAVE_BUFFER_SIZE];

void I2C_ISR(void)
{
	static uint8_t index = 0;
	static uint8_t datalen = 0;

	uint8_t i = index;

	// address match or generic call
	// Clear ISR flag by reading SR1 and SR2 registers
	if ((I2C_REG->SR1 & I2C_SR1_ADDR))
	{
		// clear the ISR flag
		volatile uint32_t status = I2C_REG->SR1;
		(void)status;
		status = I2C_REG->SR2;
		(void)status;

		// // Address matched, do necessary processing
		if ((status & I2C_SR2_TRA) && !datalen)
		{
			mcu_i2c_buffer[i] = 0;
			// unlock ISR and process the info request
			// mcu_enable_global_isr();
			mcu_i2c_slave_cb(mcu_i2c_buffer, &i);
			datalen = i;
		}
		i = 0;
	}

	if ((I2C_REG->SR1 & I2C_SR1_RXNE))
	{
		mcu_i2c_buffer[i++] = I2C_REG->DR;
	}

	if ((I2C_REG->SR1 & I2C_SR1_TXE))
	{
		I2C_REG->DR = mcu_i2c_buffer[i++];
		if (i > datalen)
		{
			// send NACK
			I2C_REG->CR1 |= I2C_CR1_STOP;
			datalen = 0;
		}
	}

	// Clear ISR flag by reading SR1 and writing CR1 registers
	if ((I2C_REG->SR1 & I2C_SR1_STOPF))
	{
		// clear the ISR flag
		volatile uint32_t status = I2C_REG->SR1;
		(void)status;
		I2C_REG->CR1 |= I2C_CR1_PE;
		// stop transmission
		datalen = 0;
	}

	// An error ocurred
	if (I2C_REG->SR1 & 0XFF00)
	{
		// prepare ACK for next transmission
		index = 0;
		datalen = 0;
		I2C_REG->CR1 |= I2C_CR1_ACK;
		I2C_REG->CR2 |= (I2C_CR2_ITEVTEN | I2C_CR2_ITERREN);
		// clear ISR flag
		I2C_REG->SR1 = I2C_REG->SR1 & 0X00FF;
		volatile uint32_t status = I2C_REG->SR1;
		(void)status;
		status = I2C_REG->SR2;
		(void)status;
	}

	// prepare ACK for next transmition
	index = i;
	I2C_REG->CR1 |= I2C_CR1_ACK;
	NVIC_ClearPendingIRQ(I2C_IRQ);
}
#endif

#endif

#ifdef MCU_HAS_ONESHOT_TIMER

void MCU_ONESHOT_ISR(void)
{
	if ((ONESHOT_TIMER_REG->SR & 1))
	{
		ONESHOT_TIMER_REG->DIER = 0;
		ONESHOT_TIMER_REG->SR = 0;
		ONESHOT_TIMER_REG->CR1 = 0;

		if (mcu_timeout_cb)
		{
			mcu_timeout_cb();
		}
	}

	NVIC_ClearPendingIRQ(MCU_ONESHOT_IRQ);
}

/**
 * configures a single shot timeout in us
 * */
#ifndef mcu_config_timeout
void mcu_config_timeout(mcu_timeout_delgate fp, uint32_t timeout)
{
	// up and down counter (generates half the step rate at each event)
	uint32_t clocks = (uint32_t)((ONESHOT_TIMER_CLOCK / 1000000UL) * timeout);
	uint32_t presc = 1;

	mcu_timeout_cb = fp;

	while (clocks > 0xFFFF)
	{
		presc <<= 1;
		clocks >>= 1;
	}

	presc--;
	clocks--;

	RCC->ONESHOT_TIMER_ENREG |= ONESHOT_TIMER_APB;
	ONESHOT_TIMER_REG->CR1 = 0;
	ONESHOT_TIMER_REG->DIER = 0;
	ONESHOT_TIMER_REG->PSC = presc;
	ONESHOT_TIMER_REG->ARR = clocks;
	ONESHOT_TIMER_REG->EGR |= 0x01;
	ONESHOT_TIMER_REG->SR = 0;
	ONESHOT_TIMER_REG->CNT = 0;

	NVIC_SetPriority(MCU_ONESHOT_IRQ, 1);
	NVIC_ClearPendingIRQ(MCU_ONESHOT_IRQ);
	NVIC_EnableIRQ(MCU_ONESHOT_IRQ);

	// ONESHOT_TIMER_REG->DIER |= 1;
	// ONESHOT_TIMER_REG->CR1 |= 1; // enable timer upcounter no preload
}
#endif

/**
 * starts the timeout. Once hit the the respective callback is called
 * */
#ifndef mcu_start_timeout
void mcu_start_timeout()
{
}
#endif
#endif

#endif
