#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdbool.h>

typedef struct ringbuffer_t ringbuffer_t;

typedef ringbuffer_t* buffer_t;

buffer_t buffer_init(void* data, size_t type_size, size_t buffer_lenght);
void buffer_free(buffer_t buffer);

bool is_buffer_full(buffer_t buffer);
bool is_buffer_empty(buffer_t buffer);

void buffer_read(buffer_t buffer, void* data);
void* buffer_write(buffer_t buffer, const void* data);

void* buffer_get_last(buffer_t buffer);
void* buffer_get_first(buffer_t buffer);
void* buffer_get_prev(buffer_t buffer, void* ptr);
void* buffer_get_next(buffer_t buffer, void* ptr);

#endif
