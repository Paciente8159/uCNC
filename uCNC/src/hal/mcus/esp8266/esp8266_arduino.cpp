/*
	Name: esp8266_uart.cpp
	Description: Contains all Arduino ESP8266 C++ to C functions used by UART in µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 27-07-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifdef ESP8266
#include "../../../../cnc_config.h"
#include <Arduino.h>
#include "user_interface.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef ENABLE_WIFI
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

#ifndef WIFI_PORT
#define WIFI_PORT 23
#endif

#ifndef WIFI_USER
#define WIFI_USER "admin"
#endif

#ifndef WIFI_PASS
#define WIFI_PASS "pass"
#endif

#ifndef WIFI_SSID_MAX_LEN
#define WIFI_SSID_MAX_LEN 32
#endif

#define ARG_MAX_LEN WIFI_SSID_MAX_LEN

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
const char *update_path = "/firmware";
const char *update_username = WIFI_USER;
const char *update_password = WIFI_PASS;
#define MAX_SRV_CLIENTS 1
WiFiServer server(WIFI_PORT);
WiFiClient serverClient;

typedef struct
{
	uint8_t wifi_on;
	uint8_t wifi_mode;
	char ssid[WIFI_SSID_MAX_LEN];
	char pass[WIFI_SSID_MAX_LEN];
} wifi_settings_t;

uint16_t wifi_settings_offset;
wifi_settings_t wifi_settings;
#endif

extern "C"
{
#include "../../../cnc.h"

#ifdef BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
	uint8_t mcu_custom_grbl_cmd(uint8_t *grbl_cmd_str, uint8_t grbl_cmd_len, uint8_t next_char)
	{
		uint8_t str[64];
		uint8_t arg[ARG_MAX_LEN];
		uint8_t has_arg = (next_char == '=');
		memset(arg, 0, sizeof(arg));
		if (has_arg)
		{
			uint8_t c = serial_getc();
			uint8_t i = 0;
			while (c)
			{
				arg[i++] = c;
				if (i >= ARG_MAX_LEN)
				{
					return STATUS_INVALID_STATEMENT;
				}
				c = serial_getc();
			}
		}

#ifdef ENABLE_WIFI
		if (!strncmp((const char*)grbl_cmd_str, "WIFI", 4))
		{
			if (!strcmp((const char *)&grbl_cmd_str[4], "ON"))
			{
				WiFi.disconnect();
				switch (wifi_settings.wifi_mode)
				{
				case 1:
					WiFi.mode(WIFI_STA);
					WiFi.begin(wifi_settings.ssid, wifi_settings.pass);
					protocol_send_feedback((const char *)"Trying to connect to WiFi");
					break;
				case 2:
					WiFi.mode(WIFI_AP);
					WiFi.softAP(BOARD_NAME, wifi_settings.pass);
					protocol_send_feedback((const char *)"AP started");
					protocol_send_feedback((const char *)"SSID>" BOARD_NAME);
					sprintf((char*)str, "IP>%s", WiFi.softAPIP().toString().c_str());
					protocol_send_feedback((const char *)str);
					break;
				default:
					WiFi.mode(WIFI_AP_STA);
					WiFi.begin(wifi_settings.ssid, wifi_settings.pass);
					protocol_send_feedback((const char *)"Trying to connect to WiFi");
					WiFi.softAP(BOARD_NAME, wifi_settings.pass);
					protocol_send_feedback((const char *)"AP started");
					protocol_send_feedback((const char *)"SSID>" BOARD_NAME);
					sprintf((char*)str, "IP>%s", WiFi.softAPIP().toString().c_str());
					protocol_send_feedback((const char *)str);
					break;
				}

				wifi_settings.wifi_on = 1;
				settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
				return STATUS_OK;
			}

			if (!strcmp((const char *)&grbl_cmd_str[4], "OFF"))
			{
				WiFi.disconnect();
				wifi_settings.wifi_on = 0;
				settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
				return STATUS_OK;
			}

			if (!strcmp((const char *)&grbl_cmd_str[4], "SSID"))
			{
				if (has_arg)
				{
					uint8_t len = strlen((const char *)arg);
					if (len > WIFI_SSID_MAX_LEN)
					{
						protocol_send_feedback((const char *)"WiFi SSID is too long");
					}
					memset(wifi_settings.ssid, 0, sizeof(wifi_settings.ssid));
					strcpy((char *)wifi_settings.ssid, (const char*)arg);
					settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
					protocol_send_feedback((const char *)"WiFi SSID modified");
				}
				else
				{
					sprintf((char*)str, "SSID>%s", wifi_settings.ssid);
					protocol_send_feedback((const char *)str);
				}
				return STATUS_OK;
			}

			if (!strcmp((const char *)&grbl_cmd_str[4], "SCAN"))
			{
				// Serial.println("[MSG:Scanning Networks]");
				protocol_send_feedback((const char *)"Scanning Networks");
				int numSsid = WiFi.scanNetworks();
				if (numSsid == -1)
				{
					protocol_send_feedback((const char *)"Failed to scan!");
					while (true)
						;
				}

				// print the list of networks seen:
				sprintf((char*)str, "%d available networks", numSsid);
				protocol_send_feedback((const char *)str);

				// print the network number and name for each network found:
				for (int netid = 0; netid < numSsid; netid++)
				{
					sprintf((char*)str, "%d) %s\tSignal:  %ddBm", netid, WiFi.SSID(netid).c_str(), WiFi.RSSI(netid));
					protocol_send_feedback((const char *)str);
				}
				return STATUS_OK;
			}

			if (!strcmp((const char *)&grbl_cmd_str[4], "SAVE"))
			{
				settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
				protocol_send_feedback((const char *)"WiFi settings saved");
				return STATUS_OK;
			}

			if (!strcmp((const char *)&grbl_cmd_str[4], "RESET"))
			{
				settings_erase(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
				protocol_send_feedback((const char *)"WiFi settings deleted");
				return STATUS_OK;
			}

			if (!strcmp((const char *)&grbl_cmd_str[4], "MODE"))
			{
				if (has_arg)
				{
					int mode = atoi((const char*)arg) - 1;
					if (mode >= 0)
					{
						wifi_settings.wifi_mode = mode;
					}
					else
					{
						protocol_send_feedback((const char *)"Invalid value. STA+AP(1), STA(2), AP(3)");
					}
				}

				switch (wifi_settings.wifi_mode)
				{
				case 0:
					protocol_send_feedback((const char *)"WiFi mode>STA+AP");
					break;
				case 1:
					protocol_send_feedback((const char *)"WiFi mode>STA");
					break;
				case 2:
					protocol_send_feedback((const char *)"WiFi mode>AP");
					break;
				}
				return STATUS_OK;
			}

			if (!strcmp((const char *)&grbl_cmd_str[4], "PASS") && has_arg)
			{
				uint8_t len = strlen((const char *)arg);
				if (len > WIFI_SSID_MAX_LEN)
				{
					protocol_send_feedback((const char *)"WiFi pass is too long");
				}
				memset(wifi_settings.pass, 0, sizeof(wifi_settings.pass));
				strcpy((char *)wifi_settings.pass, (const char*)arg);
				protocol_send_feedback((const char *)"WiFi password modified");
				return STATUS_OK;
			}

			if (!strcmp((const char *)&grbl_cmd_str[4], "IP"))
			{
				if (wifi_settings.wifi_on)
				{
					switch (wifi_settings.wifi_mode)
					{
					case 1:
						sprintf((char*)str, "STA IP>%s", WiFi.softAPIP().toString().c_str());
						protocol_send_feedback((const char *)str);
						sprintf((char*)str, "AP IP>%s", WiFi.softAPIP().toString().c_str());
						protocol_send_feedback((const char *)str);
						break;
					case 2:
						sprintf((char*)str, "IP>%s", WiFi.softAPIP().toString().c_str());
						protocol_send_feedback((const char *)str);
						break;
					default:
						sprintf((char*)str, "IP>%s", WiFi.softAPIP().toString().c_str());
						protocol_send_feedback((const char *)str);
						break;
					}
				}
				else
				{
					protocol_send_feedback((const char *)"WiFi is off");
				}

				return STATUS_OK;
			}
		}
#endif
		return STATUS_INVALID_STATEMENT;
	}
#endif

	bool esp8266_wifi_clientok(void)
	{
#ifdef ENABLE_WIFI
		static uint32_t next_info = 30000;
		static bool connected = false;
		uint8_t str[64];

		if (!wifi_settings.wifi_on)
		{
			return false;
		}

		if ((WiFi.status() != WL_CONNECTED))
		{
			connected = false;
			if (next_info > mcu_millis())
			{
				return false;
			}
			next_info = mcu_millis() + 30000;
			protocol_send_feedback((const char *)"Disconnected from WiFi");
			return false;
		}

		if (!connected)
		{
			connected = true;
			protocol_send_feedback((const char *)"Connected to WiFi");
			sprintf((char*)str, "SSID>%s", wifi_settings.ssid);
			protocol_send_feedback((const char *)str);
			sprintf((char*)str, "IP>%s", WiFi.localIP().toString().c_str());
			protocol_send_feedback((const char *)str);
		}

		if (server.hasClient())
		{
			if (serverClient)
			{
				if (serverClient.connected())
				{
					serverClient.stop();
				}
			}
			serverClient = server.accept();
			serverClient.println("[MSG:New client connected]");
			return false;
		}
		else if (serverClient)
		{
			if (serverClient.connected())
			{
				return true;
			}
		}
#endif
		return false;
	}

	void esp8266_uart_init(int baud)
	{
		Serial.begin(baud);
#ifdef ENABLE_WIFI
		WiFi.setSleepMode(WIFI_NONE_SLEEP);

		wifi_settings_offset = settings_register_external_setting(sizeof(wifi_settings_t));
		if (settings_load(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t)))
		{
			settings_erase(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
		}

		WiFi.begin();
		if (!wifi_settings.wifi_on)
		{
			WiFi.disconnect();
		}
		server.begin();
		server.setNoDelay(true);
		httpUpdater.setup(&httpServer, update_path, update_username, update_password);
		httpServer.begin();
#endif
	}

#ifdef MCU_HAS_UART
#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE 64
#endif
	DECL_BUFFER(uint8_t, uart_rx, RX_BUFFER_SIZE);
	DECL_BUFFER(uint8_t, uart_tx, UART_TX_BUFFER_SIZE);
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
		while (!BUFFER_EMPTY(uart_tx))
		{
			uint8_t tmp[UART_TX_BUFFER_SIZE];
			uint8_t r;
			uint8_t max = (uint8_t)MIN(Serial.availableForWrite(), UART_TX_BUFFER_SIZE);

			BUFFER_READ(uart_tx, tmp, max, r);
			Serial.write(tmp, r);
			Serial.flush();
		}
	}
#endif

#ifdef MCU_HAS_WIFI
#ifndef WIFI_TX_BUFFER_SIZE
#define WIFI_TX_BUFFER_SIZE 64
#endif
	DECL_BUFFER(uint8_t, wifi_rx, RX_BUFFER_SIZE);
	DECL_BUFFER(uint8_t, wifi_tx, WIFI_TX_BUFFER_SIZE);

	uint8_t mcu_wifi_getc(void)
	{
		uint8_t c = 0;
		BUFFER_DEQUEUE(wifi_tx, &c);
		return c;
	}

	uint8_t mcu_wifi_available(void)
	{
		return BUFFER_READ_AVAILABLE(wifi_tx);
	}

	void mcu_wifi_clear(void)
	{
		BUFFER_CLEAR(wifi_tx);
	}
	void mcu_wifi_putc(uint8_t c)
	{
		while (BUFFER_FULL(wifi_tx))
		{
			mcu_wifi_flush();
		}
		BUFFER_ENQUEUE(wifi_tx, &c);
	}

	void mcu_wifi_flush(void)
	{
		if (esp8266_wifi_clientok())
		{
			while (!BUFFER_EMPTY(wifi_tx))
			{
				uint8_t tmp[WIFI_TX_BUFFER_SIZE];
				uint8_t r;
				uint8_t max = (uint8_t)MIN(serverClient.availableForWrite(), WIFI_TX_BUFFER_SIZE);

				BUFFER_READ(wifi_tx, tmp, max, r);
				serverClient.write(tmp, r);
			}
		}
		else
		{
			// no client (discard)
			BUFFER_CLEAR(wifi_tx);
		}
	}
#endif

	bool esp8266_uart_rx_ready(void)
	{
		bool wifiready = false;
#ifdef ENABLE_WIFI
		if (esp8266_wifi_clientok())
		{
			wifiready = (serverClient.available() > 0);
		}
#endif
		return ((Serial.available() > 0) || wifiready);
	}

	void esp8266_uart_process(void)
	{
		while (Serial.available() > 0)
		{
			system_soft_wdt_feed();
#ifndef DETACH_UART_FROM_MAIN_PROTOCOL
			uint8_t c = (uint8_t)Serial.read();
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
			mcu_uart_rx_cb((uint8_t)Serial.read());
#endif
		}

#ifdef ENABLE_WIFI
		httpServer.handleClient();
		if (esp8266_wifi_clientok())
		{
			while (serverClient.available() > 0)
			{
				system_soft_wdt_feed();
#ifndef DETACH_WIFI_FROM_MAIN_PROTOCOL
				uint8_t c = (uint8_t)serverClient.read();
				if (mcu_com_rx_cb(c))
				{
					if (BUFFER_FULL(wifi_rx))
					{
						c = OVF;
					}

					*(BUFFER_NEXT_FREE(wifi_rx)) = c;
					BUFFER_STORE(wifi_rx);
				}
#else
				mcu_wifi_rx_cb((uint8_t)serverClient.read());
#endif
			}
		}
#endif
	}
}

#ifdef MCU_HAS_SPI
#include <Arduino.h>
#include <SPI.h>
#include "esp_peri.h"
extern "C"
{
#include "../../../cnc.h"
	void esp8266_spi_init(uint32_t freq, uint8_t mode)
	{
		SPI.begin();
		SPI.setFrequency(freq);
		SPI.setDataMode(mode);
	}

	void mcu_spi_config(uint8_t mode, uint32_t freq)
	{
		SPI.setFrequency(freq);
		SPI.setDataMode(mode);
	}
}

#endif

#ifndef RAM_ONLY_SETTINGS
#include <Arduino.h>
#include <EEPROM.h>
#include <stdint.h>
extern "C"
{
	void esp8266_eeprom_init(int size)
	{
		EEPROM.begin(size);
	}

	uint8_t mcu_eeprom_getc(uint16_t address)
	{
		return EEPROM.read(address);
	}

	void mcu_eeprom_putc(uint16_t address, uint8_t value)
	{
		EEPROM.write(address, value);
	}

	void mcu_eeprom_flush(void)
	{
		if (!EEPROM.commit())
		{
			Serial.println("[MSG: EEPROM write error]");
		}
	}
}

#endif

#endif
