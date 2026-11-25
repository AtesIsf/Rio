#include "check.h"

static volatile int32 g_step;

static process sleeper(uint64 ms) {
    g_step = 1;
    sleep(ms);
    g_step = 2;
    return OK;
}

static process noop(void) {
    return OK;
}

static bool is_empty(pid p) {
    return g_process_table[p].state == PROC_EMPTY;
}

// Starts a priority-100 child that runs until it sleeps.
static pid start_sleeper(uint64 ms) {
    g_step = 0;
    pid p = create(sleeper, 100, 1, 1, "sleeper", 1, ms);
    resume(p);
    return p;
}

static void sleep_zero(void) {
    CHECK("sleep_zero_ok", sleep(0) == OK);
}

static void bad_pid(void) {
    bool ok = sleep_abort(-1) == ERR;
    ok &= sleep_abort(N_PROCS) == ERR;
    ok &= unsleep(-1) == ERR;
    ok &= unsleep(N_PROCS) == ERR;
    CHECK("sleep_fns_reject_bad_pid", ok);
}

static void awake_pid(void) {
    pid q = create(noop, 10, 1, 1, "q", 0);
    bool ok = sleep_abort(q) == ERR;
    ok &= unsleep(q) == ERR;
    kill(q);
    CHECK("sleep_fns_reject_awake", ok);
}

static void block_then_kill(void) {
    pid p = start_sleeper(100000);
    bool blocked = g_step == 1 && g_process_table[p].state == PROC_SLEEP;
    CHECK("sleep_blocks_caller", blocked);

    status_code k = kill(p);
    bool empty = is_empty(p);
    status_code again = sleep_abort(p);
    CHECK("kill_sleeping_leaves_delta",
          k == OK && empty && again == ERR);
}

static void large_ms(void) {
    pid p = start_sleeper(1ULL << 31);
    bool blocked = g_step == 1 && g_process_table[p].state == PROC_SLEEP;
    if (!is_empty(p)) {
        kill(p);
    }
    CHECK("sleep_large_ms_not_truncated", blocked);
}

static void unsleep_wakes(void) {
    pid p = start_sleeper(100000);

    status_code u = unsleep(p);
    yield();
    bool woke = g_step == 2 && is_empty(p);
    if (!is_empty(p)) {
        kill(p);
    }
    CHECK("unsleep_wakes_sleeper", u == OK && woke);
}

static void wakes_after_ms(void) {
    pid p = start_sleeper(5);

    uint64 t0 = read_time();
    while (read_time() - t0 < 50 * TICK_MS);
    yield();
    bool woke = g_step == 2 && is_empty(p);
    if (!is_empty(p)) {
        kill(p);
    }
    CHECK("sleep_wakes_after_ms", woke);
}

void test_timed(void) {
    sleep_zero();
    bad_pid();
    awake_pid();
    block_then_kill();
    large_ms();
    unsleep_wakes();
    wakes_after_ms();
}
