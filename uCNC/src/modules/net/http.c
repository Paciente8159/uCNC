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
#define HTTP_MAX_HEADER_LEN 128

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

	/* Response bookkeeping */
	bool headers_sent;
	bool chunked_mode;
	bool keep_alive;
	uint16_t hdr_len;
	char hdrs[HTTP_CUSTOM_HEADERS_SIZE];


	/* Cooperative file-response state. */
	fs_file_t *response_file;

	/* Selected route for this request (if matched) */
	http_route_t *route;
} http_client_t;

/* Socket interface (single listener) */
static socket_if_t *http_srv = NULL;

/* Client slots align with SOCKET_MAX_CLIENTS from socket.h */
static http_client_t clients[SOCKET_MAX_CLIENTS];

/* One cooperative file-read buffer shared by all clients. */
static uint8_t http_file_buffer[HTTP_FILE_CHUNK_SIZE];

/* Routes */
static http_route_t routes[HTTP_MAX_HANDLERS];
static uint8_t route_count = 0U;

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
	if (client_idx < 0 || client_idx >= SOCKET_MAX_CLIENTS)
		return;
	/* Reset request/response metadata after a synchronous response finishes. */
	memset(&clients[client_idx].req, 0, sizeof(clients[client_idx].req));
	memset(&clients[client_idx].head, 0, sizeof(clients[client_idx].head));
	memset(&clients[client_idx].upl, 0, sizeof(clients[client_idx].upl));
	memset(&clients[client_idx].fileupl, 0, sizeof(clients[client_idx].fileupl));
	clients[client_idx].have_reqline = false;
	clients[client_idx].have_headers = false;
	clients[client_idx].headers_sent = false;
	clients[client_idx].chunked_mode = false;
	clients[client_idx].hdr_len = 0U;
	clients[client_idx].route = NULL;
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
	for (uint8_t i = 0U; i < route_count; ++i)
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
	for (uint8_t i = 0U; i < route_count; ++i)
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

static int http_send_bytes(int client_idx, const void *data, size_t data_len)
{
	int sent;

	if (!http_srv || client_idx < 0 || client_idx >= SOCKET_MAX_CLIENTS)
		return SOCKET_DEVICE_INVALID;
	sent = socket_send(http_srv, (uint8_t)client_idx, data, data_len, false);
	if (sent < 0 && socket_client_is_connected(http_srv, (uint8_t)client_idx))
		(void)socket_close(http_srv, (uint8_t)client_idx);
	return sent;
}

static int http_send_headers(http_client_t *c,
							 int client_idx,
							 int code,
							 const char *content_type,
							 size_t content_length)
{
	char line[HTTP_MAX_HEADER_LEN];
	int length;
	int result;
	size_t i;

	if (!c)
		return SOCKET_DEVICE_INVALID;
	if (content_type && strlen(content_type) > sizeof(line) - sizeof("Content-Type: \r\n"))
		return SOCKET_DEVICE_INVALID;
	length = str_snprintf(line, sizeof(line), "HTTP/1.1 %d %s\r\n",
						  code, http_reason_phrase(code));
	if (length <= 0 || (size_t)length >= sizeof(line))
		return SOCKET_DEVICE_INVALID;
	result = http_send_bytes(client_idx, line, (size_t)length);
	if (result < 0)
		return result;

	length = str_snprintf(line, sizeof(line), "Connection: %s\r\n",
						  c->keep_alive ? "keep-alive" : "close");
	if (length <= 0 || (size_t)length >= sizeof(line))
		return SOCKET_DEVICE_INVALID;
	result = http_send_bytes(client_idx, line, (size_t)length);
	if (result < 0)
		return result;

	if (c->chunked_mode)
	{
		static const char chunked[] = "Transfer-Encoding: chunked\r\n";
		result = http_send_bytes(client_idx, chunked, sizeof(chunked) - 1U);
		if (result < 0)
			return result;
	}
	else
	{
		length = str_snprintf(line, sizeof(line), "Content-Length: %lu\r\n",
							  (unsigned long)content_length);
		if (length <= 0 || (size_t)length >= sizeof(line))
			return SOCKET_DEVICE_INVALID;
		result = http_send_bytes(client_idx, line, (size_t)length);
		if (result < 0)
			return result;
	}

	if (content_type)
	{
		length = str_snprintf(line, sizeof(line), "Content-Type: %s\r\n", content_type);
		if (length <= 0 || (size_t)length >= sizeof(line))
			return SOCKET_DEVICE_INVALID;
		result = http_send_bytes(client_idx, line, (size_t)length);
		if (result < 0)
			return result;
	}

	for (i = 0U; i < c->hdr_len;)
	{
		const char *header = &c->hdrs[i];
		size_t header_len = strlen(header);
		result = http_send_bytes(client_idx, header, header_len);
		if (result < 0)
			return result;
		result = http_send_bytes(client_idx, "\r\n", 2U);
		if (result < 0)
			return result;
		i += header_len + 1U;
	}
	result = http_send_bytes(client_idx, "\r\n", 2U);
	if (result < 0)
		return result;

	c->headers_sent = true;
	return SOCKET_DEVICE_OK;
}

static void http_complete_response(int client_idx)
{
	http_client_t *c;
	if (client_idx < 0 || client_idx >= SOCKET_MAX_CLIENTS)
		return;
	c = &clients[client_idx];
	if (c->keep_alive)
		reset_request_state(client_idx);
	else if (http_srv)
		(void)socket_close(http_srv, (uint8_t)client_idx);
}

bool http_send_header(int client_idx, const char *name, const char *data, bool first)
{
	http_client_t *c;
	size_t name_len;
	size_t data_len;
	size_t offset;
	int length;

	if (client_idx < 0 || client_idx >= SOCKET_MAX_CLIENTS || !name || !data)
		return false;
	c = &clients[client_idx];
	if (c->headers_sent)
		return false;
	if (first)
		c->hdr_len = 0U;

	name_len = strlen(name);
	data_len = strlen(data);
	if (name_len == 0U || name_len >= HTTP_MAX_HEADER_LEN ||
		data_len > HTTP_MAX_HEADER_LEN - name_len - 3U ||
		strchr(name, '\r') || strchr(name, '\n') || strchr(name, ':') ||
		strchr(data, '\r') || strchr(data, '\n'))
		return false;

	/* Merge a repeated header name into its existing compact line. */
	for (offset = 0U; offset < c->hdr_len;)
	{
		char *line = &c->hdrs[offset];
		size_t line_len = strlen(line);
		if (strncasecmp_local(line, (char *)name, name_len) == 0 &&
			line[name_len] == ':')
		{
			size_t extra = data_len + 2U;
			size_t tail = offset + line_len;
			if (extra > sizeof(c->hdrs) - c->hdr_len ||
				line_len + extra >= HTTP_MAX_HEADER_LEN)
				return false;
			memmove(&c->hdrs[tail + extra], &c->hdrs[tail], c->hdr_len - tail);
			memcpy(&c->hdrs[tail], ", ", 2U);
			memcpy(&c->hdrs[tail + 2U], data, data_len);
			c->hdr_len = (uint16_t)(c->hdr_len + extra);
			return true;
		}
		offset += line_len + 1U;
	}

	if (c->hdr_len >= sizeof(c->hdrs))
		return false;
	length = str_snprintf(&c->hdrs[c->hdr_len], sizeof(c->hdrs) - c->hdr_len,
						  "%s: %s", name, data);
	if (length <= 0 || (size_t)length >= sizeof(c->hdrs) - c->hdr_len)
		return false;
	c->hdr_len = (uint16_t)(c->hdr_len + (size_t)length + 1U);
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
	int prefix_len;
	int result;
	bool finishing = data_len == 0U;

	if (!http_srv || client_idx < 0 || client_idx >= SOCKET_MAX_CLIENTS ||
		(!data && data_len != 0U) || data_len > HTTP_MAX_CHUNCK_LEN ||
		data_len > (size_t)INT_MAX)
		return SOCKET_DEVICE_INVALID;
	if (!socket_client_is_connected(http_srv, (uint8_t)client_idx))
		return SOCKET_DEVICE_CLOSED;
	c = &clients[client_idx];

	/* Initial NULL/empty call selects chunked mode without emitting bytes. */
	if (!content_type && finishing && !c->headers_sent && !c->chunked_mode)
	{
		c->chunked_mode = true;
		return 0;
	}
	if (!content_type)
		c->chunked_mode = true;

	if (!c->headers_sent)
	{
		result = http_send_headers(c, client_idx, code, content_type, data_len);
		if (result < 0)
			return result;
	}

	if (data_len != 0U)
	{
		if (c->chunked_mode)
		{
			prefix_len = str_snprintf(chunk_prefix, sizeof(chunk_prefix), "%lx\r\n",
								  (unsigned long)data_len);
			if (prefix_len <= 0 || (size_t)prefix_len >= sizeof(chunk_prefix))
				return SOCKET_DEVICE_INVALID;
			result = http_send_bytes(client_idx, chunk_prefix, (size_t)prefix_len);
			if (result < 0)
				return result;
		}

		result = http_send_bytes(client_idx, data, data_len);
		if (result < 0)
			return result;
		if (c->chunked_mode)
		{
			result = http_send_bytes(client_idx, "\r\n", 2U);
			if (result < 0)
				return result;
		}
	}

	if (finishing)
	{
		if (c->chunked_mode)
		{
			result = http_send_bytes(client_idx, "0\r\n\r\n", 5U);
			if (result < 0)
				return result;
		}
		http_complete_response(client_idx);
	}
	return (int)data_len;
}

static void http_progress_file(int client_idx)
{
	http_client_t *c;
	size_t length;

	if (!http_srv || client_idx < 0 || client_idx >= SOCKET_MAX_CLIENTS)
		return;
	c = &clients[client_idx];
	if (!c->response_file)
		return;

	length = fs_read(c->response_file, http_file_buffer, sizeof(http_file_buffer));
	if (length == 0U)
	{
		fs_close(c->response_file);
		c->response_file = NULL;
		(void)http_send(client_idx, 200, NULL, NULL, 0U);
		return;
	}

	if (http_send(client_idx, 200, NULL, http_file_buffer, length) < 0)
	{
		fs_close(c->response_file);
		c->response_file = NULL;
		if (socket_client_is_connected(http_srv, (uint8_t)client_idx))
			(void)socket_close(http_srv, (uint8_t)client_idx);
	}
}

bool http_send_file(int client_idx, const char *file_path, const char *content_type)
{
	http_client_t *c;
	fs_file_t *file;
	const char *type = content_type ? content_type : "application/octet-stream";

	if (!http_srv || client_idx < 0 || client_idx >= SOCKET_MAX_CLIENTS || !file_path)
		return false;
	c = &clients[client_idx];
	if (c->response_file || c->headers_sent)
		return false;

	file = fs_open(file_path, "rb");
	if (!file)
	{
		(void)http_send_str(client_idx, 404, "text/plain", "404 Not Found");
		(void)http_send(client_idx, 404, "text/plain", NULL, 0U);
		return false;
	}

	c->chunked_mode = true;
	if (http_send_headers(c, client_idx, 200, type, 0U) < 0)
	{
		fs_close(file);
		return false;
	}
	c->response_file = file;
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

/* Advance at most one cooperative file read/send per idle callback. */
static void http_on_idle(uint8_t client_idx, uint32_t idle_ms, void *protocol)
{
	(void)idle_ms;
	(void)protocol;
	if (client_idx < SOCKET_MAX_CLIENTS && clients[client_idx].response_file)
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
	if (c->response_file)
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
			socket_add_onidle_handler(http_srv, http_on_idle);
		socket_add_ondisconnected_handler(http_srv, http_on_disconnected);

		RUNONCE_COMPLETE();
	}
}

#endif
