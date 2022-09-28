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

/**
 * The isr functions
 * The respective IRQHandler will execute these functions
 **/
#if (INTERFACE == INTERFACE_UART)
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

#if (INTERFACE == INTERFACE_UART)
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
#if (INTERFACE == INTERFACE_UART)
	/*enables RCC clocks and GPIO*/
	mcu_config_output_af(TX, GPIO_OUTALT_OD_50MHZ);
	mcu_config_input_af(RX);
#ifdef COM_REMAP
	AFIO->MAPR |= COM_REMAP;
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

	// µs counting is now done via Systick

	// initialize debugger clock (used by us delay)
	// if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
	// {
	// 	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	// 	DWT->CYCCNT = 0;
	// 	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	// }
}

void mcu_putc(char c)
{
#if (INTERFACE == INTERFACE_UART)
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
	RCC->APB1ENR |= I2C_APBEN;
	mcu_config_output_af(I2C_SCL, GPIO_OUTALT_OD_50MHZ);
	mcu_config_output_af(I2C_SDA, GPIO_OUTALT_OD_50MHZ);
#ifdef SPI_REMAP
	AFIO->MAPR |= I2C_REMAP;
#endif
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

char mcu_getc(void)
{
#if (INTERFACE == INTERFACE_UART)
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
	SysTick->LOAD = ((F_CPU / 1000) - 1);
	SysTick->VAL = 0;
	NVIC_SetPriority(SysTick_IRQn, 10);
	SysTick->CTRL = 7; // Start SysTick (ABH)
}

// void mcu_delay_us(uint16_t delay)
// {
// 	uint32_t delayTicks = DWT->CYCCNT + delay * (F_CPU / 1000000UL);
// 	while (DWT->CYCCNT < delayTicks)
// 		;
// }

#define mcu_micros ((mcu_runtime_ms * 1000) + ((SysTick->LOAD - SysTick->VAL) / (F_CPU / 1000000)))
#ifndef mcu_delay_us
void mcu_delay_us(uint16_t delay)
{
	// lpc176x_delay_us(delay);
	uint32_t target = mcu_micros + delay;
	while (target > mcu_micros)
		;
}
#endif

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

#ifdef MCU_HAS_SPI
void mcu_spi_config(uint8_t mode, uint32_t frequency)
{
	mode = CLAMP(0, mode, 4);
	uint8_t div = (uint8_t)(F_CPU / frequency);
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

	NVIC_ClearPendingIRQ(ONESHOT_TIMER_IRQ);
}

/**
 * configures a single shot timeout in us
 * */
#ifndef mcu_config_timeout
void mcu_config_timeout(mcu_timeout_delgate fp, uint32_t timeout)
{
	// up and down counter (generates half the step rate at each event)
	uint32_t clocks = (uint32_t)((F_CPU / 1000000UL) * timeout);
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

	NVIC_SetPriority(ONESHOT_TIMER_IRQ, 1);
	NVIC_ClearPendingIRQ(ONESHOT_TIMER_IRQ);
	NVIC_EnableIRQ(ONESHOT_TIMER_IRQ);

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
