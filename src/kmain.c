/**
 * kmain is the main process created by the entry point.
 * Temporarily modified to run test suites for process killing
 * (both explicit kill and natural procend termination).
 */

#include "kprint.h"
#include "process.h"
#include "schedqueue.h"
#include "scheduler.h"

// Tracking variables for tests
static volatile int g_natural_ran = 0;
static volatile int g_selfkill_ran = 0;
static volatile int g_selfkill_unreachable = 0;
static volatile int g_ready_ran = 0;

static void dummy_proc(void) {
    // Should never run
}

static void ready_proc(void) {
    g_ready_ran = 1;
}

static void natural_proc(void) {
    kputs("  [natural_proc] Running... will now return to trigger procend()");
    g_natural_ran = 1;
}

static void self_kill_proc(void) {
    kputs("  [self_kill_proc] Running... calling kill(curr_pid) explicitly");
    g_selfkill_ran = 1;
    kill(curr_pid);
    // The code below should NEVER be reached because kill() switches context away
    g_selfkill_unreachable = 1;
    kputs("  [ERROR] self_kill_proc continued executing after kill()!");
}

process kmain(void) {
    int total_tests = 0;
    int passed_tests = 0;

    kputs("========================================");
    kputs("     RIO PROCESS KILL TEST SUITE        ");
    kputs("========================================");

    // ---------------------------------------------------------
    // Test 1: Invalid PID checks
    // ---------------------------------------------------------
    kputs("\n--- Test 1: Invalid PID Rejection ---");
    total_tests++;
    if (kill(0) == ERR && kill(-1) == ERR && kill(N_PROCS) == ERR) {
        kputs("[PASS] kill() correctly rejected invalid PIDs (0, -1, N_PROCS)");
        passed_tests++;
    } else {
        kputs("[FAIL] kill() did not reject invalid PIDs properly");
    }

    // ---------------------------------------------------------
    // Test 2: Explicit kill of suspended process (PROC_SUSP)
    // ---------------------------------------------------------
    kputs("\n--- Test 2: Explicit Kill of Suspended Process ---");
    total_tests++;
    pid p_susp = create(dummy_proc, 20, "susp_proc", 0);
    kprintf("Created 'susp_proc' (PID: %d, state: %d)\n", p_susp, g_process_table[p_susp].state);

    if (g_process_table[p_susp].state == PROC_SUSP) {
        status_code ret = kill(p_susp);
        if (ret == OK && g_process_table[p_susp].state == PROC_EMPTY && !sched_queue_includes(p_susp)) {
            kputs("[PASS] Explicitly killed suspended process; state is now PROC_EMPTY");
            passed_tests++;
        } else {
            kprintf("[FAIL] kill() returned %d, state=%d\n", ret, g_process_table[p_susp].state);
        }
    } else {
        kputs("[FAIL] Created process was not in PROC_SUSP state");
    }

    // ---------------------------------------------------------
    // Test 3: Explicit kill of ready process (PROC_READY)
    // ---------------------------------------------------------
    kputs("\n--- Test 3: Explicit Kill of Ready Process ---");
    total_tests++;
    pid p_ready = create(ready_proc, 20, "ready_proc", 0);
    resume(p_ready);
    kprintf("Created and resumed 'ready_proc' (PID: %d, in_queue: %d)\n",
            p_ready, sched_queue_includes(p_ready));

    if (g_process_table[p_ready].state == PROC_READY && sched_queue_includes(p_ready)) {
        status_code ret = kill(p_ready);

        // Run a witness process to yield the CPU safely and verify ready_proc is skipped
        pid p_witness = create(dummy_proc, 30, "witness_proc", 0);
        resume(p_witness);
        yield();

        if (ret == OK && g_process_table[p_ready].state == PROC_EMPTY &&
            !sched_queue_includes(p_ready) && g_ready_ran == 0) {
            kputs("[PASS] Explicitly killed ready process; removed from queue and never ran");
            passed_tests++;
        } else {
            kprintf("[FAIL] state=%d, in_queue=%d, ran=%d\n",
                    g_process_table[p_ready].state, sched_queue_includes(p_ready), g_ready_ran);
        }
    } else {
        kputs("[FAIL] Process was not ready or not in queue after resume");
    }

    // ---------------------------------------------------------
    // Test 4: Natural termination via procend()
    // ---------------------------------------------------------
    kputs("\n--- Test 4: Natural Termination via procend() ---");
    total_tests++;
    pid p_natural = create(natural_proc, 30, "natural_proc", 0);
    resume(p_natural);
    kprintf("Created and resumed 'natural_proc' (PID: %d). Yielding...\n", p_natural);
    yield();

    kprintf("Back in kmain. Checking 'natural_proc' (PID: %d)...\n", p_natural);
    if (g_natural_ran == 1 && g_process_table[p_natural].state == PROC_EMPTY &&
        !sched_queue_includes(p_natural)) {
        kputs("[PASS] Process terminated naturally via procend(); state is PROC_EMPTY");
        passed_tests++;
    } else {
        kprintf("[FAIL] ran=%d, state=%d, in_queue=%d\n",
                g_natural_ran, g_process_table[p_natural].state, sched_queue_includes(p_natural));
    }

    // ---------------------------------------------------------
    // Test 5: Explicit self-kill (kill(curr_pid))
    // ---------------------------------------------------------
    kputs("\n--- Test 5: Explicit Self-Kill (kill(curr_pid)) ---");
    total_tests++;
    pid p_selfkill = create(self_kill_proc, 30, "selfkill_proc", 0);
    resume(p_selfkill);
    kprintf("Created and resumed 'selfkill_proc' (PID: %d). Yielding...\n", p_selfkill);
    yield();

    kprintf("Back in kmain. Checking 'selfkill_proc' (PID: %d)...\n", p_selfkill);
    if (g_selfkill_ran == 1 && g_selfkill_unreachable == 0 &&
        g_process_table[p_selfkill].state == PROC_EMPTY &&
        !sched_queue_includes(p_selfkill)) {
        kputs("[PASS] Process explicitly killed itself; state is PROC_EMPTY, post-kill code unreachable");
        passed_tests++;
    } else {
        kprintf("[FAIL] ran=%d, unreachable_reached=%d, state=%d, in_queue=%d\n",
                g_selfkill_ran, g_selfkill_unreachable,
                g_process_table[p_selfkill].state, sched_queue_includes(p_selfkill));
    }

    // ---------------------------------------------------------
    // Summary
    // ---------------------------------------------------------
    kputs("\n========================================");
    kprintf("TEST RESULTS: %d/%d PASSED\n", passed_tests, total_tests);
    kputs("========================================");

    return OK;
}
