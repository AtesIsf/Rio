#include <rio.h>

status_code memset(byte *dest, byte val, uint64 len) {
    if (dest == nullptr) {
        return ERR;
    }

    for (uint64 i = 0; i < len; i++) {
        *dest++ = val;
    }

    return OK;
}

