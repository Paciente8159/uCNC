/*
	Name: http.h
	Description: Small, allocation-free HTTP/1.1 server for uCNC.

	Copyright: Copyright (c) Joao Martins
	Author: Joao Martins

	uCNC is free software: you can redistribute it and/or modify it under the
	terms of the GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later
	version.
*/
#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "../file_system.h"
#include "socket.h"
#include "utils/http_request.h"

#define HTTP_UPLOAD_START 0U
#define HTTP_UPLOAD_PART 1U
#define HTTP_UPLOAD_END 2U
#define HTTP_UPLOAD_ABORT 3U

#ifndef FS_PATH_NAME_MAX_LEN
#define FS_PATH_NAME_MAX_LEN 256U
#endif

/* Largest payload accepted by one http_send() call. */
#ifndef HTTP_MAX_CHUNCK_LEN
#define HTTP_MAX_CHUNCK_LEN SOCKET_MAX_DATA_SIZE
#endif


/* Total compact storage for staged custom response headers per client. */
#ifndef HTTP_CUSTOM_HEADERS_SIZE
#define HTTP_CUSTOM_HEADERS_SIZE 256U
#endif

#if HTTP_CUSTOM_HEADERS_SIZE == 0U || HTTP_CUSTOM_HEADERS_SIZE > UINT16_MAX
#error "HTTP_CUSTOM_HEADERS_SIZE must be in [1, 65535]"
#endif

/* Shared file-read chunk size used by cooperative file responses. */
#ifndef HTTP_FILE_CHUNK_SIZE
#define HTTP_FILE_CHUNK_SIZE 256U
#endif


#if HTTP_FILE_CHUNK_SIZE == 0U || HTTP_FILE_CHUNK_SIZE > HTTP_MAX_CHUNCK_LEN
#error "HTTP_FILE_CHUNK_SIZE must be in [1, HTTP_MAX_CHUNCK_LEN]"
#endif

typedef struct http_upload_
{
	uint8_t status;       /* One HTTP_UPLOAD_* value. */
	char *filename;       /* Borrowed sanitized base name; callback-lifetime only. */
	uint8_t *data;        /* Borrowed current upload bytes; callback-lifetime only. */
	size_t datalen;       /* Bytes in data for HTTP_UPLOAD_PART. */
	size_t filelen;       /* Total upload size reported by the request parser. */
} http_upload_t;

/*
 * Called synchronously when a matched request or upload transition is ready.
 * client_idx is valid only during the connection. A request handler may call
 * response helpers. A file handler must inspect http_file_upload_status() and
 * copy/consume PART bytes before returning; those bytes are borrowed RX data.
 * Delegates execute from socket_server_dotasks(), never from an ISR/backend task.
 */
typedef void (*http_delegate)(int client_idx);

/* Module entry. Initialization uses HTTP_PORT and the static socket core. */
DECL_MODULE(http_server);

/*
 * Adds one route to the fixed route table.
 *
 * uri is borrowed permanently and must remain valid. '*' matches any sequence.
 * method is HTTP_REQ_* or HTTP_REQ_ANY. Either handler may be NULL. Duplicate
 * routes and additions after the fixed table is full are ignored.
 */
void http_add(const char *uri,
			  uint8_t method,
			  http_delegate request_handler,
			  http_delegate file_handler);

/* Returns the parsed argument count, or 0 for an invalid client. */
int http_request_hasargs(int client_idx);

/*
 * Copies the current request URI into uri and always NUL terminates when maxlen
 * is nonzero. Invalid clients produce an empty string. uri may not be NULL.
 */
void http_request_uri(int client_idx, char *uri, size_t maxlen);

/*
 * Copies a named request argument into argvalue.
 * Returns true when found. Returns false for invalid arguments/clients or when
 * absent; argvalue is set to an empty string when writable.
 */
bool http_request_arg(int client_idx,
					  const char *argname,
					  char *argvalue,
					  size_t maxlen);

/* Returns the parsed HTTP_REQ_* method, or HTTP_REQ_OTHER if client is invalid. */
uint8_t http_request_method(int client_idx);

/*
 * Adds/replaces response-header text before the response starts.
 *
 * first clears previously staged custom headers before adding this one. name
 * and data are copied into bounded static client state. Calls after headers have
 * started, invalid input, overflow, or a full header table return false.
 */
bool http_send_header(int client_idx,
					  const char *name,
					  const char *data,
					  bool first);

/*
 * Sends one response operation synchronously without retaining a TX copy.
 *
 * Usage is unchanged: a fixed body is sent with one data call followed by an
 * empty finishing call; chunked mode is selected by an initial NULL/empty call,
 * followed by data chunks and an empty finishing call. Header lines, chunk
 * framing and payload bytes are sent with blocking socket_send().
 *
 * Returns data_len for a completed data operation, 0 for prepare/finish, or a
 * negative socket_device_result_t on invalid state, timeout, disconnect, or
 * transport failure. A failed partial HTTP response closes that connection.
 */
int http_send(int client_idx,
			  int code,
			  const char *content_type,
			  const void *data,
			  size_t data_len);

/* Same contract/result as http_send(), using strlen(data) for non-NULL text. */
static inline int http_send_str(int client_idx,
							int code,
							const char *content_type,
							const char *data)
{
	return http_send(client_idx, code, content_type,
					 data, data ? strlen(data) : 0U);
}

/*
 * Opens a file and schedules cooperative chunked transfer. Response headers are
 * sent synchronously, then at most one HTTP_FILE_CHUNK_SIZE block is read and
 * sent per idle callback. All clients share one file-read buffer; no per-client
 * file payload buffer or content-type copy is retained.
 */
bool http_send_file(int client_idx,
					const char *file_path,
					const char *content_type);

/* Returns a snapshot of upload state; invalid clients return a zeroed value. */
http_upload_t http_file_upload_status(int client_idx);

/*
 * Copies the current upload base name and NUL terminates when maxlen is nonzero.
 * Invalid clients produce an empty string. filename may not be NULL.
 */
void http_file_upload_name(int client_idx, char *filename, size_t maxlen);

#ifdef __cplusplus
}
#endif

#endif /* HTTP_SERVER_H */
