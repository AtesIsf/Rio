#include <rio.h>

status_code test_semaphore(void) {
    int32 fails = 0;
    sem s = -1;

    // 1. Parameter validation
    if (sem_create(-1, &s) != ERR) {
        kputs("FAIL: sem_create with negative count did not return ERR");
        fails++;
    }
    if (sem_create(1, NULL) != ERR) {
        kputs("FAIL: sem_create with NULL sem pointer did not return ERR");
        fails++;
    }

    // 2. Normal creation and free
    if (sem_create(2, &s) != OK || !validsem(s)) {
        kputs("FAIL: sem_create failed on valid count");
        fails++;
    }
    if (g_semaphores[s].count != 2 || g_semaphores[s].state != SEM_USED) {
        kputs("FAIL: created semaphore fields uninitialized");
        fails++;
    }
    if (sem_free(s) != OK) {
        kputs("FAIL: sem_free failed on valid semaphore");
        fails++;
    }
    if (sem_free(s) != ERR) {
        kputs("FAIL: double sem_free did not return ERR");
        fails++;
    }
    if (sem_free(-1) != ERR || sem_free(N_SEM) != ERR) {
        kputs("FAIL: sem_free with out-of-bounds sem did not return ERR");
        fails++;
    }

    // 3. Counter operations without blocking
    sem_create(2, &s);
    sem_wait(s);
    if (g_semaphores[s].count != 1) {
        kputs("FAIL: sem_wait did not decrement count to 1");
        fails++;
    }
    sem_wait(s);
    if (g_semaphores[s].count != 0) {
        kputs("FAIL: sem_wait did not decrement count to 0");
        fails++;
    }
    sem_release(s);
    if (g_semaphores[s].count != 1) {
        kputs("FAIL: sem_release did not increment count to 1");
        fails++;
    }
    sem_free(s);

    // 4. sem_abort validation
    if (sem_abort(-1, 1) != ERR || sem_abort(N_SEM, 1) != ERR) {
        kputs("FAIL: sem_abort with out-of-bounds sem did not return ERR");
        fails++;
    }
    if (sem_abort(s, -1) != ERR || sem_abort(s, N_PROCS) != ERR) {
        kputs("FAIL: sem_abort with out-of-bounds pid did not return ERR");
        fails++;
    }

    // 5. Semaphore capacity exhaustion
    sem allocated[N_SEM];
    int32 count = 0;
    for (int32 i = 0; i < N_SEM; i++) {
        if (sem_create(1, &allocated[i]) == OK) {
            count++;
        }
    }
    if (count != N_SEM) {
        kputs("FAIL: could not allocate up to N_SEM semaphores");
        fails++;
    }
    sem overflow = -1;
    if (sem_create(1, &overflow) != ERR) {
        kputs("FAIL: sem_create did not return ERR when exhausted");
        fails++;
    }
    for (int32 i = 0; i < count; i++) {
        sem_free(allocated[i]);
    }

    if (fails == 0) {
        kputs("PASS: test_semaphore");
        return OK;
    }
    return ERR;
}
