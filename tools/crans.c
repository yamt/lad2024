#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "bitbuf.h"
#include "crans.h"
#include "rans_common.h"

void
crans_init(struct crans *ch)
{
        ch->context = 0;
        ch->ntables = 0;
        memset(ch->counts, 0, sizeof(ch->counts));
}

void
crans_update(struct crans *ch, const uint8_t *p, size_t len)
{
        if (len == 0) {
                return;
        }
        const uint8_t *ep = p + len;
        while (p < ep) {
                count_syms(ch->counts[ch->context], p, 1);
                ch->context = *p++;
                if (ch->ntables <= ch->context) {
                        ch->ntables = ch->context + 1;
                }
        }
}

void
crans_build(struct crans *ch)
{
        unsigned int i;
        for (i = 0; i < CRANS_NTABLES; i++) {
                rans_probs_init(&ch->ps[i], ch->counts[i]);
        }
}

void
crans_encode(struct crans *ch, const uint8_t *p, size_t len,
             struct rans_encode_state *enc, bool need_rans_init,
             struct bitbuf *bo)
{
        if (len == 0) {
                return;
        }
        size_t i = len;
        while (1) {
                i--;
                uint8_t prev;
                if (i == 0) {
                        prev = 0;
                } else {
                        prev = p[i - 1];
                }
                const struct rans_probs *ps = &ch->ps[prev];
                uint8_t sym = p[i];
                rans_prob_t b_s = rans_b(ps->ls, sym);
                rans_prob_t l_s = ps->ls[sym];
                if (need_rans_init) {
                        enc->x = RANS_I_SYM_MIN(l_s);
                        need_rans_init = false;
                }
                rans_encode_sym(enc, sym, b_s, l_s, bo);
                if (i == 0) {
                        break;
                }
        }
}

void
crans_table(const struct crans *ch, rans_prob_t *out,
            rans_prob_t *outsp[CRANS_NTABLES], size_t lensp[CRANS_NTABLES])
{
        unsigned int i;
        for (i = 0; i < CRANS_NTABLES; i++) {
                size_t outlen;
                rans_probs_table(&ch->ps[i], out, &outlen);
                outsp[i] = out;
                lensp[i] = outlen;
                out += outlen;
        }
}

void
crans_table_with_trans(const struct crans *ch, rans_prob_t *out,
                       rans_sym_t trans[CRANS_NTABLES],
                       rans_prob_t *outsp[CRANS_NTABLES],
                       size_t lensp[CRANS_NTABLES])
{
        unsigned int i;
        for (i = 0; i < CRANS_NTABLES; i++) {
                size_t outlen;
                rans_probs_table_with_trans(&ch->ps[i], out, trans, &outlen);
#if 0
                printf("crans_table_with_trans [0x%02x] %zu\n", i, outlen);
                unsigned int j;
                for (j = 0; j < outlen; j++) {
                        printf("\t0x%02x\t%u (%.2f%%)\n", trans[j], out[j],
                               (double)100 * out[j] / RANS_M);
                }
#endif
                outsp[i] = out;
                lensp[i] = outlen;
                out += outlen;
                trans += outlen;
        }
}

void
crans_table_with_shared_trans(const struct crans *ch, rans_prob_t *out,
                              rans_prob_t *outsp[CRANS_NTABLES],
                              size_t lensp[CRANS_NTABLES], rans_sym_t *trans,
                              size_t *nsymsp)
{
        /*
         * note: we can't simply use rans_probs_table_with_trans
         * because we want to have a single shared trans table,
         * not per rans_sym_t.
         */

        bool used[RANS_NSYMS];
        memset(used, 0, sizeof(used));
        unsigned int nsyms = 0;
        unsigned int i;
        for (i = 0; i < CRANS_NTABLES; i++) {
                unsigned int j;
                for (j = 0; j < RANS_NSYMS; j++) {
                        if (ch->ps[j].ls[i] > 0) {
                                used[i] = true;
                                nsyms++;
                                break;
                        }
                }
        }
        unsigned int cur = 0;
        for (i = 0; i < CRANS_NTABLES; i++) {
                if (!used[i]) {
                        continue;
                }
                trans[cur] = i;
                outsp[cur] = out;
                lensp[cur] = nsyms;
                unsigned int j;
                for (j = 0; j < RANS_NSYMS; j++) {
                        rans_prob_t l_s = ch->ps[i].ls[j];
                        if (used[j]) {
                                *out++ = l_s;
                        } else {
                                assert(l_s == 0);
                        }
                }
                cur++;
        }
        assert(cur == nsyms);
        *nsymsp = nsyms;
}
