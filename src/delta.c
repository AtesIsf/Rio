#include "lobby.h"
#include <rio.h>

struct LobbyEntry g_delta;

void delta_init() {
    g_delta.next = LOBBY_NULL;
    g_delta.delta = 0;
}

/**
 * Must be only called from within the hardware clock handler, so interrupts
 * are naturally disabled.
 */

status_code delta_tick() {
    pid proc = 0;
    int32 delta = 0;

    status_code result = lobby_peek(&g_delta, &proc, &delta);
    if (result == ERR) {
        return ERR;
    }

    decrement_delta(&g_delta, proc); 
    delta--;
    defer_yield = true;
    bool resumed_procs = false;
    while (delta == 0) {
        lobby_dequeue(&g_delta, &proc);
        // handle sleeping proc
        if (g_process_table[proc].state == PROC_SLEEP) {
            resume(proc);
            resumed_procs = true;
        }

        status_code result = lobby_peek(&g_delta, &proc, &delta);
        if (result == ERR) {
            break;
        }
    }
    defer_yield = false;

    if (resumed_procs) {
        yield();
    }

    return OK;
}

