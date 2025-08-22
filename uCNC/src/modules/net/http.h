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

#include "utils/http_request.h"   // request_ctx_t + parse_request_line(...)
#include "socket.h"               // raw socket server API
#include "../file_system.h"          // fs_open/fs_read/fs_close (for http_send_file)

/* Upload status (keep provided names; add synonyms to avoid typos breaking code) */
#define HHTP_UPLOAD_START 0
#define HHTP_UPLOAD_PART  1
#define HHTP_UPLOAD_END   2
#define HHTP_UPLOAD_ABORT 3
/* Optional aliases */
#define HTTP_UPLOAD_START HHTP_UPLOAD_START
#define HTTP_UPLOAD_PART  HHTP_UPLOAD_PART
#define HTTP_UPLOAD_END   HHTP_UPLOAD_END
#define HTTP_UPLOAD_ABORT HHTP_UPLOAD_ABORT

#ifndef FS_PATH_NAME_MAX_LEN
#define FS_PATH_NAME_MAX_LEN 256
#endif

typedef struct http_upload_ {
  uint8_t  status;                         /* HHTP_UPLOAD_* */
  char     filename[FS_PATH_NAME_MAX_LEN]; /* sanitized base name */
  uint8_t *data;                           /* pointer to current chunk buffer */
  size_t   datalen;                        /* current chunk size */
} http_upload_t;

typedef void (*http_delegate)(void);

/* Module entry */
DECL_MODULE(http_server);

/* Main run loop (non-blocking), mirrors telnet_server_run */
void http_server_run(void);

/* Routing */
void http_add(const char *uri, uint8_t method, http_delegate request_handler, http_delegate file_handler);

/* Request accessors (current client context) */
int  http_request_hasargs(void);
void http_request_uri(char *uri, size_t maxlen);
bool http_request_arg(const char *argname, char *argvalue, size_t maxlen);
uint8_t http_request_method(void);

/* Response helpers (current client context) */
void http_send(int code, const char *content_type, const uint8_t *data, size_t data_len);
static inline void http_send_str(int code, const char *content_type, const char *data) {
  http_send(code, content_type, (const uint8_t*)data, data ? strlen(data) : 0);
}
void http_send_header(const char *name, const char *data, bool first);
bool http_send_file(const char *file_path, const char *content_type);

/* Upload helpers (polled by file_handler) */
http_upload_t http_file_upload_status(void);
void          http_file_upload_name(char *filename, size_t maxlen);
char         *http_file_upload_buffer(size_t *len);

#endif /* HTTP_SERVER_H */

