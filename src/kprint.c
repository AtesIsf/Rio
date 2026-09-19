/**
 * kprint.c
 *
 * Implements the functions to perform output for the kernel
 *
 * Based on the OSdev wiki
 */

#include "kprint.h"

#include "ktypes.h"

// The 16550A UART serial port lives here (for printing text).
static unsigned char *uart = UART_ADDR; 

/**
 * Prints the given character to the 16550A UART port.
 *
 * Parameters:
 * char c -> char to be printed
 *
 * Returns: status_code
 */

status_code kputc(char c) {
    *uart = c;
    return OK;
}

/**
 * Prints the given string to the 16550A UART port.
 *
 * Parameters:
 * const char *str -> string to be printed
 *
 * Returns: status_code
 */

status_code kputs(const char *str) {
    if (str == NULL) return ERR;

    const char *c = &str[0];
    while (*c != '\0') {
        kputc(*c);
        c++;
    }
    kputc('\n');

    return OK;
}

