#include "bitin.h"
#include "rans_decode.h"

struct crans_decode_context {
        struct rans_decode_state dec;
        uint8_t ctx;
#if defined(RANS_DECODE_BITS)
        struct bitin inp;
#else
        const uint8_t *inp;
#endif
        size_t nbits;
};

uint8_t crans_decode_byte(struct crans_decode_context *ctx,
                          const rans_prob_t *tbl, const rans_sym_t *trans,
                          const uint16_t *tblidx);
