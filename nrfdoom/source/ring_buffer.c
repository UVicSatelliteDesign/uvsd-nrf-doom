#include "ring_buffer.h"

void init_ring_buffer(ring_buffer_t *rb) {
    for (size_t index = 0; index < RING_BUFFER_SIZE; index++) {
        rb->buffer[index] = 0;
    }
    rb->tail = 0;
    rb->head = 0;
}

// Test if ring buffer is empty
bool ring_buffer_is_empty(const ring_buffer_t *rb) { return (rb->head == rb->tail); }

// Test if ring buffer is full
bool ring_buffer_is_full(const ring_buffer_t *rb) {
    return (rb->tail == (rb->head + 1) % RING_BUFFER_SIZE);
}

// Write to ring buffer head. Returns true on success and false if full
bool ring_buffer_add(ring_buffer_t *rb, uint8_t input_val) {
    if (ring_buffer_is_full(rb))
        return false;
    else {
        rb->buffer[rb->head] = input_val;
        rb->head = (rb->head + 1) % RING_BUFFER_SIZE;
        return true;
    }
}

// Read from ring buffer tail. Returns true on success and false if empty.
bool ring_buffer_pop(ring_buffer_t *rb, uint8_t *output_val) {
    if (ring_buffer_is_empty(rb) || output_val == NULL)
        return false;
    else {
        *output_val = rb->buffer[rb->tail];
        rb->tail = (rb->tail + 1) % RING_BUFFER_SIZE;
        return true;
    }
}
