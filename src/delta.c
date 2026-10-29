#include <rio.h>

struct LobbyEntry g_delta;

void delta_init() {
    g_delta.next = LOBBY_NULL;
    g_delta.delta = 0;
}

status_code delta_tick() {

    return OK;
}

