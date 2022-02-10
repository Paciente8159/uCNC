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

#include "../../../cnc.h"

#if (MCU == MCU_AVR)

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

// define the mcu internal servo variables
#if SERVOS_MASK > 0
static uint8_t mcu_servos[8];
#endif

static FORCEINLINE void mcu_clear_servos()
{
        // disables the interrupt of OCIEB (leaves only OCIEA)
        RTC_TIMSK = (1 << RTC_OCIEA);
        RTC_TIFR = (1 << 2);
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
#if SERVO6 >= 0
        mcu_clear_output(SERVO6);
#endif
#if SERVO7 >= 0
        mcu_clear_output(SERVO7);
#endif;
}

// gets the mcu running time in ms
static volatile uint32_t mcu_runtime_ms;
ISR(RTC_COMPA_vect, ISR_BLOCK)
{
#if SERVOS_MASK > 0
        static uint8_t ms_servo_counter = 0;
        uint8_t servo_mux = ms_servo_counter;

        // counts to 20 and reloads
        // every even millisecond sets output (will be active at least 1ms)
        if (!(servo_mux & 0x01))
        {
                mcu_clear_servos();
                switch (servo_mux >> 1)
                {
#if SERVO0 >= 0
                case 0:
                        mcu_set_output(SERVO0);
                        break;
#endif
#if SERVO1 >= 0
                case 1:
                        mcu_set_output(SERVO1);
                        break;
#endif
#if SERVO2 >= 0
                case 2:
                        mcu_set_output(SERVO2);
                        break;
#endif
#if SERVO3 >= 0
                case 3:
                        mcu_set_output(SERVO3);
                        break;
#endif
#if SERVO4 >= 0
                case 4:
                        mcu_set_output(SERVO4);
                        break;
#endif
#if SERVO5 >= 0
                case 5:
                        mcu_set_output(SERVO5);
                        break;
#endif
#if SERVO6 >= 0
                case 6:
                        mcu_set_output(SERVO6);
                        break;
#endif
#if SERVO7 >= 0
                case 7:
                        mcu_set_output(SERVO7);
                        break;
#endif
                }
        }
        else // every odd millisecond loads OCRB and enables interrupt
        {
                RTC_OCRB = mcu_servos[(servo_mux >> 1)];
                if (RTC_OCRB)
                {
                        if (RTC_OCRB != RTC_OCRA)
                        {
                                RTC_TIFR = 7;
                                RTC_TIMSK |= (1 << RTC_OCIEB);
                                if (RTC_OCRB < RTC_TCNT)
                                {
                                        mcu_clear_servos();
                                }
                        }
                }
                else
                {
                        mcu_clear_servos();
                }
        }
        servo_mux++;
        ms_servo_counter = (servo_mux != 20) ? servo_mux : 0;

#endif
        mcu_toggle_output(DOUT0);
        mcu_runtime_ms++;
        cnc_scheduletasks();
}

// naked ISR to reduce impact since doen't need to change any register (just an interrupt mask and pin outputs)
ISR(RTC_COMPB_vect, ISR_NAKED)
{
        mcu_clear_servos();
        reti();
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
#if (PCINTA_DIN_IO_MASK == 1)
        io_inputs_isr();
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
#if (PCINTA_DIN_IO_MASK == 4)
        io_inputs_isr();
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
#if (PCINTA_DIN_IO_MASK == 16)
        io_inputs_isr();
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
#if (PCINTA_DIN_IO_MASK == 64)
        io_inputs_isr();
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
#if (PCINTB_DIN_IO_MASK == 1)
        io_inputs_isr();
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
#if (PCINTB_DIN_IO_MASK == 4)
        io_inputs_isr();
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
#if (PCINTB_DIN_IO_MASK == 16)
        io_inputs_isr();
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
#if (PCINTB_DIN_IO_MASK == 64)
        io_inputs_isr();
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

#if (PCINT0_DIN_IO_MASK != 0)
        if (diff & PCINT0_DIN_IO_MASK)
        {
                io_inputs_isr();
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

#if (PCINT1_DIN_IO_MASK != 0)
        if (diff & PCINT1_DIN_IO_MASK)
        {
                io_inputs_isr();
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

#if (PCINT2_DIN_IO_MASK != 0)
        if (diff & PCINT2_DIN_IO_MASK)
        {
                io_inputs_isr();
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
        // disable WDT
        wdt_reset();
        MCUSR &= ~(1 << WDRF);
        WDTCSR |= (1 << WDCE) | (1 << WDE);
        WDTCSR = 0x00;

        // configure all pins
        // autogenerated
#if STEP0 >= 0
        mcu_config_output(STEP0);
#endif
#if STEP1 >= 0
        mcu_config_output(STEP1);
#endif
#if STEP2 >= 0
        mcu_config_output(STEP2);
#endif
#if STEP3 >= 0
        mcu_config_output(STEP3);
#endif
#if STEP4 >= 0
        mcu_config_output(STEP4);
#endif
#if STEP5 >= 0
        mcu_config_output(STEP5);
#endif
#if STEP6 >= 0
        mcu_config_output(STEP6);
#endif
#if STEP7 >= 0
        mcu_config_output(STEP7);
#endif
#if DIR0 >= 0
        mcu_config_output(DIR0);
#endif
#if DIR1 >= 0
        mcu_config_output(DIR1);
#endif
#if DIR2 >= 0
        mcu_config_output(DIR2);
#endif
#if DIR3 >= 0
        mcu_config_output(DIR3);
#endif
#if DIR4 >= 0
        mcu_config_output(DIR4);
#endif
#if DIR5 >= 0
        mcu_config_output(DIR5);
#endif
#if STEP0_EN >= 0
        mcu_config_output(STEP0_EN);
#endif
#if STEP1_EN >= 0
        mcu_config_output(STEP1_EN);
#endif
#if STEP2_EN >= 0
        mcu_config_output(STEP2_EN);
#endif
#if STEP3_EN >= 0
        mcu_config_output(STEP3_EN);
#endif
#if STEP4_EN >= 0
        mcu_config_output(STEP4_EN);
#endif
#if STEP5_EN >= 0
        mcu_config_output(STEP5_EN);
#endif
#if PWM0 >= 0
        mcu_config_pwm(PWM0);
#endif
#if PWM1 >= 0
        mcu_config_pwm(PWM1);
#endif
#if PWM2 >= 0
        mcu_config_pwm(PWM2);
#endif
#if PWM3 >= 0
        mcu_config_pwm(PWM3);
#endif
#if PWM4 >= 0
        mcu_config_pwm(PWM4);
#endif
#if PWM5 >= 0
        mcu_config_pwm(PWM5);
#endif
#if PWM6 >= 0
        mcu_config_pwm(PWM6);
#endif
#if PWM7 >= 0
        mcu_config_pwm(PWM7);
#endif
#if PWM8 >= 0
        mcu_config_pwm(PWM8);
#endif
#if PWM9 >= 0
        mcu_config_pwm(PWM9);
#endif
#if PWM10 >= 0
        mcu_config_pwm(PWM10);
#endif
#if PWM11 >= 0
        mcu_config_pwm(PWM11);
#endif
#if PWM12 >= 0
        mcu_config_pwm(PWM12);
#endif
#if PWM13 >= 0
        mcu_config_pwm(PWM13);
#endif
#if PWM14 >= 0
        mcu_config_pwm(PWM14);
#endif
#if PWM15 >= 0
        mcu_config_pwm(PWM15);
#endif
#if DOUT0 >= 0
        mcu_config_output(DOUT0);
#endif
#if DOUT1 >= 0
        mcu_config_output(DOUT1);
#endif
#if DOUT2 >= 0
        mcu_config_output(DOUT2);
#endif
#if DOUT3 >= 0
        mcu_config_output(DOUT3);
#endif
#if DOUT4 >= 0
        mcu_config_output(DOUT4);
#endif
#if DOUT5 >= 0
        mcu_config_output(DOUT5);
#endif
#if DOUT6 >= 0
        mcu_config_output(DOUT6);
#endif
#if DOUT7 >= 0
        mcu_config_output(DOUT7);
#endif
#if DOUT8 >= 0
        mcu_config_output(DOUT8);
#endif
#if DOUT9 >= 0
        mcu_config_output(DOUT9);
#endif
#if DOUT10 >= 0
        mcu_config_output(DOUT10);
#endif
#if DOUT11 >= 0
        mcu_config_output(DOUT11);
#endif
#if DOUT12 >= 0
        mcu_config_output(DOUT12);
#endif
#if DOUT13 >= 0
        mcu_config_output(DOUT13);
#endif
#if DOUT14 >= 0
        mcu_config_output(DOUT14);
#endif
#if DOUT15 >= 0
        mcu_config_output(DOUT15);
#endif
#if SERVO0 >= 0
        mcu_config_output(SERVO0);
#endif
#if SERVO1 >= 0
        mcu_config_output(SERVO1);
#endif
#if SERVO2 >= 0
        mcu_config_output(SERVO2);
#endif
#if SERVO3 >= 0
        mcu_config_output(SERVO3);
#endif
#if SERVO4 >= 0
        mcu_config_output(SERVO4);
#endif
#if SERVO5 >= 0
        mcu_config_output(SERVO5);
#endif
#if SERVO6 >= 0
        mcu_config_output(SERVO6);
#endif
#if SERVO7 >= 0
        mcu_config_output(SERVO7);
#endif
#if TX >= 0
        mcu_config_output(TX);
#endif

#if LIMIT_X >= 0
        mcu_config_input(LIMIT_X);
#ifdef LIMIT_X_PULLUP
        mcu_config_pullup(LIMIT_X);
#endif
#ifdef LIMIT_X_ISR
        mcu_config_input_isr(LIMIT_X);
#endif
#endif
#if LIMIT_Y >= 0
        mcu_config_input(LIMIT_Y);
#ifdef LIMIT_Y_PULLUP
        mcu_config_pullup(LIMIT_Y);
#endif
#ifdef LIMIT_Y_ISR
        mcu_config_input_isr(LIMIT_Y);
#endif
#endif
#if LIMIT_Z >= 0
        mcu_config_input(LIMIT_Z);
#ifdef LIMIT_Z_PULLUP
        mcu_config_pullup(LIMIT_Z);
#endif
#ifdef LIMIT_Z_ISR
        mcu_config_input_isr(LIMIT_Z);
#endif
#endif
#if LIMIT_X2 >= 0
        mcu_config_input(LIMIT_X2);
#ifdef LIMIT_X2_PULLUP
        mcu_config_pullup(LIMIT_X2);
#endif
#ifdef LIMIT_X2_ISR
        mcu_config_input_isr(LIMIT_X2);
#endif
#endif
#if LIMIT_Y2 >= 0
        mcu_config_input(LIMIT_Y2);
#ifdef LIMIT_Y2_PULLUP
        mcu_config_pullup(LIMIT_Y2);
#endif
#ifdef LIMIT_Y2_ISR
        mcu_config_input_isr(LIMIT_Y2);
#endif
#endif
#if LIMIT_Z2 >= 0
        mcu_config_input(LIMIT_Z2);
#ifdef LIMIT_Z2_PULLUP
        mcu_config_pullup(LIMIT_Z2);
#endif
#ifdef LIMIT_Z2_ISR
        mcu_config_input_isr(LIMIT_Z2);
#endif
#endif
#if LIMIT_A >= 0
        mcu_config_input(LIMIT_A);
#ifdef LIMIT_A_PULLUP
        mcu_config_pullup(LIMIT_A);
#endif
#ifdef LIMIT_A_ISR
        mcu_config_input_isr(LIMIT_A);
#endif
#endif
#if LIMIT_B >= 0
        mcu_config_input(LIMIT_B);
#ifdef LIMIT_B_PULLUP
        mcu_config_pullup(LIMIT_B);
#endif
#ifdef LIMIT_B_ISR
        mcu_config_input_isr(LIMIT_B);
#endif
#endif
#if LIMIT_C >= 0
        mcu_config_input(LIMIT_C);
#ifdef LIMIT_C_PULLUP
        mcu_config_pullup(LIMIT_C);
#endif
#ifdef LIMIT_C_ISR
        mcu_config_input_isr(LIMIT_C);
#endif
#endif
#if PROBE >= 0
        mcu_config_input(PROBE);
#ifdef PROBE_PULLUP
        mcu_config_pullup(PROBE);
#endif
#ifdef PROBE_ISR
        mcu_config_input_isr(PROBE);
#endif
#endif
#if ESTOP >= 0
        mcu_config_input(ESTOP);
#ifdef ESTOP_PULLUP
        mcu_config_pullup(ESTOP);
#endif
#ifdef ESTOP_ISR
        mcu_config_input_isr(ESTOP);
#endif
#endif
#if SAFETY_DOOR >= 0
        mcu_config_input(SAFETY_DOOR);
#ifdef SAFETY_DOOR_PULLUP
        mcu_config_pullup(SAFETY_DOOR);
#endif
#ifdef SAFETY_DOOR_ISR
        mcu_config_input_isr(SAFETY_DOOR);
#endif
#endif
#if FHOLD >= 0
        mcu_config_input(FHOLD);
#ifdef FHOLD_PULLUP
        mcu_config_pullup(FHOLD);
#endif
#ifdef FHOLD_ISR
        mcu_config_input_isr(FHOLD);
#endif
#endif
#if CS_RES >= 0
        mcu_config_input(CS_RES);
#ifdef CS_RES_PULLUP
        mcu_config_pullup(CS_RES);
#endif
#ifdef CS_RES_ISR
        mcu_config_input_isr(CS_RES);
#endif
#endif
#if ANALOG0 >= 0
        mcu_config_input(ANALOG0);
#endif
#if ANALOG1 >= 0
        mcu_config_input(ANALOG1);
#endif
#if ANALOG2 >= 0
        mcu_config_input(ANALOG2);
#endif
#if ANALOG3 >= 0
        mcu_config_input(ANALOG3);
#endif
#if ANALOG4 >= 0
        mcu_config_input(ANALOG4);
#endif
#if ANALOG5 >= 0
        mcu_config_input(ANALOG5);
#endif
#if ANALOG6 >= 0
        mcu_config_input(ANALOG6);
#endif
#if ANALOG7 >= 0
        mcu_config_input(ANALOG7);
#endif
#if ANALOG8 >= 0
        mcu_config_input(ANALOG8);
#endif
#if ANALOG9 >= 0
        mcu_config_input(ANALOG9);
#endif
#if ANALOG10 >= 0
        mcu_config_input(ANALOG10);
#endif
#if ANALOG11 >= 0
        mcu_config_input(ANALOG11);
#endif
#if ANALOG12 >= 0
        mcu_config_input(ANALOG12);
#endif
#if ANALOG13 >= 0
        mcu_config_input(ANALOG13);
#endif
#if ANALOG14 >= 0
        mcu_config_input(ANALOG14);
#endif
#if ANALOG15 >= 0
        mcu_config_input(ANALOG15);
#endif
#if DIN0 >= 0
        mcu_config_input(DIN0);
#ifdef DIN0_PULLUP
        mcu_config_pullup(DIN0);
#endif
#ifdef DIN0_ISR
        mcu_config_input_isr(DIN0);
#endif
#endif
#if DIN1 >= 0
        mcu_config_input(DIN1);
#ifdef DIN1_PULLUP
        mcu_config_pullup(DIN1);
#endif
#ifdef DIN1_ISR
        mcu_config_input_isr(DIN1);
#endif
#endif
#if DIN2 >= 0
        mcu_config_input(DIN2);
#ifdef DIN2_PULLUP
        mcu_config_pullup(DIN2);
#endif
#ifdef DIN2_ISR
        mcu_config_input_isr(DIN2);
#endif
#endif
#if DIN3 >= 0
        mcu_config_input(DIN3);
#ifdef DIN3_PULLUP
        mcu_config_pullup(DIN3);
#endif
#ifdef DIN3_ISR
        mcu_config_input_isr(DIN3);
#endif
#endif
#if DIN4 >= 0
        mcu_config_input(DIN4);
#ifdef DIN4_PULLUP
        mcu_config_pullup(DIN4);
#endif
#ifdef DIN4_ISR
        mcu_config_input_isr(DIN4);
#endif
#endif
#if DIN5 >= 0
        mcu_config_input(DIN5);
#ifdef DIN5_PULLUP
        mcu_config_pullup(DIN5);
#endif
#ifdef DIN5_ISR
        mcu_config_input_isr(DIN5);
#endif
#endif
#if DIN6 >= 0
        mcu_config_input(DIN6);
#ifdef DIN6_PULLUP
        mcu_config_pullup(DIN6);
#endif
#ifdef DIN6_ISR
        mcu_config_input_isr(DIN6);
#endif
#endif
#if DIN7 >= 0
        mcu_config_input(DIN7);
#ifdef DIN7_PULLUP
        mcu_config_pullup(DIN7);
#endif
#ifdef DIN7_ISR
        mcu_config_input_isr(DIN7);
#endif
#endif
#if DIN8 >= 0
        mcu_config_input(DIN8);
#ifdef DIN8_PULLUP
        mcu_config_pullup(DIN8);
#endif
#endif
#if DIN9 >= 0
        mcu_config_input(DIN9);
#ifdef DIN9_PULLUP
        mcu_config_pullup(DIN9);
#endif
#endif
#if DIN10 >= 0
        mcu_config_input(DIN10);
#ifdef DIN10_PULLUP
        mcu_config_pullup(DIN10);
#endif
#endif
#if DIN11 >= 0
        mcu_config_input(DIN11);
#ifdef DIN11_PULLUP
        mcu_config_pullup(DIN11);
#endif
#endif
#if DIN12 >= 0
        mcu_config_input(DIN12);
#ifdef DIN12_PULLUP
        mcu_config_pullup(DIN12);
#endif
#endif
#if DIN13 >= 0
        mcu_config_input(DIN13);
#ifdef DIN13_PULLUP
        mcu_config_pullup(DIN13);
#endif
#endif
#if DIN14 >= 0
        mcu_config_input(DIN14);
#ifdef DIN14_PULLUP
        mcu_config_pullup(DIN14);
#endif
#endif
#if DIN15 >= 0
        mcu_config_input(DIN15);
#ifdef DIN15_PULLUP
        mcu_config_pullup(DIN15);
#endif
#endif
#if RX >= 0
        mcu_config_output(RX);
#endif

        // Set COM port
        //  Set baud rate
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

// enable interrupts on pin changes
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

        // disable probe isr
        mcu_disable_probe_isr();
        // enable interrupts
        mcu_enable_global_isr();
}

// IO functions
void mcu_set_servo(uint8_t servo, uint8_t value)
{
#if SERVOS_MASK > 0
        uint8_t scaled = (uint8_t)(((uint16_t)(value * RTC_OCRA)) >> 8);
        mcu_servos[servo - SERVO0_UCNC_INTERNAL_PIN] = scaled;
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

void mcu_putc(char c)
{
#ifdef ENABLE_SYNC_TX
        loop_until_bit_is_set(UCSRA, UDRE);
#endif
        COM_OUTREG = c;
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

// RealTime
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
        // stops timer
        ITP_TCCRB = 0;
        // CTC mode
        ITP_TCCRA = 0;
        // resets counter
        ITP_TCNT = 0;
        // set step clock
        ITP_OCRA = clocks_speed;
        // sets OCR0B to half
        // this will allways fire step_reset between pulses
        ITP_OCRB = ITP_OCRA >> 1;
        // clears interrupt flags by writing 1's
        ITP_TIFR = 7;
        // enable timer interrupts on both match registers
        ITP_TIMSK |= (1 << ITP_OCIEB) | (1 << ITP_OCIEA);

        // start timer in CTC mode with the correct prescaler
        ITP_TCCRB = (uint8_t)prescaller;
}

// se implementar amass deixo de necessitar de prescaler
void mcu_change_itp_isr(uint16_t clocks_speed, uint16_t prescaller)
{
        // stops timer
        // ITP_TCCRB = 0;
        ITP_OCRB = clocks_speed >> 1;
        ITP_OCRA = clocks_speed;
        // sets OCR0B to half
        // this will allways fire step_reset between pulses

        // reset timer
        // ITP_TCNT = 0;
        // start timer in CTC mode with the correct prescaler
        ITP_TCCRB = (uint8_t)prescaller;
}

void mcu_stop_itp_isr(void)
{
        ITP_TCCRB = 0;
        ITP_TIMSK &= ~((1 << ITP_OCIEB) | (1 << ITP_OCIEA));
}

// gets the mcu running time in ms
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
        // stops timer
        RTC_TCCRB = 0;
        RTC_TCCRA = 0;
        // resets counter
        RTC_TCNT = 0;
        // set step clock
        RTC_OCRA = clocks;
        // CTC mode
        RTC_TCCRA |= 2;
        // clears interrupt flags by writing 1's
        RTC_TIFR = 7;
        // enable timer interrupts on both match registers
        RTC_TIMSK |= (1 << RTC_OCIEA);
// start timer in CTC mode with the correct prescaler
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
        // read any char that is received
        while (CHECKBIT(UCSRA, RXC))
        {
                unsigned char c = mcu_getc();
                serial_rx_isr(c);
        }
#endif
}

// This was copied from grbl
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
                        EECR = ((1 << EEMPE) | (0 << EEPM1) | (0 << EEPM0)); // Erase+Write mode.
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
        // do nothing
}
#endif
