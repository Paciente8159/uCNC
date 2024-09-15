
#ifndef IO_HAL_H
#define IO_HAL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*IO HAL*/
#if ASSERT_PIN_IO(STEP0)
#define io1_config_output mcu_config_output(STEP0)
#define io1_set_output mcu_set_output(STEP0)
#define io1_clear_output mcu_clear_output(STEP0)
#define io1_toggle_output mcu_toggle_output(STEP0)
#define io1_get_output mcu_get_output(STEP0)
#if !defined(STEP0_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io1_config_input mcu_config_input(STEP0)
#else
#define io1_config_input mcu_config_input(STEP0);mcu_config_pullup(STEP0)
#endif
#define io1_config_pullup mcu_config_pullup(STEP0)
#define io1_get_input mcu_get_input(STEP0)
#elif ASSERT_PIN_EXTENDED(STEP0)
#define io1_config_output
#define io1_set_output ic74hc595_set_pin(STEP0);ic74hc595_shift_io_pins()
#define io1_clear_output ic74hc595_clear_pin(STEP0);ic74hc595_shift_io_pins()
#define io1_toggle_output ic74hc595_toggle_pin(STEP0);ic74hc595_shift_io_pins()
#define io1_get_output ic74hc595_get_pin(STEP0)
#define io1_config_input
#define io1_config_pullup
#define io1_get_input 0
#else
#define io1_config_output
#define io1_set_output
#define io1_clear_output
#define io1_toggle_output
#define io1_get_output 0
#define io1_config_input
#define io1_config_pullup
#define io1_get_input 0
#endif
#if ASSERT_PIN_IO(STEP1)
#define io2_config_output mcu_config_output(STEP1)
#define io2_set_output mcu_set_output(STEP1)
#define io2_clear_output mcu_clear_output(STEP1)
#define io2_toggle_output mcu_toggle_output(STEP1)
#define io2_get_output mcu_get_output(STEP1)
#if !defined(STEP1_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io2_config_input mcu_config_input(STEP1)
#else
#define io2_config_input mcu_config_input(STEP1);mcu_config_pullup(STEP1)
#endif
#define io2_config_pullup mcu_config_pullup(STEP1)
#define io2_get_input mcu_get_input(STEP1)
#elif ASSERT_PIN_EXTENDED(STEP1)
#define io2_config_output
#define io2_set_output ic74hc595_set_pin(STEP1);ic74hc595_shift_io_pins()
#define io2_clear_output ic74hc595_clear_pin(STEP1);ic74hc595_shift_io_pins()
#define io2_toggle_output ic74hc595_toggle_pin(STEP1);ic74hc595_shift_io_pins()
#define io2_get_output ic74hc595_get_pin(STEP1)
#define io2_config_input
#define io2_config_pullup
#define io2_get_input 0
#else
#define io2_config_output
#define io2_set_output
#define io2_clear_output
#define io2_toggle_output
#define io2_get_output 0
#define io2_config_input
#define io2_config_pullup
#define io2_get_input 0
#endif
#if ASSERT_PIN_IO(STEP2)
#define io3_config_output mcu_config_output(STEP2)
#define io3_set_output mcu_set_output(STEP2)
#define io3_clear_output mcu_clear_output(STEP2)
#define io3_toggle_output mcu_toggle_output(STEP2)
#define io3_get_output mcu_get_output(STEP2)
#if !defined(STEP2_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io3_config_input mcu_config_input(STEP2)
#else
#define io3_config_input mcu_config_input(STEP2);mcu_config_pullup(STEP2)
#endif
#define io3_config_pullup mcu_config_pullup(STEP2)
#define io3_get_input mcu_get_input(STEP2)
#elif ASSERT_PIN_EXTENDED(STEP2)
#define io3_config_output
#define io3_set_output ic74hc595_set_pin(STEP2);ic74hc595_shift_io_pins()
#define io3_clear_output ic74hc595_clear_pin(STEP2);ic74hc595_shift_io_pins()
#define io3_toggle_output ic74hc595_toggle_pin(STEP2);ic74hc595_shift_io_pins()
#define io3_get_output ic74hc595_get_pin(STEP2)
#define io3_config_input
#define io3_config_pullup
#define io3_get_input 0
#else
#define io3_config_output
#define io3_set_output
#define io3_clear_output
#define io3_toggle_output
#define io3_get_output 0
#define io3_config_input
#define io3_config_pullup
#define io3_get_input 0
#endif
#if ASSERT_PIN_IO(STEP3)
#define io4_config_output mcu_config_output(STEP3)
#define io4_set_output mcu_set_output(STEP3)
#define io4_clear_output mcu_clear_output(STEP3)
#define io4_toggle_output mcu_toggle_output(STEP3)
#define io4_get_output mcu_get_output(STEP3)
#if !defined(STEP3_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io4_config_input mcu_config_input(STEP3)
#else
#define io4_config_input mcu_config_input(STEP3);mcu_config_pullup(STEP3)
#endif
#define io4_config_pullup mcu_config_pullup(STEP3)
#define io4_get_input mcu_get_input(STEP3)
#elif ASSERT_PIN_EXTENDED(STEP3)
#define io4_config_output
#define io4_set_output ic74hc595_set_pin(STEP3);ic74hc595_shift_io_pins()
#define io4_clear_output ic74hc595_clear_pin(STEP3);ic74hc595_shift_io_pins()
#define io4_toggle_output ic74hc595_toggle_pin(STEP3);ic74hc595_shift_io_pins()
#define io4_get_output ic74hc595_get_pin(STEP3)
#define io4_config_input
#define io4_config_pullup
#define io4_get_input 0
#else
#define io4_config_output
#define io4_set_output
#define io4_clear_output
#define io4_toggle_output
#define io4_get_output 0
#define io4_config_input
#define io4_config_pullup
#define io4_get_input 0
#endif
#if ASSERT_PIN_IO(STEP4)
#define io5_config_output mcu_config_output(STEP4)
#define io5_set_output mcu_set_output(STEP4)
#define io5_clear_output mcu_clear_output(STEP4)
#define io5_toggle_output mcu_toggle_output(STEP4)
#define io5_get_output mcu_get_output(STEP4)
#if !defined(STEP4_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io5_config_input mcu_config_input(STEP4)
#else
#define io5_config_input mcu_config_input(STEP4);mcu_config_pullup(STEP4)
#endif
#define io5_config_pullup mcu_config_pullup(STEP4)
#define io5_get_input mcu_get_input(STEP4)
#elif ASSERT_PIN_EXTENDED(STEP4)
#define io5_config_output
#define io5_set_output ic74hc595_set_pin(STEP4);ic74hc595_shift_io_pins()
#define io5_clear_output ic74hc595_clear_pin(STEP4);ic74hc595_shift_io_pins()
#define io5_toggle_output ic74hc595_toggle_pin(STEP4);ic74hc595_shift_io_pins()
#define io5_get_output ic74hc595_get_pin(STEP4)
#define io5_config_input
#define io5_config_pullup
#define io5_get_input 0
#else
#define io5_config_output
#define io5_set_output
#define io5_clear_output
#define io5_toggle_output
#define io5_get_output 0
#define io5_config_input
#define io5_config_pullup
#define io5_get_input 0
#endif
#if ASSERT_PIN_IO(STEP5)
#define io6_config_output mcu_config_output(STEP5)
#define io6_set_output mcu_set_output(STEP5)
#define io6_clear_output mcu_clear_output(STEP5)
#define io6_toggle_output mcu_toggle_output(STEP5)
#define io6_get_output mcu_get_output(STEP5)
#if !defined(STEP5_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io6_config_input mcu_config_input(STEP5)
#else
#define io6_config_input mcu_config_input(STEP5);mcu_config_pullup(STEP5)
#endif
#define io6_config_pullup mcu_config_pullup(STEP5)
#define io6_get_input mcu_get_input(STEP5)
#elif ASSERT_PIN_EXTENDED(STEP5)
#define io6_config_output
#define io6_set_output ic74hc595_set_pin(STEP5);ic74hc595_shift_io_pins()
#define io6_clear_output ic74hc595_clear_pin(STEP5);ic74hc595_shift_io_pins()
#define io6_toggle_output ic74hc595_toggle_pin(STEP5);ic74hc595_shift_io_pins()
#define io6_get_output ic74hc595_get_pin(STEP5)
#define io6_config_input
#define io6_config_pullup
#define io6_get_input 0
#else
#define io6_config_output
#define io6_set_output
#define io6_clear_output
#define io6_toggle_output
#define io6_get_output 0
#define io6_config_input
#define io6_config_pullup
#define io6_get_input 0
#endif
#if ASSERT_PIN_IO(STEP6)
#define io7_config_output mcu_config_output(STEP6)
#define io7_set_output mcu_set_output(STEP6)
#define io7_clear_output mcu_clear_output(STEP6)
#define io7_toggle_output mcu_toggle_output(STEP6)
#define io7_get_output mcu_get_output(STEP6)
#if !defined(STEP6_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io7_config_input mcu_config_input(STEP6)
#else
#define io7_config_input mcu_config_input(STEP6);mcu_config_pullup(STEP6)
#endif
#define io7_config_pullup mcu_config_pullup(STEP6)
#define io7_get_input mcu_get_input(STEP6)
#elif ASSERT_PIN_EXTENDED(STEP6)
#define io7_config_output
#define io7_set_output ic74hc595_set_pin(STEP6);ic74hc595_shift_io_pins()
#define io7_clear_output ic74hc595_clear_pin(STEP6);ic74hc595_shift_io_pins()
#define io7_toggle_output ic74hc595_toggle_pin(STEP6);ic74hc595_shift_io_pins()
#define io7_get_output ic74hc595_get_pin(STEP6)
#define io7_config_input
#define io7_config_pullup
#define io7_get_input 0
#else
#define io7_config_output
#define io7_set_output
#define io7_clear_output
#define io7_toggle_output
#define io7_get_output 0
#define io7_config_input
#define io7_config_pullup
#define io7_get_input 0
#endif
#if ASSERT_PIN_IO(STEP7)
#define io8_config_output mcu_config_output(STEP7)
#define io8_set_output mcu_set_output(STEP7)
#define io8_clear_output mcu_clear_output(STEP7)
#define io8_toggle_output mcu_toggle_output(STEP7)
#define io8_get_output mcu_get_output(STEP7)
#if !defined(STEP7_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io8_config_input mcu_config_input(STEP7)
#else
#define io8_config_input mcu_config_input(STEP7);mcu_config_pullup(STEP7)
#endif
#define io8_config_pullup mcu_config_pullup(STEP7)
#define io8_get_input mcu_get_input(STEP7)
#elif ASSERT_PIN_EXTENDED(STEP7)
#define io8_config_output
#define io8_set_output ic74hc595_set_pin(STEP7);ic74hc595_shift_io_pins()
#define io8_clear_output ic74hc595_clear_pin(STEP7);ic74hc595_shift_io_pins()
#define io8_toggle_output ic74hc595_toggle_pin(STEP7);ic74hc595_shift_io_pins()
#define io8_get_output ic74hc595_get_pin(STEP7)
#define io8_config_input
#define io8_config_pullup
#define io8_get_input 0
#else
#define io8_config_output
#define io8_set_output
#define io8_clear_output
#define io8_toggle_output
#define io8_get_output 0
#define io8_config_input
#define io8_config_pullup
#define io8_get_input 0
#endif
#if ASSERT_PIN_IO(DIR0)
#define io9_config_output mcu_config_output(DIR0)
#define io9_set_output mcu_set_output(DIR0)
#define io9_clear_output mcu_clear_output(DIR0)
#define io9_toggle_output mcu_toggle_output(DIR0)
#define io9_get_output mcu_get_output(DIR0)
#if !defined(DIR0_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io9_config_input mcu_config_input(DIR0)
#else
#define io9_config_input mcu_config_input(DIR0);mcu_config_pullup(DIR0)
#endif
#define io9_config_pullup mcu_config_pullup(DIR0)
#define io9_get_input mcu_get_input(DIR0)
#elif ASSERT_PIN_EXTENDED(DIR0)
#define io9_config_output
#define io9_set_output ic74hc595_set_pin(DIR0);ic74hc595_shift_io_pins()
#define io9_clear_output ic74hc595_clear_pin(DIR0);ic74hc595_shift_io_pins()
#define io9_toggle_output ic74hc595_toggle_pin(DIR0);ic74hc595_shift_io_pins()
#define io9_get_output ic74hc595_get_pin(DIR0)
#define io9_config_input
#define io9_config_pullup
#define io9_get_input 0
#else
#define io9_config_output
#define io9_set_output
#define io9_clear_output
#define io9_toggle_output
#define io9_get_output 0
#define io9_config_input
#define io9_config_pullup
#define io9_get_input 0
#endif
#if ASSERT_PIN_IO(DIR1)
#define io10_config_output mcu_config_output(DIR1)
#define io10_set_output mcu_set_output(DIR1)
#define io10_clear_output mcu_clear_output(DIR1)
#define io10_toggle_output mcu_toggle_output(DIR1)
#define io10_get_output mcu_get_output(DIR1)
#if !defined(DIR1_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io10_config_input mcu_config_input(DIR1)
#else
#define io10_config_input mcu_config_input(DIR1);mcu_config_pullup(DIR1)
#endif
#define io10_config_pullup mcu_config_pullup(DIR1)
#define io10_get_input mcu_get_input(DIR1)
#elif ASSERT_PIN_EXTENDED(DIR1)
#define io10_config_output
#define io10_set_output ic74hc595_set_pin(DIR1);ic74hc595_shift_io_pins()
#define io10_clear_output ic74hc595_clear_pin(DIR1);ic74hc595_shift_io_pins()
#define io10_toggle_output ic74hc595_toggle_pin(DIR1);ic74hc595_shift_io_pins()
#define io10_get_output ic74hc595_get_pin(DIR1)
#define io10_config_input
#define io10_config_pullup
#define io10_get_input 0
#else
#define io10_config_output
#define io10_set_output
#define io10_clear_output
#define io10_toggle_output
#define io10_get_output 0
#define io10_config_input
#define io10_config_pullup
#define io10_get_input 0
#endif
#if ASSERT_PIN_IO(DIR2)
#define io11_config_output mcu_config_output(DIR2)
#define io11_set_output mcu_set_output(DIR2)
#define io11_clear_output mcu_clear_output(DIR2)
#define io11_toggle_output mcu_toggle_output(DIR2)
#define io11_get_output mcu_get_output(DIR2)
#if !defined(DIR2_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io11_config_input mcu_config_input(DIR2)
#else
#define io11_config_input mcu_config_input(DIR2);mcu_config_pullup(DIR2)
#endif
#define io11_config_pullup mcu_config_pullup(DIR2)
#define io11_get_input mcu_get_input(DIR2)
#elif ASSERT_PIN_EXTENDED(DIR2)
#define io11_config_output
#define io11_set_output ic74hc595_set_pin(DIR2);ic74hc595_shift_io_pins()
#define io11_clear_output ic74hc595_clear_pin(DIR2);ic74hc595_shift_io_pins()
#define io11_toggle_output ic74hc595_toggle_pin(DIR2);ic74hc595_shift_io_pins()
#define io11_get_output ic74hc595_get_pin(DIR2)
#define io11_config_input
#define io11_config_pullup
#define io11_get_input 0
#else
#define io11_config_output
#define io11_set_output
#define io11_clear_output
#define io11_toggle_output
#define io11_get_output 0
#define io11_config_input
#define io11_config_pullup
#define io11_get_input 0
#endif
#if ASSERT_PIN_IO(DIR3)
#define io12_config_output mcu_config_output(DIR3)
#define io12_set_output mcu_set_output(DIR3)
#define io12_clear_output mcu_clear_output(DIR3)
#define io12_toggle_output mcu_toggle_output(DIR3)
#define io12_get_output mcu_get_output(DIR3)
#if !defined(DIR3_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io12_config_input mcu_config_input(DIR3)
#else
#define io12_config_input mcu_config_input(DIR3);mcu_config_pullup(DIR3)
#endif
#define io12_config_pullup mcu_config_pullup(DIR3)
#define io12_get_input mcu_get_input(DIR3)
#elif ASSERT_PIN_EXTENDED(DIR3)
#define io12_config_output
#define io12_set_output ic74hc595_set_pin(DIR3);ic74hc595_shift_io_pins()
#define io12_clear_output ic74hc595_clear_pin(DIR3);ic74hc595_shift_io_pins()
#define io12_toggle_output ic74hc595_toggle_pin(DIR3);ic74hc595_shift_io_pins()
#define io12_get_output ic74hc595_get_pin(DIR3)
#define io12_config_input
#define io12_config_pullup
#define io12_get_input 0
#else
#define io12_config_output
#define io12_set_output
#define io12_clear_output
#define io12_toggle_output
#define io12_get_output 0
#define io12_config_input
#define io12_config_pullup
#define io12_get_input 0
#endif
#if ASSERT_PIN_IO(DIR4)
#define io13_config_output mcu_config_output(DIR4)
#define io13_set_output mcu_set_output(DIR4)
#define io13_clear_output mcu_clear_output(DIR4)
#define io13_toggle_output mcu_toggle_output(DIR4)
#define io13_get_output mcu_get_output(DIR4)
#if !defined(DIR4_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io13_config_input mcu_config_input(DIR4)
#else
#define io13_config_input mcu_config_input(DIR4);mcu_config_pullup(DIR4)
#endif
#define io13_config_pullup mcu_config_pullup(DIR4)
#define io13_get_input mcu_get_input(DIR4)
#elif ASSERT_PIN_EXTENDED(DIR4)
#define io13_config_output
#define io13_set_output ic74hc595_set_pin(DIR4);ic74hc595_shift_io_pins()
#define io13_clear_output ic74hc595_clear_pin(DIR4);ic74hc595_shift_io_pins()
#define io13_toggle_output ic74hc595_toggle_pin(DIR4);ic74hc595_shift_io_pins()
#define io13_get_output ic74hc595_get_pin(DIR4)
#define io13_config_input
#define io13_config_pullup
#define io13_get_input 0
#else
#define io13_config_output
#define io13_set_output
#define io13_clear_output
#define io13_toggle_output
#define io13_get_output 0
#define io13_config_input
#define io13_config_pullup
#define io13_get_input 0
#endif
#if ASSERT_PIN_IO(DIR5)
#define io14_config_output mcu_config_output(DIR5)
#define io14_set_output mcu_set_output(DIR5)
#define io14_clear_output mcu_clear_output(DIR5)
#define io14_toggle_output mcu_toggle_output(DIR5)
#define io14_get_output mcu_get_output(DIR5)
#if !defined(DIR5_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io14_config_input mcu_config_input(DIR5)
#else
#define io14_config_input mcu_config_input(DIR5);mcu_config_pullup(DIR5)
#endif
#define io14_config_pullup mcu_config_pullup(DIR5)
#define io14_get_input mcu_get_input(DIR5)
#elif ASSERT_PIN_EXTENDED(DIR5)
#define io14_config_output
#define io14_set_output ic74hc595_set_pin(DIR5);ic74hc595_shift_io_pins()
#define io14_clear_output ic74hc595_clear_pin(DIR5);ic74hc595_shift_io_pins()
#define io14_toggle_output ic74hc595_toggle_pin(DIR5);ic74hc595_shift_io_pins()
#define io14_get_output ic74hc595_get_pin(DIR5)
#define io14_config_input
#define io14_config_pullup
#define io14_get_input 0
#else
#define io14_config_output
#define io14_set_output
#define io14_clear_output
#define io14_toggle_output
#define io14_get_output 0
#define io14_config_input
#define io14_config_pullup
#define io14_get_input 0
#endif
#if ASSERT_PIN_IO(DIR6)
#define io15_config_output mcu_config_output(DIR6)
#define io15_set_output mcu_set_output(DIR6)
#define io15_clear_output mcu_clear_output(DIR6)
#define io15_toggle_output mcu_toggle_output(DIR6)
#define io15_get_output mcu_get_output(DIR6)
#if !defined(DIR6_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io15_config_input mcu_config_input(DIR6)
#else
#define io15_config_input mcu_config_input(DIR6);mcu_config_pullup(DIR6)
#endif
#define io15_config_pullup mcu_config_pullup(DIR6)
#define io15_get_input mcu_get_input(DIR6)
#elif ASSERT_PIN_EXTENDED(DIR6)
#define io15_config_output
#define io15_set_output ic74hc595_set_pin(DIR6);ic74hc595_shift_io_pins()
#define io15_clear_output ic74hc595_clear_pin(DIR6);ic74hc595_shift_io_pins()
#define io15_toggle_output ic74hc595_toggle_pin(DIR6);ic74hc595_shift_io_pins()
#define io15_get_output ic74hc595_get_pin(DIR6)
#define io15_config_input
#define io15_config_pullup
#define io15_get_input 0
#else
#define io15_config_output
#define io15_set_output
#define io15_clear_output
#define io15_toggle_output
#define io15_get_output 0
#define io15_config_input
#define io15_config_pullup
#define io15_get_input 0
#endif
#if ASSERT_PIN_IO(DIR7)
#define io16_config_output mcu_config_output(DIR7)
#define io16_set_output mcu_set_output(DIR7)
#define io16_clear_output mcu_clear_output(DIR7)
#define io16_toggle_output mcu_toggle_output(DIR7)
#define io16_get_output mcu_get_output(DIR7)
#if !defined(DIR7_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io16_config_input mcu_config_input(DIR7)
#else
#define io16_config_input mcu_config_input(DIR7);mcu_config_pullup(DIR7)
#endif
#define io16_config_pullup mcu_config_pullup(DIR7)
#define io16_get_input mcu_get_input(DIR7)
#elif ASSERT_PIN_EXTENDED(DIR7)
#define io16_config_output
#define io16_set_output ic74hc595_set_pin(DIR7);ic74hc595_shift_io_pins()
#define io16_clear_output ic74hc595_clear_pin(DIR7);ic74hc595_shift_io_pins()
#define io16_toggle_output ic74hc595_toggle_pin(DIR7);ic74hc595_shift_io_pins()
#define io16_get_output ic74hc595_get_pin(DIR7)
#define io16_config_input
#define io16_config_pullup
#define io16_get_input 0
#else
#define io16_config_output
#define io16_set_output
#define io16_clear_output
#define io16_toggle_output
#define io16_get_output 0
#define io16_config_input
#define io16_config_pullup
#define io16_get_input 0
#endif
#if ASSERT_PIN_IO(STEP0_EN)
#define io17_config_output mcu_config_output(STEP0_EN)
#define io17_set_output mcu_set_output(STEP0_EN)
#define io17_clear_output mcu_clear_output(STEP0_EN)
#define io17_toggle_output mcu_toggle_output(STEP0_EN)
#define io17_get_output mcu_get_output(STEP0_EN)
#if !defined(STEP0_EN_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io17_config_input mcu_config_input(STEP0_EN)
#else
#define io17_config_input mcu_config_input(STEP0_EN);mcu_config_pullup(STEP0_EN)
#endif
#define io17_config_pullup mcu_config_pullup(STEP0_EN)
#define io17_get_input mcu_get_input(STEP0_EN)
#elif ASSERT_PIN_EXTENDED(STEP0_EN)
#define io17_config_output
#define io17_set_output ic74hc595_set_pin(STEP0_EN);ic74hc595_shift_io_pins()
#define io17_clear_output ic74hc595_clear_pin(STEP0_EN);ic74hc595_shift_io_pins()
#define io17_toggle_output ic74hc595_toggle_pin(STEP0_EN);ic74hc595_shift_io_pins()
#define io17_get_output ic74hc595_get_pin(STEP0_EN)
#define io17_config_input
#define io17_config_pullup
#define io17_get_input 0
#else
#define io17_config_output
#define io17_set_output
#define io17_clear_output
#define io17_toggle_output
#define io17_get_output 0
#define io17_config_input
#define io17_config_pullup
#define io17_get_input 0
#endif
#if ASSERT_PIN_IO(STEP1_EN)
#define io18_config_output mcu_config_output(STEP1_EN)
#define io18_set_output mcu_set_output(STEP1_EN)
#define io18_clear_output mcu_clear_output(STEP1_EN)
#define io18_toggle_output mcu_toggle_output(STEP1_EN)
#define io18_get_output mcu_get_output(STEP1_EN)
#if !defined(STEP1_EN_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io18_config_input mcu_config_input(STEP1_EN)
#else
#define io18_config_input mcu_config_input(STEP1_EN);mcu_config_pullup(STEP1_EN)
#endif
#define io18_config_pullup mcu_config_pullup(STEP1_EN)
#define io18_get_input mcu_get_input(STEP1_EN)
#elif ASSERT_PIN_EXTENDED(STEP1_EN)
#define io18_config_output
#define io18_set_output ic74hc595_set_pin(STEP1_EN);ic74hc595_shift_io_pins()
#define io18_clear_output ic74hc595_clear_pin(STEP1_EN);ic74hc595_shift_io_pins()
#define io18_toggle_output ic74hc595_toggle_pin(STEP1_EN);ic74hc595_shift_io_pins()
#define io18_get_output ic74hc595_get_pin(STEP1_EN)
#define io18_config_input
#define io18_config_pullup
#define io18_get_input 0
#else
#define io18_config_output
#define io18_set_output
#define io18_clear_output
#define io18_toggle_output
#define io18_get_output 0
#define io18_config_input
#define io18_config_pullup
#define io18_get_input 0
#endif
#if ASSERT_PIN_IO(STEP2_EN)
#define io19_config_output mcu_config_output(STEP2_EN)
#define io19_set_output mcu_set_output(STEP2_EN)
#define io19_clear_output mcu_clear_output(STEP2_EN)
#define io19_toggle_output mcu_toggle_output(STEP2_EN)
#define io19_get_output mcu_get_output(STEP2_EN)
#if !defined(STEP2_EN_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io19_config_input mcu_config_input(STEP2_EN)
#else
#define io19_config_input mcu_config_input(STEP2_EN);mcu_config_pullup(STEP2_EN)
#endif
#define io19_config_pullup mcu_config_pullup(STEP2_EN)
#define io19_get_input mcu_get_input(STEP2_EN)
#elif ASSERT_PIN_EXTENDED(STEP2_EN)
#define io19_config_output
#define io19_set_output ic74hc595_set_pin(STEP2_EN);ic74hc595_shift_io_pins()
#define io19_clear_output ic74hc595_clear_pin(STEP2_EN);ic74hc595_shift_io_pins()
#define io19_toggle_output ic74hc595_toggle_pin(STEP2_EN);ic74hc595_shift_io_pins()
#define io19_get_output ic74hc595_get_pin(STEP2_EN)
#define io19_config_input
#define io19_config_pullup
#define io19_get_input 0
#else
#define io19_config_output
#define io19_set_output
#define io19_clear_output
#define io19_toggle_output
#define io19_get_output 0
#define io19_config_input
#define io19_config_pullup
#define io19_get_input 0
#endif
#if ASSERT_PIN_IO(STEP3_EN)
#define io20_config_output mcu_config_output(STEP3_EN)
#define io20_set_output mcu_set_output(STEP3_EN)
#define io20_clear_output mcu_clear_output(STEP3_EN)
#define io20_toggle_output mcu_toggle_output(STEP3_EN)
#define io20_get_output mcu_get_output(STEP3_EN)
#if !defined(STEP3_EN_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io20_config_input mcu_config_input(STEP3_EN)
#else
#define io20_config_input mcu_config_input(STEP3_EN);mcu_config_pullup(STEP3_EN)
#endif
#define io20_config_pullup mcu_config_pullup(STEP3_EN)
#define io20_get_input mcu_get_input(STEP3_EN)
#elif ASSERT_PIN_EXTENDED(STEP3_EN)
#define io20_config_output
#define io20_set_output ic74hc595_set_pin(STEP3_EN);ic74hc595_shift_io_pins()
#define io20_clear_output ic74hc595_clear_pin(STEP3_EN);ic74hc595_shift_io_pins()
#define io20_toggle_output ic74hc595_toggle_pin(STEP3_EN);ic74hc595_shift_io_pins()
#define io20_get_output ic74hc595_get_pin(STEP3_EN)
#define io20_config_input
#define io20_config_pullup
#define io20_get_input 0
#else
#define io20_config_output
#define io20_set_output
#define io20_clear_output
#define io20_toggle_output
#define io20_get_output 0
#define io20_config_input
#define io20_config_pullup
#define io20_get_input 0
#endif
#if ASSERT_PIN_IO(STEP4_EN)
#define io21_config_output mcu_config_output(STEP4_EN)
#define io21_set_output mcu_set_output(STEP4_EN)
#define io21_clear_output mcu_clear_output(STEP4_EN)
#define io21_toggle_output mcu_toggle_output(STEP4_EN)
#define io21_get_output mcu_get_output(STEP4_EN)
#if !defined(STEP4_EN_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io21_config_input mcu_config_input(STEP4_EN)
#else
#define io21_config_input mcu_config_input(STEP4_EN);mcu_config_pullup(STEP4_EN)
#endif
#define io21_config_pullup mcu_config_pullup(STEP4_EN)
#define io21_get_input mcu_get_input(STEP4_EN)
#elif ASSERT_PIN_EXTENDED(STEP4_EN)
#define io21_config_output
#define io21_set_output ic74hc595_set_pin(STEP4_EN);ic74hc595_shift_io_pins()
#define io21_clear_output ic74hc595_clear_pin(STEP4_EN);ic74hc595_shift_io_pins()
#define io21_toggle_output ic74hc595_toggle_pin(STEP4_EN);ic74hc595_shift_io_pins()
#define io21_get_output ic74hc595_get_pin(STEP4_EN)
#define io21_config_input
#define io21_config_pullup
#define io21_get_input 0
#else
#define io21_config_output
#define io21_set_output
#define io21_clear_output
#define io21_toggle_output
#define io21_get_output 0
#define io21_config_input
#define io21_config_pullup
#define io21_get_input 0
#endif
#if ASSERT_PIN_IO(STEP5_EN)
#define io22_config_output mcu_config_output(STEP5_EN)
#define io22_set_output mcu_set_output(STEP5_EN)
#define io22_clear_output mcu_clear_output(STEP5_EN)
#define io22_toggle_output mcu_toggle_output(STEP5_EN)
#define io22_get_output mcu_get_output(STEP5_EN)
#if !defined(STEP5_EN_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io22_config_input mcu_config_input(STEP5_EN)
#else
#define io22_config_input mcu_config_input(STEP5_EN);mcu_config_pullup(STEP5_EN)
#endif
#define io22_config_pullup mcu_config_pullup(STEP5_EN)
#define io22_get_input mcu_get_input(STEP5_EN)
#elif ASSERT_PIN_EXTENDED(STEP5_EN)
#define io22_config_output
#define io22_set_output ic74hc595_set_pin(STEP5_EN);ic74hc595_shift_io_pins()
#define io22_clear_output ic74hc595_clear_pin(STEP5_EN);ic74hc595_shift_io_pins()
#define io22_toggle_output ic74hc595_toggle_pin(STEP5_EN);ic74hc595_shift_io_pins()
#define io22_get_output ic74hc595_get_pin(STEP5_EN)
#define io22_config_input
#define io22_config_pullup
#define io22_get_input 0
#else
#define io22_config_output
#define io22_set_output
#define io22_clear_output
#define io22_toggle_output
#define io22_get_output 0
#define io22_config_input
#define io22_config_pullup
#define io22_get_input 0
#endif
#if ASSERT_PIN_IO(STEP6_EN)
#define io23_config_output mcu_config_output(STEP6_EN)
#define io23_set_output mcu_set_output(STEP6_EN)
#define io23_clear_output mcu_clear_output(STEP6_EN)
#define io23_toggle_output mcu_toggle_output(STEP6_EN)
#define io23_get_output mcu_get_output(STEP6_EN)
#if !defined(STEP6_EN_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io23_config_input mcu_config_input(STEP6_EN)
#else
#define io23_config_input mcu_config_input(STEP6_EN);mcu_config_pullup(STEP6_EN)
#endif
#define io23_config_pullup mcu_config_pullup(STEP6_EN)
#define io23_get_input mcu_get_input(STEP6_EN)
#elif ASSERT_PIN_EXTENDED(STEP6_EN)
#define io23_config_output
#define io23_set_output ic74hc595_set_pin(STEP6_EN);ic74hc595_shift_io_pins()
#define io23_clear_output ic74hc595_clear_pin(STEP6_EN);ic74hc595_shift_io_pins()
#define io23_toggle_output ic74hc595_toggle_pin(STEP6_EN);ic74hc595_shift_io_pins()
#define io23_get_output ic74hc595_get_pin(STEP6_EN)
#define io23_config_input
#define io23_config_pullup
#define io23_get_input 0
#else
#define io23_config_output
#define io23_set_output
#define io23_clear_output
#define io23_toggle_output
#define io23_get_output 0
#define io23_config_input
#define io23_config_pullup
#define io23_get_input 0
#endif
#if ASSERT_PIN_IO(STEP7_EN)
#define io24_config_output mcu_config_output(STEP7_EN)
#define io24_set_output mcu_set_output(STEP7_EN)
#define io24_clear_output mcu_clear_output(STEP7_EN)
#define io24_toggle_output mcu_toggle_output(STEP7_EN)
#define io24_get_output mcu_get_output(STEP7_EN)
#if !defined(STEP7_EN_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io24_config_input mcu_config_input(STEP7_EN)
#else
#define io24_config_input mcu_config_input(STEP7_EN);mcu_config_pullup(STEP7_EN)
#endif
#define io24_config_pullup mcu_config_pullup(STEP7_EN)
#define io24_get_input mcu_get_input(STEP7_EN)
#elif ASSERT_PIN_EXTENDED(STEP7_EN)
#define io24_config_output
#define io24_set_output ic74hc595_set_pin(STEP7_EN);ic74hc595_shift_io_pins()
#define io24_clear_output ic74hc595_clear_pin(STEP7_EN);ic74hc595_shift_io_pins()
#define io24_toggle_output ic74hc595_toggle_pin(STEP7_EN);ic74hc595_shift_io_pins()
#define io24_get_output ic74hc595_get_pin(STEP7_EN)
#define io24_config_input
#define io24_config_pullup
#define io24_get_input 0
#else
#define io24_config_output
#define io24_set_output
#define io24_clear_output
#define io24_toggle_output
#define io24_get_output 0
#define io24_config_input
#define io24_config_pullup
#define io24_get_input 0
#endif
#if ASSERT_PIN_IO(PWM0)
#define io25_config_output mcu_config_output(PWM0)
#define io25_set_output mcu_set_output(PWM0)
#define io25_clear_output mcu_clear_output(PWM0)
#define io25_toggle_output mcu_toggle_output(PWM0)
#define io25_get_output mcu_get_output(PWM0)
#if !defined(PWM0_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io25_config_input mcu_config_input(PWM0)
#else
#define io25_config_input mcu_config_input(PWM0);mcu_config_pullup(PWM0)
#endif
#define io25_config_pullup mcu_config_pullup(PWM0)
#define io25_get_input mcu_get_input(PWM0)
#elif ASSERT_PIN_EXTENDED(PWM0)
#define io25_config_output
#define io25_set_output ic74hc595_set_pin(PWM0);ic74hc595_shift_io_pins()
#define io25_clear_output ic74hc595_clear_pin(PWM0);ic74hc595_shift_io_pins()
#define io25_toggle_output ic74hc595_toggle_pin(PWM0);ic74hc595_shift_io_pins()
#define io25_get_output ic74hc595_get_pin(PWM0)
#define io25_config_input
#define io25_config_pullup
#define io25_get_input 0
#else
#define io25_config_output
#define io25_set_output
#define io25_clear_output
#define io25_toggle_output
#define io25_get_output 0
#define io25_config_input
#define io25_config_pullup
#define io25_get_input 0
#endif
#if ASSERT_PIN_IO(PWM1)
#define io26_config_output mcu_config_output(PWM1)
#define io26_set_output mcu_set_output(PWM1)
#define io26_clear_output mcu_clear_output(PWM1)
#define io26_toggle_output mcu_toggle_output(PWM1)
#define io26_get_output mcu_get_output(PWM1)
#if !defined(PWM1_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io26_config_input mcu_config_input(PWM1)
#else
#define io26_config_input mcu_config_input(PWM1);mcu_config_pullup(PWM1)
#endif
#define io26_config_pullup mcu_config_pullup(PWM1)
#define io26_get_input mcu_get_input(PWM1)
#elif ASSERT_PIN_EXTENDED(PWM1)
#define io26_config_output
#define io26_set_output ic74hc595_set_pin(PWM1);ic74hc595_shift_io_pins()
#define io26_clear_output ic74hc595_clear_pin(PWM1);ic74hc595_shift_io_pins()
#define io26_toggle_output ic74hc595_toggle_pin(PWM1);ic74hc595_shift_io_pins()
#define io26_get_output ic74hc595_get_pin(PWM1)
#define io26_config_input
#define io26_config_pullup
#define io26_get_input 0
#else
#define io26_config_output
#define io26_set_output
#define io26_clear_output
#define io26_toggle_output
#define io26_get_output 0
#define io26_config_input
#define io26_config_pullup
#define io26_get_input 0
#endif
#if ASSERT_PIN_IO(PWM2)
#define io27_config_output mcu_config_output(PWM2)
#define io27_set_output mcu_set_output(PWM2)
#define io27_clear_output mcu_clear_output(PWM2)
#define io27_toggle_output mcu_toggle_output(PWM2)
#define io27_get_output mcu_get_output(PWM2)
#if !defined(PWM2_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io27_config_input mcu_config_input(PWM2)
#else
#define io27_config_input mcu_config_input(PWM2);mcu_config_pullup(PWM2)
#endif
#define io27_config_pullup mcu_config_pullup(PWM2)
#define io27_get_input mcu_get_input(PWM2)
#elif ASSERT_PIN_EXTENDED(PWM2)
#define io27_config_output
#define io27_set_output ic74hc595_set_pin(PWM2);ic74hc595_shift_io_pins()
#define io27_clear_output ic74hc595_clear_pin(PWM2);ic74hc595_shift_io_pins()
#define io27_toggle_output ic74hc595_toggle_pin(PWM2);ic74hc595_shift_io_pins()
#define io27_get_output ic74hc595_get_pin(PWM2)
#define io27_config_input
#define io27_config_pullup
#define io27_get_input 0
#else
#define io27_config_output
#define io27_set_output
#define io27_clear_output
#define io27_toggle_output
#define io27_get_output 0
#define io27_config_input
#define io27_config_pullup
#define io27_get_input 0
#endif
#if ASSERT_PIN_IO(PWM3)
#define io28_config_output mcu_config_output(PWM3)
#define io28_set_output mcu_set_output(PWM3)
#define io28_clear_output mcu_clear_output(PWM3)
#define io28_toggle_output mcu_toggle_output(PWM3)
#define io28_get_output mcu_get_output(PWM3)
#if !defined(PWM3_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io28_config_input mcu_config_input(PWM3)
#else
#define io28_config_input mcu_config_input(PWM3);mcu_config_pullup(PWM3)
#endif
#define io28_config_pullup mcu_config_pullup(PWM3)
#define io28_get_input mcu_get_input(PWM3)
#elif ASSERT_PIN_EXTENDED(PWM3)
#define io28_config_output
#define io28_set_output ic74hc595_set_pin(PWM3);ic74hc595_shift_io_pins()
#define io28_clear_output ic74hc595_clear_pin(PWM3);ic74hc595_shift_io_pins()
#define io28_toggle_output ic74hc595_toggle_pin(PWM3);ic74hc595_shift_io_pins()
#define io28_get_output ic74hc595_get_pin(PWM3)
#define io28_config_input
#define io28_config_pullup
#define io28_get_input 0
#else
#define io28_config_output
#define io28_set_output
#define io28_clear_output
#define io28_toggle_output
#define io28_get_output 0
#define io28_config_input
#define io28_config_pullup
#define io28_get_input 0
#endif
#if ASSERT_PIN_IO(PWM4)
#define io29_config_output mcu_config_output(PWM4)
#define io29_set_output mcu_set_output(PWM4)
#define io29_clear_output mcu_clear_output(PWM4)
#define io29_toggle_output mcu_toggle_output(PWM4)
#define io29_get_output mcu_get_output(PWM4)
#if !defined(PWM4_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io29_config_input mcu_config_input(PWM4)
#else
#define io29_config_input mcu_config_input(PWM4);mcu_config_pullup(PWM4)
#endif
#define io29_config_pullup mcu_config_pullup(PWM4)
#define io29_get_input mcu_get_input(PWM4)
#elif ASSERT_PIN_EXTENDED(PWM4)
#define io29_config_output
#define io29_set_output ic74hc595_set_pin(PWM4);ic74hc595_shift_io_pins()
#define io29_clear_output ic74hc595_clear_pin(PWM4);ic74hc595_shift_io_pins()
#define io29_toggle_output ic74hc595_toggle_pin(PWM4);ic74hc595_shift_io_pins()
#define io29_get_output ic74hc595_get_pin(PWM4)
#define io29_config_input
#define io29_config_pullup
#define io29_get_input 0
#else
#define io29_config_output
#define io29_set_output
#define io29_clear_output
#define io29_toggle_output
#define io29_get_output 0
#define io29_config_input
#define io29_config_pullup
#define io29_get_input 0
#endif
#if ASSERT_PIN_IO(PWM5)
#define io30_config_output mcu_config_output(PWM5)
#define io30_set_output mcu_set_output(PWM5)
#define io30_clear_output mcu_clear_output(PWM5)
#define io30_toggle_output mcu_toggle_output(PWM5)
#define io30_get_output mcu_get_output(PWM5)
#if !defined(PWM5_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io30_config_input mcu_config_input(PWM5)
#else
#define io30_config_input mcu_config_input(PWM5);mcu_config_pullup(PWM5)
#endif
#define io30_config_pullup mcu_config_pullup(PWM5)
#define io30_get_input mcu_get_input(PWM5)
#elif ASSERT_PIN_EXTENDED(PWM5)
#define io30_config_output
#define io30_set_output ic74hc595_set_pin(PWM5);ic74hc595_shift_io_pins()
#define io30_clear_output ic74hc595_clear_pin(PWM5);ic74hc595_shift_io_pins()
#define io30_toggle_output ic74hc595_toggle_pin(PWM5);ic74hc595_shift_io_pins()
#define io30_get_output ic74hc595_get_pin(PWM5)
#define io30_config_input
#define io30_config_pullup
#define io30_get_input 0
#else
#define io30_config_output
#define io30_set_output
#define io30_clear_output
#define io30_toggle_output
#define io30_get_output 0
#define io30_config_input
#define io30_config_pullup
#define io30_get_input 0
#endif
#if ASSERT_PIN_IO(PWM6)
#define io31_config_output mcu_config_output(PWM6)
#define io31_set_output mcu_set_output(PWM6)
#define io31_clear_output mcu_clear_output(PWM6)
#define io31_toggle_output mcu_toggle_output(PWM6)
#define io31_get_output mcu_get_output(PWM6)
#if !defined(PWM6_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io31_config_input mcu_config_input(PWM6)
#else
#define io31_config_input mcu_config_input(PWM6);mcu_config_pullup(PWM6)
#endif
#define io31_config_pullup mcu_config_pullup(PWM6)
#define io31_get_input mcu_get_input(PWM6)
#elif ASSERT_PIN_EXTENDED(PWM6)
#define io31_config_output
#define io31_set_output ic74hc595_set_pin(PWM6);ic74hc595_shift_io_pins()
#define io31_clear_output ic74hc595_clear_pin(PWM6);ic74hc595_shift_io_pins()
#define io31_toggle_output ic74hc595_toggle_pin(PWM6);ic74hc595_shift_io_pins()
#define io31_get_output ic74hc595_get_pin(PWM6)
#define io31_config_input
#define io31_config_pullup
#define io31_get_input 0
#else
#define io31_config_output
#define io31_set_output
#define io31_clear_output
#define io31_toggle_output
#define io31_get_output 0
#define io31_config_input
#define io31_config_pullup
#define io31_get_input 0
#endif
#if ASSERT_PIN_IO(PWM7)
#define io32_config_output mcu_config_output(PWM7)
#define io32_set_output mcu_set_output(PWM7)
#define io32_clear_output mcu_clear_output(PWM7)
#define io32_toggle_output mcu_toggle_output(PWM7)
#define io32_get_output mcu_get_output(PWM7)
#if !defined(PWM7_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io32_config_input mcu_config_input(PWM7)
#else
#define io32_config_input mcu_config_input(PWM7);mcu_config_pullup(PWM7)
#endif
#define io32_config_pullup mcu_config_pullup(PWM7)
#define io32_get_input mcu_get_input(PWM7)
#elif ASSERT_PIN_EXTENDED(PWM7)
#define io32_config_output
#define io32_set_output ic74hc595_set_pin(PWM7);ic74hc595_shift_io_pins()
#define io32_clear_output ic74hc595_clear_pin(PWM7);ic74hc595_shift_io_pins()
#define io32_toggle_output ic74hc595_toggle_pin(PWM7);ic74hc595_shift_io_pins()
#define io32_get_output ic74hc595_get_pin(PWM7)
#define io32_config_input
#define io32_config_pullup
#define io32_get_input 0
#else
#define io32_config_output
#define io32_set_output
#define io32_clear_output
#define io32_toggle_output
#define io32_get_output 0
#define io32_config_input
#define io32_config_pullup
#define io32_get_input 0
#endif
#if ASSERT_PIN_IO(PWM8)
#define io33_config_output mcu_config_output(PWM8)
#define io33_set_output mcu_set_output(PWM8)
#define io33_clear_output mcu_clear_output(PWM8)
#define io33_toggle_output mcu_toggle_output(PWM8)
#define io33_get_output mcu_get_output(PWM8)
#if !defined(PWM8_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io33_config_input mcu_config_input(PWM8)
#else
#define io33_config_input mcu_config_input(PWM8);mcu_config_pullup(PWM8)
#endif
#define io33_config_pullup mcu_config_pullup(PWM8)
#define io33_get_input mcu_get_input(PWM8)
#elif ASSERT_PIN_EXTENDED(PWM8)
#define io33_config_output
#define io33_set_output ic74hc595_set_pin(PWM8);ic74hc595_shift_io_pins()
#define io33_clear_output ic74hc595_clear_pin(PWM8);ic74hc595_shift_io_pins()
#define io33_toggle_output ic74hc595_toggle_pin(PWM8);ic74hc595_shift_io_pins()
#define io33_get_output ic74hc595_get_pin(PWM8)
#define io33_config_input
#define io33_config_pullup
#define io33_get_input 0
#else
#define io33_config_output
#define io33_set_output
#define io33_clear_output
#define io33_toggle_output
#define io33_get_output 0
#define io33_config_input
#define io33_config_pullup
#define io33_get_input 0
#endif
#if ASSERT_PIN_IO(PWM9)
#define io34_config_output mcu_config_output(PWM9)
#define io34_set_output mcu_set_output(PWM9)
#define io34_clear_output mcu_clear_output(PWM9)
#define io34_toggle_output mcu_toggle_output(PWM9)
#define io34_get_output mcu_get_output(PWM9)
#if !defined(PWM9_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io34_config_input mcu_config_input(PWM9)
#else
#define io34_config_input mcu_config_input(PWM9);mcu_config_pullup(PWM9)
#endif
#define io34_config_pullup mcu_config_pullup(PWM9)
#define io34_get_input mcu_get_input(PWM9)
#elif ASSERT_PIN_EXTENDED(PWM9)
#define io34_config_output
#define io34_set_output ic74hc595_set_pin(PWM9);ic74hc595_shift_io_pins()
#define io34_clear_output ic74hc595_clear_pin(PWM9);ic74hc595_shift_io_pins()
#define io34_toggle_output ic74hc595_toggle_pin(PWM9);ic74hc595_shift_io_pins()
#define io34_get_output ic74hc595_get_pin(PWM9)
#define io34_config_input
#define io34_config_pullup
#define io34_get_input 0
#else
#define io34_config_output
#define io34_set_output
#define io34_clear_output
#define io34_toggle_output
#define io34_get_output 0
#define io34_config_input
#define io34_config_pullup
#define io34_get_input 0
#endif
#if ASSERT_PIN_IO(PWM10)
#define io35_config_output mcu_config_output(PWM10)
#define io35_set_output mcu_set_output(PWM10)
#define io35_clear_output mcu_clear_output(PWM10)
#define io35_toggle_output mcu_toggle_output(PWM10)
#define io35_get_output mcu_get_output(PWM10)
#if !defined(PWM10_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io35_config_input mcu_config_input(PWM10)
#else
#define io35_config_input mcu_config_input(PWM10);mcu_config_pullup(PWM10)
#endif
#define io35_config_pullup mcu_config_pullup(PWM10)
#define io35_get_input mcu_get_input(PWM10)
#elif ASSERT_PIN_EXTENDED(PWM10)
#define io35_config_output
#define io35_set_output ic74hc595_set_pin(PWM10);ic74hc595_shift_io_pins()
#define io35_clear_output ic74hc595_clear_pin(PWM10);ic74hc595_shift_io_pins()
#define io35_toggle_output ic74hc595_toggle_pin(PWM10);ic74hc595_shift_io_pins()
#define io35_get_output ic74hc595_get_pin(PWM10)
#define io35_config_input
#define io35_config_pullup
#define io35_get_input 0
#else
#define io35_config_output
#define io35_set_output
#define io35_clear_output
#define io35_toggle_output
#define io35_get_output 0
#define io35_config_input
#define io35_config_pullup
#define io35_get_input 0
#endif
#if ASSERT_PIN_IO(PWM11)
#define io36_config_output mcu_config_output(PWM11)
#define io36_set_output mcu_set_output(PWM11)
#define io36_clear_output mcu_clear_output(PWM11)
#define io36_toggle_output mcu_toggle_output(PWM11)
#define io36_get_output mcu_get_output(PWM11)
#if !defined(PWM11_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io36_config_input mcu_config_input(PWM11)
#else
#define io36_config_input mcu_config_input(PWM11);mcu_config_pullup(PWM11)
#endif
#define io36_config_pullup mcu_config_pullup(PWM11)
#define io36_get_input mcu_get_input(PWM11)
#elif ASSERT_PIN_EXTENDED(PWM11)
#define io36_config_output
#define io36_set_output ic74hc595_set_pin(PWM11);ic74hc595_shift_io_pins()
#define io36_clear_output ic74hc595_clear_pin(PWM11);ic74hc595_shift_io_pins()
#define io36_toggle_output ic74hc595_toggle_pin(PWM11);ic74hc595_shift_io_pins()
#define io36_get_output ic74hc595_get_pin(PWM11)
#define io36_config_input
#define io36_config_pullup
#define io36_get_input 0
#else
#define io36_config_output
#define io36_set_output
#define io36_clear_output
#define io36_toggle_output
#define io36_get_output 0
#define io36_config_input
#define io36_config_pullup
#define io36_get_input 0
#endif
#if ASSERT_PIN_IO(PWM12)
#define io37_config_output mcu_config_output(PWM12)
#define io37_set_output mcu_set_output(PWM12)
#define io37_clear_output mcu_clear_output(PWM12)
#define io37_toggle_output mcu_toggle_output(PWM12)
#define io37_get_output mcu_get_output(PWM12)
#if !defined(PWM12_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io37_config_input mcu_config_input(PWM12)
#else
#define io37_config_input mcu_config_input(PWM12);mcu_config_pullup(PWM12)
#endif
#define io37_config_pullup mcu_config_pullup(PWM12)
#define io37_get_input mcu_get_input(PWM12)
#elif ASSERT_PIN_EXTENDED(PWM12)
#define io37_config_output
#define io37_set_output ic74hc595_set_pin(PWM12);ic74hc595_shift_io_pins()
#define io37_clear_output ic74hc595_clear_pin(PWM12);ic74hc595_shift_io_pins()
#define io37_toggle_output ic74hc595_toggle_pin(PWM12);ic74hc595_shift_io_pins()
#define io37_get_output ic74hc595_get_pin(PWM12)
#define io37_config_input
#define io37_config_pullup
#define io37_get_input 0
#else
#define io37_config_output
#define io37_set_output
#define io37_clear_output
#define io37_toggle_output
#define io37_get_output 0
#define io37_config_input
#define io37_config_pullup
#define io37_get_input 0
#endif
#if ASSERT_PIN_IO(PWM13)
#define io38_config_output mcu_config_output(PWM13)
#define io38_set_output mcu_set_output(PWM13)
#define io38_clear_output mcu_clear_output(PWM13)
#define io38_toggle_output mcu_toggle_output(PWM13)
#define io38_get_output mcu_get_output(PWM13)
#if !defined(PWM13_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io38_config_input mcu_config_input(PWM13)
#else
#define io38_config_input mcu_config_input(PWM13);mcu_config_pullup(PWM13)
#endif
#define io38_config_pullup mcu_config_pullup(PWM13)
#define io38_get_input mcu_get_input(PWM13)
#elif ASSERT_PIN_EXTENDED(PWM13)
#define io38_config_output
#define io38_set_output ic74hc595_set_pin(PWM13);ic74hc595_shift_io_pins()
#define io38_clear_output ic74hc595_clear_pin(PWM13);ic74hc595_shift_io_pins()
#define io38_toggle_output ic74hc595_toggle_pin(PWM13);ic74hc595_shift_io_pins()
#define io38_get_output ic74hc595_get_pin(PWM13)
#define io38_config_input
#define io38_config_pullup
#define io38_get_input 0
#else
#define io38_config_output
#define io38_set_output
#define io38_clear_output
#define io38_toggle_output
#define io38_get_output 0
#define io38_config_input
#define io38_config_pullup
#define io38_get_input 0
#endif
#if ASSERT_PIN_IO(PWM14)
#define io39_config_output mcu_config_output(PWM14)
#define io39_set_output mcu_set_output(PWM14)
#define io39_clear_output mcu_clear_output(PWM14)
#define io39_toggle_output mcu_toggle_output(PWM14)
#define io39_get_output mcu_get_output(PWM14)
#if !defined(PWM14_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io39_config_input mcu_config_input(PWM14)
#else
#define io39_config_input mcu_config_input(PWM14);mcu_config_pullup(PWM14)
#endif
#define io39_config_pullup mcu_config_pullup(PWM14)
#define io39_get_input mcu_get_input(PWM14)
#elif ASSERT_PIN_EXTENDED(PWM14)
#define io39_config_output
#define io39_set_output ic74hc595_set_pin(PWM14);ic74hc595_shift_io_pins()
#define io39_clear_output ic74hc595_clear_pin(PWM14);ic74hc595_shift_io_pins()
#define io39_toggle_output ic74hc595_toggle_pin(PWM14);ic74hc595_shift_io_pins()
#define io39_get_output ic74hc595_get_pin(PWM14)
#define io39_config_input
#define io39_config_pullup
#define io39_get_input 0
#else
#define io39_config_output
#define io39_set_output
#define io39_clear_output
#define io39_toggle_output
#define io39_get_output 0
#define io39_config_input
#define io39_config_pullup
#define io39_get_input 0
#endif
#if ASSERT_PIN_IO(PWM15)
#define io40_config_output mcu_config_output(PWM15)
#define io40_set_output mcu_set_output(PWM15)
#define io40_clear_output mcu_clear_output(PWM15)
#define io40_toggle_output mcu_toggle_output(PWM15)
#define io40_get_output mcu_get_output(PWM15)
#if !defined(PWM15_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io40_config_input mcu_config_input(PWM15)
#else
#define io40_config_input mcu_config_input(PWM15);mcu_config_pullup(PWM15)
#endif
#define io40_config_pullup mcu_config_pullup(PWM15)
#define io40_get_input mcu_get_input(PWM15)
#elif ASSERT_PIN_EXTENDED(PWM15)
#define io40_config_output
#define io40_set_output ic74hc595_set_pin(PWM15);ic74hc595_shift_io_pins()
#define io40_clear_output ic74hc595_clear_pin(PWM15);ic74hc595_shift_io_pins()
#define io40_toggle_output ic74hc595_toggle_pin(PWM15);ic74hc595_shift_io_pins()
#define io40_get_output ic74hc595_get_pin(PWM15)
#define io40_config_input
#define io40_config_pullup
#define io40_get_input 0
#else
#define io40_config_output
#define io40_set_output
#define io40_clear_output
#define io40_toggle_output
#define io40_get_output 0
#define io40_config_input
#define io40_config_pullup
#define io40_get_input 0
#endif
#if ASSERT_PIN_IO(SERVO0)
#define io41_config_output mcu_config_output(SERVO0)
#define io41_set_output mcu_set_output(SERVO0)
#define io41_clear_output mcu_clear_output(SERVO0)
#define io41_toggle_output mcu_toggle_output(SERVO0)
#define io41_get_output mcu_get_output(SERVO0)
#if !defined(SERVO0_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io41_config_input mcu_config_input(SERVO0)
#else
#define io41_config_input mcu_config_input(SERVO0);mcu_config_pullup(SERVO0)
#endif
#define io41_config_pullup mcu_config_pullup(SERVO0)
#define io41_get_input mcu_get_input(SERVO0)
#elif ASSERT_PIN_EXTENDED(SERVO0)
#define io41_config_output
#define io41_set_output ic74hc595_set_pin(SERVO0);ic74hc595_shift_io_pins()
#define io41_clear_output ic74hc595_clear_pin(SERVO0);ic74hc595_shift_io_pins()
#define io41_toggle_output ic74hc595_toggle_pin(SERVO0);ic74hc595_shift_io_pins()
#define io41_get_output ic74hc595_get_pin(SERVO0)
#define io41_config_input
#define io41_config_pullup
#define io41_get_input 0
#else
#define io41_config_output
#define io41_set_output
#define io41_clear_output
#define io41_toggle_output
#define io41_get_output 0
#define io41_config_input
#define io41_config_pullup
#define io41_get_input 0
#endif
#if ASSERT_PIN_IO(SERVO1)
#define io42_config_output mcu_config_output(SERVO1)
#define io42_set_output mcu_set_output(SERVO1)
#define io42_clear_output mcu_clear_output(SERVO1)
#define io42_toggle_output mcu_toggle_output(SERVO1)
#define io42_get_output mcu_get_output(SERVO1)
#if !defined(SERVO1_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io42_config_input mcu_config_input(SERVO1)
#else
#define io42_config_input mcu_config_input(SERVO1);mcu_config_pullup(SERVO1)
#endif
#define io42_config_pullup mcu_config_pullup(SERVO1)
#define io42_get_input mcu_get_input(SERVO1)
#elif ASSERT_PIN_EXTENDED(SERVO1)
#define io42_config_output
#define io42_set_output ic74hc595_set_pin(SERVO1);ic74hc595_shift_io_pins()
#define io42_clear_output ic74hc595_clear_pin(SERVO1);ic74hc595_shift_io_pins()
#define io42_toggle_output ic74hc595_toggle_pin(SERVO1);ic74hc595_shift_io_pins()
#define io42_get_output ic74hc595_get_pin(SERVO1)
#define io42_config_input
#define io42_config_pullup
#define io42_get_input 0
#else
#define io42_config_output
#define io42_set_output
#define io42_clear_output
#define io42_toggle_output
#define io42_get_output 0
#define io42_config_input
#define io42_config_pullup
#define io42_get_input 0
#endif
#if ASSERT_PIN_IO(SERVO2)
#define io43_config_output mcu_config_output(SERVO2)
#define io43_set_output mcu_set_output(SERVO2)
#define io43_clear_output mcu_clear_output(SERVO2)
#define io43_toggle_output mcu_toggle_output(SERVO2)
#define io43_get_output mcu_get_output(SERVO2)
#if !defined(SERVO2_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io43_config_input mcu_config_input(SERVO2)
#else
#define io43_config_input mcu_config_input(SERVO2);mcu_config_pullup(SERVO2)
#endif
#define io43_config_pullup mcu_config_pullup(SERVO2)
#define io43_get_input mcu_get_input(SERVO2)
#elif ASSERT_PIN_EXTENDED(SERVO2)
#define io43_config_output
#define io43_set_output ic74hc595_set_pin(SERVO2);ic74hc595_shift_io_pins()
#define io43_clear_output ic74hc595_clear_pin(SERVO2);ic74hc595_shift_io_pins()
#define io43_toggle_output ic74hc595_toggle_pin(SERVO2);ic74hc595_shift_io_pins()
#define io43_get_output ic74hc595_get_pin(SERVO2)
#define io43_config_input
#define io43_config_pullup
#define io43_get_input 0
#else
#define io43_config_output
#define io43_set_output
#define io43_clear_output
#define io43_toggle_output
#define io43_get_output 0
#define io43_config_input
#define io43_config_pullup
#define io43_get_input 0
#endif
#if ASSERT_PIN_IO(SERVO3)
#define io44_config_output mcu_config_output(SERVO3)
#define io44_set_output mcu_set_output(SERVO3)
#define io44_clear_output mcu_clear_output(SERVO3)
#define io44_toggle_output mcu_toggle_output(SERVO3)
#define io44_get_output mcu_get_output(SERVO3)
#if !defined(SERVO3_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io44_config_input mcu_config_input(SERVO3)
#else
#define io44_config_input mcu_config_input(SERVO3);mcu_config_pullup(SERVO3)
#endif
#define io44_config_pullup mcu_config_pullup(SERVO3)
#define io44_get_input mcu_get_input(SERVO3)
#elif ASSERT_PIN_EXTENDED(SERVO3)
#define io44_config_output
#define io44_set_output ic74hc595_set_pin(SERVO3);ic74hc595_shift_io_pins()
#define io44_clear_output ic74hc595_clear_pin(SERVO3);ic74hc595_shift_io_pins()
#define io44_toggle_output ic74hc595_toggle_pin(SERVO3);ic74hc595_shift_io_pins()
#define io44_get_output ic74hc595_get_pin(SERVO3)
#define io44_config_input
#define io44_config_pullup
#define io44_get_input 0
#else
#define io44_config_output
#define io44_set_output
#define io44_clear_output
#define io44_toggle_output
#define io44_get_output 0
#define io44_config_input
#define io44_config_pullup
#define io44_get_input 0
#endif
#if ASSERT_PIN_IO(SERVO4)
#define io45_config_output mcu_config_output(SERVO4)
#define io45_set_output mcu_set_output(SERVO4)
#define io45_clear_output mcu_clear_output(SERVO4)
#define io45_toggle_output mcu_toggle_output(SERVO4)
#define io45_get_output mcu_get_output(SERVO4)
#if !defined(SERVO4_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io45_config_input mcu_config_input(SERVO4)
#else
#define io45_config_input mcu_config_input(SERVO4);mcu_config_pullup(SERVO4)
#endif
#define io45_config_pullup mcu_config_pullup(SERVO4)
#define io45_get_input mcu_get_input(SERVO4)
#elif ASSERT_PIN_EXTENDED(SERVO4)
#define io45_config_output
#define io45_set_output ic74hc595_set_pin(SERVO4);ic74hc595_shift_io_pins()
#define io45_clear_output ic74hc595_clear_pin(SERVO4);ic74hc595_shift_io_pins()
#define io45_toggle_output ic74hc595_toggle_pin(SERVO4);ic74hc595_shift_io_pins()
#define io45_get_output ic74hc595_get_pin(SERVO4)
#define io45_config_input
#define io45_config_pullup
#define io45_get_input 0
#else
#define io45_config_output
#define io45_set_output
#define io45_clear_output
#define io45_toggle_output
#define io45_get_output 0
#define io45_config_input
#define io45_config_pullup
#define io45_get_input 0
#endif
#if ASSERT_PIN_IO(SERVO5)
#define io46_config_output mcu_config_output(SERVO5)
#define io46_set_output mcu_set_output(SERVO5)
#define io46_clear_output mcu_clear_output(SERVO5)
#define io46_toggle_output mcu_toggle_output(SERVO5)
#define io46_get_output mcu_get_output(SERVO5)
#if !defined(SERVO5_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io46_config_input mcu_config_input(SERVO5)
#else
#define io46_config_input mcu_config_input(SERVO5);mcu_config_pullup(SERVO5)
#endif
#define io46_config_pullup mcu_config_pullup(SERVO5)
#define io46_get_input mcu_get_input(SERVO5)
#elif ASSERT_PIN_EXTENDED(SERVO5)
#define io46_config_output
#define io46_set_output ic74hc595_set_pin(SERVO5);ic74hc595_shift_io_pins()
#define io46_clear_output ic74hc595_clear_pin(SERVO5);ic74hc595_shift_io_pins()
#define io46_toggle_output ic74hc595_toggle_pin(SERVO5);ic74hc595_shift_io_pins()
#define io46_get_output ic74hc595_get_pin(SERVO5)
#define io46_config_input
#define io46_config_pullup
#define io46_get_input 0
#else
#define io46_config_output
#define io46_set_output
#define io46_clear_output
#define io46_toggle_output
#define io46_get_output 0
#define io46_config_input
#define io46_config_pullup
#define io46_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT0)
#define io47_config_output mcu_config_output(DOUT0)
#define io47_set_output mcu_set_output(DOUT0)
#define io47_clear_output mcu_clear_output(DOUT0)
#define io47_toggle_output mcu_toggle_output(DOUT0)
#define io47_get_output mcu_get_output(DOUT0)
#if !defined(DOUT0_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io47_config_input mcu_config_input(DOUT0)
#else
#define io47_config_input mcu_config_input(DOUT0);mcu_config_pullup(DOUT0)
#endif
#define io47_config_pullup mcu_config_pullup(DOUT0)
#define io47_get_input mcu_get_input(DOUT0)
#elif ASSERT_PIN_EXTENDED(DOUT0)
#define io47_config_output
#define io47_set_output ic74hc595_set_pin(DOUT0);ic74hc595_shift_io_pins()
#define io47_clear_output ic74hc595_clear_pin(DOUT0);ic74hc595_shift_io_pins()
#define io47_toggle_output ic74hc595_toggle_pin(DOUT0);ic74hc595_shift_io_pins()
#define io47_get_output ic74hc595_get_pin(DOUT0)
#define io47_config_input
#define io47_config_pullup
#define io47_get_input 0
#else
#define io47_config_output
#define io47_set_output
#define io47_clear_output
#define io47_toggle_output
#define io47_get_output 0
#define io47_config_input
#define io47_config_pullup
#define io47_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT1)
#define io48_config_output mcu_config_output(DOUT1)
#define io48_set_output mcu_set_output(DOUT1)
#define io48_clear_output mcu_clear_output(DOUT1)
#define io48_toggle_output mcu_toggle_output(DOUT1)
#define io48_get_output mcu_get_output(DOUT1)
#if !defined(DOUT1_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io48_config_input mcu_config_input(DOUT1)
#else
#define io48_config_input mcu_config_input(DOUT1);mcu_config_pullup(DOUT1)
#endif
#define io48_config_pullup mcu_config_pullup(DOUT1)
#define io48_get_input mcu_get_input(DOUT1)
#elif ASSERT_PIN_EXTENDED(DOUT1)
#define io48_config_output
#define io48_set_output ic74hc595_set_pin(DOUT1);ic74hc595_shift_io_pins()
#define io48_clear_output ic74hc595_clear_pin(DOUT1);ic74hc595_shift_io_pins()
#define io48_toggle_output ic74hc595_toggle_pin(DOUT1);ic74hc595_shift_io_pins()
#define io48_get_output ic74hc595_get_pin(DOUT1)
#define io48_config_input
#define io48_config_pullup
#define io48_get_input 0
#else
#define io48_config_output
#define io48_set_output
#define io48_clear_output
#define io48_toggle_output
#define io48_get_output 0
#define io48_config_input
#define io48_config_pullup
#define io48_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT2)
#define io49_config_output mcu_config_output(DOUT2)
#define io49_set_output mcu_set_output(DOUT2)
#define io49_clear_output mcu_clear_output(DOUT2)
#define io49_toggle_output mcu_toggle_output(DOUT2)
#define io49_get_output mcu_get_output(DOUT2)
#if !defined(DOUT2_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io49_config_input mcu_config_input(DOUT2)
#else
#define io49_config_input mcu_config_input(DOUT2);mcu_config_pullup(DOUT2)
#endif
#define io49_config_pullup mcu_config_pullup(DOUT2)
#define io49_get_input mcu_get_input(DOUT2)
#elif ASSERT_PIN_EXTENDED(DOUT2)
#define io49_config_output
#define io49_set_output ic74hc595_set_pin(DOUT2);ic74hc595_shift_io_pins()
#define io49_clear_output ic74hc595_clear_pin(DOUT2);ic74hc595_shift_io_pins()
#define io49_toggle_output ic74hc595_toggle_pin(DOUT2);ic74hc595_shift_io_pins()
#define io49_get_output ic74hc595_get_pin(DOUT2)
#define io49_config_input
#define io49_config_pullup
#define io49_get_input 0
#else
#define io49_config_output
#define io49_set_output
#define io49_clear_output
#define io49_toggle_output
#define io49_get_output 0
#define io49_config_input
#define io49_config_pullup
#define io49_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT3)
#define io50_config_output mcu_config_output(DOUT3)
#define io50_set_output mcu_set_output(DOUT3)
#define io50_clear_output mcu_clear_output(DOUT3)
#define io50_toggle_output mcu_toggle_output(DOUT3)
#define io50_get_output mcu_get_output(DOUT3)
#if !defined(DOUT3_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io50_config_input mcu_config_input(DOUT3)
#else
#define io50_config_input mcu_config_input(DOUT3);mcu_config_pullup(DOUT3)
#endif
#define io50_config_pullup mcu_config_pullup(DOUT3)
#define io50_get_input mcu_get_input(DOUT3)
#elif ASSERT_PIN_EXTENDED(DOUT3)
#define io50_config_output
#define io50_set_output ic74hc595_set_pin(DOUT3);ic74hc595_shift_io_pins()
#define io50_clear_output ic74hc595_clear_pin(DOUT3);ic74hc595_shift_io_pins()
#define io50_toggle_output ic74hc595_toggle_pin(DOUT3);ic74hc595_shift_io_pins()
#define io50_get_output ic74hc595_get_pin(DOUT3)
#define io50_config_input
#define io50_config_pullup
#define io50_get_input 0
#else
#define io50_config_output
#define io50_set_output
#define io50_clear_output
#define io50_toggle_output
#define io50_get_output 0
#define io50_config_input
#define io50_config_pullup
#define io50_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT4)
#define io51_config_output mcu_config_output(DOUT4)
#define io51_set_output mcu_set_output(DOUT4)
#define io51_clear_output mcu_clear_output(DOUT4)
#define io51_toggle_output mcu_toggle_output(DOUT4)
#define io51_get_output mcu_get_output(DOUT4)
#if !defined(DOUT4_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io51_config_input mcu_config_input(DOUT4)
#else
#define io51_config_input mcu_config_input(DOUT4);mcu_config_pullup(DOUT4)
#endif
#define io51_config_pullup mcu_config_pullup(DOUT4)
#define io51_get_input mcu_get_input(DOUT4)
#elif ASSERT_PIN_EXTENDED(DOUT4)
#define io51_config_output
#define io51_set_output ic74hc595_set_pin(DOUT4);ic74hc595_shift_io_pins()
#define io51_clear_output ic74hc595_clear_pin(DOUT4);ic74hc595_shift_io_pins()
#define io51_toggle_output ic74hc595_toggle_pin(DOUT4);ic74hc595_shift_io_pins()
#define io51_get_output ic74hc595_get_pin(DOUT4)
#define io51_config_input
#define io51_config_pullup
#define io51_get_input 0
#else
#define io51_config_output
#define io51_set_output
#define io51_clear_output
#define io51_toggle_output
#define io51_get_output 0
#define io51_config_input
#define io51_config_pullup
#define io51_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT5)
#define io52_config_output mcu_config_output(DOUT5)
#define io52_set_output mcu_set_output(DOUT5)
#define io52_clear_output mcu_clear_output(DOUT5)
#define io52_toggle_output mcu_toggle_output(DOUT5)
#define io52_get_output mcu_get_output(DOUT5)
#if !defined(DOUT5_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io52_config_input mcu_config_input(DOUT5)
#else
#define io52_config_input mcu_config_input(DOUT5);mcu_config_pullup(DOUT5)
#endif
#define io52_config_pullup mcu_config_pullup(DOUT5)
#define io52_get_input mcu_get_input(DOUT5)
#elif ASSERT_PIN_EXTENDED(DOUT5)
#define io52_config_output
#define io52_set_output ic74hc595_set_pin(DOUT5);ic74hc595_shift_io_pins()
#define io52_clear_output ic74hc595_clear_pin(DOUT5);ic74hc595_shift_io_pins()
#define io52_toggle_output ic74hc595_toggle_pin(DOUT5);ic74hc595_shift_io_pins()
#define io52_get_output ic74hc595_get_pin(DOUT5)
#define io52_config_input
#define io52_config_pullup
#define io52_get_input 0
#else
#define io52_config_output
#define io52_set_output
#define io52_clear_output
#define io52_toggle_output
#define io52_get_output 0
#define io52_config_input
#define io52_config_pullup
#define io52_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT6)
#define io53_config_output mcu_config_output(DOUT6)
#define io53_set_output mcu_set_output(DOUT6)
#define io53_clear_output mcu_clear_output(DOUT6)
#define io53_toggle_output mcu_toggle_output(DOUT6)
#define io53_get_output mcu_get_output(DOUT6)
#if !defined(DOUT6_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io53_config_input mcu_config_input(DOUT6)
#else
#define io53_config_input mcu_config_input(DOUT6);mcu_config_pullup(DOUT6)
#endif
#define io53_config_pullup mcu_config_pullup(DOUT6)
#define io53_get_input mcu_get_input(DOUT6)
#elif ASSERT_PIN_EXTENDED(DOUT6)
#define io53_config_output
#define io53_set_output ic74hc595_set_pin(DOUT6);ic74hc595_shift_io_pins()
#define io53_clear_output ic74hc595_clear_pin(DOUT6);ic74hc595_shift_io_pins()
#define io53_toggle_output ic74hc595_toggle_pin(DOUT6);ic74hc595_shift_io_pins()
#define io53_get_output ic74hc595_get_pin(DOUT6)
#define io53_config_input
#define io53_config_pullup
#define io53_get_input 0
#else
#define io53_config_output
#define io53_set_output
#define io53_clear_output
#define io53_toggle_output
#define io53_get_output 0
#define io53_config_input
#define io53_config_pullup
#define io53_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT7)
#define io54_config_output mcu_config_output(DOUT7)
#define io54_set_output mcu_set_output(DOUT7)
#define io54_clear_output mcu_clear_output(DOUT7)
#define io54_toggle_output mcu_toggle_output(DOUT7)
#define io54_get_output mcu_get_output(DOUT7)
#if !defined(DOUT7_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io54_config_input mcu_config_input(DOUT7)
#else
#define io54_config_input mcu_config_input(DOUT7);mcu_config_pullup(DOUT7)
#endif
#define io54_config_pullup mcu_config_pullup(DOUT7)
#define io54_get_input mcu_get_input(DOUT7)
#elif ASSERT_PIN_EXTENDED(DOUT7)
#define io54_config_output
#define io54_set_output ic74hc595_set_pin(DOUT7);ic74hc595_shift_io_pins()
#define io54_clear_output ic74hc595_clear_pin(DOUT7);ic74hc595_shift_io_pins()
#define io54_toggle_output ic74hc595_toggle_pin(DOUT7);ic74hc595_shift_io_pins()
#define io54_get_output ic74hc595_get_pin(DOUT7)
#define io54_config_input
#define io54_config_pullup
#define io54_get_input 0
#else
#define io54_config_output
#define io54_set_output
#define io54_clear_output
#define io54_toggle_output
#define io54_get_output 0
#define io54_config_input
#define io54_config_pullup
#define io54_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT8)
#define io55_config_output mcu_config_output(DOUT8)
#define io55_set_output mcu_set_output(DOUT8)
#define io55_clear_output mcu_clear_output(DOUT8)
#define io55_toggle_output mcu_toggle_output(DOUT8)
#define io55_get_output mcu_get_output(DOUT8)
#if !defined(DOUT8_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io55_config_input mcu_config_input(DOUT8)
#else
#define io55_config_input mcu_config_input(DOUT8);mcu_config_pullup(DOUT8)
#endif
#define io55_config_pullup mcu_config_pullup(DOUT8)
#define io55_get_input mcu_get_input(DOUT8)
#elif ASSERT_PIN_EXTENDED(DOUT8)
#define io55_config_output
#define io55_set_output ic74hc595_set_pin(DOUT8);ic74hc595_shift_io_pins()
#define io55_clear_output ic74hc595_clear_pin(DOUT8);ic74hc595_shift_io_pins()
#define io55_toggle_output ic74hc595_toggle_pin(DOUT8);ic74hc595_shift_io_pins()
#define io55_get_output ic74hc595_get_pin(DOUT8)
#define io55_config_input
#define io55_config_pullup
#define io55_get_input 0
#else
#define io55_config_output
#define io55_set_output
#define io55_clear_output
#define io55_toggle_output
#define io55_get_output 0
#define io55_config_input
#define io55_config_pullup
#define io55_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT9)
#define io56_config_output mcu_config_output(DOUT9)
#define io56_set_output mcu_set_output(DOUT9)
#define io56_clear_output mcu_clear_output(DOUT9)
#define io56_toggle_output mcu_toggle_output(DOUT9)
#define io56_get_output mcu_get_output(DOUT9)
#if !defined(DOUT9_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io56_config_input mcu_config_input(DOUT9)
#else
#define io56_config_input mcu_config_input(DOUT9);mcu_config_pullup(DOUT9)
#endif
#define io56_config_pullup mcu_config_pullup(DOUT9)
#define io56_get_input mcu_get_input(DOUT9)
#elif ASSERT_PIN_EXTENDED(DOUT9)
#define io56_config_output
#define io56_set_output ic74hc595_set_pin(DOUT9);ic74hc595_shift_io_pins()
#define io56_clear_output ic74hc595_clear_pin(DOUT9);ic74hc595_shift_io_pins()
#define io56_toggle_output ic74hc595_toggle_pin(DOUT9);ic74hc595_shift_io_pins()
#define io56_get_output ic74hc595_get_pin(DOUT9)
#define io56_config_input
#define io56_config_pullup
#define io56_get_input 0
#else
#define io56_config_output
#define io56_set_output
#define io56_clear_output
#define io56_toggle_output
#define io56_get_output 0
#define io56_config_input
#define io56_config_pullup
#define io56_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT10)
#define io57_config_output mcu_config_output(DOUT10)
#define io57_set_output mcu_set_output(DOUT10)
#define io57_clear_output mcu_clear_output(DOUT10)
#define io57_toggle_output mcu_toggle_output(DOUT10)
#define io57_get_output mcu_get_output(DOUT10)
#if !defined(DOUT10_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io57_config_input mcu_config_input(DOUT10)
#else
#define io57_config_input mcu_config_input(DOUT10);mcu_config_pullup(DOUT10)
#endif
#define io57_config_pullup mcu_config_pullup(DOUT10)
#define io57_get_input mcu_get_input(DOUT10)
#elif ASSERT_PIN_EXTENDED(DOUT10)
#define io57_config_output
#define io57_set_output ic74hc595_set_pin(DOUT10);ic74hc595_shift_io_pins()
#define io57_clear_output ic74hc595_clear_pin(DOUT10);ic74hc595_shift_io_pins()
#define io57_toggle_output ic74hc595_toggle_pin(DOUT10);ic74hc595_shift_io_pins()
#define io57_get_output ic74hc595_get_pin(DOUT10)
#define io57_config_input
#define io57_config_pullup
#define io57_get_input 0
#else
#define io57_config_output
#define io57_set_output
#define io57_clear_output
#define io57_toggle_output
#define io57_get_output 0
#define io57_config_input
#define io57_config_pullup
#define io57_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT11)
#define io58_config_output mcu_config_output(DOUT11)
#define io58_set_output mcu_set_output(DOUT11)
#define io58_clear_output mcu_clear_output(DOUT11)
#define io58_toggle_output mcu_toggle_output(DOUT11)
#define io58_get_output mcu_get_output(DOUT11)
#if !defined(DOUT11_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io58_config_input mcu_config_input(DOUT11)
#else
#define io58_config_input mcu_config_input(DOUT11);mcu_config_pullup(DOUT11)
#endif
#define io58_config_pullup mcu_config_pullup(DOUT11)
#define io58_get_input mcu_get_input(DOUT11)
#elif ASSERT_PIN_EXTENDED(DOUT11)
#define io58_config_output
#define io58_set_output ic74hc595_set_pin(DOUT11);ic74hc595_shift_io_pins()
#define io58_clear_output ic74hc595_clear_pin(DOUT11);ic74hc595_shift_io_pins()
#define io58_toggle_output ic74hc595_toggle_pin(DOUT11);ic74hc595_shift_io_pins()
#define io58_get_output ic74hc595_get_pin(DOUT11)
#define io58_config_input
#define io58_config_pullup
#define io58_get_input 0
#else
#define io58_config_output
#define io58_set_output
#define io58_clear_output
#define io58_toggle_output
#define io58_get_output 0
#define io58_config_input
#define io58_config_pullup
#define io58_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT12)
#define io59_config_output mcu_config_output(DOUT12)
#define io59_set_output mcu_set_output(DOUT12)
#define io59_clear_output mcu_clear_output(DOUT12)
#define io59_toggle_output mcu_toggle_output(DOUT12)
#define io59_get_output mcu_get_output(DOUT12)
#if !defined(DOUT12_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io59_config_input mcu_config_input(DOUT12)
#else
#define io59_config_input mcu_config_input(DOUT12);mcu_config_pullup(DOUT12)
#endif
#define io59_config_pullup mcu_config_pullup(DOUT12)
#define io59_get_input mcu_get_input(DOUT12)
#elif ASSERT_PIN_EXTENDED(DOUT12)
#define io59_config_output
#define io59_set_output ic74hc595_set_pin(DOUT12);ic74hc595_shift_io_pins()
#define io59_clear_output ic74hc595_clear_pin(DOUT12);ic74hc595_shift_io_pins()
#define io59_toggle_output ic74hc595_toggle_pin(DOUT12);ic74hc595_shift_io_pins()
#define io59_get_output ic74hc595_get_pin(DOUT12)
#define io59_config_input
#define io59_config_pullup
#define io59_get_input 0
#else
#define io59_config_output
#define io59_set_output
#define io59_clear_output
#define io59_toggle_output
#define io59_get_output 0
#define io59_config_input
#define io59_config_pullup
#define io59_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT13)
#define io60_config_output mcu_config_output(DOUT13)
#define io60_set_output mcu_set_output(DOUT13)
#define io60_clear_output mcu_clear_output(DOUT13)
#define io60_toggle_output mcu_toggle_output(DOUT13)
#define io60_get_output mcu_get_output(DOUT13)
#if !defined(DOUT13_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io60_config_input mcu_config_input(DOUT13)
#else
#define io60_config_input mcu_config_input(DOUT13);mcu_config_pullup(DOUT13)
#endif
#define io60_config_pullup mcu_config_pullup(DOUT13)
#define io60_get_input mcu_get_input(DOUT13)
#elif ASSERT_PIN_EXTENDED(DOUT13)
#define io60_config_output
#define io60_set_output ic74hc595_set_pin(DOUT13);ic74hc595_shift_io_pins()
#define io60_clear_output ic74hc595_clear_pin(DOUT13);ic74hc595_shift_io_pins()
#define io60_toggle_output ic74hc595_toggle_pin(DOUT13);ic74hc595_shift_io_pins()
#define io60_get_output ic74hc595_get_pin(DOUT13)
#define io60_config_input
#define io60_config_pullup
#define io60_get_input 0
#else
#define io60_config_output
#define io60_set_output
#define io60_clear_output
#define io60_toggle_output
#define io60_get_output 0
#define io60_config_input
#define io60_config_pullup
#define io60_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT14)
#define io61_config_output mcu_config_output(DOUT14)
#define io61_set_output mcu_set_output(DOUT14)
#define io61_clear_output mcu_clear_output(DOUT14)
#define io61_toggle_output mcu_toggle_output(DOUT14)
#define io61_get_output mcu_get_output(DOUT14)
#if !defined(DOUT14_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io61_config_input mcu_config_input(DOUT14)
#else
#define io61_config_input mcu_config_input(DOUT14);mcu_config_pullup(DOUT14)
#endif
#define io61_config_pullup mcu_config_pullup(DOUT14)
#define io61_get_input mcu_get_input(DOUT14)
#elif ASSERT_PIN_EXTENDED(DOUT14)
#define io61_config_output
#define io61_set_output ic74hc595_set_pin(DOUT14);ic74hc595_shift_io_pins()
#define io61_clear_output ic74hc595_clear_pin(DOUT14);ic74hc595_shift_io_pins()
#define io61_toggle_output ic74hc595_toggle_pin(DOUT14);ic74hc595_shift_io_pins()
#define io61_get_output ic74hc595_get_pin(DOUT14)
#define io61_config_input
#define io61_config_pullup
#define io61_get_input 0
#else
#define io61_config_output
#define io61_set_output
#define io61_clear_output
#define io61_toggle_output
#define io61_get_output 0
#define io61_config_input
#define io61_config_pullup
#define io61_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT15)
#define io62_config_output mcu_config_output(DOUT15)
#define io62_set_output mcu_set_output(DOUT15)
#define io62_clear_output mcu_clear_output(DOUT15)
#define io62_toggle_output mcu_toggle_output(DOUT15)
#define io62_get_output mcu_get_output(DOUT15)
#if !defined(DOUT15_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io62_config_input mcu_config_input(DOUT15)
#else
#define io62_config_input mcu_config_input(DOUT15);mcu_config_pullup(DOUT15)
#endif
#define io62_config_pullup mcu_config_pullup(DOUT15)
#define io62_get_input mcu_get_input(DOUT15)
#elif ASSERT_PIN_EXTENDED(DOUT15)
#define io62_config_output
#define io62_set_output ic74hc595_set_pin(DOUT15);ic74hc595_shift_io_pins()
#define io62_clear_output ic74hc595_clear_pin(DOUT15);ic74hc595_shift_io_pins()
#define io62_toggle_output ic74hc595_toggle_pin(DOUT15);ic74hc595_shift_io_pins()
#define io62_get_output ic74hc595_get_pin(DOUT15)
#define io62_config_input
#define io62_config_pullup
#define io62_get_input 0
#else
#define io62_config_output
#define io62_set_output
#define io62_clear_output
#define io62_toggle_output
#define io62_get_output 0
#define io62_config_input
#define io62_config_pullup
#define io62_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT16)
#define io63_config_output mcu_config_output(DOUT16)
#define io63_set_output mcu_set_output(DOUT16)
#define io63_clear_output mcu_clear_output(DOUT16)
#define io63_toggle_output mcu_toggle_output(DOUT16)
#define io63_get_output mcu_get_output(DOUT16)
#if !defined(DOUT16_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io63_config_input mcu_config_input(DOUT16)
#else
#define io63_config_input mcu_config_input(DOUT16);mcu_config_pullup(DOUT16)
#endif
#define io63_config_pullup mcu_config_pullup(DOUT16)
#define io63_get_input mcu_get_input(DOUT16)
#elif ASSERT_PIN_EXTENDED(DOUT16)
#define io63_config_output
#define io63_set_output ic74hc595_set_pin(DOUT16);ic74hc595_shift_io_pins()
#define io63_clear_output ic74hc595_clear_pin(DOUT16);ic74hc595_shift_io_pins()
#define io63_toggle_output ic74hc595_toggle_pin(DOUT16);ic74hc595_shift_io_pins()
#define io63_get_output ic74hc595_get_pin(DOUT16)
#define io63_config_input
#define io63_config_pullup
#define io63_get_input 0
#else
#define io63_config_output
#define io63_set_output
#define io63_clear_output
#define io63_toggle_output
#define io63_get_output 0
#define io63_config_input
#define io63_config_pullup
#define io63_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT17)
#define io64_config_output mcu_config_output(DOUT17)
#define io64_set_output mcu_set_output(DOUT17)
#define io64_clear_output mcu_clear_output(DOUT17)
#define io64_toggle_output mcu_toggle_output(DOUT17)
#define io64_get_output mcu_get_output(DOUT17)
#if !defined(DOUT17_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io64_config_input mcu_config_input(DOUT17)
#else
#define io64_config_input mcu_config_input(DOUT17);mcu_config_pullup(DOUT17)
#endif
#define io64_config_pullup mcu_config_pullup(DOUT17)
#define io64_get_input mcu_get_input(DOUT17)
#elif ASSERT_PIN_EXTENDED(DOUT17)
#define io64_config_output
#define io64_set_output ic74hc595_set_pin(DOUT17);ic74hc595_shift_io_pins()
#define io64_clear_output ic74hc595_clear_pin(DOUT17);ic74hc595_shift_io_pins()
#define io64_toggle_output ic74hc595_toggle_pin(DOUT17);ic74hc595_shift_io_pins()
#define io64_get_output ic74hc595_get_pin(DOUT17)
#define io64_config_input
#define io64_config_pullup
#define io64_get_input 0
#else
#define io64_config_output
#define io64_set_output
#define io64_clear_output
#define io64_toggle_output
#define io64_get_output 0
#define io64_config_input
#define io64_config_pullup
#define io64_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT18)
#define io65_config_output mcu_config_output(DOUT18)
#define io65_set_output mcu_set_output(DOUT18)
#define io65_clear_output mcu_clear_output(DOUT18)
#define io65_toggle_output mcu_toggle_output(DOUT18)
#define io65_get_output mcu_get_output(DOUT18)
#if !defined(DOUT18_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io65_config_input mcu_config_input(DOUT18)
#else
#define io65_config_input mcu_config_input(DOUT18);mcu_config_pullup(DOUT18)
#endif
#define io65_config_pullup mcu_config_pullup(DOUT18)
#define io65_get_input mcu_get_input(DOUT18)
#elif ASSERT_PIN_EXTENDED(DOUT18)
#define io65_config_output
#define io65_set_output ic74hc595_set_pin(DOUT18);ic74hc595_shift_io_pins()
#define io65_clear_output ic74hc595_clear_pin(DOUT18);ic74hc595_shift_io_pins()
#define io65_toggle_output ic74hc595_toggle_pin(DOUT18);ic74hc595_shift_io_pins()
#define io65_get_output ic74hc595_get_pin(DOUT18)
#define io65_config_input
#define io65_config_pullup
#define io65_get_input 0
#else
#define io65_config_output
#define io65_set_output
#define io65_clear_output
#define io65_toggle_output
#define io65_get_output 0
#define io65_config_input
#define io65_config_pullup
#define io65_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT19)
#define io66_config_output mcu_config_output(DOUT19)
#define io66_set_output mcu_set_output(DOUT19)
#define io66_clear_output mcu_clear_output(DOUT19)
#define io66_toggle_output mcu_toggle_output(DOUT19)
#define io66_get_output mcu_get_output(DOUT19)
#if !defined(DOUT19_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io66_config_input mcu_config_input(DOUT19)
#else
#define io66_config_input mcu_config_input(DOUT19);mcu_config_pullup(DOUT19)
#endif
#define io66_config_pullup mcu_config_pullup(DOUT19)
#define io66_get_input mcu_get_input(DOUT19)
#elif ASSERT_PIN_EXTENDED(DOUT19)
#define io66_config_output
#define io66_set_output ic74hc595_set_pin(DOUT19);ic74hc595_shift_io_pins()
#define io66_clear_output ic74hc595_clear_pin(DOUT19);ic74hc595_shift_io_pins()
#define io66_toggle_output ic74hc595_toggle_pin(DOUT19);ic74hc595_shift_io_pins()
#define io66_get_output ic74hc595_get_pin(DOUT19)
#define io66_config_input
#define io66_config_pullup
#define io66_get_input 0
#else
#define io66_config_output
#define io66_set_output
#define io66_clear_output
#define io66_toggle_output
#define io66_get_output 0
#define io66_config_input
#define io66_config_pullup
#define io66_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT20)
#define io67_config_output mcu_config_output(DOUT20)
#define io67_set_output mcu_set_output(DOUT20)
#define io67_clear_output mcu_clear_output(DOUT20)
#define io67_toggle_output mcu_toggle_output(DOUT20)
#define io67_get_output mcu_get_output(DOUT20)
#if !defined(DOUT20_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io67_config_input mcu_config_input(DOUT20)
#else
#define io67_config_input mcu_config_input(DOUT20);mcu_config_pullup(DOUT20)
#endif
#define io67_config_pullup mcu_config_pullup(DOUT20)
#define io67_get_input mcu_get_input(DOUT20)
#elif ASSERT_PIN_EXTENDED(DOUT20)
#define io67_config_output
#define io67_set_output ic74hc595_set_pin(DOUT20);ic74hc595_shift_io_pins()
#define io67_clear_output ic74hc595_clear_pin(DOUT20);ic74hc595_shift_io_pins()
#define io67_toggle_output ic74hc595_toggle_pin(DOUT20);ic74hc595_shift_io_pins()
#define io67_get_output ic74hc595_get_pin(DOUT20)
#define io67_config_input
#define io67_config_pullup
#define io67_get_input 0
#else
#define io67_config_output
#define io67_set_output
#define io67_clear_output
#define io67_toggle_output
#define io67_get_output 0
#define io67_config_input
#define io67_config_pullup
#define io67_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT21)
#define io68_config_output mcu_config_output(DOUT21)
#define io68_set_output mcu_set_output(DOUT21)
#define io68_clear_output mcu_clear_output(DOUT21)
#define io68_toggle_output mcu_toggle_output(DOUT21)
#define io68_get_output mcu_get_output(DOUT21)
#if !defined(DOUT21_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io68_config_input mcu_config_input(DOUT21)
#else
#define io68_config_input mcu_config_input(DOUT21);mcu_config_pullup(DOUT21)
#endif
#define io68_config_pullup mcu_config_pullup(DOUT21)
#define io68_get_input mcu_get_input(DOUT21)
#elif ASSERT_PIN_EXTENDED(DOUT21)
#define io68_config_output
#define io68_set_output ic74hc595_set_pin(DOUT21);ic74hc595_shift_io_pins()
#define io68_clear_output ic74hc595_clear_pin(DOUT21);ic74hc595_shift_io_pins()
#define io68_toggle_output ic74hc595_toggle_pin(DOUT21);ic74hc595_shift_io_pins()
#define io68_get_output ic74hc595_get_pin(DOUT21)
#define io68_config_input
#define io68_config_pullup
#define io68_get_input 0
#else
#define io68_config_output
#define io68_set_output
#define io68_clear_output
#define io68_toggle_output
#define io68_get_output 0
#define io68_config_input
#define io68_config_pullup
#define io68_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT22)
#define io69_config_output mcu_config_output(DOUT22)
#define io69_set_output mcu_set_output(DOUT22)
#define io69_clear_output mcu_clear_output(DOUT22)
#define io69_toggle_output mcu_toggle_output(DOUT22)
#define io69_get_output mcu_get_output(DOUT22)
#if !defined(DOUT22_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io69_config_input mcu_config_input(DOUT22)
#else
#define io69_config_input mcu_config_input(DOUT22);mcu_config_pullup(DOUT22)
#endif
#define io69_config_pullup mcu_config_pullup(DOUT22)
#define io69_get_input mcu_get_input(DOUT22)
#elif ASSERT_PIN_EXTENDED(DOUT22)
#define io69_config_output
#define io69_set_output ic74hc595_set_pin(DOUT22);ic74hc595_shift_io_pins()
#define io69_clear_output ic74hc595_clear_pin(DOUT22);ic74hc595_shift_io_pins()
#define io69_toggle_output ic74hc595_toggle_pin(DOUT22);ic74hc595_shift_io_pins()
#define io69_get_output ic74hc595_get_pin(DOUT22)
#define io69_config_input
#define io69_config_pullup
#define io69_get_input 0
#else
#define io69_config_output
#define io69_set_output
#define io69_clear_output
#define io69_toggle_output
#define io69_get_output 0
#define io69_config_input
#define io69_config_pullup
#define io69_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT23)
#define io70_config_output mcu_config_output(DOUT23)
#define io70_set_output mcu_set_output(DOUT23)
#define io70_clear_output mcu_clear_output(DOUT23)
#define io70_toggle_output mcu_toggle_output(DOUT23)
#define io70_get_output mcu_get_output(DOUT23)
#if !defined(DOUT23_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io70_config_input mcu_config_input(DOUT23)
#else
#define io70_config_input mcu_config_input(DOUT23);mcu_config_pullup(DOUT23)
#endif
#define io70_config_pullup mcu_config_pullup(DOUT23)
#define io70_get_input mcu_get_input(DOUT23)
#elif ASSERT_PIN_EXTENDED(DOUT23)
#define io70_config_output
#define io70_set_output ic74hc595_set_pin(DOUT23);ic74hc595_shift_io_pins()
#define io70_clear_output ic74hc595_clear_pin(DOUT23);ic74hc595_shift_io_pins()
#define io70_toggle_output ic74hc595_toggle_pin(DOUT23);ic74hc595_shift_io_pins()
#define io70_get_output ic74hc595_get_pin(DOUT23)
#define io70_config_input
#define io70_config_pullup
#define io70_get_input 0
#else
#define io70_config_output
#define io70_set_output
#define io70_clear_output
#define io70_toggle_output
#define io70_get_output 0
#define io70_config_input
#define io70_config_pullup
#define io70_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT24)
#define io71_config_output mcu_config_output(DOUT24)
#define io71_set_output mcu_set_output(DOUT24)
#define io71_clear_output mcu_clear_output(DOUT24)
#define io71_toggle_output mcu_toggle_output(DOUT24)
#define io71_get_output mcu_get_output(DOUT24)
#if !defined(DOUT24_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io71_config_input mcu_config_input(DOUT24)
#else
#define io71_config_input mcu_config_input(DOUT24);mcu_config_pullup(DOUT24)
#endif
#define io71_config_pullup mcu_config_pullup(DOUT24)
#define io71_get_input mcu_get_input(DOUT24)
#elif ASSERT_PIN_EXTENDED(DOUT24)
#define io71_config_output
#define io71_set_output ic74hc595_set_pin(DOUT24);ic74hc595_shift_io_pins()
#define io71_clear_output ic74hc595_clear_pin(DOUT24);ic74hc595_shift_io_pins()
#define io71_toggle_output ic74hc595_toggle_pin(DOUT24);ic74hc595_shift_io_pins()
#define io71_get_output ic74hc595_get_pin(DOUT24)
#define io71_config_input
#define io71_config_pullup
#define io71_get_input 0
#else
#define io71_config_output
#define io71_set_output
#define io71_clear_output
#define io71_toggle_output
#define io71_get_output 0
#define io71_config_input
#define io71_config_pullup
#define io71_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT25)
#define io72_config_output mcu_config_output(DOUT25)
#define io72_set_output mcu_set_output(DOUT25)
#define io72_clear_output mcu_clear_output(DOUT25)
#define io72_toggle_output mcu_toggle_output(DOUT25)
#define io72_get_output mcu_get_output(DOUT25)
#if !defined(DOUT25_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io72_config_input mcu_config_input(DOUT25)
#else
#define io72_config_input mcu_config_input(DOUT25);mcu_config_pullup(DOUT25)
#endif
#define io72_config_pullup mcu_config_pullup(DOUT25)
#define io72_get_input mcu_get_input(DOUT25)
#elif ASSERT_PIN_EXTENDED(DOUT25)
#define io72_config_output
#define io72_set_output ic74hc595_set_pin(DOUT25);ic74hc595_shift_io_pins()
#define io72_clear_output ic74hc595_clear_pin(DOUT25);ic74hc595_shift_io_pins()
#define io72_toggle_output ic74hc595_toggle_pin(DOUT25);ic74hc595_shift_io_pins()
#define io72_get_output ic74hc595_get_pin(DOUT25)
#define io72_config_input
#define io72_config_pullup
#define io72_get_input 0
#else
#define io72_config_output
#define io72_set_output
#define io72_clear_output
#define io72_toggle_output
#define io72_get_output 0
#define io72_config_input
#define io72_config_pullup
#define io72_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT26)
#define io73_config_output mcu_config_output(DOUT26)
#define io73_set_output mcu_set_output(DOUT26)
#define io73_clear_output mcu_clear_output(DOUT26)
#define io73_toggle_output mcu_toggle_output(DOUT26)
#define io73_get_output mcu_get_output(DOUT26)
#if !defined(DOUT26_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io73_config_input mcu_config_input(DOUT26)
#else
#define io73_config_input mcu_config_input(DOUT26);mcu_config_pullup(DOUT26)
#endif
#define io73_config_pullup mcu_config_pullup(DOUT26)
#define io73_get_input mcu_get_input(DOUT26)
#elif ASSERT_PIN_EXTENDED(DOUT26)
#define io73_config_output
#define io73_set_output ic74hc595_set_pin(DOUT26);ic74hc595_shift_io_pins()
#define io73_clear_output ic74hc595_clear_pin(DOUT26);ic74hc595_shift_io_pins()
#define io73_toggle_output ic74hc595_toggle_pin(DOUT26);ic74hc595_shift_io_pins()
#define io73_get_output ic74hc595_get_pin(DOUT26)
#define io73_config_input
#define io73_config_pullup
#define io73_get_input 0
#else
#define io73_config_output
#define io73_set_output
#define io73_clear_output
#define io73_toggle_output
#define io73_get_output 0
#define io73_config_input
#define io73_config_pullup
#define io73_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT27)
#define io74_config_output mcu_config_output(DOUT27)
#define io74_set_output mcu_set_output(DOUT27)
#define io74_clear_output mcu_clear_output(DOUT27)
#define io74_toggle_output mcu_toggle_output(DOUT27)
#define io74_get_output mcu_get_output(DOUT27)
#if !defined(DOUT27_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io74_config_input mcu_config_input(DOUT27)
#else
#define io74_config_input mcu_config_input(DOUT27);mcu_config_pullup(DOUT27)
#endif
#define io74_config_pullup mcu_config_pullup(DOUT27)
#define io74_get_input mcu_get_input(DOUT27)
#elif ASSERT_PIN_EXTENDED(DOUT27)
#define io74_config_output
#define io74_set_output ic74hc595_set_pin(DOUT27);ic74hc595_shift_io_pins()
#define io74_clear_output ic74hc595_clear_pin(DOUT27);ic74hc595_shift_io_pins()
#define io74_toggle_output ic74hc595_toggle_pin(DOUT27);ic74hc595_shift_io_pins()
#define io74_get_output ic74hc595_get_pin(DOUT27)
#define io74_config_input
#define io74_config_pullup
#define io74_get_input 0
#else
#define io74_config_output
#define io74_set_output
#define io74_clear_output
#define io74_toggle_output
#define io74_get_output 0
#define io74_config_input
#define io74_config_pullup
#define io74_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT28)
#define io75_config_output mcu_config_output(DOUT28)
#define io75_set_output mcu_set_output(DOUT28)
#define io75_clear_output mcu_clear_output(DOUT28)
#define io75_toggle_output mcu_toggle_output(DOUT28)
#define io75_get_output mcu_get_output(DOUT28)
#if !defined(DOUT28_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io75_config_input mcu_config_input(DOUT28)
#else
#define io75_config_input mcu_config_input(DOUT28);mcu_config_pullup(DOUT28)
#endif
#define io75_config_pullup mcu_config_pullup(DOUT28)
#define io75_get_input mcu_get_input(DOUT28)
#elif ASSERT_PIN_EXTENDED(DOUT28)
#define io75_config_output
#define io75_set_output ic74hc595_set_pin(DOUT28);ic74hc595_shift_io_pins()
#define io75_clear_output ic74hc595_clear_pin(DOUT28);ic74hc595_shift_io_pins()
#define io75_toggle_output ic74hc595_toggle_pin(DOUT28);ic74hc595_shift_io_pins()
#define io75_get_output ic74hc595_get_pin(DOUT28)
#define io75_config_input
#define io75_config_pullup
#define io75_get_input 0
#else
#define io75_config_output
#define io75_set_output
#define io75_clear_output
#define io75_toggle_output
#define io75_get_output 0
#define io75_config_input
#define io75_config_pullup
#define io75_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT29)
#define io76_config_output mcu_config_output(DOUT29)
#define io76_set_output mcu_set_output(DOUT29)
#define io76_clear_output mcu_clear_output(DOUT29)
#define io76_toggle_output mcu_toggle_output(DOUT29)
#define io76_get_output mcu_get_output(DOUT29)
#if !defined(DOUT29_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io76_config_input mcu_config_input(DOUT29)
#else
#define io76_config_input mcu_config_input(DOUT29);mcu_config_pullup(DOUT29)
#endif
#define io76_config_pullup mcu_config_pullup(DOUT29)
#define io76_get_input mcu_get_input(DOUT29)
#elif ASSERT_PIN_EXTENDED(DOUT29)
#define io76_config_output
#define io76_set_output ic74hc595_set_pin(DOUT29);ic74hc595_shift_io_pins()
#define io76_clear_output ic74hc595_clear_pin(DOUT29);ic74hc595_shift_io_pins()
#define io76_toggle_output ic74hc595_toggle_pin(DOUT29);ic74hc595_shift_io_pins()
#define io76_get_output ic74hc595_get_pin(DOUT29)
#define io76_config_input
#define io76_config_pullup
#define io76_get_input 0
#else
#define io76_config_output
#define io76_set_output
#define io76_clear_output
#define io76_toggle_output
#define io76_get_output 0
#define io76_config_input
#define io76_config_pullup
#define io76_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT30)
#define io77_config_output mcu_config_output(DOUT30)
#define io77_set_output mcu_set_output(DOUT30)
#define io77_clear_output mcu_clear_output(DOUT30)
#define io77_toggle_output mcu_toggle_output(DOUT30)
#define io77_get_output mcu_get_output(DOUT30)
#if !defined(DOUT30_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io77_config_input mcu_config_input(DOUT30)
#else
#define io77_config_input mcu_config_input(DOUT30);mcu_config_pullup(DOUT30)
#endif
#define io77_config_pullup mcu_config_pullup(DOUT30)
#define io77_get_input mcu_get_input(DOUT30)
#elif ASSERT_PIN_EXTENDED(DOUT30)
#define io77_config_output
#define io77_set_output ic74hc595_set_pin(DOUT30);ic74hc595_shift_io_pins()
#define io77_clear_output ic74hc595_clear_pin(DOUT30);ic74hc595_shift_io_pins()
#define io77_toggle_output ic74hc595_toggle_pin(DOUT30);ic74hc595_shift_io_pins()
#define io77_get_output ic74hc595_get_pin(DOUT30)
#define io77_config_input
#define io77_config_pullup
#define io77_get_input 0
#else
#define io77_config_output
#define io77_set_output
#define io77_clear_output
#define io77_toggle_output
#define io77_get_output 0
#define io77_config_input
#define io77_config_pullup
#define io77_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT31)
#define io78_config_output mcu_config_output(DOUT31)
#define io78_set_output mcu_set_output(DOUT31)
#define io78_clear_output mcu_clear_output(DOUT31)
#define io78_toggle_output mcu_toggle_output(DOUT31)
#define io78_get_output mcu_get_output(DOUT31)
#if !defined(DOUT31_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io78_config_input mcu_config_input(DOUT31)
#else
#define io78_config_input mcu_config_input(DOUT31);mcu_config_pullup(DOUT31)
#endif
#define io78_config_pullup mcu_config_pullup(DOUT31)
#define io78_get_input mcu_get_input(DOUT31)
#elif ASSERT_PIN_EXTENDED(DOUT31)
#define io78_config_output
#define io78_set_output ic74hc595_set_pin(DOUT31);ic74hc595_shift_io_pins()
#define io78_clear_output ic74hc595_clear_pin(DOUT31);ic74hc595_shift_io_pins()
#define io78_toggle_output ic74hc595_toggle_pin(DOUT31);ic74hc595_shift_io_pins()
#define io78_get_output ic74hc595_get_pin(DOUT31)
#define io78_config_input
#define io78_config_pullup
#define io78_get_input 0
#else
#define io78_config_output
#define io78_set_output
#define io78_clear_output
#define io78_toggle_output
#define io78_get_output 0
#define io78_config_input
#define io78_config_pullup
#define io78_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT32)
#define io79_config_output mcu_config_output(DOUT32)
#define io79_set_output mcu_set_output(DOUT32)
#define io79_clear_output mcu_clear_output(DOUT32)
#define io79_toggle_output mcu_toggle_output(DOUT32)
#define io79_get_output mcu_get_output(DOUT32)
#if !defined(DOUT32_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io79_config_input mcu_config_input(DOUT32)
#else
#define io79_config_input mcu_config_input(DOUT32);mcu_config_pullup(DOUT32)
#endif
#define io79_config_pullup mcu_config_pullup(DOUT32)
#define io79_get_input mcu_get_input(DOUT32)
#elif ASSERT_PIN_EXTENDED(DOUT32)
#define io79_config_output
#define io79_set_output ic74hc595_set_pin(DOUT32);ic74hc595_shift_io_pins()
#define io79_clear_output ic74hc595_clear_pin(DOUT32);ic74hc595_shift_io_pins()
#define io79_toggle_output ic74hc595_toggle_pin(DOUT32);ic74hc595_shift_io_pins()
#define io79_get_output ic74hc595_get_pin(DOUT32)
#define io79_config_input
#define io79_config_pullup
#define io79_get_input 0
#else
#define io79_config_output
#define io79_set_output
#define io79_clear_output
#define io79_toggle_output
#define io79_get_output 0
#define io79_config_input
#define io79_config_pullup
#define io79_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT33)
#define io80_config_output mcu_config_output(DOUT33)
#define io80_set_output mcu_set_output(DOUT33)
#define io80_clear_output mcu_clear_output(DOUT33)
#define io80_toggle_output mcu_toggle_output(DOUT33)
#define io80_get_output mcu_get_output(DOUT33)
#if !defined(DOUT33_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io80_config_input mcu_config_input(DOUT33)
#else
#define io80_config_input mcu_config_input(DOUT33);mcu_config_pullup(DOUT33)
#endif
#define io80_config_pullup mcu_config_pullup(DOUT33)
#define io80_get_input mcu_get_input(DOUT33)
#elif ASSERT_PIN_EXTENDED(DOUT33)
#define io80_config_output
#define io80_set_output ic74hc595_set_pin(DOUT33);ic74hc595_shift_io_pins()
#define io80_clear_output ic74hc595_clear_pin(DOUT33);ic74hc595_shift_io_pins()
#define io80_toggle_output ic74hc595_toggle_pin(DOUT33);ic74hc595_shift_io_pins()
#define io80_get_output ic74hc595_get_pin(DOUT33)
#define io80_config_input
#define io80_config_pullup
#define io80_get_input 0
#else
#define io80_config_output
#define io80_set_output
#define io80_clear_output
#define io80_toggle_output
#define io80_get_output 0
#define io80_config_input
#define io80_config_pullup
#define io80_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT34)
#define io81_config_output mcu_config_output(DOUT34)
#define io81_set_output mcu_set_output(DOUT34)
#define io81_clear_output mcu_clear_output(DOUT34)
#define io81_toggle_output mcu_toggle_output(DOUT34)
#define io81_get_output mcu_get_output(DOUT34)
#if !defined(DOUT34_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io81_config_input mcu_config_input(DOUT34)
#else
#define io81_config_input mcu_config_input(DOUT34);mcu_config_pullup(DOUT34)
#endif
#define io81_config_pullup mcu_config_pullup(DOUT34)
#define io81_get_input mcu_get_input(DOUT34)
#elif ASSERT_PIN_EXTENDED(DOUT34)
#define io81_config_output
#define io81_set_output ic74hc595_set_pin(DOUT34);ic74hc595_shift_io_pins()
#define io81_clear_output ic74hc595_clear_pin(DOUT34);ic74hc595_shift_io_pins()
#define io81_toggle_output ic74hc595_toggle_pin(DOUT34);ic74hc595_shift_io_pins()
#define io81_get_output ic74hc595_get_pin(DOUT34)
#define io81_config_input
#define io81_config_pullup
#define io81_get_input 0
#else
#define io81_config_output
#define io81_set_output
#define io81_clear_output
#define io81_toggle_output
#define io81_get_output 0
#define io81_config_input
#define io81_config_pullup
#define io81_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT35)
#define io82_config_output mcu_config_output(DOUT35)
#define io82_set_output mcu_set_output(DOUT35)
#define io82_clear_output mcu_clear_output(DOUT35)
#define io82_toggle_output mcu_toggle_output(DOUT35)
#define io82_get_output mcu_get_output(DOUT35)
#if !defined(DOUT35_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io82_config_input mcu_config_input(DOUT35)
#else
#define io82_config_input mcu_config_input(DOUT35);mcu_config_pullup(DOUT35)
#endif
#define io82_config_pullup mcu_config_pullup(DOUT35)
#define io82_get_input mcu_get_input(DOUT35)
#elif ASSERT_PIN_EXTENDED(DOUT35)
#define io82_config_output
#define io82_set_output ic74hc595_set_pin(DOUT35);ic74hc595_shift_io_pins()
#define io82_clear_output ic74hc595_clear_pin(DOUT35);ic74hc595_shift_io_pins()
#define io82_toggle_output ic74hc595_toggle_pin(DOUT35);ic74hc595_shift_io_pins()
#define io82_get_output ic74hc595_get_pin(DOUT35)
#define io82_config_input
#define io82_config_pullup
#define io82_get_input 0
#else
#define io82_config_output
#define io82_set_output
#define io82_clear_output
#define io82_toggle_output
#define io82_get_output 0
#define io82_config_input
#define io82_config_pullup
#define io82_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT36)
#define io83_config_output mcu_config_output(DOUT36)
#define io83_set_output mcu_set_output(DOUT36)
#define io83_clear_output mcu_clear_output(DOUT36)
#define io83_toggle_output mcu_toggle_output(DOUT36)
#define io83_get_output mcu_get_output(DOUT36)
#if !defined(DOUT36_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io83_config_input mcu_config_input(DOUT36)
#else
#define io83_config_input mcu_config_input(DOUT36);mcu_config_pullup(DOUT36)
#endif
#define io83_config_pullup mcu_config_pullup(DOUT36)
#define io83_get_input mcu_get_input(DOUT36)
#elif ASSERT_PIN_EXTENDED(DOUT36)
#define io83_config_output
#define io83_set_output ic74hc595_set_pin(DOUT36);ic74hc595_shift_io_pins()
#define io83_clear_output ic74hc595_clear_pin(DOUT36);ic74hc595_shift_io_pins()
#define io83_toggle_output ic74hc595_toggle_pin(DOUT36);ic74hc595_shift_io_pins()
#define io83_get_output ic74hc595_get_pin(DOUT36)
#define io83_config_input
#define io83_config_pullup
#define io83_get_input 0
#else
#define io83_config_output
#define io83_set_output
#define io83_clear_output
#define io83_toggle_output
#define io83_get_output 0
#define io83_config_input
#define io83_config_pullup
#define io83_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT37)
#define io84_config_output mcu_config_output(DOUT37)
#define io84_set_output mcu_set_output(DOUT37)
#define io84_clear_output mcu_clear_output(DOUT37)
#define io84_toggle_output mcu_toggle_output(DOUT37)
#define io84_get_output mcu_get_output(DOUT37)
#if !defined(DOUT37_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io84_config_input mcu_config_input(DOUT37)
#else
#define io84_config_input mcu_config_input(DOUT37);mcu_config_pullup(DOUT37)
#endif
#define io84_config_pullup mcu_config_pullup(DOUT37)
#define io84_get_input mcu_get_input(DOUT37)
#elif ASSERT_PIN_EXTENDED(DOUT37)
#define io84_config_output
#define io84_set_output ic74hc595_set_pin(DOUT37);ic74hc595_shift_io_pins()
#define io84_clear_output ic74hc595_clear_pin(DOUT37);ic74hc595_shift_io_pins()
#define io84_toggle_output ic74hc595_toggle_pin(DOUT37);ic74hc595_shift_io_pins()
#define io84_get_output ic74hc595_get_pin(DOUT37)
#define io84_config_input
#define io84_config_pullup
#define io84_get_input 0
#else
#define io84_config_output
#define io84_set_output
#define io84_clear_output
#define io84_toggle_output
#define io84_get_output 0
#define io84_config_input
#define io84_config_pullup
#define io84_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT38)
#define io85_config_output mcu_config_output(DOUT38)
#define io85_set_output mcu_set_output(DOUT38)
#define io85_clear_output mcu_clear_output(DOUT38)
#define io85_toggle_output mcu_toggle_output(DOUT38)
#define io85_get_output mcu_get_output(DOUT38)
#if !defined(DOUT38_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io85_config_input mcu_config_input(DOUT38)
#else
#define io85_config_input mcu_config_input(DOUT38);mcu_config_pullup(DOUT38)
#endif
#define io85_config_pullup mcu_config_pullup(DOUT38)
#define io85_get_input mcu_get_input(DOUT38)
#elif ASSERT_PIN_EXTENDED(DOUT38)
#define io85_config_output
#define io85_set_output ic74hc595_set_pin(DOUT38);ic74hc595_shift_io_pins()
#define io85_clear_output ic74hc595_clear_pin(DOUT38);ic74hc595_shift_io_pins()
#define io85_toggle_output ic74hc595_toggle_pin(DOUT38);ic74hc595_shift_io_pins()
#define io85_get_output ic74hc595_get_pin(DOUT38)
#define io85_config_input
#define io85_config_pullup
#define io85_get_input 0
#else
#define io85_config_output
#define io85_set_output
#define io85_clear_output
#define io85_toggle_output
#define io85_get_output 0
#define io85_config_input
#define io85_config_pullup
#define io85_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT39)
#define io86_config_output mcu_config_output(DOUT39)
#define io86_set_output mcu_set_output(DOUT39)
#define io86_clear_output mcu_clear_output(DOUT39)
#define io86_toggle_output mcu_toggle_output(DOUT39)
#define io86_get_output mcu_get_output(DOUT39)
#if !defined(DOUT39_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io86_config_input mcu_config_input(DOUT39)
#else
#define io86_config_input mcu_config_input(DOUT39);mcu_config_pullup(DOUT39)
#endif
#define io86_config_pullup mcu_config_pullup(DOUT39)
#define io86_get_input mcu_get_input(DOUT39)
#elif ASSERT_PIN_EXTENDED(DOUT39)
#define io86_config_output
#define io86_set_output ic74hc595_set_pin(DOUT39);ic74hc595_shift_io_pins()
#define io86_clear_output ic74hc595_clear_pin(DOUT39);ic74hc595_shift_io_pins()
#define io86_toggle_output ic74hc595_toggle_pin(DOUT39);ic74hc595_shift_io_pins()
#define io86_get_output ic74hc595_get_pin(DOUT39)
#define io86_config_input
#define io86_config_pullup
#define io86_get_input 0
#else
#define io86_config_output
#define io86_set_output
#define io86_clear_output
#define io86_toggle_output
#define io86_get_output 0
#define io86_config_input
#define io86_config_pullup
#define io86_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT40)
#define io87_config_output mcu_config_output(DOUT40)
#define io87_set_output mcu_set_output(DOUT40)
#define io87_clear_output mcu_clear_output(DOUT40)
#define io87_toggle_output mcu_toggle_output(DOUT40)
#define io87_get_output mcu_get_output(DOUT40)
#if !defined(DOUT40_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io87_config_input mcu_config_input(DOUT40)
#else
#define io87_config_input mcu_config_input(DOUT40);mcu_config_pullup(DOUT40)
#endif
#define io87_config_pullup mcu_config_pullup(DOUT40)
#define io87_get_input mcu_get_input(DOUT40)
#elif ASSERT_PIN_EXTENDED(DOUT40)
#define io87_config_output
#define io87_set_output ic74hc595_set_pin(DOUT40);ic74hc595_shift_io_pins()
#define io87_clear_output ic74hc595_clear_pin(DOUT40);ic74hc595_shift_io_pins()
#define io87_toggle_output ic74hc595_toggle_pin(DOUT40);ic74hc595_shift_io_pins()
#define io87_get_output ic74hc595_get_pin(DOUT40)
#define io87_config_input
#define io87_config_pullup
#define io87_get_input 0
#else
#define io87_config_output
#define io87_set_output
#define io87_clear_output
#define io87_toggle_output
#define io87_get_output 0
#define io87_config_input
#define io87_config_pullup
#define io87_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT41)
#define io88_config_output mcu_config_output(DOUT41)
#define io88_set_output mcu_set_output(DOUT41)
#define io88_clear_output mcu_clear_output(DOUT41)
#define io88_toggle_output mcu_toggle_output(DOUT41)
#define io88_get_output mcu_get_output(DOUT41)
#if !defined(DOUT41_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io88_config_input mcu_config_input(DOUT41)
#else
#define io88_config_input mcu_config_input(DOUT41);mcu_config_pullup(DOUT41)
#endif
#define io88_config_pullup mcu_config_pullup(DOUT41)
#define io88_get_input mcu_get_input(DOUT41)
#elif ASSERT_PIN_EXTENDED(DOUT41)
#define io88_config_output
#define io88_set_output ic74hc595_set_pin(DOUT41);ic74hc595_shift_io_pins()
#define io88_clear_output ic74hc595_clear_pin(DOUT41);ic74hc595_shift_io_pins()
#define io88_toggle_output ic74hc595_toggle_pin(DOUT41);ic74hc595_shift_io_pins()
#define io88_get_output ic74hc595_get_pin(DOUT41)
#define io88_config_input
#define io88_config_pullup
#define io88_get_input 0
#else
#define io88_config_output
#define io88_set_output
#define io88_clear_output
#define io88_toggle_output
#define io88_get_output 0
#define io88_config_input
#define io88_config_pullup
#define io88_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT42)
#define io89_config_output mcu_config_output(DOUT42)
#define io89_set_output mcu_set_output(DOUT42)
#define io89_clear_output mcu_clear_output(DOUT42)
#define io89_toggle_output mcu_toggle_output(DOUT42)
#define io89_get_output mcu_get_output(DOUT42)
#if !defined(DOUT42_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io89_config_input mcu_config_input(DOUT42)
#else
#define io89_config_input mcu_config_input(DOUT42);mcu_config_pullup(DOUT42)
#endif
#define io89_config_pullup mcu_config_pullup(DOUT42)
#define io89_get_input mcu_get_input(DOUT42)
#elif ASSERT_PIN_EXTENDED(DOUT42)
#define io89_config_output
#define io89_set_output ic74hc595_set_pin(DOUT42);ic74hc595_shift_io_pins()
#define io89_clear_output ic74hc595_clear_pin(DOUT42);ic74hc595_shift_io_pins()
#define io89_toggle_output ic74hc595_toggle_pin(DOUT42);ic74hc595_shift_io_pins()
#define io89_get_output ic74hc595_get_pin(DOUT42)
#define io89_config_input
#define io89_config_pullup
#define io89_get_input 0
#else
#define io89_config_output
#define io89_set_output
#define io89_clear_output
#define io89_toggle_output
#define io89_get_output 0
#define io89_config_input
#define io89_config_pullup
#define io89_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT43)
#define io90_config_output mcu_config_output(DOUT43)
#define io90_set_output mcu_set_output(DOUT43)
#define io90_clear_output mcu_clear_output(DOUT43)
#define io90_toggle_output mcu_toggle_output(DOUT43)
#define io90_get_output mcu_get_output(DOUT43)
#if !defined(DOUT43_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io90_config_input mcu_config_input(DOUT43)
#else
#define io90_config_input mcu_config_input(DOUT43);mcu_config_pullup(DOUT43)
#endif
#define io90_config_pullup mcu_config_pullup(DOUT43)
#define io90_get_input mcu_get_input(DOUT43)
#elif ASSERT_PIN_EXTENDED(DOUT43)
#define io90_config_output
#define io90_set_output ic74hc595_set_pin(DOUT43);ic74hc595_shift_io_pins()
#define io90_clear_output ic74hc595_clear_pin(DOUT43);ic74hc595_shift_io_pins()
#define io90_toggle_output ic74hc595_toggle_pin(DOUT43);ic74hc595_shift_io_pins()
#define io90_get_output ic74hc595_get_pin(DOUT43)
#define io90_config_input
#define io90_config_pullup
#define io90_get_input 0
#else
#define io90_config_output
#define io90_set_output
#define io90_clear_output
#define io90_toggle_output
#define io90_get_output 0
#define io90_config_input
#define io90_config_pullup
#define io90_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT44)
#define io91_config_output mcu_config_output(DOUT44)
#define io91_set_output mcu_set_output(DOUT44)
#define io91_clear_output mcu_clear_output(DOUT44)
#define io91_toggle_output mcu_toggle_output(DOUT44)
#define io91_get_output mcu_get_output(DOUT44)
#if !defined(DOUT44_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io91_config_input mcu_config_input(DOUT44)
#else
#define io91_config_input mcu_config_input(DOUT44);mcu_config_pullup(DOUT44)
#endif
#define io91_config_pullup mcu_config_pullup(DOUT44)
#define io91_get_input mcu_get_input(DOUT44)
#elif ASSERT_PIN_EXTENDED(DOUT44)
#define io91_config_output
#define io91_set_output ic74hc595_set_pin(DOUT44);ic74hc595_shift_io_pins()
#define io91_clear_output ic74hc595_clear_pin(DOUT44);ic74hc595_shift_io_pins()
#define io91_toggle_output ic74hc595_toggle_pin(DOUT44);ic74hc595_shift_io_pins()
#define io91_get_output ic74hc595_get_pin(DOUT44)
#define io91_config_input
#define io91_config_pullup
#define io91_get_input 0
#else
#define io91_config_output
#define io91_set_output
#define io91_clear_output
#define io91_toggle_output
#define io91_get_output 0
#define io91_config_input
#define io91_config_pullup
#define io91_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT45)
#define io92_config_output mcu_config_output(DOUT45)
#define io92_set_output mcu_set_output(DOUT45)
#define io92_clear_output mcu_clear_output(DOUT45)
#define io92_toggle_output mcu_toggle_output(DOUT45)
#define io92_get_output mcu_get_output(DOUT45)
#if !defined(DOUT45_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io92_config_input mcu_config_input(DOUT45)
#else
#define io92_config_input mcu_config_input(DOUT45);mcu_config_pullup(DOUT45)
#endif
#define io92_config_pullup mcu_config_pullup(DOUT45)
#define io92_get_input mcu_get_input(DOUT45)
#elif ASSERT_PIN_EXTENDED(DOUT45)
#define io92_config_output
#define io92_set_output ic74hc595_set_pin(DOUT45);ic74hc595_shift_io_pins()
#define io92_clear_output ic74hc595_clear_pin(DOUT45);ic74hc595_shift_io_pins()
#define io92_toggle_output ic74hc595_toggle_pin(DOUT45);ic74hc595_shift_io_pins()
#define io92_get_output ic74hc595_get_pin(DOUT45)
#define io92_config_input
#define io92_config_pullup
#define io92_get_input 0
#else
#define io92_config_output
#define io92_set_output
#define io92_clear_output
#define io92_toggle_output
#define io92_get_output 0
#define io92_config_input
#define io92_config_pullup
#define io92_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT46)
#define io93_config_output mcu_config_output(DOUT46)
#define io93_set_output mcu_set_output(DOUT46)
#define io93_clear_output mcu_clear_output(DOUT46)
#define io93_toggle_output mcu_toggle_output(DOUT46)
#define io93_get_output mcu_get_output(DOUT46)
#if !defined(DOUT46_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io93_config_input mcu_config_input(DOUT46)
#else
#define io93_config_input mcu_config_input(DOUT46);mcu_config_pullup(DOUT46)
#endif
#define io93_config_pullup mcu_config_pullup(DOUT46)
#define io93_get_input mcu_get_input(DOUT46)
#elif ASSERT_PIN_EXTENDED(DOUT46)
#define io93_config_output
#define io93_set_output ic74hc595_set_pin(DOUT46);ic74hc595_shift_io_pins()
#define io93_clear_output ic74hc595_clear_pin(DOUT46);ic74hc595_shift_io_pins()
#define io93_toggle_output ic74hc595_toggle_pin(DOUT46);ic74hc595_shift_io_pins()
#define io93_get_output ic74hc595_get_pin(DOUT46)
#define io93_config_input
#define io93_config_pullup
#define io93_get_input 0
#else
#define io93_config_output
#define io93_set_output
#define io93_clear_output
#define io93_toggle_output
#define io93_get_output 0
#define io93_config_input
#define io93_config_pullup
#define io93_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT47)
#define io94_config_output mcu_config_output(DOUT47)
#define io94_set_output mcu_set_output(DOUT47)
#define io94_clear_output mcu_clear_output(DOUT47)
#define io94_toggle_output mcu_toggle_output(DOUT47)
#define io94_get_output mcu_get_output(DOUT47)
#if !defined(DOUT47_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io94_config_input mcu_config_input(DOUT47)
#else
#define io94_config_input mcu_config_input(DOUT47);mcu_config_pullup(DOUT47)
#endif
#define io94_config_pullup mcu_config_pullup(DOUT47)
#define io94_get_input mcu_get_input(DOUT47)
#elif ASSERT_PIN_EXTENDED(DOUT47)
#define io94_config_output
#define io94_set_output ic74hc595_set_pin(DOUT47);ic74hc595_shift_io_pins()
#define io94_clear_output ic74hc595_clear_pin(DOUT47);ic74hc595_shift_io_pins()
#define io94_toggle_output ic74hc595_toggle_pin(DOUT47);ic74hc595_shift_io_pins()
#define io94_get_output ic74hc595_get_pin(DOUT47)
#define io94_config_input
#define io94_config_pullup
#define io94_get_input 0
#else
#define io94_config_output
#define io94_set_output
#define io94_clear_output
#define io94_toggle_output
#define io94_get_output 0
#define io94_config_input
#define io94_config_pullup
#define io94_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT48)
#define io95_config_output mcu_config_output(DOUT48)
#define io95_set_output mcu_set_output(DOUT48)
#define io95_clear_output mcu_clear_output(DOUT48)
#define io95_toggle_output mcu_toggle_output(DOUT48)
#define io95_get_output mcu_get_output(DOUT48)
#if !defined(DOUT48_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io95_config_input mcu_config_input(DOUT48)
#else
#define io95_config_input mcu_config_input(DOUT48);mcu_config_pullup(DOUT48)
#endif
#define io95_config_pullup mcu_config_pullup(DOUT48)
#define io95_get_input mcu_get_input(DOUT48)
#elif ASSERT_PIN_EXTENDED(DOUT48)
#define io95_config_output
#define io95_set_output ic74hc595_set_pin(DOUT48);ic74hc595_shift_io_pins()
#define io95_clear_output ic74hc595_clear_pin(DOUT48);ic74hc595_shift_io_pins()
#define io95_toggle_output ic74hc595_toggle_pin(DOUT48);ic74hc595_shift_io_pins()
#define io95_get_output ic74hc595_get_pin(DOUT48)
#define io95_config_input
#define io95_config_pullup
#define io95_get_input 0
#else
#define io95_config_output
#define io95_set_output
#define io95_clear_output
#define io95_toggle_output
#define io95_get_output 0
#define io95_config_input
#define io95_config_pullup
#define io95_get_input 0
#endif
#if ASSERT_PIN_IO(DOUT49)
#define io96_config_output mcu_config_output(DOUT49)
#define io96_set_output mcu_set_output(DOUT49)
#define io96_clear_output mcu_clear_output(DOUT49)
#define io96_toggle_output mcu_toggle_output(DOUT49)
#define io96_get_output mcu_get_output(DOUT49)
#if !defined(DOUT49_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io96_config_input mcu_config_input(DOUT49)
#else
#define io96_config_input mcu_config_input(DOUT49);mcu_config_pullup(DOUT49)
#endif
#define io96_config_pullup mcu_config_pullup(DOUT49)
#define io96_get_input mcu_get_input(DOUT49)
#elif ASSERT_PIN_EXTENDED(DOUT49)
#define io96_config_output
#define io96_set_output ic74hc595_set_pin(DOUT49);ic74hc595_shift_io_pins()
#define io96_clear_output ic74hc595_clear_pin(DOUT49);ic74hc595_shift_io_pins()
#define io96_toggle_output ic74hc595_toggle_pin(DOUT49);ic74hc595_shift_io_pins()
#define io96_get_output ic74hc595_get_pin(DOUT49)
#define io96_config_input
#define io96_config_pullup
#define io96_get_input 0
#else
#define io96_config_output
#define io96_set_output
#define io96_clear_output
#define io96_toggle_output
#define io96_get_output 0
#define io96_config_input
#define io96_config_pullup
#define io96_get_input 0
#endif
#if ASSERT_PIN_IO(LIMIT_X)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io100_config_output mcu_config_output(LIMIT_X)
#define io100_set_output mcu_set_output(LIMIT_X)
#define io100_clear_output mcu_clear_output(LIMIT_X)
#define io100_toggle_output mcu_toggle_output(LIMIT_X)
#define io100_get_output mcu_get_output(LIMIT_X)
#endif
#if !defined(LIMIT_X_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io100_config_input mcu_config_input(LIMIT_X)
#else
#define io100_config_input mcu_config_input(LIMIT_X);mcu_config_pullup(LIMIT_X)
#endif
#define io100_config_pullup mcu_config_pullup(LIMIT_X)
#define io100_get_input mcu_get_input(LIMIT_X)
#elif ASSERT_PIN_EXTENDED(LIMIT_X)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io100_config_output
#define io100_set_output ic74hc595_set_pin(LIMIT_X);ic74hc595_shift_io_pins()
#define io100_clear_output ic74hc595_clear_pin(LIMIT_X);ic74hc595_shift_io_pins()
#define io100_toggle_output ic74hc595_toggle_pin(LIMIT_X);ic74hc595_shift_io_pins()
#define io100_get_output ic74hc595_get_pin(LIMIT_X)
#endif
#define io100_config_input
#define io100_config_pullup
#define io100_get_input 0
#else
#define io100_config_output
#define io100_set_output
#define io100_clear_output
#define io100_toggle_output
#define io100_get_output 0
#define io100_config_input
#define io100_config_pullup
#define io100_get_input 0
#endif
#if ASSERT_PIN_IO(LIMIT_Y)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io101_config_output mcu_config_output(LIMIT_Y)
#define io101_set_output mcu_set_output(LIMIT_Y)
#define io101_clear_output mcu_clear_output(LIMIT_Y)
#define io101_toggle_output mcu_toggle_output(LIMIT_Y)
#define io101_get_output mcu_get_output(LIMIT_Y)
#endif
#if !defined(LIMIT_Y_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io101_config_input mcu_config_input(LIMIT_Y)
#else
#define io101_config_input mcu_config_input(LIMIT_Y);mcu_config_pullup(LIMIT_Y)
#endif
#define io101_config_pullup mcu_config_pullup(LIMIT_Y)
#define io101_get_input mcu_get_input(LIMIT_Y)
#elif ASSERT_PIN_EXTENDED(LIMIT_Y)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io101_config_output
#define io101_set_output ic74hc595_set_pin(LIMIT_Y);ic74hc595_shift_io_pins()
#define io101_clear_output ic74hc595_clear_pin(LIMIT_Y);ic74hc595_shift_io_pins()
#define io101_toggle_output ic74hc595_toggle_pin(LIMIT_Y);ic74hc595_shift_io_pins()
#define io101_get_output ic74hc595_get_pin(LIMIT_Y)
#endif
#define io101_config_input
#define io101_config_pullup
#define io101_get_input 0
#else
#define io101_config_output
#define io101_set_output
#define io101_clear_output
#define io101_toggle_output
#define io101_get_output 0
#define io101_config_input
#define io101_config_pullup
#define io101_get_input 0
#endif
#if ASSERT_PIN_IO(LIMIT_Z)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io102_config_output mcu_config_output(LIMIT_Z)
#define io102_set_output mcu_set_output(LIMIT_Z)
#define io102_clear_output mcu_clear_output(LIMIT_Z)
#define io102_toggle_output mcu_toggle_output(LIMIT_Z)
#define io102_get_output mcu_get_output(LIMIT_Z)
#endif
#if !defined(LIMIT_Z_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io102_config_input mcu_config_input(LIMIT_Z)
#else
#define io102_config_input mcu_config_input(LIMIT_Z);mcu_config_pullup(LIMIT_Z)
#endif
#define io102_config_pullup mcu_config_pullup(LIMIT_Z)
#define io102_get_input mcu_get_input(LIMIT_Z)
#elif ASSERT_PIN_EXTENDED(LIMIT_Z)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io102_config_output
#define io102_set_output ic74hc595_set_pin(LIMIT_Z);ic74hc595_shift_io_pins()
#define io102_clear_output ic74hc595_clear_pin(LIMIT_Z);ic74hc595_shift_io_pins()
#define io102_toggle_output ic74hc595_toggle_pin(LIMIT_Z);ic74hc595_shift_io_pins()
#define io102_get_output ic74hc595_get_pin(LIMIT_Z)
#endif
#define io102_config_input
#define io102_config_pullup
#define io102_get_input 0
#else
#define io102_config_output
#define io102_set_output
#define io102_clear_output
#define io102_toggle_output
#define io102_get_output 0
#define io102_config_input
#define io102_config_pullup
#define io102_get_input 0
#endif
#if ASSERT_PIN_IO(LIMIT_X2)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io103_config_output mcu_config_output(LIMIT_X2)
#define io103_set_output mcu_set_output(LIMIT_X2)
#define io103_clear_output mcu_clear_output(LIMIT_X2)
#define io103_toggle_output mcu_toggle_output(LIMIT_X2)
#define io103_get_output mcu_get_output(LIMIT_X2)
#endif
#if !defined(LIMIT_X2_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io103_config_input mcu_config_input(LIMIT_X2)
#else
#define io103_config_input mcu_config_input(LIMIT_X2);mcu_config_pullup(LIMIT_X2)
#endif
#define io103_config_pullup mcu_config_pullup(LIMIT_X2)
#define io103_get_input mcu_get_input(LIMIT_X2)
#elif ASSERT_PIN_EXTENDED(LIMIT_X2)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io103_config_output
#define io103_set_output ic74hc595_set_pin(LIMIT_X2);ic74hc595_shift_io_pins()
#define io103_clear_output ic74hc595_clear_pin(LIMIT_X2);ic74hc595_shift_io_pins()
#define io103_toggle_output ic74hc595_toggle_pin(LIMIT_X2);ic74hc595_shift_io_pins()
#define io103_get_output ic74hc595_get_pin(LIMIT_X2)
#endif
#define io103_config_input
#define io103_config_pullup
#define io103_get_input 0
#else
#define io103_config_output
#define io103_set_output
#define io103_clear_output
#define io103_toggle_output
#define io103_get_output 0
#define io103_config_input
#define io103_config_pullup
#define io103_get_input 0
#endif
#if ASSERT_PIN_IO(LIMIT_Y2)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io104_config_output mcu_config_output(LIMIT_Y2)
#define io104_set_output mcu_set_output(LIMIT_Y2)
#define io104_clear_output mcu_clear_output(LIMIT_Y2)
#define io104_toggle_output mcu_toggle_output(LIMIT_Y2)
#define io104_get_output mcu_get_output(LIMIT_Y2)
#endif
#if !defined(LIMIT_Y2_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io104_config_input mcu_config_input(LIMIT_Y2)
#else
#define io104_config_input mcu_config_input(LIMIT_Y2);mcu_config_pullup(LIMIT_Y2)
#endif
#define io104_config_pullup mcu_config_pullup(LIMIT_Y2)
#define io104_get_input mcu_get_input(LIMIT_Y2)
#elif ASSERT_PIN_EXTENDED(LIMIT_Y2)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io104_config_output
#define io104_set_output ic74hc595_set_pin(LIMIT_Y2);ic74hc595_shift_io_pins()
#define io104_clear_output ic74hc595_clear_pin(LIMIT_Y2);ic74hc595_shift_io_pins()
#define io104_toggle_output ic74hc595_toggle_pin(LIMIT_Y2);ic74hc595_shift_io_pins()
#define io104_get_output ic74hc595_get_pin(LIMIT_Y2)
#endif
#define io104_config_input
#define io104_config_pullup
#define io104_get_input 0
#else
#define io104_config_output
#define io104_set_output
#define io104_clear_output
#define io104_toggle_output
#define io104_get_output 0
#define io104_config_input
#define io104_config_pullup
#define io104_get_input 0
#endif
#if ASSERT_PIN_IO(LIMIT_Z2)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io105_config_output mcu_config_output(LIMIT_Z2)
#define io105_set_output mcu_set_output(LIMIT_Z2)
#define io105_clear_output mcu_clear_output(LIMIT_Z2)
#define io105_toggle_output mcu_toggle_output(LIMIT_Z2)
#define io105_get_output mcu_get_output(LIMIT_Z2)
#endif
#if !defined(LIMIT_Z2_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io105_config_input mcu_config_input(LIMIT_Z2)
#else
#define io105_config_input mcu_config_input(LIMIT_Z2);mcu_config_pullup(LIMIT_Z2)
#endif
#define io105_config_pullup mcu_config_pullup(LIMIT_Z2)
#define io105_get_input mcu_get_input(LIMIT_Z2)
#elif ASSERT_PIN_EXTENDED(LIMIT_Z2)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io105_config_output
#define io105_set_output ic74hc595_set_pin(LIMIT_Z2);ic74hc595_shift_io_pins()
#define io105_clear_output ic74hc595_clear_pin(LIMIT_Z2);ic74hc595_shift_io_pins()
#define io105_toggle_output ic74hc595_toggle_pin(LIMIT_Z2);ic74hc595_shift_io_pins()
#define io105_get_output ic74hc595_get_pin(LIMIT_Z2)
#endif
#define io105_config_input
#define io105_config_pullup
#define io105_get_input 0
#else
#define io105_config_output
#define io105_set_output
#define io105_clear_output
#define io105_toggle_output
#define io105_get_output 0
#define io105_config_input
#define io105_config_pullup
#define io105_get_input 0
#endif
#if ASSERT_PIN_IO(LIMIT_A)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io106_config_output mcu_config_output(LIMIT_A)
#define io106_set_output mcu_set_output(LIMIT_A)
#define io106_clear_output mcu_clear_output(LIMIT_A)
#define io106_toggle_output mcu_toggle_output(LIMIT_A)
#define io106_get_output mcu_get_output(LIMIT_A)
#endif
#if !defined(LIMIT_A_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io106_config_input mcu_config_input(LIMIT_A)
#else
#define io106_config_input mcu_config_input(LIMIT_A);mcu_config_pullup(LIMIT_A)
#endif
#define io106_config_pullup mcu_config_pullup(LIMIT_A)
#define io106_get_input mcu_get_input(LIMIT_A)
#elif ASSERT_PIN_EXTENDED(LIMIT_A)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io106_config_output
#define io106_set_output ic74hc595_set_pin(LIMIT_A);ic74hc595_shift_io_pins()
#define io106_clear_output ic74hc595_clear_pin(LIMIT_A);ic74hc595_shift_io_pins()
#define io106_toggle_output ic74hc595_toggle_pin(LIMIT_A);ic74hc595_shift_io_pins()
#define io106_get_output ic74hc595_get_pin(LIMIT_A)
#endif
#define io106_config_input
#define io106_config_pullup
#define io106_get_input 0
#else
#define io106_config_output
#define io106_set_output
#define io106_clear_output
#define io106_toggle_output
#define io106_get_output 0
#define io106_config_input
#define io106_config_pullup
#define io106_get_input 0
#endif
#if ASSERT_PIN_IO(LIMIT_B)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io107_config_output mcu_config_output(LIMIT_B)
#define io107_set_output mcu_set_output(LIMIT_B)
#define io107_clear_output mcu_clear_output(LIMIT_B)
#define io107_toggle_output mcu_toggle_output(LIMIT_B)
#define io107_get_output mcu_get_output(LIMIT_B)
#endif
#if !defined(LIMIT_B_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io107_config_input mcu_config_input(LIMIT_B)
#else
#define io107_config_input mcu_config_input(LIMIT_B);mcu_config_pullup(LIMIT_B)
#endif
#define io107_config_pullup mcu_config_pullup(LIMIT_B)
#define io107_get_input mcu_get_input(LIMIT_B)
#elif ASSERT_PIN_EXTENDED(LIMIT_B)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io107_config_output
#define io107_set_output ic74hc595_set_pin(LIMIT_B);ic74hc595_shift_io_pins()
#define io107_clear_output ic74hc595_clear_pin(LIMIT_B);ic74hc595_shift_io_pins()
#define io107_toggle_output ic74hc595_toggle_pin(LIMIT_B);ic74hc595_shift_io_pins()
#define io107_get_output ic74hc595_get_pin(LIMIT_B)
#endif
#define io107_config_input
#define io107_config_pullup
#define io107_get_input 0
#else
#define io107_config_output
#define io107_set_output
#define io107_clear_output
#define io107_toggle_output
#define io107_get_output 0
#define io107_config_input
#define io107_config_pullup
#define io107_get_input 0
#endif
#if ASSERT_PIN_IO(LIMIT_C)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io108_config_output mcu_config_output(LIMIT_C)
#define io108_set_output mcu_set_output(LIMIT_C)
#define io108_clear_output mcu_clear_output(LIMIT_C)
#define io108_toggle_output mcu_toggle_output(LIMIT_C)
#define io108_get_output mcu_get_output(LIMIT_C)
#endif
#if !defined(LIMIT_C_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io108_config_input mcu_config_input(LIMIT_C)
#else
#define io108_config_input mcu_config_input(LIMIT_C);mcu_config_pullup(LIMIT_C)
#endif
#define io108_config_pullup mcu_config_pullup(LIMIT_C)
#define io108_get_input mcu_get_input(LIMIT_C)
#elif ASSERT_PIN_EXTENDED(LIMIT_C)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io108_config_output
#define io108_set_output ic74hc595_set_pin(LIMIT_C);ic74hc595_shift_io_pins()
#define io108_clear_output ic74hc595_clear_pin(LIMIT_C);ic74hc595_shift_io_pins()
#define io108_toggle_output ic74hc595_toggle_pin(LIMIT_C);ic74hc595_shift_io_pins()
#define io108_get_output ic74hc595_get_pin(LIMIT_C)
#endif
#define io108_config_input
#define io108_config_pullup
#define io108_get_input 0
#else
#define io108_config_output
#define io108_set_output
#define io108_clear_output
#define io108_toggle_output
#define io108_get_output 0
#define io108_config_input
#define io108_config_pullup
#define io108_get_input 0
#endif
#if ASSERT_PIN_IO(PROBE)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io109_config_output mcu_config_output(PROBE)
#define io109_set_output mcu_set_output(PROBE)
#define io109_clear_output mcu_clear_output(PROBE)
#define io109_toggle_output mcu_toggle_output(PROBE)
#define io109_get_output mcu_get_output(PROBE)
#endif
#if !defined(PROBE_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io109_config_input mcu_config_input(PROBE)
#else
#define io109_config_input mcu_config_input(PROBE);mcu_config_pullup(PROBE)
#endif
#define io109_config_pullup mcu_config_pullup(PROBE)
#define io109_get_input mcu_get_input(PROBE)
#elif ASSERT_PIN_EXTENDED(PROBE)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io109_config_output
#define io109_set_output ic74hc595_set_pin(PROBE);ic74hc595_shift_io_pins()
#define io109_clear_output ic74hc595_clear_pin(PROBE);ic74hc595_shift_io_pins()
#define io109_toggle_output ic74hc595_toggle_pin(PROBE);ic74hc595_shift_io_pins()
#define io109_get_output ic74hc595_get_pin(PROBE)
#endif
#define io109_config_input
#define io109_config_pullup
#define io109_get_input 0
#else
#define io109_config_output
#define io109_set_output
#define io109_clear_output
#define io109_toggle_output
#define io109_get_output 0
#define io109_config_input
#define io109_config_pullup
#define io109_get_input 0
#endif
#if ASSERT_PIN_IO(ESTOP)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io110_config_output mcu_config_output(ESTOP)
#define io110_set_output mcu_set_output(ESTOP)
#define io110_clear_output mcu_clear_output(ESTOP)
#define io110_toggle_output mcu_toggle_output(ESTOP)
#define io110_get_output mcu_get_output(ESTOP)
#endif
#if !defined(ESTOP_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io110_config_input mcu_config_input(ESTOP)
#else
#define io110_config_input mcu_config_input(ESTOP);mcu_config_pullup(ESTOP)
#endif
#define io110_config_pullup mcu_config_pullup(ESTOP)
#define io110_get_input mcu_get_input(ESTOP)
#elif ASSERT_PIN_EXTENDED(ESTOP)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io110_config_output
#define io110_set_output ic74hc595_set_pin(ESTOP);ic74hc595_shift_io_pins()
#define io110_clear_output ic74hc595_clear_pin(ESTOP);ic74hc595_shift_io_pins()
#define io110_toggle_output ic74hc595_toggle_pin(ESTOP);ic74hc595_shift_io_pins()
#define io110_get_output ic74hc595_get_pin(ESTOP)
#endif
#define io110_config_input
#define io110_config_pullup
#define io110_get_input 0
#else
#define io110_config_output
#define io110_set_output
#define io110_clear_output
#define io110_toggle_output
#define io110_get_output 0
#define io110_config_input
#define io110_config_pullup
#define io110_get_input 0
#endif
#if ASSERT_PIN_IO(SAFETY_DOOR)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io111_config_output mcu_config_output(SAFETY_DOOR)
#define io111_set_output mcu_set_output(SAFETY_DOOR)
#define io111_clear_output mcu_clear_output(SAFETY_DOOR)
#define io111_toggle_output mcu_toggle_output(SAFETY_DOOR)
#define io111_get_output mcu_get_output(SAFETY_DOOR)
#endif
#if !defined(SAFETY_DOOR_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io111_config_input mcu_config_input(SAFETY_DOOR)
#else
#define io111_config_input mcu_config_input(SAFETY_DOOR);mcu_config_pullup(SAFETY_DOOR)
#endif
#define io111_config_pullup mcu_config_pullup(SAFETY_DOOR)
#define io111_get_input mcu_get_input(SAFETY_DOOR)
#elif ASSERT_PIN_EXTENDED(SAFETY_DOOR)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io111_config_output
#define io111_set_output ic74hc595_set_pin(SAFETY_DOOR);ic74hc595_shift_io_pins()
#define io111_clear_output ic74hc595_clear_pin(SAFETY_DOOR);ic74hc595_shift_io_pins()
#define io111_toggle_output ic74hc595_toggle_pin(SAFETY_DOOR);ic74hc595_shift_io_pins()
#define io111_get_output ic74hc595_get_pin(SAFETY_DOOR)
#endif
#define io111_config_input
#define io111_config_pullup
#define io111_get_input 0
#else
#define io111_config_output
#define io111_set_output
#define io111_clear_output
#define io111_toggle_output
#define io111_get_output 0
#define io111_config_input
#define io111_config_pullup
#define io111_get_input 0
#endif
#if ASSERT_PIN_IO(FHOLD)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io112_config_output mcu_config_output(FHOLD)
#define io112_set_output mcu_set_output(FHOLD)
#define io112_clear_output mcu_clear_output(FHOLD)
#define io112_toggle_output mcu_toggle_output(FHOLD)
#define io112_get_output mcu_get_output(FHOLD)
#endif
#if !defined(FHOLD_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io112_config_input mcu_config_input(FHOLD)
#else
#define io112_config_input mcu_config_input(FHOLD);mcu_config_pullup(FHOLD)
#endif
#define io112_config_pullup mcu_config_pullup(FHOLD)
#define io112_get_input mcu_get_input(FHOLD)
#elif ASSERT_PIN_EXTENDED(FHOLD)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io112_config_output
#define io112_set_output ic74hc595_set_pin(FHOLD);ic74hc595_shift_io_pins()
#define io112_clear_output ic74hc595_clear_pin(FHOLD);ic74hc595_shift_io_pins()
#define io112_toggle_output ic74hc595_toggle_pin(FHOLD);ic74hc595_shift_io_pins()
#define io112_get_output ic74hc595_get_pin(FHOLD)
#endif
#define io112_config_input
#define io112_config_pullup
#define io112_get_input 0
#else
#define io112_config_output
#define io112_set_output
#define io112_clear_output
#define io112_toggle_output
#define io112_get_output 0
#define io112_config_input
#define io112_config_pullup
#define io112_get_input 0
#endif
#if ASSERT_PIN_IO(CS_RES)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io113_config_output mcu_config_output(CS_RES)
#define io113_set_output mcu_set_output(CS_RES)
#define io113_clear_output mcu_clear_output(CS_RES)
#define io113_toggle_output mcu_toggle_output(CS_RES)
#define io113_get_output mcu_get_output(CS_RES)
#endif
#if !defined(CS_RES_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io113_config_input mcu_config_input(CS_RES)
#else
#define io113_config_input mcu_config_input(CS_RES);mcu_config_pullup(CS_RES)
#endif
#define io113_config_pullup mcu_config_pullup(CS_RES)
#define io113_get_input mcu_get_input(CS_RES)
#elif ASSERT_PIN_EXTENDED(CS_RES)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io113_config_output
#define io113_set_output ic74hc595_set_pin(CS_RES);ic74hc595_shift_io_pins()
#define io113_clear_output ic74hc595_clear_pin(CS_RES);ic74hc595_shift_io_pins()
#define io113_toggle_output ic74hc595_toggle_pin(CS_RES);ic74hc595_shift_io_pins()
#define io113_get_output ic74hc595_get_pin(CS_RES)
#endif
#define io113_config_input
#define io113_config_pullup
#define io113_get_input 0
#else
#define io113_config_output
#define io113_set_output
#define io113_clear_output
#define io113_toggle_output
#define io113_get_output 0
#define io113_config_input
#define io113_config_pullup
#define io113_get_input 0
#endif
















#if ASSERT_PIN_IO(DIN0)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io130_config_output mcu_config_output(DIN0)
#define io130_set_output mcu_set_output(DIN0)
#define io130_clear_output mcu_clear_output(DIN0)
#define io130_toggle_output mcu_toggle_output(DIN0)
#define io130_get_output mcu_get_output(DIN0)
#endif
#if !defined(DIN0_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io130_config_input mcu_config_input(DIN0)
#else
#define io130_config_input mcu_config_input(DIN0);mcu_config_pullup(DIN0)
#endif
#define io130_config_pullup mcu_config_pullup(DIN0)
#define io130_get_input mcu_get_input(DIN0)
#elif ASSERT_PIN_EXTENDED(DIN0)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io130_config_output
#define io130_set_output ic74hc595_set_pin(DIN0);ic74hc595_shift_io_pins()
#define io130_clear_output ic74hc595_clear_pin(DIN0);ic74hc595_shift_io_pins()
#define io130_toggle_output ic74hc595_toggle_pin(DIN0);ic74hc595_shift_io_pins()
#define io130_get_output ic74hc595_get_pin(DIN0)
#endif
#define io130_config_input
#define io130_config_pullup
#define io130_get_input 0
#else
#define io130_config_output
#define io130_set_output
#define io130_clear_output
#define io130_toggle_output
#define io130_get_output 0
#define io130_config_input
#define io130_config_pullup
#define io130_get_input 0
#endif
#if ASSERT_PIN_IO(DIN1)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io131_config_output mcu_config_output(DIN1)
#define io131_set_output mcu_set_output(DIN1)
#define io131_clear_output mcu_clear_output(DIN1)
#define io131_toggle_output mcu_toggle_output(DIN1)
#define io131_get_output mcu_get_output(DIN1)
#endif
#if !defined(DIN1_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io131_config_input mcu_config_input(DIN1)
#else
#define io131_config_input mcu_config_input(DIN1);mcu_config_pullup(DIN1)
#endif
#define io131_config_pullup mcu_config_pullup(DIN1)
#define io131_get_input mcu_get_input(DIN1)
#elif ASSERT_PIN_EXTENDED(DIN1)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io131_config_output
#define io131_set_output ic74hc595_set_pin(DIN1);ic74hc595_shift_io_pins()
#define io131_clear_output ic74hc595_clear_pin(DIN1);ic74hc595_shift_io_pins()
#define io131_toggle_output ic74hc595_toggle_pin(DIN1);ic74hc595_shift_io_pins()
#define io131_get_output ic74hc595_get_pin(DIN1)
#endif
#define io131_config_input
#define io131_config_pullup
#define io131_get_input 0
#else
#define io131_config_output
#define io131_set_output
#define io131_clear_output
#define io131_toggle_output
#define io131_get_output 0
#define io131_config_input
#define io131_config_pullup
#define io131_get_input 0
#endif
#if ASSERT_PIN_IO(DIN2)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io132_config_output mcu_config_output(DIN2)
#define io132_set_output mcu_set_output(DIN2)
#define io132_clear_output mcu_clear_output(DIN2)
#define io132_toggle_output mcu_toggle_output(DIN2)
#define io132_get_output mcu_get_output(DIN2)
#endif
#if !defined(DIN2_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io132_config_input mcu_config_input(DIN2)
#else
#define io132_config_input mcu_config_input(DIN2);mcu_config_pullup(DIN2)
#endif
#define io132_config_pullup mcu_config_pullup(DIN2)
#define io132_get_input mcu_get_input(DIN2)
#elif ASSERT_PIN_EXTENDED(DIN2)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io132_config_output
#define io132_set_output ic74hc595_set_pin(DIN2);ic74hc595_shift_io_pins()
#define io132_clear_output ic74hc595_clear_pin(DIN2);ic74hc595_shift_io_pins()
#define io132_toggle_output ic74hc595_toggle_pin(DIN2);ic74hc595_shift_io_pins()
#define io132_get_output ic74hc595_get_pin(DIN2)
#endif
#define io132_config_input
#define io132_config_pullup
#define io132_get_input 0
#else
#define io132_config_output
#define io132_set_output
#define io132_clear_output
#define io132_toggle_output
#define io132_get_output 0
#define io132_config_input
#define io132_config_pullup
#define io132_get_input 0
#endif
#if ASSERT_PIN_IO(DIN3)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io133_config_output mcu_config_output(DIN3)
#define io133_set_output mcu_set_output(DIN3)
#define io133_clear_output mcu_clear_output(DIN3)
#define io133_toggle_output mcu_toggle_output(DIN3)
#define io133_get_output mcu_get_output(DIN3)
#endif
#if !defined(DIN3_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io133_config_input mcu_config_input(DIN3)
#else
#define io133_config_input mcu_config_input(DIN3);mcu_config_pullup(DIN3)
#endif
#define io133_config_pullup mcu_config_pullup(DIN3)
#define io133_get_input mcu_get_input(DIN3)
#elif ASSERT_PIN_EXTENDED(DIN3)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io133_config_output
#define io133_set_output ic74hc595_set_pin(DIN3);ic74hc595_shift_io_pins()
#define io133_clear_output ic74hc595_clear_pin(DIN3);ic74hc595_shift_io_pins()
#define io133_toggle_output ic74hc595_toggle_pin(DIN3);ic74hc595_shift_io_pins()
#define io133_get_output ic74hc595_get_pin(DIN3)
#endif
#define io133_config_input
#define io133_config_pullup
#define io133_get_input 0
#else
#define io133_config_output
#define io133_set_output
#define io133_clear_output
#define io133_toggle_output
#define io133_get_output 0
#define io133_config_input
#define io133_config_pullup
#define io133_get_input 0
#endif
#if ASSERT_PIN_IO(DIN4)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io134_config_output mcu_config_output(DIN4)
#define io134_set_output mcu_set_output(DIN4)
#define io134_clear_output mcu_clear_output(DIN4)
#define io134_toggle_output mcu_toggle_output(DIN4)
#define io134_get_output mcu_get_output(DIN4)
#endif
#if !defined(DIN4_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io134_config_input mcu_config_input(DIN4)
#else
#define io134_config_input mcu_config_input(DIN4);mcu_config_pullup(DIN4)
#endif
#define io134_config_pullup mcu_config_pullup(DIN4)
#define io134_get_input mcu_get_input(DIN4)
#elif ASSERT_PIN_EXTENDED(DIN4)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io134_config_output
#define io134_set_output ic74hc595_set_pin(DIN4);ic74hc595_shift_io_pins()
#define io134_clear_output ic74hc595_clear_pin(DIN4);ic74hc595_shift_io_pins()
#define io134_toggle_output ic74hc595_toggle_pin(DIN4);ic74hc595_shift_io_pins()
#define io134_get_output ic74hc595_get_pin(DIN4)
#endif
#define io134_config_input
#define io134_config_pullup
#define io134_get_input 0
#else
#define io134_config_output
#define io134_set_output
#define io134_clear_output
#define io134_toggle_output
#define io134_get_output 0
#define io134_config_input
#define io134_config_pullup
#define io134_get_input 0
#endif
#if ASSERT_PIN_IO(DIN5)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io135_config_output mcu_config_output(DIN5)
#define io135_set_output mcu_set_output(DIN5)
#define io135_clear_output mcu_clear_output(DIN5)
#define io135_toggle_output mcu_toggle_output(DIN5)
#define io135_get_output mcu_get_output(DIN5)
#endif
#if !defined(DIN5_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io135_config_input mcu_config_input(DIN5)
#else
#define io135_config_input mcu_config_input(DIN5);mcu_config_pullup(DIN5)
#endif
#define io135_config_pullup mcu_config_pullup(DIN5)
#define io135_get_input mcu_get_input(DIN5)
#elif ASSERT_PIN_EXTENDED(DIN5)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io135_config_output
#define io135_set_output ic74hc595_set_pin(DIN5);ic74hc595_shift_io_pins()
#define io135_clear_output ic74hc595_clear_pin(DIN5);ic74hc595_shift_io_pins()
#define io135_toggle_output ic74hc595_toggle_pin(DIN5);ic74hc595_shift_io_pins()
#define io135_get_output ic74hc595_get_pin(DIN5)
#endif
#define io135_config_input
#define io135_config_pullup
#define io135_get_input 0
#else
#define io135_config_output
#define io135_set_output
#define io135_clear_output
#define io135_toggle_output
#define io135_get_output 0
#define io135_config_input
#define io135_config_pullup
#define io135_get_input 0
#endif
#if ASSERT_PIN_IO(DIN6)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io136_config_output mcu_config_output(DIN6)
#define io136_set_output mcu_set_output(DIN6)
#define io136_clear_output mcu_clear_output(DIN6)
#define io136_toggle_output mcu_toggle_output(DIN6)
#define io136_get_output mcu_get_output(DIN6)
#endif
#if !defined(DIN6_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io136_config_input mcu_config_input(DIN6)
#else
#define io136_config_input mcu_config_input(DIN6);mcu_config_pullup(DIN6)
#endif
#define io136_config_pullup mcu_config_pullup(DIN6)
#define io136_get_input mcu_get_input(DIN6)
#elif ASSERT_PIN_EXTENDED(DIN6)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io136_config_output
#define io136_set_output ic74hc595_set_pin(DIN6);ic74hc595_shift_io_pins()
#define io136_clear_output ic74hc595_clear_pin(DIN6);ic74hc595_shift_io_pins()
#define io136_toggle_output ic74hc595_toggle_pin(DIN6);ic74hc595_shift_io_pins()
#define io136_get_output ic74hc595_get_pin(DIN6)
#endif
#define io136_config_input
#define io136_config_pullup
#define io136_get_input 0
#else
#define io136_config_output
#define io136_set_output
#define io136_clear_output
#define io136_toggle_output
#define io136_get_output 0
#define io136_config_input
#define io136_config_pullup
#define io136_get_input 0
#endif
#if ASSERT_PIN_IO(DIN7)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io137_config_output mcu_config_output(DIN7)
#define io137_set_output mcu_set_output(DIN7)
#define io137_clear_output mcu_clear_output(DIN7)
#define io137_toggle_output mcu_toggle_output(DIN7)
#define io137_get_output mcu_get_output(DIN7)
#endif
#if !defined(DIN7_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io137_config_input mcu_config_input(DIN7)
#else
#define io137_config_input mcu_config_input(DIN7);mcu_config_pullup(DIN7)
#endif
#define io137_config_pullup mcu_config_pullup(DIN7)
#define io137_get_input mcu_get_input(DIN7)
#elif ASSERT_PIN_EXTENDED(DIN7)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io137_config_output
#define io137_set_output ic74hc595_set_pin(DIN7);ic74hc595_shift_io_pins()
#define io137_clear_output ic74hc595_clear_pin(DIN7);ic74hc595_shift_io_pins()
#define io137_toggle_output ic74hc595_toggle_pin(DIN7);ic74hc595_shift_io_pins()
#define io137_get_output ic74hc595_get_pin(DIN7)
#endif
#define io137_config_input
#define io137_config_pullup
#define io137_get_input 0
#else
#define io137_config_output
#define io137_set_output
#define io137_clear_output
#define io137_toggle_output
#define io137_get_output 0
#define io137_config_input
#define io137_config_pullup
#define io137_get_input 0
#endif
#if ASSERT_PIN_IO(DIN8)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io138_config_output mcu_config_output(DIN8)
#define io138_set_output mcu_set_output(DIN8)
#define io138_clear_output mcu_clear_output(DIN8)
#define io138_toggle_output mcu_toggle_output(DIN8)
#define io138_get_output mcu_get_output(DIN8)
#endif
#if !defined(DIN8_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io138_config_input mcu_config_input(DIN8)
#else
#define io138_config_input mcu_config_input(DIN8);mcu_config_pullup(DIN8)
#endif
#define io138_config_pullup mcu_config_pullup(DIN8)
#define io138_get_input mcu_get_input(DIN8)
#elif ASSERT_PIN_EXTENDED(DIN8)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io138_config_output
#define io138_set_output ic74hc595_set_pin(DIN8);ic74hc595_shift_io_pins()
#define io138_clear_output ic74hc595_clear_pin(DIN8);ic74hc595_shift_io_pins()
#define io138_toggle_output ic74hc595_toggle_pin(DIN8);ic74hc595_shift_io_pins()
#define io138_get_output ic74hc595_get_pin(DIN8)
#endif
#define io138_config_input
#define io138_config_pullup
#define io138_get_input 0
#else
#define io138_config_output
#define io138_set_output
#define io138_clear_output
#define io138_toggle_output
#define io138_get_output 0
#define io138_config_input
#define io138_config_pullup
#define io138_get_input 0
#endif
#if ASSERT_PIN_IO(DIN9)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io139_config_output mcu_config_output(DIN9)
#define io139_set_output mcu_set_output(DIN9)
#define io139_clear_output mcu_clear_output(DIN9)
#define io139_toggle_output mcu_toggle_output(DIN9)
#define io139_get_output mcu_get_output(DIN9)
#endif
#if !defined(DIN9_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io139_config_input mcu_config_input(DIN9)
#else
#define io139_config_input mcu_config_input(DIN9);mcu_config_pullup(DIN9)
#endif
#define io139_config_pullup mcu_config_pullup(DIN9)
#define io139_get_input mcu_get_input(DIN9)
#elif ASSERT_PIN_EXTENDED(DIN9)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io139_config_output
#define io139_set_output ic74hc595_set_pin(DIN9);ic74hc595_shift_io_pins()
#define io139_clear_output ic74hc595_clear_pin(DIN9);ic74hc595_shift_io_pins()
#define io139_toggle_output ic74hc595_toggle_pin(DIN9);ic74hc595_shift_io_pins()
#define io139_get_output ic74hc595_get_pin(DIN9)
#endif
#define io139_config_input
#define io139_config_pullup
#define io139_get_input 0
#else
#define io139_config_output
#define io139_set_output
#define io139_clear_output
#define io139_toggle_output
#define io139_get_output 0
#define io139_config_input
#define io139_config_pullup
#define io139_get_input 0
#endif
#if ASSERT_PIN_IO(DIN10)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io140_config_output mcu_config_output(DIN10)
#define io140_set_output mcu_set_output(DIN10)
#define io140_clear_output mcu_clear_output(DIN10)
#define io140_toggle_output mcu_toggle_output(DIN10)
#define io140_get_output mcu_get_output(DIN10)
#endif
#if !defined(DIN10_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io140_config_input mcu_config_input(DIN10)
#else
#define io140_config_input mcu_config_input(DIN10);mcu_config_pullup(DIN10)
#endif
#define io140_config_pullup mcu_config_pullup(DIN10)
#define io140_get_input mcu_get_input(DIN10)
#elif ASSERT_PIN_EXTENDED(DIN10)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io140_config_output
#define io140_set_output ic74hc595_set_pin(DIN10);ic74hc595_shift_io_pins()
#define io140_clear_output ic74hc595_clear_pin(DIN10);ic74hc595_shift_io_pins()
#define io140_toggle_output ic74hc595_toggle_pin(DIN10);ic74hc595_shift_io_pins()
#define io140_get_output ic74hc595_get_pin(DIN10)
#endif
#define io140_config_input
#define io140_config_pullup
#define io140_get_input 0
#else
#define io140_config_output
#define io140_set_output
#define io140_clear_output
#define io140_toggle_output
#define io140_get_output 0
#define io140_config_input
#define io140_config_pullup
#define io140_get_input 0
#endif
#if ASSERT_PIN_IO(DIN11)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io141_config_output mcu_config_output(DIN11)
#define io141_set_output mcu_set_output(DIN11)
#define io141_clear_output mcu_clear_output(DIN11)
#define io141_toggle_output mcu_toggle_output(DIN11)
#define io141_get_output mcu_get_output(DIN11)
#endif
#if !defined(DIN11_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io141_config_input mcu_config_input(DIN11)
#else
#define io141_config_input mcu_config_input(DIN11);mcu_config_pullup(DIN11)
#endif
#define io141_config_pullup mcu_config_pullup(DIN11)
#define io141_get_input mcu_get_input(DIN11)
#elif ASSERT_PIN_EXTENDED(DIN11)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io141_config_output
#define io141_set_output ic74hc595_set_pin(DIN11);ic74hc595_shift_io_pins()
#define io141_clear_output ic74hc595_clear_pin(DIN11);ic74hc595_shift_io_pins()
#define io141_toggle_output ic74hc595_toggle_pin(DIN11);ic74hc595_shift_io_pins()
#define io141_get_output ic74hc595_get_pin(DIN11)
#endif
#define io141_config_input
#define io141_config_pullup
#define io141_get_input 0
#else
#define io141_config_output
#define io141_set_output
#define io141_clear_output
#define io141_toggle_output
#define io141_get_output 0
#define io141_config_input
#define io141_config_pullup
#define io141_get_input 0
#endif
#if ASSERT_PIN_IO(DIN12)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io142_config_output mcu_config_output(DIN12)
#define io142_set_output mcu_set_output(DIN12)
#define io142_clear_output mcu_clear_output(DIN12)
#define io142_toggle_output mcu_toggle_output(DIN12)
#define io142_get_output mcu_get_output(DIN12)
#endif
#if !defined(DIN12_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io142_config_input mcu_config_input(DIN12)
#else
#define io142_config_input mcu_config_input(DIN12);mcu_config_pullup(DIN12)
#endif
#define io142_config_pullup mcu_config_pullup(DIN12)
#define io142_get_input mcu_get_input(DIN12)
#elif ASSERT_PIN_EXTENDED(DIN12)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io142_config_output
#define io142_set_output ic74hc595_set_pin(DIN12);ic74hc595_shift_io_pins()
#define io142_clear_output ic74hc595_clear_pin(DIN12);ic74hc595_shift_io_pins()
#define io142_toggle_output ic74hc595_toggle_pin(DIN12);ic74hc595_shift_io_pins()
#define io142_get_output ic74hc595_get_pin(DIN12)
#endif
#define io142_config_input
#define io142_config_pullup
#define io142_get_input 0
#else
#define io142_config_output
#define io142_set_output
#define io142_clear_output
#define io142_toggle_output
#define io142_get_output 0
#define io142_config_input
#define io142_config_pullup
#define io142_get_input 0
#endif
#if ASSERT_PIN_IO(DIN13)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io143_config_output mcu_config_output(DIN13)
#define io143_set_output mcu_set_output(DIN13)
#define io143_clear_output mcu_clear_output(DIN13)
#define io143_toggle_output mcu_toggle_output(DIN13)
#define io143_get_output mcu_get_output(DIN13)
#endif
#if !defined(DIN13_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io143_config_input mcu_config_input(DIN13)
#else
#define io143_config_input mcu_config_input(DIN13);mcu_config_pullup(DIN13)
#endif
#define io143_config_pullup mcu_config_pullup(DIN13)
#define io143_get_input mcu_get_input(DIN13)
#elif ASSERT_PIN_EXTENDED(DIN13)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io143_config_output
#define io143_set_output ic74hc595_set_pin(DIN13);ic74hc595_shift_io_pins()
#define io143_clear_output ic74hc595_clear_pin(DIN13);ic74hc595_shift_io_pins()
#define io143_toggle_output ic74hc595_toggle_pin(DIN13);ic74hc595_shift_io_pins()
#define io143_get_output ic74hc595_get_pin(DIN13)
#endif
#define io143_config_input
#define io143_config_pullup
#define io143_get_input 0
#else
#define io143_config_output
#define io143_set_output
#define io143_clear_output
#define io143_toggle_output
#define io143_get_output 0
#define io143_config_input
#define io143_config_pullup
#define io143_get_input 0
#endif
#if ASSERT_PIN_IO(DIN14)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io144_config_output mcu_config_output(DIN14)
#define io144_set_output mcu_set_output(DIN14)
#define io144_clear_output mcu_clear_output(DIN14)
#define io144_toggle_output mcu_toggle_output(DIN14)
#define io144_get_output mcu_get_output(DIN14)
#endif
#if !defined(DIN14_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io144_config_input mcu_config_input(DIN14)
#else
#define io144_config_input mcu_config_input(DIN14);mcu_config_pullup(DIN14)
#endif
#define io144_config_pullup mcu_config_pullup(DIN14)
#define io144_get_input mcu_get_input(DIN14)
#elif ASSERT_PIN_EXTENDED(DIN14)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io144_config_output
#define io144_set_output ic74hc595_set_pin(DIN14);ic74hc595_shift_io_pins()
#define io144_clear_output ic74hc595_clear_pin(DIN14);ic74hc595_shift_io_pins()
#define io144_toggle_output ic74hc595_toggle_pin(DIN14);ic74hc595_shift_io_pins()
#define io144_get_output ic74hc595_get_pin(DIN14)
#endif
#define io144_config_input
#define io144_config_pullup
#define io144_get_input 0
#else
#define io144_config_output
#define io144_set_output
#define io144_clear_output
#define io144_toggle_output
#define io144_get_output 0
#define io144_config_input
#define io144_config_pullup
#define io144_get_input 0
#endif
#if ASSERT_PIN_IO(DIN15)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io145_config_output mcu_config_output(DIN15)
#define io145_set_output mcu_set_output(DIN15)
#define io145_clear_output mcu_clear_output(DIN15)
#define io145_toggle_output mcu_toggle_output(DIN15)
#define io145_get_output mcu_get_output(DIN15)
#endif
#if !defined(DIN15_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io145_config_input mcu_config_input(DIN15)
#else
#define io145_config_input mcu_config_input(DIN15);mcu_config_pullup(DIN15)
#endif
#define io145_config_pullup mcu_config_pullup(DIN15)
#define io145_get_input mcu_get_input(DIN15)
#elif ASSERT_PIN_EXTENDED(DIN15)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io145_config_output
#define io145_set_output ic74hc595_set_pin(DIN15);ic74hc595_shift_io_pins()
#define io145_clear_output ic74hc595_clear_pin(DIN15);ic74hc595_shift_io_pins()
#define io145_toggle_output ic74hc595_toggle_pin(DIN15);ic74hc595_shift_io_pins()
#define io145_get_output ic74hc595_get_pin(DIN15)
#endif
#define io145_config_input
#define io145_config_pullup
#define io145_get_input 0
#else
#define io145_config_output
#define io145_set_output
#define io145_clear_output
#define io145_toggle_output
#define io145_get_output 0
#define io145_config_input
#define io145_config_pullup
#define io145_get_input 0
#endif
#if ASSERT_PIN_IO(DIN16)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io146_config_output mcu_config_output(DIN16)
#define io146_set_output mcu_set_output(DIN16)
#define io146_clear_output mcu_clear_output(DIN16)
#define io146_toggle_output mcu_toggle_output(DIN16)
#define io146_get_output mcu_get_output(DIN16)
#endif
#if !defined(DIN16_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io146_config_input mcu_config_input(DIN16)
#else
#define io146_config_input mcu_config_input(DIN16);mcu_config_pullup(DIN16)
#endif
#define io146_config_pullup mcu_config_pullup(DIN16)
#define io146_get_input mcu_get_input(DIN16)
#elif ASSERT_PIN_EXTENDED(DIN16)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io146_config_output
#define io146_set_output ic74hc595_set_pin(DIN16);ic74hc595_shift_io_pins()
#define io146_clear_output ic74hc595_clear_pin(DIN16);ic74hc595_shift_io_pins()
#define io146_toggle_output ic74hc595_toggle_pin(DIN16);ic74hc595_shift_io_pins()
#define io146_get_output ic74hc595_get_pin(DIN16)
#endif
#define io146_config_input
#define io146_config_pullup
#define io146_get_input 0
#else
#define io146_config_output
#define io146_set_output
#define io146_clear_output
#define io146_toggle_output
#define io146_get_output 0
#define io146_config_input
#define io146_config_pullup
#define io146_get_input 0
#endif
#if ASSERT_PIN_IO(DIN17)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io147_config_output mcu_config_output(DIN17)
#define io147_set_output mcu_set_output(DIN17)
#define io147_clear_output mcu_clear_output(DIN17)
#define io147_toggle_output mcu_toggle_output(DIN17)
#define io147_get_output mcu_get_output(DIN17)
#endif
#if !defined(DIN17_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io147_config_input mcu_config_input(DIN17)
#else
#define io147_config_input mcu_config_input(DIN17);mcu_config_pullup(DIN17)
#endif
#define io147_config_pullup mcu_config_pullup(DIN17)
#define io147_get_input mcu_get_input(DIN17)
#elif ASSERT_PIN_EXTENDED(DIN17)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io147_config_output
#define io147_set_output ic74hc595_set_pin(DIN17);ic74hc595_shift_io_pins()
#define io147_clear_output ic74hc595_clear_pin(DIN17);ic74hc595_shift_io_pins()
#define io147_toggle_output ic74hc595_toggle_pin(DIN17);ic74hc595_shift_io_pins()
#define io147_get_output ic74hc595_get_pin(DIN17)
#endif
#define io147_config_input
#define io147_config_pullup
#define io147_get_input 0
#else
#define io147_config_output
#define io147_set_output
#define io147_clear_output
#define io147_toggle_output
#define io147_get_output 0
#define io147_config_input
#define io147_config_pullup
#define io147_get_input 0
#endif
#if ASSERT_PIN_IO(DIN18)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io148_config_output mcu_config_output(DIN18)
#define io148_set_output mcu_set_output(DIN18)
#define io148_clear_output mcu_clear_output(DIN18)
#define io148_toggle_output mcu_toggle_output(DIN18)
#define io148_get_output mcu_get_output(DIN18)
#endif
#if !defined(DIN18_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io148_config_input mcu_config_input(DIN18)
#else
#define io148_config_input mcu_config_input(DIN18);mcu_config_pullup(DIN18)
#endif
#define io148_config_pullup mcu_config_pullup(DIN18)
#define io148_get_input mcu_get_input(DIN18)
#elif ASSERT_PIN_EXTENDED(DIN18)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io148_config_output
#define io148_set_output ic74hc595_set_pin(DIN18);ic74hc595_shift_io_pins()
#define io148_clear_output ic74hc595_clear_pin(DIN18);ic74hc595_shift_io_pins()
#define io148_toggle_output ic74hc595_toggle_pin(DIN18);ic74hc595_shift_io_pins()
#define io148_get_output ic74hc595_get_pin(DIN18)
#endif
#define io148_config_input
#define io148_config_pullup
#define io148_get_input 0
#else
#define io148_config_output
#define io148_set_output
#define io148_clear_output
#define io148_toggle_output
#define io148_get_output 0
#define io148_config_input
#define io148_config_pullup
#define io148_get_input 0
#endif
#if ASSERT_PIN_IO(DIN19)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io149_config_output mcu_config_output(DIN19)
#define io149_set_output mcu_set_output(DIN19)
#define io149_clear_output mcu_clear_output(DIN19)
#define io149_toggle_output mcu_toggle_output(DIN19)
#define io149_get_output mcu_get_output(DIN19)
#endif
#if !defined(DIN19_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io149_config_input mcu_config_input(DIN19)
#else
#define io149_config_input mcu_config_input(DIN19);mcu_config_pullup(DIN19)
#endif
#define io149_config_pullup mcu_config_pullup(DIN19)
#define io149_get_input mcu_get_input(DIN19)
#elif ASSERT_PIN_EXTENDED(DIN19)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io149_config_output
#define io149_set_output ic74hc595_set_pin(DIN19);ic74hc595_shift_io_pins()
#define io149_clear_output ic74hc595_clear_pin(DIN19);ic74hc595_shift_io_pins()
#define io149_toggle_output ic74hc595_toggle_pin(DIN19);ic74hc595_shift_io_pins()
#define io149_get_output ic74hc595_get_pin(DIN19)
#endif
#define io149_config_input
#define io149_config_pullup
#define io149_get_input 0
#else
#define io149_config_output
#define io149_set_output
#define io149_clear_output
#define io149_toggle_output
#define io149_get_output 0
#define io149_config_input
#define io149_config_pullup
#define io149_get_input 0
#endif
#if ASSERT_PIN_IO(DIN20)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io150_config_output mcu_config_output(DIN20)
#define io150_set_output mcu_set_output(DIN20)
#define io150_clear_output mcu_clear_output(DIN20)
#define io150_toggle_output mcu_toggle_output(DIN20)
#define io150_get_output mcu_get_output(DIN20)
#endif
#if !defined(DIN20_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io150_config_input mcu_config_input(DIN20)
#else
#define io150_config_input mcu_config_input(DIN20);mcu_config_pullup(DIN20)
#endif
#define io150_config_pullup mcu_config_pullup(DIN20)
#define io150_get_input mcu_get_input(DIN20)
#elif ASSERT_PIN_EXTENDED(DIN20)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io150_config_output
#define io150_set_output ic74hc595_set_pin(DIN20);ic74hc595_shift_io_pins()
#define io150_clear_output ic74hc595_clear_pin(DIN20);ic74hc595_shift_io_pins()
#define io150_toggle_output ic74hc595_toggle_pin(DIN20);ic74hc595_shift_io_pins()
#define io150_get_output ic74hc595_get_pin(DIN20)
#endif
#define io150_config_input
#define io150_config_pullup
#define io150_get_input 0
#else
#define io150_config_output
#define io150_set_output
#define io150_clear_output
#define io150_toggle_output
#define io150_get_output 0
#define io150_config_input
#define io150_config_pullup
#define io150_get_input 0
#endif
#if ASSERT_PIN_IO(DIN21)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io151_config_output mcu_config_output(DIN21)
#define io151_set_output mcu_set_output(DIN21)
#define io151_clear_output mcu_clear_output(DIN21)
#define io151_toggle_output mcu_toggle_output(DIN21)
#define io151_get_output mcu_get_output(DIN21)
#endif
#if !defined(DIN21_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io151_config_input mcu_config_input(DIN21)
#else
#define io151_config_input mcu_config_input(DIN21);mcu_config_pullup(DIN21)
#endif
#define io151_config_pullup mcu_config_pullup(DIN21)
#define io151_get_input mcu_get_input(DIN21)
#elif ASSERT_PIN_EXTENDED(DIN21)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io151_config_output
#define io151_set_output ic74hc595_set_pin(DIN21);ic74hc595_shift_io_pins()
#define io151_clear_output ic74hc595_clear_pin(DIN21);ic74hc595_shift_io_pins()
#define io151_toggle_output ic74hc595_toggle_pin(DIN21);ic74hc595_shift_io_pins()
#define io151_get_output ic74hc595_get_pin(DIN21)
#endif
#define io151_config_input
#define io151_config_pullup
#define io151_get_input 0
#else
#define io151_config_output
#define io151_set_output
#define io151_clear_output
#define io151_toggle_output
#define io151_get_output 0
#define io151_config_input
#define io151_config_pullup
#define io151_get_input 0
#endif
#if ASSERT_PIN_IO(DIN22)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io152_config_output mcu_config_output(DIN22)
#define io152_set_output mcu_set_output(DIN22)
#define io152_clear_output mcu_clear_output(DIN22)
#define io152_toggle_output mcu_toggle_output(DIN22)
#define io152_get_output mcu_get_output(DIN22)
#endif
#if !defined(DIN22_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io152_config_input mcu_config_input(DIN22)
#else
#define io152_config_input mcu_config_input(DIN22);mcu_config_pullup(DIN22)
#endif
#define io152_config_pullup mcu_config_pullup(DIN22)
#define io152_get_input mcu_get_input(DIN22)
#elif ASSERT_PIN_EXTENDED(DIN22)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io152_config_output
#define io152_set_output ic74hc595_set_pin(DIN22);ic74hc595_shift_io_pins()
#define io152_clear_output ic74hc595_clear_pin(DIN22);ic74hc595_shift_io_pins()
#define io152_toggle_output ic74hc595_toggle_pin(DIN22);ic74hc595_shift_io_pins()
#define io152_get_output ic74hc595_get_pin(DIN22)
#endif
#define io152_config_input
#define io152_config_pullup
#define io152_get_input 0
#else
#define io152_config_output
#define io152_set_output
#define io152_clear_output
#define io152_toggle_output
#define io152_get_output 0
#define io152_config_input
#define io152_config_pullup
#define io152_get_input 0
#endif
#if ASSERT_PIN_IO(DIN23)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io153_config_output mcu_config_output(DIN23)
#define io153_set_output mcu_set_output(DIN23)
#define io153_clear_output mcu_clear_output(DIN23)
#define io153_toggle_output mcu_toggle_output(DIN23)
#define io153_get_output mcu_get_output(DIN23)
#endif
#if !defined(DIN23_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io153_config_input mcu_config_input(DIN23)
#else
#define io153_config_input mcu_config_input(DIN23);mcu_config_pullup(DIN23)
#endif
#define io153_config_pullup mcu_config_pullup(DIN23)
#define io153_get_input mcu_get_input(DIN23)
#elif ASSERT_PIN_EXTENDED(DIN23)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io153_config_output
#define io153_set_output ic74hc595_set_pin(DIN23);ic74hc595_shift_io_pins()
#define io153_clear_output ic74hc595_clear_pin(DIN23);ic74hc595_shift_io_pins()
#define io153_toggle_output ic74hc595_toggle_pin(DIN23);ic74hc595_shift_io_pins()
#define io153_get_output ic74hc595_get_pin(DIN23)
#endif
#define io153_config_input
#define io153_config_pullup
#define io153_get_input 0
#else
#define io153_config_output
#define io153_set_output
#define io153_clear_output
#define io153_toggle_output
#define io153_get_output 0
#define io153_config_input
#define io153_config_pullup
#define io153_get_input 0
#endif
#if ASSERT_PIN_IO(DIN24)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io154_config_output mcu_config_output(DIN24)
#define io154_set_output mcu_set_output(DIN24)
#define io154_clear_output mcu_clear_output(DIN24)
#define io154_toggle_output mcu_toggle_output(DIN24)
#define io154_get_output mcu_get_output(DIN24)
#endif
#if !defined(DIN24_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io154_config_input mcu_config_input(DIN24)
#else
#define io154_config_input mcu_config_input(DIN24);mcu_config_pullup(DIN24)
#endif
#define io154_config_pullup mcu_config_pullup(DIN24)
#define io154_get_input mcu_get_input(DIN24)
#elif ASSERT_PIN_EXTENDED(DIN24)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io154_config_output
#define io154_set_output ic74hc595_set_pin(DIN24);ic74hc595_shift_io_pins()
#define io154_clear_output ic74hc595_clear_pin(DIN24);ic74hc595_shift_io_pins()
#define io154_toggle_output ic74hc595_toggle_pin(DIN24);ic74hc595_shift_io_pins()
#define io154_get_output ic74hc595_get_pin(DIN24)
#endif
#define io154_config_input
#define io154_config_pullup
#define io154_get_input 0
#else
#define io154_config_output
#define io154_set_output
#define io154_clear_output
#define io154_toggle_output
#define io154_get_output 0
#define io154_config_input
#define io154_config_pullup
#define io154_get_input 0
#endif
#if ASSERT_PIN_IO(DIN25)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io155_config_output mcu_config_output(DIN25)
#define io155_set_output mcu_set_output(DIN25)
#define io155_clear_output mcu_clear_output(DIN25)
#define io155_toggle_output mcu_toggle_output(DIN25)
#define io155_get_output mcu_get_output(DIN25)
#endif
#if !defined(DIN25_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io155_config_input mcu_config_input(DIN25)
#else
#define io155_config_input mcu_config_input(DIN25);mcu_config_pullup(DIN25)
#endif
#define io155_config_pullup mcu_config_pullup(DIN25)
#define io155_get_input mcu_get_input(DIN25)
#elif ASSERT_PIN_EXTENDED(DIN25)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io155_config_output
#define io155_set_output ic74hc595_set_pin(DIN25);ic74hc595_shift_io_pins()
#define io155_clear_output ic74hc595_clear_pin(DIN25);ic74hc595_shift_io_pins()
#define io155_toggle_output ic74hc595_toggle_pin(DIN25);ic74hc595_shift_io_pins()
#define io155_get_output ic74hc595_get_pin(DIN25)
#endif
#define io155_config_input
#define io155_config_pullup
#define io155_get_input 0
#else
#define io155_config_output
#define io155_set_output
#define io155_clear_output
#define io155_toggle_output
#define io155_get_output 0
#define io155_config_input
#define io155_config_pullup
#define io155_get_input 0
#endif
#if ASSERT_PIN_IO(DIN26)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io156_config_output mcu_config_output(DIN26)
#define io156_set_output mcu_set_output(DIN26)
#define io156_clear_output mcu_clear_output(DIN26)
#define io156_toggle_output mcu_toggle_output(DIN26)
#define io156_get_output mcu_get_output(DIN26)
#endif
#if !defined(DIN26_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io156_config_input mcu_config_input(DIN26)
#else
#define io156_config_input mcu_config_input(DIN26);mcu_config_pullup(DIN26)
#endif
#define io156_config_pullup mcu_config_pullup(DIN26)
#define io156_get_input mcu_get_input(DIN26)
#elif ASSERT_PIN_EXTENDED(DIN26)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io156_config_output
#define io156_set_output ic74hc595_set_pin(DIN26);ic74hc595_shift_io_pins()
#define io156_clear_output ic74hc595_clear_pin(DIN26);ic74hc595_shift_io_pins()
#define io156_toggle_output ic74hc595_toggle_pin(DIN26);ic74hc595_shift_io_pins()
#define io156_get_output ic74hc595_get_pin(DIN26)
#endif
#define io156_config_input
#define io156_config_pullup
#define io156_get_input 0
#else
#define io156_config_output
#define io156_set_output
#define io156_clear_output
#define io156_toggle_output
#define io156_get_output 0
#define io156_config_input
#define io156_config_pullup
#define io156_get_input 0
#endif
#if ASSERT_PIN_IO(DIN27)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io157_config_output mcu_config_output(DIN27)
#define io157_set_output mcu_set_output(DIN27)
#define io157_clear_output mcu_clear_output(DIN27)
#define io157_toggle_output mcu_toggle_output(DIN27)
#define io157_get_output mcu_get_output(DIN27)
#endif
#if !defined(DIN27_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io157_config_input mcu_config_input(DIN27)
#else
#define io157_config_input mcu_config_input(DIN27);mcu_config_pullup(DIN27)
#endif
#define io157_config_pullup mcu_config_pullup(DIN27)
#define io157_get_input mcu_get_input(DIN27)
#elif ASSERT_PIN_EXTENDED(DIN27)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io157_config_output
#define io157_set_output ic74hc595_set_pin(DIN27);ic74hc595_shift_io_pins()
#define io157_clear_output ic74hc595_clear_pin(DIN27);ic74hc595_shift_io_pins()
#define io157_toggle_output ic74hc595_toggle_pin(DIN27);ic74hc595_shift_io_pins()
#define io157_get_output ic74hc595_get_pin(DIN27)
#endif
#define io157_config_input
#define io157_config_pullup
#define io157_get_input 0
#else
#define io157_config_output
#define io157_set_output
#define io157_clear_output
#define io157_toggle_output
#define io157_get_output 0
#define io157_config_input
#define io157_config_pullup
#define io157_get_input 0
#endif
#if ASSERT_PIN_IO(DIN28)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io158_config_output mcu_config_output(DIN28)
#define io158_set_output mcu_set_output(DIN28)
#define io158_clear_output mcu_clear_output(DIN28)
#define io158_toggle_output mcu_toggle_output(DIN28)
#define io158_get_output mcu_get_output(DIN28)
#endif
#if !defined(DIN28_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io158_config_input mcu_config_input(DIN28)
#else
#define io158_config_input mcu_config_input(DIN28);mcu_config_pullup(DIN28)
#endif
#define io158_config_pullup mcu_config_pullup(DIN28)
#define io158_get_input mcu_get_input(DIN28)
#elif ASSERT_PIN_EXTENDED(DIN28)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io158_config_output
#define io158_set_output ic74hc595_set_pin(DIN28);ic74hc595_shift_io_pins()
#define io158_clear_output ic74hc595_clear_pin(DIN28);ic74hc595_shift_io_pins()
#define io158_toggle_output ic74hc595_toggle_pin(DIN28);ic74hc595_shift_io_pins()
#define io158_get_output ic74hc595_get_pin(DIN28)
#endif
#define io158_config_input
#define io158_config_pullup
#define io158_get_input 0
#else
#define io158_config_output
#define io158_set_output
#define io158_clear_output
#define io158_toggle_output
#define io158_get_output 0
#define io158_config_input
#define io158_config_pullup
#define io158_get_input 0
#endif
#if ASSERT_PIN_IO(DIN29)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io159_config_output mcu_config_output(DIN29)
#define io159_set_output mcu_set_output(DIN29)
#define io159_clear_output mcu_clear_output(DIN29)
#define io159_toggle_output mcu_toggle_output(DIN29)
#define io159_get_output mcu_get_output(DIN29)
#endif
#if !defined(DIN29_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io159_config_input mcu_config_input(DIN29)
#else
#define io159_config_input mcu_config_input(DIN29);mcu_config_pullup(DIN29)
#endif
#define io159_config_pullup mcu_config_pullup(DIN29)
#define io159_get_input mcu_get_input(DIN29)
#elif ASSERT_PIN_EXTENDED(DIN29)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io159_config_output
#define io159_set_output ic74hc595_set_pin(DIN29);ic74hc595_shift_io_pins()
#define io159_clear_output ic74hc595_clear_pin(DIN29);ic74hc595_shift_io_pins()
#define io159_toggle_output ic74hc595_toggle_pin(DIN29);ic74hc595_shift_io_pins()
#define io159_get_output ic74hc595_get_pin(DIN29)
#endif
#define io159_config_input
#define io159_config_pullup
#define io159_get_input 0
#else
#define io159_config_output
#define io159_set_output
#define io159_clear_output
#define io159_toggle_output
#define io159_get_output 0
#define io159_config_input
#define io159_config_pullup
#define io159_get_input 0
#endif
#if ASSERT_PIN_IO(DIN30)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io160_config_output mcu_config_output(DIN30)
#define io160_set_output mcu_set_output(DIN30)
#define io160_clear_output mcu_clear_output(DIN30)
#define io160_toggle_output mcu_toggle_output(DIN30)
#define io160_get_output mcu_get_output(DIN30)
#endif
#if !defined(DIN30_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io160_config_input mcu_config_input(DIN30)
#else
#define io160_config_input mcu_config_input(DIN30);mcu_config_pullup(DIN30)
#endif
#define io160_config_pullup mcu_config_pullup(DIN30)
#define io160_get_input mcu_get_input(DIN30)
#elif ASSERT_PIN_EXTENDED(DIN30)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io160_config_output
#define io160_set_output ic74hc595_set_pin(DIN30);ic74hc595_shift_io_pins()
#define io160_clear_output ic74hc595_clear_pin(DIN30);ic74hc595_shift_io_pins()
#define io160_toggle_output ic74hc595_toggle_pin(DIN30);ic74hc595_shift_io_pins()
#define io160_get_output ic74hc595_get_pin(DIN30)
#endif
#define io160_config_input
#define io160_config_pullup
#define io160_get_input 0
#else
#define io160_config_output
#define io160_set_output
#define io160_clear_output
#define io160_toggle_output
#define io160_get_output 0
#define io160_config_input
#define io160_config_pullup
#define io160_get_input 0
#endif
#if ASSERT_PIN_IO(DIN31)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io161_config_output mcu_config_output(DIN31)
#define io161_set_output mcu_set_output(DIN31)
#define io161_clear_output mcu_clear_output(DIN31)
#define io161_toggle_output mcu_toggle_output(DIN31)
#define io161_get_output mcu_get_output(DIN31)
#endif
#if !defined(DIN31_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io161_config_input mcu_config_input(DIN31)
#else
#define io161_config_input mcu_config_input(DIN31);mcu_config_pullup(DIN31)
#endif
#define io161_config_pullup mcu_config_pullup(DIN31)
#define io161_get_input mcu_get_input(DIN31)
#elif ASSERT_PIN_EXTENDED(DIN31)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io161_config_output
#define io161_set_output ic74hc595_set_pin(DIN31);ic74hc595_shift_io_pins()
#define io161_clear_output ic74hc595_clear_pin(DIN31);ic74hc595_shift_io_pins()
#define io161_toggle_output ic74hc595_toggle_pin(DIN31);ic74hc595_shift_io_pins()
#define io161_get_output ic74hc595_get_pin(DIN31)
#endif
#define io161_config_input
#define io161_config_pullup
#define io161_get_input 0
#else
#define io161_config_output
#define io161_set_output
#define io161_clear_output
#define io161_toggle_output
#define io161_get_output 0
#define io161_config_input
#define io161_config_pullup
#define io161_get_input 0
#endif
#if ASSERT_PIN_IO(DIN32)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io162_config_output mcu_config_output(DIN32)
#define io162_set_output mcu_set_output(DIN32)
#define io162_clear_output mcu_clear_output(DIN32)
#define io162_toggle_output mcu_toggle_output(DIN32)
#define io162_get_output mcu_get_output(DIN32)
#endif
#if !defined(DIN32_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io162_config_input mcu_config_input(DIN32)
#else
#define io162_config_input mcu_config_input(DIN32);mcu_config_pullup(DIN32)
#endif
#define io162_config_pullup mcu_config_pullup(DIN32)
#define io162_get_input mcu_get_input(DIN32)
#elif ASSERT_PIN_EXTENDED(DIN32)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io162_config_output
#define io162_set_output ic74hc595_set_pin(DIN32);ic74hc595_shift_io_pins()
#define io162_clear_output ic74hc595_clear_pin(DIN32);ic74hc595_shift_io_pins()
#define io162_toggle_output ic74hc595_toggle_pin(DIN32);ic74hc595_shift_io_pins()
#define io162_get_output ic74hc595_get_pin(DIN32)
#endif
#define io162_config_input
#define io162_config_pullup
#define io162_get_input 0
#else
#define io162_config_output
#define io162_set_output
#define io162_clear_output
#define io162_toggle_output
#define io162_get_output 0
#define io162_config_input
#define io162_config_pullup
#define io162_get_input 0
#endif
#if ASSERT_PIN_IO(DIN33)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io163_config_output mcu_config_output(DIN33)
#define io163_set_output mcu_set_output(DIN33)
#define io163_clear_output mcu_clear_output(DIN33)
#define io163_toggle_output mcu_toggle_output(DIN33)
#define io163_get_output mcu_get_output(DIN33)
#endif
#if !defined(DIN33_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io163_config_input mcu_config_input(DIN33)
#else
#define io163_config_input mcu_config_input(DIN33);mcu_config_pullup(DIN33)
#endif
#define io163_config_pullup mcu_config_pullup(DIN33)
#define io163_get_input mcu_get_input(DIN33)
#elif ASSERT_PIN_EXTENDED(DIN33)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io163_config_output
#define io163_set_output ic74hc595_set_pin(DIN33);ic74hc595_shift_io_pins()
#define io163_clear_output ic74hc595_clear_pin(DIN33);ic74hc595_shift_io_pins()
#define io163_toggle_output ic74hc595_toggle_pin(DIN33);ic74hc595_shift_io_pins()
#define io163_get_output ic74hc595_get_pin(DIN33)
#endif
#define io163_config_input
#define io163_config_pullup
#define io163_get_input 0
#else
#define io163_config_output
#define io163_set_output
#define io163_clear_output
#define io163_toggle_output
#define io163_get_output 0
#define io163_config_input
#define io163_config_pullup
#define io163_get_input 0
#endif
#if ASSERT_PIN_IO(DIN34)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io164_config_output mcu_config_output(DIN34)
#define io164_set_output mcu_set_output(DIN34)
#define io164_clear_output mcu_clear_output(DIN34)
#define io164_toggle_output mcu_toggle_output(DIN34)
#define io164_get_output mcu_get_output(DIN34)
#endif
#if !defined(DIN34_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io164_config_input mcu_config_input(DIN34)
#else
#define io164_config_input mcu_config_input(DIN34);mcu_config_pullup(DIN34)
#endif
#define io164_config_pullup mcu_config_pullup(DIN34)
#define io164_get_input mcu_get_input(DIN34)
#elif ASSERT_PIN_EXTENDED(DIN34)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io164_config_output
#define io164_set_output ic74hc595_set_pin(DIN34);ic74hc595_shift_io_pins()
#define io164_clear_output ic74hc595_clear_pin(DIN34);ic74hc595_shift_io_pins()
#define io164_toggle_output ic74hc595_toggle_pin(DIN34);ic74hc595_shift_io_pins()
#define io164_get_output ic74hc595_get_pin(DIN34)
#endif
#define io164_config_input
#define io164_config_pullup
#define io164_get_input 0
#else
#define io164_config_output
#define io164_set_output
#define io164_clear_output
#define io164_toggle_output
#define io164_get_output 0
#define io164_config_input
#define io164_config_pullup
#define io164_get_input 0
#endif
#if ASSERT_PIN_IO(DIN35)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io165_config_output mcu_config_output(DIN35)
#define io165_set_output mcu_set_output(DIN35)
#define io165_clear_output mcu_clear_output(DIN35)
#define io165_toggle_output mcu_toggle_output(DIN35)
#define io165_get_output mcu_get_output(DIN35)
#endif
#if !defined(DIN35_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io165_config_input mcu_config_input(DIN35)
#else
#define io165_config_input mcu_config_input(DIN35);mcu_config_pullup(DIN35)
#endif
#define io165_config_pullup mcu_config_pullup(DIN35)
#define io165_get_input mcu_get_input(DIN35)
#elif ASSERT_PIN_EXTENDED(DIN35)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io165_config_output
#define io165_set_output ic74hc595_set_pin(DIN35);ic74hc595_shift_io_pins()
#define io165_clear_output ic74hc595_clear_pin(DIN35);ic74hc595_shift_io_pins()
#define io165_toggle_output ic74hc595_toggle_pin(DIN35);ic74hc595_shift_io_pins()
#define io165_get_output ic74hc595_get_pin(DIN35)
#endif
#define io165_config_input
#define io165_config_pullup
#define io165_get_input 0
#else
#define io165_config_output
#define io165_set_output
#define io165_clear_output
#define io165_toggle_output
#define io165_get_output 0
#define io165_config_input
#define io165_config_pullup
#define io165_get_input 0
#endif
#if ASSERT_PIN_IO(DIN36)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io166_config_output mcu_config_output(DIN36)
#define io166_set_output mcu_set_output(DIN36)
#define io166_clear_output mcu_clear_output(DIN36)
#define io166_toggle_output mcu_toggle_output(DIN36)
#define io166_get_output mcu_get_output(DIN36)
#endif
#if !defined(DIN36_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io166_config_input mcu_config_input(DIN36)
#else
#define io166_config_input mcu_config_input(DIN36);mcu_config_pullup(DIN36)
#endif
#define io166_config_pullup mcu_config_pullup(DIN36)
#define io166_get_input mcu_get_input(DIN36)
#elif ASSERT_PIN_EXTENDED(DIN36)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io166_config_output
#define io166_set_output ic74hc595_set_pin(DIN36);ic74hc595_shift_io_pins()
#define io166_clear_output ic74hc595_clear_pin(DIN36);ic74hc595_shift_io_pins()
#define io166_toggle_output ic74hc595_toggle_pin(DIN36);ic74hc595_shift_io_pins()
#define io166_get_output ic74hc595_get_pin(DIN36)
#endif
#define io166_config_input
#define io166_config_pullup
#define io166_get_input 0
#else
#define io166_config_output
#define io166_set_output
#define io166_clear_output
#define io166_toggle_output
#define io166_get_output 0
#define io166_config_input
#define io166_config_pullup
#define io166_get_input 0
#endif
#if ASSERT_PIN_IO(DIN37)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io167_config_output mcu_config_output(DIN37)
#define io167_set_output mcu_set_output(DIN37)
#define io167_clear_output mcu_clear_output(DIN37)
#define io167_toggle_output mcu_toggle_output(DIN37)
#define io167_get_output mcu_get_output(DIN37)
#endif
#if !defined(DIN37_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io167_config_input mcu_config_input(DIN37)
#else
#define io167_config_input mcu_config_input(DIN37);mcu_config_pullup(DIN37)
#endif
#define io167_config_pullup mcu_config_pullup(DIN37)
#define io167_get_input mcu_get_input(DIN37)
#elif ASSERT_PIN_EXTENDED(DIN37)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io167_config_output
#define io167_set_output ic74hc595_set_pin(DIN37);ic74hc595_shift_io_pins()
#define io167_clear_output ic74hc595_clear_pin(DIN37);ic74hc595_shift_io_pins()
#define io167_toggle_output ic74hc595_toggle_pin(DIN37);ic74hc595_shift_io_pins()
#define io167_get_output ic74hc595_get_pin(DIN37)
#endif
#define io167_config_input
#define io167_config_pullup
#define io167_get_input 0
#else
#define io167_config_output
#define io167_set_output
#define io167_clear_output
#define io167_toggle_output
#define io167_get_output 0
#define io167_config_input
#define io167_config_pullup
#define io167_get_input 0
#endif
#if ASSERT_PIN_IO(DIN38)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io168_config_output mcu_config_output(DIN38)
#define io168_set_output mcu_set_output(DIN38)
#define io168_clear_output mcu_clear_output(DIN38)
#define io168_toggle_output mcu_toggle_output(DIN38)
#define io168_get_output mcu_get_output(DIN38)
#endif
#if !defined(DIN38_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io168_config_input mcu_config_input(DIN38)
#else
#define io168_config_input mcu_config_input(DIN38);mcu_config_pullup(DIN38)
#endif
#define io168_config_pullup mcu_config_pullup(DIN38)
#define io168_get_input mcu_get_input(DIN38)
#elif ASSERT_PIN_EXTENDED(DIN38)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io168_config_output
#define io168_set_output ic74hc595_set_pin(DIN38);ic74hc595_shift_io_pins()
#define io168_clear_output ic74hc595_clear_pin(DIN38);ic74hc595_shift_io_pins()
#define io168_toggle_output ic74hc595_toggle_pin(DIN38);ic74hc595_shift_io_pins()
#define io168_get_output ic74hc595_get_pin(DIN38)
#endif
#define io168_config_input
#define io168_config_pullup
#define io168_get_input 0
#else
#define io168_config_output
#define io168_set_output
#define io168_clear_output
#define io168_toggle_output
#define io168_get_output 0
#define io168_config_input
#define io168_config_pullup
#define io168_get_input 0
#endif
#if ASSERT_PIN_IO(DIN39)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io169_config_output mcu_config_output(DIN39)
#define io169_set_output mcu_set_output(DIN39)
#define io169_clear_output mcu_clear_output(DIN39)
#define io169_toggle_output mcu_toggle_output(DIN39)
#define io169_get_output mcu_get_output(DIN39)
#endif
#if !defined(DIN39_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io169_config_input mcu_config_input(DIN39)
#else
#define io169_config_input mcu_config_input(DIN39);mcu_config_pullup(DIN39)
#endif
#define io169_config_pullup mcu_config_pullup(DIN39)
#define io169_get_input mcu_get_input(DIN39)
#elif ASSERT_PIN_EXTENDED(DIN39)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io169_config_output
#define io169_set_output ic74hc595_set_pin(DIN39);ic74hc595_shift_io_pins()
#define io169_clear_output ic74hc595_clear_pin(DIN39);ic74hc595_shift_io_pins()
#define io169_toggle_output ic74hc595_toggle_pin(DIN39);ic74hc595_shift_io_pins()
#define io169_get_output ic74hc595_get_pin(DIN39)
#endif
#define io169_config_input
#define io169_config_pullup
#define io169_get_input 0
#else
#define io169_config_output
#define io169_set_output
#define io169_clear_output
#define io169_toggle_output
#define io169_get_output 0
#define io169_config_input
#define io169_config_pullup
#define io169_get_input 0
#endif
#if ASSERT_PIN_IO(DIN40)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io170_config_output mcu_config_output(DIN40)
#define io170_set_output mcu_set_output(DIN40)
#define io170_clear_output mcu_clear_output(DIN40)
#define io170_toggle_output mcu_toggle_output(DIN40)
#define io170_get_output mcu_get_output(DIN40)
#endif
#if !defined(DIN40_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io170_config_input mcu_config_input(DIN40)
#else
#define io170_config_input mcu_config_input(DIN40);mcu_config_pullup(DIN40)
#endif
#define io170_config_pullup mcu_config_pullup(DIN40)
#define io170_get_input mcu_get_input(DIN40)
#elif ASSERT_PIN_EXTENDED(DIN40)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io170_config_output
#define io170_set_output ic74hc595_set_pin(DIN40);ic74hc595_shift_io_pins()
#define io170_clear_output ic74hc595_clear_pin(DIN40);ic74hc595_shift_io_pins()
#define io170_toggle_output ic74hc595_toggle_pin(DIN40);ic74hc595_shift_io_pins()
#define io170_get_output ic74hc595_get_pin(DIN40)
#endif
#define io170_config_input
#define io170_config_pullup
#define io170_get_input 0
#else
#define io170_config_output
#define io170_set_output
#define io170_clear_output
#define io170_toggle_output
#define io170_get_output 0
#define io170_config_input
#define io170_config_pullup
#define io170_get_input 0
#endif
#if ASSERT_PIN_IO(DIN41)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io171_config_output mcu_config_output(DIN41)
#define io171_set_output mcu_set_output(DIN41)
#define io171_clear_output mcu_clear_output(DIN41)
#define io171_toggle_output mcu_toggle_output(DIN41)
#define io171_get_output mcu_get_output(DIN41)
#endif
#if !defined(DIN41_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io171_config_input mcu_config_input(DIN41)
#else
#define io171_config_input mcu_config_input(DIN41);mcu_config_pullup(DIN41)
#endif
#define io171_config_pullup mcu_config_pullup(DIN41)
#define io171_get_input mcu_get_input(DIN41)
#elif ASSERT_PIN_EXTENDED(DIN41)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io171_config_output
#define io171_set_output ic74hc595_set_pin(DIN41);ic74hc595_shift_io_pins()
#define io171_clear_output ic74hc595_clear_pin(DIN41);ic74hc595_shift_io_pins()
#define io171_toggle_output ic74hc595_toggle_pin(DIN41);ic74hc595_shift_io_pins()
#define io171_get_output ic74hc595_get_pin(DIN41)
#endif
#define io171_config_input
#define io171_config_pullup
#define io171_get_input 0
#else
#define io171_config_output
#define io171_set_output
#define io171_clear_output
#define io171_toggle_output
#define io171_get_output 0
#define io171_config_input
#define io171_config_pullup
#define io171_get_input 0
#endif
#if ASSERT_PIN_IO(DIN42)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io172_config_output mcu_config_output(DIN42)
#define io172_set_output mcu_set_output(DIN42)
#define io172_clear_output mcu_clear_output(DIN42)
#define io172_toggle_output mcu_toggle_output(DIN42)
#define io172_get_output mcu_get_output(DIN42)
#endif
#if !defined(DIN42_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io172_config_input mcu_config_input(DIN42)
#else
#define io172_config_input mcu_config_input(DIN42);mcu_config_pullup(DIN42)
#endif
#define io172_config_pullup mcu_config_pullup(DIN42)
#define io172_get_input mcu_get_input(DIN42)
#elif ASSERT_PIN_EXTENDED(DIN42)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io172_config_output
#define io172_set_output ic74hc595_set_pin(DIN42);ic74hc595_shift_io_pins()
#define io172_clear_output ic74hc595_clear_pin(DIN42);ic74hc595_shift_io_pins()
#define io172_toggle_output ic74hc595_toggle_pin(DIN42);ic74hc595_shift_io_pins()
#define io172_get_output ic74hc595_get_pin(DIN42)
#endif
#define io172_config_input
#define io172_config_pullup
#define io172_get_input 0
#else
#define io172_config_output
#define io172_set_output
#define io172_clear_output
#define io172_toggle_output
#define io172_get_output 0
#define io172_config_input
#define io172_config_pullup
#define io172_get_input 0
#endif
#if ASSERT_PIN_IO(DIN43)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io173_config_output mcu_config_output(DIN43)
#define io173_set_output mcu_set_output(DIN43)
#define io173_clear_output mcu_clear_output(DIN43)
#define io173_toggle_output mcu_toggle_output(DIN43)
#define io173_get_output mcu_get_output(DIN43)
#endif
#if !defined(DIN43_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io173_config_input mcu_config_input(DIN43)
#else
#define io173_config_input mcu_config_input(DIN43);mcu_config_pullup(DIN43)
#endif
#define io173_config_pullup mcu_config_pullup(DIN43)
#define io173_get_input mcu_get_input(DIN43)
#elif ASSERT_PIN_EXTENDED(DIN43)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io173_config_output
#define io173_set_output ic74hc595_set_pin(DIN43);ic74hc595_shift_io_pins()
#define io173_clear_output ic74hc595_clear_pin(DIN43);ic74hc595_shift_io_pins()
#define io173_toggle_output ic74hc595_toggle_pin(DIN43);ic74hc595_shift_io_pins()
#define io173_get_output ic74hc595_get_pin(DIN43)
#endif
#define io173_config_input
#define io173_config_pullup
#define io173_get_input 0
#else
#define io173_config_output
#define io173_set_output
#define io173_clear_output
#define io173_toggle_output
#define io173_get_output 0
#define io173_config_input
#define io173_config_pullup
#define io173_get_input 0
#endif
#if ASSERT_PIN_IO(DIN44)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io174_config_output mcu_config_output(DIN44)
#define io174_set_output mcu_set_output(DIN44)
#define io174_clear_output mcu_clear_output(DIN44)
#define io174_toggle_output mcu_toggle_output(DIN44)
#define io174_get_output mcu_get_output(DIN44)
#endif
#if !defined(DIN44_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io174_config_input mcu_config_input(DIN44)
#else
#define io174_config_input mcu_config_input(DIN44);mcu_config_pullup(DIN44)
#endif
#define io174_config_pullup mcu_config_pullup(DIN44)
#define io174_get_input mcu_get_input(DIN44)
#elif ASSERT_PIN_EXTENDED(DIN44)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io174_config_output
#define io174_set_output ic74hc595_set_pin(DIN44);ic74hc595_shift_io_pins()
#define io174_clear_output ic74hc595_clear_pin(DIN44);ic74hc595_shift_io_pins()
#define io174_toggle_output ic74hc595_toggle_pin(DIN44);ic74hc595_shift_io_pins()
#define io174_get_output ic74hc595_get_pin(DIN44)
#endif
#define io174_config_input
#define io174_config_pullup
#define io174_get_input 0
#else
#define io174_config_output
#define io174_set_output
#define io174_clear_output
#define io174_toggle_output
#define io174_get_output 0
#define io174_config_input
#define io174_config_pullup
#define io174_get_input 0
#endif
#if ASSERT_PIN_IO(DIN45)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io175_config_output mcu_config_output(DIN45)
#define io175_set_output mcu_set_output(DIN45)
#define io175_clear_output mcu_clear_output(DIN45)
#define io175_toggle_output mcu_toggle_output(DIN45)
#define io175_get_output mcu_get_output(DIN45)
#endif
#if !defined(DIN45_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io175_config_input mcu_config_input(DIN45)
#else
#define io175_config_input mcu_config_input(DIN45);mcu_config_pullup(DIN45)
#endif
#define io175_config_pullup mcu_config_pullup(DIN45)
#define io175_get_input mcu_get_input(DIN45)
#elif ASSERT_PIN_EXTENDED(DIN45)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io175_config_output
#define io175_set_output ic74hc595_set_pin(DIN45);ic74hc595_shift_io_pins()
#define io175_clear_output ic74hc595_clear_pin(DIN45);ic74hc595_shift_io_pins()
#define io175_toggle_output ic74hc595_toggle_pin(DIN45);ic74hc595_shift_io_pins()
#define io175_get_output ic74hc595_get_pin(DIN45)
#endif
#define io175_config_input
#define io175_config_pullup
#define io175_get_input 0
#else
#define io175_config_output
#define io175_set_output
#define io175_clear_output
#define io175_toggle_output
#define io175_get_output 0
#define io175_config_input
#define io175_config_pullup
#define io175_get_input 0
#endif
#if ASSERT_PIN_IO(DIN46)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io176_config_output mcu_config_output(DIN46)
#define io176_set_output mcu_set_output(DIN46)
#define io176_clear_output mcu_clear_output(DIN46)
#define io176_toggle_output mcu_toggle_output(DIN46)
#define io176_get_output mcu_get_output(DIN46)
#endif
#if !defined(DIN46_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io176_config_input mcu_config_input(DIN46)
#else
#define io176_config_input mcu_config_input(DIN46);mcu_config_pullup(DIN46)
#endif
#define io176_config_pullup mcu_config_pullup(DIN46)
#define io176_get_input mcu_get_input(DIN46)
#elif ASSERT_PIN_EXTENDED(DIN46)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io176_config_output
#define io176_set_output ic74hc595_set_pin(DIN46);ic74hc595_shift_io_pins()
#define io176_clear_output ic74hc595_clear_pin(DIN46);ic74hc595_shift_io_pins()
#define io176_toggle_output ic74hc595_toggle_pin(DIN46);ic74hc595_shift_io_pins()
#define io176_get_output ic74hc595_get_pin(DIN46)
#endif
#define io176_config_input
#define io176_config_pullup
#define io176_get_input 0
#else
#define io176_config_output
#define io176_set_output
#define io176_clear_output
#define io176_toggle_output
#define io176_get_output 0
#define io176_config_input
#define io176_config_pullup
#define io176_get_input 0
#endif
#if ASSERT_PIN_IO(DIN47)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io177_config_output mcu_config_output(DIN47)
#define io177_set_output mcu_set_output(DIN47)
#define io177_clear_output mcu_clear_output(DIN47)
#define io177_toggle_output mcu_toggle_output(DIN47)
#define io177_get_output mcu_get_output(DIN47)
#endif
#if !defined(DIN47_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io177_config_input mcu_config_input(DIN47)
#else
#define io177_config_input mcu_config_input(DIN47);mcu_config_pullup(DIN47)
#endif
#define io177_config_pullup mcu_config_pullup(DIN47)
#define io177_get_input mcu_get_input(DIN47)
#elif ASSERT_PIN_EXTENDED(DIN47)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io177_config_output
#define io177_set_output ic74hc595_set_pin(DIN47);ic74hc595_shift_io_pins()
#define io177_clear_output ic74hc595_clear_pin(DIN47);ic74hc595_shift_io_pins()
#define io177_toggle_output ic74hc595_toggle_pin(DIN47);ic74hc595_shift_io_pins()
#define io177_get_output ic74hc595_get_pin(DIN47)
#endif
#define io177_config_input
#define io177_config_pullup
#define io177_get_input 0
#else
#define io177_config_output
#define io177_set_output
#define io177_clear_output
#define io177_toggle_output
#define io177_get_output 0
#define io177_config_input
#define io177_config_pullup
#define io177_get_input 0
#endif
#if ASSERT_PIN_IO(DIN48)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io178_config_output mcu_config_output(DIN48)
#define io178_set_output mcu_set_output(DIN48)
#define io178_clear_output mcu_clear_output(DIN48)
#define io178_toggle_output mcu_toggle_output(DIN48)
#define io178_get_output mcu_get_output(DIN48)
#endif
#if !defined(DIN48_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io178_config_input mcu_config_input(DIN48)
#else
#define io178_config_input mcu_config_input(DIN48);mcu_config_pullup(DIN48)
#endif
#define io178_config_pullup mcu_config_pullup(DIN48)
#define io178_get_input mcu_get_input(DIN48)
#elif ASSERT_PIN_EXTENDED(DIN48)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io178_config_output
#define io178_set_output ic74hc595_set_pin(DIN48);ic74hc595_shift_io_pins()
#define io178_clear_output ic74hc595_clear_pin(DIN48);ic74hc595_shift_io_pins()
#define io178_toggle_output ic74hc595_toggle_pin(DIN48);ic74hc595_shift_io_pins()
#define io178_get_output ic74hc595_get_pin(DIN48)
#endif
#define io178_config_input
#define io178_config_pullup
#define io178_get_input 0
#else
#define io178_config_output
#define io178_set_output
#define io178_clear_output
#define io178_toggle_output
#define io178_get_output 0
#define io178_config_input
#define io178_config_pullup
#define io178_get_input 0
#endif
#if ASSERT_PIN_IO(DIN49)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io179_config_output mcu_config_output(DIN49)
#define io179_set_output mcu_set_output(DIN49)
#define io179_clear_output mcu_clear_output(DIN49)
#define io179_toggle_output mcu_toggle_output(DIN49)
#define io179_get_output mcu_get_output(DIN49)
#endif
#if !defined(DIN49_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io179_config_input mcu_config_input(DIN49)
#else
#define io179_config_input mcu_config_input(DIN49);mcu_config_pullup(DIN49)
#endif
#define io179_config_pullup mcu_config_pullup(DIN49)
#define io179_get_input mcu_get_input(DIN49)
#elif ASSERT_PIN_EXTENDED(DIN49)
#ifdef DISABLE_HAL_CONFIG_PROTECTION
#define io179_config_output
#define io179_set_output ic74hc595_set_pin(DIN49);ic74hc595_shift_io_pins()
#define io179_clear_output ic74hc595_clear_pin(DIN49);ic74hc595_shift_io_pins()
#define io179_toggle_output ic74hc595_toggle_pin(DIN49);ic74hc595_shift_io_pins()
#define io179_get_output ic74hc595_get_pin(DIN49)
#endif
#define io179_config_input
#define io179_config_pullup
#define io179_get_input 0
#else
#define io179_config_output
#define io179_set_output
#define io179_clear_output
#define io179_toggle_output
#define io179_get_output 0
#define io179_config_input
#define io179_config_pullup
#define io179_get_input 0
#endif
#if ASSERT_PIN_IO(SPI_CS)
#define io207_config_output mcu_config_output(SPI_CS)
#define io207_set_output mcu_set_output(SPI_CS)
#define io207_clear_output mcu_clear_output(SPI_CS)
#define io207_toggle_output mcu_toggle_output(SPI_CS)
#define io207_get_output mcu_get_output(SPI_CS)
#if !defined(SPI_CS_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io207_config_input mcu_config_input(SPI_CS)
#else
#define io207_config_input mcu_config_input(SPI_CS);mcu_config_pullup(SPI_CS)
#endif
#define io207_config_pullup mcu_config_pullup(SPI_CS)
#define io207_get_input mcu_get_input(SPI_CS)
#elif ASSERT_PIN_EXTENDED(SPI_CS)
#define io207_config_output
#define io207_set_output ic74hc595_set_pin(SPI_CS);ic74hc595_shift_io_pins()
#define io207_clear_output ic74hc595_clear_pin(SPI_CS);ic74hc595_shift_io_pins()
#define io207_toggle_output ic74hc595_toggle_pin(SPI_CS);ic74hc595_shift_io_pins()
#define io207_get_output ic74hc595_get_pin(SPI_CS)
#define io207_config_input
#define io207_config_pullup
#define io207_get_input 0
#else
#define io207_config_output
#define io207_set_output
#define io207_clear_output
#define io207_toggle_output
#define io207_get_output 0
#define io207_config_input
#define io207_config_pullup
#define io207_get_input 0
#endif
#if ASSERT_PIN_IO(SPI2_CS)
#define io215_config_output mcu_config_output(SPI2_CS)
#define io215_set_output mcu_set_output(SPI2_CS)
#define io215_clear_output mcu_clear_output(SPI2_CS)
#define io215_toggle_output mcu_toggle_output(SPI2_CS)
#define io215_get_output mcu_get_output(SPI2_CS)
#if !defined(SPI2_CS_PULLUP) && !defined(FORCE_HAL_IO_DIRECTION_ONREQUEST)
#define io215_config_input mcu_config_input(SPI2_CS)
#else
#define io215_config_input mcu_config_input(SPI2_CS);mcu_config_pullup(SPI2_CS)
#endif
#define io215_config_pullup mcu_config_pullup(SPI2_CS)
#define io215_get_input mcu_get_input(SPI2_CS)
#elif ASSERT_PIN_EXTENDED(SPI2_CS)
#define io215_config_output
#define io215_set_output ic74hc595_set_pin(SPI2_CS);ic74hc595_shift_io_pins()
#define io215_clear_output ic74hc595_clear_pin(SPI2_CS);ic74hc595_shift_io_pins()
#define io215_toggle_output ic74hc595_toggle_pin(SPI2_CS);ic74hc595_shift_io_pins()
#define io215_get_output ic74hc595_get_pin(SPI2_CS)
#define io215_config_input
#define io215_config_pullup
#define io215_get_input 0
#else
#define io215_config_output
#define io215_set_output
#define io215_clear_output
#define io215_toggle_output
#define io215_get_output 0
#define io215_config_input
#define io215_config_pullup
#define io215_get_input 0
#endif

	/*PWM*/
	extern uint8_t g_soft_pwm_res;
	extern uint8_t g_io_soft_pwm[16];
	extern uint8_t mcu_softpwm_freq_config(uint16_t freq);

#if ASSERT_PIN_IO(PWM0)
#define io25_config_pwm(freq) mcu_config_pwm(PWM0, freq)
#define io25_set_pwm(value) mcu_set_pwm(PWM0, value)
#define io25_get_pwm mcu_get_pwm(PWM0)
#elif ASSERT_PIN_EXTENDED(PWM0)
#define io25_config_pwm(freq)                       \
	{                                                 \
		g_soft_pwm_res = mcu_softpwm_freq_config(freq); \
	}
#define io25_set_pwm(value)                                 \
	{                                                         \
		g_io_soft_pwm[PWM0 - PWM_PINS_OFFSET] = (0xFF & value); \
	}
#define io25_get_pwm g_io_soft_pwm[PWM0 - PWM_PINS_OFFSET]
#else
#define io25_config_pwm(freq)
#define io25_set_pwm(value)
#define io25_get_pwm 0
#endif
#if ASSERT_PIN_IO(PWM1)
#define io26_config_pwm(freq) mcu_config_pwm(PWM1, freq)
#define io26_set_pwm(value) mcu_set_pwm(PWM1, value)
#define io26_get_pwm mcu_get_pwm(PWM1)
#elif ASSERT_PIN_EXTENDED(PWM1)
#define io26_config_pwm(freq)                       \
	{                                                 \
		g_soft_pwm_res = mcu_softpwm_freq_config(freq); \
	}
#define io26_set_pwm(value)                                 \
	{                                                         \
		g_io_soft_pwm[PWM1 - PWM_PINS_OFFSET] = (0xFF & value); \
	}
#define io26_get_pwm g_io_soft_pwm[PWM1 - PWM_PINS_OFFSET]
#else
#define io26_config_pwm(freq)
#define io26_set_pwm(value)
#define io26_get_pwm 0
#endif
#if ASSERT_PIN_IO(PWM2)
#define io27_config_pwm(freq) mcu_config_pwm(PWM2, freq)
#define io27_set_pwm(value) mcu_set_pwm(PWM2, value)
#define io27_get_pwm mcu_get_pwm(PWM2)
#elif ASSERT_PIN_EXTENDED(PWM2)
#define io27_config_pwm(freq)                       \
	{                                                 \
		g_soft_pwm_res = mcu_softpwm_freq_config(freq); \
	}
#define io27_set_pwm(value)                                 \
	{                                                         \
		g_io_soft_pwm[PWM2 - PWM_PINS_OFFSET] = (0xFF & value); \
	}
#define io27_get_pwm g_io_soft_pwm[PWM2 - PWM_PINS_OFFSET]
#else
#define io27_config_pwm(freq)
#define io27_set_pwm(value)
#define io27_get_pwm 0
#endif
#if ASSERT_PIN_IO(PWM3)
#define io28_config_pwm(freq) mcu_config_pwm(PWM3, freq)
#define io28_set_pwm(value) mcu_set_pwm(PWM3, value)
#define io28_get_pwm mcu_get_pwm(PWM3)
#elif ASSERT_PIN_EXTENDED(PWM3)
#define io28_config_pwm(freq)                       \
	{                                                 \
		g_soft_pwm_res = mcu_softpwm_freq_config(freq); \
	}
#define io28_set_pwm(value)                                 \
	{                                                         \
		g_io_soft_pwm[PWM3 - PWM_PINS_OFFSET] = (0xFF & value); \
	}
#define io28_get_pwm g_io_soft_pwm[PWM3 - PWM_PINS_OFFSET]
#else
#define io28_config_pwm(freq)
#define io28_set_pwm(value)
#define io28_get_pwm 0
#endif
#if ASSERT_PIN_IO(PWM4)
#define io29_config_pwm(freq) mcu_config_pwm(PWM4, freq)
#define io29_set_pwm(value) mcu_set_pwm(PWM4, value)
#define io29_get_pwm mcu_get_pwm(PWM4)
#elif ASSERT_PIN_EXTENDED(PWM4)
#define io29_config_pwm(freq)                       \
	{                                                 \
		g_soft_pwm_res = mcu_softpwm_freq_config(freq); \
	}
#define io29_set_pwm(value)                                 \
	{                                                         \
		g_io_soft_pwm[PWM4 - PWM_PINS_OFFSET] = (0xFF & value); \
	}
#define io29_get_pwm g_io_soft_pwm[PWM4 - PWM_PINS_OFFSET]
#else
#define io29_config_pwm(freq)
#define io29_set_pwm(value)
#define io29_get_pwm 0
#endif
#if ASSERT_PIN_IO(PWM5)
#define io30_config_pwm(freq) mcu_config_pwm(PWM5, freq)
#define io30_set_pwm(value) mcu_set_pwm(PWM5, value)
#define io30_get_pwm mcu_get_pwm(PWM5)
#elif ASSERT_PIN_EXTENDED(PWM5)
#define io30_config_pwm(freq)                       \
	{                                                 \
		g_soft_pwm_res = mcu_softpwm_freq_config(freq); \
	}
#define io30_set_pwm(value)                                 \
	{                                                         \
		g_io_soft_pwm[PWM5 - PWM_PINS_OFFSET] = (0xFF & value); \
	}
#define io30_get_pwm g_io_soft_pwm[PWM5 - PWM_PINS_OFFSET]
#else
#define io30_config_pwm(freq)
#define io30_set_pwm(value)
#define io30_get_pwm 0
#endif
#if ASSERT_PIN_IO(PWM6)
#define io31_config_pwm(freq) mcu_config_pwm(PWM6, freq)
#define io31_set_pwm(value) mcu_set_pwm(PWM6, value)
#define io31_get_pwm mcu_get_pwm(PWM6)
#elif ASSERT_PIN_EXTENDED(PWM6)
#define io31_config_pwm(freq)                       \
	{                                                 \
		g_soft_pwm_res = mcu_softpwm_freq_config(freq); \
	}
#define io31_set_pwm(value)                                 \
	{                                                         \
		g_io_soft_pwm[PWM6 - PWM_PINS_OFFSET] = (0xFF & value); \
	}
#define io31_get_pwm g_io_soft_pwm[PWM6 - PWM_PINS_OFFSET]
#else
#define io31_config_pwm(freq)
#define io31_set_pwm(value)
#define io31_get_pwm 0
#endif
#if ASSERT_PIN_IO(PWM7)
#define io32_config_pwm(freq) mcu_config_pwm(PWM7, freq)
#define io32_set_pwm(value) mcu_set_pwm(PWM7, value)
#define io32_get_pwm mcu_get_pwm(PWM7)
#elif ASSERT_PIN_EXTENDED(PWM7)
#define io32_config_pwm(freq)                       \
	{                                                 \
		g_soft_pwm_res = mcu_softpwm_freq_config(freq); \
	}
#define io32_set_pwm(value)                                 \
	{                                                         \
		g_io_soft_pwm[PWM7 - PWM_PINS_OFFSET] = (0xFF & value); \
	}
#define io32_get_pwm g_io_soft_pwm[PWM7 - PWM_PINS_OFFSET]
#else
#define io32_config_pwm(freq)
#define io32_set_pwm(value)
#define io32_get_pwm 0
#endif
#if ASSERT_PIN_IO(PWM8)
#define io33_config_pwm(freq) mcu_config_pwm(PWM8, freq)
#define io33_set_pwm(value) mcu_set_pwm(PWM8, value)
#define io33_get_pwm mcu_get_pwm(PWM8)
#elif ASSERT_PIN_EXTENDED(PWM8)
#define io33_config_pwm(freq)                       \
	{                                                 \
		g_soft_pwm_res = mcu_softpwm_freq_config(freq); \
	}
#define io33_set_pwm(value)                                 \
	{                                                         \
		g_io_soft_pwm[PWM8 - PWM_PINS_OFFSET] = (0xFF & value); \
	}
#define io33_get_pwm g_io_soft_pwm[PWM8 - PWM_PINS_OFFSET]
#else
#define io33_config_pwm(freq)
#define io33_set_pwm(value)
#define io33_get_pwm 0
#endif
#if ASSERT_PIN_IO(PWM9)
#define io34_config_pwm(freq) mcu_config_pwm(PWM9, freq)
#define io34_set_pwm(value) mcu_set_pwm(PWM9, value)
#define io34_get_pwm mcu_get_pwm(PWM9)
#elif ASSERT_PIN_EXTENDED(PWM9)
#define io34_config_pwm(freq)                       \
	{                                                 \
		g_soft_pwm_res = mcu_softpwm_freq_config(freq); \
	}
#define io34_set_pwm(value)                                 \
	{                                                         \
		g_io_soft_pwm[PWM9 - PWM_PINS_OFFSET] = (0xFF & value); \
	}
#define io34_get_pwm g_io_soft_pwm[PWM9 - PWM_PINS_OFFSET]
#else
#define io34_config_pwm(freq)
#define io34_set_pwm(value)
#define io34_get_pwm 0
#endif
#if ASSERT_PIN_IO(PWM10)
#define io35_config_pwm(freq) mcu_config_pwm(PWM10, freq)
#define io35_set_pwm(value) mcu_set_pwm(PWM10, value)
#define io35_get_pwm mcu_get_pwm(PWM10)
#elif ASSERT_PIN_EXTENDED(PWM10)
#define io35_config_pwm(freq)                       \
	{                                                 \
		g_soft_pwm_res = mcu_softpwm_freq_config(freq); \
	}
#define io35_set_pwm(value)                                  \
	{                                                          \
		g_io_soft_pwm[PWM10 - PWM_PINS_OFFSET] = (0xFF & value); \
	}
#define io35_get_pwm g_io_soft_pwm[PWM10 - PWM_PINS_OFFSET]
#else
#define io35_config_pwm(freq)
#define io35_set_pwm(value)
#define io35_get_pwm 0
#endif
#if ASSERT_PIN_IO(PWM11)
#define io36_config_pwm(freq) mcu_config_pwm(PWM11, freq)
#define io36_set_pwm(value) mcu_set_pwm(PWM11, value)
#define io36_get_pwm mcu_get_pwm(PWM11)
#elif ASSERT_PIN_EXTENDED(PWM11)
#define io36_config_pwm(freq)                       \
	{                                                 \
		g_soft_pwm_res = mcu_softpwm_freq_config(freq); \
	}
#define io36_set_pwm(value)                                  \
	{                                                          \
		g_io_soft_pwm[PWM11 - PWM_PINS_OFFSET] = (0xFF & value); \
	}
#define io36_get_pwm g_io_soft_pwm[PWM11 - PWM_PINS_OFFSET]
#else
#define io36_config_pwm(freq)
#define io36_set_pwm(value)
#define io36_get_pwm 0
#endif
#if ASSERT_PIN_IO(PWM12)
#define io37_config_pwm(freq) mcu_config_pwm(PWM12, freq)
#define io37_set_pwm(value) mcu_set_pwm(PWM12, value)
#define io37_get_pwm mcu_get_pwm(PWM12)
#elif ASSERT_PIN_EXTENDED(PWM12)
#define io37_config_pwm(freq)                       \
	{                                                 \
		g_soft_pwm_res = mcu_softpwm_freq_config(freq); \
	}
#define io37_set_pwm(value)                                  \
	{                                                          \
		g_io_soft_pwm[PWM12 - PWM_PINS_OFFSET] = (0xFF & value); \
	}
#define io37_get_pwm g_io_soft_pwm[PWM12 - PWM_PINS_OFFSET]
#else
#define io37_config_pwm(freq)
#define io37_set_pwm(value)
#define io37_get_pwm 0
#endif
#if ASSERT_PIN_IO(PWM13)
#define io38_config_pwm(freq) mcu_config_pwm(PWM13, freq)
#define io38_set_pwm(value) mcu_set_pwm(PWM13, value)
#define io38_get_pwm mcu_get_pwm(PWM13)
#elif ASSERT_PIN_EXTENDED(PWM13)
#define io38_config_pwm(freq)                       \
	{                                                 \
		g_soft_pwm_res = mcu_softpwm_freq_config(freq); \
	}
#define io38_set_pwm(value)                                  \
	{                                                          \
		g_io_soft_pwm[PWM13 - PWM_PINS_OFFSET] = (0xFF & value); \
	}
#define io38_get_pwm g_io_soft_pwm[PWM13 - PWM_PINS_OFFSET]
#else
#define io38_config_pwm(freq)
#define io38_set_pwm(value)
#define io38_get_pwm 0
#endif
#if ASSERT_PIN_IO(PWM14)
#define io39_config_pwm(freq) mcu_config_pwm(PWM14, freq)
#define io39_set_pwm(value) mcu_set_pwm(PWM14, value)
#define io39_get_pwm mcu_get_pwm(PWM14)
#elif ASSERT_PIN_EXTENDED(PWM14)
#define io39_config_pwm(freq)                       \
	{                                                 \
		g_soft_pwm_res = mcu_softpwm_freq_config(freq); \
	}
#define io39_set_pwm(value)                                  \
	{                                                          \
		g_io_soft_pwm[PWM14 - PWM_PINS_OFFSET] = (0xFF & value); \
	}
#define io39_get_pwm g_io_soft_pwm[PWM14 - PWM_PINS_OFFSET]
#else
#define io39_config_pwm(freq)
#define io39_set_pwm(value)
#define io39_get_pwm 0
#endif
#if ASSERT_PIN_IO(PWM15)
#define io40_config_pwm(freq) mcu_config_pwm(PWM15, freq)
#define io40_set_pwm(value) mcu_set_pwm(PWM15, value)
#define io40_get_pwm mcu_get_pwm(PWM15)
#elif ASSERT_PIN_EXTENDED(PWM15)
#define io40_config_pwm(freq)                       \
	{                                                 \
		g_soft_pwm_res = mcu_softpwm_freq_config(freq); \
	}
#define io40_set_pwm(value)                                  \
	{                                                          \
		g_io_soft_pwm[PWM15 - PWM_PINS_OFFSET] = (0xFF & value); \
	}
#define io40_get_pwm g_io_soft_pwm[PWM15 - PWM_PINS_OFFSET]
#else
#define io40_config_pwm(freq)
#define io40_set_pwm(value)
#define io40_get_pwm 0
#endif

/*SERVO*/
#if ASSERT_PIN_IO(SERVO0)
#define io41_set_pwm(value) mcu_set_servo(SERVO0, value)
#define io41_get_pwm mcu_get_servo(SERVO0)
#elif ASSERT_PIN_EXTENDED(SERVO0)
#define io41_set_pwm(value)
#define io41_get_pwm 0
#else
#define io41_set_pwm(value)
#define io41_get_pwm 0
#endif
#if ASSERT_PIN_IO(SERVO1)
#define io42_set_pwm(value) mcu_set_servo(SERVO1, value)
#define io42_get_pwm mcu_get_servo(SERVO1)
#elif ASSERT_PIN_EXTENDED(SERVO1)
#define io42_set_pwm(value)
#define io42_get_pwm 0
#else
#define io42_set_pwm(value)
#define io42_get_pwm 0
#endif
#if ASSERT_PIN_IO(SERVO2)
#define io43_set_pwm(value) mcu_set_servo(SERVO2, value)
#define io43_get_pwm mcu_get_servo(SERVO2)
#elif ASSERT_PIN_EXTENDED(SERVO2)
#define io43_set_pwm(value)
#define io43_get_pwm 0
#else
#define io43_set_pwm(value)
#define io43_get_pwm 0
#endif
#if ASSERT_PIN_IO(SERVO3)
#define io44_set_pwm(value) mcu_set_servo(SERVO3, value)
#define io44_get_pwm mcu_get_servo(SERVO3)
#elif ASSERT_PIN_EXTENDED(SERVO3)
#define io44_set_pwm(value)
#define io44_get_pwm 0
#else
#define io44_set_pwm(value)
#define io44_get_pwm 0
#endif
#if ASSERT_PIN_IO(SERVO4)
#define io45_set_pwm(value) mcu_set_servo(SERVO4, value)
#define io45_get_pwm mcu_get_servo(SERVO4)
#elif ASSERT_PIN_EXTENDED(SERVO4)
#define io45_set_pwm(value)
#define io45_get_pwm 0
#else
#define io45_set_pwm(value)
#define io45_get_pwm 0
#endif
#if ASSERT_PIN_IO(SERVO5)
#define io46_set_pwm(value) mcu_set_servo(SERVO5, value)
#define io46_get_pwm mcu_get_servo(SERVO5)
#elif ASSERT_PIN_EXTENDED(SERVO5)
#define io46_set_pwm(value)
#define io46_get_pwm 0
#else
#define io46_set_pwm(value)
#define io46_get_pwm 0
#endif

/*ANALOG*/
#if ASSERT_PIN_IO(ANALOG0)
#define io114_config_analog mcu_config_analog(ANALOG0)
#define io114_get_analog mcu_get_analog(ANALOG0)
#elif ASSERT_PIN_EXTENDED(ANALOG0)
#define io114_config_analog
#define io114_get_analog 0
#else
#define io114_config_analog
#define io114_get_analog 0
#endif
#if ASSERT_PIN_IO(ANALOG1)
#define io115_config_analog mcu_config_analog(ANALOG1)
#define io115_get_analog mcu_get_analog(ANALOG1)
#elif ASSERT_PIN_EXTENDED(ANALOG1)
#define io115_config_analog
#define io115_get_analog 0
#else
#define io115_config_analog
#define io115_get_analog 0
#endif
#if ASSERT_PIN_IO(ANALOG2)
#define io116_config_analog mcu_config_analog(ANALOG2)
#define io116_get_analog mcu_get_analog(ANALOG2)
#elif ASSERT_PIN_EXTENDED(ANALOG2)
#define io116_config_analog
#define io116_get_analog 0
#else
#define io116_config_analog
#define io116_get_analog 0
#endif
#if ASSERT_PIN_IO(ANALOG3)
#define io117_config_analog mcu_config_analog(ANALOG3)
#define io117_get_analog mcu_get_analog(ANALOG3)
#elif ASSERT_PIN_EXTENDED(ANALOG3)
#define io117_config_analog
#define io117_get_analog 0
#else
#define io117_config_analog
#define io117_get_analog 0
#endif
#if ASSERT_PIN_IO(ANALOG4)
#define io118_config_analog mcu_config_analog(ANALOG4)
#define io118_get_analog mcu_get_analog(ANALOG4)
#elif ASSERT_PIN_EXTENDED(ANALOG4)
#define io118_config_analog
#define io118_get_analog 0
#else
#define io118_config_analog
#define io118_get_analog 0
#endif
#if ASSERT_PIN_IO(ANALOG5)
#define io119_config_analog mcu_config_analog(ANALOG5)
#define io119_get_analog mcu_get_analog(ANALOG5)
#elif ASSERT_PIN_EXTENDED(ANALOG5)
#define io119_config_analog
#define io119_get_analog 0
#else
#define io119_config_analog
#define io119_get_analog 0
#endif
#if ASSERT_PIN_IO(ANALOG6)
#define io120_config_analog mcu_config_analog(ANALOG6)
#define io120_get_analog mcu_get_analog(ANALOG6)
#elif ASSERT_PIN_EXTENDED(ANALOG6)
#define io120_config_analog
#define io120_get_analog 0
#else
#define io120_config_analog
#define io120_get_analog 0
#endif
#if ASSERT_PIN_IO(ANALOG7)
#define io121_config_analog mcu_config_analog(ANALOG7)
#define io121_get_analog mcu_get_analog(ANALOG7)
#elif ASSERT_PIN_EXTENDED(ANALOG7)
#define io121_config_analog
#define io121_get_analog 0
#else
#define io121_config_analog
#define io121_get_analog 0
#endif
#if ASSERT_PIN_IO(ANALOG8)
#define io122_config_analog mcu_config_analog(ANALOG8)
#define io122_get_analog mcu_get_analog(ANALOG8)
#elif ASSERT_PIN_EXTENDED(ANALOG8)
#define io122_config_analog
#define io122_get_analog 0
#else
#define io122_config_analog
#define io122_get_analog 0
#endif
#if ASSERT_PIN_IO(ANALOG9)
#define io123_config_analog mcu_config_analog(ANALOG9)
#define io123_get_analog mcu_get_analog(ANALOG9)
#elif ASSERT_PIN_EXTENDED(ANALOG9)
#define io123_config_analog
#define io123_get_analog 0
#else
#define io123_config_analog
#define io123_get_analog 0
#endif
#if ASSERT_PIN_IO(ANALOG10)
#define io124_config_analog mcu_config_analog(ANALOG10)
#define io124_get_analog mcu_get_analog(ANALOG10)
#elif ASSERT_PIN_EXTENDED(ANALOG10)
#define io124_config_analog
#define io124_get_analog 0
#else
#define io124_config_analog
#define io124_get_analog 0
#endif
#if ASSERT_PIN_IO(ANALOG11)
#define io125_config_analog mcu_config_analog(ANALOG11)
#define io125_get_analog mcu_get_analog(ANALOG11)
#elif ASSERT_PIN_EXTENDED(ANALOG11)
#define io125_config_analog
#define io125_get_analog 0
#else
#define io125_config_analog
#define io125_get_analog 0
#endif
#if ASSERT_PIN_IO(ANALOG12)
#define io126_config_analog mcu_config_analog(ANALOG12)
#define io126_get_analog mcu_get_analog(ANALOG12)
#elif ASSERT_PIN_EXTENDED(ANALOG12)
#define io126_config_analog
#define io126_get_analog 0
#else
#define io126_config_analog
#define io126_get_analog 0
#endif
#if ASSERT_PIN_IO(ANALOG13)
#define io127_config_analog mcu_config_analog(ANALOG13)
#define io127_get_analog mcu_get_analog(ANALOG13)
#elif ASSERT_PIN_EXTENDED(ANALOG13)
#define io127_config_analog
#define io127_get_analog 0
#else
#define io127_config_analog
#define io127_get_analog 0
#endif
#if ASSERT_PIN_IO(ANALOG14)
#define io128_config_analog mcu_config_analog(ANALOG14)
#define io128_get_analog mcu_get_analog(ANALOG14)
#elif ASSERT_PIN_EXTENDED(ANALOG14)
#define io128_config_analog
#define io128_get_analog 0
#else
#define io128_config_analog
#define io128_get_analog 0
#endif
#if ASSERT_PIN_IO(ANALOG15)
#define io129_config_analog mcu_config_analog(ANALOG15)
#define io129_get_analog mcu_get_analog(ANALOG15)
#elif ASSERT_PIN_EXTENDED(ANALOG15)
#define io129_config_analog
#define io129_get_analog 0
#else
#define io129_config_analog
#define io129_get_analog 0
#endif

/*output HAL*/
#define _io_hal_config_output_(pin) io##pin##_config_output
#define _io_hal_set_output_(pin) io##pin##_set_output
#define _io_hal_clear_output_(pin) io##pin##_clear_output
#define _io_hal_toggle_output_(pin) io##pin##_toggle_output
#define _io_hal_get_output_(pin) io##pin##_get_output

#ifndef FORCE_HAL_IO_DIRECTION_ONREQUEST
#define io_hal_config_output(pin) _io_hal_config_output_(pin)
#define io_hal_set_output(pin) _io_hal_set_output_(pin)
#define io_hal_clear_output(pin) _io_hal_clear_output_(pin)
#define io_hal_toggle_output(pin) _io_hal_toggle_output_(pin)
#define io_hal_get_output(pin) _io_hal_get_output_(pin)
#else
#define io_hal_config_output(pin) _io_hal_config_output_(pin)
#define io_hal_set_output(pin) \
	_io_hal_config_output_(pin); \
	_io_hal_set_output_(pin)
#define io_hal_clear_output(pin) \
	_io_hal_config_output_(pin);   \
	_io_hal_clear_output_(pin)
#define io_hal_toggle_output(pin) \
	_io_hal_config_output_(pin);    \
	_io_hal_toggle_output_(pin)
#define io_hal_get_output(pin) \
	({_io_hal_config_output_(pin); \
	_io_hal_get_output_(pin); })
#endif

/*input HAL*/
#define _io_hal_config_input_(pin) io##pin##_config_input
#define io_hal_config_input(pin) _io_hal_config_input_(pin)
#define _io_hal_config_pullup_(pin) io##pin##_config_pullup
#define io_hal_config_pullup(pin) _io_hal_config_pullup_(pin)
#define _io_hal_get_input_(pin) io##pin##_get_input

#ifndef FORCE_HAL_IO_DIRECTION_ONREQUEST
#define io_hal_get_input(pin) _io_hal_get_input_(pin)
#else
#define io_hal_get_input(pin) ({_io_hal_config_input_(pin);_io_hal_get_input_(pin); })
#endif

/*pwm and servo HAL*/
#define _io_hal_config_pwm_(pin, freq) io##pin##_config_pwm(freq)
#define io_hal_config_pwm(pin, freq) _io_hal_config_pwm_(pin, freq)
#define _io_hal_set_pwm_(pin, value) io##pin##_set_pwm(value)
#define io_hal_set_pwm(pin, value) _io_hal_set_pwm_(pin, value)
#define _io_hal_get_pwm_(pin) io##pin##_get_pwm
#define io_hal_get_pwm(pin) _io_hal_get_pwm_(pin)

/*analog HAL*/
#define _io_hal_config_analog_(pin) io##pin##_config_analog
#define io_hal_config_analog(pin) _io_hal_config_analog_(pin)
#define _io_hal_get_analog_(pin) io##pin##_get_analog
#define io_hal_get_analog(pin) _io_hal_get_analog_(pin)

#ifdef __cplusplus
}
#endif

#endif
