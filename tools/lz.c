#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lz.h"

static woff_t
win_start(const struct lz_encode_state *s)
{
        if (s->curoff > MATCH_DISTANCE_MAX) {
                return s->curoff - MATCH_DISTANCE_MAX;
        }
        return 0;
}

/* include the curoff */
static woff_t
valid_forward_data(const struct lz_encode_state *s)
{
        assert(s->valid_size >= s->curoff);
        return s->valid_size - s->curoff;
}

static woff_t
lookahead_size(const struct lz_encode_state *s)
{
        woff_t sz = valid_forward_data(s);
        if (sz > MATCH_LEN_MAX) {
                return MATCH_LEN_MAX;
        }
        return sz;
}

static woff_t
bufidx(const struct lz_encode_state *s, woff_t off)
{
        return (s->bufstart + off) % LZ_ENCODE_BUF_SIZE;
}

static uint8_t
data_at(const struct lz_encode_state *s, woff_t off)
{
        assert(off < LZ_ENCODE_BUF_SIZE);
        return s->buf[bufidx(s, off)];
}

static woff_t
find_match(struct lz_encode_state *s, woff_t *posp)
{
        woff_t matchlen = 0;
        woff_t matchpos = 0;
        woff_t i;
        woff_t la_size = lookahead_size(s);
        for (i = win_start(s); i < s->curoff; i++) {
                woff_t mlen = 0;
                while (mlen < la_size &&
                       data_at(s, s->curoff + mlen) == data_at(s, i + mlen)) {
                        mlen++;
                }
                /* note: prefer smaller distance */
                if (mlen >= MATCH_LEN_MIN && mlen >= matchlen) {
                        matchlen = mlen;
                        matchpos = i;
                }
        }
        assert(matchlen == 0 || matchlen >= MATCH_LEN_MIN);
        assert(matchlen <= MATCH_LEN_MAX);
        *posp = matchpos;
        return matchlen;
}

static bool
fill_part(struct lz_encode_state *s, woff_t off, woff_t len, const void **pp,
          size_t *lenp)
{
        size_t sz = len;
        if (sz > *lenp) {
                sz = *lenp;
        }
        memcpy(&s->buf[off], *pp, sz);
        *pp = (const uint8_t *)*pp + sz;
        *lenp -= sz;
        s->valid_size += sz;
        return sz != 0;
}

static void
fill_buffer(struct lz_encode_state *s, const void **pp, size_t *lenp)
{
        /* forget the out of window data */
        woff_t off = win_start(s);
        if (off > 0) {
                s->bufstart = bufidx(s, off);
                s->valid_size -= off;
                s->curoff -= off;
        }

        if (s->valid_size == LZ_ENCODE_BUF_SIZE) {
                return;
        }

        /* read as much as possible */
        off = bufidx(s, s->valid_size);
        if (s->bufstart <= off) {
                if (fill_part(s, off, LZ_ENCODE_BUF_SIZE - off, pp, lenp)) {
                        fill_part(s, 0, s->bufstart, pp, lenp);
                }
        } else {
                fill_part(s, off, s->bufstart - off, pp, lenp);
        }
        assert(s->valid_size <= LZ_ENCODE_BUF_SIZE);
        assert(s->valid_size == LZ_ENCODE_BUF_SIZE || *lenp == 0);
}

static void
lz_encode_impl(struct lz_encode_state *s, const void *p, size_t len,
               bool flushing)
{
        /* "lazy matching" described in RFC 1951 */
        bool lazy_matching = true;

        while (true) {
                if (flushing) {
                        if (valid_forward_data(s) == 0) {
                                break;
                        }
                } else {
                        woff_t needed = MATCH_LEN_MAX + lazy_matching;
                        if (valid_forward_data(s) < needed) {
                                fill_buffer(s, &p, &len);
                        }
                        if (valid_forward_data(s) < needed) {
                                break;
                        }
                }
                woff_t mpos;
                woff_t mlen = find_match(s, &mpos);
                if (lazy_matching && mlen != 0) {
                        s->curoff++;
                        woff_t mpos2;
                        woff_t mlen2 = find_match(s, &mpos2);
                        s->curoff--;
                        if (mlen2 > mlen) {
                                mlen = 0;
                        }
                }
                if (mlen == 0) {
                        s->out_literal(s->out_ctx, data_at(s, s->curoff));
                        s->curoff++;
                } else {
                        assert(s->curoff > mpos);
                        assert(s->curoff - mpos <= MATCH_DISTANCE_MAX);
                        s->out_match(s->out_ctx, s->curoff - mpos, mlen);
                        s->curoff += mlen;
                }
        }
        assert(len == 0);
}

void
lz_encode_init(struct lz_encode_state *s)
{
        memset(s, 0, sizeof(*s));
}

void
lz_encode(struct lz_encode_state *s, const void *p, size_t len)
{
        lz_encode_impl(s, p, len, false);
}

void
lz_encode_flush(struct lz_encode_state *s)
{
        lz_encode_impl(s, NULL, 0, true);
        s->curoff = s->valid_size = 0;
}
