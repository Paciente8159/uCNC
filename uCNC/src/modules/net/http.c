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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "http.h"

/* Defaults and bounds (tuned for small RAM) */
#ifndef HTTP_PORT
#define HTTP_PORT 80
#endif

#define HTTP_MAX_HANDLERS     8
#define HTTP_MAX_HEADERS      8
#define HTTP_MAX_HEADER_NAME  32
#define HTTP_MAX_HEADER_VALUE 64
#define HTTP_HEADER_BUF_SIZE  512
#define HTTP_UPLOAD_BUF_SIZE  512
#define HTTP_MAX_BOUNDARY_LEN 70

typedef struct {
  char name[HTTP_MAX_HEADER_NAME];
  char value[HTTP_MAX_HEADER_VALUE];
} http_header_kv_t;

typedef struct {
  const char    *uri;
  uint8_t        method;          /* HTTP_REQ_* from http_request.h */
  http_delegate  request_handler; /* called when request ready */
  http_delegate  file_handler;    /* called on upload START/PART/END/ABORT */
} http_route_t;

/* Per-client state */
typedef struct {
  int             fd;
  request_ctx_t   req;

  /* Request parsing */
  bool            have_reqline;
  bool            have_headers;
  size_t          hlen;
  char            hbuf[HTTP_HEADER_BUF_SIZE];

  /* Upload state (multipart/form-data) */
  bool            upload_active;
  bool            upload_started;
  char            boundary[HTTP_MAX_BOUNDARY_LEN];
  size_t          boundary_len;
  char            upload_name[FS_PATH_NAME_MAX_LEN];

  /* Small chunk buffer surfaced to app */
  uint8_t         up_buf[HTTP_UPLOAD_BUF_SIZE];
  size_t          up_len;
  uint8_t         up_status;           /* HHTP_UPLOAD_* */

  /* Response bookkeeping */
  bool            headers_sent;
  bool            chunked_mode;
  size_t          hdr_count;
  http_header_kv_t hdrs[HTTP_MAX_HEADERS];

  /* Selected route for this request (if matched) */
  const http_route_t *route;
} http_client_t;

/* Socket interface (single listener) */
static socket_if_t *http_srv = NULL;

/* Client slots align with SOCKET_MAX_CLIENTS from socket.h */
static http_client_t clients[SOCKET_MAX_CLIENTS];

/* Routes */
static http_route_t routes[HTTP_MAX_HANDLERS];
static size_t route_count = 0;

/* “Current” client while executing a delegate */
static http_client_t *cur = NULL;

/* ---------------- utilities (local) ---------------- */

static void client_reset(http_client_t *c) {
  memset(&c->req, 0, sizeof(c->req));
  c->have_reqline   = false;
  c->have_headers   = false;
  c->hlen           = 0;
  c->upload_active  = false;
  c->upload_started = false;
  c->boundary[0]    = '\0';
  c->boundary_len   = 0;
  c->upload_name[0] = '\0';
  c->up_len         = 0;
  c->up_status      = HHTP_UPLOAD_ABORT; /* idle */
  c->headers_sent   = false;
  c->chunked_mode   = false;
  c->hdr_count      = 0;
  c->route          = NULL;
}

static http_client_t *get_client_slot(int fd) {
  for (int i = 0; i < SOCKET_MAX_CLIENTS; i++) {
    if (clients[i].fd == fd) return &clients[i];
  }
  for (int i = 0; i < SOCKET_MAX_CLIENTS; i++) {
    if (clients[i].fd < 0) {
      clients[i].fd = fd;
      client_reset(&clients[i]);
      return &clients[i];
    }
  }
  return NULL;
}

static void release_client(int fd) {
  for (int i = 0; i < SOCKET_MAX_CLIENTS; i++) {
    if (clients[i].fd == fd) {
      if (clients[i].upload_active) {
        /* Notify abort if we die mid-upload */
        clients[i].up_status = HHTP_UPLOAD_ABORT;
        if (clients[i].route && clients[i].route->file_handler) {
          cur = &clients[i];
          clients[i].route->file_handler();
          cur = NULL;
        }
      }
      clients[i].fd = -1;
      client_reset(&clients[i]);
      return;
    }
  }
}

/* naive memmem to avoid libc dep */
static char *memmem_local(const char *h, size_t hlen, const char *n, size_t nlen) {
  if (!h || !n || !nlen || hlen < nlen) return NULL;
  for (size_t i = 0; i + nlen <= hlen; i++) {
    if (h[i] == n[0] && memcmp(h + i, n, nlen) == 0) return (char *)(h + i);
  }
  return NULL;
}

static char *strcasestr_local(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;
    for (; *haystack; haystack++) {
        const char *h = haystack;
        const char *n = needle;
        while (*h && *n && tolower((unsigned char)*h) == tolower((unsigned char)*n)) {
            h++; n++;
        }
        if (!*n) return (char *)haystack;
    }
    return NULL;
}


static void extract_boundary_line(const char *headers, char *dst, size_t dstsz, size_t *out_len) {
  /* Look for boundary=xyz in Content-Type */
  const char *p = strcasestr_local(headers, "Content-Type:");
  *out_len = 0;
  if (!p) { dst[0] = '\0'; return; }
  const char *eol = strstr(p, "\r\n");
  if (!eol) eol = p + strlen(p);
  const char *b = strcasestr_local(p, "boundary=");
  if (!b || b > eol) { dst[0] = '\0'; return; }
  b += 9;
  /* build the actual delimiter used in body: starts with -- */
  snprintf(dst, dstsz, "--%.*s", (int)(eol - b), b);
  dst[dstsz - 1] = '\0';
  *out_len = strlen(dst);
}

static void extract_filename_from_part_headers(const char *part_hdrs, char *dst, size_t dstsz) {
  const char *p = strcasestr_local(part_hdrs, "filename=\"");
  if (p) {
    p += 10;
    const char *q = strchr(p, '"');
    size_t n = q ? (size_t)(q - p) : strlen(p);
    if (n >= dstsz) n = dstsz - 1;
    memcpy(dst, p, n);
    dst[n] = '\0';
  } else {
    snprintf(dst, dstsz, "upload.bin");
  }
}

/* route lookup: exact match on URI and method */
static const http_route_t *match_route(const char *uri, uint8_t method) {
  for (size_t i = 0; i < route_count; i++) {
    if ((routes[i].method == HTTP_REQ_ANY || routes[i].method == method) &&
        strcmp(routes[i].uri, uri) == 0) {
      return &routes[i];
    }
  }
  return NULL;
}

/* --------------- public routing API --------------- */

void http_add(const char *uri, uint8_t method, http_delegate request_handler, http_delegate file_handler) {
  if (route_count < HTTP_MAX_HANDLERS) {
    routes[route_count].uri = uri;
    routes[route_count].method = method;
    routes[route_count].request_handler = request_handler;
    routes[route_count].file_handler = file_handler;
    route_count++;
  }
}

/* --------------- request accessors ---------------- */

int http_request_hasargs(void) {
  return cur ? (cur->req.arg_count > 0) : 0;
}

void http_request_uri(char *uri, size_t maxlen) {
  if (!uri || maxlen == 0) return;
  if (cur) {
    strncpy(uri, cur->req.uri, maxlen - 1);
    uri[maxlen - 1] = '\0';
  } else {
    uri[0] = '\0';
  }
}

bool http_request_arg(const char *argname, char *argvalue, size_t maxlen) {
  if (!cur || !argname || !argvalue || maxlen == 0) return false;
  for (size_t i = 0; i < cur->req.arg_count; i++) {
    if (strcmp(cur->req.arg_name[i], argname) == 0) {
      strncpy(argvalue, cur->req.arg_val[i], maxlen - 1);
      argvalue[maxlen - 1] = '\0';
      return true;
    }
  }
  return false;
}

uint8_t http_request_method(void) {
  return cur ? cur->req.method : HTTP_REQ_OTHER;
}

/* --------------- response helpers ----------------- */

void http_send_header(const char *name, const char *data, bool first) {
  if (!cur || !name || !data) return;
  if (first) { cur->hdr_count = 0; cur->headers_sent = false; }
  /* append or add */
  for (size_t i = 0; i < cur->hdr_count; i++) {
    if (strcasecmp(cur->hdrs[i].name, name) == 0) {
      size_t l = strlen(cur->hdrs[i].value);
      if (l + 2 < HTTP_MAX_HEADER_VALUE) {
        strncat(cur->hdrs[i].value, ", ", HTTP_MAX_HEADER_VALUE - l - 1);
        strncat(cur->hdrs[i].value, data, HTTP_MAX_HEADER_VALUE - strlen(cur->hdrs[i].value) - 1);
      }
      return;
    }
  }
  if (cur->hdr_count < HTTP_MAX_HEADERS) {
    strncpy(cur->hdrs[cur->hdr_count].name, name, HTTP_MAX_HEADER_NAME - 1);
    cur->hdrs[cur->hdr_count].name[HTTP_MAX_HEADER_NAME - 1] = '\0';
    strncpy(cur->hdrs[cur->hdr_count].value, data, HTTP_MAX_HEADER_VALUE - 1);
    cur->hdrs[cur->hdr_count].value[HTTP_MAX_HEADER_VALUE - 1] = '\0';
    cur->hdr_count++;
  }
}

void http_send(int code, const char *content_type, const uint8_t *data, size_t data_len) {
  if (!cur || cur->fd < 0) return;

  char buf[128];

  /* Enable chunked mode when content_type == NULL (headers delayed) */
  if (content_type == NULL) {
    cur->chunked_mode = true;
    if (!cur->headers_sent) {
      http_send_header("Transfer-Encoding", "chunked", false);
    }
    if (!data && data_len == 0) {
      /* Just preparing; do not emit yet */
      return;
    }
  }

  if (!cur->headers_sent) {
    int n = snprintf(buf, sizeof(buf), "HTTP/1.1 %d OK\r\n", code);
    socket_send(http_srv, cur->fd, buf, (size_t)n, 0);
    if (content_type) {
      n = snprintf(buf, sizeof(buf), "Content-Type: %s\r\n", content_type);
      socket_send(http_srv, cur->fd, buf, (size_t)n, 0);
    }
    for (size_t i = 0; i < cur->hdr_count; i++) {
      n = snprintf(buf, sizeof(buf), "%s: %s\r\n", cur->hdrs[i].name, cur->hdrs[i].value);
      socket_send(http_srv, cur->fd, buf, (size_t)n, 0);
    }
    socket_send(http_srv, cur->fd, "\r\n", 2, 0);
    cur->headers_sent = true;
  }

  if (data && data_len > 0) {
    if (cur->chunked_mode) {
      int n = snprintf(buf, sizeof(buf), "%x\r\n", (unsigned int)data_len);
      socket_send(http_srv, cur->fd, buf, (size_t)n, 0);
      socket_send(http_srv, cur->fd, (char*)data, data_len, 0);
      socket_send(http_srv, cur->fd, "\r\n", 2, 0);
    } else {
      socket_send(http_srv, cur->fd, (char*)data, data_len, 0);
    }
  }

  /* Close if zero-length body marks completion */
  if (!data || data_len == 0) {
    if (cur->chunked_mode) {
      socket_send(http_srv, cur->fd, "0\r\n\r\n", 5, 0);
    }
    /* Server closes; simple keep-alive is omitted for MCU simplicity */
    bsd_close(cur->fd);
    cur->fd = -1;
    cur->headers_sent = false;
    cur->chunked_mode = false;
    cur = NULL;
  }
}

bool http_send_file(const char *file_path, const char *content_type) {
  if (!cur || cur->fd < 0 || !file_path) return false;

  uint8_t fbuf[512];
  size_t nread;
  fs_file_t *fp = fs_open(file_path, "rb");
  if (!fp) {
    http_send_str(404, "text/plain", "404 Not Found");
    http_send(404, "text/plain", NULL, 0);
    return false;
  }

  /* Chunked stream (size-free) */
  http_send(200, NULL, NULL, 0); /* prepare chunked */
  while ((nread = fs_read(fp, fbuf, sizeof(fbuf))) > 0) {
    http_send(200, content_type ? content_type : "application/octet-stream", fbuf, nread);
  }
  fs_close(fp);
  http_send(200, content_type ? content_type : "application/octet-stream", NULL, 0);
  return true;
}

/* --------------- upload accessors ----------------- */

static http_upload_t up_view;

http_upload_t http_file_upload_status(void) {
  if (!cur) {
    http_upload_t z; memset(&z, 0, sizeof(z)); return z;
  }
  up_view.status  = cur->up_status;
  strncpy(up_view.filename, cur->upload_name, sizeof(up_view.filename) - 1);
  up_view.filename[sizeof(up_view.filename) - 1] = '\0';
  up_view.data    = cur->up_len ? cur->up_buf : NULL;
  up_view.datalen = cur->up_len;
  return up_view;
}

void http_file_upload_name(char *filename, size_t maxlen) {
  if (!filename || maxlen == 0) return;
  if (!cur) { filename[0] = '\0'; return; }
  strncpy(filename, cur->upload_name, maxlen - 1);
  filename[maxlen - 1] = '\0';
}

char *http_file_upload_buffer(size_t *len) {
  if (!cur) { if (len) *len = 0; return NULL; }
  if (len) *len = cur->up_len;
  return (char*)cur->up_buf;
}

/* --------------- request dispatch ----------------- */

static void dispatch_request(http_client_t *c) {
  c->route = match_route(c->req.uri, c->req.method);
  if (!c->route) {
    /* Minimal 404 */
    cur = c;
    http_send_str(404, "text/plain", "404 Not Found");
    http_send(404, "text/plain", NULL, 0);
    cur = NULL;
    return;
  }
  /* Set current and call user's request handler */
  cur = c;
  if (c->route->request_handler) c->route->request_handler();
  cur = NULL;
}

/* --------------- upload handling ------------------ */

static void maybe_invoke_file_handler(http_client_t *c) {
  if (!c->route || !c->route->file_handler) return;
  cur = c;
  c->route->file_handler();
  cur = NULL;
}

/* Process body bytes; supports multipart/form-data streaming */
static void handle_upload_bytes(http_client_t *c, const char *buf, size_t len) {
  if (!c->upload_active || c->boundary_len == 0) return;

  const char *p = buf;
  size_t rem = len;

  /* On first body data after headers: expect boundary line and part headers */
  if (!c->upload_started) {
    /* Ensure we have the starting boundary */
    char *b = memmem_local(p, rem, c->boundary, c->boundary_len);
    if (!b) return; /* wait for more */
    p = b + c->boundary_len;
    rem = len - (size_t)(p - buf);

    /* After boundary, optional CRLF and part headers until CRLFCRLF */
    char *part_hdr_end = memmem_local(p, rem, "\r\n\r\n", 4);
    if (!part_hdr_end) {
      /* Not all part headers arrived yet; accumulate is omitted to save RAM.
         Wait for next call where we get them contiguous. */
      return;
    }
    /* Extract filename */
    char tmp[256];
    size_t phlen = (size_t)(part_hdr_end - p);
    size_t copy = phlen < sizeof(tmp) - 1 ? phlen : sizeof(tmp) - 1;
    memcpy(tmp, p, copy); tmp[copy] = '\0';
    extract_filename_from_part_headers(tmp, c->upload_name, sizeof(c->upload_name));

    /* File data starts after CRLFCRLF */
    p = part_hdr_end + 4;
    rem = len - (size_t)(p - buf);

    c->upload_started = true;
    c->up_status = HHTP_UPLOAD_START;
    c->up_len = 0;
    maybe_invoke_file_handler(c);
  }

  /* Stream until boundary or end */
  if (rem == 0) return;

  /* Look for end boundary in current chunk */
  char *bpos = memmem_local(p, rem, c->boundary, c->boundary_len);
  size_t to_copy;
  bool at_end = false;

  if (bpos) {
    /* Data ends right before "\r\n--boundary" (there is usually \r\n before boundary) */
    const char *endmark = bpos - 2; /* try to skip preceding CRLF */
    if (endmark >= p) {
      to_copy = (size_t)(endmark - p);
    } else {
      to_copy = 0;
    }
    at_end = true;
  } else {
    to_copy = rem;
  }

  /* Copy small slice to up_buf (bounded) and notify */
  size_t chunk = (to_copy > HTTP_UPLOAD_BUF_SIZE) ? HTTP_UPLOAD_BUF_SIZE : to_copy;
  if (chunk) {
    memcpy(c->up_buf, p, chunk);
    c->up_len = chunk;
    c->up_status = c->upload_started ? HHTP_UPLOAD_PART : HHTP_UPLOAD_START;
    maybe_invoke_file_handler(c);
  } else {
    c->up_len = 0;
  }

  if (at_end) {
    /* Final notification with zero data to mark end consistently */
    c->up_len = 0;
    c->up_status = HHTP_UPLOAD_END;
    maybe_invoke_file_handler(c);
    c->upload_active = false;
  }
}

/* --------------- socket callbacks ----------------- */

static void http_on_connected(int client_fd) {
  /* assign slot */
  (void)get_client_slot(client_fd);
}

static void http_on_disconnected(int client_fd) {
  release_client(client_fd);
}

static void http_on_data(int client_fd, void *data, size_t data_len) {
  http_client_t *c = get_client_slot(client_fd);
  if (!c || data_len == 0) return;

  char *bytes = (char *)data;
  size_t off = 0;

  /* 1) Request line */
  if (!c->have_reqline) {
    char *eol = memmem_local(bytes, data_len, "\r\n", 2);
    if (!eol) {
      /* Request line too long or split; drop for simplicity to save RAM */
      bsd_close(client_fd);
      release_client(client_fd);
      return;
    }
    char saved = *eol; *eol = '\0';
    parse_request_line(&c->req, bytes);
    *eol = saved;
    c->have_reqline = true;
    off = (size_t)(eol - bytes) + 2;
  }

  /* 2) Headers */
  if (!c->have_headers) {
    size_t to_add = (data_len - off);
    if (to_add > 0) {
      size_t can = (HTTP_HEADER_BUF_SIZE - c->hlen);
      if (to_add > can) to_add = can;
      memcpy(c->hbuf + c->hlen, bytes + off, to_add);
      c->hlen += to_add;
      off += to_add;
    }
    /* header end? */
    char *hend = memmem_local(c->hbuf, c->hlen, "\r\n\r\n", 4);
    if (!hend) {
      /* need more header bytes */
      return;
    }
    c->have_headers = true;

    /* Extract boundary if multipart */
    extract_boundary_line(c->hbuf, c->boundary, sizeof(c->boundary), &c->boundary_len);
    c->upload_active = (c->req.method == HTTP_REQ_POST || c->req.method == HTTP_REQ_PUT) && (c->boundary_len > 0);

    /* Body starts after CRLFCRLF within accumulated header buf? */
    size_t header_total = (size_t)((hend - c->hbuf) + 4);

    /* If we already buffered extra (unlikely since we cap), process that surplus first */
    if (c->hlen > header_total) {
      size_t surplus = c->hlen - header_total;
      handle_upload_bytes(c, c->hbuf + header_total, surplus);
    }

    /* Dispatch immediately for non-upload requests */
    if (!c->upload_active) {
      dispatch_request(c);
    }
  }

  /* 3) Body bytes in this packet */
  if (c->have_headers) {
    handle_upload_bytes(c, bytes + off, data_len - off);
  }
}

/* --------------- module + run loop ---------------- */

DECL_MODULE(http_server) {
  /* init slots */
  for (int i = 0; i < SOCKET_MAX_CLIENTS; i++) {
    clients[i].fd = -1;
    client_reset(&clients[i]);
  }
  http_srv = socket_start(IP_ANY, HTTP_PORT, 2 /*AF_INET*/, 1 /*SOCK_STREAM*/, 0);
  if (!http_srv) return;
  socket_add_ondata_handler(http_srv, http_on_data);
  socket_add_onconnected_handler(http_srv, http_on_connected);
  socket_add_ondisconnected_handler(http_srv, http_on_disconnected);
}

void http_server_run(void) {
  socket_server_run(http_srv);
}
