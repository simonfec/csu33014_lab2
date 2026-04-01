#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct TPoolArgs {
    int m;
    int w;
    int height;
    int nchannels;
    int kernel_order;
    float*** image;
    int16_t**** kernels;
    float* output;
} TPoolArgs;

typedef void (*TPoolWorkFunc)(TPoolArgs);

typedef struct TPoolWork {
    TPoolWorkFunc func;
    TPoolArgs args;
} TPoolWork;

typedef struct WorkStack {
    pthread_mutex_t stack_lock;
    pthread_cond_t work_popped;
    pthread_cond_t work_pushed;
    TPoolWork* stack; // Protected by lock
    unsigned int stack_next; // Protected by lock
    unsigned int stack_size;
    bool no_more_work; // Protected by lock
} WorkStack;

/**
 * Allocates a workstack.
 * @param stack_size The maximum number of entries in the stack.
 * @returns The allocated workstack or NULL if it couldn't be allocated.
 */
WorkStack* workstack_alloc(unsigned int stack_size);

/**
 * Deallocates a workstack allocated with the workstack_alloc function
 * @param stack The stack to dealloc
 */
void workstack_dealloc(WorkStack* stack);

/**
 * Push work to the workstack.
 * @param stack The stack to push work to.
 * @param work The item to be pushed to the stack.
 * @note If the stack is full this function will block until it has space.
 */
void workstack_push(WorkStack* stack, TPoolWork work);

/**
 * Pops work off the workstack.
 * @param stack The stack to pop work from.
 * @param[out] work_out The work that is being popped.
 * @returns True if work has been popped, false if the stack has been permanantly emptied.
 * @note If the stack is empty this function will block until work is pushed. This will only return false
 * when there is no more work, and workstack_no_more_work has been called.
 */
bool workstack_pop(WorkStack* stack, TPoolWork* work_out);

 /**
 * Indicate that no more work will be pushed to the work stack.
 * @param stack Workstack to update.
 */
void workstack_no_more_work(WorkStack* stack);