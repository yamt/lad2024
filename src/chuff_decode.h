#include "huff_decode.h"

struct chuff_decode_context {
        struct huff_decode_context hctx;
        uint8_t chuff_ctx;
};

uint8_t chuff_decode_byte(struct chuff_decode_context *ctx, const uint8_t *tbl,
                          const uint16_t *tblidx);
