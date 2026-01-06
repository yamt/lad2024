#include <assert.h>

#include "bitbuf.h"

void
bitbuf_init(struct bitbuf *s, uint8_t *p)
{
        s->p = p;
        s->buf = 0;
        s->bufoff = 0;
}

static void
bitbuf_write1(struct bitbuf *s, uint16_t bits, uint8_t nbits)
{
        unsigned int shift = 32 - s->bufoff - nbits;
        s->buf |= (uint32_t)bits << shift;
        s->bufoff += nbits;
        while (s->bufoff >= 8) {
                *s->p++ = s->buf >> 24;
                s->buf <<= 8;
                s->bufoff -= 8;
        }
}

void
bitbuf_write(struct bitbuf *s, const uint8_t *bits, uint16_t nbits)
{
        while (nbits > 8) {
                bitbuf_write1(s, *bits++, 8);
                nbits -= 8;
        }
        bitbuf_write1(s, *bits, nbits);
}

void
bitbuf_flush(struct bitbuf *s)
{
        if (s->bufoff > 0) {
                assert(s->bufoff < 8);
                *s->p++ = s->buf >> 24;
        }
}
