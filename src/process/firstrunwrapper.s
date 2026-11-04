# Since standard cswtch saves registers s0-s11, we need a wrapper to ensure
# these registers actually end up as the function arguments when a process
# is run for the first time.
#
# void first_run_wrapper()
#

.globl first_run_wrapper
first_run_wrapper:
    # load saved registers into the argument slots
    mv a0, s1
    mv a1, s2
    mv a2, s3
    mv a3, s4
    mv a4, s5
    mv a5, s6
    mv a6, s7
    mv a7, s8

    # globally defined constant for the final return address of a proc
    la ra, procend

    # this is where the actual frame pointer is
    jr s0

    ret
