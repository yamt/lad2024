#include <stdbool.h>

#include "huff_decode.h"
#include "util.h"

void
huff_decode_init(struct huff_decode_context *ctx, const uint8_t *p)
{
        ctx->p = p;
        ctx->bitoff = 0;
}

static uint8_t
huff_get_raw_bit(struct huff_decode_context *ctx)
{
        uint8_t u8 = *ctx->p;
        uint8_t bit = (u8 >> (7 - ctx->bitoff)) & 1;
        ctx->bitoff++;
        if (ctx->bitoff == 8) {
                ctx->bitoff = 0;
                ctx->p++;
        }
        return bit;
}

huff_sym_t
huff_decode_sym(struct huff_decode_context *ctx, const uint8_t *table)
{
        const uint8_t *entry = table;
        while (true) {
                uint8_t bit = huff_get_raw_bit(ctx);
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
