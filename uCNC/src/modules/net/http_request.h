/*
	Name: http_request.h
	Description: Implements HTTP request parsing functions for µCNC.

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
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#define HTTP_REQ_ANY 0
#define HTTP_REQ_GET 1
#define HTTP_REQ_POST 2
#define HTTP_REQ_PUT 3
#define HTTP_REQ_DELETE 4
#define HTTP_REQ_OTHER 128

#ifndef FS_PATH_NAME_MAX_LEN
#define FS_PATH_NAME_MAX_LEN 256
#endif
#ifndef MAX_URL_LEN
#define MAX_URL_LEN FS_PATH_NAME_MAX_LEN
#endif
#ifndef MAX_URL_ARGS
#define MAX_URL_ARGS 10
#endif
#ifndef MAX_URL_ARG_LEN
#define MAX_URL_ARG_LEN 50
#endif


	/* Per-connection request context */
	typedef struct
	{
		uint8_t method;
		char uri[MAX_URL_LEN];
		size_t arg_count;
		char arg_name[MAX_URL_ARGS][MAX_URL_ARG_LEN];
		char arg_val[MAX_URL_ARGS][MAX_URL_ARG_LEN];
	} request_ctx_t;

	static void trim_line_end(char *s)
	{
		if (!s)
			return;
		size_t n = strlen(s);
		while (n > 0 && (s[n - 1] == '\r' || s[n - 1] == '\n'))
		{
			s[--n] = '\0';
		}
	}

	static void to_upper_str(char *s)
	{
		if (!s)
			return;
		for (; *s; ++s)
			*s = (char)toupper((unsigned char)*s);
	}

	static int hex_val(int c)
	{
		if (c >= '0' && c <= '9')
			return c - '0';
		c = tolower(c);
		if (c >= 'a' && c <= 'f')
			return 10 + (c - 'a');
		return -1;
	}

	static void url_decode(const char *src, char *dst, size_t dst_size, int plus_to_space)
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

	static uint8_t map_method(const char *m)
	{
		if (strcmp(m, "GET") == 0)
			return HTTP_REQ_GET;
		if (strcmp(m, "POST") == 0)
			return HTTP_REQ_POST;
		if (strcmp(m, "PUT") == 0)
			return HTTP_REQ_PUT;
		if (strcmp(m, "DELETE") == 0)
			return HTTP_REQ_DELETE;
		// Other verbs (HEAD, OPTIONS, PATCH, CONNECT, TRACE, etc.)
		return HTTP_REQ_OTHER;
	}

	static void reset_ctx(request_ctx_t *ctx)
	{
		ctx->method = HTTP_REQ_ANY;
		if (FS_PATH_NAME_MAX_LEN > 0)
			ctx->uri[0] = '\0';
		ctx->arg_count = 0;
		for (size_t i = 0; i < MAX_URL_ARGS; ++i)
		{
			if (MAX_URL_ARG_LEN > 0)
			{
				ctx->arg_name[i][0] = '\0';
				ctx->arg_val[i][0] = '\0';
			}
		}
	}

	static void set_uri(request_ctx_t *ctx, const char *path)
	{
		if (!path || !*path)
			path = "/";
		// Normalize: ensure it starts with '/'
		if (path[0] != '/')
		{
			// Special-case asterisk form "*"
			if (strcmp(path, "*") == 0)
			{
				// Treat as server-wide; normalize to "/"
				snprintf(ctx->uri, FS_PATH_NAME_MAX_LEN, "/");
				return;
			}
			snprintf(ctx->uri, FS_PATH_NAME_MAX_LEN, "/%s", path);
		}
		else
		{
			snprintf(ctx->uri, FS_PATH_NAME_MAX_LEN, "%s", path);
		}
		ctx->uri[FS_PATH_NAME_MAX_LEN - 1] = '\0';
		char *hash = strchr(ctx->uri, '#');
		if(hash){
			*hash = '\0';
		}
	}

	static void parse_query_into_ctx(request_ctx_t *ctx, const char *query)
	{
		if (!query || !*query) return;
		
		const char *p = query;
		while (*p && ctx->arg_count < MAX_URL_ARGS) {
			const char *amp = strchr(p, '&');
			const char *eq = strchr(p, '=');
			
			if (amp && eq && eq > amp) eq = NULL;
			
			size_t name_len = eq ? (size_t)(eq - p) : 
							 (amp ? (size_t)(amp - p) : strlen(p));
			
			if (name_len >= MAX_URL_ARG_LEN) name_len = MAX_URL_ARG_LEN - 1;
			
			// Decode name directly into ctx buffer
			memset(ctx->arg_name[ctx->arg_count], 0, sizeof(ctx->arg_name[ctx->arg_count]));
			url_decode(p, ctx->arg_name[ctx->arg_count], name_len, 1);
			
			// Decode value if present
			if (eq) {
				const char *val = eq + 1;
				size_t val_len = amp ? (size_t)(amp - val) : strlen(val);
				if (val_len >= MAX_URL_ARG_LEN) val_len = MAX_URL_ARG_LEN - 1;
				memset(ctx->arg_val[ctx->arg_count], 0, sizeof(ctx->arg_val[ctx->arg_count]));
				url_decode(val, ctx->arg_val[ctx->arg_count], val_len, 1);
			} else {
				ctx->arg_val[ctx->arg_count][0] = '\0';
			}
			
			ctx->arg_count++;
			
			if (!amp) break;
			p = amp + 1;
		}
	}

	static void split_path_and_query(const char *target, char *out_path, 
                               size_t out_path_size, const char **out_query)
{
    const char *qmark = strchr(target, '?');
    const char *hash = strchr(target, '#');
    
    // Get path length up to first ? or #
    size_t path_len = qmark ? (size_t)(qmark - target) : 
                     (hash ? (size_t)(hash - target) : strlen(target));
                     
    if (path_len >= out_path_size) {
        path_len = out_path_size - 1;
    }
    
    // Copy path
    memcpy(out_path, target, path_len);
    out_path[path_len] = '\0';
    
    // Set query pointer after ? if it exists, otherwise NULL
    *out_query = qmark ? (qmark + 1) : NULL;
    
    // If we have both query and hash, null-terminate query at hash
    if (*out_query && hash) {
        ((char*)*out_query)[hash - *out_query] = '\0';
    }
}

	static void parse_target_into_ctx(request_ctx_t *ctx, const char *method, const char *target)
	{
		// Determine target form and normalize to path + query
		// Cases:
		// 1) origin-form: starts with '/'
		// 2) absolute-form: scheme://authority/path?query
		// 3) authority-form: host:port (CONNECT)
		// 4) asterisk-form: "*"
		char pathbuf[FS_PATH_NAME_MAX_LEN];
		const char *query = NULL;

		if (target[0] == '/')
		{
			split_path_and_query(target, pathbuf, sizeof(pathbuf), &query);
			set_uri(ctx, pathbuf);
			parse_query_into_ctx(ctx, query);
			return;
		}

		if (strcmp(target, "*") == 0)
		{
			set_uri(ctx, "/");
			return;
		}

		const char *scheme_sep = strstr(target, "://");
		if (scheme_sep)
		{
			// absolute-form: skip scheme://authority
			const char *after_auth = strchr(scheme_sep + 3, '/');
			if (!after_auth)
			{
				set_uri(ctx, "/");
				return;
			}
			split_path_and_query(after_auth, pathbuf, sizeof(pathbuf), &query);
			set_uri(ctx, pathbuf);
			parse_query_into_ctx(ctx, query);
			return;
		}

		// Potential authority-form (usually with CONNECT): host[:port]
		int looks_authority = (strchr(target, '/') == NULL) && (strchr(target, ' ') == NULL);
		if (looks_authority || strcmp(method, "CONNECT") == 0)
		{
			// No path; normalize to "/" and ignore args
			set_uri(ctx, "/");
			return;
		}

		// Fallback: treat as path-ish token
		split_path_and_query(target, pathbuf, sizeof(pathbuf), &query);
		set_uri(ctx, pathbuf);
		parse_query_into_ctx(ctx, query);
	}

	static void parse_tokens(const char *line, char *method, size_t msz, char *target, size_t tsz, char *version, size_t vsz)
	{
		method[0] = target[0] = version[0] = '\0';
		const char *p = line;
		const char *token_start;

		// Skip leading spaces
		while (*p && isspace((unsigned char)*p)) p++;
		
		// Method
		token_start = p;
		while (*p && !isspace((unsigned char)*p)) p++;
		size_t len = (size_t)(p - token_start);
		if (len >= msz) len = msz - 1;
		memcpy(method, token_start, len);
		method[len] = '\0';
		
		// Skip spaces between tokens
		while (*p && isspace((unsigned char)*p)) p++;
		
		// Target
		token_start = p;
		while (*p && !isspace((unsigned char)*p)) p++;
		len = (size_t)(p - token_start);
		if (len >= tsz) len = tsz - 1;
		memcpy(target, token_start, len);
		target[len] = '\0';
		
		// Skip spaces between tokens
		while (*p && isspace((unsigned char)*p)) p++;
		
		// Version
		token_start = p;
		while (*p && !isspace((unsigned char)*p)) p++;
		len = (size_t)(p - token_start);
		if (len >= vsz) len = vsz - 1;
		memcpy(version, token_start, len);
		version[len] = '\0';
	}

	static void parse_request_line(request_ctx_t *ctx, char *line)
	{
		if (!ctx || !line) return;
		reset_ctx(ctx);

		// Find end of line to avoid processing CRLF
		char *end = line;
		while (*end && *end != '\r' && *end != '\n') end++;
		
		// Temporarily null-terminate at end of line
		char saved = *end;
		*end = '\0';

		// Parse tokens
		char method[16], target[MAX_URL_LEN];
		char version[16]; // HTTP version isn't used but kept for protocol validation
		parse_tokens(line, method, sizeof(method), target, sizeof(target), 
					version, sizeof(version));

		// Restore original line ending
		*end = saved;

		if (method[0] == '\0' || target[0] == '\0') {
			return; // Malformed
		}

		to_upper_str(method);
		ctx->method = map_method(method);

		// Parse target directly into ctx
		parse_target_into_ctx(ctx, method, target);

		// Default URI if none set
		if (ctx->uri[0] == '\0') {
			set_uri(ctx, "/");
		}
	}

#ifdef __cplusplus
}
#endif

#endif
