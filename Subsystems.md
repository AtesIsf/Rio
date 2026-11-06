# Subsystems

This document describes each subsystem shown in the architecture flowchart, including all contributing files and the public API endpoints (functions) exposed by each subsystem.

## Table of Contents

- [Memory Manager](#memory-manager)
- [Process Manager](#process-manager)
- [Interrupt Manager](#interrupt-manager)
- [CPU Scheduler](#cpu-scheduler)
- [Semaphore Manager](#semaphore-manager)
- [Timed Event Manager](#timed-event-manager)
- [Console Driver](#console-driver)

## Memory Manager
**Files:** `include/memory.h`, `src/memory/memory.c`

| Endpoint | Description |
| :--- | :--- |
| `syscall get_stack(byte *top, int32 n)` | Allocates `n` contiguous 4 KiB pages for a process stack. Returns `OK` or `ERR`. |
| `syscall free_stack(byte *top, int32 n)` | Deallocates `n` contiguous 4 KiB stack pages starting at `top`. Returns `OK` or `ERR`. |
| `syscall get_heap(byte *start, int32 n)` | Allocates `n` contiguous 4 KiB pages to serve as a heap region. Returns `OK` or `ERR`. |
| `syscall free_heap(byte *start, int32 n)` | Deallocates `n` contiguous 4 KiB heap pages starting at `start`. Returns `OK` or `ERR`. |

## Process Manager
**Files:** `include/process.h`, `src/process/process.c`, `src/process/firstrunwrapper.s`

| Endpoint | Description |
| :--- | :--- |
| `syscall create(void *fn, int32 priority, uint32 n_stack_pages, uint32 n_heap_pages, const char *name, int32 n_args, ...)` | Allocates stack and heap pages, configures initial register context and arguments, and sets up a new suspended process (`PROC_SUSP`). Returns PID or `ERR`. |
| `syscall resume(pid id)` | Transitions a process to `PROC_READY`, inserts it into the scheduler ready queue, and triggers preemption (`yield()`). |
| `syscall kill(pid id)` | Terminates a process, deallocates its stack (`free_stack`), removes it from its active queue (ready queue, semaphore lobby, or delta list), and frees its table slot. |

## Interrupt Manager
**Files:** `include/interrupt.h`, `src/interrupt/interrupt.c`, `src/interrupt/interruptdispatcher.s`, `include/hardwaredef.h`

| Endpoint | Description |
| :--- | :--- |
| `intmask interrupt_save(void)` | Atomically disables supervisor interrupts (`sstatus.SIE`) and returns previous interrupt mask (enters critical section). |
| `void interrupt_restore(intmask mask)` | Restores supervisor interrupts to the given mask state (exits critical section). |

## CPU Scheduler
**Files:** `include/scheduler.h`, `src/scheduler/scheduler.c`, `src/scheduler/cswtch.s`, `include/schedqueue.h`, `src/scheduler/schedqueue.c`

| Endpoint | Description |
| :--- | :--- |
| `syscall yield(void)` | Core scheduler endpoint; re-enqueues current process if still running, dequeues the highest-priority ready process, and switches context. |
| `syscall scheduler_add(pid id)` | Inserts a process into the scheduler's ready queue according to its priority. |
| `syscall scheduler_remove(pid id)` | Removes a process from the scheduler's ready queue by PID. |

## Semaphore Manager
**Files:** `include/semaphore.h`, `src/semaphore/semaphore.c`

| Endpoint | Description |
| :--- | :--- |
| `syscall sem_create(int32 count, sem *id)` | Allocates an unused semaphore initialized with `count` and returns its descriptor ID. |
| `syscall sem_wait(sem id)` | Decrements semaphore count; if negative, places calling process into the semaphore wait lobby and yields CPU. |
| `syscall sem_release(sem id)` | Increments semaphore count; if waiting processes exist, dequeues the next process from the lobby and resumes it. |
| `syscall sem_free(sem id)` | Deallocates a semaphore and unblocks/resumes all waiting processes. |
| `syscall sem_abort(sem id, pid proc_id)` | Removes a waiting process from a semaphore without granting the resource. |

## Timed Event Manager
**Files:** `include/delta.h`, `src/timed/delta.c`, `include/sleep.h`, `src/timed/sleep.c`

| Endpoint | Description |
| :--- | :--- |
| `syscall sleep(uint64 ms)` | Suspends the calling process for `ms` milliseconds by inserting it into the delta list and calling `yield()`. |
| `syscall unsleep(pid id)` | Cancels a sleeping process's timer early, removing it from the delta list and calling `resume()`. |
| `syscall sleep_abort(pid id)` | Removes a sleeping process from the delta list without resuming it (used by `kill`). |

## Console Driver
**Files:** `include/kprint.h`, `src/console/kprint.c`

| Endpoint | Description |
| :--- | :--- |
| `status_code kputc(char c)` | Transmits a single character to the 16550 UART serial transmitter register (`UART_ADDR`). |
| `status_code kprint(const char *str)` | Transmits a null-terminated string to UART by repeatedly calling `kputc`. |
| `status_code kputs(const char *str)` | Transmits a null-terminated string followed by a newline (`\n`) to UART. |
| `void kprintf(const char *format, ...)` | Kernel formatted printing function supporting `%d`, `%i`, `%u`, `%x`, `%p`, `%c`, and `%s`. |
| `void kvprintf(const char *format, va_list arg)` | Core variadic formatted string emitter writing directly to UART. |
