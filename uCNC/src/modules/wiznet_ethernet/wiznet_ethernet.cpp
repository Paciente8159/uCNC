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
#include "../../cnc_hal_config_helper.h"

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
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress eth_ip(192, 168, 0, 200);

// Enter the IP address of the server you're connecting to:
IPAddress eth_server(1, 1, 1, 1);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 23 is default for telnet;
// if you're using Processing's ChatServer, use port 10002):
EthernetClient client;

extern "C"
{
#include "../../cnc.h"
#include "../softspi.h"
#include "../../modules/endpoint.h"
#include "../../modules/websocket.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifndef WIZNET_DRIVER
#define WIZNET_DRIVER W5500
#define _WIZCHIP_ WIZNET_DRIVER
#endif

#define WIZNET_HW_SPI 1
#define WIZNET_SW_SPI 2

#ifndef WIZNET_BUS
#define WIZNET_BUS WIZNET_HW_SPI
#endif

#ifndef WIZNET_CS
#define WIZNET_CS DOUT28
#endif

#if (WIZNET_BUS == WIZNET_HW_SPI)
#define WIZNET_SPI MCU_SPI
#endif

#if (UCNC_MODULE_VERSION < 10903 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

	void wiznet_spi_config(void)
	{
		softspi_config(WIZNET_SPI, 0, 14000000UL);
	}

	void wiznet_cs_select(void)
	{
		io_clear_output(WIZNET_CS);
	}

	void wiznet_cs_deselect(void)
	{
		io_set_output(WIZNET_CS);
	}

	uint8_t wiznet_spi_transmit(uint8_t c)
	{
		return softspi_xmit(WIZNET_SPI, c);
	}

	void wiznet_spi_start(void)
	{
		softspi_start(WIZNET_SPI);
	}

	void wiznet_spi_stop(void)
	{
		softspi_stop(WIZNET_SPI);
	}

	DECL_MODULE(wiznet_ethernet)
	{
		io_config_output(WIZNET_CS);
		Ethernet

		// 		// serial_stream_register(&web_pendant_stream);
		// 		endpoint_add("/", 0, &web_pendant_request, NULL);

		// ADD_EVENT_LISTENER(websocket_client_connected, web_pendant_ws_connected);
		// ADD_EVENT_LISTENER(websocket_client_disconnected, web_pendant_ws_disconnected);
		// ADD_EVENT_LISTENER(websocket_client_receive, web_pendant_ws_receive);

		// serial_stream_register(&web_pendant_stream);

		// ADD_EVENT_LISTENER(cnc_dotasks, web_pendant_status_update);
	}
}