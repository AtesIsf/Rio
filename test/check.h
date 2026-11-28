#ifndef CHECK_H
#define CHECK_H

#include <rio.h>

extern int32 g_pass;
extern int32 g_fail;

void test_lobby(void);
void test_klib(void);
void test_interrupt(void);
void test_process(void);
void test_scheduler(void);
void test_semaphore(void);
void test_timed(void);
void test_memory(void);
void test_vm(void);
void test_kprint(void);

// cond is evaluated once; keep cleanup out of cond so && cannot skip it.
// Passes are silent; only defects are printed.
#define CHECK(name, cond)                                               \
    do {                                                                \
        bool ok_ = (cond);                                              \
        if (ok_) {                                                      \
            g_pass++;                                                   \
        } else {                                                        \
            g_fail++;                                                   \
            kprintf("  [DEFECT] %s (%s:%d)\n", name, __FILE__, __LINE__); \
        }                                                               \
    } while (0)

#endif
