#include <rio.h>

process timed_dummy(void) {
    return OK;
}

status_code test_timed(void) {
    int32 fails = 0;

    // 1. Empty delta queue tick
    delta_init();
    if (delta_tick() != ERR) {
        kputs("FAIL: delta_tick on empty delta list did not return ERR");
        fails++;
    }

    // 2. sleep(0) should return OK immediately
    if (sleep(0) != OK) {
        kputs("FAIL: sleep(0) did not return OK");
        fails++;
    }

    // 3. sleep_abort and unsleep boundary checks
    if (sleep_abort(-1) != ERR || sleep_abort(N_PROCS) != ERR) {
        kputs("FAIL: sleep_abort accepted invalid pid");
        fails++;
    }
    if (unsleep(-1) != ERR || unsleep(N_PROCS) != ERR) {
        kputs("FAIL: unsleep accepted invalid pid");
        fails++;
    }

    // 4. sleep_abort on non-sleeping process returns ERR
    pid dummy = create(timed_dummy, 30, "tdummy", 0);
    if (sleep_abort(dummy) != ERR) {
        kputs("FAIL: sleep_abort on unscheduled pid did not return ERR");
        fails++;
    }

    // 5. Simulated delta queue tick and wake up
    delta_init();
    g_process_table[dummy].state = PROC_SLEEP;
    lobby_enqueue(&g_delta, dummy, 1);

    if (delta_tick() != OK) {
        kputs("FAIL: delta_tick failed to process single expiring entry");
        fails++;
    }
    if (g_process_table[dummy].state != PROC_READY) {
        kputs("FAIL: expired sleeping process state was not set to PROC_READY");
        fails++;
    }

    kill(dummy);

    if (fails == 0) {
        kputs("PASS: test_timed");
        return OK;
    }
    return ERR;
}
