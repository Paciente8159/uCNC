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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "http.h"

#ifndef HTTP_PORT
#define HTTP_PORT 80
#endif

#define HTTP_MAX_HANDLERS 8
#define HTTP_MAX_HEADERS 8
#define HTTP_MAX_HEADER_NAME 32
#define HTTP_MAX_HEADER_VALUE 64
#define HTTP_HEADER_BUF_SIZE 1024
#define HTTP_UPLOAD_BUF_SIZE 512
#define HTTP_MAX_BOUNDARY_LEN 70

typedef struct
{
	char name[HTTP_MAX_HEADER_NAME];
	char value[HTTP_MAX_HEADER_VALUE];
} http_header_kv_t;

typedef struct
{
	char *uri;
	uint8_t method;								 /* HTTP_REQ_* from http_request.h */
	http_delegate request_handler; /* called when request ready */
	http_delegate file_handler;		 /* called on upload START/PART/END/ABORT */
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
	char hbuf[HTTP_HEADER_BUF_SIZE];

	/* Small chunk buffer surfaced to app */
	uint8_t up_buf[HTTP_UPLOAD_BUF_SIZE];
	size_t up_len;
	uint8_t up_status; /* HTTP_UPLOAD_* */

	/* Response bookkeeping */
	bool headers_sent;
	bool chunked_mode;
	bool keep_alive;
	size_t hdr_count;
	http_header_kv_t hdrs[HTTP_MAX_HEADERS];

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
	memset(&clients[client_idx], 0, sizeof(clients[client_idx]));
}

static void reset_request_state(int client_idx)
{
	// Reset only per-request parsing state, keep socket & keep_alive flag
	memset(&clients[client_idx].req, 0, sizeof(clients[client_idx].req));
	memset(&clients[client_idx].head, 0, sizeof(clients[client_idx].head));
	memset(&clients[client_idx].upl, 0, sizeof(clients[client_idx].upl));
	memset(&clients[client_idx].fileupl, 0, sizeof(clients[client_idx].fileupl));
	clients[client_idx].have_reqline = false;
	clients[client_idx].have_headers = false;
	clients[client_idx].headers_sent = false;
	clients[client_idx].chunked_mode = false;
	clients[client_idx].hdr_count = 0;
	clients[client_idx].route = NULL;
}

static void release_client(int client_idx)
{
	if (client_idx >= 0)
	{
		if (clients[client_idx].upl.status == REQ_UPLOAD_INIT_FINISHED)
		{
			/* Notify abort if we die mid-upload */
			clients[client_idx].up_status = HTTP_UPLOAD_ABORT;
			if (clients[client_idx].route && clients[client_idx].route->file_handler)
			{
				clients[client_idx].route->file_handler(client_idx);
			}
		}
		client_reset(client_idx);
		return;
	}
}

static bool uri_matches(char *pattern, char *uri)
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

static http_route_t *match_route(char *uri, uint8_t method)
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

void http_add(char *uri, uint8_t method, http_delegate request_handler, http_delegate file_handler)
{
	if (route_count < HTTP_MAX_HANDLERS)
	{
		routes[route_count].uri = uri;
		routes[route_count].method = method;
		routes[route_count].request_handler = request_handler;
		routes[route_count].file_handler = file_handler;
		route_count++;
	}
}

int http_request_hasargs(int client_idx)
{
	http_client_t *c = &clients[client_idx];
	return c ? (c->req.arg_count > 0) : 0;
}

void http_request_uri(int client_idx, char *uri, size_t maxlen)
{
	http_client_t *c = &clients[client_idx];
	if (!uri || maxlen == 0)
		return;
	if (c)
	{
		strncpy(uri, c->req.uri, maxlen - 1);
		uri[maxlen - 1] = '\0';
	}
	else
	{
		uri[0] = '\0';
	}
}

bool http_request_arg(int client_idx, char *argname, char *argvalue, size_t maxlen)
{
	http_client_t *c = &clients[client_idx];
	if (!c || !argname || !argvalue || maxlen == 0)
		return false;
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
	http_client_t *c = &clients[client_idx];
	return c ? c->req.method : HTTP_REQ_OTHER;
}

/* --------------- response helpers ----------------- */

void http_send_header(int client_idx, char *name, char *data, bool first)
{
	http_client_t *c = &clients[client_idx];
	if (!c || !name || !data)
		return;
	if (first)
	{
		c->hdr_count = 0;
		c->headers_sent = false;
	}
	/* append or add */
	for (size_t i = 0; i < c->hdr_count; i++)
	{
		if (strcasecmp(c->hdrs[i].name, name) == 0)
		{
			size_t l = strlen(c->hdrs[i].value);
			if (l + 2 < HTTP_MAX_HEADER_VALUE)
			{
				strncat(c->hdrs[i].value, ", ", HTTP_MAX_HEADER_VALUE - l - 1);
				strncat(c->hdrs[i].value, data, HTTP_MAX_HEADER_VALUE - strlen(c->hdrs[i].value) - 1);
			}
			return;
		}
	}
	if (c->hdr_count < HTTP_MAX_HEADERS)
	{
		strncpy(c->hdrs[c->hdr_count].name, name, HTTP_MAX_HEADER_NAME - 1);
		c->hdrs[c->hdr_count].name[HTTP_MAX_HEADER_NAME - 1] = '\0';
		strncpy(c->hdrs[c->hdr_count].value, data, HTTP_MAX_HEADER_VALUE - 1);
		c->hdrs[c->hdr_count].value[HTTP_MAX_HEADER_VALUE - 1] = '\0';
		c->hdr_count++;
	}
}

void http_send(int client_idx, int code, char *content_type, uint8_t *data, size_t data_len)
{
	http_client_t *c = &clients[client_idx];
	if (!c || client_idx < 0)
		return;

	char buf[128];

	/* Enable chunked mode when content_type == NULL (headers delayed) */
	if (content_type == NULL)
	{
		c->chunked_mode = true;
		if (!c->headers_sent)
		{
			http_send_header(client_idx, "Transfer-Encoding", "chunked", false);
		}
		if (!data && data_len == 0)
		{
			/* Just preparing; do not emit yet */
			return;
		}
	}

	if (!c->headers_sent)
	{
		int n = snprintf(buf, sizeof(buf), "HTTP/1.1 %d OK\r\n", code);
		socket_send(http_srv, client_idx, buf, (size_t)n, 0);

		n = snprintf(buf, sizeof(buf), "Connection: %s\r\n", (c->keep_alive ? "keep-alive" : "close"));
		socket_send(http_srv, client_idx, buf, (size_t)n, 0);

		if (content_type)
		{
			n = snprintf(buf, sizeof(buf), "Content-Type: %s\r\n", content_type);
			socket_send(http_srv, client_idx, buf, (size_t)n, 0);
		}
		for (size_t i = 0; i < c->hdr_count; i++)
		{
			n = snprintf(buf, sizeof(buf), "%s: %s\r\n", c->hdrs[i].name, c->hdrs[i].value);
			socket_send(http_srv, client_idx, buf, (size_t)n, 0);
		}
		socket_send(http_srv, client_idx, "\r\n", 2, 0);
		c->headers_sent = true;
	}

	if (data && data_len > 0)
	{
		if (c->chunked_mode)
		{
			int n = snprintf(buf, sizeof(buf), "%x\r\n", (unsigned int)data_len);
			socket_send(http_srv, client_idx, buf, (size_t)n, 0);
			socket_send(http_srv, client_idx, (char *)data, data_len, 0);
			socket_send(http_srv, client_idx, "\r\n", 2, 0);
		}
		else
		{
			socket_send(http_srv, client_idx, (char *)data, data_len, 0);
		}
	}

	/* Close if zero-length body marks completion */
	if (!data || data_len == 0)
	{
		if (c->chunked_mode)
		{
			socket_send(http_srv, client_idx, "0\r\n\r\n", 5, 0);
		}

		if (!c->keep_alive)
		{
			// Close connection if not persistent
			socket_free(http_srv, client_idx);
			client_reset(client_idx); // full reset, frees slot
		}
		else
		{
			// Persistent connection: reset only request state
			reset_request_state(client_idx);
			// Keep socket open for next request
		}
	}
}

bool http_send_file(int client_idx, char *file_path, char *content_type)
{
	http_client_t *c = &clients[client_idx];
	if (!c || client_idx < 0 || !file_path)
		return false;

	uint8_t fbuf[512];
	size_t nread;
	fs_file_t *fp = fs_open(file_path, "rb");
	if (!fp)
	{
		http_send_str(client_idx, 404, "text/plain", "404 Not Found");
		http_send(client_idx, 404, "text/plain", NULL, 0);
		return false;
	}

	/* Chunked stream (size-free) */
	http_send(client_idx, 200, NULL, NULL, 0); /* prepare chunked */
	while ((nread = fs_read(fp, fbuf, sizeof(fbuf))) > 0)
	{
		http_send(client_idx, 200, content_type ? content_type : "application/octet-stream", fbuf, nread);
	}
	fs_close(fp);
	http_send(client_idx, 200, content_type ? content_type : "application/octet-stream", NULL, 0);
	return true;
}

/* --------------- upload accessors ----------------- */

http_upload_t http_file_upload_status(int client_idx)
{
	return clients[client_idx].fileupl;
}

void http_file_upload_name(int client_idx, char *filename, size_t maxlen)
{
	http_client_t *c = &clients[client_idx];

	if (!filename || maxlen == 0)
		return;
	if (!c)
	{
		filename[0] = '\0';
		return;
	}
	size_t start = strlen(filename);
	strncpy(&filename[start], c->upl.upload_name, maxlen - 1);
	filename[maxlen - 1] = '\0';
}

char *http_file_upload_buffer(int client_idx, size_t *len)
{
	http_client_t *c = &clients[client_idx];
	if (!c)
	{
		if (len)
			*len = 0;
		return NULL;
	}
	if (len)
		*len = c->up_len;
	return (char *)c->up_buf;
}

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
		maybe_invoke_file_handler(client_idx);
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
			c->fileupl.data = (uint8_t*)buffer;
			maybe_invoke_file_handler(client_idx);
			c->upl.upload_len -= c->fileupl.datalen;
			*buf = &buffer[c->fileupl.datalen];
			buffer = *buf;
			return;
		}
		else if (!c->upl.upload_len)
		{
			c->fileupl.status = HTTP_UPLOAD_END;
			maybe_invoke_file_handler(client_idx);
			c->upl.status = REQ_UPLOAD_FINISH;
			append_str(c->upl.boundary, "--");
		}
	}

	if (c->upl.status == REQ_UPLOAD_FINISH)
	{
		http_request_parse_header(&c->head, buf, len);
		if (c->head.status == REQ_HEAD_FINISHED)
		{
			if(c->head.name[0]==0){/*discard empty line*/}
			else if(!strncmp(c->head.name, c->upl.boundary, strlen(c->upl.boundary))){
				http_send_str(client_idx, 200, "text/plain; charset=UTF-8", "File uploaded successfully");
				http_send(client_idx, 200, "text/plain; charset=UTF-8", NULL, 0);
				reset_request_state(client_idx);
			}
			else{
				http_send_str(client_idx, 413, "text/plain; charset=UTF-8", "File error");
				http_send(client_idx, 413, "text/plain; charset=UTF-8", NULL, 0);
			}
		}
	}
}

/* --------------- socket callbacks ----------------- */

static void http_on_connected(uint8_t client_idx, void* protocol)
{
	client_reset(client_idx);
}

static void http_on_disconnected(uint8_t client_idx, void* protocol)
{
	release_client(client_idx);
}

static void http_on_data(uint8_t client_idx, char *data, size_t data_len, void* protocol)
{
	http_client_t *c = &clients[client_idx]; // get_client_slot(client_idx);
	if (!c || data_len == 0)
		return;

	char *bytes = (char *)data;

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
					if (!strncasecmp_local("connection", c->head.name, 10))
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
		http_srv = socket_start_listen(IP_ANY, HTTP_PORT, 2 /*AF_INET*/, 1 /*SOCK_STREAM*/, 0);
		if (!http_srv)
			return;
		socket_add_ondata_handler(http_srv, http_on_data);
		socket_add_onconnected_handler(http_srv, http_on_connected);
		socket_add_ondisconnected_handler(http_srv, http_on_disconnected);

		RUNONCE_COMPLETE();
	}
}

#endif
