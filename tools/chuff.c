#include <assert.h>

#include "bitbuf.h"
#include "chuff.h"

void
chuff_init(struct chuff *ch)
{
        unsigned int i;
        for (i = 0; i < CHUFF_NTABLES; i++) {
                huff_init(&ch->trees[i]);
        }
        ch->context = 0;
        ch->ntables = 0;
}

void
chuff_update(struct chuff *ch, const uint8_t *p, size_t len)
{
        const uint8_t *ep = p + len;
        while (p < ep) {
                uint8_t c = *p++;
                huff_update(&ch->trees[ch->context], &c, 1);
                ch->context = c;
                if (ch->ntables < c + 1) {
                        ch->ntables = c + 1;
                }
        }
}

void
chuff_build(struct chuff *ch)
{
        unsigned int i;
        for (i = 0; i < CHUFF_NTABLES; i++) {
                huff_build(&ch->trees[i]);
        }
}

void
chuff_encode(struct chuff *ch, const uint8_t *p, size_t len, struct bitbuf *os,
             uint8_t **outp)
{
        const uint8_t *cp = p;
        const uint8_t *ep = cp + len;
        while (cp < ep) {
                const struct hufftree *tree = &ch->trees[ch->context];
                uint8_t c = *cp++;
                assert(c < ch->ntables);
                uint8_t nbits;
                uint16_t bits = huff_encode_byte(tree, c, &nbits);
                bitbuf_write(os, outp, bits, nbits);
                ch->context = c;
        }
}

void
chuff_table(const struct chuff *ch, uint8_t *out,
            uint8_t *outsp[CHUFF_NTABLES], size_t lensp[CHUFF_NTABLES])
{
        unsigned int i;
        for (i = 0; i < ch->ntables; i++) {
                size_t outlen;
                huff_table(&ch->trees[i], out, &outlen);
                outsp[i] = out;
                lensp[i] = outlen;
                out += outlen;
        }
}
