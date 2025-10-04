/*
	Name: buffer.c
	Description: Some useful circular buffers functions and macros.
		These are small (255 slots max) sized buffer optimized for speed.

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

static FORCEINLINE void set_flag(ring_buffer_t *b, uint8_t idx)
{
	ATOMIC_FETCH_OR(uint8_t, b->flags[idx >> 3], (uint8_t)(1u << (idx & 7)), __ATOMIC_RELEASE);
}

static FORCEINLINE bool test_flag(ring_buffer_t *b, uint8_t idx)
{
	uint8_t byte = ATOMIC_LOAD_N(uint8_t, b->flags[idx >> 3], __ATOMIC_ACQUIRE);
	return (byte & (uint8_t)(1u << (idx & 7))) != 0;
}

// Clear can be release or relaxed; release is conservative if producers/consumers ever gate on flags.
static FORCEINLINE void clear_flag(ring_buffer_t *b, uint8_t idx)
{
	ATOMIC_FETCH_AND(uint8_t, b->flags[idx >> 3], (uint8_t)~(1u << (idx & 7)), __ATOMIC_RELAXED);
}

static FORCEINLINE uint8_t index_wrap_around(uint8_t val, uint8_t cap)
{
	return (val >= cap) ? 0 : val;
}

uint8_t buffer_write_available(ring_buffer_t *buffer)
{
	uint8_t head = (uint8_t)ATOMIC_LOAD_N(uint8_t, buffer->head, __ATOMIC_ACQUIRE);
	uint8_t tail = (uint8_t)ATOMIC_LOAD_N(uint8_t, buffer->tail, __ATOMIC_ACQUIRE);
	return (head >= tail) ? (buffer->size - (head - tail) - 1) : (tail - head - 1);
}

uint8_t buffer_read_available(ring_buffer_t *buffer)
{
	uint8_t tail = (uint8_t)ATOMIC_LOAD_N(uint8_t, buffer->tail, __ATOMIC_ACQUIRE);
	uint8_t head = (uint8_t)ATOMIC_LOAD_N(uint8_t, buffer->head, __ATOMIC_ACQUIRE);
	return (head >= tail) ? (head - tail) : (buffer->size - (tail - head));
}

bool buffer_empty(ring_buffer_t *buffer)
{
	return ATOMIC_COMPARE(buffer->tail, buffer->head);
}

bool buffer_full(ring_buffer_t *buffer)
{
	uint8_t head = (uint8_t)ATOMIC_LOAD_N(uint8_t, buffer->head, __ATOMIC_ACQUIRE);
	head = index_wrap_around(head + 1, buffer->size);
	return ATOMIC_COMPARE(buffer->tail, head);
}

void buffer_peek(ring_buffer_t *buffer, void *ptr) {
    uint8_t tail = ATOMIC_LOAD_N(uint8_t, buffer->tail, __ATOMIC_RELAXED);
    uint8_t head = ATOMIC_LOAD_N(uint8_t, buffer->head, __ATOMIC_RELAXED);
    if (tail == head || !test_flag(buffer, tail)) {
        memset(ptr, 0, buffer->elem_size);
        return;
    }
    memcpy(ptr, &buffer->data[(size_t)tail * buffer->elem_size], buffer->elem_size);
}

void buffer_enqueue(ring_buffer_t *buffer, void *ptr)
{
	for (;;)
	{
		uint8_t head = ATOMIC_LOAD_N(uint8_t, buffer->head, __ATOMIC_ACQUIRE);
		uint8_t tail = ATOMIC_LOAD_N(uint8_t, buffer->tail, __ATOMIC_ACQUIRE);

		uint8_t next_slot = index_wrap_around(head + 1, buffer->size);

		if (next_slot == tail)
		{
			continue; // buffer full, spin
		}

		if (!ATOMIC_COMPARE_EXCHANGE_N(buffer->head, head, next_slot, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
		{
			continue; // lost race, retry
		}

		// Write data into slot
		memcpy(&buffer->data[(size_t)head * (size_t)buffer->elem_size], ptr, buffer->elem_size);

		// Publish slot as ready
		set_flag(buffer, head);
		return;
	}
}

void buffer_dequeue(ring_buffer_t *buffer, void *ptr)
{
	for (;;)
	{
		uint8_t head = ATOMIC_LOAD_N(uint8_t, buffer->head, __ATOMIC_ACQUIRE);
		uint8_t tail = ATOMIC_LOAD_N(uint8_t, buffer->tail, __ATOMIC_ACQUIRE);

		if (tail == head)
		{
			memset(ptr, 0, buffer->elem_size);
			return; // empty
		}

		uint8_t next_avail = index_wrap_around(tail + 1, buffer->size);

		// Spin until producer has set ready flag
		if (!test_flag(buffer, tail))
		{
			ATOMIC_SPIN();
			continue;
		}

		if (!ATOMIC_COMPARE_EXCHANGE_N(buffer->tail, tail, next_avail, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
		{
			ATOMIC_SPIN();
			continue; // lost race
		}

		// Copy out data
		memcpy(ptr, &buffer->data[(size_t)tail * (size_t)buffer->elem_size], buffer->elem_size);

		// Clear ready flag
		clear_flag(buffer, tail);
		return;
	}
}

void buffer_write(ring_buffer_t *buffer, void *ptr, uint8_t len, uint8_t *written)
{
	uint8_t count = 0;
	uint8_t *src = (uint8_t *)ptr;

	for (uint8_t i = 0; i < len; i++)
	{
		uint8_t head, tail, next_slot;

		for (;;)
		{
			head = ATOMIC_LOAD_N(uint8_t, buffer->head, __ATOMIC_ACQUIRE);
			tail = ATOMIC_LOAD_N(uint8_t, buffer->tail, __ATOMIC_ACQUIRE);

			next_slot = index_wrap_around(head + 1, buffer->size);

			if (next_slot == tail)
			{
				// buffer full
				if (written)
				{
					*written = count;
				}
				return;
			}

			if (!ATOMIC_COMPARE_EXCHANGE_N(buffer->head, head, next_slot, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
			{
				continue; // lost race, retry
			}

			// Write element
			memcpy(&buffer->data[(size_t)head * (size_t)buffer->elem_size],
						 &src[(size_t)i * (size_t)buffer->elem_size],
						 buffer->elem_size);

			// Publish slot
			set_flag(buffer, head);

			count++;
			break;
		}
	}

	if (written)
	{
		*written = count;
	}
}

void buffer_read(ring_buffer_t *buffer, void *ptr, uint8_t len, uint8_t *read)
{
	uint8_t count = 0;
	uint8_t *dst = (uint8_t *)ptr;

	for (uint8_t i = 0; i < len; i++)
	{
		uint8_t head, tail, next_avail;

		for (;;)
		{
			head = ATOMIC_LOAD_N(uint8_t, buffer->head, __ATOMIC_ACQUIRE);
			tail = ATOMIC_LOAD_N(uint8_t, buffer->tail, __ATOMIC_ACQUIRE);

			if (tail == head)
			{
				// buffer empty
				if (read)
				{
					*read = count;
				}
				return;
			}

			// Wait until producer published slot
			if (!test_flag(buffer, tail))
			{
				ATOMIC_SPIN();
				continue;
			}

			next_avail = index_wrap_around(tail + 1, buffer->size);

			if (!ATOMIC_COMPARE_EXCHANGE_N(buffer->tail, tail, next_avail, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
			{
				ATOMIC_SPIN();
				continue; // lost race
			}

			// Copy element out
			memcpy(&dst[(size_t)i * (size_t)buffer->elem_size],
						 &buffer->data[(size_t)tail * (size_t)buffer->elem_size],
						 buffer->elem_size);

			// Clear ready flag
			clear_flag(buffer, tail);

			count++;
			break;
		}
	}

	if (read)
		*read = count;
}

void buffer_clear(ring_buffer_t *buffer)
{
	// Reset indices
	ATOMIC_STORE_N(buffer->head, 0, __ATOMIC_RELEASE);
	ATOMIC_STORE_N(buffer->tail, 0, __ATOMIC_RELEASE);

	// Clear flags
	uint8_t flag_bytes = (buffer->size + 7) >> 3;
	memset(buffer->flags, 0, flag_bytes);

	// Optionally clear data (not strictly needed)
	memset(buffer->data, 0, buffer->size * buffer->elem_size);
}
