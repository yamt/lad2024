#include "chuff_decode.h"

uint8_t
chuff_decode_byte(struct chuff_decode_context *ctx, const uint8_t *tbl,
                  const uint16_t *tblidx)
{
        ctx->chuff_ctx = (uint8_t)huff_decode_sym(
                &ctx->hctx, &tbl[tblidx[ctx->chuff_ctx]]);
        return ctx->chuff_ctx;
}
