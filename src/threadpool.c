// Authors
// - Colin Simon-Fellowes
// - Alex Robertson

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "threadpool.h"

void* worker_routine(void* args);

void threadpool_dealloc(Threadpool* pool) {
    if (pool != NULL) {
        free(pool->workers);
        workstack_dealloc(pool->work_stack);
    }
    free(pool);
}

Threadpool* threadpool_alloc(unsigned int nthreads, unsigned int work_buf_size) {
    assert(nthreads > 0);
    assert(work_buf_size > 0);

    // Allocate memory for all fields of the thread pool
    Threadpool* pool = malloc(sizeof(Threadpool));
    if (pool == NULL) return NULL;

    pool->nworkers = nthreads;

    pool->workers = malloc(nthreads * sizeof(pthread_t));
    if (pool->workers == NULL) {
        threadpool_dealloc(pool);
        return NULL;
    }

    pool->work_stack = workstack_alloc(work_buf_size);
    if (pool->work_stack == NULL) {
        threadpool_dealloc(pool);
        return NULL;
    }

    // Create all worker threads
    for (int i = 0; i < nthreads; i++) {
        int ret = pthread_create(&pool->workers[i], NULL, worker_routine, (void*)pool->work_stack);
        if (ret != 0) {
            threadpool_dealloc(pool);
            return NULL;
        }
    }
    return pool;
}

void threadpool_schedule(Threadpool* pool, TPoolWork work) {
    workstack_push(pool->work_stack, work);
}

void threadpool_join(Threadpool* pool) {
    // Join and dealloc threadpool at the same time to avoid inconsistent state where the pool is technically alive
    // but the workers are joined.

    // Signal no more work coming then join all threads
    workstack_no_more_work(pool->work_stack);
    for (int i = 0; i < pool->nworkers; i++)
        pthread_join(pool->workers[i], NULL);
    
    threadpool_dealloc(pool);
}

void* worker_routine(void* args) {
    WorkStack* work_stack = args;
    while (true) {
        TPoolWork work;
        bool work_remaining = workstack_pop(work_stack, &work);
        if (!work_remaining) return NULL; // Signalled that there's to be no more work
        work.func(work.args);
    }
}