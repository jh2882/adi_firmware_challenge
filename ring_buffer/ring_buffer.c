//========================================================================
// ring_buffer.c
//========================================================================
// Ring Buffer implementation

#include "ring_buffer.h"

bool ringbuf_init ( ringbuf_t *rb, uint8_t *storage, size_t capacity ){

    if ( rb == NULL || storage == NULL || capacity <= 1){
        // could throw an error, but in this case just returning without intializing 
        return false;
    }

    else{
    rb -> buffer     = storage;
    rb -> write_slot = storage;
    rb -> read_slot  = storage;
    // atomic_init(&rb->write_slot, storage);
    // atomic_init(&rb->read_slot, storage); 
    rb -> capacity   = capacity + 1;
    return true;
    }

}

// Used by producer only (to write)
bool ringbuf_push ( ringbuf_t *rb, uint8_t byte ){

    if ( rb == NULL ){
        return false;
    }

    uint8_t *next_write = rb -> write_slot +1; // what is the next write slot? 
 
    // what if reached end?
    if ( next_write == rb -> buffer + rb -> capacity){   // check
        next_write = rb -> buffer; // next write becomes the beginning
    }
    
    if ( next_write == rb -> read_slot ){ // the buffer is full!!!
        return false;
    }

    *rb -> write_slot = byte; // writes byte to the available write slot
    rb -> write_slot = next_write; // write slot increments to the next 

    return true;
}

// Used by consumer only (to read)
bool ringbuf_pop ( ringbuf_t *rb, uint8_t *out ){
    if ( rb == NULL || out == NULL ){
        return false;
    }

    // empty buffer
    if ( rb -> read_slot == rb -> write_slot ){
        return false;
    }

    *out = *rb -> read_slot; // assign out to the value 
    rb -> read_slot++;

    if (rb -> read_slot == rb -> buffer + rb -> capacity ){ // reached end?
        rb -> read_slot = rb -> buffer; // read slot becomes the beginning
    }

    return true;
}

int main(void) {

    // testing for a buffer with capacity of 5

    uint8_t storage[100];
    ringbuf_t rb;
    uint8_t value;

    ringbuf_init(&rb, storage, 5);

    printf("===== START =====\n");

    // pushing
    printf("\n--- PUSHING ---\n");

    printf("Push 1: %s\n", ringbuf_push(&rb, 1) ? "Success" : "Failed");
    printf("Push 2: %s\n", ringbuf_push(&rb, 2) ? "Success" : "Failed");
    printf("Push 3: %s\n", ringbuf_push(&rb, 3) ? "Success" : "Failed");
    printf("Push 4: %s\n", ringbuf_push(&rb, 4) ? "Success" : "Failed");
    printf("Push 5: %s\n", ringbuf_push(&rb, 5) ? "Success" : "Failed");

    // expected to fail, capacity is only 5
    printf("Push 6: %s\n", ringbuf_push(&rb, 6) ? "Success" : "Failed");


    // popping
    printf("\n--- POPPING ---\n");

    while (ringbuf_pop(&rb, &value)) {
        printf("Popped: %d\n", value);
    }

    // buffer now empty, ringbuf_pop should return false
    printf("Pop from empty buffer: %s\n",
           ringbuf_pop(&rb, &value) ? "Success" : "Failed");

    // wrap around
    printf("\n--- WRAPAROUND ---\n");

    ringbuf_push(&rb, 1);
    ringbuf_push(&rb, 2);
    ringbuf_push(&rb, 3);
    ringbuf_push(&rb, 4);

    ringbuf_pop(&rb, &value);
    printf("Popped: %d\n", value);

    ringbuf_pop(&rb, &value);
    printf("Popped: %d\n", value);

    ringbuf_push(&rb, 5);
    ringbuf_push(&rb, 6);

    while (ringbuf_pop(&rb, &value)) {
        printf("Popped: %d\n", value);
    }

    printf("\n===== DONE =====\n");

    return 0;
}