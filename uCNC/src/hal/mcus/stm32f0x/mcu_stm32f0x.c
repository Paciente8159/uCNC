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

#if (MCU == MCU_STM32F0X)
#include "core_cm0.h"
#include "stm32f0xx.h"
#include "mcumap_stm32f0x.h"
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

#if (FLASH_BANK1_END <= 0x0801FFFFUL)
#ifndef FLASH_PAGE_SIZE
#define FLASH_PAGE_SIZE (1 << 10)
#endif
#define FLASH_EEPROM_PAGES (((NVM_STORAGE_SIZE - 1) >> 10) + 1)
#define FLASH_EEPROM (FLASH_LIMIT - ((FLASH_EEPROM_PAGES << 10) - 1))
#define FLASH_PAGE_MASK (0xFFFF - (1 << 10) + 1)
#define FLASH_PAGE_OFFSET_MASK (0xFFFF & ~FLASH_PAGE_MASK)
#else
#ifndef FLASH_PAGE_SIZE
#define FLASH_PAGE_SIZE (1 << 11)
#endif
#define FLASH_EEPROM_PAGES (((NVM_STORAGE_SIZE - 1) >> 11) + 1)
#define FLASH_EEPROM (FLASH_LIMIT - ((FLASH_EEPROM_PAGES << 11) - 1))
#define FLASH_PAGE_MASK (0xFFFF - (1 << 11) + 1)
#define FLASH_PAGE_OFFSET_MASK (0xFFFF & ~FLASH_PAGE_MASK)
#endif

#define READ_FLASH(ram_ptr, flash_ptr) (*ram_ptr = ~(*flash_ptr))
#define WRITE_FLASH(flash_ptr, ram_ptr) (*flash_ptr = ~(*ram_ptr))
static uint8_t stm32_flash_page[FLASH_PAGE_SIZE];
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
#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE 64
#endif
DECL_BUFFER(uint8_t, uart_tx, UART_TX_BUFFER_SIZE);
DECL_BUFFER(uint8_t, uart_rx, RX_BUFFER_SIZE);

void MCU_SERIAL_ISR(void)
{
	__ATOMIC_FORCEON__
	{
		if (COM_UART->ISR & USART_ISR_RXNE)
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

		if ((COM_UART->ISR & USART_ISR_TXE) && (COM_UART->CR1 & USART_CR1_TXEIE))
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
		if (COM2_UART->ISR & USART_ISR_RXNE)
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
#endif
		}

		if ((COM2_UART->ISR & USART_ISR_TXE) && (COM2_UART->CR1 & USART_CR1_TXEIE))
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
void USB_IRQHandler(void)
{
	mcu_disable_global_isr();
	tusb_cdc_isr_handler();
	NVIC_ClearPendingIRQ(USB_IRQn);
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

#ifndef ARDUINO_ARCH_STM32
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

void mcu_clocks_init()
{
	SystemCoreClockUpdate();

	// initialize debugger clock (used by us delay)
	// if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
	// {
	// 	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	// 	DWT->CYCCNT = 0;
	// 	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	// }
}

void mcu_usart_init(void)
{
#ifdef MCU_HAS_USB
	// configure USB as Virtual COM port
	mcu_config_input(USB_DM);
	mcu_config_input(USB_DP);
	// mcu_config_af(USB_DP, GPIO_OTG_FS);
	// mcu_config_af(USB_DM, GPIO_OTG_FS);
	RCC->APB1ENR |= (RCC_APB1ENR_USBEN);
	/* Disable all interrupts. */

	NVIC_SetPriority(USB_IRQn, 10);
	NVIC_ClearPendingIRQ(USB_IRQn);
	NVIC_EnableIRQ(USB_IRQn);

	tusb_cdc_init();
#endif

#ifdef MCU_HAS_UART
	/*enables RCC clocks and GPIO*/
	RCC->COM_APB |= (COM_APBEN);
	mcu_config_af(TX, UART_TX_AFIO);
	mcu_config_af(RX, UART_RX_AFIO);
	/*setup UART*/
	COM_UART->CR1 = 0; // 8 bits No parity M=0 PCE=0
	COM_UART->CR2 = 0; // 1 stop bit STOP=00
	COM_UART->CR3 = 0;
	COM_UART->ISR = 0;
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
	mcu_config_af(TX2, UART2_TX_AFIO);
	mcu_config_af(RX2, UART2_RX_AFIO);
	/*setup UART*/
	COM2_UART->CR1 = 0; // 8 bits No parity M=0 PCE=0
	COM2_UART->CR2 = 0; // 1 stop bit STOP=00
	COM2_UART->CR3 = 0;
	COM2_UART->ISR = 0;
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

	RCC->AHBENR |= RCC_AHBENR_DMAEN;
#if ASSERT_PIN_IO(SPI_CS)
	mcu_config_af(SPI_CS, SPI_CS_AFIO);
#endif
	// initialize the SPI configuration register
	SPI_REG->CR1 = SPI_CR1_SSM		 // software slave management enabled
								 | SPI_CR1_SSI	 // internal slave select
								 | SPI_CR1_MSTR; // SPI master mode
																 //    | (SPI_SPEED << 3) | SPI_MODE;
	spi_config_t spi_conf = {0};
	spi_conf.mode = SPI_MODE;
	mcu_spi_config(spi_conf, SPI_FREQ);

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

	RCC->AHBENR |= RCC_AHBENR_DMAEN;
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

	NVIC_SetPriority(SPI2_IRQ, 2);
	NVIC_ClearPendingIRQ(SPI2_IRQ);
	NVIC_EnableIRQ(SPI2_IRQ);

	SPI2_REG->CR1 |= SPI_CR1_SPE;
#endif
#ifdef MCU_HAS_I2C
	RCC->APB1ENR |= I2C_APBEN;
	mcu_config_af(I2C_CLK, I2C_CLK_AFIO);
	mcu_config_af(I2C_DATA, I2C_DATA_AFIO);
	mcu_config_pullup(I2C_CLK);
	mcu_config_pullup(I2C_DATA);
	// set opendrain
	mcu_config_opendrain(I2C_CLK);
	mcu_config_opendrain(I2C_DATA);
	// reset I2C
	I2C_REG->CR1 |= I2C_CR1_SWRST;
	I2C_REG->CR1 &= ~I2C_CR1_SWRST;
	// set max freq
	mcu_i2c_config(I2C_FREQ);
#endif

	mcu_disable_probe_isr();
	stm32_flash_current_page = -1;
	stm32_global_isr_enabled = false;
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

			*(BUFFER_NEXT_FREE(usb_rx)) = c;
			BUFFER_STORE(usb_rx);
		}
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
		mcu_eeprom_flush();
		stm32_flash_modified = false;
		stm32_flash_current_page = address_page;
		uint16_t counter = (uint16_t)(FLASH_PAGE_SIZE >> 2);
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
	if (NVM_STORAGE_SIZE <= address)
	{
		DBGMSG("EEPROM invalid address @ %u", address);
		return 0;
	}
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
	FLASH->CR = 0;						 // Ensure PG bit is low
	FLASH->CR |= FLASH_CR_PER; // set the PER bit
	FLASH->AR = (FLASH_EEPROM + address);
	FLASH->CR |= FLASH_CR_STRT; // set the start bit
	while (FLASH->SR & FLASH_SR_BSY)
		; // wait while busy
	FLASH->CR = 0;
}

void mcu_eeprom_putc(uint16_t address, uint8_t value)
{
	if (NVM_STORAGE_SIZE <= address)
	{
		DBGMSG("EEPROM invalid address @ %u", address);
	}

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
		uint16_t counter = (uint16_t)(FLASH_PAGE_SIZE >> 1);
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
				proto_error(42); // STATUS_SETTING_WRITE_FAIL
			if (FLASH->SR & FLASH_SR_WRPRTERR)
				proto_error(43); // STATUS_SETTING_PROTECTED_FAIL
			FLASH->CR = 0;		 // Ensure PG bit is low
			FLASH->SR = 0;
			eeprom++;
			ptr++;
		}
		stm32_flash_modified = false;
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
		if (!(DMA1->ISR >> (SPI_DMA_TX_IFR_POS + 5)) ||
				(!(DMA1->ISR >> (SPI_DMA_RX_IFR_POS + 5)) && rx_data))
			return true;

		// SPI hardware still transmitting the last byte
		if (SPI_REG->SR & SPI_SR_BSY)
			return true;

		// Disable DMA use
		SPI_REG->CR2 &= ~SPI_CR2_TXDMAEN;
		if (rx_data)
			SPI_REG->CR2 &= ~SPI_CR2_RXDMAEN;
		// Disable channels
		SPI_DMA_TX_CHANNEL->CCR &= ~DMA_CCR_EN;
		if (rx_data)
			SPI_DMA_RX_CHANNEL->CCR &= ~DMA_CCR_EN;

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
	if ((SPI_DMA_TX_CHANNEL->CCR & DMA_CCR_EN) || ((SPI_DMA_RX_CHANNEL->CCR & DMA_CCR_EN) && rx_data))
		return true;

	/***     Setup Transmit DMA     ***/

	// Clear flags
	DMA1->IFCR |= SPI_DMA_TX_IFCR_MASK;

	SPI_DMA_TX_CHANNEL->CCR =
			(0b01 << DMA_CCR_PL_Pos) | // Priority medium
			DMA_CCR_MINC |						 // Increment memory
			DMA_CCR_DIR;							 // Memory to peripheral

	SPI_DMA_TX_CHANNEL->CPAR = (uint32_t)&SPI_REG->DR;
	SPI_DMA_TX_CHANNEL->CMAR = (uint32_t)tx_data;

	SPI_DMA_TX_CHANNEL->CNDTR = datalen;

	// Enable DMA use for transmission
	SPI_REG->CR2 |= SPI_CR2_TXDMAEN;

	if (rx_data)
	{
		/***     Setup Receive DMA     ***/

		// Clear flags
		DMA1->IFCR |= SPI_DMA_RX_IFCR_MASK;

		SPI_DMA_RX_CHANNEL->CCR =
				(0b01 << DMA_CCR_PL_Pos) | // Priority medium
				DMA_CCR_MINC;							 // Increment memory

		SPI_DMA_RX_CHANNEL->CPAR = (uint32_t)&SPI_REG->DR;
		SPI_DMA_RX_CHANNEL->CMAR = (uint32_t)rx_data;

		SPI_DMA_RX_CHANNEL->CNDTR = datalen;

		// Enable DMA use for reception
		SPI_REG->CR2 |= SPI_CR2_RXDMAEN;
	}

	/***     DMA Setup Complete     ***/

	// Start streams
	SPI_DMA_TX_CHANNEL->CCR |= DMA_CCR_EN;
	if (rx_data)
		SPI_DMA_RX_CHANNEL->CCR |= DMA_CCR_EN;

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
		if (!(DMA1->ISR >> (SPI2_DMA_TX_IFR_POS + 5)) ||
				(!(DMA1->ISR >> (SPI2_DMA_RX_IFR_POS + 5)) && rx_data))
			return true;

		// SPI2 hardware still transmitting the last byte
		if (SPI2_REG->SR & SPI_SR_BSY)
			return true;

		// Disable DMA use
		SPI2_REG->CR2 &= ~SPI_CR2_TXDMAEN;
		if (rx_data)
			SPI2_REG->CR2 &= ~SPI_CR2_RXDMAEN;
		// Disable channels
		SPI2_DMA_TX_CHANNEL->CCR &= ~DMA_CCR_EN;
		if (rx_data)
			SPI2_DMA_RX_CHANNEL->CCR &= ~DMA_CCR_EN;

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
	if ((SPI2_DMA_TX_CHANNEL->CCR & DMA_CCR_EN) || ((SPI2_DMA_RX_CHANNEL->CCR & DMA_CCR_EN) && rx_data))
		return true;

	/***     Setup Transmit DMA     ***/

	// Clear flags
	DMA1->IFCR |= SPI2_DMA_TX_IFCR_MASK;

	SPI2_DMA_TX_CHANNEL->CCR =
			(0b01 << DMA_CCR_PL_Pos) | // Priority medium
			DMA_CCR_MINC |						 // Increment memory
			DMA_CCR_DIR;							 // Memory to peripheral

	SPI2_DMA_TX_CHANNEL->CPAR = (uint32_t)&SPI2_REG->DR;
	SPI2_DMA_TX_CHANNEL->CMAR = (uint32_t)tx_data;

	SPI2_DMA_TX_CHANNEL->CNDTR = datalen;

	// Enable DMA use for transmission
	SPI2_REG->CR2 |= SPI_CR2_TXDMAEN;

	if (rx_data)
	{
		/***     Setup Receive DMA     ***/

		// Clear flags
		DMA1->IFCR |= SPI2_DMA_RX_IFCR_MASK;

		SPI2_DMA_RX_CHANNEL->CCR =
				(0b01 << DMA_CCR_PL_Pos) | // Priority medium
				DMA_CCR_MINC;							 // Increment memory

		SPI2_DMA_RX_CHANNEL->CPAR = (uint32_t)&SPI2_REG->DR;
		SPI2_DMA_RX_CHANNEL->CMAR = (uint32_t)rx_data;

		SPI2_DMA_RX_CHANNEL->CNDTR = datalen;

		// Enable DMA use for reception
		SPI2_REG->CR2 |= SPI_CR2_RXDMAEN;
	}

	/***     DMA Setup Complete     ***/

	// Start streams
	SPI2_DMA_TX_CHANNEL->CCR |= DMA_CCR_EN;
	if (rx_data)
		SPI2_DMA_RX_CHANNEL->CCR |= DMA_CCR_EN;

	// Transmission started
	spi2_port_state = SPI_TRANSMITTING;
	return true;
}
#endif

#ifdef MCU_HAS_I2C

#if I2C_ADDRESS == 0
#ifndef mcu_i2c_send
#define STM_VAL2REG(val, X) (((val << X##_Pos) & X##_Msk))
// master sends command to slave
uint8_t mcu_i2c_send(uint8_t address, uint8_t *data, uint8_t datalen, bool release, uint32_t ms_timeout)
{
	if (!data || !datalen)
	{
		return I2C_NOTOK;
	}

	I2C_REG->CR2 = (address << 1) | I2C_CR2_START | STM_VAL2REG(datalen, I2C_CR2_NBYTES);
	for (uint8_t i = 0; i < datalen; i++)
	{
		if ((I2C_REG->ISR & I2C_ISR_NACKF))
		{
			return I2C_NOTOK;
		}
		uint32_t t = ms_timeout;
		__TIMEOUT_MS__(t)
		{
			if ((I2C_REG->ISR & I2C_ISR_TXIS))
			{
				break;
			}
		}

		__TIMEOUT_ASSERT__(t)
		{
			return I2C_NOTOK;
		}

		I2C_REG->TXDR = data[i];
	}

	if (release)
	{
		uint32_t t = ms_timeout;
		__TIMEOUT_MS__(t)
		{
			if ((I2C_REG->ISR & I2C_ISR_TC))
			{
				break;
			}
		}

		__TIMEOUT_ASSERT__(t)
		{
			return I2C_NOTOK;
		}

		I2C_REG->CR2 |= I2C_CR2_STOP;

		t = ms_timeout;
		__TIMEOUT_MS__(t)
		{
			if ((I2C_REG->ISR & I2C_ISR_STOPF))
			{
				break;
			}
		}

		__TIMEOUT_ASSERT__(t)
		{
			return I2C_NOTOK;
		}

		I2C_REG->ICR |= I2C_ICR_STOPCF;
		I2C_REG->CR2 = 0x0;
	}

	return I2C_OK;
}
#endif

#ifndef mcu_i2c_receive
// master receive response from slave
uint8_t mcu_i2c_receive(uint8_t address, uint8_t *data, uint8_t datalen, uint32_t ms_timeout)
{
	if (!data || !datalen)
	{
		return I2C_NOTOK;
	}

	uint32_t t = ms_timeout;
	__TIMEOUT_MS__(t)
	{
		if (!(I2C_REG->ISR & I2C_ISR_BUSY))
		{
			break;
		}
	}

	__TIMEOUT_ASSERT__(t)
	{
		return I2C_NOTOK;
	}

	I2C_REG->CR2 = I2C_CR2_RD_WRN | I2C_CR2_AUTOEND | STM_VAL2REG(datalen, I2C_CR2_NBYTES) | (address << 1) | (I2C_CR2_START);

	for (uint8_t i = 0; i < datalen; i++)
	{
		t = ms_timeout;
		__TIMEOUT_MS__(t)
		{
			if (((I2C_REG->ISR & I2C_ISR_BERR)) || ((I2C_REG->ISR & I2C_ISR_ARLO)))
			{
				return I2C_NOTOK;
			}
			if ((I2C_REG->ISR & I2C_ISR_RXNE))
			{
				break;
			}
		}

		__TIMEOUT_ASSERT__(t)
		{
			return I2C_NOTOK;
		}

		data[i] = (uint8_t)I2C_REG->RXDR;
	}

	t = ms_timeout;
	__TIMEOUT_MS__(t)
	{
		if (!(I2C_REG->ISR & I2C_ISR_STOPF))
		{
			break;
		}
	}

	__TIMEOUT_ASSERT__(t)
	{
		return I2C_NOTOK;
	}

	I2C_REG->ICR |= I2C_ICR_STOPCF;
	I2C_REG->CR2 = 0x0;

	return I2C_OK;
}
#endif
#endif

#ifndef mcu_i2c_config
void mcu_i2c_config(uint32_t frequency)
{
	RCC->APB1ENR &= ~I2C_APBEN;
	RCC->CFGR3 &= ~RCC_CFGR3_I2C1SW; // use HSI clock source
	while (RCC->CFGR3 & RCC_CFGR3_I2C1SW)
		;
	RCC->APB1ENR |= I2C_APBEN;
	mcu_config_opendrain(I2C_CLK);
	mcu_config_opendrain(I2C_DATA);
	mcu_config_af(I2C_CLK, GPIO_AF);
	mcu_config_af(I2C_DATA, GPIO_AF);
#ifdef I2C_REMAP
	AFIO->MAPR |= I2C_REMAP;
#endif
	// reset I2C
	I2C_REG->CR1 |= I2C_CR1_SWRST;
	I2C_REG->CR1 &= ~I2C_CR1_SWRST;
#if I2C_ADDRESS == 0
	// set max freq
	uint8_t presc = 0;
	while ((HSI_VALUE / (presc + 1)) > (frequency * 255UL))
	{
		presc++;
	}

	float i2c_osc = (uint8_t)CLAMP(0, (HSI_VALUE / (presc + 1)), 0x0F);
	uint8_t scll = 0, sclh = scll = (uint8_t)CLAMP(0, (i2c_osc / (frequency * 2) - 1), 0xFFFF); // half time clock up and clock down
	float risetime = 8333333.3f, falltime = 8333333.3f;
	if (frequency <= 100000UL) // standart mode (max fall time 1000ns(1Mhz) and max rise time 300ns(3,3MHZ))
	{
		risetime = 1000000.0f;
		falltime = 3333333.3f;
	}
	else if (frequency <= 400000UL) // fast mode
	{
		risetime = 3333333.3f;
		falltime = 3333333.3f;
	}

	uint8_t scldel = (uint8_t)CLAMP(0, (ceilf((float)i2c_osc / risetime) - 1), 0x0F);
	uint8_t sdadel = (uint8_t)CLAMP(0, (ceilf((float)i2c_osc / falltime) - 1), 0x0F);

	sclh -= sdadel;

	I2C_REG->TIMEOUTR = (STM_VAL2REG(presc, I2C_TIMINGR_PRESC) | STM_VAL2REG(scll, I2C_TIMINGR_SCLL) | STM_VAL2REG(sclh, I2C_TIMINGR_SCLH) | STM_VAL2REG(scldel, I2C_TIMINGR_SCLDEL) | STM_VAL2REG(sdadel, I2C_TIMINGR_SDADEL));
#else
	// set address
	I2C_REG->OAR1 &= ~(I2C_OAR1_OA1MODE | 0x0F);
	I2C_REG->OAR1 |= (I2C_ADDRESS << 1);
	I2C_REG->OAR2 = 0;
	I2C_REG->CR1 &= ~I2C_CR1_NOSTRETCH;
	// enable interrupts
	I2C_REG->CR1 |= (I2C_CR1_RXIE | I2C_CR1_TXIE | I2C_CR1_ADDRIE);
	NVIC_SetPriority(I2C_IRQ, 10);
	NVIC_ClearPendingIRQ(I2C_IRQ);
	NVIC_EnableIRQ(I2C_IRQ);
#endif
	// initialize the SPI configuration register
	I2C_REG->CR1 |= (I2C_CR1_PE | I2C_CR1_GCEN);
#if I2C_ADDRESS != 0
	// prepare ACK in slave mode
	I2C_REG->CR2 &= ~I2C_CR2_NACK;
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
	if ((I2C_REG->ISR & I2C_ISR_ADDR))
	{
		// clear the ISR flag
		volatile uint32_t status = I2C_REG->ISR;

		// // Address matched, do necessary processing
		if ((status & I2C_ISR_ADDR) && !datalen)
		{
			mcu_i2c_buffer[i] = 0;
			// unlock ISR and process the info request
			// mcu_enable_global_isr();
			mcu_i2c_slave_cb(mcu_i2c_buffer, &i);
			datalen = i;
		}
		i = 0;
	}

	if ((I2C_REG->ISR & I2C_ISR_RXNE))
	{
		mcu_i2c_buffer[i++] = I2C_REG->RXDR;
	}

	if ((I2C_REG->ISR & I2C_ISR_TXE))
	{
		I2C_REG->TXDR = mcu_i2c_buffer[i++];
		if (i > datalen)
		{
			// send NACK
			I2C_REG->CR2 |= I2C_CR2_STOP;
			datalen = 0;
		}
	}

	// Clear ISR flag by reading SR1 and writing CR1 registers
	if ((I2C_REG->ISR & I2C_ISR_STOPF))
	{
		// clear the ISR flag
		I2C_REG->ICR |= I2C_ICR_STOPCF;
		// stop transmission
		datalen = 0;
	}

	// An error ocurred
	if (I2C_REG->ISR & 0X3F00)
	{
		// prepare ACK for next transmission
		index = 0;
		datalen = 0;
		I2C_REG->CR1 &= ~I2C_CR1_NACKIE;
		I2C_REG->CR1 |= (I2C_CR1_RXIE | I2C_CR1_TXIE);
		// clear ISR flag
		I2C_REG->ICR = 0X00FF;
		volatile uint32_t status = I2C_REG->ISR;
	}

	// prepare ACK for next transmition
	index = i;
	I2C_REG->CR1 &= ~I2C_CR1_NACKIE;
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
