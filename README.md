# RIO

An operating system for the RISC-V architecture, written in C23.

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

The provided Makefile supports both native and containerized development (auto-detecting `podman` or `docker`). Any native target can be run inside a container via `podman-<target>` or `docker-<target>`:

- **Build kernel binary** (`rio.elf`):
  ```bash
  make build          # Container: make podman-build / make docker-build
  ```

- **Run in QEMU**:
  Runs the compiled kernel binary (`rio.elf`) inside QEMU using the RISC-V virt machine with direct serial console output. Press `Ctrl+A` followed by `X` to exit.
  ```bash
  make run            # Container interactive shell: make podman-run
  ```

- **Run test suite** (`rio-test.elf`):
  Builds and runs the bare-metal test suite headlessly in QEMU, self-terminating via the SiFive test finisher device upon completion.
  ```bash
  make test           # Container: make podman-test / make docker-test
  make test-build     # Compile test binary without running
  ```

- **Clean build artifacts**:
  Deletes compiled binaries (`rio.elf`, `rio-test.elf`) and object files in `bin/` while preserving directory tracking.
  ```bash
  make clean          # Container: make podman-clean / make docker-clean
  ```

- **Build container image**:
  Builds the minimal Debian-based container image (`rio`):
  ```bash
  make podman-image   # or: make docker-image
  ```

## System Architecture

The following flowchart illustrates how RIO's core OS abstractions interact with each other (see [Subsystems.md](Subsystems.md) for detailed public API endpoints and subsystem documentation):

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
    HW_TIMER -.->|"Timer interrupt"| INT
    INT -->|"yield"| SCHED
    SCHED -.->|"Context switch"| TASKS

    %% Concurrency & Critical Sections
    SCHED -->|"interrupt_save, interrupt_restore"| INT
    PROC -->|"interrupt_save, interrupt_restore"| INT
    SEM -->|"interrupt_save, interrupt_restore"| INT
    TIMED -->|"interrupt_save, interrupt_restore"| INT

    %% Process & Memory Interactions
    PROC -->|"get_stack, free_stack, get_heap, free_heap"| MEM
    PROC -->|"scheduler_add, yield"| SCHED

    %% Synchronization (Semaphores)
    TASKS -->|"sem_wait, sem_release"| SEM
    SEM -->|"resume"| PROC
    SEM -->|"yield"| SCHED
    PROC -->|"sem_abort"| SEM

    %% Timed Events & Sleep
    TASKS -->|"sleep, unsleep"| TIMED
    INT -->|"Clock interrupt (delta_tick)"| TIMED
    TIMED -->|"resume"| PROC
    TIMED -->|"yield"| SCHED
    PROC -->|"sleep_abort"| TIMED

    %% Task Execution & I/O
    TASKS -->|"create, resume, kill"| PROC
    TASKS -->|"kprintf, kputs, kprint"| CONSOLE
    CONSOLE -->|"kputc"| HW_UART
```
