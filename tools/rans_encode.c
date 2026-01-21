/*-
 * Copyright (c)2026 YAMAMOTO Takashi,
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <assert.h>
#if defined(RANS_DEBUG)
#include <math.h>
#include <stdio.h>
#endif

#include "bitbuf.h"
#include "rans_encode.h"

void
rans_encode_init(struct rans_encode_state *st)
{
        st->x = RANS_I_MIN;
}

void
rans_encode_init_zero(struct rans_encode_state *st)
{
        st->x = 0;
}

void
rans_encode_init_with_prob(struct rans_encode_state *st, rans_prob_t l_s)
{
        st->x = RANS_I_SYM_MIN(l_s);
#if defined(RANS_DEBUG)
        printf("enc init with prob %u (x=%08x) %.3f bits (%+.3f)\n", l_s,
               st->x, log2(st->x), log2(st->x) - log2(RANS_I_MIN));
#endif
        assert(st->x <= RANS_I_MAX);
}

void
rans_encode_set_extra(struct rans_encode_state *st, rans_I extra)
{
        assert(st->x <= RANS_I_MAX);
        assert(extra <= RANS_EXTRA_MAX);
#if defined(RANS_DEBUG)
        printf("enc set extra %08x (x=%08x)\n", extra, st->x + extra);
#endif
        st->x += extra;
        assert(st->x <= RANS_I_MAX);
}

static void
encode_normalize(struct rans_encode_state *st, rans_prob_t l_s,
                 struct bitbuf *bo)
{
        rans_I i_sym_max = RANS_I_SYM_MAX(l_s);
        while (st->x > i_sym_max) {
                uint16_t out = (uint16_t)(st->x % RANS_B);
                bitbuf_rev_write(bo, out, RANS_B_BITS);
                rans_I newx = st->x / RANS_B;
#if defined(RANS_DEBUG)
                double oldbits = log2(st->x);
                double newbits = log2(newx);
                double increase = newbits - oldbits;
                printf("enc normalize %08x -> %08x, %.3f bits (%+.3f), out: "
                       "%02x\n",
                       st->x, newx, newbits, increase, out);
                assert(increase < 0);
#endif
                assert(st->x > newx);
                st->x = newx;
        }
        assert(st->x <= RANS_I_SYM_MAX(l_s));
}

void
rans_encode_sym(struct rans_encode_state *st, rans_sym_t s, rans_prob_t b_s,
                rans_prob_t l_s, struct bitbuf *bo)
{
        assert(l_s > 0);
        encode_normalize(st, l_s, bo);
        rans_I q = st->x / l_s;
        rans_I r = st->x - q * l_s;
        rans_I newx = q * RANS_M + b_s + r;
#if defined(RANS_DEBUG)
        double oldbits = log2(st->x);
        double newbits = log2(newx);
        double increase = newbits - oldbits;
        double symbits = log2((double)RANS_M / l_s);
        double error = increase - symbits;
        printf("C(%02x, %08x) -> %08x, %.3f bits (%+.3f err %+.3f), (b_s=%u, "
               "l_s=%u, "
               "x/l_s=%u, "
               "mod(x,l_s)=%u)\n",
               s, st->x, newx, newbits, increase, error, b_s, l_s, q, r);
        assert(st->x == 0 || increase >= 0);
#endif
        assert(st->x <= newx);
        st->x = newx;
        assert(st->x <= RANS_I_MAX);
}

void
rans_encode_flush(struct rans_encode_state *st, struct bitbuf *bo)
{
        while (st->x > 0) {
                uint16_t out = (uint16_t)(st->x % RANS_B);
                bitbuf_rev_write(bo, out, RANS_B_BITS);
                rans_I newx = st->x / RANS_B;
#if defined(RANS_DEBUG)
                double oldbits = log2(st->x);
                double newbits = log2(newx);
                double increase = newbits - oldbits;
                printf("enc flush %08x -> %08x, %.3f bits (%+.3f), out: "
                       "%02x\n",
                       st->x, newx, newbits, increase, out);
                assert(increase < 0);
#endif
                assert(st->x > newx);
                st->x = newx;
        }
}
