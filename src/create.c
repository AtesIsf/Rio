/**
 * create.c -- the function to create a process
 */

#include "create.h"

#include "process.h"
#include "memory.h"

#include <stdarg.h>

extern void first_run_wrapper(void);

/**
 * Creates a new process using the given parameters and returns its pid.
 * Returns ERR if an error is encountered.
 */

syscall create(void *fn, int32 priority, const char *name, int32 n_args, ...) {
    // TODO: disable interrupts here after you implement it

    if (fn == NULL || priority < 1 || n_args < 0) {
        return ERR;
    }

    pid id = get_valid_pid();
    // no valid pid left
    if (id == -1) {
        return ERR;
    }

    byte *stk = get_stack();
    if (stk == NULL) {
        return ERR;
    }
    // already 16-byte aligned
    uint64 sp = (uint64) stk;

    // if n_args > 8, then the rest are placed on the stack in risc-v
    // must be in 16-byte aligned chunks.
    if (n_args > 8) {
        int32 extra = n_args - 8;
        sp -= ((extra * sizeof(uint64) + 15) & ~0xF);
    }

    struct Context *ctx = &g_process_table[id].ctx;
    ctx->ra = (uint64) first_run_wrapper;
    ctx->sp = sp;
    ctx->s0 = (uint64) fn;
    ctx->s1 = 0;
    ctx->s2 = 0;
    ctx->s3 = 0;
    ctx->s4 = 0;
    ctx->s5 = 0;
    ctx->s6 = 0;
    ctx->s7 = 0;
    ctx->s8 = 0;
    ctx->s9 = 0;
    ctx->s10 = 0;
    ctx->s11 = 0;

    va_list ap;
    va_start(ap, n_args);

    uint64 *stack_args = (uint64 *) sp;
    for (int32 i = 0; i < n_args; i++) {
        uint64 arg = va_arg(ap, uint64);
        switch (i) {
            case 0:
                ctx->s1 = arg;
                break;
            case 1:
                ctx->s2 = arg;
                break;
            case 2:
                ctx->s3 = arg;
                break;
            case 3:
                ctx->s4 = arg;
                break;
            case 4:
                ctx->s5 = arg;
                break;
            case 5:
                ctx->s6 = arg;
                break;
            case 6:
                ctx->s7 = arg;
                break;
            case 7:
                ctx->s8 = arg;
                break;
            default:
                stack_args[i - 8] = arg;
                break;
        }
    }
    va_end(ap);

    g_process_table[id].stack_base = stk;
    g_process_table[id].name = name;
    g_process_table[id].priority = priority;
    g_process_table[id].state = PROC_SUSP;

    return id;
}
