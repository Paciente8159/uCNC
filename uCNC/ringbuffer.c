#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "ringbuffer.h"

struct ringbuffer_t {
	void* data;
	size_t head;
	size_t tail;
	size_t elem_count; //of the buffer
	size_t elem_size;
	bool full;
};

static uint8_t next_write(buffer_t buffer)
{
	if(++(buffer->head) == buffer->elem_count)
	{
		buffer->head = 0;
	}
	
	buffer->full = (buffer->head==buffer->tail);
}

static uint8_t next_read(buffer_t buffer)
{
	if(++(buffer->tail) == buffer->elem_count)
	{
		buffer->tail = 0;
	}
	
	buffer->full = false;	
}

buffer_t buffer_init(void* data, size_t type_size, size_t buffer_lenght)
{
	buffer_t buf = (buffer_t)malloc(sizeof(struct ringbuffer_t));
	buf->data = data;
	buf->elem_count = buffer_lenght;
	buf->elem_size = type_size;
	buf->head = 0;
	buf->tail = 0;
	buf->full = false;
	
	return buf;
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
	return (!buffer->full && (buffer->head == buffer->tail));
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
	next_write(buffer);
	return dest;
}

void* buffer_get_last(buffer_t buffer)
{
	if(is_buffer_empty(buffer))
	{
		return NULL;
	}
	
	uint8_t address = (buffer->head != 0) ? (buffer->head - 1) : (buffer->elem_count - 1); 
	
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
	if(ptr == buffer_get_last(buffer))
	{
		return NULL;
	}
	
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
