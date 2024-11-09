#include <assert.h>
#include <stdbool.h>

#include "huff_decode.h"

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

uint8_t
huff_decode_byte(struct huff_decode_context *ctx, const uint8_t *table)
{
	const uint8_t *entry = table;
    while (true) {
        uint8_t bit = huff_get_raw_bit(ctx);
        assert(bit == 0 || bit == 1);
        // printf("decode bit %u\n", bit);
        if (((*entry) & (1 << bit)) != 0) {
            return entry[bit + 1];
        } else {
            uint8_t idx = entry[bit + 1];
            assert(idx != 0);
            entry = &table[3 * idx];
        }
	}
}
