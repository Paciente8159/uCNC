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
#include "../../../../cnc_config.h"
#include <Arduino.h>
#include "user_interface.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef ENABLE_WIFI
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

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
#define WIFI_USER "admin"
#endif

#ifndef WIFI_PASS
#define WIFI_PASS "pass"
#endif

#ifndef WIFI_SSID_MAX_LEN
#define WIFI_SSID_MAX_LEN 32
#endif

#define ARG_MAX_LEN WIFI_SSID_MAX_LEN

#ifndef OTA_URI
#define OTA_URI "/firmware"
#endif

ESP8266WebServer web_server(WEBSERVER_PORT);
ESP8266HTTPUpdateServer httpUpdater;
const char *update_path = OTA_URI;
const char *update_username = WIFI_USER;
const char *update_password = WIFI_PASS;
#define MAX_SRV_CLIENTS 1
WiFiServer telnet_server(TELNET_PORT);
WiFiClient telnet_client;

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
	uint8_t mcu_custom_grbl_cmd(uint8_t *grbl_cmd_str, uint8_t grbl_cmd_len, uint8_t next_char)
	{
		uint8_t str[64];
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

#ifdef ENABLE_WIFI
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
					strcpy((char *)wifi_settings.ssid, (const char *)arg);
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
					sprintf((char *)str, "%d) %s\tSignal:  %ddBm", netid, WiFi.SSID(netid).c_str(), WiFi.RSSI(netid));
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
				if (len > WIFI_SSID_MAX_LEN)
				{
					protocol_send_feedback((const char *)"WiFi pass is too long");
				}
				memset(wifi_settings.pass, 0, sizeof(wifi_settings.pass));
				strcpy((char *)wifi_settings.pass, (const char *)arg);
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

	bool esp8266_wifi_clientok(void)
	{
#ifdef ENABLE_WIFI
		static uint32_t next_info = 30000;
		static bool connected = false;
		uint8_t str[64];

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
			if (telnet_client)
			{
				if (telnet_client.connected())
				{
					telnet_client.stop();
				}
			}
			telnet_client = telnet_server.accept();
			telnet_client.println("[MSG:New client connected]");
			return false;
		}
		else if (telnet_client)
		{
			if (telnet_client.connected())
			{
				return true;
			}
		}
#endif
		return false;
	}

#if defined(ENABLE_WIFI) && defined(MCU_HAS_ENDPOINTS)

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
		static File upload_file;
		if (!web_server.uri().startsWith(FS_URI) || (web_server.method() != HTTP_POST && web_server.method() != HTTP_PUT))
		{
			return;
		}

		String urlpath = String((web_server.uri().substring(FS_URI_LEN).length() != 0) ? web_server.uri().substring(FS_URI_LEN) : "/");

		if (!FLASH_FS.exists(urlpath))
		{
			return;
		}

		HTTPUpload &upload = web_server.upload();
		if (upload.status == UPLOAD_FILE_START)
		{
			if (web_server.method() == HTTP_POST)
			{
				if (!urlpath.endsWith("/"))
				{
					urlpath.concat("/");
				}

				urlpath.concat(upload.filename);
			}
			upload_file = FLASH_FS.open(urlpath, "w");
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

	void fs_file_browser()
	{
		File fp;
		char path[256];

		// updated page
		if (web_server.hasArg("update") && web_server.method() == HTTP_GET)
		{
			web_server.sendHeader("Content-Encoding", "gzip");
			web_server.send_P(200, __romstr__("text/html"), fs_write_page, FS_WRITE_GZ_SIZE);
			return;
		}

		String urlpath = String((web_server.uri().substring(FS_URI_LEN).length() != 0) ? web_server.uri().substring(FS_URI_LEN) : "/");

		if (!FLASH_FS.exists(urlpath))
		{
			endpoint_send(404, "application/json", "{\"result\":\"notfound\"}");
			return;
		}

		fp = FLASH_FS.open(urlpath, "r");

		switch (web_server.method())
		{
		case HTTP_DELETE:
			if (fp.isDirectory())
			{
				FLASH_FS.rmdir(urlpath);
			}
			else
			{
				FLASH_FS.remove(urlpath);
			}
			__FALL_THROUGH__
		case HTTP_PUT:
		case HTTP_POST:
			if (web_server.hasArg("redirect"))
			{
				memset(path, 0, 256);
				web_server.sendHeader("Location", web_server.arg("redirect"));
				sprintf(path, "{\"redirect\":\"%s\"}", web_server.arg("redirect").c_str());
				web_server.send(303, "application/json", path);
			}
			else
			{
				endpoint_send(200, "application/json", "{\"result\":\"ok\"}");
			}

			break;
		default: // handle as get
			if (fp.isDirectory())
			{
				// start chunck transmition;
				endpoint_request_uri(path, 256);
				endpoint_send(200, NULL, NULL);
				endpoint_send(200, "application/json", "{\"result\":\"ok\",\"path\":\"");
				endpoint_send(200, "application/json", path);
				endpoint_send(200, "application/json", "\",\"data\":[");
				File file = fp.openNextFile();

				while (file)
				{
					memset(path, 0, 256);
					if (file.isDirectory())
					{
						sprintf(path, "{\"type\":\"dir\",\"name\":\"%s\",\"attr\":%d},", file.name(), 0);
					}
					else
					{
						sprintf(path, "{\"type\":\"file\",\"name\":\"%s\",\"attr\":0,\"size\":%lu,\"date\":0}", file.name(), (unsigned long int)file.size());
					}

					file = fp.openNextFile();
					if (file)
					{
						// trailling comma
						path[strlen(path)] = ',';
					}
					endpoint_send(200, "application/json", path);
				}
				endpoint_send(200, "application/json", "]}\n");
				// close the stream
				endpoint_send(200, "application/json", NULL);
			}
			else
			{
				web_server.streamFile(fp, "application/octet-stream");
			}
			break;
		}

		fp.close();
	}

	// call to the webserver initializer
	void endpoint_add(const char *uri, uint8_t method, endpoint_delegate request_handler, endpoint_delegate file_handler)
	{
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
		static uint8_t in_chuncks = 0;
		if (!content_type)
		{
			in_chuncks = 1;
			web_server.setContentLength(CONTENT_LENGTH_UNKNOWN);
		}
		else
		{
			switch (in_chuncks)
			{
			case 1:
				in_chuncks = 2;
				__FALL_THROUGH__
			case 0:
				web_server.send(code, content_type, data);
				break;
			default:
				if (data)
				{
					web_server.sendContent(data);
					in_chuncks = 2;
				}
				else
				{
					web_server.sendContent("");
					in_chuncks = 0;
				}
				break;
			}
		}
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

	endpoint_upload_t endpoint_file_upload_status(void)
	{
		HTTPUpload &upload = web_server.upload();
		endpoint_upload_t status = {.status=(uint8_t)upload.status, .data = upload.buf, .datalen = upload.currentSize};
		return status;
	}

	uint8_t endpoint_request_method(void)
	{
		switch (web_server.method())
		{
		case HTTP_GET:
			return ENDPOINT_GET;
		case HTTP_POST:
			return ENDPOINT_POST;
		case HTTP_PUT:
			return ENDPOINT_PUT;
		case HTTP_DELETE:
			return ENDPOINT_DELETE;
		default:
			return (ENDPOINT_OTHER | (uint8_t)web_server.method());
		}
	}

	void endpoint_file_upload_name(char *filename, size_t maxlen)
	{
		HTTPUpload &upload = web_server.upload();
		strncpy(filename, upload.filename.c_str(), maxlen);
	}

#endif

#if defined(ENABLE_WIFI) && defined(MCU_HAS_WEBSOCKETS)
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

	void esp8266_uart_init(int baud)
	{
		Serial.begin(baud);
#ifdef ENABLE_WIFI
		WiFi.setSleepMode(WIFI_NONE_SLEEP);

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
		httpUpdater.setup(&web_server, update_path, update_username, update_password);
#endif
		endpoint_add(FS_URI, HTTP_ANY, fs_file_browser, fs_file_updater);
		endpoint_add(FS_URI "/*", HTTP_ANY, fs_file_browser, fs_file_updater);
		web_server.begin();

#ifdef MCU_HAS_WEBSOCKETS
		socket_server.begin();
		socket_server.onEvent(webSocketEvent);
#endif
#endif
	}

#ifdef MCU_HAS_UART
#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE 64
#endif
	DECL_BUFFER(uint8_t, uart_rx, RX_BUFFER_SIZE);
	DECL_BUFFER(uint8_t, uart_tx, UART_TX_BUFFER_SIZE);
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
			uint8_t r;
			uint8_t max = (uint8_t)MIN(Serial.availableForWrite(), UART_TX_BUFFER_SIZE);

			BUFFER_READ(uart_tx, tmp, max, r);
			Serial.write(tmp, r);
			Serial.flush();
		}
	}
#endif

#ifdef MCU_HAS_WIFI
#ifndef WIFI_TX_BUFFER_SIZE
#define WIFI_TX_BUFFER_SIZE 64
#endif
	DECL_BUFFER(uint8_t, wifi_rx, RX_BUFFER_SIZE);
	DECL_BUFFER(uint8_t, wifi_tx, WIFI_TX_BUFFER_SIZE);

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
		if (esp8266_wifi_clientok())
		{
			while (!BUFFER_EMPTY(wifi_tx))
			{
				uint8_t tmp[WIFI_TX_BUFFER_SIZE + 1];
				memset(tmp, 0, sizeof(tmp));
				uint8_t r;
				uint8_t max = (uint8_t)MIN(telnet_client.availableForWrite(), WIFI_TX_BUFFER_SIZE);

				BUFFER_READ(wifi_tx, tmp, max, r);
				telnet_client.write(tmp, r);
			}
		}
		else
		{
			// no client (discard)
			BUFFER_CLEAR(wifi_tx);
		}
	}
#endif

	void esp8266_uart_process(void)
	{
		while (Serial.available() > 0)
		{
			system_soft_wdt_feed();
#ifndef DETACH_UART_FROM_MAIN_PROTOCOL
			uint8_t c = (uint8_t)Serial.read();
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
			mcu_uart_rx_cb((uint8_t)Serial.read());
#endif
		}

#ifdef ENABLE_WIFI
		if (esp8266_wifi_clientok())
		{
			while (telnet_client.available() > 0)
			{
				system_soft_wdt_feed();
#ifndef DETACH_WIFI_FROM_MAIN_PROTOCOL
				uint8_t c = (uint8_t)telnet_client.read();
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
				mcu_wifi_rx_cb((uint8_t)telnet_client.read());
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
	}
}

#ifdef MCU_HAS_SPI
#include <Arduino.h>
#include <SPI.h>
#include "esp_peri.h"
extern "C"
{
#include "../../../cnc.h"
	void esp8266_spi_init(uint32_t freq, uint8_t mode)
	{
		SPI.begin();
		SPI.setFrequency(freq);
		SPI.setDataMode(mode);
	}

	void mcu_spi_config(uint8_t mode, uint32_t freq)
	{
		SPI.setFrequency(freq);
		SPI.setDataMode(mode);
	}
}

#endif

#ifndef RAM_ONLY_SETTINGS
#include <Arduino.h>
#include <EEPROM.h>
#include <stdint.h>
extern "C"
{
	void esp8266_eeprom_init(int size)
	{
		EEPROM.begin(size);
	}

	uint8_t mcu_eeprom_getc(uint16_t address)
	{
		if (NVM_STORAGE_SIZE <= address)
		{
			DEBUG_STR("EEPROM invalid address @ ");
			DEBUG_INT(address);
			DEBUG_PUTC('\n');
			return 0;
		}
		return EEPROM.read(address);
	}

	void mcu_eeprom_putc(uint16_t address, uint8_t value)
	{
		if (NVM_STORAGE_SIZE <= address)
		{
			DEBUG_STR("EEPROM invalid address @ ");
			DEBUG_INT(address);
			DEBUG_PUTC('\n');
		}
		EEPROM.write(address, value);
	}

	void mcu_eeprom_flush(void)
	{
		if (!EEPROM.commit())
		{
			Serial.println("[MSG: EEPROM write error]");
		}
	}
}

#endif

#endif
