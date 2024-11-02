#include <string.h>

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

uint32_t
fletcher32(const void *p, size_t len)
{
        const uint8_t *cp = p;
        uint32_t a;
        uint32_t b;

        a = b = 0;
        while (len > 1) {
                size_t blen = len & ~1;
                if (blen > 360 * 2) {
                        blen = 360 * 2;
                }
                len -= blen;
                do {
                        uint16_t v;
                        memcpy(&v, cp, sizeof(v));
                        a += v;
                        b += a;
                        cp += sizeof(v);
                        blen -= sizeof(v);
                } while (blen > 0);
                a %= 65535;
                b %= 65535;
        }
        if (len > 0) {
                a += *cp;
                b += a;
                a %= 65535;
                b %= 65535;
        }
        return (b << 16) | a;
}
