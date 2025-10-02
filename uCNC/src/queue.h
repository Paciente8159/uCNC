/*
	Name: queue.h
	Description: Some useful queue (circular buffers) functions and macros.

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

#ifndef QUEUE_H
#define QUEUE_H

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

	typedef struct ring_buffer_
	{
		volatile uint8_t count;
		volatile uint8_t head;
		volatile uint8_t tail;
		uint8_t *data;
		const uint8_t size;
		const uint8_t elem_size;
	} ring_buffer_t;

#ifndef USE_MACRO_BUFFER
#ifndef USE_CUSTOM_BUFFER_IMPLEMENTATION
#define DECL_BUFFER(type, name, size) \
	DECL_MUTEX(name)                    \
	static type name##_queue[size];     \
	ring_buffer_t name = {0, 0, 0, name##_queue, size, sizeof(type)}

	uint8_t queue_write_available(ring_buffer_t *buffer);
	uint8_t queue_read_available(ring_buffer_t *buffer);
	bool queue_empty(ring_buffer_t *buffer);
	bool queue_full(ring_buffer_t *buffer);
	void queue_peek(ring_buffer_t *buffer, void *ptr);
	void queue_dequeue(ring_buffer_t *buffer, void *ptr);
	void queue_enqueue(ring_buffer_t *buffer, void *ptr);
	void queue_write(ring_buffer_t *buffer, void *ptr, uint8_t len, uint8_t *written);
	void queue_read(ring_buffer_t *buffer, void *ptr, uint8_t len, uint8_t *read);
	void queue_clear(ring_buffer_t *buffer);

#define BUFFER_INIT(type, buffer, size)
#define BUFFER_WRITE_AVAILABLE(buffer) queue_write_available(&buffer)
#define BUFFER_READ_AVAILABLE(buffer) queue_read_available(&buffer)
#define BUFFER_EMPTY(buffer) queue_empty(&buffer)
#define BUFFER_FULL(buffer) queue_full(&buffer)
#define BUFFER_PEEK(buffer, ptr) queue_peek(&buffer, ptr)
#define BUFFER_DEQUEUE(buffer, ptr) ({MUTEX_INIT(buffer); MUTEX_TAKE(buffer){queue_dequeue(&buffer, ptr);} })
#define BUFFER_ENQUEUE(buffer, ptr) ({MUTEX_INIT(buffer); MUTEX_TAKE(buffer){queue_enqueue(&buffer, ptr);} })
#define BUFFER_WRITE(buffer, ptr, len, written) ({MUTEX_INIT(buffer); MUTEX_TAKE(buffer){queue_write(&buffer, ptr, len, &written);} })
#define BUFFER_READ(buffer, ptr, len, read) ({MUTEX_INIT(buffer); MUTEX_TAKE(buffer){queue_read(&buffer, ptr, len, &read);} })
#define BUFFER_CLEAR(buffer) ({MUTEX_INIT(buffer); MUTEX_TAKE(buffer){queue_clear(&buffer);} })
#endif
#else

#define DECL_QUEUE(type, name, size) \
	static type name##_queue[size];    \
	enum                               \
	{                                  \
		name##_size = size;              \
	}                                  \
	ring_buffer_t name

#define QUEUE_INIT(type, buffer, size)
#define QUEUE_WRITE_AVAILABLE(buffer) (buffer##_size - buffer.count)
#define QUEUE_READ_AVAILABLE(buffer) (buffer.count)
#define QUEUE_EMPTY(buffer) (!buffer.count)
#define QUEUE_FULL(buffer) (buffer.count == buffer##_size)
#define QUEUE_PEEK(buffer) (buffer##_queue[buffer.tail])

#define QUEUE_DEQUEUE(buffer, ptr)                                  \
	{                                                                  \
		if (!BUFFER_EMPTY(buffer))                                       \
		{                                                                \
			uint8_t tail;                                                  \
			__ATOMIC__                                                     \
			{                                                              \
				tail = (buffer)tail;                                          \
			}                                                              \
			memcpy(ptr, &buffer##_queue[tail], sizeof(buffer##_queue[0])); \
			tail++;                                                        \
			if (tail >= buffer##_size)                                     \
			{                                                              \
				tail = 0;                                                    \
			}                                                              \
			__ATOMIC__                                                     \
			{                                                              \
				buffer.tail = tail;                                          \
				buffer.count--;                                              \
			}                                                              \
		}                                                                \
	}

#define BUFFER_ENQUEUE(buffer, ptr)                                  \
	{                                                                  \
		if (!BUFFER_FULL(buffer))                                        \
		{                                                                \
			uint8_t head;                                                  \
			__ATOMIC__                                                     \
			{                                                              \
				head = buffer.head;                                          \
			}                                                              \
			memcpy(&buffer##_queue[head], ptr, sizeof(buffer##_queue[0])); \
			head++;                                                        \
			if (head >= buffer##_size)                                     \
			{                                                              \
				head = 0;                                                    \
			}                                                              \
			__ATOMIC__                                                     \
			{                                                              \
				buffer.head = head;                                          \
				buffer.count++;                                              \
			}                                                              \
		}                                                                \
	}

#define BUFFER_WRITE(buffer, ptr, len, written) ({                                   \
	uint8_t count, head;                                                               \
	__ATOMIC__                                                                         \
	{                                                                                  \
		head = buffer.head;                                                              \
		count = buffer.count;                                                            \
	}                                                                                  \
	count = MIN(buffer##_size - count, len);                                           \
	written = 0;                                                                       \
	if (count)                                                                         \
	{                                                                                  \
		uint8_t avail = (buffer##_size - head);                                          \
		if (avail < count && avail)                                                      \
		{                                                                                \
			memcpy(&buffer##_queue[head], ptr, avail * sizeof(buffer##_queue[0]));         \
			written = avail;                                                               \
			count -= avail;                                                                \
			head = 0;                                                                      \
		}                                                                                \
		else                                                                             \
		{                                                                                \
			avail = 0;                                                                     \
		}                                                                                \
		if (count)                                                                       \
		{                                                                                \
			memcpy(&buffer##_queue[head], &ptr[avail], count * sizeof(buffer##_queue[0])); \
			written += count;                                                              \
			__ATOMIC__                                                                     \
			{                                                                              \
				head += count;                                                               \
				if (head == buffer##_size)                                                   \
				{                                                                            \
					head = 0;                                                                  \
				}                                                                            \
				buffer.head = head;                                                          \
				buffer.count += written;                                                     \
			}                                                                              \
		}                                                                                \
	}                                                                                  \
})

#define BUFFER_READ(buffer, ptr, len, read) ({                                       \
	uint8_t count, tail;                                                               \
	__ATOMIC__                                                                         \
	{                                                                                  \
		tail = buffer.tail;                                                              \
		count = buffer.count;                                                            \
	}                                                                                  \
	if (count > len)                                                                   \
	{                                                                                  \
		count = len;                                                                     \
	}                                                                                  \
	read = 0;                                                                          \
	if (count)                                                                         \
	{                                                                                  \
		uint8_t avail = buffer##_size - tail;                                            \
		if (avail < count && avail)                                                      \
		{                                                                                \
			memcpy(ptr, &buffer##_queue[tail], avail * sizeof(buffer##_queue[0]));         \
			read = avail;                                                                  \
			count -= avail;                                                                \
			tail = 0;                                                                      \
		}                                                                                \
		else                                                                             \
		{                                                                                \
			avail = 0;                                                                     \
		}                                                                                \
		if (count)                                                                       \
		{                                                                                \
			memcpy(&ptr[avail], &buffer##_queue[tail], count * sizeof(buffer##_queue[0])); \
			read += count;                                                                 \
			__ATOMIC__                                                                     \
			{                                                                              \
				tail += count;                                                               \
				if (tail == buffer##_size)                                                   \
				{                                                                            \
					tail = 0;                                                                  \
				}                                                                            \
				buffer.tail = tail;                                                          \
				buffer.count -= read;                                                        \
			}                                                                              \
		}                                                                                \
	}                                                                                  \
})

#define BUFFER_CLEAR(buffer)     \
	{                              \
			__ATOMIC__{                \
					buffer##_queue[0] = 0; \
	buffer.tail = 0;               \
	buffer.head = 0;               \
	buffer.count = 0;              \
	}                              \
	}
#endif

#ifdef __cplusplus
}
#endif
#endif