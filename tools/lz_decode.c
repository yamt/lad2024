#include <assert.h>
#include <string.h>

#include "lz_decode.h"

void
lz_decode_init(struct lz_decode_state *s)
{
        memset(s, 0, sizeof(*s));
}

void
lz_decode_apply_match(uint8_t *cur, woff_t len, woff_t dist)
{
        assert(MATCH_LEN_MIN <= len);
        assert(len <= MATCH_LEN_MAX);
        assert(MATCH_DISTANCE_MIN <= dist);
        assert(dist <= MATCH_DISTANCE_MAX);
        const uint8_t *src = cur - dist;
        woff_t i;
        for (i = 0; i < len; i++) {
                cur[i] = src[i];
        }
}

void
lz_decode_clean_window(struct lz_decode_state *s)
{
        if (LZ_DECODE_BUF_SIZE - s->endidx < MATCH_LEN_MAX) {
                assert(MATCH_DISTANCE_MAX < s->endidx);
                woff_t off = s->endidx - MATCH_DISTANCE_MAX;
                assert(off > 0);
                memmove(&s->buf[0], &s->buf[off], MATCH_DISTANCE_MAX);
                s->readidx = s->endidx = MATCH_DISTANCE_MAX;
        }
}
