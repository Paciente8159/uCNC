/*
	Name: esp32s_usb.c
	Description: Implements the µCNC HAL for ESP32S.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 13-03-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifdef ESP32
#include <Arduino.H>
#include "USBCDC.h"
#include <stdint.h>

static inline void esp_usb_init(){
	Serial.begin();
}

static inline uint8_t esp_usb_read(){
	return (uint8_t)Serial.read();
}

static inline void esp_usb_write(uint8_t c){
	Serial.write(c);
}

static inline int esp_usb_avail(){
	return Serial.available();
}

static inline int esp_usb_ready(){
	return Serial.availableForWrite();
}

static inline int esp_usb_flush(){
	Serial.flush();
}

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../cnc.h"
#if (MCU == MCU_ESP32S || defined(ESP32S3) || defined(ESP32S2))

#ifdef MCU_HAS_USB

#ifndef USB_TX_BUFFER_SIZE
#define USB_TX_BUFFER_SIZE 64
#endif
	DECL_BUFFER(uint8_t, usb_rx, RX_BUFFER_SIZE);
	DECL_BUFFER(uint8_t, usb_tx, USB_TX_BUFFER_SIZE);

	void mcu_usb_init(void)
	{
		esp_usb_init();
	}

	void mcu_usb_dotasks(void)
	{
		while (esp_usb_avail())
		{
			uint8_t c = esp_usb_read();
#ifndef DETACH_UART_FROM_MAIN_PROTOCOL
			if (mcu_com_rx_cb(c))
			{
				if (BUFFER_FULL(usb_rx))
				{
					c = OVF;
				}

				BUFFER_ENQUEUE(usb_rx, &c);
			}
#else
			mcu_uart_rx_cb(c);
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
		if (!esp_usb_ready())
		{
			esp_usb_flush();
		}
		esp_usb_write(c);
	}

	void mcu_usb_flush(void)
	{
		// tusb_cdc_flush();
		// while (!tusb_cdc_write_available())
		// {
		// 	mcu_dotasks(); // tinyusb device task
		// 	if (!tusb_cdc_connected)
		// 	{
		// 		return;
		// 	}
		// }
	}

#endif

// #ifdef MCU_HAS_USBxxx
// #include "tusb_config.h"
// #include "tusb.h"
// #include "device/usbd.h"
// #include "class/cdc/cdc_device.h"
// #include "esp_rom_gpio.h"

// #define tusb_cdc_init tusb_init
// #define tusb_cdc_isr_handler() tud_int_handler(0)
// #define tusb_cdc_task tud_task
// #define tusb_cdc_available() tud_cdc_n_available(0)
// #define tusb_cdc_read() tud_cdc_n_read_char(0)
// #define tusb_cdc_flush() tud_cdc_n_write_flush(0)
// #define tusb_cdc_write(ch) tud_cdc_n_write_char(0, ch)
// #define tusb_cdc_write_available() tud_cdc_n_write_available(0)
// #define tusb_cdc_write_buffer(buffer, bufsize) tud_cdc_n_write(0, buffer, bufsize)
// #define tusb_cdc_connected tud_cdc_n_connected(0)

// #ifndef USB_TX_BUFFER_SIZE
// #define USB_TX_BUFFER_SIZE 64
// #endif
// DECL_BUFFER(uint8_t, usb_rx, RX_BUFFER_SIZE);
// DECL_BUFFER(uint8_t, usb_tx, USB_TX_BUFFER_SIZE);

// #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0)
// #include "esp_private/usb_phy.h"
// static usb_phy_handle_t phy_hdl;
// bool usb_init(void)
// {
// 	// Configure USB PHY
// 	usb_phy_config_t phy_conf = {
// 			.controller = USB_PHY_CTRL_OTG,
// 			.target = USB_PHY_TARGET_INT,

// 	// maybe we can use USB_OTG_MODE_DEFAULT and switch using dwc2 driver
// #if CFG_TUD_ENABLED
// 			.otg_mode = USB_OTG_MODE_DEVICE,
// #elif CFG_TUH_ENABLED
// 			.otg_mode = USB_OTG_MODE_HOST,
// #endif
// 			// https://github.com/hathach/tinyusb/issues/2943#issuecomment-2601888322
// 			// Set speed to undefined (auto-detect) to avoid timinng/racing issue with S3 with host such as macOS
// 			.otg_speed = USB_PHY_SPEED_UNDEFINED,
// 	};

// 	usb_new_phy(&phy_conf, &phy_hdl);

// 	return true;
// }
// #else
// #include "esp_private/usb_phy.h"
// #include "hal/usb_hal.h"
// #include "soc/usb_periph.h"
// static void configure_pins(usb_hal_context_t *usb)
// {
// 	/* usb_periph_iopins currently configures USB_OTG as USB Device.
// 	 * Introduce additional parameters in usb_hal_context_t when adding support
// 	 * for USB Host. */
// 	for (const usb_iopin_dsc_t *iopin = usb_periph_iopins; iopin->pin != -1; ++iopin)
// 	{
// 		if ((usb->use_external_phy) || (iopin->ext_phy_only == 0))
// 		{
// 			esp_rom_gpio_pad_select_gpio(iopin->pin);
// 			if (iopin->is_output)
// 			{
// 				esp_rom_gpio_connect_out_signal(iopin->pin, iopin->func, false, false);
// 			}
// 			else
// 			{
// 				esp_rom_gpio_connect_in_signal(iopin->pin, iopin->func, false);
// 				if ((iopin->pin != GPIO_MATRIX_CONST_ZERO_INPUT) && (iopin->pin != GPIO_MATRIX_CONST_ONE_INPUT))
// 				{
// 					gpio_ll_input_enable(&GPIO, iopin->pin);
// 				}
// 			}
// 			esp_rom_gpio_pad_unhold(iopin->pin);
// 		}
// 	}

// 	if (!usb->use_external_phy)
// 	{
// 		gpio_set_drive_capability(USB_DM_BIT, GPIO_DRIVE_CAP_3);
// 		gpio_set_drive_capability(USB_DP_BIT, GPIO_DRIVE_CAP_3);
// 	}
// }

// bool usb_init(void)
// {
// 	// USB Controller Hal init
// 	periph_module_reset(PERIPH_USB_MODULE);
// 	periph_module_enable(PERIPH_USB_MODULE);

// 	usb_hal_context_t hal = {
// 			.use_external_phy = false // use built-in PHY
// 	};

// 	usb_hal_init(&hal);
// 	configure_pins(&hal);

// 	return true;
// }
// #endif

// // void tinyusb_cdc_rx_callback(int itf, cdcacm_event_t *event)
// // {
// //     /* initialization */
// //     size_t rx_size = 0;

// //     /* read */
// //     esp_err_t ret = tinyusb_cdcacm_read(itf, buf, CONFIG_TINYUSB_CDC_RX_BUFSIZE, &rx_size);
// //     if (ret == ESP_OK) {
// //         ESP_LOGI(TAG, "Data from channel %d:", itf);
// //         ESP_LOG_BUFFER_HEXDUMP(TAG, buf, rx_size, ESP_LOG_INFO);
// //     } else {
// //         ESP_LOGE(TAG, "Read error");
// //     }

// //     /* write back */
// //     tinyusb_cdcacm_write_queue(itf, buf, rx_size);
// //     tinyusb_cdcacm_write_flush(itf, 0);
// // }

// void mcu_usb_init(void)
// {
// 	usb_init();
// }

// void mcu_usb_dotasks(void)
// {
// 	tusb_cdc_task(); // tinyusb device task

// 	while (tusb_cdc_available())
// 	{
// 		uint8_t c = (uint8_t)tusb_cdc_read();
// #if !defined(DETACH_USB_FROM_MAIN_PROTOCOL)
// 		if (mcu_com_rx_cb(c))
// 		{
// 			if (BUFFER_FULL(usb_rx))
// 			{
// 				c = OVF;
// 			}

// 			BUFFER_ENQUEUE(usb_rx, &c);
// 		}
// #else
// 		mcu_usb_rx_cb(c);
// #endif
// 	}
// }

// uint8_t mcu_usb_getc(void)
// {
// 	uint8_t c = 0;
// 	BUFFER_DEQUEUE(usb_rx, &c);
// 	return c;
// }

// uint8_t mcu_usb_available(void)
// {
// 	return BUFFER_READ_AVAILABLE(usb_rx);
// }

// void mcu_usb_clear(void)
// {
// 	BUFFER_CLEAR(usb_rx);
// }

// void mcu_usb_putc(uint8_t c)
// {
// 	if (!tusb_cdc_write_available())
// 	{
// 		mcu_usb_flush();
// 	}
// 	tusb_cdc_write(c);
// }

// void mcu_usb_flush(void)
// {
// 	tusb_cdc_flush();
// 	while (!tusb_cdc_write_available())
// 	{
// 		mcu_dotasks(); // tinyusb device task
// 		if (!tusb_cdc_connected)
// 		{
// 			return;
// 		}
// 	}
// }

// #endif
#endif

#ifdef __cplusplus
}  // end extern "C"
#endif

#endif
