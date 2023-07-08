
#ifndef IO_HAL_H
#define IO_HAL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*IO HAL*/
#if ASSERT_PIN_IO(1)
#define io1_config_output mcu_config_output(1)
#define io1_set_output mcu_set_output(1)
#define io1_clear_output mcu_clear_output(1)
#define io1_toggle_output mcu_toggle_output(1)
#define io1_get_output mcu_get_output(1)
#define io1_config_input mcu_config_input(1)
#define io1_config_pullup mcu_config_pullup(1)
#define io1_get_input mcu_get_input(1)
#elif ASSERT_PIN_EXTENDED(1)
#define io1_config_output
#define io1_set_output ic74hc595_set_pin(1);ic74hc595_shift_io_pins()
#define io1_clear_output ic74hc595_clear_pin(1);ic74hc595_shift_io_pins()
#define io1_toggle_output ic74hc595_toggle_pin(1);ic74hc595_shift_io_pins()
#define io1_get_output ic74hc595_get_pin(1)
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
#if ASSERT_PIN_IO(2)
#define io2_config_output mcu_config_output(2)
#define io2_set_output mcu_set_output(2)
#define io2_clear_output mcu_clear_output(2)
#define io2_toggle_output mcu_toggle_output(2)
#define io2_get_output mcu_get_output(2)
#define io2_config_input mcu_config_input(2)
#define io2_config_pullup mcu_config_pullup(2)
#define io2_get_input mcu_get_input(2)
#elif ASSERT_PIN_EXTENDED(2)
#define io2_config_output
#define io2_set_output ic74hc595_set_pin(2);ic74hc595_shift_io_pins()
#define io2_clear_output ic74hc595_clear_pin(2);ic74hc595_shift_io_pins()
#define io2_toggle_output ic74hc595_toggle_pin(2);ic74hc595_shift_io_pins()
#define io2_get_output ic74hc595_get_pin(2)
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
#if ASSERT_PIN_IO(3)
#define io3_config_output mcu_config_output(3)
#define io3_set_output mcu_set_output(3)
#define io3_clear_output mcu_clear_output(3)
#define io3_toggle_output mcu_toggle_output(3)
#define io3_get_output mcu_get_output(3)
#define io3_config_input mcu_config_input(3)
#define io3_config_pullup mcu_config_pullup(3)
#define io3_get_input mcu_get_input(3)
#elif ASSERT_PIN_EXTENDED(3)
#define io3_config_output
#define io3_set_output ic74hc595_set_pin(3);ic74hc595_shift_io_pins()
#define io3_clear_output ic74hc595_clear_pin(3);ic74hc595_shift_io_pins()
#define io3_toggle_output ic74hc595_toggle_pin(3);ic74hc595_shift_io_pins()
#define io3_get_output ic74hc595_get_pin(3)
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
#if ASSERT_PIN_IO(4)
#define io4_config_output mcu_config_output(4)
#define io4_set_output mcu_set_output(4)
#define io4_clear_output mcu_clear_output(4)
#define io4_toggle_output mcu_toggle_output(4)
#define io4_get_output mcu_get_output(4)
#define io4_config_input mcu_config_input(4)
#define io4_config_pullup mcu_config_pullup(4)
#define io4_get_input mcu_get_input(4)
#elif ASSERT_PIN_EXTENDED(4)
#define io4_config_output
#define io4_set_output ic74hc595_set_pin(4);ic74hc595_shift_io_pins()
#define io4_clear_output ic74hc595_clear_pin(4);ic74hc595_shift_io_pins()
#define io4_toggle_output ic74hc595_toggle_pin(4);ic74hc595_shift_io_pins()
#define io4_get_output ic74hc595_get_pin(4)
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
#if ASSERT_PIN_IO(5)
#define io5_config_output mcu_config_output(5)
#define io5_set_output mcu_set_output(5)
#define io5_clear_output mcu_clear_output(5)
#define io5_toggle_output mcu_toggle_output(5)
#define io5_get_output mcu_get_output(5)
#define io5_config_input mcu_config_input(5)
#define io5_config_pullup mcu_config_pullup(5)
#define io5_get_input mcu_get_input(5)
#elif ASSERT_PIN_EXTENDED(5)
#define io5_config_output
#define io5_set_output ic74hc595_set_pin(5);ic74hc595_shift_io_pins()
#define io5_clear_output ic74hc595_clear_pin(5);ic74hc595_shift_io_pins()
#define io5_toggle_output ic74hc595_toggle_pin(5);ic74hc595_shift_io_pins()
#define io5_get_output ic74hc595_get_pin(5)
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
#if ASSERT_PIN_IO(6)
#define io6_config_output mcu_config_output(6)
#define io6_set_output mcu_set_output(6)
#define io6_clear_output mcu_clear_output(6)
#define io6_toggle_output mcu_toggle_output(6)
#define io6_get_output mcu_get_output(6)
#define io6_config_input mcu_config_input(6)
#define io6_config_pullup mcu_config_pullup(6)
#define io6_get_input mcu_get_input(6)
#elif ASSERT_PIN_EXTENDED(6)
#define io6_config_output
#define io6_set_output ic74hc595_set_pin(6);ic74hc595_shift_io_pins()
#define io6_clear_output ic74hc595_clear_pin(6);ic74hc595_shift_io_pins()
#define io6_toggle_output ic74hc595_toggle_pin(6);ic74hc595_shift_io_pins()
#define io6_get_output ic74hc595_get_pin(6)
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
#if ASSERT_PIN_IO(7)
#define io7_config_output mcu_config_output(7)
#define io7_set_output mcu_set_output(7)
#define io7_clear_output mcu_clear_output(7)
#define io7_toggle_output mcu_toggle_output(7)
#define io7_get_output mcu_get_output(7)
#define io7_config_input mcu_config_input(7)
#define io7_config_pullup mcu_config_pullup(7)
#define io7_get_input mcu_get_input(7)
#elif ASSERT_PIN_EXTENDED(7)
#define io7_config_output
#define io7_set_output ic74hc595_set_pin(7);ic74hc595_shift_io_pins()
#define io7_clear_output ic74hc595_clear_pin(7);ic74hc595_shift_io_pins()
#define io7_toggle_output ic74hc595_toggle_pin(7);ic74hc595_shift_io_pins()
#define io7_get_output ic74hc595_get_pin(7)
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
#if ASSERT_PIN_IO(8)
#define io8_config_output mcu_config_output(8)
#define io8_set_output mcu_set_output(8)
#define io8_clear_output mcu_clear_output(8)
#define io8_toggle_output mcu_toggle_output(8)
#define io8_get_output mcu_get_output(8)
#define io8_config_input mcu_config_input(8)
#define io8_config_pullup mcu_config_pullup(8)
#define io8_get_input mcu_get_input(8)
#elif ASSERT_PIN_EXTENDED(8)
#define io8_config_output
#define io8_set_output ic74hc595_set_pin(8);ic74hc595_shift_io_pins()
#define io8_clear_output ic74hc595_clear_pin(8);ic74hc595_shift_io_pins()
#define io8_toggle_output ic74hc595_toggle_pin(8);ic74hc595_shift_io_pins()
#define io8_get_output ic74hc595_get_pin(8)
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
#if ASSERT_PIN_IO(9)
#define io9_config_output mcu_config_output(9)
#define io9_set_output mcu_set_output(9)
#define io9_clear_output mcu_clear_output(9)
#define io9_toggle_output mcu_toggle_output(9)
#define io9_get_output mcu_get_output(9)
#define io9_config_input mcu_config_input(9)
#define io9_config_pullup mcu_config_pullup(9)
#define io9_get_input mcu_get_input(9)
#elif ASSERT_PIN_EXTENDED(9)
#define io9_config_output
#define io9_set_output ic74hc595_set_pin(9);ic74hc595_shift_io_pins()
#define io9_clear_output ic74hc595_clear_pin(9);ic74hc595_shift_io_pins()
#define io9_toggle_output ic74hc595_toggle_pin(9);ic74hc595_shift_io_pins()
#define io9_get_output ic74hc595_get_pin(9)
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
#if ASSERT_PIN_IO(10)
#define io10_config_output mcu_config_output(10)
#define io10_set_output mcu_set_output(10)
#define io10_clear_output mcu_clear_output(10)
#define io10_toggle_output mcu_toggle_output(10)
#define io10_get_output mcu_get_output(10)
#define io10_config_input mcu_config_input(10)
#define io10_config_pullup mcu_config_pullup(10)
#define io10_get_input mcu_get_input(10)
#elif ASSERT_PIN_EXTENDED(10)
#define io10_config_output
#define io10_set_output ic74hc595_set_pin(10);ic74hc595_shift_io_pins()
#define io10_clear_output ic74hc595_clear_pin(10);ic74hc595_shift_io_pins()
#define io10_toggle_output ic74hc595_toggle_pin(10);ic74hc595_shift_io_pins()
#define io10_get_output ic74hc595_get_pin(10)
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
#if ASSERT_PIN_IO(11)
#define io11_config_output mcu_config_output(11)
#define io11_set_output mcu_set_output(11)
#define io11_clear_output mcu_clear_output(11)
#define io11_toggle_output mcu_toggle_output(11)
#define io11_get_output mcu_get_output(11)
#define io11_config_input mcu_config_input(11)
#define io11_config_pullup mcu_config_pullup(11)
#define io11_get_input mcu_get_input(11)
#elif ASSERT_PIN_EXTENDED(11)
#define io11_config_output
#define io11_set_output ic74hc595_set_pin(11);ic74hc595_shift_io_pins()
#define io11_clear_output ic74hc595_clear_pin(11);ic74hc595_shift_io_pins()
#define io11_toggle_output ic74hc595_toggle_pin(11);ic74hc595_shift_io_pins()
#define io11_get_output ic74hc595_get_pin(11)
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
#if ASSERT_PIN_IO(12)
#define io12_config_output mcu_config_output(12)
#define io12_set_output mcu_set_output(12)
#define io12_clear_output mcu_clear_output(12)
#define io12_toggle_output mcu_toggle_output(12)
#define io12_get_output mcu_get_output(12)
#define io12_config_input mcu_config_input(12)
#define io12_config_pullup mcu_config_pullup(12)
#define io12_get_input mcu_get_input(12)
#elif ASSERT_PIN_EXTENDED(12)
#define io12_config_output
#define io12_set_output ic74hc595_set_pin(12);ic74hc595_shift_io_pins()
#define io12_clear_output ic74hc595_clear_pin(12);ic74hc595_shift_io_pins()
#define io12_toggle_output ic74hc595_toggle_pin(12);ic74hc595_shift_io_pins()
#define io12_get_output ic74hc595_get_pin(12)
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
#if ASSERT_PIN_IO(13)
#define io13_config_output mcu_config_output(13)
#define io13_set_output mcu_set_output(13)
#define io13_clear_output mcu_clear_output(13)
#define io13_toggle_output mcu_toggle_output(13)
#define io13_get_output mcu_get_output(13)
#define io13_config_input mcu_config_input(13)
#define io13_config_pullup mcu_config_pullup(13)
#define io13_get_input mcu_get_input(13)
#elif ASSERT_PIN_EXTENDED(13)
#define io13_config_output
#define io13_set_output ic74hc595_set_pin(13);ic74hc595_shift_io_pins()
#define io13_clear_output ic74hc595_clear_pin(13);ic74hc595_shift_io_pins()
#define io13_toggle_output ic74hc595_toggle_pin(13);ic74hc595_shift_io_pins()
#define io13_get_output ic74hc595_get_pin(13)
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
#if ASSERT_PIN_IO(14)
#define io14_config_output mcu_config_output(14)
#define io14_set_output mcu_set_output(14)
#define io14_clear_output mcu_clear_output(14)
#define io14_toggle_output mcu_toggle_output(14)
#define io14_get_output mcu_get_output(14)
#define io14_config_input mcu_config_input(14)
#define io14_config_pullup mcu_config_pullup(14)
#define io14_get_input mcu_get_input(14)
#elif ASSERT_PIN_EXTENDED(14)
#define io14_config_output
#define io14_set_output ic74hc595_set_pin(14);ic74hc595_shift_io_pins()
#define io14_clear_output ic74hc595_clear_pin(14);ic74hc595_shift_io_pins()
#define io14_toggle_output ic74hc595_toggle_pin(14);ic74hc595_shift_io_pins()
#define io14_get_output ic74hc595_get_pin(14)
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
#if ASSERT_PIN_IO(15)
#define io15_config_output mcu_config_output(15)
#define io15_set_output mcu_set_output(15)
#define io15_clear_output mcu_clear_output(15)
#define io15_toggle_output mcu_toggle_output(15)
#define io15_get_output mcu_get_output(15)
#define io15_config_input mcu_config_input(15)
#define io15_config_pullup mcu_config_pullup(15)
#define io15_get_input mcu_get_input(15)
#elif ASSERT_PIN_EXTENDED(15)
#define io15_config_output
#define io15_set_output ic74hc595_set_pin(15);ic74hc595_shift_io_pins()
#define io15_clear_output ic74hc595_clear_pin(15);ic74hc595_shift_io_pins()
#define io15_toggle_output ic74hc595_toggle_pin(15);ic74hc595_shift_io_pins()
#define io15_get_output ic74hc595_get_pin(15)
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
#if ASSERT_PIN_IO(16)
#define io16_config_output mcu_config_output(16)
#define io16_set_output mcu_set_output(16)
#define io16_clear_output mcu_clear_output(16)
#define io16_toggle_output mcu_toggle_output(16)
#define io16_get_output mcu_get_output(16)
#define io16_config_input mcu_config_input(16)
#define io16_config_pullup mcu_config_pullup(16)
#define io16_get_input mcu_get_input(16)
#elif ASSERT_PIN_EXTENDED(16)
#define io16_config_output
#define io16_set_output ic74hc595_set_pin(16);ic74hc595_shift_io_pins()
#define io16_clear_output ic74hc595_clear_pin(16);ic74hc595_shift_io_pins()
#define io16_toggle_output ic74hc595_toggle_pin(16);ic74hc595_shift_io_pins()
#define io16_get_output ic74hc595_get_pin(16)
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
#if ASSERT_PIN_IO(17)
#define io17_config_output mcu_config_output(17)
#define io17_set_output mcu_set_output(17)
#define io17_clear_output mcu_clear_output(17)
#define io17_toggle_output mcu_toggle_output(17)
#define io17_get_output mcu_get_output(17)
#define io17_config_input mcu_config_input(17)
#define io17_config_pullup mcu_config_pullup(17)
#define io17_get_input mcu_get_input(17)
#elif ASSERT_PIN_EXTENDED(17)
#define io17_config_output
#define io17_set_output ic74hc595_set_pin(17);ic74hc595_shift_io_pins()
#define io17_clear_output ic74hc595_clear_pin(17);ic74hc595_shift_io_pins()
#define io17_toggle_output ic74hc595_toggle_pin(17);ic74hc595_shift_io_pins()
#define io17_get_output ic74hc595_get_pin(17)
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
#if ASSERT_PIN_IO(18)
#define io18_config_output mcu_config_output(18)
#define io18_set_output mcu_set_output(18)
#define io18_clear_output mcu_clear_output(18)
#define io18_toggle_output mcu_toggle_output(18)
#define io18_get_output mcu_get_output(18)
#define io18_config_input mcu_config_input(18)
#define io18_config_pullup mcu_config_pullup(18)
#define io18_get_input mcu_get_input(18)
#elif ASSERT_PIN_EXTENDED(18)
#define io18_config_output
#define io18_set_output ic74hc595_set_pin(18);ic74hc595_shift_io_pins()
#define io18_clear_output ic74hc595_clear_pin(18);ic74hc595_shift_io_pins()
#define io18_toggle_output ic74hc595_toggle_pin(18);ic74hc595_shift_io_pins()
#define io18_get_output ic74hc595_get_pin(18)
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
#if ASSERT_PIN_IO(19)
#define io19_config_output mcu_config_output(19)
#define io19_set_output mcu_set_output(19)
#define io19_clear_output mcu_clear_output(19)
#define io19_toggle_output mcu_toggle_output(19)
#define io19_get_output mcu_get_output(19)
#define io19_config_input mcu_config_input(19)
#define io19_config_pullup mcu_config_pullup(19)
#define io19_get_input mcu_get_input(19)
#elif ASSERT_PIN_EXTENDED(19)
#define io19_config_output
#define io19_set_output ic74hc595_set_pin(19);ic74hc595_shift_io_pins()
#define io19_clear_output ic74hc595_clear_pin(19);ic74hc595_shift_io_pins()
#define io19_toggle_output ic74hc595_toggle_pin(19);ic74hc595_shift_io_pins()
#define io19_get_output ic74hc595_get_pin(19)
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
#if ASSERT_PIN_IO(20)
#define io20_config_output mcu_config_output(20)
#define io20_set_output mcu_set_output(20)
#define io20_clear_output mcu_clear_output(20)
#define io20_toggle_output mcu_toggle_output(20)
#define io20_get_output mcu_get_output(20)
#define io20_config_input mcu_config_input(20)
#define io20_config_pullup mcu_config_pullup(20)
#define io20_get_input mcu_get_input(20)
#elif ASSERT_PIN_EXTENDED(20)
#define io20_config_output
#define io20_set_output ic74hc595_set_pin(20);ic74hc595_shift_io_pins()
#define io20_clear_output ic74hc595_clear_pin(20);ic74hc595_shift_io_pins()
#define io20_toggle_output ic74hc595_toggle_pin(20);ic74hc595_shift_io_pins()
#define io20_get_output ic74hc595_get_pin(20)
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
#if ASSERT_PIN_IO(21)
#define io21_config_output mcu_config_output(21)
#define io21_set_output mcu_set_output(21)
#define io21_clear_output mcu_clear_output(21)
#define io21_toggle_output mcu_toggle_output(21)
#define io21_get_output mcu_get_output(21)
#define io21_config_input mcu_config_input(21)
#define io21_config_pullup mcu_config_pullup(21)
#define io21_get_input mcu_get_input(21)
#elif ASSERT_PIN_EXTENDED(21)
#define io21_config_output
#define io21_set_output ic74hc595_set_pin(21);ic74hc595_shift_io_pins()
#define io21_clear_output ic74hc595_clear_pin(21);ic74hc595_shift_io_pins()
#define io21_toggle_output ic74hc595_toggle_pin(21);ic74hc595_shift_io_pins()
#define io21_get_output ic74hc595_get_pin(21)
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
#if ASSERT_PIN_IO(22)
#define io22_config_output mcu_config_output(22)
#define io22_set_output mcu_set_output(22)
#define io22_clear_output mcu_clear_output(22)
#define io22_toggle_output mcu_toggle_output(22)
#define io22_get_output mcu_get_output(22)
#define io22_config_input mcu_config_input(22)
#define io22_config_pullup mcu_config_pullup(22)
#define io22_get_input mcu_get_input(22)
#elif ASSERT_PIN_EXTENDED(22)
#define io22_config_output
#define io22_set_output ic74hc595_set_pin(22);ic74hc595_shift_io_pins()
#define io22_clear_output ic74hc595_clear_pin(22);ic74hc595_shift_io_pins()
#define io22_toggle_output ic74hc595_toggle_pin(22);ic74hc595_shift_io_pins()
#define io22_get_output ic74hc595_get_pin(22)
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
#if ASSERT_PIN_IO(23)
#define io23_config_output mcu_config_output(23)
#define io23_set_output mcu_set_output(23)
#define io23_clear_output mcu_clear_output(23)
#define io23_toggle_output mcu_toggle_output(23)
#define io23_get_output mcu_get_output(23)
#define io23_config_input mcu_config_input(23)
#define io23_config_pullup mcu_config_pullup(23)
#define io23_get_input mcu_get_input(23)
#elif ASSERT_PIN_EXTENDED(23)
#define io23_config_output
#define io23_set_output ic74hc595_set_pin(23);ic74hc595_shift_io_pins()
#define io23_clear_output ic74hc595_clear_pin(23);ic74hc595_shift_io_pins()
#define io23_toggle_output ic74hc595_toggle_pin(23);ic74hc595_shift_io_pins()
#define io23_get_output ic74hc595_get_pin(23)
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
#if ASSERT_PIN_IO(24)
#define io24_config_output mcu_config_output(24)
#define io24_set_output mcu_set_output(24)
#define io24_clear_output mcu_clear_output(24)
#define io24_toggle_output mcu_toggle_output(24)
#define io24_get_output mcu_get_output(24)
#define io24_config_input mcu_config_input(24)
#define io24_config_pullup mcu_config_pullup(24)
#define io24_get_input mcu_get_input(24)
#elif ASSERT_PIN_EXTENDED(24)
#define io24_config_output
#define io24_set_output ic74hc595_set_pin(24);ic74hc595_shift_io_pins()
#define io24_clear_output ic74hc595_clear_pin(24);ic74hc595_shift_io_pins()
#define io24_toggle_output ic74hc595_toggle_pin(24);ic74hc595_shift_io_pins()
#define io24_get_output ic74hc595_get_pin(24)
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
#if ASSERT_PIN_IO(25)
#define io25_config_output mcu_config_output(25)
#define io25_set_output mcu_set_output(25)
#define io25_clear_output mcu_clear_output(25)
#define io25_toggle_output mcu_toggle_output(25)
#define io25_get_output mcu_get_output(25)
#define io25_config_input mcu_config_input(25)
#define io25_config_pullup mcu_config_pullup(25)
#define io25_get_input mcu_get_input(25)
#elif ASSERT_PIN_EXTENDED(25)
#define io25_config_output
#define io25_set_output ic74hc595_set_pin(25);ic74hc595_shift_io_pins()
#define io25_clear_output ic74hc595_clear_pin(25);ic74hc595_shift_io_pins()
#define io25_toggle_output ic74hc595_toggle_pin(25);ic74hc595_shift_io_pins()
#define io25_get_output ic74hc595_get_pin(25)
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
#if ASSERT_PIN_IO(26)
#define io26_config_output mcu_config_output(26)
#define io26_set_output mcu_set_output(26)
#define io26_clear_output mcu_clear_output(26)
#define io26_toggle_output mcu_toggle_output(26)
#define io26_get_output mcu_get_output(26)
#define io26_config_input mcu_config_input(26)
#define io26_config_pullup mcu_config_pullup(26)
#define io26_get_input mcu_get_input(26)
#elif ASSERT_PIN_EXTENDED(26)
#define io26_config_output
#define io26_set_output ic74hc595_set_pin(26);ic74hc595_shift_io_pins()
#define io26_clear_output ic74hc595_clear_pin(26);ic74hc595_shift_io_pins()
#define io26_toggle_output ic74hc595_toggle_pin(26);ic74hc595_shift_io_pins()
#define io26_get_output ic74hc595_get_pin(26)
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
#if ASSERT_PIN_IO(27)
#define io27_config_output mcu_config_output(27)
#define io27_set_output mcu_set_output(27)
#define io27_clear_output mcu_clear_output(27)
#define io27_toggle_output mcu_toggle_output(27)
#define io27_get_output mcu_get_output(27)
#define io27_config_input mcu_config_input(27)
#define io27_config_pullup mcu_config_pullup(27)
#define io27_get_input mcu_get_input(27)
#elif ASSERT_PIN_EXTENDED(27)
#define io27_config_output
#define io27_set_output ic74hc595_set_pin(27);ic74hc595_shift_io_pins()
#define io27_clear_output ic74hc595_clear_pin(27);ic74hc595_shift_io_pins()
#define io27_toggle_output ic74hc595_toggle_pin(27);ic74hc595_shift_io_pins()
#define io27_get_output ic74hc595_get_pin(27)
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
#if ASSERT_PIN_IO(28)
#define io28_config_output mcu_config_output(28)
#define io28_set_output mcu_set_output(28)
#define io28_clear_output mcu_clear_output(28)
#define io28_toggle_output mcu_toggle_output(28)
#define io28_get_output mcu_get_output(28)
#define io28_config_input mcu_config_input(28)
#define io28_config_pullup mcu_config_pullup(28)
#define io28_get_input mcu_get_input(28)
#elif ASSERT_PIN_EXTENDED(28)
#define io28_config_output
#define io28_set_output ic74hc595_set_pin(28);ic74hc595_shift_io_pins()
#define io28_clear_output ic74hc595_clear_pin(28);ic74hc595_shift_io_pins()
#define io28_toggle_output ic74hc595_toggle_pin(28);ic74hc595_shift_io_pins()
#define io28_get_output ic74hc595_get_pin(28)
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
#if ASSERT_PIN_IO(29)
#define io29_config_output mcu_config_output(29)
#define io29_set_output mcu_set_output(29)
#define io29_clear_output mcu_clear_output(29)
#define io29_toggle_output mcu_toggle_output(29)
#define io29_get_output mcu_get_output(29)
#define io29_config_input mcu_config_input(29)
#define io29_config_pullup mcu_config_pullup(29)
#define io29_get_input mcu_get_input(29)
#elif ASSERT_PIN_EXTENDED(29)
#define io29_config_output
#define io29_set_output ic74hc595_set_pin(29);ic74hc595_shift_io_pins()
#define io29_clear_output ic74hc595_clear_pin(29);ic74hc595_shift_io_pins()
#define io29_toggle_output ic74hc595_toggle_pin(29);ic74hc595_shift_io_pins()
#define io29_get_output ic74hc595_get_pin(29)
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
#if ASSERT_PIN_IO(30)
#define io30_config_output mcu_config_output(30)
#define io30_set_output mcu_set_output(30)
#define io30_clear_output mcu_clear_output(30)
#define io30_toggle_output mcu_toggle_output(30)
#define io30_get_output mcu_get_output(30)
#define io30_config_input mcu_config_input(30)
#define io30_config_pullup mcu_config_pullup(30)
#define io30_get_input mcu_get_input(30)
#elif ASSERT_PIN_EXTENDED(30)
#define io30_config_output
#define io30_set_output ic74hc595_set_pin(30);ic74hc595_shift_io_pins()
#define io30_clear_output ic74hc595_clear_pin(30);ic74hc595_shift_io_pins()
#define io30_toggle_output ic74hc595_toggle_pin(30);ic74hc595_shift_io_pins()
#define io30_get_output ic74hc595_get_pin(30)
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
#if ASSERT_PIN_IO(31)
#define io31_config_output mcu_config_output(31)
#define io31_set_output mcu_set_output(31)
#define io31_clear_output mcu_clear_output(31)
#define io31_toggle_output mcu_toggle_output(31)
#define io31_get_output mcu_get_output(31)
#define io31_config_input mcu_config_input(31)
#define io31_config_pullup mcu_config_pullup(31)
#define io31_get_input mcu_get_input(31)
#elif ASSERT_PIN_EXTENDED(31)
#define io31_config_output
#define io31_set_output ic74hc595_set_pin(31);ic74hc595_shift_io_pins()
#define io31_clear_output ic74hc595_clear_pin(31);ic74hc595_shift_io_pins()
#define io31_toggle_output ic74hc595_toggle_pin(31);ic74hc595_shift_io_pins()
#define io31_get_output ic74hc595_get_pin(31)
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
#if ASSERT_PIN_IO(32)
#define io32_config_output mcu_config_output(32)
#define io32_set_output mcu_set_output(32)
#define io32_clear_output mcu_clear_output(32)
#define io32_toggle_output mcu_toggle_output(32)
#define io32_get_output mcu_get_output(32)
#define io32_config_input mcu_config_input(32)
#define io32_config_pullup mcu_config_pullup(32)
#define io32_get_input mcu_get_input(32)
#elif ASSERT_PIN_EXTENDED(32)
#define io32_config_output
#define io32_set_output ic74hc595_set_pin(32);ic74hc595_shift_io_pins()
#define io32_clear_output ic74hc595_clear_pin(32);ic74hc595_shift_io_pins()
#define io32_toggle_output ic74hc595_toggle_pin(32);ic74hc595_shift_io_pins()
#define io32_get_output ic74hc595_get_pin(32)
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
#if ASSERT_PIN_IO(33)
#define io33_config_output mcu_config_output(33)
#define io33_set_output mcu_set_output(33)
#define io33_clear_output mcu_clear_output(33)
#define io33_toggle_output mcu_toggle_output(33)
#define io33_get_output mcu_get_output(33)
#define io33_config_input mcu_config_input(33)
#define io33_config_pullup mcu_config_pullup(33)
#define io33_get_input mcu_get_input(33)
#elif ASSERT_PIN_EXTENDED(33)
#define io33_config_output
#define io33_set_output ic74hc595_set_pin(33);ic74hc595_shift_io_pins()
#define io33_clear_output ic74hc595_clear_pin(33);ic74hc595_shift_io_pins()
#define io33_toggle_output ic74hc595_toggle_pin(33);ic74hc595_shift_io_pins()
#define io33_get_output ic74hc595_get_pin(33)
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
#if ASSERT_PIN_IO(34)
#define io34_config_output mcu_config_output(34)
#define io34_set_output mcu_set_output(34)
#define io34_clear_output mcu_clear_output(34)
#define io34_toggle_output mcu_toggle_output(34)
#define io34_get_output mcu_get_output(34)
#define io34_config_input mcu_config_input(34)
#define io34_config_pullup mcu_config_pullup(34)
#define io34_get_input mcu_get_input(34)
#elif ASSERT_PIN_EXTENDED(34)
#define io34_config_output
#define io34_set_output ic74hc595_set_pin(34);ic74hc595_shift_io_pins()
#define io34_clear_output ic74hc595_clear_pin(34);ic74hc595_shift_io_pins()
#define io34_toggle_output ic74hc595_toggle_pin(34);ic74hc595_shift_io_pins()
#define io34_get_output ic74hc595_get_pin(34)
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
#if ASSERT_PIN_IO(35)
#define io35_config_output mcu_config_output(35)
#define io35_set_output mcu_set_output(35)
#define io35_clear_output mcu_clear_output(35)
#define io35_toggle_output mcu_toggle_output(35)
#define io35_get_output mcu_get_output(35)
#define io35_config_input mcu_config_input(35)
#define io35_config_pullup mcu_config_pullup(35)
#define io35_get_input mcu_get_input(35)
#elif ASSERT_PIN_EXTENDED(35)
#define io35_config_output
#define io35_set_output ic74hc595_set_pin(35);ic74hc595_shift_io_pins()
#define io35_clear_output ic74hc595_clear_pin(35);ic74hc595_shift_io_pins()
#define io35_toggle_output ic74hc595_toggle_pin(35);ic74hc595_shift_io_pins()
#define io35_get_output ic74hc595_get_pin(35)
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
#if ASSERT_PIN_IO(36)
#define io36_config_output mcu_config_output(36)
#define io36_set_output mcu_set_output(36)
#define io36_clear_output mcu_clear_output(36)
#define io36_toggle_output mcu_toggle_output(36)
#define io36_get_output mcu_get_output(36)
#define io36_config_input mcu_config_input(36)
#define io36_config_pullup mcu_config_pullup(36)
#define io36_get_input mcu_get_input(36)
#elif ASSERT_PIN_EXTENDED(36)
#define io36_config_output
#define io36_set_output ic74hc595_set_pin(36);ic74hc595_shift_io_pins()
#define io36_clear_output ic74hc595_clear_pin(36);ic74hc595_shift_io_pins()
#define io36_toggle_output ic74hc595_toggle_pin(36);ic74hc595_shift_io_pins()
#define io36_get_output ic74hc595_get_pin(36)
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
#if ASSERT_PIN_IO(37)
#define io37_config_output mcu_config_output(37)
#define io37_set_output mcu_set_output(37)
#define io37_clear_output mcu_clear_output(37)
#define io37_toggle_output mcu_toggle_output(37)
#define io37_get_output mcu_get_output(37)
#define io37_config_input mcu_config_input(37)
#define io37_config_pullup mcu_config_pullup(37)
#define io37_get_input mcu_get_input(37)
#elif ASSERT_PIN_EXTENDED(37)
#define io37_config_output
#define io37_set_output ic74hc595_set_pin(37);ic74hc595_shift_io_pins()
#define io37_clear_output ic74hc595_clear_pin(37);ic74hc595_shift_io_pins()
#define io37_toggle_output ic74hc595_toggle_pin(37);ic74hc595_shift_io_pins()
#define io37_get_output ic74hc595_get_pin(37)
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
#if ASSERT_PIN_IO(38)
#define io38_config_output mcu_config_output(38)
#define io38_set_output mcu_set_output(38)
#define io38_clear_output mcu_clear_output(38)
#define io38_toggle_output mcu_toggle_output(38)
#define io38_get_output mcu_get_output(38)
#define io38_config_input mcu_config_input(38)
#define io38_config_pullup mcu_config_pullup(38)
#define io38_get_input mcu_get_input(38)
#elif ASSERT_PIN_EXTENDED(38)
#define io38_config_output
#define io38_set_output ic74hc595_set_pin(38);ic74hc595_shift_io_pins()
#define io38_clear_output ic74hc595_clear_pin(38);ic74hc595_shift_io_pins()
#define io38_toggle_output ic74hc595_toggle_pin(38);ic74hc595_shift_io_pins()
#define io38_get_output ic74hc595_get_pin(38)
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
#if ASSERT_PIN_IO(39)
#define io39_config_output mcu_config_output(39)
#define io39_set_output mcu_set_output(39)
#define io39_clear_output mcu_clear_output(39)
#define io39_toggle_output mcu_toggle_output(39)
#define io39_get_output mcu_get_output(39)
#define io39_config_input mcu_config_input(39)
#define io39_config_pullup mcu_config_pullup(39)
#define io39_get_input mcu_get_input(39)
#elif ASSERT_PIN_EXTENDED(39)
#define io39_config_output
#define io39_set_output ic74hc595_set_pin(39);ic74hc595_shift_io_pins()
#define io39_clear_output ic74hc595_clear_pin(39);ic74hc595_shift_io_pins()
#define io39_toggle_output ic74hc595_toggle_pin(39);ic74hc595_shift_io_pins()
#define io39_get_output ic74hc595_get_pin(39)
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
#if ASSERT_PIN_IO(40)
#define io40_config_output mcu_config_output(40)
#define io40_set_output mcu_set_output(40)
#define io40_clear_output mcu_clear_output(40)
#define io40_toggle_output mcu_toggle_output(40)
#define io40_get_output mcu_get_output(40)
#define io40_config_input mcu_config_input(40)
#define io40_config_pullup mcu_config_pullup(40)
#define io40_get_input mcu_get_input(40)
#elif ASSERT_PIN_EXTENDED(40)
#define io40_config_output
#define io40_set_output ic74hc595_set_pin(40);ic74hc595_shift_io_pins()
#define io40_clear_output ic74hc595_clear_pin(40);ic74hc595_shift_io_pins()
#define io40_toggle_output ic74hc595_toggle_pin(40);ic74hc595_shift_io_pins()
#define io40_get_output ic74hc595_get_pin(40)
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
#if ASSERT_PIN_IO(41)
#define io41_config_output mcu_config_output(41)
#define io41_set_output mcu_set_output(41)
#define io41_clear_output mcu_clear_output(41)
#define io41_toggle_output mcu_toggle_output(41)
#define io41_get_output mcu_get_output(41)
#define io41_config_input mcu_config_input(41)
#define io41_config_pullup mcu_config_pullup(41)
#define io41_get_input mcu_get_input(41)
#elif ASSERT_PIN_EXTENDED(41)
#define io41_config_output
#define io41_set_output ic74hc595_set_pin(41);ic74hc595_shift_io_pins()
#define io41_clear_output ic74hc595_clear_pin(41);ic74hc595_shift_io_pins()
#define io41_toggle_output ic74hc595_toggle_pin(41);ic74hc595_shift_io_pins()
#define io41_get_output ic74hc595_get_pin(41)
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
#if ASSERT_PIN_IO(42)
#define io42_config_output mcu_config_output(42)
#define io42_set_output mcu_set_output(42)
#define io42_clear_output mcu_clear_output(42)
#define io42_toggle_output mcu_toggle_output(42)
#define io42_get_output mcu_get_output(42)
#define io42_config_input mcu_config_input(42)
#define io42_config_pullup mcu_config_pullup(42)
#define io42_get_input mcu_get_input(42)
#elif ASSERT_PIN_EXTENDED(42)
#define io42_config_output
#define io42_set_output ic74hc595_set_pin(42);ic74hc595_shift_io_pins()
#define io42_clear_output ic74hc595_clear_pin(42);ic74hc595_shift_io_pins()
#define io42_toggle_output ic74hc595_toggle_pin(42);ic74hc595_shift_io_pins()
#define io42_get_output ic74hc595_get_pin(42)
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
#if ASSERT_PIN_IO(43)
#define io43_config_output mcu_config_output(43)
#define io43_set_output mcu_set_output(43)
#define io43_clear_output mcu_clear_output(43)
#define io43_toggle_output mcu_toggle_output(43)
#define io43_get_output mcu_get_output(43)
#define io43_config_input mcu_config_input(43)
#define io43_config_pullup mcu_config_pullup(43)
#define io43_get_input mcu_get_input(43)
#elif ASSERT_PIN_EXTENDED(43)
#define io43_config_output
#define io43_set_output ic74hc595_set_pin(43);ic74hc595_shift_io_pins()
#define io43_clear_output ic74hc595_clear_pin(43);ic74hc595_shift_io_pins()
#define io43_toggle_output ic74hc595_toggle_pin(43);ic74hc595_shift_io_pins()
#define io43_get_output ic74hc595_get_pin(43)
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
#if ASSERT_PIN_IO(44)
#define io44_config_output mcu_config_output(44)
#define io44_set_output mcu_set_output(44)
#define io44_clear_output mcu_clear_output(44)
#define io44_toggle_output mcu_toggle_output(44)
#define io44_get_output mcu_get_output(44)
#define io44_config_input mcu_config_input(44)
#define io44_config_pullup mcu_config_pullup(44)
#define io44_get_input mcu_get_input(44)
#elif ASSERT_PIN_EXTENDED(44)
#define io44_config_output
#define io44_set_output ic74hc595_set_pin(44);ic74hc595_shift_io_pins()
#define io44_clear_output ic74hc595_clear_pin(44);ic74hc595_shift_io_pins()
#define io44_toggle_output ic74hc595_toggle_pin(44);ic74hc595_shift_io_pins()
#define io44_get_output ic74hc595_get_pin(44)
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
#if ASSERT_PIN_IO(45)
#define io45_config_output mcu_config_output(45)
#define io45_set_output mcu_set_output(45)
#define io45_clear_output mcu_clear_output(45)
#define io45_toggle_output mcu_toggle_output(45)
#define io45_get_output mcu_get_output(45)
#define io45_config_input mcu_config_input(45)
#define io45_config_pullup mcu_config_pullup(45)
#define io45_get_input mcu_get_input(45)
#elif ASSERT_PIN_EXTENDED(45)
#define io45_config_output
#define io45_set_output ic74hc595_set_pin(45);ic74hc595_shift_io_pins()
#define io45_clear_output ic74hc595_clear_pin(45);ic74hc595_shift_io_pins()
#define io45_toggle_output ic74hc595_toggle_pin(45);ic74hc595_shift_io_pins()
#define io45_get_output ic74hc595_get_pin(45)
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
#if ASSERT_PIN_IO(46)
#define io46_config_output mcu_config_output(46)
#define io46_set_output mcu_set_output(46)
#define io46_clear_output mcu_clear_output(46)
#define io46_toggle_output mcu_toggle_output(46)
#define io46_get_output mcu_get_output(46)
#define io46_config_input mcu_config_input(46)
#define io46_config_pullup mcu_config_pullup(46)
#define io46_get_input mcu_get_input(46)
#elif ASSERT_PIN_EXTENDED(46)
#define io46_config_output
#define io46_set_output ic74hc595_set_pin(46);ic74hc595_shift_io_pins()
#define io46_clear_output ic74hc595_clear_pin(46);ic74hc595_shift_io_pins()
#define io46_toggle_output ic74hc595_toggle_pin(46);ic74hc595_shift_io_pins()
#define io46_get_output ic74hc595_get_pin(46)
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
#if ASSERT_PIN_IO(47)
#define io47_config_output mcu_config_output(47)
#define io47_set_output mcu_set_output(47)
#define io47_clear_output mcu_clear_output(47)
#define io47_toggle_output mcu_toggle_output(47)
#define io47_get_output mcu_get_output(47)
#define io47_config_input mcu_config_input(47)
#define io47_config_pullup mcu_config_pullup(47)
#define io47_get_input mcu_get_input(47)
#elif ASSERT_PIN_EXTENDED(47)
#define io47_config_output
#define io47_set_output ic74hc595_set_pin(47);ic74hc595_shift_io_pins()
#define io47_clear_output ic74hc595_clear_pin(47);ic74hc595_shift_io_pins()
#define io47_toggle_output ic74hc595_toggle_pin(47);ic74hc595_shift_io_pins()
#define io47_get_output ic74hc595_get_pin(47)
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
#if ASSERT_PIN_IO(48)
#define io48_config_output mcu_config_output(48)
#define io48_set_output mcu_set_output(48)
#define io48_clear_output mcu_clear_output(48)
#define io48_toggle_output mcu_toggle_output(48)
#define io48_get_output mcu_get_output(48)
#define io48_config_input mcu_config_input(48)
#define io48_config_pullup mcu_config_pullup(48)
#define io48_get_input mcu_get_input(48)
#elif ASSERT_PIN_EXTENDED(48)
#define io48_config_output
#define io48_set_output ic74hc595_set_pin(48);ic74hc595_shift_io_pins()
#define io48_clear_output ic74hc595_clear_pin(48);ic74hc595_shift_io_pins()
#define io48_toggle_output ic74hc595_toggle_pin(48);ic74hc595_shift_io_pins()
#define io48_get_output ic74hc595_get_pin(48)
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
#if ASSERT_PIN_IO(49)
#define io49_config_output mcu_config_output(49)
#define io49_set_output mcu_set_output(49)
#define io49_clear_output mcu_clear_output(49)
#define io49_toggle_output mcu_toggle_output(49)
#define io49_get_output mcu_get_output(49)
#define io49_config_input mcu_config_input(49)
#define io49_config_pullup mcu_config_pullup(49)
#define io49_get_input mcu_get_input(49)
#elif ASSERT_PIN_EXTENDED(49)
#define io49_config_output
#define io49_set_output ic74hc595_set_pin(49);ic74hc595_shift_io_pins()
#define io49_clear_output ic74hc595_clear_pin(49);ic74hc595_shift_io_pins()
#define io49_toggle_output ic74hc595_toggle_pin(49);ic74hc595_shift_io_pins()
#define io49_get_output ic74hc595_get_pin(49)
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
#if ASSERT_PIN_IO(50)
#define io50_config_output mcu_config_output(50)
#define io50_set_output mcu_set_output(50)
#define io50_clear_output mcu_clear_output(50)
#define io50_toggle_output mcu_toggle_output(50)
#define io50_get_output mcu_get_output(50)
#define io50_config_input mcu_config_input(50)
#define io50_config_pullup mcu_config_pullup(50)
#define io50_get_input mcu_get_input(50)
#elif ASSERT_PIN_EXTENDED(50)
#define io50_config_output
#define io50_set_output ic74hc595_set_pin(50);ic74hc595_shift_io_pins()
#define io50_clear_output ic74hc595_clear_pin(50);ic74hc595_shift_io_pins()
#define io50_toggle_output ic74hc595_toggle_pin(50);ic74hc595_shift_io_pins()
#define io50_get_output ic74hc595_get_pin(50)
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
#if ASSERT_PIN_IO(51)
#define io51_config_output mcu_config_output(51)
#define io51_set_output mcu_set_output(51)
#define io51_clear_output mcu_clear_output(51)
#define io51_toggle_output mcu_toggle_output(51)
#define io51_get_output mcu_get_output(51)
#define io51_config_input mcu_config_input(51)
#define io51_config_pullup mcu_config_pullup(51)
#define io51_get_input mcu_get_input(51)
#elif ASSERT_PIN_EXTENDED(51)
#define io51_config_output
#define io51_set_output ic74hc595_set_pin(51);ic74hc595_shift_io_pins()
#define io51_clear_output ic74hc595_clear_pin(51);ic74hc595_shift_io_pins()
#define io51_toggle_output ic74hc595_toggle_pin(51);ic74hc595_shift_io_pins()
#define io51_get_output ic74hc595_get_pin(51)
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
#if ASSERT_PIN_IO(52)
#define io52_config_output mcu_config_output(52)
#define io52_set_output mcu_set_output(52)
#define io52_clear_output mcu_clear_output(52)
#define io52_toggle_output mcu_toggle_output(52)
#define io52_get_output mcu_get_output(52)
#define io52_config_input mcu_config_input(52)
#define io52_config_pullup mcu_config_pullup(52)
#define io52_get_input mcu_get_input(52)
#elif ASSERT_PIN_EXTENDED(52)
#define io52_config_output
#define io52_set_output ic74hc595_set_pin(52);ic74hc595_shift_io_pins()
#define io52_clear_output ic74hc595_clear_pin(52);ic74hc595_shift_io_pins()
#define io52_toggle_output ic74hc595_toggle_pin(52);ic74hc595_shift_io_pins()
#define io52_get_output ic74hc595_get_pin(52)
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
#if ASSERT_PIN_IO(53)
#define io53_config_output mcu_config_output(53)
#define io53_set_output mcu_set_output(53)
#define io53_clear_output mcu_clear_output(53)
#define io53_toggle_output mcu_toggle_output(53)
#define io53_get_output mcu_get_output(53)
#define io53_config_input mcu_config_input(53)
#define io53_config_pullup mcu_config_pullup(53)
#define io53_get_input mcu_get_input(53)
#elif ASSERT_PIN_EXTENDED(53)
#define io53_config_output
#define io53_set_output ic74hc595_set_pin(53);ic74hc595_shift_io_pins()
#define io53_clear_output ic74hc595_clear_pin(53);ic74hc595_shift_io_pins()
#define io53_toggle_output ic74hc595_toggle_pin(53);ic74hc595_shift_io_pins()
#define io53_get_output ic74hc595_get_pin(53)
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
#if ASSERT_PIN_IO(54)
#define io54_config_output mcu_config_output(54)
#define io54_set_output mcu_set_output(54)
#define io54_clear_output mcu_clear_output(54)
#define io54_toggle_output mcu_toggle_output(54)
#define io54_get_output mcu_get_output(54)
#define io54_config_input mcu_config_input(54)
#define io54_config_pullup mcu_config_pullup(54)
#define io54_get_input mcu_get_input(54)
#elif ASSERT_PIN_EXTENDED(54)
#define io54_config_output
#define io54_set_output ic74hc595_set_pin(54);ic74hc595_shift_io_pins()
#define io54_clear_output ic74hc595_clear_pin(54);ic74hc595_shift_io_pins()
#define io54_toggle_output ic74hc595_toggle_pin(54);ic74hc595_shift_io_pins()
#define io54_get_output ic74hc595_get_pin(54)
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
#if ASSERT_PIN_IO(55)
#define io55_config_output mcu_config_output(55)
#define io55_set_output mcu_set_output(55)
#define io55_clear_output mcu_clear_output(55)
#define io55_toggle_output mcu_toggle_output(55)
#define io55_get_output mcu_get_output(55)
#define io55_config_input mcu_config_input(55)
#define io55_config_pullup mcu_config_pullup(55)
#define io55_get_input mcu_get_input(55)
#elif ASSERT_PIN_EXTENDED(55)
#define io55_config_output
#define io55_set_output ic74hc595_set_pin(55);ic74hc595_shift_io_pins()
#define io55_clear_output ic74hc595_clear_pin(55);ic74hc595_shift_io_pins()
#define io55_toggle_output ic74hc595_toggle_pin(55);ic74hc595_shift_io_pins()
#define io55_get_output ic74hc595_get_pin(55)
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
#if ASSERT_PIN_IO(56)
#define io56_config_output mcu_config_output(56)
#define io56_set_output mcu_set_output(56)
#define io56_clear_output mcu_clear_output(56)
#define io56_toggle_output mcu_toggle_output(56)
#define io56_get_output mcu_get_output(56)
#define io56_config_input mcu_config_input(56)
#define io56_config_pullup mcu_config_pullup(56)
#define io56_get_input mcu_get_input(56)
#elif ASSERT_PIN_EXTENDED(56)
#define io56_config_output
#define io56_set_output ic74hc595_set_pin(56);ic74hc595_shift_io_pins()
#define io56_clear_output ic74hc595_clear_pin(56);ic74hc595_shift_io_pins()
#define io56_toggle_output ic74hc595_toggle_pin(56);ic74hc595_shift_io_pins()
#define io56_get_output ic74hc595_get_pin(56)
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
#if ASSERT_PIN_IO(57)
#define io57_config_output mcu_config_output(57)
#define io57_set_output mcu_set_output(57)
#define io57_clear_output mcu_clear_output(57)
#define io57_toggle_output mcu_toggle_output(57)
#define io57_get_output mcu_get_output(57)
#define io57_config_input mcu_config_input(57)
#define io57_config_pullup mcu_config_pullup(57)
#define io57_get_input mcu_get_input(57)
#elif ASSERT_PIN_EXTENDED(57)
#define io57_config_output
#define io57_set_output ic74hc595_set_pin(57);ic74hc595_shift_io_pins()
#define io57_clear_output ic74hc595_clear_pin(57);ic74hc595_shift_io_pins()
#define io57_toggle_output ic74hc595_toggle_pin(57);ic74hc595_shift_io_pins()
#define io57_get_output ic74hc595_get_pin(57)
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
#if ASSERT_PIN_IO(58)
#define io58_config_output mcu_config_output(58)
#define io58_set_output mcu_set_output(58)
#define io58_clear_output mcu_clear_output(58)
#define io58_toggle_output mcu_toggle_output(58)
#define io58_get_output mcu_get_output(58)
#define io58_config_input mcu_config_input(58)
#define io58_config_pullup mcu_config_pullup(58)
#define io58_get_input mcu_get_input(58)
#elif ASSERT_PIN_EXTENDED(58)
#define io58_config_output
#define io58_set_output ic74hc595_set_pin(58);ic74hc595_shift_io_pins()
#define io58_clear_output ic74hc595_clear_pin(58);ic74hc595_shift_io_pins()
#define io58_toggle_output ic74hc595_toggle_pin(58);ic74hc595_shift_io_pins()
#define io58_get_output ic74hc595_get_pin(58)
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
#if ASSERT_PIN_IO(59)
#define io59_config_output mcu_config_output(59)
#define io59_set_output mcu_set_output(59)
#define io59_clear_output mcu_clear_output(59)
#define io59_toggle_output mcu_toggle_output(59)
#define io59_get_output mcu_get_output(59)
#define io59_config_input mcu_config_input(59)
#define io59_config_pullup mcu_config_pullup(59)
#define io59_get_input mcu_get_input(59)
#elif ASSERT_PIN_EXTENDED(59)
#define io59_config_output
#define io59_set_output ic74hc595_set_pin(59);ic74hc595_shift_io_pins()
#define io59_clear_output ic74hc595_clear_pin(59);ic74hc595_shift_io_pins()
#define io59_toggle_output ic74hc595_toggle_pin(59);ic74hc595_shift_io_pins()
#define io59_get_output ic74hc595_get_pin(59)
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
#if ASSERT_PIN_IO(60)
#define io60_config_output mcu_config_output(60)
#define io60_set_output mcu_set_output(60)
#define io60_clear_output mcu_clear_output(60)
#define io60_toggle_output mcu_toggle_output(60)
#define io60_get_output mcu_get_output(60)
#define io60_config_input mcu_config_input(60)
#define io60_config_pullup mcu_config_pullup(60)
#define io60_get_input mcu_get_input(60)
#elif ASSERT_PIN_EXTENDED(60)
#define io60_config_output
#define io60_set_output ic74hc595_set_pin(60);ic74hc595_shift_io_pins()
#define io60_clear_output ic74hc595_clear_pin(60);ic74hc595_shift_io_pins()
#define io60_toggle_output ic74hc595_toggle_pin(60);ic74hc595_shift_io_pins()
#define io60_get_output ic74hc595_get_pin(60)
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
#if ASSERT_PIN_IO(61)
#define io61_config_output mcu_config_output(61)
#define io61_set_output mcu_set_output(61)
#define io61_clear_output mcu_clear_output(61)
#define io61_toggle_output mcu_toggle_output(61)
#define io61_get_output mcu_get_output(61)
#define io61_config_input mcu_config_input(61)
#define io61_config_pullup mcu_config_pullup(61)
#define io61_get_input mcu_get_input(61)
#elif ASSERT_PIN_EXTENDED(61)
#define io61_config_output
#define io61_set_output ic74hc595_set_pin(61);ic74hc595_shift_io_pins()
#define io61_clear_output ic74hc595_clear_pin(61);ic74hc595_shift_io_pins()
#define io61_toggle_output ic74hc595_toggle_pin(61);ic74hc595_shift_io_pins()
#define io61_get_output ic74hc595_get_pin(61)
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
#if ASSERT_PIN_IO(62)
#define io62_config_output mcu_config_output(62)
#define io62_set_output mcu_set_output(62)
#define io62_clear_output mcu_clear_output(62)
#define io62_toggle_output mcu_toggle_output(62)
#define io62_get_output mcu_get_output(62)
#define io62_config_input mcu_config_input(62)
#define io62_config_pullup mcu_config_pullup(62)
#define io62_get_input mcu_get_input(62)
#elif ASSERT_PIN_EXTENDED(62)
#define io62_config_output
#define io62_set_output ic74hc595_set_pin(62);ic74hc595_shift_io_pins()
#define io62_clear_output ic74hc595_clear_pin(62);ic74hc595_shift_io_pins()
#define io62_toggle_output ic74hc595_toggle_pin(62);ic74hc595_shift_io_pins()
#define io62_get_output ic74hc595_get_pin(62)
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
#if ASSERT_PIN_IO(63)
#define io63_config_output mcu_config_output(63)
#define io63_set_output mcu_set_output(63)
#define io63_clear_output mcu_clear_output(63)
#define io63_toggle_output mcu_toggle_output(63)
#define io63_get_output mcu_get_output(63)
#define io63_config_input mcu_config_input(63)
#define io63_config_pullup mcu_config_pullup(63)
#define io63_get_input mcu_get_input(63)
#elif ASSERT_PIN_EXTENDED(63)
#define io63_config_output
#define io63_set_output ic74hc595_set_pin(63);ic74hc595_shift_io_pins()
#define io63_clear_output ic74hc595_clear_pin(63);ic74hc595_shift_io_pins()
#define io63_toggle_output ic74hc595_toggle_pin(63);ic74hc595_shift_io_pins()
#define io63_get_output ic74hc595_get_pin(63)
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
#if ASSERT_PIN_IO(64)
#define io64_config_output mcu_config_output(64)
#define io64_set_output mcu_set_output(64)
#define io64_clear_output mcu_clear_output(64)
#define io64_toggle_output mcu_toggle_output(64)
#define io64_get_output mcu_get_output(64)
#define io64_config_input mcu_config_input(64)
#define io64_config_pullup mcu_config_pullup(64)
#define io64_get_input mcu_get_input(64)
#elif ASSERT_PIN_EXTENDED(64)
#define io64_config_output
#define io64_set_output ic74hc595_set_pin(64);ic74hc595_shift_io_pins()
#define io64_clear_output ic74hc595_clear_pin(64);ic74hc595_shift_io_pins()
#define io64_toggle_output ic74hc595_toggle_pin(64);ic74hc595_shift_io_pins()
#define io64_get_output ic74hc595_get_pin(64)
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
#if ASSERT_PIN_IO(65)
#define io65_config_output mcu_config_output(65)
#define io65_set_output mcu_set_output(65)
#define io65_clear_output mcu_clear_output(65)
#define io65_toggle_output mcu_toggle_output(65)
#define io65_get_output mcu_get_output(65)
#define io65_config_input mcu_config_input(65)
#define io65_config_pullup mcu_config_pullup(65)
#define io65_get_input mcu_get_input(65)
#elif ASSERT_PIN_EXTENDED(65)
#define io65_config_output
#define io65_set_output ic74hc595_set_pin(65);ic74hc595_shift_io_pins()
#define io65_clear_output ic74hc595_clear_pin(65);ic74hc595_shift_io_pins()
#define io65_toggle_output ic74hc595_toggle_pin(65);ic74hc595_shift_io_pins()
#define io65_get_output ic74hc595_get_pin(65)
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
#if ASSERT_PIN_IO(66)
#define io66_config_output mcu_config_output(66)
#define io66_set_output mcu_set_output(66)
#define io66_clear_output mcu_clear_output(66)
#define io66_toggle_output mcu_toggle_output(66)
#define io66_get_output mcu_get_output(66)
#define io66_config_input mcu_config_input(66)
#define io66_config_pullup mcu_config_pullup(66)
#define io66_get_input mcu_get_input(66)
#elif ASSERT_PIN_EXTENDED(66)
#define io66_config_output
#define io66_set_output ic74hc595_set_pin(66);ic74hc595_shift_io_pins()
#define io66_clear_output ic74hc595_clear_pin(66);ic74hc595_shift_io_pins()
#define io66_toggle_output ic74hc595_toggle_pin(66);ic74hc595_shift_io_pins()
#define io66_get_output ic74hc595_get_pin(66)
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
#if ASSERT_PIN_IO(67)
#define io67_config_output mcu_config_output(67)
#define io67_set_output mcu_set_output(67)
#define io67_clear_output mcu_clear_output(67)
#define io67_toggle_output mcu_toggle_output(67)
#define io67_get_output mcu_get_output(67)
#define io67_config_input mcu_config_input(67)
#define io67_config_pullup mcu_config_pullup(67)
#define io67_get_input mcu_get_input(67)
#elif ASSERT_PIN_EXTENDED(67)
#define io67_config_output
#define io67_set_output ic74hc595_set_pin(67);ic74hc595_shift_io_pins()
#define io67_clear_output ic74hc595_clear_pin(67);ic74hc595_shift_io_pins()
#define io67_toggle_output ic74hc595_toggle_pin(67);ic74hc595_shift_io_pins()
#define io67_get_output ic74hc595_get_pin(67)
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
#if ASSERT_PIN_IO(68)
#define io68_config_output mcu_config_output(68)
#define io68_set_output mcu_set_output(68)
#define io68_clear_output mcu_clear_output(68)
#define io68_toggle_output mcu_toggle_output(68)
#define io68_get_output mcu_get_output(68)
#define io68_config_input mcu_config_input(68)
#define io68_config_pullup mcu_config_pullup(68)
#define io68_get_input mcu_get_input(68)
#elif ASSERT_PIN_EXTENDED(68)
#define io68_config_output
#define io68_set_output ic74hc595_set_pin(68);ic74hc595_shift_io_pins()
#define io68_clear_output ic74hc595_clear_pin(68);ic74hc595_shift_io_pins()
#define io68_toggle_output ic74hc595_toggle_pin(68);ic74hc595_shift_io_pins()
#define io68_get_output ic74hc595_get_pin(68)
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
#if ASSERT_PIN_IO(69)
#define io69_config_output mcu_config_output(69)
#define io69_set_output mcu_set_output(69)
#define io69_clear_output mcu_clear_output(69)
#define io69_toggle_output mcu_toggle_output(69)
#define io69_get_output mcu_get_output(69)
#define io69_config_input mcu_config_input(69)
#define io69_config_pullup mcu_config_pullup(69)
#define io69_get_input mcu_get_input(69)
#elif ASSERT_PIN_EXTENDED(69)
#define io69_config_output
#define io69_set_output ic74hc595_set_pin(69);ic74hc595_shift_io_pins()
#define io69_clear_output ic74hc595_clear_pin(69);ic74hc595_shift_io_pins()
#define io69_toggle_output ic74hc595_toggle_pin(69);ic74hc595_shift_io_pins()
#define io69_get_output ic74hc595_get_pin(69)
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
#if ASSERT_PIN_IO(70)
#define io70_config_output mcu_config_output(70)
#define io70_set_output mcu_set_output(70)
#define io70_clear_output mcu_clear_output(70)
#define io70_toggle_output mcu_toggle_output(70)
#define io70_get_output mcu_get_output(70)
#define io70_config_input mcu_config_input(70)
#define io70_config_pullup mcu_config_pullup(70)
#define io70_get_input mcu_get_input(70)
#elif ASSERT_PIN_EXTENDED(70)
#define io70_config_output
#define io70_set_output ic74hc595_set_pin(70);ic74hc595_shift_io_pins()
#define io70_clear_output ic74hc595_clear_pin(70);ic74hc595_shift_io_pins()
#define io70_toggle_output ic74hc595_toggle_pin(70);ic74hc595_shift_io_pins()
#define io70_get_output ic74hc595_get_pin(70)
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
#if ASSERT_PIN_IO(71)
#define io71_config_output mcu_config_output(71)
#define io71_set_output mcu_set_output(71)
#define io71_clear_output mcu_clear_output(71)
#define io71_toggle_output mcu_toggle_output(71)
#define io71_get_output mcu_get_output(71)
#define io71_config_input mcu_config_input(71)
#define io71_config_pullup mcu_config_pullup(71)
#define io71_get_input mcu_get_input(71)
#elif ASSERT_PIN_EXTENDED(71)
#define io71_config_output
#define io71_set_output ic74hc595_set_pin(71);ic74hc595_shift_io_pins()
#define io71_clear_output ic74hc595_clear_pin(71);ic74hc595_shift_io_pins()
#define io71_toggle_output ic74hc595_toggle_pin(71);ic74hc595_shift_io_pins()
#define io71_get_output ic74hc595_get_pin(71)
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
#if ASSERT_PIN_IO(72)
#define io72_config_output mcu_config_output(72)
#define io72_set_output mcu_set_output(72)
#define io72_clear_output mcu_clear_output(72)
#define io72_toggle_output mcu_toggle_output(72)
#define io72_get_output mcu_get_output(72)
#define io72_config_input mcu_config_input(72)
#define io72_config_pullup mcu_config_pullup(72)
#define io72_get_input mcu_get_input(72)
#elif ASSERT_PIN_EXTENDED(72)
#define io72_config_output
#define io72_set_output ic74hc595_set_pin(72);ic74hc595_shift_io_pins()
#define io72_clear_output ic74hc595_clear_pin(72);ic74hc595_shift_io_pins()
#define io72_toggle_output ic74hc595_toggle_pin(72);ic74hc595_shift_io_pins()
#define io72_get_output ic74hc595_get_pin(72)
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
#if ASSERT_PIN_IO(73)
#define io73_config_output mcu_config_output(73)
#define io73_set_output mcu_set_output(73)
#define io73_clear_output mcu_clear_output(73)
#define io73_toggle_output mcu_toggle_output(73)
#define io73_get_output mcu_get_output(73)
#define io73_config_input mcu_config_input(73)
#define io73_config_pullup mcu_config_pullup(73)
#define io73_get_input mcu_get_input(73)
#elif ASSERT_PIN_EXTENDED(73)
#define io73_config_output
#define io73_set_output ic74hc595_set_pin(73);ic74hc595_shift_io_pins()
#define io73_clear_output ic74hc595_clear_pin(73);ic74hc595_shift_io_pins()
#define io73_toggle_output ic74hc595_toggle_pin(73);ic74hc595_shift_io_pins()
#define io73_get_output ic74hc595_get_pin(73)
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
#if ASSERT_PIN_IO(74)
#define io74_config_output mcu_config_output(74)
#define io74_set_output mcu_set_output(74)
#define io74_clear_output mcu_clear_output(74)
#define io74_toggle_output mcu_toggle_output(74)
#define io74_get_output mcu_get_output(74)
#define io74_config_input mcu_config_input(74)
#define io74_config_pullup mcu_config_pullup(74)
#define io74_get_input mcu_get_input(74)
#elif ASSERT_PIN_EXTENDED(74)
#define io74_config_output
#define io74_set_output ic74hc595_set_pin(74);ic74hc595_shift_io_pins()
#define io74_clear_output ic74hc595_clear_pin(74);ic74hc595_shift_io_pins()
#define io74_toggle_output ic74hc595_toggle_pin(74);ic74hc595_shift_io_pins()
#define io74_get_output ic74hc595_get_pin(74)
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
#if ASSERT_PIN_IO(75)
#define io75_config_output mcu_config_output(75)
#define io75_set_output mcu_set_output(75)
#define io75_clear_output mcu_clear_output(75)
#define io75_toggle_output mcu_toggle_output(75)
#define io75_get_output mcu_get_output(75)
#define io75_config_input mcu_config_input(75)
#define io75_config_pullup mcu_config_pullup(75)
#define io75_get_input mcu_get_input(75)
#elif ASSERT_PIN_EXTENDED(75)
#define io75_config_output
#define io75_set_output ic74hc595_set_pin(75);ic74hc595_shift_io_pins()
#define io75_clear_output ic74hc595_clear_pin(75);ic74hc595_shift_io_pins()
#define io75_toggle_output ic74hc595_toggle_pin(75);ic74hc595_shift_io_pins()
#define io75_get_output ic74hc595_get_pin(75)
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
#if ASSERT_PIN_IO(76)
#define io76_config_output mcu_config_output(76)
#define io76_set_output mcu_set_output(76)
#define io76_clear_output mcu_clear_output(76)
#define io76_toggle_output mcu_toggle_output(76)
#define io76_get_output mcu_get_output(76)
#define io76_config_input mcu_config_input(76)
#define io76_config_pullup mcu_config_pullup(76)
#define io76_get_input mcu_get_input(76)
#elif ASSERT_PIN_EXTENDED(76)
#define io76_config_output
#define io76_set_output ic74hc595_set_pin(76);ic74hc595_shift_io_pins()
#define io76_clear_output ic74hc595_clear_pin(76);ic74hc595_shift_io_pins()
#define io76_toggle_output ic74hc595_toggle_pin(76);ic74hc595_shift_io_pins()
#define io76_get_output ic74hc595_get_pin(76)
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
#if ASSERT_PIN_IO(77)
#define io77_config_output mcu_config_output(77)
#define io77_set_output mcu_set_output(77)
#define io77_clear_output mcu_clear_output(77)
#define io77_toggle_output mcu_toggle_output(77)
#define io77_get_output mcu_get_output(77)
#define io77_config_input mcu_config_input(77)
#define io77_config_pullup mcu_config_pullup(77)
#define io77_get_input mcu_get_input(77)
#elif ASSERT_PIN_EXTENDED(77)
#define io77_config_output
#define io77_set_output ic74hc595_set_pin(77);ic74hc595_shift_io_pins()
#define io77_clear_output ic74hc595_clear_pin(77);ic74hc595_shift_io_pins()
#define io77_toggle_output ic74hc595_toggle_pin(77);ic74hc595_shift_io_pins()
#define io77_get_output ic74hc595_get_pin(77)
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
#if ASSERT_PIN_IO(78)
#define io78_config_output mcu_config_output(78)
#define io78_set_output mcu_set_output(78)
#define io78_clear_output mcu_clear_output(78)
#define io78_toggle_output mcu_toggle_output(78)
#define io78_get_output mcu_get_output(78)
#define io78_config_input mcu_config_input(78)
#define io78_config_pullup mcu_config_pullup(78)
#define io78_get_input mcu_get_input(78)
#elif ASSERT_PIN_EXTENDED(78)
#define io78_config_output
#define io78_set_output ic74hc595_set_pin(78);ic74hc595_shift_io_pins()
#define io78_clear_output ic74hc595_clear_pin(78);ic74hc595_shift_io_pins()
#define io78_toggle_output ic74hc595_toggle_pin(78);ic74hc595_shift_io_pins()
#define io78_get_output ic74hc595_get_pin(78)
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
#if ASSERT_PIN_IO(100)
#define io100_config_output mcu_config_output(100)
#define io100_set_output mcu_set_output(100)
#define io100_clear_output mcu_clear_output(100)
#define io100_toggle_output mcu_toggle_output(100)
#define io100_get_output mcu_get_output(100)
#define io100_config_input mcu_config_input(100)
#define io100_config_pullup mcu_config_pullup(100)
#define io100_get_input mcu_get_input(100)
#elif ASSERT_PIN_EXTENDED(100)
#define io100_config_output
#define io100_set_output ic74hc595_set_pin(100);ic74hc595_shift_io_pins()
#define io100_clear_output ic74hc595_clear_pin(100);ic74hc595_shift_io_pins()
#define io100_toggle_output ic74hc595_toggle_pin(100);ic74hc595_shift_io_pins()
#define io100_get_output ic74hc595_get_pin(100)
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
#if ASSERT_PIN_IO(101)
#define io101_config_output mcu_config_output(101)
#define io101_set_output mcu_set_output(101)
#define io101_clear_output mcu_clear_output(101)
#define io101_toggle_output mcu_toggle_output(101)
#define io101_get_output mcu_get_output(101)
#define io101_config_input mcu_config_input(101)
#define io101_config_pullup mcu_config_pullup(101)
#define io101_get_input mcu_get_input(101)
#elif ASSERT_PIN_EXTENDED(101)
#define io101_config_output
#define io101_set_output ic74hc595_set_pin(101);ic74hc595_shift_io_pins()
#define io101_clear_output ic74hc595_clear_pin(101);ic74hc595_shift_io_pins()
#define io101_toggle_output ic74hc595_toggle_pin(101);ic74hc595_shift_io_pins()
#define io101_get_output ic74hc595_get_pin(101)
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
#if ASSERT_PIN_IO(102)
#define io102_config_output mcu_config_output(102)
#define io102_set_output mcu_set_output(102)
#define io102_clear_output mcu_clear_output(102)
#define io102_toggle_output mcu_toggle_output(102)
#define io102_get_output mcu_get_output(102)
#define io102_config_input mcu_config_input(102)
#define io102_config_pullup mcu_config_pullup(102)
#define io102_get_input mcu_get_input(102)
#elif ASSERT_PIN_EXTENDED(102)
#define io102_config_output
#define io102_set_output ic74hc595_set_pin(102);ic74hc595_shift_io_pins()
#define io102_clear_output ic74hc595_clear_pin(102);ic74hc595_shift_io_pins()
#define io102_toggle_output ic74hc595_toggle_pin(102);ic74hc595_shift_io_pins()
#define io102_get_output ic74hc595_get_pin(102)
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
#if ASSERT_PIN_IO(103)
#define io103_config_output mcu_config_output(103)
#define io103_set_output mcu_set_output(103)
#define io103_clear_output mcu_clear_output(103)
#define io103_toggle_output mcu_toggle_output(103)
#define io103_get_output mcu_get_output(103)
#define io103_config_input mcu_config_input(103)
#define io103_config_pullup mcu_config_pullup(103)
#define io103_get_input mcu_get_input(103)
#elif ASSERT_PIN_EXTENDED(103)
#define io103_config_output
#define io103_set_output ic74hc595_set_pin(103);ic74hc595_shift_io_pins()
#define io103_clear_output ic74hc595_clear_pin(103);ic74hc595_shift_io_pins()
#define io103_toggle_output ic74hc595_toggle_pin(103);ic74hc595_shift_io_pins()
#define io103_get_output ic74hc595_get_pin(103)
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
#if ASSERT_PIN_IO(104)
#define io104_config_output mcu_config_output(104)
#define io104_set_output mcu_set_output(104)
#define io104_clear_output mcu_clear_output(104)
#define io104_toggle_output mcu_toggle_output(104)
#define io104_get_output mcu_get_output(104)
#define io104_config_input mcu_config_input(104)
#define io104_config_pullup mcu_config_pullup(104)
#define io104_get_input mcu_get_input(104)
#elif ASSERT_PIN_EXTENDED(104)
#define io104_config_output
#define io104_set_output ic74hc595_set_pin(104);ic74hc595_shift_io_pins()
#define io104_clear_output ic74hc595_clear_pin(104);ic74hc595_shift_io_pins()
#define io104_toggle_output ic74hc595_toggle_pin(104);ic74hc595_shift_io_pins()
#define io104_get_output ic74hc595_get_pin(104)
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
#if ASSERT_PIN_IO(105)
#define io105_config_output mcu_config_output(105)
#define io105_set_output mcu_set_output(105)
#define io105_clear_output mcu_clear_output(105)
#define io105_toggle_output mcu_toggle_output(105)
#define io105_get_output mcu_get_output(105)
#define io105_config_input mcu_config_input(105)
#define io105_config_pullup mcu_config_pullup(105)
#define io105_get_input mcu_get_input(105)
#elif ASSERT_PIN_EXTENDED(105)
#define io105_config_output
#define io105_set_output ic74hc595_set_pin(105);ic74hc595_shift_io_pins()
#define io105_clear_output ic74hc595_clear_pin(105);ic74hc595_shift_io_pins()
#define io105_toggle_output ic74hc595_toggle_pin(105);ic74hc595_shift_io_pins()
#define io105_get_output ic74hc595_get_pin(105)
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
#if ASSERT_PIN_IO(106)
#define io106_config_output mcu_config_output(106)
#define io106_set_output mcu_set_output(106)
#define io106_clear_output mcu_clear_output(106)
#define io106_toggle_output mcu_toggle_output(106)
#define io106_get_output mcu_get_output(106)
#define io106_config_input mcu_config_input(106)
#define io106_config_pullup mcu_config_pullup(106)
#define io106_get_input mcu_get_input(106)
#elif ASSERT_PIN_EXTENDED(106)
#define io106_config_output
#define io106_set_output ic74hc595_set_pin(106);ic74hc595_shift_io_pins()
#define io106_clear_output ic74hc595_clear_pin(106);ic74hc595_shift_io_pins()
#define io106_toggle_output ic74hc595_toggle_pin(106);ic74hc595_shift_io_pins()
#define io106_get_output ic74hc595_get_pin(106)
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
#if ASSERT_PIN_IO(107)
#define io107_config_output mcu_config_output(107)
#define io107_set_output mcu_set_output(107)
#define io107_clear_output mcu_clear_output(107)
#define io107_toggle_output mcu_toggle_output(107)
#define io107_get_output mcu_get_output(107)
#define io107_config_input mcu_config_input(107)
#define io107_config_pullup mcu_config_pullup(107)
#define io107_get_input mcu_get_input(107)
#elif ASSERT_PIN_EXTENDED(107)
#define io107_config_output
#define io107_set_output ic74hc595_set_pin(107);ic74hc595_shift_io_pins()
#define io107_clear_output ic74hc595_clear_pin(107);ic74hc595_shift_io_pins()
#define io107_toggle_output ic74hc595_toggle_pin(107);ic74hc595_shift_io_pins()
#define io107_get_output ic74hc595_get_pin(107)
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
#if ASSERT_PIN_IO(108)
#define io108_config_output mcu_config_output(108)
#define io108_set_output mcu_set_output(108)
#define io108_clear_output mcu_clear_output(108)
#define io108_toggle_output mcu_toggle_output(108)
#define io108_get_output mcu_get_output(108)
#define io108_config_input mcu_config_input(108)
#define io108_config_pullup mcu_config_pullup(108)
#define io108_get_input mcu_get_input(108)
#elif ASSERT_PIN_EXTENDED(108)
#define io108_config_output
#define io108_set_output ic74hc595_set_pin(108);ic74hc595_shift_io_pins()
#define io108_clear_output ic74hc595_clear_pin(108);ic74hc595_shift_io_pins()
#define io108_toggle_output ic74hc595_toggle_pin(108);ic74hc595_shift_io_pins()
#define io108_get_output ic74hc595_get_pin(108)
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
#if ASSERT_PIN_IO(109)
#define io109_config_output mcu_config_output(109)
#define io109_set_output mcu_set_output(109)
#define io109_clear_output mcu_clear_output(109)
#define io109_toggle_output mcu_toggle_output(109)
#define io109_get_output mcu_get_output(109)
#define io109_config_input mcu_config_input(109)
#define io109_config_pullup mcu_config_pullup(109)
#define io109_get_input mcu_get_input(109)
#elif ASSERT_PIN_EXTENDED(109)
#define io109_config_output
#define io109_set_output ic74hc595_set_pin(109);ic74hc595_shift_io_pins()
#define io109_clear_output ic74hc595_clear_pin(109);ic74hc595_shift_io_pins()
#define io109_toggle_output ic74hc595_toggle_pin(109);ic74hc595_shift_io_pins()
#define io109_get_output ic74hc595_get_pin(109)
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
#if ASSERT_PIN_IO(110)
#define io110_config_output mcu_config_output(110)
#define io110_set_output mcu_set_output(110)
#define io110_clear_output mcu_clear_output(110)
#define io110_toggle_output mcu_toggle_output(110)
#define io110_get_output mcu_get_output(110)
#define io110_config_input mcu_config_input(110)
#define io110_config_pullup mcu_config_pullup(110)
#define io110_get_input mcu_get_input(110)
#elif ASSERT_PIN_EXTENDED(110)
#define io110_config_output
#define io110_set_output ic74hc595_set_pin(110);ic74hc595_shift_io_pins()
#define io110_clear_output ic74hc595_clear_pin(110);ic74hc595_shift_io_pins()
#define io110_toggle_output ic74hc595_toggle_pin(110);ic74hc595_shift_io_pins()
#define io110_get_output ic74hc595_get_pin(110)
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
#if ASSERT_PIN_IO(111)
#define io111_config_output mcu_config_output(111)
#define io111_set_output mcu_set_output(111)
#define io111_clear_output mcu_clear_output(111)
#define io111_toggle_output mcu_toggle_output(111)
#define io111_get_output mcu_get_output(111)
#define io111_config_input mcu_config_input(111)
#define io111_config_pullup mcu_config_pullup(111)
#define io111_get_input mcu_get_input(111)
#elif ASSERT_PIN_EXTENDED(111)
#define io111_config_output
#define io111_set_output ic74hc595_set_pin(111);ic74hc595_shift_io_pins()
#define io111_clear_output ic74hc595_clear_pin(111);ic74hc595_shift_io_pins()
#define io111_toggle_output ic74hc595_toggle_pin(111);ic74hc595_shift_io_pins()
#define io111_get_output ic74hc595_get_pin(111)
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
#if ASSERT_PIN_IO(112)
#define io112_config_output mcu_config_output(112)
#define io112_set_output mcu_set_output(112)
#define io112_clear_output mcu_clear_output(112)
#define io112_toggle_output mcu_toggle_output(112)
#define io112_get_output mcu_get_output(112)
#define io112_config_input mcu_config_input(112)
#define io112_config_pullup mcu_config_pullup(112)
#define io112_get_input mcu_get_input(112)
#elif ASSERT_PIN_EXTENDED(112)
#define io112_config_output
#define io112_set_output ic74hc595_set_pin(112);ic74hc595_shift_io_pins()
#define io112_clear_output ic74hc595_clear_pin(112);ic74hc595_shift_io_pins()
#define io112_toggle_output ic74hc595_toggle_pin(112);ic74hc595_shift_io_pins()
#define io112_get_output ic74hc595_get_pin(112)
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
#if ASSERT_PIN_IO(113)
#define io113_config_output mcu_config_output(113)
#define io113_set_output mcu_set_output(113)
#define io113_clear_output mcu_clear_output(113)
#define io113_toggle_output mcu_toggle_output(113)
#define io113_get_output mcu_get_output(113)
#define io113_config_input mcu_config_input(113)
#define io113_config_pullup mcu_config_pullup(113)
#define io113_get_input mcu_get_input(113)
#elif ASSERT_PIN_EXTENDED(113)
#define io113_config_output
#define io113_set_output ic74hc595_set_pin(113);ic74hc595_shift_io_pins()
#define io113_clear_output ic74hc595_clear_pin(113);ic74hc595_shift_io_pins()
#define io113_toggle_output ic74hc595_toggle_pin(113);ic74hc595_shift_io_pins()
#define io113_get_output ic74hc595_get_pin(113)
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
#if ASSERT_PIN_IO(114)
#define io114_config_output mcu_config_output(114)
#define io114_set_output mcu_set_output(114)
#define io114_clear_output mcu_clear_output(114)
#define io114_toggle_output mcu_toggle_output(114)
#define io114_get_output mcu_get_output(114)
#define io114_config_input mcu_config_input(114)
#define io114_config_pullup mcu_config_pullup(114)
#define io114_get_input mcu_get_input(114)
#elif ASSERT_PIN_EXTENDED(114)
#define io114_config_output
#define io114_set_output ic74hc595_set_pin(114);ic74hc595_shift_io_pins()
#define io114_clear_output ic74hc595_clear_pin(114);ic74hc595_shift_io_pins()
#define io114_toggle_output ic74hc595_toggle_pin(114);ic74hc595_shift_io_pins()
#define io114_get_output ic74hc595_get_pin(114)
#define io114_config_input
#define io114_config_pullup
#define io114_get_input 0
#else
#define io114_config_output
#define io114_set_output
#define io114_clear_output
#define io114_toggle_output
#define io114_get_output 0
#define io114_config_input
#define io114_config_pullup
#define io114_get_input 0
#endif
#if ASSERT_PIN_IO(115)
#define io115_config_output mcu_config_output(115)
#define io115_set_output mcu_set_output(115)
#define io115_clear_output mcu_clear_output(115)
#define io115_toggle_output mcu_toggle_output(115)
#define io115_get_output mcu_get_output(115)
#define io115_config_input mcu_config_input(115)
#define io115_config_pullup mcu_config_pullup(115)
#define io115_get_input mcu_get_input(115)
#elif ASSERT_PIN_EXTENDED(115)
#define io115_config_output
#define io115_set_output ic74hc595_set_pin(115);ic74hc595_shift_io_pins()
#define io115_clear_output ic74hc595_clear_pin(115);ic74hc595_shift_io_pins()
#define io115_toggle_output ic74hc595_toggle_pin(115);ic74hc595_shift_io_pins()
#define io115_get_output ic74hc595_get_pin(115)
#define io115_config_input
#define io115_config_pullup
#define io115_get_input 0
#else
#define io115_config_output
#define io115_set_output
#define io115_clear_output
#define io115_toggle_output
#define io115_get_output 0
#define io115_config_input
#define io115_config_pullup
#define io115_get_input 0
#endif
#if ASSERT_PIN_IO(116)
#define io116_config_output mcu_config_output(116)
#define io116_set_output mcu_set_output(116)
#define io116_clear_output mcu_clear_output(116)
#define io116_toggle_output mcu_toggle_output(116)
#define io116_get_output mcu_get_output(116)
#define io116_config_input mcu_config_input(116)
#define io116_config_pullup mcu_config_pullup(116)
#define io116_get_input mcu_get_input(116)
#elif ASSERT_PIN_EXTENDED(116)
#define io116_config_output
#define io116_set_output ic74hc595_set_pin(116);ic74hc595_shift_io_pins()
#define io116_clear_output ic74hc595_clear_pin(116);ic74hc595_shift_io_pins()
#define io116_toggle_output ic74hc595_toggle_pin(116);ic74hc595_shift_io_pins()
#define io116_get_output ic74hc595_get_pin(116)
#define io116_config_input
#define io116_config_pullup
#define io116_get_input 0
#else
#define io116_config_output
#define io116_set_output
#define io116_clear_output
#define io116_toggle_output
#define io116_get_output 0
#define io116_config_input
#define io116_config_pullup
#define io116_get_input 0
#endif
#if ASSERT_PIN_IO(117)
#define io117_config_output mcu_config_output(117)
#define io117_set_output mcu_set_output(117)
#define io117_clear_output mcu_clear_output(117)
#define io117_toggle_output mcu_toggle_output(117)
#define io117_get_output mcu_get_output(117)
#define io117_config_input mcu_config_input(117)
#define io117_config_pullup mcu_config_pullup(117)
#define io117_get_input mcu_get_input(117)
#elif ASSERT_PIN_EXTENDED(117)
#define io117_config_output
#define io117_set_output ic74hc595_set_pin(117);ic74hc595_shift_io_pins()
#define io117_clear_output ic74hc595_clear_pin(117);ic74hc595_shift_io_pins()
#define io117_toggle_output ic74hc595_toggle_pin(117);ic74hc595_shift_io_pins()
#define io117_get_output ic74hc595_get_pin(117)
#define io117_config_input
#define io117_config_pullup
#define io117_get_input 0
#else
#define io117_config_output
#define io117_set_output
#define io117_clear_output
#define io117_toggle_output
#define io117_get_output 0
#define io117_config_input
#define io117_config_pullup
#define io117_get_input 0
#endif
#if ASSERT_PIN_IO(118)
#define io118_config_output mcu_config_output(118)
#define io118_set_output mcu_set_output(118)
#define io118_clear_output mcu_clear_output(118)
#define io118_toggle_output mcu_toggle_output(118)
#define io118_get_output mcu_get_output(118)
#define io118_config_input mcu_config_input(118)
#define io118_config_pullup mcu_config_pullup(118)
#define io118_get_input mcu_get_input(118)
#elif ASSERT_PIN_EXTENDED(118)
#define io118_config_output
#define io118_set_output ic74hc595_set_pin(118);ic74hc595_shift_io_pins()
#define io118_clear_output ic74hc595_clear_pin(118);ic74hc595_shift_io_pins()
#define io118_toggle_output ic74hc595_toggle_pin(118);ic74hc595_shift_io_pins()
#define io118_get_output ic74hc595_get_pin(118)
#define io118_config_input
#define io118_config_pullup
#define io118_get_input 0
#else
#define io118_config_output
#define io118_set_output
#define io118_clear_output
#define io118_toggle_output
#define io118_get_output 0
#define io118_config_input
#define io118_config_pullup
#define io118_get_input 0
#endif
#if ASSERT_PIN_IO(119)
#define io119_config_output mcu_config_output(119)
#define io119_set_output mcu_set_output(119)
#define io119_clear_output mcu_clear_output(119)
#define io119_toggle_output mcu_toggle_output(119)
#define io119_get_output mcu_get_output(119)
#define io119_config_input mcu_config_input(119)
#define io119_config_pullup mcu_config_pullup(119)
#define io119_get_input mcu_get_input(119)
#elif ASSERT_PIN_EXTENDED(119)
#define io119_config_output
#define io119_set_output ic74hc595_set_pin(119);ic74hc595_shift_io_pins()
#define io119_clear_output ic74hc595_clear_pin(119);ic74hc595_shift_io_pins()
#define io119_toggle_output ic74hc595_toggle_pin(119);ic74hc595_shift_io_pins()
#define io119_get_output ic74hc595_get_pin(119)
#define io119_config_input
#define io119_config_pullup
#define io119_get_input 0
#else
#define io119_config_output
#define io119_set_output
#define io119_clear_output
#define io119_toggle_output
#define io119_get_output 0
#define io119_config_input
#define io119_config_pullup
#define io119_get_input 0
#endif
#if ASSERT_PIN_IO(120)
#define io120_config_output mcu_config_output(120)
#define io120_set_output mcu_set_output(120)
#define io120_clear_output mcu_clear_output(120)
#define io120_toggle_output mcu_toggle_output(120)
#define io120_get_output mcu_get_output(120)
#define io120_config_input mcu_config_input(120)
#define io120_config_pullup mcu_config_pullup(120)
#define io120_get_input mcu_get_input(120)
#elif ASSERT_PIN_EXTENDED(120)
#define io120_config_output
#define io120_set_output ic74hc595_set_pin(120);ic74hc595_shift_io_pins()
#define io120_clear_output ic74hc595_clear_pin(120);ic74hc595_shift_io_pins()
#define io120_toggle_output ic74hc595_toggle_pin(120);ic74hc595_shift_io_pins()
#define io120_get_output ic74hc595_get_pin(120)
#define io120_config_input
#define io120_config_pullup
#define io120_get_input 0
#else
#define io120_config_output
#define io120_set_output
#define io120_clear_output
#define io120_toggle_output
#define io120_get_output 0
#define io120_config_input
#define io120_config_pullup
#define io120_get_input 0
#endif
#if ASSERT_PIN_IO(121)
#define io121_config_output mcu_config_output(121)
#define io121_set_output mcu_set_output(121)
#define io121_clear_output mcu_clear_output(121)
#define io121_toggle_output mcu_toggle_output(121)
#define io121_get_output mcu_get_output(121)
#define io121_config_input mcu_config_input(121)
#define io121_config_pullup mcu_config_pullup(121)
#define io121_get_input mcu_get_input(121)
#elif ASSERT_PIN_EXTENDED(121)
#define io121_config_output
#define io121_set_output ic74hc595_set_pin(121);ic74hc595_shift_io_pins()
#define io121_clear_output ic74hc595_clear_pin(121);ic74hc595_shift_io_pins()
#define io121_toggle_output ic74hc595_toggle_pin(121);ic74hc595_shift_io_pins()
#define io121_get_output ic74hc595_get_pin(121)
#define io121_config_input
#define io121_config_pullup
#define io121_get_input 0
#else
#define io121_config_output
#define io121_set_output
#define io121_clear_output
#define io121_toggle_output
#define io121_get_output 0
#define io121_config_input
#define io121_config_pullup
#define io121_get_input 0
#endif
#if ASSERT_PIN_IO(122)
#define io122_config_output mcu_config_output(122)
#define io122_set_output mcu_set_output(122)
#define io122_clear_output mcu_clear_output(122)
#define io122_toggle_output mcu_toggle_output(122)
#define io122_get_output mcu_get_output(122)
#define io122_config_input mcu_config_input(122)
#define io122_config_pullup mcu_config_pullup(122)
#define io122_get_input mcu_get_input(122)
#elif ASSERT_PIN_EXTENDED(122)
#define io122_config_output
#define io122_set_output ic74hc595_set_pin(122);ic74hc595_shift_io_pins()
#define io122_clear_output ic74hc595_clear_pin(122);ic74hc595_shift_io_pins()
#define io122_toggle_output ic74hc595_toggle_pin(122);ic74hc595_shift_io_pins()
#define io122_get_output ic74hc595_get_pin(122)
#define io122_config_input
#define io122_config_pullup
#define io122_get_input 0
#else
#define io122_config_output
#define io122_set_output
#define io122_clear_output
#define io122_toggle_output
#define io122_get_output 0
#define io122_config_input
#define io122_config_pullup
#define io122_get_input 0
#endif
#if ASSERT_PIN_IO(123)
#define io123_config_output mcu_config_output(123)
#define io123_set_output mcu_set_output(123)
#define io123_clear_output mcu_clear_output(123)
#define io123_toggle_output mcu_toggle_output(123)
#define io123_get_output mcu_get_output(123)
#define io123_config_input mcu_config_input(123)
#define io123_config_pullup mcu_config_pullup(123)
#define io123_get_input mcu_get_input(123)
#elif ASSERT_PIN_EXTENDED(123)
#define io123_config_output
#define io123_set_output ic74hc595_set_pin(123);ic74hc595_shift_io_pins()
#define io123_clear_output ic74hc595_clear_pin(123);ic74hc595_shift_io_pins()
#define io123_toggle_output ic74hc595_toggle_pin(123);ic74hc595_shift_io_pins()
#define io123_get_output ic74hc595_get_pin(123)
#define io123_config_input
#define io123_config_pullup
#define io123_get_input 0
#else
#define io123_config_output
#define io123_set_output
#define io123_clear_output
#define io123_toggle_output
#define io123_get_output 0
#define io123_config_input
#define io123_config_pullup
#define io123_get_input 0
#endif
#if ASSERT_PIN_IO(124)
#define io124_config_output mcu_config_output(124)
#define io124_set_output mcu_set_output(124)
#define io124_clear_output mcu_clear_output(124)
#define io124_toggle_output mcu_toggle_output(124)
#define io124_get_output mcu_get_output(124)
#define io124_config_input mcu_config_input(124)
#define io124_config_pullup mcu_config_pullup(124)
#define io124_get_input mcu_get_input(124)
#elif ASSERT_PIN_EXTENDED(124)
#define io124_config_output
#define io124_set_output ic74hc595_set_pin(124);ic74hc595_shift_io_pins()
#define io124_clear_output ic74hc595_clear_pin(124);ic74hc595_shift_io_pins()
#define io124_toggle_output ic74hc595_toggle_pin(124);ic74hc595_shift_io_pins()
#define io124_get_output ic74hc595_get_pin(124)
#define io124_config_input
#define io124_config_pullup
#define io124_get_input 0
#else
#define io124_config_output
#define io124_set_output
#define io124_clear_output
#define io124_toggle_output
#define io124_get_output 0
#define io124_config_input
#define io124_config_pullup
#define io124_get_input 0
#endif
#if ASSERT_PIN_IO(125)
#define io125_config_output mcu_config_output(125)
#define io125_set_output mcu_set_output(125)
#define io125_clear_output mcu_clear_output(125)
#define io125_toggle_output mcu_toggle_output(125)
#define io125_get_output mcu_get_output(125)
#define io125_config_input mcu_config_input(125)
#define io125_config_pullup mcu_config_pullup(125)
#define io125_get_input mcu_get_input(125)
#elif ASSERT_PIN_EXTENDED(125)
#define io125_config_output
#define io125_set_output ic74hc595_set_pin(125);ic74hc595_shift_io_pins()
#define io125_clear_output ic74hc595_clear_pin(125);ic74hc595_shift_io_pins()
#define io125_toggle_output ic74hc595_toggle_pin(125);ic74hc595_shift_io_pins()
#define io125_get_output ic74hc595_get_pin(125)
#define io125_config_input
#define io125_config_pullup
#define io125_get_input 0
#else
#define io125_config_output
#define io125_set_output
#define io125_clear_output
#define io125_toggle_output
#define io125_get_output 0
#define io125_config_input
#define io125_config_pullup
#define io125_get_input 0
#endif
#if ASSERT_PIN_IO(126)
#define io126_config_output mcu_config_output(126)
#define io126_set_output mcu_set_output(126)
#define io126_clear_output mcu_clear_output(126)
#define io126_toggle_output mcu_toggle_output(126)
#define io126_get_output mcu_get_output(126)
#define io126_config_input mcu_config_input(126)
#define io126_config_pullup mcu_config_pullup(126)
#define io126_get_input mcu_get_input(126)
#elif ASSERT_PIN_EXTENDED(126)
#define io126_config_output
#define io126_set_output ic74hc595_set_pin(126);ic74hc595_shift_io_pins()
#define io126_clear_output ic74hc595_clear_pin(126);ic74hc595_shift_io_pins()
#define io126_toggle_output ic74hc595_toggle_pin(126);ic74hc595_shift_io_pins()
#define io126_get_output ic74hc595_get_pin(126)
#define io126_config_input
#define io126_config_pullup
#define io126_get_input 0
#else
#define io126_config_output
#define io126_set_output
#define io126_clear_output
#define io126_toggle_output
#define io126_get_output 0
#define io126_config_input
#define io126_config_pullup
#define io126_get_input 0
#endif
#if ASSERT_PIN_IO(127)
#define io127_config_output mcu_config_output(127)
#define io127_set_output mcu_set_output(127)
#define io127_clear_output mcu_clear_output(127)
#define io127_toggle_output mcu_toggle_output(127)
#define io127_get_output mcu_get_output(127)
#define io127_config_input mcu_config_input(127)
#define io127_config_pullup mcu_config_pullup(127)
#define io127_get_input mcu_get_input(127)
#elif ASSERT_PIN_EXTENDED(127)
#define io127_config_output
#define io127_set_output ic74hc595_set_pin(127);ic74hc595_shift_io_pins()
#define io127_clear_output ic74hc595_clear_pin(127);ic74hc595_shift_io_pins()
#define io127_toggle_output ic74hc595_toggle_pin(127);ic74hc595_shift_io_pins()
#define io127_get_output ic74hc595_get_pin(127)
#define io127_config_input
#define io127_config_pullup
#define io127_get_input 0
#else
#define io127_config_output
#define io127_set_output
#define io127_clear_output
#define io127_toggle_output
#define io127_get_output 0
#define io127_config_input
#define io127_config_pullup
#define io127_get_input 0
#endif
#if ASSERT_PIN_IO(128)
#define io128_config_output mcu_config_output(128)
#define io128_set_output mcu_set_output(128)
#define io128_clear_output mcu_clear_output(128)
#define io128_toggle_output mcu_toggle_output(128)
#define io128_get_output mcu_get_output(128)
#define io128_config_input mcu_config_input(128)
#define io128_config_pullup mcu_config_pullup(128)
#define io128_get_input mcu_get_input(128)
#elif ASSERT_PIN_EXTENDED(128)
#define io128_config_output
#define io128_set_output ic74hc595_set_pin(128);ic74hc595_shift_io_pins()
#define io128_clear_output ic74hc595_clear_pin(128);ic74hc595_shift_io_pins()
#define io128_toggle_output ic74hc595_toggle_pin(128);ic74hc595_shift_io_pins()
#define io128_get_output ic74hc595_get_pin(128)
#define io128_config_input
#define io128_config_pullup
#define io128_get_input 0
#else
#define io128_config_output
#define io128_set_output
#define io128_clear_output
#define io128_toggle_output
#define io128_get_output 0
#define io128_config_input
#define io128_config_pullup
#define io128_get_input 0
#endif
#if ASSERT_PIN_IO(129)
#define io129_config_output mcu_config_output(129)
#define io129_set_output mcu_set_output(129)
#define io129_clear_output mcu_clear_output(129)
#define io129_toggle_output mcu_toggle_output(129)
#define io129_get_output mcu_get_output(129)
#define io129_config_input mcu_config_input(129)
#define io129_config_pullup mcu_config_pullup(129)
#define io129_get_input mcu_get_input(129)
#elif ASSERT_PIN_EXTENDED(129)
#define io129_config_output
#define io129_set_output ic74hc595_set_pin(129);ic74hc595_shift_io_pins()
#define io129_clear_output ic74hc595_clear_pin(129);ic74hc595_shift_io_pins()
#define io129_toggle_output ic74hc595_toggle_pin(129);ic74hc595_shift_io_pins()
#define io129_get_output ic74hc595_get_pin(129)
#define io129_config_input
#define io129_config_pullup
#define io129_get_input 0
#else
#define io129_config_output
#define io129_set_output
#define io129_clear_output
#define io129_toggle_output
#define io129_get_output 0
#define io129_config_input
#define io129_config_pullup
#define io129_get_input 0
#endif
#if ASSERT_PIN_IO(130)
#define io130_config_output mcu_config_output(130)
#define io130_set_output mcu_set_output(130)
#define io130_clear_output mcu_clear_output(130)
#define io130_toggle_output mcu_toggle_output(130)
#define io130_get_output mcu_get_output(130)
#define io130_config_input mcu_config_input(130)
#define io130_config_pullup mcu_config_pullup(130)
#define io130_get_input mcu_get_input(130)
#elif ASSERT_PIN_EXTENDED(130)
#define io130_config_output
#define io130_set_output ic74hc595_set_pin(130);ic74hc595_shift_io_pins()
#define io130_clear_output ic74hc595_clear_pin(130);ic74hc595_shift_io_pins()
#define io130_toggle_output ic74hc595_toggle_pin(130);ic74hc595_shift_io_pins()
#define io130_get_output ic74hc595_get_pin(130)
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
#if ASSERT_PIN_IO(131)
#define io131_config_output mcu_config_output(131)
#define io131_set_output mcu_set_output(131)
#define io131_clear_output mcu_clear_output(131)
#define io131_toggle_output mcu_toggle_output(131)
#define io131_get_output mcu_get_output(131)
#define io131_config_input mcu_config_input(131)
#define io131_config_pullup mcu_config_pullup(131)
#define io131_get_input mcu_get_input(131)
#elif ASSERT_PIN_EXTENDED(131)
#define io131_config_output
#define io131_set_output ic74hc595_set_pin(131);ic74hc595_shift_io_pins()
#define io131_clear_output ic74hc595_clear_pin(131);ic74hc595_shift_io_pins()
#define io131_toggle_output ic74hc595_toggle_pin(131);ic74hc595_shift_io_pins()
#define io131_get_output ic74hc595_get_pin(131)
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
#if ASSERT_PIN_IO(132)
#define io132_config_output mcu_config_output(132)
#define io132_set_output mcu_set_output(132)
#define io132_clear_output mcu_clear_output(132)
#define io132_toggle_output mcu_toggle_output(132)
#define io132_get_output mcu_get_output(132)
#define io132_config_input mcu_config_input(132)
#define io132_config_pullup mcu_config_pullup(132)
#define io132_get_input mcu_get_input(132)
#elif ASSERT_PIN_EXTENDED(132)
#define io132_config_output
#define io132_set_output ic74hc595_set_pin(132);ic74hc595_shift_io_pins()
#define io132_clear_output ic74hc595_clear_pin(132);ic74hc595_shift_io_pins()
#define io132_toggle_output ic74hc595_toggle_pin(132);ic74hc595_shift_io_pins()
#define io132_get_output ic74hc595_get_pin(132)
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
#if ASSERT_PIN_IO(133)
#define io133_config_output mcu_config_output(133)
#define io133_set_output mcu_set_output(133)
#define io133_clear_output mcu_clear_output(133)
#define io133_toggle_output mcu_toggle_output(133)
#define io133_get_output mcu_get_output(133)
#define io133_config_input mcu_config_input(133)
#define io133_config_pullup mcu_config_pullup(133)
#define io133_get_input mcu_get_input(133)
#elif ASSERT_PIN_EXTENDED(133)
#define io133_config_output
#define io133_set_output ic74hc595_set_pin(133);ic74hc595_shift_io_pins()
#define io133_clear_output ic74hc595_clear_pin(133);ic74hc595_shift_io_pins()
#define io133_toggle_output ic74hc595_toggle_pin(133);ic74hc595_shift_io_pins()
#define io133_get_output ic74hc595_get_pin(133)
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
#if ASSERT_PIN_IO(134)
#define io134_config_output mcu_config_output(134)
#define io134_set_output mcu_set_output(134)
#define io134_clear_output mcu_clear_output(134)
#define io134_toggle_output mcu_toggle_output(134)
#define io134_get_output mcu_get_output(134)
#define io134_config_input mcu_config_input(134)
#define io134_config_pullup mcu_config_pullup(134)
#define io134_get_input mcu_get_input(134)
#elif ASSERT_PIN_EXTENDED(134)
#define io134_config_output
#define io134_set_output ic74hc595_set_pin(134);ic74hc595_shift_io_pins()
#define io134_clear_output ic74hc595_clear_pin(134);ic74hc595_shift_io_pins()
#define io134_toggle_output ic74hc595_toggle_pin(134);ic74hc595_shift_io_pins()
#define io134_get_output ic74hc595_get_pin(134)
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
#if ASSERT_PIN_IO(135)
#define io135_config_output mcu_config_output(135)
#define io135_set_output mcu_set_output(135)
#define io135_clear_output mcu_clear_output(135)
#define io135_toggle_output mcu_toggle_output(135)
#define io135_get_output mcu_get_output(135)
#define io135_config_input mcu_config_input(135)
#define io135_config_pullup mcu_config_pullup(135)
#define io135_get_input mcu_get_input(135)
#elif ASSERT_PIN_EXTENDED(135)
#define io135_config_output
#define io135_set_output ic74hc595_set_pin(135);ic74hc595_shift_io_pins()
#define io135_clear_output ic74hc595_clear_pin(135);ic74hc595_shift_io_pins()
#define io135_toggle_output ic74hc595_toggle_pin(135);ic74hc595_shift_io_pins()
#define io135_get_output ic74hc595_get_pin(135)
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
#if ASSERT_PIN_IO(136)
#define io136_config_output mcu_config_output(136)
#define io136_set_output mcu_set_output(136)
#define io136_clear_output mcu_clear_output(136)
#define io136_toggle_output mcu_toggle_output(136)
#define io136_get_output mcu_get_output(136)
#define io136_config_input mcu_config_input(136)
#define io136_config_pullup mcu_config_pullup(136)
#define io136_get_input mcu_get_input(136)
#elif ASSERT_PIN_EXTENDED(136)
#define io136_config_output
#define io136_set_output ic74hc595_set_pin(136);ic74hc595_shift_io_pins()
#define io136_clear_output ic74hc595_clear_pin(136);ic74hc595_shift_io_pins()
#define io136_toggle_output ic74hc595_toggle_pin(136);ic74hc595_shift_io_pins()
#define io136_get_output ic74hc595_get_pin(136)
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
#if ASSERT_PIN_IO(137)
#define io137_config_output mcu_config_output(137)
#define io137_set_output mcu_set_output(137)
#define io137_clear_output mcu_clear_output(137)
#define io137_toggle_output mcu_toggle_output(137)
#define io137_get_output mcu_get_output(137)
#define io137_config_input mcu_config_input(137)
#define io137_config_pullup mcu_config_pullup(137)
#define io137_get_input mcu_get_input(137)
#elif ASSERT_PIN_EXTENDED(137)
#define io137_config_output
#define io137_set_output ic74hc595_set_pin(137);ic74hc595_shift_io_pins()
#define io137_clear_output ic74hc595_clear_pin(137);ic74hc595_shift_io_pins()
#define io137_toggle_output ic74hc595_toggle_pin(137);ic74hc595_shift_io_pins()
#define io137_get_output ic74hc595_get_pin(137)
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
#if ASSERT_PIN_IO(138)
#define io138_config_output mcu_config_output(138)
#define io138_set_output mcu_set_output(138)
#define io138_clear_output mcu_clear_output(138)
#define io138_toggle_output mcu_toggle_output(138)
#define io138_get_output mcu_get_output(138)
#define io138_config_input mcu_config_input(138)
#define io138_config_pullup mcu_config_pullup(138)
#define io138_get_input mcu_get_input(138)
#elif ASSERT_PIN_EXTENDED(138)
#define io138_config_output
#define io138_set_output ic74hc595_set_pin(138);ic74hc595_shift_io_pins()
#define io138_clear_output ic74hc595_clear_pin(138);ic74hc595_shift_io_pins()
#define io138_toggle_output ic74hc595_toggle_pin(138);ic74hc595_shift_io_pins()
#define io138_get_output ic74hc595_get_pin(138)
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
#if ASSERT_PIN_IO(139)
#define io139_config_output mcu_config_output(139)
#define io139_set_output mcu_set_output(139)
#define io139_clear_output mcu_clear_output(139)
#define io139_toggle_output mcu_toggle_output(139)
#define io139_get_output mcu_get_output(139)
#define io139_config_input mcu_config_input(139)
#define io139_config_pullup mcu_config_pullup(139)
#define io139_get_input mcu_get_input(139)
#elif ASSERT_PIN_EXTENDED(139)
#define io139_config_output
#define io139_set_output ic74hc595_set_pin(139);ic74hc595_shift_io_pins()
#define io139_clear_output ic74hc595_clear_pin(139);ic74hc595_shift_io_pins()
#define io139_toggle_output ic74hc595_toggle_pin(139);ic74hc595_shift_io_pins()
#define io139_get_output ic74hc595_get_pin(139)
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
#if ASSERT_PIN_IO(140)
#define io140_config_output mcu_config_output(140)
#define io140_set_output mcu_set_output(140)
#define io140_clear_output mcu_clear_output(140)
#define io140_toggle_output mcu_toggle_output(140)
#define io140_get_output mcu_get_output(140)
#define io140_config_input mcu_config_input(140)
#define io140_config_pullup mcu_config_pullup(140)
#define io140_get_input mcu_get_input(140)
#elif ASSERT_PIN_EXTENDED(140)
#define io140_config_output
#define io140_set_output ic74hc595_set_pin(140);ic74hc595_shift_io_pins()
#define io140_clear_output ic74hc595_clear_pin(140);ic74hc595_shift_io_pins()
#define io140_toggle_output ic74hc595_toggle_pin(140);ic74hc595_shift_io_pins()
#define io140_get_output ic74hc595_get_pin(140)
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
#if ASSERT_PIN_IO(141)
#define io141_config_output mcu_config_output(141)
#define io141_set_output mcu_set_output(141)
#define io141_clear_output mcu_clear_output(141)
#define io141_toggle_output mcu_toggle_output(141)
#define io141_get_output mcu_get_output(141)
#define io141_config_input mcu_config_input(141)
#define io141_config_pullup mcu_config_pullup(141)
#define io141_get_input mcu_get_input(141)
#elif ASSERT_PIN_EXTENDED(141)
#define io141_config_output
#define io141_set_output ic74hc595_set_pin(141);ic74hc595_shift_io_pins()
#define io141_clear_output ic74hc595_clear_pin(141);ic74hc595_shift_io_pins()
#define io141_toggle_output ic74hc595_toggle_pin(141);ic74hc595_shift_io_pins()
#define io141_get_output ic74hc595_get_pin(141)
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
#if ASSERT_PIN_IO(142)
#define io142_config_output mcu_config_output(142)
#define io142_set_output mcu_set_output(142)
#define io142_clear_output mcu_clear_output(142)
#define io142_toggle_output mcu_toggle_output(142)
#define io142_get_output mcu_get_output(142)
#define io142_config_input mcu_config_input(142)
#define io142_config_pullup mcu_config_pullup(142)
#define io142_get_input mcu_get_input(142)
#elif ASSERT_PIN_EXTENDED(142)
#define io142_config_output
#define io142_set_output ic74hc595_set_pin(142);ic74hc595_shift_io_pins()
#define io142_clear_output ic74hc595_clear_pin(142);ic74hc595_shift_io_pins()
#define io142_toggle_output ic74hc595_toggle_pin(142);ic74hc595_shift_io_pins()
#define io142_get_output ic74hc595_get_pin(142)
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
#if ASSERT_PIN_IO(143)
#define io143_config_output mcu_config_output(143)
#define io143_set_output mcu_set_output(143)
#define io143_clear_output mcu_clear_output(143)
#define io143_toggle_output mcu_toggle_output(143)
#define io143_get_output mcu_get_output(143)
#define io143_config_input mcu_config_input(143)
#define io143_config_pullup mcu_config_pullup(143)
#define io143_get_input mcu_get_input(143)
#elif ASSERT_PIN_EXTENDED(143)
#define io143_config_output
#define io143_set_output ic74hc595_set_pin(143);ic74hc595_shift_io_pins()
#define io143_clear_output ic74hc595_clear_pin(143);ic74hc595_shift_io_pins()
#define io143_toggle_output ic74hc595_toggle_pin(143);ic74hc595_shift_io_pins()
#define io143_get_output ic74hc595_get_pin(143)
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
#if ASSERT_PIN_IO(144)
#define io144_config_output mcu_config_output(144)
#define io144_set_output mcu_set_output(144)
#define io144_clear_output mcu_clear_output(144)
#define io144_toggle_output mcu_toggle_output(144)
#define io144_get_output mcu_get_output(144)
#define io144_config_input mcu_config_input(144)
#define io144_config_pullup mcu_config_pullup(144)
#define io144_get_input mcu_get_input(144)
#elif ASSERT_PIN_EXTENDED(144)
#define io144_config_output
#define io144_set_output ic74hc595_set_pin(144);ic74hc595_shift_io_pins()
#define io144_clear_output ic74hc595_clear_pin(144);ic74hc595_shift_io_pins()
#define io144_toggle_output ic74hc595_toggle_pin(144);ic74hc595_shift_io_pins()
#define io144_get_output ic74hc595_get_pin(144)
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
#if ASSERT_PIN_IO(145)
#define io145_config_output mcu_config_output(145)
#define io145_set_output mcu_set_output(145)
#define io145_clear_output mcu_clear_output(145)
#define io145_toggle_output mcu_toggle_output(145)
#define io145_get_output mcu_get_output(145)
#define io145_config_input mcu_config_input(145)
#define io145_config_pullup mcu_config_pullup(145)
#define io145_get_input mcu_get_input(145)
#elif ASSERT_PIN_EXTENDED(145)
#define io145_config_output
#define io145_set_output ic74hc595_set_pin(145);ic74hc595_shift_io_pins()
#define io145_clear_output ic74hc595_clear_pin(145);ic74hc595_shift_io_pins()
#define io145_toggle_output ic74hc595_toggle_pin(145);ic74hc595_shift_io_pins()
#define io145_get_output ic74hc595_get_pin(145)
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
#if ASSERT_PIN_IO(146)
#define io146_config_output mcu_config_output(146)
#define io146_set_output mcu_set_output(146)
#define io146_clear_output mcu_clear_output(146)
#define io146_toggle_output mcu_toggle_output(146)
#define io146_get_output mcu_get_output(146)
#define io146_config_input mcu_config_input(146)
#define io146_config_pullup mcu_config_pullup(146)
#define io146_get_input mcu_get_input(146)
#elif ASSERT_PIN_EXTENDED(146)
#define io146_config_output
#define io146_set_output ic74hc595_set_pin(146);ic74hc595_shift_io_pins()
#define io146_clear_output ic74hc595_clear_pin(146);ic74hc595_shift_io_pins()
#define io146_toggle_output ic74hc595_toggle_pin(146);ic74hc595_shift_io_pins()
#define io146_get_output ic74hc595_get_pin(146)
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
#if ASSERT_PIN_IO(147)
#define io147_config_output mcu_config_output(147)
#define io147_set_output mcu_set_output(147)
#define io147_clear_output mcu_clear_output(147)
#define io147_toggle_output mcu_toggle_output(147)
#define io147_get_output mcu_get_output(147)
#define io147_config_input mcu_config_input(147)
#define io147_config_pullup mcu_config_pullup(147)
#define io147_get_input mcu_get_input(147)
#elif ASSERT_PIN_EXTENDED(147)
#define io147_config_output
#define io147_set_output ic74hc595_set_pin(147);ic74hc595_shift_io_pins()
#define io147_clear_output ic74hc595_clear_pin(147);ic74hc595_shift_io_pins()
#define io147_toggle_output ic74hc595_toggle_pin(147);ic74hc595_shift_io_pins()
#define io147_get_output ic74hc595_get_pin(147)
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
#if ASSERT_PIN_IO(148)
#define io148_config_output mcu_config_output(148)
#define io148_set_output mcu_set_output(148)
#define io148_clear_output mcu_clear_output(148)
#define io148_toggle_output mcu_toggle_output(148)
#define io148_get_output mcu_get_output(148)
#define io148_config_input mcu_config_input(148)
#define io148_config_pullup mcu_config_pullup(148)
#define io148_get_input mcu_get_input(148)
#elif ASSERT_PIN_EXTENDED(148)
#define io148_config_output
#define io148_set_output ic74hc595_set_pin(148);ic74hc595_shift_io_pins()
#define io148_clear_output ic74hc595_clear_pin(148);ic74hc595_shift_io_pins()
#define io148_toggle_output ic74hc595_toggle_pin(148);ic74hc595_shift_io_pins()
#define io148_get_output ic74hc595_get_pin(148)
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
#if ASSERT_PIN_IO(149)
#define io149_config_output mcu_config_output(149)
#define io149_set_output mcu_set_output(149)
#define io149_clear_output mcu_clear_output(149)
#define io149_toggle_output mcu_toggle_output(149)
#define io149_get_output mcu_get_output(149)
#define io149_config_input mcu_config_input(149)
#define io149_config_pullup mcu_config_pullup(149)
#define io149_get_input mcu_get_input(149)
#elif ASSERT_PIN_EXTENDED(149)
#define io149_config_output
#define io149_set_output ic74hc595_set_pin(149);ic74hc595_shift_io_pins()
#define io149_clear_output ic74hc595_clear_pin(149);ic74hc595_shift_io_pins()
#define io149_toggle_output ic74hc595_toggle_pin(149);ic74hc595_shift_io_pins()
#define io149_get_output ic74hc595_get_pin(149)
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
#if ASSERT_PIN_IO(150)
#define io150_config_output mcu_config_output(150)
#define io150_set_output mcu_set_output(150)
#define io150_clear_output mcu_clear_output(150)
#define io150_toggle_output mcu_toggle_output(150)
#define io150_get_output mcu_get_output(150)
#define io150_config_input mcu_config_input(150)
#define io150_config_pullup mcu_config_pullup(150)
#define io150_get_input mcu_get_input(150)
#elif ASSERT_PIN_EXTENDED(150)
#define io150_config_output
#define io150_set_output ic74hc595_set_pin(150);ic74hc595_shift_io_pins()
#define io150_clear_output ic74hc595_clear_pin(150);ic74hc595_shift_io_pins()
#define io150_toggle_output ic74hc595_toggle_pin(150);ic74hc595_shift_io_pins()
#define io150_get_output ic74hc595_get_pin(150)
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
#if ASSERT_PIN_IO(151)
#define io151_config_output mcu_config_output(151)
#define io151_set_output mcu_set_output(151)
#define io151_clear_output mcu_clear_output(151)
#define io151_toggle_output mcu_toggle_output(151)
#define io151_get_output mcu_get_output(151)
#define io151_config_input mcu_config_input(151)
#define io151_config_pullup mcu_config_pullup(151)
#define io151_get_input mcu_get_input(151)
#elif ASSERT_PIN_EXTENDED(151)
#define io151_config_output
#define io151_set_output ic74hc595_set_pin(151);ic74hc595_shift_io_pins()
#define io151_clear_output ic74hc595_clear_pin(151);ic74hc595_shift_io_pins()
#define io151_toggle_output ic74hc595_toggle_pin(151);ic74hc595_shift_io_pins()
#define io151_get_output ic74hc595_get_pin(151)
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
#if ASSERT_PIN_IO(152)
#define io152_config_output mcu_config_output(152)
#define io152_set_output mcu_set_output(152)
#define io152_clear_output mcu_clear_output(152)
#define io152_toggle_output mcu_toggle_output(152)
#define io152_get_output mcu_get_output(152)
#define io152_config_input mcu_config_input(152)
#define io152_config_pullup mcu_config_pullup(152)
#define io152_get_input mcu_get_input(152)
#elif ASSERT_PIN_EXTENDED(152)
#define io152_config_output
#define io152_set_output ic74hc595_set_pin(152);ic74hc595_shift_io_pins()
#define io152_clear_output ic74hc595_clear_pin(152);ic74hc595_shift_io_pins()
#define io152_toggle_output ic74hc595_toggle_pin(152);ic74hc595_shift_io_pins()
#define io152_get_output ic74hc595_get_pin(152)
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
#if ASSERT_PIN_IO(153)
#define io153_config_output mcu_config_output(153)
#define io153_set_output mcu_set_output(153)
#define io153_clear_output mcu_clear_output(153)
#define io153_toggle_output mcu_toggle_output(153)
#define io153_get_output mcu_get_output(153)
#define io153_config_input mcu_config_input(153)
#define io153_config_pullup mcu_config_pullup(153)
#define io153_get_input mcu_get_input(153)
#elif ASSERT_PIN_EXTENDED(153)
#define io153_config_output
#define io153_set_output ic74hc595_set_pin(153);ic74hc595_shift_io_pins()
#define io153_clear_output ic74hc595_clear_pin(153);ic74hc595_shift_io_pins()
#define io153_toggle_output ic74hc595_toggle_pin(153);ic74hc595_shift_io_pins()
#define io153_get_output ic74hc595_get_pin(153)
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
#if ASSERT_PIN_IO(154)
#define io154_config_output mcu_config_output(154)
#define io154_set_output mcu_set_output(154)
#define io154_clear_output mcu_clear_output(154)
#define io154_toggle_output mcu_toggle_output(154)
#define io154_get_output mcu_get_output(154)
#define io154_config_input mcu_config_input(154)
#define io154_config_pullup mcu_config_pullup(154)
#define io154_get_input mcu_get_input(154)
#elif ASSERT_PIN_EXTENDED(154)
#define io154_config_output
#define io154_set_output ic74hc595_set_pin(154);ic74hc595_shift_io_pins()
#define io154_clear_output ic74hc595_clear_pin(154);ic74hc595_shift_io_pins()
#define io154_toggle_output ic74hc595_toggle_pin(154);ic74hc595_shift_io_pins()
#define io154_get_output ic74hc595_get_pin(154)
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
#if ASSERT_PIN_IO(155)
#define io155_config_output mcu_config_output(155)
#define io155_set_output mcu_set_output(155)
#define io155_clear_output mcu_clear_output(155)
#define io155_toggle_output mcu_toggle_output(155)
#define io155_get_output mcu_get_output(155)
#define io155_config_input mcu_config_input(155)
#define io155_config_pullup mcu_config_pullup(155)
#define io155_get_input mcu_get_input(155)
#elif ASSERT_PIN_EXTENDED(155)
#define io155_config_output
#define io155_set_output ic74hc595_set_pin(155);ic74hc595_shift_io_pins()
#define io155_clear_output ic74hc595_clear_pin(155);ic74hc595_shift_io_pins()
#define io155_toggle_output ic74hc595_toggle_pin(155);ic74hc595_shift_io_pins()
#define io155_get_output ic74hc595_get_pin(155)
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
#if ASSERT_PIN_IO(156)
#define io156_config_output mcu_config_output(156)
#define io156_set_output mcu_set_output(156)
#define io156_clear_output mcu_clear_output(156)
#define io156_toggle_output mcu_toggle_output(156)
#define io156_get_output mcu_get_output(156)
#define io156_config_input mcu_config_input(156)
#define io156_config_pullup mcu_config_pullup(156)
#define io156_get_input mcu_get_input(156)
#elif ASSERT_PIN_EXTENDED(156)
#define io156_config_output
#define io156_set_output ic74hc595_set_pin(156);ic74hc595_shift_io_pins()
#define io156_clear_output ic74hc595_clear_pin(156);ic74hc595_shift_io_pins()
#define io156_toggle_output ic74hc595_toggle_pin(156);ic74hc595_shift_io_pins()
#define io156_get_output ic74hc595_get_pin(156)
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
#if ASSERT_PIN_IO(157)
#define io157_config_output mcu_config_output(157)
#define io157_set_output mcu_set_output(157)
#define io157_clear_output mcu_clear_output(157)
#define io157_toggle_output mcu_toggle_output(157)
#define io157_get_output mcu_get_output(157)
#define io157_config_input mcu_config_input(157)
#define io157_config_pullup mcu_config_pullup(157)
#define io157_get_input mcu_get_input(157)
#elif ASSERT_PIN_EXTENDED(157)
#define io157_config_output
#define io157_set_output ic74hc595_set_pin(157);ic74hc595_shift_io_pins()
#define io157_clear_output ic74hc595_clear_pin(157);ic74hc595_shift_io_pins()
#define io157_toggle_output ic74hc595_toggle_pin(157);ic74hc595_shift_io_pins()
#define io157_get_output ic74hc595_get_pin(157)
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
#if ASSERT_PIN_IO(158)
#define io158_config_output mcu_config_output(158)
#define io158_set_output mcu_set_output(158)
#define io158_clear_output mcu_clear_output(158)
#define io158_toggle_output mcu_toggle_output(158)
#define io158_get_output mcu_get_output(158)
#define io158_config_input mcu_config_input(158)
#define io158_config_pullup mcu_config_pullup(158)
#define io158_get_input mcu_get_input(158)
#elif ASSERT_PIN_EXTENDED(158)
#define io158_config_output
#define io158_set_output ic74hc595_set_pin(158);ic74hc595_shift_io_pins()
#define io158_clear_output ic74hc595_clear_pin(158);ic74hc595_shift_io_pins()
#define io158_toggle_output ic74hc595_toggle_pin(158);ic74hc595_shift_io_pins()
#define io158_get_output ic74hc595_get_pin(158)
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
#if ASSERT_PIN_IO(159)
#define io159_config_output mcu_config_output(159)
#define io159_set_output mcu_set_output(159)
#define io159_clear_output mcu_clear_output(159)
#define io159_toggle_output mcu_toggle_output(159)
#define io159_get_output mcu_get_output(159)
#define io159_config_input mcu_config_input(159)
#define io159_config_pullup mcu_config_pullup(159)
#define io159_get_input mcu_get_input(159)
#elif ASSERT_PIN_EXTENDED(159)
#define io159_config_output
#define io159_set_output ic74hc595_set_pin(159);ic74hc595_shift_io_pins()
#define io159_clear_output ic74hc595_clear_pin(159);ic74hc595_shift_io_pins()
#define io159_toggle_output ic74hc595_toggle_pin(159);ic74hc595_shift_io_pins()
#define io159_get_output ic74hc595_get_pin(159)
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
#if ASSERT_PIN_IO(160)
#define io160_config_output mcu_config_output(160)
#define io160_set_output mcu_set_output(160)
#define io160_clear_output mcu_clear_output(160)
#define io160_toggle_output mcu_toggle_output(160)
#define io160_get_output mcu_get_output(160)
#define io160_config_input mcu_config_input(160)
#define io160_config_pullup mcu_config_pullup(160)
#define io160_get_input mcu_get_input(160)
#elif ASSERT_PIN_EXTENDED(160)
#define io160_config_output
#define io160_set_output ic74hc595_set_pin(160);ic74hc595_shift_io_pins()
#define io160_clear_output ic74hc595_clear_pin(160);ic74hc595_shift_io_pins()
#define io160_toggle_output ic74hc595_toggle_pin(160);ic74hc595_shift_io_pins()
#define io160_get_output ic74hc595_get_pin(160)
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
#if ASSERT_PIN_IO(161)
#define io161_config_output mcu_config_output(161)
#define io161_set_output mcu_set_output(161)
#define io161_clear_output mcu_clear_output(161)
#define io161_toggle_output mcu_toggle_output(161)
#define io161_get_output mcu_get_output(161)
#define io161_config_input mcu_config_input(161)
#define io161_config_pullup mcu_config_pullup(161)
#define io161_get_input mcu_get_input(161)
#elif ASSERT_PIN_EXTENDED(161)
#define io161_config_output
#define io161_set_output ic74hc595_set_pin(161);ic74hc595_shift_io_pins()
#define io161_clear_output ic74hc595_clear_pin(161);ic74hc595_shift_io_pins()
#define io161_toggle_output ic74hc595_toggle_pin(161);ic74hc595_shift_io_pins()
#define io161_get_output ic74hc595_get_pin(161)
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

/*PWM*/
#if ASSERT_PIN_IO(PWM0)
#define io25_config_pwm(freq) mcu_config_pwm(PWM0, freq)
#define io25_set_pwm(value) mcu_set_pwm(PWM0, value)
#define io25_get_pwm mcu_get_pwm(PWM0)
#elif ASSERT_PIN_EXTENDED(PWM0)
#define io25_config_pwm(freq) {g_soft_pwm_res = mcu_softpwm_freq_config(freq);}
#define io25_set_pwm(value) io_set_soft_pwm(PWM0, value)
#define io25_get_pwm io_get_soft_pwm(PWM0)
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
#define io26_config_pwm(freq) {g_soft_pwm_res = mcu_softpwm_freq_config(freq);}
#define io26_set_pwm(value) io_set_soft_pwm(PWM1, value)
#define io26_get_pwm io_get_soft_pwm(PWM1)
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
#define io27_config_pwm(freq) {g_soft_pwm_res = mcu_softpwm_freq_config(freq);}
#define io27_set_pwm(value) io_set_soft_pwm(PWM2, value)
#define io27_get_pwm io_get_soft_pwm(PWM2)
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
#define io28_config_pwm(freq) {g_soft_pwm_res = mcu_softpwm_freq_config(freq);}
#define io28_set_pwm(value) io_set_soft_pwm(PWM3, value)
#define io28_get_pwm io_get_soft_pwm(PWM3)
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
#define io29_config_pwm(freq) {g_soft_pwm_res = mcu_softpwm_freq_config(freq);}
#define io29_set_pwm(value) io_set_soft_pwm(PWM4, value)
#define io29_get_pwm io_get_soft_pwm(PWM4)
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
#define io30_config_pwm(freq) {g_soft_pwm_res = mcu_softpwm_freq_config(freq);}
#define io30_set_pwm(value) io_set_soft_pwm(PWM5, value)
#define io30_get_pwm io_get_soft_pwm(PWM5)
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
#define io31_config_pwm(freq) {g_soft_pwm_res = mcu_softpwm_freq_config(freq);}
#define io31_set_pwm(value) io_set_soft_pwm(PWM6, value)
#define io31_get_pwm io_get_soft_pwm(PWM6)
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
#define io32_config_pwm(freq) {g_soft_pwm_res = mcu_softpwm_freq_config(freq);}
#define io32_set_pwm(value) io_set_soft_pwm(PWM7, value)
#define io32_get_pwm io_get_soft_pwm(PWM7)
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
#define io33_config_pwm(freq) {g_soft_pwm_res = mcu_softpwm_freq_config(freq);}
#define io33_set_pwm(value) io_set_soft_pwm(PWM8, value)
#define io33_get_pwm io_get_soft_pwm(PWM8)
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
#define io34_config_pwm(freq) {g_soft_pwm_res = mcu_softpwm_freq_config(freq);}
#define io34_set_pwm(value) io_set_soft_pwm(PWM9, value)
#define io34_get_pwm io_get_soft_pwm(PWM9)
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
#define io35_config_pwm(freq) {g_soft_pwm_res = mcu_softpwm_freq_config(freq);}
#define io35_set_pwm(value) io_set_soft_pwm(PWM10, value)
#define io35_get_pwm io_get_soft_pwm(PWM10)
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
#define io36_config_pwm(freq) {g_soft_pwm_res = mcu_softpwm_freq_config(freq);}
#define io36_set_pwm(value) io_set_soft_pwm(PWM11, value)
#define io36_get_pwm io_get_soft_pwm(PWM11)
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
#define io37_config_pwm(freq) {g_soft_pwm_res = mcu_softpwm_freq_config(freq);}
#define io37_set_pwm(value) io_set_soft_pwm(PWM12, value)
#define io37_get_pwm io_get_soft_pwm(PWM12)
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
#define io38_config_pwm(freq) {g_soft_pwm_res = mcu_softpwm_freq_config(freq);}
#define io38_set_pwm(value) io_set_soft_pwm(PWM13, value)
#define io38_get_pwm io_get_soft_pwm(PWM13)
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
#define io39_config_pwm(freq) {g_soft_pwm_res = mcu_softpwm_freq_config(freq);}
#define io39_set_pwm(value) io_set_soft_pwm(PWM14, value)
#define io39_get_pwm io_get_soft_pwm(PWM14)
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
#define io40_config_pwm(freq) {g_soft_pwm_res = mcu_softpwm_freq_config(freq);}
#define io40_set_pwm(value) io_set_soft_pwm(PWM15, value)
#define io40_get_pwm io_get_soft_pwm(PWM15)
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
#define io_hal_config_output(pin) _io_hal_config_output_(pin)
#define _io_hal_set_output_(pin) io##pin##_set_output
#define io_hal_set_output(pin) _io_hal_set_output_(pin)
#define _io_hal_clear_output_(pin) io##pin##_clear_output
#define io_hal_clear_output(pin) _io_hal_clear_output_(pin)
#define _io_hal_toggle_output_(pin) io##pin##_toggle_output
#define io_hal_toggle_output(pin) _io_hal_toggle_output_(pin)
#define _io_hal_get_output_(pin) io##pin##_get_output
#define io_hal_get_output(pin) _io_hal_get_output_(pin)

/*input HAL*/
#define _io_hal_config_input_(pin) io##pin##_config_input
#define io_hal_config_input(pin) _io_hal_config_input_(pin)
#define _io_hal_config_pullup_(pin) io##pin##_config_pullup
#define io_hal_config_pullup(pin) _io_hal_config_pullup_(pin)
#define _io_hal_get_input_(pin) io##pin##_get_input
#define io_hal_get_input(pin) _io_hal_get_input_(pin)

/*pwm and servo HAL*/
#define _io_hal_config_pwm_(pin,freq) io##pin##_config_pwm(freq)
#define io_hal_config_pwm(pin,freq) _io_hal_config_pwm_(pin,freq)
#define _io_hal_set_pwm_(pin,value) io##pin##_set_pwm(value)
#define io_hal_set_pwm(pin,value) _io_hal_set_pwm_(pin,value)
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
