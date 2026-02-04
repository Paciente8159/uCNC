#include "../../../cnc.h"

#ifdef ESP32
extern volatile uint32_t i2s_mode;
#define I2S_MODE __atomic_load_n((uint32_t *)&i2s_mode, __ATOMIC_RELAXED)

void __attribute__((weak)) mcu_uart_start(void) {}
void __attribute__((weak)) mcu_uart_dotasks(void) {}
void __attribute__((weak)) mcu_uart2_start(void) {}
void __attribute__((weak)) mcu_uart2_dotasks(void) {}
void __attribute__((weak)) mcu_eeprom_init(int size) { (void)size; }

void __attribute__((weak)) mcu_usb_dotasks(void) {}
void __attribute__((weak)) mcu_wifi_init(void) {}
void __attribute__((weak)) mcu_wifi_dotasks(void) {}
void __attribute__((weak)) mcu_bt_dotasks(void) {}
#endif