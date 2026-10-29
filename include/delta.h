#ifndef DELTA_H
#define DELTA_H

#include "ktypes.h"
#include "lobby.h"

extern struct LobbyEntry g_delta;

void delta_init();

status_code delta_tick();

#endif
