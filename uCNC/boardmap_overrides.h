#ifndef BOADMAP_OVERRIDES_H
#define BOADMAP_OVERRIDES_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "boardmap_reset.h"
#define MCU MCU_RP2040
#define KINEMATIC KINEMATIC_CARTESIAN
#define AXIS_COUNT 2
#define TOOL_COUNT 1
#define BAUDRATE 115200
#define BOARD_NAME "MantisMan-V2"
#define UART_PORT 0
#define UART2_PORT 1
#define STEP0_BIT 11
#define STEP1_BIT 6
#define DIR0_BIT 10
#define DIR1_BIT 5
#define STEP0_EN_BIT 12
#define STEP1_EN_BIT 7
#define PWM0_BIT 17
#define PWM1_BIT 18
#define PWM2_BIT 20
#define PWM3_BIT 21
#define PWM4_BIT 23
#define PWM5_BIT 29
#define DOUT0_BIT 24
#define DOUT1_BIT 8
#define DOUT2_BIT 15
#define LIMIT_X_BIT 4
#define PROBE_BIT 16
#define ANALOG0_BIT 26
#define ANALOG1_BIT 27
#define DIN0_BIT 22
#define DIN1_BIT 9
#define TX_BIT 0
#define RX_BIT 1
//Custom configurations


#ifdef __cplusplus
}
#endif
#endif
