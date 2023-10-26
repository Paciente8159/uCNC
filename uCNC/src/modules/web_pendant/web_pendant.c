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
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#if defined(MCU_HAS_WIFI) && defined(MCU_HAS_ENDPOINTS)

#if (UCNC_MODULE_VERSION < 10801 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

#ifdef DECL_SERIAL_STREAM
DECL_BUFFER(uint8_t, web_pendant_stream_buffer, RX_BUFFER_SIZE);
static uint8_t web_pendant_getc(void)
{
	uint8_t c = 0;
	BUFFER_DEQUEUE(web_pendant_stream_buffer, &c);
	return c;
}

uint8_t web_pendant_available(void)
{
	return BUFFER_READ_AVAILABLE(web_pendant_stream_buffer);
}

void web_pendant_clear(void)
{
	BUFFER_CLEAR(web_pendant_stream_buffer);
}

DECL_SERIAL_STREAM(web_pendant_stream, web_pendant_getc, web_pendant_available, web_pendant_clear, NULL, NULL);

uint8_t web_pendant_send_cmd(const char *__s)
{
	// if machine is running (not jogging) rejects the command
	if (cnc_get_exec_state(EXEC_RUN | EXEC_JOG) == EXEC_RUN)
	{
		return STATUS_SYSTEM_GC_LOCK;
	}

	uint8_t len = strlen(__s);
	uint8_t w;

	if (BUFFER_WRITE_AVAILABLE(web_pendant_stream_buffer) < len)
	{
		return STATUS_STREAM_FAILED;
	}

	BUFFER_WRITE(web_pendant_stream_buffer, __s, len, w);

	return STATUS_OK;
}

#endif

void web_pendant_request(void)
{
	// if does not have args return the page
	if (!endpoint_request_hasarg(NULL))
	{
		if (!endpoint_send_file("/index.html.gz", "text/html"))
		{
			protocol_send_string(__romstr__("Server error. File not found"));
			endpoint_send(404, "text/plain", "FileNotFound");
		}
	}
	else{
		// if(endpoint_request_hasarg("rtcmd")){
		// 	const char* arg = endpoint_request_arg("rtcmd");
		// 	// run the rtcmd char directly through the rx callback
		// 	//mcu_com_rx_cb(arg[0]);
		// 	// serial_print_str(arg);
		// }

		// if(endpoint_request_hasarg("cmd")){
		// 	const char* arg = endpoint_request_arg("cmd");
		// 	//web_pendant_send_cmd(arg);
		// 	// serial_print_str(arg);
		// }
	}
}

void web_pendant_status_request(void)
{
	float axis[AXIS_COUNT];
	int32_t steppos[AXIS_TO_STEPPERS];
	itp_get_rt_position(steppos);
	kinematics_steps_to_coordinates(steppos, axis);
	float feed = itp_get_rt_feed(); // convert from mm/s to mm/m
#if TOOL_COUNT > 0
	uint16_t spindle = tool_get_speed();
#else
	uint16_t spindle = 0;
#endif

	char response[256];
	char part[32];

	sprintf(response, "{\"ax\":\"%d\"", AXIS_COUNT);

	// CHECK STATE
	if (cnc_has_alarm())
	{
		sprintf(part, ",\"st\":\"Alarm\"");
	}
	else if (mc_get_checkmode())
	{
		sprintf(part, ",\"st\":\"Check\"");
	}
	else
	{
		uint8_t state = cnc_get_exec_state(0xFF);
		uint8_t filter = 0x80;
		while (!(state & filter) && filter)
		{
			filter >>= 1;
		}

		state &= filter;

		switch (state)
		{
#if ASSERT_PIN(SAFETY_DOOR)
		case EXEC_DOOR:
			sprintf(part, ",\"st\":\"Door\"");
			break;
#endif
		case EXEC_UNHOMED:
		case EXEC_LIMITS:
			if (!cnc_get_exec_state(EXEC_HOMING))
			{
				sprintf(part, ",\"st\":\"Alarm\"");
			}
			else
			{
				sprintf(part, ",\"st\":\"Home\"");
			}
			break;
		case EXEC_HOLD:
			sprintf(part, ",\"st\":\"Hold\"");
			break;
		case EXEC_HOMING:
			sprintf(part, ",\"st\":\"Home\"");
			break;
		case EXEC_JOG:
			sprintf(part, ",\"st\":\"Jog\"");
			break;
		case EXEC_RUN:
			sprintf(part, ",\"st\":\"Run\"");
			break;
		default:
			sprintf(part, ",\"st\":\"Idle\"");
			break;
		}
	}

	strcat(response, part);

	// GET POS

#if (AXIS_COUNT >= 1)
	sprintf(part, ",\"x\":\"%0.3f\"", axis[0]);
	strcat(response, part);
#endif
#if (AXIS_COUNT == 2)
#if defined(USE_Y_AS_Z_ALIAS))
	sprintf(part, ",\"z\":\"%0.3f\"", axis[1]);
#else
	sprintf(part, ",\"y\":\"%0.3f\"", axis[1]);
#endif
	strcat(response, part);
#endif
#if (AXIS_COUNT >= 2)
	sprintf(part, ",\"y\":\"%0.3f\"", axis[1]);
	strcat(response, part);
#endif
#if (AXIS_COUNT >= 3)
	sprintf(part, ",\"z\":\"%0.3f\"", axis[2]);
	strcat(response, part);
#endif
#if (AXIS_COUNT >= 4)
	sprintf(part, ",\"a\":\"%0.3f\"", axis[3]);
	strcat(response, part);
#endif
#if (AXIS_COUNT >= 5)
	sprintf(part, ",\"b\":\"%0.3f\"", axis[4]);
	strcat(response, part);
#endif
#if (AXIS_COUNT == 6)
	sprintf(part, ",\"c\":\"%0.3f\"", axis[5]);
	strcat(response, part);
#endif

	sprintf(part, ",\"f\":\"%0.0f\",\"s\":\"%d\"}", feed, spindle);
	strcat(response, part);

	endpoint_send(200, "application/json", response);
}

DECL_MODULE(web_pendant)
{
	endpoint_add("/", 255, &web_pendant_request, NULL);
	endpoint_add("/status", 255, &web_pendant_status_request, NULL);
}

#endif
