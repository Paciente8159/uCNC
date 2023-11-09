#include "../../../cnc.h"

#if (MCU == MCU_RP2040)

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

#ifdef MCU_HAS_UART2
#ifndef UART2_TX_BUFFER_SIZE
#define UART2_TX_BUFFER_SIZE 64
#endif
DECL_BUFFER(uint8_t, uart2_tx, UART2_TX_BUFFER_SIZE);
DECL_BUFFER(uint8_t, uart2_rx, RX_BUFFER_SIZE);

// RX interrupt handler
static void __not_in_flash_func(mcu_uart2_irq)(void)
{
    uint32_t st = UART2_HW->ris;
    if (st & (UART_UARTRIS_RXRIS_BITS | UART_UARTRIS_RTRIS_BITS))
    {
        while (uart_is_readable(UART2_REG))
        {
            uint8_t c = (uart_getc(UART2_REG) & 0xFF);
#if !defined(DETACH_UART2_FROM_MAIN_PROTOCOL)
            if (mcu_com_rx_cb(c))
            {
                if (BUFFER_FULL(uart2_rx))
                {
                    c = OVF;
                }

                BUFFER_ENQUEUE(uart2_rx, &c);
            }
#else
            mcu_uart_rx_cb(c);
#endif
        }
    }

    if (st & UART_UARTRIS_TXRIS_BITS)
    {
        while (uart_is_writable(UART2_REG))
        {
            if (BUFFER_EMPTY(uart2_tx))
            {
                hw_clear_bits(&UART2_HW->icr, UART_UARTICR_TXIC_BITS);
                hw_clear_bits(&UART2_HW->imsc, UART_UARTIMSC_TXIM_BITS);
                return;
            }
            uint8_t c;
            BUFFER_DEQUEUE(uart2_tx, &c);
            uart_putc(UART2_REG, (char)c);
        }
    }
}

void mcu_uart2_init(int baud)
{
    BUFFER_INIT(uint8_t, uart2_tx, UART2_TX_BUFFER_SIZE);
    BUFFER_INIT(uint8_t, uart2_rx, RX_BUFFER_SIZE);

    gpio_set_function(TX2_BIT, GPIO_FUNC_UART);
    gpio_set_function(RX2_BIT, GPIO_FUNC_UART);

    uart_init(UART2_REG, baud);

    uart_set_hw_flow(UART2_REG, false, false);
    uart_set_format(UART2_REG, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART2_REG, false);

    irq_set_exclusive_handler(UART2_IRQn, mcu_uart2_irq);
    irq_set_enabled(UART2_IRQn, true);

    hw_set_bits(&UART2_HW->imsc, UART_UARTIMSC_RXIM_BITS | UART_UARTIMSC_RTIM_BITS);
}

uint8_t mcu_uart2_getc(void)
{
    uint8_t c = 0;
    BUFFER_DEQUEUE(uart2_rx, &c);
    return c;
}

uint8_t mcu_uart2_available(void)
{
    return BUFFER_READ_AVAILABLE(uart2_rx);
}

void mcu_uart2_clear(void)
{
    BUFFER_CLEAR(uart2_rx);
}

void mcu_uart2_putc(uint8_t c)
{
    while (BUFFER_FULL(uart2_tx))
    {
        mcu_uart2_flush();
    }
    BUFFER_ENQUEUE(uart2_tx, &c);
}

void mcu_uart2_flush(void)
{
    if (!(UART2_HW->imsc & UART_UARTIMSC_TXIM_BITS) && !BUFFER_EMPTY(uart2_tx)) // not ready start flushing
    {
        uint8_t c;
        BUFFER_DEQUEUE(uart2_tx, &c);
        COM2_OUTREG = c;
        // enable tx interrupts
        hw_set_bits(&UART2_HW->imsc, UART_UARTIMSC_TXIM_BITS);
        
#if ASSERT_PIN(ACTIVITY_LED)
        io_toggle_output(ACTIVITY_LED);
#endif
    }
}

#endif
#endif
