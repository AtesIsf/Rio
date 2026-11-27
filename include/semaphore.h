#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <lobby.h>
#include <ktypes.h>
#include <process.h>

/* constants */

#ifndef N_SEM
#define N_SEM (64)
#endif

/* struct and definitions */

#define SEMQUEUE_NULL (-1)

enum sem_state : uint8 {
    SEM_FREE = 0,
    SEM_USED = 1,
};

struct SemEntry {
    struct LobbyEntry head;
    int32 count;
    enum sem_state state;
};

extern struct SemEntry g_semaphores[N_SEM];

/* prototypes */

void semaphore_init();

syscall sem_wait(sem id);

syscall sem_release(sem id);

[[nodiscard]]
syscall sem_create(int32 count, sem *id);

syscall sem_free(sem id);

syscall sem_abort(sem id, pid proc_id);

#define validsem(x) (x < N_SEM && x >= 0)

#endif
