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

/*
 * bit-stream output buffer
 *
 * the format of the output bit stream: within a byte, MSB are first.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct bitbuf {
        /* small buffer to concat bits together */
        uint32_t buf;
        unsigned int bufoff;

        /* byte output. */
        uint8_t *p;
        size_t datalen;
        size_t allocated;
        size_t datalen_bits;

#if !defined(NDEBUG)
        int direction;
#endif
};

void bitbuf_init(struct bitbuf *s);
/*
 * bitbuf_write: append the least significant "nbits" of "bits".
 *
 * bitbuf_write_multi: append multiple bytes.
 * if the last byte for bitbuf_write_multi is partial, it's written as
 * it would be with bitbuf_write(s, last_byte, nbits % 8). that is,
 * the LSBs of the last byte was written. note that it's different
 * from the output bit stream, where the last partial byte is filled
 * from MSBs.
 */
void bitbuf_write(struct bitbuf *s, uint16_t bits, uint8_t nbits);
void bitbuf_write_multi(struct bitbuf *s, const uint8_t *bits, size_t nbits);
void bitbuf_flush(struct bitbuf *s);
void bitbuf_clear(struct bitbuf *s);

/*
 * the following "rev" variants are used to produce reversed stream.
 * (eg. for rANS)
 * "rev" api and the corresponding non-rev api should not be used
 * intermixed on a bitbuf instance.
 *
 * bits within a single bitbuf_rev_write call are NOT reversed.
 *
 * bitbuf_rev_flush arranges the data so that the first bit is
 * byte-aligned. ie. the first bit will be the MSB of *s->p.
 */
void bitbuf_rev_write(struct bitbuf *s, uint16_t bits, uint8_t nbits);
void bitbuf_rev_flush(struct bitbuf *s);
