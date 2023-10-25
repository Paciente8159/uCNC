/*
	Name: endpoint.h
	Description: Webserver endpoints for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 25-10-0223

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

#include "../module.h"

	typedef void (*endpoint_delegate)(void);
	typedef struct endpoint_
	{
		const char *uri;
		uint8_t method;
		endpoint_delegate request_handler;
		endpoint_delegate file_handler;
	} endpoint_t;

	DECL_MODULE(endpoint);
	void endpoint_add(const char* uri, uint8_t method, endpoint_delegate request_handler, endpoint_delegate file_handler);
	int endpoint_request_has_args(void);
	const char * endpoint_request_arg(const char* name);
	void endpoint_send(int code, const char *content_type, const char *data, uint8_t len);
	void endpoint_send_header(const char *name, const char *data, bool first);

#ifdef __cplusplus
}
#endif

#endif