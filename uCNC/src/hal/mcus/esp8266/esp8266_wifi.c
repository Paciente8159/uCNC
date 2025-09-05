// /*
// 	Name: esp8266_wifi.c
// 	Description: Wifi for ESP8266.

// 	Copyright: Copyright (c) João Martins
// 	Author: João Martins
// 	Date: 22-08-2025

// 	µCNC is free software: you can redistribute it and/or modify
// 	it under the terms of the GNU General Public License as published by
// 	the Free Software Foundation, either version 3 of the License, or
// 	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

// 	µCNC is distributed WITHOUT ANY WARRANTY;
// 	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// 	See the	GNU General Public License for more details.
// */

// #include "../../../cnc.h"

// #if (MCU == MCU_ESP8266)

// #include <stdint.h>
// #include <string.h>
// #include <stdbool.h>
// #include "c_types.h"
// #include "ets_sys.h"
// #include "osapi.h"
// #include "user_interface.h"
// #include "mem.h"
// #include "lwip/dhcp.h"
// #include "lwip/dns.h"
// #include "lwip/inet.h"

// #ifndef WIFI_USER
// #define WIFI_USER "admin"
// #endif

// #ifndef WIFI_PASS
// #define WIFI_PASS "pass"
// #endif

// #ifndef WIFI_SSID_MAX_LEN
// #define WIFI_SSID_MAX_LEN 32
// #endif

// #define ARG_MAX_LEN WIFI_SSID_MAX_LEN

// #define WIFI_MODE_OFF 0
// #define WIFI_MODE_STA 1
// #define WIFI_MODE_AP 2
// #define WIFI_MODE_STA_AP (WIFI_MODE_STA | WIFI_MODE_AP)

// typedef struct
// {
// 	uint8_t wifi_on;
// 	uint8_t wifi_mode;
// 	char ssid[WIFI_SSID_MAX_LEN];
// 	char pass[WIFI_SSID_MAX_LEN];
// } wifi_settings_t;

// uint16_t wifi_settings_offset;
// wifi_settings_t wifi_settings;

// static inline void safe_strcpy(char *dst, const char *src, size_t dstsz)
// {
// 	if (!dst || !dstsz)
// 		return;
// 	if (!src)
// 	{
// 		dst[0] = '\0';
// 		return;
// 	}
// 	size_t n = os_strlen(src);
// 	if (n >= dstsz)
// 		n = dstsz - 1;
// 	os_memcpy(dst, src, n);
// 	dst[n] = '\0';
// }

// static void ensure_dhcp_sta_started(void)
// {
// 	// Start DHCP client on STA if not started
// 	wifi_station_dhcpc_start();
// }

// static void ensure_dhcps_ap_started(void)
// {
// 	// Ensure SoftAP has a sane IP and DHCP server started (192.168.4.1/24 by default)
// 	struct ip_info ip;
// 	if (!wifi_get_ip_info(SOFTAP_IF, &ip) || ip.ip.addr == 0)
// 	{
// 		// Set default 192.168.4.1/24
// 		struct ip_info info;
// 		info.ip.addr = IPADDR_ANY; // default first
// 		info.gw.addr = IPADDR_ANY;
// 		info.netmask.addr = IPADDR_ANY;

// 		// Stop DHCP server while changing IP info
// 		wifi_softap_dhcps_stop();

// 		info.ip.addr = ipaddr_addr("192.168.4.1");
// 		info.gw.addr = ipaddr_addr("192.168.4.1");
// 		info.netmask.addr = ipaddr_addr("255.255.255.0");
// 		wifi_set_ip_info(SOFTAP_IF, &info);

// 		// Restart DHCP server
// 		wifi_softap_dhcps_start();
// 	}
// 	else
// 	{
// 		// Make sure DHCP server is running
// 		wifi_softap_dhcps_start();
// 	}
// }

// // ---------- API implementation ----------

// // connect to a wifi network
// void esp8266_wifi_sta_init(char *ssid, char *pass)
// {
// 	// Disable all Wi-Fi sleep/power saving for lowest latency
// 	wifi_set_sleep_type(NONE_SLEEP_T);
// 	// Set mode to STA (will not disturb AP if user later enables STA+AP via set_mode)
// 	wifi_set_opmode_current(STATION_MODE);

// 	// Prepare station config
// 	struct station_config conf;
// 	os_memset(&conf, 0, sizeof(conf));

// 	// SSID (max 32), password (0/open or 8..64 for WPA/WPA2 or 64 hex PSK)
// 	// The NON-OS SDK expects up to 32 bytes in ssid, 64 in password (no need for explicit null if length is exact).
// 	safe_strcpy((char *)conf.ssid, ssid, sizeof(conf.ssid));
// 	safe_strcpy((char *)conf.password, pass ? pass : "", sizeof(conf.password));

// 	// Apply config (current, not persistent)
// 	wifi_station_set_config_current(&conf);

// 	// Start DHCP and connect
// 	ensure_dhcp_sta_started();
// 	wifi_station_connect();
// }

// // get the current IP address (STA). Returns IPv4 in host byte order.
// uint32_t esp8266_wifi_ip()
// {
// 	struct ip_info ip;
// 	if (wifi_get_ip_info(STATION_IF, &ip) && ip.ip.addr != 0)
// 	{
// 		// Convert from network to host byte order for convenience
// 		return ntohl(ip.ip.addr);
// 	}
// 	return 0;
// }

// // set wifi mode (1-STA+AP, 2-STA only, 3-AP only)
// void esp8266_wifi_set_mode(uint8_t mode)
// {
// 	uint8 opmode = NULL_MODE;
// 	switch (mode)
// 	{
// 	case WIFI_MODE_STA_AP:
// 		opmode = STATIONAP_MODE;
// 		break; // STA+AP
// 	case WIFI_MODE_STA:
// 		opmode = STATION_MODE;
// 		break; // STA only
// 	case WIFI_MODE_AP:
// 		opmode = SOFTAP_MODE;
// 		break; // AP only
// 	default:
// 		opmode = NULL_MODE;
// 		break;
// 	}
// 	wifi_set_opmode_current(opmode);
// }

// // start an Access point
// void esp8266_wifi_ap_init(char *ap_name, char *pass)
// {
// 	// Ensure AP mode enabled (will coexist if user selected STA+AP)
// 	wifi_set_opmode_current((wifi_get_opmode() == STATION_MODE) ? STATIONAP_MODE : SOFTAP_MODE);

// 	struct softap_config conf;
// 	os_memset(&conf, 0, sizeof(conf));

// 	// SSID
// 	size_t ssid_len = ap_name ? os_strlen(ap_name) : 0;
// 	if (ssid_len > sizeof(conf.ssid))
// 		ssid_len = sizeof(conf.ssid);
// 	os_memcpy(conf.ssid, ap_name, ssid_len);
// 	conf.ssid_len = ssid_len;

// 	// Password and auth mode
// 	size_t psk_len = pass ? os_strlen(pass) : 0;
// 	if (psk_len >= 8 && psk_len <= 64)
// 	{
// 		os_memcpy(conf.password, pass, (psk_len > sizeof(conf.password) ? sizeof(conf.password) : psk_len));
// 		conf.authmode = AUTH_WPA2_PSK;
// 	}
// 	else
// 	{
// 		// Open network if password is empty or invalid-length
// 		conf.password[0] = 0;
// 		conf.authmode = AUTH_OPEN;
// 	}

// 	// Defaults: channel 1, broadcast SSID, max 4 clients, 100 ms beacon
// 	conf.channel = 1;
// 	conf.ssid_hidden = 0;
// 	conf.max_connection = 4;
// 	conf.beacon_interval = 100;

// 	// Apply config (current, not persistent)
// 	wifi_softap_set_config_current(&conf);

// 	// Ensure SoftAP IP and DHCP server are sane/started
// 	ensure_dhcps_ap_started();
// }

// // get the AP IP. Returns IPv4 in host byte order.
// uint32_t esp8266_wifi_ap_ip()
// {
// 	struct ip_info ip;
// 	if (wifi_get_ip_info(SOFTAP_IF, &ip) && ip.ip.addr != 0)
// 	{
// 		return ntohl(ip.ip.addr);
// 	}
// 	return 0;
// }

// // scans and prints all available networks
// // format: ><ssid> <channel> <transmission power>dBm  (power == RSSI)
// static volatile bool g_scan_done = false;

// static void ICACHE_FLASH_ATTR scan_done_cb(void *arg, STATUS status)
// {
// 	g_scan_done = true;
// 	if (status != OK || arg == NULL)
// 	{
// 		os_printf("Scan failed (status=%d)\n", status);
// 		return;
// 	}

// 	struct bss_info *bss = (struct bss_info *)arg;
// 	// SDK returns a STAILQ linked list starting with 'bss'
// 	for (struct bss_info *it = bss; it != NULL; it = STAILQ_NEXT(it, next))
// 	{
// 		char ssid[33];
// 		os_memset(ssid, 0, sizeof(ssid));
// 		// SSID field in bss_info is 32 bytes, not necessarily null-terminated
// 		os_memcpy(ssid, it->ssid, 32);
// 		int ch = it->channel;
// 		int rssi = it->rssi; // dBm
// 		os_printf("><%s %d %ddBm\n", ssid, ch, rssi);
// 	}
// }

// void esp8266_wifi_scan(void)
// {
// 	// Ensure STA is enabled for scanning and disconnected (SDK scans on STA interface)
// 	wifi_set_opmode_current((wifi_get_opmode() == SOFTAP_MODE) ? STATIONAP_MODE : STATION_MODE);
// 	wifi_station_disconnect();

// 	struct scan_config cfg;
// 	os_memset(&cfg, 0, sizeof(cfg));
// 	cfg.ssid = NULL; // all SSIDs
// 	cfg.bssid = NULL;
// 	cfg.channel = 0; // all channels
// 	cfg.show_hidden = 0;

// 	g_scan_done = false;
// 	if (!wifi_station_scan(&cfg, scan_done_cb))
// 	{
// 		os_printf("Scan start failed\n");
// 		return;
// 	}

// 	// Optionally wait (non-busy) until scan callback runs.
// 	// In NON-OS SDK, avoid tight busy-wait; here we just return and let callback print.
// 	// If a blocking wait is required, integrate with system_os_task() and post events.
// }

// // Disconnect from all Wi-Fi (STA and AP) and optionally turn off Wi-Fi hardware
// void esp8266_wifi_disconnect_all(bool wifioff)
// {
// 	// Disconnect STA
// 	wifi_station_disconnect();

// 	// Clear STA config (optional — prevents auto‑reconnect until re‑set)
// 	struct station_config sta_conf;
// 	os_memset(&sta_conf, 0, sizeof(sta_conf));
// 	wifi_station_set_config_current(&sta_conf);

// 	// Disconnect AP: disable SSID/password and stop DHCP server
// 	wifi_softap_dhcps_stop();
// 	struct softap_config ap_conf;
// 	os_memset(&ap_conf, 0, sizeof(ap_conf));
// 	ap_conf.authmode = AUTH_OPEN;
// 	wifi_softap_set_config_current(&ap_conf);

// 	if (wifioff)
// 	{
// 		// Fully turn off Wi-Fi
// 		wifi_set_opmode_current(NULL_MODE);
// 	}
// }

// // Return value is the SDK's STATION_STATUS_* enum cast to uint8_t.
// // Common values:
// //  STATION_IDLE        = 0
// //  STATION_CONNECTING  = 1
// //  STATION_WRONG_PASSWORD = 2
// //  STATION_NO_AP_FOUND = 3
// //  STATION_CONNECT_FAIL = 4
// //  STATION_GOT_IP      = 5
// uint8_t esp8266_sta_get_status(void)
// {
// 	return (uint8_t)wifi_station_get_connect_status();
// }

// void esp8266_wifi_on(void)
// {
// 	__ATOMIC__
// 	{
// 		esp8266_wifi_disconnect_all(true);
// 		esp8266_wifi_set_mode(wifi_settings.wifi_mode);
// 		if (wifi_settings.wifi_mode & WIFI_MODE_STA)
// 		{
// 			esp8266_wifi_sta_init(wifi_settings.ssid, wifi_settings.pass);
// 			proto_info("Trying to connect to WiFi");
// 			break;
// 		}
// 		if (wifi_settings.wifi_mode & WIFI_MODE_AP)
// 		{
// 			esp8266_wifi_ap_init(BOARD_NAME, wifi_settings.pass);
// 			proto_info("AP started");
// 			proto_info("SSID>" BOARD_NAME);
// 			proto_info("IP>%I", esp8266_wifi_ap_ip());
// 			break;
// 		}

// 		wifi_settings.wifi_on = 1;
// 	}
// }

// void esp8266_wifi_off(void)
// {
// 	__ATOMIC__
// 	{
// 		wifi_settings.wifi_on = 0;
// 		esp8266_wifi_disconnect_all(true);
// 	}
// }

// #ifdef BOARD_HAS_CUSTOM_SYSTEM_COMMANDS

// bool mcu_custom_grbl_cmd(void *args)
// {
// 	grbl_cmd_args_t *cmd_params = (grbl_cmd_args_t *)args;
// 	char arg[ARG_MAX_LEN];
// 	uint8_t has_arg = (cmd_params->next_char == '=');
// 	memset(arg, 0, sizeof(arg));

// #ifdef ENABLE_SOCKETS
// 	if (!strncmp((const char *)(cmd_params->cmd), "WIFI", 4))
// 	{
// 		if (!strcmp((const char *)&(cmd_params->cmd)[4], "ON"))
// 		{
// 			esp8266_wifi_on();
// 			settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
// 			*(cmd_params->error) = STATUS_OK;
// 			return EVENT_HANDLED;
// 		}

// 		if (!strcmp((const char *)&(cmd_params->cmd)[4], "OFF"))
// 		{
// 			esp8266_wifi_off();
// 			settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
// 			*(cmd_params->error) = STATUS_OK;
// 			return EVENT_HANDLED;
// 		}

// 		if (!strcmp((const char *)&(cmd_params->cmd)[4], "SSID"))
// 		{
// 			if (has_arg)
// 			{
// 				int8_t len = parser_get_grbl_cmd_arg(arg, ARG_MAX_LEN);

// 				if (len < 0)
// 				{
// 					*(cmd_params->error) = STATUS_INVALID_STATEMENT;
// 					return EVENT_HANDLED;
// 				}

// 				if (len > WIFI_SSID_MAX_LEN)
// 				{
// 					proto_info("WiFi SSID is too long");
// 				}
// 				memset(wifi_settings.ssid, 0, sizeof(wifi_settings.ssid));
// 				strcpy((char *)wifi_settings.ssid, (const char *)arg);
// 				settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
// 				proto_info("WiFi SSID modified");
// 			}
// 			else
// 			{
// 				proto_info("SSID>%s", wifi_settings.ssid);
// 			}
// 			*(cmd_params->error) = STATUS_OK;
// 			return EVENT_HANDLED;
// 		}

// 		if (!strcmp((const char *)&(cmd_params->cmd)[4], "SCAN"))
// 		{
// 			proto_info("Scanning Networks");
// 			esp8266_wifi_scan();
// 			*(cmd_params->error) = STATUS_OK;
// 			return EVENT_HANDLED;
// 		}

// 		if (!strcmp((const char *)&(cmd_params->cmd)[4], "SAVE"))
// 		{
// 			settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
// 			proto_info("WiFi settings saved");
// 			*(cmd_params->error) = STATUS_OK;
// 			return EVENT_HANDLED;
// 		}

// 		if (!strcmp((const char *)&(cmd_params->cmd)[4], "RESET"))
// 		{
// 			settings_erase(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
// 			proto_info("WiFi settings deleted");
// 			*(cmd_params->error) = STATUS_OK;
// 			return EVENT_HANDLED;
// 		}

// 		if (!strcmp((const char *)&(cmd_params->cmd)[4], "MODE"))
// 		{
// 			if (has_arg)
// 			{
// 				int8_t len = parser_get_grbl_cmd_arg(arg, ARG_MAX_LEN);

// 				if (len < 0)
// 				{
// 					*(cmd_params->error) = STATUS_INVALID_STATEMENT;
// 					return EVENT_HANDLED;
// 				}

// 				int mode = atoi((const char *)arg);
// 				if (mode >= 0)
// 				{
// 					wifi_settings.wifi_mode = mode;
// 				}
// 				else
// 				{
// 					proto_info("Invalid value. OFF(0), STA(1), AP(2), STA+AP(3)");
// 				}
// 			}

// 			switch (wifi_settings.wifi_mode)
// 			{
// 			case 3:
// 				proto_info("WiFi mode>STA+AP");
// 				break;
// 			case 1:
// 				proto_info("WiFi mode>STA");
// 				break;
// 			case 2:
// 				proto_info("WiFi mode>AP");
// 				break;
// 			default:
// 				proto_info("WiFi mode>OFF");
// 				break;
// 			}
// 			*(cmd_params->error) = STATUS_OK;
// 			return EVENT_HANDLED;
// 		}

// 		if (!strcmp((const char *)&(cmd_params->cmd)[4], "PASS") && has_arg)
// 		{
// 			int8_t len = parser_get_grbl_cmd_arg(arg, ARG_MAX_LEN);

// 			if (len < 0)
// 			{
// 				*(cmd_params->error) = STATUS_INVALID_STATEMENT;
// 				return EVENT_HANDLED;
// 			}

// 			if (len > WIFI_SSID_MAX_LEN)
// 			{
// 				proto_info("WiFi pass is too long");
// 				return EVENT_HANDLED;
// 			}
// 			memset(wifi_settings.pass, 0, sizeof(wifi_settings.pass));
// 			strcpy((char *)wifi_settings.pass, (const char *)arg);
// 			proto_info("WiFi password modified");
// 			*(cmd_params->error) = STATUS_OK;
// 			return EVENT_HANDLED;
// 		}

// 		if (!strcmp((const char *)&(cmd_params->cmd)[4], "IP"))
// 		{
// 			if (wifi_settings.wifi_on)
// 			{
// 				if (wifi_settings.wifi_mode & WIFI_MODE_STA)
// 				{
// 					proto_info("STA IP>%I", esp8266_wifi_ip());
// 				}

// 				if (wifi_settings.wifi_mode & WIFI_MODE_AP)
// 				{
// 					proto_info("AP IP>%s", esp8266_wifi_ap_ip());
// 				}
// 			}
// 			else
// 			{
// 				proto_info("WiFi is off");
// 			}

// 			*(cmd_params->error) = STATUS_OK;
// 			return EVENT_HANDLED;
// 		}
// 	}
// #endif
// 	return EVENT_CONTINUE;
// }

// CREATE_EVENT_LISTENER(grbl_cmd, mcu_custom_grbl_cmd);
// #endif

// void esp8266_wifi_init()
// {
// 	DBGMSG("Wifi assert");
// #ifdef ENABLE_SOCKETS
// 	DBGMSG("Wifi startup");

// 	wifi_settings_offset = settings_register_external_setting(sizeof(wifi_settings_t));
// 	if (settings_load(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t)))
// 	{
// 		memset(&wifi_settings, 0, sizeof(wifi_settings));
// 		memcpy(wifi_settings.ssid, BOARD_NAME, strlen((const char *)BOARD_NAME));
// 		memcpy(wifi_settings.pass, WIFI_PASS, strlen((const char *)WIFI_PASS));
// 		settings_save(wifi_settings_offset, (uint8_t *)&wifi_settings, sizeof(wifi_settings_t));
// 	}

// 	if (wifi_settings.wifi_on)
// 	{
// 		esp8266_wifi_set_mode(wifi_settings.wifi_mode);
// 		esp8266_wifi_on();
// 	}
// 	else
// 	{
// 		esp8266_wifi_off();
// 	}
// 	// telnet_server.begin();
// 	// telnet_server.setNoDelay(true);
// 	// LOAD_MODULE(socket_server);
// 	// LOAD_MODULE(telnet_server);
// 	// extern socket_if_t *telnet_sock;
// 	// extern const telnet_protocol_t telnet_proto;
// 	// telnet_sock = telnet_start_listen(&telnet_proto, 23);

// // #ifdef MCU_HAS_ENDPOINTS
// // 	FLASH_FS.begin();
// // 	flash_fs = {
// // 			.drive = 'C',
// // 			.open = flash_fs_open,
// // 			.read = flash_fs_read,
// // 			.write = flash_fs_write,
// // 			.seek = flash_fs_seek,
// // 			.available = flash_fs_available,
// // 			.close = flash_fs_close,
// // 			.remove = flash_fs_remove,
// // 			.opendir = flash_fs_opendir,
// // 			.mkdir = flash_fs_mkdir,
// // 			.rmdir = flash_fs_rmdir,
// // 			.next_file = flash_fs_next_file,
// // 			.finfo = flash_fs_info,
// // 			.next = NULL};
// // 	fs_mount(&flash_fs);
// // #endif
// // #ifndef CUSTOM_OTA_ENDPOINT
// // 	httpUpdater.setup(&web_server, OTA_URI, update_username, update_password);
// // #endif
// // 	web_server.begin();

// // #ifdef MCU_HAS_WEBSOCKETS
// // 	socket_server.begin();
// // 	socket_server.onEvent(webSocketEvent);
// // #endif
// #endif

// #ifdef BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
// 	ADD_EVENT_LISTENER(grbl_cmd, mcu_custom_grbl_cmd);
// #endif
// }

// void esp8266_wifi_dotasks(void) {}

// #endif
