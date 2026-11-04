# The dispatcher for interrupts
# Created using kernelvec.s from xv6 as a guide
# basically stores registers, handles interrupt, restores registers

.globl handle_interrupt
.global dispatch_interrupt
.align 4 # must be aligned to 2^4 bytes for some stvec hardware reason
dispatch_interrupt:
    addi sp, sp, -144 #must be 2^4=16 byte aligned
    # store caller-saved registers
    sd ra, 0(sp)
    sd gp, 8(sp)
    sd t0, 16(sp)
    sd t1, 24(sp)
    sd t2, 32(sp)
    sd a0, 40(sp)
    sd a1, 48(sp)
    sd a2, 56(sp)
    sd a3, 64(sp)
    sd a4, 72(sp)
    sd a5, 80(sp)
    sd a6, 88(sp)
    sd a7, 96(sp)
    sd t3, 104(sp)
    sd t4, 112(sp)
    sd t5, 120(sp)
    sd t6, 128(sp)
    # since the registers above fill up until 136(sp), I have space
    # for an extra register. Thus, I will handle sepc restore here
    # instead of in handle_interrupt() as before
    csrr t0, sepc
    sd t0, 136(sp)

    call handle_interrupt

    # restore registers from stack
    ld t0, 136(sp)
    csrw sepc, t0

    ld ra, 0(sp)
    ld gp, 8(sp)
    ld t0, 16(sp)
    ld t1, 24(sp)
    ld t2, 32(sp)
    ld a0, 40(sp)
    ld a1, 48(sp)
    ld a2, 56(sp)
    ld a3, 64(sp)
    ld a4, 72(sp)
    ld a5, 80(sp)
    ld a6, 88(sp)
    ld a7, 96(sp)
    ld t3, 104(sp)
    ld t4, 112(sp)
    ld t5, 120(sp)
    ld t6, 128(sp)

    addi sp, sp, 144
    sret
