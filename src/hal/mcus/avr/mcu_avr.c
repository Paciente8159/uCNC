/*
	Name: mcu_avr.c
	Description: Implements mcu interface on AVR.
		Besides all the functions declared in the mcu.h it also implements the code responsible
		for handling:
			interpolator.h
				void itp_step_isr();
				void itp_step_reset_isr();
			serial.h
				void serial_rx_isr(unsinged char c);
				char serial_tx_isr();
			trigger_control.h
				void io_limits_isr();
				void io_controls_isr();
                void io_probe_isr(uint8_t probe);

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

#if (MCU == MCU_AVR)
#include "interface/serial.h"
#include "core/interpolator.h"
#include "core/io_control.h"
#include "modules/pid_controller.h"

#include <math.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
//#include <avr/delay.h>
#include <avr/eeprom.h>
#include <avr/cpufunc.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#ifndef BAUDRATE
#define BAUDRATE 115200
#endif

        //gets the mcu running time in ms
        static volatile uint32_t mcu_runtime_ms;
        ISR(RTC_COMPA_vect, ISR_BLOCK)
        {
                mcu_runtime_ms++;
#if PID_CONTROLLERS > 0
                mcu_enable_global_isr();
                pid_update_isr();
#endif
        }

        ISR(ITP_COMPA_vect, ISR_BLOCK)
        {
                itp_step_isr();
        }

        ISR(ITP_COMPB_vect, ISR_BLOCK)
        {
                itp_step_reset_isr();
        }

#ifndef USE_INPUTS_POOLING_ONLY

#if (PCINTA_MASK == 1)
        ISR(INT0_vect, ISR_BLOCK) // input pin on change service routine
        {
#if (PCINTA_LIMITS_MASK == 1)
                io_limits_isr();
#endif
#if (PCINTA_CONTROLS_MASK == 1)
                io_controls_isr();
#endif
#if (PROBE_ISRA == 1)
                io_probe_isr();
#endif
        }
#endif
#if (PCINTA_MASK == 4)
        ISR(INT1_vect, ISR_BLOCK) // input pin on change service routine
        {
#if (PCINTA_LIMITS_MASK == 4)
                io_limits_isr();
#endif
#if (PCINTA_CONTROLS_MASK == 4)
                io_controls_isr();
#endif
#if (PROBE_ISRA == 4)
                io_probe_isr();
#endif
        }
#endif
#if (PCINTA_MASK == 16)
        ISR(INT2_vect, ISR_BLOCK) // input pin on change service routine
        {
#if (PCINTA_LIMITS_MASK == 16)
                io_limits_isr();
#endif
#if (PCINTA_CONTROLS_MASK == 16)
                io_controls_isr();
#endif
#if (PROBE_ISRA == 16)
                io_probe_isr();
#endif
        }
#endif
#if (PCINTA_MASK == 64)
        ISR(INT3_vect, ISR_BLOCK) // input pin on change service routine
        {
#if (PCINTA_LIMITS_MASK == 64)
                io_limits_isr();
#endif
#if (PCINTA_CONTROLS_MASK == 64)
                io_controls_isr();
#endif
#if (PROBE_ISRA == 64)
                io_probe_isr();
#endif
        }
#endif
#if (PCINTB_MASK == 1)
        ISR(INT4_vect, ISR_BLOCK) // input pin on change service routine
        {
#if (PCINTB_LIMITS_MASK == 1)
                io_limits_isr();
#endif
#if (PCINTB_CONTROLS_MASK == 1)
                io_controls_isr();
#endif
#if (PROBE_ISRB == 1)
                io_probe_isr();
#endif
        }
#endif
#if (PCINTB_MASK == 4)
        ISR(INT5_vect, ISR_BLOCK) // input pin on change service routine
        {
#if (PCINTB_LIMITS_MASK == 4)
                io_limits_isr();
#endif
#if (PCINTB_CONTROLS_MASK == 4)
                io_controls_isr();
#endif
#if (PROBE_ISRB == 4)
                io_probe_isr();
#endif
        }
#endif
#if (PCINTB_MASK == 16)
        ISR(INT6_vect, ISR_BLOCK) // input pin on change service routine
        {
#if (PCINTB_LIMITS_MASK == 16)
                io_limits_isr();
#endif
#if (PCINTB_CONTROLS_MASK == 16)
                io_controls_isr();
#endif
#if (PROBE_ISRB == 16)
                io_probe_isr();
#endif
        }
#endif
#if (PCINTB_MASK == 64)
        ISR(INT7_vect, ISR_BLOCK) // input pin on change service routine
        {
#if (PCINTB_LIMITS_MASK == 64)
                io_limits_isr();
#endif
#if (PCINTB_CONTROLS_MASK == 64)
                io_controls_isr();
#endif
#if (PROBE_ISRB == 64)
                io_probe_isr();
#endif
        }
#endif

#if (PCINT0_MASK != 0)
        ISR(PCINT0_vect, ISR_BLOCK) // input pin on change service routine
        {
                static uint8_t prev_value = 0;
                uint8_t value = PCINT0_INREG;
                uint8_t diff = prev_value ^ value;
                prev_value = value;

#if (PCINT0_LIMITS_MASK != 0)
                if (diff & PCINT0_LIMITS_MASK)
                {
                        io_limits_isr();
                }
#endif
#if (PCINT0_CONTROLS_MASK != 0)
                if (diff & PCINT0_CONTROLS_MASK)
                {
                        io_controls_isr();
                }
#endif

#if (PROBE_ISR0 != 0)
                if (CHECKBIT(diff, PROBE_BIT))
                {
                        io_probe_isr();
                }
#endif
        }
#endif

#if (PCINT1_MASK != 0)
        ISR(PCINT1_vect, ISR_BLOCK) // input pin on change service routine
        {
                static uint8_t prev_value = 0;
                uint8_t value = PCINT1_INREG;
                uint8_t diff = prev_value ^ value;
                prev_value = value;

#if (PCINT1_LIMITS_MASK != 0)
                if (diff & PCINT1_LIMITS_MASK)
                {
                        io_limits_isr();
                }
#endif
#if (PCINT1_CONTROLS_MASK != 0)
                if (diff & PCINT1_CONTROLS_MASK)
                {
                        io_controls_isr();
                }
#endif

#if (PROBE_ISR1 != 0)
                if (CHECKBIT(diff, PROBE_BIT))
                {
                        io_probe_isr();
                }
#endif
        }
#endif

#if (PCINT2_MASK != 0)
        ISR(PCINT2_vect, ISR_BLOCK) // input pin on change service routine
        {
                static uint8_t prev_value = 0;
                uint8_t value = PCINT2_INREG;
                uint8_t diff = prev_value ^ value;
                prev_value = value;

#if (PCINT2_LIMITS_MASK != 0)
                if (diff & PCINT2_LIMITS_MASK)
                {
                        io_limits_isr();
                }
#endif
#if (PCINT2_CONTROLS_MASK != 0)
                if (diff & PCINT2_CONTROLS_MASK)
                {
                        io_controls_isr();
                }
#endif

#if (PROBE_ISR2 != 0)
                if (CHECKBIT(diff, PROBE_BIT))
                {
                        io_probe_isr();
                }
#endif
        }
#endif

#endif

        ISR(COM_RX_vect, ISR_BLOCK)
        {
                serial_rx_isr(COM_INREG);
        }
#ifndef ENABLE_SYNC_TX
        ISR(COM_TX_vect, ISR_BLOCK)
        {
                CLEARBIT(UCSRB, UDRIE);
                serial_tx_isr();
        }
#endif

#define mcu_config_output(x) SETBIT(__indirect__(x, DIRREG), __indirect__(x, BIT))
#define mcu_config_input(x) CLEARBIT(__indirect__(x, DIRREG), __indirect__(x, BIT))
#define mcu_config_pullup(x) SETBIT(__indirect__(x, OUTREG), __indirect__(x, BIT))
#define mcu_config_input_isr(x) SETFLAG(__indirect__(x, ISRREG), __indirect__(x, ISR_MASK))
#define mcu_config_pwm(x)                                               \
        {                                                               \
                SETBIT(__indirect__(x, DIRREG), __indirect__(x, BIT));  \
                __indirect__(x, TMRAREG) = __indirect__(x, MODE);       \
                __indirect__(x, TMRBREG) = __indirect__(x, PRESCALLER); \
                __indirect__(x, OCRREG) = 0;                            \
        }

        static void mcu_start_rtc();

        void mcu_init(void)
        {
                //disable WDT
                wdt_reset();
                MCUSR &= ~(1 << WDRF);
                WDTCSR |= (1 << WDCE) | (1 << WDE);
                WDTCSR = 0x00;

                //configure all pins
                //autogenerated
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
                mcu_config_input(ANALOG0);
#endif
#ifdef ANALOG1
                mcu_config_input(ANALOG1);
#endif
#ifdef ANALOG2
                mcu_config_input(ANALOG2);
#endif
#ifdef ANALOG3
                mcu_config_input(ANALOG3);
#endif
#ifdef ANALOG4
                mcu_config_input(ANALOG4);
#endif
#ifdef ANALOG5
                mcu_config_input(ANALOG5);
#endif
#ifdef ANALOG6
                mcu_config_input(ANALOG6);
#endif
#ifdef ANALOG7
                mcu_config_input(ANALOG7);
#endif
#ifdef ANALOG8
                mcu_config_input(ANALOG8);
#endif
#ifdef ANALOG9
                mcu_config_input(ANALOG9);
#endif
#ifdef ANALOG10
                mcu_config_input(ANALOG10);
#endif
#ifdef ANALOG11
                mcu_config_input(ANALOG11);
#endif
#ifdef ANALOG12
                mcu_config_input(ANALOG12);
#endif
#ifdef ANALOG13
                mcu_config_input(ANALOG13);
#endif
#ifdef ANALOG14
                mcu_config_input(ANALOG14);
#endif
#ifdef ANALOG15
                mcu_config_input(ANALOG15);
#endif
#ifdef DIN0
                mcu_config_input(DIN0);
#ifdef DIN0_PULLUP
                mcu_config_pullup(DIN0);
#endif
#ifdef DIN0_ISR
                mcu_config_input_isr(DIN0);
#endif
#endif
#ifdef DIN1
                mcu_config_input(DIN1);
#ifdef DIN1_PULLUP
                mcu_config_pullup(DIN1);
#endif
#ifdef DIN1_ISR
                mcu_config_input_isr(DIN1);
#endif
#endif
#ifdef DIN2
                mcu_config_input(DIN2);
#ifdef DIN2_PULLUP
                mcu_config_pullup(DIN2);
#endif
#ifdef DIN2_ISR
                mcu_config_input_isr(DIN2);
#endif
#endif
#ifdef DIN3
                mcu_config_input(DIN3);
#ifdef DIN3_PULLUP
                mcu_config_pullup(DIN3);
#endif
#ifdef DIN3_ISR
                mcu_config_input_isr(DIN3);
#endif
#endif
#ifdef DIN4
                mcu_config_input(DIN4);
#ifdef DIN4_PULLUP
                mcu_config_pullup(DIN4);
#endif
#ifdef DIN4_ISR
                mcu_config_input_isr(DIN4);
#endif
#endif
#ifdef DIN5
                mcu_config_input(DIN5);
#ifdef DIN5_PULLUP
                mcu_config_pullup(DIN5);
#endif
#ifdef DIN5_ISR
                mcu_config_input_isr(DIN5);
#endif
#endif
#ifdef DIN6
                mcu_config_input(DIN6);
#ifdef DIN6_PULLUP
                mcu_config_pullup(DIN6);
#endif
#ifdef DIN6_ISR
                mcu_config_input_isr(DIN6);
#endif
#endif
#ifdef DIN7
                mcu_config_input(DIN7);
#ifdef DIN7_PULLUP
                mcu_config_pullup(DIN7);
#endif
#ifdef DIN7_ISR
                mcu_config_input_isr(DIN7);
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
#ifdef TX
                mcu_config_output(TX);
#endif
#ifdef RX
                mcu_config_input(RX);
#endif

                //Set COM port
                // Set baud rate
                uint16_t UBRR_value;
#if BAUDRATE < 57600
                UBRR_value = ((F_CPU / (8L * BAUDRATE)) - 1) / 2;
                UCSRA &= ~(1 << U2X); // baud doubler off  - Only needed on Uno XXX
#else
                UBRR_value = ((F_CPU / (4L * BAUDRATE)) - 1) / 2;
                UCSRA |= (1 << U2X); // baud doubler on for high baud rates, i.e. 115200
#endif
                UBRRH = UBRR_value >> 8;
                UBRRL = UBRR_value;

                // enable rx, tx, and interrupt on complete reception of a byte and UDR empty
                UCSRB |= (1 << RXEN | 1 << TXEN | 1 << RXCIE);

//enable interrupts on pin changes
#ifndef USE_INPUTS_POOLING_ONLY
#if ((PCINT0_LIMITS_MASK | PCINT0_CONTROLS_MASK | PROBE_ISR0) != 0)
                SETBIT(PCICR, PCIE0);
#else
                CLEARBIT(PCICR, PCIE0);
#endif
#if ((PCINT1_LIMITS_MASK | PCINT1_CONTROLS_MASK | PROBE_ISR1) != 0)
                SETBIT(PCICR, PCIE1);
#else
                CLEARBIT(PCICR, PCIE1);
#endif
#if ((PCINT2_LIMITS_MASK | PCINT2_CONTROLS_MASK | PROBE_ISR2) != 0)
                SETBIT(PCICR, PCIE2);
#else
                CLEARBIT(PCICR, PCIE2);
#endif
#endif

                mcu_start_rtc();

                //disable probe isr
                mcu_disable_probe_isr();
                //enable interrupts
                mcu_enable_global_isr();
        }

        //IO functions
        void mcu_putc(char c)
        {
                if (c != 0)
                {
#ifdef ENABLE_SYNC_TX
                        loop_until_bit_is_set(UCSRA, UDRE);
#endif
                        COM_OUTREG = c;
                }
#ifndef ENABLE_SYNC_TX
                SETBIT(UCSRB, UDRIE);
#endif
        }

        char mcu_getc(void)
        {
#ifdef ENABLE_SYNC_RX
                loop_until_bit_is_set(UCSRA, RXC);
#endif
                return COM_INREG;
        }

        //RealTime
        void mcu_freq_to_clocks(float frequency, uint16_t *ticks, uint16_t *prescaller)
        {
                if (frequency < F_STEP_MIN)
                        frequency = F_STEP_MIN;
                if (frequency > F_STEP_MAX)
                        frequency = F_STEP_MAX;

                float clockcounter = F_CPU;

                if (frequency >= 245)
                {
                        *prescaller = 9;
                }
                else if (frequency >= 31)
                {
                        *prescaller = 10;
                        clockcounter *= 0.125;
                }
                else if (frequency >= 4)
                {
                        *prescaller = 11;
                        clockcounter *= 0.015625;
                }
                else if (frequency >= 1)
                {
                        *prescaller = 12;
                        clockcounter *= 0.00390625;
                }
                else
                {
                        *prescaller = 13;
                        clockcounter *= 0.0009765625;
                }

                *ticks = floorf((clockcounter / frequency)) - 1;
        }
        /*
	initializes the pulse ISR
	In Arduino this is done in TIMER1
	The frequency range is from 4Hz to F_PULSE
*/
        void mcu_start_itp_isr(uint16_t clocks_speed, uint16_t prescaller)
        {
                //stops timer
                ITP_TCCRB = 0;
                //CTC mode
                ITP_TCCRA = 0;
                //resets counter
                ITP_TCNT = 0;
                //set step clock
                ITP_OCRA = clocks_speed;
                //sets OCR0B to half
                //this will allways fire step_reset between pulses
                ITP_OCRB = ITP_OCRA >> 1;
                ITP_TIFR = 0;
                // enable timer interrupts on both match registers
                ITP_TIMSK |= (1 << ITP_OCIEB) | (1 << ITP_OCIEA);

                //start timer in CTC mode with the correct prescaler
                ITP_TCCRB = (uint8_t)prescaller;
        }

        // se implementar amass deixo de necessitar de prescaler
        void mcu_change_itp_isr(uint16_t clocks_speed, uint16_t prescaller)
        {
                //stops timer
                //ITP_TCCRB = 0;
                ITP_OCRB = clocks_speed >> 1;
                ITP_OCRA = clocks_speed;
                //sets OCR0B to half
                //this will allways fire step_reset between pulses

                //reset timer
                //ITP_TCNT = 0;
                //start timer in CTC mode with the correct prescaler
                ITP_TCCRB = (uint8_t)prescaller;
        }

        void mcu_stop_itp_isr(void)
        {
                ITP_TCCRB = 0;
                ITP_TIMSK &= ~((1 << ITP_OCIEB) | (1 << ITP_OCIEA));
        }

        //gets the mcu running time in ms
        uint32_t mcu_millis()
        {
                uint32_t val = mcu_runtime_ms;
                return val;
        }

        void mcu_start_rtc()
        {
#if (F_CPU <= 16000000UL)
                uint8_t clocks = ((F_CPU / 1000) >> 6) - 1;
#else
                uint8_t clocks = ((F_CPU / 1000) >> 8) - 1;
#endif
                //stops timer
                RTC_TCCRB = 0;
                //CTC mode
                RTC_TCCRA = 0;
                //resets counter
                RTC_TCNT = 0;
                //set step clock
                RTC_OCRA = clocks;
                RTC_TCCRA |= 2;
                RTC_TIFR = 0;
                // enable timer interrupts on both match registers
                RTC_TIMSK |= (1 << RTC_OCIEA);
//start timer in CTC mode with the correct prescaler
#if (F_CPU <= 16000000UL)
#if (RTC_TIMER != 2)
                RTC_TCCRB |= 3;
#else
                RTC_TCCRB |= 4;
#endif
#else
#if (RTC_TIMER != 2)
                RTC_TCCRB |= 4;
#else
                RTC_TCCRB |= 6;
#endif
#endif
        }

        void mcu_dotasks()
        {
#ifdef ENABLE_SYNC_RX
                //read any char that is received
                while (CHECKBIT(UCSRA, RXC))
                {
                        unsigned char c = mcu_getc();
                        serial_rx_isr(c);
                }
#endif
        }

//This was copied from grbl
#ifndef EEPE
#define EEPE EEWE   //!< EEPROM program/write enable.
#define EEMPE EEMWE //!< EEPROM master program/write enable.
#endif

#ifndef SELFPRGEN
#define SELFPRGEN SPMEN
#endif

/* These two are unfortunately not defined in the device include files. */
#define EEPM1 5 //!< EEPROM Programming Mode Bit 1.
#define EEPM0 4 //!< EEPROM Programming Mode Bit 0.

        uint8_t mcu_eeprom_getc(uint16_t address)
        {
                do
                {

                } while (EECR & (1 << EEPE)); // Wait for completion of previous write.
                EEAR = address;               // Set EEPROM address register.
                EECR = (1 << EERE);           // Start EEPROM read operation.
                return EEDR;                  // Return the byte read from EEPROM.
        }

        void mcu_eeprom_putc(uint16_t address, uint8_t value)
        {
                uint8_t old_value; // Old EEPROM value.
                uint8_t diff_mask; // Difference mask, i.e. old value XOR new value.

                cli(); // Ensure atomic operation for the write operation.

                do
                {
                } while (EECR & (1 << EEPE)); // Wait for completion of previous write.

                do
                {
                } while (SPMCSR & (1 << SELFPRGEN)); // Wait for completion of SPM.

                EEAR = address;                // Set EEPROM address register.
                EECR = (1 << EERE);            // Start EEPROM read operation.
                old_value = EEDR;              // Get old EEPROM value.
                diff_mask = old_value ^ value; // Get bit differences.
                // Check if any bits are changed to '1' in the new value.
                if (diff_mask & value)
                {
                        // Now we know that _some_ bits need to be erased to '1'.
                        // Check if any bits in the new value are '0'.
                        if (value != 0xff)
                        {
                                // Now we know that some bits need to be programmed to '0' also.
                                EEDR = value;                                        // Set EEPROM data register.
                                EECR = ((1 << EEMPE) | (0 << EEPM1) | (0 << EEPM0)); //Erase+Write mode.
                                EECR |= (1 << EEPE);                                 // Start Erase+Write operation.
                        }
                        else
                        {
                                // Now we know that all bits should be erased.
                                EECR = ((1 << EEMPE) | (1 << EEPM0)); // Erase-only mode.
                                EECR |= (1 << EEPE);                  // Start Erase-only operation.
                        }
                }
                else
                {
                        // Now we know that _no_ bits need to be erased to '1'.
                        // Check if any bits are changed from '1' in the old value.
                        if (diff_mask)
                        {
                                // Now we know that _some_ bits need to the programmed to '0'.
                                EEDR = value;                         // Set EEPROM data register.
                                EECR = ((1 << EEMPE) | (1 << EEPM1)); // Write-only mode.
                                EECR |= (1 << EEPE);                  // Start Write-only operation.
                        }
                }

                do
                {
                } while (EECR & (1 << EEPE)); // Wait for completion of previous write before enabling interrupts.

                sei(); // Restore interrupt flag state.
        }

        void mcu_eeprom_flush()
        {
                //do nothing
        }
#endif

#ifdef __cplusplus
}
#endif
