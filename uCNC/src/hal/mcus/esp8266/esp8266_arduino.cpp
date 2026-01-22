/*
	Name: esp8266_wifi.cpp
	Description: Implements Wifi and support services (including webserver, sockets and flash file system) for ESP8266 using Arduino libraries.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 07-02-2025

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
#include <Updater.h> // ESP8266 Arduino core Update class
#include "user_interface.h"
#include <stdint.h>
#include <stdbool.h>
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

extern "C"
{
#include "../../../cnc.h"
}

#ifndef ARG_MAX_LEN
#define ARG_MAX_LEN 32
#endif

#ifdef ENABLE_SOCKETS
#include <ESP8266WiFi.h>
// #include <Update.h>
// #include <ESP8266WebServer.h>
// #include <ESP8266HTTPUpdateServer.h>

#ifndef WIFI_USER
#define WIFI_USER "admin"
#endif

#ifndef WIFI_PASS
#define WIFI_PASS "pass"
#endif

#ifndef WIFI_SSID_MAX_LEN
#define WIFI_SSID_MAX_LEN 32
#endif

// #ifndef OTA_URI
// #define OTA_URI "/firmware"
// #endif

// ESP8266WebServer web_server(443);
// ESP8266HTTPUpdateServer httpUpdater;
// const char *update_path = OTA_URI;
const char *update_username = WIFI_USER;
const char *update_password = WIFI_PASS;
// WiFiServer telnet_server(TELNET_PORT);
// WiFiClient telnet_client;

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

#if defined(BOARD_HAS_CUSTOM_SYSTEM_COMMANDS) && defined(ENABLE_SOCKETS)
	bool mcu_custom_grbl_cmd(void *args)
	{
		grbl_cmd_args_t *cmd_params = (grbl_cmd_args_t *)args;
		char arg[ARG_MAX_LEN];
		uint8_t has_arg = (cmd_params->next_char == '=');
		memset(arg, 0, sizeof(arg));

#ifdef ENABLE_SOCKETS
		if (!strncmp((const char *)(cmd_params->cmd), "WIFI", 4))
		{
			if (!strcmp((const char *)&(cmd_params->cmd)[4], "ON"))
			{
				ATOMIC_CODEBLOCK
				{
					WiFi.disconnect();
					switch (wifi_settings.wifi_mode)
					{
					case 1:
						WiFi.mode(WIFI_STA);
						WiFi.begin(wifi_settings.ssid, wifi_settings.pass);
						proto_info("Trying to connect to WiFi");
						break;
					case 2:
						WiFi.mode(WIFI_AP);
						WiFi.softAP(BOARD_NAME, wifi_settings.pass);
						proto_info("AP started");
						proto_info("SSID>" BOARD_NAME);
						proto_info("IP>%s", WiFi.softAPIP().toString().c_str());
						break;
					default:
						WiFi.mode(WIFI_AP_STA);
						WiFi.begin(wifi_settings.ssid, wifi_settings.pass);
						proto_info("Trying to connect to WiFi");
						WiFi.softAP(BOARD_NAME, wifi_settings.pass);
						proto_info("AP started");
						proto_info("SSID>" BOARD_NAME);
						proto_info("IP>%s", WiFi.softAPIP().toString().c_str());
						break;
					}

					wifi_settings.wifi_on = 1;
				}
				settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
				*(cmd_params->error) = STATUS_OK;
				return EVENT_HANDLED;
			}

			if (!strcmp((const char *)&(cmd_params->cmd)[4], "OFF"))
			{
				ATOMIC_CODEBLOCK
				{
					WiFi.disconnect();
					wifi_settings.wifi_on = 0;
				}
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
						proto_info("WiFi SSID is too long");
					}
					memset(wifi_settings.ssid, 0, sizeof(wifi_settings.ssid));
					strcpy((char *)wifi_settings.ssid, (const char *)arg);
					settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
					proto_info("WiFi SSID modified");
				}
				else
				{
					proto_info("SSID>%s", wifi_settings.ssid);
				}
				*(cmd_params->error) = STATUS_OK;
				return EVENT_HANDLED;
			}

			if (!strcmp((const char *)&(cmd_params->cmd)[4], "SCAN"))
			{
				proto_info("Scanning Networks");
				int numSsid = WiFi.scanNetworks();
				if (numSsid == -1)
				{
					proto_info("Failed to scan!");
					return EVENT_HANDLED;
				}

				// print the list of networks seen:
				proto_info("%d available networks", numSsid);

				// print the network number and name for each network found:
				for (int netid = 0; netid < numSsid; netid++)
				{
					proto_info("%d) %s\tSignal:  %ddBm", netid, WiFi.SSID(netid).c_str(), WiFi.RSSI(netid));
				}
				*(cmd_params->error) = STATUS_OK;
				return EVENT_HANDLED;
			}

			if (!strcmp((const char *)&(cmd_params->cmd)[4], "SAVE"))
			{
				settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
				proto_info("WiFi settings saved");
				*(cmd_params->error) = STATUS_OK;
				return EVENT_HANDLED;
			}

			if (!strcmp((const char *)&(cmd_params->cmd)[4], "RESET"))
			{
				settings_erase(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
				proto_info("WiFi settings deleted");
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
						proto_info("Invalid value. STA+AP(1), STA(2), AP(3)");
					}
				}

				switch (wifi_settings.wifi_mode)
				{
				case 0:
					proto_info("WiFi mode>STA+AP");
					break;
				case 1:
					proto_info("WiFi mode>STA");
					break;
				case 2:
					proto_info("WiFi mode>AP");
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
					proto_info("WiFi pass is too long");
					return EVENT_HANDLED;
				}
				memset(wifi_settings.pass, 0, sizeof(wifi_settings.pass));
				strcpy((char *)wifi_settings.pass, (const char *)arg);
				proto_info("WiFi password modified");
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
						proto_info("STA IP>%s", WiFi.localIP().toString().c_str());
						proto_info("AP IP>%s", WiFi.softAPIP().toString().c_str());
						break;
					case 2:
						proto_info("IP>%s", WiFi.localIP().toString().c_str());
						break;
					default:
						proto_info("IP>%s", WiFi.softAPIP().toString().c_str());
						break;
					}
				}
				else
				{
					proto_info("WiFi is off");
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
#ifdef ENABLE_SOCKETS
		static uint32_t next_info = 30000;
		static bool connected = false;

		// STA enabled only
		if (!wifi_settings.wifi_on || wifi_settings.wifi_mode > 1)
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
			proto_info("Disconnected from WiFi");
			return false;
		}

		if (!connected)
		{
			connected = true;
			proto_info("Connected to WiFi");
			proto_info("SSID>%s", wifi_settings.ssid);
			proto_info("IP>%s", WiFi.localIP().toString().c_str());
		}
#endif
		return false;
	}

#ifdef ENABLE_SOCKETS
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

	static Dir current_dir;
	bool flash_fs_next_file(fs_file_t *fp, fs_file_info_t *finfo)
	{
		ATOMIC_CODEBLOCK
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
		}
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
		char sanitized_mode[10];
		memset(sanitized_mode, 0, sizeof(sanitized_mode));
		// does not support binary format
		for (uint8_t i = 0, j = 0; i < strlen(mode); i++)
		{
			if (mode[i] != 'b')
			{
				sanitized_mode[j] = mode[i];
				j++;
			}
		}
		ATOMIC_CODEBLOCK
		{
			fs_file_t *fp = (fs_file_t *)calloc(1, sizeof(fs_file_t));
			if (fp)
			{
				fp->file_ptr = calloc(1, sizeof(File));
				if (fp->file_ptr)
				{
					*(static_cast<File *>(fp->file_ptr)) = FLASH_FS.open(path, sanitized_mode);
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
					fs_safe_free(fp->file_ptr);
				}
				fs_safe_free(fp);
			}
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
#endif

#ifdef ENABLE_SOCKETS
#include "../../../modules/net/http.h"
	// HTML form for firmware upload (simplified from ESP8266HTTPUpdateServer)
	static const char updateForm[] PROGMEM =
		"<!DOCTYPE html><html><body>"
		"<form method='POST' action='/update' enctype='multipart/form-data'>"
		"Firmware:<br><input type='file' name='firmware'>"
		"<input type='submit' value='Update'>"
		"</form></body></html>";
	const char type_html[] = "text/html";
	const char type_text[] = "text/plain";

	// HTML form for firmware upload (simplified from ESP8266HTTPUpdateServer)
	// Request handler for GET /update
	static void ota_page_cb(int client_idx)
	{
		http_send_str(client_idx, 200, (char *)type_html, (char *)updateForm);
		http_send(client_idx, 200, (char *)type_html, NULL, 0);
	}

	// File upload handler for POST /update
	static void ota_upload_cb(int client_idx)
	{
		http_upload_t up = http_file_upload_status(client_idx);

		if (up.status == HTTP_UPLOAD_START)
		{
#ifdef FLASH_FS
			if (!FLASH_FS.begin())
			{
				const char fail[] = "Flash error";
				http_send_str(client_idx, 415, (char *)type_text, (char *)fail);
				http_send(client_idx, 415, (char *)type_text, NULL, 0);
				return;
			}
#endif

			// Called once at start of upload
			proto_printf("Update start: %s\n", up.filename);
			uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
			if (!Update.begin(maxSketchSpace, U_FLASH))
			{
				Update.printError(Serial);
			}
		}
		else if (up.status == HTTP_UPLOAD_PART)
		{
			// Called for each chunk
			if (Update.write(up.data, up.datalen) != up.datalen)
			{
				Update.printError(Serial);
			}
		}
		else if (up.status == HTTP_UPLOAD_END)
		{
			// Called once at end of upload
			if (Update.end(true))
			{
				proto_printf("Update Success: %u bytes\r\n", up.datalen);
				const char suc[] = "Update Success! Rebooting...";
				http_send_str(client_idx, 200, (char *)type_text, (char *)suc);
				http_send(client_idx, 200, (char *)type_text, NULL, 0);
#ifdef FLASH_FS
				FLASH_FS.end();
#endif
				delay(100);
				ESP.restart();
			}
			else
			{
				const char fail[] = "Update Failed";
				http_send_str(client_idx, 500, (char *)type_text, (char *)fail);
				http_send(client_idx, 500, (char *)type_text, NULL, 0);
			}
		}
		else if (up.status == HTTP_UPLOAD_ABORT)
		{
			Update.end();
			proto_print("Update aborted\r\n");
		}
	}

	static char update_uri[] = "/update";
	void ota_server_start(void)
	{
		LOAD_MODULE(http_server);
		http_add(update_uri, HTTP_REQ_ANY, ota_page_cb, ota_upload_cb);
	}
#endif

	void esp8266_pre_init()
	{
#ifdef ENABLE_SOCKETS
		WiFi.setSleepMode(WIFI_NONE_SLEEP);
		WiFi.begin();

		ota_server_start();
#endif
	}
	void esp8266_wifi_init()
	{
		DBGMSG("Wifi assert");
#ifdef ENABLE_SOCKETS
		DBGMSG("Wifi startup");

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
			switch (wifi_settings.wifi_mode)
			{
			case 1:
				WiFi.mode(WIFI_STA);
				WiFi.begin((char *)wifi_settings.ssid, (char *)wifi_settings.pass);
				proto_info("Trying to connect to WiFi");
				break;
			case 2:
				WiFi.mode(WIFI_AP);
				WiFi.softAP(BOARD_NAME, (char *)wifi_settings.pass);
				proto_info("AP started");
				proto_info("SSID>" BOARD_NAME);
				proto_info("IP>%s", WiFi.softAPIP().toString().c_str());
				break;
			default:
				WiFi.mode(WIFI_AP_STA);
				WiFi.begin((char *)wifi_settings.ssid, (char *)wifi_settings.pass);
				proto_info("Trying to connect to WiFi");
				WiFi.softAP(BOARD_NAME, (char *)wifi_settings.pass);
				proto_info("AP started");
				proto_info("SSID>" BOARD_NAME);
				proto_info("IP>%s", WiFi.softAPIP().toString().c_str());
				break;
			}
		}
#endif

		// #if defined(ENABLE_SOCKETS)
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

#if defined(BOARD_HAS_CUSTOM_SYSTEM_COMMANDS) && defined(ENABLE_SOCKETS)
		ADD_EVENT_LISTENER(grbl_cmd, mcu_custom_grbl_cmd);
#endif
	}

	void esp8266_wifi_dotasks(void)
	{
		esp8266_wifi_clientok();
		yield();
	}
}

#endif