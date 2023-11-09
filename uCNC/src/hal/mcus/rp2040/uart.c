#include "../../../cnc.h"

#if (MCU == MCU_RP2040)

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

#ifdef MCU_HAS_UART
#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE 64
#endif
DECL_BUFFER(uint8_t, uart_tx, UART_TX_BUFFER_SIZE);
DECL_BUFFER(uint8_t, uart_rx, RX_BUFFER_SIZE);

// RX interrupt handler
static void mcu_uart_irq(void)
{
    uint32_t st = UART_HW->ris;
    if (st & (UART_UARTRIS_RXRIS_BITS | UART_UARTRIS_RTRIS_BITS))
    {
        while (uart_is_readable(UART_REG))
        {
            uint8_t c = (uart_getc(UART_REG) & 0xFF);
#if !defined(DETACH_UART_FROM_MAIN_PROTOCOL)
            if (mcu_com_rx_cb(c))
            {
                if (BUFFER_FULL(uart_rx))
                {
                    c = OVF;
                }

                BUFFER_ENQUEUE(uart_rx, &c);
            }
#else
            mcu_uart_rx_cb(c);
#endif
        }
    }

    if (st & UART_UARTRIS_TXRIS_BITS)
    {
        while (uart_is_writable(UART_REG))
        {
            if (BUFFER_EMPTY(uart_tx))
            {
                hw_clear_bits(&UART_HW->icr, UART_UARTICR_TXIC_BITS);
                hw_clear_bits(&UART_HW->imsc, UART_UARTIMSC_TXIM_BITS);
                return;
            }
            uint8_t c;
            BUFFER_DEQUEUE(uart_tx, &c);
            uart_putc(UART_REG, (char)c);
        }
    }
}

void rp2040_uart_init(int baud)
{
    BUFFER_INIT(uint8_t, uart_tx, UART_TX_BUFFER_SIZE);
    BUFFER_INIT(uint8_t, uart_rx, RX_BUFFER_SIZE);

    gpio_set_function(TX_BIT, GPIO_FUNC_UART);
    gpio_set_function(RX_BIT, GPIO_FUNC_UART);

    uart_init(UART_REG, baud);

    uart_set_hw_flow(UART_REG, false, false);
    uart_set_format(UART_REG, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_REG, false);

    irq_set_exclusive_handler(UART_IRQn, mcu_uart_irq);
    irq_set_enabled(UART_IRQn, true);

    hw_set_bits(&UART_HW->imsc, UART_UARTIMSC_RXIM_BITS | UART_UARTIMSC_RTIM_BITS);
}

uint8_t mcu_uart_getc(void)
{
    uint8_t c = 0;
    BUFFER_DEQUEUE(uart_rx, &c);
    return c;
}

uint8_t mcu_uart_available(void)
{
    return BUFFER_READ_AVAILABLE(uart_rx);
}

void mcu_uart_clear(void)
{
    BUFFER_CLEAR(uart_rx);
}

void mcu_uart_putc(uint8_t c)
{
    while (BUFFER_FULL(uart_tx))
    {
        mcu_uart_flush();
    }
    BUFFER_ENQUEUE(uart_tx, &c);
}

void mcu_uart_flush(void)
{
    if (!(UART_HW->imsc & UART_UARTIMSC_TXIM_BITS) && !BUFFER_EMPTY(uart_tx)) // not ready start flushing
    {
        uint8_t c;
        BUFFER_DEQUEUE(uart_tx, &c);
        COM_OUTREG = c;
        // enable tx interrupts
        hw_set_bits(&UART_HW->imsc, UART_UARTIMSC_TXIM_BITS);
        
#if ASSERT_PIN(ACTIVITY_LED)
        io_toggle_output(ACTIVITY_LED);
#endif
    }
}

#endif
#endif
