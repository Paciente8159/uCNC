/*
	Name: esp32_arduino.cpp
	Description: Contains all Arduino ESP32 C++ to C functions used by µCNC.

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
#include "../../../../cnc_config.h"

#ifndef BT_ID_MAX_LEN
#define BT_ID_MAX_LEN 32
#endif

#ifndef WIFI_SSID_MAX_LEN
#define WIFI_SSID_MAX_LEN 32
#endif

#define ARG_MAX_LEN MAX(WIFI_SSID_MAX_LEN, BT_ID_MAX_LEN)

#ifdef ENABLE_BLUETOOTH
#include <BluetoothSerial.h>
BluetoothSerial SerialBT;

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
	uint8_t mcu_custom_grbl_cmd(char *grbl_cmd_str, uint8_t grbl_cmd_len, char next_char)
	{
		char str[TX_BUFFER_SIZE];
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
				SerialBT.begin(BOARD_NAME);
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
					sprintf(str, "%d) %s\tSignal:  %ddBm", netid, WiFi.SSID(netid).c_str(), WiFi.RSSI(netid));
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
				if (len > WIFI_SSID_MAX_LEN)
				{
					protocol_send_feedback("WiFi pass is too long");
				}
				memset(wifi_settings.pass, 0, sizeof(wifi_settings.pass));
				strcpy(wifi_settings.pass, arg);
				protocol_send_feedback("WiFi password modified");
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
		static uint32_t next_info = 30000;
		static bool connected = false;
		char str[TX_BUFFER_SIZE];

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

	void esp32_wifi_bt_init(void)
	{
#ifdef ENABLE_WIFI
#ifndef ENABLE_BLUETOOTH
		WiFi.setSleep(WIFI_PS_NONE);
#endif
		wifi_settings_offset = settings_register_external_setting(sizeof(wifi_settings_t));
		if (settings_load(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t)))
		{
			settings_erase(wifi_settings_offset, sizeof(wifi_settings_t));
			memset(&wifi_settings, 0, sizeof(wifi_settings_t));
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
#ifdef ENABLE_BLUETOOTH
		bt_settings_offset = settings_register_external_setting(1);
		if (settings_load(bt_settings_offset, &bt_on, 1))
		{
			settings_erase(bt_settings_offset, 1);
			bt_on = 0;
		}

		if (bt_on)
		{
			SerialBT.begin(BOARD_NAME);
		}
#endif
	}

	void esp32_wifi_bt_flush(char *buffer)
	{
#ifdef ENABLE_WIFI
		if (esp32_wifi_clientok())
		{
			serverClient.println(buffer);
			serverClient.flush();
		}
#endif
#ifdef ENABLE_BLUETOOTH
		if (SerialBT.hasClient())
		{
			SerialBT.println(buffer);
			SerialBT.flush();
		}
#endif
	}

	unsigned char esp32_wifi_bt_read(void)
	{
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

	bool esp32_wifi_bt_rx_ready(void)
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
		return (wifiready || btready);
	}

	void esp32_wifi_bt_process(void)
	{
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
		if (esp32_wifi_clientok())
		{
			while (serverClient.available() > 0)
			{
				esp_task_wdt_reset();
				mcu_com_rx_cb((unsigned char)serverClient.read());
			}
		}

		httpServer.handleClient();
#endif
	}

#ifdef MCU_HAS_I2C
#include <Wire.h>

#if (I2C_ADDRESS != 0)
	static uint8_t mcu_i2c_buffer_len;
	static uint8_t mcu_i2c_buffer[I2C_SLAVE_BUFFER_SIZE];
	void esp32_i2c_onreceive(int len)
	{
		uint8_t l = I2C_REG.readBytes(mcu_i2c_buffer, len);
		mcu_i2c_buffer_len = l;
		mcu_i2c_slave_cb(mcu_i2c_buffer, l);
	}

	void esp32_i2c_onrequest(void)
	{
		I2C_REG.write(mcu_i2c_buffer, mcu_i2c_buffer_len);
	}

#endif

	void mcu_i2c_config(uint32_t frequency)
	{
#if (I2C_ADDRESS == 0)
		I2C_REG.begin(I2C_DATA_BIT, I2C_CLK_BIT, frequency);
#else
		I2C_REG.onReceive(esp32_i2c_onreceive);
		I2C_REG.onRequest(esp32_i2c_onrequest);
		I2C_REG.begin(I2C_ADDRESS, I2C_DATA_BIT, I2C_CLK_BIT, frequency);
#endif
	}

	uint8_t mcu_i2c_send(uint8_t address, uint8_t *data, uint8_t datalen)
	{
		I2C_REG.beginTransmission(address);
		I2C_REG.write(data, datalen);
		return (I2C_REG.endTransmission(true) == 0) ? I2C_OK : I2C_NOTOK;
	}

	uint8_t mcu_i2c_receive(uint8_t address, uint8_t *data, uint8_t datalen)
	{
		if (I2C_REG.requestFrom(address, datalen) == datalen)
		{
			I2C_REG.readBytes(data, datalen);
			return I2C_OK;
		}

		return I2C_NOTOK;
	}
#endif
}

#endif
