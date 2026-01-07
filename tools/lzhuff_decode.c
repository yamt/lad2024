#include <assert.h>
#include <string.h>

#include "huff_decode.h"
#include "lz_decode.h"
#include "lzhuff_decode.h"

woff_t
lzhuff_decode_step(struct lz_decode_state *s, struct bitin *in,
                   const uint8_t *huff_lit, const uint8_t *huff_dist,
                   huff_sym_t match_base)
{
        assert(LZ_DECODE_BUF_SIZE - s->endidx >= MATCH_LEN_MAX);
        uint8_t *out = &s->buf[s->endidx];
        huff_sym_t sym = huff_decode_sym(in, huff_lit);
        if (sym >= match_base) {
                woff_t len = sym - match_base + MATCH_LEN_MIN;
                woff_t dist = huff_decode_sym(in, huff_dist);
                dist += MATCH_DISTANCE_MIN;
                assert(dist <= s->endidx);
                lz_apply_match(out, len, dist);
                return len;
        } else {
                *out = sym;
                return 1;
        }
}

huff_sym_t
lzhuff_decode_sym(struct lz_decode_state *s, struct bitin *in,
                  const uint8_t *huff_lit, const uint8_t *huff_dist,
                  huff_sym_t match_base)
{
        assert(s->readidx <= s->endidx);
        assert(s->endidx <= LZ_DECODE_BUF_SIZE);
        if (s->readidx == s->endidx) {
                if (LZ_DECODE_BUF_SIZE - s->endidx < MATCH_LEN_MAX) {
                        assert(MATCH_DISTANCE_MAX < s->endidx);
                        woff_t off = s->endidx - MATCH_DISTANCE_MAX;
                        assert(off > 0);
                        memmove(&s->buf[0], &s->buf[off], MATCH_DISTANCE_MAX);
                        s->readidx = s->endidx = MATCH_DISTANCE_MAX;
                }
                woff_t len = lzhuff_decode_step(s, in, huff_lit, huff_dist,
                                                match_base);
                s->endidx += len;
        }
        return s->buf[s->readidx++];
}
