#include "../../../../cnc_config.h"
#include <Arduino.h>
#include <stdbool.h>

extern "C"
{
    void esp8266_uart_init(int baud)
    {
        Serial.begin(baud);
    }

    char esp8266_uart_read(void)
    {
        return Serial.read();
    }

    void esp8266_uart_write(char c)
    {
        Serial.write(c);
    }

    bool esp8266_uart_rx_ready(void)
    {
        return (Serial.available() != 0);
    }

    bool esp8266_uart_tx_ready(void)
    {
        return (Serial.availableForWrite() != 0);
    }

    void esp8266_uart_flush(void)
    {
        Serial.flush();
    }

    void mcu_com_rx_cb(unsigned char c);
}
