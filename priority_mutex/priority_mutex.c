//========================================================================
// priority_mutex.c
//========================================================================
// Priority Mutex Implementation

#define _POSIX_C_SOURCE 200809L
#include "priority_mutex.h"

#include <sched.h>
#include <stddef.h>

// reads the priority of the thread, static because only only used by this file
static int read_priority(pthread_t thread, int *policy) {
    struct sched_param param; // scheduling parameter

    pthread_getschedparam(thread, policy, &param); // gets thread's priority

    return param.sched_priority;
}

// implements priority inheritance
static void set_priority(pthread_t thread, int policy, int priority) {
    struct sched_param param = {0}; 

    param.sched_priority = priority; // sets the requested priority

    pthread_setschedparam(thread, policy, &param); // changes the thread's schedulign parameters
}

//------------------------------------------------------------------
// Waiting logic
//------------------------------------------------------------------

// waiter struct if a thread tries to acquire a mutex that is busy, allows for linked list with NEXT field
struct priority_waiter {
    pthread_t       thread;     // thread that is waiting
    int             priority;   // priority of the waiter
    bool            granted;    // initialized to false, becomes true when waiter gets mutec
    pthread_cond_t  condition;  
    struct          priority_waiter *next; // next waiter in the linked list
};

// inserting a waiting thread into linked list
static void insert_waiter(priority_mutex_t *m, priority_waiter_t *waiter) {
    priority_waiter_t **slot = &m->waiters;

    while (*slot != NULL && (*slot)->priority >= waiter->priority) // finding the insertion point
        slot = &(*slot)->next;

    waiter->next = *slot;

    *slot = waiter;
}

//------------------------------------------------------------------
// Mutex Logic
//------------------------------------------------------------------

// initializing
void priority_mutex_init(priority_mutex_t *m) {
    
    pthread_mutex_init(&m->guard, NULL);

    // automatic initialization values
    m->locked = false;
    m->owner_policy = 0;
    m->owner_base_priority = 0;
    m->owner_effective_priority = 0;
    m->waiters = NULL;              // NULL pointer since there is no list of waiters yet

}

// destroying the mutex after done using
void priority_mutex_destroy(priority_mutex_t *m) {
    pthread_mutex_destroy(&m->guard); // releases tge guard
}

//------------------------------------------------------------------
// Locking Logic
//------------------------------------------------------------------

void priority_mutex_lock(priority_mutex_t *m) {

    int policy;
    int priority = read_priority(pthread_self(), &policy);
    
    pthread_mutex_lock(&m->guard);

    // mutex is available
    if (!m->locked) {
        m->locked = true; // mutex is now locked
        m->owner = pthread_self(); // owner is the thread that is calling
        m->owner_policy = policy; // mutex gets the policy of the owner
        m->owner_base_priority = priority; // mutex gets the priority of the owner
        m->owner_effective_priority = priority; // no inheritance since mutex is free, effective priority same as base
        pthread_mutex_unlock(&m->guard); // releases guard lock, allows for NON MUTEX threads to share guard
        return;
    }


    // MUTEX OCCUPIED, add calling thread as a waiter

    // creates a waiter
    priority_waiter_t waiter = {
            .thread = pthread_self(), 
            .priority = priority, 
            .granted = false, 
            .next = NULL
    };

    pthread_cond_init(&waiter.condition, NULL);

    insert_waiter(m, &waiter); // uses function to insert the waiter in the right place

    // priority inheritance
    if (priority > m->owner_effective_priority) { // checks if the waiter has a higher priority than the current owner of the mutex
        set_priority(m->owner, m->owner_policy, priority); // if so, boosts owner's schedulign priority to waiter's
        m->owner_effective_priority = priority; // record new affective priority (NOT BASE)
    }

    // "sleeping" until mutex is granted
    while (!waiter.granted) {
        pthread_cond_wait(&waiter.condition, &m->guard);
    }

    // exits while loop, meaning that mutex is available
    pthread_mutex_unlock(&m->guard);
    pthread_cond_destroy(&waiter.condition);
    return;
}

//------------------------------------------------------------------
// Unlocking Logic
//------------------------------------------------------------------

void priority_mutex_unlock(priority_mutex_t *m) {

    pthread_mutex_lock(&m->guard);

    if (m->owner_effective_priority != m->owner_base_priority) {
        set_priority(m->owner, m->owner_policy, m->owner_base_priority);
    }

    if (m->waiters == NULL) { // if there are no waiters, set mutex to unlocked
        m->locked = false;
        m->owner_effective_priority = 0;
    } 
    else {
        priority_waiter_t *next = m->waiters; // selects the first node of linked list as waiter
        m->waiters = next->next;
        m->owner = next->thread; // transfers ownership
        int policy;
        int priority = read_priority(next->thread, &policy);

        // mutex now tracks the new owner
        m->owner_policy = policy;
        m->owner_base_priority = priority;
        m->owner_effective_priority = priority;

        next->granted = true;
        pthread_cond_signal(&next->condition);
    }
    pthread_mutex_unlock(&m->guard);
    return;
}

//------------------------------------------------------------------
// Helper and functions for running an example threading sequence
//------------------------------------------------------------------

#include <stdio.h>
#include <unistd.h>

priority_mutex_t test_mutex;

void *low_thread(void *arg) {
    priority_mutex_lock(&test_mutex);
    printf("LOW: Acquired mutex\n");

    sleep(3);

    printf("LOW: Releasing mutex\n");
    priority_mutex_unlock(&test_mutex);
    return NULL;
}

void *medium_thread(void *arg) {
    printf("MEDIUM: Running unrelated work\n");
    sleep(2);
    printf("MEDIUM: Finished\n");
    return NULL;
}

void *high_thread(void *arg) {
    printf("HIGH: Waiting for mutex\n");

    priority_mutex_lock(&test_mutex);
    printf("HIGH: Acquired mutex\n");

    priority_mutex_unlock(&test_mutex);
    return NULL;
}

int main(void) {
    pthread_t low, medium, high;

    priority_mutex_init(&test_mutex);

    pthread_create(&low, NULL, low_thread, NULL);
    sleep(1);

    pthread_create(&medium, NULL, medium_thread, NULL);
    pthread_create(&high, NULL, high_thread, NULL);

    pthread_join(low, NULL);
    pthread_join(medium, NULL);
    pthread_join(high, NULL);

    priority_mutex_destroy(&test_mutex);

    printf("All threads finished.\n");
    return 0;
}