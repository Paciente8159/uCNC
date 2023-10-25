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
#include "../../modules/flash_fs.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#if defined(MCU_HAS_WIFI) && defined(MCU_HAS_ENDPOINTS)

#if (UCNC_MODULE_VERSION < 10801 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

void web_pendant_request(void)
{
	if (!endpoint_send_file("/index.html.gz", "text/html"))
	{
		endpoint_send(404, "text/plain", "FileNotFound");
	}
}

void web_pendant_status_request(void)
{
	uint8_t state = 0;
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

	sprintf(response, '{"ax":"%d\0"', AXIS_COUNT);

	// CHECK STATE
	uint8_t state = 0;
	if (cnc_has_alarm())
	{
		sprintf(part, ',"st":"Alarm\0"');
	}
	else if (mc_get_checkmode())
	{
		sprintf(part, ',"st":"Check\0"');
	}
	else
	{
		switch (state)
		{
#if ASSERT_PIN(SAFETY_DOOR)
		case EXEC_DOOR:
			sprintf(part, ',"st":"Door\0"');
			break;
#endif
		case EXEC_UNHOMED:
		case EXEC_LIMITS:
			if (!cnc_get_exec_state(EXEC_HOMING))
			{
				sprintf(part, ',"st":"Alarm\0"');
			}
			else
			{
				sprintf(part, ',"st":"Home\0"');
			}
			break;
		case EXEC_HOLD:
			sprintf(part, ',"st":"Hold\0"');
			break;
		case EXEC_HOMING:
			sprintf(part, ',"st":"Home\0"');
			break;
		case EXEC_JOG:
			sprintf(part, ',"st":"Jog\0"');
			break;
		case EXEC_RUN:
			sprintf(part, ',"st":"Run\0"');
			break;
		default:
			sprintf(part, ',"st":"Idle\0"');
			break;
		}
	}

	strcat(response, part);

	// GET POS

#if (AXIS_COUNT >= 1)
	sprintf(part, ',"x":"%0.3f\0"', axis[0]);
	strcat(response, part);
#endif
#if (AXIS_COUNT == 2)
#if defined(USE_Y_AS_Z_ALIAS))
	sprintf(part, ',"z":"%0.3f\0"', axis[1]);
#else
	sprintf(part, ',"y":"%0.3f\0"', axis[1]);
#endif
	strcat(response, part);
#endif
#if (AXIS_COUNT >= 2)
	sprintf(part, ',"y":"%0.3f\0"', axis[1]);
	strcat(response, part);
#endif
#if (AXIS_COUNT >= 3)
	sprintf(part, ',"z":"%0.3f\0"', axis[2]);
	strcat(response, part);
#endif
#if (AXIS_COUNT >= 4)
	sprintf(part, ',"a":"%0.3f\0"', axis[3]);
	strcat(response, part);
#endif
#if (AXIS_COUNT >= 5)
	sprintf(part, ',"b":"%0.3f\0"', axis[4]);
	strcat(response, part);
#endif
#if (AXIS_COUNT == 6)
	sprintf(part, ',"c":"%0.3f\0"', axis[5]);
	strcat(response, part);
#endif

	sprintf(part, '",f":"%0.0f,s":"%d\0"', feed, spindle);
	strcat(response, part);

	endpoint_send(200, "application/json", response);
}

DECL_MODULE(web_pendant)
{
	endpoint_add("/", 255, &web_pendant_request, NULL);
	endpoint_add("/status", 255, &web_pendant_status_request, NULL);
}

#endif
