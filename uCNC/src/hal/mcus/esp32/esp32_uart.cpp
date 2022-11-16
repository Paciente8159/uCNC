/*
	Name: esp32_uart.cpp
	Description: Contains all Arduino ESP32 C++ to C functions used by UART in µCNC.

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

#ifdef ESP32
#include <Arduino.h>
#include "esp_task_wdt.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef ENABLE_BLUETOOTH
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;
#endif

#ifdef ENABLE_WIFI
#include <WiFi.h>
#include <WebServer.h>
// #include <HTTPUpdateServer.h>

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

WebServer httpServer(80);
// HTTPUpdateServer httpUpdater;
const char *update_path = "/firmware";
const char *update_username = WIFI_USER;
const char *update_password = WIFI_PASS;
#define MAX_SRV_CLIENTS 1
WiFiServer server(WIFI_PORT);
WiFiClient serverClient;

typedef struct
{
	uint8_t wifi_mode;
	char ssid[WIFI_SSID_MAX_LEN];
	char pass[WIFI_SSID_MAX_LEN];
} wifi_settings_t;

uint16_t wifi_settings_offset;
wifi_settings_t wifi_settings;
#endif

#ifndef ESP32_BUFFER_SIZE
#define ESP32_BUFFER_SIZE 255
#endif

static char esp32_tx_buffer[ESP32_BUFFER_SIZE];
static uint8_t esp32_tx_buffer_counter;
extern "C"
{
#include "../../../cnc.h"

#ifdef BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
	uint8_t mcu_custom_grbl_cmd(char *grbl_cmd_str, uint8_t grbl_cmd_len)
	{
#ifdef ENABLE_WIFI
		char arg[WIFI_SSID_MAX_LEN];
		char has_arg = (grbl_cmd_str[grbl_cmd_len] == '=');
		memset(arg, 0, sizeof(arg));
		if (has_arg)
		{
			grbl_cmd_str[grbl_cmd_len] = 0;
			char c = serial_getc();
			uint8_t i = 0;
			while (c)
			{
				arg[i++] = c;
				if (i >= WIFI_SSID_MAX_LEN)
				{
					return STATUS_INVALID_STATEMENT;
				}
				c = serial_getc();
			}
		}

		if (!strncmp(grbl_cmd_str, "WIFI", 4))
		{
			if (!strcmp(grbl_cmd_str, "WIFION"))
			{
				WiFi.disconnect();
				switch (wifi_settings.wifi_mode)
				{
				case 1:
					WiFi.mode(WIFI_STA);
					WiFi.begin(wifi_settings.ssid, wifi_settings.pass);
					Serial.println("[MSG:Trying to connect to WiFi]");
					break;
				case 2:
					WiFi.mode(WIFI_AP);
					WiFi.softAP(BOARD_NAME, wifi_settings.pass);
					Serial.println("[MSG:AP started]");
					break;
				default:
					WiFi.mode(WIFI_AP_STA);
					WiFi.begin(wifi_settings.ssid, wifi_settings.pass);
					Serial.println("[MSG:Trying to connect to WiFi]");
					WiFi.softAP(BOARD_NAME, wifi_settings.pass);
					Serial.println("[MSG:AP started]");
					break;
				}

				return STATUS_OK;
			}

			if (!strcmp(grbl_cmd_str, "WIFIOFF"))
			{
				WiFi.disconnect();
				return STATUS_OK;
			}

			if (!strcmp(grbl_cmd_str, "WIFISSID"))
			{
				if (has_arg)
				{
					uint8_t len = strlen(arg);
					if (len > WIFI_SSID_MAX_LEN)
					{
						Serial.println("[MSG:WiFi SSID is too long]");
					}
					memset(wifi_settings.ssid, 0, sizeof(wifi_settings.ssid));
					strcpy(wifi_settings.ssid, arg);
					settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
					Serial.println("[MSG:WiFi SSID saved]");
				}
				else
				{
					Serial.print("[MSG: SSID ");
					Serial.print(wifi_settings.ssid);
					Serial.println("]");
				}
				return STATUS_OK;
			}

			if (!strcmp(grbl_cmd_str, "WIFISCAN"))
			{
				Serial.println("[MSG: Scanning Networks]");
				int numSsid = WiFi.scanNetworks();
				if (numSsid == -1)
				{
					Serial.println("[MSG: Failed to scan!]");
					while (true)
						;
				}

				// print the list of networks seen:
				Serial.print("[MSG: ");
				Serial.print(numSsid);
				Serial.println(" available networks]");

				// print the network number and name for each network found:
				for (int netid = 0; netid < numSsid; netid++)
				{
					Serial.print("[MSG: ");
					Serial.print(netid);
					Serial.print(") ");
					Serial.print(WiFi.SSID(netid));
					Serial.print("\tSignal: ");
					Serial.print(WiFi.RSSI(netid));
					Serial.print(" dBm");
					Serial.println("]");
				}
				return STATUS_OK;
			}

			if (!strcmp(grbl_cmd_str, "WIFIMODE"))
			{
				if (has_arg)
				{
					int mode = atoi(arg) - 1;
					if (mode > 0)
					{
						settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
						Serial.println("[MSG:WiFi mode saved]");
					}
					else
					{
						Serial.println("[MSG:Invalid value. STA+AP(1), STA(2), AP(3)]");
					}
				}
				return STATUS_OK;
			}

			if (!strcmp(grbl_cmd_str, "WIFIPASS") && has_arg)
			{
				uint8_t len = strlen(arg);
				if (len > WIFI_SSID_MAX_LEN)
				{
					Serial.println("[MSG:WiFi pass is too long]");
				}
				memset(wifi_settings.pass, 0, sizeof(wifi_settings.pass));
				strcpy(wifi_settings.pass, arg);
				settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
				Serial.println("[MSG:WiFi password saved]");
				return STATUS_OK;
			}
		}
#endif
		return STATUS_INVALID_STATEMENT;
	}
#endif

	bool esp32_wifi_clientok(void)
	{
#ifdef ENABLE_WIFI
		static uint32_t next_info = 0;
		static bool connected = false;

		if ((WiFi.status() != WL_CONNECTED))
		{
			connected = false;
			if (next_info > mcu_millis())
			{
				return false;
			}
			next_info = mcu_millis() + 30000;
			Serial.println("[MSG:Disconnected from WiFi]");
			return false;
		}

		if (!connected)
		{
			connected = true;
			Serial.println("[MSG:Connected to WiFi]");
			Serial.print("[MSG: AP>");
			Serial.print(WiFi.SSID());
			Serial.println("]");
			Serial.print("[MSG: PI>");
			Serial.print(WiFi.localIP());
			Serial.println("]");
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
			serverClient = server.available();
			serverClient.println("[MSG: New client connected]\r\n");
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

	void esp32_uart_init(int baud)
	{
		Serial.begin(baud);
#ifdef ENABLE_WIFI
		WiFi.setSleep(WIFI_PS_NONE);
		WiFi.begin();
		server.begin();
		server.setNoDelay(true); /*
		 httpUpdater.setup(&httpServer, update_path, update_username, update_password);
		 httpServer.begin();*/

		wifi_settings_offset = settings_register_external_setting(sizeof(wifi_settings_t));
		Serial.println("offset");
		Serial.println(wifi_settings_offset);
		if (settings_load(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t)))
		{
			// settings_erase(wifi_settings_offset, sizeof(wifi_settings_t));
			// memset(&wifi_settings, 0, sizeof(wifi_settings_t));
		}
#endif
#ifdef ENABLE_BLUETOOTH
		SerialBT.begin("ESP32");
		Serial.println("[Bluetooth Started! Ready to pair...]");
#endif
		esp32_tx_buffer_counter = 0;
	}

	void esp32_uart_flush(void)
	{
		Serial.println(esp32_tx_buffer);
		Serial.flush();
#ifdef ENABLE_WIFI
		if (esp32_wifi_clientok())
		{
			serverClient.println(esp32_tx_buffer);
			serverClient.flush();
		}
#endif
#ifdef ENABLE_BLUETOOTH
		if (SerialBT.hasClient())
		{
			SerialBT.println(esp32_tx_buffer);
			SerialBT.flush();
		}
#endif
		esp32_tx_buffer_counter = 0;
	}

	unsigned char esp32_uart_read(void)
	{
		if (Serial.available() > 0)
		{
			return (unsigned char)Serial.read();
		}

#ifdef ENABLE_WIFI
		if (esp32_wifi_clientok())
		{
			if (serverClient.available() > 0)
			{
				return (unsigned char)serverClient.read();
			}
		}
#endif

#ifdef ENABLE_BLUETOOTH
		if (SerialBT.hasClient())
		{
			return (unsigned char)SerialBT.read();
		}
#endif

		return (unsigned char)0;
	}

	void esp32_uart_write(char c)
	{
		switch (c)
		{
		case '\n':
		case '\r':
			if (esp32_tx_buffer_counter)
			{
				esp32_tx_buffer[esp32_tx_buffer_counter] = 0;
				esp32_uart_flush();
			}
			break;
		default:
			if (esp32_tx_buffer_counter >= (ESP32_BUFFER_SIZE - 1))
			{
				esp32_tx_buffer[esp32_tx_buffer_counter] = 0;
				esp32_uart_flush();
			}

			esp32_tx_buffer[esp32_tx_buffer_counter++] = c;
			break;
		}
	}

	bool esp32_uart_rx_ready(void)
	{
		bool wifiready = false;
#ifdef ENABLE_WIFI
		if (esp32_wifi_clientok())
		{
			wifiready = (serverClient.available() > 0);
		}
#endif

		bool btready = false;
#ifdef ENABLE_BLUETOOTH
		btready = (SerialBT.available() > 0);
#endif
		return ((Serial.available() > 0) || wifiready || btready);
	}

	bool esp32_uart_tx_ready(void)
	{
		return (esp32_tx_buffer_counter != ESP32_BUFFER_SIZE);
	}

	void esp32_uart_process(void)
	{
		while (Serial.available() > 0)
		{
			esp_task_wdt_reset();
			mcu_com_rx_cb((unsigned char)Serial.read());
		}

#ifdef ENABLE_BLUETOOTH
		if (SerialBT.hasClient())
		{
			while (SerialBT.available() > 0)
			{
				esp_task_wdt_reset();
				mcu_com_rx_cb((unsigned char)SerialBT.read());
			}
		}
#endif

#ifdef ENABLE_WIFI
		// httpServer.handleClient();

		if (esp32_wifi_clientok())
		{
			while (serverClient.available() > 0)
			{
				esp_task_wdt_reset();
				mcu_com_rx_cb((unsigned char)serverClient.read());
			}
		}
#endif
	}
}

#endif
