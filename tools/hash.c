#include "hash.h"

uint32_t
sdbm_hash(const void *p, size_t len)
{
        const uint8_t *cp = p;
        uint32_t hash = 0;
        size_t i;
        for (i = 0; i < len; i++) {
                hash = hash * 65599 + cp[i];
        }
        return hash;
}
