#include "work_stack.h"
#include <stdlib.h>

WorkStack* workstack_alloc(unsigned int stack_size) {
    WorkStack* stack = malloc(sizeof(WorkStack));
    if (stack == NULL) return NULL;
    stack->stack_size = stack_size;
    stack->stack = malloc(stack_size * sizeof(TPoolWork));
    stack->stack_next = 0;
    pthread_mutex_init(&stack->stack_lock, NULL);
    pthread_cond_init(&stack->work_popped, NULL);
    pthread_cond_init(&stack->work_pushed, NULL);
    if (stack->stack == NULL) return NULL;
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
    stack->stack[stack->stack_next] = work;
    stack->stack_next++;

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
    stack->stack_next--;
    *work_out = stack->stack[stack->stack_next];

    pthread_cond_signal(&stack->work_popped);
    pthread_mutex_unlock(&stack->stack_lock);
    return true;
}

void workstack_no_more_work(WorkStack* stack) {
    pthread_mutex_lock(&stack->stack_lock);
    stack->no_more_work = true;
    // Signal to all workers to wake up so they realise no more work's coming
    // all workers that are already running will realise there's no more work when
    // they try to wait for more work next
    pthread_cond_broadcast(&stack->work_pushed);
    pthread_mutex_unlock(&stack->stack_lock);
}