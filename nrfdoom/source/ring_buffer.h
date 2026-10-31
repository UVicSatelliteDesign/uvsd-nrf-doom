#ifndef RING_BUFFER_H
#define RING_BUFFER_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define RING_BUFFER_SIZE 64

typedef struct {
    uint8_t buffer[RING_BUFFER_SIZE];
    volatile size_t head;
    volatile size_t tail;
} ring_buffer_t;

// Initialize all buffer indicies to 0
void init_ring_buffer(ring_buffer_t *rb);

// Test if ring buffer is empty
bool ring_buffer_is_empty(const ring_buffer_t *rb);

// Test if ring buffer is full
bool ring_buffer_is_full(const ring_buffer_t *rb);

// Write to ring buffer head. Returns true on success and false if full
bool ring_buffer_add(ring_buffer_t *rb, uint8_t input_val);

// Read from ring buffer tail. Returns true on success and false if empty.
bool ring_buffer_pop(ring_buffer_t *rb, uint8_t *output_val);
#endif
