#if !defined(USE_CRANS)
#include "chuff_decode.h"
#include "huff_decode.h"

uint8_t
chuff_decode_byte(struct chuff_decode_context *ctx, const uint8_t *tbl,
                  const uint16_t *tblidx)
{
        ctx->ctx = (uint8_t)huff_decode_sym(&ctx->in, &tbl[tblidx[ctx->ctx]]);
        return ctx->ctx;
}
#endif
