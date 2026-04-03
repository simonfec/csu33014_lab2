// Authors
// - Colin Simon-Fellowes
// - Alex Robertson

#include <pthread.h>
#include "work_stack.h"

typedef struct Threadpool {
    pthread_t* workers;
    unsigned int nworkers;

    WorkStack* work_stack;
} Threadpool;

/**
 * Allocate a threadpool.
 * @param nthreads The number of threads the threadpool will spawn.
 * @pre nthreads > 0
 * @param work_buf_size The number of elements in the work buffer, larger will create a smaller chance of blocking on threadpool_schedule.
 * @pre work_buf_size > 0
 * @returns The allocated threadpool or NULL if it wasn't allocated successfully.
 */
Threadpool* threadpool_alloc(unsigned int nthreads, unsigned int work_buf_size);

/**
 * Schedules a job on the threadpool.
 * @param pool The threadpool to schedule work on.
 * @param work The job to be scheduled.
 * @note Will block until there's space in the internal buffer to push the work to.
 */
void threadpool_schedule(Threadpool* pool, TPoolWork work);

/**
 * Wait for all the work scheduled on the threadpool to finish.
 * @param pool The threadpool to join
 * @warning This function deallocates and destroys the threadpool, the pool pointer will no longer be valid.
 */
void threadpool_join(Threadpool* pool);