#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#if defined(RANS_DEBUG)
#include <stdio.h>
#endif

#include "rans_common.h"
#include "rans_probs.h"

static size_t
calc_psum(size_t ps[RANS_NSYMS])
{
        size_t psum = 0;
        unsigned int i;
        for (i = 0; i < RANS_NSYMS; i++) {
                psum += ps[i];
        }
        return psum;
}

void
rans_probs_init(struct rans_probs *ps, size_t ops[RANS_NSYMS])
{
        /*
         * scale probabilities in ops[] so that:
         *
         * - sum(ps) = M
         * - each p fits prob_t
         * - the distinction between zero and non-zero is preserved
         */
        size_t psum = calc_psum(ops);
        if (psum == 0) {
                memset(ps->ls, 0, sizeof(ps->ls));
                return;
        }

        unsigned int i;
        for (i = 0; i < RANS_NSYMS; i++) {
                size_t n = (ops[i] * RANS_M + psum - 1) / psum;
                assert((ops[i] > 0) == (n > 0));
                if (n == RANS_M) {
                        n = RANS_M - 1;
                }
                assert(n < RANS_M);
                ops[i] = n;
                assert(ops[i] < RANS_M);
        }

        psum = calc_psum(ops);
        assert(psum > 0);
        if (psum != RANS_M) {
                int diff = RANS_M - psum;
                i = 0;
                while (diff > 0) {
                        if (ops[i] == 0 && ops[i] < RANS_M - 1) {
                                ops[i]++;
                                diff--;
                        }
                        i = (i + 1) % RANS_NSYMS;
                }
                while (diff < 0) {
                        if (ops[i] != 0 && ops[i] > 1) {
                                ops[i]--;
                                diff++;
                        }
                        i = (i + 1) % RANS_NSYMS;
                }
        }
        assert(calc_psum(ops) == RANS_M);

        /*
         * copy to ps->ls
         */
        for (i = 0; i < RANS_NSYMS; i++) {
                assert(ops[i] < RANS_M);
                ps->ls[i] = ops[i];
        }

        for (i = 0; i < RANS_NSYMS; i++) {
                rans_prob_t l_s = ps->ls[i];
                if (l_s == 0) {
                        continue;
                }
#if defined(RANS_DEBUG)
                rans_prob_t b_s = rans_b(ps->ls, i);
                printf("[%02x] l_s=%u, %u-%u Is={%08x,...,%08x}\n", i, l_s,
                       b_s, b_s + l_s - 1, RANS_I_SYM_MIN(l_s),
                       RANS_I_SYM_MAX(l_s));
#endif
        }
#if defined(RANS_DEBUG)
        printf("I={%08x,...,%08x}\n", RANS_I_MIN, RANS_I_MAX);
#endif
}

void
count_syms(size_t counts[RANS_NSYMS], const void *input, size_t inputsize)
{
        const uint8_t *cp = input;
        size_t i;
        for (i = 0; i < inputsize; i++) {
                rans_sym_t sym = cp[i];
                counts[sym]++;
        }
}

void
rans_probs_table_with_trans(const struct rans_probs *ps, rans_prob_t *out,
                            rans_sym_t *trans, size_t *nelemp)
{
        unsigned int nsyms = 0;
        unsigned int i;
        for (i = 0; i < RANS_NSYMS; i++) {
                if (ps->ls[i] > 0) {
                        trans[nsyms] = i;
                        out[nsyms] = ps->ls[i];
                        nsyms++;
                }
        }
        *nelemp = nsyms;
}

void
rans_probs_table(const struct rans_probs *ps, rans_prob_t *out, size_t *nelemp)
{
        unsigned int n = RANS_NSYMS;
        while (true) {
                n--;
                if (ps->ls[n] > 0) {
                        n++;
                        break;
                }
                if (n == 0) {
                        break;
                }
        }
        unsigned int i;
        for (i = 0; i < n; i++) {
                out[i] = ps->ls[i];
        }
        *nelemp = n;
}
