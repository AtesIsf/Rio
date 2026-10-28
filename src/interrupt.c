#include "rio.h"

void *interrupt_handler_table[8];

status_code init_interrupt_table() {
    interrupt_handler_table[0] = NULL;
    interrupt_handler_table[1] = handler_clock;
    return OK;
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
        code = IRQ_TIMER;
        break;

    case SCAUSE_EXTERNAL: // external device, TODO: must get plic
        break;

    case SCAUSE_SOFTWARE: // unused for now
        break;
    default:
        kprintf("[Warning] Uknown scause value: %x\n", cause);
    }

    return code;
}

/**
 * TODO: Handles a clock interrupt
 */
status_code handler_clock() {
    return OK;
}

