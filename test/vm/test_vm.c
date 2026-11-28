#include "check.h"

extern byte text_end; // provided by linker

// distinct VPN[2], VPN[1], VPN[0] (3, 5, 7) expose level mix-ups
#define VA ((byte *) ((3UL << 30) | (5UL << 21) | (7UL << 12)))
#define PA ((byte *) 0x87654000UL) // any aligned PA; test tables are inert
#define PERM (PTE_V | PTE_R | PTE_W | PTE_X | PTE_U)

// zeroed table; stays nullptr if kalloc fails (kmemset rejects nullptr)
static pte_t *new_root(void) {
    pte_t *t = (pte_t *) kalloc(1);
    kmemset((byte *) t, 0, PAGE_SIZE);
    return t;
}

static bool is_empty(pte_t *t) {
    for (uint64 i = 0; i < PAGE_SIZE / sizeof(pte_t); i++) {
        if (t[i] != 0) {
            return false;
        }
    }
    return true;
}

// live root from satp PPN (bits 43:0); nullptr unless it is a RAM page
static pte_t *kernel_root(void) {
    uint64 pa = (read_satp() & ((1UL << 44) - 1)) << 12;
    return pa >= KERNEL_BASE && pa < RAM_END ? (pte_t *) pa : nullptr;
}

static void satp_mode(void) {
    uint64 satp = read_satp();

    CHECK("vm_satp_mode_sv39",
          (satp & (0xFUL << 60)) == VM_SV39 && kernel_root() != nullptr);
}

static void kernel_wx(void) {
    uint64 end = (uint64) &text_end;
    pte_t *k = kernel_root();
    pte_t *t = pte_walk(k, (byte *) (end - PAGE_SIZE), false);
    pte_t *d = pte_walk(k, (byte *) end, false);

    bool ok = t != nullptr && d != nullptr
           && PTE2PA(*t) == end - PAGE_SIZE && PTE2PA(*d) == end
           && (*t & PERM) == (PTE_V | PTE_R | PTE_X)
           && (*d & PERM) == (PTE_V | PTE_R | PTE_W);

    CHECK("vm_kernel_text_data_wx", ok);
}

static void macros(void) {
    uint64 va = (uint64) VA | 0xABC;
    uint64 pa = 0xF0123456789000UL; // 56-bit PA, widest Sv39 allows
    uint64 pte = PA2PTE(pa);

    bool ok = VPN_INDEX(2, va) == 3 && VPN_INDEX(1, va) == 5
           && VPN_INDEX(0, va) == 7
           && VPN_INDEX(2, (1UL << 39) - 1) == 0x1FF
           && PTE2PA(pte) == pa && PTE2PA(pte | 0x3FF) == pa
           && PTE_FLAGS(pte) == 0 && (pte >> 54) == 0
           && PTE_FLAGS(pte | PTE_R | PTE_V) == (PTE_R | PTE_V)
           && PA2PTE(pa | 0xFFF) == pte;

    CHECK("vm_macros_sv39_layout", ok);
}

static void walk_null_paths(void) {
    pte_t *root = new_root();
    pte_t *a = pte_walk(nullptr, VA, true);
    pte_t *b = pte_walk(root, (byte *) (MAX_VIRT_ADDR + 1), true);
    pte_t *c = pte_walk(root, VA, false);

    bool ok = root != nullptr && a == nullptr && b == nullptr
           && c == nullptr && is_empty(root);
    kfree((byte *) root, 1);

    CHECK("pte_walk_null_paths_alloc_nothing", ok);
}

// Sv39: bits 63:39 must equal bit 38, so 1 << 38 is not a valid VA
static void walk_max_va(void) {
    pte_t *root = new_root();
    pte_t *p = pte_walk(root, (byte *) MAX_VIRT_ADDR, true);

    bool ok = root != nullptr && p == nullptr && is_empty(root);
    kfree((byte *) root, 1);

    CHECK("pte_walk_rejects_max_virt_addr", ok);
}

static void walk_alloc(void) {
    pte_t *root = new_root();
    pte_t *p = pte_walk(root, VA, true);
    pte_t *again = pte_walk(root, VA, false);
    pte_t *next = pte_walk(root, VA + PAGE_SIZE, false);

    bool ok = p != nullptr && again == p && next == p + 1
           && is_empty((pte_t *) PAGE_ROUND_DOWN((uint64) p))
           && (root[VPN_INDEX(2, VA)] & PERM) == PTE_V;
    kfree((byte *) root, 1);

    CHECK("pte_walk_alloc_builds_path", ok);
}

static void map_bad_args(void) {
    pte_t *root = new_root();
    status_code a = map_pages(nullptr, VA, PA, 1, PTE_R);
    // distinct pages: a broken check must not reach the remap panic
    status_code b = map_pages(root, VA + PAGE_SIZE / 2, PA, 1, PTE_R);
    status_code c = map_pages(root, VA + PAGE_SIZE, PA + 1, 1, PTE_R);

    bool ok = root != nullptr && a == ERR && b == ERR && c == ERR
           && is_empty(root);
    kfree((byte *) root, 1);

    CHECK("map_pages_rejects_bad_args", ok);
}

static void map_zero_pages(void) {
    pte_t *root = new_root();
    // OK vs ERR for n == 0 is undocumented; only "maps nothing" is asserted
    (void) map_pages(root, VA, PA, 0, PTE_R);

    bool ok = root != nullptr && is_empty(root);
    kfree((byte *) root, 1);

    CHECK("map_pages_zero_pages_maps_nothing", ok);
}

static void map_exact(void) {
    pte_t *root = new_root();
    status_code r = map_pages(root, VA, PA, 2, PTE_R | PTE_W);
    pte_t *p = pte_walk(root, VA, false);

    uint64 rw = PTE_V | PTE_R | PTE_W;
    bool ok = r == OK && p != nullptr
           && PTE2PA(p[0]) == (uint64) PA && (p[0] & PERM) == rw
           && PTE2PA(p[1]) == (uint64) PA + PAGE_SIZE
           && (p[1] & PERM) == rw
           && p[-1] == 0 && p[2] == 0;
    kfree((byte *) root, 1);

    CHECK("map_pages_maps_exactly_n", ok);
}

void test_vm(void) {
    satp_mode();
    kernel_wx();
    macros();
    walk_null_paths();
    walk_max_va();
    walk_alloc();
    map_bad_args();
    map_zero_pages();
    map_exact();
}
