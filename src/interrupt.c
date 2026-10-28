#include <rio.h>

void *interrupt_handler_table[8];

void interrupts_init() {
    interrupt_handler_table[0] = NULL;
    interrupt_handler_table[1] = handler_clock;
    write_stvec((uint64) dispatch_interrupt);
    write_sstatus(SSTATUS_SIE);
}

/**
 * Handles an interrupt that occurs
 */
status_code handle_interrupt() {
    intmask mask = interrupt_save();

    irq code = get_irq();
    // dereference a function pointer that takes no arguments
    status_code (*handler)();
    handler = interrupt_handler_table[code];
    // don't dereference if NULL
    status_code return_val = handler == NULL ? ERR : handler();

    interrupt_restore(mask);
    return return_val;
}

/**
 * When an interrupt occurs, this function returns the relevant IRQ
 * as defined in interrupt.h
 */
irq get_irq() {
    uint64 cause = read_scause();
    irq code = IRQ_UNKNOWN;
    
    switch (cause) {
    case SCAUSE_TIMER: // clock timer
        code = IRQ_CLOCK;
        break;

    case SCAUSE_EXTERNAL: // external device, TODO: must get plic
        break;

    case SCAUSE_SOFTWARE: // unused for now
        break;
    default:
        kprintf("[Warning] Unknown scause value: %x\n", cause);
    }

    return code;
}

/**
 * Handles a clock interrupt. Every 20ms, a context switch occurs.
 */
status_code handler_clock() {
    static int32 tick_counter = 1;
    tick_counter = (tick_counter + 1) % QUANTUM;

    // TODO: handle per-rick operations like sleeping

    if (tick_counter == 0) {
        write_stimecmp(read_time() + TICK_MS);
        yield();
    }

    return OK;
}

