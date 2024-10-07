#ifndef COOPERATIVE_MULTITASKING
#define COOPERATIVE_MULTITASKING

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>

#define STK_SZ  4096
#define NUM_CTX 16

enum context_state
{
    INVALID = 0,
    VALID   = 1,
    DONE    = 2,
};

struct worker_context
{
    enum context_state state;
    ucontext_t context;
};

extern struct worker_context contexts[NUM_CTX];
extern uint8_t current_context_idx;

typedef void (*fptr)(int32_t, int32_t);
typedef void (*ctx_ptr)(void);

void t_init(void);
int32_t t_create(fptr foo, int32_t arg1, int32_t arg2);
int32_t t_yield(void);
void t_finish(void);

#endif
