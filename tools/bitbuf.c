/*-
 * Copyright (c)2026 YAMAMOTO Takashi,
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

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
bitbuf_output_byte(struct bitbuf *s, uint8_t data)
{
        if (s->datalen == s->allocated) {
                size_t newsize = s->allocated * 2;
                if (newsize < 64) {
                        newsize = 64;
                }
                uint8_t *np = realloc(s->p, newsize);
                if (np == NULL) {
                        abort(); /* XXX */
                }
                s->p = np;
                s->allocated = newsize;
        }
        s->p[s->datalen++] = s->buf >> 24;
}

static void
bitbuf_flush1(struct bitbuf *s, unsigned int thresh)
{
        while (s->bufoff >= thresh) {
                bitbuf_output_byte(s, s->buf >> 24);
                s->buf <<= 8;
                if (s->bufoff > 8) {
                        s->bufoff -= 8;
                } else {
                        s->bufoff = 0;
                }
        }
}

void
bitbuf_write(struct bitbuf *s, uint16_t bits, uint8_t nbits)
{
#if !defined(NDEBUG)
        assert(s->direction >= 0);
        s->direction = 1;
#endif
        /*
         * input: the least significant "nbits" of "bits".
         * output: s->buf is filled from MSBs.
         */
        assert(nbits > 0);
        assert(nbits <= 16);

        /* assert that unused bits are zero */
        assert((bits & (0xffff << nbits)) == 0);
        assert((s->buf & (0xffffffff >> s->bufoff)) == 0);

        unsigned int shift = 32 - s->bufoff - nbits;
        s->buf |= (uint32_t)bits << shift;
        s->bufoff += nbits;
        bitbuf_flush1(s, 8);
}

void
bitbuf_write_multi(struct bitbuf *s, const uint8_t *bits, size_t nbits)
{
#if !defined(NDEBUG)
        assert(s->direction >= 0);
        s->direction = 1;
#endif
        assert(bits != NULL);
        assert(nbits != 0);
        while (nbits > 8) {
                bitbuf_write(s, *bits++, 8);
                nbits -= 8;
        }
        bitbuf_write(s, *bits, nbits);
}

void
bitbuf_flush(struct bitbuf *s)
{
#if !defined(NDEBUG)
        assert(s->direction >= 0);
        s->direction = 1;
#endif
        bitbuf_flush1(s, 1);
}

void
bitbuf_clear(struct bitbuf *s)
{
        free(s->p);
        bitbuf_init(s);
}

/* reversed output */

static void
bitbuf_rev_output_byte(struct bitbuf *s, uint8_t data)
{
        if (s->datalen == s->allocated) {
                size_t newsize = s->allocated * 2;
                if (newsize < 64) {
                        newsize = 64;
                }
                uint8_t *np = realloc(s->p, newsize);
                if (np == NULL) {
                        abort(); /* XXX */
                }
                memmove(np + newsize - s->datalen, np, s->datalen);
                s->p = np;
                s->allocated = newsize;
        }
        s->p[s->allocated - (++s->datalen)] = (uint8_t)s->buf;
}

static void
bitbuf_rev_flush1(struct bitbuf *s, unsigned int thresh)
{
        while (s->bufoff >= thresh) {
                bitbuf_rev_output_byte(s, s->buf & 0xff);
                s->buf >>= 8;
                if (s->bufoff > 8) {
                        s->datalen_bits += 8;
                        s->bufoff -= 8;
                } else {
                        s->datalen_bits += s->bufoff;
                        s->bufoff = 0;
                }
        }
}

void
bitbuf_rev_write(struct bitbuf *s, uint16_t bits, uint8_t nbits)
{
#if !defined(NDEBUG)
        assert(s->direction <= 0);
        s->direction = -1;
#endif
        /*
         * input: the least significant "nbits" of "bits".
         * output: s->buf is filled from LSBs.
         */
        assert(nbits > 0);
        assert(nbits <= 16);

        /* assert that unused bits are zero */
        assert((bits & (0xffff << nbits)) == 0);
        assert((s->buf & (0xffffffff << s->bufoff)) == 0);

        unsigned int shift = s->bufoff;
        s->buf |= (uint32_t)bits << shift;
        s->bufoff += nbits;
        bitbuf_rev_flush1(s, 8);
}

void
bitbuf_rev_flush(struct bitbuf *s)
{
#if !defined(NDEBUG)
        assert(s->direction <= 0);
        s->direction = -1;
#endif
        bitbuf_rev_flush1(s, 1);
        if (s->datalen == 0) {
                return;
        }
        assert((s->datalen_bits + 7) / 8 == s->datalen);
        unsigned int shift = (-s->datalen_bits) % 8;

        /* shift all data to make the first bit byte aligned */
        const uint8_t *cp = &s->p[s->allocated - s->datalen];
        uint8_t *dp = s->p;
        assert(dp <= cp);
        uint8_t next = *cp;
        assert((next & (0xff << (8 - shift))) == 0);
        size_t i;
        for (i = 0; i < s->datalen; i++) {
                uint8_t b = next;
                if (i < s->datalen - 1) {
                        next = cp[1];
                } else {
                        next = 0;
                }
                cp++;
                *dp++ = (b << shift) | (next >> (8 - shift));
        }
        assert(dp == s->p + s->datalen);
        assert((s->datalen_bits % 8) == 0 ||
               (s->p[s->datalen - 1] & (0xff >> (s->datalen_bits % 8))) == 0);
}

#if defined(TEST)
int
main(void)
{
        struct bitbuf b;
        unsigned int i;
        for (i = 0; i <= 256; i++) {
                bitbuf_init(&b);
                unsigned int j;
                for (j = 0; j < i; j++) {
                        bitbuf_rev_write(&b, 1, 1);
                }
                bitbuf_rev_flush(&b);
                assert(b.datalen == (i + 7) / 8);
                assert(b.datalen_bits == i);
                if (b.datalen > 0) {
                        for (j = 0; j < b.datalen - 1; j++) {
                                assert(b.p[j] == 0xff);
                        }
                        unsigned int bits = b.datalen_bits % 8;
                        uint8_t last_byte;
                        if (bits == 0) {
                                last_byte = 0xff;
                        } else {
                                last_byte = 0xff << (8 - bits);
                        }
                        assert(b.p[b.datalen - 1] == last_byte);
                }
                bitbuf_clear(&b);
        }
}
#endif
