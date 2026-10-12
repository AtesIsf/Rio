#include "process.h"

#include "memory.h"
#include "schedqueue.h"
#include "scheduler.h"

#include <stdarg.h>

// the cswtch() wrapper used by create
extern void first_run_wrapper(void);

/**
 * process_table_init() must be called on startup!
 */

struct ProcessTableEntry g_process_table[N_PROCS];

void process_table_init() {
    for (int32 i = 1; i < N_PROCS; i++) {
        g_process_table[i].state = PROC_EMPTY;
        g_process_table[i].name = "";
    }
}

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

/**
 * Enqueues a process into the ready list. Returns OK on success and ERR on
 * error.
 */

syscall resume(pid id) {
    // TODO: disable interrupts after you implement it

    if (!validpid(id)) {
        return ERR;
    }

    status_code code = sched_queue_enqueue(id, g_process_table[id].priority);
    if (code == ERR) {
        return ERR;
    }

    g_process_table[id].state = PROC_READY;
    
    return OK;
}

/**
 * Kills the given process.
 */

syscall kill(pid id) {
    // TODO: disable interrupts after you implement it
    if (!validpid(id) || id == 0) { // can't be the NULL proc, so id != 0
        return ERR;
    }

    free_stack(g_process_table[id].stack_base);

    // TODO: update as more process states are defined
    switch (g_process_table[id].state) {
        case PROC_CURR:
            g_process_table[id].state = PROC_EMPTY;
            yield();
            break;

        case PROC_READY:
            sched_queue_remove(id);
            g_process_table[id].state = PROC_EMPTY;
            break;

        case PROC_SLEEP:
        case PROC_WAIT:
            // TODO: semaphore handling later
        case PROC_EMPTY:
        case PROC_SUSP:
        default:
            g_process_table[id].state = PROC_EMPTY;
    }

    return OK;
}

/**
 * Gets called when a process terminates
 */

void procend() {
    kill(curr_pid);
}

int32 get_valid_pid() {
    int32 curr = -1;

    for (int32 i = 0; i < N_PROCS; i++) {
        if (g_process_table[i].state != PROC_EMPTY) {
            continue;
        }
        curr = i;
        break;
    }

    return curr;
}

