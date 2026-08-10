/*
	Name: base64.h
	Description: base64 encoding util for µCNC.

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

#ifndef BASE64_H
#define BASE64_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdint.h>

static const char b64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int base64_encode(const uint8_t* in, size_t inlen, char* out, size_t outcap) {
    size_t olen = 4 * ((inlen + 2) / 3);
    if (outcap < olen + 1) return -1;
    size_t i = 0, o = 0;
    while (i + 2 < inlen) {
        uint32_t n = ((uint32_t)in[i] << 16) | ((uint32_t)in[i+1] << 8) | in[i+2];
        out[o++] = b64_table[(n >> 18) & 63];
        out[o++] = b64_table[(n >> 12) & 63];
        out[o++] = b64_table[(n >> 6) & 63];
        out[o++] = b64_table[n & 63];
        i += 3;
    }
    if (i + 1 < inlen) {
        uint32_t n = ((uint32_t)in[i] << 16) | ((uint32_t)in[i+1] << 8);
        out[o++] = b64_table[(n >> 18) & 63];
        out[o++] = b64_table[(n >> 12) & 63];
        out[o++] = b64_table[(n >> 6) & 63];
        out[o++] = '=';
    } else if (i < inlen) {
        uint32_t n = ((uint32_t)in[i] << 16);
        out[o++] = b64_table[(n >> 18) & 63];
        out[o++] = b64_table[(n >> 12) & 63];
        out[o++] = '=';
        out[o++] = '=';
    }
    out[o] = '\0';
    return (int)o;
}

#ifdef __cplusplus
}
#endif

#endif