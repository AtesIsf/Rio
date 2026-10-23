#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "hardwaredef.h"
#include "ktypes.h"

/**
 * In RISC-V, there are 3 types of interrupts (traps):
 * 1) system call -> user program uses ecall instruction
 * 2) exception: illegal instruction
 * 3) device interrupt
 */

/**
 * Saves and returns the mask of the current interrupt state. Basically
 * Xinu's disable().
 */
static inline intmask interrupt_save() {
    intmask mask;
    __asm__ volatile("csrrc %0, sstatus, %1"
                     : "=r"(mask)
                     : "r"(SSTATUS_SIE)
                     : "memory");
    return mask;
}

/**
 * Sets the interrupt state to the given mask. Basically Xinu's restore().
 */
static inline void interrupt_restore(intmask mask) {
    if (mask & SSTATUS_SIE) {
        // sets SIE if the bit is one in the mask
        __asm__ volatile("csrs sstatus, %0"
                         :
                         : "r"(SSTATUS_SIE)
                         : "memory");
    } else {
        // clears SIE if the bit is zero in the mask
        __asm__ volatile("csrc sstatus, %0"
                         :
                         : "r"(SSTATUS_SIE)
                         : "memory");
    }
}

#endif
