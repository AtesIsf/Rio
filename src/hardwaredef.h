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

#endif
