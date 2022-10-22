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

#ifdef ENABLE_BLUETOOTH
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;
#endif

#ifdef ENABLE_WIFI
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <HTTPUpdateServer.h>

#ifndef WIFI_PORT
#define WIFI_PORT 23
#endif

#ifndef WIFI_USER
#define WIFI_USER "admin"
#endif

#ifndef WIFI_PASS
#define WIFI_PASS "pass"
#endif

WebServer httpServer(80);
HTTPUpdateServer httpUpdater;
const char *update_path = "/firmware";
const char *update_username = WIFI_USER;
const char *update_password = WIFI_PASS;
#define MAX_SRV_CLIENTS 1
WiFiServer server(WIFI_PORT);
WiFiClient serverClient;
WiFiManager wifiManager;
#endif

#ifndef ESP32_BUFFER_SIZE
#define ESP32_BUFFER_SIZE 255
#endif

static char esp32_tx_buffer[ESP32_BUFFER_SIZE];
static uint8_t esp32_tx_buffer_counter;

extern "C"
{
	#include "../../../cnc.h"

	bool esp32_wifi_clientok(void)
	{
#ifdef ENABLE_WIFI
		static bool connected = false;
		static bool process_busy = false;
		static uint32_t next_info = 0;

		if (WiFi.status() != WL_CONNECTED && next_info < mcu_millis())
		{
			next_info = mcu_millis() + 30000;
			connected = false;
			if (process_busy)
			{
				return false;
			}
			process_busy = true;
			Serial.println("[MSG:Disconnected from WiFi]");
			process_busy = false;
			return false;
		}

		if (!connected)
		{
			connected = true;
			Serial.println("[MSG: WiFi AP connected]");
			Serial.print("[MSG: Board IP @ ");
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
#ifdef WIFI_DEBUG
		wifiManager.setDebugOutput(true);
#else
		wifiManager.setDebugOutput(false);
#endif
		wifiManager.setConfigPortalBlocking(false);
		wifiManager.setConfigPortalTimeout(30);
		if (!wifiManager.autoConnect("ESP32"))
		{
			Serial.println("[MSG: WiFi manager up]");
			Serial.println("[MSG: Setup page @ 192.168.4.1]");
		}
		server.begin();
		server.setNoDelay(true);
		httpUpdater.setup(&httpServer, update_path, update_username, update_password);
		httpServer.begin();
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

		wifiManager.process();
		httpServer.handleClient();

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
