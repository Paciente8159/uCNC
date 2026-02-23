#ifndef BOADMAP_OVERRIDES_H
#define BOADMAP_OVERRIDES_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "boardmap_reset.h"
#define MCU MCU_AVR
#define BOARD "src/hal/boards/avr/boardmap_ramps14.h"
#define KINEMATIC KINEMATIC_CARTESIAN
#define AXIS_COUNT 3
#define BAUDRATE 115200
#define BOARD_NAME "Mega-L"
#define PCINT0_PORT B
#define PCINT1_PORT J
#define PCINT2_PORT K
#define UART_PORT 3
#define ITP_TIMER 1
#define RTC_TIMER 0
#define ONESHOT_TIMER 4
#define IC74HC595_COUNT 0
#define IC74HC165_COUNT 0
#define STEP0_BIT 2
#define STEP0_PORT A
#define STEP1_BIT 3
#define STEP1_PORT A
#define STEP2_BIT 4
#define STEP2_PORT A
#define DIR0_BIT 7
#define DIR0_PORT C
#define DIR1_BIT 6
#define DIR1_PORT C
#define DIR2_BIT 5
#define DIR2_PORT C
#define STEP0_EN_BIT 7
#define STEP0_EN_PORT B
#define STEP1_EN_BIT 7
#define STEP1_EN_PORT B
#define STEP2_EN_BIT 7
#define STEP2_EN_PORT B
#define PWM0_BIT 4
#define PWM0_PORT H
#define PWM0_CHANNEL B
#define PWM0_TIMER 4
#define DOUT2_BIT 5
#define DOUT2_PORT H
#define DOUT31_BIT 7
#define DOUT31_PORT B
#define LIMIT_X_BIT 4
#define LIMIT_X_PORT B
#define LIMIT_X_PULLUP
#define LIMIT_X_ISR 0
#define LIMIT_Y_PULLUP
#define LIMIT_Z_BIT 6
#define LIMIT_Z_PORT B
#define LIMIT_Z_PULLUP
#define LIMIT_Z_ISR 0
#define PROBE_BIT 7
#define PROBE_PORT K
#define PROBE_PULLUP
#define PROBE_ISR 2
#define TX_BIT 1
#define TX_PORT J
#define RX_BIT 0
#define RX_PORT J
#define RX_PULLUP
#define TOOL_COUNT 1
//Custom configurations
#define BOARDMAP_RAMPS14_H true
#define AVR6 true
#define SPI_FREQ 100000UL
#define F_CPU 16000000UL


#ifdef __cplusplus
}
#endif
#endif
