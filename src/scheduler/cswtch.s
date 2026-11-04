
# Context switch code based off of xv6-riscv, with interrupt flag
# saving like Xinu, since I use the Xinu process model
#
# void cswtch(struct Context *prev, struct Context *next)
# save into prev, load from next

# Only callee-saved registers are saved here because the RISC-V
# convention states that if a non-callee register must be saved, the
# compiler must store those variables in the stack

.globl cswtch
cswtch:
    # *((*prev) + offset) = register
    sd ra, 0(a0) 
    sd sp, 8(a0) 
    csrr t0, sstatus
    sd t0, 16(a0)
    sd s0, 24(a0) 
    sd s1, 32(a0) 
    sd s2, 40(a0) 
    sd s3, 48(a0) 
    sd s4, 56(a0) 
    sd s5, 64(a0) 
    sd s6, 72(a0) 
    sd s7, 80(a0) 
    sd s8, 88(a0) 
    sd s9, 96(a0) 
    sd s10, 104(a0) 
    sd s11, 112(a0) 

    # register = *((*next) + offset)
    ld ra, 0(a1) 
    ld sp, 8(a1) 
    ld t0, 16(a1)
    csrw sstatus, t0
    ld s0, 24(a1) 
    ld s1, 32(a1) 
    ld s2, 40(a1) 
    ld s3, 48(a1) 
    ld s4, 56(a1) 
    ld s5, 64(a1) 
    ld s6, 72(a1) 
    ld s7, 80(a1) 
    ld s8, 88(a1) 
    ld s9, 96(a1) 
    ld s10, 104(a1) 
    ld s11, 112(a1) 

    # the new return address is the ra that comes from the next
    # Context. thus, this function simply returns to the next proc
    ret

