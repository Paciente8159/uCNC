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
#include <Arduino.h>
#include "user_interface.h"
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

extern "C"
{
#include "../../../cnc.h"

	bool esp8266_wifi_clientok(void)
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
			serverClient = server.accept();
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

	void esp8266_uart_init(int baud)
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
		if (!wifiManager.autoConnect("ESP8266"))
		{
			Serial.println("[MSG: WiFi manager up]");
			Serial.println("[MSG: Setup page @ 192.168.4.1]");
		}
		else
		{
			Serial.print(WiFi.localIP());
		}

		server.begin();
		server.setNoDelay(true);
		httpUpdater.setup(&httpServer, update_path, update_username, update_password);
		httpServer.begin();
#endif
	}

	unsigned char esp8266_uart_read(void)
	{
		return (unsigned char)Serial.read();
	}

#ifdef MCU_HAS_UART
	void mcu_uart_putc(uint8_t c)
	{
#if defined(ENABLE_SYNC_TX) || defined(DETACH_UART_FROM_MAIN_PROTOCOL)
		Serial.write(c);
#endif
	}

	void mcu_uart_flush(void)
	{
#if !defined(ENABLE_SYNC_TX) && !defined(DETACH_UART_FROM_MAIN_PROTOCOL)
		if (mcu_uart_tx_tail != mcu_com_tx_head)
		{
			if (mcu_uart_tx_tail > mcu_com_tx_head)
			{
				Serial.write(&mcu_com_tx_buffer[mcu_uart_tx_tail], (TX_BUFFER_SIZE - mcu_uart_tx_tail));
				Serial.flush();
				mcu_uart_tx_tail = 0;
			}

			Serial.write(&mcu_com_tx_buffer[mcu_uart_tx_tail], (mcu_com_tx_head - mcu_uart_tx_tail));
			Serial.flush();
			mcu_uart_tx_tail = mcu_com_tx_head;
		}
#endif
	}
#endif

#ifdef MCU_HAS_WIFI
	void mcu_wifi_putc(uint8_t c)
	{
#ifdef ENABLE_WIFI
		if (esp8266_wifi_clientok())
		{
			serverClient.write(c);
		}
#endif
	}

	void mcu_wifi_flush(void)
	{
#ifdef ENABLE_WIFI
		if (esp8266_wifi_clientok())
		{
			serverClient.flush();
		}
#endif
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
			mcu_com_rx_cb((uint8_t)Serial.read());
#else
			mcu_wifi_rx_cb((uint8_t)Serial.read());
#endif
		}

#ifdef ENABLE_WIFI
		wifiManager.process();
		httpServer.handleClient();
		if (esp8266_wifi_clientok())
		{
			while (serverClient.available() > 0)
			{
				system_soft_wdt_feed();
#ifndef DETACH_WIFI_FROM_MAIN_PROTOCOL
				mcu_com_rx_cb((uint8_t)serverClient.read());
#else
				mcu_wifi_rx_cb((uint8_t)serverClient.read());
#endif
			}
		}
#endif
	}
}

#endif
