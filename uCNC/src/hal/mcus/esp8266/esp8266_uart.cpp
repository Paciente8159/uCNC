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

#include "../../../../cnc_config.h"
#include "../mcu.h"
#ifdef ESP8266
#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef ENABLE_WIFI
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>

#ifndef WIFI_PORT
#define WIFI_PORT 23
#endif

#ifndef WIFI_USER
#define WIFI_USER "admin"
#endif

#ifndef WIFI_PASS
#define WIFI_PASS "pass"
#endif

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
const char *update_path = "/firmware";
const char *update_username = WIFI_USER;
const char *update_password = WIFI_PASS;
#define MAX_SRV_CLIENTS 1
WiFiServer server(WIFI_PORT);
WiFiClient serverClient;
WiFiManager wifiManager;
#endif

#ifndef ESP8266_BUFFER_SIZE
#define ESP8266_BUFFER_SIZE 255
#endif

static char esp8266_tx_buffer[ESP8266_BUFFER_SIZE];
static uint8_t esp8266_tx_buffer_counter;

extern "C"
{
	bool esp8266_wifi_clientok(void)
	{
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

		return false;
	}

	void esp8266_uart_init(int baud)
	{
		Serial.begin(baud);
#ifdef WIFI_DEBUG
		wifiManager.setDebugOutput(true);
#else
		wifiManager.setDebugOutput(false);
#endif
		wifiManager.setConfigPortalBlocking(false);
		wifiManager.setConfigPortalTimeout(30);
		if (!wifiManager.autoConnect("ESP8266"))
		{
			Serial.println("[MSG: WiFi manager up]");
			Serial.println("[MSG: Setup page @ 192.168.4.1]");
		}

		server.begin();
		server.setNoDelay(true);
		httpUpdater.setup(&httpServer, update_path, update_username, update_password);
		httpServer.begin();
		WiFi.setSleepMode(WIFI_NONE_SLEEP);

		esp8266_tx_buffer_counter = 0;
	}

	void esp8266_uart_flush(void)
	{
		Serial.println(esp8266_tx_buffer);
		Serial.flush();
		if (esp8266_wifi_clientok())
		{
			serverClient.println(esp8266_tx_buffer);
			serverClient.flush();
		}
		esp8266_tx_buffer_counter = 0;
	}

	unsigned char esp8266_uart_read(void)
	{
		return (unsigned char)Serial.read();
	}

	void esp8266_uart_write(char c)
	{
		switch (c)
		{
		case '\n':
		case '\r':
			if (esp8266_tx_buffer_counter)
			{
				esp8266_tx_buffer[esp8266_tx_buffer_counter] = 0;
				esp8266_uart_flush();
			}
			break;
		default:
			if (esp8266_tx_buffer_counter >= (ESP8266_BUFFER_SIZE - 1))
			{
				esp8266_tx_buffer[esp8266_tx_buffer_counter] = 0;
				esp8266_uart_flush();
			}

			esp8266_tx_buffer[esp8266_tx_buffer_counter++] = c;
			break;
		}
	}

	bool esp8266_uart_rx_ready(void)
	{
		bool wifiready = false;
		if (esp8266_wifi_clientok())
		{
			wifiready = (serverClient.available() > 0);
		}

		return ((Serial.available() > 0) || wifiready);
	}

	bool esp8266_uart_tx_ready(void)
	{
		return (esp8266_tx_buffer_counter != ESP8266_BUFFER_SIZE);
	}

	void esp8266_uart_process(void)
	{
		while (Serial.available() > 0)
		{
			system_soft_wdt_feed();
			mcu_com_rx_cb((unsigned char)Serial.read());
		}

		wifiManager.process();
		httpServer.handleClient();
		if (esp8266_wifi_clientok())
		{
			while (serverClient.available() > 0)
			{
				system_soft_wdt_feed();
				mcu_com_rx_cb((unsigned char)serverClient.read());
			}
		}
	}
}

#endif
