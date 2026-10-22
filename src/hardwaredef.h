#ifndef HARDWAREDEF_H
#define HARDWAREDEF_H

#include "ktypes.h"

/**
 * There are multiple structs to be defined to ensure compatibility with
 * what RISC-V expects. They are defined in this file.
 */

// Inspired by xv6-riscv. It defines the 64-bit registers for context switches
struct Context {
    uint64 ra; // return address
    uint64 sp; // stack pointer

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

// read the stvec register
static inline uint64 read_stvec() {
    uint64 val;
    __asm__ volatile("csrr %0, stvec" : "=r"(val));
    return val;
}

// write to the stvec register
static inline void write_stvec(uint64 val) {
    __asm__ volatile("csrw stvec, %0" : : "r"(val));
}

// read the sepc register
static inline uint64 read_sepc() {
    uint64 val;
    __asm__ volatile("csrr %0, sepc" : "=r"(val));
    return val;
}

// write to the sepc register
static inline void write_sepc(uint64 val) {
    __asm__ volatile("csrw sepc, %0" : : "r"(val));
}

// read the scause register
static inline uint64 read_scause() {
    uint64 val;
    __asm__ volatile("csrr %0, scause" : "=r"(val));
    return val;
}

// write to the scause register
static inline void write_scause(uint64 val) {
    __asm__ volatile("csrw scause, %0" : : "r"(val));
}

// read the sscratch register
static inline uint64 read_sscratch() {
    uint64 val;
    __asm__ volatile("csrr %0, sscratch" : "=r"(val));
    return val;
}

// write to the sscratch register
static inline void write_sscratch(uint64 val) {
    __asm__ volatile("csrw sscratch, %0" : : "r"(val));
}

// read the sstatus register
static inline uint64 read_sstatus() {
    uint64 val;
    __asm__ volatile("csrr %0, sstatus" : "=r"(val));
    return val;
}

// write to the sstatus register
static inline void write_sstatus(uint64 val) {
    __asm__ volatile("csrw sstatus, %0" : : "r"(val));
}

#endif
