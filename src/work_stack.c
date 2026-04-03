// Authors
// - Colin Simon-Fellowes
// - Alex Robertson

#include "work_stack.h"
#include <stdlib.h>

WorkStack* workstack_alloc(unsigned int stack_size) {
    // Allocate memory
    WorkStack* stack = malloc(sizeof(WorkStack));
    if (stack == NULL) return NULL;

    stack->stack = malloc(stack_size * sizeof(TPoolWork));
    if (stack->stack == NULL) {
        workstack_dealloc(stack);
        return NULL;
    }

    // Initialise stack params, cond vars, and mutex
    stack->stack_size = stack_size;
    stack->stack_next = 0;

    // Always returns 0 so no error checking needed
    pthread_mutex_init(&stack->stack_lock, NULL);
    pthread_cond_init(&stack->work_popped, NULL);
    pthread_cond_init(&stack->work_pushed, NULL);

    return stack;
}

void workstack_dealloc(WorkStack* stack) {
    if (stack != NULL) {
        free(stack->stack);
        pthread_mutex_destroy(&stack->stack_lock);
        pthread_cond_destroy(&stack->work_popped);
        pthread_cond_destroy(&stack->work_pushed);
    }
    free(stack);
}

void workstack_push(WorkStack* stack, TPoolWork work) {
    pthread_mutex_lock(&stack->stack_lock);

    // Wait for there to be a free space in the stack
    while (stack->stack_next >= stack->stack_size) {
        pthread_cond_wait(&stack->work_popped, &stack->stack_lock);
    }

    // Push work onto the stack now we know there's space
    stack->stack[stack->stack_next] = work;
    stack->stack_next++;

    // Signal one waiter that there's now work it can get
    pthread_cond_signal(&stack->work_pushed);
    pthread_mutex_unlock(&stack->stack_lock);
}

bool workstack_pop(WorkStack* stack, TPoolWork* work_out) {
    pthread_mutex_lock(&stack->stack_lock);

    // Wait for there to be work in the stack
    while (stack->stack_next == 0) {
        // Make sure more work's actually coming
        if (stack->no_more_work) {
            pthread_mutex_unlock(&stack->stack_lock);
            return false;
        }
        pthread_cond_wait(&stack->work_pushed, &stack->stack_lock);
    }
    // Pop work off the stack now we know there is some work there
    stack->stack_next--;
    *work_out = stack->stack[stack->stack_next];

    // Signal that work's been popped in case the stack was full and a thread is waiting
    // to push more work
    pthread_cond_signal(&stack->work_popped);
    pthread_mutex_unlock(&stack->stack_lock);
    return true;
}

void workstack_no_more_work(WorkStack* stack) {
    pthread_mutex_lock(&stack->stack_lock);
    stack->no_more_work = true;
    // Signal to all sleeping workers to wake up so they realise no more work's coming.
    // All workers that are already running will realise there's no more work when
    // they next try to wait for more work
    pthread_cond_broadcast(&stack->work_pushed);
    pthread_mutex_unlock(&stack->stack_lock);
}