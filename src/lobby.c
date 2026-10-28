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
 * element is LOBBY_NULL or if head or id is NULL, OK otherwise. Should
 * be called with interrupts disabled.
 */

status_code lobby_dequeue(struct LobbyEntry *head, pid *id) {
    if (head == NULL || id == NULL) {
        return ERR;
    }
    pid next_id = head->next;
    *id = next_id;

    if (next_id == LOBBY_NULL) {
        return ERR;
    }

    head->next = lobby[next_id].next;
    lobby[next_id].next = LOBBY_NULL;
    lobby[next_id].delta = 0;
    return OK;
}

/**
 * Enqueues the given id into the given queue. Returns ERR if the id is
 * invalid, the head is NULL, or if the delta is negative. Should be called
 * with interrupts disabled. Semaphores should always enqueue processes with
 * delta = 0 for order preservation!
 */

status_code lobby_enqueue(struct LobbyEntry *head, pid id, int32 delta) {
    if (head == NULL || !validpid(id) || delta < 0) {
        return ERR;
    }

    pid curr = head->next;
    pid prev = LOBBY_NULL;
    // TODO: may want to optimize this later, O(n) semaphore insertion
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

    // adjusts next delta if it exists
    if (curr != LOBBY_NULL) {
        lobby[curr].delta -= delta;
    }

    return OK;
}

/**
 * Takes a peek into the first element in a queue and populates the given
 * parameters with its data. Returns ERR if the first element is LOBBY_NULL
 * or if a parameter is NULL, returns OK otherwise. Should be called
 * with interrupts disabled.
 */

status_code lobby_peek(struct LobbyEntry *head, pid *id, int32 *delta) {
    if (head == NULL || id == NULL || delta == NULL) {
        return ERR;
    }

    *id = head->next;

    if (*id == LOBBY_NULL) {
        *delta = 0;
        return ERR;
    }

    *delta = lobby[head->next].delta;
    return OK;
}

/**
 * Removes the specified process from the given list. Returns ERR if the
 * head is NULL, if the id is invalid, or if the process isn't in this queue.
 * Returns OK Otherwise. Should be called with interrupts disabled.
 */

status_code lobby_remove(struct LobbyEntry *head, pid id) {
    if (head == NULL || !validpid(id)) {
        return ERR;
    }

    pid curr = head->next;
    pid prev = LOBBY_NULL;
    // TODO: may want to optimize this later, O(n) semaphore removal
    while (curr != LOBBY_NULL && curr != id) {
        prev = curr;
        curr = lobby[curr].next;
    }

    if (curr == LOBBY_NULL) {
        return ERR;
    }

    // else -> curr == id
    if (prev == LOBBY_NULL) { // element 1
        head->next = lobby[curr].next;
    } else {
        lobby[prev].next = lobby[curr].next;
    }

    // fix next delta
    if (lobby[curr].next != LOBBY_NULL) {
        lobby[lobby[curr].next].delta += lobby[curr].delta;
    }

    lobby[curr].next = LOBBY_NULL;
    lobby[curr].delta = 0;

    return OK;
}

