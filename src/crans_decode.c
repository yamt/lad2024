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
        rans_sym_t ch = rans_decode_sym(&ctx->dec, ps, &ctx->inp);
        if (trans != NULL) {
                ch = trans[idx + ch];
        }
        ctx->ctx = ch;
        return ch;
}
#endif
