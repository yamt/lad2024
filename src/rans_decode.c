#include <assert.h>
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
 */
static sym_t
find_sym_and_b(const prob_t ls[NSYMS], I r, prob_t *bp)
{
        assert(r < M);
        prob_t b = 0;
        unsigned int i;
        for (i = 0; i < NSYMS - 1; i++) {
                prob_t p = ls[i];
                if (r < (I)b + p) {
                        break;
                }
                b += p;
        }
        assert(i < NSYMS);
#if defined(RANS_DEBUG)
        assert(b == rans_b(ls, i));
#endif
        *bp = b;
        return (sym_t)i;
}

static void
decode_normalize(struct rans_decode_state *st, const uint8_t **inpp)
{
        while (st->x < I_MIN) {
                uint8_t in = *(*inpp)++;
                I newx = st->x * B + in;
#if defined(RANS_DEBUG)
                printf("dec normalize in=%02x, %08x -> %08x\n", in, st->x,
                       newx);
#endif
                st->x = newx;
        }
        assert(st->x >= I_MIN);
        assert(st->x <= I_MAX);
}

sym_t
rans_decode_sym(struct rans_decode_state *st, const prob_t ls[NSYMS],
                const uint8_t **inpp)
{
        decode_normalize(st, inpp);
        assert(st->x >= I_MIN);
        assert(st->x <= I_MAX);
        I q_x_m = st->x / M;
        I mod_x_m = st->x % M;
        prob_t b_s;
        sym_t s = find_sym_and_b(ls, mod_x_m, &b_s);
        prob_t l_s = ls[s];
        I newx = l_s * q_x_m + mod_x_m - b_s;
#if defined(RANS_DEBUG)
        printf("D(%08x) -> (%02x, %08x) (b_s=%u, l_s=%u, x/m=%u, "
               "mod(x,m)=%u)\n",
               st->x, s, newx, b_s, l_s, q_x_m, mod_x_m);
#endif
        st->x = newx;
        return s;
}
