#include <pthread.h>
#include <stdbool.h>

typedef struct TPoolArgs {

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

WorkStack* workstack_alloc(unsigned int stack_size);
void workstack_dealloc(WorkStack* stack);

// Blocks until there is capacity
void workstack_push(WorkStack* stack, TPoolWork work);
// Blocks until there is work to return, returns false if there is not going to be any more work
bool workstack_pop(WorkStack* stack, TPoolWork* work_out);

void workstack_no_more_work(WorkStack* stack);