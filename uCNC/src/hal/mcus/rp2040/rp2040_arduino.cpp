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
#include <string.h>
#include "../../../cnc.h"

/**
 *
 * This handles all communications via Serial USB, Serial UART and WiFi
 *
 * **/

#if (defined(ENABLE_WIFI) || defined(ENABLE_BLUETOOTH))
#ifndef WIFI_SSID_MAX_LEN
#define WIFI_SSID_MAX_LEN 32
#endif

#ifndef WIFI_PASS_MAX_LEN
#define WIFI_PASS_MAX_LEN 32
#endif

#define ARG_MAX_LEN MAX(WIFI_SSID_MAX_LEN, WIFI_PASS_MAX_LEN)

#ifdef ENABLE_BLUETOOTH
#include <SerialBT.h>

uint8_t bt_on;
uint16_t bt_settings_offset;
#endif

#ifdef ENABLE_WIFI
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPUpdateServer.h>

#ifndef WIFI_PORT
#define WIFI_PORT 23
#endif

#ifndef WIFI_USER
#define WIFI_USER "admin\0"
#endif

#ifndef WIFI_PASS
#define WIFI_PASS "pass\0"
#endif

WebServer httpServer(80);
HTTPUpdateServer httpUpdater;
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
	char pass[WIFI_PASS_MAX_LEN];
} wifi_settings_t;

uint16_t wifi_settings_offset;
wifi_settings_t wifi_settings;
#endif

#ifdef BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
uint8_t mcu_custom_grbl_cmd(char *grbl_cmd_str, uint8_t grbl_cmd_len, char next_char)
{
	char str[128];
	char arg[ARG_MAX_LEN];
	char has_arg = (next_char == '=');
	memset(arg, 0, sizeof(arg));
	if (has_arg)
	{
		char c = serial_getc();
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

#ifdef ENABLE_BLUETOOTH
	if (!strncmp(grbl_cmd_str, "BTH", 3))
	{
		if (!strcmp(&grbl_cmd_str[3], "ON"))
		{
			SerialBT.begin(BAUDRATE, SERIAL_8N1);
			protocol_send_feedback("Bluetooth enabled");
			bt_on = 1;
			settings_save(bt_settings_offset, &bt_on, 1);

			return STATUS_OK;
		}

		if (!strcmp(&grbl_cmd_str[3], "OFF"))
		{
			SerialBT.end();
			protocol_send_feedback("Bluetooth disabled");
			bt_on = 0;
			settings_save(bt_settings_offset, &bt_on, 1);

			return STATUS_OK;
		}
	}
#endif
#ifdef ENABLE_WIFI
	if (!strncmp(grbl_cmd_str, "WIFI", 4))
	{
		if (!strcmp(&grbl_cmd_str[4], "ON"))
		{
			WiFi.disconnect();
			switch (wifi_settings.wifi_mode)
			{
			case 1:
				WiFi.mode(WIFI_STA);
				WiFi.begin(wifi_settings.ssid, wifi_settings.pass);
				protocol_send_feedback("Trying to connect to WiFi");
				break;
			case 2:
				WiFi.mode(WIFI_AP);
				WiFi.softAP(BOARD_NAME, wifi_settings.pass);
				protocol_send_feedback("AP started");
				protocol_send_feedback("SSID>" BOARD_NAME);
				sprintf(str, "IP>%s", WiFi.softAPIP().toString().c_str());
				protocol_send_feedback(str);
				break;
			default:
				WiFi.mode(WIFI_AP_STA);
				WiFi.begin(wifi_settings.ssid, wifi_settings.pass);
				protocol_send_feedback("Trying to connect to WiFi");
				WiFi.softAP(BOARD_NAME, wifi_settings.pass);
				protocol_send_feedback("AP started");
				protocol_send_feedback("SSID>" BOARD_NAME);
				sprintf(str, "IP>%s", WiFi.softAPIP().toString().c_str());
				protocol_send_feedback(str);
				break;
			}

			wifi_settings.wifi_on = 1;
			settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
			return STATUS_OK;
		}

		if (!strcmp(&grbl_cmd_str[4], "OFF"))
		{
			WiFi.disconnect();
			wifi_settings.wifi_on = 0;
			settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
			return STATUS_OK;
		}

		if (!strcmp(&grbl_cmd_str[4], "SSID"))
		{
			if (has_arg)
			{
				uint8_t len = strlen(arg);
				if (len > WIFI_SSID_MAX_LEN)
				{
					protocol_send_feedback("WiFi SSID is too long");
				}
				memset(wifi_settings.ssid, 0, sizeof(wifi_settings.ssid));
				strcpy(wifi_settings.ssid, arg);
				settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
				protocol_send_feedback("WiFi SSID modified");
			}
			else
			{
				sprintf(str, "SSID>%s", wifi_settings.ssid);
				protocol_send_feedback(str);
			}
			return STATUS_OK;
		}

		if (!strcmp(&grbl_cmd_str[4], "SCAN"))
		{
			// Serial.println("[MSG:Scanning Networks]");
			protocol_send_feedback("Scanning Networks");
			int numSsid = WiFi.scanNetworks();
			if (numSsid == -1)
			{
				protocol_send_feedback("Failed to scan!");
				while (true)
					;
			}

			// print the list of networks seen:
			sprintf(str, "%d available networks", numSsid);
			protocol_send_feedback(str);

			// print the network number and name for each network found:
			for (int netid = 0; netid < numSsid; netid++)
			{
				sprintf(str, "%d) %s\tSignal:  %ddBm", netid, WiFi.SSID(netid), WiFi.RSSI(netid));
				protocol_send_feedback(str);
			}
			return STATUS_OK;
		}

		if (!strcmp(&grbl_cmd_str[4], "SAVE"))
		{
			settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
			protocol_send_feedback("WiFi settings saved");
			return STATUS_OK;
		}

		if (!strcmp(&grbl_cmd_str[4], "RESET"))
		{
			settings_erase(wifi_settings_offset, sizeof(wifi_settings_t));
			memset(&wifi_settings, 0, sizeof(wifi_settings_t));
			protocol_send_feedback("WiFi settings deleted");
			return STATUS_OK;
		}

		if (!strcmp(&grbl_cmd_str[4], "MODE"))
		{
			if (has_arg)
			{
				int mode = atoi(arg) - 1;
				if (mode >= 0)
				{
					wifi_settings.wifi_mode = mode;
				}
				else
				{
					protocol_send_feedback("Invalid value. STA+AP(1), STA(2), AP(3)");
				}
			}

			switch (wifi_settings.wifi_mode)
			{
			case 0:
				protocol_send_feedback("WiFi mode>STA+AP");
				break;
			case 1:
				protocol_send_feedback("WiFi mode>STA");
				break;
			case 2:
				protocol_send_feedback("WiFi mode>AP");
				break;
			}
			return STATUS_OK;
		}

		if (!strcmp(&grbl_cmd_str[4], "PASS") && has_arg)
		{
			uint8_t len = strlen(arg);
			if (len > WIFI_PASS_MAX_LEN)
			{
				protocol_send_feedback("WiFi pass is too long");
			}
			memset(wifi_settings.pass, 0, sizeof(wifi_settings.pass));
			strcpy(wifi_settings.pass, arg);
			protocol_send_feedback("WiFi password modified");
			return STATUS_OK;
		}

		if (!strcmp(&grbl_cmd_str[4], "IP"))
		{
			if (wifi_settings.wifi_on)
			{
				switch (wifi_settings.wifi_mode)
				{
				case 1:
					sprintf(str, "STA IP>%s", WiFi.softAPIP().toString().c_str());
					protocol_send_feedback(str);
					sprintf(str, "AP IP>%s", WiFi.softAPIP().toString().c_str());
					protocol_send_feedback(str);
					break;
				case 2:
					sprintf(str, "IP>%s", WiFi.softAPIP().toString().c_str());
					protocol_send_feedback(str);
					break;
				default:
					sprintf(str, "IP>%s", WiFi.softAPIP().toString().c_str());
					protocol_send_feedback(str);
					break;
				}
			}
			else
			{
				protocol_send_feedback("WiFi is off");
			}

			return STATUS_OK;
		}
	}
#endif
	return STATUS_INVALID_STATEMENT;
}
#endif

bool rp2040_wifi_clientok(void)
{
#ifdef ENABLE_WIFI
	static uint32_t next_info = 30000;
	static bool connected = false;
	char str[128];

	if (!wifi_settings.wifi_on)
	{
		return false;
	}

	if ((WiFi.status() != WL_CONNECTED))
	{
		connected = false;
		if (next_info > millis())
		{
			return false;
		}
		next_info = millis() + 30000;
		protocol_send_feedback("Disconnected from WiFi");
		return false;
	}

	if (!connected)
	{
		connected = true;
		protocol_send_feedback("Connected to WiFi");
		sprintf(str, "SSID>%s", wifi_settings.ssid);
		protocol_send_feedback(str);
		sprintf(str, "IP>%s", WiFi.localIP().toString().c_str());
		protocol_send_feedback(str);
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

void rp2040_wifi_bt_init(void)
{
#ifdef ENABLE_WIFI
	wifi_settings = {0};
	memcpy(wifi_settings.ssid, BOARD_NAME, strlen(BOARD_NAME));
	memcpy(wifi_settings.pass, WIFI_PASS, strlen(WIFI_PASS));

	wifi_settings_offset = settings_register_external_setting(sizeof(wifi_settings_t));
	if (settings_load(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t)))
	{
		settings_erase(wifi_settings_offset, sizeof(wifi_settings_t));
		memset(&wifi_settings, 0, sizeof(wifi_settings_t));
	}

	WiFi.begin(wifi_settings.ssid, wifi_settings.pass);
	if (!wifi_settings.wifi_on)
	{
		WiFi.disconnect();
	}
	server.begin();
	server.setNoDelay(true);
	httpUpdater.setup(&httpServer, update_path, update_username, update_password);
	httpServer.begin();
#endif
#ifdef ENABLE_BLUETOOTH
	bt_settings_offset = settings_register_external_setting(1);
	if (settings_load(bt_settings_offset, &bt_on, 1))
	{
		settings_erase(bt_settings_offset, 1);
		bt_on = 0;
	}

	if (bt_on)
	{
		SerialBT.begin(BAUDRATE, SERIAL_8N1);
	}
#endif
}

#ifdef MCU_HAS_WIFI
#ifndef WIFI_TX_BUFFER_SIZE
#define WIFI_TX_BUFFER_SIZE 64
#endif
DECL_BUFFER(uint8_t, wifi, WIFI_TX_BUFFER_SIZE);
void mcu_wifi_putc(uint8_t c)
{
	while (BUFFER_FULL(wifi))
	{
		mcu_wifi_flush();
	}
	BUFFER_ENQUEUE(wifi, &c);
}

void mcu_wifi_flush(void)
{
	if (rp2040_wifi_clientok())
	{
		while (!BUFFER_EMPTY(wifi))
		{
			uint8_t tmp[WIFI_TX_BUFFER_SIZE];
			uint8_t r;

			BUFFER_READ(wifi, tmp, WIFI_TX_BUFFER_SIZE, r);
			serverClient.write(tmp, r);
		}
	}
	else
	{
		// no client (discard)
		BUFFER_CLEAR(wifi);
	}
}
#endif

#ifdef MCU_HAS_BLUETOOTH
#ifndef BLUETOOTH_TX_BUFFER_SIZE
#define BLUETOOTH_TX_BUFFER_SIZE 64
#endif
DECL_BUFFER(uint8_t, bluetooth, BLUETOOTH_TX_BUFFER_SIZE);
void mcu_bt_putc(uint8_t c)
{
	while (BUFFER_FULL(bluetooth))
	{
		mcu_bt_flush();
	}
	BUFFER_ENQUEUE(bluetooth, &c);
}

void mcu_bt_flush(void)
{
	while (!BUFFER_EMPTY(bluetooth))
	{
		uint8_t tmp[BLUETOOTH_TX_BUFFER_SIZE];
		uint8_t r;

		BUFFER_READ(bluetooth, tmp, BLUETOOTH_TX_BUFFER_SIZE, r);
		SerialBT.write(tmp, r);
		SerialBT.flush();
	}
}
#endif

unsigned char rp2040_wifi_bt_read(void)
{
#ifdef ENABLE_WIFI
	if (rp2040_wifi_clientok())
	{
		if (serverClient.available() > 0)
		{
			return (unsigned char)serverClient.read();
		}
	}
#endif

#ifdef ENABLE_BLUETOOTH
	return (unsigned char)SerialBT.read();
#endif

	return (unsigned char)0;
}

bool rp2040_wifi_bt_rx_ready(void)
{
	bool wifiready = false;
#ifdef ENABLE_WIFI
	if (rp2040_wifi_clientok())
	{
		wifiready = (serverClient.available() > 0);
	}
#endif

	bool btready = false;
#ifdef ENABLE_BLUETOOTH
	btready = (SerialBT.available() > 0);
#endif

	return (wifiready || btready);
}

void rp2040_wifi_bt_process(void)
{
#ifdef ENABLE_WIFI
	if (rp2040_wifi_clientok())
	{
		while (serverClient.available() > 0)
		{
#ifndef DETACH_WIFI_FROM_MAIN_PROTOCOL
			mcu_com_rx_cb((uint8_t)serverClient.read());
#else
			mcu_wifi_rx_cb((uint8_t)serverClient.read());
#endif
		}
	}

	httpServer.handleClient();
#endif

#ifdef ENABLE_BLUETOOTH
	while (SerialBT.available() > 0)
	{
#ifndef DETACH_BLUETOOTH_FROM_MAIN_PROTOCOL
		mcu_com_rx_cb((uint8_t)SerialBT.read());
#else
		mcu_bt_rx_cb((uint8_t)SerialBT.read());
#endif
	}
#endif
}

#endif

extern "C"
{
	void rp2040_uart_init(int baud)
	{
#ifdef MCU_HAS_USB
		Serial.begin(baud);
#endif
#ifdef MCU_HAS_UART
		COM_UART.setTX(TX_BIT);
		COM_UART.setRX(RX_BIT);
		COM_UART.begin(BAUDRATE);
#endif
#ifdef MCU_HAS_UART2
		COM2_UART.setTX(TX2_BIT);
		COM2_UART.setRX(RX2_BIT);
		COM2_UART.begin(BAUDRATE2);
#endif
#if (defined(ENABLE_WIFI) || defined(ENABLE_BLUETOOTH))
		rp2040_wifi_bt_init();
#endif
	}

#ifdef MCU_HAS_USB
	void mcu_usb_putc(uint8_t c)
	{
		Serial.write(c);
	}

	void mcu_usb_flush(void)
	{
		Serial.flush();
	}
#endif

#ifdef MCU_HAS_UART
#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE 64
#endif
	DECL_BUFFER(uint8_t, uart, UART_TX_BUFFER_SIZE);
	void mcu_uart_putc(uint8_t c)
	{
		while (BUFFER_FULL(uart))
		{
			mcu_uart_flush();
		}
		BUFFER_ENQUEUE(uart, &c);
	}

	void mcu_uart_flush(void)
	{
		while (!BUFFER_EMPTY(uart))
		{
			uint8_t tmp[UART_TX_BUFFER_SIZE];
			uint8_t r = 0;

			BUFFER_READ(uart, tmp, UART_TX_BUFFER_SIZE, r);
			COM_UART.write(tmp, r);
			COM_UART.flush();
		}
	}
#endif

#ifdef MCU_HAS_UART2
#ifndef UART2_TX_BUFFER_SIZE
#define UART2_TX_BUFFER_SIZE 64
#endif
	DECL_BUFFER(uint8_t, uart2, UART2_TX_BUFFER_SIZE);
	void mcu_uart2_putc(uint8_t c)
	{
		while (BUFFER_FULL(uart2))
		{
			mcu_uart2_flush();
		}
		BUFFER_ENQUEUE(uart2, &c);
	}

	void mcu_uart2_flush(void)
	{
		while (!BUFFER_EMPTY(uart2))
		{
			uint8_t tmp[UART2_TX_BUFFER_SIZE];
			uint8_t r;

			BUFFER_READ(uart2, tmp, UART2_TX_BUFFER_SIZE, r);
			COM2_UART.write(tmp, r);
			COM2_UART.flush();
		}
	}
#endif

	bool rp2040_uart_rx_ready(void)
	{
		bool wifiready = false;
#if (defined(ENABLE_WIFI) || defined(ENABLE_BLUETOOTH))
		if (rp2040_wifi_clientok())
		{
			wifiready = (rp2040_wifi_bt_rx_ready() > 0);
		}
#endif
		return ((Serial.available() > 0) || wifiready);
	}

	void rp2040_uart_process(void)
	{
#ifdef MCU_HAS_USB
		while (Serial.available() > 0)
		{
#ifndef DETACH_USB_FROM_MAIN_PROTOCOL
			mcu_com_rx_cb((uint8_t)Serial.read());
#else
			mcu_usb_rx_cb((uint8_t)Serial.read());
#endif
		}
#endif

#ifdef MCU_HAS_UART
		while (COM_UART.available() > 0)
		{
#ifndef DETACH_UART_FROM_MAIN_PROTOCOL
			mcu_com_rx_cb((uint8_t)COM_UART.read());
#else
			mcu_uart_rx_cb((uint8_t)COM_UART.read());
#endif
		}
#endif

#ifdef MCU_HAS_UART2
		while (COM2_UART.available() > 0)
		{
#ifndef DETACH_UART2_FROM_MAIN_PROTOCOL
			mcu_com_rx_cb((uint8_t)COM2_UART.read());
#else
			mcu_uart2_rx_cb((uint8_t)COM2_UART.read());
#endif
		}
#endif

#if (defined(ENABLE_WIFI) || defined(ENABLE_BLUETOOTH))
		rp2040_wifi_bt_process();
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
	void rp2040_eeprom_init(int size)
	{
		EEPROM.begin(size);
	}

	uint8_t rp2040_eeprom_read(uint16_t address)
	{
		return EEPROM.read(address);
	}

	void rp2040_eeprom_write(uint16_t address, uint8_t value)
	{
		EEPROM.write(address, value);
	}

	void rp2040_eeprom_flush(void)
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
	void mcu_spi_config(uint8_t mode, uint32_t freq)
	{
		COM_SPI.setRX(SPI_SDI_BIT);
		COM_SPI.setTX(SPI_SDO_BIT);
		COM_SPI.setSCK(SPI_CLK_BIT);
		COM_SPI.setCS(SPI_CS_BIT);
		COM_SPI.end();
		COM_SPI.begin();
		COM_SPI.beginTransaction(SPISettings(freq, 1 /*MSBFIRST*/, mode));
	}

	uint8_t mcu_spi_xmit(uint8_t data)
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
#if (I2C_ADDRESS != 0)
	static uint8_t mcu_i2c_buffer_len;
	static uint8_t mcu_i2c_buffer[I2C_SLAVE_BUFFER_SIZE];
	void rp2040_i2c_onreceive(int len)
	{
		uint8_t l = I2C_REG.readBytes(mcu_i2c_buffer, len);
		mcu_i2c_slave_cb(mcu_i2c_buffer, &l);
		mcu_i2c_buffer_len = l;
	}

	void rp2040_i2c_onrequest(void)
	{
		I2C_REG.write(mcu_i2c_buffer, mcu_i2c_buffer_len);
	}

#endif

	void mcu_i2c_config(uint32_t frequency)
	{
		I2C_REG.setSDA(I2C_DATA_BIT);
		I2C_REG.setSCL(I2C_CLK_BIT);
#if I2C_ADDRESS == 0
		I2C_REG.setClock(frequency);
		I2C_REG.begin();
#else
		I2C_REG.onReceive(rp2040_i2c_onreceive);
		I2C_REG.onRequest(rp2040_i2c_onrequest);
		I2C_REG.begin(I2C_ADDRESS);
#endif
	}

	uint8_t mcu_i2c_send(uint8_t address, uint8_t *data, uint8_t datalen, bool release)
	{
		I2C_REG.beginTransmission(address);
		I2C_REG.write(data, datalen);
		return (I2C_REG.endTransmission(release) == 0) ? I2C_OK : I2C_NOTOK;
	}

	uint8_t mcu_i2c_receive(uint8_t address, uint8_t *data, uint8_t datalen, uint32_t ms_timeout)
	{
		if (I2C_REG.requestFrom(address, datalen) == datalen)
		{
			I2C_REG.readBytes(data, datalen);
			return I2C_OK;
		}

		return I2C_NOTOK;
	}
}
#endif

#endif
