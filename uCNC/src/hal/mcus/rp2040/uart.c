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
static void __not_in_flash_func(mcu_uart_isr)(void)
{
    uint32_t status = UART_HW->mis;
    if (status & (UART_UARTMIS_RXMIS_BITS | UART_UARTIMSC_RTIM_BITS))
    {
        uint8_t c = (COM_INREG & 0xFF);
#if !defined(DETACH_UART_FROM_MAIN_PROTOCOL)
        if (mcu_com_rx_cb(c))
        {
            if (BUFFER_FULL(uart_rx))
            {
                c = OVF;
            }

            *(BUFFER_NEXT_FREE(uart_rx)) = c;
            BUFFER_STORE(uart_rx);
        }
#else
        mcu_uart_rx_cb(c);
#endif
    }

    // if (status & UART_UARTMIS_TXMIS_BITS)
    // {
    //     if (BUFFER_EMPTY(uart_tx)/* || ((UART_HW->fr & UART_UARTFR_TXFF_BITS))*/)
    //     {
    //         // hw_clear_bits(&UART_HW->imsc, UART_UARTIMSC_TXIM_BITS);
    //         return;
    //     }
    //     uint8_t c;
    //     BUFFER_DEQUEUE(uart_tx, &c);
    //     COM_OUTREG = c;
    // }
}

void rp2040_uart_init()
{
    // Set up our UART with a basic baud rate.
    uart_init(UART_REG, BAUDRATE);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(TX_BIT, GPIO_FUNC_UART);
    gpio_set_function(RX_BIT, GPIO_FUNC_UART);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(UART_REG, false, false);

    // Set our data format
    uart_set_format(UART_REG, 8, 1, UART_PARITY_NONE);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(UART_REG, true);

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQn, mcu_uart_isr);
    irq_set_enabled(UART_IRQn, true);

    // Now enable the UART to send interrupts
    hw_set_bits(&UART_HW->imsc, UART_UARTIMSC_RXIM_BITS | UART_UARTIMSC_RTIM_BITS);
    uart_set_irq_enables(UART_REG, true, false);
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
    if (!BUFFER_EMPTY(uart_tx))
    {
        uint8_t tmp[UART_TX_BUFFER_SIZE];
        uint8_t r;
        BUFFER_READ(uart_tx, tmp, UART_TX_BUFFER_SIZE, r);
        uart_puts(UART_REG, tmp);
    }
    //     if (!(UART_HW->imsc & UART_UARTIMSC_TXIM_BITS) && !(UART_HW->fr & UART_UARTFR_TXFF_BITS) && !BUFFER_EMPTY(uart_tx)) // not ready start flushing
    //     {
    //         // enable tx interrupts
    //         hw_set_bits(&UART_HW->imsc, UART_UARTIMSC_TXIM_BITS);
    //         // sends first char to trigger isr
    //         uint8_t c;
    //         BUFFER_DEQUEUE(uart_tx, &c);
    //         COM_OUTREG = c;
    // #if ASSERT_PIN(ACTIVITY_LED)
    //         io_toggle_output(ACTIVITY_LED);
    // #endif
    //     }
}

#endif
#endif
