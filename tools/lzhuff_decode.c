#include <assert.h>
#include <string.h>

#include "huff_decode.h"
#include "lz_decode.h"
#include "lzhuff_decode.h"

woff_t
lzhuff_decode_step(uint8_t *buf, woff_t idx, struct bitin *in,
                   const uint8_t *huff_lit, const uint8_t *huff_dist,
                   huff_sym_t match_base)
{
        assert(LZ_DECODE_BUF_SIZE - idx >= MATCH_LEN_MAX);
        uint8_t *out = &buf[idx];
        huff_sym_t sym = huff_decode_sym(in, huff_lit);
        if (sym >= match_base) {
                woff_t len = sym - match_base + MATCH_LEN_MIN;
                assert(len <= MATCH_LEN_MAX);
                woff_t dist = huff_decode_sym(in, huff_dist);
                assert(dist <= MATCH_DISTANCE_MAX - MATCH_DISTANCE_MIN);
                dist += MATCH_DISTANCE_MIN;
                assert(dist <= idx);
                lz_decode_apply_match(out, len, dist);
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
                lz_decode_clean_window(s);
                woff_t len =
                        lzhuff_decode_step(s->buf, s->endidx, in, huff_lit,
                                           huff_dist, match_base);
                s->endidx += len;
        }
        return s->buf[s->readidx++];
}
