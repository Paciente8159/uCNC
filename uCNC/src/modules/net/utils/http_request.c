/*
	Name: http_request.c
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

#include "../../../cnc.h"
#include "http_request.h"


int strncasecmp_local(char *s1, char *s2, size_t len)
{
	while (len--)
	{
		unsigned char c1 = (unsigned char)*s1++;
		unsigned char c2 = (unsigned char)*s2++;

		if (c1 >= 'A' && c1 <= 'Z')
			c1 += 'a' - 'A';
		if (c2 >= 'A' && c2 <= 'Z')
			c2 += 'a' - 'A';

		if (c1 != c2)
			return c1 - c2;
		if (c1 == '\0')
			return 0;
	}
	return 0;
}

char *strcasestr_local(char *haystack, char *needle)
{
	if (!*needle) // Empty needle matches at start
		return haystack;

	for (const char *h = haystack; *h; h++)
	{
		const char *n = needle;
		const char *hh = h;

		// Compare until mismatch or end of needle
		while (*n && *hh)
		{
			unsigned char c1 = (unsigned char)*hh;
			unsigned char c2 = (unsigned char)*n;

			if (c1 >= 'A' && c1 <= 'Z')
				c1 += 'a' - 'A';
			if (c2 >= 'A' && c2 <= 'Z')
				c2 += 'a' - 'A';

			if (c1 != c2)
				break;

			hh++;
			n++;
		}

		if (*n == '\0') // Found full match
			return h;
	}

	return NULL; // No match
}

char *strntrim_local(char *s)
{
	if (s == NULL)
		return NULL;

	// Skip leading whitespace
	char *start = s;
	while (*start && (start[0] == ' ' || start[0] == '\t'))
		start++;

	// Find end of string, then skip trailing whitespace
	char *end = s + strlen(s);
	while (end > start && (end[-1] == ' ' || end[-1] == '\t'))
		end--;

	// Compute new length and shift left if needed
	size_t new_len = (size_t)(end - start);
	if (start != s)
		memmove(s, start, new_len); // safe for overlap

	s[new_len] = '\0';
	return s;
}

char *find_char(char *buf, size_t len, char c)
{

	for (size_t i = 0; i < len; i++)
	{
		if (buf[i] == c)
			return (char *)&buf[i];
		if (!buf[i])
		{
			return NULL;
		}
	}
	return NULL;
}

char *find_closest(char *buffer, size_t len, char c1, char c2)
{
	char *p1 = find_char(buffer, len, c1);
	char *p2 = find_char(buffer, len, c2);

	if (p1 && p2)
	{
		return MIN(p1, p2);
	}
	else if (p1)
	{
		return p1;
	}

	return p2;
}

int hex_val(int c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	c = tolower(c);
	if (c >= 'a' && c <= 'f')
		return 10 + (c - 'a');
	return -1;
}

void url_decode(char *src, char *dst, size_t dst_size, int plus_to_space)
{
	// Decodes %HH and, if plus_to_space!=0, converts '+' to space
	if (!dst_size)
		return;
	size_t di = 0;
	for (size_t i = 0; src[i] && i < dst_size;)
	{
		char c = src[i];
		if (c == '%' && src[i + 1] && src[i + 2])
		{
			int hi = hex_val((unsigned char)src[i + 1]);
			int lo = hex_val((unsigned char)src[i + 2]);
			if (hi >= 0 && lo >= 0)
			{
				dst[di++] = (char)((hi << 4) | lo);
				dst[di] = 0;
				i += 3;
				continue;
			}
		}
		if (plus_to_space && c == '+')
			c = ' ';
		dst[di++] = c;
		i++;
	}
}

void strncat_local(char *dst, size_t maxlen, char *src, size_t ncount)
{
	size_t p = strlen(dst);
	maxlen -= p;
	maxlen = MIN(strlen(src), maxlen);
	strncpy(&dst[p], src, MIN(maxlen, ncount));
}

uint8_t http_discard_line(uint8_t status, uint8_t initial_condition, char **buf, size_t *len)
{
	char *buffer = *buf;
	char *target = NULL;

	if (!*len)
		return status;

	if (status == initial_condition)
	{
		target = find_char(buffer, *len, '\r');
		if (target)
		{
			*target++ = 0;
			status++;
			(*len) -= (target - buffer);
			buffer = target;
		}
		else
		{
			status = initial_condition;
			*len = 0;
		}
	}

	if (!*len)
		return status;

	if (status == (initial_condition + 1))
	{
		if (*buffer == '\n')
		{
			*buffer++ = 0;
			status++;
			(*len)--;
		}
		else
		{
			status = initial_condition;
		}
	}

	*buf = buffer;
	return status;
}

void http_request_parse_start(request_ctx_t *ctx, char **buf, size_t *len)
{
	char *buffer = *buf;
	ctx->last_char = buffer[*len - 1];
	// Initialize context if not already processed
	if (ctx->status < REQ_START_FINISHED)
	{
		char *target = buffer;

		// this also signals the line start (since a chunk of the http request may not contain a complete line)
		if (ctx->status < REQ_START_METHOD_PARSED)
		{
			target = find_char(buffer, 10, ' ');
			if (!target) // it should at least find the method at the start of the string. If not something went wrong
			{
				return;
			}
			size_t l = (target - buffer);
			*target++ = 0;

			if (!strncasecmp_local(buffer, "GET", l))
			{
				ctx->method = HTTP_REQ_GET;
			}
			else if (!strncasecmp_local(buffer, "POST", l))
			{
				ctx->method = HTTP_REQ_POST;
			}
			else if (!strncasecmp_local(buffer, "PUT", l))
			{
				ctx->method = HTTP_REQ_PUT;
			}
			else if (!strncasecmp_local(buffer, "DELETE", l))
			{
				ctx->method = HTTP_REQ_DELETE;
			}
			else
			{
				ctx->method = HTTP_REQ_OTHER;
			}

			(*len) -= (target - buffer);
			buffer = target;
			ctx->status = REQ_START_METHOD_PARSED;
		}

		if (!*len)
			return;

		if (ctx->status < REQ_START_URI_PARSED)
		{
			// now tries to find the method target
			target = find_char(buffer, *len, ' ');
			size_t upto = *len;
			if (target)
			{
				upto = (target - buffer);
			}
			// tries anchor
			target = find_closest(buffer, upto, '#', '?');
			if (!target)
			{
				target = find_char(buffer, *len, ' ');
			}

			if (target)
			{
				switch (*target)
				{
				case '?':
					ctx->status = REQ_START_QUERYVAR_FOUND;
					break;
				case ' ':
					ctx->status = REQ_START_QUERY_PARSED;
					break;
				case '#':
					ctx->status = REQ_START_URI_PARSED;
					break;
				}
				*target++ = 0;
			}
			else
			{
				target = &buffer[*len];
			}

			append_str(ctx->uri, buffer);
			(*len) -= (target - buffer);
			buffer = target;
		}

		// parses query
		while (ctx->status < REQ_START_QUERY_PARSED)
		{
			if (!*len)
				return;

			uint8_t counter = ctx->arg_count;
			int arg_offset = 0;
			int arg_max = 0;
			if (counter >= MAX_URL_ARGS)
			{
				ctx->status = REQ_START_QUERY_PARSED;
				break;
			}

			switch (ctx->status)
			{
			case REQ_START_URI_PARSED:
				target = find_closest(buffer, *len, '?', ' ');
				if (target)
				{
					ctx->status = (*target == ' ') ? REQ_START_QUERY_PARSED : REQ_START_QUERYVAR_FOUND;
				}
				else
				{
					*len = 0;
					break;
				}

				*target++ = 0;
				(*len) -= (target - buffer);
				buffer = target;
				break;
			case REQ_START_QUERYVAR_FOUND:
				target = find_closest(buffer, *len, '=', '&');
				if (target)
				{
					if (*target == '=')
					{
						ctx->status = REQ_START_QUERYARG_FOUND;
					}
					*target++ = 0;
					// memset(ctx->arg_val[ctx->arg_count], 0, sizeof(ctx->arg_val[ctx->arg_count]));
					counter++;
				}
				else
				{
					target = find_char(buffer, *len, ' ');
					if (target)
					{
						*target++ = 0;
						ctx->status = REQ_START_QUERY_PARSED;
					}
					else
					{
						target = &buffer[*len];
					}
				}

				append_str(ctx->arg_name[ctx->arg_count], buffer);
				(*len) -= (target - buffer);
				buffer = target;
				ctx->arg_count = counter;
				break;
			case REQ_START_QUERYARG_FOUND:
				arg_offset = strlen(ctx->arg_name[ctx->arg_count - 1]) + 1;
				arg_max = sizeof(ctx->arg_name[ctx->arg_count]) - arg_offset;
				target = find_char(buffer, *len, '&');
				if (target)
				{
					*target++ = 0;
					// memset(ctx->arg_name[ctx->arg_count], 0, sizeof(ctx->arg_name[ctx->arg_count]));
					ctx->status = REQ_START_QUERYVAR_FOUND;
				}
				else
				{
					target = find_char(buffer, *len, ' ');
					if (target)
					{
						*target++ = 0;
						ctx->status = REQ_START_QUERY_PARSED;
					}
					else
					{
						target = &buffer[*len];
					}
				}

				ctx->arg_val[ctx->arg_count - 1] = &ctx->arg_name[ctx->arg_count - 1][arg_offset];
				strncat_local(ctx->arg_val[ctx->arg_count - 1], arg_max, buffer, strlen(buffer));
				(*len) -= (target - buffer);
				buffer = target;
				break;
			}
		}

		ctx->status = http_discard_line(ctx->status, REQ_START_QUERY_PARSED, &buffer, len);
	}

	*buf = buffer;
}

void http_request_parse_header(request_header_t *header, char **buf, size_t *len)
{
	char *buffer = *buf;
	char *target = NULL;

	if (header->status == REQ_HEAD_FINISHED)
	{
		// auto reset
		memset(header, 0, sizeof(request_header_t));
	}

	if (header->status < REQ_HEAD_NAME_PARSED)
	{
		target = find_closest(buffer, *len, ':', '\r');
		if (target)
		{
			if (*target == '\r')
			{
				header->status = REQ_HEAD_VALUE_PARSED;
			}
			else
			{
				header->status = REQ_HEAD_NAME_PARSED;
				*target++ = 0;
			}
			// strncpy(&header->name[used], buffer, maxlen);
			size_t copylen = (target - buffer);
			strncat_local(header->name, sizeof(header->name), buffer, copylen);
			(*len) -= copylen;
			buffer = target;
		}
		else
		{
			// target = strncpy(&header->name[used], buffer, maxlen);
			strncat_local(header->name, sizeof(header->name), buffer, *len);
			*len = 0;
			buffer = target;
		}
	}

	if (header->status < REQ_HEAD_VALUE_PARSED)
	{

		if (!*len)
			return;

		size_t offset = strlen(header->name) + 1;
		header->value = &header->name[offset];
		size_t used = strlen(header->value);
		size_t maxlen = sizeof(header->name) - offset;
		
		target = find_char(buffer, *len, '\r');
		size_t ncount = *len;
		if (target || (ncount > (maxlen - used)))
		{
			header->status = REQ_HEAD_VALUE_PARSED;
			if (target)
			{
				ncount = (target - buffer);
			}
			else
			{
				target = &buffer[ncount];
			}
		}
		else
		{
			target = &buffer[*len];
		}

		strncat_local(header->value, maxlen, buffer, ncount);
		(*len) -= ncount;
		buffer = target;
	}

	header->status = http_discard_line(header->status, REQ_HEAD_VALUE_PARSED, &buffer, len);
	*buf = buffer;
}

void http_request_ws_handshake(ws_handshake_t *wsh, request_header_t *header)
{
	if (!strncasecmp_local("upgrade", header->name, 7) && strcasestr_local(header->value, "websocket"))
	{
		wsh->hs_got_upgrade = true;
	}
	else if (!strncasecmp_local("connection", header->name, 9) && strcasestr_local(header->value, "upgrade"))
	{
		wsh->hs_got_connection = true;
	}
	else if (!strncasecmp_local("sec-websocket-key", header->name, 17))
	{
		strntrim_local(header->value);
		size_t l1 = strlen(header->value);
		size_t l2 = sizeof(wsh->hs_key);
		strncpy(wsh->hs_key, header->value, MAX(l1, l2));
		wsh->hs_got_key = true;
	}
	else if (!strncasecmp_local("sec-websocket-version", header->name, 21) && strcasestr_local(header->value, "13"))
	{
		wsh->hs_got_version = true;
	}
}

void http_request_file_upload(request_upload_t *upload, request_header_t *header)
{
	if (!strncasecmp_local("content-type", header->name, sizeof("content-type")))
	{
		char *b = (char *)strcasestr_local(header->value, "boundary=");
		memset(upload->boundary, 0, sizeof(upload->boundary));
		upload->boundary_len = 0;
		if (b)
		{
			b += 9;
			upload->boundary[0] = '-';
			upload->boundary[1] = '-';
			strncpy(&upload->boundary[2], b, sizeof(upload->boundary) - 2);
			char *b = find_char(upload->boundary, '"', strlen(upload->boundary));
			if (b)
			{
				*b = 0;
			}
			upload->boundary_len = strlen(upload->boundary);
			upload->status = REQ_UPLOAD_INIT;
		}
	}
	else if (!strncasecmp_local("content-length", header->name, sizeof("content-length")))
	{
		strntrim_local(header->value);
		upload->upload_len = atol(header->value);
	}
	else if (!strncasecmp_local("content-disposition", header->name, sizeof("content-disposition")))
	{
		char *b = strcasestr_local(header->value, "filename=\"");
		memset(upload->upload_name, 0, sizeof(upload->upload_name));
		if (b)
		{
			strncpy(upload->upload_name, b, sizeof(upload->upload_name));
			b = find_char(upload->upload_name, sizeof(upload->upload_name), '"');
			if (b)
			{
				*b = 0;
			}
		}
		else
		{
			strncpy(upload->upload_name, "upload.bin\0", sizeof(upload->upload_name));
		}
	}
}

void http_request_multipart_chunk(char **buf, size_t *len, request_upload_t *upload, request_header_t *header)
{
	char *target = NULL;

	size_t datalen = *len;

	if (upload->status < REQ_UPLOAD_INIT_FINISHED)
	{
		while (*len)
		{
			http_request_parse_header(header, buf, len);
			if (header->status == REQ_HEAD_FINISHED)
			{
				if (header->name[0] == 0)
				{
					upload->status = (upload->status == REQ_UPLOAD_FILENAME_FOUND) ? REQ_UPLOAD_INIT_FINISHED : REQ_UPLOAD_INIT;
					upload->upload_len -= (datalen - *len);
					// updates the expected file size
					// the boundary end marker is \r\n<boundary>--\r\n
					upload->upload_len -= (upload->boundary_len + 6);
					return;
				}

				switch (upload->status)
				{
				case REQ_UPLOAD_INIT:
					if (!strncmp(upload->boundary, header->name, upload->boundary_len))
					{
						// bound boundary
						upload->status = REQ_UPLOAD_BOUNDARY_FOUND;
					}
					break;
				case REQ_UPLOAD_BOUNDARY_FOUND:
					if (!strncasecmp_local("content-disposition", header->name, sizeof("content-disposition")))
					{
						target = strcasestr_local(header->value, "filename=\"");
						if (target)
						{
							target += 10;
							for (int i = 0; i < sizeof(upload->upload_name); i++)
							{
								upload->upload_name[i] = 0;
								if (!*target || *target == '"')
								{
									break;
								}
								upload->upload_name[i] = *target++;
							}

							// bound boundary
							upload->status = REQ_UPLOAD_FILENAME_FOUND;
						}
					}
					break;
				}
			}
		}
		upload->upload_len -= (datalen - *len);
	}
}
