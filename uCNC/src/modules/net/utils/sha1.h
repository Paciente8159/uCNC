/*
	Name: sha1.h
	Description: Implements SHA1 encryption utilities for µCNC.

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

#ifndef SHA1_H
#define SHA1_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <stdint.h>
#include <string.h>

	typedef struct
	{
		uint32_t state[5];
		uint64_t bitcount;
		uint8_t buffer[64];
	} sha1_ctx;

	static uint32_t rol32(uint32_t v, uint32_t n) { return (v << n) | (v >> (32U - n)); }

	static void sha1_transform(uint32_t state[5], const uint8_t buffer[64])
	{
		uint32_t a, b, c, d, e, t, w[80];
		for (int i = 0; i < 16; ++i)
		{
			w[i] = ((uint32_t)buffer[4 * i] << 24) | ((uint32_t)buffer[4 * i + 1] << 16) |
						 ((uint32_t)buffer[4 * i + 2] << 8) | (uint32_t)buffer[4 * i + 3];
		}
		for (int i = 16; i < 80; ++i)
		{
			w[i] = rol32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
		}
		a = state[0];
		b = state[1];
		c = state[2];
		d = state[3];
		e = state[4];
		for (int i = 0; i < 80; ++i)
		{
			uint32_t f, k;
			if (i < 20)
			{
				f = (b & c) | ((~b) & d);
				k = 0x5A827999;
			}
			else if (i < 40)
			{
				f = b ^ c ^ d;
				k = 0x6ED9EBA1;
			}
			else if (i < 60)
			{
				f = (b & c) | (b & d) | (c & d);
				k = 0x8F1BBCDC;
			}
			else
			{
				f = b ^ c ^ d;
				k = 0xCA62C1D6;
			}
			t = rol32(a, 5) + f + e + k + w[i];
			e = d;
			d = c;
			c = rol32(b, 30);
			b = a;
			a = t;
		}
		state[0] += a;
		state[1] += b;
		state[2] += c;
		state[3] += d;
		state[4] += e;
	}

	static void sha1_init(sha1_ctx *ctx)
	{
		ctx->state[0] = 0x67452301UL;
		ctx->state[1] = 0xEFCDAB89UL;
		ctx->state[2] = 0x98BADCFEUL;
		ctx->state[3] = 0x10325476UL;
		ctx->state[4] = 0xC3D2E1F0UL;
		ctx->bitcount = 0;
	}

	static void sha1_update(sha1_ctx *ctx, const uint8_t *data, size_t len)
	{
		size_t filled = (size_t)((ctx->bitcount >> 3) & 0x3F);
		ctx->bitcount += ((uint64_t)len) << 3;
		size_t i = 0;

		if (filled)
		{
			size_t to_copy = 64 - filled;
			if (to_copy > len)
				to_copy = len;
			memcpy(ctx->buffer + filled, data, to_copy);
			filled += to_copy;
			i += to_copy;
			if (filled == 64)
			{
				sha1_transform(ctx->state, ctx->buffer);
				filled = 0;
			}
		}

		for (; i + 63 < len; i += 64)
		{
			sha1_transform(ctx->state, data + i);
		}

		if (i < len)
		{
			memcpy(ctx->buffer + filled, data + i, len - i);
		}
	}

	static void sha1_final(sha1_ctx *ctx, uint8_t out[20])
	{
		uint8_t pad = 0x80;
		uint8_t zero = 0x00;
		uint8_t len_bytes[8];
		for (int i = 0; i < 8; ++i)
		{
			len_bytes[7 - i] = (uint8_t)((ctx->bitcount >> (i * 8)) & 0xFF);
		}
		sha1_update(ctx, &pad, 1);
		while (((ctx->bitcount >> 3) & 0x3F) != 56)
			sha1_update(ctx, &zero, 1);
		sha1_update(ctx, len_bytes, 8);
		for (int i = 0; i < 5; ++i)
		{
			out[4 * i + 0] = (uint8_t)((ctx->state[i] >> 24) & 0xFF);
			out[4 * i + 1] = (uint8_t)((ctx->state[i] >> 16) & 0xFF);
			out[4 * i + 2] = (uint8_t)((ctx->state[i] >> 8) & 0xFF);
			out[4 * i + 3] = (uint8_t)((ctx->state[i] >> 0) & 0xFF);
		}
	}

#ifdef __cplusplus
}
#endif

#endif
