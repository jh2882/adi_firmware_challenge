//========================================================================
// ring_buffer.h
//========================================================================
// Header function for Ring Buffer implementation

/* 
    Assumptions made:
    1. capacity requested is at least 1
    2. rb and storage are not NULL
    3. storage array > capacity
*/ 

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdatomic.h>  
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct { 
    uint8_t *buffer;
    _Atomic (uint8_t *) write_slot;
    _Atomic (uint8_t *) read_slot;
    size_t capacity;

} ringbuf_t;


bool ringbuf_init(
    ringbuf_t *rb, 
    uint8_t *storage, 
    size_t capacity
);

// Used by producer only
bool ringbuf_push(
    ringbuf_t *rb, 
    uint8_t byte
);

// Used by consumer only
bool ringbuf_pop(
    ringbuf_t *rb,
    uint8_t *out
);

#endif