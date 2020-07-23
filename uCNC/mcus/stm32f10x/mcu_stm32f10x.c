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

static void mcu_usart_init(void)
{/*
	RCC->APB2ENR |= (RCC_APB2ENR_AFIOEN | USART_RCC_USARTEN);
	RCC->APB2ENR |= __indirect__(TX, APB2EN);
	__indirect__(TX, GPIO)->__indirect__(TX, CR) &= ~(GPIO_RESET << ((__indirect__(TX, CROFF)) << 2));
	__indirect__(TX, GPIO)->__indirect__(TX, CR) |= (GPIO_OUTALT_PP_50MHZ << ((__indirect__(TX, CROFF)) << 2));
	RCC->APB2ENR |= __indirect__(RX, APB2EN);
	__indirect__(RX, GPIO)->__indirect__(RX, CR) &= ~(GPIO_RESET << ((__indirect__(RX, CROFF)) << 2));
	__indirect__(RX, GPIO)->__indirect__(RX, CR) |= (GPIO_IN_FLOAT << ((__indirect__(RX, CROFF)) << 2));
	COM_PORT->CR1 &= ~0x1400UL; //8 bits No parity M=0 PCE=0
	COM_PORT->CR2 &= ~0x3000UL; //1 stop bit STOP=00
	// //115200 baudrate
	float baudrate = ((float)F_CPU / (16.0f * (float)BAUD));
	uint16_t brr = (uint16_t)baudrate;
	baudrate -= brr;
	brr <<= 4;
	brr += (uint16_t)roundf(16.0f * baudrate);
	COM_PORT->BRR = brr;
	COM_PORT->CR1 |= 0x200CUL; // enable TE, RE, UE
							  // //IRQn_Type irq = USART1_IRQn;
							  // //NVIC_EnableIRQ(irq); */
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
#ifdef LIMIT_X_ISR
	mcu_config_input_isr(LIMIT_X);
#endif
#endif
#ifdef LIMIT_Y
	mcu_config_input(LIMIT_Y);
#ifdef LIMIT_Y_ISR
	mcu_config_input_isr(LIMIT_Y);
#endif
#endif
#ifdef LIMIT_Z
	mcu_config_input(LIMIT_Z);
#ifdef LIMIT_Z_ISR
	mcu_config_input_isr(LIMIT_Z);
#endif
#endif
#ifdef LIMIT_X2
	mcu_config_input(LIMIT_X2);
#ifdef LIMIT_X2_ISR
	mcu_config_input_isr(LIMIT_X2);
#endif
#endif
#ifdef LIMIT_Y2
	mcu_config_input(LIMIT_Y2);
#ifdef LIMIT_Y2_ISR
	mcu_config_input_isr(LIMIT_Y2);
#endif
#endif
#ifdef LIMIT_Z2
	mcu_config_input(LIMIT_Z2);
#ifdef LIMIT_Z2_ISR
	mcu_config_input_isr(LIMIT_Z2);
#endif
#endif
#ifdef LIMIT_A
	mcu_config_input(LIMIT_A);
#ifdef LIMIT_A_ISR
	mcu_config_input_isr(LIMIT_A);
#endif
#endif
#ifdef LIMIT_B
	mcu_config_input(LIMIT_B);
#ifdef LIMIT_B_ISR
	mcu_config_input_isr(LIMIT_B);
#endif
#endif
#ifdef LIMIT_C
	mcu_config_input(LIMIT_C);
#ifdef LIMIT_C_ISR
	mcu_config_input_isr(LIMIT_C);
#endif
#endif
#ifdef PROBE
	mcu_config_input(PROBE);
#ifdef PROBE_ISR
	mcu_config_input_isr(PROBE);
#endif
#endif
#ifdef ESTOP
	mcu_config_input(ESTOP);
#ifdef ESTOP_ISR
	mcu_config_input_isr(ESTOP);
#endif
#endif
#ifdef SAFETY_DOOR
	mcu_config_input(SAFETY_DOOR);
#ifdef SAFETY_DOOR_ISR
	mcu_config_input_isr(SAFETY_DOOR);
#endif
#endif
#ifdef FHOLD
	mcu_config_input(FHOLD);
#ifdef FHOLD_ISR
	mcu_config_input_isr(FHOLD);
#endif
#endif
#ifdef CS_RES
	mcu_config_input(CS_RES);
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
#ifdef RX
	mcu_config_analog(RX);
#endif
#ifdef DIN0
	mcu_config_input(DIN0);
#endif
#ifdef DIN1
	mcu_config_input(DIN1);
#endif
#ifdef DIN2
	mcu_config_input(DIN2);
#endif
#ifdef DIN3
	mcu_config_input(DIN3);
#endif
#ifdef DIN4
	mcu_config_input(DIN4);
#endif
#ifdef DIN5
	mcu_config_input(DIN5);
#endif
#ifdef DIN6
	mcu_config_input(DIN6);
#endif
#ifdef DIN7
	mcu_config_input(DIN7);
#endif
#ifdef DIN8
	mcu_config_input(DIN8);
#endif
#ifdef DIN9
	mcu_config_input(DIN9);
#endif
#ifdef DIN10
	mcu_config_input(DIN10);
#endif
#ifdef DIN11
	mcu_config_input(DIN11);
#endif
#ifdef DIN12
	mcu_config_input(DIN12);
#endif
#ifdef DIN13
	mcu_config_input(DIN13);
#endif
#ifdef DIN14
	mcu_config_input(DIN14);
#endif
#ifdef DIN15
	mcu_config_input(DIN15);
#endif

#ifdef COM_PORT
	mcu_usart_init();
#endif

mcu_enable_interrupts();
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
}
void mcu_stop_send(void)
{
	//not used with USB VCP
}

#ifdef TX_BUFFER_SIZE
#undef TX_BUFFER_SIZE
#endif
#define TX_BUFFER_SIZE 112
static uint8_t mcu_tx_buffer[TX_BUFFER_SIZE];

extern uint8_t CDC_Transmit_FS(uint8_t *Buf, uint16_t Len);

void mcu_putc(char c)
{
	static uint16_t i = 0;
	mcu_tx_buffer[i] = (uint8_t)c;
	i++;
	if (c == '\n' || c == '\r' || i==(TX_BUFFER_SIZE - 1))
	{
		mcu_tx_buffer[i] = 0;
		CDC_Transmit_FS(mcu_tx_buffer, i);
		i = 0;
	}
}

char mcu_getc(void)
{
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
	uint32_t totalticks = (uint32_t)((float)(F_CPU>>1)/frequency);
	*prescaller = 1;
	while(totalticks<0xFFFFUL)
	{
		*prescaller <<= 1;
		totalticks >>=1;
	}

	*prescaller--;
	*ticks = (uint16_t)totalticks;
}

//starts a constant rate pulse at a given frequency.
void mcu_start_step_ISR(uint16_t ticks, uint16_t prescaller)
{
	TIMER_REG->PSC = prescaller;
	TIMER_REG->ARR = ticks;
	TIMER_REG->CR1 |= 0xE1UL; //enable counter in center-aligned mode and autoreload pre buffered
	TIMER_REG->DIER |= 0x1UL;
	HAL_NVIC_EnableIRQ(TIMER_IRQ);
}

//modifies the pulse frequency
void mcu_change_step_ISR(uint16_t ticks, uint16_t prescaller)
{
	TIMERREG->PSC = prescaller;
	TIMERREG->ARR = ticks;
}

//stops the pulse
void mcu_step_stop_ISR(void)
{
	TIMERREG->CR1 &= ~0x1UL;
	TIMERREG->DIER &= ~0x1UL;
}

//Custom delay function
//void mcu_delay_ms(uint16_t miliseconds);

//Non volatile memory
uint8_t mcu_eeprom_getc(uint16_t address)
{
}

void mcu_eeprom_putc(uint16_t address, uint8_t value)
{
}

#ifdef __PERFSTATS__
uint16_t mcu_get_step_clocks(void);
uint16_t mcu_get_step_reset_clocks(void);
#endif
