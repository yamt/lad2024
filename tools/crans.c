#include <assert.h>
#include <string.h>

#include "byteout.h"
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
             struct rans_encode_state *enc, struct byteout *bo)
{
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
