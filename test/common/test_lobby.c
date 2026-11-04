#include <rio.h>

status_code test_lobby(void) {
    int32 fails = 0;
    struct LobbyEntry head = { .next = LOBBY_NULL, .delta = 0 };
    pid id = 0;
    int32 delta = 0;

    lobby_init();

    // 1. Dequeue from empty queue
    if (lobby_dequeue(&head, &id) != ERR) {
        kputs("FAIL: lobby_dequeue on empty did not return ERR");
        fails++;
    }

    // 2. Null pointer checks
    if (lobby_dequeue(NULL, &id) != ERR || lobby_dequeue(&head, NULL) != ERR) {
        kputs("FAIL: lobby_dequeue null checks failed");
        fails++;
    }
    if (lobby_enqueue(NULL, 1, 0) != ERR) {
        kputs("FAIL: lobby_enqueue null head did not return ERR");
        fails++;
    }

    // 3. Invalid arguments
    if (lobby_enqueue(&head, -1, 0) != ERR || lobby_enqueue(&head, N_PROCS, 0) != ERR) {
        kputs("FAIL: lobby_enqueue invalid pid did not return ERR");
        fails++;
    }
    if (lobby_enqueue(&head, 1, -10) != ERR) {
        kputs("FAIL: lobby_enqueue negative delta did not return ERR");
        fails++;
    }

    // 4. FIFO order preservation for delta = 0 (semaphore requirement)
    lobby_init();
    head.next = LOBBY_NULL;
    lobby_enqueue(&head, 2, 0);
    lobby_enqueue(&head, 3, 0);
    lobby_enqueue(&head, 4, 0);

    lobby_dequeue(&head, &id);
    if (id != 2) { kputs("FAIL: lobby FIFO pop 1 != 2"); fails++; }
    lobby_dequeue(&head, &id);
    if (id != 3) { kputs("FAIL: lobby FIFO pop 2 != 3"); fails++; }
    lobby_dequeue(&head, &id);
    if (id != 4) { kputs("FAIL: lobby FIFO pop 3 != 4"); fails++; }

    // 5. Delta sorted order (sleep delta queue requirement)
    lobby_init();
    head.next = LOBBY_NULL;
    lobby_enqueue(&head, 2, 10);
    lobby_enqueue(&head, 3, 5);
    lobby_enqueue(&head, 4, 15);

    lobby_peek(&head, &id, &delta);
    if (id != 3 || delta != 5) { kputs("FAIL: lobby delta peek 1 failed"); fails++; }
    lobby_dequeue(&head, &id);

    lobby_peek(&head, &id, &delta);
    if (id != 2 || delta != 5) { kputs("FAIL: lobby delta peek 2 failed"); fails++; }
    lobby_dequeue(&head, &id);

    lobby_peek(&head, &id, &delta);
    if (id != 4 || delta != 5) { kputs("FAIL: lobby delta peek 3 failed"); fails++; }
    lobby_dequeue(&head, &id);

    // 6. Remove node and maintain accumulated delta
    lobby_init();
    head.next = LOBBY_NULL;
    lobby_enqueue(&head, 2, 10);
    lobby_enqueue(&head, 3, 20); // relative delta is 10
    lobby_remove(&head, 2);      // removing 2 must add its delta (10) to 3 -> 20

    lobby_peek(&head, &id, &delta);
    if (id != 3 || delta != 20) {
        kputs("FAIL: lobby_remove did not fix successor delta");
        fails++;
    }

    if (fails == 0) {
        kputs("PASS: test_lobby");
        return OK;
    }
    return ERR;
}
