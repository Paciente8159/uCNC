/*
	Name: http.c
	Description: Implements a simple Telnet Server based on BSD/POSIX Sockets for µCNC.

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

#include "../../cnc.h"
#ifdef ENABLE_SOCKETS

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "http.h"

#ifndef HTTP_PORT
#define HTTP_PORT 80
#endif

#define HTTP_MAX_HANDLERS 8
#define HTTP_MAX_HEADERS 8
#define HTTP_MAX_HEADER_LEN 128
#define HTTP_CONTENT_TYPE_MAX 64

typedef struct
{
	char line[HTTP_MAX_HEADER_LEN];
} http_header_kv_t;

typedef struct
{
	char *uri;
	uint8_t method;				   /* HTTP_REQ_* from http_request.h */
	http_delegate request_handler; /* called when request ready */
	http_delegate file_handler;	   /* called on upload START/PART/END/ABORT */
} http_route_t;

/* Per-client state */
typedef struct
{
	request_ctx_t req;
	request_header_t head;
	request_upload_t upl;
	http_upload_t fileupl;

	/* Request parsing */
	bool have_reqline;
	bool have_headers;
	size_t hlen;

	/* Response bookkeeping */
	bool headers_sent;
	bool chunked_mode;
	bool keep_alive;
	bool response_end_pending;
	size_t hdr_count;
	http_header_kv_t hdrs[HTTP_MAX_HEADERS];

	/* Persistent nonblocking transmit continuation state. */
	uint8_t tx_buffer[HTTP_TX_BUFFER_SIZE];
	size_t tx_offset;
	size_t tx_length;

	/* Cooperative file-response state. */
	fs_file_t *response_file;
	char response_content_type[HTTP_CONTENT_TYPE_MAX];
	uint8_t file_buffer[HTTP_FILE_CHUNK_SIZE];
	size_t file_buffer_len;

	/* Selected route for this request (if matched) */
	http_route_t *route;
} http_client_t;

/* Socket interface (single listener) */
static socket_if_t *http_srv = NULL;

/* Client slots align with SOCKET_MAX_CLIENTS from socket.h */
static http_client_t clients[SOCKET_MAX_CLIENTS];

/* Routes */
static http_route_t routes[HTTP_MAX_HANDLERS];
static size_t route_count = 0;

static void client_reset(int client_idx)
{
	if (client_idx < 0 || client_idx >= SOCKET_MAX_CLIENTS)
		return;
	if (clients[client_idx].response_file)
		fs_close(clients[client_idx].response_file);
	memset(&clients[client_idx], 0, sizeof(clients[client_idx]));
}

static void reset_request_state(int client_idx)
{
	http_client_t *c;
	if (client_idx < 0 || client_idx >= SOCKET_MAX_CLIENTS)
		return;
	c = &clients[client_idx];
	/* Reset request/response metadata only after the accepted TX queue drains. */
	memset(&clients[client_idx].req, 0, sizeof(clients[client_idx].req));
	memset(&clients[client_idx].head, 0, sizeof(clients[client_idx].head));
	memset(&clients[client_idx].upl, 0, sizeof(clients[client_idx].upl));
	memset(&clients[client_idx].fileupl, 0, sizeof(clients[client_idx].fileupl));
	clients[client_idx].have_reqline = false;
	clients[client_idx].have_headers = false;
	clients[client_idx].headers_sent = false;
	clients[client_idx].chunked_mode = false;
	clients[client_idx].response_end_pending = false;
	clients[client_idx].hdr_count = 0;
	clients[client_idx].route = NULL;
	c->file_buffer_len = 0U;
	c->response_content_type[0] = '\0';
}

static void release_client(int client_idx)
{
	if (client_idx >= 0 && client_idx < SOCKET_MAX_CLIENTS)
	{
		if (clients[client_idx].upl.status == REQ_UPLOAD_INIT_FINISHED)
		{
			/* Notify abort if we die mid-upload */
			clients[client_idx].fileupl.status = HTTP_UPLOAD_ABORT;
			if (clients[client_idx].route && clients[client_idx].route->file_handler)
			{
				clients[client_idx].route->file_handler(client_idx);
			}
		}
		client_reset(client_idx);
		return;
	}
}

static bool uri_matches(const char *pattern, const char *uri)
{
	// Simple wildcard match: '*' matches any sequence
	while (*pattern && *uri)
	{
		if (*pattern == '*')
		{
			// Skip consecutive '*' characters
			while (*pattern == '*')
				pattern++;
			if (!*pattern)
				return true; // Trailing '*' matches everything
			while (*uri)
			{
				if (uri_matches(pattern, uri))
					return true;
				uri++;
			}
			return false;
		}
		else if (*pattern == *uri)
		{
			pattern++;
			uri++;
		}
		else
		{
			return false;
		}
	}
	// Handle trailing '*' in pattern
	while (*pattern == '*')
		pattern++;
	return !*pattern && !*uri;
}

static http_route_t *match_route(const char *uri, uint8_t method)
{
	for (size_t i = 0; i < route_count; i++)
	{
		if ((routes[i].method == HTTP_REQ_ANY || routes[i].method == method) &&
			uri_matches(routes[i].uri, uri))
		{
			return &routes[i];
		}
	}
	return NULL;
}

void http_add(const char *uri, uint8_t method, http_delegate request_handler, http_delegate file_handler)
{
	if (!uri || uri[0] == '\0')
		return;
	for (size_t i = 0; i < route_count; i++)
	{
		// check if handler already exists
		if (!strncasecmp_local(routes[i].uri, (char *)uri, strlen(uri)) && (strlen(uri) == strlen(routes[i].uri)) && (routes[i].method == method))
		{
			return;
		}
	}

	if (route_count < HTTP_MAX_HANDLERS)
	{
		routes[route_count].uri = (char *)uri;
		routes[route_count].method = method;
		routes[route_count].request_handler = request_handler;
		routes[route_count].file_handler = file_handler;
		route_count++;
	}
}

int http_request_hasargs(int client_idx)
{
	if (client_idx < 0 || client_idx >= SOCKET_MAX_CLIENTS)
		return 0;
	return clients[client_idx].req.arg_count > 0;
}

void http_request_uri(int client_idx, char *uri, size_t maxlen)
{
	if (!uri || maxlen == 0)
		return;
	if (client_idx >= 0 && client_idx < SOCKET_MAX_CLIENTS)
	{
		strncpy(uri, clients[client_idx].req.uri, maxlen - 1);
		uri[maxlen - 1] = '\0';
	}
	else
	{
		uri[0] = '\0';
	}
}

bool http_request_arg(int client_idx, const char *argname, char *argvalue, size_t maxlen)
{
	http_client_t *c;
	if (!argvalue || maxlen == 0)
		return false;
	argvalue[0] = '\0';
	if (client_idx < 0 || client_idx >= SOCKET_MAX_CLIENTS || !argname)
		return false;
	c = &clients[client_idx];
	for (size_t i = 0; i < c->req.arg_count; i++)
	{
		if (strcmp(c->req.arg_name[i], argname) == 0)
		{
			strncpy(argvalue, c->req.arg_val[i], maxlen - 1);
			argvalue[maxlen - 1] = '\0';
			return true;
		}
	}
	return false;
}

uint8_t http_request_method(int client_idx)
{
	if (client_idx < 0 || client_idx >= SOCKET_MAX_CLIENTS)
		return HTTP_REQ_OTHER;
	return clients[client_idx].req.method;
}

/* --------------- response helpers ----------------- */

static const char *http_reason_phrase(int code)
{
	switch (code)
	{
	case 101: return "Switching Protocols";
	case 200: return "OK";
	case 201: return "Created";
	case 204: return "No Content";
	case 400: return "Bad Request";
	case 404: return "Not Found";
	case 405: return "Method Not Allowed";
	case 413: return "Payload Too Large";
	case 500: return "Internal Server Error";
	default: return "Status";
	}
}

static bool http_append_bytes(char *output,
							  size_t capacity,
							  size_t *used,
							  const char *text,
							  size_t text_len)
{
	if (!output || !used || (!text && text_len != 0U) ||
		*used > capacity || text_len > capacity - *used)
		return false;
	if (text_len != 0U)
		memcpy(&output[*used], text, text_len);
	*used += text_len;
	return true;
}

static bool http_append_string(char *output,
							   size_t capacity,
							   size_t *used,
							   const char *text)
{
	return text && http_append_bytes(output, capacity, used, text, strlen(text));
}

static int http_build_headers(const http_client_t *c,
							  int code,
							  const char *content_type,
							  size_t content_length,
							  char *output,
							  size_t capacity)
{
	char line[HTTP_MAX_HEADER_LEN];
	size_t used = 0U;
	int length;
	size_t i;

	length = str_snprintf(line, sizeof(line), "HTTP/1.1 %d %s\r\n",
						  code, http_reason_phrase(code));
	if (length <= 0 || (size_t)length >= sizeof(line) ||
		!http_append_bytes(output, capacity, &used, line, (size_t)length))
		return SOCKET_DEVICE_INVALID;
	length = str_snprintf(line, sizeof(line), "Connection: %s\r\n",
						  c->keep_alive ? "keep-alive" : "close");
	if (length <= 0 || (size_t)length >= sizeof(line) ||
		!http_append_bytes(output, capacity, &used, line, (size_t)length))
		return SOCKET_DEVICE_INVALID;

	if (c->chunked_mode)
	{
		if (!http_append_string(output, capacity, &used,
								"Transfer-Encoding: chunked\r\n"))
			return SOCKET_DEVICE_INVALID;
	}
	else
	{
		length = str_snprintf(line, sizeof(line), "Content-Length: %lu\r\n",
							  (unsigned long)content_length);
		if (length <= 0 || (size_t)length >= sizeof(line) ||
			!http_append_bytes(output, capacity, &used, line, (size_t)length))
			return SOCKET_DEVICE_INVALID;
	}
	if (content_type)
	{
		length = str_snprintf(line, sizeof(line), "Content-Type: %s\r\n", content_type);
		if (length <= 0 || (size_t)length >= sizeof(line) ||
			!http_append_bytes(output, capacity, &used, line, (size_t)length))
			return SOCKET_DEVICE_INVALID;
	}
	for (i = 0U; i < c->hdr_count; ++i)
	{
		if (!http_append_string(output, capacity, &used, c->hdrs[i].line) ||
			!http_append_string(output, capacity, &used, "\r\n"))
			return SOCKET_DEVICE_INVALID;
	}
	if (!http_append_string(output, capacity, &used, "\r\n"))
		return SOCKET_DEVICE_INVALID;
	return used > (size_t)INT_MAX ? SOCKET_DEVICE_INVALID : (int)used;
}

static void http_compact_tx(http_client_t *c)
{
	if (c && c->tx_offset != 0U)
	{
		memmove(c->tx_buffer, &c->tx_buffer[c->tx_offset], c->tx_length);
		c->tx_offset = 0U;
	}
}

static void http_complete_response(int client_idx)
{
	http_client_t *c;
	if (client_idx < 0 || client_idx >= SOCKET_MAX_CLIENTS)
		return;
	c = &clients[client_idx];
	if (c->tx_length != 0U || !c->response_end_pending)
		return;
	c->response_end_pending = false;
	if (c->keep_alive)
		reset_request_state(client_idx);
	else if (http_srv)
		(void)socket_close(http_srv, (uint8_t)client_idx);
}

/*
 * Makes exactly one nonblocking send attempt. This is the only HTTP helper
 * that calls socket_send(); every unsent suffix remains in persistent storage.
 */
static int http_flush(int client_idx)
{
	http_client_t *c;
	int sent;

	if (!http_srv || client_idx < 0 || client_idx >= SOCKET_MAX_CLIENTS)
		return SOCKET_DEVICE_INVALID;
	c = &clients[client_idx];
	if (c->tx_length == 0U)
	{
		c->tx_offset = 0U;
		http_complete_response(client_idx);
		return 0;
	}

	sent = socket_send(http_srv, (uint8_t)client_idx,
					   &c->tx_buffer[c->tx_offset], c->tx_length);
	if (sent > 0)
	{
		size_t consumed = (size_t)sent;
		if (consumed > c->tx_length)
			consumed = c->tx_length;
		c->tx_offset += consumed;
		c->tx_length -= consumed;
		if (c->tx_length == 0U)
			c->tx_offset = 0U;
		http_complete_response(client_idx);
	}
	else if (sent < 0 && sent != SOCKET_DEVICE_WOULD_BLOCK)
	{
		(void)socket_close(http_srv, (uint8_t)client_idx);
	}
	return sent;
}

bool http_send_header(int client_idx, const char *name, const char *data, bool first)
{
	http_client_t *c;
	size_t name_len;
	size_t data_len;
	size_t i;
	int length;

	if (client_idx < 0 || client_idx >= SOCKET_MAX_CLIENTS || !name || !data)
		return false;
	c = &clients[client_idx];
	if (c->headers_sent || c->response_end_pending)
		return false;
	if (first)
		c->hdr_count = 0U;
	name_len = strlen(name);
	data_len = strlen(data);
	if (name_len == 0U || name_len >= HTTP_MAX_HEADER_LEN ||
		strchr(name, '\r') || strchr(name, '\n') || strchr(name, ':') ||
		strchr(data, '\r') || strchr(data, '\n'))
		return false;

	for (i = 0U; i < c->hdr_count; ++i)
	{
		char *line = c->hdrs[i].line;
		size_t line_len;
		if (strncasecmp_local(line, (char *)name, name_len) != 0 || line[name_len] != ':')
			continue;
		line_len = strlen(line);
		if (data_len + 2U >= sizeof(c->hdrs[i].line) - line_len)
			return false;
		memcpy(&line[line_len], ", ", 2U);
		memcpy(&line[line_len + 2U], data, data_len + 1U);
		return true;
	}
	if (c->hdr_count >= HTTP_MAX_HEADERS)
		return false;
	length = str_snprintf(c->hdrs[c->hdr_count].line,
						  sizeof(c->hdrs[c->hdr_count].line), "%s: %s", name, data);
	if (length <= 0 || (size_t)length >= sizeof(c->hdrs[c->hdr_count].line))
		return false;
	c->hdr_count++;
	return true;
}

int http_send(int client_idx,
			  int code,
			  const char *content_type,
			  const void *data,
			  size_t data_len)
{
	http_client_t *c;
	char chunk_prefix[24];
	int header_len = 0;
	int prefix_len = 0;
	size_t required;
	size_t tail;
	bool finishing = data_len == 0U;
	bool was_pending;
	size_t base;

	if (!http_srv || client_idx < 0 || client_idx >= SOCKET_MAX_CLIENTS ||
		(!data && data_len != 0U) || data_len > HTTP_MAX_CHUNCK_LEN ||
		data_len > (size_t)INT_MAX)
		return SOCKET_DEVICE_INVALID;
	if (!socket_client_is_connected(http_srv, (uint8_t)client_idx))
		return SOCKET_DEVICE_CLOSED;
	c = &clients[client_idx];
	if (c->response_end_pending)
		return SOCKET_DEVICE_WOULD_BLOCK;

	/* The first NULL/empty call selects chunked mode without emitting bytes. */
	if (!content_type && finishing && !c->headers_sent && !c->chunked_mode)
	{
		c->chunked_mode = true;
		return 0;
	}
	if (!content_type)
		c->chunked_mode = true;

	was_pending = c->tx_length != 0U;
	http_compact_tx(c);
	base = c->tx_length;
	if (!c->headers_sent)
	{
		size_t header_capacity = sizeof(c->tx_buffer) - base;
		if (header_capacity > HTTP_RESPONSE_HEADER_SIZE)
			header_capacity = HTTP_RESPONSE_HEADER_SIZE;
		header_len = http_build_headers(c, code, content_type, data_len,
									(char *)&c->tx_buffer[base],
									header_capacity);
		if (header_len < 0)
			return header_len;
	}
	if (data_len != 0U && c->chunked_mode)
	{
		prefix_len = str_snprintf(chunk_prefix, sizeof(chunk_prefix), "%lx\r\n",
								  (unsigned long)data_len);
		if (prefix_len <= 0 || (size_t)prefix_len >= sizeof(chunk_prefix))
			return SOCKET_DEVICE_INVALID;
	}

	required = (size_t)header_len + (size_t)prefix_len + data_len;
	if (data_len != 0U && c->chunked_mode)
		required += 2U;
	if (finishing && c->chunked_mode)
		required += 5U;
	if (required > sizeof(c->tx_buffer) - c->tx_length)
		return SOCKET_DEVICE_WOULD_BLOCK;

	tail = base + (size_t)header_len;
	if (prefix_len != 0)
	{
		memcpy(&c->tx_buffer[tail], chunk_prefix, (size_t)prefix_len);
		tail += (size_t)prefix_len;
	}
	if (data_len != 0U)
	{
		memcpy(&c->tx_buffer[tail], data, data_len);
		tail += data_len;
		if (c->chunked_mode)
		{
			memcpy(&c->tx_buffer[tail], "\r\n", 2U);
			tail += 2U;
		}
	}
	if (finishing && c->chunked_mode)
	{
		memcpy(&c->tx_buffer[tail], "0\r\n\r\n", 5U);
		tail += 5U;
	}
	c->tx_length = tail;
	if (header_len != 0)
		c->headers_sent = true;
	if (finishing)
		c->response_end_pending = true;
	if (!was_pending)
		(void)http_flush(client_idx);
	return (int)data_len;
}

static void http_progress_file(int client_idx)
{
	http_client_t *c;
	int accepted;

	if (!http_srv || client_idx < 0 || client_idx >= SOCKET_MAX_CLIENTS)
		return;
	c = &clients[client_idx];
	if (!c->response_file || c->tx_length != 0U || c->response_end_pending)
		return;
	if (c->file_buffer_len == 0U)
		c->file_buffer_len = fs_read(c->response_file,
									 c->file_buffer, sizeof(c->file_buffer));
	if (c->file_buffer_len == 0U)
	{
		fs_close(c->response_file);
		c->response_file = NULL;
		(void)http_send(client_idx, 200, c->response_content_type, NULL, 0U);
		return;
	}
	accepted = http_send(client_idx, 200, c->response_content_type,
						 c->file_buffer, c->file_buffer_len);
	if (accepted >= 0)
		c->file_buffer_len = 0U;
}

bool http_send_file(int client_idx, const char *file_path, const char *content_type)
{
	http_client_t *c;
	fs_file_t *file;
	const char *type = content_type ? content_type : "application/octet-stream";

	if (!http_srv || client_idx < 0 || client_idx >= SOCKET_MAX_CLIENTS || !file_path)
		return false;
	c = &clients[client_idx];
	if (c->response_file || c->headers_sent || c->response_end_pending)
		return false;
	file = fs_open(file_path, "rb");
	if (!file)
	{
		(void)http_send_str(client_idx, 404, "text/plain", "404 Not Found");
		(void)http_send(client_idx, 404, "text/plain", NULL, 0U);
		return false;
	}
	if (strlen(type) >= sizeof(c->response_content_type))
	{
		fs_close(file);
		return false;
	}
	strcpy(c->response_content_type, type);
	c->response_file = file;
	c->file_buffer_len = 0U;
	if (http_send(client_idx, 200, NULL, NULL, 0U) < 0)
	{
		fs_close(file);
		c->response_file = NULL;
		return false;
	}
	http_progress_file(client_idx);
	return true;
}

/* --------------- upload accessors ----------------- */

http_upload_t http_file_upload_status(int client_idx)
{
	http_upload_t empty;
	memset(&empty, 0, sizeof(empty));
	if (client_idx < 0 || client_idx >= SOCKET_MAX_CLIENTS)
		return empty;
	return clients[client_idx].fileupl;
}

void http_file_upload_name(int client_idx, char *filename, size_t maxlen)
{
	if (!filename || maxlen == 0)
		return;
	if (client_idx < 0 || client_idx >= SOCKET_MAX_CLIENTS)
	{
		filename[0] = '\0';
		return;
	}
	strncpy(filename, clients[client_idx].upl.upload_name, maxlen - 1);
	filename[maxlen - 1] = '\0';
}

// not necessary
// char *http_file_upload_buffer(int client_idx, size_t *len)
// {
// 	http_client_t *c = &clients[client_idx];
// 	if (!c)
// 	{
// 		if (len)
// 			*len = 0;
// 		return NULL;
// 	}
// 	if (len)
// 		*len = c->up_len;
// 	return (char *)c->up_buf;
// }

/* --------------- request dispatch ----------------- */

static void dispatch_request(int client_idx)
{
	http_client_t *c = &clients[client_idx];
	if (!c->route)
	{
		/* Minimal 404 */
		http_send_str(client_idx, 404, "text/plain", "404 Not Found");
		http_send(client_idx, 404, "text/plain", NULL, 0);
		return;
	}
	/* Set current and call user's request handler */
	if (c->route->request_handler)
		c->route->request_handler(client_idx);
}

/* --------------- upload handling ------------------ */

static void maybe_invoke_file_handler(int client_idx)
{
	http_client_t *c = &clients[client_idx];
	if (!c->route || !c->route->file_handler)
		return;
	c->route->file_handler(client_idx);
}

static void handle_upload_bytes(int client_idx, char **buf, size_t *len)
{
	http_client_t *c = &clients[client_idx];
	if (!c->upl.status || c->upl.boundary_len == 0 || *len == 0)
		return;

	// First-time: wait for starting boundary + part headers
	http_request_multipart_chunk(buf, len, &c->upl, &c->head);
	char *buffer = *buf;
	if (c->upl.status < REQ_UPLOAD_INIT_FINISHED)
	{
		return;
	}
	else if (c->upl.status < REQ_UPLOAD_START)
	{
		c->upl.status = REQ_UPLOAD_START;
		c->fileupl.status = HTTP_UPLOAD_START;
		c->fileupl.filename = c->upl.upload_name;
		c->fileupl.filelen = c->upl.upload_len;
		c->fileupl.datalen = 0;
		maybe_invoke_file_handler(client_idx);
		// check if file was aborted or not
		if (!c->route)
		{
			return;
		}
	}

	// Stream file data until boundary
	if (c->upl.status == REQ_UPLOAD_START)
	{
		if (*len && c->upl.upload_len)
		{
			if (*len < c->upl.upload_len)
			{
				c->fileupl.datalen = *len;
				*len = 0;
			}
			else
			{
				c->fileupl.datalen = c->upl.upload_len;
				*len -= c->upl.upload_len;
			}
			c->fileupl.status = HTTP_UPLOAD_PART;
			c->fileupl.data = (uint8_t *)buffer;
			maybe_invoke_file_handler(client_idx);
			// check if file was aborted or not
			if (!c->route)
			{
				return;
			}
			c->upl.upload_len -= c->fileupl.datalen;
			*buf = &buffer[c->fileupl.datalen];
			buffer = *buf;
			return;
		}
		else if (!c->upl.upload_len)
		{
			c->fileupl.status = HTTP_UPLOAD_END;
			c->fileupl.data = NULL;
			c->fileupl.datalen = 0;
			c->upl.status = REQ_UPLOAD_FINISH;
			if (!c->upl.boundary_len)
			{
				*buf = &buffer[*len];
				*len = 0;
				return;
			}
			append_str(c->upl.boundary, (char *)"--");
			maybe_invoke_file_handler(client_idx);
			// check if file was aborted or not
			if (!c->route)
			{
				return;
			}
		}
	}

	if (c->upl.status == REQ_UPLOAD_FINISH)
	{
		http_request_parse_header(&c->head, buf, len);
		if (c->head.status == REQ_HEAD_FINISHED)
		{
			if (c->head.name[0] == 0)
			{ /*discard empty line*/
			}
			else if (!strncmp(c->head.name, c->upl.boundary, strlen(c->upl.boundary)))
			{
				http_send_str(client_idx, 200, "text/plain; charset=UTF-8", "File uploaded successfully");
				http_send(client_idx, 200, "text/plain; charset=UTF-8", NULL, 0);
			}
			else
			{
				http_send_str(client_idx, 413, "text/plain; charset=UTF-8", "File error");
				http_send(client_idx, 413, "text/plain; charset=UTF-8", NULL, 0);
			}
		}
	}
}

/* --------------- socket callbacks ----------------- */

static void http_on_connected(uint8_t client_idx, void *protocol)
{
	(void)protocol;
	if (client_idx < SOCKET_MAX_CLIENTS)
		client_reset(client_idx);
}

static void http_on_disconnected(uint8_t client_idx, int reason, void *protocol)
{
	(void)reason;
	(void)protocol;
	if (client_idx < SOCKET_MAX_CLIENTS)
		release_client(client_idx);
}

/*
 * Writable resumes only the exact persistent suffix previously accepted by
 * http_send(). A file is advanced only after that suffix has fully drained.
 */
static void http_on_writable(uint8_t client_idx, void *protocol)
{
	(void)protocol;
	if (client_idx >= SOCKET_MAX_CLIENTS)
		return;
	(void)http_flush((int)client_idx);
	if (clients[client_idx].tx_length == 0U)
		http_progress_file((int)client_idx);
}

/* Fully accepted file chunks may not cause a writable edge, so idle advances
 * at most one file read while no older bytes are pending. */
static void http_on_idle(uint8_t client_idx, uint32_t idle_ms, void *protocol)
{
	(void)idle_ms;
	(void)protocol;
	if (client_idx < SOCKET_MAX_CLIENTS && clients[client_idx].tx_length == 0U)
		http_progress_file((int)client_idx);
}

static void http_on_data(uint8_t client_idx,
					 const uint8_t *data,
					 size_t data_len,
					 void *protocol)
{
	http_client_t *c;
	char *bytes;
	(void)protocol;
	if (client_idx >= SOCKET_MAX_CLIENTS || !data || data_len == 0U)
		return;
	c = &clients[client_idx];
	/* HTTP pipelining is deliberately unsupported while a response is active. */
	if (c->response_end_pending || c->response_file)
		return;
	/* Request parser advances the pointer but must treat pointed RX bytes read-only. */
	bytes = (char *)(uintptr_t)data;

	do
	{
		// parse request start line
		if (!c->have_reqline)
		{
			http_request_parse_start(&c->req, &bytes, &data_len);
			if ((c->req.status == REQ_START_FINISHED))
			{
				c->route = match_route(c->req.uri, c->req.method);
				c->have_reqline = true;
			}
		}
		else if (!c->have_headers) // parse request headers
		{
			while (data_len)
			{
				http_request_parse_header(&c->head, &bytes, &data_len);
				if (c->head.status == REQ_HEAD_FINISHED)
				{
					if (!strncasecmp_local((char *)"connection", c->head.name, 10))
					{
						if (strcasestr_local(c->head.value, "keep-alive"))
							c->keep_alive = true;
						if (strcasestr_local(c->head.value, "close"))
							c->keep_alive = false;
					}
					if (c->head.name[0] == 0)
					{
						// empty line
						// headers done
						c->have_headers = true;

						// dispatch response
						if (!c->upl.status)
						{
							dispatch_request(client_idx);
						}
						else
						{
							handle_upload_bytes(client_idx, &bytes, &data_len);
						}
						break;
					}
					if (c->req.method == HTTP_REQ_POST || c->req.method == HTTP_REQ_PUT)
					{
						http_request_file_upload(&c->upl, &c->head);
					}
				}
			}
		}
		else
		{
			handle_upload_bytes(client_idx, &bytes, &data_len);
		}

	} while (data_len);
}

DECL_MODULE(http_server)
{
	RUNONCE
	{
		LOAD_MODULE(socket_server);
		/* init slots */
		for (int i = 0; i < SOCKET_MAX_CLIENTS; i++)
		{
			client_reset(i);
		}
		http_srv = socket_start(IP_ANY, HTTP_PORT);
		if (!http_srv)
			return;
		socket_set_protocol(http_srv, clients);
		socket_add_ondata_handler(http_srv, http_on_data);
		socket_add_onconnected_handler(http_srv, http_on_connected);
		socket_add_onwritable_handler(http_srv, http_on_writable);
		socket_add_onidle_handler(http_srv, http_on_idle);
		socket_add_ondisconnected_handler(http_srv, http_on_disconnected);

		RUNONCE_COMPLETE();
	}
}

#endif
