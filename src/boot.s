/* Setting up the stack then jumping to the kernel */
/* Taken from the OSDev wiki */

.section .init

.option norvc

.type start, @function
.global start
start:
	.cfi_startproc
	
.option push
.option norelax
	la gp, global_pointer
.option pop
	
	/* Reset satp */
	csrw satp, zero
	
	/* Setup stack */
	la sp, stack_top
	
	/* Clear the BSS section */
	la t5, bss_start
	la t6, bss_end
bss_clear:
	sd zero, (t5)
	addi t5, t5, 8
	bltu t5, t6, bss_clear
	
    /* give S/U modes full RAM access */
    li t0, 0x3fffffffffffff
    csrw pmpaddr0, t0
    li t0, 0x0f
    csrw pmpcfg0, t0

    /* let s-mode handle interrupts */
    li t0, 0xffff
    csrw medeleg, t0
    csrw mideleg, t0

    /* set mstatus.MPP to S-mode */
    csrr t0, mstatus
    li t1, ~(3 << 11)
    and t0, t0, t1
    li t1, (1 << 11)
    or t0, t0, t1
    csrw mstatus, t0

    /* set mepc and jump with mret */
    la t0, kmain
    csrw mepc, t0
    mret
	
	.cfi_endproc

.end
