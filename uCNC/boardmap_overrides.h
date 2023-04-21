#ifndef BOADMAP_OVERRIDES_H
#define BOADMAP_OVERRIDES_H
#ifdef __cplusplus
extern "C"
{
#endif
#ifdef MCU
#undef MCU
#endif
#define MCU MCU_AVR
#ifdef KINEMATIC
#undef KINEMATIC
#endif
#define KINEMATIC KINEMATIC_CARTESIAN
#ifdef AXIS_COUNT
#undef AXIS_COUNT
#endif
#define AXIS_COUNT 3
#ifdef TOOL_COUNT
#undef TOOL_COUNT
#endif
#define TOOL_COUNT 1
#ifdef BAUDRATE
#undef BAUDRATE
#endif
#define BAUDRATE 115200
#ifdef BOARD
#undef BOARD
#endif
#define BOARD BOARD_RAMPS14
#ifdef BOARD_NAME
#undef BOARD_NAME
#endif
#define BOARD_NAME "RAMPS 1.4"
#ifdef PCINT0_PORT
#undef PCINT0_PORT
#endif
#define PCINT0_PORT B
#ifdef PCINT1_PORT
#undef PCINT1_PORT
#endif
#define PCINT1_PORT J
#ifdef PCINT2_PORT
#undef PCINT2_PORT
#endif
#define PCINT2_PORT K
#ifdef ITP_TIMER
#undef ITP_TIMER
#endif
#define ITP_TIMER 1
#ifdef RTC_TIMER
#undef RTC_TIMER
#endif
#define RTC_TIMER 0
#ifdef SERVO_TIMER
#undef SERVO_TIMER
#endif
#ifdef ONESHOT_TIMER
#undef ONESHOT_TIMER
#endif
#define ONESHOT_TIMER 4
#ifdef STEP0_BIT
#undef STEP0_BIT
#endif
#define STEP0_BIT 0
#ifdef STEP0_PORT
#undef STEP0_PORT
#endif
#define STEP0_PORT F
#ifdef STEP1_BIT
#undef STEP1_BIT
#endif
#define STEP1_BIT 6
#ifdef STEP1_PORT
#undef STEP1_PORT
#endif
#define STEP1_PORT F
#ifdef STEP2_BIT
#undef STEP2_BIT
#endif
#define STEP2_BIT 3
#ifdef STEP2_PORT
#undef STEP2_PORT
#endif
#define STEP2_PORT L
#ifdef STEP3_BIT
#undef STEP3_BIT
#endif
#define STEP3_BIT 4
#ifdef STEP3_PORT
#undef STEP3_PORT
#endif
#define STEP3_PORT A
#ifdef STEP4_BIT
#undef STEP4_BIT
#endif
#define STEP4_BIT 1
#ifdef STEP4_PORT
#undef STEP4_PORT
#endif
#define STEP4_PORT C
#ifdef STEP5_BIT
#undef STEP5_BIT
#endif
#ifdef STEP5_PORT
#undef STEP5_PORT
#endif
#ifdef STEP6_BIT
#undef STEP6_BIT
#endif
#ifdef STEP6_PORT
#undef STEP6_PORT
#endif
#ifdef STEP7_BIT
#undef STEP7_BIT
#endif
#ifdef STEP7_PORT
#undef STEP7_PORT
#endif
#ifdef DIR0_BIT
#undef DIR0_BIT
#endif
#define DIR0_BIT 1
#ifdef DIR0_PORT
#undef DIR0_PORT
#endif
#define DIR0_PORT F
#ifdef DIR1_BIT
#undef DIR1_BIT
#endif
#define DIR1_BIT 7
#ifdef DIR1_PORT
#undef DIR1_PORT
#endif
#define DIR1_PORT F
#ifdef DIR2_BIT
#undef DIR2_BIT
#endif
#define DIR2_BIT 1
#ifdef DIR2_PORT
#undef DIR2_PORT
#endif
#define DIR2_PORT L
#ifdef DIR3_BIT
#undef DIR3_BIT
#endif
#define DIR3_BIT 6
#ifdef DIR3_PORT
#undef DIR3_PORT
#endif
#define DIR3_PORT A
#ifdef DIR4_BIT
#undef DIR4_BIT
#endif
#define DIR4_BIT 3
#ifdef DIR4_PORT
#undef DIR4_PORT
#endif
#define DIR4_PORT C
#ifdef DIR5_BIT
#undef DIR5_BIT
#endif
#ifdef DIR5_PORT
#undef DIR5_PORT
#endif
#ifdef DIR6_BIT
#undef DIR6_BIT
#endif
#ifdef DIR6_PORT
#undef DIR6_PORT
#endif
#ifdef DIR7_BIT
#undef DIR7_BIT
#endif
#ifdef DIR7_PORT
#undef DIR7_PORT
#endif
#ifdef STEP0_EN_BIT
#undef STEP0_EN_BIT
#endif
#define STEP0_EN_BIT 7
#ifdef STEP0_EN_PORT
#undef STEP0_EN_PORT
#endif
#define STEP0_EN_PORT D
#ifdef STEP1_EN_BIT
#undef STEP1_EN_BIT
#endif
#define STEP1_EN_BIT 2
#ifdef STEP1_EN_PORT
#undef STEP1_EN_PORT
#endif
#define STEP1_EN_PORT F
#ifdef STEP2_EN_BIT
#undef STEP2_EN_BIT
#endif
#define STEP2_EN_BIT 0
#ifdef STEP2_EN_PORT
#undef STEP2_EN_PORT
#endif
#define STEP2_EN_PORT K
#ifdef STEP3_EN_BIT
#undef STEP3_EN_BIT
#endif
#define STEP3_EN_BIT 2
#ifdef STEP3_EN_PORT
#undef STEP3_EN_PORT
#endif
#define STEP3_EN_PORT A
#ifdef STEP4_EN_BIT
#undef STEP4_EN_BIT
#endif
#define STEP4_EN_BIT 7
#ifdef STEP4_EN_PORT
#undef STEP4_EN_PORT
#endif
#define STEP4_EN_PORT C
#ifdef STEP5_EN_BIT
#undef STEP5_EN_BIT
#endif
#ifdef STEP5_EN_PORT
#undef STEP5_EN_PORT
#endif
#ifdef STEP6_EN_BIT
#undef STEP6_EN_BIT
#endif
#ifdef STEP6_EN_PORT
#undef STEP6_EN_PORT
#endif
#ifdef STEP7_EN_BIT
#undef STEP7_EN_BIT
#endif
#ifdef STEP7_EN_PORT
#undef STEP7_EN_PORT
#endif
#ifdef PWM0_BIT
#undef PWM0_BIT
#endif
#define PWM0_BIT 5
#ifdef PWM0_PORT
#undef PWM0_PORT
#endif
#define PWM0_PORT H
#ifdef PWM0_CHANNEL
#undef PWM0_CHANNEL
#endif
#define PWM0_CHANNEL C
#ifdef PWM0_TIMER
#undef PWM0_TIMER
#endif
#define PWM0_TIMER 4
#ifdef PWM0_MUX
#undef PWM0_MUX
#endif
#ifdef PWM1_BIT
#undef PWM1_BIT
#endif
#define PWM1_BIT 6
#ifdef PWM1_PORT
#undef PWM1_PORT
#endif
#define PWM1_PORT H
#ifdef PWM1_CHANNEL
#undef PWM1_CHANNEL
#endif
#define PWM1_CHANNEL B
#ifdef PWM1_TIMER
#undef PWM1_TIMER
#endif
#define PWM1_TIMER 2
#ifdef PWM1_MUX
#undef PWM1_MUX
#endif
#ifdef PWM2_BIT
#undef PWM2_BIT
#endif
#define PWM2_BIT 4
#ifdef PWM2_PORT
#undef PWM2_PORT
#endif
#define PWM2_PORT B
#ifdef PWM2_CHANNEL
#undef PWM2_CHANNEL
#endif
#define PWM2_CHANNEL A
#ifdef PWM2_TIMER
#undef PWM2_TIMER
#endif
#define PWM2_TIMER 2
#ifdef PWM2_MUX
#undef PWM2_MUX
#endif
#ifdef PWM3_BIT
#undef PWM3_BIT
#endif
#ifdef PWM3_PORT
#undef PWM3_PORT
#endif
#ifdef PWM3_CHANNEL
#undef PWM3_CHANNEL
#endif
#ifdef PWM3_TIMER
#undef PWM3_TIMER
#endif
#ifdef PWM3_MUX
#undef PWM3_MUX
#endif
#ifdef PWM4_BIT
#undef PWM4_BIT
#endif
#ifdef PWM4_PORT
#undef PWM4_PORT
#endif
#ifdef PWM4_CHANNEL
#undef PWM4_CHANNEL
#endif
#ifdef PWM4_TIMER
#undef PWM4_TIMER
#endif
#ifdef PWM4_MUX
#undef PWM4_MUX
#endif
#ifdef PWM5_BIT
#undef PWM5_BIT
#endif
#ifdef PWM5_PORT
#undef PWM5_PORT
#endif
#ifdef PWM5_CHANNEL
#undef PWM5_CHANNEL
#endif
#ifdef PWM5_TIMER
#undef PWM5_TIMER
#endif
#ifdef PWM5_MUX
#undef PWM5_MUX
#endif
#ifdef PWM6_BIT
#undef PWM6_BIT
#endif
#ifdef PWM6_PORT
#undef PWM6_PORT
#endif
#ifdef PWM6_CHANNEL
#undef PWM6_CHANNEL
#endif
#ifdef PWM6_TIMER
#undef PWM6_TIMER
#endif
#ifdef PWM6_MUX
#undef PWM6_MUX
#endif
#ifdef PWM7_BIT
#undef PWM7_BIT
#endif
#ifdef PWM7_PORT
#undef PWM7_PORT
#endif
#ifdef PWM7_CHANNEL
#undef PWM7_CHANNEL
#endif
#ifdef PWM7_TIMER
#undef PWM7_TIMER
#endif
#ifdef PWM7_MUX
#undef PWM7_MUX
#endif
#ifdef PWM8_BIT
#undef PWM8_BIT
#endif
#ifdef PWM8_PORT
#undef PWM8_PORT
#endif
#ifdef PWM8_CHANNEL
#undef PWM8_CHANNEL
#endif
#ifdef PWM8_TIMER
#undef PWM8_TIMER
#endif
#ifdef PWM8_MUX
#undef PWM8_MUX
#endif
#ifdef PWM9_BIT
#undef PWM9_BIT
#endif
#ifdef PWM9_PORT
#undef PWM9_PORT
#endif
#ifdef PWM9_CHANNEL
#undef PWM9_CHANNEL
#endif
#ifdef PWM9_TIMER
#undef PWM9_TIMER
#endif
#ifdef PWM9_MUX
#undef PWM9_MUX
#endif
#ifdef PWM10_BIT
#undef PWM10_BIT
#endif
#ifdef PWM10_PORT
#undef PWM10_PORT
#endif
#ifdef PWM10_CHANNEL
#undef PWM10_CHANNEL
#endif
#ifdef PWM10_TIMER
#undef PWM10_TIMER
#endif
#ifdef PWM10_MUX
#undef PWM10_MUX
#endif
#ifdef PWM11_BIT
#undef PWM11_BIT
#endif
#ifdef PWM11_PORT
#undef PWM11_PORT
#endif
#ifdef PWM11_CHANNEL
#undef PWM11_CHANNEL
#endif
#ifdef PWM11_TIMER
#undef PWM11_TIMER
#endif
#ifdef PWM11_MUX
#undef PWM11_MUX
#endif
#ifdef PWM12_BIT
#undef PWM12_BIT
#endif
#ifdef PWM12_PORT
#undef PWM12_PORT
#endif
#ifdef PWM12_CHANNEL
#undef PWM12_CHANNEL
#endif
#ifdef PWM12_TIMER
#undef PWM12_TIMER
#endif
#ifdef PWM12_MUX
#undef PWM12_MUX
#endif
#ifdef PWM13_BIT
#undef PWM13_BIT
#endif
#ifdef PWM13_PORT
#undef PWM13_PORT
#endif
#ifdef PWM13_CHANNEL
#undef PWM13_CHANNEL
#endif
#ifdef PWM13_TIMER
#undef PWM13_TIMER
#endif
#ifdef PWM13_MUX
#undef PWM13_MUX
#endif
#ifdef PWM14_BIT
#undef PWM14_BIT
#endif
#ifdef PWM14_PORT
#undef PWM14_PORT
#endif
#ifdef PWM14_CHANNEL
#undef PWM14_CHANNEL
#endif
#ifdef PWM14_TIMER
#undef PWM14_TIMER
#endif
#ifdef PWM14_MUX
#undef PWM14_MUX
#endif
#ifdef PWM15_BIT
#undef PWM15_BIT
#endif
#ifdef PWM15_PORT
#undef PWM15_PORT
#endif
#ifdef PWM15_CHANNEL
#undef PWM15_CHANNEL
#endif
#ifdef PWM15_TIMER
#undef PWM15_TIMER
#endif
#ifdef PWM15_MUX
#undef PWM15_MUX
#endif
#ifdef SERVO0_BIT
#undef SERVO0_BIT
#endif
#define SERVO0_BIT 5
#ifdef SERVO0_PORT
#undef SERVO0_PORT
#endif
#define SERVO0_PORT B
#ifdef SERVO1_BIT
#undef SERVO1_BIT
#endif
#define SERVO1_BIT 3
#ifdef SERVO1_PORT
#undef SERVO1_PORT
#endif
#define SERVO1_PORT H
#ifdef SERVO2_BIT
#undef SERVO2_BIT
#endif
#define SERVO2_BIT 3
#ifdef SERVO2_PORT
#undef SERVO2_PORT
#endif
#define SERVO2_PORT E
#ifdef SERVO3_BIT
#undef SERVO3_BIT
#endif
#define SERVO3_BIT 5
#ifdef SERVO3_PORT
#undef SERVO3_PORT
#endif
#define SERVO3_PORT G
#ifdef SERVO4_BIT
#undef SERVO4_BIT
#endif
#ifdef SERVO4_PORT
#undef SERVO4_PORT
#endif
#ifdef SERVO5_BIT
#undef SERVO5_BIT
#endif
#ifdef SERVO5_PORT
#undef SERVO5_PORT
#endif
#ifdef DOUT0_BIT
#undef DOUT0_BIT
#endif
#ifdef DOUT0_PORT
#undef DOUT0_PORT
#endif
#ifdef DOUT1_BIT
#undef DOUT1_BIT
#endif
#ifdef DOUT1_PORT
#undef DOUT1_PORT
#endif
#ifdef DOUT2_BIT
#undef DOUT2_BIT
#endif
#ifdef DOUT2_PORT
#undef DOUT2_PORT
#endif
#ifdef DOUT3_BIT
#undef DOUT3_BIT
#endif
#ifdef DOUT3_PORT
#undef DOUT3_PORT
#endif
#ifdef DOUT4_BIT
#undef DOUT4_BIT
#endif
#ifdef DOUT4_PORT
#undef DOUT4_PORT
#endif
#ifdef DOUT5_BIT
#undef DOUT5_BIT
#endif
#ifdef DOUT5_PORT
#undef DOUT5_PORT
#endif
#ifdef DOUT6_BIT
#undef DOUT6_BIT
#endif
#ifdef DOUT6_PORT
#undef DOUT6_PORT
#endif
#ifdef DOUT7_BIT
#undef DOUT7_BIT
#endif
#ifdef DOUT7_PORT
#undef DOUT7_PORT
#endif
#ifdef DOUT8_BIT
#undef DOUT8_BIT
#endif
#ifdef DOUT8_PORT
#undef DOUT8_PORT
#endif
#ifdef DOUT9_BIT
#undef DOUT9_BIT
#endif
#ifdef DOUT9_PORT
#undef DOUT9_PORT
#endif
#ifdef DOUT10_BIT
#undef DOUT10_BIT
#endif
#ifdef DOUT10_PORT
#undef DOUT10_PORT
#endif
#ifdef DOUT11_BIT
#undef DOUT11_BIT
#endif
#ifdef DOUT11_PORT
#undef DOUT11_PORT
#endif
#ifdef DOUT12_BIT
#undef DOUT12_BIT
#endif
#ifdef DOUT12_PORT
#undef DOUT12_PORT
#endif
#ifdef DOUT13_BIT
#undef DOUT13_BIT
#endif
#ifdef DOUT13_PORT
#undef DOUT13_PORT
#endif
#ifdef DOUT14_BIT
#undef DOUT14_BIT
#endif
#ifdef DOUT14_PORT
#undef DOUT14_PORT
#endif
#ifdef DOUT15_BIT
#undef DOUT15_BIT
#endif
#ifdef DOUT15_PORT
#undef DOUT15_PORT
#endif
#ifdef DOUT16_BIT
#undef DOUT16_BIT
#endif
#ifdef DOUT16_PORT
#undef DOUT16_PORT
#endif
#ifdef DOUT17_BIT
#undef DOUT17_BIT
#endif
#ifdef DOUT17_PORT
#undef DOUT17_PORT
#endif
#ifdef DOUT18_BIT
#undef DOUT18_BIT
#endif
#ifdef DOUT18_PORT
#undef DOUT18_PORT
#endif
#ifdef DOUT19_BIT
#undef DOUT19_BIT
#endif
#ifdef DOUT19_PORT
#undef DOUT19_PORT
#endif
#ifdef DOUT20_BIT
#undef DOUT20_BIT
#endif
#define DOUT20_BIT 1
#ifdef DOUT20_PORT
#undef DOUT20_PORT
#endif
#define DOUT20_PORT G
#ifdef DOUT21_BIT
#undef DOUT21_BIT
#endif
#define DOUT21_BIT 5
#ifdef DOUT21_PORT
#undef DOUT21_PORT
#endif
#define DOUT21_PORT F
#ifdef DOUT22_BIT
#undef DOUT22_BIT
#endif
#define DOUT22_BIT 7
#ifdef DOUT22_PORT
#undef DOUT22_PORT
#endif
#define DOUT22_PORT L
#ifdef DOUT23_BIT
#undef DOUT23_BIT
#endif
#define DOUT23_BIT 5
#ifdef DOUT23_PORT
#undef DOUT23_PORT
#endif
#define DOUT23_PORT L
#ifdef DOUT24_BIT
#undef DOUT24_BIT
#endif
#ifdef DOUT24_PORT
#undef DOUT24_PORT
#endif
#ifdef DOUT25_BIT
#undef DOUT25_BIT
#endif
#ifdef DOUT25_PORT
#undef DOUT25_PORT
#endif
#ifdef DOUT26_BIT
#undef DOUT26_BIT
#endif
#ifdef DOUT26_PORT
#undef DOUT26_PORT
#endif
#ifdef DOUT27_BIT
#undef DOUT27_BIT
#endif
#ifdef DOUT27_PORT
#undef DOUT27_PORT
#endif
#ifdef DOUT28_BIT
#undef DOUT28_BIT
#endif
#ifdef DOUT28_PORT
#undef DOUT28_PORT
#endif
#ifdef DOUT29_BIT
#undef DOUT29_BIT
#endif
#ifdef DOUT29_PORT
#undef DOUT29_PORT
#endif
#ifdef DOUT30_BIT
#undef DOUT30_BIT
#endif
#ifdef DOUT30_PORT
#undef DOUT30_PORT
#endif
#ifdef DOUT31_BIT
#undef DOUT31_BIT
#endif
#define DOUT31_BIT 7
#ifdef DOUT31_PORT
#undef DOUT31_PORT
#endif
#define DOUT31_PORT B
#ifdef LIMIT_X_BIT
#undef LIMIT_X_BIT
#endif
#define LIMIT_X_BIT 5
#ifdef LIMIT_X_PORT
#undef LIMIT_X_PORT
#endif
#define LIMIT_X_PORT E
#ifdef LIMIT_X_ISR
#undef LIMIT_X_ISR
#endif
#define LIMIT_X_ISR -6
#ifdef LIMIT_Y_BIT
#undef LIMIT_Y_BIT
#endif
#define LIMIT_Y_BIT 1
#ifdef LIMIT_Y_PORT
#undef LIMIT_Y_PORT
#endif
#define LIMIT_Y_PORT J
#ifdef LIMIT_Y_ISR
#undef LIMIT_Y_ISR
#endif
#define LIMIT_Y_ISR 1
#ifdef LIMIT_Z_BIT
#undef LIMIT_Z_BIT
#endif
#define LIMIT_Z_BIT 3
#ifdef LIMIT_Z_PORT
#undef LIMIT_Z_PORT
#endif
#define LIMIT_Z_PORT D
#ifdef LIMIT_Z_ISR
#undef LIMIT_Z_ISR
#endif
#define LIMIT_Z_ISR -4
#ifdef LIMIT_X2_BIT
#undef LIMIT_X2_BIT
#endif
#define LIMIT_X2_BIT 4
#ifdef LIMIT_X2_PORT
#undef LIMIT_X2_PORT
#endif
#define LIMIT_X2_PORT E
#ifdef LIMIT_X2_ISR
#undef LIMIT_X2_ISR
#endif
#define LIMIT_X2_ISR -5
#ifdef LIMIT_Y2_BIT
#undef LIMIT_Y2_BIT
#endif
#define LIMIT_Y2_BIT 0
#ifdef LIMIT_Y2_PORT
#undef LIMIT_Y2_PORT
#endif
#define LIMIT_Y2_PORT J
#ifdef LIMIT_Y2_ISR
#undef LIMIT_Y2_ISR
#endif
#define LIMIT_Y2_ISR 1
#ifdef LIMIT_Z2_BIT
#undef LIMIT_Z2_BIT
#endif
#ifdef LIMIT_Z2_PORT
#undef LIMIT_Z2_PORT
#endif
#ifdef LIMIT_Z2_ISR
#undef LIMIT_Z2_ISR
#endif
#ifdef LIMIT_A_BIT
#undef LIMIT_A_BIT
#endif
#ifdef LIMIT_A_PORT
#undef LIMIT_A_PORT
#endif
#ifdef LIMIT_A_ISR
#undef LIMIT_A_ISR
#endif
#ifdef LIMIT_B_BIT
#undef LIMIT_B_BIT
#endif
#ifdef LIMIT_B_PORT
#undef LIMIT_B_PORT
#endif
#ifdef LIMIT_B_ISR
#undef LIMIT_B_ISR
#endif
#ifdef LIMIT_C_BIT
#undef LIMIT_C_BIT
#endif
#ifdef LIMIT_C_PORT
#undef LIMIT_C_PORT
#endif
#ifdef LIMIT_C_ISR
#undef LIMIT_C_ISR
#endif
#ifdef PROBE_BIT
#undef PROBE_BIT
#endif
#define PROBE_BIT 2
#ifdef PROBE_PORT
#undef PROBE_PORT
#endif
#define PROBE_PORT D
#ifdef PROBE_ISR
#undef PROBE_ISR
#endif
#define PROBE_ISR -3
#ifdef ESTOP_BIT
#undef ESTOP_BIT
#endif
#ifdef ESTOP_PORT
#undef ESTOP_PORT
#endif
#ifdef ESTOP_ISR
#undef ESTOP_ISR
#endif
#ifdef SAFETY_DOOR_BIT
#undef SAFETY_DOOR_BIT
#endif
#ifdef SAFETY_DOOR_PORT
#undef SAFETY_DOOR_PORT
#endif
#ifdef SAFETY_DOOR_ISR
#undef SAFETY_DOOR_ISR
#endif
#ifdef FHOLD_BIT
#undef FHOLD_BIT
#endif
#ifdef FHOLD_PORT
#undef FHOLD_PORT
#endif
#ifdef FHOLD_ISR
#undef FHOLD_ISR
#endif
#ifdef CS_RES_BIT
#undef CS_RES_BIT
#endif
#ifdef CS_RES_PORT
#undef CS_RES_PORT
#endif
#ifdef CS_RES_ISR
#undef CS_RES_ISR
#endif
#ifdef ANALOG0_BIT
#undef ANALOG0_BIT
#endif
#ifdef ANALOG0_PORT
#undef ANALOG0_PORT
#endif
#ifdef ANALOG0_CHANNEL
#undef ANALOG0_CHANNEL
#endif
#ifdef ANALOG1_BIT
#undef ANALOG1_BIT
#endif
#ifdef ANALOG1_PORT
#undef ANALOG1_PORT
#endif
#ifdef ANALOG1_CHANNEL
#undef ANALOG1_CHANNEL
#endif
#ifdef ANALOG2_BIT
#undef ANALOG2_BIT
#endif
#ifdef ANALOG2_PORT
#undef ANALOG2_PORT
#endif
#ifdef ANALOG2_CHANNEL
#undef ANALOG2_CHANNEL
#endif
#ifdef ANALOG3_BIT
#undef ANALOG3_BIT
#endif
#ifdef ANALOG3_PORT
#undef ANALOG3_PORT
#endif
#ifdef ANALOG3_CHANNEL
#undef ANALOG3_CHANNEL
#endif
#ifdef ANALOG4_BIT
#undef ANALOG4_BIT
#endif
#ifdef ANALOG4_PORT
#undef ANALOG4_PORT
#endif
#ifdef ANALOG4_CHANNEL
#undef ANALOG4_CHANNEL
#endif
#ifdef ANALOG5_BIT
#undef ANALOG5_BIT
#endif
#ifdef ANALOG5_PORT
#undef ANALOG5_PORT
#endif
#ifdef ANALOG5_CHANNEL
#undef ANALOG5_CHANNEL
#endif
#ifdef ANALOG6_BIT
#undef ANALOG6_BIT
#endif
#ifdef ANALOG6_PORT
#undef ANALOG6_PORT
#endif
#ifdef ANALOG6_CHANNEL
#undef ANALOG6_CHANNEL
#endif
#ifdef ANALOG7_BIT
#undef ANALOG7_BIT
#endif
#ifdef ANALOG7_PORT
#undef ANALOG7_PORT
#endif
#ifdef ANALOG7_CHANNEL
#undef ANALOG7_CHANNEL
#endif
#ifdef ANALOG8_BIT
#undef ANALOG8_BIT
#endif
#ifdef ANALOG8_PORT
#undef ANALOG8_PORT
#endif
#ifdef ANALOG8_CHANNEL
#undef ANALOG8_CHANNEL
#endif
#ifdef ANALOG9_BIT
#undef ANALOG9_BIT
#endif
#ifdef ANALOG9_PORT
#undef ANALOG9_PORT
#endif
#ifdef ANALOG9_CHANNEL
#undef ANALOG9_CHANNEL
#endif
#ifdef ANALOG10_BIT
#undef ANALOG10_BIT
#endif
#ifdef ANALOG10_PORT
#undef ANALOG10_PORT
#endif
#ifdef ANALOG10_CHANNEL
#undef ANALOG10_CHANNEL
#endif
#ifdef ANALOG11_BIT
#undef ANALOG11_BIT
#endif
#ifdef ANALOG11_PORT
#undef ANALOG11_PORT
#endif
#ifdef ANALOG11_CHANNEL
#undef ANALOG11_CHANNEL
#endif
#ifdef ANALOG12_BIT
#undef ANALOG12_BIT
#endif
#ifdef ANALOG12_PORT
#undef ANALOG12_PORT
#endif
#ifdef ANALOG12_CHANNEL
#undef ANALOG12_CHANNEL
#endif
#ifdef ANALOG13_BIT
#undef ANALOG13_BIT
#endif
#ifdef ANALOG13_PORT
#undef ANALOG13_PORT
#endif
#ifdef ANALOG13_CHANNEL
#undef ANALOG13_CHANNEL
#endif
#ifdef ANALOG14_BIT
#undef ANALOG14_BIT
#endif
#ifdef ANALOG14_PORT
#undef ANALOG14_PORT
#endif
#ifdef ANALOG14_CHANNEL
#undef ANALOG14_CHANNEL
#endif
#ifdef ANALOG15_BIT
#undef ANALOG15_BIT
#endif
#ifdef ANALOG15_PORT
#undef ANALOG15_PORT
#endif
#ifdef ANALOG15_CHANNEL
#undef ANALOG15_CHANNEL
#endif
#ifdef DIN0_BIT
#undef DIN0_BIT
#endif
#ifdef DIN0_PORT
#undef DIN0_PORT
#endif
#ifdef DIN0_PULLUP
#undef DIN0_PULLUP
#endif
#ifdef DIN0_ISR
#undef DIN0_ISR
#endif
#ifdef DIN1_BIT
#undef DIN1_BIT
#endif
#ifdef DIN1_PORT
#undef DIN1_PORT
#endif
#ifdef DIN1_PULLUP
#undef DIN1_PULLUP
#endif
#ifdef DIN1_ISR
#undef DIN1_ISR
#endif
#ifdef DIN2_BIT
#undef DIN2_BIT
#endif
#ifdef DIN2_PORT
#undef DIN2_PORT
#endif
#ifdef DIN2_PULLUP
#undef DIN2_PULLUP
#endif
#ifdef DIN2_ISR
#undef DIN2_ISR
#endif
#ifdef DIN3_BIT
#undef DIN3_BIT
#endif
#ifdef DIN3_PORT
#undef DIN3_PORT
#endif
#ifdef DIN3_PULLUP
#undef DIN3_PULLUP
#endif
#ifdef DIN3_ISR
#undef DIN3_ISR
#endif
#ifdef DIN4_BIT
#undef DIN4_BIT
#endif
#ifdef DIN4_PORT
#undef DIN4_PORT
#endif
#ifdef DIN4_PULLUP
#undef DIN4_PULLUP
#endif
#ifdef DIN4_ISR
#undef DIN4_ISR
#endif
#ifdef DIN5_BIT
#undef DIN5_BIT
#endif
#ifdef DIN5_PORT
#undef DIN5_PORT
#endif
#ifdef DIN5_PULLUP
#undef DIN5_PULLUP
#endif
#ifdef DIN5_ISR
#undef DIN5_ISR
#endif
#ifdef DIN6_BIT
#undef DIN6_BIT
#endif
#ifdef DIN6_PORT
#undef DIN6_PORT
#endif
#ifdef DIN6_PULLUP
#undef DIN6_PULLUP
#endif
#ifdef DIN6_ISR
#undef DIN6_ISR
#endif
#ifdef DIN7_BIT
#undef DIN7_BIT
#endif
#ifdef DIN7_PORT
#undef DIN7_PORT
#endif
#ifdef DIN7_PULLUP
#undef DIN7_PULLUP
#endif
#ifdef DIN7_ISR
#undef DIN7_ISR
#endif
#ifdef DIN8_BIT
#undef DIN8_BIT
#endif
#ifdef DIN8_PORT
#undef DIN8_PORT
#endif
#ifdef DIN8_PULLUP
#undef DIN8_PULLUP
#endif
#ifdef DIN9_BIT
#undef DIN9_BIT
#endif
#ifdef DIN9_PORT
#undef DIN9_PORT
#endif
#ifdef DIN9_PULLUP
#undef DIN9_PULLUP
#endif
#ifdef DIN10_BIT
#undef DIN10_BIT
#endif
#ifdef DIN10_PORT
#undef DIN10_PORT
#endif
#ifdef DIN10_PULLUP
#undef DIN10_PULLUP
#endif
#ifdef DIN11_BIT
#undef DIN11_BIT
#endif
#ifdef DIN11_PORT
#undef DIN11_PORT
#endif
#ifdef DIN11_PULLUP
#undef DIN11_PULLUP
#endif
#ifdef DIN12_BIT
#undef DIN12_BIT
#endif
#ifdef DIN12_PORT
#undef DIN12_PORT
#endif
#ifdef DIN12_PULLUP
#undef DIN12_PULLUP
#endif
#ifdef DIN13_BIT
#undef DIN13_BIT
#endif
#ifdef DIN13_PORT
#undef DIN13_PORT
#endif
#ifdef DIN13_PULLUP
#undef DIN13_PULLUP
#endif
#ifdef DIN14_BIT
#undef DIN14_BIT
#endif
#ifdef DIN14_PORT
#undef DIN14_PORT
#endif
#ifdef DIN14_PULLUP
#undef DIN14_PULLUP
#endif
#ifdef DIN15_BIT
#undef DIN15_BIT
#endif
#ifdef DIN15_PORT
#undef DIN15_PORT
#endif
#ifdef DIN15_PULLUP
#undef DIN15_PULLUP
#endif
#ifdef DIN16_BIT
#undef DIN16_BIT
#endif
#ifdef DIN16_PORT
#undef DIN16_PORT
#endif
#ifdef DIN16_PULLUP
#undef DIN16_PULLUP
#endif
#ifdef DIN17_BIT
#undef DIN17_BIT
#endif
#ifdef DIN17_PORT
#undef DIN17_PORT
#endif
#ifdef DIN17_PULLUP
#undef DIN17_PULLUP
#endif
#ifdef DIN18_BIT
#undef DIN18_BIT
#endif
#ifdef DIN18_PORT
#undef DIN18_PORT
#endif
#ifdef DIN18_PULLUP
#undef DIN18_PULLUP
#endif
#ifdef DIN19_BIT
#undef DIN19_BIT
#endif
#define DIN19_BIT 0
#ifdef DIN19_PORT
#undef DIN19_PORT
#endif
#define DIN19_PORT L
#ifdef DIN19_PULLUP
#undef DIN19_PULLUP
#endif
#define DIN19_PULLUP
#ifdef DIN20_BIT
#undef DIN20_BIT
#endif
#define DIN20_BIT 1
#ifdef DIN20_PORT
#undef DIN20_PORT
#endif
#define DIN20_PORT K
#ifdef DIN20_PULLUP
#undef DIN20_PULLUP
#endif
#define DIN20_PULLUP
#ifdef DIN21_BIT
#undef DIN21_BIT
#endif
#define DIN21_BIT 2
#ifdef DIN21_PORT
#undef DIN21_PORT
#endif
#define DIN21_PORT K
#ifdef DIN21_PULLUP
#undef DIN21_PULLUP
#endif
#define DIN21_PULLUP
#ifdef DIN22_BIT
#undef DIN22_BIT
#endif
#define DIN22_BIT 3
#ifdef DIN22_PORT
#undef DIN22_PORT
#endif
#define DIN22_PORT K
#ifdef DIN22_PULLUP
#undef DIN22_PULLUP
#endif
#define DIN22_PULLUP
#ifdef DIN23_BIT
#undef DIN23_BIT
#endif
#define DIN23_BIT 4
#ifdef DIN23_PORT
#undef DIN23_PORT
#endif
#define DIN23_PORT K
#ifdef DIN23_PULLUP
#undef DIN23_PULLUP
#endif
#define DIN23_PULLUP
#ifdef DIN24_BIT
#undef DIN24_BIT
#endif
#ifdef DIN24_PORT
#undef DIN24_PORT
#endif
#ifdef DIN24_PULLUP
#undef DIN24_PULLUP
#endif
#ifdef DIN25_BIT
#undef DIN25_BIT
#endif
#ifdef DIN25_PORT
#undef DIN25_PORT
#endif
#ifdef DIN25_PULLUP
#undef DIN25_PULLUP
#endif
#ifdef DIN26_BIT
#undef DIN26_BIT
#endif
#ifdef DIN26_PORT
#undef DIN26_PORT
#endif
#ifdef DIN26_PULLUP
#undef DIN26_PULLUP
#endif
#ifdef DIN27_BIT
#undef DIN27_BIT
#endif
#ifdef DIN27_PORT
#undef DIN27_PORT
#endif
#ifdef DIN27_PULLUP
#undef DIN27_PULLUP
#endif
#ifdef DIN28_BIT
#undef DIN28_BIT
#endif
#ifdef DIN28_PORT
#undef DIN28_PORT
#endif
#ifdef DIN28_PULLUP
#undef DIN28_PULLUP
#endif
#ifdef DIN29_BIT
#undef DIN29_BIT
#endif
#ifdef DIN29_PORT
#undef DIN29_PORT
#endif
#ifdef DIN29_PULLUP
#undef DIN29_PULLUP
#endif
#ifdef DIN30_BIT
#undef DIN30_BIT
#endif
#define DIN30_BIT 0
#ifdef DIN30_PORT
#undef DIN30_PORT
#endif
#define DIN30_PORT D
#ifdef DIN30_PULLUP
#undef DIN30_PULLUP
#endif
#define DIN30_PULLUP
#ifdef DIN31_BIT
#undef DIN31_BIT
#endif
#define DIN31_BIT 1
#ifdef DIN31_PORT
#undef DIN31_PORT
#endif
#define DIN31_PORT D
#ifdef DIN31_PULLUP
#undef DIN31_PULLUP
#endif
#define DIN31_PULLUP
#ifdef TX_BIT
#undef TX_BIT
#endif
#define TX_BIT 1
#ifdef TX_PORT
#undef TX_PORT
#endif
#define TX_PORT E
#ifdef RX_BIT
#undef RX_BIT
#endif
#define RX_BIT 0
#ifdef RX_PORT
#undef RX_PORT
#endif
#define RX_PORT E
#ifdef USB_DM_BIT
#undef USB_DM_BIT
#endif
#ifdef USB_DM_PORT
#undef USB_DM_PORT
#endif
#ifdef USB_DP_BIT
#undef USB_DP_BIT
#endif
#ifdef USB_DP_PORT
#undef USB_DP_PORT
#endif
#ifdef SPI_CLK_BIT
#undef SPI_CLK_BIT
#endif
#define SPI_CLK_BIT 1
#ifdef SPI_CLK_PORT
#undef SPI_CLK_PORT
#endif
#define SPI_CLK_PORT B
#ifdef SPI_SDI_BIT
#undef SPI_SDI_BIT
#endif
#define SPI_SDI_BIT 3
#ifdef SPI_SDI_PORT
#undef SPI_SDI_PORT
#endif
#define SPI_SDI_PORT B
#ifdef SPI_SDO_BIT
#undef SPI_SDO_BIT
#endif
#define SPI_SDO_BIT 2
#ifdef SPI_SDO_PORT
#undef SPI_SDO_PORT
#endif
#define SPI_SDO_PORT B
#ifdef SPI_CS_BIT
#undef SPI_CS_BIT
#endif
#define SPI_CS_BIT 0
#ifdef SPI_CS_PORT
#undef SPI_CS_PORT
#endif
#define SPI_CS_PORT B
#ifdef I2C_SCL_BIT
#undef I2C_SCL_BIT
#endif
#ifdef I2C_SCL_PORT
#undef I2C_SCL_PORT
#endif
#ifdef I2C_SDA_BIT
#undef I2C_SDA_BIT
#endif
#ifdef I2C_SDA_PORT
#undef I2C_SDA_PORT
#endif

#ifdef __cplusplus
}
#endif
#endif
