/**
 * Manipulation functions for the scheduler's process queue
 *
 * All of these require the sched_queue_init() function be called first
 */

#include "schedqueue.h"
#include "process.h"

struct SchedQueueEntry g_sched_queue[N_PROCS + 2];

/**
 * Must be called after process_table_init() as it needs an initialized
 * NULL process.
 */

void sched_queue_init() {
    g_sched_queue[SCHED_QUEUE_HEAD].next = SCHED_QUEUE_TAIL;
    g_sched_queue[SCHED_QUEUE_HEAD].prev = SCHED_QUEUE_NULL;
    g_sched_queue[SCHED_QUEUE_HEAD].priority = INT32_MAX;

    g_sched_queue[SCHED_QUEUE_TAIL].prev = SCHED_QUEUE_HEAD;
    g_sched_queue[SCHED_QUEUE_TAIL].next = SCHED_QUEUE_NULL;
    g_sched_queue[SCHED_QUEUE_TAIL].priority = INT32_MIN;

    // the NULL process has the lowest possible priority outside the tail
    sched_queue_enqueue(0, INT32_MIN + 1);
}

bool sched_queue_includes(pid id) {
    if (!validpid(id)) return false;

    pid curr = SCHED_QUEUE_HEAD;
    while (curr != SCHED_QUEUE_TAIL) {
        if (curr == id) {
            return true;
        } 
        curr = g_sched_queue[curr].next;
    }

    return false;
}

/**
 * Returns ERR if the queue is empty, OK otherwise. Removes and
 * places the pid of the head of the queue inside the given parameter.
 */
syscall sched_queue_dequeue(pid *id) {
    if (sched_queue_empty()) return ERR;

    pid target = g_sched_queue[SCHED_QUEUE_HEAD].next;

    g_sched_queue[SCHED_QUEUE_HEAD].next = g_sched_queue[target].next;
    g_sched_queue[g_sched_queue[target].next].prev = SCHED_QUEUE_HEAD;

    *id = target;
    
    return OK;
}

/**
 * Inserts the given process with the given priority into the
 * scheduler queue. Returns ERR if the process is already in the
 * queue or if the pid is invalid, returns OK otherwise.
 */
syscall sched_queue_enqueue(pid id, int32 priority) {
    if (!validpid(id) || sched_queue_includes(id)) {
        return ERR;
    }

    pid curr = SCHED_QUEUE_HEAD;

    // by definition, this will never be more than the head
    // or less than the tail
    while (g_sched_queue[curr].priority >= priority) {
        curr = g_sched_queue[curr].next;
    }

    // insert new pid behind curr
    pid prev_entry = g_sched_queue[curr].prev;

    g_sched_queue[curr].prev = id;
    g_sched_queue[prev_entry].next = id;

    g_sched_queue[id].prev = prev_entry;
    g_sched_queue[id].next = curr;

    return OK;
}

