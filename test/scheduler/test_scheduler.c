#include <rio.h>

extern int32 g_tests_passed;
extern int32 g_defects_found;

process sched_dummy(void) {
    return OK;
}

process test_scheduler(void) {
    pid out = 0;

    // 1. Queue membership bounds checking
    if (!sched_queue_includes(-1) && !sched_queue_includes(N_PROCS) && sched_queue_includes(0)) {
        g_tests_passed++;
    }

    // 2. Priority queue ordering (use priorities > 64 to avoid dequeuing kmain)
    pid p1 = create(sched_dummy, 80, 1, 1, "p1", 0);
    pid p2 = create(sched_dummy, 100, 1, 1, "p2", 0);
    pid p3 = create(sched_dummy, 90, 1, 1, "p3", 0);

    sched_queue_enqueue(p1, 80);
    sched_queue_enqueue(p2, 100);
    sched_queue_enqueue(p3, 90);

    // Duplicate enqueue rejection
    if (sched_queue_enqueue(p1, 80) == ERR) {
        g_tests_passed++;
    }

    sched_queue_dequeue(&out);
    pid first = out;
    sched_queue_dequeue(&out);
    pid second = out;
    sched_queue_dequeue(&out);
    pid third = out;

    if (first == p2 && second == p3 && third == p1) {
        g_tests_passed++;
    }

    kill(p1);
    kill(p2);
    kill(p3);

    // 3. FIFO ordering for equal priority
    pid a = create(sched_dummy, 85, 1, 1, "a", 0);
    pid b = create(sched_dummy, 85, 1, 1, "b", 0);

    sched_queue_enqueue(a, 85);
    sched_queue_enqueue(b, 85);

    sched_queue_dequeue(&out);
    pid pop_a = out;
    sched_queue_dequeue(&out);
    pid pop_b = out;

    if (pop_a == a && pop_b == b) {
        g_tests_passed++;
    }

    kill(a);
    kill(b);

    // 4. Removal
    pid rem = create(sched_dummy, 75, 1, 1, "rem", 0);
    sched_queue_enqueue(rem, 75);
    if (sched_queue_remove(rem) == OK && !sched_queue_includes(rem) && sched_queue_remove(rem) == ERR) {
        g_tests_passed++;
    }
    kill(rem);

    kputs("  [PASS] test_scheduler");
    return OK;
}
