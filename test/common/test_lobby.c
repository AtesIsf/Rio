#include "check.h"

#define A 61
#define B 62
#define C 63

static void drain(struct LobbyEntry *h) {
    pid id;
    while (lobby_dequeue(h, &id) == OK);
}

static void bad_args(void) {
    struct LobbyEntry h = { LOBBY_NULL, 0 };
    pid id = 0;
    int32 d = 0;

    bool ok = lobby_enqueue(NULL, A, 0) == ERR;
    ok &= lobby_enqueue(&h, -1, 0) == ERR;
    ok &= lobby_enqueue(&h, N_PROCS, 0) == ERR;
    ok &= lobby_enqueue(&h, A, -1) == ERR;
    ok &= lobby_dequeue(NULL, &id) == ERR;
    ok &= lobby_dequeue(&h, NULL) == ERR;
    ok &= lobby_dequeue(&h, &id) == ERR;
    ok &= lobby_peek(&h, &id, NULL) == ERR;
    ok &= lobby_peek(&h, &id, &d) == ERR;
    ok &= lobby_remove(&h, A) == ERR;
    CHECK("lobby_rejects_bad_args", ok);
}

static void fifo(void) {
    struct LobbyEntry h = { LOBBY_NULL, 0 };
    pid id[3] = { -1, -1, -1 };
    pid extra = -1;

    lobby_enqueue(&h, A, 0);
    lobby_enqueue(&h, B, 0);
    lobby_enqueue(&h, C, 0);
    lobby_dequeue(&h, &id[0]);
    lobby_dequeue(&h, &id[1]);
    lobby_dequeue(&h, &id[2]);
    status_code empty = lobby_dequeue(&h, &extra);

    CHECK("lobby_fifo_equal_delta",
          id[0] == A && id[1] == B && id[2] == C && empty == ERR);
}

static void delta_relative(void) {
    struct LobbyEntry h = { LOBBY_NULL, 0 };
    pid id = -1;
    int32 d = -1;

    lobby_enqueue(&h, A, 10);
    lobby_enqueue(&h, B, 5);
    lobby_enqueue(&h, C, 15);
    status_code p1 = lobby_peek(&h, &id, &d);
    bool first = p1 == OK && id == B && d == 5;
    status_code rm = lobby_remove(&h, A);
    pid out = -1;
    lobby_dequeue(&h, &out);
    status_code p2 = lobby_peek(&h, &id, &d);
    bool second = p2 == OK && id == C && d == 10;
    drain(&h);

    CHECK("lobby_delta_relative",
          first && rm == OK && out == B && second);
}

static void duplicate(void) {
    struct LobbyEntry h = { LOBBY_NULL, 0 };
    pid id = -1;

    status_code first = lobby_enqueue(&h, A, 0);
    status_code dup = lobby_enqueue(&h, A, 0);
    status_code one = lobby_dequeue(&h, &id);
    status_code none = lobby_dequeue(&h, &id);

    CHECK("lobby_rejects_duplicate",
          first == OK && dup == ERR && one == OK && none == ERR);
}

void test_lobby(void) {
    bad_args();
    fifo();
    delta_relative();
    duplicate();
}
