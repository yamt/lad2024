#include <assert.h>
#include <string.h>

#include "byteout.h"
#include "crans.h"
#include "rans_common.h"

void
crans_init(struct crans *ch)
{
        ch->context = 0;
		rans_encode_init(&ch->enc);
}

void
crans_update(struct crans *ch, const uint8_t *p, size_t len)
{
		size_t counts[NSYMS][NSYMS];
        memset(counts, 0, sizeof(counts));

        uint8_t prev = 0; /* XXX */
        const uint8_t *ep = p + len;
        while (p < ep) {
                count_syms(counts[prev], p, 1);
                prev = *p++;
        }
        unsigned int i;
        for (i = 0; i < CRANS_NTABLES; i++) {
                rans_probs_init(&ch->ps[i], counts[i]);
        }
}

void
crans_build(struct crans *ch)
{
}

void
crans_encode(struct crans *ch, const uint8_t *p, size_t len, struct byteout *bo)
{
        size_t i = len;
        while (1) {
                i--;
                uint8_t prev;
                if (i == 0) {
                    prev = 0;
                } else {
                    prev = p[i-1];
                }
                const struct rans_probs *ps = &ch->ps[prev];
                uint8_t sym = p[i];
                prob_t b_s = rans_b(ps->ls, sym);
                prob_t l_s = ps->ls[sym];
                rans_encode_sym(&ch->enc, sym, b_s, l_s, bo);
                if (i == 0) {
                    break;
                }
        }
}

void
crans_table(const struct crans *ch, prob_t *out,
            prob_t *outsp[CRANS_NTABLES], size_t lensp[CRANS_NTABLES])
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
