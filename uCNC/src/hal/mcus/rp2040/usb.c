#include "../../../cnc.h"

#if (MCU == MCU_RP2040)

#ifdef MCU_HAS_USB
#include <tusb.h>
#endif

#include "pico/stdlib.h"
#include "hardware/irq.h"

#ifdef MCU_HAS_UART
DECL_BUFFER(uint8_t, usb_rx, RX_BUFFER_SIZE);

void mcu_usb_init(void)
{
    BUFFER_INIT(uint8_t, usb_rx, RX_BUFFER_SIZE);
    tusb_cdc_init();
}

void mcu_usb_dotasks()
{
    tusb_cdc_task(); // tinyusb device task

    while (tusb_cdc_available())
    {
        uint8_t c = (uint8_t)tusb_cdc_read();
#if !defined(DETACH_USB_FROM_MAIN_PROTOCOL)
        if (mcu_com_rx_cb(c))
        {
            if (BUFFER_FULL(usb_rx))
            {
                c = OVF;
            }

            uint8_t c;
            BUFFER_ENQUEUE(usb_rx, &c);
        }
#else
        mcu_usb_rx_cb(c);
#endif
    }
}

uint8_t mcu_usb_getc(void)
{
    uint8_t c = 0;
    BUFFER_DEQUEUE(usb_rx, &c);
    return c;
}

uint8_t mcu_usb_available(void)
{
    return BUFFER_READ_AVAILABLE(usb_rx);
}

void mcu_usb_clear(void)
{
    BUFFER_CLEAR(usb_rx);
}

void mcu_usb_putc(uint8_t c)
{
    if (!tusb_cdc_write_available())
	{
		mcu_usb_flush();
	}
	tusb_cdc_write(c);
}

void mcu_usb_flush(void)
{
    tusb_cdc_flush();
	while (!tusb_cdc_write_available())
	{
		mcu_dotasks(); // tinyusb device task
		if (!tusb_cdc_connected)
		{
			return;
		}
	}
}

#endif
#endif
