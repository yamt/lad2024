#include <stdbool.h>

#include "bitin.h"
#include "huff_decode.h"
#include "util.h"

huff_sym_t
huff_decode_sym(struct bitin *in, const uint8_t *table)
{
        const uint8_t *entry = table;
        while (true) {
                uint8_t bit = bitin_get_bit(in);
                ASSERT(bit == 0 || bit == 1);
                // printf("decode bit %u\n", bit);
                uint8_t flags = *entry;
                uint8_t child_flags = (flags >> (4 * bit)) & 0x0f;
#if HUFF_SYM_BITS > 8
                uint8_t upper = child_flags >> 1;
#else
                ASSERT((child_flags & 0xee) == 0);
                uint8_t upper = 0;
#endif
                uint16_t child_value =
                        (uint16_t)(((uint16_t)upper << 8) | entry[bit + 1]);
                if ((child_flags & 1) != 0) {
                        return (huff_sym_t)child_value;
                } else {
                        ASSERT(child_value != 0);
                        entry = &table[3 * child_value];
                }
        }
}
