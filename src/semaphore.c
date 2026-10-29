#include <rio.h>

struct SemEntry g_semaphores[N_SEM];

void semaphore_init() {
    for (int32 i = 0; i < N_SEM; i++) {
        g_semaphores[i].head = (struct LobbyEntry) {
            .next = LOBBY_NULL, .delta = 0
        };
        g_semaphores[i].count = 0;
        g_semaphores[i].state = SEM_FREE;
    } 
}

syscall sem_wait(sem id) {
    intmask mask = interrupt_save();

    if (!validsem(id)) {
        interrupt_restore(mask);
        return ERR;
    }

    struct SemEntry *s = &g_semaphores[id];
    if (s->state == SEM_FREE) {
        interrupt_restore(mask);
        return ERR;
    }

    if (--(s->count) < 0) {
        struct ProcessTableEntry *proc = &g_process_table[curr_pid];
        proc->state = PROC_WAIT;
        proc->sem_id = id;
        lobby_enqueue(&s->head, curr_pid, 0); 
        yield();
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

    struct SemEntry *s = &g_semaphores[id];
    if (s->state == SEM_FREE) {
        interrupt_restore(mask);
        return ERR;
    }

    if ((s->count)++ < 0) {
        pid proc = 0;
        status_code result = lobby_dequeue(&s->head, &proc);
        if (result != ERR) {
            resume(proc);
        }
    }

    interrupt_restore(mask);
    return OK;
}

syscall sem_create(int32 count, sem *id) {
    intmask mask = interrupt_save();

    if (count < 0 || id == NULL) {
        interrupt_restore(mask);
        return ERR;
    }

    sem found_sem = -1;
    for (int32 i = 0; i < N_SEM; i++) {
        if (g_semaphores[i].state == SEM_FREE) {
            found_sem = i;
            break;
        }
    }

    // no free entries
    if (found_sem == -1) {
        interrupt_restore(mask);
        return ERR;
    }

    g_semaphores[found_sem].state = SEM_USED;
    g_semaphores[found_sem].count = count;
    *id = found_sem;

    interrupt_restore(mask);
    return OK;
}

syscall sem_free(sem id) {
    intmask mask = interrupt_save();

    if (!validsem(id) || g_semaphores[id].state == SEM_FREE) {
        interrupt_restore(mask);
        return ERR;
    }

    // otherwise, each resume calls yield and messes up the system
    defer_yield = true;
    pid proc = 0;
    while (lobby_dequeue(&g_semaphores[id].head, &proc) == OK) {
        g_process_table[proc].sem_id = -1;
        resume(proc);
    }
    defer_yield = false;

    g_semaphores[id].state = SEM_FREE;
    g_semaphores[id].count = 0;

    interrupt_restore(mask);
    return OK;
}

