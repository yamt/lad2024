#if defined(RANS_DEBUG)
#include <stdio.h>
#endif

#if defined(RANS_DEBUG)
#include "rans_common.h"
#endif
#include "rans_decode.h"

#if defined(RANS_DECODE_BITS)
#include "bitin.h"
#endif

#if !defined(RANS_DECODE_BITS) && RANS_B_BITS != 8
#error RANS_B_BITS != 8 requires RANS_DECODE_BITS
#endif

void
rans_decode_init(struct rans_decode_state *st)
{
        st->x = 0;
}

/*
 * s(x) in the paper.
 * also calculate b_s and return it via *bp.
 */
static rans_sym_t
find_sym_and_b(const rans_prob_t ls[RANS_NSYMS], rans_I r, rans_prob_t *bp)
{
        RANS_ASSERT(r < RANS_M);
        rans_prob_t b = 0;
        unsigned int i;
        for (i = 0; i < RANS_NSYMS - 1; i++) {
                rans_prob_t p = ls[i];
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
        rans_I newx = st->x * RANS_B + input;
#if defined(RANS_DEBUG)
        printf("dec normalize in=%02x, %08x -> %08x\n", in, st->x, newx);
#endif
        st->x = newx;
}

rans_sym_t
rans_decode_sym(struct rans_decode_state *st, const rans_prob_t ls[RANS_NSYMS])
{
        RANS_ASSERT(st->x <= RANS_I_MAX);
        rans_I q_x_m = st->x / RANS_M;
        rans_I mod_x_m = st->x % RANS_M;
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
