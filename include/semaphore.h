#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <ktypes.h>
#include <process.h>

/* constants */

#ifndef N_SEM
#define N_SEM (64)
#endif

/* struct and definitions */

#define SEMQUEUE_NULL (-1)

#define SEM_FREE (0)
#define SEM_USED (1)

struct SemEntry {
    int32 count;
    int32 head;
    int32 state;
};

struct SemQueueNode {
    int32 next;
};

extern struct SemEntry g_semaphores[N_SEM];

/* prototypes */

void semaphore_init();

syscall sem_wait(sem id);

syscall sem_release(sem id);

syscall sem_create(int32 count);

syscall sem_free(sem id);

#define validsem(x) (x < N_SEM && x >= 0)

#endif
