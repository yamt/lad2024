#include <stddef.h>

#if defined(USE_CRANS)
#include "crans_decode.h"
#include "rans_decode.h"

uint8_t
crans_decode_byte(struct crans_decode_context *ctx, const rans_prob_t *tbl,
                  const rans_sym_t *trans, const uint16_t *tblidx)
{
        uint16_t idx = tblidx[ctx->ctx];
        const rans_prob_t *ps = &tbl[idx];
        while (rans_decode_need_more(&ctx->dec) && ctx->nbits >= RANS_B_BITS) {
#if defined(RANS_DECODE_BITS)
                uint16_t bits = bitin_get_bits(&ctx->inp, RANS_B_BITS);
#else
                uint8_t bits = *ctx->inp++;
#endif
                rans_decode_feed(&ctx->dec, bits);
                ctx->nbits -= RANS_B_BITS;
        }
        rans_sym_t ch = rans_decode_sym(&ctx->dec, ps);
        if (trans != NULL) {
                ch = trans[idx + ch];
        }
        ctx->ctx = ch;
        return ch;
}
#endif
