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
	bool mcu_custom_grbl_cmd(void *args)
	{
		grbl_cmd_args_t *cmd_params = (grbl_cmd_args_t *)args;
		uint8_t str[64];
		char arg[ARG_MAX_LEN];
		uint8_t has_arg = (cmd_params->next_char == '=');
		memset(arg, 0, sizeof(arg));

#ifdef ENABLE_WIFI
		if (!strncmp((const char *)(cmd_params->cmd), "WIFI", 4))
		{
			if (!strcmp((const char *)&(cmd_params->cmd)[4], "ON"))
			{
				WiFi.disconnect();
				switch (wifi_settings.wifi_mode)
				{
				case 1:
					WiFi.mode(WIFI_STA);
					WiFi.begin(wifi_settings.ssid, wifi_settings.pass);
					grbl_protocol_feedback("Trying to connect to WiFi");
					break;
				case 2:
					WiFi.mode(WIFI_AP);
					WiFi.softAP(BOARD_NAME, wifi_settings.pass);
					grbl_protocol_feedback("AP started");
					grbl_protocol_feedback("SSID>" BOARD_NAME);
					sprintf((char *)str, "IP>%s", WiFi.softAPIP().toString().c_str());
					grbl_protocol_feedback("%s", str);
					break;
				default:
					WiFi.mode(WIFI_AP_STA);
					WiFi.begin(wifi_settings.ssid, wifi_settings.pass);
					grbl_protocol_feedback("Trying to connect to WiFi");
					WiFi.softAP(BOARD_NAME, wifi_settings.pass);
					grbl_protocol_feedback("AP started");
					grbl_protocol_feedback("SSID>" BOARD_NAME);
					sprintf((char *)str, "IP>%s", WiFi.softAPIP().toString().c_str());
					grbl_protocol_feedback("%s", str);
					break;
				}

				wifi_settings.wifi_on = 1;
				settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
				*(cmd_params->error) = STATUS_OK;
				return EVENT_HANDLED;
			}

			if (!strcmp((const char *)&(cmd_params->cmd)[4], "OFF"))
			{
				WiFi.disconnect();
				wifi_settings.wifi_on = 0;
				settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
				*(cmd_params->error) = STATUS_OK;
				return EVENT_HANDLED;
			}

			if (!strcmp((const char *)&(cmd_params->cmd)[4], "SSID"))
			{
				if (has_arg)
				{
					int8_t len = parser_get_grbl_cmd_arg(arg, ARG_MAX_LEN);

					if (len < 0)
					{
						*(cmd_params->error) = STATUS_INVALID_STATEMENT;
						return EVENT_HANDLED;
					}

					if (len > WIFI_SSID_MAX_LEN)
					{
						grbl_protocol_feedback("WiFi SSID is too long");
					}
					memset(wifi_settings.ssid, 0, sizeof(wifi_settings.ssid));
					strcpy((char *)wifi_settings.ssid, (const char *)arg);
					settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
					grbl_protocol_feedback("WiFi SSID modified");
				}
				else
				{
					sprintf((char *)str, "SSID>%s", wifi_settings.ssid);
					grbl_protocol_feedback("%s", str);
				}
				*(cmd_params->error) = STATUS_OK;
				return EVENT_HANDLED;
			}

			if (!strcmp((const char *)&(cmd_params->cmd)[4], "SCAN"))
			{
				// Serial.println("[MSG:Scanning Networks]");
				grbl_protocol_feedback("Scanning Networks");
				int numSsid = WiFi.scanNetworks();
				if (numSsid == -1)
				{
					grbl_protocol_feedback("Failed to scan!");
					return EVENT_HANDLED;
				}

				// print the list of networks seen:
				sprintf((char *)str, "%d available networks", numSsid);
				grbl_protocol_feedback("%s", str);

				// print the network number and name for each network found:
				for (int netid = 0; netid < numSsid; netid++)
				{
					sprintf((char *)str, "%d) %s\tSignal:  %ddBm", netid, WiFi.SSID(netid).c_str(), WiFi.RSSI(netid));
					grbl_protocol_feedback("%s", str);
				}
				*(cmd_params->error) = STATUS_OK;
				return EVENT_HANDLED;
			}

			if (!strcmp((const char *)&(cmd_params->cmd)[4], "SAVE"))
			{
				settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
				grbl_protocol_feedback("WiFi settings saved");
				*(cmd_params->error) = STATUS_OK;
				return EVENT_HANDLED;
			}

			if (!strcmp((const char *)&(cmd_params->cmd)[4], "RESET"))
			{
				settings_erase(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
				grbl_protocol_feedback("WiFi settings deleted");
				*(cmd_params->error) = STATUS_OK;
				return EVENT_HANDLED;
			}

			if (!strcmp((const char *)&(cmd_params->cmd)[4], "MODE"))
			{
				if (has_arg)
				{
					int8_t len = parser_get_grbl_cmd_arg(arg, ARG_MAX_LEN);

					if (len < 0)
					{
						*(cmd_params->error) = STATUS_INVALID_STATEMENT;
						return EVENT_HANDLED;
					}

					int mode = atoi((const char *)arg) - 1;
					if (mode >= 0)
					{
						wifi_settings.wifi_mode = mode;
					}
					else
					{
						grbl_protocol_feedback("Invalid value. STA+AP(1), STA(2), AP(3)");
					}
				}

				switch (wifi_settings.wifi_mode)
				{
				case 0:
					grbl_protocol_feedback("WiFi mode>STA+AP");
					break;
				case 1:
					grbl_protocol_feedback("WiFi mode>STA");
					break;
				case 2:
					grbl_protocol_feedback("WiFi mode>AP");
					break;
				}
				*(cmd_params->error) = STATUS_OK;
				return EVENT_HANDLED;
			}

			if (!strcmp((const char *)&(cmd_params->cmd)[4], "PASS") && has_arg)
			{
				int8_t len = parser_get_grbl_cmd_arg(arg, ARG_MAX_LEN);

				if (len < 0)
				{
					*(cmd_params->error) = STATUS_INVALID_STATEMENT;
					return EVENT_HANDLED;
				}

				if (len > WIFI_SSID_MAX_LEN)
				{
					grbl_protocol_feedback("WiFi pass is too long");
					return EVENT_HANDLED;
				}
				memset(wifi_settings.pass, 0, sizeof(wifi_settings.pass));
				strcpy((char *)wifi_settings.pass, (const char *)arg);
				grbl_protocol_feedback("WiFi password modified");
				*(cmd_params->error) = STATUS_OK;
				return EVENT_HANDLED;
			}

			if (!strcmp((const char *)&(cmd_params->cmd)[4], "IP"))
			{
				if (wifi_settings.wifi_on)
				{
					switch (wifi_settings.wifi_mode)
					{
					case 1:
						sprintf((char *)str, "STA IP>%s", WiFi.localIP().toString().c_str());
						grbl_protocol_feedback("%s", str);
						sprintf((char *)str, "AP IP>%s", WiFi.softAPIP().toString().c_str());
						grbl_protocol_feedback("%s", str);
						break;
					case 2:
						sprintf((char *)str, "IP>%s", WiFi.localIP().toString().c_str());
						grbl_protocol_feedback("%s", str);
						break;
					default:
						sprintf((char *)str, "IP>%s", WiFi.softAPIP().toString().c_str());
						grbl_protocol_feedback("%s", str);
						break;
					}
				}
				else
				{
					grbl_protocol_feedback("WiFi is off");
				}

				*(cmd_params->error) = STATUS_OK;
				return EVENT_HANDLED;
			}
		}
#endif
		return EVENT_CONTINUE;
	}

	CREATE_EVENT_LISTENER(grbl_cmd, mcu_custom_grbl_cmd);
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
			grbl_protocol_feedback("Disconnected from WiFi");
			return false;
		}

		if (!connected)
		{
			connected = true;
			grbl_protocol_feedback("Connected to WiFi");
			sprintf((char *)str, "SSID>%s", wifi_settings.ssid);
			grbl_protocol_feedback("%s", str);
			sprintf((char *)str, "IP>%s", WiFi.localIP().toString().c_str());
			grbl_protocol_feedback("%s", str);
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

#if defined(MCU_HAS_WIFI) && defined(MCU_HAS_ENDPOINTS)

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

/**
 * Implements the function calls for the file system C wrapper
 */
#include "../../../modules/file_system.h"
#define fileptr_t(ptr) static_cast<File>(*(reinterpret_cast<File *>(ptr)))
	fs_t flash_fs;

	int flash_fs_available(fs_file_t *fp)
	{
		return fileptr_t(fp->file_ptr).available();
	}

	void flash_fs_close(fs_file_t *fp)
	{
		fileptr_t(fp->file_ptr).close();
	}

	bool flash_fs_remove(const char *path)
	{
		return FLASH_FS.remove(path);
	}

	bool flash_fs_next_file(fs_file_t *fp, fs_file_info_t *finfo)
	{
		File f = ((File *)fp->file_ptr)->openNextFile();
		if (!f || !finfo)
		{
			return false;
		}
		memset(finfo->full_name, 0, sizeof(finfo->full_name));
		strncpy(finfo->full_name, f.name(), (FS_PATH_NAME_MAX_LEN - strlen(f.name())));
		finfo->is_dir = f.isDirectory();
		finfo->size = f.size();
		finfo->timestamp = f.getLastWrite();
		f.close();
		return true;
	}

	size_t flash_fs_read(fs_file_t *fp, uint8_t *buffer, size_t len)
	{
		return fileptr_t(fp->file_ptr).read(buffer, len);
	}

	size_t flash_fs_write(fs_file_t *fp, const uint8_t *buffer, size_t len)
	{
		return fileptr_t(fp->file_ptr).write(buffer, len);
	}

	bool flash_fs_info(const char *path, fs_file_info_t *finfo)
	{
		File f = FLASH_FS.open(path, "r");
		if (f && finfo)
		{
			memset(finfo->full_name, 0, sizeof(finfo->full_name));
			strncpy(finfo->full_name, f.name(), (FS_PATH_NAME_MAX_LEN - strlen(f.name())));
			finfo->is_dir = f.isDirectory();
			finfo->size = f.size();
			finfo->timestamp = (uint32_t)f.getLastWrite();
			f.close();
			return true;
		}

		return false;
	}

	fs_file_t *flash_fs_open(const char *path, const char *mode)
	{
		fs_file_t *fp = (fs_file_t *)calloc(1, sizeof(fs_file_t));
		if (fp)
		{
			fp->file_ptr = calloc(1, sizeof(File));
			if (fp->file_ptr)
			{
				*(static_cast<File *>(fp->file_ptr)) = FLASH_FS.open(path, mode);
				if (*(static_cast<File *>(fp->file_ptr)))
				{
					memset(fp->file_info.full_name, 0, sizeof(fp->file_info.full_name));
					fp->file_info.full_name[0] = '/';
					fp->file_info.full_name[1] = flash_fs.drive;
					fp->file_info.full_name[2] = '/';
					strncat(fp->file_info.full_name, ((File *)fp->file_ptr)->name(), FS_PATH_NAME_MAX_LEN - 3);
					fp->file_info.is_dir = ((File *)fp->file_ptr)->isDirectory();
					fp->file_info.size = ((File *)fp->file_ptr)->size();
					fp->file_info.timestamp = (uint32_t)((File *)fp->file_ptr)->getLastWrite();
					fp->fs_ptr = &flash_fs;
					return fp;
				}
				free(fp->file_ptr);
			}
			free(fp);
		}
		return NULL;
	}

	fs_file_t *flash_fs_opendir(const char *path)
	{
		return flash_fs_open(path, "r");
	}

	bool flash_fs_seek(fs_file_t *fp, uint32_t position)
	{
		return fp->fs_ptr->seek(fp, position);
	}

	bool flash_fs_mkdir(const char *path)
	{
		return FLASH_FS.mkdir(path);
	}

	bool flash_fs_rmdir(const char *path)
	{
		return FLASH_FS.rmdir(path);
	}

/**
 * Implements the function calls for the enpoints C wrapper
 */
#include "../../../modules/endpoint.h"
	void endpoint_add(const char *uri, uint8_t method, endpoint_delegate request_handler, endpoint_delegate file_handler)
	{
		if (!method)
		{
			method = HTTP_ANY;
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

	void endpoint_send(int code, const char *content_type, const uint8_t *data, size_t data_len)
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
				web_server.send(code, content_type, data, data_len);
				break;
			default:
				if (data)
				{
					web_server.sendContent((char *)data, data_len);
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
		endpoint_upload_t status = {.status = (uint8_t)upload.status, .data = upload.buf, .datalen = upload.currentSize};
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
		strncat(filename, upload.filename.c_str(), maxlen - strlen(filename));
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
		DEBUG("Wifi assert");
#ifdef ENABLE_WIFI
		DEBUG("Wifi startup");
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
				grbl_protocol_feedback("Trying to connect to WiFi");
				break;
			case 2:
				WiFi.mode(WIFI_AP);
				WiFi.softAP(BOARD_NAME, (char *)wifi_settings.pass);
				grbl_protocol_feedback("AP started");
				grbl_protocol_feedback("SSID>" BOARD_NAME);
				sprintf((char *)str, "IP>%s", WiFi.softAPIP().toString().c_str());
				grbl_protocol_feedback("%s", str);
				break;
			default:
				WiFi.mode(WIFI_AP_STA);
				WiFi.begin((char *)wifi_settings.ssid, (char *)wifi_settings.pass);
				grbl_protocol_feedback("Trying to connect to WiFi");
				WiFi.softAP(BOARD_NAME, (char *)wifi_settings.pass);
				grbl_protocol_feedback("AP started");
				grbl_protocol_feedback("SSID>" BOARD_NAME);
				sprintf((char *)str, "IP>%s", WiFi.softAPIP().toString().c_str());
				grbl_protocol_feedback("%s", str);
				break;
			}
		}
		telnet_server.begin();
		telnet_server.setNoDelay(true);
#ifdef MCU_HAS_ENDPOINTS
		FLASH_FS.begin();
		flash_fs = {
				.drive = 'C',
				.open = flash_fs_open,
				.read = flash_fs_read,
				.write = flash_fs_write,
				.seek = flash_fs_seek,
				.available = flash_fs_available,
				.close = flash_fs_close,
				.remove = flash_fs_remove,
				.opendir = flash_fs_opendir,
				.mkdir = flash_fs_mkdir,
				.rmdir = flash_fs_rmdir,
				.next_file = flash_fs_next_file,
				.finfo = flash_fs_info,
				.next = NULL};
		fs_mount(&flash_fs);
#endif
#ifndef CUSTOM_OTA_ENDPOINT
		httpUpdater.setup(&web_server, OTA_URI, update_username, update_password);
#endif
		web_server.begin();

#ifdef MCU_HAS_WEBSOCKETS
		socket_server.begin();
		socket_server.onEvent(webSocketEvent);
#endif
#endif

#ifdef BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
		ADD_EVENT_LISTENER(grbl_cmd, mcu_custom_grbl_cmd);
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

				BUFFER_ENQUEUE(uart_rx, &c);
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

					BUFFER_ENQUEUE(wifi_rx, &c);
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
// #include "esp_peri.h"
extern "C"
{
#include "../../../cnc.h"
	void esp8266_spi_init(uint32_t freq, uint8_t mode)
	{
		SPI.begin();
		spi_config_t conf = {0};
		conf.mode = SPI_MODE;
		mcu_spi_config(conf, freq);
	}

	void mcu_spi_config(spi_config_t config, uint32_t freq)
	{
		SPI.setFrequency(freq);
		SPI.setDataMode(config.mode);
		SPI.setBitOrder(MSBFIRST);
	}

	void mcu_spi_start(spi_config_t config, uint32_t freq)
	{
		SPI.beginTransaction(SPISettings(freq, MSBFIRST, config.mode));
	}

	bool mcu_spi_bulk_transfer(const uint8_t *out, uint8_t *in, uint16_t len){
		SPI.transferBytes(out, int, len);
		return false;
	}

	void mcu_spi_end(void)
	{
		SPI.endTransaction();
	}

	uint8_t mcu_spi_xmit(uint8_t c)
	{
		return SPI.transfer(c);
		// while (SPI1CMD & SPIBUSY)
		// 	;
		// SPI1W0 = c;
		// SPI1CMD |= SPIBUSY;
		// while (SPI1CMD & SPIBUSY)
		// 	;
		// return (uint8_t)(SPI1W0 & 0xff);
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
			DEBUG("EEPROM invalid address @ %u",address);
			return 0;
		}
		return EEPROM.read(address);
	}

	void mcu_eeprom_putc(uint16_t address, uint8_t value)
	{
		if (NVM_STORAGE_SIZE <= address)
		{
			DEBUG("EEPROM invalid address @ %u",address);
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
