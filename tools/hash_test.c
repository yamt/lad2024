#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"

void
test(const char *data, uint32_t expected)
{
        size_t len = strlen(data);
        uint32_t hash = fletcher32(data, len);
        printf("fletcher32(%s) => %" PRIx32 " (expected %" PRIx32 ")\n", data,
               hash, expected);
        if (hash != expected) {
                abort();
        }
}

int
main(void)
{
        /* https://en.wikipedia.org/wiki/Fletcher's_checksum#Test_vectors */
        test("abcde", 0xf04fc729);
        test("abcdef", 0x56502d2a);
        test("abcdefgh", 0xebe19591);
}
