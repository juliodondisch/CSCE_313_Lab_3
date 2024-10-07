#include "threading.h"
#include <stdio.h>
#include <stdlib.h>


void t_init(void) {
    
    for (int j = 1; j < NUM_CTX; j++) {
        contexts[j].state = INVALID;
        contexts[j].context.uc_stack.ss_sp = NULL;
        contexts[j].context.uc_stack.ss_size = 0;
    }

    current_context_idx = 0;
    contexts[current_context_idx].state = DONE;
}

int32_t t_create(fptr foo, int32_t arg1, int32_t arg2) {

    for (volatile int i = 0; i < NUM_CTX; i++) {
        if (contexts[i].state == INVALID) {
            if (getcontext(&contexts[i].context) == -1) {
                perror("Couldn't get context");
                return 1;
            }
            
            contexts[i].context.uc_stack.ss_sp = malloc(STK_SZ);
            if (contexts[i].context.uc_stack.ss_sp == NULL) {
                perror("Couldn't allocate stack");
                return 1;
            }
            contexts[i].context.uc_stack.ss_size = STK_SZ;
            contexts[i].context.uc_stack.ss_flags = 0;
            contexts[i].context.uc_link = NULL;

            makecontext(&contexts[i].context, (void (*)(void))foo, 2, arg1, arg2);
            contexts[i].state = VALID;

            return 0;
        }
    }
    return 1;
}


int32_t t_yield(void) {

    int next_context_idx = -1;
    for (int i = 1; i <= NUM_CTX; i++) {
        int idx = (current_context_idx + i) % NUM_CTX;
        if (contexts[idx].state == VALID) {
            next_context_idx = idx;
            break;
        }
    }

    if (next_context_idx == -1) {
        return 0;
    }

    int prev_context_idx = current_context_idx;
    current_context_idx = (uint8_t)next_context_idx;


    if (swapcontext(&contexts[prev_context_idx].context, &contexts[current_context_idx].context) == -1) {
        perror("Swapcontext failed");
        return -1;
    }


    int valid_count = 0;
    for (int i = 0; i < NUM_CTX; i++) {
        if (contexts[i].state == VALID && i != current_context_idx) {
            valid_count++;
        }
    }

    return valid_count;
}

void t_finish(void) {

    if (contexts[current_context_idx].state != VALID) {
        fprintf(stderr, "Error, attempting to finish an invalid context\n");
        return;
    }

    if (contexts[current_context_idx].context.uc_stack.ss_sp != NULL) {
        free((char *)contexts[current_context_idx].context.uc_stack.ss_sp - 0xF30);
        contexts[current_context_idx].context.uc_stack.ss_sp = NULL;
        contexts[current_context_idx].context.uc_stack.ss_size = 0;
    }

    contexts[current_context_idx].state = DONE;


    int next_context = -1;
    for (int j = 1; j <= NUM_CTX; j++) {
        int idx = (current_context_idx + j) % NUM_CTX;
        if (contexts[idx].state == VALID) {
            next_context = idx;
            break;
        }
    }

    if (next_context != -1) {
        current_context_idx = (uint8_t)next_context;
        setcontext(&contexts[current_context_idx].context);
    } 
    else {
        printf("Context's finished, exiting!\n");
        exit(0);
    }
}
