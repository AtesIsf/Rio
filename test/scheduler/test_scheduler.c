#include <rio.h>

process sched_dummy(void) {
    return OK;
}

status_code test_scheduler(void) {
    int32 fails = 0;
    pid out = 0;

    // 1. Queue membership bounds checking
    if (sched_queue_includes(-1) || sched_queue_includes(N_PROCS)) {
        kputs("FAIL: sched_queue_includes accepted out-of-bounds pid");
        fails++;
    }
    if (!sched_queue_includes(0)) {
        kputs("FAIL: PID 0 (NULL proc) not found in ready queue");
        fails++;
    }

    // 2. Priority queue ordering
    pid p1 = create(sched_dummy, 20, "p1", 0);
    pid p2 = create(sched_dummy, 40, "p2", 0);
    pid p3 = create(sched_dummy, 30, "p3", 0);

    sched_queue_enqueue(p1, 20);
    sched_queue_enqueue(p2, 40);
    sched_queue_enqueue(p3, 30);

    // Duplicate enqueue must be rejected
    if (sched_queue_enqueue(p1, 20) != ERR) {
        kputs("FAIL: duplicate sched_queue_enqueue did not return ERR");
        fails++;
    }

    sched_queue_dequeue(&out);
    if (out != p2) { kputs("FAIL: expected highest priority p2"); fails++; }
    sched_queue_dequeue(&out);
    if (out != p3) { kputs("FAIL: expected medium priority p3"); fails++; }
    sched_queue_dequeue(&out);
    if (out != p1) { kputs("FAIL: expected lowest priority p1"); fails++; }

    kill(p1);
    kill(p2);
    kill(p3);

    // 3. FIFO ordering for identical priority
    pid a = create(sched_dummy, 25, "a", 0);
    pid b = create(sched_dummy, 25, "b", 0);

    sched_queue_enqueue(a, 25);
    sched_queue_enqueue(b, 25);

    sched_queue_dequeue(&out);
    if (out != a) { kputs("FAIL: FIFO order broken for equal priority (first)"); fails++; }
    sched_queue_dequeue(&out);
    if (out != b) { kputs("FAIL: FIFO order broken for equal priority (second)"); fails++; }

    kill(a);
    kill(b);

    // 4. Removal
    pid rem = create(sched_dummy, 15, "rem", 0);
    sched_queue_enqueue(rem, 15);
    if (sched_queue_remove(rem) != OK) {
        kputs("FAIL: sched_queue_remove failed on valid queued process");
        fails++;
    }
    if (sched_queue_includes(rem)) {
        kputs("FAIL: removed process still included in queue");
        fails++;
    }
    if (sched_queue_remove(rem) != ERR) {
        kputs("FAIL: removing non-queued process did not return ERR");
        fails++;
    }
    kill(rem);

    if (fails == 0) {
        kputs("PASS: test_scheduler");
        return OK;
    }
    return ERR;
}
