#include <assert.h>

#include "bitbuf.h"

void
bitbuf_init(struct bitbuf *s)
{
        s->buf = 0;
        s->bufoff = 0;
}

void
bitbuf_write(struct bitbuf *s, uint8_t **outpp, uint16_t bits, uint8_t nbits)
{
        unsigned int shift = 32 - s->bufoff - nbits;
        s->buf |= (uint32_t)bits << shift;
        s->bufoff += nbits;
        while (s->bufoff >= 8) {
                *(*outpp)++ = s->buf >> 24;
                s->buf <<= 8;
                s->bufoff -= 8;
        }
}

void
bitbuf_flush(struct bitbuf *s, uint8_t **outpp)
{
        if (s->bufoff > 0) {
                assert(s->bufoff < 8);
                *(*outpp)++ = s->buf >> 24;
        }
}
