/*
Name: esp8266_wifi.cpp
Description: Contains all Arduino ESP8266 C++ to C functions used by WiFi in µCNC.

Copyright: Copyright (c) João Martins
Author: João Martins
Date: 24-06-2022

µCNC is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version. Please see <http://www.gnu.org/licenses/>

µCNC is distributed WITHOUT ANY WARRANTY;
Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the	GNU General Public License for more details.
*/

#include "../../../../cnc_config.h"
#ifdef ESP8266
#ifdef ENABLE_WIFI
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef WIFI_BUFFER_SIZE
#define WIFI_BUFFER_SIZE 255
#endif

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
typedef struct wifi_rxbuffer_
{
	char buffer[WIFI_BUFFER_SIZE];
	uint8_t len;
	uint8_t current;
} wifi_rxbuffer_t;

static wifi_rxbuffer_t wifi_rx_buffer;
static wifi_rxbuffer_t wifi_tx_buffer;

extern "C"
{
	bool esp8266_wifi_clientok(void)
{
	if (server.hasClient())
	{
		if (serverClient && serverClient.connected())
		{
			serverClient.stop();
			//flushes serial
			while(Serial.available()){
				Serial.read();
			}
		}
		serverClient = server.available();
#ifdef WIFI_DEBUG
		Serial.println("[MSG: New client accepted]");
#endif
		serverClient.write("[MSG: New client connected]\r\n");
		return false;
	}
	else if (serverClient && serverClient.connected())
	{
		return true;
	}

	return false;
}

	void esp8266_wifi_init(int baud)
	{
#ifdef WIFI_DEBUG
		Serial.setRxBufferSize(1024);
		Serial.begin(baud);
		wifiManager.setDebugOutput(true);
#else
		wifiManager.setDebugOutput(false);
#endif
		wifiManager.autoConnect("ESP8266");
#ifdef WIFI_DEBUG
		Serial.println("[MSG: WiFi manager up]");
		Serial.println("[MSG: Setup page @ 192.168.4.1]");
#endif
		server.begin();
		server.setNoDelay(true);
		httpUpdater.setup(&httpServer, update_path, update_username, update_password);
		httpServer.begin();
		WiFi.setSleepMode(WIFI_NONE_SLEEP);

		memset(wifi_rx_buffer.buffer, 0, WIFI_BUFFER_SIZE);
		wifi_rx_buffer.len = 0;
		wifi_rx_buffer.current = 0;

		memset(wifi_tx_buffer.buffer, 0, WIFI_BUFFER_SIZE);
		wifi_tx_buffer.len = 0;
		wifi_tx_buffer.current = 0;
	}

	char esp8266_wifi_read(void)
	{
		if (wifi_rx_buffer.len != 0 && wifi_rx_buffer.len > wifi_rx_buffer.current)
		{
			return wifi_rx_buffer.buffer[wifi_rx_buffer.current++];
		}

		if (esp8266_wifi_clientok())
		{
			size_t rxlen = serverClient.available();
			if (rxlen > 0)
			{
				serverClient.readBytes(wifi_rx_buffer.buffer, rxlen);
				wifi_rx_buffer.len = rxlen;
				wifi_rx_buffer.current = 1;
				return wifi_rx_buffer.buffer[0];
			}
		}

		return 0;
	}

	void esp8266_wifi_write(char c)
	{
		if (esp8266_wifi_clientok())
		{
			wifi_tx_buffer.buffer[wifi_tx_buffer.len] = c;
			wifi_tx_buffer.len++;
			if (c == '\n')
			{
				serverClient.write(wifi_tx_buffer.buffer, (size_t)wifi_tx_buffer.len);
				memset(wifi_tx_buffer.buffer, 0, WIFI_BUFFER_SIZE);
				wifi_tx_buffer.len = 0;
			}
		}
	}

	bool esp8266_wifi_rx_ready(void)
	{
		if (wifi_rx_buffer.len != 0 && wifi_rx_buffer.len > wifi_rx_buffer.current)
		{
			return true;
		}

		if (esp8266_wifi_clientok())
		{
			return (serverClient.available() != 0);
		}

		return false;
	}

	bool esp8266_wifi_tx_ready(void)
	{
		if (esp8266_wifi_clientok())
		{
			if (wifi_tx_buffer.len < WIFI_BUFFER_SIZE)
			{
				return true;
			}
		}

		return false;
	}

	void esp8266_wifi_flush(void)
	{
		if (esp8266_wifi_clientok())
		{
			serverClient.flush();
		}

		httpServer.handleClient();
	}
}

#endif
#endif
