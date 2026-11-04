#include <rio.h>

extern int32 g_tests_passed;
extern int32 g_defects_found;

static volatile int32 g_worker_step = 0;
static sem g_sync_sem = -1;

process worker_step(void) {
    g_worker_step = 42;
    return OK;
}

process sem_consumer(void) {
    g_worker_step = 1;
    sem_wait(g_sync_sem);
    g_worker_step = 2;
    return OK;
}

process wait_forever(void) {
    sem_wait(g_sync_sem);
    return OK;
}

process sleep_forever(void) {
    sleep(100000);
    return OK;
}

process test_integration(void) {
    // 1. Cooperative process creation, execution, and termination
    g_worker_step = 0;
    pid w1 = create(worker_step, 75, "w1", 0);
    if (validpid(w1)) {
        resume(w1);
        if (g_worker_step == 42 && g_process_table[w1].state == PROC_EMPTY) {
            g_tests_passed++;
        }
    }

    // 2. Cross-process semaphore blocking and waking
    g_worker_step = 0;
    sem_create(0, &g_sync_sem);
    pid w2 = create(sem_consumer, 75, "w2", 0);
    resume(w2);

    if (g_worker_step == 1 && g_process_table[w2].state == PROC_WAIT && g_semaphores[g_sync_sem].count == -1) {
        g_tests_passed++;
    }

    sem_release(g_sync_sem);
    if (g_worker_step == 2 && g_process_table[w2].state == PROC_EMPTY) {
        g_tests_passed++;
    }
    sem_free(g_sync_sem);

    // 3. kill() unblocks and restores semaphore count via sem_abort
    sem_create(0, &g_sync_sem);
    pid w3 = create(wait_forever, 75, "w3", 0);
    resume(w3);

    kill(w3);
    if (g_semaphores[g_sync_sem].count == 0 && g_process_table[w3].state == PROC_EMPTY) {
        g_tests_passed++;
    }
    sem_free(g_sync_sem);

    // 4. kill() cleans up sleeping process from delta list via sleep_abort
    pid w4 = create(sleep_forever, 75, "w4", 0);
    resume(w4);

    if (g_process_table[w4].state == PROC_SLEEP) {
        g_tests_passed++;
    }
    kill(w4);
    if (g_process_table[w4].state == PROC_EMPTY && sleep_abort(w4) == ERR) {
        g_tests_passed++;
    }

    kputs("  [PASS] test_integration");
    return OK;
}
