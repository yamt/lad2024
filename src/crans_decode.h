#include "rans_decode.h"

struct crans_decode_context {
        struct rans_decode_state dec;
        uint8_t ctx;
        const uint8_t *inp;
};

uint8_t crans_decode_byte(struct crans_decode_context *ctx,
                          const rans_prob_t *tbl, const rans_sym_t *trans,
                          const uint16_t *tblidx);
