/*
	Name: http_request.h
	Description: HTTP request helper functions for  µCNC.

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
#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

// Request method definitions
#define HTTP_REQ_ANY 255
#define HTTP_REQ_NONE 0
#define HTTP_REQ_GET 1
#define HTTP_REQ_POST 2
#define HTTP_REQ_PUT 3
#define HTTP_REQ_DELETE 4
#define HTTP_REQ_OTHER 127

// URL and arguments limits
#define MAX_URL_LEN 128
#define MAX_URL_ARGS 8
#define MAX_URL_ARG_LEN 64

#define MAX_HEADER_LEN 128

// uploads
#define MAX_UPLOAD_BOUNDARY_LEN MAX_HEADER_LEN
#ifndef FS_PATH_NAME_MAX_LEN
#define FS_PATH_NAME_MAX_LEN 256
#endif

#define REQ_START_INIT 0
#define REQ_START_METHOD_PARSED 1
#define REQ_START_URI_PARSED 2
// #define REQ_START_ANCHOR_FOUND 3
// #define REQ_START_ANCHOR_PARSED 4
#define REQ_START_QUERY_FOUND 5
#define REQ_START_QUERYVAR_FOUND 6
#define REQ_START_QUERYARG_FOUND 7
#define REQ_START_QUERY_PARSED 8
#define REQ_START_EOL_FOUND 9
#define REQ_START_FINISHED 10

#define REQ_HEAD_INIT 0
#define REQ_HEAD_NAME_PARSED 1
#define REQ_HEAD_VALUE_PARSED 2
#define REQ_HEAD_EOL_FOUND 3
#define REQ_HEAD_FINISHED 4

#define REQ_UPLOAD_NONE 0
#define REQ_UPLOAD_INIT 1
#define REQ_UPLOAD_BOUNDARY_FOUND 2
#define REQ_UPLOAD_FILENAME_FOUND 3
#define REQ_UPLOAD_INIT_FINISHED 4
#define REQ_UPLOAD_START 5
#define REQ_UPLOAD_FINISH 6
#define REQ_UPLOAD_CLOSE 7

typedef struct
{
	uint8_t status;
	int8_t method;
	char uri[MAX_URL_LEN];
	size_t arg_count;
	char arg_name[MAX_URL_ARGS][MAX_URL_ARG_LEN];
	char *arg_val[MAX_URL_ARGS];
	char last_char;
} request_ctx_t;

typedef struct
{
	uint8_t status;
	char name[MAX_HEADER_LEN];
	char *value;
} request_header_t;

typedef struct
{
	uint8_t status;
	long int upload_len;
	char boundary[MAX_UPLOAD_BOUNDARY_LEN];
	size_t boundary_len;
	size_t partial_len;
	char upload_name[FS_PATH_NAME_MAX_LEN];
} request_upload_t;

typedef struct
{
	// Handshake accumulation
	bool hs_got_upgrade;
	bool hs_got_connection;
	bool hs_got_key;
	bool hs_got_version;
	char hs_key[64];
	bool req_complete;
} ws_handshake_t;

int strncasecmp_local(char *s1, char *s2, size_t len);
char *strcasestr_local(char *haystack, char *needle);
char *strntrim_local(char *s);
char *find_char(char *buf, size_t len, char c);
char *find_closest(char *buffer, size_t len, char c1, char c2);
int hex_val(int c);
void url_decode(char *src, char *dst, size_t dst_size, int plus_to_space);
void strncat_local(char *dst, size_t maxlen, char *src, size_t ncount);
#define append_str(dst, src) strncat_local(dst, sizeof(dst), src, strlen(src))
uint8_t http_discard_line(uint8_t status, uint8_t initial_condition, char **buf, size_t *len);
void http_request_parse_start(request_ctx_t *ctx, char **buf, size_t *len);
void http_request_parse_header(request_header_t *header, char **buf, size_t *len);
void http_request_ws_handshake(ws_handshake_t *wsh, request_header_t *header);
void http_request_file_upload(request_upload_t *upload, request_header_t *header);
void http_request_multipart_chunk(char **buf, size_t *len, request_upload_t *upload, request_header_t *header);
#ifdef __cplusplus
}
#endif

#endif
