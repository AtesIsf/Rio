#include "check.h"

static volatile int32 g_log[8];
static volatile int32 g_nlog;

static process noop(void) {
    return OK;
}

static void log_tag(int32 tag) {
    g_log[g_nlog++] = tag;
}

static process logger(uint64 tag) {
    log_tag((int32) tag);
    return OK;
}

// logs, yields and logs again with interrupts off so only yield switches
static process rr(uint64 tag) {
    intmask mask = interrupt_save();
    log_tag((int32) tag);
    yield();
    log_tag((int32) tag);
    interrupt_restore(mask);
    return OK;
}

static process starter(uint64 a, uint64 b, uint64 c) {
    resume((pid) a);
    resume((pid) b);
    if (c != 0) {
        resume((pid) c);
    }
    return OK;
}

static void bad_pid(void) {
    bool ok = scheduler_add(-1) == ERR;
    ok &= scheduler_add(N_PROCS) == ERR;
    ok &= scheduler_remove(-1) == ERR;
    ok &= scheduler_remove(N_PROCS) == ERR;
    CHECK("sched_rejects_bad_pid", ok);
}

static void dup_add_absent_remove(void) {
    pid q = create(noop, 10, 1, 1, "q", 0);

    status_code a1 = scheduler_add(q);
    status_code a2 = scheduler_add(q);
    status_code r1 = scheduler_remove(q);
    status_code r2 = scheduler_remove(q);
    kill(q);

    CHECK("sched_dup_add_absent_remove",
          a1 == OK && a2 == ERR && r1 == OK && r2 == ERR);
}

static void highest_priority_first(void) {
    g_nlog = 0;
    pid lo = create(logger, 80, 1, 1, "lo", 1, (uint64) 80);
    pid hi = create(logger, 100, 1, 1, "hi", 1, (uint64) 100);
    pid mid = create(logger, 90, 1, 1, "mid", 1, (uint64) 90);
    pid s = create(starter, 200, 1, 1, "start", 3,
                   (uint64) lo, (uint64) hi, (uint64) mid);

    resume(s);

    CHECK("sched_highest_priority_first",
          g_nlog == 3 && g_log[0] == 100 && g_log[1] == 90 &&
          g_log[2] == 80);
}

static void round_robin(void) {
    g_nlog = 0;
    pid a = create(rr, 85, 1, 1, "rrA", 1, (uint64) 'A');
    pid b = create(rr, 85, 1, 1, "rrB", 1, (uint64) 'B');
    pid s = create(starter, 200, 1, 1, "start", 3,
                   (uint64) a, (uint64) b, (uint64) 0);

    resume(s);

    CHECK("sched_round_robin_equal_prio",
          g_nlog == 4 && g_log[0] == 'A' && g_log[1] == 'B' &&
          g_log[2] == 'A' && g_log[3] == 'B');
}

void test_scheduler(void) {
    bad_pid();
    dup_add_absent_remove();
    highest_priority_first();
    round_robin();
}
