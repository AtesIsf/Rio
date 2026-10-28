#include <rio.h>

struct SemEntry g_semaphores[N_SEM];

void semaphore_init() {
    for (int32 i = 0; i < N_SEM; i++) {
        g_semaphores[i].count = 0;
        g_semaphores[i].head = SEMQUEUE_NULL;
        g_semaphores[i].state = SEM_FREE;
    } 
}

syscall sem_wait(sem id) {
    intmask mask = interrupt_save();

    if (!validsem(id)) {
        interrupt_restore(mask);
        return ERR;
    }

    interrupt_restore(mask);
    return OK;
}

syscall sem_release(sem id) {
    intmask mask = interrupt_save();

    if (!validsem(id)) {
        interrupt_restore(mask);
        return ERR;
    }

    interrupt_restore(mask);
    return OK;
}

syscall sem_create(int32 count) {
    intmask mask = interrupt_save();

    if (count < 0) {
        interrupt_restore(mask);
        return ERR;
    }

    interrupt_restore(mask);
    return OK;

}

syscall sem_free(sem id) {
    intmask mask = interrupt_save();

    if (!validsem(id)) {
        interrupt_restore(mask);
        return ERR;
    }

    interrupt_restore(mask);
    return OK;
}

