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

#if defined(RANS_DEBUG)
#include <stdio.h>
#endif

#if defined(RANS_DEBUG)
#include "rans_common.h"
#endif
#include "rans_decode.h"

void
rans_decode_init(struct rans_decode_state *st)
{
        st->x = 0;
}

/*
 * s(x) in the paper.
 * also calculate b_s and return it via *bp.
 *
 * this is a simple and dumb implementation.
 * in case you want to optimize for speed, it's trivial to use a table,
 * especially for a small RANS_M.
 */
static rans_sym_t
find_sym_and_b(const rans_prob_t ls[RANS_NSYMS], rans_prob_t r,
               rans_prob_t *bp)
{
        rans_prob_t b = 0;
        unsigned int i;
        for (i = 0; i < RANS_NSYMS - 1; i++) {
                rans_prob_t p = ls[i];
                /*
                 * b+p can be up to RANS_M, which might not fit rans_prob_t.
                 */
                if (r < (rans_I)b + p) {
                        break;
                }
                b += p;
        }
        RANS_ASSERT(i < RANS_NSYMS);
#if defined(RANS_DEBUG)
        RANS_ASSERT(b == rans_b(ls, i));
#endif
        *bp = b;
        return (rans_sym_t)i;
}

bool
rans_decode_need_more(const struct rans_decode_state *st)
{
        return st->x < RANS_I_MIN;
}

void
rans_decode_feed(struct rans_decode_state *st, uint16_t input)
{
        RANS_ASSERT((input & (0xffff << RANS_B_BITS)) == 0);
        rans_I newx = st->x * RANS_B + input;
#if defined(RANS_DEBUG)
        printf("dec normalize in=%02x, %08x -> %08x\n", input, st->x, newx);
#endif
        st->x = newx;
}

rans_sym_t
rans_decode_sym(struct rans_decode_state *st, const rans_prob_t ls[RANS_NSYMS])
{
        RANS_ASSERT(st->x <= RANS_I_MAX);
        rans_I q_x_m = st->x / RANS_M;
        rans_prob_t mod_x_m = (rans_prob_t)(st->x % RANS_M);
        rans_prob_t b_s;
        rans_sym_t s = find_sym_and_b(ls, mod_x_m, &b_s);
        rans_prob_t l_s = ls[s];
        rans_I newx = l_s * q_x_m + mod_x_m - b_s;
#if defined(RANS_DEBUG)
        printf("D(%08x) -> (%02x, %08x) (b_s=%u, l_s=%u, x/m=%u, "
               "mod(x,m)=%u)\n",
               st->x, s, newx, b_s, l_s, q_x_m, mod_x_m);
#endif
        st->x = newx;
        return s;
}

rans_I
rans_decode_get_extra(struct rans_decode_state *st)
{
        RANS_ASSERT(st->x <= RANS_I_MAX);
        return st->x - RANS_I_MIN;
}
