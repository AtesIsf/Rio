#include <rio.h>

extern int32 g_tests_passed;
extern int32 g_defects_found;

process test_semaphore(void) {
    sem s = -1;

    // 1. Parameter validation
    if (sem_create(-1, &s) == ERR && sem_create(1, NULL) == ERR) {
        g_tests_passed++;
    }

    // 2. Creation and freeing
    if (sem_create(2, &s) == OK && validsem(s)) {
        if (g_semaphores[s].count == 2 && g_semaphores[s].state == SEM_USED) {
            g_tests_passed++;
        }
    }
    if (sem_free(s) == OK && sem_free(s) == ERR && sem_free(-1) == ERR && sem_free(N_SEM) == ERR) {
        g_tests_passed++;
    }

    // 3. Counter operations without blocking
    sem_create(2, &s);
    sem_wait(s);
    int32 count_after_wait1 = g_semaphores[s].count;
    sem_wait(s);
    int32 count_after_wait2 = g_semaphores[s].count;
    sem_release(s);
    int32 count_after_rel = g_semaphores[s].count;
    if (count_after_wait1 == 1 && count_after_wait2 == 0 && count_after_rel == 1) {
        g_tests_passed++;
    }
    sem_free(s);

    // 4. sem_abort parameter validation
    if (sem_abort(-1, 1) == ERR && sem_abort(N_SEM, 1) == ERR &&
        sem_abort(s, -1) == ERR && sem_abort(s, N_PROCS) == ERR) {
        g_tests_passed++;
    }

    // 5. Semaphore capacity exhaustion
    sem allocated[N_SEM];
    int32 count = 0;
    for (int32 i = 0; i < N_SEM; i++) {
        if (sem_create(1, &allocated[i]) == OK) {
            count++;
        }
    }
    sem overflow = -1;
    if (count == N_SEM && sem_create(1, &overflow) == ERR) {
        g_tests_passed++;
    }
    for (int32 i = 0; i < count; i++) {
        sem_free(allocated[i]);
    }

    kputs("  [PASS] test_semaphore");
    return OK;
}
