# RIO

An operating system for the RISC-V architecture.

Using the following textbooks as guides: 

* "Operating System Design: The Xinu Approach" by Douglar Comer, my
professor for CS354: Operating Systems at Purdue University.

* "xv6: a simple, Unix-like teaching operating system" by Russ Cox, Frans
Kaashoek, and Rober Morris.

## Table of Contents

- [Installation](#installation)
  - [Option 1: Docker / Podman (Recommended, OS-Agnostic)](#option-1-docker--podman-recommended-os-agnostic)
  - [Option 2: Native Installation (Arch Linux)](#option-2-native-installation-arch-linux)
- [Makefile Guide](#makefile-guide)
- [Project Structure](#project-structure)
- [Modules](#modules)
  - [Memory Manager](#memory-manager)
  - [Process Manager](#process-manager)
  - [Interrupt Manager](#interrupt-manager)
  - [CPU Scheduler](#cpu-scheduler)
  - [Semaphore Manager](#semaphore-manager)
  - [Timed Event Manager](#timed-event-manager)
  - [Console Driver](#console-driver)
  - [Hardware & Execution Layers](#hardware--execution-layers)
- [System Architecture](#system-architecture)

## Installation

### Option 1: Docker / Podman (Recommended, OS-Agnostic)

RIO provides a minimal container environment based on `debian:stable-slim` containing the pre-configured 64-bit RISC-V cross-compilation toolchain and QEMU emulator. The `Makefile` automatically detects whether Podman or Docker is installed.

To build the container image:
```bash
make podman-image
# or with docker:
make docker-image
```

### Option 2: Native Installation (Arch Linux)

To compile and run RIO natively on Arch Linux, install the RISC-V bare-metal toolchain, 
GNU Make, and QEMU:

```bash
sudo pacman -S base-devel riscv64-elf-gcc riscv64-elf-binutils qemu-system-riscv
```

Package details:
- `base-devel`: Provides GNU `make` and essential compilation utilities.
- `riscv64-elf-gcc`: GCC cross-compiler targeting 64-bit bare-metal RISC-V.
- `riscv64-elf-binutils`: Assembler and binary utilities for RISC-V (`riscv64-elf-as`).
- `qemu-system-riscv`: QEMU system emulator for RISC-V (`qemu-system-riscv64`).

## Makefile Guide

The provided Makefile supports both native development and containerized development (auto-detecting `podman` or `docker`):

- **Build kernel binary** (`rio.elf`):
  ```bash
  # Native:
  make build

  # Container (Podman / Docker):
  make podman-build   # or: make docker-build
  ```

- **Run in QEMU**:
  Runs the compiled kernel binary (`rio.elf`) inside QEMU using the RISC-V virt machine with direct serial console output.
  ```bash
  # Native:
  make run

  # Container (Podman / Docker):
  make podman-run     # or: make docker-run
  ```
  To exit QEMU, press `Ctrl+A` followed by `X`.

- **Clean build artifacts**:
  Deletes all generated binary files (`bin/*.o`, `bin/*.d`, `rio.elf`).
  ```bash
  # Native:
  make clean

  # Container (Podman / Docker):
  make podman-clean   # or: make docker-clean
  ```

- **Build container image**:
  Builds the minimal Debian-based container image (`rio`):
  ```bash
  make podman-image   # or: make docker-image
  ```

## Project Structure

```text
rio/
├── bin/                       # Output directory for compiled object files (.o)
├── include/                   # Kernel header files
│   ├── clock.h                # Hardware clock definitions and quantum constant
│   ├── delta.h                # Delta list definitions and tick handler prototype
│   ├── hardwaredef.h          # RISC-V hardware definitions and register context
│   ├── interrupt.h            # Interrupt definitions, IRQ codes, and mask functions (save/restore)
│   ├── kmain.h                # Initial main process function prototype
│   ├── kprint.h               # Header declarations for kernel printing functions
│   ├── ktypes.h               # Kernel type definitions, limits, and status codes
│   ├── lobby.h                # Generic process wait queue (lobby) structures and prototypes
│   ├── memory.h               # Memory block and heap allocator definitions and prototypes
│   ├── process.h              # Process table structures, states, and prototypes
│   ├── rio.h                  # Master header aggregating all project headers and common declarations
│   ├── schedqueue.h           # Scheduler ready queue definitions and prototypes
│   ├── scheduler.h            # Scheduler declarations and context switch interface
│   ├── semaphore.h            # Semaphore structures, state definitions, and prototypes
│   └── sleep.h                # Sleep and unsleep system call prototypes
├── src/                       # Kernel assembly and C source files
│   ├── boot.s                 # Assembly startup code
│   ├── clock.c                # Hardware clock initialization (clock_init)
│   ├── cswtch.s               # Assembly context switch routine
│   ├── delta.c                # Delta list initialization and per-tick countdown handler (delta_tick)
│   ├── firstrunwrapper.s      # Assembly wrapper for newly created processes
│   ├── interrupt.c            # Interrupt initialization, handler table, and dispatch logic
│   ├── interruptdispatcher.s  # Assembly interrupt entry point and register save/restore
│   ├── kernel.c               # Kernel entry point (__kentry) and null process loop
│   ├── kmain.c                # Initial main process logic (kmain)
│   ├── kprint.c               # Kernel printing functions and UART communication
│   ├── lobby.c                # Generic process wait queue (lobby) operations (enqueue, dequeue, remove)
│   ├── memory.c               # Memory block allocation for process stacks and heap, and simple heap allocator (chef_malloc)
│   ├── process.c              # Process management (create, resume, kill) and process table
│   ├── schedqueue.c           # Scheduler ready queue operations (enqueue, dequeue, remove)
│   ├── scheduler.c            # Process scheduler implementation (yield)
│   ├── semaphore.c            # Counting semaphore implementation (sem_create, sem_wait, sem_release, sem_free)
│   └── sleep.c                # Process sleep management (sleep, unsleep)
├── .dockerignore              # Ignore rules for container builds
├── compile_flags.txt          # Compiler flags for clangd and language servers
├── Dockerfile                 # Minimal Debian-based container definition
├── LICENSE                    # GNU General Public License v3
├── linker.ld                  # Linker script mapping sections to the RAM base address
├── Makefile                   # Build rules and QEMU execution targets
└── README.md                  # Project documentation and setup instructions
```

## Modules

This section describes each subsystem shown in the architecture flowchart below, including all contributing files and the public API endpoints (functions) exposed by each subsystem.

### Memory Manager
**Files:** `include/memory.h`, `src/memory.c`

| Endpoint | Description |
| :--- | :--- |
| `byte *get_stack(void)` | Allocates a 4 KiB memory block for a process stack and returns the top address (grows downward). |
| `status_code free_stack(byte *addr)` | Deallocates a process stack block, marking its state as free (`MEM_FREE`). |
| `byte *get_heap(void)` | Allocates a 4 KiB memory block to serve as a kernel heap region (`MEM_HEAP`). |
| `status_code free_heap(byte *addr)` | Deallocates a heap memory block, returning it to the pool of free blocks. |
| `void *chef_malloc(byte *data, int32 size)` | Bump allocator allocating an 8-byte aligned chunk of `size` bytes within the given heap block. |

### Process Manager
**Files:** `include/process.h`, `src/process.c`, `src/firstrunwrapper.s`

| Endpoint | Description |
| :--- | :--- |
| `syscall create(void *fn, int32 priority, const char *name, int32 n_args, ...)` | Allocates a stack, configures initial register context and arguments, and sets up a new suspended process (`PROC_SUSP`). Returns PID or `ERR`. |
| `syscall resume(pid id)` | Transitions a process to `PROC_READY`, inserts it into the scheduler ready queue, and triggers preemption (`yield()`). |
| `syscall kill(pid id)` | Terminates a process, deallocates its stack (`free_stack`), removes it from its active queue (ready queue, semaphore lobby, or delta list), and frees its table slot. |

### Interrupt Manager
**Files:** `include/interrupt.h`, `src/interrupt.c`, `src/interruptdispatcher.s`, `include/hardwaredef.h`

| Endpoint | Description |
| :--- | :--- |
| `intmask interrupt_save(void)` | Atomically disables supervisor interrupts (`sstatus.SIE`) and returns previous interrupt mask (enters critical section). |
| `void interrupt_restore(intmask mask)` | Restores supervisor interrupts to the given mask state (exits critical section). |

### CPU Scheduler
**Files:** `include/scheduler.h`, `src/scheduler.c`, `src/cswtch.s`, `include/schedqueue.h`, `src/schedqueue.c`

| Endpoint | Description |
| :--- | :--- |
| `syscall yield(void)` | Core scheduler endpoint; re-enqueues current process if still running, dequeues the highest-priority ready process, and switches context. |
| `syscall scheduler_add(pid id)` | Inserts a process into the scheduler's ready queue according to its priority. |
| `syscall scheduler_remove(pid id)` | Removes a process from the scheduler's ready queue by PID. |

### Semaphore Manager
**Files:** `include/semaphore.h`, `src/semaphore.c`, `include/lobby.h`, `src/lobby.c`

| Endpoint | Description |
| :--- | :--- |
| `syscall sem_create(int32 count, sem *id)` | Allocates an unused semaphore initialized with `count` and returns its descriptor ID. |
| `syscall sem_wait(sem id)` | Decrements semaphore count; if negative, places calling process into the semaphore wait lobby and yields CPU. |
| `syscall sem_release(sem id)` | Increments semaphore count; if waiting processes exist, dequeues the next process from the lobby and resumes it. |
| `syscall sem_free(sem id)` | Deallocates a semaphore and unblocks/resumes all waiting processes. |
| `syscall sem_abort(sem id, pid proc_id)` | Removes a waiting process from a semaphore without granting the resource. |

### Timed Event Manager
**Files:** `include/delta.h`, `src/delta.c`, `include/sleep.h`, `src/sleep.c`

| Endpoint | Description |
| :--- | :--- |
| `syscall sleep(uint64 ms)` | Suspends the calling process for `ms` milliseconds by inserting it into the delta list and calling `yield()`. |
| `syscall unsleep(pid id)` | Cancels a sleeping process's timer early, removing it from the delta list and calling `resume()`. |

### Console Driver
**Files:** `include/kprint.h`, `src/kprint.c`

| Endpoint | Description |
| :--- | :--- |
| `status_code kputc(char c)` | Transmits a single character to the 16550 UART serial transmitter register (`UART_ADDR`). |
| `status_code kprint(const char *str)` | Transmits a null-terminated string to UART by repeatedly calling `kputc`. |
| `status_code kputs(const char *str)` | Transmits a null-terminated string followed by a newline (`\n`) to UART. |
| `void kprintf(const char *format, ...)` | Kernel formatted printing function supporting `%d`, `%i`, `%u`, `%x`, `%p`, `%c`, and `%s`. |
| `void kvprintf(const char *format, va_list arg)` | Core variadic formatted string emitter writing directly to UART. |

### Hardware & Execution Layers
**Files:** `include/clock.h`, `src/clock.c`, `include/hardwaredef.h`, `include/kmain.h`, `src/kmain.c`

| Subsystem / Component | Files | Endpoints / Operations | Description |
| :--- | :--- | :--- | :--- |
| **RISC-V Timer (`stimecmp`)** | `include/clock.h`, `src/clock.c`, `include/hardwaredef.h` | `read_time()`, `write_stimecmp()` | Hardware timer CSR read/write accessors. |
| **16550 UART Serial** | `include/kprint.h` | Memory-mapped I/O (`UART_ADDR`) | Physical 16550 UART hardware chip mapped at address `0x10000000`. |
| **Active Processes** | `include/kmain.h`, `src/kmain.c` | `kmain()` | Entry point for the first kernel process (`kmain`) and subsequent user tasks. |

## System Architecture

The following flowchart illustrates how RIO's core OS abstractions interact with each other:

```mermaid
flowchart TD
    subgraph Hardware ["Hardware Layer"]
        HW_TIMER["RISC-V Timer (stimecmp)"]
        HW_UART["16550 UART Serial"]
    end

    subgraph Core ["Kernel Abstractions"]
        direction TB
        MEM["Memory Manager"]
        PROC["Process Manager"]
        INT["Interrupt Manager"]
        SCHED["CPU Scheduler"]
        SEM["Semaphore Manager"]
        TIMED["Timed Event Manager"]
        CONSOLE["Console Driver"]
    end

    subgraph Exec ["Execution"]
        TASKS["Active Processes (kmain, Tasks)"]
    end

    %% Preemption & Scheduling
    HW_TIMER -->|"Timer interrupt"| INT
    INT -->|"yield"| SCHED
    SCHED -.->|"Context switch"| TASKS

    %% Concurrency & Critical Sections
    SCHED -.->|"interrupt_save, interrupt_restore"| INT
    PROC -.->|"interrupt_save, interrupt_restore"| INT
    SEM -.->|"interrupt_save, interrupt_restore"| INT
    TIMED -.->|"interrupt_save, interrupt_restore"| INT

    %% Process & Memory Interactions
    PROC -->|"get_stack, free_stack"| MEM
    PROC -->|"scheduler_add, yield"| SCHED

    %% Synchronization (Semaphores)
    TASKS -->|"sem_wait, sem_release"| SEM
    SEM -->|"resume"| PROC
    SEM -->|"yield"| SCHED
    PROC -->|"sem_abort"| SEM

    %% Timed Events & Sleep
    TASKS -->|"sleep, unsleep"| TIMED
    INT -.->|"Clock interrupt (delta_tick)"| TIMED
    TIMED -->|"resume"| PROC
    TIMED -->|"yield"| SCHED

    %% Task Execution & I/O
    TASKS -->|"create, resume, kill"| PROC
    TASKS -->|"chef_malloc, get_heap, free_heap"| MEM
    TASKS -->|"kprintf, kputs, kprint"| CONSOLE
    CONSOLE -->|"kputc"| HW_UART
```
