Priority Inheritance Mutex

Overview

What is priority inversion?
Priority inversion occurs when a high priority thread is forced to wait for a lower priority thread that is holding a resource that it needs. it becomes a major problem when a medium priority thread interrupts the lower priority thread, preventing it from finishing work and releasing the resource that the high priority thread needs/

This project implements a custom mutex with priority inheritance using POSIX threads in C. Priority inheritance is used to address priority inversion, which occurs when a high-priority thread is blocked by a lower-priority thread holding a mutex. By temporarily increasing the priority of the mutex owner, the implementation allows the lower-priority thread to finish its work and release the mutex sooner.

Implementation

    The implementation consists of two files:

    - priority_mutex.h: Defines the mutex structure and declares functions.
    - priority_mutex.c: Implements the mutex operations, priority inheritance, and management of waiting threads.

Each mutex maintains its current owner, the owner's original and effective priorities, and a linked list of waiting threads.

When a thread attempts to acquire an available mutex, it becomes the owner. If the mutex is already locked, the thread is inserted into the waiting list in orderr of priority and waits.

If the waiting thread has a higher priority than the current owner, the owner's scheduling priority is temporarily increased using pthread_setschedparam().