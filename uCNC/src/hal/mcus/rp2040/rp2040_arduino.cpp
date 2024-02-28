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

#if (defined(MCU_HAS_WIFI) || defined(ENABLE_BLUETOOTH))

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

#ifdef MCU_HAS_WIFI
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPUpdateServer.h>

#ifndef TELNET_PORT
#define TELNET_PORT 23
#endif

#ifndef WEBSERVER_PORT
#define WEBSERVER_PORT 80
#endif

#ifndef WEBSOCKET_PORT
#define WEBSOCKET_PORT 8080
#endif

#ifndef WEBSOCKET_MAX_CLIENTS
#define WEBSOCKET_MAX_CLIENTS 2
#endif

#ifndef WIFI_USER
#define WIFI_USER "admin\0"
#endif

#ifndef WIFI_PASS
#define WIFI_PASS "pass\0"
#endif

#ifndef OTA_URI
#define OTA_URI "/firmware"
#endif

#ifndef FS_WRITE_URI
#define FS_WRITE_URI "/fs"
#endif
#define FS_WRITE_GZ_SIZE 305
const char fs_write_page[305] PROGMEM = {0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0a, 0x55, 0x51, 0x3d, 0x4f, 0xc3, 0x30, 0x10, 0xdd, 0x91, 0xf8, 0x0f, 0x87, 0x67, 0x52, 0x43, 0x27, 0x84, 0xec, 0x2c, 0x85, 0x4a, 0x4c, 0x74, 0x68, 0x85, 0x18, 0x2f, 0xf6, 0xb5, 0xb1, 0xe4, 0xd8, 0x56, 0x72, 0x69, 0x55, 0x7e, 0x3d, 0x97, 0xa4, 0x03, 0x0c, 0xfe, 0x7a, 0xf7, 0xee, 0xdd, 0xd3, 0xb3, 0x79, 0x78, 0xfb, 0xdc, 0xec, 0xbf, 0x77, 0xef, 0xd0, 0x72, 0x17, 0x6b, 0x33, 0xed, 0x10, 0x31, 0x9d, 0xac, 0xa2, 0xa4, 0xe4, 0x4d, 0xe8, 0x6b, 0xd3, 0x11, 0x23, 0xb8, 0x16, 0xfb, 0x81, 0xd8, 0xaa, 0xc3, 0x7e, 0x5b, 0xbd, 0xa8, 0x1b, 0x9a, 0xb0, 0x23, 0xab, 0xce, 0x81, 0x2e, 0x25, 0xf7, 0xac, 0xc0, 0xe5, 0xc4, 0x94, 0x84, 0x75, 0x09, 0x9e, 0x5b, 0xeb, 0xe9, 0x1c, 0x1c, 0x55, 0xf3, 0xe3, 0x11, 0x42, 0x0a, 0x1c, 0x30, 0x56, 0x83, 0xc3, 0x48, 0xf6, 0x79, 0xf5, 0x24, 0x2a, 0x1c, 0x38, 0x52, 0xfd, 0x45, 0x0d, 0xec, 0x28, 0x79, 0x4c, 0x0c, 0x63, 0xf1, 0xc8, 0x64, 0xf4, 0x52, 0x31, 0x7a, 0xf1, 0xd0, 0x64, 0x7f, 0x15, 0x3f, 0xeb, 0x7f, 0xd4, 0xc3, 0x4c, 0x85, 0x6d, 0xee, 0x3b, 0xe1, 0xad, 0x6b, 0x73, 0x94, 0x1b, 0xa0, 0xe3, 0x90, 0x93, 0x55, 0xfa, 0x38, 0x28, 0x10, 0x97, 0x6d, 0xf6, 0x56, 0x95, 0x3c, 0x88, 0x3d, 0x4a, 0x8e, 0xaf, 0x45, 0x1c, 0x77, 0x63, 0xe4, 0x50, 0xb0, 0x67, 0x3d, 0xb5, 0x54, 0x22, 0x83, 0x62, 0x26, 0x62, 0x43, 0x11, 0x04, 0xb1, 0xea, 0x18, 0x22, 0x7d, 0xa4, 0x32, 0xb2, 0xaa, 0x37, 0x6d, 0xce, 0x03, 0x01, 0xc2, 0xea, 0xf4, 0x03, 0x13, 0xfe, 0x6a, 0xf4, 0xcc, 0xac, 0x4d, 0x98, 0x18, 0xb0, 0x48, 0x4e, 0x15, 0x05, 0xc1, 0xff, 0xed, 0xbd, 0xe5, 0xf3, 0x07, 0x40, 0xe7, 0xa8, 0x48, 0x3e, 0xa2, 0x25, 0x03, 0x9b, 0x5e, 0xd6, 0xc8, 0x9c, 0xd3, 0x4d, 0x64, 0x18, 0x9b, 0x2e, 0xc8, 0xcc, 0x43, 0x89, 0x19, 0xbd, 0xd1, 0x4b, 0x51, 0x52, 0x98, 0x6c, 0xca, 0xb1, 0xc4, 0xa0, 0xe7, 0xdf, 0xba, 0xbf, 0xfb, 0x05, 0x44, 0x67, 0x16, 0x56, 0xbf, 0x01, 0x00, 0x00};

WebServer web_server(WEBSERVER_PORT);
HTTPUpdateServer httpUpdater;
const char *update_username = WIFI_USER;
const char *update_password = WIFI_PASS;
#define MAX_SRV_CLIENTS 1
WiFiServer telnet_server(TELNET_PORT);
WiFiClient server_client;

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
uint8_t mcu_custom_grbl_cmd(uint8_t *grbl_cmd_str, uint8_t grbl_cmd_len, uint8_t next_char)
{
	uint8_t str[128];
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

#ifdef ENABLE_BLUETOOTH
	if (!strncmp((const char *)grbl_cmd_str, "BTH", 3))
	{
		if (!strcmp((const char *)&grbl_cmd_str[3], "ON"))
		{
			SerialBT.begin(BAUDRATE, SERIAL_8N1);
			protocol_send_feedback((const char *)"Bluetooth enabled");
			bt_on = 1;
			settings_save(bt_settings_offset, &bt_on, 1);

			return STATUS_OK;
		}

		if (!strcmp((const char *)&grbl_cmd_str[3], "OFF"))
		{
			SerialBT.end();
			protocol_send_feedback((const char *)"Bluetooth disabled");
			bt_on = 0;
			settings_save(bt_settings_offset, &bt_on, 1);

			return STATUS_OK;
		}
	}
#endif
#ifdef MCU_HAS_WIFI
	if (!strncmp((const char *)grbl_cmd_str, "WIFI", 4))
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
				sprintf((char *)str, "IP>%s", WiFi.softAPIP().toString().c_str());
				protocol_send_feedback((const char *)str);
				break;
			default:
				WiFi.mode(WIFI_AP_STA);
				WiFi.begin(wifi_settings.ssid, wifi_settings.pass);
				protocol_send_feedback((const char *)"Trying to connect to WiFi");
				WiFi.softAP(BOARD_NAME, wifi_settings.pass);
				protocol_send_feedback((const char *)"AP started");
				protocol_send_feedback((const char *)"SSID>" BOARD_NAME);
				sprintf((char *)str, "IP>%s", WiFi.softAPIP().toString().c_str());
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
				strcpy(wifi_settings.ssid, (const char *)arg);
				settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
				protocol_send_feedback((const char *)"WiFi SSID modified");
			}
			else
			{
				sprintf((char *)str, "SSID>%s", wifi_settings.ssid);
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
			sprintf((char *)str, "%d available networks", numSsid);
			protocol_send_feedback((const char *)str);

			// print the network number and name for each network found:
			for (int netid = 0; netid < numSsid; netid++)
			{
				sprintf((char *)str, "%d) %s\tSignal:  %ddBm", netid, WiFi.SSID(netid), WiFi.RSSI(netid));
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
				int mode = atoi((const char *)arg) - 1;
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
			if (len > WIFI_PASS_MAX_LEN)
			{
				protocol_send_feedback((const char *)"WiFi pass is too long");
			}
			memset(wifi_settings.pass, 0, sizeof(wifi_settings.pass));
			strcpy(wifi_settings.pass, (const char *)arg);
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
					sprintf((char *)str, "STA IP>%s", WiFi.localIP().toString().c_str());
					protocol_send_feedback((const char *)str);
					sprintf((char *)str, "AP IP>%s", WiFi.softAPIP().toString().c_str());
					protocol_send_feedback((const char *)str);
					break;
				case 2:
					sprintf((char *)str, "IP>%s", WiFi.localIP().toString().c_str());
					protocol_send_feedback((const char *)str);
					break;
				default:
					sprintf((char *)str, "IP>%s", WiFi.softAPIP().toString().c_str());
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

bool rp2040_wifi_clientok(void)
{
#ifdef MCU_HAS_WIFI
	static uint32_t next_info = 30000;
	static bool connected = false;
	uint8_t str[128];

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
		protocol_send_feedback((const char *)"Disconnected from WiFi");
		return false;
	}

	if (!connected)
	{
		connected = true;
		protocol_send_feedback((const char *)"Connected to WiFi");
		sprintf((char *)str, "SSID>%s", wifi_settings.ssid);
		protocol_send_feedback((const char *)str);
		sprintf((char *)str, "IP>%s", WiFi.localIP().toString().c_str());
		protocol_send_feedback((const char *)str);
	}

	if (telnet_server.hasClient())
	{
		if (server_client)
		{
			if (server_client.connected())
			{
				server_client.stop();
			}
		}
		server_client = telnet_server.available();
		server_client.println("[MSG:New client connected]");
		return false;
	}
	else if (server_client)
	{
		if (server_client.connected())
		{
			return true;
		}
	}
#endif
	return false;
}

#if defined(MCU_HAS_WIFI) && defined(MCU_HAS_ENDPOINTS)

#include "../../../modules/endpoint.h"
#define MCU_FLASH_FS_LITTLE_FS 1
#define MCU_FLASH_FS_SPIFFS 2

#ifndef MCU_FLASH_FS
#define MCU_FLASH_FS MCU_FLASH_FS_LITTLE_FS
#endif

#if (MCU_FLASH_FS == MCU_FLASH_FS_LITTLE_FS)
#include "FS.h"
#include <LittleFS.h>
#define FLASH_FS LittleFS
#elif (MCU_FLASH_FS == MCU_FLASH_FS_SPIFFS)
#include "FS.h"
#include <SPIFFS.h>
#define FLASH_FS SPIFFS
#endif
static File upload_file;

void fs_file_updater()
{
	if (web_server.uri() != FS_WRITE_URI || web_server.method() != HTTP_POST || !web_server.hasArg("update"))
	{
		return;
	}

	HTTPUpload &upload = web_server.upload();
	if (upload.status == UPLOAD_FILE_START)
	{
		String filename = upload.filename;
		if (web_server.hasArg("path"))
		{
			String path = web_server.arg("path");
			filename = path + ((!filename.startsWith("/") && !path.startsWith("/")) ? "/" : "") + filename;
		}
		if (!filename.startsWith("/"))
		{
			filename = "/" + filename;
		}
		upload_file = FLASH_FS.open(filename, "w");
		filename = String();
	}
	else if (upload.status == UPLOAD_FILE_WRITE)
	{
		if (upload_file)
		{
			upload_file.write(upload.buf, upload.currentSize);
		}
	}
	else if (upload.status == UPLOAD_FILE_END)
	{
		if (upload_file)
		{
			upload_file.close();
		}
	}
}

void endpoint_add(const char *uri, uint8_t method, endpoint_delegate request_handler, endpoint_delegate file_handler)
{
	if (!method)
	{
		method = 255;
	}

	String s = String(uri);

	if (s.endsWith("*"))
	{
		web_server.on(UriWildcard(s.substring(0, s.length() - 1)), (HTTPMethod)method, request_handler, file_handler);
	}
	else
	{
		web_server.on(Uri(uri), (HTTPMethod)method, request_handler, file_handler);
	}
}

void endpoint_request_uri(char *uri, size_t maxlen)
{
	strncpy(uri, web_server.uri().c_str(), maxlen);
}

int endpoint_request_hasargs(void)
{
	return web_server.args();
}

bool endpoint_request_arg(const char *argname, char *argvalue, size_t maxlen)
{
	if (!web_server.hasArg(String(argname)))
	{
		argvalue[0] = 0;
		return false;
	}
	strncpy(argvalue, web_server.arg(String(argname)).c_str(), maxlen);
	return true;
}

void endpoint_send(int code, const char *content_type, const char *data)
{
	web_server.send(code, content_type, data);
}

void endpoint_send_header(const char *name, const char *data, bool first)
{
	web_server.sendHeader(name, data, first);
}

bool endpoint_send_file(const char *file_path, const char *content_type)
{
	if (FLASH_FS.exists(file_path))
	{
		File file = FLASH_FS.open(file_path, "r");
		web_server.streamFile(file, content_type);
		file.close();
		return true;
	}
	return false;
}

#endif

#if defined(MCU_HAS_WIFI) && defined(MCU_HAS_WEBSOCKETS)
#include "WebSocketsServer.h"
#include "../../../modules/websocket.h"
WebSocketsServer socket_server(WEBSOCKET_PORT);

WEAK_EVENT_HANDLER(websocket_client_connected)
{
	DEFAULT_EVENT_HANDLER(websocket_client_connected);
}

WEAK_EVENT_HANDLER(websocket_client_disconnected)
{
	DEFAULT_EVENT_HANDLER(websocket_client_disconnected);
}

WEAK_EVENT_HANDLER(websocket_client_receive)
{
	DEFAULT_EVENT_HANDLER(websocket_client_receive);
}

WEAK_EVENT_HANDLER(websocket_client_error)
{
	DEFAULT_EVENT_HANDLER(websocket_client_error);
}

void websocket_send(uint8_t clientid, uint8_t *data, size_t length, uint8_t flags)
{
	switch (flags & WS_SEND_TYPE)
	{
	case WS_SEND_TXT:
		if (flags & WS_SEND_BROADCAST)
		{
			socket_server.broadcastTXT(data, length);
		}
		else
		{
			socket_server.sendTXT(clientid, data, length);
		}
		break;
	case WS_SEND_BIN:
		if (flags & WS_SEND_BROADCAST)
		{
			socket_server.broadcastTXT(data, length);
		}
		else
		{
			socket_server.sendTXT(clientid, data, length);
		}
		break;
	case WS_SEND_PING:
		if (flags & WS_SEND_BROADCAST)
		{
			socket_server.broadcastPing(data, length);
		}
		else
		{
			socket_server.sendPing(clientid, data, length);
		}
		break;
	}
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
	websocket_event_t event = {num, (uint32_t)socket_server.remoteIP(num), type, payload, length};
	switch (type)
	{
	case WStype_DISCONNECTED:
		EVENT_INVOKE(websocket_client_disconnected, &event);
		break;
	case WStype_CONNECTED:
		EVENT_INVOKE(websocket_client_connected, &event);
		break;
	case WStype_ERROR:
		EVENT_INVOKE(websocket_client_error, &event);
		break;
	case WStype_TEXT:
	case WStype_BIN:
	case WStype_FRAGMENT_TEXT_START:
	case WStype_FRAGMENT_BIN_START:
	case WStype_FRAGMENT:
	case WStype_FRAGMENT_FIN:
	case WStype_PING:
	case WStype_PONG:
		EVENT_INVOKE(websocket_client_receive, &event);
		break;
	}
}
#endif

void rp2040_wifi_bt_init(void)
{
#ifdef MCU_HAS_WIFI

	wifi_settings_offset = settings_register_external_setting(sizeof(wifi_settings_t));
	if (settings_load(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t)))
	{
		wifi_settings = {0};
		memcpy(wifi_settings.ssid, BOARD_NAME, strlen((const char *)BOARD_NAME));
		memcpy(wifi_settings.pass, WIFI_PASS, strlen((const char *)WIFI_PASS));
		settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
	}

	if (wifi_settings.wifi_on)
	{
		uint8_t str[64];

		switch (wifi_settings.wifi_mode)
		{
		case 1:
			WiFi.mode(WIFI_STA);
			WiFi.begin((char *)wifi_settings.ssid, (char *)wifi_settings.pass);
			protocol_send_feedback("Trying to connect to WiFi");
			break;
		case 2:
			WiFi.mode(WIFI_AP);
			WiFi.softAP(BOARD_NAME, (char *)wifi_settings.pass);
			protocol_send_feedback("AP started");
			protocol_send_feedback("SSID>" BOARD_NAME);
			sprintf((char *)str, "IP>%s", WiFi.softAPIP().toString().c_str());
			protocol_send_feedback((const char *)str);
			break;
		default:
			WiFi.mode(WIFI_AP_STA);
			WiFi.begin((char *)wifi_settings.ssid, (char *)wifi_settings.pass);
			protocol_send_feedback("Trying to connect to WiFi");
			WiFi.softAP(BOARD_NAME, (char *)wifi_settings.pass);
			protocol_send_feedback("AP started");
			protocol_send_feedback("SSID>" BOARD_NAME);
			sprintf((char *)str, "IP>%s", WiFi.softAPIP().toString().c_str());
			protocol_send_feedback((const char *)str);
			break;
		}
	}
	telnet_server.begin();
	telnet_server.setNoDelay(true);
#ifdef MCU_HAS_ENDPOINTS
	FLASH_FS.begin();
#endif
#ifndef CUSTOM_OTA_ENDPOINT
	httpUpdater.setup(&web_server, OTA_URI, update_username, update_password);
#endif
	web_server.on(
		FS_WRITE_URI, HTTP_GET, []()
		{ web_server.sendHeader("Content-Encoding", "gzip");
		web_server.send_P(200, __romstr__("text/html"), fs_write_page, FS_WRITE_GZ_SIZE); },
		NULL);
	web_server.on(
		FS_WRITE_URI, HTTP_POST, []()
		{ 
			if(web_server.hasArg("redirect")){
			web_server.sendHeader("Location", web_server.arg("redirect"));
			web_server.send(303);
		}
		else{
			web_server.send(200, "text/plain", "");
		} },
		fs_file_updater);
	web_server.begin();

#ifdef MCU_HAS_WEBSOCKETS
	socket_server.begin();
	socket_server.onEvent(webSocketEvent);
#endif
#endif
#ifdef ENABLE_BLUETOOTH
	bt_settings_offset = settings_register_external_setting(1);
	if (settings_load(bt_settings_offset, &bt_on, 1))
	{
		settings_erase(bt_settings_offset, (uint8_t *)&bt_on, 1);
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
DECL_BUFFER(uint8_t, wifi_tx, WIFI_TX_BUFFER_SIZE);
DECL_BUFFER(uint8_t, wifi_rx, RX_BUFFER_SIZE);

uint8_t mcu_wifi_getc(void)
{
	uint8_t c = 0;
	BUFFER_DEQUEUE(wifi_rx, &c);
	return c;
}

uint8_t mcu_wifi_available(void)
{
	return BUFFER_READ_AVAILABLE(wifi_rx);
}

void mcu_wifi_clear(void)
{
	BUFFER_CLEAR(wifi_rx);
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
	if (rp2040_wifi_clientok())
	{
		while (!BUFFER_EMPTY(wifi_tx))
		{
			uint8_t tmp[WIFI_TX_BUFFER_SIZE + 1];
			memset(tmp, 0, sizeof(tmp));
			uint8_t r;

			BUFFER_READ(wifi_tx, tmp, WIFI_TX_BUFFER_SIZE, r);
			server_client.write(tmp, r);
		}
	}
	else
	{
		// no client (discard)
		BUFFER_CLEAR(wifi_tx);
	}
}
#endif

#ifdef MCU_HAS_BLUETOOTH
#ifndef BLUETOOTH_TX_BUFFER_SIZE
#define BLUETOOTH_TX_BUFFER_SIZE 64
#endif
DECL_BUFFER(uint8_t, bt_tx, BLUETOOTH_TX_BUFFER_SIZE);
DECL_BUFFER(uint8_t, bt_rx, RX_BUFFER_SIZE);

uint8_t mcu_bt_getc(void)
{
	uint8_t c = 0;
	BUFFER_DEQUEUE(bt_rx, &c);
	return c;
}

uint8_t mcu_bt_available(void)
{
	return BUFFER_READ_AVAILABLE(bt_rx);
}

void mcu_bt_clear(void)
{
	BUFFER_CLEAR(bt_rx);
}

void mcu_bt_putc(uint8_t c)
{
	while (BUFFER_FULL(bt_tx))
	{
		mcu_bt_flush();
	}
	BUFFER_ENQUEUE(bt_tx, &c);
}

void mcu_bt_flush(void)
{
	while (!BUFFER_EMPTY(bt_tx))
	{
		uint8_t tmp[BLUETOOTH_TX_BUFFER_SIZE + 1];
		memset(tmp, 0, sizeof(tmp));
		uint8_t r;

		BUFFER_READ(bt_tx, tmp, BLUETOOTH_TX_BUFFER_SIZE, r);
		SerialBT.write(tmp, r);
		SerialBT.flush();
	}
}
#endif

uint8_t rp2040_wifi_bt_read(void)
{
#ifdef MCU_HAS_WIFI
	if (rp2040_wifi_clientok())
	{
		if (server_client.available() > 0)
		{
			return (uint8_t)server_client.read();
		}
	}
#endif

#ifdef ENABLE_BLUETOOTH
	return (uint8_t)SerialBT.read();
#endif

	return (uint8_t)0;
}

void rp2040_wifi_bt_process(void)
{
#ifdef MCU_HAS_WIFI
	DECL_BUFFER(uint8_t, wifi_rx, RX_BUFFER_SIZE);

	if (rp2040_wifi_clientok())
	{
		while (server_client.available() > 0)
		{
#ifndef DETACH_WIFI_FROM_MAIN_PROTOCOL
			uint8_t c = (uint8_t)server_client.read();
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
			mcu_wifi_rx_cb((uint8_t)server_client.read());
#endif
		}
	}

	if (wifi_settings.wifi_on)
	{
		web_server.handleClient();
#ifdef MCU_HAS_WEBSOCKETS
		socket_server.loop();
#endif
	}
#endif

#ifdef ENABLE_BLUETOOTH
	DECL_BUFFER(uint8_t, bt_rx, RX_BUFFER_SIZE);

	while (SerialBT.available() > 0)
	{
#ifndef DETACH_BLUETOOTH_FROM_MAIN_PROTOCOL
		uint8_t c = (uint8_t)SerialBT.read();
		if (mcu_com_rx_cb(c))
		{
			if (BUFFER_FULL(bt_rx))
			{
				c = OVF;
			}

			*(BUFFER_NEXT_FREE(bt_rx)) = c;
			BUFFER_STORE(bt_rx);
		}

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
#if (defined(MCU_HAS_WIFI) || defined(ENABLE_BLUETOOTH))
		rp2040_wifi_bt_init();
#endif
	}

#ifdef MCU_HAS_USB
#ifndef USB_TX_BUFFER_SIZE
#define USB_TX_BUFFER_SIZE 64
#endif
	DECL_BUFFER(uint8_t, usb_tx, USB_TX_BUFFER_SIZE);
	DECL_BUFFER(uint8_t, usb_rx, RX_BUFFER_SIZE);

	uint8_t mcu_usb_getc(void)
	{
		uint8_t c = 0;
		BUFFER_DEQUEUE(usb_rx, &c);
		return c;
	}

	uint8_t mcu_usb_available(void)
	{
		return BUFFER_READ_AVAILABLE(usb_rx);
	}

	void mcu_usb_clear(void)
	{
		BUFFER_CLEAR(usb_rx);
	}

	void mcu_usb_putc(uint8_t c)
	{
		while (BUFFER_FULL(usb_tx))
		{
			mcu_usb_flush();
		}
		BUFFER_ENQUEUE(usb_tx, &c);
	}

	void mcu_usb_flush(void)
	{
		while (!BUFFER_EMPTY(usb_tx))
		{
			uint8_t tmp[USB_TX_BUFFER_SIZE + 1];
			memset(tmp, 0, sizeof(tmp));
			uint8_t r;

			BUFFER_READ(usb_tx, tmp, USB_TX_BUFFER_SIZE, r);
			Serial.write(tmp, r);
			Serial.flush();
		}
	}
#endif

#ifdef MCU_HAS_UART
#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE 64
#endif
	DECL_BUFFER(uint8_t, uart_tx, UART_TX_BUFFER_SIZE);
	DECL_BUFFER(uint8_t, uart_rx, RX_BUFFER_SIZE);

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
			uint8_t tmp[UART_TX_BUFFER_SIZE + 1];
			memset(tmp, 0, sizeof(tmp));
			uint8_t r = 0;

			BUFFER_READ(uart_tx, tmp, UART_TX_BUFFER_SIZE, r);
			COM_UART.write(tmp, r);
			COM_UART.flush();
		}
	}
#endif

#ifdef MCU_HAS_UART2
#ifndef UART2_TX_BUFFER_SIZE
#define UART2_TX_BUFFER_SIZE 64
#endif
	DECL_BUFFER(uint8_t, uart2_tx, UART2_TX_BUFFER_SIZE);
	DECL_BUFFER(uint8_t, uart2_rx, RX_BUFFER_SIZE);

	uint8_t mcu_uart2_getc(void)
	{
		uint8_t c = 0;
		BUFFER_DEQUEUE(uart2_rx, &c);
		return c;
	}

	uint8_t mcu_uart2_available(void)
	{
		return BUFFER_READ_AVAILABLE(uart2_rx);
	}

	void mcu_uart2_clear(void)
	{
		BUFFER_CLEAR(uart2_rx);
	}

	void mcu_uart2_putc(uint8_t c)
	{
		while (BUFFER_FULL(uart2_tx))
		{
			mcu_uart2_flush();
		}
		BUFFER_ENQUEUE(uart2_tx, &c);
	}

	void mcu_uart2_flush(void)
	{
		while (!BUFFER_EMPTY(uart2_tx))
		{
			uint8_t tmp[UART2_TX_BUFFER_SIZE + 1];
			memset(tmp, 0, sizeof(tmp));
			uint8_t r;

			BUFFER_READ(uart2_tx, tmp, UART2_TX_BUFFER_SIZE, r);
			COM2_UART.write(tmp, r);
			COM2_UART.flush();
		}
	}
#endif

	void rp2040_uart_process(void)
	{
#ifdef MCU_HAS_USB
		while (Serial.available() > 0)
		{
#ifndef DETACH_USB_FROM_MAIN_PROTOCOL
			uint8_t c = (uint8_t)Serial.read();
			if (mcu_com_rx_cb(c))
			{
				if (BUFFER_FULL(usb_rx))
				{
					c = OVF;
				}

				*(BUFFER_NEXT_FREE(usb_rx)) = c;
				BUFFER_STORE(usb_rx);
			}

#else
			mcu_usb_rx_cb((uint8_t)Serial.read());
#endif
		}
#endif

#ifdef MCU_HAS_UART
		while (COM_UART.available() > 0)
		{
#ifndef DETACH_UART_FROM_MAIN_PROTOCOL
			uint8_t c = (uint8_t)COM_UART.read();
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
			mcu_uart_rx_cb((uint8_t)COM_UART.read());
#endif
		}
#endif

#ifdef MCU_HAS_UART2
		while (COM2_UART.available() > 0)
		{
			uint8_t c = (uint8_t)COM2_UART.read();
#ifndef DETACH_UART2_FROM_MAIN_PROTOCOL

			if (mcu_com_rx_cb(c))
			{
				if (BUFFER_FULL(uart2_rx))
				{
					c = OVF;
				}

				*(BUFFER_NEXT_FREE(uart2_rx)) = c;
				BUFFER_STORE(uart2_rx);
			}

#else
			mcu_uart2_rx_cb(c);
#ifndef UART2_DISABLE_BUFFER
			if (BUFFER_FULL(uart2_rx))
			{
				c = OVF;
			}

			*(BUFFER_NEXT_FREE(uart2_rx)) = c;
			BUFFER_STORE(uart2_rx);
#endif
#endif
		}
#endif

#if (defined(MCU_HAS_WIFI) || defined(ENABLE_BLUETOOTH))
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
			protocol_send_feedback((const char *)" EEPROM write error");
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

	uint8_t mcu_i2c_send(uint8_t address, uint8_t *data, uint8_t datalen, bool release, uint32_t ms_timeout)
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
