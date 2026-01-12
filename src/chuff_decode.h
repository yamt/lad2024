#include "bitin.h"

struct chuff_decode_context {
        struct bitin in;
        uint8_t ctx;
};

uint8_t chuff_decode_byte(struct chuff_decode_context *ctx, const uint8_t *tbl,
                          const uint16_t *tblidx);
