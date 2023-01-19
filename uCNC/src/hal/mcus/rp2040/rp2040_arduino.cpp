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

#ifdef ARDUINO_ARCH_RP2040
#include <stdint.h>
#include <stdbool.h>
#include <Arduino.h>

/**
 *
 * This handles all communications via Serial USB, Serial UART and WiFi
 *
 * **/

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

#ifdef ENABLE_WIFI
	static bool rp2040_wifi_clientok(void)
	{
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
		return false;
	}
#endif

	void rp2040_uart_init(int baud)
	{
#ifdef MCU_HAS_USB
		Serial.begin(baud);
#endif
#ifdef MCU_HAS_UART
		COM_UART.setTX(TX_BIT);
		COM_UART.setRX(RX_BIT);
		COM_UART.begin(baud);
#endif
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
			protocol_send_feedback(" WiFi manager up");
			protocol_send_feedback(" Setup page @ 192.168.4.1");
		}
		else
		{
			protocol_send_ip(WiFi.localIP());
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
#ifdef MCU_HAS_USB
		Serial.println(rp2040_tx_buffer);
		Serial.flush();
#endif
#ifdef MCU_HAS_UART
		COM_UART.println(rp2040_tx_buffer);
		COM_UART.flush();
#endif
#ifdef ENABLE_WIFI
		if (rp2040_wifi_clientok())
		{
			serverClient.println(rp2040_tx_buffer);
			serverClient.flush();
		}
#endif
		rp2040_tx_buffer_counter = 0;
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
#ifdef MCU_HAS_USB
		while (Serial.available() > 0)
		{
			mcu_com_rx_cb((unsigned char)Serial.read());
		}
#endif

#ifdef MCU_HAS_UART
		while (COM_UART.available() > 0)
		{
			mcu_com_rx_cb((unsigned char)COM_UART.read());
		}
#endif

#ifdef ENABLE_WIFI
		wifiManager.process();
		httpServer.handleClient();
		if (rp2040_wifi_clientok())
		{
			while (serverClient.available() > 0)
			{
				mcu_com_rx_cb((unsigned char)serverClient.read());
			}
		}
#endif
	}
}

/**
 *
 * This handles EEPROM simulation on flash memory
 *
 * **/

#ifndef RAM_ONLY_SETTINGS
#include <EEPROM.h>
extern "C"
{
#include "../../../cnc.h"
	static void rp2040_eeprom_init(int size)
	{
		EEPROM.begin(size);
	}

	static uint8_t rp2040_eeprom_read(uint16_t address)
	{
		return EEPROM.read(address);
	}

	static void rp2040_eeprom_write(uint16_t address, uint8_t value)
	{
		EEPROM.write(address, value);
	}

	static void rp2040_eeprom_flush(void)
	{
		if (!EEPROM.commit())
		{
			protocol_send_feedback(" EEPROM write error");
		}
	}
}
#endif

/**
 *
 * This handles SPI communications
 *
 * **/

#ifdef MCU_HAS_SPI
#include <SPI.h>
extern "C"
{
	static void rp2040_spi_config(uint8_t mode, uint32_t freq)
	{
		COM_SPI.setRX(SPI_SDI);
		COM_SPI.setTX(SPI_SDO);
		COM_SPI.setSCK(SPI_CLK);
		COM_SPI.setCS(SPI_CS);
		COM_SPI.end();
		COM_SPI.begin();
		COM_SPI.beginTransaction(SPISettings(freq, 1 /*MSBFIRST*/, mode));
	}

	static uint8_t rp2040_spi_xmit(uint8_t data)
	{
		return COM_SPI.transfer(data);
	}
}

#endif

/**
 *
 * This handles I2C communications
 *
 * **/

#ifdef MCU_HAS_I2C
#include <Wire.h>
extern "C"
{
	static void rp2040_i2c_init(uint32_t freq)
	{
		COM_I2C.setSDA(I2C_DATA);
		COM_I2C.setSCL(I2C_CLK);
		COM_I2C.setClock(freq);
		COM_I2C.begin();
	}

	static void rp2040_spi_config(uint32_t freq)
	{
		COM_I2C.setClock(freq);
	}

	static uint8_t rp2040_i2c_write(uint8_t data, bool send_start, bool send_stop)
	{
		if (send_start)
		{
			// init
			COM_I2C.beginTransmission(data);
			return 1;
		}

		if (send_stop)
		{
			COM_I2C.endTransmission();
			return 1;
		}

		COM_I2C.write(data);

		return 1;
	}

	static uint8_t rp2040_i2c_read(bool with_ack, bool send_stop)
	{
		uint8_t c = 0;
		if (COM_I2C.available() <= 0)
		{
			return 0;
		}

		c = COM_I2C.read();

		if (send_stop)
		{
			COM_I2C.endTransmission();
		}

		return c;
	}
}
#endif

#endif
