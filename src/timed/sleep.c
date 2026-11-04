#include <rio.h>

syscall sleep(uint64 ms) {
    intmask mask = interrupt_save();
    // don't bother with lobby operations
    if (ms == 0) {
        yield();
        interrupt_restore(mask);
        return OK;
    }

    status_code result = lobby_enqueue(&g_delta, curr_pid, ms);
    if (result == ERR) {
        interrupt_restore(mask);
        return OK;
    }

    g_process_table[curr_pid].state = PROC_SLEEP;
    yield();

    interrupt_restore(mask);
    return OK;
}

syscall unsleep(pid id) {
    intmask mask = interrupt_save();
    if (!validpid(id)) {
        interrupt_restore(mask);
        return ERR;
    }

    status_code result = lobby_remove(&g_delta, id);
    if (result == ERR) {
        interrupt_restore(mask);
        return OK;
    }

    resume(id);

    interrupt_restore(mask);
    return OK;
}

