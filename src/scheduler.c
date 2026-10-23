#include "rio.h"

pid curr_pid;

/**
 * Switches context to the next process in the schedqueue
 *
 * Upholds the same scheduling invariant as Xinu:
 * "The scheduler always runs the process with the highest priority,
 * and scheduling is round-robin between processes of equal priority."
 *
 * Return OK on success and ERR if an error is encountered.
 */

syscall yield() {
    intmask mask = interrupt_save();

    // This could only happen if the queue isn't initialized or if the
    // NULL process has been removed for whatever reason (which should
    // never happen).
    if (sched_queue_empty()) {
        interrupt_restore(mask);
        return ERR;
    }

    if (g_process_table[curr_pid].state != PROC_EMPTY) {
        sched_queue_enqueue(curr_pid, g_process_table[curr_pid].priority);
        g_process_table[curr_pid].state = PROC_READY;
    }

    pid new_id;
    status_code ret_val = sched_queue_dequeue(&new_id);
    if (ret_val == ERR) {
        interrupt_restore(mask);
        return ERR;
    }
    g_process_table[new_id].state = PROC_CURR;

    struct Context *new_ctx = &g_process_table[new_id].ctx;

    // update global curr_pid
    pid old_pid = curr_pid;
    curr_pid = new_id;

    cswtch(&g_process_table[old_pid].ctx, new_ctx);

    interrupt_restore(mask);
    return OK;
}

