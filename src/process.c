#include "process.h"
#include "create.h"
#include "kprint.h"
#include "schedqueue.h"

/**
 * process_table_init() must be called on startup!
 */

struct ProcessTableEntry g_process_table[N_PROCS];

void process_table_init() {
    for (int32 i = 0; i < N_PROCS; i++) {
        g_process_table[i].state = PROC_EMPTY;
        g_process_table[i].name = "";
    }

    // this should result in NULL pid = 0
    resume(create(null_process, 1, "NULL", 0));
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

process null_process() {
    // TODO: You may define a debug flag and remove this unless debugging
    kputs("NULL Process");
    while (true);
    return OK;
}

