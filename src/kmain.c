/**
 * kmain is the main process created by the entry point. It is basically
 * a main function for the kernel, but running as a process as well.
 */

#include <rio.h>

/* Helper macro for assertions in tests */
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
/* Test 1: Semaphore Creation & Parameter Validation                         */
/* ========================================================================= */

static void test_sem_create_validation(void) {
    kputs("\n[Test 1] Semaphore Creation & Parameter Validation...");
    sem s = -1;

    /* 1. Valid creation with count >= 0 */
    status_code status = sem_create(1, &s);
    TEST_ASSERT(status == OK, "sem_create(1, &s) failed");
    TEST_ASSERT(validsem(s), "sem_create returned invalid id");
    TEST_ASSERT(g_semaphores[s].state == SEM_USED, "sem state is not SEM_USED");
    TEST_ASSERT(g_semaphores[s].count == 1, "sem count is not 1");
    sem_free(s);

    status = sem_create(0, &s);
    TEST_ASSERT(status == OK, "sem_create(0, &s) failed");
    TEST_ASSERT(validsem(s), "sem_create(0) returned invalid id");
    TEST_ASSERT(g_semaphores[s].count == 0, "sem count is not 0");
    sem_free(s);

    /* 2. Negative count must return ERR */
    status = sem_create(-1, &s);
    TEST_ASSERT(status == ERR, "sem_create with count -1 should return ERR");

    status = sem_create(-100, &s);
    TEST_ASSERT(status == ERR, "sem_create with count -100 should return ERR");

    /* 3. NULL id pointer must return ERR */
    status = sem_create(1, NULL);
    TEST_ASSERT(status == ERR, "sem_create with NULL id should return ERR");

    /* 4. Table exhaustion & slot recycling */
    sem all_sems[N_SEM];
    for (int32 i = 0; i < N_SEM; i++) {
        status = sem_create(0, &all_sems[i]);
        TEST_ASSERT(status == OK, "Failed to allocate semaphore up to N_SEM");
    }

    /* 65th semaphore allocation must fail */
    sem extra_sem;
    status = sem_create(0, &extra_sem);
    TEST_ASSERT(status == ERR, "Allocation beyond N_SEM should return ERR");

    /* Free index 5 and ensure it gets reused */
    sem recycled_id = all_sems[5];
    status = sem_free(recycled_id);
    TEST_ASSERT(status == OK, "sem_free failed on allocated sem");

    status = sem_create(42, &extra_sem);
    TEST_ASSERT(status == OK, "sem_create failed after freeing a slot");
    TEST_ASSERT(extra_sem == recycled_id, "sem_create did not reuse freed slot");
    TEST_ASSERT(g_semaphores[extra_sem].count == 42, "reused sem count incorrect");

    /* Free all semaphores */
    all_sems[5] = extra_sem;
    for (int32 i = 0; i < N_SEM; i++) {
        status = sem_free(all_sems[i]);
        TEST_ASSERT(status == OK, "sem_free cleanup failed");
    }

    kputs("  [PASS] Creation, invalid params, exhaustion & recycling verified.");
    tests_passed++;
}

/* ========================================================================= */
/* Test 2: Counting Semaphore Semantics (Non-blocking)                       */
/* ========================================================================= */

static void test_sem_counting_nonblocking(void) {
    kputs("\n[Test 2] Counting Semaphore Semantics (Non-blocking)...");
    sem s;
    status_code status = sem_create(3, &s);
    TEST_ASSERT(status == OK, "sem_create(3) failed");
    TEST_ASSERT(g_semaphores[s].count == 3, "initial count != 3");

    /* Decrement 3 times without blocking */
    status = sem_wait(s);
    TEST_ASSERT(status == OK, "first sem_wait failed");
    TEST_ASSERT(g_semaphores[s].count == 2, "count != 2 after 1st wait");

    status = sem_wait(s);
    TEST_ASSERT(status == OK, "second sem_wait failed");
    TEST_ASSERT(g_semaphores[s].count == 1, "count != 1 after 2nd wait");

    status = sem_wait(s);
    TEST_ASSERT(status == OK, "third sem_wait failed");
    TEST_ASSERT(g_semaphores[s].count == 0, "count != 0 after 3rd wait");

    /* Increment back to 3 and beyond */
    status = sem_release(s);
    TEST_ASSERT(status == OK, "first sem_release failed");
    TEST_ASSERT(g_semaphores[s].count == 1, "count != 1 after 1st release");

    status = sem_release(s);
    TEST_ASSERT(status == OK, "second sem_release failed");
    TEST_ASSERT(g_semaphores[s].count == 2, "count != 2 after 2nd release");

    status = sem_release(s);
    TEST_ASSERT(status == OK, "third sem_release failed");
    TEST_ASSERT(g_semaphores[s].count == 3, "count != 3 after 3rd release");

    status = sem_release(s);
    TEST_ASSERT(status == OK, "fourth sem_release failed");
    TEST_ASSERT(g_semaphores[s].count == 4, "count != 4 after 4th release");

    sem_free(s);
    kputs("  [PASS] Counting wait and release correctly tracked counts.");
    tests_passed++;
}

/* ========================================================================= */
/* Test 3: Error Handling on Invalid IDs & Freed Semaphores                  */
/* ========================================================================= */

static void test_sem_invalid_and_freed(void) {
    kputs("\n[Test 3] Error Handling on Invalid IDs & Freed Semaphores...");

    /* Invalid IDs */
    TEST_ASSERT(sem_wait(-1) == ERR, "sem_wait(-1) should return ERR");
    TEST_ASSERT(sem_wait(N_SEM) == ERR, "sem_wait(N_SEM) should return ERR");
    TEST_ASSERT(sem_wait(100) == ERR, "sem_wait(100) should return ERR");

    TEST_ASSERT(sem_release(-1) == ERR, "sem_release(-1) should return ERR");
    TEST_ASSERT(sem_release(N_SEM) == ERR, "sem_release(N_SEM) should return ERR");
    TEST_ASSERT(sem_release(100) == ERR, "sem_release(100) should return ERR");

    TEST_ASSERT(sem_free(-1) == ERR, "sem_free(-1) should return ERR");
    TEST_ASSERT(sem_free(N_SEM) == ERR, "sem_free(N_SEM) should return ERR");
    TEST_ASSERT(sem_free(100) == ERR, "sem_free(100) should return ERR");

    /* Operations on unallocated / freed semaphore */
    sem s;
    sem_create(1, &s);
    TEST_ASSERT(sem_free(s) == OK, "sem_free failed");

    /* Double free */
    TEST_ASSERT(sem_free(s) == ERR, "Double sem_free should return ERR");

    /* Wait & Release on freed semaphore */
    TEST_ASSERT(sem_wait(s) == ERR, "sem_wait on freed sem should return ERR");
    TEST_ASSERT(sem_release(s) == ERR, "sem_release on freed sem should return ERR");

    kputs("  [PASS] Invalid sem IDs and freed semaphores handled safely.");
    tests_passed++;
}

/* ========================================================================= */
/* Test 4: Mutual Exclusion with Binary Semaphore                            */
/* ========================================================================= */

static sem g_mutex;
static sem g_mutex_done;
static volatile int32 g_shared_counter = 0;

static void mutex_worker(void) {
    for (int32 i = 0; i < 50; i++) {
        sem_wait(g_mutex);

        /* Read value, yield to test preemption/context switch during CS */
        int32 temp = g_shared_counter;
        yield();
        g_shared_counter = temp + 1;

        sem_release(g_mutex);
    }
    sem_release(g_mutex_done);
}

static void test_sem_mutual_exclusion(void) {
    kputs("\n[Test 4] Mutual Exclusion with Binary Semaphore...");
    sem_create(1, &g_mutex);
    sem_create(0, &g_mutex_done);
    g_shared_counter = 0;

    pid w1 = create(mutex_worker, 64, "mutex_w1", 0);
    pid w2 = create(mutex_worker, 64, "mutex_w2", 0);
    resume(w1);
    resume(w2);

    /* Wait for both workers to finish */
    sem_wait(g_mutex_done);
    sem_wait(g_mutex_done);

    TEST_ASSERT(g_shared_counter == 100, "shared_counter != 100 with mutex");

    sem_free(g_mutex);
    sem_free(g_mutex_done);

    kputs("  [PASS] Mutex protected shared counter across yields (final = 100).");
    tests_passed++;
}

/* ========================================================================= */
/* Test 5: Process Blocking & Signaling (sem_wait / sem_release)              */
/* ========================================================================= */

static sem g_sync_sem;
static sem g_sync_ack;
static volatile bool g_worker_ran = false;

static void sync_worker(void) {
    /* Blocks immediately because g_sync_sem count is 0 */
    sem_wait(g_sync_sem);
    g_worker_ran = true;
    sem_release(g_sync_ack);
}

static void test_sem_blocking_signaling(void) {
    kputs("\n[Test 5] Process Blocking & Signaling (sem_wait / sem_release)...");
    sem_create(0, &g_sync_sem);
    sem_create(0, &g_sync_ack);
    g_worker_ran = false;

    pid worker = create(sync_worker, 64, "sync_worker", 0);
    resume(worker);

    /* The worker was resumed and yielded inside sem_wait */
    TEST_ASSERT(g_process_table[worker].state == PROC_WAIT,
                "Worker should be in PROC_WAIT state");
    TEST_ASSERT(g_process_table[worker].sem_id == g_sync_sem,
                "Worker sem_id should match g_sync_sem");
    TEST_ASSERT(g_semaphores[g_sync_sem].count == -1,
                "Semaphore count should be -1 while worker is waiting");
    TEST_ASSERT(!g_worker_ran, "Worker should not have run past sem_wait");

    /* Release semaphore to unblock worker */
    sem_release(g_sync_sem);

    /* Wait for worker to finish signaling ack */
    sem_wait(g_sync_ack);
    TEST_ASSERT(g_worker_ran, "Worker did not execute after being unblocked");

    sem_free(g_sync_sem);
    sem_free(g_sync_ack);

    kputs("  [PASS] Worker blocked in PROC_WAIT and was unblocked by sem_release.");
    tests_passed++;
}

/* ========================================================================= */
/* Test 6: Mass Wakeup via sem_free                                          */
/* ========================================================================= */

static sem g_mass_sem;
static sem g_mass_done;
static volatile int32 g_mass_woken_count = 0;

static void mass_worker(void) {
    sem_wait(g_mass_sem);
    g_mass_woken_count++;
    sem_release(g_mass_done);
}

static void test_sem_free_mass_wakeup(void) {
    kputs("\n[Test 6] Mass Wakeup via sem_free...");
    sem_create(0, &g_mass_sem);
    sem_create(0, &g_mass_done);
    g_mass_woken_count = 0;

    pid w1 = create(mass_worker, 64, "mass_w1", 0);
    pid w2 = create(mass_worker, 64, "mass_w2", 0);
    pid w3 = create(mass_worker, 64, "mass_w3", 0);

    resume(w1);
    resume(w2);
    resume(w3);

    /* Verify all 3 workers are waiting on g_mass_sem */
    TEST_ASSERT(g_process_table[w1].state == PROC_WAIT, "w1 should be PROC_WAIT");
    TEST_ASSERT(g_process_table[w2].state == PROC_WAIT, "w2 should be PROC_WAIT");
    TEST_ASSERT(g_process_table[w3].state == PROC_WAIT, "w3 should be PROC_WAIT");
    TEST_ASSERT(g_semaphores[g_mass_sem].count == -3, "sem count should be -3");

    /* Calling sem_free must wake up all 3 waiting processes */
    status_code status = sem_free(g_mass_sem);
    TEST_ASSERT(status == OK, "sem_free failed");

    /* Wait for all 3 workers to signal done */
    sem_wait(g_mass_done);
    sem_wait(g_mass_done);
    sem_wait(g_mass_done);

    TEST_ASSERT(g_mass_woken_count == 3, "All 3 workers should have woken up");
    sem_free(g_mass_done);

    kputs("  [PASS] sem_free successfully woke up all 3 waiting processes.");
    tests_passed++;
}

/* ========================================================================= */
/* Test 7: Kill on a Waiting Process (Cleans up Lobby & Count)               */
/* ========================================================================= */

static sem g_kill_sem;

static void waiting_target_proc(void) {
    sem_wait(g_kill_sem);
    /* Should never reach here if killed */
}

static void test_sem_kill_waiting_proc(void) {
    kputs("\n[Test 7] Kill on a Waiting Process (Cleans up Lobby & Count)...");
    sem_create(0, &g_kill_sem);

    pid target = create(waiting_target_proc, 64, "kill_target", 0);
    resume(target);

    TEST_ASSERT(g_process_table[target].state == PROC_WAIT,
                "Target process should be in PROC_WAIT");
    TEST_ASSERT(g_semaphores[g_kill_sem].count == -1,
                "Semaphore count should be -1");

    /* Kill target process while waiting */
    status_code status = kill(target);
    TEST_ASSERT(status == OK, "kill() on waiting process failed");
    TEST_ASSERT(g_process_table[target].state == PROC_EMPTY,
                "Killed process state should be PROC_EMPTY");
    TEST_ASSERT(g_semaphores[g_kill_sem].count == 0,
                "Semaphore count should be restored to 0 after kill");
    TEST_ASSERT(g_semaphores[g_kill_sem].head.next == LOBBY_NULL,
                "Lobby queue should be empty after kill");

    sem_free(g_kill_sem);

    kputs("  [PASS] kill() cleaned up waiting process from semaphore lobby.");
    tests_passed++;
}

/* ========================================================================= */
/* Test 8: Classic Producer-Consumer with Bounded Buffer                     */
/* ========================================================================= */

#define PC_BUFFER_SIZE (4)
#define PC_TOTAL_ITEMS (10)

static int32 g_pc_buffer[PC_BUFFER_SIZE];
static int32 g_pc_in = 0;
static int32 g_pc_out = 0;

static sem g_sem_mutex;
static sem g_sem_empty;
static sem g_sem_full;
static sem g_sem_done;

static int32 g_pc_produced_count = 0;
static int32 g_pc_consumed_count = 0;
static bool g_pc_data_correct = true;

static void producer(void) {
    for (int32 item = 1; item <= PC_TOTAL_ITEMS; item++) {
        sem_wait(g_sem_empty);
        sem_wait(g_sem_mutex);

        g_pc_buffer[g_pc_in] = item;
        kprintf("  [Producer] Produced item %d -> slot %d\n", item, g_pc_in);
        g_pc_in = (g_pc_in + 1) % PC_BUFFER_SIZE;
        g_pc_produced_count++;

        sem_release(g_sem_mutex);
        sem_release(g_sem_full);
    }
    sem_release(g_sem_done);
}

static void consumer(void) {
    for (int32 expected = 1; expected <= PC_TOTAL_ITEMS; expected++) {
        sem_wait(g_sem_full);
        sem_wait(g_sem_mutex);

        int32 item = g_pc_buffer[g_pc_out];
        kprintf("  [Consumer] Consumed item %d <- slot %d\n", item, g_pc_out);
        if (item != expected) {
            g_pc_data_correct = false;
        }
        g_pc_out = (g_pc_out + 1) % PC_BUFFER_SIZE;
        g_pc_consumed_count++;

        sem_release(g_sem_mutex);
        sem_release(g_sem_empty);
    }
    sem_release(g_sem_done);
}

static void test_producer_consumer(void) {
    kputs("\n[Test 8] Classic Producer-Consumer with Bounded Buffer...");
    kprintf("  Config: Buffer Capacity = %d, Total Items = %d\n",
            PC_BUFFER_SIZE, PC_TOTAL_ITEMS);

    /* Initialize circular buffer indices & counters */
    g_pc_in = 0;
    g_pc_out = 0;
    g_pc_produced_count = 0;
    g_pc_consumed_count = 0;
    g_pc_data_correct = true;

    /* Semaphores:
     * mutex: binary semaphore for mutual exclusion (initial = 1)
     * empty: counting semaphore for available empty slots (initial = BUFFER_SIZE)
     * full : counting semaphore for filled slots ready to consume (initial = 0)
     * done : completion signaling for kmain (initial = 0)
     */
    sem_create(1, &g_sem_mutex);
    sem_create(PC_BUFFER_SIZE, &g_sem_empty);
    sem_create(0, &g_sem_full);
    sem_create(0, &g_sem_done);

    pid prod = create(producer, 64, "producer", 0);
    pid cons = create(consumer, 64, "consumer", 0);

    resume(prod);
    resume(cons);

    /* Wait for both producer and consumer to finish */
    sem_wait(g_sem_done);
    sem_wait(g_sem_done);

    TEST_ASSERT(g_pc_produced_count == PC_TOTAL_ITEMS,
                "Producer did not produce all items");
    TEST_ASSERT(g_pc_consumed_count == PC_TOTAL_ITEMS,
                "Consumer did not consume all items");
    TEST_ASSERT(g_pc_data_correct,
                "Consumer received unexpected item sequence");

    sem_free(g_sem_mutex);
    sem_free(g_sem_empty);
    sem_free(g_sem_full);
    sem_free(g_sem_done);

    kputs("  [PASS] Producer-Consumer completed all transfers with full data integrity!");
    tests_passed++;
}

/* ========================================================================= */
/* kmain Process Entry Point                                                 */
/* ========================================================================= */

process kmain(void) {
    kputs("----------------");
    kputs(" Welcome to RIO ");
    kputs("----------------");
    kputs("=== STARTING SEMAPHORE TEST SUITE ===");

    test_sem_create_validation();
    test_sem_counting_nonblocking();
    test_sem_invalid_and_freed();
    test_sem_mutual_exclusion();
    test_sem_blocking_signaling();
    test_sem_free_mass_wakeup();
    test_sem_kill_waiting_proc();
    test_producer_consumer();

    kputs("\n========================================");
    kprintf(" Test Results: %d passed, %d failed\n", tests_passed, tests_failed);
    if (tests_failed == 0) {
        kputs(" ALL SEMAPHORE TESTS PASSED SUCCESSFULLY!");
    } else {
        kputs(" SOME SEMAPHORE TESTS FAILED!");
    }
    kputs("========================================");

    return OK;
}
