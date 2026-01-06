#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "bitbuf.h"

void
bitbuf_init(struct bitbuf *s)
{
        memset(s, 0, sizeof(*s));
}

static void
bitbuf_flush1(struct bitbuf *s, unsigned int thresh)
{
        while (s->bufoff >= thresh) {
                if (s->datalen == s->allocated) {
                        size_t newsize = s->allocated * 2;
                        if (newsize < 64) {
                                newsize = 64;
                        }
                        uint8_t *np = realloc(s->p, newsize);
                        if (np == NULL) {
                                abort();
                        }
                        s->p = np;
                        s->allocated = newsize;
                }
                s->p[s->datalen++] = s->buf >> 24;
                s->buf <<= 8;
                if (s->bufoff > 8) {
                        s->bufoff -= 8;
                } else {
                        s->bufoff = 0;
                }
        }
}

static void
bitbuf_write1(struct bitbuf *s, uint16_t bits, uint8_t nbits)
{
        unsigned int shift = 32 - s->bufoff - nbits;
        s->buf |= (uint32_t)bits << shift;
        s->bufoff += nbits;
        bitbuf_flush1(s, 8);
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
        bitbuf_flush1(s, 1);
}

void
bitbuf_clear(struct bitbuf *s)
{
        free(s->p);
        bitbuf_init(s);
}
