/*
	Name: esp32_bt.c
	Description: Implements the µCNC BT shim for all ESP32 variants.
	Uses Arduino

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 11-09-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../../cnc_config.h"

#ifdef ESP32
#include <Arduino.h>
#endif
#if CONFIG_IDF_TARGET_ESP32 && defined(ENABLE_BLUETOOTH)
#include "esp_task_wdt.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <BluetoothSerial.h>
BluetoothSerial SerialBT;

extern "C"
{
#include "../../../cnc.h"

#ifndef BT_ID_MAX_LEN
#define BT_ID_MAX_LEN 32
#endif

#if defined(MCU_HAS_BLUETOOTH) && defined(ENABLE_BLUETOOTH)

	uint8_t bt_on;
	uint16_t bt_settings_offset;

#ifdef BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
	bool mcu_bt_grbl_cmd(void *args)
	{
		grbl_cmd_args_t *cmd_params = (grbl_cmd_args_t *)args;
		char arg[BT_ID_MAX_LEN];
		memset(arg, 0, sizeof(arg));

		if (!strncmp((const char *)(cmd_params->cmd), "BTH", 3))
		{
			if (!strcmp((const char *)&(cmd_params->cmd)[3], "ON"))
			{
				SerialBT.begin(BOARD_NAME);
				proto_info("Bluetooth enabled");
				bt_on = 1;
				settings_save(bt_settings_offset, &bt_on, 1);
				*(cmd_params->error) = STATUS_OK;
				return EVENT_HANDLED;
			}

			if (!strcmp((const char *)&(cmd_params->cmd)[3], "OFF"))
			{
				SerialBT.end();
				proto_info("Bluetooth disabled");
				bt_on = 0;
				settings_save(bt_settings_offset, &bt_on, 1);
				*(cmd_params->error) = STATUS_OK;
				return EVENT_HANDLED;
			}
		}

		return EVENT_CONTINUE;
	}

	CREATE_EVENT_LISTENER(grbl_cmd, mcu_bt_grbl_cmd);
#endif

#ifndef BLUETOOTH_TX_BUFFER_SIZE
#define BLUETOOTH_TX_BUFFER_SIZE 64
#endif
	DECL_BUFFER(uint8_t, bt_rx, RX_BUFFER_SIZE);
	DECL_BUFFER(uint8_t, bt_tx, BLUETOOTH_TX_BUFFER_SIZE);

	void mcu_bt_init(void)
	{
		bt_settings_offset = settings_register_external_setting(1);
		if (settings_load(bt_settings_offset, &bt_on, 1))
		{
			settings_erase(bt_settings_offset, (uint8_t *)&bt_on, 1);
		}

		if (bt_on)
		{
			SerialBT.begin(BOARD_NAME);
		}

#ifdef BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
		ADD_EVENT_LISTENER(grbl_cmd, mcu_bt_grbl_cmd);
#endif
	}

	void mcu_bt_dotasks(void)
	{
		if (SerialBT.hasClient())
		{
			while (SerialBT.available() > 0)
			{
				esp_task_wdt_reset();
#ifndef DETACH_BLUETOOTH_FROM_MAIN_PROTOCOL
				uint8_t c = SerialBT.read();
				if (mcu_com_rx_cb(c))
				{
					if (!BUFFER_TRY_ENQUEUE(bt_rx, &c))
					{
						STREAM_OVF(c);
					}
				}
#else
				mcu_bt_rx_cb((uint8_t)SerialBT.read());
#endif
			}
		}
	}

	uint8_t mcu_bt_getc(void)
	{
		uint8_t c = 0;
		BUFFER_TRY_DEQUEUE(bt_rx, &c);
		return c;
	}

	uint8_t mcu_bt_available(void)
	{
		return BUFFER_READ_AVAILABLE(bt_rx);
	}

	void mcu_bt_clear(void)
	{
		BUFFER_CLEAR(bt_rx);
	}

	void mcu_bt_putc(uint8_t c)
	{
		while (!BUFFER_TRY_ENQUEUE(bt_tx, &c))
		{
			mcu_bt_flush();
		}
	}

	void mcu_bt_flush(void)
	{
		if (SerialBT.hasClient())
		{
			while (!BUFFER_EMPTY(bt_tx))
			{
				uint8_t tmp[BLUETOOTH_TX_BUFFER_SIZE + 1];
				memset(tmp, 0, sizeof(tmp));
				uint8_t r;

				BUFFER_READ(bt_tx, tmp, BLUETOOTH_TX_BUFFER_SIZE, r);
				SerialBT.write(tmp, r);
				SerialBT.flush();
			}
		}
		else
		{
			// no client (discard)
			BUFFER_CLEAR(bt_tx);
		}
	}
#endif
}
#endif
