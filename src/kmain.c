/**
 * kmain is the main process created by the entry point. It is basically
 * a main function for the kernel, but running as a process as well.
 */

#include <rio.h>

#define TEST_ASSERT(cond, msg)                                 \
    do {                                                       \
        if (!(cond)) {                                         \
            kprintf("  [FAIL] %s (line %d)\n", msg, __LINE__); \
            tests_failed++;                                    \
            return;                                            \
        }                                                      \
    } while (0)

static int32 tests_passed = 0;
static int32 tests_failed = 0;

/* ========================================================================= */
/* Test 1: Delta List Unit Operations & Relative Delta Math                 */
/* ========================================================================= */

static void test_delta_list_math(void) {
    kputs("\n[Test 1] Delta List Ordering & Relative Delta Calculations...");

    struct LobbyEntry test_queue = { .next = LOBBY_NULL, .delta = 0 };
    pid p = -1;
    int32 d = -1;

    /* 1. Peek on empty list must return ERR */
    TEST_ASSERT(lobby_peek(&test_queue, &p, &d) == ERR,
                "Peek on empty queue should return ERR");

    /* 2. Insert process 1 with delay 30 */
    TEST_ASSERT(lobby_enqueue(&test_queue, 1, 30) == OK, "enqueue p1 failed");
    TEST_ASSERT(lobby_peek(&test_queue, &p, &d) == OK, "peek after p1 failed");
    TEST_ASSERT(p == 1 && d == 30, "p1 should have delta 30");

    /* 3. Insert process 2 with delay 10 (earlier than p1) */
    TEST_ASSERT(lobby_enqueue(&test_queue, 2, 10) == OK, "enqueue p2 failed");
    TEST_ASSERT(lobby_peek(&test_queue, &p, &d) == OK, "peek after p2 failed");
    TEST_ASSERT(p == 2 && d == 10, "p2 should be head with delta 10");

    /* 4. Insert process 3 with delay 20 (between p2 and p1) */
    TEST_ASSERT(lobby_enqueue(&test_queue, 3, 20) == OK, "enqueue p3 failed");
    /* Head must still be p2 with delta 10 */
    TEST_ASSERT(lobby_peek(&test_queue, &p, &d) == OK, "peek head failed");
    TEST_ASSERT(p == 2 && d == 10, "head should still be p2 (delta 10)");

    /* 5. Test decrement_delta */
    TEST_ASSERT(decrement_delta(&test_queue, 2) == OK, "decrement_delta failed");
    TEST_ASSERT(lobby_peek(&test_queue, &p, &d) == OK, "peek after decrement failed");
    TEST_ASSERT(p == 2 && d == 9, "p2 delta should now be 9");

    /* decrement_delta parameter validation */
    TEST_ASSERT(decrement_delta(NULL, 2) == ERR, "NULL head should return ERR");
    TEST_ASSERT(decrement_delta(&test_queue, -1) == ERR, "invalid pid should return ERR");
    TEST_ASSERT(decrement_delta(&test_queue, 50) == ERR, "pid not in list should return ERR");

    /* 6. Remove middle process 3 and verify queue integrity */
    TEST_ASSERT(lobby_remove(&test_queue, 3) == OK, "remove middle process failed");
    /* Dequeue p2 */
    TEST_ASSERT(lobby_dequeue(&test_queue, &p) == OK && p == 2, "dequeue p2 failed");
    /* Remaining should be p1 with adjusted relative delta */
    TEST_ASSERT(lobby_peek(&test_queue, &p, &d) == OK && p == 1,
                "remaining process should be p1");

    /* Clean up */
    lobby_dequeue(&test_queue, &p);

    kputs("  [PASS] Delta list insertion, relative math, and removal verified.");
    tests_passed++;
}

/* ========================================================================= */
/* Test 2: Basic sleep(0) and sleep(ms) with Automatic Wakeup                */
/* ========================================================================= */

static sem g_test2_done;
static volatile bool g_test2_worker_ran = false;

static void worker_basic_sleep(void) {
    /* sleep for 20ms */
    sleep(20);
    g_test2_worker_ran = true;
    sem_release(g_test2_done);
}

static void test_basic_sleep(void) {
    kputs("\n[Test 2] Basic sleep(0) and Timed sleep(20ms)...");

    /* 1. sleep(0) should yield immediately and return OK */
    status_code s0 = sleep(0);
    TEST_ASSERT(s0 == OK, "sleep(0) should return OK");

    /* 2. Timed sleep */
    sem_create(0, &g_test2_done);
    g_test2_worker_ran = false;

    pid worker = create(worker_basic_sleep, 64, "sleep_w2", 0);
    resume(worker);

    /* While worker is sleeping, its state must be PROC_SLEEP */
    TEST_ASSERT(g_process_table[worker].state == PROC_SLEEP || g_test2_worker_ran,
                "Worker should enter PROC_SLEEP state");

    /* Wait for timer interrupt to count down delta and wake up worker */
    sem_wait(g_test2_done);

    TEST_ASSERT(g_test2_worker_ran, "Worker did not execute after sleep");
    sem_free(g_test2_done);

    kputs("  [PASS] sleep(0) yielded and sleep(20ms) woke up automatically.");
    tests_passed++;
}

/* ========================================================================= */
/* Test 3: Multiple Sleeping Processes & Correct Wakeup Ordering              */
/* ========================================================================= */

static sem g_test3_done;
static volatile int32 g_wakeup_order[3];
static volatile int32 g_wakeup_count = 0;

static void sleeper_1(void) {
    sleep(30); /* Sleeps longest: should wake up 3rd */
    g_wakeup_order[g_wakeup_count++] = 1;
    sem_release(g_test3_done);
}

static void sleeper_2(void) {
    sleep(10); /* Sleeps shortest: should wake up 1st */
    g_wakeup_order[g_wakeup_count++] = 2;
    sem_release(g_test3_done);
}

static void sleeper_3(void) {
    sleep(20); /* Sleeps medium: should wake up 2nd */
    g_wakeup_order[g_wakeup_count++] = 3;
    sem_release(g_test3_done);
}

static void test_sleep_ordering(void) {
    kputs("\n[Test 3] Multiple Sleeping Processes Wakeup Ordering...");
    kputs("  Spawning 3 workers with delays 30ms, 10ms, 20ms...");
    sem_create(0, &g_test3_done);
    g_wakeup_count = 0;

    pid p1 = create(sleeper_1, 64, "sleeper_30ms", 0);
    pid p2 = create(sleeper_2, 64, "sleeper_10ms", 0);
    pid p3 = create(sleeper_3, 64, "sleeper_20ms", 0);

    resume(p1);
    resume(p2);
    resume(p3);

    /* Wait for all 3 sleepers to wake up */
    sem_wait(g_test3_done);
    sem_wait(g_test3_done);
    sem_wait(g_test3_done);

    TEST_ASSERT(g_wakeup_count == 3, "Not all 3 sleepers completed");
    kprintf("  Wakeup sequence recorded: [%d, %d, %d]\n",
            g_wakeup_order[0], g_wakeup_order[1], g_wakeup_order[2]);

    TEST_ASSERT(g_wakeup_order[0] == 2, "First to wake up should be sleeper_2 (10ms)");
    TEST_ASSERT(g_wakeup_order[1] == 3, "Second to wake up should be sleeper_3 (20ms)");
    TEST_ASSERT(g_wakeup_order[2] == 1, "Third to wake up should be sleeper_1 (30ms)");

    sem_free(g_test3_done);
    kputs("  [PASS] All processes awakened in exact order of expiration.");
    tests_passed++;
}

/* ========================================================================= */
/* Test 4: Concurrent Processes with Identical Sleep Durations               */
/* ========================================================================= */

static sem g_test4_done;
static volatile int32 g_simultaneous_woken = 0;

static void same_duration_sleeper(void) {
    sleep(15);
    g_simultaneous_woken++;
    sem_release(g_test4_done);
}

static void test_identical_sleep(void) {
    kputs("\n[Test 4] Processes with Identical Sleep Durations (Delta = 0)...");
    sem_create(0, &g_test4_done);
    g_simultaneous_woken = 0;

    pid p1 = create(same_duration_sleeper, 64, "same_1", 0);
    pid p2 = create(same_duration_sleeper, 64, "same_2", 0);

    resume(p1);
    resume(p2);

    sem_wait(g_test4_done);
    sem_wait(g_test4_done);

    TEST_ASSERT(g_simultaneous_woken == 2,
                "Both processes should wake up together");

    sem_free(g_test4_done);
    kputs("  [PASS] Multiple processes with matching expiration unblocked together.");
    tests_passed++;
}

/* ========================================================================= */
/* Test 5: Premature Wakeup via unsleep()                                     */
/* ========================================================================= */

static sem g_test5_done;
static volatile bool g_unslept_woke = false;

static void long_sleeper(void) {
    /* Attempt to sleep for 50,000ms (50 seconds) */
    sleep(50000);
    g_unslept_woke = true;
    sem_release(g_test5_done);
}

static void test_unsleep(void) {
    kputs("\n[Test 5] Premature Process Wakeup via unsleep()...");

    /* Parameter validation */
    TEST_ASSERT(unsleep(-1) == ERR, "unsleep(-1) should return ERR");
    TEST_ASSERT(unsleep(N_PROCS) == ERR, "unsleep(N_PROCS) should return ERR");

    sem_create(0, &g_test5_done);
    g_unslept_woke = false;

    pid sleeper = create(long_sleeper, 64, "long_sleeper", 0);
    resume(sleeper);

    /* Verify process is asleep */
    TEST_ASSERT(g_process_table[sleeper].state == PROC_SLEEP,
                "Target process should be sleeping");

    /* Wake up process early using unsleep */
    status_code status = unsleep(sleeper);
    TEST_ASSERT(status == OK, "unsleep() failed on sleeping process");

    /* Target process should run and finish immediately */
    sem_wait(g_test5_done);
    TEST_ASSERT(g_unslept_woke, "Process did not resume after unsleep");

    /* Calling unsleep on already awake process should be handled safely */
    TEST_ASSERT(unsleep(sleeper) == OK, "unsleep on awake proc returned error");

    sem_free(g_test5_done);
    kputs("  [PASS] unsleep() cleanly removed process from delta queue and resumed it.");
    tests_passed++;
}

/* ========================================================================= */
/* Test 6: Delta Queue Preservation when Unsleeping Head Element             */
/* ========================================================================= */

static sem g_test6_done;
static volatile bool g_p2_woke = false;

static void sleeper_p1(void) {
    sleep(50000); /* Will be unslept */
}

static void sleeper_p2(void) {
    sleep(20); /* Should wake up on its own after 20ms */
    g_p2_woke = true;
    sem_release(g_test6_done);
}

static void test_unsleep_delta_preservation(void) {
    kputs("\n[Test 6] Delta Adjustment when Removing First Element...");
    sem_create(0, &g_test6_done);
    g_p2_woke = false;

    /* Enqueue p2 (20ms) then p1 (sleeps long) */
    pid p2 = create(sleeper_p2, 64, "p2_timed", 0);
    pid p1 = create(sleeper_p1, 64, "p1_cancel", 0);

    resume(p2);
    resume(p1);

    /* Cancel p1 */
    TEST_ASSERT(unsleep(p1) == OK, "unsleep p1 failed");

    /* p2 must still wake up correctly after 20ms */
    sem_wait(g_test6_done);
    TEST_ASSERT(g_p2_woke, "p2 did not wake up after p1 was unslept");

    sem_free(g_test6_done);
    kputs("  [PASS] Remaining queue timers preserved after unsleeping another element.");
    tests_passed++;
}

/* ========================================================================= */
/* Test 7: Paced Producer-Consumer Using Sleep & Semaphores                  */
/* ========================================================================= */

#define PC_ITEMS 4
static int32 g_timed_buffer[PC_ITEMS];
static int32 g_timed_in = 0;
static int32 g_timed_out = 0;

static sem g_timed_mutex;
static sem g_timed_empty;
static sem g_timed_full;
static sem g_timed_done;
static bool g_timed_correct = true;

static void timed_producer(void) {
    for (int32 i = 1; i <= PC_ITEMS; i++) {
        sleep(5); /* Small delay between produces */

        sem_wait(g_timed_empty);
        sem_wait(g_timed_mutex);

        g_timed_buffer[g_timed_in] = i;
        kprintf("  [Timed Producer] Produced %d -> slot %d\n", i, g_timed_in);
        g_timed_in = (g_timed_in + 1) % PC_ITEMS;

        sem_release(g_timed_mutex);
        sem_release(g_timed_full);
    }
    sem_release(g_timed_done);
}

static void timed_consumer(void) {
    for (int32 i = 1; i <= PC_ITEMS; i++) {
        sleep(5); /* Small delay between consumes */

        sem_wait(g_timed_full);
        sem_wait(g_timed_mutex);

        int32 item = g_timed_buffer[g_timed_out];
        kprintf("  [Timed Consumer] Consumed %d <- slot %d\n", item, g_timed_out);
        if (item != i) {
            g_timed_correct = false;
        }
        g_timed_out = (g_timed_out + 1) % PC_ITEMS;

        sem_release(g_timed_mutex);
        sem_release(g_timed_empty);
    }
    sem_release(g_timed_done);
}

static void test_timed_producer_consumer(void) {
    kputs("\n[Test 7] Paced Producer-Consumer (Sleep + Semaphores)...");

    g_timed_in = 0;
    g_timed_out = 0;
    g_timed_correct = true;

    sem_create(1, &g_timed_mutex);
    sem_create(PC_ITEMS, &g_timed_empty);
    sem_create(0, &g_timed_full);
    sem_create(0, &g_timed_done);

    pid prod = create(timed_producer, 64, "timed_prod", 0);
    pid cons = create(timed_consumer, 64, "timed_cons", 0);

    resume(prod);
    resume(cons);

    sem_wait(g_timed_done);
    sem_wait(g_timed_done);

    TEST_ASSERT(g_timed_correct, "Data inconsistency in timed producer-consumer");

    sem_free(g_timed_mutex);
    sem_free(g_timed_empty);
    sem_free(g_timed_full);
    sem_free(g_timed_done);

    kputs("  [PASS] Timed Producer-Consumer synchronized and completed cleanly.");
    tests_passed++;
}

/* ========================================================================= */
/* kmain Process Entry Point                                                 */
/* ========================================================================= */

process kmain(void) {
    kputs("----------------");
    kputs(" Welcome to RIO ");
    kputs("----------------");
    kputs("=== STARTING DELTA LIST & SLEEP TEST SUITE ===");

    test_delta_list_math();
    test_basic_sleep();
    test_sleep_ordering();
    test_identical_sleep();
    test_unsleep();
    test_unsleep_delta_preservation();
    test_timed_producer_consumer();

    kputs("\n========================================");
    kprintf(" Test Results: %d passed, %d failed\n", tests_passed, tests_failed);
    if (tests_failed == 0) {
        kputs(" ALL DELTA LIST & SLEEP TESTS PASSED!");
    } else {
        kputs(" SOME TESTS FAILED!");
    }
    kputs("========================================");

    return OK;
}
