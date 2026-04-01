#include <pthread.h>
#include "work_stack.h"

typedef struct Threadpool {
    pthread_t* workers;
    unsigned int nworkers;

    WorkStack* work_stack;
} Threadpool;

// Allocate/free a threadpool with n threads in it
Threadpool* threadpool_alloc(unsigned int nthreads, unsigned int work_buf_size);
void threadpool_dealloc(Threadpool* pool);

// Schedule work on the threadpool, if the task work_stack is full, blocks until task queue has capacity
void threadpool_schedule(Threadpool* pool, TPoolWork work);

// Wait for all jobs on the threadpool to finish
void threadpool_join(Threadpool* pool);