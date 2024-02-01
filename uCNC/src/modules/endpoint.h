/*
	Name: endpoint.h
	Description: Webserver endpoints for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 25-10-2023

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef ENDPOINT_H
#define ENDPOINT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../cnc.h"
#include <stddef.h>

	typedef void (*endpoint_delegate)(void);

	void endpoint_add(const char* uri, uint8_t method, endpoint_delegate request_handler, endpoint_delegate file_handler);
	int endpoint_request_hasargs(void);
	bool endpoint_request_arg(const char* argname, char* argvalue, size_t maxlen);
	void endpoint_send(int code, const char *content_type, const char *data);
	void endpoint_send_header(const char *name, const char *data, bool first);
	bool endpoint_send_file(const char * file_path, const char *content_type);

#ifdef __cplusplus
}
#endif

#endif