Single Consumer, Single Producer Ring Buffer

Overview

Ring buffer with fixed capacity, single producer, and single consumer. Storage is provided by caller and does not use dynamic memory allocation.

Design

- write_slot points to the next position to write
- read_slot points to the next unread value
- uses one empty slot within the buffer to differentiate full and empty states
- uses atomic pointers ( note: I am aware that in order for safe accessing by producer and consumer requires synchronizing through aatomic load/store operations, but I was unsure about how exactly to implement using the stdatomic library )

Files
- ring_buffer.c: implemetation and example main() function that uses the functions
- ring_buffer.h: struct definition and function declarations, also includes necessary standard libraries
- example_output.txt: sample program output (from main() in ring_buffer.c)