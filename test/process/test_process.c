#include <rio.h>

extern int32 g_tests_passed;
extern int32 g_defects_found;

process dummy_proc(void) {
    return OK;
}

process test_process(void) {
    // 1. Parameter validation for create
    if (create(NULL, 10, 1, 1, "test", 0) == ERR &&
        create(dummy_proc, 0, 1, 1, "test", 0) == ERR &&
        create(dummy_proc, -5, 1, 1, "test", 0) == ERR &&
        create(dummy_proc, 10, 1, 1, "test", -1) == ERR &&
        create(dummy_proc, 10, 0, 1, "test", 0) == ERR &&
        create(dummy_proc, 10, 1, 0, "test", 0) == ERR) {
        g_tests_passed++;
    }

    // 2. Process creation initial state
    pid p = create(dummy_proc, 42, 1, 1, "proc_test", 0);
    if (validpid(p) && g_process_table[p].state == PROC_SUSP &&
        g_process_table[p].priority == 42 &&
        g_process_table[p].n_stack_pages == 1 &&
        g_process_table[p].n_heap_pages == 1) {
        g_tests_passed++;
    }

    // 3. kill validation on out-of-bounds PID and PID 0 (NULL proc)
    if (kill(-1) == ERR && kill(N_PROCS) == ERR && kill(0) == ERR) {
        g_tests_passed++;
    }

    // 4. Clean kill of suspended process
    if (kill(p) == OK && g_process_table[p].state == PROC_EMPTY) {
        g_tests_passed++;
    }

    // 5. Adversarial: kill() on already dead / PROC_EMPTY process
    if (kill(p) == OK) {
        kputs("  [DEFECT] kill() returns OK on PROC_EMPTY process and re-frees stack_base");
        g_defects_found++;
    } else {
        g_tests_passed++;
    }

    // 6. Adversarial: resume() on uncreated PROC_EMPTY process
    pid empty_pid = get_valid_pid();
    if (validpid(empty_pid)) {
        if (resume(empty_pid) == OK) {
            kputs("  [DEFECT] resume() enqueues uninitialized PROC_EMPTY process into scheduler ready queue");
            g_defects_found++;
            scheduler_remove(empty_pid);
            g_process_table[empty_pid].state = PROC_EMPTY;
        } else {
            g_tests_passed++;
        }
    }

    kputs("  [PASS] test_process");
    return OK;
}
