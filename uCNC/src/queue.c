/*
	Name: util.c
	Description: Some useful constants and macros.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 29/11/2023

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "cnc.h"

uint8_t queue_write_available(ring_buffer_t *buffer)
{
	return (buffer->size - buffer->count);
}

uint8_t queue_read_available(ring_buffer_t *buffer)
{
	return buffer->count;
}

bool queue_empty(ring_buffer_t *buffer)
{
	return buffer->count;
}

bool queue_full(ring_buffer_t *buffer)
{
	return (buffer->size == buffer->count);
}

void queue_peek(ring_buffer_t *buffer, void *ptr)
{
	if (!buffer_empty(buffer))
	{
		memcpy(ptr, &buffer->data[buffer->tail * buffer->elem_size], buffer->elem_size);
	}
	else
	{
		memset(ptr, 0, buffer->elem_size);
	}
}

void queue_dequeue(ring_buffer_t *buffer, void *ptr)
{
	if (!buffer_empty(buffer))
	{
		uint8_t tail = 0;
		BUFFER_ENTER_CRITICAL(buffer);
		tail = buffer->tail;
		BUFFER_EXIT_CRITICAL(buffer);
		memcpy(ptr, &buffer->data[tail * buffer->elem_size], buffer->elem_size);
		tail++;
		if (tail >= buffer->size)
		{
			tail = 0;
		}
		BUFFER_ENTER_CRITICAL(buffer);
		buffer->tail = tail;
		buffer->count--;
		BUFFER_EXIT_CRITICAL(buffer);
	}
}

void queue_enqueue(ring_buffer_t *buffer, void *ptr)
{
	if (!buffer_full(buffer))
	{
		uint8_t head = 0;
		BUFFER_ENTER_CRITICAL(buffer);
		head = buffer->head;
		BUFFER_EXIT_CRITICAL(buffer);
		memcpy(&buffer->data[head * buffer->elem_size], ptr, buffer->elem_size);
		head++;
		if (head >= buffer->size)
		{
			head = 0;
		}
		BUFFER_ENTER_CRITICAL(buffer);
		buffer->head = head;
		buffer->count++;
		BUFFER_EXIT_CRITICAL(buffer);
	}
}

void queue_write(ring_buffer_t *buffer, void *ptr, uint8_t len, uint8_t *written)
{
	uint8_t count = 0, head = 0, *p = (uint8_t *)ptr;
	BUFFER_ENTER_CRITICAL(buffer);
	head = buffer->head;
	count = buffer->count;
	BUFFER_EXIT_CRITICAL(buffer);
	count = MIN(buffer->size - count, len);
	*written = 0;
	if (count)
	{
		uint8_t avail = (buffer->size - head);
		if (avail < count && avail)
		{
			memcpy(&buffer->data[head * buffer->elem_size], ptr, avail * buffer->elem_size);
			*written = avail;
			count -= avail;
			head = 0;
		}
		else
		{
			avail = 0;
		}
		if (count)
		{
			memcpy(&buffer->data[head * buffer->elem_size], &p[avail * buffer->elem_size], count * buffer->elem_size);
			*written += count;
			head += count;
			if (head == buffer->size)
			{
				head = 0;
			}
			BUFFER_ENTER_CRITICAL(buffer);
			buffer->head = head;
			buffer->count += *written;
			BUFFER_EXIT_CRITICAL(buffer);
		}
	}
}

void queue_read(ring_buffer_t *buffer, void *ptr, uint8_t len, uint8_t *read)
{
	uint8_t count = 0, tail = 0, *p = (uint8_t *)ptr;
	BUFFER_ENTER_CRITICAL(buffer);
	tail = buffer->tail;
	count = buffer->count;
	BUFFER_EXIT_CRITICAL(buffer);

	if (count > len)
	{
		count = len;
	}
	*read = 0;
	if (count)
	{
		uint8_t avail = buffer->size - tail;
		if (avail < count && avail)
		{
			memcpy(ptr, &buffer->data[tail * buffer->elem_size], avail * buffer->elem_size);
			*read = avail;
			count -= avail;
			tail = 0;
		}
		else
		{
			avail = 0;
		}
		if (count)
		{
			memcpy(&p[avail * buffer->elem_size], &buffer->data[tail * buffer->elem_size], count * buffer->elem_size);
			*read += count;
			tail += count;
			if (tail == buffer->size)
			{
				tail = 0;
			}
			BUFFER_ENTER_CRITICAL(buffer);
			buffer->tail = tail;
			buffer->count -= *read;
			BUFFER_EXIT_CRITICAL(buffer);
		}
	}
}

void queue_clear(ring_buffer_t *buffer)
{
	BUFFER_ENTER_CRITICAL(buffer);
	memset(buffer->data, 0, buffer->elem_size * buffer->size);
	buffer->tail = 0;
	buffer->head = 0;
	buffer->count = 0;
	BUFFER_EXIT_CRITICAL(buffer);
}
