/**
 * kmain is the main process created by the entry point. It is basically
 * a main function for the kernel, but running as a process as well.
 */

#include <rio.h>

static int tests_run = 0;
static int tests_passed = 0;

static void assert_test(bool condition, const char *test_name) {
    tests_run++;
    if (condition) {
        tests_passed++;
        kprintf("  [PASS] %s\n", test_name);
    } else {
        kprintf("  [FAIL] %s\n", test_name);
    }
}

static volatile int g_worker_ran = 0;

process worker_set_flag(void) {
    g_worker_ran = 1;
    return OK;
}

process worker_wait_sem(sem s, volatile int *woke_flag) {
    sem_wait(s);
    if (woke_flag != NULL) {
        *woke_flag = 1;
    }
    return OK;
}

process worker_dummy(void) {
    return OK;
}

static void test_suite_scheduler_add(void) {
    kputs("\n=== Test Suite: scheduler_add ===");

    // 1. Invalid PID boundary tests
    assert_test(scheduler_add(-1) == ERR, "scheduler_add(-1) returns ERR");
    assert_test(scheduler_add(N_PROCS) == ERR, "scheduler_add(N_PROCS) returns ERR");
    assert_test(scheduler_add(N_PROCS + 5) == ERR, "scheduler_add(N_PROCS + 5) returns ERR");

    // 2. Add process to queue and verify presence
    pid p1 = create(worker_dummy, 30, "p1", 0);
    assert_test(p1 > 0, "Created process p1 for scheduler_add");
    assert_test(!sched_queue_includes(p1), "p1 not yet in ready queue");

    status_code add_res = scheduler_add(p1);
    assert_test(add_res == OK, "scheduler_add(p1) returns OK");
    assert_test(sched_queue_includes(p1), "p1 is now in ready queue");

    // 3. Duplicate add returns ERR
    assert_test(scheduler_add(p1) == ERR, "scheduler_add(p1) duplicate returns ERR");

    // Clean up
    scheduler_remove(p1);
    kill(p1);

    // 4. Functional test: process added with scheduler_add gets scheduled
    g_worker_ran = 0;
    pid p2 = create(worker_set_flag, 70, "p2_flag", 0);
    assert_test(p2 > 0, "Created high-priority worker p2");

    g_process_table[p2].state = PROC_READY;
    assert_test(scheduler_add(p2) == OK, "scheduler_add(p2) returns OK");

    yield(); // Switches to p2 (priority 70 > kmain 64)
    assert_test(g_worker_ran == 1, "p2 executed and set flag");
}

static void test_suite_scheduler_remove(void) {
    kputs("\n=== Test Suite: scheduler_remove ===");

    // 1. Invalid PID boundary tests
    assert_test(scheduler_remove(-1) == ERR, "scheduler_remove(-1) returns ERR");
    assert_test(scheduler_remove(N_PROCS) == ERR, "scheduler_remove(N_PROCS) returns ERR");

    // 2. Remove process not in queue returns ERR
    pid p = create(worker_dummy, 30, "p_dummy", 0);
    assert_test(scheduler_remove(p) == ERR, "scheduler_remove on unqueued process returns ERR");

    // 3. Add then remove
    scheduler_add(p);
    assert_test(sched_queue_includes(p), "Process enqueued");
    assert_test(scheduler_remove(p) == OK, "scheduler_remove returns OK");
    assert_test(!sched_queue_includes(p), "Process no longer in ready queue");
    assert_test(scheduler_remove(p) == ERR, "Second scheduler_remove returns ERR");

    kill(p);
}

static void test_suite_sem_abort(void) {
    kputs("\n=== Test Suite: sem_abort ===");

    // 1. Boundary tests
    assert_test(sem_abort(-1, 1) == ERR, "sem_abort with invalid sem id (-1) returns ERR");
    assert_test(sem_abort(N_SEM, 1) == ERR, "sem_abort with invalid sem id (N_SEM) returns ERR");

    sem s = -1;
    sem_create(0, &s);
    assert_test(validsem(s), "Created semaphore s");
    assert_test(sem_abort(s, -1) == ERR, "sem_abort with invalid pid (-1) returns ERR");
    assert_test(sem_abort(s, N_PROCS) == ERR, "sem_abort with invalid pid (N_PROCS) returns ERR");

    // 2. Process not waiting on semaphore
    pid p_free = create(worker_dummy, 30, "p_free", 0);
    assert_test(sem_abort(s, p_free) == ERR, "sem_abort on non-waiting process returns ERR");
    kill(p_free);
    sem_free(s);

    // 3. Free semaphore returns ERR
    assert_test(sem_abort(s, 1) == ERR, "sem_abort on free semaphore returns ERR");

    // 4. Direct sem_abort on waiting process
    sem s_wait = -1;
    sem_create(0, &s_wait);
    volatile int woke1 = 0;
    pid w1 = create(worker_wait_sem, 70, "w1", 2, (uint64)s_wait, (uint64)&woke1);
    resume(w1); // w1 runs, calls sem_wait(s_wait), blocks because count is 0

    assert_test(g_process_table[w1].state == PROC_WAIT, "w1 is in PROC_WAIT state");
    assert_test(g_semaphores[s_wait].count == -1, "Semaphore count decremented to -1");

    status_code abort_res = sem_abort(s_wait, w1);
    assert_test(abort_res == OK, "sem_abort(s_wait, w1) returns OK");
    assert_test(g_semaphores[s_wait].count == 0, "Semaphore count restored to 0 after sem_abort");

    kill(w1);
    sem_free(s_wait);

    // 5. Integration test: kill() uses sem_abort for PROC_WAIT processes
    sem s_kill = -1;
    sem_create(0, &s_kill);
    volatile int woke2 = 0;
    pid w2 = create(worker_wait_sem, 70, "w2", 2, (uint64)s_kill, (uint64)&woke2);
    resume(w2);

    assert_test(g_process_table[w2].state == PROC_WAIT, "w2 is waiting on s_kill");
    assert_test(g_semaphores[s_kill].count == -1, "s_kill count is -1");

    kill(w2); // Should trigger sem_abort internally in kill()
    assert_test(g_process_table[w2].state == PROC_EMPTY, "w2 state is PROC_EMPTY after kill");
    assert_test(g_semaphores[s_kill].count == 0, "s_kill count restored to 0 after kill");

    sem_free(s_kill);

    // 6. Queue order test: removing middle waiter from a multi-waiter queue
    sem s_multi = -1;
    sem_create(0, &s_multi);
    volatile int woke_a = 0, woke_b = 0, woke_c = 0;
    pid wa = create(worker_wait_sem, 70, "wa", 2, (uint64)s_multi, (uint64)&woke_a);
    pid wb = create(worker_wait_sem, 70, "wb", 2, (uint64)s_multi, (uint64)&woke_b);
    pid wc = create(worker_wait_sem, 70, "wc", 2, (uint64)s_multi, (uint64)&woke_c);

    resume(wa);
    resume(wb);
    resume(wc);

    assert_test(g_semaphores[s_multi].count == -3, "All 3 waiters queued, count is -3");

    // Abort the middle waiter wb
    assert_test(sem_abort(s_multi, wb) == OK, "sem_abort middle waiter wb returns OK");
    assert_test(g_semaphores[s_multi].count == -2, "Count restored to -2 after aborting wb");

    // Now release twice: should wake wa and wc, skipping wb
    sem_release(s_multi);
    assert_test(woke_a == 1, "First release woke wa");
    assert_test(woke_b == 0, "wb was aborted and did not wake");
    assert_test(woke_c == 0, "wc has not yet woken");

    sem_release(s_multi);
    assert_test(woke_c == 1, "Second release woke wc");
    assert_test(woke_b == 0, "wb still not woken");

    kill(wb);
    sem_free(s_multi);
}

process kmain(void) {
    kputs("----------------");
    kputs(" Welcome to RIO ");
    kputs("----------------");

    test_suite_scheduler_add();
    test_suite_scheduler_remove();
    test_suite_sem_abort();

    kputs("\n=================================");
    kprintf("Test Results: %d/%d tests passed\n", tests_passed, tests_run);
    kputs("=================================");

    return OK;
}

