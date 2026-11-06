#include <rio.h>

extern int32 g_tests_passed;
extern int32 g_defects_found;

process timed_dummy(void) {
    return OK;
}

process test_timed(void) {
    // 1. Empty delta queue tick returns ERR
    delta_init();
    if (delta_tick() == ERR) {
        g_tests_passed++;
    }

    // 2. sleep(0) fast path
    if (sleep(0) == OK) {
        g_tests_passed++;
    }

    // 3. sleep_abort and unsleep boundary checks
    if (sleep_abort(-1) == ERR && sleep_abort(N_PROCS) == ERR &&
        unsleep(-1) == ERR && unsleep(N_PROCS) == ERR) {
        g_tests_passed++;
    }

    // 4. sleep_abort on non-sleeping process returns ERR
    pid dummy = create(timed_dummy, 30, 1, 1, "tdummy", 0);
    if (sleep_abort(dummy) == ERR) {
        g_tests_passed++;
    }

    // 5. Delta tick wakes sleeping process to PROC_READY
    delta_init();
    g_process_table[dummy].state = PROC_SLEEP;
    lobby_enqueue(&g_delta, dummy, 1);

    if (delta_tick() == OK && g_process_table[dummy].state == PROC_READY) {
        g_tests_passed++;
    }
    kill(dummy);

    // 6. Adversarial: unsleep() on non-sleeping process
    pid not_sleeping = create(timed_dummy, 30, 1, 1, "t_awake", 0);
    if (unsleep(not_sleeping) == OK) {
        kputs("  [DEFECT] unsleep() silently returns OK on non-sleeping process when lobby_remove fails");
        g_defects_found++;
    } else {
        g_tests_passed++;
    }
    kill(not_sleeping);

    kputs("  [PASS] test_timed");
    return OK;
}
