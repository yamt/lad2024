#include <assert.h>
#if defined(RANS_DEBUG)
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
rans_encode_set_extra(struct rans_encode_state *st, rans_I extra)
{
        assert(st->x >= RANS_I_MIN);
        assert(st->x <= RANS_I_MAX);
#if defined(RANS_DEBUG)
        printf("enc set extra %08x + %08x -> %08x\n", st->x, extra,
               st->x + extra);
#endif
        assert(extra <= RANS_EXTRA_MAX);
        st->x += extra;
        assert(st->x >= RANS_I_MIN);
        assert(st->x <= RANS_I_MAX);
}

static void
encode_normalize(struct rans_encode_state *st, rans_sym_t sym, rans_prob_t l_s,
                 struct bitbuf *bo)
{
        assert(st->x >= RANS_I_SYM_MIN(l_s));
        rans_I i_sym_max = RANS_I_SYM_MAX(l_s);
        while (st->x > i_sym_max) {
                uint16_t out = (uint16_t)(st->x % RANS_B);
                bitbuf_rev_write(bo, out, RANS_B_BITS);
                rans_I newx = st->x / RANS_B;
#if defined(RANS_DEBUG)
                printf("enc normalize %08x -> %08x, out: %02x\n", st->x, newx,
                       out);
#endif
                st->x = newx;
        }
        assert(st->x >= RANS_I_SYM_MIN(l_s));
        assert(st->x <= RANS_I_SYM_MAX(l_s));
}

void
rans_encode_sym(struct rans_encode_state *st, rans_sym_t s, rans_prob_t b_s,
                rans_prob_t l_s, struct bitbuf *bo)
{
        assert(l_s > 0);
        encode_normalize(st, s, l_s, bo);
        rans_I q = st->x / l_s;
        rans_I r = st->x - q * l_s;
        rans_I newx = q * RANS_M + b_s + r;
#if defined(RANS_DEBUG)
        printf("C(%02x, %08x) -> %08x (b_s=%u, l_s=%u, x/l_s=%u, "
               "mod(x,l_s)=%u)\n",
               s, st->x, newx, b_s, l_s, q, r);
#endif
        st->x = newx;
        assert(st->x >= RANS_I_MIN);
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
                printf("enc flush %08x -> %08x, out: %02x\n", st->x, newx,
                       out);
#endif
                st->x = newx;
        }
}
