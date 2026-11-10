//========================================================================
// priority_mutex.h
//========================================================================
// Header for Priority Mutex Implementation

/*
    Assumptions made:
    1. all pointers are valid, caller never tries to pass NULL to a function
    2. thread never locks a mutex it owns, and only the owner will call unlock()
    3. a thread only owns one mutex at a time
    4. all POSIX thread operations succeed
*/

#ifndef priority_mutex_H
#define priority_mutex_H

#include <stdbool.h>
#include <pthread.h> // threading library
/*
    pthread_t:          a thread
    pthread_mutext_t:   regular mutex
    pthread_cond_t:     condition variable
*/

// struct for the waiting 
typedef struct priority_waiter priority_waiter_t;

// mutex struct
typedef struct {
    pthread_mutex_t guard; // protects shared resources by only allowing one thread modify them at a time
    pthread_t       owner; // thread that currently owns the mutex (the custom one, not guard)
    bool            locked; // if mutex is currently occupied
    int             owner_policy; // policy of owner, needed for pthread_setschedparam()
    int             owner_base_priority; // priority of owner BEFORE inheritance
    int             owner_effective_priority; // priority of the owner AFTER inheritance (temporary)
    priority_waiter_t     *waiters; // points to linked list of threads waiting for the mutex
} priority_mutex_t;

void priority_mutex_init   (priority_mutex_t *mutex); // initializes the mutex
void priority_mutex_destroy(priority_mutex_t *mutex); // 
void priority_mutex_lock   (priority_mutex_t *mutex); // tries to acquire mutex, waits if occupied
void priority_mutex_unlock (priority_mutex_t *mutex); // releases the mutex, selects next owner

#endif