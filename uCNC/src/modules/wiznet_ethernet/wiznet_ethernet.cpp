/*
	Name: wiznet_ethernet.c
	Description: Implements a Wiznet Ethernet interface for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 09-07-2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "src/Ethernet.h"

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

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
uint8_t eth_mac[] = {
		0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress eth_ip(192, 168, 1, 200);
IPAddress eth_dns(192, 168, 1, 254);
EthernetServer eth_telnet_server(TELNET_PORT);
EthernetClient eth_telnet_client;

#ifdef __cplusplus
extern "C"
{
#endif

#include "wiznet_ethernet.h"

	bool eth_clientok(void)
	{
		static uint32_t next_info = 30000;
		static bool connected = false;
		uint8_t str[64];

		static bool eth_started = false;

		if (!eth_started)
		{
			protocol_send_feedback("Ethernet start DHCP");
			Ethernet.begin(eth_mac /*eth_ip, eth_dns, eth_dns*/);
			if (Ethernet.hardwareStatus() == EthernetNoHardware)
			{
				protocol_send_feedback("Ethernet shield was not found. :(");
				return false;
			}
			eth_started = true;
		}

		if (Ethernet.linkStatus() != LinkON)
		{
			connected = false;
			if (next_info > mcu_millis())
			{
				return false;
			}
			next_info = mcu_millis() + 30000;
			protocol_send_feedback("Disconnected from ETH");
			return false;
		}

		if (!connected)
		{
			connected = true;
			protocol_send_feedback("Connected to ETH");
			sprintf((char *)str, "IP>%s", Ethernet.localIP().toString().c_str());
			protocol_send_feedback((const char *)str);
			eth_telnet_server.begin();
		}

		EthernetClient client = eth_telnet_server.available();
		if (client)
		{
			if (eth_telnet_client)
			{
				if (eth_telnet_client.connected())
				{
					eth_telnet_client.stop();
				}
			}
			eth_telnet_client = eth_telnet_server.accept();
			eth_telnet_client.println("[MSG:New client connected]");
			return true;
		}
		else if (eth_telnet_client)
		{
			if (eth_telnet_client.connected())
			{
				return true;
			}
		}
		return false;
	}

	bool eth_print_info(void *args)
	{
		// grbl_cmd_args_t *cmd_params = (grbl_cmd_args_t *)args;
		// uint8_t str[64];
		// char arg[ARG_MAX_LEN];
		// uint8_t has_arg = (cmd_params->next_char == '=');
		// memset(arg, 0, sizeof(arg));

		// if (!strncmp((const char *)(cmd_params->cmd), "ETH", 4))
		// {
		// }

		return EVENT_CONTINUE;
	}

#ifndef ETH_TX_BUFFER_SIZE
#define ETH_TX_BUFFER_SIZE 64
#endif
	DECL_BUFFER(uint8_t, eth_rx, RX_BUFFER_SIZE);
	DECL_BUFFER(uint8_t, eth_tx, ETH_TX_BUFFER_SIZE);

	void eth_flush(void)
	{
		if (eth_clientok())
		{
			while (!BUFFER_EMPTY(eth_tx))
			{
				uint8_t tmp[ETH_TX_BUFFER_SIZE + 1];
				memset(tmp, 0, sizeof(tmp));
				uint8_t r;

				BUFFER_READ(eth_tx, tmp, ETH_TX_BUFFER_SIZE, r);
				eth_telnet_client.write(tmp, r);
			}
		}
		else
		{
			// no client (discard)
			BUFFER_CLEAR(eth_tx);
		}
	}

	uint8_t eth_getc(void)
	{
		uint8_t c = 0;
		BUFFER_DEQUEUE(eth_rx, &c);
		return c;
	}

	uint8_t eth_available(void)
	{
		return BUFFER_READ_AVAILABLE(eth_rx);
	}

	void eth_clear(void)
	{
		BUFFER_CLEAR(eth_rx);
	}

	void eth_putc(uint8_t c)
	{
		while (BUFFER_FULL(eth_tx))
		{
			eth_flush();
		}
		BUFFER_ENQUEUE(eth_tx, &c);
	}

	bool eth_loop(void *arg)
	{
		// eth_telnet_server.statusreport();
		if (eth_clientok())
		{
			while (eth_telnet_client.available() > 0)
			{
				uint8_t c = eth_telnet_client.read();
				if (mcu_com_rx_cb(c))
				{
					if (BUFFER_FULL(eth_rx))
					{
						c = OVF;
					}

					BUFFER_ENQUEUE(eth_rx, &c);
				}
			}
		}

		return EVENT_CONTINUE;
	}
	CREATE_EVENT_LISTENER(cnc_alarm, eth_loop);
	CREATE_EVENT_LISTENER(cnc_dotasks, eth_loop);

	DECL_MODULE(wiznet_ethernet)
	{

// 		// serial_stream_register(&web_pendant_stream);
// 		endpoint_add("/", 0, &web_pendant_request, NULL);

// ADD_EVENT_LISTENER(websocket_client_connected, web_pendant_ws_connected);
// ADD_EVENT_LISTENER(websocket_client_disconnected, web_pendant_ws_disconnected);
// ADD_EVENT_LISTENER(websocket_client_receive, web_pendant_ws_receive);

// serial_stream_register(&web_pendant_stream);
#ifdef ENABLE_MAIN_LOOP_MODULES
		ADD_EVENT_LISTENER(cnc_alarm, eth_loop);
		ADD_EVENT_LISTENER(cnc_dotasks, eth_loop);
#else
#warning "Main loop extensions are not enabled. Ethernet will not work."
#endif

// #if (defined(ENABLE_PARSER_MODULES) || defined(BOARD_HAS_CUSTOM_SYSTEM_COMMANDS))
// 		ADD_EVENT_LISTENER(grbl_cmd, eth_print_info);
// #else
// #warning "Parser extensions are not enabled. Ethernet Grbl commands will not work."
// #endif
	}

#ifdef __cplusplus
}
#endif