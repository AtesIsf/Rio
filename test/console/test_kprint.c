#include "check.h"

// 16550 registers; loopback (MCR bit 4) routes TX bytes back into RX
#define UART(off) (*(volatile unsigned char *) (UART_ADDR + (off)))
#define FCR 2
#define MCR 4
#define LSR 5
#define FIFO_ON_RESET 0x07
#define LOOPBACK 0x10
#define DATA_READY 0x01
#define FIFO_LEN 16

static intmask cap_begin(void) {
    intmask mask = interrupt_save();
    UART(FCR) = FIFO_ON_RESET;
    UART(MCR) = LOOPBACK;
    return mask;
}

static bool cap_end(intmask mask, const char *want) {
    char got[FIFO_LEN + 1];
    int32 n = 0;
    while (n < FIFO_LEN && (UART(LSR) & DATA_READY)) {
        got[n++] = UART(0);
    }
    got[n] = '\0';
    UART(MCR) = 0;
    UART(FCR) = 0;
    interrupt_restore(mask);

    int32 i = 0;
    while (got[i] != '\0' && got[i] == want[i]) i++;
    return got[i] == want[i];
}

static bool loopback_works(void) {
    intmask m = cap_begin();
    kputc('Z');
    return cap_end(m, "Z");
}

static void null_and_empty(void) {
    bool ok = kprint(nullptr) == ERR;
    ok &= kputs(nullptr) == ERR;
    ok &= kprint("") == OK;
    CHECK("kprint_null_and_empty", ok);
}

static void formats(void) {
    intmask m = cap_begin();
    kputs("ok");
    CHECK("kputs_appends_newline", cap_end(m, "ok\n"));

    m = cap_begin();
    kprintf("%d", INT32_MIN);
    CHECK("kprintf_d_int_min", cap_end(m, "-2147483648"));

    m = cap_begin();
    kprintf("%d|%i", -7, 0);
    CHECK("kprintf_d_i_signs_and_zero", cap_end(m, "-7|0"));

    m = cap_begin();
    kprintf("%u", UINT32_MAX);
    CHECK("kprintf_u_max", cap_end(m, "4294967295"));

    m = cap_begin();
    kprintf("%x", 0xdeadbeef);
    CHECK("kprintf_x", cap_end(m, "deadbeef"));

    m = cap_begin();
    kprintf("%c%s", 'r', "io");
    CHECK("kprintf_c_s", cap_end(m, "rio"));

    m = cap_begin();
    kprintf("%p", (uint64) 0x80001000);
    CHECK("kprintf_p_32bit", cap_end(m, "0x80001000"));
}

// %p stores 14 digits into char buf[7]: stack smash, may crash the kernel
static void p_64bit(void) {
    intmask m = cap_begin();
    kprintf("%p", (uint64) 0x00ffffffffffffff);
    CHECK("kprintf_p_64bit", cap_end(m, "0xffffffffffffff"));
}

void test_kprint(void) {
    null_and_empty();
    if (!loopback_works()) {
        kputs("  [SKIP] console format: no UART loopback");
        return;
    }
    formats();
    p_64bit();
}
