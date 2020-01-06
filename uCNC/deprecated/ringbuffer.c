/*
	Name: ringbuffer.c - a generic ringbuffer c library with minimalistic dynamic memory usage
	Copyright: 2019 João Martins
	Author: João Martins
	Date: Oct/2019
	Description: uCNC is a free cnc controller software designed to be flexible and
	portable to several	microcontrollers/architectures.
	uCNC is a FREE SOFTWARE under the terms of the GPLv3 (see <http://www.gnu.org/licenses/>).
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "ringbuffer.h"

struct ringbuffer_t {
	void* data;
	uint8_t head;
	uint8_t tail;
	uint8_t elem_count; //of the buffer
	uint8_t elem_size;
	bool full;
	bool empty;
};

static inline void next_write(buffer_t buffer) __attribute__((always_inline));
static inline void next_write(buffer_t buffer)
{
	if(++(buffer->head) == buffer->elem_count)
	{
		buffer->head = 0;
	}
	
	buffer->full = (buffer->head==buffer->tail);
	buffer->empty = false;
}

static inline void next_read(buffer_t buffer) __attribute__((always_inline));
static inline void next_read(buffer_t buffer)
{
	if(++(buffer->tail) == buffer->elem_count)
	{
		buffer->tail = 0;
	}
	
	buffer->full = false;
	buffer->empty = (buffer->head==buffer->tail);	
}

buffer_t buffer_init(void* data, uint8_t type_size, uint8_t buffer_lenght)
{
	buffer_t buffer = (buffer_t)malloc(sizeof(struct ringbuffer_t));
	buffer->data = data;
	buffer->elem_count = buffer_lenght;
	buffer->elem_size = type_size;
	buffer->head = 0;
	buffer->tail = 0;
	buffer->full = false;
	buffer->empty = true;
	return buffer;
}

void buffer_free(buffer_t buffer)
{
	free(buffer);
}

bool is_buffer_full(buffer_t buffer)
{
	return buffer->full;
}

bool is_buffer_empty(buffer_t buffer)
{	
	return buffer->empty;
}

void buffer_read(buffer_t buffer, void* data)
{
	if(is_buffer_empty(buffer))
	{
		data = NULL;
		return;
	}
	
	if(data != NULL)
	{
		const void* ptr = ((uint8_t*)(buffer->data))+(buffer->elem_size * buffer->tail);
		memcpy(data, ptr, buffer->elem_size);
	}
	
	next_read(buffer);
}

void* buffer_write(buffer_t buffer, const void* data)
{
	if(is_buffer_full(buffer))
	{
		return NULL;
	}
	
	void* dest = ((uint8_t*)buffer->data)+(buffer->elem_size*buffer->head);
	if(data != NULL)
	{
		memcpy(dest, data, buffer->elem_size);
	}
	
	buffer->head++;
	if(buffer->head == buffer->elem_count)
	{
		buffer->head = 0;
	}
	
	buffer->empty = false;
	buffer->full = (buffer->head==buffer->tail);

	return dest;
}

void buffer_clear(buffer_t buffer)
{
	buffer->head = 0;
	buffer->tail = 0;
	buffer->full = false;
	buffer->empty = true;
}

void* buffer_get_next_free(buffer_t buffer)
{
	if(!is_buffer_full(buffer))
	{
		return ((uint8_t*)buffer->data)+(buffer->elem_size*buffer->head);
	}
	else
	{
		return NULL;
	}
}

void* buffer_get_last(buffer_t buffer)
{
	size_t address = 0;

	if(!is_buffer_empty(buffer))
	{
		address = (buffer->head != 0) ? (buffer->head - 1) : (buffer->elem_count - 1);
	}
	else
	{
		address = buffer->head;
	}
	
	void* dest = ((uint8_t*)buffer->data)+(buffer->elem_size*address);
	return dest;
}

void* buffer_get_first(buffer_t buffer)
{
	if(is_buffer_empty(buffer))
	{
		return NULL;
	}
	
	void* dest = ((uint8_t*)buffer->data)+(buffer->elem_size*buffer->tail);
	return dest;
}

void* buffer_get_prev(buffer_t buffer, void* ptr)
{
	/*if(is_buffer_empty(buffer))
	{
		return NULL;
	}*/
	
	if(ptr == buffer_get_first(buffer))
	{
		return NULL;
	}
	
	if(ptr != buffer->data)
	{
		ptr = ((uint8_t*)ptr)-buffer->elem_size;
	}
	else
	{
		ptr = ((uint8_t*)buffer->data)+(buffer->elem_size*(buffer->elem_count-1));
	}
	
	return ptr;
}

void* buffer_get_next(buffer_t buffer, void* ptr)
{
	void* last_elem = ((uint8_t*)buffer->data)+(buffer->elem_size*(buffer->elem_count-1));
	
	if(ptr != last_elem)
	{
		ptr = ((uint8_t*)ptr)+buffer->elem_size;
	}
	else
	{
		ptr = buffer->data;
	}
	
	return ptr;
}
