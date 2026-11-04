#include <rio.h>

static volatile int32 g_worker_step = 0;
static sem g_sync_sem = -1;

process worker_step(void) {
    g_worker_step = 42;
    return OK;
}

process sem_consumer(void) {
    g_worker_step = 1;
    sem_wait(g_sync_sem); // will block because count is 0
    g_worker_step = 2;
    return OK;
}

process wait_forever(void) {
    sem_wait(g_sync_sem);
    return OK;
}

process sleep_forever(void) {
    sleep(100000);
    return OK;
}

status_code test_integration(void) {
    int32 fails = 0;

    // 1. Cooperative process creation, execution, and cleanup
    g_worker_step = 0;
    pid w1 = create(worker_step, 70, "w1", 0);
    if (!validpid(w1)) {
        kputs("FAIL: could not create worker_step process");
        fails++;
    } else {
        resume(w1); // switches context to w1 (prio 70 > kmain 64)
        if (g_worker_step != 42) {
            kputs("FAIL: worker_step process did not execute");
            fails++;
        }
        if (g_process_table[w1].state != PROC_EMPTY) {
            kputs("FAIL: terminated process state is not PROC_EMPTY");
            fails++;
        }
    }

    // 2. Cross-process semaphore blocking and waking
    g_worker_step = 0;
    sem_create(0, &g_sync_sem);
    pid w2 = create(sem_consumer, 70, "w2", 0);
    resume(w2); // runs w2, sets g_worker_step=1, blocks in sem_wait, returns to kmain

    if (g_worker_step != 1) {
        kputs("FAIL: sem_consumer did not set initial step");
        fails++;
    }
    if (g_process_table[w2].state != PROC_WAIT) {
        kputs("FAIL: blocked consumer state is not PROC_WAIT");
        fails++;
    }
    if (g_semaphores[g_sync_sem].count != -1) {
        kputs("FAIL: semaphore count after wait is not -1");
        fails++;
    }

    sem_release(g_sync_sem); // unblocks w2, switches to w2, completes, returns to kmain
    if (g_worker_step != 2) {
        kputs("FAIL: unblocked consumer did not run to completion");
        fails++;
    }
    if (g_process_table[w2].state != PROC_EMPTY) {
        kputs("FAIL: completed consumer was not cleanly terminated");
        fails++;
    }
    sem_free(g_sync_sem);

    // 3. kill() unblocks and adjusts waiting semaphore via sem_abort
    sem_create(0, &g_sync_sem);
    pid w3 = create(wait_forever, 70, "w3", 0);
    resume(w3); // blocks on g_sync_sem (count -1)

    if (g_semaphores[g_sync_sem].count != -1) {
        kputs("FAIL: wait_forever did not decrement sem count");
        fails++;
    }
    kill(w3); // must invoke sem_abort internally and restore count to 0
    if (g_semaphores[g_sync_sem].count != 0) {
        kputs("FAIL: kill did not restore semaphore count to 0 via sem_abort");
        fails++;
    }
    if (g_process_table[w3].state != PROC_EMPTY) {
        kputs("FAIL: killed waiting process state is not PROC_EMPTY");
        fails++;
    }
    sem_free(g_sync_sem);

    // 4. kill() cleans up sleeping process from delta list via sleep_abort
    pid w4 = create(sleep_forever, 70, "w4", 0);
    resume(w4); // sleeps for 100000 ms, returns to kmain

    if (g_process_table[w4].state != PROC_SLEEP) {
        kputs("FAIL: sleeping process state is not PROC_SLEEP");
        fails++;
    }
    kill(w4); // must invoke sleep_abort internally
    if (g_process_table[w4].state != PROC_EMPTY) {
        kputs("FAIL: killed sleeping process state is not PROC_EMPTY");
        fails++;
    }
    if (sleep_abort(w4) != ERR) {
        kputs("FAIL: killed process remained in delta queue");
        fails++;
    }

    if (fails == 0) {
        kputs("PASS: test_integration");
        return OK;
    }
    return ERR;
}
