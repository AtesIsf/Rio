#ifndef HARDWAREDEF_H
#define HARDWAREDEF_H

#include "ktypes.h"

/**
 * There are multiple structs to be defined to ensure compatibility with
 * what RISC-V expects. They are defined in this file.
 */

// on first run, interrupts are enabled and the function returns to s-mode
#define INITIAL_PROCESS_SSTATUS (SSTATUS_SIE | SSTATUS_SPP)

// Inspired by xv6-riscv. It defines the 64-bit registers for context switches
struct Context {
    uint64 ra; // return address
    uint64 sp; // stack pointer
    uint64 sstatus; // interrupt flags, required since I use the Xinu system

    // s registers are saved in risc-v
    uint64 s0; // frame pointer
    uint64 s1;
    uint64 s2;
    uint64 s3;
    uint64 s4;
    uint64 s5;
    uint64 s6;
    uint64 s7;
    uint64 s8;
    uint64 s9;
    uint64 s10;
    uint64 s11;
};

/* Interrupt Hardware Definitions */

/**
 * I used the xv6 textbook to learn about these. I prefer to use "interrupt"
 * instead of "trap" though. The helper functions below are to be used by
 * interrupt.c and interrupt.h.
 *
 * **stvec**: hardware register where the kernel places the address of the
 * interrupt handler code. RISC-V jumps here to handle the interrupt.
 *
 * **sepc**: When an interrupt occurs, RISC-V stores the pc here since it is
 * overwritten afterwards. The sret instruction copies sepc into pc. The
 * kernel can write to sepc to control where sret goes.
 *
 * **scause**: RISC-V writes the reason for the interrupt here.
 *
 * **sscratch**: The kernel interrupt handler code uses this register to not
 * overwrite user registers before saving them.
 *
 * **sstatus**: The SIE bit (Supervisor Interrupt Enable) controls whether
 * device interrupts are enabled. If the kernel clears SIE, RISC-V will defer
 * interrupts until the kernel sets the SIE. The SPP bit (Supervisor Previous
 * Priviledge) indicates whether the interrupt came from User mode or
 * Supervisor mode, and controls to what mode sret returns to.
 */

/* definitions from xv6 */
#define SSTATUS_SPP  (1L << 8) // Previous mode, 1 = Supervisor, 0 = User
#define SSTATUS_SPIE (1L << 5) // Supervisor Previous Interrupt Enable
#define SSTATUS_UPIE (1L << 4) // User Previous Interrupt Enable
#define SSTATUS_SIE  (1L << 1) // Supervisor Interrupt Enable
#define SSTATUS_UIE  (1L << 0) // User Interrupt Enable

/* scause Definitions, from page 71 of the Priviledged ISA pdf */
#define SCAUSE_SOFTWARE (0x8000000000000001L) // Supervisor software interrupt
#define SCAUSE_TIMER    (0x8000000000000005L) // Supervisor timer interrupt
#define SCAUSE_EXTERNAL (0x8000000000000009L) // Supervisor external interrupt

static inline uint64 read_stvec() {
    uint64 val;
    __asm__ volatile("csrr %0, stvec" : "=r"(val));
    return val;
}

static inline void write_stvec(uint64 val) {
    __asm__ volatile("csrw stvec, %0" : : "r"(val));
}

static inline uint64 read_sepc() {
    uint64 val;
    __asm__ volatile("csrr %0, sepc" : "=r"(val));
    return val;
}

static inline void write_sepc(uint64 val) {
    __asm__ volatile("csrw sepc, %0" : : "r"(val));
}

static inline uint64 read_scause() {
    uint64 val;
    __asm__ volatile("csrr %0, scause" : "=r"(val));
    return val;
}

static inline void write_scause(uint64 val) {
    __asm__ volatile("csrw scause, %0" : : "r"(val));
}

static inline uint64 read_sscratch() {
    uint64 val;
    __asm__ volatile("csrr %0, sscratch" : "=r"(val));
    return val;
}

static inline void write_sscratch(uint64 val) {
    __asm__ volatile("csrw sscratch, %0" : : "r"(val));
}

static inline uint64 read_sstatus() {
    uint64 val;
    __asm__ volatile("csrr %0, sstatus" : "=r"(val));
    return val;
}

static inline void write_sstatus(uint64 val) {
    __asm__ volatile("csrw sstatus, %0" : : "r"(val));
}

/**
 * I used the xv6 textbook for these again. The definitions below are to
 * use the hardware clock of risc-v.
 *
 * Registers for the hardware clock:
 *
 * **time**: contains the timer that the hardware increments in a steady rate.
 * Understandably, there is no helper to write to this register
 *
 * **stimecmp**: contains the timestamp for the next clock interrupt
 */

static inline uint64 read_time() {
    uint64 val;
    __asm__ volatile("csrr %0, time" : "=r"(val));
    return val;
} 

static inline uint64 read_stimecmp() {
    uint64 val;
    __asm__ volatile("csrr %0, stimecmp" : "=r"(val));
    return val;
}

static inline void write_stimecmp(uint64 val) {
    __asm__ volatile("csrw stimecmp, %0" : : "r"(val));
}

#endif
