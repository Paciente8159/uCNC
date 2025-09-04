/*
	Name: http.h
	Description: Implements a simple HTTP Server based on BSD/POSIX Sockets for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 20-08-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "utils/http_request.h" // request_ctx_t + parse_request_line(...)
#include "socket.h"							// raw socket server API
#include "../file_system.h"			// fs_open/fs_read/fs_close (for http_send_file)

#define HTTP_UPLOAD_START 0
#define HTTP_UPLOAD_PART 1
#define HTTP_UPLOAD_END 2
#define HTTP_UPLOAD_ABORT 3

#ifndef FS_PATH_NAME_MAX_LEN
#define FS_PATH_NAME_MAX_LEN 256
#endif

typedef struct http_upload_
{
	uint8_t status; /* HTTP_UPLOAD_* */
	char *filename; /* sanitized base name */
	uint8_t *data;	/* pointer to current chunk buffer */
	size_t datalen; /* current chunk size */
} http_upload_t;

typedef void (*http_delegate)(int client_idx);

/* Module entry */
DECL_MODULE(http_server);

/* Routing */
void http_add(const char *uri, uint8_t method, http_delegate request_handler, http_delegate file_handler);

/* Request accessors (current client context) */
int http_request_hasargs(int client_idx);
void http_request_uri(int client_idx, char *uri, size_t maxlen);
bool http_request_arg(int client_idx, const char *argname, char *argvalue, size_t maxlen);
uint8_t http_request_method(int client_idx);

/* Response helpers (current client context) */
void http_send(int client_idx, int code, const char *content_type, const uint8_t *data, size_t data_len);
static inline void http_send_str(int client_idx, int code, const char *content_type, const char *data)
{
	http_send(client_idx, code, content_type, (const uint8_t *)data, data ? strlen(data) : 0);
}
void http_send_header(int client_idx, const char *name, const char *data, bool first);
bool http_send_file(int client_idx, const char *file_path, const char *content_type);

/* Upload helpers (polled by file_handler) */
http_upload_t http_file_upload_status(int client_idx);
void http_file_upload_name(int client_idx, char *filename, size_t maxlen);
char *http_file_upload_buffer(int client_idx, size_t *len);

#endif /* HTTP_SERVER_H */
