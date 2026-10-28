#ifndef LOBBY_H
#define LOBBY_H

/**
 * The Lobby is a singly linked list of processes that are in some sort
 * of waiting state, wheather that be for a semaphore or sleeping.
 *
 * Both the semaphore and deltalist subsystem can use this data structure
 * for their back end since a process cannot be both sleeping and waiting.
 */

#include <ktypes.h>

// the index will by the pid
struct LobbyEntry {
    pid next;
    int32 delta; // irrelevant for semaphores, so can modify this freely
};

#define LOBBY_NULL (-1)

void lobby_init();

status_code lobby_dequeue(struct LobbyEntry *head, pid *id);

status_code lobby_enqueue(struct LobbyEntry *head, pid id, int32 delta);

status_code lobby_peek(struct LobbyEntry *head, pid *id, int32 *delta);

#endif
