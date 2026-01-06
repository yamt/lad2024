#include "lzhuff.h"
#include "bitbuf.h"

static void
encode_sym(const struct hufftree *h, huff_sym_t ch, struct bitbuf *os)
{
        const uint8_t *sym;
        uint16_t nbits;
        huff_encode_sym(h, ch, &nbits);
        bitbuf_write(os, sym, nbits);
}

static void
out_literal(void *ctx, uint8_t ch)
{
        struct lzhuff *lzh = ctx;
        if (lzh->os == NULL) {
                huff_update_sym(&lzh->huff_lit, ch);
        } else {
                encode_sym(&lzh->huff_lit, ch, lzh->os);
        }
}

static void
out_match(void *ctx, woff_t dist, woff_t len)
{
        struct lzhuff *lzh = ctx;
        if (lzh->os == NULL) {
                huff_update_sym(&lzh->huff_lit, len);
                huff_update_sym(&lzh->huff_dist, dist);
        } else {
                encode_sym(&lzh->huff_lit, len, lzh->os);
                encode_sym(&lzh->huff_dist, dist, lzh->os);
        }
}

void
lzhuff_init(struct lzhuff *lzh)
{
        huff_init(&lzh->huff_lit);
        huff_init(&lzh->huff_dist);
        lz_encode_init(&lzh->lz);
        lzh->lz.out_literal = out_literal;
        lzh->lz.out_match = out_match;
        lzh->lz.out_ctx = lzh;
}

void
lzhuff_update(struct lzhuff *lzh, const void *p, size_t len)
{
        lz_encode(&lzh->lz, p, len);
}

void
lzhuff_build(struct lzhuff *lzh)
{
        lz_encode_flush(&lzh->lz);
        huff_build(&lzh->huff_lit);
        huff_build(&lzh->huff_dist);
}

void
lzhuff_encode_init(struct lzhuff *lzh, struct bitbuf *os)
{
        lz_encode_init(&lzh->lz);
        lzh->os = os;
}

void
lzhuff_encode(struct lzhuff *lzh, const void *p, size_t len)
{
        lz_encode(&lzh->lz, p, len);
}

void
lzhuff_encode_flush(struct lzhuff *lzh)
{
        lz_encode_flush(&lzh->lz);
}
