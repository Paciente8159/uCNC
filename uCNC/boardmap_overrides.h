#ifndef BOADMAP_OVERRIDES_H
#define BOADMAP_OVERRIDES_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "boardmap_reset.h"
#define MCU MCU_STM32H7X
#define KINEMATIC KINEMATIC_CARTESIAN
#define AXIS_COUNT 3
#define TOOL_COUNT 1
#define BAUDRATE 115200
#define BOARD_NAME "STM32 Generic H750"
#define UART_PORT 1
#define UART2_PORT 1
#define SPI_PORT 1
#define SPI2_PORT 1
#define I2C_PORT 1
#define ITP_TIMER 2
#define SERVO_TIMER 3
#define ONESHOT_TIMER 1
#define STEP0_BIT 0
#define STEP0_PORT A
#define STEP1_BIT 1
#define STEP1_PORT A
#define STEP2_BIT 2
#define STEP2_PORT A
#define STEP3_BIT 3
#define STEP3_PORT A
#define DIR0_BIT 4
#define DIR0_PORT A
#define DIR1_BIT 5
#define DIR1_PORT A
#define DIR2_BIT 6
#define DIR2_PORT A
#define DIR3_BIT 7
#define DIR3_PORT A
#define STEP0_EN_BIT 15
#define STEP0_EN_PORT A
#define PWM0_BIT 8
#define PWM0_PORT A
#define PWM0_CHANNEL 1
#define PWM0_TIMER 1
#define DOUT0_BIT 0
#define DOUT0_PORT B
#define DOUT2_BIT 4
#define DOUT2_PORT B
#define DOUT3_BIT 3
#define DOUT3_PORT B
#define DOUT31_BIT 13
#define DOUT31_PORT C
#define LIMIT_X_BIT 12
#define LIMIT_X_PORT B
#define LIMIT_Y_BIT 13
#define LIMIT_Y_PORT B
#define LIMIT_Z_BIT 14
#define LIMIT_Z_PORT B
#define LIMIT_A_BIT 15
#define LIMIT_A_PORT B
#define ANALOG0_BIT 0
#define ANALOG0_PORT A
#define ANALOG0_CHANNEL 16
#define ANALOG0_ADC 1
#define ANALOG1_CHANNEL -1
#define ANALOG1_ADC 1
#define ANALOG2_CHANNEL -1
#define ANALOG2_ADC 1
#define ANALOG3_CHANNEL -1
#define ANALOG3_ADC 1
#define ANALOG4_CHANNEL -1
#define ANALOG4_ADC 1
#define ANALOG5_CHANNEL -1
#define ANALOG5_ADC 1
#define ANALOG6_CHANNEL -1
#define ANALOG6_ADC 1
#define ANALOG7_CHANNEL -1
#define ANALOG7_ADC 1
#define ANALOG8_CHANNEL -1
#define ANALOG8_ADC 1
#define ANALOG9_CHANNEL -1
#define ANALOG9_ADC 1
#define ANALOG10_CHANNEL -1
#define ANALOG10_ADC 1
#define ANALOG11_CHANNEL -1
#define ANALOG11_ADC 1
#define ANALOG12_CHANNEL -1
#define ANALOG12_ADC 1
#define ANALOG13_CHANNEL -1
#define ANALOG13_ADC 1
#define ANALOG14_CHANNEL -1
#define ANALOG14_ADC 1
#define ANALOG15_CHANNEL -1
#define ANALOG15_ADC 1
#define DIN19_BIT 4
#define DIN19_PORT D
#define DIN19_PULLUP
#define TX_BIT 9
#define TX_PORT A
#define RX_BIT 10
#define RX_PORT A
#define USB_DM_BIT 11
#define USB_DM_PORT A
#define USB_DP_BIT 12
#define USB_DP_PORT A
#define SPI_CLK_BIT 5
#define SPI_CLK_PORT A
#define SPI_SDI_BIT 6
#define SPI_SDI_PORT A
#define SPI_SDO_BIT 7
#define SPI_SDO_PORT A
#define SPI_CS_BIT 4
#define SPI_CS_PORT A
#define I2C_CLK_BIT 8
#define I2C_CLK_PORT B
#define I2C_DATA_BIT 9
#define I2C_DATA_PORT B
#define SHIFT_REGISTER_DELAY_CYCLES 50
#define IC74HC595_COUNT 0
#define IC74HC165_COUNT 0
//Custom configurations
#define RX_PULLUP 


#ifdef __cplusplus
}
#endif
#endif
