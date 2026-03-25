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

#if (defined(ESP32) || defined(ESP32S3) || defined(ESP32C3))
#include <Arduino.h>
#include "esp_task_wdt.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

extern "C"
{
#include "../../../cnc.h"
}

#ifndef BT_ID_MAX_LEN
#define BT_ID_MAX_LEN 32
#endif

#ifndef WIFI_SSID_MAX_LEN
#define WIFI_SSID_MAX_LEN 32
#endif

#define ARG_MAX_LEN MAX(WIFI_SSID_MAX_LEN, BT_ID_MAX_LEN)

#ifdef ENABLE_WIFI
#include <Update.h>

#ifndef WIFI_USER
#define WIFI_USER "admin"
#endif

#ifndef WIFI_PASS
#define WIFI_PASS "pass"
#endif

#ifndef OTA_URI
#define OTA_URI "/update"
#endif

typedef struct
{
	uint8_t wifi_on;
	uint8_t wifi_mode;
	uint8_t ssid[WIFI_SSID_MAX_LEN];
	uint8_t pass[WIFI_SSID_MAX_LEN];
} wifi_settings_t;

uint16_t wifi_settings_offset;
wifi_settings_t wifi_settings;

#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/inet.h"

static esp_netif_t *netif_sta = NULL;
static esp_netif_t *netif_ap = NULL;
static bool wifi_initialized = false;

void esp32_wifi_stop(void)
{
	if (!wifi_initialized)
		return;

	esp_wifi_stop();
	esp_wifi_deinit();

	if (netif_sta)
	{
		esp_netif_destroy(netif_sta);
		netif_sta = NULL;
	}
	if (netif_ap)
	{
		esp_netif_destroy(netif_ap);
		netif_ap = NULL;
	}

	wifi_initialized = false;
}

uint32_t esp32_wifi_get_ip(void)
{
	if (!wifi_initialized)
		return 0;

	if (wifi_settings.wifi_mode == 2) // AP only
		return 0;

	esp_netif_ip_info_t ip;
	if (netif_sta && esp_netif_get_ip_info(netif_sta, &ip) == ESP_OK)
		return ip.ip.addr;

	return 0;
}

uint32_t esp32_wifi_ap_get_ip(void)
{
	if (!wifi_initialized)
		return 0;

	if (wifi_settings.wifi_mode == 1) // STA only
		return 0;

	esp_netif_ip_info_t ip;
	if (netif_ap && esp_netif_get_ip_info(netif_ap, &ip) == ESP_OK)
		return ip.ip.addr;

	return 0;
}

void esp32_wifi_config(bool force)
{
	/* Always stop and re-init to avoid errors */
	esp32_wifi_stop();

	if (!force && wifi_settings.wifi_on == 0)
	{
		return;
	}

	ESP_ERROR_CHECK(esp_netif_init());

	/* Create interfaces */
	if (wifi_settings.wifi_mode != 2)
	{
		netif_sta = esp_netif_create_default_wifi_sta();
	}
	if (wifi_settings.wifi_mode != 1)
	{
		netif_ap = esp_netif_create_default_wifi_ap();
	}

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	/* Disable power saving */
	esp_wifi_set_ps(WIFI_PS_NONE);

	wifi_config_t wifi_cfg;
	memset(&wifi_cfg, 0, sizeof(wifi_cfg));

	/* STA CONFIG */
	if (wifi_settings.wifi_mode != 2)
	{
		strncpy((char *)wifi_cfg.sta.ssid, (const char *)wifi_settings.ssid, sizeof(wifi_cfg.sta.ssid));
		strncpy((char *)wifi_cfg.sta.password, (const char *)wifi_settings.pass, sizeof(wifi_cfg.sta.password));
		wifi_cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
		wifi_cfg.sta.pmf_cfg.capable = true;
		wifi_cfg.sta.pmf_cfg.required = false;

#ifdef USE_STATIC_IP
		esp_netif_ip_info_t ip;
		ip.ip.addr = STATIC_IP_IP;
		ip.gw.addr = STATIC_IP_GW;
		ip.netmask.addr = STATIC_IP_SUB;
		esp_netif_dhcpc_stop(netif_sta);
		esp_netif_set_ip_info(netif_sta, &ip);
#endif
	}

	/* AP CONFIG */
	if (wifi_settings.wifi_mode != 1)
	{
		strncpy((char *)wifi_cfg.ap.ssid, BOARD_NAME, sizeof(wifi_cfg.ap.ssid));
		wifi_cfg.ap.ssid_len = strlen(BOARD_NAME);
		wifi_cfg.ap.channel = 1;
		wifi_cfg.ap.max_connection = 4;
		wifi_cfg.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
		strncpy((char *)wifi_cfg.ap.password, (const char *)wifi_settings.pass, sizeof(wifi_cfg.ap.password));
		if (strlen((const char *)wifi_settings.pass) == 0)
			wifi_cfg.ap.authmode = WIFI_AUTH_OPEN;
	}

	/* Set mode */
	wifi_mode_t mode = WIFI_MODE_NULL;
	if (wifi_settings.wifi_mode == 0)
		mode = WIFI_MODE_APSTA;
	if (wifi_settings.wifi_mode == 1)
		mode = WIFI_MODE_STA;
	if (wifi_settings.wifi_mode == 2)
		mode = WIFI_MODE_AP;

	ESP_ERROR_CHECK(esp_wifi_set_mode(mode));

	/* Apply configs */
	if (wifi_settings.wifi_mode != 2)
		ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));

	if (wifi_settings.wifi_mode != 1)
		ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_cfg));

	ESP_ERROR_CHECK(esp_wifi_start());

	if (wifi_settings.wifi_mode != 2)
		esp_wifi_connect();

	wifi_initialized = true;
}

uint8_t esp32_wifi_scan(void)
{
	uint16_t ap_count = 0;
	wifi_ap_record_t *list = NULL;

	esp_wifi_scan_start(NULL, true);
	esp_wifi_scan_get_ap_num(&ap_count);

	if (ap_count == 0)
		return 0;

	list = (wifi_ap_record_t *)calloc(ap_count, sizeof(wifi_ap_record_t));
	esp_wifi_scan_get_ap_records(&ap_count, list);

	for (uint16_t i = 0; i < ap_count; i++)
	{
		proto_info("%d) %s\tSignal: %d dBm",
				   i + 1,
				   (char *)list[i].ssid,
				   list[i].rssi);
	}

	free(list);
	return ap_count;
}
#endif

/**
 * Custom WiFi+BT commands
 */
#ifdef BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
extern "C"
{
	bool mcu_custom_grbl_cmd(void *args)
	{
		grbl_cmd_args_t *cmd_params = (grbl_cmd_args_t *)args;
		char arg[ARG_MAX_LEN];
		uint8_t has_arg = (cmd_params->next_char == '=');
		memset(arg, 0, sizeof(arg));

#ifdef ENABLE_WIFI
		if (!strncmp((const char *)(cmd_params->cmd), "WIFI", 4))
		{
			if (!strcmp((const char *)&(cmd_params->cmd)[4], "ON"))
			{
				wifi_settings.wifi_on = 1;
				esp32_wifi_config(false);

				settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
				*(cmd_params->error) = STATUS_OK;
				return EVENT_HANDLED;
			}

			if (!strcmp((const char *)&(cmd_params->cmd)[4], "OFF"))
			{
				wifi_settings.wifi_on = 0;
				esp32_wifi_config(false);
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
				// Serial.println("[MSG:Scanning Networks]");
				proto_info("Scanning Networks");
				int numSsid = esp32_wifi_scan();
				// int numSsid = WiFi.scanNetworks();
				if (numSsid == -1)
				{
					proto_info("Failed to scan!");
					return EVENT_HANDLED;
				}

				// print the list of networks seen:
				proto_info("%d available networks", numSsid);

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
						proto_info("IP>%I", esp32_wifi_get_ip());
						break;
					case 2:
						proto_info("IP>%I", esp32_wifi_ap_get_ip());
						break;
					default:
						proto_info("STA IP>%I", esp32_wifi_get_ip());
						proto_info("AP IP>%I", esp32_wifi_ap_get_ip());
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
}
#endif

/**
 * Flash File System
 */
#ifdef ENABLE_SOCKETS

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
				fs_safe_free(fp->file_ptr);
			}
			fs_safe_free(fp);
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
}
#endif

/**
 * OTA
 */
#ifdef ENABLE_SOCKETS
extern "C"
{
#include "../../../modules/net/http.h"
	// HTML form for firmware upload (simplified from ESP8266HTTPUpdateServer)
	static const char updateForm[] __rom__ =
		"<!DOCTYPE html><html><body>"
		"<form method='POST' action='" OTA_URI "' enctype='multipart/form-data'>"
		"Firmware:<br><input type='file' name='firmware'>"
		"<input type='submit' value='Update'>"
		"</form></body></html>";
	const char type_html[] = "text/html";
	const char type_text[] = "text/plain";

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
			ESP_LOGI("OTA", "Update start: %s\n", up.filename);
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
				const char suc[] = "Update Success! Rebooting...";
				ESP_LOGI("OTA", "Update Success: %lu bytes\r\n", up.datalen);
				http_send_str(client_idx, 200, (char *)type_text, (char *)suc);
				http_send(client_idx, 200, (char *)type_text, NULL, 0);
				cnc_delay_ms(100);
				ESP.restart();
			}
			else
			{
				// Update.printError(Serial);
				const char fail[] = "Update Failed";
				http_send_str(client_idx, 500, (char *)type_text, (char *)fail);
				http_send(client_idx, 500, (char *)type_text, NULL, 0);
				cnc_delay_ms(100);
				ESP.restart();
			}
		}
		else if (up.status == HTTP_UPLOAD_ABORT)
		{
			Update.end();
			proto_printf("Update aborted\r\n");
			cnc_delay_ms(100);
			ESP.restart();
		}
	}

	void ota_server_start(void)
	{
		RUNONCE
		{
			LOAD_MODULE(http_server);
			http_add(OTA_URI, HTTP_REQ_ANY, ota_page_cb, ota_upload_cb);
			RUNONCE_COMPLETE();
		}
	}
}
#endif

/**
 * Custom SOCKETS
 */
#if defined(ENABLE_WIFI)
#include "../../../modules/net/socket.h"
#include <errno.h>
#include <fcntl.h>
#include "lwip/sockets.h"

int bsd_socket(int domain, int type, int protocol)
{
	int fd = lwip_socket(domain, type, protocol);
	if (fd < 0)
		return -1;

	lwip_fcntl(fd, F_SETFL, O_NONBLOCK);
	return fd;
}

int bsd_bind(int sockfd, const struct bsd_sockaddr_in *addr, int addrlen)
{
	struct sockaddr_in a;
	memset(&a, 0, sizeof(a));
	a.sin_family = AF_INET;
	a.sin_port = addr->sin_port;
	a.sin_addr.s_addr = addr->sin_addr;
	return lwip_bind(sockfd, (struct sockaddr *)&a, sizeof(a));
}

int bsd_listen(int sockfd, int backlog)
{
	return lwip_listen(sockfd, backlog);
}

int bsd_accept(int sockfd, struct bsd_sockaddr_in *addr, int *addrlen)
{
	struct sockaddr_in a;
	socklen_t len = sizeof(a);

	int fd = lwip_accept(sockfd, (struct sockaddr *)&a, &len);
	if (fd < 0)
		return -1;

	lwip_fcntl(fd, F_SETFL, O_NONBLOCK);

	if (addr)
	{
		addr->sin_family = a.sin_family;
		addr->sin_port = a.sin_port;
		addr->sin_addr = a.sin_addr.s_addr;
	}
	if (addrlen)
		*addrlen = sizeof(struct bsd_sockaddr_in);

	return fd;
}

int bsd_recv(int sockfd, void *buf, size_t len, int flags)
{
	int r = lwip_recv(sockfd, buf, len, flags);
	if (r < 0 && errno == EWOULDBLOCK)
		return -1;
	return r;
}

int bsd_send(int sockfd, const void *buf, size_t len, int flags)
{
	int r = lwip_send(sockfd, buf, len, flags);
	if (r < 0 && errno == EWOULDBLOCK)
		return -1;
	return r;
}

int bsd_close(int fd)
{
	return lwip_close(fd);
}

socket_device_t wifi_socket = {
	.socket = bsd_socket,
	.bind = bsd_bind,
	.listen = bsd_listen,
	.accept = bsd_accept,
	.recv = bsd_recv,
	.send = bsd_send,
	.close = bsd_close};

#endif

#if defined(ENABLE_SOCKETS)
#include "../../../module.h"
static void mcu_wifi_task(void *arg)
{
	esp32_wifi_config(false);

	for (;;)
	{
		if (wifi_settings.wifi_on)
		{
#if defined(ENABLE_SOCKETS) && defined(MCU_HAS_RTOS)
			socket_server_dotasks();
#endif
		}
		vTaskDelay(1);
	}
}
#endif

extern "C"
{
#include "../../../modules/net/socket.h"
	void esp32_pre_init(void)
	{
		static bool event_loop_created = false;

		if (!event_loop_created)
		{
			ESP_ERROR_CHECK(esp_event_loop_create_default());
			event_loop_created = true;
		}

#ifdef ENABLE_WIFI
		esp32_wifi_config(true);
		// register WiFi as the device default network device
		socket_register_device(&wifi_socket);
#endif
	}

	void mcu_wifi_init(void)
	{
		if (FLASH_FS.begin())
		{
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
		}

#ifdef ENABLE_WIFI
		ota_server_start();

		wifi_settings_offset = settings_register_external_setting(sizeof(wifi_settings_t));
		if (settings_load(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t)))
		{
			wifi_settings = {0};
			memcpy(wifi_settings.ssid, BOARD_NAME, strlen((const char *)BOARD_NAME));
			memcpy(wifi_settings.pass, WIFI_PASS, strlen((const char *)WIFI_PASS));
			settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
		}

		xTaskCreatePinnedToCore(mcu_wifi_task, "wifiTask", 8192, NULL, 1, NULL, CONFIG_ARDUINO_RUNNING_CORE);
#endif

#ifdef BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
		ADD_EVENT_LISTENER(grbl_cmd, mcu_custom_grbl_cmd);
#endif
	}
}

#endif
