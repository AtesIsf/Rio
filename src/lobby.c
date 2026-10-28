#include <rio.h>

static struct LobbyEntry lobby[N_PROCS];

void lobby_init() {
    for (int32 i = 0; i < N_PROCS; i++) {
        lobby[i].next = LOBBY_NULL;
        lobby[i].delta = 0;
    }
}

/**
 * Dequeues an element from the given queue. Returns ERR if the returned
 * element is LOBBY_NULL or if head is NULL, OK otherwise. Should be called
 * with interrupts disabled.
 */

status_code lobby_dequeue(struct LobbyEntry *head, pid *id) {
    if (head == NULL) {
        return ERR;
    }
    pid next_id = head->next;
    *id = next_id;

    if (next_id == LOBBY_NULL) {
        return ERR;
    }

    head->next = lobby[next_id].next;
    lobby[next_id].next = LOBBY_NULL;
    return OK;
}

/**
 * Enqueues the given id into the given queue. Returns ERR if the id is
 * invalid, the head is NULL, or if the delta is negative. Should be called
 * with interrupts disabled. Semaphores should always enqueue processes with
 * delta = 0 to avoid order preservation!
 */

status_code lobby_enqueue(struct LobbyEntry *head, pid id, int32 delta) {
    if (head == NULL || !validpid(id) || delta < 0) {
        return ERR;
    }

    pid curr = head->next;
    pid prev = LOBBY_NULL;
    // TODO: add delta calculation here!
    while (curr != LOBBY_NULL) {
        if (delta - lobby[curr].delta >= 0) {
            delta -= lobby[curr].delta;
        } else {
            break;
        }
        prev = curr;
        curr = lobby[curr].next;
    }

    // empty queue
    if (prev == LOBBY_NULL) {
        head->next = id;
    } else {
        lobby[prev].next = id;
    }

    lobby[id].next = curr;
    lobby[id].delta = delta;

    return OK;
}

/**
 * Takes a peek into the first element in a queue and populates the given
 * parameters with its data. Returns ERR if the first element is LOBBY_NULL
 * or if head is NULL, returns OK otherwise. Should be called with interrupts
 * disabled.
 */

status_code lobby_peek(struct LobbyEntry *head, pid *id, int32 *delta) {
    if (head == NULL) {
        return ERR;
    }

    *id = head->next;
    *delta = head->delta;

    if (*id == LOBBY_NULL) {
        return ERR;
    }
    return OK;
}

