#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "hardwaredef.h"
#include "ktypes.h"
#include "process.h"

extern pid curr_pid;

// implemented in cswtch.s
extern void cswtch(struct Context *prev, struct Context *next);

syscall yield();

#endif
