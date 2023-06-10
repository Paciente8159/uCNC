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
#include <math.h>

#ifdef MCU_HAS_USB
#include <tusb_ucnc.h>
#endif

#ifndef FLASH_SIZE
#define FLASH_SIZE (FLASH_BANK1_END - FLASH_BASE + 1)
#endif

// this is needed if a custom flash size is defined
#define FLASH_LIMIT (FLASH_BASE + FLASH_SIZE - 1)

#if (FLASH_LIMIT > FLASH_BANK1_END)
#error "The set FLASH_SIZE is beyond the chip capability"
#endif

// set the FLASH EEPROM SIZE
#define FLASH_EEPROM_SIZE 0x400

#if (FLASH_BANK1_END <= 0x0801FFFFUL)
#define FLASH_EEPROM_PAGES (((FLASH_EEPROM_SIZE - 1) >> 10) + 1)
#define FLASH_EEPROM (FLASH_LIMIT - ((FLASH_EEPROM_PAGES << 10) - 1))
#define FLASH_PAGE_MASK (0xFFFF - (1 << 10) + 1)
#define FLASH_PAGE_OFFSET_MASK (0xFFFF & ~FLASH_PAGE_MASK)
#else
#define FLASH_EEPROM_PAGES (((FLASH_EEPROM_SIZE - 1) >> 11) + 1)
#define FLASH_EEPROM (FLASH_LIMIT - ((FLASH_EEPROM_PAGES << 11) - 1))
#define FLASH_PAGE_MASK (0xFFFF - (1 << 11) + 1)
#define FLASH_PAGE_OFFSET_MASK (0xFFFF & ~FLASH_PAGE_MASK)
#endif

#define READ_FLASH(ram_ptr, flash_ptr) (*ram_ptr = ~(*flash_ptr))
#define WRITE_FLASH(flash_ptr, ram_ptr) (*flash_ptr = ~(*ram_ptr))
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

/**
 * The isr functions
 * The respective IRQHandler will execute these functions
 **/
#ifdef MCU_HAS_UART
void MCU_SERIAL_ISR(void)
{
	mcu_disable_global_isr();
	if (COM_UART->SR & USART_SR_RXNE)
	{
		unsigned char c = COM_INREG;
#if !defined(DETACH_UART_FROM_MAIN_PROTOCOL)
		mcu_com_rx_cb(c);
#else
		mcu_uart_rx_cb(c);
#endif
	}

#if !defined(ENABLE_SYNC_TX) && !defined(DETACH_UART_FROM_MAIN_PROTOCOL)
	if ((COM_UART->SR & USART_SR_TXE) && (COM_UART->CR1 & USART_CR1_TXEIE))
	{
		COM_UART->CR1 &= ~(USART_CR1_TXEIE);
		mcu_uart_flush();
	}
#endif
	mcu_enable_global_isr();
}
#endif

#ifdef MCU_HAS_UART2
void MCU_SERIAL2_ISR(void)
{
	mcu_disable_global_isr();
	if (COM2_UART->SR & USART_SR_RXNE)
	{
		unsigned char c = COM2_INREG;
#if !defined(DETACH_UART2_FROM_MAIN_PROTOCOL)
		mcu_com_rx_cb(c);
#else
		mcu_uart2_rx_cb(c);
#endif
	}

#if !defined(ENABLE_SYNC_TX) && !defined(DETACH_UART2_FROM_MAIN_PROTOCOL)
	if ((COM2_UART->SR & USART_SR_TXE) && (COM2_UART->CR1 & USART_CR1_TXEIE))
	{
		COM2_UART->CR1 &= ~(USART_CR1_TXEIE);
		mcu_uart2_flush();
	}
#endif
	mcu_enable_global_isr();
}
#endif

#ifdef MCU_HAS_USB
void USB_HP_CAN1_TX_IRQHandler(void)
{
	mcu_disable_global_isr();
	tusb_cdc_isr_handler();
	mcu_enable_global_isr();
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
	mcu_disable_global_isr();
	tusb_cdc_isr_handler();
	mcu_enable_global_isr();
}

void USBWakeUp_IRQHandler(void)
{
	mcu_disable_global_isr();
	tusb_cdc_isr_handler();
	mcu_enable_global_isr();
}
#endif

// define the mcu internal servo variables
#if SERVOS_MASK > 0

static uint8_t mcu_servos[6];

static FORCEINLINE void mcu_clear_servos()
{
#if ASSERT_PIN(SERVO0)
	mcu_clear_output(SERVO0);
#endif
#if ASSERT_PIN(SERVO1)
	mcu_clear_output(SERVO1);
#endif
#if ASSERT_PIN(SERVO2)
	mcu_clear_output(SERVO2);
#endif
#if ASSERT_PIN(SERVO3)
	mcu_clear_output(SERVO3);
#endif
#if ASSERT_PIN(SERVO4)
	mcu_clear_output(SERVO4);
#endif
#if ASSERT_PIN(SERVO5)
	mcu_clear_output(SERVO5);
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
#if (SERVO_TIMER != 6 && SERVO_TIMER != 7)
	SERVO_TIMER_REG->CCER = 0;
	SERVO_TIMER_REG->CCMR1 = 0;
#if (SERVO_TIMER < 10)
	SERVO_TIMER_REG->CCMR2 = 0;
#endif
#endif
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
#if ASSERT_PIN(SERVO0)
	case SERVO0_FRAME:
		servo_start_timeout(mcu_servos[0]);
		mcu_set_output(SERVO0);
		break;
#endif
#if ASSERT_PIN(SERVO1)
	case SERVO1_FRAME:
		mcu_set_output(SERVO1);
		servo_start_timeout(mcu_servos[1]);
		break;
#endif
#if ASSERT_PIN(SERVO2)
	case SERVO2_FRAME:
		mcu_set_output(SERVO2);
		servo_start_timeout(mcu_servos[2]);
		break;
#endif
#if ASSERT_PIN(SERVO3)
	case SERVO3_FRAME:
		mcu_set_output(SERVO3);
		servo_start_timeout(mcu_servos[3]);
		break;
#endif
#if ASSERT_PIN(SERVO4)
	case SERVO4_FRAME:
		mcu_set_output(SERVO4);
		servo_start_timeout(mcu_servos[4]);
		break;
#endif
#if ASSERT_PIN(SERVO5)
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

void mcu_clocks_init()
{
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
	tusb_cdc_init();
#endif

#ifdef MCU_HAS_UART
	/*enables RCC clocks and GPIO*/
	mcu_config_output_af(TX, GPIO_OUTALT_OD_50MHZ);
	mcu_config_input_af(RX);
#ifdef COM_REMAP
	AFIO->MAPR |= COM_REMAP;
#endif
	RCC->COM_APB |= (COM_APBEN);
	/*setup UART*/
	COM_UART->CR1 = 0; // 8 bits No parity M=0 PCE=0
	COM_UART->CR2 = 0; // 1 stop bit STOP=00
	COM_UART->CR3 = 0;
	COM_UART->SR = 0;
	// //115200 baudrate
	float baudrate = ((float)(UART_CLOCK >> 4) / ((float)BAUDRATE));
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
}

#ifdef MCU_HAS_USB
void mcu_usb_putc(uint8_t c)
{
	tusb_cdc_write(c);
}

void mcu_usb_flush(void)
{
	tusb_cdc_flush();
}
#endif

#ifdef MCU_HAS_UART

void mcu_uart_putc(uint8_t c)
{
	while (!(COM_UART->SR & USART_SR_TXE))
		;

	COM_OUTREG = c;
#if !defined(ENABLE_SYNC_TX) && !defined(DETACH_UART_FROM_MAIN_PROTOCOL)
	COM_UART->CR1 |= (USART_CR1_TXEIE);
#endif
}

void mcu_uart_flush(void)
{
#if !defined(ENABLE_SYNC_TX) && !defined(DETACH_UART_FROM_MAIN_PROTOCOL)
	if ((COM_UART->SR & USART_SR_TXE)) // not ready start flushing
	{
		uint8_t read = mcu_uart_tx_tail;
		if (read == mcu_com_tx_head)
		{
			return;
		}

		unsigned char c = mcu_com_tx_buffer[read];
		if (++read == TX_BUFFER_SIZE)
		{
			read = 0;
		}
		mcu_uart_tx_tail = read;
		mcu_uart_putc(c);
#if ASSERT_PIN(ACTIVITY_LED)
		mcu_toggle_output(ACTIVITY_LED);
#endif
	}
#endif
}

#endif

#ifdef MCU_HAS_UART2

void mcu_uart2_putc(uint8_t c)
{
	while (!(COM2_UART->SR & USART_SR_TXE))
		;

	COM2_OUTREG = c;
#if !defined(ENABLE_SYNC_TX) && !defined(DETACH_UART2_FROM_MAIN_PROTOCOL)
	COM2_UART->CR1 |= (USART_CR1_TXEIE);
#endif
}

void mcu_uart2_flush(void)
{
#if !defined(ENABLE_SYNC_TX) && !defined(DETACH_UART2_FROM_MAIN_PROTOCOL)
	if ((COM2_UART->SR & USART_SR_TXE)) // not ready start flushing
	{
		uint8_t read = mcu_uart2_tx_tail;
		if (read == mcu_com_tx_head)
		{
			return;
		}

		unsigned char c = mcu_com_tx_buffer[read];
		if (++read == TX_BUFFER_SIZE)
		{
			read = 0;
		}
		mcu_uart2_tx_tail = read;
		mcu_uart2_putc(c);
#if ASSERT_PIN(ACTIVITY_LED)
		mcu_toggle_output(ACTIVITY_LED);
#endif
	}
#endif
}

#endif

void mcu_init(void)
{
	mcu_clocks_init();
	stm32_flash_current_page = -1;
	stm32_global_isr_enabled = false;
	mcu_io_init();
	mcu_usart_init();
	mcu_rtc_init();

#if SERVOS_MASK > 0
	servo_timer_init();
#endif

#ifdef MCU_HAS_SPI
	SPI_ENREG |= SPI_ENVAL;
	mcu_config_input_af(SPI_SDI);
	mcu_config_output_af(SPI_CLK, GPIO_OUTALT_PP_50MHZ);
	mcu_config_output_af(SPI_SDO, GPIO_OUTALT_PP_50MHZ);
	mcu_config_output_af(SPI_CS, GPIO_OUTALT_PP_50MHZ);
#ifdef SPI_REMAP
	AFIO->MAPR |= SPI_REMAP;
#endif
	// initialize the SPI configuration register
	SPI_REG->CR1 = SPI_CR1_SSM	   // software slave management enabled
				   | SPI_CR1_SSI   // internal slave select
				   | SPI_CR1_MSTR; // SPI master mode
								   //    | (SPI_SPEED << 3) | SPI_MODE;
	mcu_spi_config(SPI_MODE, SPI_FREQ);
	SPI_REG->CR1 |= SPI_CR1_SPE;
#endif

#ifdef MCU_HAS_I2C
	mcu_i2c_config(I2C_FREQ);
#endif

	mcu_disable_probe_isr();
	mcu_enable_global_isr();
}

/**
 * enables the pin probe mcu isr on change
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_enable_probe_isr
void mcu_enable_probe_isr(void)
{
}
#endif

/**
 * disables the pin probe mcu isr on change
 * can be defined either as a function or a macro call
 * */
#ifndef mcu_disable_probe_isr
void mcu_disable_probe_isr(void)
{
}
#endif

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
	// up and down counter (generates half the step rate at each event)
	uint32_t totalticks = (uint32_t)((float)(ITP_TIMER_CLOCK >> 1) / frequency);

	*prescaller = 1;
	while (totalticks > 0xFFFF)
	{
		*prescaller <<= 1;
		totalticks >>= 1;
	}

	*prescaller--;
	*ticks = (uint16_t)totalticks;
}

float mcu_clocks_to_freq(uint16_t ticks, uint16_t prescaller)
{
	return ((float)ITP_TIMER_CLOCK / (float)(((uint32_t)ticks) << (prescaller + 1)));
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
#if (ITP_TIMER != 6 && ITP_TIMER != 7)
	ITP_TIMER_REG->CCER = 0;
	ITP_TIMER_REG->CCMR1 = 0;
#if (ITP_TIMER < 10)
	ITP_TIMER_REG->CCMR2 = 0;
#endif
#endif
	ITP_TIMER_REG->SR &= ~0x01;

	NVIC_SetPriority(MCU_ITP_IRQ, 1);
	NVIC_ClearPendingIRQ(MCU_ITP_IRQ);
	NVIC_EnableIRQ(MCU_ITP_IRQ);

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
	return ((mcu_runtime_ms * 1000) + ((SysTick->LOAD - SysTick->VAL) / (F_CPU / 1000000)));
}

void mcu_rtc_init()
{
	SysTick->CTRL = 0;
	SysTick->LOAD = ((F_CPU / 1000) - 1);
	SysTick->VAL = 0;
	NVIC_SetPriority(SysTick_IRQn, 10);
	SysTick->CTRL = 7; // Start SysTick (ABH)
}

void mcu_dotasks()
{
#ifdef MCU_HAS_USB
	tusb_cdc_flush();
	tusb_cdc_task(); // tinyusb device task

	while (tusb_cdc_available())
	{
		unsigned char c = (unsigned char)tusb_cdc_read();
#if !defined(DETACH_USB_FROM_MAIN_PROTOCOL)
		mcu_com_rx_cb(c);
#else
		mcu_usb_rx_cb(c);
#endif
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
			READ_FLASH(ptr, eeprom);
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
			WRITE_FLASH(eeprom, ptr);
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

#ifdef MCU_HAS_SPI
void mcu_spi_config(uint8_t mode, uint32_t frequency)
{
	mode = CLAMP(0, mode, 4);
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
	SPI_REG->CR1 &= SPI_CR1_SPE;
	// clear speed and mode
	SPI_REG->CR1 &= 0x3B;
	SPI_REG->CR1 |= (speed << 3) | mode;
	// enable SPI
	SPI_REG->CR1 |= SPI_CR1_SPE;
}
#endif

#ifdef MCU_HAS_I2C
#if I2C_ADDRESS == 0

void mcu_i2c_write_stop(bool *stop)
{
	if (*stop)
	{
		uint32_t ms_timeout = mcu_millis() + 25;

		I2C_REG->CR1 |= I2C_CR1_STOP;
		while ((I2C_REG->CR1 & I2C_CR1_STOP) && (ms_timeout > mcu_millis()))
			;
	}
}

static uint8_t mcu_i2c_write(uint8_t data, bool send_start, bool send_stop)
{
	bool stop __attribute__((__cleanup__(mcu_i2c_write_stop))) = send_stop;
	uint32_t ms_timeout = mcu_millis() + 25;

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
		while (!((I2C_REG->SR1 & I2C_SR1_SB) && (I2C_REG->SR2 & I2C_SR2_MSL) && (I2C_REG->SR2 & I2C_SR2_BUSY)))
		{
			if (I2C_REG->SR1 & I2C_SR1_ARLO)
			{
				stop = false;
				return I2C_NOTOK;
			}
			if (ms_timeout < mcu_millis())
			{
				stop = true;
				return I2C_NOTOK;
			}
		}
		if (I2C_REG->SR1 & I2C_SR1_AF)
		{
			stop = true;
			return I2C_NOTOK;
		}
	}

	I2C_REG->DR = data;
	while (!(I2C_REG->SR1 & status))
	{
		if (I2C_REG->SR1 & I2C_SR1_AF)
		{
			break;
		}
		if (I2C_REG->SR1 & I2C_SR1_ARLO)
		{
			stop = false;
			return I2C_NOTOK;
		}
		if (ms_timeout < mcu_millis())
		{
			stop = true;
			return I2C_NOTOK;
		}
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
	ms_timeout += mcu_millis();
	bool stop __attribute__((__cleanup__(mcu_i2c_write_stop))) = send_stop;

	if (!with_ack)
	{
		I2C_REG->CR1 &= ~I2C_CR1_ACK;
	}
	else
	{
		I2C_REG->CR1 |= I2C_CR1_ACK;
	}

	while (!(I2C_REG->SR1 & I2C_SR1_RXNE))
	{
		if (ms_timeout < mcu_millis())
		{
			stop = true;
			return I2C_NOTOK;
		}
	}

	*data = I2C_REG->DR;
	return I2C_OK;
}

#ifndef mcu_i2c_send
// master sends command to slave
uint8_t mcu_i2c_send(uint8_t address, uint8_t *data, uint8_t datalen, bool release)
{
	if (data && datalen)
	{
		if (mcu_i2c_write(address << 1, true, false) == I2C_OK) // start, send address, write
		{
			// send data, stop
			do
			{
				datalen--;
				bool last = (datalen == 0);
				if (mcu_i2c_write(*data, false, (release & last)) != I2C_OK)
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
		if (mcu_i2c_write((address << 1) | 0x01, true, false) == I2C_OK) // start, send address, write
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
	mcu_config_output_af(I2C_CLK, GPIO_OUTALT_OD_50MHZ);
	mcu_config_output_af(I2C_DATA, GPIO_OUTALT_OD_50MHZ);
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
			mcu_enable_global_isr();
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
#if (ONESHOT_TIMER != 6 && ONESHOT_TIMER != 7)
	ONESHOT_TIMER_REG->CCER = 0;
	ONESHOT_TIMER_REG->CCMR1 = 0;
#if (ONESHOT_TIMER < 10)
	ONESHOT_TIMER_REG->CCMR2 = 0;
#endif
#endif

	NVIC_SetPriority(MCU_ONESHOT_IRQ, 3);
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
