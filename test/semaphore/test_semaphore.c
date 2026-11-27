#include "check.h"

static volatile int32 g_step;

static process waiter(uint64 s) {
    g_step = 1;
    sem_wait((sem) s);
    g_step = 2;
    return OK;
}

static bool is_empty(pid p) {
    return g_process_table[p].state == PROC_EMPTY;
}

// Creates a semaphore with count 0 and a priority-100 child blocked on it.
static pid start_waiter(sem *s) {
    g_step = 0;
    (void) sem_create(0, s);
    pid w = create(waiter, 100, 1, 1, "waiter", 1, (uint64) *s);
    resume(w);
    return w;
}

static void bad_create(void) {
    sem s = -1;
    bool ok = sem_create(-1, &s) == ERR;
    ok &= sem_create(0, nullptr) == ERR;
    CHECK("sem_create_rejects_bad_args", ok);
}

static void bad_or_freed_id(void) {
    sem s = -1;
    (void) sem_create(1, &s);
    sem_free(s);

    bool ok = true;
    sem ids[3] = { -1, N_SEM, s };
    for (int32 i = 0; i < 3; i++) {
        ok &= sem_wait(ids[i]) == ERR;
        ok &= sem_release(ids[i]) == ERR;
        ok &= sem_free(ids[i]) == ERR;
        ok &= sem_abort(ids[i], 0) == ERR;
    }
    CHECK("sem_ops_reject_bad_or_freed_id", ok);
}

static void counts_without_blocking(void) {
    sem s = -1;
    (void) sem_create(2, &s);

    status_code w1 = sem_wait(s);
    status_code w2 = sem_wait(s);
    int32 zero = g_semaphores[s].count;
    status_code r = sem_release(s);
    int32 one = g_semaphores[s].count;
    sem_free(s);

    CHECK("sem_counts_without_blocking",
          w1 == OK && w2 == OK && zero == 0 && r == OK && one == 1);
}

static void table_exhaustion(void) {
    sem ids[N_SEM];
    int32 free_slots = 0;
    for (int32 i = 0; i < N_SEM; i++) {
        free_slots += g_semaphores[i].state == SEM_FREE;
    }

    int32 made = 0;
    while (made < free_slots && sem_create(0, &ids[made]) == OK) {
        made++;
    }
    sem extra;
    status_code over = sem_create(0, &extra);
    for (int32 i = 0; i < made; i++) {
        sem_free(ids[i]);
    }

    CHECK("sem_table_exhaustion",
          free_slots > 0 && made == free_slots && over == ERR);
}

static void wait_blocks_at_zero(void) {
    sem s;
    pid w = start_waiter(&s);

    bool blocked = g_step == 1 &&
                   g_process_table[w].state == PROC_WAIT &&
                   g_semaphores[s].count == -1;
    kill(w);
    sem_free(s);

    CHECK("sem_wait_blocks_at_zero", blocked);
}

static void release_wakes_waiter(void) {
    sem s;
    pid w = start_waiter(&s);

    sem_release(s);
    yield();
    bool woke = g_step == 2 && is_empty(w);
    if (!is_empty(w)) {
        kill(w);
    }
    sem_free(s);

    CHECK("sem_release_wakes_waiter", woke);
}

static void abort_removes_without_grant(void) {
    sem s;
    pid w = start_waiter(&s);

    status_code a1 = sem_abort(s, w);
    int32 count = g_semaphores[s].count;
    int32 step = g_step;
    status_code a2 = sem_abort(s, w);
    status_code a3 = sem_abort(s, curr_pid);
    kill(w);
    sem_free(s);

    CHECK("sem_abort_removes_no_grant",
          a1 == OK && count == 0 && step == 1 && a2 == ERR && a3 == ERR);
}

static void kill_waiting_restores_count(void) {
    sem s;
    pid w = start_waiter(&s);

    status_code k = kill(w);
    bool empty = is_empty(w);
    int32 count = g_semaphores[s].count;
    sem_release(s);
    int32 after = g_semaphores[s].count;
    sem_free(s);

    CHECK("kill_waiting_restores_count",
          k == OK && empty && count == 0 && after == 1);
}

static void free_wakes_waiters(void) {
    sem s;
    pid w = start_waiter(&s);

    status_code f = sem_free(s);
    yield();
    bool woke = g_step == 2 && is_empty(w);
    bool rejected = sem_wait(s) == ERR;
    if (!is_empty(w)) {
        kill(w);
    }

    CHECK("sem_free_wakes_waiters", f == OK && woke && rejected);
}

void test_semaphore(void) {
    bad_create();
    bad_or_freed_id();
    counts_without_blocking();
    table_exhaustion();
    wait_blocks_at_zero();
    release_wakes_waiter();
    abort_removes_without_grant();
    kill_waiting_restores_count();
    free_wakes_waiters();
}
