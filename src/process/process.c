#include <rio.h>

/**
 * process_table_init() must be called on startup!
 */

struct ProcessTableEntry g_process_table[N_PROCS];

void process_table_init() {
    for (int32 i = 1; i < N_PROCS; i++) {
        g_process_table[i].ctx = (struct Context) { 0 };
        g_process_table[i].stack_base = NULL;
        g_process_table[i].heap_start = NULL;
        g_process_table[i].name = "";
        g_process_table[i].priority = 0;
        g_process_table[i].n_stack_pages = 0;
        g_process_table[i].n_heap_pages = 0;
        g_process_table[i].sem_id = PROC_NOSEM;
        g_process_table[i].state = PROC_EMPTY;
    }
}

/**
 * Creates a new process using the given parameters and returns its pid.
 * Returns ERR if an error is encountered.
 */

syscall create(void *fn, int32 priority, uint32 n_stack_pages,
                uint32 n_heap_pages, const char *name, int32 n_args, ...) {
    intmask mask = interrupt_save();

    if (fn == NULL || priority < 1 || n_args < 0) {
        interrupt_restore(mask);
        return ERR;
    }

    pid id = get_valid_pid();
    // no valid pid left
    if (id == -1) {
        interrupt_restore(mask);
        return ERR;
    }

    byte *stk = NULL;
    status_code result = kalloc(stk, n_stack_pages);
    if (result == ERR) {
        interrupt_restore(mask);
        return ERR;
    }
    // already 16-byte aligned, move sp to top
    uint64 sp = ((uint64) stk) + (n_stack_pages * PAGE_SIZE);

    // allocate proc heap space as well
    byte *hp = NULL;
    result = kalloc(hp, n_heap_pages);
    if (result == ERR) {
        interrupt_restore(mask);
        return ERR;
    }

    // if n_args > 8, then the rest are placed on the stack in risc-v
    // must be in 16-byte aligned chunks.
    if (n_args > 8) {
        int32 extra = n_args - 8;
        sp -= ((extra * sizeof(uint64) + 15) & ~0xF);
    }

    struct Context *ctx = &g_process_table[id].ctx;
    ctx->ra = (uint64) first_run_wrapper;
    ctx->sp = sp;
    ctx->sstatus = INITIAL_PROCESS_SSTATUS;
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
    g_process_table[id].n_stack_pages = n_stack_pages;
    g_process_table[id].heap_start = hp;
    g_process_table[id].n_heap_pages = n_heap_pages;
    g_process_table[id].name = name;
    g_process_table[id].priority = priority;
    g_process_table[id].state = PROC_SUSP;

    interrupt_restore(mask);
    return id;
}

/**
 * Enqueues a process into the ready list. Returns OK on success and ERR on
 * error.
 */

syscall resume(pid id) {
    intmask mask = interrupt_save();

    if (!validpid(id)) {
        interrupt_restore(mask);
        return ERR;
    }

    status_code code = scheduler_add(id);
    if (code == ERR) {
        interrupt_restore(mask);
        return ERR;
    }

    g_process_table[id].state = PROC_READY;

    // preserves the invariant
    yield();
    
    interrupt_restore(mask);
    return OK;
}

/**
 * Kills the given process.
 */

syscall kill(pid id) {
    intmask mask = interrupt_save();

    if (!validpid(id) || id == 0) { // can't be the NULL proc, so id != 0
        interrupt_restore(mask);
        return ERR;
    }

    kfree(g_process_table[id].stack_base,
                g_process_table[id].n_stack_pages);
    kfree(g_process_table[id].heap_start,
                g_process_table[id].n_heap_pages);

    // TODO: update as more process states are defined
    switch (g_process_table[id].state) {
        case PROC_CURR:
            g_process_table[id].state = PROC_EMPTY;
            yield();
            break;

        case PROC_READY:
            scheduler_remove(id);
            g_process_table[id].state = PROC_EMPTY;
            break;

        case PROC_WAIT:
            sem_abort(g_process_table[id].sem_id, id);
            g_process_table[id].sem_id = PROC_NOSEM;
            g_process_table[id].state = PROC_EMPTY;
            break;

        case PROC_SLEEP:
            sleep_abort(id);
            g_process_table[id].state = PROC_EMPTY;
            break;

        case PROC_EMPTY:
        case PROC_SUSP:
        default:
            g_process_table[id].state = PROC_EMPTY;
    }

    interrupt_restore(mask);
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

