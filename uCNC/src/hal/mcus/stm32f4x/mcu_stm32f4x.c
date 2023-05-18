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

// set the FLASH EEPROM SIZE
#define FLASH_EEPROM_SIZE 0x400
#define FLASH_EEPROM_SIZE_WORD (FLASH_EEPROM_SIZE >> 2)
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
void MCU_SERIAL_ISR(void)
{
	mcu_disable_global_isr();
	if (COM_UART->SR & USART_SR_RXNE)
	{
		unsigned char c = COM_INREG;
		mcu_com_rx_cb(c);
	}

#ifndef ENABLE_SYNC_TX
	if ((COM_UART->SR & USART_SR_TXE) && (COM_UART->CR1 & USART_CR1_TXEIE))
	{
		COM_UART->CR1 &= ~(USART_CR1_TXEIE);
		mcu_com_tx_cb();
	}
#endif
	mcu_enable_global_isr();
}
#endif

#if (defined(MCU_HAS_UART2))
void MCU_SERIAL2_ISR(void)
{
	mcu_disable_global_isr();
	if (COM2_UART->SR & USART_SR_RXNE)
	{
		unsigned char c = COM2_INREG;
#if !defined(UART2_DETACH_MAIN_PROTOCOL)
		mcu_com_rx_cb(c);
#elif defined(UART2_PASSTHROUGH)
		mcu_uart_putc(c);
		mcu_uart_rcv_cb(c);
#endif
	}

#ifndef ENABLE_SYNC_TX
	if ((COM2_UART->SR & USART_SR_TXE) && (COM2_UART->CR1 & USART_CR1_TXEIE))
	{
		COM2_UART->CR1 &= ~(USART_CR1_TXEIE);
		mcu_com_tx_cb();
	}
#endif
	mcu_enable_global_isr();
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

void mcu_putc(char c)
{
#ifdef ENABLE_SYNC_TX
	while (!mcu_tx_ready())
	{
#ifdef MCU_HAS_USB
		tusb_cdc_flush();
#endif
	}
#endif

#ifdef MCU_HAS_UART
	COM_OUTREG = c;
#ifndef ENABLE_SYNC_TX
	COM_UART->CR1 |= (USART_CR1_TXEIE);
#endif
#endif

#if (defined(MCU_HAS_UART2) && !defined(UART2_DETACH_MAIN_PROTOCOL))
	COM2_OUTREG = c;
#ifndef ENABLE_SYNC_TX
	COM2_UART->CR1 |= (USART_CR1_TXEIE);
#endif
#endif

#ifdef MCU_HAS_USB
	if (c != 0)
	{
		tusb_cdc_write(c);
	}
	if (c == '\r' || c == 0)
	{
		tusb_cdc_flush();
	}
#endif
}

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
	mcu_config_af(SPI_SDI, SPI_AFIO);
	mcu_config_af(SPI_CLK, SPI_AFIO);
	mcu_config_af(SPI_SDO, SPI_AFIO);
	// initialize the SPI configuration register
	SPI_REG->CR1 = SPI_CR1_SSM	   // software slave management enabled
				   | SPI_CR1_SSI   // internal slave select
				   | SPI_CR1_MSTR; // SPI master mode
								   //    | (SPI_SPEED << 3) | SPI_MODE;
	mcu_spi_config(SPI_MODE, SPI_FREQ);

	SPI_REG->CR1 |= SPI_CR1_SPE;
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
	// initialize the SPI configuration register
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

// Analog input
#ifndef mcu_get_analog
uint8_t mcu_get_analog(uint8_t channel)
{
}
#endif

// PWM
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
	uint32_t totalticks = (uint32_t)((float)(TIMER_CLOCK >> 1) / frequency);
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
	return ((mcu_runtime_ms * 1000) + ((SysTick->LOAD - SysTick->VAL) / (F_CPU / 1000000)));
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
	tusb_cdc_flush();
	tusb_cdc_task(); // tinyusb device task

	while (tusb_cdc_available())
	{
		unsigned char c = (unsigned char)tusb_cdc_read();
		mcu_com_rx_cb(c);
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
	FLASH->CR = 0;																				// Ensure PG bit is low
	FLASH->CR |= FLASH_CR_SER | (((FLASH_SECTORS - 1) << FLASH_CR_SNB_Pos) & FLASH_CR_MER_Msk); // set the SER bit
	FLASH->CR |= FLASH_CR_STRT;																	// set the start bit
	while (FLASH->SR & FLASH_SR_BSY)
		; // wait while busy
	FLASH->CR = 0;
}

void mcu_eeprom_putc(uint16_t address, uint8_t value)
{
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
				protocol_send_error(42); // STATUS_SETTING_WRITE_FAIL
			if (FLASH->SR & FLASH_SR_WRPERR)
				protocol_send_error(43); // STATUS_SETTING_PROTECTED_FAIL
			FLASH->CR = 0;				 // Ensure PG bit is low
			FLASH->SR = 0;
			eeprom++;
			ptr++;
		}
		stm32_flash_modified = EEPROM_CLEAN;

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
#ifndef mcu_i2c_write
uint8_t mcu_i2c_write(uint8_t data, bool send_start, bool send_stop)
{
	uint32_t status = send_start ? I2C_SR1_ADDR : I2C_SR1_BTF;
	I2C_REG->SR1 &= ~I2C_SR1_AF;
	if (send_start)
	{
		// init
		I2C_REG->CR1 |= I2C_CR1_START;
		while (!((I2C_REG->SR1 & I2C_SR1_SB) && (I2C_REG->SR2 & I2C_SR2_MSL) && (I2C_REG->SR2 & I2C_SR2_BUSY)))
			;
		if (I2C_REG->SR1 & I2C_SR1_AF)
		{
			I2C_REG->CR1 |= I2C_CR1_STOP;
			while ((I2C_REG->CR1 & I2C_CR1_STOP))
				;
			return 0;
		}
	}

	I2C_REG->DR = data;
	while (!(I2C_REG->SR1 & status))
		;
	// read SR2 to clear ADDR
	if (send_start)
	{
		status = I2C_REG->SR2;
	}

	if (I2C_REG->SR1 & I2C_SR1_AF)
	{
		I2C_REG->CR1 |= I2C_CR1_STOP;
		while ((I2C_REG->CR1 & I2C_CR1_STOP))
			;
		return 0;
	}

	if (send_stop)
	{
		I2C_REG->CR1 |= I2C_CR1_STOP;
		while ((I2C_REG->CR1 & I2C_CR1_STOP))
			;
	}

	return 1;
}
#endif

#ifndef mcu_i2c_read
uint8_t mcu_i2c_read(bool with_ack, bool send_stop)
{
	uint8_t c = 0;

	if (!with_ack)
	{
		I2C_REG->CR1 &= ~I2C_CR1_ACK;
	}
	else
	{
		I2C_REG->CR1 |= I2C_CR1_ACK;
	}

	while (!(I2C_REG->SR1 & I2C_SR1_RXNE))
		;
	;
	c = I2C_REG->DR;

	if (send_stop)
	{
		I2C_REG->CR1 |= I2C_CR1_STOP;
		while ((I2C_REG->CR1 & I2C_CR1_STOP))
			;
	}

	return c;
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

#if (defined(MCU_HAS_UART2) && defined(UART2_DETACH_MAIN_PROTOCOL))
#ifndef mcu_uart_putc
void mcu_uart_putc(uint8_t c)
{
	while (!(COM2_UART->SR & USART_SR_TXE))
		;
	COM2_OUTREG = c;
}
#endif
#ifndef mcu_uart_getc
int16_t mcu_uart_getc(uint32_t timeout)
{
	timeout += mcu_millis();
	while (!(COM2_UART->SR & USART_SR_RXNE))
	{
		if (timeout < mcu_millis())
		{
			return -1;
		}
	}
	return COM2_INREG;
}
#endif
#endif

#endif
