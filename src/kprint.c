/**
 * kprint.c
 *
 * Implements the functions to perform output for the kernel
 *
 * Based on the OSdev wiki
 */

#include "kprint.h"

#include <stdarg.h>
#include "ktypes.h"

// The 16550A UART serial port lives here (for printing text).
static unsigned char *uart = UART_ADDR; 

static inline char to_hex(int8 digit) {
    if (digit < 10) return '0' + digit;
    else return 'a' + (digit - 10);
}

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

status_code kprint(const char *str) {
    if (str == NULL) return ERR;

    while (*str != '\0') {
        kputc(*str);
        str++;
    }

    return OK;
}

/**
 * Prints the given string to the 16550A UART port with a newline at the end.
 *
 * Parameters:
 * const char *str -> string to be printed
 *
 * Returns: status_code
 */

status_code kputs(const char *str) {
    if (kprint(str) == ERR) {
        return ERR;
    }
    kputc('\n');
    return OK;
}

/**
 * vprintf for the kernel space. Called by kprintf
 *
 * based on the implementation from the OSdev wiki
 */

void kvprintf(const char *format, va_list arg) {
    while (*format) {
        if (*format == '%') {
            ++format;
            if (!*format) return;

            switch (*format) {
                case 'i':
                case 'd': {
                    int32 n = va_arg(arg, int);
                    if (n == INT32_MIN) {
                        kprint("-2147483648");
                        break;
                    }
                    if (n < 0) {
                        kputc('-');
                        n = ~n + 1;
                    }

                    char least_sig = '0' + n % 10;
                    n /= 10;
                    char buf[9];
                    char *bptr = buf;
                    while (n) {
                        *bptr++ = '0' + n % 10;
                        n /= 10;
                    }
                    while (bptr != buf) {
                        kputc(*--bptr);
                    }
                    kputc(least_sig);
                }
                break;

                case 'u': {
                    uint32 u = va_arg(arg, uint32);
                    char least_sig = '0' + u % 10;
                    u /= 10;
                    char buf[9];
                    char *bptr = buf;
                    while (u) {
                        *bptr++ = '0' + u % 10;
                        u /= 10;
                    }
                    while (bptr != buf) {
                        kputc(*--bptr);
                    }
                    kputc(least_sig);
                }
                break;

                case 'o': {
                    uint32 u = va_arg(arg, uint32);
                    char least_sig = '0' + u % 8;
                    u /= 8;
                    char buf[10];
                    char *bptr = buf;
                    while (u) {
                        *bptr++ = '0' + u % 8;
                        u /= 8;
                    }
                    while (bptr != buf) {
                        kputc(*--bptr);
                    }
                    kputc(least_sig);
                }
                break;

                case 'x': {
                    uint32 u = va_arg(arg, uint32);
                    char least_sig = to_hex(u % 16);
                    u /= 16;
                    char buf[7];
                    char *bptr = buf;
                    while (u) {
                        *bptr++ = to_hex(u % 16);
                        u /= 16;
                    }
                    while (bptr != buf) {
                        kputc(*--bptr);
                    }
                    kputc(least_sig);
                }
                break;

                case 'p': {
                    kprint("0x");
                    uint64 n = va_arg(arg, uint64);
                    char least_sig = to_hex(n % 16);
                    n /= 16;
                    char buf[7];
                    char *bptr = buf;
                    while (n) {
                        *bptr++ = to_hex(n % 16);
                        n /= 16;
                    }
                    while (bptr != buf) {
                        kputc(*--bptr);
                    }
                    kputc(least_sig);
                }
                break;

                case 'c':
                    kputc(va_arg(arg, int32));
                    break;
                case 's':
                    kprint(va_arg(arg, char *));
                    break;

                default:
                    kputc('%');
                    kputc(*format);
            }
            format++;
        } else {
            kputc(*format++);
        }
    }
}

/**
 * printf for the kernel space
 *
 * based on the implementation from the OSdev wiki
 */

void kprintf(const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    kvprintf(format, arg);
    va_end(arg);
}

