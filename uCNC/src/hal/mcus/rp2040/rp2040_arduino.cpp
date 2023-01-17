/*
	Name: rp2040_arduino.cpp
	Description: Contains all Arduino RP2040 C++ to C functions ports needed by µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 17-01-2023

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifdef RP2040
#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef ENABLE_WIFI
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPUpdate.h>

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

#ifndef RP2040_BUFFER_SIZE
#define RP2040_BUFFER_SIZE 255
#endif

static char rp2040_tx_buffer[RP2040_BUFFER_SIZE];
static uint8_t rp2040_tx_buffer_counter;

extern "C"
{
#include "../../../cnc.h"

	bool rp2040_wifi_clientok(void)
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

	void rp2040_uart_init(int baud)
	{
		Serial.begin(baud);
#ifdef ENABLE_WIFI
		WiFi.setSleepMode(WIFI_NONE_SLEEP);
#ifdef WIFI_DEBUG
		wifiManager.setDebugOutput(true);
#else
		wifiManager.setDebugOutput(false);
#endif
		wifiManager.setConfigPortalBlocking(false);
		wifiManager.setConfigPortalTimeout(30);
		if (!wifiManager.autoConnect("RP2040"))
		{
			Serial.println("[MSG: WiFi manager up]");
			Serial.println("[MSG: Setup page @ 192.168.4.1]");
		}
		else{
			Serial.print(WiFi.localIP());
		}

		server.begin();
		server.setNoDelay(true);
		httpUpdater.setup(&httpServer, update_path, update_username, update_password);
		httpServer.begin();
#endif
		rp2040_tx_buffer_counter = 0;
	}

	void rp2040_uart_flush(void)
	{
		Serial.println(rp2040_tx_buffer);
		Serial.flush();
#ifdef ENABLE_WIFI
		if (rp2040_wifi_clientok())
		{
			serverClient.println(rp2040_tx_buffer);
			serverClient.flush();
		}
#endif
		rp2040_tx_buffer_counter = 0;
	}

	unsigned char rp2040_uart_read(void)
	{
		return (unsigned char)Serial.read();
	}

	void rp2040_uart_write(char c)
	{
		switch (c)
		{
		case '\n':
		case '\r':
			if (rp2040_tx_buffer_counter)
			{
				rp2040_tx_buffer[rp2040_tx_buffer_counter] = 0;
				rp2040_uart_flush();
			}
			break;
		default:
			if (rp2040_tx_buffer_counter >= (RP2040_BUFFER_SIZE - 1))
			{
				rp2040_tx_buffer[rp2040_tx_buffer_counter] = 0;
				rp2040_uart_flush();
			}

			rp2040_tx_buffer[rp2040_tx_buffer_counter++] = c;
			break;
		}
	}

	bool rp2040_uart_rx_ready(void)
	{
		bool wifiready = false;
#ifdef ENABLE_WIFI
		if (rp2040_wifi_clientok())
		{
			wifiready = (serverClient.available() > 0);
		}
#endif
		return ((Serial.available() > 0) || wifiready);
	}

	bool rp2040_uart_tx_ready(void)
	{
		return (rp2040_tx_buffer_counter != RP2040_BUFFER_SIZE);
	}

	void rp2040_uart_process(void)
	{
		while (Serial.available() > 0)
		{
			system_soft_wdt_feed();
			mcu_com_rx_cb((unsigned char)Serial.read());
		}

#ifdef ENABLE_WIFI
		wifiManager.process();
		httpServer.handleClient();
		if (rp2040_wifi_clientok())
		{
			while (serverClient.available() > 0)
			{
				system_soft_wdt_feed();
				mcu_com_rx_cb((unsigned char)serverClient.read());
			}
		}
#endif
	}
}

#endif
