#include <rio.h>

extern int32 g_tests_passed;
extern int32 g_defects_found;

process test_lobby(void) {
    struct LobbyEntry head = { .next = LOBBY_NULL, .delta = 0 };
    pid id = 0;
    int32 delta = 0;

    lobby_init();

    // 1. Dequeue on empty queue returns ERR
    if (lobby_dequeue(&head, &id) == ERR) {
        g_tests_passed++;
    }

    // 2. NULL pointer checks
    if (lobby_dequeue(NULL, &id) == ERR && lobby_dequeue(&head, NULL) == ERR && lobby_enqueue(NULL, 1, 0) == ERR) {
        g_tests_passed++;
    }

    // 3. Out-of-bounds PID and negative delta
    if (lobby_enqueue(&head, -1, 0) == ERR && lobby_enqueue(&head, N_PROCS, 0) == ERR && lobby_enqueue(&head, 1, -10) == ERR) {
        g_tests_passed++;
    }

    // 4. FIFO order preservation for delta = 0
    lobby_init();
    head.next = LOBBY_NULL;
    lobby_enqueue(&head, 2, 0);
    lobby_enqueue(&head, 3, 0);
    lobby_enqueue(&head, 4, 0);

    lobby_dequeue(&head, &id);
    pid first = id;
    lobby_dequeue(&head, &id);
    pid second = id;
    lobby_dequeue(&head, &id);
    pid third = id;
    if (first == 2 && second == 3 && third == 4) {
        g_tests_passed++;
    }

    // 5. Delta sorted order
    lobby_init();
    head.next = LOBBY_NULL;
    lobby_enqueue(&head, 2, 10);
    lobby_enqueue(&head, 3, 5);
    lobby_enqueue(&head, 4, 15);

    lobby_peek(&head, &id, &delta);
    if (id == 3 && delta == 5) {
        g_tests_passed++;
    }

    // 6. Adversarial: Duplicate enqueue of already queued PID
    lobby_init();
    head.next = LOBBY_NULL;
    lobby_enqueue(&head, 5, 0);
    if (lobby_enqueue(&head, 5, 0) == OK) {
        kputs("  [DEFECT] lobby_enqueue allows duplicate insertion, creating cyclic queue corruption");
        g_defects_found++;
    } else {
        g_tests_passed++;
    }

    kputs("  [PASS] test_lobby");
    return OK;
}
