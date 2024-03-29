/*
	Name: web_pendant.c
	Description: Implements a Web Pendant for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 25/10/2023

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"
#include "../../modules/endpoint.h"
#include "../../modules/websocket.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#if defined(MCU_HAS_WIFI) && defined(MCU_HAS_ENDPOINTS) && defined(MCU_HAS_WEBSOCKETS)

#ifndef WEB_PENDANT_REFRESH_MS
#define WEB_PENDANT_REFRESH_MS 200
#endif

#if (UCNC_MODULE_VERSION < 10890 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

DECL_BUFFER(uint8_t, web_pendant_rx, 128);
DECL_BUFFER(uint8_t, web_pendant_tx, 128);
// DECL_BUFFER(uint8_t, web_pendant_tx, 128);
static websocket_client_t ws_web_pendant_client;

void web_pendant_request(void)
{
	// if does not have args return the page
	if (!endpoint_request_hasargs())
	{
		if (!endpoint_send_file("/index.html.gz", "text/html"))
		{
			protocol_send_string(__romstr__("Server error. File not found"));
			endpoint_send_str(404, "text/plain", "FileNotFound");
		}
	}
}

bool web_pendant_ws_connected(void *args)
{
	websocket_event_t *e = args;
	ws_web_pendant_client.id = e->id;
	ws_web_pendant_client.ip = e->ip;

	return EVENT_CONTINUE;
}
CREATE_EVENT_LISTENER(websocket_client_connected, web_pendant_ws_connected);

bool web_pendant_ws_disconnected(void *args)
{
	websocket_event_t *e = args;
	if (ws_web_pendant_client.id == e->id)
	{
		ws_web_pendant_client.ip = 0;
		ws_web_pendant_client.id = 0;
	}

	return EVENT_CONTINUE;
}
CREATE_EVENT_LISTENER(websocket_client_disconnected, web_pendant_ws_disconnected);

bool web_pendant_ws_receive(void *args)
{
	websocket_event_t *e = args;
	if (ws_web_pendant_client.ip)
	{
		if (e->event == WS_EVENT_TEXT)
		{
			for (size_t i = 0; i < e->length; i++)
			{
				uint8_t c = e->data[i];
				if (mcu_com_rx_cb(c))
				{
					if (BUFFER_FULL(web_pendant_rx))
					{
						c = OVF;
					}

					*(BUFFER_NEXT_FREE(web_pendant_rx)) = c;
					BUFFER_STORE(web_pendant_rx);
				}
			}
		}
	}
	return EVENT_CONTINUE;
}
CREATE_EVENT_LISTENER(websocket_client_receive, web_pendant_ws_receive);

/**
 * Creates the serial stream handler functions
 * **/
uint8_t web_pendant_getc(void)
{
	uint8_t c = 0;
	BUFFER_DEQUEUE(web_pendant_rx, &c);
	return c;
}

uint8_t web_pendant_available(void)
{
	return BUFFER_READ_AVAILABLE(web_pendant_rx);
}

void web_pendant_clear(void)
{
	BUFFER_CLEAR(web_pendant_rx);
}

void web_pendant_flush(void)
{
	static uint32_t flush_timeout = 0;

	if (!BUFFER_FULL(web_pendant_tx) && flush_timeout > mcu_millis())
	{
		return;
	}

	while (!BUFFER_EMPTY(web_pendant_tx))
	{
		uint8_t tmp[128 + 1];
		memset(tmp, 0, sizeof(tmp));
		uint8_t r = 0;

		BUFFER_READ(web_pendant_tx, tmp, 128, r);
		websocket_send(ws_web_pendant_client.id, (uint8_t *)tmp, r, WS_SEND_TXT);
		flush_timeout = mcu_millis() + WEB_PENDANT_REFRESH_MS;
	}
}

void web_pendant_putc(uint8_t c)
{
	while (BUFFER_FULL(web_pendant_tx))
	{
		web_pendant_flush();
	}
	BUFFER_ENQUEUE(web_pendant_tx, &c);
}

DECL_SERIAL_STREAM(web_pendant_stream, web_pendant_getc, web_pendant_available, web_pendant_clear, web_pendant_putc, web_pendant_flush);

DECL_MODULE(web_pendant)
{
	// serial_stream_register(&web_pendant_stream);
	endpoint_add("/", 0, &web_pendant_request, NULL);

	ADD_EVENT_LISTENER(websocket_client_connected, web_pendant_ws_connected);
	ADD_EVENT_LISTENER(websocket_client_disconnected, web_pendant_ws_disconnected);
	ADD_EVENT_LISTENER(websocket_client_receive, web_pendant_ws_receive);

	serial_stream_register(&web_pendant_stream);
}

#endif
