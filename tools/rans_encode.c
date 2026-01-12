#include <assert.h>
#if defined(RANS_DEBUG)
#include <stdio.h>
#endif

#include "byteout.h"
#include "rans_encode.h"

void
rans_encode_init(struct rans_encode_state *st)
{
        st->x = I_MIN;
}

static void
encode_normalize(struct rans_encode_state *st, sym_t sym, prob_t p_sym,
                 struct byteout *bo)
{
        assert(st->x >= I_SYM_MIN(p_sym));
        I i_sym_max = I_SYM_MAX(p_sym);
        while (st->x > i_sym_max) {
                uint8_t out = (uint8_t)(st->x % B);
                rev_byteout_write(bo, out);
                I newx = st->x / B;
#if defined(RANS_DEBUG)
                printf("enc normalize %08x -> %08x, out: %02x\n", st->x, newx,
                       out);
#endif
                st->x = newx;
        }
        assert(st->x >= I_SYM_MIN(p_sym));
        assert(st->x <= I_SYM_MAX(p_sym));
}

void
rans_encode_sym(struct rans_encode_state *st, sym_t s, prob_t b_s, prob_t l_s,
                struct byteout *bo)
{
        assert(l_s > 0);
        encode_normalize(st, s, l_s, bo);
        I q = st->x / l_s;
        I r = st->x - q * l_s;
        I newx = q * M + b_s + r;
#if defined(RANS_DEBUG)
        printf("C(%02x, %08x) -> %08x (b_s=%u, l_s=%u, x/l_s=%u, "
               "mod(x,l_s)=%u)\n",
               s, st->x, newx, b_s, l_s, q, r);
#endif
        st->x = newx;
        assert(st->x >= I_MIN);
        assert(st->x <= I_MAX);
}

void
rans_encode_flush(struct rans_encode_state *st, struct byteout *bo)
{
        while (st->x > 0) {
                uint8_t out = (uint8_t)(st->x % B);
                rev_byteout_write(bo, out);
                I newx = st->x / B;
#if defined(RANS_DEBUG)
                printf("enc flush %08x -> %08x, out: %02x\n", st->x, newx,
                       out);
#endif
                st->x = newx;
        }
}
