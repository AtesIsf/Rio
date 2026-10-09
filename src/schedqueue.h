#ifndef SCHED_QUEUE_H
#define SCHED_QUEUE_H

#include "ktypes.h"
#include "process.h"

// Ready process queue
struct SchedQueueEntry {
    int32 priority;
    pid next;
    pid prev;
};

#ifndef N_PROCS
#define N_PROCS (64)
#endif

extern struct SchedQueueEntry g_sched_queue[N_PROCS + 2];

#define SCHED_QUEUE_HEAD (N_PROCS + 1)
#define SCHED_QUEUE_TAIL (N_PROCS)
#define SCHED_QUEUE_NULL (-1)

// function prototypes

void sched_queue_init();

bool sched_queue_includes(pid id);

syscall sched_queue_dequeue(pid *id);

syscall sched_queue_enqueue(pid id, int32 priority);

void sched_queue_debug();

// macros

#define sched_queue_empty() ( \
    g_sched_queue[SCHED_QUEUE_HEAD].next == SCHED_QUEUE_TAIL && \
    g_sched_queue[SCHED_QUEUE_TAIL].prev == SCHED_QUEUE_HEAD )

#endif
