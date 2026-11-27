#include "check.h"

static volatile bool g_args_ok;
static volatile bool g_ran;

static process noop(void) {
    return OK;
}

static process mark(void) {
    g_ran = true;
    return OK;
}

static process args10(uint64 a0, uint64 a1, uint64 a2, uint64 a3,
                      uint64 a4, uint64 a5, uint64 a6, uint64 a7,
                      uint64 a8, uint64 a9) {
    g_args_ok = a0 == 1 && a1 == 2 && a2 == 3 && a3 == 4 && a4 == 5 &&
                a5 == 6 && a6 == 7 && a7 == 8 && a8 == 9 && a9 == 10;
    return OK;
}

static void bad_args(void) {
    bool ok = create(nullptr, 10, 1, 1, "x", 0) == ERR;
    ok &= create(noop, 0, 1, 1, "x", 0) == ERR;
    ok &= create(noop, -5, 1, 1, "x", 0) == ERR;
    ok &= create(noop, 10, 1, 1, "x", -1) == ERR;
    CHECK("create_rejects_bad_args", ok);
}

static void suspended_setup_and_kill(void) {
    pid p = create(noop, 42, 2, 3, "p2", 0);
    struct ProcessTableEntry *e = &g_process_table[p];
    byte *stk = e->stack_base;
    byte *hp = e->heap_start;
    bool valid = validpid(p) && p != 0;

    bool fields = e->state == PROC_SUSP && e->priority == 42 &&
                  e->n_stack_pages == 2 && e->n_heap_pages == 3 &&
                  e->name[0] == 'p' && e->name[1] == '2';
    bool aligned = stk != nullptr && hp != nullptr &&
                   (uint64) stk % PAGE_SIZE == 0 &&
                   (uint64) hp % PAGE_SIZE == 0;
    bool disjoint = stk + 2 * PAGE_SIZE <= hp || hp + 3 * PAGE_SIZE <= stk;
    bool queued = sched_queue_includes(p);
    CHECK("create_sets_up_suspended",
          valid && fields && aligned && disjoint && !queued);

    status_code k = kill(p);
    bool empty = e->state == PROC_EMPTY;
    // double free is ERR; a leaked block would be reclaimed (OK) instead
    status_code f1 = kfree(stk, 2);
    status_code f2 = kfree(hp, 3);
    CHECK("kill_suspended_frees_pages",
          k == OK && empty && f1 == ERR && f2 == ERR);
}

static void resume_rejects_non_suspended(void) {
    bool ok = resume(-1) == ERR;
    ok &= resume(N_PROCS) == ERR;
    ok &= resume(curr_pid) == ERR;
    ok &= resume(get_valid_pid()) == ERR;
    CHECK("resume_rejects_non_suspended", ok);
}

static void kill_rejects_bad_targets(void) {
    bool ok = kill(-1) == ERR;
    ok &= kill(N_PROCS) == ERR;
    ok &= kill(0) == ERR;
    ok &= kill(get_valid_pid()) == ERR;
    CHECK("kill_rejects_bad_targets", ok);
}

static void args_and_exit(void) {
    g_args_ok = false;
    pid p = create(args10, 100, 1, 1, "a10", 10, (uint64) 1, (uint64) 2,
                   (uint64) 3, (uint64) 4, (uint64) 5, (uint64) 6,
                   (uint64) 7, (uint64) 8, (uint64) 9, (uint64) 10);
    byte *stk = g_process_table[p].stack_base;
    byte *hp = g_process_table[p].heap_start;

    resume(p); // priority 100 preempts kmain and runs to completion
    bool args = g_args_ok;
    bool empty = g_process_table[p].state == PROC_EMPTY;
    status_code f1 = kfree(stk, 1);
    status_code f2 = kfree(hp, 1);

    CHECK("resume_preempts_passes_10_args", args);
    CHECK("exit_frees_slot_and_pages",
          empty && f1 == ERR && f2 == ERR);
}

static void kill_ready_dequeues(void) {
    g_ran = false;
    pid q = create(mark, 10, 1, 1, "mark", 0);

    status_code r1 = resume(q);
    bool ready = g_process_table[q].state == PROC_READY;
    bool queued = sched_queue_includes(q);
    bool not_run = !g_ran;
    status_code r2 = resume(q);
    status_code k = kill(q);
    bool empty = g_process_table[q].state == PROC_EMPTY;
    bool gone = !sched_queue_includes(q);

    CHECK("kill_ready_dequeues",
          r1 == OK && ready && queued && not_run && r2 == ERR &&
          k == OK && empty && gone && !g_ran);
}

static void table_exhaustion(void) {
    pid ids[N_PROCS];
    int32 free_slots = 0;
    for (int32 i = 0; i < N_PROCS; i++) {
        free_slots += g_process_table[i].state == PROC_EMPTY;
    }

    int32 made = 0;
    for (; made < free_slots; made++) {
        ids[made] = create(noop, 10, 1, 1, "fill", 0);
        if (ids[made] == ERR) break;
    }
    status_code extra = create(noop, 10, 1, 1, "extra", 0);
    for (int32 i = 0; i < made; i++) {
        kill(ids[i]);
    }

    CHECK("proc_table_exhaustion",
          free_slots > 0 && made == free_slots && extra == ERR);
}

// shortcut: assumes first-fit puts the heap on the lowest free page
static void create_failure_leaks_heap(void) {
    byte *a = kalloc(1);
    kfree(a, 1);

    status_code r = create(noop, 42, 0, 1, "nostack", 0);
    bool leaked = kfree(a, 1) == OK; // also reclaims the leak

    CHECK("create_fail_no_heap_leak", r == ERR && !leaked);
}

void test_process(void) {
    bad_args();
    suspended_setup_and_kill();
    resume_rejects_non_suspended();
    kill_rejects_bad_targets();
    args_and_exit();
    kill_ready_dequeues();
    table_exhaustion();
    create_failure_leaks_heap();
}
