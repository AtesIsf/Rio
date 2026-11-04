#include <rio.h>

process dummy_proc(void) {
    return OK;
}

status_code test_process(void) {
    int32 fails = 0;

    // 1. Parameter validation for create
    if (create(NULL, 10, "test", 0) != ERR) {
        kputs("FAIL: create(NULL fn) did not return ERR");
        fails++;
    }
    if (create(dummy_proc, 0, "test", 0) != ERR) {
        kputs("FAIL: create(priority=0) did not return ERR");
        fails++;
    }
    if (create(dummy_proc, -5, "test", 0) != ERR) {
        kputs("FAIL: create(negative priority) did not return ERR");
        fails++;
    }
    if (create(dummy_proc, 10, "test", -1) != ERR) {
        kputs("FAIL: create(negative n_args) did not return ERR");
        fails++;
    }

    // 2. Process creation state and fields
    pid p = create(dummy_proc, 42, "proc_test", 0);
    if (!validpid(p)) {
        kputs("FAIL: create returned invalid pid");
        fails++;
    } else {
        if (g_process_table[p].state != PROC_SUSP) {
            kputs("FAIL: created process state is not PROC_SUSP");
            fails++;
        }
        if (g_process_table[p].priority != 42) {
            kputs("FAIL: created process priority mismatch");
            fails++;
        }
    }

    // 3. kill validation
    if (kill(-1) != ERR || kill(N_PROCS) != ERR) {
        kputs("FAIL: kill out-of-bounds pid did not return ERR");
        fails++;
    }
    if (kill(0) != ERR) {
        kputs("FAIL: kill(0) (NULL proc) did not return ERR");
        fails++;
    }

    // 4. Clean termination of suspended process
    if (kill(p) != OK) {
        kputs("FAIL: kill(p) failed on valid suspended process");
        fails++;
    }
    if (g_process_table[p].state != PROC_EMPTY) {
        kputs("FAIL: killed process state is not PROC_EMPTY");
        fails++;
    }

    // 5. resume validation
    if (resume(-1) != ERR || resume(N_PROCS) != ERR) {
        kputs("FAIL: resume out-of-bounds pid did not return ERR");
        fails++;
    }

    if (fails == 0) {
        kputs("PASS: test_process");
        return OK;
    }
    return ERR;
}
