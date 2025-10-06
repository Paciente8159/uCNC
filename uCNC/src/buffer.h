/*
	Name: buffer.h
	Description: Some useful circular buffers functions and macros.
	These are small (255 slots max) sized buffer optimized for speed.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 02/10/2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifndef BUFFER_H
#define BUFFER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

	/**
	 * RING BUFFER UTILS
	 * **/

#ifndef buffer_index_t
#define buffer_index_t uint8_t
#ifndef __GNUC__
#define buf_index_byteoffset 3 /*defined by log2(8 * sizeof(buffer_index_t))*/
#endif
#endif

#ifdef __GNUC__
#define buf_index_bits (8u * sizeof(buffer_index_t))
#define buf_index_bitoffset (buf_index_bits - 1u)
#define buf_index_byteoffset (__builtin_ctz(buf_index_bits))
#else
#if sizeof(buffer_index_t) == 1
#define buf_index_byteoffset 3 /* log2(8)  */
#elif sizeof(buffer_index_t) == 2
#define buf_index_byteoffset 4 /* log2(16) */
#elif sizeof(buffer_index_t) == 4
#define buf_index_byteoffset 5 /* log2(32) */
#elif sizeof(buffer_index_t) == 8
#define buf_index_byteoffset 6 /* log2(64) */
#endif
#endif

#ifndef buf_index_byteoffset
#error "You need to manually define buf_index_byteoffset. This should be a value equal to log2(8 * sizeof(buffer_index_t))"
#endif

	typedef struct ring_buffer_
	{
		volatile buffer_index_t head;
		volatile buffer_index_t tail;
		buffer_index_t *flags;
		uint8_t *data;
		const uint8_t size;
		const uint8_t elem_size;
	} ring_buffer_t;

#ifndef USE_MACRO_BUFFER
	uint8_t buffer_write_available(ring_buffer_t *buffer);
	uint8_t buffer_read_available(ring_buffer_t *buffer);
	bool buffer_empty(ring_buffer_t *buffer);
	bool buffer_full(ring_buffer_t *buffer);
	void buffer_peek(ring_buffer_t *buffer, void *ptr);
	void buffer_dequeue(ring_buffer_t *buffer, void *ptr);
	void buffer_enqueue(ring_buffer_t *buffer, void *ptr);
	void buffer_write(ring_buffer_t *buffer, void *ptr, uint8_t len, uint8_t *written);
	void buffer_read(ring_buffer_t *buffer, void *ptr, uint8_t len, uint8_t *read);
	void buffer_clear(ring_buffer_t *buffer);

#ifndef USE_CUSTOM_BUFFER_IMPLEMENTATION
#define DECL_BUFFER(type, name, size)                                                               \
	static type name##_bufferdata[size];                                                              \
	static buffer_index_t name##_bufferflags[((size + buf_index_bitoffset) >> buf_index_byteoffset)]; \
	ring_buffer_t name = {0, 0, name##_bufferflags, name##_bufferdata, size, sizeof(type)}

#define BUFFER_INIT(type, buffer, size)
#define BUFFER_WRITE_AVAILABLE(buffer) buffer_write_available(&buffer)
#define BUFFER_READ_AVAILABLE(buffer) buffer_read_available(&buffer)
#define BUFFER_EMPTY(buffer) buffer_empty(&buffer)
#define BUFFER_FULL(buffer) buffer_full(&buffer)
#define BUFFER_PEEK(buffer, ptr) buffer_peek(&buffer, ptr)
#define BUFFER_DEQUEUE(buffer, ptr) buffer_dequeue(&buffer, ptr)
#define BUFFER_ENQUEUE(buffer, ptr) buffer_enqueue(&buffer, ptr)
#define BUFFER_WRITE(buffer, ptr, len, written) buffer_write(&buffer, ptr, len, &written)
#define BUFFER_READ(buffer, ptr, len, read) buffer_read(&buffer, ptr, len, &read)
#define BUFFER_CLEAR(buffer) buffer_clear(&buffer);
#endif
#else
#define DECL_BUFFER(type, name, size)  \
	static type name##_bufferdata[size]; \
	enum                                 \
	{                                    \
		name##_size = size                 \
	};                                   \
	struct                               \
	{                                    \
		volatile uint8_t head;      \
		volatile uint8_t tail;      \
		volatile uint8_t count;     \
	} name

#define BUFFER_INIT(type, buffer, size)
#define BUFFER_WRITE_AVAILABLE(buffer) (buffer##_size - buffer.count)
#define BUFFER_READ_AVAILABLE(buffer) (buffer.count)
#define BUFFER_EMPTY(buffer) (!buffer.count)
#define BUFFER_FULL(buffer) (buffer.count == buffer##_size)
#define BUFFER_PEEK(buffer) (buffer##_bufferdata[buffer.tail])

#define BUFFER_DEQUEUE(buffer, ptr)                                            \
	{                                                                            \
		if (!BUFFER_EMPTY(buffer))                                                 \
		{                                                                          \
			uint8_t tail;                                                            \
			ATOMIC_CODEBLOCK                                                         \
			{                                                                        \
				tail = buffer.tail;                                                    \
			}                                                                        \
			memcpy(ptr, &buffer##_bufferdata[tail], sizeof(buffer##_bufferdata[0])); \
			tail++;                                                                  \
			if (tail >= buffer##_size)                                               \
			{                                                                        \
				tail = 0;                                                              \
			}                                                                        \
			ATOMIC_CODEBLOCK                                                         \
			{                                                                        \
				buffer.tail = tail;                                                    \
				buffer.count--;                                                        \
			}                                                                        \
		}                                                                          \
	}

#define BUFFER_ENQUEUE(buffer, ptr)                                            \
	{                                                                            \
		if (!BUFFER_FULL(buffer))                                                  \
		{                                                                          \
			uint8_t head;                                                            \
			ATOMIC_CODEBLOCK                                                         \
			{                                                                        \
				head = buffer.head;                                                    \
			}                                                                        \
			memcpy(&buffer##_bufferdata[head], ptr, sizeof(buffer##_bufferdata[0])); \
			head++;                                                                  \
			if (head >= buffer##_size)                                               \
			{                                                                        \
				head = 0;                                                              \
			}                                                                        \
			ATOMIC_CODEBLOCK                                                         \
			{                                                                        \
				buffer.head = head;                                                    \
				buffer.count++;                                                        \
			}                                                                        \
		}                                                                          \
	}

#define BUFFER_WRITE(buffer, ptr, len, written) ({                                             \
	uint8_t count, head;                                                                         \
	ATOMIC_CODEBLOCK                                                                             \
	{                                                                                            \
		head = buffer.head;                                                                        \
		count = buffer.count;                                                                      \
	}                                                                                            \
	count = MIN(buffer##_size - count, len);                                                     \
	(written) = 0;                                                                                 \
	if (count)                                                                                   \
	{                                                                                            \
		uint8_t avail = (buffer##_size - head);                                                    \
		if (avail < count && avail)                                                                \
		{                                                                                          \
			memcpy(&buffer##_bufferdata[head], ptr, avail * sizeof(buffer##_bufferdata[0]));         \
			(written) = avail;                                                                         \
			count -= avail;                                                                          \
			head = 0;                                                                                \
		}                                                                                          \
		else                                                                                       \
		{                                                                                          \
			avail = 0;                                                                               \
		}                                                                                          \
		if (count)                                                                                 \
		{                                                                                          \
			memcpy(&buffer##_bufferdata[head], &ptr[avail], count * sizeof(buffer##_bufferdata[0])); \
			(written) += count;                                                                        \
			ATOMIC_CODEBLOCK                                                                         \
			{                                                                                        \
				head += count;                                                                         \
				if (head == buffer##_size)                                                             \
				{                                                                                      \
					head = 0;                                                                            \
				}                                                                                      \
				buffer.head = head;                                                                    \
				buffer.count += (written);                                                               \
			}                                                                                        \
		}                                                                                          \
	}                                                                                            \
})

#define BUFFER_READ(buffer, ptr, len, read) ({                                                 \
	uint8_t count, tail;                                                                         \
	ATOMIC_CODEBLOCK                                                                             \
	{                                                                                            \
		tail = buffer.tail;                                                                        \
		count = buffer.count;                                                                      \
	}                                                                                            \
	if (count > len)                                                                             \
	{                                                                                            \
		count = len;                                                                               \
	}                                                                                            \
	(read) = 0;                                                                                    \
	if (count)                                                                                   \
	{                                                                                            \
		uint8_t avail = buffer##_size - tail;                                                      \
		if (avail < count && avail)                                                                \
		{                                                                                          \
			memcpy(ptr, &buffer##_bufferdata[tail], avail * sizeof(buffer##_bufferdata[0]));         \
			(read) = avail;                                                                            \
			count -= avail;                                                                          \
			tail = 0;                                                                                \
		}                                                                                          \
		else                                                                                       \
		{                                                                                          \
			avail = 0;                                                                               \
		}                                                                                          \
		if (count)                                                                                 \
		{                                                                                          \
			memcpy(&ptr[avail], &buffer##_bufferdata[tail], count * sizeof(buffer##_bufferdata[0])); \
			(read) += count;                                                                           \
			ATOMIC_CODEBLOCK                                                                         \
			{                                                                                        \
				tail += count;                                                                         \
				if (tail == buffer##_size)                                                             \
				{                                                                                      \
					tail = 0;                                                                            \
				}                                                                                      \
				buffer.tail = tail;                                                                    \
				buffer.count -= (read);                                                                  \
			}                                                                                        \
		}                                                                                          \
	}                                                                                            \
})

#define BUFFER_CLEAR(buffer)          \
	{                                   \
			ATOMIC_CODEBLOCK{               \
					buffer##_bufferdata[0] = 0; \
	buffer.tail = 0;                    \
	buffer.head = 0;                    \
	buffer.count = 0;                   \
	}                                   \
	}

#endif

#ifdef __cplusplus
}
#endif
#endif